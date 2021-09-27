#include <cstdlib>
#include <cstdio>
#include <cinttypes>
#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "vc_state.h"
#include "vc_device.h"

#include "si_list.h"
#include "si_script.h"
#include "si_timeline.h"
#include "si_evaluator.h"

#include "gp_window.h"
#include "gp_process.h"

#include "ds_addresses.h"
#include "ds_pos.h"
#include "ds_clock.h"
#include "ds_player.h"

int main(int argc, char* argv[])
{
	// Load up the .yml scripts that define our scripted interactions
	si_list list;
	if (!list.load(L"..\\data"))
	{
		printf("ERROR: Failed to load scripts\n");
		return 1;
	}

	// Find the script that we parsed from our test file
	const si_script* script = list.find("script");
	if (!script)
	{
		printf("ERROR: Failed to find test script\n");
		return 1;
	}

	// Load that script into a timeline for playback
	si_timeline timeline;
	timeline.load(*script);

	// Connect an emulated X360 controller to the ViGEmBus driver
	vc_device device;
	if (!device.init())
	{
		printf("ERROR: Failed to initialize virtual controller device\n");
		return 1;
	}

	// Find the DS1R window: it should already be running
	gp_window window;
	if (!window.find(L"DARK SOULS"))
	{
		printf("ERROR: Failed to find window (is DarkSoulsRemastered.exe running?)\n");
		return 1;
	}

	// Open a process handle so we can read and write memory
	gp_process process;
	if (!process.open(window.handle, L"DarkSoulsRemastered.exe"))
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

	// Move the window and give focus to the game
	system("cls");
	window.move_to(1920, 0);
	window.activate();

	Sleep(1000);

	// Initialize our interfaces for reading/manipulating the game
	ds_clock clock(process, addresses);
	ds_player player(process, addresses);

	while (true)
	{
		system("cls");

		// Warp the player to the initial position for the script, then wait
		player.set_pos(script->warp_pos);
		Sleep(static_cast<DWORD>(1000.0f * script->settle_time));

		// Center the camera, then wait another brief moment
		vc_state state;
		state.update_button(si_control::button_rs, true);
		device.update(state);
		Sleep(20);
		state.update_button(si_control::button_rs, false);
		device.update(state);
		Sleep(1000);

		// Start playback of our events
		si_evaluator evaluator(timeline);
		evaluator.start();

		bool finished = false;
		while (!finished)
		{
			const uint32_t delta = clock.read();
			if (delta > 0)
			{
				finished = evaluator.tick();
				device.update(evaluator.control_state);

				const ds_pos pos = player.get_pos();

				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0, 0 });
				printf("FRAME: %u        \n", clock.frame_count);
				printf("   dt: %7.3f ms  \n", clock.real_frame_time * 1000.0);
				printf("POS X: %7.3f     \n", pos.x);
				printf("POS Y: %7.3f     \n", pos.y);
				printf("POS Z: %7.3f     \n", pos.z);
				printf("ANGLE: %7.3f     \n", pos.angle);
				printf("(deg): %7.3f     \n", pos.angle * 57.2957795f);
			}
		}

		state.reset();
		device.update(state);

		printf("\n\nDone!\n");
		Sleep(2000);
	}
}
