#ifndef _SLOCK_H
#define _SLOCK_H

#include <stddef.h>

struct slock;
typedef struct slock slock_t;
extern size_t const slock_t_size;

int slock_init   (slock_t*);
int slock_destroy(slock_t*);
int slock_lock   (slock_t*);
int slock_unlock (slock_t*);

#endif
