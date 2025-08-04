/* SPDX-License-Identifier: GPL-2.0-or-later */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "mime.h"
#include "util.h"

const char *GetMimeType(const char *filename)
{
	const char *ext = NULL;
	const char *dot = xstrrchr(filename, '.');

	if (!dot)
		return "application/octet-stream";

	ext = dot + 1;

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
}
