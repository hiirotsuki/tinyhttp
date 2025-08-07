/* SPDX-License-Identifier: GPL-2.0-or-later */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "mime.h"
#include "util.h"

struct mimeType *mimeTypes;
size_t mimeTypesSize;

struct mimeType
{
	const char *ext;
	const char *mime;
};

int LoadMimeTypes(const char *filename)
{
	char *p;
	DWORD size;
	HANDLE hMem = NULL;
	HANDLE hStructArray = NULL;
	size_t lineCount = 0, i;
	HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
		return 0;

	size = GetFileSize(hFile, NULL);
	if(size == INVALID_FILE_SIZE)
		goto error;

	hMem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + 1);
	if(!hMem)
		goto error;

	ReadFile(hFile, hMem, size, NULL, NULL);

	p = (char *)hMem;
	while(1)
	{
		char *newline;
		char *equals = xstrchr(p, '=');
		if (!equals)
			break;
		*equals = '\0';
		
		newline = xstrchr(equals + 1, '\n');
		if (!newline)
		{
			lineCount++;
			break;
		}
		*newline = '\0';
		lineCount++;
		p = newline + 1;
	}

	p = (char *)hMem;
	for(i = 0; i < size; i++)
	{
		if(p[i] == '\r')
			p[i] = '\0';
	}

	hStructArray = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lineCount * sizeof(struct mimeType));
	if(!hStructArray)
		goto error;

	mimeTypes = (struct mimeType *)hStructArray;
	mimeTypesSize = lineCount;

	p = (char *)hMem;
	for(i = 0; i < lineCount; i++)
	{
		mimeTypes[i].ext = p;
		p += lstrlenA(p) + 1;
		mimeTypes[i].mime = p;
		p += lstrlenA(p) + 1;
	}

	/* HeapFree(GetProcessHeap(), 0, hMem); */
	CloseHandle(hFile);
	return 1;

error:
	CloseHandle(hFile);
	if(hMem)
		HeapFree(GetProcessHeap(), 0, hMem);
	if(hStructArray)
		HeapFree(GetProcessHeap(), 0, hStructArray);
	return 0;
}

const char *GetMimeType(const char *filename)
{
	size_t i;
	const char *ext = NULL;
	const char *dot = xstrrchr(filename, '.');

	if(!dot)
		goto end;

	ext = dot + 1;

	for(i = 0; i < mimeTypesSize; i++)
	{
		if(lstrcmpiA(ext, mimeTypes[i].ext) == 0)
			return mimeTypes[i].mime;
	}

end:
	return "application/octet-stream";

#if 0
	if (lstrcmpiA(ext, "txt") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "html") == 0) return "text/html; charset=utf-8";
	if (lstrcmpiA(ext, "htm") == 0) return "text/html; charset=utf-8";
	if (lstrcmpiA(ext, "css") == 0) return "text/css; charset=utf-8";
	if (lstrcmpiA(ext, "js") == 0) return "text/javascript; charset=utf-8";
	if (lstrcmpiA(ext, "json") == 0) return "application/json; charset=utf-8";
	if (lstrcmpiA(ext, "xml") == 0) return "text/xml; charset=utf-8";
	if (lstrcmpiA(ext, "csv") == 0) return "text/csv; charset=utf-8";
	if (lstrcmpiA(ext, "md") == 0) return "text/markdown; charset=utf-8";
	if (lstrcmpiA(ext, "log") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "ini") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "cfg") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "conf") == 0) return "text/plain; charset=utf-8";

	if (lstrcmpiA(ext, "c") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "cpp") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "h") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "hpp") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "py") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "java") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "php") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "rb") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "go") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "rs") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "sh") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "bat") == 0) return "text/plain; charset=utf-8";
	if (lstrcmpiA(ext, "ps1") == 0) return "text/plain; charset=utf-8";

	if (lstrcmpiA(ext, "jpg") == 0) return "image/jpeg";
	if (lstrcmpiA(ext, "jpeg") == 0) return "image/jpeg";
	if (lstrcmpiA(ext, "png") == 0) return "image/png";
	if (lstrcmpiA(ext, "gif") == 0) return "image/gif";
	if (lstrcmpiA(ext, "bmp") == 0) return "image/bmp";
	if (lstrcmpiA(ext, "svg") == 0) return "image/svg+xml";
	if (lstrcmpiA(ext, "ico") == 0) return "image/x-icon";
	if (lstrcmpiA(ext, "webp") == 0) return "image/webp";

	if (lstrcmpiA(ext, "mp3") == 0) return "audio/mpeg";
	if (lstrcmpiA(ext, "wav") == 0) return "audio/wav";
	if (lstrcmpiA(ext, "mp4") == 0) return "video/mp4";
	if (lstrcmpiA(ext, "avi") == 0) return "video/x-msvideo";
	if (lstrcmpiA(ext, "mov") == 0) return "video/quicktime";

	if (lstrcmpiA(ext, "zip") == 0) return "application/zip";
	if (lstrcmpiA(ext, "rar") == 0) return "application/x-rar-compressed";
	if (lstrcmpiA(ext, "7z") == 0) return "application/x-7z-compressed";
	if (lstrcmpiA(ext, "tar") == 0) return "application/x-tar";
	if (lstrcmpiA(ext, "gz") == 0) return "application/gzip";

	if (lstrcmpiA(ext, "pdf") == 0) return "application/pdf";
	if (lstrcmpiA(ext, "doc") == 0) return "application/msword";
	if (lstrcmpiA(ext, "docx") == 0) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	if (lstrcmpiA(ext, "xls") == 0) return "application/vnd.ms-excel";
	if (lstrcmpiA(ext, "xlsx") == 0) return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";

	return "application/octet-stream";
#endif
}
