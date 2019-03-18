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
	unsigned long long media_start_timestamp; // ý����ʼʱ���
	unsigned long long media_current_timestamp; // ý�嵱ǰʱ���
	int current_play_time; // ��ǰ����ʱ��
	int media_duration; // ý����ʱ��
	PLAY_STATUS status; // ״̬
	int display_width; // ��ʾ��
	int display_height; // ��ʾ��
	char media_url[1024]; // ý���ַ
	pthread_t ts_load_thread; // ts������߳�
	pthread_t ts_demux_thread; // ts���װ�߳�
	pthread_t pes_decode_thread; // pes�����߳�
	TS_LOADER *loader;
	TS_DEMUXER *demuxer;
} OTTER_PLAYER;

// ����������
OTTER_PLAYER * create_player(int display_width,int display_height);

// �趨ý��
int set_media(OTTER_PLAYER *p, char * media_url, int duration);

// ��ʱ��㲥��
int play_or_seek(OTTER_PLAYER *p, int time);

// ���ٲ�����
int destroy_player(OTTER_PLAYER *p);

int _create_loader(OTTER_PLAYER *p, int time);
int _destroy_loader(OTTER_PLAYER *p);
int _create_demuxer(OTTER_PLAYER *p);
int _destroy_demuxer(OTTER_PLAYER *p);

// ��ȡý����ʼʱ���
static void _get_media_start_timestamp(OTTER_PLAYER *p);