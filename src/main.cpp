#include <cstdlib>
#include <cstdio>
#include <cinttypes>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ds_process.h"
#include "ds_addresses.h"
#include "ds_pos.h"
#include "ds_player.h"

#include "ViGEm/Client.h"

int main(int argc, char* argv[])
{
	// Verify that ViGEm links and runs
	vigem_free(vigem_alloc());

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

	// Initialize our interface for viewing/updating player position
	ds_player player(process, addresses);

	// Test it out by warping the player when we first run
	const ds_pos warp_pos(90.0f, 25.0f, 107.0f, 0.0f);
	player.set_pos(warp_pos);

	// Print our player position until we break with Ctrl+C
	system("cls");
	while (true)
	{
		COORD coord{ 0, 0 };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

		const ds_pos pos = player.get_pos();
		printf(" X: %7.3f, Y: %7.3f, Z: %7.3f | Angle: %7.3f deg (%7.3f rad)      \n", pos.x, pos.y, pos.z, pos.angle * 57.2957795f, pos.angle);
	}
}
