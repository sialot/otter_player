#include "frame_data.h"

// 创建帧池
FRAME_DATA_POOL * frame_data_pool_create(int size)
{
	FRAME_DATA_POOL *pool = (FRAME_DATA_POOL *)malloc(sizeof(FRAME_DATA_POOL));
	if (pool == NULL)
	{
		printf("frame_data_pool_create failed. \n");
		return NULL;
	}
	pool->queue = NULL;

	// 创建队列作为池元素存储
	PRIORITY_QUEUE *queue = priority_queue_create(size, 0);
	if (pool == NULL)
	{
		frame_data_pool_destroy(pool);
		return NULL;
	}
	pool->queue = queue;

	// 填充帧
	if (_fill_frame_data(pool) < 0)
	{
		printf("_fill_frame_data failed!\n");
		return NULL;
	}
	return pool;
}

// 填充帧
int _fill_frame_data(FRAME_DATA_POOL * p)
{
	for (int i = 0; i < p->queue->size; i++)
	{
		FRAME_DATA *f = (FRAME_DATA *)malloc(sizeof(FRAME_DATA));
		if (!f)
		{
			printf("frame_data_fill failed \n");
			return -1;
		}

		// 创建默认大小1k的可扩展缓存空间
		BYTE_LIST *data_buffer = byte_list_create(0);
		if (data_buffer == NULL)
		{
			printf("frame_data_fill failed \n");
			return -1;
		}
		f->data_buffer = data_buffer;

		frame_data_pool_return(p, f);
	}

	return 0;
}

// 初始化帧
void _init_frame_data(FRAME_DATA * f)
{
	byte_list_clean(f->data_buffer);
	f->av_type = 0;
	f->stream_type = 0;
	f->channels = 0;
	f->time_stamp = 0;
	f->dts = 0;
	f->pts = 0;
	f->next = NULL;
	f->prev = NULL;
	f->data = f->data_buffer->pBytes;
	f->len = f->data_buffer->used_len;
}

// 借帧
FRAME_DATA * frame_data_pool_borrow(FRAME_DATA_POOL * p)
{
	FRAME_DATA *f = priority_queue_poll_without_wait(p->queue);
	if (f == NULL)
	{
		printf("pool is empty. process failed! \n");
		return NULL;
	}

	return f;
}

// 还帧
int frame_data_pool_return(FRAME_DATA_POOL *p, FRAME_DATA * f)
{
	// 按帧数据空间从大到小排序
	priority_queue_push(p->queue, f, 104857600 - f->data_buffer->size);
	_init_frame_data(f);
	return 0;
}

// 设置帧数据
void frame_data_set(FRAME_DATA *f, FRAME_AV_TYPE av_type, unsigned stream_type, unsigned long long dts, unsigned long long pts, unsigned char * data, int len)
{
	f->av_type = av_type;
	f->stream_type = stream_type;
	f->channels = 0;
	f->time_stamp = 0;
	f->dts = dts;
	f->pts = pts;
	f->next = NULL;
	f->prev = NULL;
	byte_list_add_list(f->data_buffer, data, len);
	f->len = f->data_buffer->used_len;
	f->data = f->data_buffer->pBytes;
}

// 销毁帧
void frame_data_destory(FRAME_DATA * f)
{
	byte_list_destroy(f->data_buffer);
	free(f);
}

// 销毁帧池
int frame_data_pool_destroy(FRAME_DATA_POOL * p)
{
	if (p->queue != NULL)
	{
		priority_queue_destroy(p->queue);
	}

	free(p);

	return 0;
}

// 创建优先队列
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

// 入列
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
	else if (cur->next == NULL)// 放队尾
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
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_broadcast(&(q->msg_cond));
	return 0;
}

// 阻塞拉取
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
		q->head = NULL;
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
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_broadcast(&(q->msg_cond));
	return item;
}

// 非阻塞拉取
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
		q->head = NULL;
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

	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_broadcast(&(q->msg_cond));
	return item;
}

// 判断是否符合类型
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

// 按类型拉取
FRAME_DATA * priority_queue_poll_by_type(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type)
{
	pthread_mutex_lock(&(q->data_mutex));
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
		q->head = NULL;
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
	pthread_mutex_unlock(&(q->data_mutex));
	pthread_cond_broadcast(&(q->msg_cond));
	return item;
}

// 队列是否满
int is_priority_queue_full(PRIORITY_QUEUE * q)
{
	return (q->size == q->used);
}

// 队列是否空
int is_priority_queue_empty(PRIORITY_QUEUE * q)
{
	return (q->used == 0);
}

// 清空队列
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

// 销毁队列
int priority_queue_destroy(PRIORITY_QUEUE *queue)
{
	priority_queue_clean(queue);
	pthread_mutex_destroy(&(queue->data_mutex));
	pthread_cond_destroy(&(queue->msg_cond));
	free(queue);
	return 0;
}
