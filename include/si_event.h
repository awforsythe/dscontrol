#pragma once

#include <cinttypes>

#include "si_control.h"

namespace c4 { namespace yml { class NodeRef; } }

struct si_stick_target
{
	float angle;
	float distance;

	si_stick_target();
};

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
