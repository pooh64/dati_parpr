#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>

typedef struct sync_buf {
	int size;
	pthread_mutex_t mtx;
	char data[400];
} sync_buf_t;

void sync_buf_init(sync_buf_t *buf)
{
	buf->size = 0;
	pthread_mutex_init(&buf->mtx, NULL);
}

void sync_buf_destroy(sync_buf_t *buf)
{
	pthread_mutex_destroy(&buf->mtx);
}

int sync_buf_read(sync_buf_t *buf, char *val)
{
	int rc = 1;
	pthread_mutex_lock(&buf->mtx);
	if (buf->size == 0)
		rc = 0;
	else
		*val = buf->data[--buf->size];
	pthread_mutex_unlock(&buf->mtx);
	return rc;
}

int sync_buf_write(sync_buf_t *buf, char val)
{
	int rc = 1;
	pthread_mutex_lock(&buf->mtx);
	if (buf->size == sizeof(buf->data))
		rc = 0;
	else
		buf->data[buf->size++] = val;
	pthread_mutex_unlock(&buf->mtx);
	return rc;
}

struct routine_info {
	int id;
	sync_buf_t *buf;
};

void *routine_reader(void *arg)
{
	int          id = ((struct routine_info*) arg)->id;
	sync_buf_t *buf = ((struct routine_info*) arg)->buf;

	printf("reader %d spawned\n", id);

	char path[16];
	snprintf(path, sizeof(path), "%d.txt", id + 1);
	FILE *file = fopen(path, "w");

	int count = 0;
	char val;
	while (count != 100) {
		if (sync_buf_read(buf, &val)) {
			fwrite(&val, 1, 1, file);
			count++;
		}
	}

	return NULL;
}

void *routine_writer(void *arg)
{
	int          id = ((struct routine_info*) arg)->id;
	sync_buf_t *buf = ((struct routine_info*) arg)->buf;

	printf("writer %d spawned\n", id);

	int count = 0;
	char val = '1' + id - 0;
	while (count != 100) {
		if (sync_buf_write(buf, val))
			count++;
	}

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t *thr = malloc(sizeof(*thr) * 8);
	struct routine_info *info = malloc(sizeof(*info) * 8);

	sync_buf_t *buf = malloc(sizeof(*buf));
	sync_buf_init(buf);

	for (int i = 0; i < 4; ++i) {
		info[i] = (struct routine_info) { .id = i, .buf = buf };
		pthread_create(&thr[i], NULL, &routine_writer, &info[i]);
	}

	for (int i = 4; i < 8; ++i) {
		info[i] = (struct routine_info) { .id = i - 4, .buf = buf };
		pthread_create(&thr[i], NULL, &routine_reader, &info[i]);
	}

	for (int i = 0; i < 8; ++i)
		pthread_join(thr[i], NULL);

	sync_buf_destroy(buf);
	return 0;
}
