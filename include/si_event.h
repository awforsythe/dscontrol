#pragma once

#include <cinttypes>

#include "si_control.h"

namespace c4 { namespace yml { class NodeRef; } }

/** Target position for an analog stick, in polar coordinates. */
struct si_stick_target
{
	float angle;
	float distance;

	si_stick_target();
};

/** Scripted input event: describes either a desired button state or a target analog
	stick position at a discrete point in time. Duration indicates how long the button
	is to be held or how long it should take the stick to move from its current
	position to the target.
*/
struct si_event
{
	static const float SINGLE_FRAME_DURATION;

	float time;
	si_control control;
	si_stick_target stick;
	float duration;

	si_event();
	bool parse(float in_time, float sequence_duration, si_control in_control, const c4::yml::NodeRef& value);
};
