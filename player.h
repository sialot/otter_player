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

// 播放状态
typedef enum PLAY_STATUS
{
	INIT_FINISH,
	WORKING,
	STOPED
}PLAY_STATUS;

// 播放器结构体
typedef struct OTTER_PLAYER
{
	unsigned int media_start_timestamp; // 媒体起始时间戳
	int current_play_time; // 当前播放时间
	PLAY_STATUS status; // 状态
	int display_width; // 显示宽
	int display_height; // 显示高
	pthread_t ts_load_thread; // ts层加载线程
	pthread_t ts_demux_thread; // ts解封装线程
	pthread_t audio_decode_thread; // pes解码线程
	pthread_t video_decode_thread; // pes解码线程
	TS_DEMUXER *demuxer;
	DECODER_MASTER *decoder_master;
	BLOCK_QUEUE *ts_pkt_queue; // ts包缓存队列
	BYTE_LIST *ts_pkt_buffer; // ts包缓存
} OTTER_PLAYER;

// js帧
typedef struct JS_FRAME
{
	unsigned int len : 32; // 数据长度
	unsigned int cur_time : 32; // 当前时间
	FRAME_AV_TYPE av_type : 32; // 类型
	int channels : 32;
	unsigned char * data; // 数据
} JS_FRAME;

// 多线程参数
typedef struct _player_thread_param
{
	OTTER_PLAYER * p;      // 播放器
} _player_thread_param;

// 创建播放器
OTTER_PLAYER * create_player(int display_width,int display_height);

// 按时间点播放
int play(OTTER_PLAYER *p);

// 队列是否为空
int js_can_load_file(OTTER_PLAYER *p);

// js推送媒体数据
void js_push_data(OTTER_PLAYER * p, unsigned char * bytes, int len);

// js获取帧数据
JS_FRAME * js_poll_frame(OTTER_PLAYER *p);

// 创建、销毁解封装
static int _create_demuxer(OTTER_PLAYER *p);
static int _destroy_demuxer(OTTER_PLAYER *p);

// 创建、销毁解码
static int _create_decoder_master(OTTER_PLAYER *p);
static int _destroy_decoder_master(OTTER_PLAYER *p);

// TREAD FUNCTIONS
// 线程函数，解封装
void *_media_demux_start(void * args);

// 线程函数，音频解码
void *_audio_decode_start(void * args);

// 线程函数，视频解码
void *_video_decode_start(void * args);

// 销毁播放器
int destroy_player(OTTER_PLAYER *p);