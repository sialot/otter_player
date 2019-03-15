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

// ������
typedef struct TS_LOADER
{
	int media_file_size;            // ý���С���ֽڣ�
	char media_url[1024];           // ý���ַ
	int duration;                   // �ܴ�С
	int current_range;              // ��ǰ���ص�
	int start_time;                 // ������ʼʱ��
	int is_can_seek;                // �Ƿ���Բ��ҽ���
	int is_finish;                  // �Ƿ���ؽ���
	pthread_t http_thread;          // http�����߳�
	pthread_t wait_http_thread;     // http����ȴ��߳�
	pthread_mutex_t data_mutex;     // �߳���
	pthread_cond_t msg_cond;        // �̵߳ȴ���������
	BLOCK_QUEUE *ts_pkt_queue;      // ts���������
} TS_LOADER;

// ���̲߳���
typedef struct _thread_param {
	TS_LOADER * loaderPointer;      // ������ָ��
	int start;                      // http����range��ʼλ��  
	int end;                        // http����range����λ��
} _thread_param;

// ����������
TS_LOADER * ts_loader_create(char *pMediaUrl, int duration, int start_time, int buffer_count);

// ��Χ���أ����ּ��أ�ÿ�ε��ð�PKT_NUM_PER_TIME�����أ�
void ts_loader_range_load(TS_LOADER *l);

// �ݻټ�����
void ts_loader_destroy(TS_LOADER *l);

// ��ȡ�ļ���С
void _get_file_size(TS_LOADER * l);

// ��ȡ�ļ�����
void _get_file_data(TS_LOADER * l);

// �̺߳�������ȡ�ļ���С
void *_call_xhr_get_file_size(void * args);

// js����������xmlhttprequest
void _js_xhr_get_file_size(TS_LOADER * l, char * url);

// js�ص�����ȡ�ļ���С
void _xhr_on_file_size_success(TS_LOADER *l, int size);

// �̺߳����������ļ�
void *_call_xhr_load_file(void * args);

// js����������xmlhttprequest
void _js_xhr_load_file(TS_LOADER * l, char * url, int start, int end);

// js�ص�����ȡ�ļ���С
void _xhr_on_load_success(TS_LOADER * l, unsigned char * bytes, int len);

// �ȴ�http���ؽ��
void *_wait_http_result(void * args);