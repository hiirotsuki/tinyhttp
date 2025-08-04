/* SPDX-License-Identifier: GPL-2.0-or-later */

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "util.h"
#include "mime.h"

#ifndef __GNUC__
#define __attribute__(x)
#endif

void ConsoleWrite(const char *message);

#define BUFFER_SIZE 8192
#define MAX_PATH_LEN 1024

typedef struct {
	char *requestBuffer;
	char *fileBuffer;
	char *baseAllocation;
} threadBuffers;

const char HTTP_HEADER[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";
const char HTTP_404[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found\n";

const char HTML_START[] = 
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"	<meta charset=\"utf-8\">\n"
"	<title>Directory Listing</title>\n"
"	<style>\n"
"		body { margin: 40px; }\n"
"		h1 { color: #333; }\n"
"		a { text-decoration: none; color: #0066cc; }\n"
"		a:hover { text-decoration: underline; }\n"
"		.file { margin: 5px 0; }\n"
"		.dir { font-weight: bold; }\n"
"	</style>\n"
"</head>\n"
"<body>\n"
"	<h1>Directory Listing</h1>\n"
"	<hr>\n";

const char HTML_END[] = 
"	<hr>\n"
"	<p><em>Tiny HTTP Server</em></p>\n"
"</body>\n"
"</html>\n";

int DirectoryExists(const wchar_t *path)
{
	DWORD attrib = GetFileAttributesW(path);
	if (attrib == INVALID_FILE_ATTRIBUTES)
		return 0;

	return (attrib & FILE_ATTRIBUTE_DIRECTORY);
}

int HexToInt(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return 0;
}

void UrlDecode(char *dst, const char *src)
{
	char *p = dst;
	char *end = dst + MAX_PATH_LEN - 1;
	
	while (*src && p < end)
	{
		if (*src == '%' && src[1] && src[2])
		{
			*p++ = (char)(HexToInt(src[1]) * 16 + HexToInt(src[2]));
			src += 3;
		}
		else if (*src == '+')
		{
			*p++ = ' ';
			src++;
		}
		else
		{
			*p++ = *src++;
		}
	}
	*p = '\0';
}

int Utf8ToWide(const char *utf8, wchar_t *wide_str, int max_len)
{
	const unsigned char *src = (const unsigned char *)utf8;
	wchar_t *dst = wide_str;
	wchar_t *end = wide_str + max_len - 1;
	
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
	return (int)(dst - wide_str + 1);
}

int WideToUtf8(const wchar_t *wide_str, char *utf8, int max_len)
{
	const wchar_t *src = wide_str;
	unsigned char *dst = (unsigned char *)utf8;
	unsigned char *end = (unsigned char *)utf8 + max_len - 1;

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

void SendFile(SOCKET clientSocket, const char *filePath, char *fileBuffer)
{
	wchar_t widePath[MAX_PATH_LEN];
	Utf8ToWide(filePath, widePath, MAX_PATH_LEN);
	
	HANDLE hFile = CreateFileW(widePath, GENERIC_READ, FILE_SHARE_READ, 
							   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		send(clientSocket, HTTP_404, sizeof(HTTP_404) - 1, 0);
		return;
	}

	char header[512];
	wsprintfA(header, "HTTP/1.1 200 OK\r\n"
					 "Content-Type: %s\r\n"
					 "Content-Length: %lu\r\n\r\n", GetMimeType(filePath), GetFileSize(hFile, NULL));
	send(clientSocket, header, lstrlenA(header), 0);

	if (fileBuffer)
	{
		DWORD bytesRead;
		while (ReadFile(hFile, fileBuffer, BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0)
			send(clientSocket, fileBuffer, (int)bytesRead, 0);
	}

	CloseHandle(hFile);
}

void SendDirectoryListing(SOCKET clientSocket, const char *path)
{
	WIN32_FIND_DATAW findData;
	HANDLE hFind;
	wchar_t searchPath[MAX_PATH_LEN];
	char htmlLine[MAX_PATH_LEN + 100];
	char filenameUtf8[MAX_PATH_LEN];

	wchar_t widePath[MAX_PATH_LEN];
	Utf8ToWide(path, widePath, MAX_PATH_LEN);
	wsprintfW(searchPath, L"%s\\*", (lstrcmpA(path, ".") == 0) ? L"." : widePath);

	hFind = FindFirstFileW(searchPath, &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		send(clientSocket, HTTP_404, sizeof(HTTP_404) - 1, 0);
		return;
	}

	send(clientSocket, HTTP_HEADER, sizeof(HTTP_HEADER) - 1, 0);
	send(clientSocket, HTML_START, sizeof(HTML_START) - 1, 0);

	if (lstrcmpA(path, "www") != 0)
	{
		const char parentLink[] = "	<div class=\"file\"><a href=\"../\">../</a> (Parent Directory)</div>\n";
		send(clientSocket, parentLink, sizeof(parentLink) - 1, 0);
	}

	do
	{
		if (lstrcmpW(findData.cFileName, L".") == 0 || lstrcmpW(findData.cFileName, L"..") == 0)
			continue;

		WideToUtf8(findData.cFileName, filenameUtf8, sizeof(filenameUtf8));

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			wsprintfA(htmlLine,
				"	<div class=\"dir\"><a href=\"%s/\">%s/</a></div>\n",
				filenameUtf8, filenameUtf8);
		} else {
			wsprintfA(htmlLine,
				"	<div class=\"file\"><a href=\"%s\">%s</a></div>\n",
				filenameUtf8, filenameUtf8);
		}
		send(clientSocket, htmlLine, lstrlenA(htmlLine), 0);
	}
	while (FindNextFileW(hFind, &findData) != 0);

	FindClose(hFind);
	send(clientSocket, HTML_END, sizeof(HTML_END) - 1, 0);
}

int ParseHttpRequest(const char *buffer, char *method, char *path, char *version)
{
	const char *p = buffer;
	char *dst;

	while (*p == ' ' || *p == '\t') p++;

	dst = method;
	while (*p && *p != ' ' && *p != '\t' && (dst - method) < 15)
		*dst++ = *p++;

	*dst = '\0';

	while (*p == ' ' || *p == '\t') p++;
	
	dst = path;
	while (*p && *p != ' ' && *p != '\t' && (dst - path) < MAX_PATH_LEN - 1)
		*dst++ = *p++;

	*dst = '\0';

	while (*p == ' ' || *p == '\t') p++;

	dst = version;
	while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && (dst - version) < 15)
		*dst++ = *p++;

	*dst = '\0';

	return (method[0] && path[0] && version[0]) ? 3 : 0;
}

void HandleRequest(SOCKET clientSocket, threadBuffers *buffers)
{
	char *p;

	if (!buffers || !buffers->requestBuffer)
		return;

	char method[16], path[MAX_PATH_LEN], version[16];
	char decodedPath[MAX_PATH_LEN];
	char logBuffer[512];
	WIN32_FIND_DATAW findData;
	HANDLE hFind;
	wchar_t widePath[MAX_PATH_LEN];

	int bytesRead = recv(clientSocket, buffers->requestBuffer, BUFFER_SIZE - 1, 0);

	if (bytesRead <= 0)
		return;

	buffers->requestBuffer[bytesRead] = '\0';

	ConsoleWrite("Request: ");
	char *lineEnd = xstrchr(buffers->requestBuffer, '\r');
	if (lineEnd)
	{
		int requestLen = lineEnd - buffers->requestBuffer;
		if (requestLen > 1000)
		{
			char tempChar = buffers->requestBuffer[1000];
			buffers->requestBuffer[1000] = '\0';
			ConsoleWrite(buffers->requestBuffer);
			ConsoleWrite("... [truncated]");
			buffers->requestBuffer[1000] = tempChar;
		}
		else
		{
			*lineEnd = '\0';
			ConsoleWrite(buffers->requestBuffer);
			*lineEnd = '\r';
		}
	}
	else
	{
		int requestLen = lstrlenA(buffers->requestBuffer);
		if (requestLen > 1000)
		{
			char tempChar = buffers->requestBuffer[1000];
			buffers->requestBuffer[1000] = '\0';
			ConsoleWrite(buffers->requestBuffer);
			ConsoleWrite("... [truncated]");
			buffers->requestBuffer[1000] = tempChar;
		}
		else
		{
			ConsoleWrite(buffers->requestBuffer);
		}
	}

	ConsoleWrite("\r\n");

	if (ParseHttpRequest(buffers->requestBuffer, method, path, version) != 3)
	{
		send(clientSocket, HTTP_404, sizeof(HTTP_404) - 1, 0);
		return;
	}

	char safePath[256];
	int pathLen = lstrlenA(path);
	if (pathLen > 250)
	{
		lstrcpynA(safePath, path, 247);
		lstrcatA(safePath, "...");
	}
	else
	{
		lstrcpyA(safePath, path);
	}
	wsprintfA(logBuffer, "Method: %s, Path: %s\r\n", method, safePath);
	ConsoleWrite(logBuffer);

	if (lstrcmpA(method, "GET") != 0)
	{
		send(clientSocket, HTTP_404, sizeof(HTTP_404) - 1, 0);
		return;
	}

	UrlDecode(decodedPath, path + 1);

	if (lstrlenA(decodedPath) == 0)
		lstrcpyA(decodedPath, "www");
	else
	{
		char tempPath[MAX_PATH_LEN];
		lstrcpyA(tempPath, decodedPath);
		wsprintfA(decodedPath, "www\\%s", tempPath);
	}

	for (p = decodedPath; *p; p++)
		if (*p == '/') *p = '\\';

	int len = lstrlenA(decodedPath);
	if (len > 1 && decodedPath[len - 1] == '\\')
		decodedPath[len - 1] = '\0';
	
	wsprintfA(logBuffer, "Decoded path: '%s'\r\n", decodedPath);
	ConsoleWrite(logBuffer);

	Utf8ToWide(decodedPath, widePath, MAX_PATH_LEN);
	hFind = FindFirstFileW(widePath, &findData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		wsprintfA(logBuffer, "File not found: %s\r\n", decodedPath);
		ConsoleWrite(logBuffer);
		send(clientSocket, HTTP_404, sizeof(HTTP_404) - 1, 0);
		return;
	}
	FindClose(hFind);

	if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		SendDirectoryListing(clientSocket, decodedPath);
	else
		SendFile(clientSocket, decodedPath, buffers->fileBuffer);
}

DWORD WINAPI ClientThread(LPVOID param)
{
	SOCKET clientSocket = (SOCKET)param;
	struct sockaddr_in clientAddr;
	int addr_len = sizeof(clientAddr);
	char buffer[256];

	threadBuffers buffers;
	buffers.baseAllocation = (char *)HeapAlloc(GetProcessHeap(), 0, BUFFER_SIZE * 2);

	if (buffers.baseAllocation)
	{
		buffers.requestBuffer = buffers.baseAllocation;
		buffers.fileBuffer = buffers.baseAllocation + BUFFER_SIZE;
		HandleRequest(clientSocket, &buffers);
		HeapFree(GetProcessHeap(), 0, buffers.baseAllocation);
	}
	else
	{
		const char errorResponse[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\n500 Internal Server Error\n";
		send(clientSocket, errorResponse, sizeof(errorResponse) - 1, 0);
	}

	shutdown(clientSocket, SD_SEND);

	if (getpeername(clientSocket, (struct sockaddr*)&clientAddr, &addr_len) == 0)
	{
		wsprintfA(buffer, "Connection from %s:%d closed\r\n", 
				 inet_ntoa(clientAddr.sin_addr), 
				 ntohs(clientAddr.sin_port));
		ConsoleWrite(buffer);
	}
	
	closesocket(clientSocket);
	return 0;
}

unsigned int ReadPortFromIni(void)
{
	wchar_t *p;
	wchar_t iniPath[MAX_PATH];
	wchar_t exePath[MAX_PATH];
	
	GetModuleFileNameW(NULL, exePath, MAX_PATH);

	wchar_t *lastSlash = exePath;
	for (p = exePath; *p; p++)
	{
		if (*p == L'\\' || *p == L'/')
			lastSlash = p;
	}

	lstrcpyW(iniPath, exePath);
	lstrcpyW(lastSlash + 1, L"tinyhttp.ini");

	unsigned int port = GetPrivateProfileIntW(L"tinyhttp", L"port", 8080, iniPath);

	if (port < 1 || port > 65535)
		port = 8080;
	
	return port;
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

void DisplayAvailableIPs(unsigned int port)
{
	char buffer[512];
	DWORD bufferSize = 15000;
	PIP_ADAPTER_INFO adapterInfo = HeapAlloc(GetProcessHeap(), 0, bufferSize);
	
	if (!adapterInfo)
	{
		ConsoleWrite("Error: Failed to allocate memory for adapter info\r\n");
		return;
	}
	
	DWORD result = GetAdaptersInfo(adapterInfo, &bufferSize);
	
	if (result == ERROR_BUFFER_OVERFLOW)
	{
		HeapFree(GetProcessHeap(), 0, adapterInfo);
		adapterInfo = HeapAlloc(GetProcessHeap(), 0, bufferSize);
		if (!adapterInfo)
		{
			ConsoleWrite("Error: Failed to reallocate memory for adapter info\r\n");
			return;
		}
		result = GetAdaptersInfo(adapterInfo, &bufferSize);
	}
	
	if (result == NO_ERROR)
	{
		ConsoleWrite("\r\nServer accessible at:\r\n");
		wsprintfA(buffer, "  http://localhost:%d/\r\n", port);
		ConsoleWrite(buffer);
		wsprintfA(buffer, "  http://127.0.0.1:%d/\r\n", port);
		ConsoleWrite(buffer);
		
		PIP_ADAPTER_INFO adapter = adapterInfo;
		while (adapter)
		{
			if (adapter->Type == MIB_IF_TYPE_ETHERNET)
			{
				PIP_ADDR_STRING addrString = &adapter->IpAddressList;
				while (addrString)
				{
					if (lstrcmpA(addrString->IpAddress.String, "0.0.0.0") != 0 &&
						lstrcmpA(addrString->IpAddress.String, "127.0.0.1") != 0)
					{
						wsprintfA(buffer, "  http://%s:%d\r\n", 
								 addrString->IpAddress.String, port);
						ConsoleWrite(buffer);
					}
					addrString = addrString->Next;
				}
			}
			adapter = adapter->Next;
		}
		ConsoleWrite("\r\n");
	}
	else
	{
		ConsoleWrite("Error: Failed to get adapter information\r\n");
	}
	
	HeapFree(GetProcessHeap(), 0, adapterInfo);
}

#if defined(_NOCRT)
int mainCRTStartup(void)
#else
int main(int argc, char *argv[])
#endif
{
	WSADATA wsaData;
	SOCKET serverSocket, clientSocket;
	struct sockaddr_in serverAddr = {0};
	struct sockaddr_in clientAddr = {0};
	int clientLen = sizeof(clientAddr);
	char buffer[256];
	unsigned int port = ReadPortFromIni();

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ConsoleWrite("Error: WSAStartup failed\r\n");
		return 1;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		ConsoleWrite("Error: Socket creation failed\r\n");
		WSACleanup();
		return 1;
	}

	BOOL opt = TRUE;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR)
	{
		ConsoleWrite("Error: setsockopt failed\r\n");
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		ConsoleWrite("Error: Bind failed\r\n");
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	if (listen(serverSocket, 5) == SOCKET_ERROR)
	{
		ConsoleWrite("Error: Listen failed\r\n");
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	ConsoleWrite("Win32 HTTP Server started successfully!\r\n");
	DisplayAvailableIPs(port);
	
	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(NULL, exePath, MAX_PATH);
	
	wchar_t *lastSlash = xstrrchrW(exePath, L'\\');
	*lastSlash = L'\0';

	wchar_t wwwPath[MAX_PATH];
	lstrcpyW(wwwPath, exePath);
	lstrcatW(wwwPath, L"\\www");

	if(!DirectoryExists(wwwPath))
	{
		if(!CreateDirectoryW(wwwPath, NULL))
		{
			ConsoleWrite("Error: Failed to create www directory\r\n");
			return 1;
		}
	}

	ConsoleWrite("Serving directory: ");

	char wwwUtf8[MAX_PATH];
	WideToUtf8(wwwPath, wwwUtf8, sizeof(wwwUtf8));
	ConsoleWrite(wwwUtf8);
	ConsoleWrite("\r\n");

	ConsoleWrite("Press Ctrl+C to stop\r\n");

	while (1)
	{
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
		if (clientSocket == INVALID_SOCKET)
			continue;

		wsprintfA(buffer, "Connection from %s:%d\r\n", 
				inet_ntoa(clientAddr.sin_addr), 
				ntohs(clientAddr.sin_port));
		ConsoleWrite(buffer);

		HANDLE threadHandle = CreateThread(NULL, 0, ClientThread, (LPVOID)clientSocket, 0, NULL);
		if (threadHandle == NULL)
		{
			ConsoleWrite("Error: Failed to create thread\r\n");
			closesocket(clientSocket);
			continue;
		}
		
		CloseHandle(threadHandle);
	}

	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
