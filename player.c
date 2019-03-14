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

// 每次缓存的最大ts包数
const int TS_LOAD_BUFFER_COUNT = 500;

// 每次缓存的最大pes包数
const int PES_BUFFER_COUNT = 256;

EM_PORT_API(OTTER_PLAYER *) create_player(int display_width, int display_height)
{
	OTTER_PLAYER *p = malloc(sizeof(OTTER_PLAYER));

	if (p == NULL)
	{
		printf("create player failed. can't init mem!\n");
		return NULL;
	}

	p->current_play_time = 0;
	p->media_duration = 0;
	p->status = INIT_FINISH;
	p->display_height = display_height;
	p->display_width = display_width;
	p->media_url[0] = '\0';
	p->ts_load_thread = NULL;
	p->ts_demux_thread = NULL;
	p->pes_decode_thread = NULL;

	p->ts_pkt_queue = block_queue_create(TS_LOAD_BUFFER_COUNT);
	if (p->ts_pkt_queue == NULL)
	{
		printf("ts_pkt_queue init failed!\n");
		return NULL;
	}

	p->pes_pkt_queue = block_queue_create(PES_BUFFER_COUNT);
	if (p->pes_pkt_queue == NULL)
	{
		printf("pes_pkt_queue init failed!\n");
		return NULL;
	}

	printf("player inited! \n");
	return p;
}

EM_PORT_API(int) set_media(OTTER_PLAYER *p, char * media_url, int duration)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}
	if (p->status == PLAYING)
	{
		_reset_player(p);
	}
	p->media_duration = duration;
	strcpy(p->media_url, media_url);
	printf("set media_duration >>%d\n", p->media_duration);
	return 0;
}

EM_PORT_API(int) play(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}
	return play_by_time(p, p->current_play_time);
}

EM_PORT_API(int) play_by_time(OTTER_PLAYER *p, int time)
{
	printf("play_by_time(%d) \n", time);
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	// 播放状态确认
	if (p->status == PLAYING || p->status == PAUSING)
	{
		// 销毁加载器
		_destroy_loader(p);
	}

	// 创建加载器
	if (_create_loader(p, time) == -1)
	{
		printf("_prepare_loader failed!\n");
		return -1;
	}

	// 解封装

	// 解码
	// TODO
	return 0;
}

EM_PORT_API(int) do_pause()
{

	// TODO
	return 0;
}

EM_PORT_API(int) stop(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	// TODO
	return 0;
}

EM_PORT_API(int) destory_player(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}
	block_queue_destory(p->ts_pkt_queue);
	block_queue_destory(p->pes_pkt_queue);

	// TODO


	free(p);
	return 0;
}

static int _create_loader(OTTER_PLAYER *p, int time)
{

	// 创建加载器
	if (p->ts_loader == NULL)
	{
		TS_LOADER *loader = create_ts_loader(p->media_url, p->media_duration, p->ts_pkt_queue);
		if (loader == NULL)
		{
			printf("can't create loadder!\n");
			return -1;
		}
		p->ts_loader = loader;
	}

	// 创建线程
	if (p->ts_load_thread == NULL)
	{

		TS_LOADER_PARAM param;
		param.l = p->ts_loader;
		param.pCurrentPlayTime = time;

		if (pthread_create(p->ts_load_thread, NULL, (void *)ts_loader_seek_and_trans, &param) != 0)
		{
			printf("pthread_create failed!\n");
			return -1;
		}
	}

	return 0;
}

static int _destroy_loader(OTTER_PLAYER *p)
{
	// 销毁旧线程
	if (p->ts_load_thread != NULL)
	{
		pthread_exit(p->ts_load_thread);
		p->ts_load_thread = NULL;
	}

	// 销毁旧加载器
	if (p->ts_loader != NULL)
	{
		ts_loader_destroy(p->ts_loader);
		p->ts_loader = NULL;
	}
	return 0;
}

static int _reset_player(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}
	stop(p);
	p->current_play_time = 0;
	p->media_duration = 0;
	p->media_url[0] = '\0';

	// 销毁加载器
	_destroy_loader(p);

	// TODO

	return 0;
}

int test_poll_ts_pkt(OTTER_PLAYER * p)
{
	BYTE_LIST *list = (BYTE_LIST *)block_queue_poll(p->ts_pkt_queue);
	printf("PLAYER_TEST>>ts_pkt_queue poll>> data_size:%d \n", list->used_len);
	return 0;
}
