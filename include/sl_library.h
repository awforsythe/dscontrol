#pragma once

#include <cinttypes>
#include <string>
#include <set>
#include <vector>

#include "sl_title.h"

struct sl_library_dir
{
	std::wstring rootdir;
	std::set<uint64_t> app_ids;
};

struct sl_library
{
	std::vector<sl_library_dir> library_dirs;
	void init();

	std::wstring get_installed_exe_path(const sl_title& title) const;
};
