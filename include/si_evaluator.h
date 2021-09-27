#pragma once

#include <cinttypes>

#include "vc_state.h"
#include "si_control.h"

struct si_timeline;

struct si_stick_state
{
	float angle;
	float distance;
	float start_angle;
	float start_distance;
};

struct si_button_state
{
	bool is_down;
};

struct si_track_state
{
	int32_t current_event_index;
	int32_t next_event_index;
	
	union
	{
		si_stick_state stick;
		si_button_state button;
	}
	input;
};

struct si_evaluator
{
	const si_timeline& timeline;

	double playback_time;
	uint64_t prev_timestamp;
	
	si_track_state track_states[si_control_count];
	vc_state control_state;

	si_evaluator(const si_timeline& in_timeline);
	void start();
	bool tick();
};
