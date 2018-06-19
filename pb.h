/*
 * pb.h
 * Copyright (C) 2018 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PB_H
#define PB_H

/* Init the library.
 *
 * Before `pb_init()` is called or if `pb_init()` fails `pb()`
 * assumes a dumb terminal.
 *
 * returns: 0 on success, < 0 on failure.
 */
int pb_init(void);

/* Write a message
 *
 * If a message with the same *id is already displayed on the screen
 * pb replaces the line with the new output.
 *
 * If no message with the same *id is found, a message is created and
 * displayed on the screen and a new *id is set. *id is always set to
 * a value >= 1024.
 *
 * returns: number of characters processed with `fmt, ...` excluding
 * the padding or < 0 on failure.
 */
int pb(int *id, const int progress, const char *fmt, ...);

/* Clean up resources held by pb.
 *
 * After `pb_clean()` is called `pb()` assumes a dumb terminal.
 *
 * pb_clean fails if `pb_init()` wasn't called beforehand.
 *
 * returns: 0 on success, < 0 on failure.
 */
int pb_clean(void);

#endif /* !PROGRESS_H */
