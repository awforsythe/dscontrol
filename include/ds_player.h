#pragma once

#include <cinttypes>

#include "ds_pos.h"

struct gp_process;
struct ds_memmap;

/** Interface for reading and controlling player character data in DS1R. */
struct ds_player
{
	gp_process& process;
	ds_memmap& memmap;

	ds_player(gp_process& in_process, ds_memmap& in_memmap);
	ds_pos get_pos() const;
	bool set_pos(const ds_pos& pos);
};
