#pragma once

#include <string>

struct si_script
{
	std::string name;

	si_script();
	bool load(const char* filepath);
};
