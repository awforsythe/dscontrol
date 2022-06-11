#pragma once

#include <map>
#include <string>

#include "si_script.h"

/** A list of script files loaded from disk. */
struct si_list
{
	std::map<std::string, si_script> scripts;

	bool load(const wchar_t* dirpath);
	const si_script* find(const char* name) const;
};
