#include "si_script.h"

#include <fstream>
#include <vector>

#include "ryml_std.hpp"
#include "ryml.hpp"

si_script::si_script()
{
}

bool si_script::load(const char* filepath)
{
	std::ifstream file;
	file.open(filepath);
	file.seekg(0, std::ios::end);
	const size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buf(size);
	file.read(buf.data(), size);
	file.close();

	ryml::substr view = c4::basic_substring<char>(buf.data(), size);
	ryml::Tree tree = ryml::parse(view);
	ryml::NodeRef root = tree.rootref();

	if (!root.is_map() || !root.has_child("name"))
	{
		return false;
	}

	root["name"] >> name;
	printf("'%s': 'name' is '%s'\n", filepath, name.c_str());
	return true;
}
