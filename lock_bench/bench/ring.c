#define _GNU_SOURCE

#include "configure.h"
#include "../lock.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include <sched.h>
#include <sys/sysinfo.h>

// #define AFFINITY

#define cpu_relax() __asm__ volatile("pause")

size_t     g_n_threads;
pthread_t *g_thread;

typedef struct channel {
	uint64_t volatile empty;
	uint64_t volatile data;
	lock_t *volatile lock;
} channel_t;

channel_t *g_ring;

static inline
int channel_empty(channel_t *c)
{
	return !!c->empty;
}

static inline
int channel_write(channel_t *c, uint64_t *data)
{
	int rc = 0;
	if (!c->empty)
		return 0;
	if (lock_acquire(c->lock) < 0)
		goto handle_err;

	if (c->empty) {
		c->data = *data;
		c->empty = 0;
		rc = 1;
	}

	if (lock_release(c->lock) < 0)
		goto handle_err;
	return rc;
handle_err:
	return -1;
}

static inline
int channel_read(channel_t *c, uint64_t *data)
{
	int rc = 0;
	if (c->empty)
		return 0;
	if (lock_acquire(c->lock) < 0)
		return -1;

	if (!c->empty) {
		*data = c->data;
		c->empty = 1;
		rc = 1;
	}

	if (lock_release(c->lock) < 0)
		return -1;
	return rc;
}

int build_ring(void)
{
	if (!(g_ring = aligned_alloc(64, sizeof(*g_ring) * g_n_threads)))
		return -1;
	for (size_t i = 0; i < g_n_threads; ++i) {
		if (!(g_ring[i].lock = lock_alloc(g_n_threads)))
			return -1;
		g_ring[i].empty = 1;
	}
	return 0;
}

void test_transmitter(channel_t *in, channel_t *out)
{
	int rc;
	uint64_t data;
	while (1) {
		while (!(rc = channel_read(in, &data)))
			cpu_relax();
		if (rc < 0)
			goto handle_err;
		while (!(rc = channel_write(out, &data)))
			cpu_relax();
		if (rc < 0)
			goto handle_err;
	}
handle_err:
	fprintf(stderr, "test_transmitter failed\n");
	exit(EXIT_FAILURE);
}

void test_master(channel_t *in, channel_t *out)
{
	int rc;
	uint64_t data, sent = 0, received = 0;
	while (received < BENCH_RING_TARGET) {
		if ((rc = channel_write(out, &sent)) < 0)
			goto handle_err;
		if (rc)
			sent++;
		if ((rc = channel_read(in, &data)) < 0)
			goto handle_err;
		if (rc) {
			if (data != received) {
				fprintf(stderr, "wrong ring sequence\n");
				goto handle_err;
			}
			received++;
		}
		cpu_relax();
	}
	return;
handle_err:
	fprintf(stderr, "test_master failed\n");
	exit(EXIT_FAILURE);
}

void *test_routine(void *arg)
{
	uintptr_t thr_id = (uintptr_t) arg;
	if (thr_id == 0) {
		test_master(&g_ring[g_n_threads - 1], &g_ring[0]);
		for (size_t i = 1; i < g_n_threads; ++i)
			pthread_cancel(g_thread[i]);
	} else {
		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
		test_transmitter(&g_ring[thr_id - 1], &g_ring[thr_id]);
	}
	return NULL;
}

int cpu_set_next(cpu_set_t *set, int cur)
{
	for (int i = cur + 1; i < CPU_SETSIZE; ++i) {
		if (CPU_ISSET(i, set))
			return i;
	}
	return -1;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		exit(EXIT_FAILURE);

	g_n_threads = atoi(argv[1]);
	if (build_ring())
		goto handle_err;

	if (!(g_thread = malloc(sizeof(*g_thread) * g_n_threads)))
		goto handle_err;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
#ifdef AFFINITY
	cpu_set_t cpuset_full, cpuset_tmp;
	pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset_full);
	int curcpu = -1;
	CPU_ZERO(&cpuset_tmp);
#endif
	for (int i = 0; i < g_n_threads; ++i) {
#ifdef AFFINITY
		if ((curcpu = cpu_set_next(&cpuset_full, curcpu)) < 0)
			goto handle_err;
		CPU_SET(curcpu, &cpuset_tmp);
		pthread_attr_setaffinity_np(&attr, sizeof(cpuset_tmp),
			&cpuset_tmp);
		CPU_CLR(curcpu, &cpuset_tmp);
#endif
		if (pthread_create(&g_thread[i], &attr,
			&test_routine, (void*) (uintptr_t) i))
			goto handle_err;
	}

	pthread_attr_destroy(&attr);

	for (int i = 0; i < g_n_threads; ++i) {
		if (pthread_join(g_thread[i], NULL))
			goto handle_err;
	}

	return 0;
handle_err:
	fprintf(stderr, "something went wrong...\n");
	return EXIT_FAILURE;
}
