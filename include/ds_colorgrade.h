#pragma once

struct gp_process;
struct ds_memmap;

namespace ds_colorgrade
{
	void enable_blackout(gp_process& process, const ds_memmap& memmap);
	void disable_blackout(gp_process& process, const ds_memmap& memmap);
}
