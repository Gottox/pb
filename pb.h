/*
 * pb.h
 * Copyright (C) 2018 Enno Boland <g@s01.de>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PB_H
#define PB_H

int pb_init(void);
int pb(int *id, const int progress, const char *fmt, ...);
void pb_clean(void);

#endif /* !PROGRESS_H */
