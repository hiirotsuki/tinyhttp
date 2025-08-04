/* SPDX-License-Identifier: GPL-2.0-or-later */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int Utf8ToWide(const char *utf8, wchar_t *wideStr, int maxLen)
{
	const unsigned char *src = (const unsigned char *)utf8;
	wchar_t *dst = wideStr;
	wchar_t *end = wideStr + maxLen - 1;
	
	while (*src && dst < end)
	{
		if (*src < 0x80)
		{
			*dst++ = *src++;
		}
		else if ((*src & 0xE0) == 0xC0)
		{
			if (src[1] && (src[1] & 0xC0) == 0x80)
			{
				*dst++ = ((src[0] & 0x1F) << 6) | (src[1] & 0x3F);
				src += 2;
			}
			else break;
		}
		else if ((*src & 0xF0) == 0xE0)
		{
			if (src[1] && src[2] && (src[1] & 0xC0) == 0x80 && (src[2] & 0xC0) == 0x80)
			{
				*dst++ = ((src[0] & 0x0F) << 12) | ((src[1] & 0x3F) << 6) | (src[2] & 0x3F);
				src += 3;
			}
			else break;
		}
		else if ((*src & 0xF8) == 0xF0)
		{
			if (src[1] && src[2] && src[3] && (src[1] & 0xC0) == 0x80 && (src[2] & 0xC0) == 0x80 && (src[3] & 0xC0) == 0x80)
			{
				unsigned int codepoint = ((src[0] & 0x07) << 18) | ((src[1] & 0x3F) << 12) | 
										((src[2] & 0x3F) << 6) | (src[3] & 0x3F);
				if (codepoint > 0xFFFF && dst + 1 < end)
				{
					codepoint -= 0x10000;
					*dst++ = 0xD800 + (codepoint >> 10);
					*dst++ = 0xDC00 + (codepoint & 0x3FF);
				}
				else if (codepoint <= 0xFFFF)
					*dst++ = (wchar_t)codepoint;

				src += 4;
			}
			else break;
		}
		else src++;
	}

	*dst = L'\0';
	return (int)(dst - wideStr + 1);
}

int WideToUtf8(const wchar_t *wideStr, char *utf8, int maxLen)
{
	const wchar_t *src = wideStr;
	unsigned char *dst = (unsigned char *)utf8;
	unsigned char *end = (unsigned char *)utf8 + maxLen - 1;

	while (*src && dst < end)
	{
		unsigned int codepoint = *src;

		if (codepoint >= 0xD800 && codepoint <= 0xDBFF && src[1] >= 0xDC00 && src[1] <= 0xDFFF)
		{
			codepoint = 0x10000 + ((codepoint & 0x3FF) << 10) + (src[1] & 0x3FF);
			src += 2;
		}
		else src++;
		
		if (codepoint < 0x80)
		{
			*dst++ = (unsigned char)codepoint;
		}
		else if (codepoint < 0x800)
		{
			if (dst + 1 >= end) break;
			*dst++ = 0xC0 | (codepoint >> 6);
			*dst++ = 0x80 | (codepoint & 0x3F);
		}
		else if (codepoint < 0x10000)
		{
			if (dst + 2 >= end) break;
			*dst++ = 0xE0 | (codepoint >> 12);
			*dst++ = 0x80 | ((codepoint >> 6) & 0x3F);
			*dst++ = 0x80 | (codepoint & 0x3F);
		}
		else
		{
			if (dst + 3 >= end) break;
			*dst++ = 0xF0 | (codepoint >> 18);
			*dst++ = 0x80 | ((codepoint >> 12) & 0x3F);
			*dst++ = 0x80 | ((codepoint >> 6) & 0x3F);
			*dst++ = 0x80 | (codepoint & 0x3F);
		}
	}
	
	*dst = '\0';
	return (int)((char *)dst - utf8 + 1);
}