#pragma once

#include <cinttypes>

struct gp_process;
struct ds_addresses;

struct ds_clock
{
	gp_process& process;
	uint8_t* playtime_addr;
	uint32_t playtime;
	uint32_t frame_count;
	uint64_t prev_frame_timestamp;
	double real_frame_time;

	ds_clock(gp_process& in_process, ds_addresses& in_addresses);
	uint32_t read();
	
	inline void reset() { frame_count = 0; }
};
