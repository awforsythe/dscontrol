#pragma once

#include <vector>

#include "si_control.h"

struct si_script;
struct si_event;

typedef std::vector<const si_event*> si_track;

struct si_timeline
{
	const si_script* script;
	si_track tracks[si_control_count];

	si_timeline();
	void reset();
	bool load(const si_script& in_script);
};
