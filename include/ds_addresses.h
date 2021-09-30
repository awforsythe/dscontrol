#pragma once

#include <cinttypes>

struct gp_process;
struct ds_bases;

struct ds_addresses
{
	uint8_t* playtime;

	struct
	{
		uint8_t* target_pitch;
	}
	camera;

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

	struct
	{
		uint8_t* last_bonfire;
	}
	chr_class_warp;

	bool resolve(const gp_process& process, const ds_bases& bases);
};
