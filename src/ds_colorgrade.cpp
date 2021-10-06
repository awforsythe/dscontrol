#include "ds_colorgrade.h"

#include "gp_process.h"
#include "ds_memmap.h"

void ds_colorgrade::enable_blackout(gp_process& process, const ds_memmap& memmap)
{
	static const float RGB_BLACK[3] = { 0.0f, 0.0f, 0.0f };
	process.write(memmap.colorgrade.override_brightness_rgb, reinterpret_cast<const uint8_t*>(RGB_BLACK), sizeof(RGB_BLACK));
	process.poke<uint32_t>(memmap.colorgrade.override_flag, 1);
}

void ds_colorgrade::disable_blackout(gp_process& process, const ds_memmap& memmap)
{
	static const float RGB_WHITE[3] = { 1.0f, 1.0f, 1.0f };
	process.write(memmap.colorgrade.override_brightness_rgb, reinterpret_cast<const uint8_t*>(RGB_WHITE), sizeof(RGB_WHITE));
	process.poke<uint32_t>(memmap.colorgrade.override_flag, 0);
}
