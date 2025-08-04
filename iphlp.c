/* SPDX-License-Identifier: GPL-2.0-or-later */

#define WIN32_LEAN_AND_MEAN
#include <iphlpapi.h>

#include "unicode.h"
#include "util.h"
#include "iphlp.h"

void DisplayAvailableIPs(unsigned short port)
{
	DWORD result;
	char buffer[512];
	DWORD bufferSize = 15000;
	PIP_ADAPTER_INFO adapterInfo = HeapAlloc(GetProcessHeap(), 0, bufferSize);
	
	if (!adapterInfo)
	{
		ConsoleWrite("Error: Failed to allocate memory for adapter info\r\n");
		return;
	}
	
	result = GetAdaptersInfo(adapterInfo, &bufferSize);
	
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
		PIP_ADAPTER_INFO adapter = adapterInfo;

		ConsoleWrite("\r\nServer accessible at:\r\n");
		wsprintfA(buffer, "  http://localhost:%d/\r\n", port);
		ConsoleWrite(buffer);
		wsprintfA(buffer, "  http://127.0.0.1:%d/\r\n", port);
		ConsoleWrite(buffer);

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
