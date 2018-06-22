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

#ifndef PB_H
#define PB_H

#include <stdarg.h>

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

int vpb(int *id, const int progress, const char *fmt, va_list args);

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
