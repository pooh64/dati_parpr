#include "../lock.h"
#include "configure.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sched.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>

size_t volatile g_counter = 0;
pid_t volatile	g_owner = 0;
size_t          g_n_threads;
pthread_t      *g_thread;
lock_t         *g_lock;

void *test_routine(void *arg)
{
	int flag_done = 0;
	do {
		if (lock_acquire(g_lock))
			goto handle_err;
#define SILENT
#ifndef SILENT
		printf("[%ld] in section\n", (long) g_owner);
		fflush(stdout);
#endif
		assert(g_owner == 0);
		g_owner = syscall(SYS_gettid);

		if (g_counter < BENCH_IO_TARGET)
			g_counter++;
		else
			flag_done = 1;

#ifndef SILENT
		printf("[%ld] g_counter %zu\n", (long) g_owner, g_counter);
		fflush(stdout);
#endif
		struct timeval t1, t2;
		gettimeofday(&t1, NULL);
		usleep(1);
		gettimeofday(&t2, NULL);
#ifndef SILENT
		printf("[%ld] sched_time = %f\n", (long) g_owner,
			t2.tv_sec - t1.tv_sec +
			0.000001 * (t2.tv_usec - t1.tv_usec));
		printf("[%ld] leaving section\n\n", (long) g_owner);
		fflush(stdout);
#endif
		assert(g_owner == syscall(SYS_gettid));
		g_owner = 0;

		if (lock_release(g_lock))
			goto handle_err;

	} while (!flag_done);

	return NULL;
handle_err:
	fprintf(stderr, "test_routine failed\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if (argc != 2)
		goto handle_err;
	g_n_threads = atoi(argv[1]);

	if (!(g_lock = lock_alloc(g_n_threads)))
		goto handle_err;

	if (!(g_thread = malloc(sizeof(*g_thread) * g_n_threads)))
		goto handle_err;

	for (int i = 0; i < g_n_threads; ++i) {
		if (pthread_create(&g_thread[i], NULL, &test_routine, NULL))
			goto handle_err;
	}

	for (int i = 0; i < g_n_threads; ++i) {
		if (pthread_join(g_thread[i], NULL))
			goto handle_err;
	}

	assert(g_counter == BENCH_IO_TARGET);

	return 0;
handle_err:
	fprintf(stderr, "something went wrong...\n");
	return EXIT_FAILURE;
}
