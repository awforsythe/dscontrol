#include <cstdio>
#include <cinttypes>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ds_process.h"
#include "ds_landmark.h"

#define DISPLAY_FLOAT 0

int main(int argc, char* argv[])
{
	ds_process process;
	if (!process.open(L"DARK SOULS", L"DarkSoulsRemastered.exe"))
	{
		printf("ERROR: Failed to initialize process (is DarkSoulsRemastered.exe running?)\n");
		return 1;
	}

	const size_t world_chr_base = ds_landmark(0x7C0206, "48 8B 05 xx xx xx xx 48 8B 48 68 48 85 C9 0F 84 xx xx xx xx 48 39 5E 10 0F 84 xx xx xx xx 48").resolve_offset(process);
	if (world_chr_base == 0)
	{
		printf("ERROR: Failed to resolve world_chr_base offset\n");
		return 1;
	}

	const size_t chr_data = process.jump(world_chr_base);
	if (chr_data == 0)
	{
		printf("ERROR: Failed to resolve chr_data offset from world_chr_base\n");
		return 1;
	}

	const size_t chunk_size = 24;
	const size_t num_chunks = 80;

	system("cls");

#if DISPLAY_FLOAT
	float chunk[chunk_size];
#else
	uint32_t chunk[chunk_size];
#endif
	while (true)
	{
		COORD coord;
		coord.X = 0;
		coord.Y = 0;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

#if DISPLAY_FLOAT
		printf("          0       1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      20      21      22      23\n");
#else
		printf("             0        1        2        3        4        5        6        7        8        9       10       11       12       13       14       15       16       17       18       19       20       21       22       23\n");
#endif

		size_t read_offset = world_chr_base;
		for (size_t chunk_index = 0; chunk_index < num_chunks; chunk_index++)
		{
			if (!process.read(read_offset, reinterpret_cast<uint8_t*>(chunk), sizeof(chunk)))
			{
				printf("ERROR: Read failed\n");
				return 1;
			}

#if DISPLAY_FLOAT
			printf("%02zu: %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f ", chunk_index, chunk[0], chunk[1], chunk[2], chunk[3], chunk[4], chunk[5], chunk[6], chunk[7], chunk[8], chunk[9], chunk[10], chunk[11]);
			printf("%7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f\n", chunk[12], chunk[13], chunk[14], chunk[15], chunk[16], chunk[17], chunk[18], chunk[19], chunk[20], chunk[21], chunk[22], chunk[23]);
#else
			printf("% 4x: %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x ", static_cast<uint32_t>(chunk_index * chunk_size), chunk[0], chunk[1], chunk[2], chunk[3], chunk[4], chunk[5], chunk[6], chunk[7], chunk[8], chunk[9], chunk[10], chunk[11]);
			printf("%08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n", chunk[12], chunk[13], chunk[14], chunk[15], chunk[16], chunk[17], chunk[18], chunk[19], chunk[20], chunk[21], chunk[22], chunk[23]);
#endif
			read_offset += chunk_size;
		}

		Sleep(1);
	}
}
