#pragma once

#include <cstdint>

#if defined (__x86_64__) || defined (_M_X64)

template <typename T>
struct tag_ptr {
	typedef uint16_t tag_t;

	union {
		uint64_t raw;
		tag_t arr[4];
	} data;

	static const int tag_index = 3;
	static const uint64_t ptr_mask = (1L << 48L) - 1;

	tag_t get_tag() { return data.arr[tag_index]; }
	T *get_ptr() { return (T*)(ptr_mask & data.raw); }

	void set(T *ptr, tag_t tag)
	{
		data.raw = (uint64_t) ptr;
		data.arr[tag_index] = tag;
	}

	void set_ptr(T *ptr)
	{
		set(ptr, data.arr[tag_index]);
	}

	void set_tag(tag_t tag)
	{
		data.arr[tag_index] = tag;
	}

	bool operator==(volatile tag_ptr const &p) const
	{ return (data.raw == p.data.raw); }

	bool operator!= (volatile tag_ptr const &p) const
	{ return !operator==(p); }

	T &operator*() const
	{
		return *get_ptr();
	}

	T *operator->() const
	{
		return get_ptr();
	}

	operator bool(void) const
	{
		return get_ptr() != 0;
	}
};

#else
#error unsupported platform
#endif
