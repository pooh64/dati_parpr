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

	tag_t get_tag() const { return data.arr[tag_index]; }
	T *get_ptr() const { return (T*)(ptr_mask & data.raw); }

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

	tag_ptr get_inc()
	{
		tag_ptr rv = *this;
		rv.data.arr[tag_index]++;
		return rv;
	}

	bool is_marked()
	{
		return data.raw & ((uint64_t) 1);
	}

	tag_ptr get_marked() const
	{
		tag_ptr rv;
		rv.data.raw = data.raw | ((uint64_t) 1);
		return rv;
	}

	tag_ptr get_unmarked() const
	{
		tag_ptr rv;
		rv.data.raw = data.raw & (~(uint64_t) 1);
		return rv;
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
