#pragma once

#include <cinttypes>

/** Scripted input control: identifies a single button or analog stick on the gamepad.
*/
enum class si_control : uint8_t
{
	left_stick,
	right_stick,
	button_a,
	button_b,
	button_x,
	button_y,
	button_lb,
	button_lt,
	button_ls,
	button_rb,
	button_rt,
	button_rs
};

static const char* si_control_names[] =
{
	"left_stick",
	"right_stick",
	"button_a",
	"button_b",
	"button_x",
	"button_y",
	"button_lb",
	"button_lt",
	"button_ls",
	"button_rb",
	"button_rt",
	"button_rs"
};

static const size_t si_control_count = sizeof(si_control_names) / sizeof(si_control_names[0]);

bool si_control_parse(const char* name, size_t len, si_control& out_control);
bool si_control_is_stick(si_control control);
