#include "ds_memmap.h"

#include <cstring>

#include "gp_process.h"
#include "gp_landmark.h"

static const gp_landmark s_stats(0x728E50, gp_landmark_type::relative, "48 8B 05 xx xx xx xx 45 33 ED 48 8B F1 48 85 C0");

static const gp_landmark s_postprocess(0x15CE80, gp_landmark_type::relative, "48 8B 05 xx xx xx xx 48 8B 48 08 48 8B 01 48 8B 40 58");
static const int32_t s_postprocess_to_colorgrade[] = { 0x738 };

static const gp_landmark s_camera(0x24E37B, gp_landmark_type::relative, "48 8B 05 xx xx xx xx 48 89 48 60 E8");
static const int32_t s_camera_to_camera_target[] = { 0x60, 0x60 };

static const gp_landmark s_world_chr(0x7C0206, gp_landmark_type::relative, "48 8B 05 xx xx xx xx 48 8B 48 68 48 85 C9 0F 84 xx xx xx xx 48 39 5E 10 0F 84 xx xx xx xx 48");
static const int32_t s_world_chr_to_chr_map_data[] = { 0x68, 0x68 };
static const int32_t s_world_chr_to_chr_pos_data[] = { 0x68, 0x68, 0x28 };

static const gp_landmark s_chr_class(0x755DB0, gp_landmark_type::relative, "48 8B 05 xx xx xx xx 48 85 C0 xx xx F3 0F 58 80 AC 00 00 00");
static const gp_landmark s_chr_class_warp(0x2C89B3, gp_landmark_type::relative, "48 8B 05 xx xx xx xx 66 0F 7F 80 xx xx xx xx 0F 28 02 66 0F 7F 80 xx xx xx xx C6 80");
static const gp_landmark s_bonfire_warp(0x485CE0, gp_landmark_type::absolute, "48 89 5C 24 08 57 48 83 EC 20 48 8B D9 8B FA 48 8B 49 08 48 85 C9 0F 84 xx xx xx xx E8 xx xx xx xx 48 8B 4B 08");

#define ds_jumplist(_arr) _arr, sizeof(_arr) / sizeof(_arr[0])

static uint8_t* jump(const gp_process& process, uint8_t* base, const int32_t* offsets, size_t num_offsets)
{
	uint8_t* addr = base;
	for (size_t offset_index = 0; offset_index < num_offsets; offset_index++)
	{
		const int32_t offset = offsets[offset_index];
		addr = process.peek<uint8_t*>(addr + offset);
	}
	return addr;
}

bool ds_memmap::init(const gp_process& process)
{
	memset(this, 0, sizeof(this));

	// a.k.a. BaseB; leads us to elapsed play time counter
	uint8_t* stats_ptr;
	if (!s_stats.resolve(process, stats_ptr))
	{
		printf("ERROR: Failed to resolve base pointer address from stats landmark\n");
		return false;
	}
	uint8_t* stats_dereferenced = process.peek<uint8_t*>(stats_ptr);
	if (!stats_dereferenced)
	{
		printf("ERROR: Failed to resolve stats from base pointer\n");
		return false;
	}

	// At this address is an integer counter that steadily ticks up to represent
	// the player's total played time: useful for synchronization. This address is
	// static, so it gives us some overall continuity between reloads. It's 0 when
	// on the main menu; when in-game it'll steadily increment, and when reloading
	// an area it'll stop incrementing.
	playtime_addr = stats_dereferenced + 0xA4;

	// a.k.a. GraphicsData; leads us to post-processing overrides
	if (!s_postprocess.resolve(process, bases.postprocess.ptr))
	{
		printf("ERROR: Failed to resolve base pointer address from postprocess landmark\n");
		return false;
	}

	// a.k.a. BaseCAR; leads us to target camera pitch value
	if (!s_camera.resolve(process, bases.camera.ptr))
	{
		printf("ERROR: Failed to resolve base pointer address from camera landmark\n");
		return false;
	}

	// WorldChr; top-level struct that contains data regarding player characters
	// as represented in the world
	if (!s_world_chr.resolve(process, bases.world_chr.ptr))
	{
		printf("ERROR: Failed to resolve base pointer address from world_chr landmark\n");
		return false;
	}

	// ChrClass; top-level struct that identifies the player
	if (!s_chr_class.resolve(process, bases.chr_class))
	{
		printf("ERROR: Failed to resolve base address from chr_class landmark\n");
		return false;
	}

	// ChrClassWarp; includes data related to our last warp/reload point (i.e. bonfire)
	if (!s_chr_class_warp.resolve(process, bases.chr_class_warp.ptr))
	{
		printf("ERROR: Failed to resolve base pointer address from chr_class_warp landmark\n");
		return false;
	}

	// BonfireWarp; records the last bonfire we spawned at
	if (!s_bonfire_warp.resolve(process, bases.bonfire_warp))
	{
		printf("ERROR: Failed to resolve base address from bonfire_warp landmark\n");
		return false;
	}

	return true;
}

