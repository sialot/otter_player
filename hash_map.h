#pragma once
#include<stdlib.h>
#include "byte_list.h"

typedef struct HASH_MAP_ENTRY
{
	struct HASH_MAP_ENTRY *next;
	int hash;
	int key;
	BYTE_LIST *value;
}HASH_MAP_ENTRY;

typedef struct HASH_MAP
{
	HASH_MAP_ENTRY **table;
	int array_len;
	int size;
}HASH_MAP;

HASH_MAP * hash_map_create();

int hash_map_put(HASH_MAP *map, int key, BYTE_LIST *value);

BYTE_LIST * hash_map_get(HASH_MAP *map, int key);

int hash_map_destory_and_remove(HASH_MAP *map, int key);

static int hash(int key);

static int get_table_index(HASH_MAP *map, int key);

static HASH_MAP_ENTRY * entry_create(int key, BYTE_LIST *value);

static int key_array_add(HASH_MAP * map, HASH_MAP_ENTRY *entry);

void hash_map_destroy(HASH_MAP *map);
