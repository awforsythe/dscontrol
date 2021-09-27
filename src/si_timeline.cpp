#include "si_timeline.h"

#include <cstdio>
#include <cassert>

#include "si_event.h"
#include "si_script.h"

si_track::si_track()
{
	reset();
}

void si_track::reset()
{
	events.clear();
	current_event_index = -1;
	next_event_index = -1;
}

bool si_track::add(const si_event& event)
{
	const si_event* prev_event = events.size() > 0 ? events[events.size() - 1] : nullptr;
	if (prev_event)
	{
		const float prev_event_end_time = prev_event->time + prev_event->duration;
		if (event.time < prev_event_end_time)
		{
			printf("ERROR: Event at %0.2f overlaps prior event at %0.2f (which lasts until %0.2f)\n", event.time, prev_event->time, prev_event_end_time);
			return false;
		}
	}
	events.push_back(&event);
	return true;
}

si_timeline::si_timeline()
{
	reset();
}

void si_timeline::reset()
{
	position = 0.0f;
	duration = 0.0f;

	for (size_t track_index = 0; track_index < si_control_count; track_index++)
	{
		si_track& track = tracks[track_index];
		track.reset();
	}
}

bool si_timeline::load(const si_script& script)
{
	reset();

	// Initialize the timeline with the playback duration
	duration = script.duration;
	assert(duration > 0.0f);

	// Add each event onto the respective control track
	for (const si_event& event : script.events)
	{
		const size_t control_index = static_cast<size_t>(event.control);
		assert(control_index >= 0 && control_index < si_control_count);
		si_track& track = tracks[control_index];

		if (!track.add(event))
		{
			printf("ERROR: Failed to add '%s' event to track\n", si_control_names[control_index]);
			return false;
		}
	}

	// Initialize the playback state of each track
	for (size_t track_index = 0; track_index < si_control_count; track_index++)
	{
		si_track& track = tracks[track_index];
		if (track.events.size() > 0)
		{
			track.next_event_index = 0;
		}
	}

	return true;
}
