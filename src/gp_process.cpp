#include "gp_process.h"

#include <cassert>
#include <cstdio>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>

static void* get_base_address(const wchar_t* module_name, DWORD pid)
{
	void* base_address = nullptr;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	MODULEENTRY32 entry = { 0 };
	entry.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(snapshot, &entry))
	{
		do
		{
			if (wcscmp(entry.szModule, module_name) == 0)
			{
				base_address = entry.modBaseAddr;
				break;
			}

		} while (Module32Next(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return base_address;
}

gp_process::gp_process()
	: pid(0)
	, handle(nullptr)
	, module_addr(nullptr)
{
}

gp_process::~gp_process()
{
	if (handle)
	{
		CloseHandle(handle);
	}
}

bool gp_process::open(void* window_handle, const wchar_t* module_name)
{
	assert(window_handle);
	assert(module_name);

	GetWindowThreadProcessId((HWND)window_handle, (LPDWORD)&pid);
	handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!handle)
	{
		printf("ERROR: Failed to open process\n");
		return false;
	}

	module_addr = reinterpret_cast<uint8_t*>(get_base_address(module_name, pid));
	if (!module_addr)
	{
		printf("ERROR: Failed to get module base address\n");
		return false;
	}

	return true;
}

bool gp_process::read(const uint8_t* addr, uint8_t* buf, size_t size) const
{
	size_t num_bytes_read = 0;
	if (addr && ReadProcessMemory(handle, addr, buf, size, &num_bytes_read) != 0)
	{
		assert(num_bytes_read == size);
		return true;
	}
	return false;
}

bool gp_process::write(uint8_t* addr, const uint8_t* buf, size_t size) const
{
	assert(addr);

	size_t num_bytes_written = 0;
	if (WriteProcessMemory(handle, addr, buf, size, &num_bytes_written) != 0)
	{
		assert(num_bytes_written == size);
		return true;
	}
	return false;
}
