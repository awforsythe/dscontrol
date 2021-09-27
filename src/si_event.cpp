#include "si_event.h"

#include <cstring>

#include "ryml_std.hpp"
#include "ryml.hpp"

const float si_event::SINGLE_FRAME_DURATION = 0.001f;

static bool parse_angle(const char* s, const size_t len, float& out_angle)
{
	// Helper struct for comparing against known, named directions
	struct _direction {
		const char* s; size_t len; float angle;
		_direction(const char* in_s, float in_angle)
			: s(in_s), len(strlen(in_s)) , angle(in_angle) {}
	};
	static const size_t NUM_DIRECTIONS = 4;
	static const _direction DIRECTIONS[NUM_DIRECTIONS] = {
		{ "north", 1.5708f }, { "south", -1.5708f }, { "east", 0.0f }, { "west", 3.14159f },
	};

	// Check our string value against each named direction, to see if it starts with that name
	for (size_t index = 0; index < NUM_DIRECTIONS; index++)
	{
		const _direction& direction = DIRECTIONS[index];
		if (len >= direction.len && strncmp(s, direction.s, direction.len) == 0)
		{
			// If a named direction is specified exactly, use its angle value and be done
			out_angle = direction.angle;
			if (len == direction.len)
			{
				return true;
			}

			// Otherwise, parse a numeric suffix, e.g. 'north+45' for NE, or 'west-10' for 10 degrees CCW of west
			if (len > direction.len)
			{
				// Require that the first extra character is either + or -
				const char sign_char = s[direction.len];
				if (sign_char != '+' && sign_char != '-')
				{
					printf("ERROR: Desired stick state of '%.*s' has invalid suffix\n", static_cast<int>(len), s);
					return false;
				}

				// We can have exactly one or two extra characters after the sign
				if (len == direction.len + 1 || len > direction.len + 3)
				{
					printf("ERROR: Desired stick state of '%.*s' has invalid suffix length ('+89' is max value)\n", static_cast<int>(len), s);
					return false;
				}

				// Explicitly allow +0 or -0, so we can treat 0 as a sentinel for atoi
				if (len == direction.len + 2 && s[direction.len + 1] == '0')
				{
					return true;
				}

				// Copy our numeric suffix into a local null-terminated buffer so we can use atoi
				const char buf[3] = { s[direction.len + 1], len > direction.len + 1 ? s[direction.len + 2] : 0, 0 };
				const int32_t delta_val = atoi(buf);
				if (delta_val <= 0)
				{
					printf("ERROR: Desired stick state of '%.*s' has invalid delta-angle in suffix\n", static_cast<int>(len), s);
					return false;
				}
				if (delta_val >= 90)
				{
					printf("ERROR: Desired stick state of '%.*s' has excessive delta-angle in suffix (valid range is -89 to +89)\n", static_cast<int>(len), s);
					return false;
				}

				// We want to treat + as clockwise and - as counter-clockwise: our angle winds CCW, so the sign is inverted
				// TODO: Wrap
				const float sign = sign_char == '+' ? -1.0f : 1.0f;
				const float delta_angle = static_cast<float>(delta_val) / 57.2958f;
				out_angle += delta_angle * sign;
				return true;
			}
		}
	}

	// If our string value didn't match any of our named directions, fail
	printf("ERROR: Desired stick state of '%.*s' is not a valid direction\n", static_cast<int>(len), s);
	return false;
}

