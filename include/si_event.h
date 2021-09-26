#pragma once

#include <cinttypes>

#include "si_control.h"

namespace c4 { namespace yml { class NodeRef; } }

struct si_event
{
	static const float SINGLE_FRAME_DURATION;

	float time;
	si_control control;
	si_control_state state;
	float duration;

	si_event();
	bool parse(float in_time, si_control in_control, const c4::yml::NodeRef& value);
};
