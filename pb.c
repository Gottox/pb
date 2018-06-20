/*
 * pb.c
 * Copyright (C) 2018 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#include "pb.h"

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MIN(x, y) ((x) > (y) ? (y) : (x))

struct Row {
	int id;
	int progress;
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
	int i;

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
pb_bar(int length, const int progress) {
	int i = 0;
	length -= 10;

	if (length >= 4) {
		fputs(" [", tty);
		for (i = 0; i < length; i++) {
			fputc(progress > i * 100 / length ? '#' : ' ', tty);
		}
		fputs("] ", tty);
	}
	fprintf(tty, "% 3i%% ", progress);
}

static void
pb_draw_row(const struct Row *row) {
	char str[] = "                    ";

	if (row->progress >= 0) {
		memcpy(str, row->msg, MIN(strlen(row->msg), (sizeof(str) - 1) * sizeof(char)));
		fwrite(str, sizeof(char), sizeof(str) - 1, tty);
		pb_bar(ws.ws_col - (sizeof(str) - 1), row->progress);
	}
	else {
		fwrite(row->msg, sizeof(char), MIN(ws.ws_col, strlen(row->msg)), tty);
	}
	fputs(
			"\x1b[K"            /* clear line */
			"\n\x1b[A",         /* make sure the line is terminated.
			                     * terminal resizing looks nicer if it does. */
			tty);
}

static void
pb_draw(const struct Row *row) {
	struct Row *r;
	fputs(
			"\x1b[s"            /* save cursor position */
			"\x1b[?25l"         /* disable cursor */
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
			"\x1b[?25h",        /* enable cursor */
			tty);
}

static void
pb_sigwinch(int sig) {
	if (ioctl(fileno(tty), TIOCGWINSZ, &ws) == -1) {
		return;
	}
	pb_cut();
	pb_draw(NULL);
}

int
pb_clean() {
	static const struct sigaction sa = { .sa_handler = SIG_DFL, { { 0 } } };
	struct Row *r;

	ws.ws_row = 0;
	for(; (r = rows);) {
		rows = r->next;
		free(r->msg);
		free(r);
	}
	tty = NULL;
	return pthread_mutex_destroy(&mutex) == 0
		&& sigaction(SIGWINCH, &sa, NULL) != -1 ? 0 : -1;
}

int
pb_init() {
	static const struct sigaction sa = { .sa_handler = pb_sigwinch, { { 0 } } };

	if (!isatty(fileno(stderr))
			|| pthread_mutex_init(&mutex, NULL) != 0
			|| sigaction(SIGWINCH, &sa, NULL) == -1) {
		return -1;
	}
	tty = stderr;
	pb_sigwinch(0);
	return 0;
}

static struct Row *
pb_get_row(const int id) {
	struct Row *r;

	if (id) {
		for (r = rows; r; r = r->next) {
			if (r && r->id == id) {
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
	int rv = 0;
	va_list args;
	struct Row *r;

	pthread_mutex_lock(&mutex);
	va_start (args, fmt);
	if (tty == NULL) {
		rv = vfprintf (stderr, fmt, args);
		if (progress != -1) {
			fprintf(stderr, " [% 3i%%]", progress);
		}
		fputc('\n', stderr);
		goto out;
	}
	r = pb_get_row(id ? *id : 0);
	r->progress = progress;
	if (id) {
		*id = r->id;
	}
	rv = vasprintf (&r->msg, fmt, args);
	pb_draw(r);
out:
	va_end (args);
	fflush(tty);
	pthread_mutex_unlock(&mutex);
	return rv;
}
