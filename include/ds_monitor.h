#pragma once

#include <cinttypes>

struct gp_process;

enum class ds_monitor_state
{
	main_menu,
	load_screen,
	in_game,
};

typedef void (*ds_monitor_state_change_callback)(ds_monitor_state old_state, ds_monitor_state new_state);
typedef void (*ds_monitor_update_callback)(uint32_t frame_index, double real_delta_time);

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
