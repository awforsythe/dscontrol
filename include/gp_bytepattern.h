#pragma once

#include <cinttypes>
#include <vector>
#include <string>

/** Represents a known sequence of bytes, optionally containing wildcards which may
	match any byte, parsed from a string; e.g. '41 0C FF xx xx xx xx EF EF'.
*/
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
