#pragma once

#include <string>

struct si_script
{
	std::string name;

	si_script();
	bool load(const wchar_t* filepath);
};
