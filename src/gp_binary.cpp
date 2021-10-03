#include "gp_binary.h"

#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

gp_binary::gp_binary(const wchar_t* in_exe_name, const wchar_t* in_window_class_name)
	: exe_name(in_exe_name)
	, window_class_name(in_window_class_name)
{
	assert(in_exe_name && in_exe_name[0]);
	assert(in_window_class_name && in_window_class_name[0]);
}

bool gp_binary::find()
{
	if (window.find(window_class_name.c_str()))
	{
		DWORD pid = 0;
		GetWindowThreadProcessId(reinterpret_cast<HWND>(window.handle), &pid);
		return process.open(pid, exe_name.c_str());
	}
	return false;
}

bool gp_binary::launch(const wchar_t* working_dir, const uint32_t steam_app_id)
{
	assert(working_dir && working_dir[0]);

	// Build the string to our executable, which we expect to find in the working directory for the process
	wchar_t args[MAX_PATH];
	const size_t working_dir_len = wcslen(working_dir);
	const size_t args_len = working_dir_len + 1 + exe_name.size();
	if (args_len + 1 >= MAX_PATH)
	{
		printf("ERROR: Joining '%ls' and '%ls' would exceed MAX_PATH chars\n", working_dir, exe_name.c_str());
		return false;
	}
	wcscpy(args, working_dir);
	args[working_dir_len] = '\\';
	wcscpy(args + working_dir_len + 1, exe_name.c_str());

	// Prepare structs that hold output data from CreateProcess
	STARTUPINFO startup_info;
	ZeroMemory(&startup_info, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);

	PROCESS_INFORMATION process_info;
	ZeroMemory(&process_info, sizeof(process_info));

	// Start up the process
	if (!CreateProcessW(nullptr, args, nullptr, nullptr, FALSE, 0, nullptr, working_dir, &startup_info, &process_info))
	{
		printf("ERROR: Failed to launch %ls\n", args);
		return false;
	}

	// Wait a moment to verify that the process doesn't immediately exit
	const DWORD ret_val = WaitForSingleObject(process_info.hProcess, 1000);
	if (ret_val != WAIT_TIMEOUT)
	{
		printf("ERROR: %ls exited with code %d after launch\n", exe_name.c_str(), ret_val);
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);
		return false;
	}

	// If the process started up OK, initialize a gp_process and find our EXE module
	if (!process.open(process_info.dwProcessId, exe_name.c_str()))
	{
		printf("ERROR: Failed to open process %ls after launch\n", exe_name.c_str());
		TerminateProcess(process_info.hProcess, 1);
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);
		return false;
	}

	// If the gp_process is OK, find the main window belonging to that process
	// TODO: Restrict based on PID
	if (!window.find(window_class_name.c_str()))
	{
		printf("ERROR: Failed to find '%ls' window after launching %ls\n", window_class_name.c_str(), exe_name.c_str());
		TerminateProcess(process_info.hProcess, 1);
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);
		return false;
	}

	return false;
}
