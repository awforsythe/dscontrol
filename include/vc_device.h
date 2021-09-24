#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ViGEm/Client.h"

struct vc_device
{
	PVIGEM_CLIENT client;
	PVIGEM_TARGET target;

	vc_device();
	~vc_device();

	bool init();
	void update(const struct vc_state& state);
};
