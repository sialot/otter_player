#pragma once
#if defined(__EMSCRIPTEN__)
#include <pthread.h>
#else
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#endif

#include<math.h>
#include <string.h>
#include "byte_list.h"
#include "block_queue.h"

//#pragma comment(lib, "pthreadVC2.lib")
typedef struct TS_LOADER
{
	int media_file_size; // ý���С���ֽڣ�
	char media_url[1024]; // ý���ַ
	int duration; // �ܴ�С
	int current_range; // ��ǰ���ص�
	int start_time;// ������ʼʱ��
	int is_can_seek;
	int is_finish;
	pthread_t http_thread;
	pthread_t wait_http_thread;
	pthread_mutex_t data_mutex;
	pthread_cond_t msg_cond;
	BLOCK_QUEUE *ts_pkt_queue;
} TS_LOADER;

typedef struct _thread_param {
	TS_LOADER * loaderPointer;
	int start;
	int end;
} _thread_param;

TS_LOADER * create_ts_loader(char *pMediaUrl, int duration, int start_time, int buffer_count);
void ts_loader_load(TS_LOADER *l);
void ts_loader_destroy(TS_LOADER *l);

void _get_file_size(TS_LOADER * l);
void _get_file_data(TS_LOADER * l);

// ��ȡ�ļ���С
void *_call_xhr_get_file_size(void * args);
void _js_xhr_get_file_size(TS_LOADER * l, char * url);
void _xhr_on_file_size_success(TS_LOADER *l, int size);

// �����ļ�
void *_call_xhr_load_file(void * args);
void _js_xhr_load_file(TS_LOADER * l, char * url, int start, int end);
void _xhr_on_load_success(TS_LOADER * l, unsigned char * bytes, int len);

// �ȴ�http���ؽ��
void *_wait_http_result(void * args);