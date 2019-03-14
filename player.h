#pragma once
#include<stdlib.h>
#include<pthread.h>
#include <string.h>

#include "block_queue.h"
#include "byte_list.h"
#include "ts_loader.h"

// ����״̬
typedef enum PLAY_STATUS
{
	INIT_FINISH, 
	PLAYING,
	PAUSING,
	STOPED
}PLAY_STATUS;

// �������ṹ��
typedef struct OTTER_PLAYER
{
	int current_play_time; // ��ǰ����ʱ��
	int media_duration; // ý����ʱ��
	PLAY_STATUS status; // ״̬
	int display_width; // ��ʾ��
	int display_height; // ��ʾ��
	char media_url[1024]; // ý���ַ
	pthread_t *ts_load_thread; // ts������߳�
	pthread_t *ts_demux_thread; // ts���װ�߳�
	pthread_t *pes_decode_thread; // pes�����߳�
	BLOCK_QUEUE *ts_pkt_queue; // ts���������
	BLOCK_QUEUE *pes_pkt_queue; // pes���������
	TS_LOADER *ts_loader;
} OTTER_PLAYER;

// ����������
OTTER_PLAYER * create_player(int display_width,int display_height);

// �趨ý��
int set_media(OTTER_PLAYER *p, char * media_url, int duration);

// ����
int play(OTTER_PLAYER *p);

// ��ʱ��㲥��
int play_by_time(OTTER_PLAYER *p, int time);

// ��ͣ
int do_pause();

// ֹͣ
int stop(OTTER_PLAYER *p);

// ���ٲ�����
int destory_player(OTTER_PLAYER *p);

// ���ò�����
static int _reset_player(OTTER_PLAYER *p);

// ׼��������
static int _create_loader(OTTER_PLAYER *p, int time);

// ���ټ�����
static int _destroy_loader(OTTER_PLAYER *p);

int test_poll_ts_pkt(OTTER_PLAYER *p);