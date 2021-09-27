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
	vc_state state;
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

	// Initialize our interfaces for reading/manipulating the game
	ds_clock clock(process, addresses);
	ds_player player(process, addresses);
	
	bool dropped_to_death = false;
	while (true)
	{
		const uint32_t delta = clock.read();
		if (delta > 0)
		{
			const ds_pos pos = player.get_pos();

			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0, 0 });
			printf("FRAME: %u        \n", clock.frame_count);
			printf("   dt: %7.3f ms  \n", clock.real_frame_time * 1000.0);
			printf("POS X: %7.3f     \n", pos.x);
			printf("POS Y: %7.3f     \n", pos.y);
			printf("POS Z: %7.3f     \n", pos.z);
			printf("ANGLE: %7.3f     \n", pos.angle);
			printf("(deg): %7.3f     \n", pos.angle * 57.2957795f);
			printf(" fell: %s        \n", dropped_to_death ? "yes" : "no");

			if (clock.frame_count == 8)
			{
				dropped_to_death = false;
			}

			if (clock.frame_count == 24)
			{
				const ds_pos warp_pos(175.0f, 200.0f, 250.0f, 0.0f);
				player.set_pos(warp_pos);
			}

			if (clock.frame_count > 100)
			{
				if (clock.frame_count < 600)
				{
					const float forward_input = min(1.0f, static_cast<float>(clock.frame_count - 100) / 200.0f);
					state.set_left_stick(0.0f, forward_input);
					device.update(state);
					printf("input: forward %5.3f  \n", forward_input);
				}
				else if (clock.frame_count < 800)
				{
					state.reset();
					device.update(state);
					printf("input: OFF                \n");
				}
				else if (clock.frame_count >= 800 && !dropped_to_death)
				{
					const ds_pos warp_pos(182.0f, 250.0f, 250.0f, 0.0f);
					player.set_pos(warp_pos);
					dropped_to_death = true;
				}
			}

			if (clock.frame_count > 1600)
			{
				clock.reset();
				addresses.resolve(process);
			}
		}
	}
}
