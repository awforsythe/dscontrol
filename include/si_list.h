#pragma once

#include <map>
#include <string>

#include "si_script.h"

struct si_list
{
	std::map<std::string, si_script> scripts;

	bool load(const wchar_t* dirpath);
};
