#include "ds_bases.h"

#include <cassert>

#include "gp_landmark.h"
#include "gp_process.h"

static const size_t LANDMARK_BUFFER_SIZE = 128;
static uint8_t s_landmark_buffer[LANDMARK_BUFFER_SIZE];

bool resolve_landmark(const gp_process& process, const gp_landmark& landmark, uint32_t offset, uint8_t*& out_addr)
{
	// We have some known, unique 'landmark' byte patterns that should exist at
	// predictable offsets within the main game executable - these are typically
	// instructions so they should be stable within any given build of the game
	if (landmark.size() > LANDMARK_BUFFER_SIZE)
	{
		printf("ERROR: Landmark byte pattern exceeds expected max size\n");
		assert(false);
		return false;
	}

	// Read the chunk of memory at the offset where we expect the landmark pattern
	// (a.k.a. "AOB") to be
	uint8_t* landmark_addr = process.to_addr(offset);
	if (!process.read(landmark_addr, s_landmark_buffer, landmark.size()))
	{
		printf("ERROR: Failed to read landmark byte pattern from process\n");
		return false;
	}

	// Check that block of memory byte-for-byte against the expected pattern,
	// allowing for wildcards: this verifies that our assumption was correct and
	// we have the right offset into the process
	if (!landmark.match(s_landmark_buffer))
	{
		printf("ERROR: Bytes at offset 0x%X do not match expected pattern for landmark\n", offset);
		return false;
	}

	// 3 bytes into the landmark is a 4-byte offset: read that value, then move
	// down from the landmark by that distance, plus 7 bytes. At *that* address
	// is a pointer to the struct this landmark ultimately points to.
	uint32_t offset_from_landmark = process.peek<uint32_t>(landmark_addr + 3);
	uint8_t* ptr_addr = landmark_addr + offset_from_landmark + 7;
	out_addr = process.peek<uint8_t*>(ptr_addr);
	if (!out_addr)
	{
		printf("ERROR: Failed to resolve address from landmark\n");
		return false;
	}
	return true;
}

bool ds_bases::resolve(const gp_process& process)
{
	// a.k.a. BaseB; leads us to elapsed play time counter
	const gp_landmark stats_landmark("48 8B 05 xx xx xx xx 45 33 ED 48 8B F1 48 85 C0");
	if (!resolve_landmark(process, stats_landmark, 0x728E50, stats))
	{
		printf("ERROR: Failed to resolve base address from stats landmark\n");
		return false;
	}

	// a.k.a. BaseCAR; leads us to target camera pitch value
	const gp_landmark camera_landmark("48 8B 05 xx xx xx xx 48 89 48 60 E8");
	if (!resolve_landmark(process, camera_landmark, 0x24E37B, camera))
	{
		printf("ERROR: Failed to resolve base address from camera landmark\n");
		return false;
	}

	// WorldChr; top-level struct that contains data regarding player characters
	// as represented in the world
	const gp_landmark world_chr_landmark("48 8B 05 xx xx xx xx 48 8B 48 68 48 85 C9 0F 84 xx xx xx xx 48 39 5E 10 0F 84 xx xx xx xx 48");
	if (!resolve_landmark(process, world_chr_landmark, 0x7C0206, world_chr))
	{
		printf("ERROR: Failed to resolve base address from WorldChar landmark\n");
		return false;
	}

	return true;
}
