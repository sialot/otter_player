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

// 每次http请求加载的ts包数 约 100kb
const int PKT_NUM_PER_TIME = 500;

// 包缓存数量 约 1mb
const int PKT_BUFFER_COUNT = 5000;

// 创建加载器
TS_LOADER * ts_loader_create(char * mediaUrl, int duration, int start_time)
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

	TS_LOADER *loader = malloc(sizeof(TS_LOADER));
	if (loader == NULL)
	{
		printf("create_ts_loader init failed.can't get mem! \n");
		return NULL;
	}

	loader->media_file_size = -1;
	loader->current_range = 0;
	loader->duration = duration;
	loader->start_time = start_time;
	strcpy(loader->media_url, mediaUrl);
	loader->is_can_seek = 0;
	loader->is_finish = 0;
	loader->ts_pkt_queue = NULL;

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

	BLOCK_QUEUE *ts_queue = block_queue_create(PKT_BUFFER_COUNT);
	if (ts_queue == NULL)
	{
		pthread_mutex_destroy(&loader->data_mutex);
		pthread_cond_destroy(&loader->msg_cond);
		printf("[%s]ts_queue init failed!\n", __FUNCTION__);
		free(loader);
		return NULL;
	}
	loader->ts_pkt_queue = ts_queue;

	// 尝试获取文件大小
	_get_file_size(loader);
	if (loader->start_time > loader->duration)
	{
		loader->current_range = loader->media_file_size;
		printf("loader->start_time > loader->duration.is_finish = 1\n");
		loader->is_finish = 1;
	}

	if (loader->media_file_size > 0 && loader->duration > 0)
	{
		long long wishSize = ((double)loader->start_time / (double)loader->duration) * loader->media_file_size;
		loader->current_range = (long long)floor(wishSize / 188.0) * 188; 
		loader->is_can_seek = 1;
	}

	printf("create_ts_loader>> url:%s, time:%d/%d, range:%lld/%lld \n", loader->media_url,
		loader->start_time, loader->duration, loader->current_range, loader->media_file_size);
	return loader;
}

// 部分加载数据
void ts_loader_range_load(TS_LOADER *l)
{
	if (l->is_finish)
	{
		return;
	}

	_get_file_data(l);

	if (l->media_file_size != 0 && l->current_range >= l->media_file_size)
	{
		printf("l->current_range >= l->media_file_size .is_finish = 1\n");
		l->is_finish = 1;
	}
}

// 拉取 ts 包
BYTE_LIST * poll_ts_pkt(TS_LOADER *l)
{
	BYTE_LIST *tsPkt = block_queue_poll(l->ts_pkt_queue);
	return tsPkt;
}

// 队列是否为空
int is_ts_queue_empty(TS_LOADER *l)
{
	return is_block_queue_empty(l->ts_pkt_queue);
}

// 摧毁ts_loader
void ts_loader_destroy(TS_LOADER * l)
{
	printf("ts_loader_destroy\n");

	if (l == NULL)
		return;

	ts_block_queue_destroy(l->ts_pkt_queue);
	pthread_mutex_destroy(&l->data_mutex);
	pthread_cond_destroy(&l->msg_cond);
	free(l);
}

// 获取文件大小
void _get_file_size(TS_LOADER * l)
{
	if (l->media_file_size != -1)
	{
		return;
	}
	_thread_param args;
	args.loaderPointer = l;

	pthread_create(&l->wait_http_thread, NULL, _wait_http_result, (void *)&args);
	pthread_create(&l->http_thread, NULL, _call_xhr_get_file_size, (void *)&args);

	// 等待结果返回
	pthread_join(l->http_thread, NULL);
	pthread_join(l->wait_http_thread, NULL);
}

