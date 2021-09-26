#pragma once

#include <string>
#include <vector>

#include "si_event.h"

struct si_script
{
	std::string name;
	float duration;
	std::vector<si_event> events;

	si_script();
	bool parse(std::vector<char>& buf);
};
