#pragma once

#include <cinttypes>

#include "ds_game.h"
#include "sl_library.h"

enum class ds_game_status : uint8_t
{
	unavailable,
	installed,
	running,
	controlled
};

struct ds_game_state
{
	ds_game_status status;
};

struct ds_launcher
{
	sl_library library;

	bool has_desired_game;
	ds_game desired_game;
};
