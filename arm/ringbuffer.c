#include <stdlib.h>
#include <string.h>
#include "ringbuffer.h"
#define WRAP(x) ((x) % RINGBUFFER_SIZE)

struct ring_buffer_s *Signal_Mod_Buffer = NULL;

struct ring_buffer_s *ring_buffer_init(struct ring_buffer_s **buf) {
	*buf = malloc(sizeof(struct ring_buffer_s));
	pthread_mutex_init(&(*buf)->mutex, NULL);
	(*buf)->start = (*buf)->end = 0;
	return *buf;
}

void ring_buffer_destroy(struct ring_buffer_s **buf) {
	pthread_mutex_destroy(&(*buf)->mutex);
	if(*buf)
		free(*buf);
	*buf = NULL;
}

int ring_buffer_push(struct ring_buffer_s *buf, int Index, int Value, int ExState) {
	pthread_mutex_lock(&buf->mutex);
	if(WRAP(buf->end + 1) == buf->start) {
		pthread_mutex_unlock(&buf->mutex);
		return 0;
	}

	buf->buffer[buf->end].Index = Index;
	buf->buffer[buf->end].Value = Value;
	buf->buffer[buf->end].ExState = ExState;
	buf->end = WRAP(buf->end + 1);
	pthread_mutex_unlock(&buf->mutex);
	return 1;
}

int ring_buffer_pop(struct ring_buffer_s *buf, int *Index, int *Value, int *ExState) {
	pthread_mutex_lock(&buf->mutex);
	if(buf->end == buf->start) {
		pthread_mutex_unlock(&buf->mutex);
		return 0;
	}

	*Index = buf->buffer[buf->start].Index;
	*Value = buf->buffer[buf->start].Value;
	*ExState = buf->buffer[buf->start].ExState;
	buf->start = WRAP(buf->start + 1);
	pthread_mutex_unlock(&buf->mutex);
	return 1;
}

int ring_buffer_size(struct ring_buffer_s *buf) {
	int size = buf->end - buf->start;
	if(size < 0) {
		return size + RINGBUFFER_SIZE;
	}
	return size;
}
