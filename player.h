#pragma once
#include<stdlib.h>
#include<pthread.h>
#include <string.h>

#include "block_queue.h"
#include "byte_list.h"
#include "ts_loader.h"

// 播放状态
typedef enum PLAY_STATUS
{
	INIT_FINISH, 
	PLAYING,
	PAUSING,
	STOPED
}PLAY_STATUS;

// 播放器结构体
typedef struct OTTER_PLAYER
{
	int current_play_time; // 当前播放时间
	int media_duration; // 媒体总时长
	PLAY_STATUS status; // 状态
	int display_width; // 显示宽
	int display_height; // 显示高
	char media_url[1024]; // 媒体地址
	pthread_t *ts_load_thread; // ts层加载线程
	pthread_t *ts_demux_thread; // ts解封装线程
	pthread_t *pes_decode_thread; // pes解码线程
	BLOCK_QUEUE *ts_pkt_queue; // ts包缓存队列
	BLOCK_QUEUE *pes_pkt_queue; // pes包缓存队列
	TS_LOADER *ts_loader;
} OTTER_PLAYER;

// 创建播放器
OTTER_PLAYER * create_player(int display_width,int display_height);

// 设定媒体
int set_media(OTTER_PLAYER *p, char * media_url, int duration);

// 播放
int play(OTTER_PLAYER *p);

// 按时间点播放
int play_by_time(OTTER_PLAYER *p, int time);

// 暂停
int do_pause();

// 停止
int stop(OTTER_PLAYER *p);

// 销毁播放器
int destory_player(OTTER_PLAYER *p);

// 重置播放器
static int _reset_player(OTTER_PLAYER *p);

// 准备加载器
static int _create_loader(OTTER_PLAYER *p, int time);

// 销毁加载器
static int _destroy_loader(OTTER_PLAYER *p);

int test_poll_ts_pkt(OTTER_PLAYER *p);