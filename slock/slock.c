#include "slock.h"
#include <pthread.h>

size_t const slock_t_size = sizeof(pthread_mutex_t);

int slock_init(slock_t *sl)
{
	return pthread_mutex_init((void*) sl, NULL);
}

int slock_destroy(slock_t *sl)
{
	return pthread_mutex_destroy((void*) sl);
}

int slock_lock(slock_t *sl)
{
	return pthread_mutex_lock((void*) sl);
}

int slock_unlock(slock_t *sl)
{
	return pthread_mutex_unlock((void*) sl);
}
