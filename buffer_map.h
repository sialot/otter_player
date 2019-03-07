#pragma once
#include "byte_list.h"

typedef struct BUFFER_MAP_ENTRY
{
	struct BUFFER_MAP_ENTRY *next;
	int hash;
	int key;
	BYTE_LIST *value;
}BUFFER_MAP_ENTRY;

typedef struct BUFFER_MAP
{
	BUFFER_MAP_ENTRY **table;
	int array_len;
	int size;
}BUFFER_MAP;

BUFFER_MAP * buffer_map_create();

int buffer_map_put(BUFFER_MAP *map, int key, BYTE_LIST *value);

BYTE_LIST * buffer_map_get(BUFFER_MAP *map, int key);

static int hash(int key);

static int get_table_index(BUFFER_MAP *map, int key);

static BUFFER_MAP_ENTRY * entry_create(int key, BYTE_LIST *value);

static int key_array_add(BUFFER_MAP * map, BUFFER_MAP_ENTRY *entry);

void buffer_map_free(BUFFER_MAP *map);

static void buffer_map_entry_free(BUFFER_MAP_ENTRY *entry);