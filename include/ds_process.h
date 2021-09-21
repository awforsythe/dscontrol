#pragma once

#include <cinttypes>

struct ds_process
{
	uint32_t pid;
	void* window_handle;
	void* process_handle;
	uint8_t* module_addr;

	ds_process();
	~ds_process();

	bool open(const wchar_t* window_class_name, const wchar_t* module_name);

	bool read(size_t offset, uint8_t* buf, size_t size) const;
	size_t jump(size_t offset) const;
	bool write(size_t offset, const uint8_t* buf, size_t size);
};
