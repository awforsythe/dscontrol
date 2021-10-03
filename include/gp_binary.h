#pragma once

#include <cinttypes>
#include <string>

#include "gp_window.h"
#include "gp_process.h"

struct gp_binary
{
	std::wstring exe_name;
	std::wstring window_class_name;

	gp_window window;
	gp_process process;

	gp_binary(const wchar_t* in_exe_name, const wchar_t* in_window_class_name);
	bool find();
	bool launch(const wchar_t* working_dir, const uint32_t steam_app_id);
};
