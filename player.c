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

	p->media_start_timestamp = 0;
	p->media_current_timestamp = 0;
	p->current_play_time = 0;
	p->media_duration = 0;
	p->status = INIT_FINISH;
	p->display_height = display_height;
	p->display_width = display_width;
	p->media_url[0] = '\0';
	p->loader = NULL;
	p->demuxer = NULL;
	return p;
}

// 设定媒体
EM_PORT_API(int) set_media(OTTER_PLAYER *p, char * media_url, int duration)
{
	printf("set media start >>url:%s, duration:%d\n", p->media_url, duration);

	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	// 播放中不允许重设媒体
	if (p->status == WORKING)
	{
		printf("player media is playing or pausing ,can't set media!\n");
		return -1;
	}

	// 清空现有缓存队列
	_destroy_loader(p);

	//pes 队列清空
	_destroy_demuxer(p);

	p->media_start_timestamp = 0;
	p->media_current_timestamp = 0;
	p->current_play_time = 0;
	p->media_duration = 0;
	p->media_duration = duration;
	strcpy(p->media_url, media_url);

	// 创建必要对象
	if (_create_loader(p, 0) || _create_demuxer(p))
	{
		printf("can't create loadder or demuxer!\n");
		destroy_player(p);
		return -1;
	}

	// 获取媒体起始时间戳
	_get_media_start_timestamp(p);
	printf("set media complete!\n");
	return 0;
}

// 按时间播放
EM_PORT_API(int) play_or_seek(OTTER_PLAYER *p, int time)
{
	printf("seek(%d) \n", time);

	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	if (p->loader->start_time != time)
	{
		_destroy_loader(p);

		if (_create_loader(p, time))
		{
			printf("can't create loadder!\n");
			destroy_player(p);
		}
	}

	// TODO
	// ...
	// ...

	return 0;
}

EM_PORT_API(int) destroy_player(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}


	// 销毁加载器和对应线程
	_destroy_loader(p);
	_destroy_demuxer(p);

	// TODO
	//pthread_t *ts_demux_thread; // ts解封装线程
	//pthread_t *pes_decode_thread; // pes解码线程

	free(p);
	return 0;
}

int _create_loader(OTTER_PLAYER * p, int time)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	TS_LOADER *loader = ts_loader_create(p->media_url, p->media_duration, 0);
	if (loader == NULL)
	{
		printf("can't create loadder!\n");
		return -1;
	}
	p->loader = loader;
	return 0;
}

int _destroy_loader(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	if (p->loader == NULL)
	{
		return 0;
	}
	ts_loader_destroy(p->loader);
	p->loader = NULL;
	return 0;
}

int _create_demuxer(OTTER_PLAYER * p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	TS_DEMUXER *demuxer = ts_demuxer_create();
	if (demuxer == NULL)
	{
		printf("can't create demuxer!\n");
		return -1;
	}
	p->demuxer = demuxer;
	return 0;
}

int _destroy_demuxer(OTTER_PLAYER * p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	if (p->demuxer == NULL)
	{
		return 0;
	}
	ts_demuxer_destroy(p->demuxer);
	p->demuxer = NULL;
	return 0;
}

// 获取媒体起始时间戳
void _get_media_start_timestamp(OTTER_PLAYER * p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return;
	}
	if (p->loader == NULL)
	{
		printf("loader is null!\n");
		return;
	}
	if (p->demuxer == NULL)
	{
		printf("demuxer is null!\n");
		return;
	}

	int ibreak = 0;
	while (!p->loader->is_finish && !ibreak)
	{
		ts_loader_range_load(p->loader);
		while (!is_ts_queue_empty(p->loader) && !ibreak) {
			BYTE_LIST *ts_pkt = poll_ts_pkt(p->loader);
			demux_ts_pkt(p->demuxer, ts_pkt->pBytes);
			byte_list_destroy(ts_pkt);

			while (!is_pes_queue_empty(p->demuxer) && !ibreak)
			{
				FRAME_DATA *pesPkt = poll_pes_pkt(p->demuxer);
				p->media_start_timestamp = pesPkt->time_stamp;
				ibreak = 1;
				printf("get start_timestamp:%lld \n", p->media_start_timestamp);
				break;
			}
		}
	}
	return;
}