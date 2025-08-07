/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef MIME_H
#define MIME_H

int LoadMimeTypes(const char *filename);
const char *GetMimeType(const char *filename);

#endif
