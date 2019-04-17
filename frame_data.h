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

// 播放状态
typedef enum FRAME_AV_TYPE
{
	AUDIO,
	VIDEO
}FRAME_AV_TYPE;

// 帧数据
typedef struct FRAME_DATA
{
	int len; // 数据长度
	unsigned int cur_time : 32;
	FRAME_AV_TYPE av_type; // VIDEO， AUDIO
	int channels;
	unsigned char * data; // 数据
	unsigned int time_stamp : 32; // 排序用时间戳
	unsigned long long dts : 33;
	unsigned long long pts : 33;
	unsigned stream_type; // 流类型
	BYTE_LIST *data_buffer; // 存储帧数据的字节列表
	struct FRAME_DATA *next;
	struct FRAME_DATA *prev;
} FRAME_DATA;

// 优先队列
typedef struct PRIORITY_QUEUE
{
	FRAME_DATA *head;
	FRAME_DATA *tail;
	int size;
	int used;
	int preparing;
	int preSize;
	pthread_mutex_t data_mutex;
	pthread_cond_t msg_cond;
} PRIORITY_QUEUE;

// 帧数据池
typedef struct FRAME_DATA_POOL
{
	PRIORITY_QUEUE *queue;
} FRAME_DATA_POOL;

/* 
 * 池相关函数
 */

// 创建帧池
FRAME_DATA_POOL *frame_data_pool_create(int size);

// 填充帧
static int _fill_frame_data(FRAME_DATA_POOL *p);

// 初始化帧
static void _init_frame_data(FRAME_DATA *f);

// 借帧
FRAME_DATA * frame_data_pool_borrow(FRAME_DATA_POOL *p);

// 还帧
int frame_data_pool_return(FRAME_DATA_POOL *p, FRAME_DATA *f);

// 设置帧数据
void frame_data_set(FRAME_DATA *f, FRAME_AV_TYPE av_type, unsigned stream_type, unsigned long long dts, unsigned long long pts, unsigned char * data, int len);

// 销毁帧
void frame_data_destory(FRAME_DATA *f);

// 销毁帧池
int frame_data_pool_destroy(FRAME_DATA_POOL *p);

/*
 * 队列相关函数
 */

// 创建优先队列
PRIORITY_QUEUE * priority_queue_create(int size, int preSize);

// 入列
int priority_queue_push(PRIORITY_QUEUE *q, FRAME_DATA *item, unsigned long long time_stamp);

// 阻塞拉取
FRAME_DATA * priority_queue_poll(PRIORITY_QUEUE *q);

// 非阻塞拉取
FRAME_DATA * priority_queue_poll_without_wait(PRIORITY_QUEUE *q);

// 判断是否符合类型
int _is_type_ok(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type);

// 按类型拉取
FRAME_DATA * priority_queue_poll_by_type(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type);

// 队列是否满
int is_priority_queue_full(PRIORITY_QUEUE *q);

// 队列是否空
int is_priority_queue_empty(PRIORITY_QUEUE *q);

// 清空队列
int priority_queue_clean(PRIORITY_QUEUE *q);

// 销毁队列
int priority_queue_destroy(PRIORITY_QUEUE *q);
