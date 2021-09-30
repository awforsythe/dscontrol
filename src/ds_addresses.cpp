#include "ds_addresses.h"

#include <cstdio>

#include "gp_process.h"
#include "ds_bases.h"

bool ds_addresses::resolve(const gp_process& process, const ds_bases& bases)
{
	// This address is an integer counter that steadily ticks up to represent the
	// player's total played time: useful for synchronization
	playtime = bases.stats + 0xA4;

	// Follow some pointers to get to a struct that holds camera data
	uint8_t* camera_addr_0 = process.peek<uint8_t*>(bases.camera + 0x60);
	uint8_t* camera_addr_1 = process.peek<uint8_t*>(camera_addr_0 + 0x60);
	if (!camera_addr_1)
	{
		printf("ERROR: Failed to resolve address for camera\n");
		return false;
	}

	// Writing to this address will set the target camera pitch
	camera.target_pitch = camera_addr_1 + 0x150;

	// WorldChar has a pointer to ChrData1 at 0x68 bytes in
	// ChrData1 has a pointer to ChrMapData at 0x68 bytes in
	uint8_t* chr_data_1 = process.peek<uint8_t*>(bases.world_chr + 0x68);
	uint8_t* chr_map_data = process.peek<uint8_t*>(chr_data_1 + 0x68);
	if (!chr_map_data)
	{
		printf("ERRROR: Failed to resolve ChrMapData address from WorldChr\n");
		return false;
	}

	// Writing to these members of ChrMapData will allow us to warp the player
	chr_warp.pos = chr_map_data + 0x110;
	chr_warp.angle = chr_map_data + 0x124;
	chr_warp.latch = chr_map_data + 0x108;

	// ChrMapData has a pointer to a ChrPosData struct, at 0x28 bytes in
	uint8_t* chr_pos_data = process.peek<uint8_t*>(chr_map_data + 0x28);
	if (!chr_pos_data)
	{
		printf("ERROR: Failed to resolve ChrPosData address from ChrMapData\n");
		return false;
	}

	// Reading these values will give us real-time player position data
	chr_pos.pos = chr_pos_data + 0x10;
	chr_pos.angle = chr_pos_data + 0x4;

	// This ID indicates the bonfire we last rested at
	chr_class_warp.last_bonfire = bases.chr_class_warp + 0x10 + 0xB24;



	return true;
}
