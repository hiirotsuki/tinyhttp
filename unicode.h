/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef UNICODE_H
#define UNICODE_H

int Utf8ToWide(const char *utf8, wchar_t *wideStr, int maxLen);
int WideToUtf8(const wchar_t *wideStr, char *utf8, int maxLen);

#endif
