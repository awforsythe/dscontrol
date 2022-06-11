#pragma once

#include <cinttypes>

#include "gp_bytepattern.h"

struct gp_process;

/** Indicates whether the landmark encodes the offset to a pointer or is a pointer
	itself.
*/
enum class gp_landmark_type : uint8_t
{
	relative,
	absolute
};

/** Game process landmark: a known sequence of bytes at a predetermined offset in
	process memory, from which we can resolve the address of a particular base struct.
	These landmarks help us find our way from fixed reference points to the
	dynamically-allocated data that we want to manipulate, by following chains of
	pointers to re-resolve our memory map on each map load.
*/
struct gp_landmark
{
	uint32_t offset;
	gp_landmark_type type;
	gp_bytepattern bytepattern;

	gp_landmark(uint32_t in_offset, gp_landmark_type in_type, const char* in_bytepattern_s);
	bool resolve(const gp_process& process, uint8_t*& out_addr) const;
};
