#pragma once
#include<math.h>
#include <string.h>
#include "block_queue.h"
typedef struct TS_LOADER
{
	int media_file_size; // ý���С���ֽڣ�
	char media_url[1024]; // ý���ַ
	int duration; // �ܴ�С
	int current_range; // ��ǰ���ص�
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
