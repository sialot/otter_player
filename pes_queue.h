#pragma once
#include <stdlib.h>

typedef struct ES_QUEUE
{
	void **items;
	int size;
	int head;
	int tail;
	int full;
} ES_QUEUE;

ES_QUEUE * es_queue_create(int size);
int es_queue_add(ES_QUEUE *queue, void *item);
void * es_queue_poll(ES_QUEUE *queue);