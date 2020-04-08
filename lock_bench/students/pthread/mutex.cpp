extern "C" {
#include "lock.h"
#include <pthread.h>
#include <stdlib.h>
};

struct lock {
	pthread_mutex_t val;
};

lock_t *lock_alloc(long unsigned n_threads)
{
	lock_t *lk;
	if (!(lk = (lock_t*) aligned_alloc(128, sizeof(*lk))))
		return NULL;
	pthread_mutex_init(&lk->val,
		(const pthread_mutexattr_t*) PTHREAD_PROCESS_PRIVATE);
	return lk;
}

int lock_free(lock_t *lk)
{
	int rc;
	if ((rc = pthread_mutex_destroy(&lk->val)))
		return rc;
	free(lk);
	return 0;
}

int lock_acquire(lock_t *lk)
{
	return pthread_mutex_lock(&lk->val);
}

int lock_release(lock_t *lk)
{
	return pthread_mutex_unlock(&lk->val);
}
