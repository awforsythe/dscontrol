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
