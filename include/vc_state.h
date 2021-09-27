#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cinttypes>

#include "si_control.h"

#include "ViGEm/Common.h"

struct vc_state
{
	XUSB_REPORT report;

	vc_state();
	void reset();
	void set_left_stick(float x, float y);

	void update_stick(si_control control, float angle, float distance);
	void update_button(si_control control, bool is_down);
};
