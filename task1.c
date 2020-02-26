#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>

void *routine(void *arg)
{
	printf("%.3d %d\n", (int) (intptr_t) arg, (int) syscall(SYS_gettid));
	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	int n_threads = atoi(argv[1]);

	pthread_t *arr = malloc(sizeof(*arr) * n_threads);

	for (int i = 0; i < n_threads; ++i)
		pthread_create(&arr[i], NULL, &routine, (void*) (intptr_t) i);

	for (int i = 0; i < n_threads; ++i)
		pthread_join(arr[i], NULL);
	return 0;
}
