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
	bool read(const uint8_t* addr, uint8_t* buf, size_t size) const;
	bool write(uint8_t* addr, const uint8_t* buf, size_t size) const;
	
	template <typename T>
	T peek(const uint8_t* addr) const
	{
		T value;
		if (read(addr, reinterpret_cast<uint8_t*>(&value), sizeof(value)))
		{
			return value;
		}
		return T();
	}

	template <typename T>
	bool poke(uint8_t* addr, const T& value)
	{
		return write(addr, reinterpret_cast<const uint8_t*>(&value), sizeof(value));
	}

	inline uint8_t* to_addr(uint32_t offset) const
	{
		return module_addr + offset;
	}

	inline uint32_t to_offset(const uint8_t* addr) const
	{
		return static_cast<uint32_t>(addr - module_addr);
	}
};
