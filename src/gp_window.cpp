#include "gp_window.h"

#include <cstdio>
#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

gp_window::gp_window()
	: handle(nullptr)
{
}

bool gp_window::find(const wchar_t* window_class_name)
{
	assert(window_class_name);

	handle = FindWindowW(window_class_name, nullptr);
	if (!handle)
	{
		printf("ERROR: Window not found\n");
		return false;
	}
	return true;
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
