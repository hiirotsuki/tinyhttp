/* SPDX-License-Identifier: GPL-2.0-or-later */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "unicode.h"

#ifndef __GNUC__
#define __attribute__(x)
#endif

char *xstrrchr(const char *s, int c)
{
	int len = lstrlenA(s);
	c = (unsigned char)c;
	while(len--)
		if(s[len] == c)
			return (char *)s + len;
	return 0;
}

wchar_t *xstrrchrW(const wchar_t *s, wchar_t c)
{
	int len = lstrlenW(s);
	while(len--)
		if(s[len] == c)
			return (wchar_t *)s + len;
	return 0;
}

char *xstrchr(const char *str, int c)
{
	c = (unsigned char)c;
	while (*str)
	{
		if (*str == c)
			return (char *)str;

		str++;
	}
	return (*str == c) ? (char *)str : NULL;
}

void *xmemchr(const void *str, int c, size_t len)
{
	const unsigned char *p = str;
	c = (unsigned char)c;
	for(; len && *p != c; p++, len--);
	return len ? (void *)p : NULL;
}

void ConsoleWrite(const char *message)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout != INVALID_HANDLE_VALUE)
	{
		wchar_t wideBuffer[4096];
		int wLen = Utf8ToWide(message, wideBuffer, sizeof(wideBuffer) / sizeof(wchar_t));
		if (wLen > 0)
		{
			__attribute__((unused)) DWORD written;
			WriteConsoleW(hStdout, wideBuffer, wLen - 1, &written, NULL);
		}
	}
}
