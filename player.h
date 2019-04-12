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
#include "block_queue.h"
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
	PLAY_STATUS status; // ״̬
	int display_width; // ��ʾ��
	int display_height; // ��ʾ��
	pthread_t ts_load_thread; // ts������߳�
	pthread_t ts_demux_thread; // ts���װ�߳�
	pthread_t audio_decode_thread; // pes�����߳�
	pthread_t video_decode_thread; // pes�����߳�
	TS_DEMUXER *demuxer;
	DECODER_MASTER *decoder_master;
	BLOCK_QUEUE *ts_pkt_queue; // ts���������
	BYTE_LIST *ts_pkt_buffer; // ts������
} OTTER_PLAYER;

// js֡
typedef struct JS_FRAME
{
	unsigned int len : 32; // ���ݳ���
	unsigned int cur_time : 32; // ��ǰʱ��
	FRAME_AV_TYPE av_type : 32; // ����
	int channels : 32;
	unsigned char * data; // ����
} JS_FRAME;

// ���̲߳���
typedef struct _player_thread_param
{
	OTTER_PLAYER * p;      // ������
} _player_thread_param;

// ����������
OTTER_PLAYER * create_player(int display_width,int display_height);

// ��ʱ��㲥��
int play(OTTER_PLAYER *p);

// �����Ƿ�Ϊ��
int js_can_load_file(OTTER_PLAYER *p);

// js����ý������
void js_push_data(OTTER_PLAYER * p, unsigned char * bytes, int len);

// js��ȡ֡����
JS_FRAME * js_poll_frame(OTTER_PLAYER *p);

// ���������ٽ��װ
static int _create_demuxer(OTTER_PLAYER *p);
static int _destroy_demuxer(OTTER_PLAYER *p);

// ���������ٽ���
static int _create_decoder_master(OTTER_PLAYER *p);
static int _destroy_decoder_master(OTTER_PLAYER *p);

// TREAD FUNCTIONS
// �̺߳��������װ
void *_media_demux_start(void * args);

// �̺߳�������Ƶ����
void *_audio_decode_start(void * args);

// �̺߳�������Ƶ����
void *_video_decode_start(void * args);

// ���ٲ�����
int destroy_player(OTTER_PLAYER *p);