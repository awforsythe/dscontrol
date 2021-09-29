#pragma once

#include <cinttypes>

struct gp_process;

struct ds_bases
{
	uint8_t* stats;
	uint8_t* camera;
	uint8_t* world_chr;
	uint8_t* chr_class;
	uint8_t* chr_class_warp;
	uint8_t* bonfire_warp;

	bool resolve(const gp_process& process);
};
