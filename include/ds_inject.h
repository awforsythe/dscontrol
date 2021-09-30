#pragma once

#include <cinttypes>

struct gp_process;
struct ds_bases;
struct ds_addresses;

namespace ds_inject
{
	bool warp_to_bonfire(gp_process& process, const ds_bases& bases, const ds_addresses& addresses, uint32_t bonfire_id);
};