bool ds_memmap::resolve(const gp_process& process)
{
	// Most of our base addresses (which we've resolved at fixed locations
	// within the process) are pointers to structs that get deallocated and moved
	// around as we load between characters/areas. We use ds_base to distinguish
	// between the 'ptr' value and the 'dereferenced' address that it points to:
	// the ptr address is fixed; but its value has to be dereferenced (by
	// following the pointer) every time we reload and re-resolve memory.
	bases.postprocess.dereferenced = process.peek<uint8_t*>(bases.postprocess.ptr);
	if (!bases.postprocess.dereferenced)
	{
		printf("ERROR: Failed to resolve postprocess from base pointer\n");
		return false;
	}

	bases.camera.dereferenced = process.peek<uint8_t*>(bases.camera.ptr);
	if (!bases.camera.dereferenced)
	{
		printf("ERROR: Failed to resolve camera from base pointer\n");
		return false;
	}

	bases.world_chr.dereferenced = process.peek<uint8_t*>(bases.world_chr.ptr);
	if (!bases.world_chr.dereferenced)
	{
		printf("ERROR: Failed to resolve world_chr from base pointer\n");
		return false;
	}

	bases.chr_class_warp.dereferenced = process.peek<uint8_t*>(bases.chr_class_warp.ptr);
	if (!bases.chr_class_warp.dereferenced)
	{
		printf("ERROR: Failed to resolve chr_class_warp from base pointer\n");
		return false;
	}

	// Follow chains of pointers to get to the structs that contain relevant game
	// data - these shift around as memory is reallocated on load etc.
	uint8_t* colorgrade_base = jump(process, bases.postprocess.dereferenced, ds_jumplist(s_postprocess_to_colorgrade));
	if (!colorgrade_base)
	{
		printf("ERROR: Failed to resolve colograde_base from postprocess\n");
		return false;
	}

	uint8_t* camera_target = jump(process, bases.camera.dereferenced, ds_jumplist(s_camera_to_camera_target));
	if (!camera_target)
	{
		printf("ERROR: Failed to resolve camera_target from camera\n");
		return false;
	}

	uint8_t* chr_map_data = jump(process, bases.world_chr.dereferenced, ds_jumplist(s_world_chr_to_chr_map_data));
	if (!chr_map_data)
	{
		printf("ERRROR: Failed to resolve chr_map_data address from world_chr\n");
		return false;
	}

	uint8_t* chr_pos_data = jump(process, bases.world_chr.dereferenced, ds_jumplist(s_world_chr_to_chr_pos_data));
	if (!chr_pos_data)
	{
		printf("ERRROR: Failed to resolve chr_pos_data address from world_chr\n");
		return false;
	}

	// Writing to these values will let us override the post-process color grading
	colorgrade.override_flag = colorgrade_base + 0x34D;
	colorgrade.override_brightness_rgb = colorgrade_base + 0x350;
	colorgrade.override_contrast_rgb = colorgrade_base + 0x360;
	colorgrade.override_saturation = colorgrade_base + 0x35C;
	colorgrade.override_hue = colorgrade_base + 0x36C;

	// Writing to this address will set the target camera pitch
	camera.target_pitch = camera_target + 0x150;

	// Writing to these members of ChrMapData will allow us to warp the player
	chr_warp.pos = chr_map_data + 0x110;
	chr_warp.angle = chr_map_data + 0x124;
	chr_warp.latch = chr_map_data + 0x108;

	// Reading these values will give us real-time player position data
	chr_pos.pos = chr_pos_data + 0x10;
	chr_pos.angle = chr_pos_data + 0x4;

	// This ID indicates the bonfire we last rested at
	chr_class_warp.last_bonfire = bases.chr_class_warp.dereferenced + 0x10 + 0xB24;

	return true;
}
