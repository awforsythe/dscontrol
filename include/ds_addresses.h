#pragma once

#include <cinttypes>

struct ds_addresses
{
	uint8_t* playtime;

	struct
	{
		uint8_t* pos;
		uint8_t* angle;
		uint8_t* latch;
	}
	chr_warp;

	struct
	{
		uint8_t* pos;
		uint8_t* angle;
	}
	chr_pos;

	bool resolve(const struct ds_process& process);
};
