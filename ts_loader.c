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

#include "ts_loader.h"

// ÿ��http������ص�ts����
const int PKT_NUM_PER_TIME = 500;

// ����������
TS_LOADER * create_ts_loader(char * mediaUrl, int duration, int start_time, int buffer_count)
{
	if (start_time < 0)
	{
		printf("create_ts_loader init failed.start_time < 0! \n");
		return NULL;
	}

	if (duration < 0)
	{
		printf("create_ts_loader init failed.duration < 0! \n");
		return NULL;
	}

	if (buffer_count <= 0)
	{
		printf("create_ts_loader init failed.buffer_count <= 0! \n");
		return NULL;
	}

	TS_LOADER *loader = malloc(sizeof(TS_LOADER));

	if (loader == NULL)
	{
		printf("create_ts_loader init failed.can't get mem! \n");
		return NULL;
	}

	if (0 != pthread_mutex_init(&loader->data_mutex, NULL))
	{
		printf("[%s]pthread_mutex_init failed!\n", __FUNCTION__);
		free(loader);
		loader = NULL;
		return NULL;
	}

	if (pthread_cond_init(&loader->msg_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&loader->data_mutex);
		printf("[%s]pthread_cond_init failed!\n", __FUNCTION__);
		free(loader);
		loader = NULL;
		return NULL;
	}

	BLOCK_QUEUE *ts_queue = block_queue_create(buffer_count);
	if (ts_queue == NULL)
	{
		pthread_mutex_destroy(&loader->data_mutex);
		pthread_cond_destroy(&loader->msg_cond);
		printf("[%s]ts_queue init failed!\n", __FUNCTION__);
		free(loader);
		return NULL;
	}
	strcpy(loader->media_url, mediaUrl);

	loader->ts_pkt_queue = ts_queue;
	loader->duration = duration;
	loader->start_time = start_time;
	loader->media_file_size = -1;
	loader->current_range = 0;
	loader->is_can_seek = 0;
	loader->is_finish = 0;

	// ���Ի�ȡ�ļ���С
	_get_file_size(loader);
	if (loader->start_time > loader->duration)
	{
		loader->current_range = loader->media_file_size;
		loader->is_finish = 1;
	}

	if (loader->media_file_size > 0 && loader->duration > 0)
	{
		double wishSize = ((double)loader->start_time / (double)loader->duration) * loader->media_file_size;
		loader->current_range = floor(wishSize / 188.0) * 188;
		loader->is_can_seek = 1;
	}

	printf("create_ts_loader>> url:%s, time:%d/%d, range:%d/%d \n", loader->media_url, 
		loader->start_time, loader->duration, loader->current_range, loader->media_file_size);
	return loader;
}

// �ֶ�����
void ts_loader_load(TS_LOADER *l)
{
	printf("ts_loader_load start\n");

	_get_file_data(l);

	l->current_range += PKT_NUM_PER_TIME * 188;
	printf("ts_loader_load finish\n");
}

// �ݻ�ts_loader
void ts_loader_destroy(TS_LOADER * l)
{
	printf("ts_loader_destroy\n");
	while (!is_block_queue_empty(l->ts_pkt_queue))
	{
		BYTE_LIST * item = (BYTE_LIST *)block_queue_poll(l->ts_pkt_queue);
		byte_list_free(item);
	}
	block_queue_destory(l->ts_pkt_queue);
	pthread_mutex_destroy(&l->data_mutex);
	pthread_cond_destroy(&l->msg_cond);
	free(l);
}

// ��ȡ�ļ���С
void _get_file_size(TS_LOADER * l)
{
	if (l->media_file_size != -1)
	{
		return;
	}
	printf("_get_file_size start\n");
	_thread_param args;
	args.loaderPointer = l;
	pthread_create(&l->http_thread, NULL, _call_xhr_get_file_size, (void *)&args);
	pthread_create(&l->wait_http_thread, NULL, _wait_http_result, (void *)&args);

	// �ȴ��������
	pthread_join(l->wait_http_thread, NULL);

	// �����߳�
	pthread_detach(l->http_thread);
	printf("_get_file_size finish\n");
}

// ��ȡ�ļ�����
void _get_file_data(TS_LOADER * l)
{
	_thread_param args;
	args.loaderPointer = l;
	args.start = l->current_range;
	args.end = l->current_range + 188;

	printf("_get_file_data start, range: %d - %d \n", args.start, args.end);

	pthread_create(&l->http_thread, NULL, _call_xhr_load_file, (void *)&args);
	pthread_create(&l->wait_http_thread, NULL, _wait_http_result, (void *)&args);
	pthread_join(l->wait_http_thread, NULL);
	pthread_detach(l->http_thread);
	printf("_get_file_data finish\n");
}

// ��ȡ�ļ���С
void *_call_xhr_get_file_size(void * args)
{
	_thread_param param = *(_thread_param *)args;
	TS_LOADER *l = param.loaderPointer;
	
	// ����js����
	_js_xhr_get_file_size(l, l->media_url);
	return NULL;
}

// js���÷���
#if defined(__EMSCRIPTEN__)
EM_PORT_API(void) _js_xhr_get_file_size(TS_LOADER * l, char * url);
#else
void _js_xhr_get_file_size(TS_LOADER * l, char * url) {};
#endif

// js�ص�����
EM_PORT_API(void) _xhr_on_file_size_success(TS_LOADER * l, int size)
{
	pthread_mutex_lock(&l->data_mutex);
	l->media_file_size = size;
	printf("_xhr_on_file_size_success, size:%d\n", l->media_file_size);

	// ֪ͨ _wait_xhr_get_file_size ����ѷ���
	pthread_cond_signal(&l->msg_cond);
	pthread_mutex_unlock(&l->data_mutex);
}


// �����ļ�
void *_call_xhr_load_file(void * args) {

	_thread_param param = *(_thread_param *)args;
	TS_LOADER *l = param.loaderPointer;

	// ����js����
	_js_xhr_load_file(l, l->media_url, param.start, param.end);
	return NULL;
}

// ���ü���
#if defined(__EMSCRIPTEN__)
EM_PORT_API(void) _js_xhr_load_file(TS_LOADER * l, char * url, int start, int end);
#else
void _xhr_load_media_file(char * url, int start, int end) {};
#endif

// �ص�����
EM_PORT_API(void) _xhr_on_load_success(TS_LOADER * l, unsigned char * bytes, int len)
{
	pthread_mutex_lock(&l->data_mutex);
	printf("_xhr_on_load_success, len:%d \n", len);
	if (len == 0)
		return;

	BYTE_LIST *pkt = byte_list_create(len);
	byte_list_add_list(pkt, bytes, len);
	block_queue_push(l->ts_pkt_queue, pkt);
	pthread_cond_signal(&l->msg_cond);
	pthread_mutex_unlock(&l->data_mutex);
}

// �ȴ���ȡ���
void *_wait_http_result(void * args)
{
	_thread_param param = *(_thread_param *)args;
	TS_LOADER *l = param.loaderPointer;
	pthread_mutex_lock(&l->data_mutex);

	// �������̣��ȴ��������
	if (pthread_cond_wait(&(l->msg_cond), &(l->data_mutex)))
	{
		printf("pthread_cond_wait failed !\n");
		pthread_mutex_unlock(&(l->data_mutex));
		return NULL;
	}
	pthread_mutex_unlock(&l->data_mutex);
	return NULL;
}