#pragma once
#include <stdlib.h>

// �ֽ��б�
typedef struct BYTE_LIST
{
	int len; // ��ʹ��
	int free; // δʹ��
	unsigned char *pBytes;
} BYTE_LIST;

BYTE_LIST*  byte_list_create(int size);
int byte_list_add(BYTE_LIST *bl, unsigned char byte);
int byte_list_add_list(BYTE_LIST *bl, unsigned char *bytes, int size);
int byte_list_clean(BYTE_LIST *bl);
int byte_list_free(BYTE_LIST *bl);