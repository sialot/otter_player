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

// 加载器
typedef struct TS_LOADER
{
	int media_file_size;            // 媒体大小（字节）
	char media_url[1024];           // 媒体地址
	int duration;                   // 总大小
	int current_range;              // 当前加载点
	int start_time;                 // 加载起始时间
	int is_can_seek;                // 是否可以查找进度
	int is_finish;                  // 是否加载结束
	pthread_t http_thread;          // http请求线程
	pthread_t wait_http_thread;     // http结果等待线程
	pthread_mutex_t data_mutex;     // 线程锁
	pthread_cond_t msg_cond;        // 线程等待条件变量
	BLOCK_QUEUE *ts_pkt_queue;      // ts包缓存队列
} TS_LOADER;

// 多线程参数
typedef struct _thread_param {
	TS_LOADER * loaderPointer;      // 加载器指针
	int start;                      // http请求range起始位置  
	int end;                        // http请求range结束位置
} _thread_param;

// 创建加载器
TS_LOADER * ts_loader_create(char *pMediaUrl, int duration, int start_time, int buffer_count);

// 范围加载（部分加载，每次调用按PKT_NUM_PER_TIME向后加载）
void ts_loader_range_load(TS_LOADER *l);

// 摧毁加载器
void ts_loader_destroy(TS_LOADER *l);

// 获取文件大小
void _get_file_size(TS_LOADER * l);

// 获取文件数据
void _get_file_data(TS_LOADER * l);

// 线程函数，获取文件大小
void *_call_xhr_get_file_size(void * args);

// js方法，调用xmlhttprequest
void _js_xhr_get_file_size(TS_LOADER * l, char * url);

// js回调，获取文件大小
void _xhr_on_file_size_success(TS_LOADER *l, int size);

// 线程函数，加载文件
void *_call_xhr_load_file(void * args);

// js方法，调用xmlhttprequest
void _js_xhr_load_file(TS_LOADER * l, char * url, int start, int end);

// js回调，获取文件大小
void _xhr_on_load_success(TS_LOADER * l, unsigned char * bytes, int len);

// 等待http返回结果
void *_wait_http_result(void * args);