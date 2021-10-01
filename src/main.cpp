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
				printf("pitch: %7.3f     \n", process.peek<float>(memmap.camera.target_pitch));
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

				const int32_t num_events_to_display = 16;
				int32_t start_index = max(0, completed_event_index - num_events_to_display / 2);
				int32_t stop_index = min(num_events - 1, start_index + num_events_to_display);
				if (stop_index - start_index < num_events_to_display)
				{
					start_index = stop_index - num_events_to_display;
				}

				for (int32_t i = start_index; i <= stop_index; i++)
				{
					const si_event& event = script->events[i];
					const bool has_completed_event = i <= completed_event_index;
					const bool event_is_recent = i == completed_event_index && evaluator.playback_time - event.time < 0.2f;
					const char* bullet = event_is_recent ? "=>" : (has_completed_event ? "=>" : "- ");
					const char* control_name = si_control_names[static_cast<uint8_t>(event.control)];

					printf("%s at %5.2f: %s", bullet, event.time, control_name);
					for (size_t pad = 12 - strlen(control_name); pad > 0; pad--)
					{
						putc(' ', stdout);
					}

					if (si_control_is_stick(event.control))
					{
						printf("% 4d deg, % 3d%% ", static_cast<int32_t>(round(event.stick.angle * 57.2957795f)), static_cast<int32_t>(event.stick.distance * 100.0f));
						if (event.duration > 0.0f)
						{
							printf("over %0.2fs         \n", event.duration);
						}
						else
						{
							printf("instant          \n");
						}
					}
					else
					{
						if (event.duration > si_event::SINGLE_FRAME_DURATION)
						{
							printf("   (hold %0.2fs)       \n", event.duration);
						}
						else
						{
							printf("                        \n");
						}
					}
				}
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
