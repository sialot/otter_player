#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// �ֽ��б�
typedef struct BYTE_LIST
{
	int  used_len; // ��ʹ��
	int  size;     // �ܴ�С
	int  finish_len; // ����ʹ������С
	unsigned char *pBytes;
} BYTE_LIST;

BYTE_LIST*  byte_list_create(int size);
int byte_list_add_list(BYTE_LIST *bl, unsigned char *bytes, int size);
int is_byte_list_finish(BYTE_LIST *bl);
int byte_list_clean(BYTE_LIST *bl);
int byte_list_destroy(BYTE_LIST *bl);