#include "ds_clock.h"

#include "ds_process.h"
#include "ds_addresses.h"

ds_clock::ds_clock(ds_process& in_process, ds_addresses& in_addresses)
	: process(in_process)
	, playtime_addr(in_addresses.playtime)
	, playtime(0)
	, frame_count(0)
{
}

uint32_t ds_clock::read()
{
	const uint32_t prev_playtime = playtime;
	playtime = process.peek<uint32_t>(playtime_addr);
	if (prev_playtime == 0 || prev_playtime > playtime)
	{
		return 0;
	}
	const uint32_t delta = playtime - prev_playtime;
	for (uint32_t i = 0; i < delta; i += 16)
	{
		frame_count++;
	}
	return delta;
}
