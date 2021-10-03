#pragma once

#include <cinttypes>

struct gp_window
{
	void* handle;

	gp_window();
	bool find(uint32_t pid, const wchar_t* window_class_name);
	bool await(uint32_t pid, const wchar_t* window_class_name, uint32_t timeout_ms);
	void move_to(int32_t x, int32_t y);
	void activate();
};
