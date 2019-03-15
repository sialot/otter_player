
#include "byte_list.h"
const int ENHANCE_SIZE = 4 * 1024;

BYTE_LIST* byte_list_create(int size)
{
	BYTE_LIST *pBytelist = (BYTE_LIST *)malloc(sizeof(BYTE_LIST));
	if (pBytelist == NULL)
	{
		return NULL;
	}
	pBytelist->size = size;
	pBytelist->used_len = 0;
	pBytelist->finish_len = 0; // ÎÞÏÞÖÆ
	pBytelist->pBytes = NULL;

	unsigned char *pBytes = (unsigned char *)malloc(sizeof(unsigned char) * size);
	if (pBytes == NULL)
	{
		free(pBytelist);
		return NULL;
	}
	pBytelist->pBytes = pBytes;

	return pBytelist;
}

int byte_list_add_list(BYTE_LIST *bl, unsigned char *bytes, int size)
{
	if (bl->size < (size + bl->used_len))
	{
		int new_size = bl->size + ENHANCE_SIZE + size;
		unsigned char *pByteNew = (unsigned char *)realloc(bl->pBytes, new_size);
		if (pByteNew == NULL)
		{
			return -1;
		}
		bl->pBytes = pByteNew;
		bl->size = new_size;
	}

	for (int i = 0; i < size; i++)
	{
		bl->pBytes[bl->used_len + i] = bytes[i];
	}

	bl->used_len = bl->used_len + size;
	return 0;
}

int byte_list_clean(BYTE_LIST * bl)
{
	bl->used_len = 0;
	bl->finish_len = 0;
	unsigned char * pByteNew = (unsigned char *)realloc(bl->pBytes, sizeof(unsigned char) * 256);
	if (pByteNew == NULL)
	{
		return -1;
	}

	bl->pBytes = pByteNew;
	bl->size = 256;
	return 0;
}

int byte_list_free(BYTE_LIST *bl)
{
	if (bl == NULL)
		return;
	if (bl->pBytes != NULL)
	{
		free(bl->pBytes);
	}
	free(bl);
	return 0;
}

void byte_list_print(BYTE_LIST *bl)
{
	for (int i = 0; i < bl->used_len; i++)
	{
		printf("BYTE_LIST [%d] : %d(%#X) \n", i, bl->pBytes[i], bl->pBytes[i]);
	}
}

int is_byte_list_finish(BYTE_LIST * bl)
{
	if (bl->finish_len != 0)
	{
		return bl->finish_len <= bl->used_len;
	}

	return 0;
}
