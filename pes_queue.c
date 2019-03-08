#include "pes_queue.h"

ES_QUEUE * es_queue_create(int size)
{
	ES_QUEUE *newQueue = (ES_QUEUE *)malloc(sizeof(ES_QUEUE));
	if (newQueue == NULL)
	{
		return NULL;
	}

	void **newItem = (void **)malloc(sizeof(void *) * size);
	if (newItem == NULL)
	{
		free(newQueue);
		return NULL;
	}
	newQueue->items = newItem;
	newQueue->size = size;
	newQueue->head = 0;
	newQueue->tail = 0;
	newQueue->full = 0;

	return newQueue;
}

int es_queue_add(ES_QUEUE *q, void *item)
{
	int tail_next = (q->tail + 1) % (q->size + 1);
	if (tail_next == q->head)
	{
		return -1;
	}
	q->items[q->tail] = item;
	q->tail = tail_next;
	return 0;
}

void * es_queue_poll(ES_QUEUE *q)
{
	if (q->tail == q->head)
	{
		return NULL;
	}
	void *item = q->items[q->head];
	q->head = (q->head + 1) % (q->size + 1);
	return item;
}