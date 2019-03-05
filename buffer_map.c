#include "buffer_map.h"
#include "byte_list.h"
typedef struct BUFFER_MAP_ENTRY
{
	Entry *next;
	int hash;
	int key;
	BYTE_LIST *value;
}BUFFER_MAP_ENTRY;

typedef struct BUFFER_MAP
{
	BUFFER_MAP_ENTRY *table;
	int size;
}BUFFER_MAP;