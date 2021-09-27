#include "vc_state.h"

#include <cassert>
#include <cmath>

static int32_t map_analog_thumbstick(float unit_value)
{
	if (unit_value == 0.0f)
	{
		return 0;
	}
	const float zero_to_one = (unit_value + 1.0f) / 2.0f;
	const int32_t quantized = static_cast<int32_t>(zero_to_one * 65535);
	const int32_t shifted = quantized - 32768;
	return max(-32768, min(32767, shifted));
}

static void set_bit(uint16_t value, uint16_t bit, bool on)
{
	if (on)
	{
		value |= bit;
	}
	else
	{
		value &= ~bit;
	}
}

vc_state::vc_state()
{
	reset();
}

void vc_state::reset()
{
	XUSB_REPORT_INIT(&report);
}

void vc_state::update_stick(si_control control, float angle, float distance)
{
	assert(si_control_is_stick(control));

	SHORT& x_value = control == si_control::left_stick ? report.sThumbLX : report.sThumbRX;
	SHORT& y_value = control == si_control::left_stick ? report.sThumbLY : report.sThumbRY;

	const float x = distance * cos(angle);
	const float y = distance * sin(angle);
	x_value = map_analog_thumbstick(x);
	y_value = map_analog_thumbstick(y);
}

void vc_state::update_button(si_control control, bool is_down)
{
	assert(!si_control_is_stick(control));

	switch (control)
	{
	case si_control::button_a:
		set_bit(report.wButtons, XUSB_GAMEPAD_A, is_down);
		break;
	case si_control::button_b:
		set_bit(report.wButtons, XUSB_GAMEPAD_B, is_down);
		break;
	case si_control::button_x:
		set_bit(report.wButtons, XUSB_GAMEPAD_X, is_down);
		break;
	case si_control::button_y:
		set_bit(report.wButtons, XUSB_GAMEPAD_Y, is_down);
		break;
	case si_control::button_lb:
		set_bit(report.wButtons, XUSB_GAMEPAD_LEFT_SHOULDER, is_down);
		break;
	case si_control::button_lt:
		report.bLeftTrigger = is_down ? 255 : 0;
		break;
	case si_control::button_ls:
		set_bit(report.wButtons, XUSB_GAMEPAD_LEFT_THUMB, is_down);
		break;
	case si_control::button_rb:
		set_bit(report.wButtons, XUSB_GAMEPAD_RIGHT_SHOULDER, is_down);
		break;
	case si_control::button_rt:
		report.bRightTrigger = is_down ? 255 : 0;
		break;
	case si_control::button_rs:
		set_bit(report.wButtons, XUSB_GAMEPAD_RIGHT_THUMB, is_down);
		break;
	default:
		assert(false);
	}
}
