#pragma once

#include <cinttypes>
#include <string>

#include "gp_window.h"
#include "gp_process.h"

struct gp_binary
{
	std::wstring exe_path;
	std::wstring window_class_name;
	uint32_t steam_app_id;

	std::wstring working_dir;
	std::wstring exe_name;

	gp_binary(const wchar_t* in_exe_path, const wchar_t* in_window_class_name, uint32_t in_steam_app_id);
	bool find_process(gp_process& process);
	bool launch_process(gp_process& process);
};
