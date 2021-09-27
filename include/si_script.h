#pragma once

#include <string>
#include <vector>

#include "si_event.h"
#include "ds_pos.h"

struct si_script
{
	std::string name;
	float duration;
	ds_pos warp_pos;
	float settle_time;
	std::vector<si_event> events;

	si_script();
	bool parse(std::vector<char>& buf);
};
