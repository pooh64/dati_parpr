#include "lock.h"

#include <stdlib.h>
#include <stdint.h>

#include <sched.h>

struct lock {
	union {
		uint32_t volatile owner;
		uint8_t __pad1[128];
	};
	union {
		uint32_t volatile ticket;
		uint8_t __pad2[128];
	};
};

lock_t *lock_alloc(long unsigned n_threads)
{
	lock_t *lk;
	if (!(lk = aligned_alloc(sizeof(*lk), sizeof(*lk))))
		return NULL;
	lk->owner  = 0;
	lk->ticket = 0;
	return lk;
}

int lock_acquire(lock_t* lk)
{
	typeof(lk->owner) id;
	id = __atomic_fetch_add(&lk->ticket, 1, __ATOMIC_RELAXED);
#define MAX_SPINS 16
	int i = 0;
	while (__atomic_load_n(&lk->owner, __ATOMIC_ACQUIRE) != id) {
		__asm volatile("pause");
#if 1
		if (i++ == MAX_SPINS) {
			i = 0;
			sched_yield();
		}
#endif
	}
	return 0;
}

int lock_release(lock_t* lk)
{
	//__atomic_fetch_add(&lk->owner, 1, __ATOMIC_RELEASE);
	typeof(lk->owner) tmp = __atomic_load_n(&lk->owner, __ATOMIC_RELAXED);
	__atomic_store_n(&lk->owner, tmp + 1, __ATOMIC_RELEASE);
	return 0;
}

int lock_free(lock_t* lk)
{
	free(lk);
	return 0;
}
