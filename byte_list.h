#pragma once
#include <stdlib.h>

// 字节列表
typedef struct BYTE_LIST
{
	int len; // 已使用
	int free; // 未使用
	unsigned char *pBytes;
} BYTE_LIST;

BYTE_LIST*  byte_list_create(int size);
int byte_list_add(BYTE_LIST *bl, unsigned char byte);
int byte_list_add_list(BYTE_LIST *bl, unsigned char *bytes, int size);
int byte_list_clean(BYTE_LIST *bl);
int byte_list_free(BYTE_LIST *bl);