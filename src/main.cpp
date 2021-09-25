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
	window.move_to(1920, 0);
	window.activate();

	// Initialize our interface for viewing/updating player position
	ds_player player(process, addresses);

	// Test it out by warping the player when we first run
	const ds_pos warp_pos(90.0f, 25.0f, 107.0f, 0.0f);
	player.set_pos(warp_pos);

	// Print our player position and other data until we break with Ctrl+C
	system("cls");
	float prev_pos_x = 0.0f;
	bool pos_x_has_changed = false;
	uint32_t prev_playtime = 0;
	uint32_t num_reads_in_prev_tick = 0;
	uint32_t num_reads_in_current_tick = 0;
	uint32_t num_reads_for_pos_x_change = 0;
	uint32_t num_nonzero_non16_deltas = 0;
	uint32_t num_nonzero_small_pos_x_change_delays = 0;
	uint32_t last_small_num_reads_for_pos_x_change = 0;
	float last_small_read_as_ratio = 0.0f;
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
				if (delta != 16)
				{
					num_nonzero_non16_deltas++;
				}

				COORD coord{ 0, 0 };
				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

				printf(" X: %7.3f, Y: %7.3f, Z: %7.3f | Angle: %7.3f deg (%7.3f rad)      \n", pos.x, pos.y, pos.z, pos.angle * 57.2957795f, pos.angle);
				printf("time: %u     \n", playtime);
				printf("delta: %u    \n", delta);
				printf("reads per tick: %u    \n", num_reads_in_current_tick);
				printf("pos_x changed in: %u  \n", num_reads_for_pos_x_change);
				printf("num small: %u         \n", num_nonzero_small_pos_x_change_delays);
				printf("last small: %u        \n", last_small_num_reads_for_pos_x_change);
				printf("as ratio: %0.5f       \n", last_small_read_as_ratio);
				printf("num deltas != 16: %u  \n", num_nonzero_non16_deltas);

				num_reads_in_prev_tick = num_reads_in_current_tick;
				num_reads_in_current_tick = 0;
				num_reads_for_pos_x_change = 0;
			}
			else
			{
				num_reads_in_current_tick++;
				if (num_reads_for_pos_x_change == 0 && pos.x != prev_pos_x)
				{
					num_reads_for_pos_x_change = num_reads_in_current_tick;
					if (num_reads_for_pos_x_change < 2000)
					{
						last_small_num_reads_for_pos_x_change = num_reads_for_pos_x_change;
						if (num_reads_in_prev_tick > 0.0f)
						{
							last_small_read_as_ratio = static_cast<float>(last_small_num_reads_for_pos_x_change) / static_cast<float>(num_reads_in_prev_tick);
						}
						num_nonzero_small_pos_x_change_delays++;
					}
				}
			}
		}
		prev_playtime = playtime;
		prev_pos_x = pos.x;
	}
}
