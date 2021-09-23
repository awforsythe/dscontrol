#include "ds_addresses.h"

#include <vector>

#include "ds_process.h"
#include "ds_landmark.h"

bool ds_addresses::resolve(const ds_process& process)
{
	// We expect to find this unique pattern of bytes at the given address: it
	// contains a pointer that will lead us to world character data
	const uint32_t world_chr_landmark_offset = 0x7C0206;
	const ds_landmark world_chr_landmark("48 8B 05 xx xx xx xx 48 8B 48 68 48 85 C9 0F 84 xx xx xx xx 48 39 5E 10 0F 84 xx xx xx xx 48");

	// Check for the landmark at that offset: verify that we can read those
	// bytes and that they match the expected pattern
	std::vector<uint8_t> buf(world_chr_landmark.size());
	uint8_t* world_chr_landmark_addr = process.to_addr(0x7C0206);
	if (!process.read(world_chr_landmark_addr, buf.data(), buf.size()))
	{
		printf("ERROR: Failed to read WorldChr landmark byte pattern\n");
		return false;
	}
	if (!world_chr_landmark.match(buf))
	{
		printf("ERROR: Bytes at offset 0x%X do not match expected pattern\n", world_chr_landmark_offset);
		return false;
	}

	// 3 bytes into the landmark is a 4-byte offset: read that value, then move
	// down from the landmark by that distance, plus 7 bytes. At *that* address
	// is a pointer to the WorldChr data for the player.
	uint32_t landmark_offset = process.peek<uint32_t>(world_chr_landmark_addr + 3);
	uint8_t* world_chr_ptr_addr = world_chr_landmark_addr + landmark_offset + 7;
	uint8_t* world_chr_addr = process.peek<uint8_t*>(world_chr_ptr_addr);
	if (!world_chr_addr)
	{
		printf("ERROR: Failed to resolve WorldChr address from landmark\n");
		return false;
	}

	// Within the WorldChr data is a pointer to the ChrData1 struct
	uint8_t* chr_data_1_ptr_addr = world_chr_addr + 0x68;
	uint8_t* chr_data_1_addr = process.peek<uint8_t*>(chr_data_1_ptr_addr);
	if (!chr_data_1_addr)
	{
		printf("ERROR: Failed to resolve ChrData1 address from WorldChr\n");
		return false;
	}

	// ChrData1 holds a pointer to the ChrMapData struct
	uint8_t* chr_map_data_ptr_addr = chr_data_1_addr + 0x68;
	uint8_t* chr_map_data_addr = process.peek<uint8_t*>(chr_map_data_ptr_addr);
	if (!chr_map_data_addr)
	{
		printf("ERROR: Failed to resolve ChrMapData address from ChrData1\n");
		return false;
	}

	// Writing to these members of ChrMapData will allow us to warp the player
	chr_warp.pos = chr_map_data_addr + 0x110;
	chr_warp.angle = chr_map_data_addr + 0x124;
	chr_warp.latch = chr_map_data_addr + 0x108;

	// ChrMapData includes a pointer to a ChrPosData struct
	uint8_t* chr_pos_data_ptr_addr = chr_map_data_addr + 0x28;
	uint8_t* chr_pos_data_addr = process.peek<uint8_t*>(chr_pos_data_ptr_addr);
	if (!chr_pos_data_addr)
	{
		printf("ERROR: Failed to resolve ChrPosData address from ChrMapData\n");
		return false;
	}

	// Reading these values will give us real-time player position data
	chr_pos.pos = chr_pos_data_addr + 0x10;
	chr_pos.angle = chr_pos_data_addr + 0x4;

	return true;
}
