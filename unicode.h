/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef UNICODE_H
#define UNICODE_H

int Utf8ToWide(const char *utf8, wchar_t *wide_str, int max_len);
int WideToUtf8(const wchar_t *wide_str, char *utf8, int max_len);

#endif
