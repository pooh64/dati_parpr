#include "slock.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>

#define N_THREADS (128L)
#define COUNTER_TARGET (N_THREADS * 1024L * 128L)

long       g_counter = 0;
pthread_t *g_thread;
slock_t   *g_lock;

void *test_routine(void *arg)
{
	long tmp;
	do {
		if (slock_lock(g_lock))
			goto handle_err;

		tmp = ++g_counter;

		if (slock_unlock(g_lock))
			goto handle_err;
	} while (tmp < COUNTER_TARGET);

	return NULL;
handle_err:
	fprintf(stderr, "test_routine failed\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if (!(g_lock = malloc(slock_t_size)))
		goto handle_err;
	if (slock_init(g_lock))
		goto handle_err;

	if (!(g_thread = malloc(sizeof(*g_thread) * N_THREADS)))
		goto handle_err;
	for (int i = 0; i < N_THREADS; ++i) {
		if (pthread_create(&g_thread[i], NULL, &test_routine, NULL))
			goto handle_err;
	}

	for (int i = 0; i < N_THREADS; ++i) {
		if (pthread_join(g_thread[i], NULL))
			goto handle_err;
	}

	return 0;
handle_err:
	fprintf(stderr, "something went wrong...\n");
	return EXIT_FAILURE;
}
