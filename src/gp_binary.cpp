#include "gp_binary.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

gp_binary::gp_binary(const wchar_t* in_exe_name, const wchar_t* in_window_class_name)
	: exe_name(in_exe_name)
	, window_class_name(in_window_class_name)
{
}

bool gp_binary::find()
{
	if (window.find(window_class_name.c_str()))
	{
		return process.open(window.handle, exe_name.c_str());
	}
	return false;
}

bool gp_binary::launch(const wchar_t* working_dir, const uint32_t steam_app_id)
{
	return false;
}
