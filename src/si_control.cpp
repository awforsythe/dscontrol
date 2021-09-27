#include "si_control.h"

#include <cstring>

bool si_control_parse(const char* name, size_t len, si_control& out_control)
{
	for (size_t index = 0; index < si_control_count; index++)
	{
		const char* value_name = si_control_names[index];
		if (strlen(value_name) == len && strncmp(name, value_name, len) == 0)
		{
			out_control = static_cast<si_control>(index);
			return true;
		}
	}
	return false;
}

bool si_control_is_stick(si_control control)
{
	return control == si_control::left_stick || control == si_control::right_stick;
}
