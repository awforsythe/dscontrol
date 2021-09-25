#include <cstdlib>
#include <cstdio>
#include <cinttypes>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "vc_state.h"
#include "vc_device.h"

#include "ds_process.h"
#include "ds_addresses.h"
#include "ds_pos.h"
#include "ds_player.h"

int main(int argc, char* argv[])
{
	// Connect an emulated X360 controller to the ViGEmBus driver
	vc_state state;
	vc_device device;
	if (!device.init())
	{
		printf("ERROR: Failed to initialize virtual controller device\n");
		return 1;
	}

	// Open an already-running DS1R process so we can manipulate its memory
	ds_process process;
	if (!process.open(L"DARK SOULS", L"DarkSoulsRemastered.exe"))
	{
		printf("ERROR: Failed to initialize process (is DarkSoulsRemastered.exe running?)\n");
		return 1;
	}

	// Peek memory and follow pointers to resolve addresses to relevant values
	ds_addresses addresses;
	if (!addresses.resolve(process))
	{
		printf("ERROR: Failed to resolve addresses\n");
		return 1;
	}

	// Initialize our interface for viewing/updating player position
	ds_player player(process, addresses);

	// Test it out by warping the player when we first run
	const ds_pos warp_pos(90.0f, 25.0f, 107.0f, 0.0f);
	player.set_pos(warp_pos);

	RECT rect;
	GetWindowRect((HWND)process.window_handle, &rect);
	printf("x: %d\n", rect.left);
	printf("y: %d\n", rect.top);
	printf("w: %d\n", rect.right - rect.left);
	printf("h: %d\n", rect.bottom - rect.top);
	MoveWindow((HWND)process.window_handle, 1912, -24, 1936, 1119, TRUE);
	system("pause");
	SetForegroundWindow((HWND)process.window_handle);

	SendMessage((HWND)process.window_handle, WM_NCACTIVATE, FALSE, 0);
	SendMessage((HWND)process.window_handle, WM_ACTIVATE, FALSE, 0);
	SendMessage((HWND)process.window_handle, WM_ACTIVATEAPP, FALSE, 0);
	SendMessage((HWND)process.window_handle, WM_KILLFOCUS, FALSE, 0);
	SendMessage((HWND)process.window_handle, WM_IME_SETCONTEXT, FALSE, ISC_SHOWUIALL);
	SendMessage((HWND)process.window_handle, WM_IME_NOTIFY, IMN_CLOSESTATUSWINDOW, 0);

	WINDOWPOS window_pos;
	ZeroMemory(&window_pos, sizeof(window_pos));
	window_pos.flags = SWP_NOSIZE | SWP_NOMOVE;
	SendMessage((HWND)process.window_handle, WM_WINDOWPOSCHANGING, 0, (LPARAM)&window_pos);

	Sleep(100);

	SendMessage((HWND)process.window_handle, WM_ACTIVATEAPP, TRUE, 0x548); // Thread ID of window being deactivated
	SendMessage((HWND)process.window_handle, WM_NCACTIVATE, TRUE, 0);
	SendMessage((HWND)process.window_handle, WM_ACTIVATE, TRUE, 0);
	SendMessage((HWND)process.window_handle, WM_IME_SETCONTEXT, TRUE, ISC_SHOWUIALL);
	SendMessage((HWND)process.window_handle, WM_IME_NOTIFY, IMN_OPENSTATUSWINDOW, 0);
	SendMessage((HWND)process.window_handle, WM_SETFOCUS, 0, 0);

	// Print our player position and other data until we break with Ctrl+C
	system("cls");
	uint32_t prev_playtime = 0;
	while (true)
	{
		state.set_left_stick(0.0f, 1.0f);
		device.update(state);

		const uint32_t playtime = process.peek<uint32_t>(addresses.playtime);
		const ds_pos pos = player.get_pos();

		if (prev_playtime != 0)
		{
			const uint32_t delta = playtime - prev_playtime;
			if (delta > 0)
			{
				COORD coord{ 0, 0 };
				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

				printf(" X: %7.3f, Y: %7.3f, Z: %7.3f | Angle: %7.3f deg (%7.3f rad)      \n", pos.x, pos.y, pos.z, pos.angle * 57.2957795f, pos.angle);
				printf("time: %u     \n", playtime);
				printf("delta: %u    \n", delta);
			}
		}
		prev_playtime = playtime;
	}
}
