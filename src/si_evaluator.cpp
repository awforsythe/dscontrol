#include "si_evaluator.h"

#include <cassert>

#include "si_timeline.h"
#include "si_script.h"
#include "si_event.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

si_evaluator::si_evaluator(const si_timeline& in_timeline)
	: timeline(in_timeline)
	, playback_time(0.0)
	, prev_timestamp(0)
	, control_state()
{
	LARGE_INTEGER val;
	if (QueryPerformanceFrequency(&val))
	{
		s_freq = val.QuadPart;
	}
	assert(s_freq != 0);

	for (uint8_t track_index = 0; track_index < si_control_count; track_index++)
	{
		const si_track& track = timeline.tracks[track_index];
		si_track_state& track_state = track_states[track_index];

		track_state.current_event_index = -1;
		track_state.next_event_index = track.size() > 0 ? 0 : -1;

		if (si_control_is_stick(static_cast<si_control>(track_index)))
		{
			track_state.input.stick.angle = 0.0f;
			track_state.input.stick.distance = 0.0f;
			track_state.input.stick.start_angle = 0.0f;
			track_state.input.stick.start_distance = 0.0f;
		}
		else
		{
			track_state.input.button.is_down = 0;
		}
	}
}

void si_evaluator::start()
{
	prev_timestamp = get_timestamp();
}

bool si_evaluator::tick()
{
	assert(prev_timestamp != 0);
	assert(s_freq != 0);

	const uint64_t timestamp = get_timestamp();
	const uint64_t timestamp_delta = timestamp - prev_timestamp;
	prev_timestamp = timestamp;

	const double elapsed = static_cast<double>(timestamp_delta) / s_freq;
	playback_time += elapsed;

	for (size_t track_index = 0; track_index < si_control_count; track_index++)
	{
		const si_track& track = timeline.tracks[track_index];
		si_track_state& track_state = track_states[track_index];

		if (track_state.current_event_index >= 0)
		{
			const si_event* event = track[track_state.current_event_index];
			const double end_time = static_cast<double>(event->time) + event->duration;
			if (playback_time >= end_time)
			{
				if (si_control_is_stick(event->control))
				{
					si_stick_state& stick = track_state.input.stick;
					stick.angle = event->stick.angle;
					stick.distance = event->stick.distance;
					control_state.update_stick(event->control, stick.angle, stick.distance);

					printf("%0.2f: '%s' to angle %0.2f, distance %0.2f\n", playback_time, si_control_names[static_cast<size_t>(event->control)], stick.angle, stick.distance);
				}
				else
				{
					track_state.input.button.is_down = false;
					control_state.update_button(event->control, false);

					printf("%0.2f: '%s' release\n", playback_time, si_control_names[static_cast<size_t>(event->control)]);
				}

				track_state.next_event_index = track.size() > track_state.current_event_index + 1 ? track_state.current_event_index + 1 : -1;
				track_state.current_event_index = -1;
			}
			else if (si_control_is_stick(event->control))
			{
				si_stick_state& stick = track_state.input.stick;
				if (event->time < end_time)
				{
					const float elapsed = static_cast<float>(playback_time - static_cast<double>(event->time));
					stick.angle = stick.start_angle + elapsed * (event->stick.angle - stick.start_angle);
					stick.distance = stick.start_distance + elapsed * (event->stick.distance - stick.start_distance);
					control_state.update_stick(event->control, stick.angle, stick.distance);

					printf("%0.2f: '%s' to angle %0.2f, distance %0.2f\n", playback_time, si_control_names[static_cast<size_t>(event->control)], stick.angle, stick.distance);
				}
			}
		}

		if (track_state.next_event_index >= 0)
		{
			const si_event* next_event = track[track_state.next_event_index];
			if (playback_time >= next_event->time)
			{
				if (si_control_is_stick(next_event->control))
				{
					si_stick_state& stick = track_state.input.stick;
					if (next_event->duration > 0.0f)
					{
						stick.start_angle = stick.angle;
						stick.start_distance = stick.distance;
					}
					else
					{
						stick.angle = next_event->stick.angle;
						stick.distance = next_event->stick.distance;
						control_state.update_stick(next_event->control, stick.angle, stick.distance);

						printf("%0.2f: '%s' to angle %0.2f, distance %0.2f\n", playback_time, si_control_names[static_cast<size_t>(next_event->control)], stick.angle, stick.distance);
					}
				}
				else
				{
					track_state.input.button.is_down = true;
					control_state.update_button(next_event->control, true);

					printf("%0.2f: '%s' press\n", playback_time, si_control_names[static_cast<size_t>(next_event->control)]);
				}

				track_state.current_event_index = track_state.next_event_index;
				track_state.next_event_index = -1;

			}
		}
	}

	return playback_time >= timeline.script->duration;
}
