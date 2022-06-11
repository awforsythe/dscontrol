#pragma once

#include <cinttypes>
#include <vector>

#include "ds_pos.h"

/** Represents a frame of telemetry data recorded from a DS1R process. */
struct ds_frame
{
	uint32_t frame_number;
	double real_time;
	ds_pos pos;

	ds_frame(uint32_t in_frame_number, double in_real_time, const ds_pos& in_pos)
		: frame_number(in_frame_number)
		, real_time(in_real_time)
		, pos(in_pos)
	{
	}
};
