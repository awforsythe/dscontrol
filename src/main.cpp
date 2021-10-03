#include <cstdlib>
#include <cstdio>
#include <cinttypes>
#include <cassert>
#include <set>

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
#include "gp_binary.h"

#include "ds_memmap.h"
#include "ds_pos.h"
#include "ds_clock.h"
#include "ds_player.h"
#include "ds_inject.h"

void enable_blackout(gp_process& process, const ds_memmap& memmap)
{
	static const float RGB_BLACK[3] = { 0.0f, 0.0f, 0.0f };
	process.write(memmap.colorgrade.override_brightness_rgb, reinterpret_cast<const uint8_t*>(RGB_BLACK), sizeof(RGB_BLACK));
	process.poke<uint32_t>(memmap.colorgrade.override_flag, 1);
}

void disable_blackout(gp_process& process, const ds_memmap& memmap)
{
	static const float RGB_WHITE[3] = { 1.0f, 1.0f, 1.0f };
	process.write(memmap.colorgrade.override_brightness_rgb, reinterpret_cast<const uint8_t*>(RGB_WHITE), sizeof(RGB_WHITE));
	process.poke<uint32_t>(memmap.colorgrade.override_flag, 0);
}

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

	// Find a DS1R window if already running; otherwise try to launch the game
	const wchar_t* DS1R_WINDOW_CLASS = L"DARK SOULS";
	const wchar_t* DS1R_EXE_NAME = L"DarkSoulsRemastered.exe";
	const wchar_t* DS1R_WORKING_DIR = L"Q:\\SteamLibrary\\steamapps\\common\\DARK SOULS REMASTERED";
	const uint32_t DS1R_STEAM_APP_ID = 570940;
	gp_binary binary(DS1R_EXE_NAME, DS1R_WINDOW_CLASS);
	if (!binary.find())
	{
		if (!binary.launch(DS1R_WORKING_DIR, DS1R_STEAM_APP_ID))
		{
			printf("ERROR: Failed to launch %ls (no existing process found)\n", DS1R_EXE_NAME);
			return 1;
		}
	}
	gp_window& window = binary.window;
	gp_process& process = binary.process;

	// Peek memory and follow pointers to resolve addresses to relevant values
	ds_memmap memmap;
	if (!memmap.init(process) || !memmap.resolve(process))
	{
		printf("ERROR: Failed to initialize memory map for running process\n");
		return 1;
	}

	// Move the window and give focus to the game
	system("cls");
	window.move_to(1920, 0);
	window.activate();

#if 0
	const ds_pos pos = player.get_pos();

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0, 0 });
	printf("FRAME: %u        \n", clock.frame_count);
	printf("   dt: %7.3f ms  \n", clock.real_frame_time * 1000.0);
	printf("POS X: %7.3f     \n", pos.x);
	printf("POS Y: %7.3f     \n", pos.y);
	printf("POS Z: %7.3f     \n", pos.z);
	printf("ANGLE: %7.3f     \n", pos.angle);
	printf("(deg): %7.3f     \n", pos.angle * 57.2957795f);
#else
	while (true)
	{
		// Warp to the desired bonfire to reset and reload the area
		const uint32_t bonfire_id = 1510980;
		printf("Warping to bonfire %u...\n", bonfire_id);
		ds_inject::warp_to_bonfire(process, memmap, bonfire_id);
		Sleep(5000);

		// Memory will have changed; re-resolve the required addresses
		if (!memmap.resolve(process))
		{
			printf("ERROR: Failed to re-resolve memory map after warp\n");
			return 1;
		}

		// Warp the player to the initial position for the script, then wait
		ds_player player(process, memmap);
		printf("Warping to start position and waiting %0.2f seconds...\n", script->settle_time);
		player.set_pos(script->warp_pos);
		Sleep(static_cast<DWORD>(1000.0f * script->settle_time));

		// Center the camera, then wait another brief moment
		printf("Centering camera...\n");
		vc_state state;
		process.poke<float>(memmap.camera.target_pitch, 0.0f);
		state.update_button(si_control::button_rs, true);
		device.update(state);
		Sleep(20);
		state.update_button(si_control::button_rs, false);
		device.update(state);
		Sleep(1000);

		// Black out the screen for a second before starting
		printf("Blacking out the screen before starting playback...\n");
		enable_blackout(process, memmap);
		Sleep(2000);
		disable_blackout(process, memmap);

		// Start playback of our events
		ds_clock clock(process, memmap);
		si_evaluator evaluator(timeline);
		evaluator.start();

		system("cls");

		std::set<ds_pos> pos_values_seen_this_frame;
		
		static const size_t MAX_POS_COUNT = 8;
		int32_t pos_value_counts[MAX_POS_COUNT];
		memset(pos_value_counts, 0, sizeof(pos_value_counts));

		bool finished = false;
		while (!finished)
		{
			const uint32_t delta = clock.read();
			if (delta > 0)
			{
				const size_t num_pos_values_last_frame = pos_values_seen_this_frame.size();
				if (num_pos_values_last_frame > 0)
				{
					pos_value_counts[min(num_pos_values_last_frame, MAX_POS_COUNT - 1)]++;
				}
				pos_values_seen_this_frame.clear();

				finished = evaluator.tick();
				device.update(evaluator.control_state);

				const ds_pos pos = player.get_pos();
				pos_values_seen_this_frame.insert(pos);

				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0, 0 });
				printf("FRAME: %u        \n", clock.frame_count);
				printf("   dt: %7.3f ms  \n", clock.real_frame_time * 1000.0);
				printf("POS X: %7.3f     \n", pos.x);
				printf("POS Y: %7.3f     \n", pos.y);
				printf("POS Z: %7.3f     \n", pos.z);
				printf("ANGLE: %7.3f     \n", pos.angle);
				printf("(deg): %7.3f     \n", pos.angle * 57.2957795f);
				printf("pitch: %7.3f     \n", process.peek<float>(memmap.camera.target_pitch));
				printf("\n");

				printf("DISCRETE POSITION VALUES COUNTED PER FRAME:\n");
				for (size_t pos_count = 1; pos_count < MAX_POS_COUNT; pos_count++)
				{
					const char* suffix = pos_count == MAX_POS_COUNT - 1 ? "+" : " ";
					printf(" %zu%s: %d\n", pos_count, suffix, pos_value_counts[pos_count]);
				}
				printf("\n");

				printf("T+%5.2f     \n", evaluator.playback_time);

				const int32_t num_events = static_cast<int32_t>(script->events.size());
				int32_t completed_event_index = -1;
				for (int32_t i = 0; i < num_events; i++)
				{
					if (evaluator.playback_time > script->events[i].time)
					{
						completed_event_index = i;
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				const ds_pos pos = player.get_pos();
				pos_values_seen_this_frame.insert(pos);
			}
		}

		state.reset();
		device.update(state);

		printf("Done!\n");

		enable_blackout(process, memmap);
		Sleep(2000);
		disable_blackout(process, memmap);

		Sleep(10);
	}
#endif
}
