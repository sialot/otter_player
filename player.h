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
	int media_duration; // 媒体总时长
	PLAY_STATUS status; // 状态
	int display_width; // 显示宽
	int display_height; // 显示高
	char media_url[1024]; // 媒体地址
	pthread_t ts_load_thread; // ts层加载线程
	pthread_t ts_demux_thread; // ts解封装线程
	pthread_t audio_decode_thread; // pes解码线程
	pthread_t video_decode_thread; // pes解码线程
	TS_LOADER *loader;
	TS_DEMUXER *demuxer;
	DECODER_MASTER *decoder_master;
} OTTER_PLAYER;

// 创建播放器
OTTER_PLAYER * create_player(int display_width,int display_height);

// 设定媒体
int set_media(OTTER_PLAYER *p, char * media_url, int duration);

// 按时间点播放
int play_or_seek(OTTER_PLAYER *p, int time);


typedef struct JS_FRAME
{
	unsigned int len :32; // 数据长度
	unsigned int cur_time :32; // 当前时间
	unsigned int av_type :32; // 类型
	int channels :32;
	unsigned char * data; // 数据
} JS_FRAME;

// js获取帧数据
JS_FRAME * js_poll_frame(OTTER_PLAYER *p);

static int _create_loader(OTTER_PLAYER *p, int time);
static int _destroy_loader(OTTER_PLAYER *p);
static int _create_demuxer(OTTER_PLAYER *p);
static int _destroy_demuxer(OTTER_PLAYER *p);
static int _create_decoder_master(OTTER_PLAYER *p);
static int _destroy_decoder_master(OTTER_PLAYER *p);

// 获取媒体起始时间戳
static void _get_media_start_timestamp(OTTER_PLAYER *p);

// 多线程参数
typedef struct _player_thread_param
{
	OTTER_PLAYER * p;      // 播放器
} _player_thread_param;

// TREAD FUNCTIONS
// 线程函数，加载文件
void *_media_load_start(void * args);

// 线程函数，解封装
void *_media_demux_start(void * args);

// 线程函数，音频解码
void *_audio_decode_start(void * args);

// 线程函数，视频解码
void *_video_decode_start(void * args);

// 销毁播放器
int destroy_player(OTTER_PLAYER *p);