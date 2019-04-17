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

// ����״̬
typedef enum FRAME_AV_TYPE
{
	AUDIO,
	VIDEO
}FRAME_AV_TYPE;

// ֡����
typedef struct FRAME_DATA
{
	int len; // ���ݳ���
	unsigned int cur_time : 32;
	FRAME_AV_TYPE av_type; // VIDEO�� AUDIO
	int channels;
	unsigned char * data; // ����
	unsigned int time_stamp : 32; // ������ʱ���
	unsigned long long dts : 33;
	unsigned long long pts : 33;
	unsigned stream_type; // ������
	BYTE_LIST *data_buffer; // �洢֡���ݵ��ֽ��б�
	struct FRAME_DATA *next;
	struct FRAME_DATA *prev;
} FRAME_DATA;

// ���ȶ���
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

// ֡���ݳ�
typedef struct FRAME_DATA_POOL
{
	PRIORITY_QUEUE *queue;
} FRAME_DATA_POOL;

/* 
 * ����غ���
 */

// ����֡��
FRAME_DATA_POOL *frame_data_pool_create(int size);

// ���֡
static int _fill_frame_data(FRAME_DATA_POOL *p);

// ��ʼ��֡
static void _init_frame_data(FRAME_DATA *f);

// ��֡
FRAME_DATA * frame_data_pool_borrow(FRAME_DATA_POOL *p);

// ��֡
int frame_data_pool_return(FRAME_DATA_POOL *p, FRAME_DATA *f);

// ����֡����
void frame_data_set(FRAME_DATA *f, FRAME_AV_TYPE av_type, unsigned stream_type, unsigned long long dts, unsigned long long pts, unsigned char * data, int len);

// ����֡
void frame_data_destory(FRAME_DATA *f);

// ����֡��
int frame_data_pool_destroy(FRAME_DATA_POOL *p);

/*
 * ������غ���
 */

// �������ȶ���
PRIORITY_QUEUE * priority_queue_create(int size, int preSize);

// ����
int priority_queue_push(PRIORITY_QUEUE *q, FRAME_DATA *item, unsigned long long time_stamp);

// ������ȡ
FRAME_DATA * priority_queue_poll(PRIORITY_QUEUE *q);

// ��������ȡ
FRAME_DATA * priority_queue_poll_without_wait(PRIORITY_QUEUE *q);

// �ж��Ƿ��������
int _is_type_ok(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type);

// ��������ȡ
FRAME_DATA * priority_queue_poll_by_type(PRIORITY_QUEUE *q, FRAME_AV_TYPE av_type);

// �����Ƿ���
int is_priority_queue_full(PRIORITY_QUEUE *q);

// �����Ƿ��
int is_priority_queue_empty(PRIORITY_QUEUE *q);

// ��ն���
int priority_queue_clean(PRIORITY_QUEUE *q);

// ���ٶ���
int priority_queue_destroy(PRIORITY_QUEUE *q);
