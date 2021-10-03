#pragma once

#include <cinttypes>

struct gp_process;
struct ds_memmap;

struct ds_clock
{
	gp_process& process;
	const uint8_t* playtime_addr;
	uint32_t playtime;
	uint32_t frame_count;
	uint64_t prev_frame_timestamp;
	double real_frame_time;

	ds_clock(gp_process& in_process, const uint8_t* in_playtime_addr);
	uint32_t read();
	
	inline void reset() { frame_count = 0; }
};
