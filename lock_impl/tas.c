#include "../lock.h"

#include <stdlib.h>
#include <stdint.h>

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
	while (!__atomic_compare_exchange(
			ptr, &(typeof(*ptr)){0}, &(typeof(*ptr)){1},
			1, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
		/* nothing */;
	return 0;
}

int lock_release(lock_t* lk)
{
	register typeof(&lk->state) ptr = &lk->state;
	__atomic_store(ptr, &(typeof(*ptr)){0}, __ATOMIC_RELEASE);
	return 0;
}

int lock_free(lock_t* lk)
{
	if (lk->state != 0)
		return -1;
	free(lk);
	return 0;
}
