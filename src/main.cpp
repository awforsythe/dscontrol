#include <cstdio>
#include <cstdlib>
#include <cinttypes>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

const wchar_t* WINDOW_CLASS = L"DARK SOULS";

int main()
{
	// Find the Dark Souls Remastered window based on its window class name
	const HWND hwnd = FindWindowW(WINDOW_CLASS, NULL);
	if (hwnd == NULL)
	{
		printf("ERROR: Window not found\n");
		return 1;
	}

	// Get the pid and open the process with access rights that'll let us read memory
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (process == NULL)
	{
		printf("ERROR: Failed to open process\n");
		return 1;
	}

	// We should be able to call ReadProcessMemory etc. with this handle
	printf("OK! Process handle: %p\n", process);

	// Close our handle on the process when we're done with it
	if (CloseHandle(process) == 0)
	{
		printf("WARNING: Failed to close process handle\n");
	}
	return 0;
}
