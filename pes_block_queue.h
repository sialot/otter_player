#pragma once
#include <stdlib.h>
#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#include <pthread.h>
#else
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#endif

typedef struct PES_BLOCK_QUEUE
{
	int size;
	int head;
	int tail;
	pthread_mutex_t data_mutex;
	pthread_cond_t msg_cond;
	void *items[0];
} PES_BLOCK_QUEUE;

#include "ts_demuxer.h"

PES_BLOCK_QUEUE * pes_block_queue_create(int size);
int pes_block_queue_push(PES_BLOCK_QUEUE *queue, TS_PES_PACKET *item);
TS_PES_PACKET * pes_block_queue_poll(PES_BLOCK_QUEUE *queue);
int is_pes_block_queue_full(PES_BLOCK_QUEUE *queue);
int is_pes_block_queue_empty(PES_BLOCK_QUEUE *queue);
int pes_block_queue_clean(PES_BLOCK_QUEUE *queue);
int pes_block_queue_destroy(PES_BLOCK_QUEUE *queue);
