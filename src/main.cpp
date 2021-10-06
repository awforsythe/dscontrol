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
#include "ds_monitor.h"
#include "ds_player.h"
#include "ds_inject.h"

/** Virtual controller: an emulated X360 controller using ViGEmuBus */
static vc_device s_device;

/** Scripted input: playback system for a series of predetermined input events */
static si_list s_list;
static const si_script* s_script = nullptr;
static si_timeline s_timeline;

/** Game process: an interface for manipulating a running instance of the game */
static const wchar_t* DS1R_EXE_NAME = L"DarkSoulsRemastered.exe";
static const wchar_t* DS1R_EXE_PATH = L"Q:\\SteamLibrary\\steamapps\\common\\DARK SOULS REMASTERED\\DarkSoulsRemastered.exe";
static const wchar_t* DS1R_WINDOW_CLASS = L"DARK SOULS";
static const uint32_t DS1R_STEAM_APP_ID = 570940;
static gp_binary s_binary(DS1R_EXE_PATH, DS1R_WINDOW_CLASS, DS1R_STEAM_APP_ID);
static gp_process s_process;
static gp_window s_window;

/** Dark Souls: an interface for manipulating memory in an instance of DS1R */
static ds_memmap s_memmap;

/** Init function: encapsulates all our required startup logic */
static bool init()
{
	// Connect an emulated X360 controller to the ViGEmBus driver
	if (!s_device.init())
	{
		printf("ERROR: Failed to initialize virtual controller device\n");
		return false;
	}

	// Load up the .yml scripts that define our scripted interactions
	if (!s_list.load(L"..\\data"))
	{
		printf("ERROR: Failed to load scripts\n");
		return false;
	}

	// Find our test script and load it into the timeline
	s_script = s_list.find("script");
	if (!s_script)
	{
		printf("ERROR: Failed to find test script\n");
		return false;
	}
	s_timeline.load(*s_script);

	// Find a DS1R window if already running; otherwise try to launch the game
	bool launched_process = false;
	if (!s_binary.find_process(s_process))
	{
		if (!s_binary.launch_process(s_process))
		{
			printf("ERROR: Failed to launch %ls (no existing process found)\n", DS1R_EXE_NAME);
			return false;
		}
		launched_process = true;
	}

	// Get a handle to the game's main window: if we just launched the process, block for a few seconds and wait
	const uint32_t await_window_timeout = 1000 * (launched_process ? 10 : 0);
	if (!s_window.await(s_process.pid, DS1R_WINDOW_CLASS, await_window_timeout))
	{
		printf("ERROR: Failed to find window of class '%ls' associated with pid %u", DS1R_WINDOW_CLASS, s_process.pid);
		if (await_window_timeout > 0)
		{
			printf(" (waited %0.2f seconds)", static_cast<float>(await_window_timeout) / 1000.0f);
		}
		printf("\n");
		return false;
	}

	// Move the window to a fixed screen position and give it focus
	s_window.move_to(1920, 0);
	s_window.activate();

	// If the window has just been launched, wait another moment for process memory to be initialized
	if (launched_process)
	{
		Sleep(1000);
	}

	// Resolve the fixed, landmark memory locations that point us to relevant game data
	if (!s_memmap.init(s_process))
	{
		printf("ERROR: Failed to initialize memory map for %s process\n", launched_process ? "newly-launched" : "already-running");
		return false;
	}

	// Success! All our program state is valid and we can proceed
	return true;
}

/** State change callback: called when we transition between main menu, load screen, in-game */
static void on_state_change(ds_monitor_state old_state, ds_monitor_state new_state)
{
	printf("\nSTATE CHANGE %d -> %d\n", static_cast<int>(old_state), static_cast<int>(new_state));
}

/** Update callback: called once per frame while in-game, approx.every 16ms */
static void on_update(uint32_t frame_index, double real_delta_time)
{
	printf(".");
}

int main(int argc, char* argv[])
{
	if (!init())
	{
		return 1;
	}

	ds_monitor monitor(s_process, s_memmap.playtime_addr, &on_state_change, &on_update);
	while (true)
	{
		monitor.poll();
	}

#if 0
	// Once we have a player character loaded, we need to resolve the full set of
	// addresses required to control the game
	if (!memmap.resolve(process))
	{
		printf("ERROR: Failed to resolve memory upon initial load\n");
		return false;
	}

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
		ds_colorgrade::enable_blackout(process, memmap);
		Sleep(2000);
		ds_colorgrade::disable_blackout(process, memmap);

		// Start playback of our events
		clock.frame_count = 0;
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

		ds_colorgrade::enable_blackout(process, memmap);
		Sleep(2000);
		ds_colorgrade::disable_blackout(process, memmap);

		Sleep(10);
	}
#endif
}
