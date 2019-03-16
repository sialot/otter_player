#ifndef EM_PORT_API
#    if defined(__EMSCRIPTEN__)
#        include <emscripten.h>
#        if defined(__cplusplus)
#            define EM_PORT_API(rettype) extern "C" rettype EMSCRIPTEN_KEEPALIVE
#        else
#            define EM_PORT_API(rettype) rettype EMSCRIPTEN_KEEPALIVE
#        endif
#    else
#        if defined(__cplusplus)
#            define EM_PORT_API(rettype) extern "C" rettype
#        else
#            define EM_PORT_API(rettype) rettype
#        endif
#    endif
#endif

#include "player.h"

// 创建播放器
EM_PORT_API(OTTER_PLAYER *) create_player(int display_width, int display_height)
{
	OTTER_PLAYER *p = malloc(sizeof(OTTER_PLAYER));

	if (p == NULL)
	{
		printf("create player failed. can't init mem!\n");
		return NULL;
	}

	// 初始化状态，显示宽高
	p->current_play_time = 0;
	p->media_start_timestamp = 0;
	p->media_duration = -1;
	p->status = INIT_FINISH;
	p->display_height = display_height;
	p->display_width = display_width;
	p->media_url[0] = '\0';
	p->ts_loader = NULL;
	p->ts_demuxer = NULL;

	printf("player inited! \n");
	return p;
}

// 设定媒体，只能一次
EM_PORT_API(int) set_media(OTTER_PLAYER *p, char * media_url, int duration)
{
	printf("set media start >>url:%s, duration:%d\n", p->media_url, duration);
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	if (p->status != INIT_FINISH)
	{
		return -1;
	}

	// 设置新媒体
	p->media_duration = duration;
	strcpy(p->media_url, media_url);

	// 获取媒体起始时间戳
	_get_media_start_timestamp(p);
	printf("set media complete!\n");
	return 0;
}

// 播放
EM_PORT_API(int) play(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}
	return seek(p, p->current_play_time);
}

// 按时间播放
EM_PORT_API(int) seek(OTTER_PLAYER *p, int time)
{
	printf("play_by_time(%d) \n", time);
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	// 工作中直接调整loader时间
	if (p->status == WORKING)
	{
		ts_loader_seek_time(p->ts_loader, time);
		return 0;
	}

	// 创建加载器
	if (p->ts_loader == NULL)
	{
		TS_LOADER *loader = ts_loader_create(p->media_url, p->media_duration, time);
		if (loader == NULL)
		{
			printf("can't create loadder!\n");
			return -1;
		}
		p->ts_loader = loader;
	}

	// 解封装
	if (p->ts_demuxer == NULL)
	{
		TS_DEMUXER *demuxer = ts_demuxer_create();
		if (demuxer == NULL)
		{
			printf("can't create demuxer!\n");
			return -1;
		}
		p->ts_demuxer = demuxer;
	}

	// 解码



	// TODO

	return 0;
}

EM_PORT_API(int) destroy_player(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}


	// TODO
	//pthread_t *ts_demux_thread; // ts解封装线程
	//pthread_t *pes_decode_thread; // pes解码线程

	free(p);
	return 0;
}

// 获取媒体起始时间戳
void _get_media_start_timestamp(OTTER_PLAYER * p)
{
	printf("_get_media_start_timestamp\n");

	TS_LOADER *loader = ts_loader_create(p->media_url, p->media_duration, 0);
	TS_DEMUXER *demuxer = ts_demuxer_create();

	int ibreak = 0;

	while (!loader->is_finish && !ibreak)
	{
		ts_loader_range_load(loader);
		while (!is_ts_queue_empty(loader) && !ibreak) {
			BYTE_LIST *ts_pkt = poll_ts_pkt(loader);
			demux_ts_pkt(demuxer, ts_pkt->pBytes);
			byte_list_destroy(ts_pkt);
			while (!is_pes_queue_empty(demuxer) && !ibreak)
			{
				TS_PES_PACKET *pesPkt = poll_pes_pkt(demuxer);
				p->media_start_timestamp = pesPkt->PTS;
				ibreak = 1;
				printf("get start_timestamp:%lld \n", p->media_start_timestamp);
				break;
			}
		}
	}
	ts_loader_destroy(loader);
	ts_demuxer_destroy(demuxer);
	return;
}

int test_poll_ts_pkt(OTTER_PLAYER * p)
{
	BYTE_LIST *list = (BYTE_LIST *)block_queue_poll(p->ts_loader->ts_pkt_queue);
	printf("PLAYER_TEST>>ts_pkt_queue poll>> data_size:%d \n", list->used_len);
	return 0;
}
