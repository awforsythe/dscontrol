#pragma once

#include <cinttypes>

#include "vc_state.h"
#include "si_control.h"

struct si_timeline;

/** Input-related playback state for tracks that control analog sticks: describes the
	state of the stick at the current playback position.
*/
struct si_stick_state
{
	float angle;
	float distance;
	float start_angle;
	float start_distance;
};

/** Input-related playback state for tracks that control buttons: describes the state
	of the button at the current playback position.
*/
struct si_button_state
{
	bool is_down;
};

/** Track state: playback state for a single track, associated with a single input
	control. For each control, we can only process a single event at a time, so we
	simply track our current input state, the event that we're currently processing (if
	any), and the next event that will be coming up (if any) when that event is done.
*/
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

/** Scripted input evaluator: contains playback state for a single loaded script.
	Evaluates that script frame-by-frame, resulting in a vc_state struct describing the
	desired state of the gamepad at that frame.
*/
struct si_evaluator
{
	const si_timeline* timeline;

	double playback_time;
	si_track_state track_states[si_control_count];
	vc_state control_state;

	si_evaluator();
	void reset(const si_timeline& in_timeline);
	bool tick(double elapsed);
};
