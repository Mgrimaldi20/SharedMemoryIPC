#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

static void WindowsError(void);

int main(int argc, char **argv)
{
	DWORD maxbufsize = 4096;

	HANDLE mapfile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, maxbufsize, L"MyFileMappingObject");
	if (!mapfile)
	{
		WindowsError();
		return(1);
	}

	unsigned char *buffer = MapViewOfFile(mapfile, FILE_MAP_ALL_ACCESS, 0, 0, maxbufsize);
	if (!buffer)
	{
		WindowsError();
		CloseHandle(mapfile);
		return(1);
	}

	snprintf(buffer, maxbufsize, "Hello, World!");
	printf("Sent: %s\n", buffer);

	printf("Press any key to continue...\n");
	int c = getchar();

	UnmapViewOfFile(buffer);
	CloseHandle(mapfile);

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
