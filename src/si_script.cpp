#include "si_script.h"

#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "ryml_std.hpp"
#include "ryml.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static bool read_file(std::vector<char>& buf, const wchar_t* filepath)
{
	HANDLE file = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	LARGE_INTEGER size64;
	if (!GetFileSizeEx(file, &size64) || size64.HighPart != 0)
	{
		CloseHandle(file);
		return false;
	}

	const uint32_t size = size64.LowPart;
	buf.resize(size);

	DWORD num_bytes_read = 0;
	const BOOL file_was_read = ReadFile(file, buf.data(), size, &num_bytes_read, nullptr);
	CloseHandle(file);
	return file_was_read && num_bytes_read == size;
}

si_script::si_script()
{
}

bool si_script::load(const wchar_t* filepath)
{
	std::vector<char> buf;
	if (!read_file(buf, filepath))
	{
		return false;
	}

	ryml::substr view = c4::basic_substring<char>(buf.data(), buf.size());
	ryml::Tree tree = ryml::parse(view);
	ryml::NodeRef root = tree.rootref();

	if (!root.is_map() || !root.has_child("name"))
	{
		return false;
	}

	root["name"] >> name;
	printf("'%ls': 'name' is '%s'\n", filepath, name.c_str());
	return true;
}
