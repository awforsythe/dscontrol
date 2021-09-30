#include "ds_inject.h"

#include <cstdio>
#include <cstring>

#include "gp_process.h"
#include "ds_bases.h"
#include "ds_addresses.h"

static uint8_t s_bonfire_warp_instructions[] = {
	0x48, 0xB9, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, // movabs rcx,0xfefefefefefefefe
	0xFE, 0xFE, 0xFE,
	0x48, 0x8B, 0x09,                         // mov    rcx,QWORD PTR [rcx]
	0xBA, 0x01, 0x00, 0x00, 0x00,             // mov    edx,0x1
	0x48, 0x83, 0xEC, 0x38,                   // sub    rsp,0x38
	0x49, 0xBE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, // movabs r14,0xfefefefefefefefe
	0xFE, 0xFE, 0xFE,
	0x41, 0xFF, 0xD6,                         // call   r14
	0x48, 0x83, 0xC4, 0x38,                   // add    rsp,0x38
	0xC3                                      // ret
};
const size_t BONFIRE_WARP_CHR_CLASS_OFFSET = 0x02;
const size_t BONFIRE_WARP_BONFIRE_OFFSET = 0x18;

bool ds_inject::warp_to_bonfire(gp_process& process, const ds_bases& bases, const ds_addresses& addresses, uint32_t bonfire_id)
{
	if (!process.poke<uint32_t>(addresses.chr_class_warp.last_bonfire, bonfire_id))
	{
		return false;
	}

	memcpy(s_bonfire_warp_instructions + BONFIRE_WARP_CHR_CLASS_OFFSET, &bases.chr_class, sizeof(void*));
	memcpy(s_bonfire_warp_instructions + BONFIRE_WARP_BONFIRE_OFFSET, &bases.bonfire_warp, sizeof(void*));
	return process.inject_and_run(s_bonfire_warp_instructions, sizeof(s_bonfire_warp_instructions));
}
