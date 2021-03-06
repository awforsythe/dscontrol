#pragma once

#include <cinttypes>

struct gp_process;

/** Overall state of the DS1R process: in the main menu; paused for a map transition;
	or in-game, controlling a character.
*/
enum class ds_monitor_state
{
	main_menu,
	load_screen,
	in_game,
};

typedef void (*ds_monitor_state_change_callback)(ds_monitor_state old_state, ds_monitor_state new_state);
typedef void (*ds_monitor_update_callback)(uint32_t frame_index, double real_delta_time);

/** DS1R monitor: continually monitors the in-game timer, which reflects the total
	played time for the current character, constantly accumulating while in-game,
	pausing during load, and resetting to 0 at the main menu. Detects state changes and
	fires callbacks allowing the application to respond.
*/
struct ds_monitor
{
	gp_process& process;
	const uint8_t* playtime_addr;

	ds_monitor_state_change_callback state_change_callback;
	ds_monitor_update_callback update_callback;

	ds_monitor_state state;
	uint32_t last_read_playtime;
	uint64_t last_read_timestamp;
	uint64_t last_playtime_change_timestamp;

	uint32_t frame_index;

	ds_monitor(gp_process& in_process, const uint8_t* in_playtime_addr, ds_monitor_state_change_callback in_state_change_callback, ds_monitor_update_callback in_update_callback);
	void poll();
};
