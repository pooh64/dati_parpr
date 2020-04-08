#include "lock.h"
#include <pthread.h>
#include <stdlib.h>

struct lock {
	pthread_spinlock_t val;
};

lock_t *lock_alloc(long unsigned n_threads)
{
	lock_t *lk;
	if (!(lk = aligned_alloc(128, sizeof(*lk))))
		return NULL;
	pthread_spin_init(&lk->val, PTHREAD_PROCESS_PRIVATE);
	return lk;
}

int lock_free(lock_t *lk)
{
	int rc;
	if ((rc = pthread_spin_destroy(&lk->val)))
		return rc;
	free(lk);
	return 0;
}

int lock_acquire(lock_t *lk)
{
	return pthread_spin_lock(&lk->val);
}

int lock_release(lock_t *lk)
{
	return pthread_spin_unlock(&lk->val);
}
