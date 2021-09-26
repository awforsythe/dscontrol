#pragma once

#include <string>
#include <vector>

struct si_script
{
	std::string name;

	si_script();
	bool parse(std::vector<char>& buf);
};
