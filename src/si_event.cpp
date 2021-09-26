#include "si_event.h"

#include <cstring>

#include "ryml_std.hpp"
#include "ryml.hpp"

const float si_event::SINGLE_FRAME_DURATION = 0.001f;

si_event::si_event()
	: time(0.0f)
	, control(si_control::left_stick)
	, duration(0.0f)
{
	memset(&state, 0, sizeof(state));
}

bool si_event::parse(float in_time, si_control in_control, const c4::yml::NodeRef& value)
{
	const bool value_is_map = value.is_map();
	if (!value_is_map && !value.is_keyval())
	{
		printf("ERROR: Control state value is not a map or a keyval\n");
		return false;
	}

	time = in_time;
	control = in_control;

	if (si_control_is_stick(in_control))
	{
		// For analog sticks, we support these value formats:
		// - a string indicating desired direction to apply instantly, e.g. 'neutral', 'north', 'east'
		// - an object containing the required key 'to', equivalent to the above, e.g. { to: 'north' } is the same as 'north'
		//   - optional key 'over', mapped to a duration, e.g. { to: 'east', over: 1.0 } starts moving the stick from its current position to east over 1 second
	}
	else
	{
		// For buttons, we only support 'press' or '{ hold: 1.0 }'
	}

	return false; // NYI
}
