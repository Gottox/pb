/*
 * progress.c
 * Copyright (C) 2018 tox <tox@rootkit>
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
static pthread_mutex_t mutex;

static void
pb_cut() {
	struct Row *r, *last = NULL;
	int i;

	for(r = rows, i = 0; r && i < ws.ws_row - 1 ; i++) {
		last = r;
		r = r->next;
	}
	for(; last && last->next;) {
		r = last->next;
		last->next = r->next;
		free(r->msg);
		free(r);
	}
}

static void
pb_bar(int length, int progress) {
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
pb_draw_row(struct Row *r) {
	char str[] = "                    ";

	if (r->progress >= 0) {
		memcpy(str, r->msg, MIN(strlen(r->msg), 20));
		fwrite(str, sizeof(char), 20, tty);
		pb_bar(ws.ws_col - 20, r->progress);
	}
	else {
		fwrite(r->msg, sizeof(char), MIN(ws.ws_col, strlen(r->msg)), tty);
	}
	// clears row, makes sure row is ended
	fputs("\x1b[K\n\x1b[A", tty);
}

static void
pb_draw(struct Row *row) {
	struct Row *r;
	fputs(
			"\x1b[s"     // save cursor
			"\x1b[7l"    // disable row wrap
			"\x1b[?25l"  // disable cursor
			"\r",        // row start
			tty);
	for(r = rows; r; r = r->next) {
		// one 1 row up
		fputs("\x1b[A", tty);
		if(row == NULL || r == row) {
			pb_draw_row(r);
		}
		if(row == r)
			break;
	}
	fputs(
			"\x1b[u"     // restore cursor
			"\x1b[7h"    // enable row wrap
			"\x1b[?25h", // enable cursor
			tty);
	fflush(tty);
}

static void
pb_sigwinch(int sig) {
	if (ioctl(fileno(tty), TIOCGWINSZ, &ws) == -1)
		return;

	pb_cut();
	pb_draw(NULL);
}

void
pb_clean() {
	fclose(tty);
}

int
pb_init() {
	struct sigaction sa;
	if (!isatty(fileno(stderr))) {
		return -1;
	}
	tty = stderr;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = pb_sigwinch;
	if (sigaction(SIGWINCH, &sa, NULL) == -1) {
		return -1;
	}

	pthread_mutex_init(&mutex, NULL);

	pb_sigwinch(0);

	return 0;
}

static struct Row *
pb_get_row(int id) {
	int i = 0;
	struct Row *r;

	if(id) {
		for(r = rows, i = 0; i < ws.ws_row - 1 && r && r->id != id; i++) {
			r = r->next;
		}
		if(r && r->id == id) {
			return r;
		}
	}
	r = calloc(sizeof(struct Row), 1);
	r->id = next_id++;
	r->next = rows;
	rows = r;
	fputc('\n', tty);
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
		if (progress != -1)
			fprintf(stderr, " [% 3i%%]", progress);
		fputc('\n', stderr);
		goto out;
	}
	r = pb_get_row(id ? *id : 0);
	r->progress = progress;
	if(id) {
		*id = r->id;
	}
	rv = vasprintf (&r->msg, fmt, args);
	pb_draw(r);
out:
	va_end (args);
	pthread_mutex_unlock(&mutex);
	return rv;
}
