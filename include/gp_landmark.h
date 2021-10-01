#pragma once

#include <cinttypes>

#include "gp_bytepattern.h"

struct gp_process;

enum class gp_landmark_type : uint8_t
{
	relative,
	absolute
};

struct gp_landmark
{
	uint32_t offset;
	gp_landmark_type type;
	gp_bytepattern bytepattern;

	gp_landmark(uint32_t in_offset, gp_landmark_type in_type, const char* in_bytepattern_s);
	bool resolve(const gp_process& process, uint8_t*& out_addr) const;
};
