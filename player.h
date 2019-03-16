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

// 播放状态
typedef enum PLAY_STATUS
{
	INIT_FINISH, 
	WORKING
}PLAY_STATUS;

// 播放器结构体
typedef struct OTTER_PLAYER
{
	int current_play_time; // 当前播放时间
	unsigned long long media_start_timestamp; // 媒体起始时间戳
	int media_duration; // 媒体总时长
	PLAY_STATUS status; // 状态
	int display_width; // 显示宽
	int display_height; // 显示高
	char media_url[1024]; // 媒体地址
	pthread_t ts_load_thread; // ts层加载线程
	pthread_t ts_demux_thread; // ts解封装线程
	pthread_t pes_decode_thread; // pes解码线程
	TS_LOADER *ts_loader;
	TS_DEMUXER *ts_demuxer;
} OTTER_PLAYER;

// 创建播放器
OTTER_PLAYER * create_player(int display_width,int display_height);

// 设定媒体
int set_media(OTTER_PLAYER *p, char * media_url, int duration);

// 播放, 返回当前起始 时间戳
int play(OTTER_PLAYER *p);

// 按时间点播放, 返回当前起始 时间戳
int seek(OTTER_PLAYER *p, int time);

// 销毁播放器
int destroy_player(OTTER_PLAYER *p);

// 获取媒体起始时间戳
static void _get_media_start_timestamp(OTTER_PLAYER *p);

int test_poll_ts_pkt(OTTER_PLAYER *p);