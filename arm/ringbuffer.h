#pragma once
#include "signals.h"
#include <pthread.h>

#define RINGBUFFER_SIZE	120

struct ring_buffer_item_s {
	int Index, Value, ExState;
};

struct ring_buffer_s {
	pthread_mutex_t mutex;
	int start, end, size;
	struct ring_buffer_item_s buffer[RINGBUFFER_SIZE];
};

struct ring_buffer_s *Signal_Mod_Buffer;

struct ring_buffer_s *ring_buffer_init(struct ring_buffer_s **buf);
void ring_buffer_destroy(struct ring_buffer_s **buf);
int ring_buffer_push(struct ring_buffer_s *buf, int Index, int Value, int ExState);
int ring_buffer_pop(struct ring_buffer_s *buf, int *Index, int *Value, int *ExState);
int ring_buffer_size(struct ring_buffer_s *buf);
