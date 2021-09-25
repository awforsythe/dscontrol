#include "ds_player.h"

#include "gp_process.h"
#include "ds_addresses.h"

ds_player::ds_player(gp_process& in_process, ds_addresses& in_addresses)
	: process(in_process)
	, addresses(in_addresses)
{
}

ds_pos ds_player::get_pos() const
{
	ds_pos result;
	if (process.read(addresses.chr_pos.pos, reinterpret_cast<uint8_t*>(&result.x), sizeof(float) * 3))
	{
		result.angle = process.peek<float>(addresses.chr_pos.angle);
	}
	return result;
}

bool ds_player::set_pos(const ds_pos& pos)
{
	if (process.write(addresses.chr_warp.pos, reinterpret_cast<const uint8_t*>(&pos.x), sizeof(float) * 3))
	{
		if (process.poke<float>(addresses.chr_warp.angle, pos.angle))
		{
			return process.poke<uint32_t>(addresses.chr_warp.latch, 1);
		}
	}
	return false;
}