static bool init_from_stick(const c4::yml::NodeRef& value, bool is_map, si_event& out_event)
{
	// Resolve direction string, from (e.g.) either 'north' or { to: 'north' }
	const char* direction_str;
	size_t direction_len;
	if (is_map)
	{
		if (!value.has_child("to") || !value["to"].is_keyval())
		{
			printf("ERROR: Control state value for stick is missing required 'to' key\n");
			return false;
		}
		direction_str = value["to"].val().data();
		direction_len = value["to"].val().len;
	}
	else
	{
		direction_str = value.val().data();
		direction_len = value.val().len;
	}

	// A direction of 'neutral' is a special case
	static const char* DIRECTION_NEUTRAL = "neutral";
	static const size_t DIRECTION_NEUTRAL_LEN = strlen(DIRECTION_NEUTRAL);
	const bool is_neutral = direction_len >= DIRECTION_NEUTRAL_LEN && strncmp(direction_str, DIRECTION_NEUTRAL, DIRECTION_NEUTRAL_LEN) == 0;
	if (is_neutral)
	{
		out_event.stick.angle = 0.0f;

		if (direction_len > DIRECTION_NEUTRAL_LEN)
		{
			printf("ERROR: Desired stick state of 'neutral' should not be suffixed\n");
			return false;
		}
	}
	else
	{
		// If non-neutral, parse the angle with optional suffix, e.g. 'north+10'
		if (!parse_angle(direction_str, direction_len, out_event.stick.angle))
		{
			printf("ERROR: Failed to parse angle from desired stick state\n");
			return false;
		}
	}

	// Parse the distance the stick should be moved toward the target position; e.g. { to: north, dist: 0.5 } is halfway up
	out_event.stick.distance = is_neutral ? 0.0f : 1.0f;
	if (is_map && value.has_child("dist"))
	{
		if (is_neutral)
		{
			printf("ERROR: Desired stick state of 'neutral' should not include a 'dist' value\n");
			return false;
		}

		const c4::yml::NodeRef dist_node = value["dist"];
		if (!dist_node.is_keyval() || !dist_node.val().is_number())
		{
			printf("ERROR: Control state value for stick has invalid 'dist' value\n");
			return false;
		}

		dist_node >> out_event.stick.distance;
		if (out_event.stick.distance == 0.0f)
		{
			printf("ERROR: Control state value for stick has 'dist' value of zero; use 'neutral' direction instead\n");
			return false;
		}

		if (out_event.stick.distance < 0.0f || out_event.stick.distance > 1.0f)
		{
			printf("ERROR: Control state value for stick has out-of-range 'dist' value; valid range is (0.0f, 1.0f]\n");
			return false;
		}
	}

	// Parse the duration of the event if not instant, e.g. { to: east, over: 2.0 } moves the stick from its current position to east, over 2 seconds
	out_event.duration = 0.0f;
	if (is_map && value.has_child("over"))
	{
		const c4::yml::NodeRef over_node = value["over"];
		if (!over_node.is_keyval() || !over_node.val().is_number())
		{
			printf("ERROR: Control state value for stick has invalid 'over' value\n");
			return false;
		}

		over_node >> out_event.duration;
		if (out_event.duration < 0.0f)
		{
			printf("ERROR: Control state value for stick has negative 'over' value\n");
			return false;
		}
	}

	return true;
}

static bool init_from_button(const c4::yml::NodeRef& value, bool is_map, si_event& out_event)
{
	// If the value for a button is a map, it may only be { hold: <number> }
	if (is_map)
	{
		if (!value.has_child("hold"))
		{
			printf("ERROR: Button action is missing required 'hold' key\n");
			return false;
		}

		if (value.num_children() > 1)
		{
			printf("ERROR: Button action has unrecognized keys (only 'hold' is supported)\n");
			return false;
		}

		const c4::yml::NodeRef hold_node = value["hold"];
		if (!hold_node.is_keyval() || !hold_node.val().is_number())
		{
			printf("ERROR: Button action has 'hold' key with invalid value (must be numeric)\n");
			return false;
		}

		hold_node >> out_event.duration;
		if (out_event.duration <= 0.0f)
		{
			printf("ERROR: Button action has invalid 'hold' duration (must be positive)\n");
			return false;
		}
		return true;
	}

	// Otherwise, a string value associated with a button control may only be 'press'
	static const char* ACTION_PRESS = "press";
	static const size_t ACTION_PRESS_LEN = strlen(ACTION_PRESS);
	if (value.val().len != ACTION_PRESS_LEN || strncmp(value.val().data(), ACTION_PRESS, ACTION_PRESS_LEN) != 0)
	{
		printf("ERROR: Button action '%.*s' is invalid\n", static_cast<int>(value.val().len), value.val().data());
		return false;
	}
	out_event.duration = si_event::SINGLE_FRAME_DURATION;
	return true;
}

si_stick_target::si_stick_target()
	: angle(0.0f)
	, distance(0.0f)
{
}

si_event::si_event()
	: time(0.0f)
	, control(si_control::left_stick)
	, stick()
	, duration(0.0f)
{
}

bool si_event::parse(float in_time, float sequence_duration, si_control in_control, const c4::yml::NodeRef& value)
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
		if (!init_from_stick(value, value_is_map, *this))
		{
			return false;
		}
	}
	else
	{
		if (!init_from_button(value, value_is_map, *this))
		{
			return false;
		}
	}
	
	if (in_time + duration > sequence_duration)
	{
		printf("ERROR: Event with duration %0.2f would finish at time %0.2f, exceeding sequence duration of %0.2f\n", duration, in_time + duration, sequence_duration);
		return false;
	}
	return true;
}
