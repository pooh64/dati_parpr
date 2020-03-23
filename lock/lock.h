#ifndef _LOCK_H
#define _LOCK_H

struct lock;
typedef struct lock lock_t;

lock_t *lock_alloc(long unsigned n_threads);
int lock_acquire(lock_t* arg);
int lock_release(lock_t* arg);
int lock_free(lock_t* arg);

#endif
