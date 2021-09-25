#include <cstdlib>
#include <cstdio>
#include <cinttypes>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "vc_state.h"
#include "vc_device.h"

#include "ds_window.h"
#include "ds_process.h"
#include "ds_addresses.h"
#include "ds_pos.h"
#include "ds_clock.h"
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

	// Find the DS1R window: it should already be running
	ds_window window;
	if (!window.find(L"DARK SOULS"))
	{
		printf("ERROR: Failed to find window (is DarkSoulsRemastered.exe running?)\n");
		return false;
	}

	// Open a process handle so we can read and write memory
	ds_process process;
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

	while (true)
	{
		const uint32_t delta = clock.read();
		if (delta > 0)
		{
			const ds_pos pos = player.get_pos();

			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0, 0 });
			printf("FRAME: %u             \n", clock.frame_count);
			printf("POS X: %7.3f          \n", pos.x);
			printf("POS Y: %7.3f          \n", pos.y);
			printf("POS Z: %7.3f          \n", pos.z);
			printf("ANGLE: %7.3f          \n", pos.angle);
			printf("(deg): %7.3f          \n", pos.angle * 57.2957795f);

			if (clock.frame_count == 200)
			{
				const ds_pos warp_pos(82.6f, 22.5f, 107.4f, 1.56f);
				player.set_pos(warp_pos);
			}

			if (clock.frame_count > 400)
			{
				if (clock.frame_count < 1000)
				{
					const float forward_input = min(1.0f, static_cast<float>(clock.frame_count - 400) / 200.0f);
					state.set_left_stick(0.0f, forward_input);
					device.update(state);
					printf("input: forward %5.3f  \n", forward_input);
				}
				else
				{
					state.reset();
					device.update(state);
					printf("input: OFF                \n");

					if (clock.frame_count > 1200)
					{
						clock.reset();
					}
				}
			}
		}
	}
}
