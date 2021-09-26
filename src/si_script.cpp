#include "si_script.h"

#include "ryml_std.hpp"
#include "ryml.hpp"

si_script::si_script()
{
}

bool si_script::parse(std::vector<char>& buf)
{
	ryml::substr view = c4::basic_substring<char>(buf.data(), buf.size());
	ryml::Tree tree = ryml::parse(view);
	ryml::NodeRef root = tree.rootref();

	if (!root.is_map() || !root.has_child("name"))
	{
		return false;
	}

	root["name"] >> name;
	return true;
}
