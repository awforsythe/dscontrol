#pragma once

#include <cinttypes>

#include "ds_pos.h"

struct gp_process;
struct ds_addresses;

struct ds_player
{
	gp_process& process;
	ds_addresses& addresses;

	ds_player(gp_process& in_process, ds_addresses& in_addresses);
	ds_pos get_pos() const;
	bool set_pos(const ds_pos& pos);
};
