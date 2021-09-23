#include <cstdio>
#include <cinttypes>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ds_process.h"
#include "ds_landmark.h"

int main(int argc, char* argv[])
{
	// Open an already-running DS1R process so we can manipulate its memory
	ds_process process;
	if (!process.open(L"DARK SOULS", L"DarkSoulsRemastered.exe"))
	{
		printf("ERROR: Failed to initialize process (is DarkSoulsRemastered.exe running?)\n");
		return 1;
	}

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
		return 1;
	}
	if (!world_chr_landmark.match(buf))
	{
		printf("ERROR: Bytes at offset 0x%X do not match expected pattern\n", world_chr_landmark_offset);
		return 1;
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
		return 1;
	}

	// Within the WorldChr data is a pointer to the ChrData1 struct
	uint8_t* chr_data_1_ptr_addr = world_chr_addr + 0x68;
	uint8_t* chr_data_1_addr = process.peek<uint8_t*>(chr_data_1_ptr_addr);
	if (!chr_data_1_addr)
	{
		printf("ERROR: Failed to resolve ChrData1 address from WorldChr\n");
		return 1;
	}
	
	// ChrData1 holds a pointer to the ChrMapData struct
	uint8_t* chr_map_data_ptr_addr = chr_data_1_addr + 0x68;
	uint8_t* chr_map_data_addr = process.peek<uint8_t*>(chr_map_data_ptr_addr);
	if (!chr_map_data_addr)
	{
		printf("ERROR: Failed to resolve ChrMapData address from ChrData1\n");
		return 1;
	}

	// Writing to these members of ChrMapData will allow us to warp the player
	uint8_t* chr_warp_pos = chr_map_data_addr + 0x110;
	uint8_t* chr_warp_angle = chr_map_data_addr + 0x124;
	uint8_t* chr_warp_latch = chr_map_data_addr + 0x108;

	// ChrMapData includes a pointer to a ChrPosData struct
	uint8_t* chr_pos_data_ptr_addr = chr_map_data_addr + 0x28;
	uint8_t* chr_pos_data_addr = process.peek<uint8_t*>(chr_pos_data_ptr_addr);
	if (!chr_pos_data_addr)
	{
		printf("ERROR: Failed to resolve ChrPosData address from ChrMapData\n");
		return 1;
	}

	// Reading these values will give us real-time player position data
	uint8_t* chr_pos = chr_pos_data_addr + 0x10;
	uint8_t* chr_angle = chr_pos_data_addr + 0x4;

	// Test it out by warping the player when we first run
	float pos[3] = { 90.0f, 25.0f, 107.0f };
	float angle = 0.0f;
	process.write(chr_warp_pos, reinterpret_cast<uint8_t*>(pos), sizeof(pos));
	process.poke<float>(chr_warp_angle, angle);
	process.poke<uint32_t>(chr_warp_latch, 1);

	// Print our player position until we break with Ctrl+C
	system("cls");
	while (true)
	{
		COORD coord{ 0, 0 };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

		if (!process.read(chr_pos, reinterpret_cast<uint8_t*>(pos), sizeof(pos)))
		{
			break;
		}
		angle = process.peek<float>(chr_angle);

		printf(" X: %7.3f, Y: %7.3f, Z: %7.3f | Angle: %7.3f deg (%7.3f rad)      \n", pos[0], pos[1], pos[2], angle * 57.2957795f, angle);
	}
}