// 获取文件数据
void _get_file_data(TS_LOADER * l)
{
	_thread_param args;
	args.loaderPointer = l;
	args.start = l->current_range;
	args.end = l->current_range + PKT_NUM_PER_TIME * 188 - 1;
	l->current_range += PKT_NUM_PER_TIME * 188;

	printf("_get_file_data start, range: %lld - %lld / %lld \n", args.start, args.end, l->media_file_size);

	pthread_create(&l->wait_http_thread, NULL, _wait_http_result, (void *)&args);
	pthread_create(&l->http_thread, NULL, _call_xhr_load_file, (void *)&args);
	pthread_join(l->http_thread, NULL);
	pthread_join(l->wait_http_thread, NULL);
}

// 获取文件大小
void *_call_xhr_get_file_size(void * args)
{
	_thread_param param = *(_thread_param *)args;
	TS_LOADER *l = param.loaderPointer;

	// 调用js函数
	_js_xhr_get_file_size(l, l->media_url);
	pthread_exit(NULL);
	return NULL;
}

// js调用方法
#if defined(__EMSCRIPTEN__)
EM_PORT_API(void) _js_xhr_get_file_size(TS_LOADER * l, char * url);
#else
void _js_xhr_get_file_size(TS_LOADER * l, char * url) {};
#endif

// js回调方法
EM_PORT_API(void) _xhr_on_file_size_success(TS_LOADER * l, char * size)
{
	pthread_mutex_lock(&l->data_mutex);
	l->media_file_size = _char_to_longlong(size);
	printf("_xhr_on_file_size_success, size:%lld\n", l->media_file_size);

	// 通知 _wait_xhr_get_file_size 结果已返回
	pthread_cond_signal(&l->msg_cond);
	pthread_mutex_unlock(&l->data_mutex);
}

// 加载文件
void *_call_xhr_load_file(void * args)
{
	_thread_param param = *(_thread_param *)args;
	TS_LOADER *l = param.loaderPointer;

	char start[30];
	sprintf(start, "%lld", param.start);
	char end[30];
	sprintf(end, "%lld", param.end);

	// 调用js函数
	_js_xhr_load_file(l, l->media_url, start, end);
	return NULL;
}

// 调用加载
#if defined(__EMSCRIPTEN__)
EM_PORT_API(void) _js_xhr_load_file(TS_LOADER * l, char * url, char * start, char * end);
#else
void _js_xhr_load_file(TS_LOADER * l, char * url, char * start, char * end) {};
#endif

// 回调方法
EM_PORT_API(void) _xhr_on_load_success(TS_LOADER * l, unsigned char * bytes, int len)
{
	pthread_mutex_lock(&l->data_mutex);
	printf("_xhr_on_load_success, len:%d \n", len);
	if (len == 0)
	{
		printf("len=0.is_finish = 1\n");
		l->is_finish = 1;
		return;
	}

	for (int i = 0; i < len / 188; i++)
	{
		unsigned char * data = bytes + i * 188;
		BYTE_LIST *pkt = byte_list_create(188);
		byte_list_add_list(pkt, data, 188);
		block_queue_push(l->ts_pkt_queue, pkt);
	}
	pthread_cond_signal(&l->msg_cond);
	pthread_mutex_unlock(&l->data_mutex);
}

// 等待获取结果
void *_wait_http_result(void * args)
{
	_thread_param param = *(_thread_param *)args;
	TS_LOADER *l = param.loaderPointer;
	pthread_mutex_lock(&l->data_mutex);

	// 阻塞进程，等待结果返回
	if (pthread_cond_wait(&(l->msg_cond), &(l->data_mutex)))
	{
		printf("pthread_cond_wait failed !\n");
		pthread_mutex_unlock(&(l->data_mutex));
		return NULL;
	}

	printf("pthread_cond_wait over !\n");
	pthread_mutex_unlock(&l->data_mutex);
	return NULL;
}

// 字符转数字
long long _char_to_longlong(char * instr)
{
	long long retval;
	int i;

	retval = 0;
	for (; *instr; instr++) {
		retval = 10 * retval + (*instr - '0');
	}
	return retval;
}