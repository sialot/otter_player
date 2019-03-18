#pragma once
#include <stdlib.h>
#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#include <pthread.h>
#else
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#endif

#include "byte_list.h"

typedef struct BLOCK_QUEUE
{
	int size;
	int head;
	int tail;
	pthread_mutex_t data_mutex;
	pthread_cond_t msg_cond;
	void *items[0];
} BLOCK_QUEUE;

#include "ts_demuxer.h"

BLOCK_QUEUE * block_queue_create(int size);
int block_queue_push(BLOCK_QUEUE *queue, void *item);
void * block_queue_poll(BLOCK_QUEUE *queue);
int is_block_queue_full(BLOCK_QUEUE *queue);
int is_block_queue_empty(BLOCK_QUEUE *queue);
int ts_block_queue_clean(BLOCK_QUEUE *queue);
int ts_block_queue_destroy(BLOCK_QUEUE *queue);
int pes_block_queue_clean(BLOCK_QUEUE *queue);
int pes_block_queue_destroy(BLOCK_QUEUE *queue);
