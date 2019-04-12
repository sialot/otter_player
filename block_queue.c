#include "block_queue.h"

BLOCK_QUEUE * block_queue_create(int size)
{
	BLOCK_QUEUE *newQueue = (BLOCK_QUEUE *)malloc(sizeof(BLOCK_QUEUE) + sizeof(void *) * (size + 1));
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

int block_queue_push(BLOCK_QUEUE *q, void *item)
{
	pthread_mutex_lock(&q->data_mutex);

	while (is_block_queue_full(q))
	{
		if (0 != pthread_cond_wait(&q->msg_cond, &q->data_mutex))//队列满，等待消息被抛出,如果5秒内，没有消息被抛出，就返回
		{
			printf("[%s]: queue is full\n", __FUNCTION__);
			pthread_mutex_unlock(&q->data_mutex);
			return -1;
		}
	}
		
	int tail_next = (q->tail + 1) % (q->size + 1);
	q->items[q->tail] = item;
	q->tail = tail_next;
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_signal(&(q->msg_cond));
	return 0;
}

void * block_queue_poll(BLOCK_QUEUE *q)
{
	pthread_mutex_lock(&(q->data_mutex));

	while (is_block_queue_empty(q))
	{
		if (0 != pthread_cond_wait(&(q->msg_cond), &(q->data_mutex)))//队列满，等待消息被抛出,如果5秒内，没有消息被抛出，就返回
		{
			printf("[%s]: queue is empty\n", __FUNCTION__);
			pthread_mutex_unlock(&(q->data_mutex));
			return NULL;
		}
	}

	void *item = q->items[q->head];
	q->head = (q->head + 1) % (q->size + 1);
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_signal(&(q->msg_cond));
	return item;
}

int is_block_queue_full(BLOCK_QUEUE * q)
{
	int tail_next = (q->tail + 1) % (q->size + 1);
	if (tail_next == q->head)
	{
		return 1;
	}
	return 0;
}

int is_block_queue_empty(BLOCK_QUEUE * q)
{
	if (q == NULL)
	{
		return 1;
	}

	if (q->tail == q->head)
	{
		return 1;
	}
	return 0;
}

int block_queue_clean(BLOCK_QUEUE * queue)
{
	while (!is_block_queue_empty(queue))
	{
		BYTE_LIST * item = (BYTE_LIST *)block_queue_poll(queue);
		byte_list_destroy(item);
	}

	return 0;
}

int block_queue_destroy(BLOCK_QUEUE *queue)
{
	block_queue_clean(queue);
	pthread_mutex_destroy(&(queue->data_mutex));
	pthread_cond_destroy(&(queue->msg_cond));
	free(queue);
	return 0;
}