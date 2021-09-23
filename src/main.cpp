#include <cstdlib>
#include <cstdio>
#include <cinttypes>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ds_process.h"
#include "ds_addresses.h"

int main(int argc, char* argv[])
{
	// Open an already-running DS1R process so we can manipulate its memory
	ds_process process;
	if (!process.open(L"DARK SOULS", L"DarkSoulsRemastered.exe"))
	{
		printf("ERROR: Failed to initialize process (is DarkSoulsRemastered.exe running?)\n");
		return 1;
	}

	// Peek memory and follow pointers to resolve addresses to relevant values
	ds_addresses addresses;
	if (!addresses.resolve(process))
	{
		printf("ERROR: Failed to resolve addresses\n");
		return 1;
	}

	// Test it out by warping the player when we first run
	float pos[3] = { 90.0f, 25.0f, 107.0f };
	float angle = 0.0f;
	process.write(addresses.chr_warp.pos, reinterpret_cast<uint8_t*>(pos), sizeof(pos));
	process.poke<float>(addresses.chr_warp.angle, angle);
	process.poke<uint32_t>(addresses.chr_warp.latch, 1);

	// Print our player position until we break with Ctrl+C
	system("cls");
	while (true)
	{
		COORD coord{ 0, 0 };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

		if (!process.read(addresses.chr_pos.pos, reinterpret_cast<uint8_t*>(pos), sizeof(pos)))
		{
			break;
		}
		angle = process.peek<float>(addresses.chr_pos.angle);

		printf(" X: %7.3f, Y: %7.3f, Z: %7.3f | Angle: %7.3f deg (%7.3f rad)      \n", pos[0], pos[1], pos[2], angle * 57.2957795f, angle);
	}
}
