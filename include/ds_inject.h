#pragma once

#include <cinttypes>

struct gp_process;
struct ds_memmap;

namespace ds_inject
{
	bool warp_to_bonfire(gp_process& process, const ds_memmap& memmap, uint32_t bonfire_id);
};
