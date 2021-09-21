#pragma once

#include <cinttypes>
#include <vector>
#include <string>

struct ds_landmark
{
	static const uint8_t FLAGS_NONE = 0;
	static const uint8_t FLAG_ANY_VALUE = 1;

	size_t offset;
	std::vector<uint8_t> bytes;
	std::vector<uint8_t> flags;

	ds_landmark(size_t in_offset, const std::string& s);
	size_t resolve_offset(const struct ds_process& process) const;
};
