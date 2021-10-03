#pragma once

#include <cinttypes>

struct gp_process;

struct ds_base
{
	uint8_t* ptr;
	uint8_t* dereferenced;
};

struct ds_memmap
{
	uint8_t* playtime_addr;

	struct
	{
		ds_base postprocess;
		ds_base camera;
		ds_base world_chr;
		uint8_t* chr_class;
		ds_base chr_class_warp;
		uint8_t* bonfire_warp;
	}
	bases;

	struct
	{
		uint8_t* playtime;
	}
	stats;

	struct
	{
		uint8_t* override_flag;
		uint8_t* override_brightness_rgb;
		uint8_t* override_contrast_rgb;
		uint8_t* override_saturation;
		uint8_t* override_hue;
	}
	colorgrade;

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

	bool init(const gp_process& process);
	bool resolve(const gp_process& process);
};
