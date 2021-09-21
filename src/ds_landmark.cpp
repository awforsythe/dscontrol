#include "ds_landmark.h"

#include <cassert>

#include "ds_process.h"

static constexpr bool is_hex_char(const char c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}

ds_landmark::ds_landmark(size_t in_offset, const std::string& s)
	: offset(in_offset)
{
	const size_t n = s.size();
	const size_t num_bytes = (n + 1) / 3;
	if (num_bytes * 3 - 1 == n)
	{
		bytes.clear();
		bytes.reserve(num_bytes);
		flags.clear();
		flags.reserve(num_bytes);

		char buf[3] = { 0, 0, 0 };
		for (size_t offset = 0; offset < n; offset += 3)
		{
			buf[0] = s[offset];
			buf[1] = s[offset + 1];
			const char delim = s[offset + 2];
			assert(delim == 0 || delim == ' ');

			if (buf[0] == 'x' && buf[1] == 'x')
			{
				bytes.push_back(0);
				flags.push_back(FLAG_ANY_VALUE);
			}
			else
			{
				assert(is_hex_char(buf[0]) && is_hex_char(buf[1]));
				bytes.push_back(static_cast<uint8_t>(strtol(buf, nullptr, 16)));
				flags.push_back(FLAGS_NONE);
			}
		}
	}
	assert(bytes.size() == num_bytes);
	assert(flags.size() == num_bytes);
}

size_t ds_landmark::resolve_offset(const ds_process& process) const
{
	std::vector<uint8_t> buf(bytes.size());
	if (!process.read(offset, buf.data(), buf.size()))
	{
		return 0;
	}

	for (size_t i = 0; i < buf.size(); i++)
	{
		const bool is_wildcard = (flags[i] & FLAG_ANY_VALUE) != 0;
		if (!is_wildcard && buf[i] != bytes[i])
		{
			return 0;
		}
	}

	return offset + *reinterpret_cast<uint32_t*>(&buf[3]) + 7;
}
