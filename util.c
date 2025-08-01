/* SPDX-License-Identifier: GPL-2.0-or-later */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
