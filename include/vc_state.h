#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cinttypes>

#include "ViGEm/Common.h"

struct vc_state
{
	XUSB_REPORT report;

	vc_state();
	void reset();
	void set_left_stick(float x, float y);
};
