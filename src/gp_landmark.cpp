#include "gp_landmark.h"

#include <cassert>
#include <vector>

#include "gp_process.h""

gp_landmark::gp_landmark(uint32_t in_offset, gp_landmark_type in_type, const char* s)
	: offset(in_offset)
	, type(in_type)
	, bytepattern(s)
{
}

bool gp_landmark::resolve(const gp_process& process, uint8_t*& out_addr) const
{
	std::vector<uint8_t> buf(bytepattern.size());
	
	uint8_t* bytepattern_addr = process.to_addr(offset);
	if (!process.read(bytepattern_addr, buf.data(), buf.size()))
	{
		return false;
	}

	if (!bytepattern.match(buf.data()))
	{
		return false;
	}

	if (type == gp_landmark_type::absolute)
	{
		out_addr = bytepattern_addr;
		return true;
	}

	// 3 bytes into the landmark is a 4-byte offset: read that value, then move
	// down from the landmark by that distance, plus 7 bytes, to get to our base
	// address
	assert(type == gp_landmark_type::relative);
	uint32_t offset_from_landmark = process.peek<uint32_t>(bytepattern_addr + 3);
	if (!offset_from_landmark)
	{
		return false;
	}
	out_addr = bytepattern_addr + offset_from_landmark + 7;
	return true;
}
