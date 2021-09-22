#include <cstdio>
#include <cinttypes>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ds_process.h"
#include "ds_landmark.h"

struct ds_jumplist
{
	std::vector<size_t> offsets;

	size_t resolve(size_t base, const ds_process& process) const
	{
		const size_t last_index = offsets.size() - 1;

		size_t value = process.jump(base);
		for (size_t i = 0; value != 0 && i <= last_index; i++)
		{
			const size_t next = value + offsets.at(i);
			value = i == last_index ? next : process.jump(next);
		}
		return value;
	}
};

int main(int argc, char* argv[])
{
	ds_process process;
	if (!process.open(L"DARK SOULS", L"DarkSoulsRemastered.exe"))
	{
		printf("ERROR: Failed to initialize process (is DarkSoulsRemastered.exe running?)\n");
		return 1;
	}

	const ds_landmark landmark_x(0x3204E0, "48 8B 05 xx xx xx xx 48 39 48 68 0F 94 C0 C3");
	const ds_jumplist x_to_player{ {0x68, 0x18, 0x28, 0x50, 0x20, 0x120 - 24} };
	
	const size_t offset_x = landmark_x.resolve_offset(process);
	const size_t player_offset = x_to_player.resolve(offset_x, process);

	const size_t chunk_size = 24;
	const size_t num_chunks = 80;

	system("cls");

	float chunk[chunk_size];
	while (true)
	{
		COORD coord;
		coord.X = 0;
		coord.Y = 0;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

		printf("          0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23\n");

		size_t read_offset = player_offset;
		for (size_t chunk_index = 0; chunk_index < num_chunks; chunk_index++)
		{
			if (!process.read(read_offset, reinterpret_cast<uint8_t*>(chunk), sizeof(chunk)))
			{
				return 1;
			}
			printf("%02zu: %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f ", chunk_index, chunk[0], chunk[1], chunk[2], chunk[3], chunk[4], chunk[5], chunk[6], chunk[7], chunk[8], chunk[9], chunk[10], chunk[11]);
			printf("%7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f\n", chunk[12], chunk[13], chunk[14], chunk[15], chunk[16], chunk[17], chunk[18], chunk[19], chunk[20], chunk[21], chunk[22], chunk[23]);
			read_offset += chunk_size;
		}

		Sleep(1);
	}
}
