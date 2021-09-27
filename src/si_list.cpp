#include "si_list.h"

#include <cinttypes>
#include <cassert>
#include <cstring>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static bool join_paths(wchar_t buf[MAX_PATH], const wchar_t* dirpath, const wchar_t* filename)
{
	const size_t dirpath_len = wcslen(dirpath);
	const size_t filename_len = wcslen(filename);
	if (dirpath_len + 1 + filename_len + 1 >= MAX_PATH)
	{
		return false;
	}
	wcscpy(buf, dirpath);
	buf[dirpath_len] = '\\';
	wcscpy(buf + dirpath_len + 1, filename);
	return true;
}

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

bool si_list::load(const wchar_t* dirpath)
{
	const DWORD dir_attribs = GetFileAttributes(dirpath);
	if (dir_attribs == INVALID_FILE_ATTRIBUTES || (dir_attribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		printf("ERROR: '%ls' is not a valid directory\n", dirpath);
		return false;
	}

	wchar_t search_pattern[MAX_PATH];
	const wchar_t* wildcard = L"*.yml";
	if (!join_paths(search_pattern, dirpath, wildcard))
	{
		printf("ERROR: MAX_PATH exceeded\n");
		return false;
	}

	WIN32_FIND_DATA found;
	HANDLE handle = FindFirstFile(search_pattern, &found);
	if (handle == INVALID_HANDLE_VALUE)
	{
		printf("ERROR: Failed to find any files matching: '%ls'\n", search_pattern);
		return false;
	}

	wchar_t filepath[MAX_PATH];
	std::vector<char> buf;
	while (true)
	{
		if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			if (!join_paths(filepath, dirpath, found.cFileName))
			{
				printf("ERROR: MAX_PATH exceeded\n");
				return false;
			}

			if (!read_file(buf, filepath))
			{
				printf("ERROR: Failed to read file: %ls\n", filepath);
				return false;
			}

			si_script script;
			if (!script.parse(buf))
			{
				printf("ERROR: Failed to parse script from file: %ls\n", filepath);
				return false;
			}
			printf("Parsed script '%s' from file: %ls\n", script.name.c_str(), filepath);
			scripts.insert(std::make_pair(script.name, script));
		}

		if (!FindNextFile(handle, &found))
		{
			const DWORD error = GetLastError();
			if (error != ERROR_NO_MORE_FILES)
			{
				printf("ERROR: Encountered error 0x%X while iterating through: '%ls'\n", error, search_pattern);
				return false;
			}
			break;
		}
	}
	return true;
}

const si_script* si_list::find(const char* name) const
{
	const auto found = scripts.find(name);
	if (found != scripts.cend())
	{
		return &found->second;
	}
	return nullptr;
}
