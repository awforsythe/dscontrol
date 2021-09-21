#include <cstdio>
#include <cinttypes>

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

int main()
{
	ds_process process;
	if (!process.open(L"DARK SOULS", L"DarkSoulsRemastered.exe"))
	{
		printf("ERROR: Failed to initialize process (is DarkSoulsRemastered.exe running?)\n");
		return 1;
	}

	const ds_landmark landmark_x(0x3204E0, "48 8B 05 xx xx xx xx 48 39 48 68 0F 94 C0 C3");
	const ds_jumplist x_to_player_pos{ {0x68, 0x18, 0x28, 0x50, 0x20, 0x120} };
	
	const size_t offset_x = landmark_x.resolve_offset(process);
	const size_t player_pos_offset = x_to_player_pos.resolve(offset_x, process);

	float player_pos[3];
	while (true)
	{
		if (!process.read(player_pos_offset, reinterpret_cast<uint8_t*>(&player_pos), sizeof(player_pos)))
		{
			break;
		}
		printf("%f, %f, %f\n", player_pos[0], player_pos[1], player_pos[2]);
	}
}
