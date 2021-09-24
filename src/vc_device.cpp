#include "vc_device.h"

#include <cstdio>
#include <cassert>

#include "vc_state.h"

vc_device::vc_device()
	: client(nullptr)
	, target(nullptr)
{
}

vc_device::~vc_device()
{
	if (target)
	{
		if (client && vigem_target_is_attached(target))
		{
			vigem_target_remove(client, target);
		}
		vigem_target_free(target);
	}

	if (client)
	{
		vigem_free(client);
	}
}

bool vc_device::init()
{
	assert(!client);

	client = vigem_alloc();
	if (!client)
	{
		printf("ERROR: vigem_alloc failed\n");
		return false;
	}

	const VIGEM_ERROR connect_result = vigem_connect(client);
	if (!VIGEM_SUCCESS(connect_result))
	{
		if (connect_result == VIGEM_ERROR_BUS_NOT_FOUND)
		{
			printf("ERROR: ViGEmBus driver not found! Download it here:\n");
			printf("  https://github.com/ViGEm/ViGEmBus/releases/\n");
		}
		else
		{
			printf("ERROR: Failed to connect to ViGEmBus driver; error 0x%X\n", connect_result);
		}
		return false;
	}

	target = vigem_target_x360_alloc();
	if (!target)
	{
		printf("ERROR: vigem_target_x360_alloc failed\n");
		return false;
	}

	const VIGEM_ERROR add_result = vigem_target_add(client, target);
	if (!VIGEM_SUCCESS(add_result))
	{
		printf("ERROR: Failed to add target to ViGEmBus; error 0x%X\n", add_result);
		return false;
	}

	return true;
}

void vc_device::update(const vc_state& state)
{
	if (target)
	{
		vigem_target_x360_update(client, target, state.report);
	}
}
