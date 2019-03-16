#pragma once

#if defined(__EMSCRIPTEN__)
#include <pthread.h>
#else
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#endif

#include<stdlib.h>
#include <string.h>

#include "byte_list.h"
#include "ts_loader.h"
#include "ts_demuxer.h"

// ����״̬
typedef enum PLAY_STATUS
{
	INIT_FINISH, 
	WORKING
}PLAY_STATUS;

// �������ṹ��
typedef struct OTTER_PLAYER
{
	int current_play_time; // ��ǰ����ʱ��
	unsigned long long media_start_timestamp; // ý����ʼʱ���
	int media_duration; // ý����ʱ��
	PLAY_STATUS status; // ״̬
	int display_width; // ��ʾ��
	int display_height; // ��ʾ��
	char media_url[1024]; // ý���ַ
	pthread_t ts_load_thread; // ts������߳�
	pthread_t ts_demux_thread; // ts���װ�߳�
	pthread_t pes_decode_thread; // pes�����߳�
	TS_LOADER *ts_loader;
	TS_DEMUXER *ts_demuxer;
} OTTER_PLAYER;

// ����������
OTTER_PLAYER * create_player(int display_width,int display_height);

// �趨ý��
int set_media(OTTER_PLAYER *p, char * media_url, int duration);

// ����, ���ص�ǰ��ʼ ʱ���
int play(OTTER_PLAYER *p);

// ��ʱ��㲥��, ���ص�ǰ��ʼ ʱ���
int seek(OTTER_PLAYER *p, int time);

// ���ٲ�����
int destroy_player(OTTER_PLAYER *p);

// ��ȡý����ʼʱ���
static void _get_media_start_timestamp(OTTER_PLAYER *p);

int test_poll_ts_pkt(OTTER_PLAYER *p);