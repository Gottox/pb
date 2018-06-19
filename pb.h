/*
 * progress.h
 * Copyright (C) 2018 tox <tox@rootkit>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PB_H
#define PB_H

int pb_init(void);
int pb(int *id, const int progress, const char *fmt, ...);
void pb_clean(void);

#endif /* !PROGRESS_H */
