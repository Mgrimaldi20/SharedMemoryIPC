#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include "../../Server/src/command.h"

static void WindowsError(void);

int main(int argc, char **argv)
{
	DWORD maxbufsize = sizeof(serverstate_t);

	HANDLE mapfile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"MyFileMappingObject");
	if (!mapfile)
	{
		WindowsError();
		return(1);
	}

	serverstate_t *buffer = MapViewOfFile(mapfile, FILE_MAP_ALL_ACCESS, 0, 0, maxbufsize);
	if (!buffer)
	{
		WindowsError();
		CloseHandle(mapfile);
		return(1);
	}

	HANDLE closeevent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"OnServerClose");
	if (!closeevent)
	{
		WindowsError();
		UnmapViewOfFile(buffer);
		CloseHandle(mapfile);
		return(1);
	}

	HANDLE getclientid = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"OnGetClientID");
	if (!getclientid)
	{
		WindowsError();
		CloseHandle(closeevent);
		UnmapViewOfFile(buffer);
		CloseHandle(mapfile);
		return(1);
	}

	printf("Client running. Press ESC to exit...\n");

	int clientid = -1;
	buffer->cmd = CMD_INIT;

	if (WaitForSingleObject(getclientid, INFINITE) == WAIT_OBJECT_0)
	{
		clientid = buffer->clientid;
		printf("Client initialized with ID: %d...\n", clientid);
	}

	while (1)
	{
		if (_kbhit())
		{
			int ch = _getch();
			if (ch == KEY_ESC)
			{
				buffer->cmd = CMD_EXIT;
				break;
			}

			buffer->cmd = CMD_KEYDOWN;
			buffer->clientid = clientid;

			if (buffer->clientid >= 0)
				buffer->state[buffer->clientid].key = ch;
		}

		if (WaitForSingleObject(closeevent, 0) == WAIT_OBJECT_0)
		{
			printf("Server closed, shutting down Client...\n");
			fflush(stdout);
			break;
		}
	}

	UnmapViewOfFile(buffer);
	CloseHandle(mapfile);
	CloseHandle(closeevent);
	CloseHandle(getclientid);

	return(0);
}

void WindowsError(void)
{
	LPVOID wlpmsgbuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&wlpmsgbuf,
		0,
		NULL
	);

	MessageBox(NULL, wlpmsgbuf, L"GetLastError", MB_OK | MB_ICONINFORMATION);
	LocalFree(wlpmsgbuf);
}
