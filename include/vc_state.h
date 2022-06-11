#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cinttypes>

#include "si_control.h"

#include "ViGEm/Common.h"

/** Virtual controller state: records button states, analog stick positions, etc. for
	an input device.
*/
struct vc_state
{
	XUSB_REPORT report;

	vc_state();
	void reset();

	void update_stick(si_control control, float angle, float distance);
	void update_button(si_control control, bool is_down);
};
