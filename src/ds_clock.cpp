#include "ds_clock.h"

#include <cinttypes>
#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ds_process.h"
#include "ds_addresses.h"

static uint64_t s_freq = 0;

static uint64_t get_timestamp()
{
	LARGE_INTEGER val;
	if (QueryPerformanceCounter(&val))
	{
		return val.QuadPart;
	}
	return 0;
}

static double get_elapsed(uint64_t from, uint64_t to)
{
	if (s_freq != 0)
	{
		return static_cast<double>(to - from) / s_freq;
	}
	return 0;
}

ds_clock::ds_clock(ds_process& in_process, ds_addresses& in_addresses)
	: process(in_process)
	, playtime_addr(in_addresses.playtime)
	, playtime(0)
	, frame_count(0)
	, prev_frame_timestamp(0)
	, real_frame_time(0.0)
{
	LARGE_INTEGER val;
	if (QueryPerformanceFrequency(&val))
	{
		s_freq = val.QuadPart;
	}
	assert(s_freq != 0);
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
	if (delta > 0)
	{
		for (uint32_t i = 0; i < delta; i += 16)
		{
			frame_count++;
		}

		const uint64_t timestamp = get_timestamp();
		if (prev_frame_timestamp != 0)
		{
			real_frame_time = get_elapsed(prev_frame_timestamp, timestamp);
		}
		prev_frame_timestamp = timestamp;
	}
	return delta;
}
