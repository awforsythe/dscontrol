#include "sl_library.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <sstream>

static wchar_t s_steampath[MAX_PATH] = { 0 };
static wchar_t s_libraryfolders_vdf[MAX_PATH] = { 0 };

static bool _sl_library_init_paths()
{
	s_steampath[0] = 0;
	s_libraryfolders_vdf[0] = 0;

	DWORD value_len = sizeof(s_steampath) / sizeof(s_steampath[0]);
	if (RegGetValue(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam", L"SteamPath", RRF_RT_REG_SZ, NULL, s_steampath, &value_len) != ERROR_SUCCESS)
	{
		s_steampath[0] = 0;
		return false;
	}

	for (size_t i = 0; i < value_len; i++)
	{
		if (s_steampath[i] == '/')
		{
			s_steampath[i] = '\\';
		}
	}

	static const wchar_t vdf_relpath[] = L"\\steamapps\\libraryfolders.vdf";
	static const size_t vdf_relpath_size = sizeof(vdf_relpath) / sizeof(vdf_relpath[0]); // Incl. null terminator
	if (wcslen(s_steampath) + vdf_relpath_size > sizeof(s_libraryfolders_vdf) / sizeof(s_libraryfolders_vdf[0]))
	{
		s_steampath[0] = 0;
		return false;
	}

	wcscpy(s_libraryfolders_vdf, s_steampath);
	wcscpy(s_libraryfolders_vdf + wcslen(s_steampath), vdf_relpath);
	return true;
}

static char* _sl_library_parse_vdf_block_key(char* search_start, char* block_start)
{
	char* openquote_ptr = nullptr;
	char* closequote_ptr = nullptr;
	for (char* ptr = block_start - 1; ptr >= search_start; ptr--)
	{
		if (*ptr == '"')
		{
			if (!closequote_ptr)
			{
				closequote_ptr = ptr;
			}
			else
			{
				openquote_ptr = ptr;
				break;
			}
		}
	}

	if (!openquote_ptr || !closequote_ptr || closequote_ptr - openquote_ptr <= 1)
	{
		return nullptr;
	}

	*closequote_ptr = 0;
	return openquote_ptr + 1;
}

static std::wstring _sl_library_parse_vdf_path_value(const char* str, size_t len)
{
	char* buf = new char[len + 1];
	memset(buf, 0, len + 1);
	size_t num_bytes_written = 0;
	bool prev_was_backslash = false;
	for (size_t i = 0; i < len; i++)
	{
		const char c = str[i];
		const bool is_backslash = c == '\\';
		const bool should_write = !is_backslash || !prev_was_backslash;
		if (should_write)
		{
			buf[num_bytes_written++] = c;
		}
		prev_was_backslash = is_backslash;
	}

	const std::string value(buf);
	delete[] buf;
	return std::wstring(value.begin(), value.end());
}

struct _sl_library_block
{
	char* key;
	char* block_start;
};

static bool _sl_library_process_vdf_block(_sl_library_block* block_stack, size_t stack_size, char* block_end, std::vector<sl_library_dir>& out_dirs)
{
#if 0
	printf("process: ");
	for (size_t i = 0; i < stack_size; i++)
	{
		if (i > 0) printf(" / ");
		printf("%s", block_stack[i].key);
	}
	printf("\n");
#endif

	// Only process individual numbered entries within a top-level libraryfolders block
	if (stack_size < 2 || strcmp(block_stack[0].key, "libraryfolders") != 0)
	{
		return true;
	}

	// Each child of the libraryfolders block should have a zero-indexed key
	const int32_t library_index = atoi(block_stack[1].key);
	if (library_index == 0 && strcmp(block_stack[1].key, "0") != 0)
	{
		return false;
	}

	// Ensure our output array is large enough to contain a libraryfolder with this index
	while (out_dirs.size() <= library_index)
	{
		out_dirs.emplace_back();
	}
	sl_library_dir& out_dir = out_dirs[library_index];

	// If this is an 'apps' block within a libraryfolder block, parse each integer app ID
	if (stack_size == 3 && strcmp(block_stack[2].key, "apps") == 0)
	{
		char* ptr = block_stack[2].block_start;
		while (ptr < block_end)
		{
			char* openquote_ptr = strchr(ptr, '"');
			if (!openquote_ptr || openquote_ptr >= block_end) break;
			char* closequote_ptr = strchr(openquote_ptr + 1, '"');
			if (!closequote_ptr || closequote_ptr >= block_end || closequote_ptr - openquote_ptr <= 1) break;

			ptr = openquote_ptr + 1;
			*closequote_ptr = 0;
			const int64_t app_id = atoll(ptr);
			if (app_id != 0)
			{
				out_dir.app_ids.insert(app_id);
			}
			ptr = closequote_ptr + 1;
		}
	}
	else if (stack_size == 2)
	{
		// If this is the libraryfolder block itself, parse the "path" value
		char* ptr = strstr(block_stack[1].block_start, "\"path\"");
		if (!ptr || ptr >= block_end) return false;

		char* openquote_ptr = strchr(ptr + 7, '"');
		if (!openquote_ptr || openquote_ptr >= block_end) return false;
		char* closequote_ptr = strchr(openquote_ptr + 1, '"');
		if (!closequote_ptr || closequote_ptr >= block_end || closequote_ptr - openquote_ptr <= 1) return false;

		const size_t value_len = closequote_ptr - openquote_ptr - 1;
		out_dir.rootdir = _sl_library_parse_vdf_path_value(openquote_ptr + 1, value_len);
	}
	return true;
}

static bool _sl_library_parse_vdf(char* data, std::vector<sl_library_dir>& out_dirs)
{
	char* ptr = data;

	// Establish a stack in which we'll store the labels of blocks we're currently parsing
	static const size_t MAX_BLOCK_DEPTH = 8;
	size_t stack_size = 0;
	_sl_library_block block_stack[MAX_BLOCK_DEPTH];

	while (ptr)
	{
		// Find the next opening and closing brace from our current position
		char* next_block_start = strstr(ptr, "{");
		char* next_block_end = strstr(ptr, "}");
		if (!next_block_end) break;

		// Either enter a new block or close the current one, depending on which brace is closer
		if (next_block_start && next_block_start < next_block_end)
		{
			// To open a new block: push the block's key value onto the stack, then move into the block
			if (stack_size >= MAX_BLOCK_DEPTH) return false;
			char* key_val = _sl_library_parse_vdf_block_key(ptr, next_block_start);
			if (!key_val) return false;
			block_stack[stack_size].key = key_val;
			block_stack[stack_size].block_start = next_block_start;
			stack_size++;
			ptr = next_block_start + 1;
		}
		else
		{
			// To close the current block: pop the top value off the stack, then move past the block
			if (stack_size == 0) return false;
			if (!_sl_library_process_vdf_block(block_stack, stack_size, next_block_end, out_dirs)) return false;
			stack_size--;
			ptr = next_block_end + 1;
		}
	}
	return true;
}

void sl_library::init()
{
	library_dirs.clear();

	// Figure out where Steam is installed and resolve the path to libraryfolders.vdf
	if (!_sl_library_init_paths())
	{
		return;
	}

	// Open libraryfolders.vdf for read
	assert(s_libraryfolders_vdf[0]);
	HANDLE file_handle = CreateFile(s_libraryfolders_vdf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_handle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// Get the size of the file, and abort if it's larger than a reasonable maximum size
	static const size_t MAX_FILE_SIZE = 16384;
	LARGE_INTEGER file_size;
	if (!GetFileSizeEx(file_handle, &file_size) || file_size.HighPart > 0 || file_size.LowPart > MAX_FILE_SIZE)
	{
		CloseHandle(file_handle);
		return;
	}

	// Allocate a buffer to hold the entire contents of the file, and read it in
	char* buf = new char[file_size.LowPart];
	DWORD num_bytes_read = 0;
	BOOL read_ok = ReadFile(file_handle, buf, file_size.LowPart, &num_bytes_read, NULL);
	CloseHandle(file_handle);

	// Parse the file to read in data about which apps are installed in which Steam library dirs
	if (read_ok && num_bytes_read == file_size.LowPart)
	{
		if (!_sl_library_parse_vdf(buf, library_dirs))
		{
			library_dirs.clear();
		}
	}
	delete[] buf;
}

std::wstring sl_library::get_installed_exe_path(const sl_title& title) const
{
	for (const sl_library_dir& dir : library_dirs)
	{
		if (dir.app_ids.find(title.app_id) != dir.app_ids.cend())
		{
			std::wostringstream woss;
			woss << dir.rootdir << L"\\steamapps\\common\\" << title.dirname << L'\\' << title.exe_relpath;
			return woss.str();
		}
	}
	return L"";
}
