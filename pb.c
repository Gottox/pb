/*
 * BSD 2-Clause License
 * 
 * Copyright (c) 2018, Enno Boland
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this 
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "pb.h"

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

struct Row {
	int id;
	short int progress;
	char *msg;
	struct Row *next;
};

static FILE *tty = NULL;
static struct winsize ws;
static int next_id = 1024;
static struct Row *rows = NULL;
static pthread_mutex_t mutex = { 0 };

static void
pb_cut() {
	struct Row *r, *last = NULL;
	short int i;

	for (r = rows, i = 0; r && i < ws.ws_row - 1 ; i++) {
		last = r;
		r = r->next;
	}
	for (; last && last->next;) {
		r = last->next;
		last->next = r->next;
		free(r->msg);
		free(r);
	}
}

static void
pb_draw_bar(const int progress) {
	const static unsigned short offset = 30;
	const long width = ws.ws_col - offset - 30;
	unsigned long i;

	fprintf(tty,
			"\x1b[%uG"         /* go to column %i (CHA) */
			" [", offset);
	for (i = 0; i < width; i++) {
		fputc(progress * width > i * 100 ? '#' : ' ', tty);
	}
	fprintf(tty,
			"] "
			"\x1b[K"           /* clear line from cursor to end (EL) */
			"% 3i%%",
			progress);
}

static void
pb_draw_row(const struct Row *row) {
	fputs("\x1b[K", tty);   /* clear line from cursor to end (EL) */
	fputs(row->msg, tty);
	if (row->progress >= 0) {
		pb_draw_bar(row->progress);
	}
	/* make sure the line is terminated. terminal resizing
	 * looks nicer if it does. */
	fputs(
			"\n"
			"\x1b[A",           /* One row up (CUU) */
			tty);
}

static void
pb_draw(const struct Row *row) {
	struct Row *r;
	fputs(
			"\x1b[s"            /* save cursor position */
			"\x1b[?7;25l"       /* disable row wrap and cursor (DECRST) */
			"\r",               /* row start */
			tty);
	for (r = rows; r; r = r->next) {
		fputs("\x1b[A", tty); /* one 1 row up */
		if (row == NULL) {
			pb_draw_row(r);
		} else if (row == r) {
			pb_draw_row(r);
			break;
		}
	}
	fputs(
			"\x1b[u"            /* restore cursor position */
			"\x1b[?7;25h",       /* enable row wrap and cursor (DECSET)*/
			tty);
}

static void
pb_sigwinch(int sig) {
	if (ioctl(fileno(tty), TIOCGWINSZ, &ws) != -1) {
		pb_cut();
		pb_draw(NULL);
	}
}

int
pb_clean() {
	struct sigaction sa = { 0 };
	struct Row *r;

	for (; (r = rows);) {
		rows = r->next;
		free(r->msg);
		free(r);
	}
	tty = NULL;
	sa.sa_handler = SIG_DFL;
	return pthread_mutex_destroy(&mutex) == 0
		&& sigaction(SIGWINCH, &sa, NULL) != -1 ? 0 : -1;
}

int
pb_init() {
	struct sigaction sa = { 0 };

	if (!isatty(fileno(stderr)) || strcmp(getenv("TERM"), "dumb") == 0
			|| pthread_mutex_init(&mutex, NULL) != 0
			|| sigaction(SIGWINCH, &sa, NULL) == -1) {
		return -1;
	}
	tty = stderr;
	sa.sa_handler = pb_sigwinch;
	pb_sigwinch(0);
	return 0;
}

static struct Row *
pb_get_row(const int id) {
	struct Row *r;

	if (id) {
		for (r = rows; r; r = r->next) {
			if (r && r->id == id) {
				free(r->msg);
				return r;
			}
		}
	}
	r = calloc(sizeof(struct Row), 1);
	r->id = next_id++;
	r->next = rows;
	rows = r;
	fputc('\n', tty);       /* Reserve a new row */
	pb_cut();
	return r;
}

int
pb(int *id, const int progress, const char *fmt, ...) {
	int rv;
	va_list args;

	va_start (args, fmt);
	rv = vpb(id, progress, fmt, args);
	va_end(args);
	return rv;
}

int
vpb(int *id, const int progress, const char *fmt, va_list args) {
	int rv = 0;
	struct Row *r;

	pthread_mutex_lock(&mutex);
	if (tty == NULL) {
		rv = vfprintf(stderr, fmt, args);
		if (progress != -1) {
			fprintf(stderr, "\t% 3i%%", progress);
		}
		fputc('\n', stderr);
	} else {
		r = pb_get_row(id ? *id : 0);
		r->progress = progress;
		if (id) {
			*id = r->id;
		}
		rv = vasprintf (&r->msg, fmt, args);
		pb_draw(r);
	}
	fflush(tty);
	pthread_mutex_unlock(&mutex);
	return rv;
}
