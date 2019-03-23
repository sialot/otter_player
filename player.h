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
#include "decoder_master.h"

// ����״̬
typedef enum PLAY_STATUS
{
	INIT_FINISH,
	WORKING,
	STOPED
}PLAY_STATUS;

// �������ṹ��
typedef struct OTTER_PLAYER
{
	unsigned int media_start_timestamp; // ý����ʼʱ���
	int current_play_time; // ��ǰ����ʱ��
	int media_duration; // ý����ʱ��
	PLAY_STATUS status; // ״̬
	int display_width; // ��ʾ��
	int display_height; // ��ʾ��
	char media_url[1024]; // ý���ַ
	pthread_t ts_load_thread; // ts������߳�
	pthread_t ts_demux_thread; // ts���װ�߳�
	pthread_t audio_decode_thread; // pes�����߳�
	pthread_t video_decode_thread; // pes�����߳�
	TS_LOADER *loader;
	TS_DEMUXER *demuxer;
	DECODER_MASTER *decoder_master;
} OTTER_PLAYER;

// ����������
OTTER_PLAYER * create_player(int display_width,int display_height);

// �趨ý��
int set_media(OTTER_PLAYER *p, char * media_url, int duration);

// ��ʱ��㲥��
int play_or_seek(OTTER_PLAYER *p, int time);


typedef struct JS_FRAME
{
	unsigned int len :32; // ���ݳ���
	unsigned int cur_time :32; // ��ǰʱ��
	unsigned int av_type :32; // ����
	int channels :32;
	unsigned char * data; // ����
} JS_FRAME;

// js��ȡ֡����
JS_FRAME * js_poll_frame(OTTER_PLAYER *p);

static int _create_loader(OTTER_PLAYER *p, int time);
static int _destroy_loader(OTTER_PLAYER *p);
static int _create_demuxer(OTTER_PLAYER *p);
static int _destroy_demuxer(OTTER_PLAYER *p);
static int _create_decoder_master(OTTER_PLAYER *p);
static int _destroy_decoder_master(OTTER_PLAYER *p);

// ��ȡý����ʼʱ���
static void _get_media_start_timestamp(OTTER_PLAYER *p);

// ���̲߳���
typedef struct _player_thread_param
{
	OTTER_PLAYER * p;      // ������
} _player_thread_param;

// TREAD FUNCTIONS
// �̺߳����������ļ�
void *_media_load_start(void * args);

// �̺߳��������װ
void *_media_demux_start(void * args);

// �̺߳�������Ƶ����
void *_audio_decode_start(void * args);

// �̺߳�������Ƶ����
void *_video_decode_start(void * args);

// ���ٲ�����
int destroy_player(OTTER_PLAYER *p);