#include "vc_state.h"

#include <cstdio>

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

vc_state::vc_state()
{
	reset();
}

void vc_state::reset()
{
	XUSB_REPORT_INIT(&report);
}

void vc_state::set_left_stick(float x, float y)
{
	report.sThumbLX = map_analog_thumbstick(x);
	report.sThumbLY = map_analog_thumbstick(y);
	printf("%d, %d\n", report.sThumbLX, report.sThumbLY);
}
