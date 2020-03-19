#include "slock.h"
#include <pthread.h>

size_t const slock_t_size = sizeof(pthread_mutex_t);

inline int slock_init(slock_t *sl)
{
	return pthread_mutex_init((void*) sl, NULL);
}

inline int slock_destroy(slock_t *sl)
{
	return pthread_mutex_destroy((void*) sl);
}

inline int slock_lock(slock_t *sl)
{
	return pthread_mutex_lock((void*) sl);
}

inline int slock_unlock(slock_t *sl)
{
	return pthread_mutex_unlock((void*) sl);
}
