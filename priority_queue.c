#include "priority_queue.h"

FRAME_DATA * frame_data_create(FRAME_AV_TYPE av_type, unsigned stream_type, unsigned int dtime, unsigned int ptime, unsigned char * data, int len)
{
	FRAME_DATA *f = (FRAME_DATA *)malloc(sizeof(FRAME_DATA));
	if (!f)
	{
		printf("frame_data_create failed \n");
		return NULL;
	}
	f->av_type = av_type;
	f->stream_type = stream_type;
	f->channels = 0;
	f->time_stamp = 0;
	f->dtime = dtime;
	f->ptime = ptime;
	f->data = data;
	f->len = len;
	f->next = NULL;
	f->prev = NULL;
	return f;
}

void frame_data_destory(FRAME_DATA * f)
{
	free(f->data);
	free(f);
}

PRIORITY_QUEUE * priority_queue_create(int size, int preSize)
{
	PRIORITY_QUEUE *q = (PRIORITY_QUEUE *)malloc(sizeof(PRIORITY_QUEUE));
	if (!q)
	{
		printf("priority_queue_create failed \n");
		return NULL;
	}

	if (0 != pthread_mutex_init(&q->data_mutex, NULL))
	{
		printf("[%s]pthread_mutex_init failed!\n", __FUNCTION__);
		free(q);
		return NULL;
	}
	if (pthread_cond_init(&q->msg_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&q->data_mutex);
		printf("[%s]pthread_cond_init failed!\n", __FUNCTION__);
		free(q);
		return NULL;
	}

	q->head = NULL;
	q->tail = NULL;
	q->size = size;
	q->used = 0;
	q->preSize = preSize;
	q->preparing = 1;
	return q;
}

int priority_queue_push(PRIORITY_QUEUE *q, FRAME_DATA *item, unsigned long long time_stamp)
{
	pthread_mutex_lock(&q->data_mutex);

	while (is_priority_queue_full(q))
	{
		if (0 != pthread_cond_wait(&q->msg_cond, &q->data_mutex))//队列满，等待消息被抛出,如果5秒内，没有消息被抛出，就返回
		{
			printf("[%s]: queue is full\n", __FUNCTION__);
			pthread_mutex_unlock(&q->data_mutex);
			return -1;
		}
	}
	item->time_stamp = time_stamp;

	FRAME_DATA *cur = q->tail;
	while (cur != NULL)
	{
		if (cur->time_stamp <= item->time_stamp)
		{
			break;
		}
		cur = cur->prev;
	}

	if (cur == NULL) // 放队首
	{
		if (q->head == NULL) // 没队首
		{
			q->head = item;
			q->tail = item;
			item->next = NULL;
			item->prev = NULL;
		}
		else  // 有队首
		{
			q->head->prev = item;
			item->next = q->head;
			item->prev = NULL;
			q->head = item;
		}
	}
	else if(cur->next == NULL)// 放队尾
	{
		cur->next = item;
		item->prev = cur;
		q->tail = item;
	}
	else // 放队中
	{
		cur->next->prev = item;
		item->next = cur->next;
		cur->next = item;
		item->prev = cur;
	}

	q->used += 1;

	if (q->used >= q->preSize)
	{
		q->preparing = 0;
	}
	//printf("push %d/%d \n", q->used, q->size);
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_broadcast(&(q->msg_cond));
	return 0;
}

FRAME_DATA * priority_queue_poll(PRIORITY_QUEUE *q)
{
	pthread_mutex_lock(&(q->data_mutex));

	while (is_priority_queue_empty(q))
	{
		if (0 != pthread_cond_wait(&(q->msg_cond), &(q->data_mutex)))//队列满，等待消息被抛出,如果5秒内，没有消息被抛出，就返回
		{
			printf("[%s]: queue is empty\n", __FUNCTION__);
			pthread_mutex_unlock(&(q->data_mutex));
			return NULL;
		}
	}

	FRAME_DATA *item = q->head;
	if (q->head->next == NULL) //最后一个
	{
		q->tail = NULL;
	}
	else
	{
		q->head = q->head->next;
		q->head->prev = NULL;
	}

	q->used--;

	if (is_priority_queue_empty(q))
	{
		q->preparing = 1;
	}
	//printf("poll %d/%d \n", q->used, q->size);
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_broadcast(&(q->msg_cond));
	return item;
}

FRAME_DATA * priority_queue_poll_without_wait(PRIORITY_QUEUE *q)
{
	pthread_mutex_lock(&(q->data_mutex));

	if (is_priority_queue_empty(q))
	{
		pthread_mutex_unlock(&(q->data_mutex));
		return NULL;
	}

	FRAME_DATA *item = q->head;
	if (q->head->next == NULL) //最后一个
	{
		q->tail = NULL;
	}
	else
	{
		q->head = q->head->next;
		q->head->prev = NULL;
	}
	q->used--;

	if (is_priority_queue_empty(q))
	{
		q->preparing = 1;
	}

	//printf("poll %d/%d \n", q->used, q->size);
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_broadcast(&(q->msg_cond));
	return item;
}

int _is_type_ok(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type)
{
	int is_ok = 0;

	if (q->head != NULL)
	{
		if (q->head->av_type == av_type)
		{
			is_ok = 1;
		}
	}

	return is_ok;
}

FRAME_DATA * priority_queue_poll_by_type(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type)
{
	pthread_mutex_lock(&(q->data_mutex));
	//printf("poll_by_type try %d/%d \n", q->used, q->size);
	while (is_priority_queue_empty(q) || q->preparing || !_is_type_ok(q, av_type))
	{
		if (0 != pthread_cond_wait(&(q->msg_cond), &(q->data_mutex)))//队列满，等待消息被抛出,如果5秒内，没有消息被抛出，就返回
		{
			printf("[%s]: queue is empty\n", __FUNCTION__);
			pthread_mutex_unlock(&(q->data_mutex));
			return NULL;
		}
	}

	FRAME_DATA *item = q->head;
	if (q->head->next == NULL) //最后一个
	{
		q->tail = NULL;
	}
	else
	{
		q->head = q->head->next;
		q->head->prev = NULL;
	}
	q->head = q->head->next;
	q->used--;

	if (is_priority_queue_empty(q))
	{
		q->preparing = 1;
	}
	//printf("poll_by_type %d/%d \n", q->used, q->size);
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_broadcast(&(q->msg_cond));
	return item;
}

int is_priority_queue_full(PRIORITY_QUEUE * q)
{
	return (q->size == q->used);
}

int is_priority_queue_empty(PRIORITY_QUEUE * q)
{
	return (q->used == 0);
}

int priority_queue_clean(PRIORITY_QUEUE * queue)
{
	pthread_mutex_lock(&queue->data_mutex);
	while (!is_priority_queue_empty(queue))
	{
		FRAME_DATA * item = (FRAME_DATA *)priority_queue_poll(queue);
		frame_data_destory(item);
	}
	pthread_mutex_unlock(&(queue->data_mutex));
	return 0;
}

int priority_queue_destroy(PRIORITY_QUEUE *queue)
{
	priority_queue_clean(queue);
	pthread_mutex_destroy(&(queue->data_mutex));
	pthread_cond_destroy(&(queue->msg_cond));
	free(queue);
	return 0;
}
