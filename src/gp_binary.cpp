#include "gp_binary.h"

#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>

static uint32_t find_process_by_name(const wchar_t* exe_name)
{
	uint32_t pid = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot, &entry))
	{
		do
		{
			if (wcscmp(entry.szExeFile, exe_name) == 0)
			{
				pid = entry.th32ProcessID;
				break;
			}

		} while (Process32Next(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return pid;
}

static bool is_alpha(wchar_t c)
{
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static void* alloc_child_process_env(uint32_t steam_app_id)
{
	assert(steam_app_id != 0);

	// We need to insert an entry for the app id, and we must maintain (case-insensitive) lexical ordering
	static const wchar_t* key_to_add = L"SteamAppId";
	static const size_t key_to_add_len = wcslen(key_to_add);
	static const size_t max_uint32_str_len = 16; // Overestimate

	// Format 'SteamAppId=%d' (including null terminator) to copy into our environment block
	wchar_t keyval_buf[64];
	static const size_t keyval_buf_capacity = sizeof(keyval_buf) / sizeof(keyval_buf[0]);
	assert(key_to_add_len + 1 + max_uint32_str_len + 1 <= keyval_buf_capacity);
	const size_t keyval_buf_size = static_cast<size_t>(wsprintf(keyval_buf, L"%ls=%u", key_to_add, steam_app_id)) + 1;

	// Get our own process's environment block (as an array of null-terminated strings with a null at the end)
	wchar_t* own_env_block = GetEnvironmentStringsW();
	if (!own_env_block)
	{
		printf("ERROR: Failed to get environment strings for own process\n");
		return nullptr;
	}

	// Iterate through the environment block to find its size and figure out where to insert our keyval
	size_t offset = 0;
	size_t insert_before_offset = 0;
	bool insert_offset_is_fixed = false;
	while (own_env_block[offset] != '\0')
	{
		// Read up to the next null terminator to get the current entry ('key=val')
		const wchar_t* env_entry = own_env_block + offset;
		const size_t env_entry_len = wcslen(env_entry);

		// Find the first equals sign within that entry, to get the key (i.e. env var name)
		const wchar_t* env_entry_delim = wcschr(env_entry, L'=');
		if (!env_entry_delim || env_entry_delim >= env_entry + env_entry_len)
		{
			printf("ERROR: Unexpected format in environment strings (missing '=')\n");
			FreeEnvironmentStringsW(own_env_block);
			return nullptr;
		}

		// Check the current environment variable to see if it should appear before or after our inserted value
		const size_t env_entry_key_len = env_entry_delim - env_entry;
		const size_t max_cmp_len = min(env_entry_key_len, key_to_add_len);
		if (!insert_offset_is_fixed && env_entry_key_len != 0 && is_alpha(env_entry[0]))
		{
			const int32_t cmp_result = _wcsnicmp(env_entry, key_to_add, max_cmp_len);
			if (cmp_result < 0)
			{
				insert_before_offset = offset + env_entry_len + 1;
			}
			else
			{
				insert_offset_is_fixed = true;
			}
		}

		// Increment the offset to reach the next entry
		offset += env_entry_len + 1;
	}

	// We now know the size of the existing environment block, so we can extend it to add an entry
	const size_t own_env_block_size = offset + 1;
	const size_t new_env_block_size = own_env_block_size + keyval_buf_size; // keyval buf includes null terminator
	assert(insert_offset_is_fixed);

	// Allocate a new environment block for our child process
	wchar_t* new_env_block = new wchar_t[new_env_block_size];
	if (!new_env_block)
	{
		printf("ERROR: Failed to allocate new environment block\n");
		FreeEnvironmentStringsW(own_env_block);
		return nullptr;
	}

	// Do all our copies on plain bytes rather than using wcscpy: we already have null terminators in place
	// First, copy all the values from our original environment block that appear before our new value alphabetically
	if (insert_before_offset > 0)
	{
		memcpy(new_env_block, own_env_block, insert_before_offset * sizeof(wchar_t));
	}

	// Copy our new value ('SteamAppId=<value>') after that
	wchar_t* keyval_write_ptr = new_env_block + insert_before_offset;
	memcpy(keyval_write_ptr, keyval_buf, keyval_buf_size * sizeof(wchar_t));

	// Finally, copy all remaining values that appear after it alphabetically
	const size_t num_chars_remaining = own_env_block_size - insert_before_offset;
	wchar_t* remainder_write_ptr = keyval_write_ptr + keyval_buf_size;
	assert(num_chars_remaining > 0);
	assert(new_env_block + new_env_block_size - num_chars_remaining == remainder_write_ptr);
	assert(own_env_block + insert_before_offset + num_chars_remaining == own_env_block + own_env_block_size);
	memcpy(remainder_write_ptr, own_env_block + insert_before_offset, num_chars_remaining * sizeof(wchar_t));

	// Free the original environment block now that we're done with it
	FreeEnvironmentStringsW(own_env_block);

	// Return the new buffer; the caller will have to free it with delete[]
	return new_env_block;
}

gp_binary::gp_binary(const wchar_t* in_exe_path, const wchar_t* in_window_class_name, uint32_t in_steam_app_id)
	: exe_path(in_exe_path)
	, window_class_name(in_window_class_name)
	, steam_app_id(in_steam_app_id)
{
	assert(in_exe_path && in_exe_path[0]);
	assert(in_window_class_name && in_window_class_name[0]);

	const wchar_t* delim = wcsrchr(in_exe_path, L'\\');
	if (delim)
	{
		const size_t delim_offset = delim - in_exe_path;
		exe_name = std::wstring(delim + 1);
		working_dir = std::wstring(in_exe_path, delim_offset);
	}
	else
	{
		exe_name = exe_path;
		working_dir = std::wstring(L".");
	}

	
}

bool gp_binary::find_process(gp_process& process)
{
	const uint32_t existing_pid = find_process_by_name(exe_name.c_str());
	if (existing_pid != 0)
	{
		return process.open(existing_pid, exe_name.c_str());
	}
	return false;
}

bool gp_binary::launch_process(gp_process& process)
{
	// CreateProcess needs a mutable string, so copy our exe path into a local buffer
	wchar_t args[MAX_PATH];
	if (exe_path.size() + 1 > sizeof(args) / sizeof(args[0]))
	{
		printf("ERROR: Executable path is too long\n");
		return false;
	}
	wcscpy(args, exe_path.c_str());

	// If we need to set environment variables, copy our own environment and extend it
	void* env_block = nullptr;
	if (steam_app_id != 0)
	{
		env_block = alloc_child_process_env(steam_app_id);
		if (!env_block)
		{
			printf("ERROR: Failed to prepare environment block for new process\n");
			return false;
		}
	}

	// Prepare structs that hold output data from CreateProcess
	STARTUPINFO startup_info;
	ZeroMemory(&startup_info, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);

	PROCESS_INFORMATION process_info;
	ZeroMemory(&process_info, sizeof(process_info));

	// Attempt to start the process, and if we allocated an env block for the child process, free it
	const BOOL created_process = CreateProcessW(nullptr, args, nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT, env_block, working_dir.c_str(), &startup_info, &process_info);
	delete[] env_block;
	if (!created_process)
	{
		printf("ERROR: CreateProcess failed with error 0x%X for %ls\n", GetLastError(), exe_path.c_str());
		return false;
	}

	// Wait a moment to verify that the process doesn't immediately exit
	bool ok = true;
	const DWORD ret_val = WaitForSingleObject(process_info.hProcess, 100);
	if (ret_val != WAIT_TIMEOUT)
	{
		printf("ERROR: %ls exited with code %d after launch\n", exe_name.c_str(), ret_val);
		ok = false;
	}

	// If the process started up OK, initialize a gp_process and find our EXE module
	if (ok && !process.open(process_info.dwProcessId, exe_name.c_str()))
	{
		printf("ERROR: Failed to open process %ls after launch\n", exe_name.c_str());
		TerminateProcess(process_info.hProcess, 1);
		ok = false;
	}

	// We're done with our process and thread handles: don't prevent the process from being cleaned up
	CloseHandle(process_info.hProcess);
	CloseHandle(process_info.hThread);
	return ok;
}
