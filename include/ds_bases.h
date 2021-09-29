#pragma once

#include <cinttypes>

struct gp_process;

struct ds_bases
{
	uint8_t* stats;
	uint8_t* camera;
	uint8_t* world_chr;

	bool resolve(const gp_process& process);
};
