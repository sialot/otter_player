#include "byte_list.h"
const int ENHANCE_SIZE = 32;
BYTE_LIST* byte_list_create(int size)
{
	BYTE_LIST *pBytelist = (BYTE_LIST *)malloc(sizeof(BYTE_LIST));
	if (pBytelist == NULL)
	{
		return NULL;
	}
	pBytelist->pBytes = (unsigned char *)malloc(sizeof(unsigned char) * size);
	if (pBytelist->pBytes == NULL)
	{
		free(pBytelist);
		return NULL;
	}
	pBytelist->free = size;
	pBytelist->len = 0;
	return pBytelist;
}

int byte_list_add(BYTE_LIST *bl, unsigned char byte)
{
	if (bl->free == 0)
	{
		unsigned char * pByteNew = realloc(bl->pBytes, sizeof(unsigned char) * (bl->len + ENHANCE_SIZE));
		if (pByteNew == NULL)
		{
			return -1;
		}
		bl->pBytes = pByteNew;
		bl->free = bl->free + ENHANCE_SIZE;
	}
	bl->pBytes[bl->len] = byte;
	bl->len = bl->len + 1;
	bl->free = bl->free - 1;
	return 0;
}

int byte_list_add_list(BYTE_LIST *bl, unsigned char *bytes, int size)
{
	if (bl->free < size)
	{
		 unsigned char * pByteNew = realloc(bl->pBytes, sizeof(unsigned char) * (bl->len + size));
		 if (pByteNew == NULL)
		 {
			 return -1;
		 }
		 bl->pBytes = pByteNew;
		 bl->free = bl->free + size;
	}

	for (int i = 0; i < size; i++)
	{
		bl->pBytes[bl->len + i] = bytes[i];
	}

	bl->len = bl->len + size;
	bl->free = bl->free - size;

	return 0;
}

int byte_list_clean(BYTE_LIST * bl)
{
	int size = bl->free + bl->len;
	bl->free = size;
	bl->len = 0;
	return 0;
}

int byte_list_free(BYTE_LIST *bl)
{
	free(bl->pBytes);
	free(bl);
	return 0;
}