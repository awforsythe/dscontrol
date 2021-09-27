#pragma once

#include <vector>

#include "si_control.h"

struct si_event;
struct si_script;

struct si_track
{
	std::vector<const si_event*> events;
	size_t current_event_index;
	size_t next_event_index;

	si_track();
	void reset();
	bool add(const si_event& event);
};

struct si_timeline
{
	float position;
	float duration;
	si_track tracks[si_control_count];

	si_timeline();
	void reset();
	bool load(const si_script& script);
};
