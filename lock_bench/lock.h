#ifndef _LOCK_H
#define _LOCK_H

#ifdef __cplusplus
extern "C" {
#endif

struct lock;
typedef struct lock lock_t;

lock_t *lock_alloc(long unsigned n_threads);
int lock_acquire(lock_t* arg);
int lock_release(lock_t* arg);
int lock_free(lock_t* arg);

#ifdef __cplusplus
}
#endif
#endif
