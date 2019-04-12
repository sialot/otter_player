#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// 字节列表
typedef struct BYTE_LIST
{
	int  used_len; // 已使用
	int  size;     // 总大小
	int  finish_len; // 限制使用最大大小
	unsigned char *pBytes;
} BYTE_LIST;

BYTE_LIST*  byte_list_create(int size);
int byte_list_add_list(BYTE_LIST *bl, unsigned char *bytes, int size);
int is_byte_list_finish(BYTE_LIST *bl);
int byte_list_clean(BYTE_LIST *bl);
int byte_list_destroy(BYTE_LIST *bl);