#include "si_script.h"

#include <cassert>
#include <string>

#include "yaml-cpp/yaml.h"

si_script::si_script()
{
}

bool si_script::load(const char* filepath)
{
	YAML::Node root;
	try
	{
		root = YAML::LoadFile(filepath);
		assert(root.IsDefined());
	}
	catch (YAML::BadFile&)
	{
		return false;
	}

	const YAML::Node name = root["name"];
	if (!name.IsDefined())
	{
		return false;
	}

	printf("'%s': 'name' is '%s'\n", filepath, name.as<std::string>().c_str());
	return true;
}
