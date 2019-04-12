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
	p->current_play_time = 0;
	p->status = INIT_FINISH;
	p->display_height = display_height;
	p->display_width = display_width;
	p->demuxer = NULL;
	p->ts_pkt_queue = NULL;
	p->ts_pkt_buffer = NULL;

	BLOCK_QUEUE *ts_queue = block_queue_create(1);
	if (ts_queue == NULL)
	{
		printf("ts_queue init failed!\n");
		destroy_player(p);
		return NULL;
	}
	p->ts_pkt_queue = ts_queue;

	if (p->ts_pkt_buffer == NULL)
	{
		BYTE_LIST *buffer = byte_list_create(10000);
		p->ts_pkt_buffer = buffer;
	}

	// 创建必要对象
	if (_create_demuxer(p) || _create_decoder_master(p))
	{
		printf("can't create loadder or demuxer or decoder_master!\n");
		destroy_player(p);
		return NULL;
	}
	printf("player is ready !\n");
	return p;
}

// 按时间播放
EM_PORT_API(int) play(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	if (p->status == INIT_FINISH)
	{
		p->status = WORKING;

		// 构建多线程参数
		_player_thread_param args;
		args.p = p;

		// 设置线程属性
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		// 创建线程
		pthread_create(&p->ts_demux_thread, &attr, _media_demux_start, (void *)&args);
		pthread_create(&p->audio_decode_thread, &attr, _audio_decode_start, (void *)&args);
		pthread_create(&p->video_decode_thread, &attr, _video_decode_start, (void *)&args);

		pthread_attr_destroy(&attr);
	}

	return 0;
}
// 队列是否为空
EM_PORT_API(int) js_can_load_file(OTTER_PLAYER *p)
{
	return (p->ts_pkt_buffer->used_len == 0);
}

// js推送媒体数据
EM_PORT_API(void) js_push_data(OTTER_PLAYER * p, unsigned char * bytes, int len)
{

	byte_list_add_list(p->ts_pkt_buffer, bytes, len);
	block_queue_push(p->ts_pkt_queue, p->ts_pkt_buffer);
}

// js获取帧数据
EM_PORT_API(JS_FRAME *) js_poll_frame(OTTER_PLAYER *p)
{
	if (p->status == INIT_FINISH)
	{
		printf("player not working! no frame data \n");
		return NULL;
	}

	FRAME_DATA *f = priority_queue_poll_without_wait(p->decoder_master->js_frame_queue);
	if (f == NULL)
	{
		return NULL;
	}

	while (f->av_type == VIDEO && f->ptime < p->current_play_time)
	{
		f = priority_queue_poll_without_wait(p->decoder_master->js_frame_queue);
	}

	JS_FRAME *jframe = malloc(sizeof(JS_FRAME));
	jframe->len = f->len;
	jframe->cur_time = f->ptime  - p->media_start_timestamp;
	jframe->channels = 0;
	jframe->av_type = f->av_type;
	if (f->av_type == AUDIO)
	{
		jframe->channels = f->channels;
		p->current_play_time = f->ptime;
	}
	jframe->data = f->data;
	free(f);
	return jframe;
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

int _create_decoder_master(OTTER_PLAYER * p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	DECODER_MASTER *decoder_master = decoder_master_create(p->display_width, p->display_height);
	if (decoder_master == NULL)
	{
		printf("can't create decoder_master!\n");
		return -1;
	}
	p->decoder_master = decoder_master;
	return 0;
}

int _destroy_decoder_master(OTTER_PLAYER * p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	if (p->decoder_master == NULL)
	{
		return 0;
	}
	decoder_master_destroy(p->decoder_master);
	p->decoder_master = NULL;
	return 0;
}

// 线程函数，解封装
void * _media_demux_start(void * args)
{
	printf("thread start [_media_demux_start]!\n");
	_player_thread_param *param = (_player_thread_param*)args;
	OTTER_PLAYER *p = param->p;

	if (p == NULL) {
		printf("player is null!\n");
		return NULL;
	}
	if (p->demuxer == NULL)
	{
		printf("demuxer is null!\n");
		return NULL;
	}

	while (p->status == WORKING)
	{
		BYTE_LIST *ts_pkt = block_queue_poll(p->ts_pkt_queue);

		int len = ts_pkt->used_len;
		for (int i = 0; i < len / 188; i++)
		{
			unsigned char * data = ts_pkt->pBytes + i * 188;
			demux_ts_pkt(p->demuxer, data);
		}

		byte_list_clean(ts_pkt);
	}
	printf("thread exit [%s]!\n", __FUNCTION__);
	return NULL;
}

// 线程函数，音频解码
void * _audio_decode_start(void * args)
{
	printf("thread start [_audio_decode_start]!\n");
	_player_thread_param *param = (_player_thread_param*)args;
	OTTER_PLAYER *p = param->p;

	if (p == NULL) {
		printf("player is null!\n");
		return NULL;
	}
	if (p->decoder_master == NULL)
	{
		printf("decoder_master is null!\n");
		return NULL;
	}
	int init_start_timestamp = 0;
	while (p->status == WORKING)
	{
		FRAME_DATA *esFrame = poll_pes_pkt_by_type(p->demuxer, AUDIO);

		if (!init_start_timestamp)
		{
			init_start_timestamp = 1;
			p->media_start_timestamp = esFrame->ptime;
		}
		decode_frame(p->decoder_master, esFrame);
		frame_data_destory(esFrame);
	}
	printf("thread exit [%s]!\n", __FUNCTION__);
	return NULL;
}

// 线程函数，音频解码
void * _video_decode_start(void * args)
{
	printf("thread start [_video_decode_start]!\n");
	_player_thread_param *param = (_player_thread_param*)args;
	OTTER_PLAYER *p = param->p;

	if (p == NULL) {
		printf("player is null!\n");
		return NULL;
	}
	if (p->decoder_master == NULL)
	{
		printf("decoder_master is null!\n");
		return NULL;
	}

	while (p->status == WORKING)
	{
		FRAME_DATA *esFrame = poll_pes_pkt_by_type(p->demuxer, VIDEO);
		decode_frame(p->decoder_master, esFrame);
		frame_data_destory(esFrame);
	}
	printf("thread exit [%s]!\n", __FUNCTION__);
	return NULL;
}

// 销毁播放器
EM_PORT_API(int) destroy_player(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	p->status = STOPED;

	// 销毁加载器和对应线程
	_destroy_demuxer(p);
	_destroy_decoder_master(p);

	if (p->ts_pkt_buffer != NULL)
	{
		byte_list_destroy(p->ts_pkt_buffer);
	}
	if (p->ts_pkt_queue != NULL)
	{
		block_queue_destroy(p->ts_pkt_queue);
	}

	free(p);
	return 0;
}