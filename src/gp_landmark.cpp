#include "gp_landmark.h"

#include <cassert>

#include "gp_process.h"

static constexpr bool is_hex_char(const char c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}

gp_landmark::gp_landmark(const std::string& s)
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
		for (size_t i = 0; i < n; i += 3)
		{
			buf[0] = s[i];
			buf[1] = s[i + 1];
			const char delim = s[i + 2];
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

bool gp_landmark::match(const uint8_t* buf) const
{
	for (size_t i = 0; i < bytes.size(); i++)
	{
		const bool is_wildcard = (flags[i] & FLAG_ANY_VALUE) != 0;
		if (!is_wildcard && buf[i] != bytes[i])
		{
			return false;
		}
	}
	return true;
}
