#include "si_control.h"

#include <cstring>

bool si_control_parse(const char* name, size_t len, si_control& out_control)
{
	const size_t num_values = sizeof(si_control_names) / sizeof(si_control_names[0]);
	for (size_t index = 0; index < num_values; index++)
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
