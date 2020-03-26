#include "../lock.h"

#include <stdlib.h>
#include <stdint.h>

/* #include <immintrin.h> */

struct lock {
	union {
		uint8_t volatile state;
		uint8_t __pad[128];
	};
};

lock_t *lock_alloc(long unsigned n_threads)
{
	lock_t *lk;
	if (!(lk = aligned_alloc(sizeof(*lk), sizeof(*lk))))
		return NULL;
	lk->state = 0;
	return lk;
}

int lock_acquire(lock_t* lk)
{
	register typeof(&lk->state) ptr = &lk->state;
#if 0
	if (__builtin_expect((__atomic_compare_exchange(
		ptr, &(typeof(*ptr)){0}, &(typeof(*ptr)){1},
		1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)), 1))
		return 0;
#else
	if (__builtin_expect((__atomic_exchange_n(ptr, 1, __ATOMIC_ACQUIRE)),
		0) == 0)
		return 0;
#endif
	do {
		do {
			__asm volatile("pause");
		} while (__atomic_load_n(ptr, __ATOMIC_RELAXED) != 0);
	} while (!__atomic_compare_exchange(
			ptr, &(typeof(*ptr)){0}, &(typeof(*ptr)){1},
			1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE));
	return 0;
}

int lock_release(lock_t* lk)
{
	register typeof(&lk->state) ptr = &lk->state;
	__atomic_store_n(ptr, 0, __ATOMIC_RELEASE);
	return 0;
}

int lock_free(lock_t* lk)
{
	if (lk->state != 0)
		return -1;
	free(lk);
	return 0;
}
