#include "si_list.h"

#include <cassert>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static bool path_join(wchar_t buf[MAX_PATH], const wchar_t* dirpath, const wchar_t* filename)
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
	if (!path_join(search_pattern, dirpath, wildcard))
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
	while (true)
	{
		if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			if (!path_join(filepath, dirpath, found.cFileName))
			{
				printf("ERROR: MAX_PATH exceeded\n");
				return false;
			}

			si_script script;
			if (!script.load(filepath))
			{
				printf("ERROR: Failed to load script '%ls'\n", filepath);
				return false;
			}
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
