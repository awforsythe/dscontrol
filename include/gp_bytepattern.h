#pragma once

#include <cinttypes>
#include <vector>
#include <string>

struct gp_bytepattern
{
	static const uint8_t FLAGS_NONE = 0;
	static const uint8_t FLAG_ANY_VALUE = 1;

	std::vector<uint8_t> bytes;
	std::vector<uint8_t> flags;

	gp_bytepattern(const char* s);
	bool match(const uint8_t* bytes) const;

	inline size_t size() const { return bytes.size(); }
};