#include "pes_block_queue.h"

PES_BLOCK_QUEUE * pes_block_queue_create(int size)
{
	PES_BLOCK_QUEUE *newQueue = (PES_BLOCK_QUEUE *)malloc(sizeof(PES_BLOCK_QUEUE) + sizeof(BYTE_LIST *) * size);
	if (newQueue == NULL)
	{
		return NULL;
	}

	newQueue->size = size;
	newQueue->head = 0;
	newQueue->tail = 0;

	if (0 != pthread_mutex_init(&(newQueue->data_mutex), NULL))
	{
		printf("[%s]pthread_mutex_init failed!\n", __FUNCTION__);
		free(newQueue);
		newQueue = NULL;
		return NULL;
	}
	if (pthread_cond_init(&(newQueue->msg_cond), NULL) != 0)
	{
		pthread_mutex_destroy(&(newQueue->data_mutex));
		printf("[%s]pthread_cond_init failed!\n", __FUNCTION__);
		free(newQueue);
		newQueue = NULL;
	}
	return newQueue;
}

int pes_block_queue_push(PES_BLOCK_QUEUE *q, TS_PES_PACKET *item)
{
	pthread_mutex_lock(&q->data_mutex);

	if (is_pes_block_queue_full(q))
	{
		if (0 != pthread_cond_wait(&q->msg_cond, &q->data_mutex))//队列满，等待消息被抛出,如果5秒内，没有消息被抛出，就返回
		{
			printf("[%s]: pthread_cond_wait failed \n", __FUNCTION__);
			pthread_mutex_unlock(&q->data_mutex);
			return -1;
		}

		return -1;
	}
		
	int tail_next = (q->tail + 1) % (q->size + 1);
	q->items[q->tail] = item;
	q->tail = tail_next;

	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_signal(&(q->msg_cond));
	return 0;
}

TS_PES_PACKET * pes_block_queue_poll(PES_BLOCK_QUEUE *q)
{
	pthread_mutex_lock(&(q->data_mutex));

	if (is_pes_block_queue_empty(q))
	{
		if (0 != pthread_cond_wait(&(q->msg_cond), &(q->data_mutex)))//队列满，等待消息被抛出,如果5秒内，没有消息被抛出，就返回
		{
			printf("[%s]: queue is empty\n", __FUNCTION__);
			pthread_mutex_unlock(&(q->data_mutex));
			return NULL;
		}
	}

	TS_PES_PACKET *item = q->items[q->head];
	q->head = (q->head + 1) % (q->size + 1);

	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_signal(&(q->msg_cond));
	return item;
}

int is_pes_block_queue_full(PES_BLOCK_QUEUE * q)
{
	int tail_next = (q->tail + 1) % (q->size + 1);
	if (tail_next == q->head)
	{
		return 1;
	}
	return 0;
}

int is_pes_block_queue_empty(PES_BLOCK_QUEUE * q)
{
	if (q->tail == q->head)
	{
		return 1;
	}
	return 0;
}

int pes_block_queue_clean(PES_BLOCK_QUEUE * queue)
{
	while (!is_pes_block_queue_empty(queue))
	{
		TS_PES_PACKET * item = (TS_PES_PACKET *)pes_block_queue_poll(queue);
		_free_ts_pes_pkt(item);
	}

	return 0;
}

int pes_block_queue_destroy(PES_BLOCK_QUEUE *queue)
{
	pthread_mutex_destroy(&(queue->data_mutex));
	pthread_cond_destroy(&(queue->msg_cond));
	pes_block_queue_clean(queue);
	free(queue);
	return 0;
}