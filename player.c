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

// ÿ�λ�������ts����
const int TS_LOAD_BUFFER_COUNT = 1000;

// ÿ�λ�������pes����
const int PES_BUFFER_COUNT = 256;

// ����������
EM_PORT_API(OTTER_PLAYER *) create_player(int display_width, int display_height)
{
	OTTER_PLAYER *p = malloc(sizeof(OTTER_PLAYER));

	if (p == NULL)
	{
		printf("create player failed. can't init mem!\n");
		return NULL;
	}

	// ��ʼ��״̬����ʾ���
	p->status = INIT_FINISH;
	p->display_height = display_height;
	p->display_width = display_width;

	// ��ʼ��ý������
	_clean_old_media_info(p);

	// ��ʼ�� �����������װ���������� ��ָ��
	p->ts_loader = NULL;

	printf("player inited! \n");
	return p;
}

// �趨ý��
EM_PORT_API(int) set_media(OTTER_PLAYER *p, char * media_url, int duration)
{
	printf("set media start >>url:%s, duration:%d\n", p->media_url, duration);
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	// �����в���������ý��
	if (p->status == PLAYING || p->status == PAUSING)
	{
		printf("player media is playing or pausing ,can't set media!\n");
		return -1;
	}

	// �����ý��
	_clean_old_media_info(p);

	// ������л������
	ts_loader_destroy(p->ts_loader);
	p->ts_loader = NULL;

	//pes ������� TODO

	// ������ý��
	p->media_duration = duration;
	strcpy(p->media_url, media_url);

	// ��ȡý����ʼʱ���
	_get_media_start_timestamp(p);
	printf("set media complete!\n");
	return 0;
}

// ����
EM_PORT_API(int) play(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}
	return play_by_time(p, p->current_play_time);
}

// ��ʱ�䲥��
EM_PORT_API(int) play_by_time(OTTER_PLAYER *p, int time)
{
	printf("play_by_time(%d) \n", time);
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	// ����������
	if (_create_loader_and_thread(p, time) == -1)
	{
		printf("_prepare_loader failed!\n");
		return -1;
	}

	// ���װ

	// ����

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

EM_PORT_API(int) destroy_player(OTTER_PLAYER *p)
{
	if (p == NULL) {
		printf("player is null!\n");
		return -1;
	}

	// ���ټ������Ͷ�Ӧ�߳�
	_destroy_loader_and_thread(p);

	// TODO
	//pthread_t *ts_demux_thread; // ts���װ�߳�
	//pthread_t *pes_decode_thread; // pes�����߳�

	free(p);
	return 0;
}

int _clean_old_media_info(OTTER_PLAYER *p)
{
	p->current_play_time = 0;
	p->media_duration = -1;
	p->media_start_timestamp = -1;
	p->media_current_timestamp = -1;
	p->media_url[0] = '\0';
	return 0;
}

// ��ȡý����ʼʱ���
int _get_media_start_timestamp(OTTER_PLAYER * p)
{
	printf("_get_media_start_timestamp\n");

	TS_LOADER *loader = create_ts_loader(p->media_url, p->media_duration, 0, TS_LOAD_BUFFER_COUNT);

	while (!loader->is_finish)
	{
		ts_loader_range_load(loader);
		while (!is_block_queue_empty(loader->ts_pkt_queue)) {
			BYTE_LIST *ts_pkt = block_queue_poll(loader->ts_pkt_queue);
			printf("ts_packet size:%d\n", ts_pkt->used_len);
		}
	}
	ts_loader_destroy(loader);
	return 0;
}

static int _create_loader_and_thread(OTTER_PLAYER *p, int time)
{	
	// ���ټ��������߳�
	_destroy_loader_and_thread(p);

	// ����������
	if (p->ts_loader == NULL)
	{
		TS_LOADER *loader = create_ts_loader(p->media_url, p->media_duration, time, TS_LOAD_BUFFER_COUNT);
		if (loader == NULL)
		{
			printf("can't create loadder!\n");
			return -1;
		}
		p->ts_loader = loader;
	}

	// �����߳�  TODO

	return 0;
}

static int _destroy_loader_and_thread(OTTER_PLAYER *p)
{
	// ���پ��߳�
	pthread_detach(p->ts_load_thread);

	// ���پɼ�����
	if (p->ts_loader != NULL)
	{
		ts_loader_destroy(p->ts_loader);
		p->ts_loader = NULL;
	}
	return 0;
}

int test_poll_ts_pkt(OTTER_PLAYER * p)
{
	BYTE_LIST *list = (BYTE_LIST *)block_queue_poll(p->ts_loader->ts_pkt_queue);
	printf("PLAYER_TEST>>ts_pkt_queue poll>> data_size:%d \n", list->used_len);
	return 0;
}
