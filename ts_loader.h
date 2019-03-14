#pragma once
#include<math.h>
#include <string.h>
#include "block_queue.h"
typedef struct TS_LOADER
{
	int media_file_size; // 媒体大小（字节）
	char media_url[1024]; // 媒体地址
	int duration; // 总大小
	int current_range; // 当前加载点
	BLOCK_QUEUE *ts_pkt_queue;
} TS_LOADER;

typedef struct TS_LOADER_PARAM
{
	TS_LOADER *l;
	int pCurrentPlayTime;
}TS_LOADER_PARAM;

TS_LOADER * create_ts_loader(char *pMediaUrl, int duration, BLOCK_QUEUE *ts_pkt_queue);

int ts_loader_seek_and_trans(TS_LOADER_PARAM *p);

int ts_loader_destroy(TS_LOADER *l);

static void _get_file_size(TS_LOADER * l);
