/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef UTIL_H
#define UTIL_H

char *xstrrchr(const char *s, int c);
wchar_t *xstrrchrW(const wchar_t *s, wchar_t c);
char *xstrchr(const char *str, int c);

#endif
