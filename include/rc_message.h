#pragma once

#include <cinttypes>

enum class ds_game : uint8_t
{
	dark_souls_remastered,
	dark_souls_2_sotfs,
	dark_souls_3,
	sekiro,
	elden_ring
};
static const size_t ds_game_count = 5;

struct rc_game_state
{
	uint8_t installed : 1;
	uint8_t running : 1;
	uint8_t controlled : 1;
};

struct rc_agent_status
{
	rc_game_state game_states[rc_game_count];
};

struct rc_message
{
	size_t payload_size;
	void* payload;
};
