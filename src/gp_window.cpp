#include "gp_window.h"

#include <cstdio>
#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct _gp_window_search
{
	uint32_t pid;
	const wchar_t* class_name;
	void* result;
};

static BOOL enum_windows_callback(HWND hwnd, LPARAM param)
{
	_gp_window_search& search = *reinterpret_cast<_gp_window_search*>(param);
	DWORD pid;
	if (GetWindowThreadProcessId(hwnd, &pid) && pid == search.pid)
	{
		static const size_t buf_size = 256;
		wchar_t buf[buf_size];
		const size_t class_name_len = GetClassNameW(hwnd, buf, buf_size);
		if (class_name_len > 0 && class_name_len + 1 < buf_size)
		{
			if (wcscmp(buf, search.class_name) == 0)
			{
				SetLastError(-1);
				search.result = hwnd;
				return FALSE;
			}
		}
	}
	return TRUE;
}

gp_window::gp_window()
	: handle(nullptr)
{
}

bool gp_window::find(uint32_t pid, const wchar_t* window_class_name)
{
	assert(window_class_name);

	_gp_window_search search{ pid, window_class_name, nullptr };
	EnumWindows(&enum_windows_callback, reinterpret_cast<LPARAM>(&search));
	if (!search.result)
	{
		return false;
	}

	handle = search.result;
	return true;
}

bool gp_window::await(uint32_t pid, const wchar_t* window_class_name, uint32_t timeout_ms)
{
	const uint32_t interval = 100;
	uint32_t timeout_elapsed = 0;
	do
	{
		const bool found = find(pid, window_class_name);
		if (found)
		{
			return true;
		}
		Sleep(interval);
		timeout_elapsed += interval;
	}
	while (timeout_elapsed <= timeout_ms);
	return false;
}

void gp_window::move_to(int32_t x, int32_t y)
{
	RECT rect;
	GetWindowRect(reinterpret_cast<HWND>(handle), &rect);
	const int32_t width = rect.right - rect.left;
	const int32_t height = rect.bottom - rect.top;

	// Compensate for window borders (TODO: this is hacky and probably not portable)
	const int32_t fudge_x = -8;
	const int32_t fudge_y = -24;
	MoveWindow(reinterpret_cast<HWND>(handle), x + fudge_x, y + fudge_y, width, height, TRUE);
}

void gp_window::activate()
{
	SetForegroundWindow(reinterpret_cast<HWND>(handle));
}
