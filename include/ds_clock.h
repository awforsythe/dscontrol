#pragma once

#include <cinttypes>

struct ds_process;
struct ds_addresses;

struct ds_clock
{
	ds_process& process;
	uint8_t* playtime_addr;
	uint32_t playtime;
	uint32_t frame_count;

	ds_clock(ds_process& in_process, ds_addresses& in_addresses);
	uint32_t read();
	
	inline void reset() { frame_count = 0; }
};
