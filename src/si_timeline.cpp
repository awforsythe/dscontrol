#include "si_timeline.h"

#include <cstdio>
#include <cassert>

#include "si_event.h"
#include "si_script.h"

si_timeline::si_timeline()
{
	reset();
}

void si_timeline::reset()
{
	for (size_t track_index = 0; track_index < si_control_count; track_index++)
	{
		si_track& track = tracks[track_index];
		track.clear();
	}
}

bool si_timeline::load(const si_script& in_script)
{
	reset();

	script = &in_script;
	assert(script->duration > 0.0f);

	// Add each event onto the respective control track
	for (const si_event& event : script->events)
	{
		const size_t control_index = static_cast<size_t>(event.control);
		assert(control_index >= 0 && control_index < si_control_count);

		si_track& track = tracks[control_index];
		if (track.size() > 0)
		{
			const si_event* prev_event = track[track.size() - 1];
			const float prev_event_end_time = prev_event->time + prev_event->duration;
			if (event.time < prev_event_end_time)
			{
				printf("ERROR: Event at %0.2f overlaps previous event at %0.2f (which lasts until %0.2f)\n", event.time, prev_event->time, prev_event_end_time);
				return false;
			}
		}
		track.push_back(&event);
	}
	return true;
}
