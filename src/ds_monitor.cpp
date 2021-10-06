#include "ds_monitor.h"

#include <cassert>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "gp_process.h"

static uint64_t s_freq = 0;
static const double FRAME_DURATION = 1.0 / 60.0;
static const double SINGLE_FRAME_THRESHOLD = FRAME_DURATION * 2.5;

static uint64_t get_timestamp()
{
	LARGE_INTEGER val;
	if (QueryPerformanceCounter(&val))
	{
		return val.QuadPart;
	}
	return 0;
}

static double get_elapsed(uint64_t from, uint64_t to)
{
	if (s_freq != 0)
	{
		return static_cast<double>(to - from) / s_freq;
	}
	return 0;
}

ds_monitor::ds_monitor(gp_process& in_process, const uint8_t* in_playtime_addr, ds_monitor_state_change_callback in_state_change_callback, ds_monitor_update_callback in_update_callback)
	: process(in_process)
	, playtime_addr(in_playtime_addr)
	, state_change_callback(in_state_change_callback)
	, update_callback(in_update_callback)
	, state(ds_monitor_state::main_menu)
	, last_read_playtime(0)
	, last_read_timestamp(0)
	, last_playtime_change_timestamp(0)
	, frame_index(0)
{
	assert(playtime_addr);
	assert(state_change_callback);
	assert(update_callback);

	LARGE_INTEGER val;
	if (QueryPerformanceFrequency(&val))
	{
		s_freq = val.QuadPart;
	}
	assert(s_freq != 0);
}

void ds_monitor::poll()
{
	// Read the playtime value: it's frozen while loading, ticking up while in-game, and 0 on the menu
	const uint64_t read_time = get_timestamp();
	const uint32_t playtime = process.peek<uint32_t>(playtime_addr);

	// Otherwise, check how much real-world, wall-clock time has elapsed since the last change
	const double elapsed_since_last_playtime_change = get_elapsed(last_playtime_change_timestamp, read_time);

	// Figure out what state we're in: if playtime is zero, we're on the main menu, end of story
	bool is_update = false;
	ds_monitor_state new_state = ds_monitor_state::main_menu;
	if (playtime != 0)
	{
		// Otherwise, check how much real-world, wall-clock time has elapsed since the last frame
		const double elapsed_since_last_playtime_change = get_elapsed(last_playtime_change_timestamp, read_time);

		// Check whether our playtime value has changed since the last time we read it
		if (playtime != last_read_playtime)
		{
			// We can tell whether we're in-game or on a loading screen based on change in playtime:
			const int64_t delta_playtime = static_cast<int64_t>(playtime) - static_cast<int64_t>(last_read_playtime);
			const bool is_single_frame_increment = delta_playtime >= 15 && delta_playtime <= 17;

			// If playtime has incremented by ~16ms, compare that to the elapsed real time
			if (is_single_frame_increment)
			{
				// If it's been longer than ~16ms in the real world, we've just finished a loading
				// screen and we should reset our frame count to 0
				if (elapsed_since_last_playtime_change > SINGLE_FRAME_THRESHOLD)
				{
					frame_index = 0;
				}

				// Either way, our current state should be in-game, and we should fire an update callback
				new_state = ds_monitor_state::in_game;
				is_update = true;
			}
			else
			{
				// If the playtime has changed by more than a frame, then we've probably changed characters,
				// so we should assume we're at a loading screen until the value changes
				new_state = ds_monitor_state::load_screen;
			}
			
			// Cache our timestamp so we'll know the last time the playtime value changed
			last_playtime_change_timestamp = read_time;
		}
		else
		{
			// If it's been significantly more than a frame and our playtime has held steady, then the game
			// is probably paused, meaning we're on a loading sreen
			if (elapsed_since_last_playtime_change > SINGLE_FRAME_THRESHOLD)
			{
				new_state = ds_monitor_state::load_screen;
			}
			else
			{
				// Otherwise, stay in whatever state we're already in
				new_state = state;
			}
		}
	}

	// If our state needs to change, fire a state change callback (prior to firing update callback)
	if (new_state != state)
	{
		const ds_monitor_state old_state = state;
		state = new_state;
		state_change_callback(old_state, new_state);
	}

	// If we're in-game and we advanced a single frame, process the update
	if (is_update)
	{
		assert(state == ds_monitor_state::in_game);
		const double real_delta_time = frame_index == 0 ? FRAME_DURATION : elapsed_since_last_playtime_change;
		update_callback(frame_index, real_delta_time);

		frame_index++;
	}

	// Cache values for the next read
	last_read_playtime = playtime;
	last_read_timestamp = read_time;
}
