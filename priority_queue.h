#pragma once
#include <stdlib.h>
#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#include <pthread.h>
#else
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#endif

// 播放状态
typedef enum FRAME_AV_TYPE
{
	AUDIO,
	VIDEO
}FRAME_AV_TYPE;

typedef struct FRAME_DATA
{
	int len; // 数据长度
	unsigned char * data; // 数据
	unsigned long long time_stamp; // 排序用时间戳
	unsigned long long DTS;
	unsigned long long PTS;
	unsigned stream_type; // 流类型
	FRAME_AV_TYPE av_type; // VIDEO， AUDIO
	struct FRAME_DATA *next;
	struct FRAME_DATA *prev;
} FRAME_DATA;

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

FRAME_DATA * frame_data_create(FRAME_AV_TYPE av_type, unsigned stream_type, unsigned long long DTS, unsigned long long PTS, unsigned char * data, int len);
void frame_data_destory(FRAME_DATA *f);
PRIORITY_QUEUE * priority_queue_create(int size, int preSize);
int priority_queue_push(PRIORITY_QUEUE *q, FRAME_DATA *item, unsigned long long time_stamp);
FRAME_DATA * priority_queue_poll(PRIORITY_QUEUE *q);
int _is_type_ok(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type);
FRAME_DATA * priority_queue_poll_by_type(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type);
int is_priority_queue_full(PRIORITY_QUEUE *q);
int is_priority_queue_empty(PRIORITY_QUEUE *q);
int priority_queue_clean(PRIORITY_QUEUE *q);
int priority_queue_destroy(PRIORITY_QUEUE *q);
