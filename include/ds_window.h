#pragma once

#include <cinttypes>

struct ds_window
{
	void* handle;

	ds_window();
	bool find(const wchar_t* window_class_name);
	void move_to(int32_t x, int32_t y);
	void activate();
};
