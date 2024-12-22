#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include "command.h"

static void WindowsError(void);

int main(int argc, char **argv)
{
	DWORD maxbufsize = sizeof(serverstate_t);

	HANDLE mapfile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, maxbufsize, L"MyFileMappingObject");
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

	HANDLE closeevent = CreateEvent(NULL, TRUE, FALSE, L"OnServerClose");
	if (!closeevent)
	{
		WindowsError();
		UnmapViewOfFile(buffer);
		CloseHandle(mapfile);
		return(1);
	}

	HANDLE getclientid = CreateEvent(NULL, FALSE, FALSE, L"OnGetClientID");
	if (!getclientid)
	{
		WindowsError();
		CloseHandle(closeevent);
		UnmapViewOfFile(buffer);
		CloseHandle(mapfile);
		return(1);
	}

	printf("Server running. Press ESC to exit...\n");

	int numclients = 0;

	while (1)
	{
		switch (buffer->cmd)
		{
			case CMD_INIT:
				if (numclients == MAX_CLIENTS)
				{
					printf("Max clients in use, cannot accept any more clients...\n");	// just a log message
					fflush(stdout);
					break;
				}

				for (int i=0; i<MAX_CLIENTS; i++)	// find an unused client ID
				{
					if (!buffer->state[i].used)
					{
						buffer->clientid = i;
						buffer->state[i].used = true;
						numclients++;

						SetEvent(getclientid);

						printf("Client initialized with ID: %d...\n", buffer->clientid);
						break;
					}
				}

				break;

			case CMD_EXIT:
				buffer->state[buffer->clientid].used = false;	// return the client ID back to the pool
				numclients--;
				printf("Client exiting, ID: %d...\n", buffer->clientid);
				break;

			case CMD_KEYDOWN:
				printf("[Client: %d] Key down: %d\n", buffer->clientid, buffer->state[buffer->clientid].key);
				break;

			case CMD_KEYUP:
				printf("[Client: %d] Key up: %d\n", buffer->clientid, buffer->state[buffer->clientid].key);
				break;

			case CMD_NONE:
			default:
				break;
		}

		buffer->cmd = CMD_NONE;

		if (_kbhit())	// kill the server by pressing ESC
		{
			if (_getch() == KEY_ESC)
				break;
		}
	}

	SetEvent(closeevent);
	
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
