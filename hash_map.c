#include "hash_map.h"

HASH_MAP * hash_map_create()
{
	HASH_MAP *map = malloc(sizeof(HASH_MAP));
	map->size = 0;
	map->array_len = 0;
	map->table = NULL;
	return map;
}

int hash_map_put(HASH_MAP *map, int key, void *value)
{
	int keyArrayIdx = get_table_index(map, key);
	if (keyArrayIdx != -1)
	{
		HASH_MAP_ENTRY *node = map->table[keyArrayIdx];

		do
		{
			if (node->key == key)
			{
				// Ìæ»»Êý¾Ý
				if (node->value != value)
				{
					free(node->value);
					node->value = value;
					return 0;
				}
			}
			else
			{
				if (node->next == NULL)
				{
					HASH_MAP_ENTRY *entry = entry_create(key, value);
					if (entry == NULL)
						return -1;
					node->next = entry;
					map->size++;
					return 0;
				}
			}

			node = node->next;
		} while (node != NULL);
	}
	else
	{
		HASH_MAP_ENTRY *entry = entry_create(key, value);
		if (entry == NULL)
			return -1;
		key_array_add(map, entry);
		map->size++;
	}

	return 0;
}

void * hash_map_get(HASH_MAP * map, int key)
{
	int hashcode = hash(key);
	int keyArrayIdx = get_table_index(map, key);
	if (keyArrayIdx != -1)
	{
		HASH_MAP_ENTRY *node = map->table[keyArrayIdx];

		do
		{
			if (node->key == key)
				return node->value;
			node = node->next;
		} while (node != NULL);
	}
	return NULL;
}

static int hash(int key)
{
	return (key * 2654435769) >> 28;
}

static int get_table_index(HASH_MAP *map, int key)
{
	int keyArrayIdx = -1;
	if (map->table != NULL)
	{
		for (int i = 0; i < map->array_len; i++)
		{
			if (map->table[i]->hash == hash(key))
			{
				keyArrayIdx = i;
				break;
			}
		}
	}
	return keyArrayIdx;
}

static HASH_MAP_ENTRY * entry_create(int key, void *value)
{
	HASH_MAP_ENTRY *entry = (HASH_MAP_ENTRY *)malloc(sizeof(HASH_MAP_ENTRY));
	if (entry == NULL)
		return NULL;
	entry->hash = hash(key);
	entry->key = key;
	entry->next = NULL;
	entry->value = value;
	return entry;
}

static int key_array_add(HASH_MAP * map, HASH_MAP_ENTRY *entry)
{
	int idx = map->array_len;
	if (map->table == NULL)
	{
		HASH_MAP_ENTRY **tableNew = (HASH_MAP_ENTRY **)malloc(sizeof(HASH_MAP_ENTRY *) * (idx + 1));
		if (tableNew == NULL)
			return -1;
		map->table = tableNew;
	}
	else
	{
		HASH_MAP_ENTRY **tableNew = (HASH_MAP_ENTRY **)realloc(map->table, sizeof(HASH_MAP_ENTRY *)*(idx + 1));
		if (tableNew == NULL)
			return -1;
		map->table = tableNew;
	}

	map->table[idx] = entry;
	map->array_len++;
	return 0;
}
void hash_map_free(HASH_MAP * map)
{
	if (map == NULL)
		return;

	free(map->table);
	free(map);
}
/*
void hash_map_free(HASH_MAP *map)
{
	if (map == NULL)
		return;

	for (int i = 0; i < map->array_len; i++)
	{
		HASH_MAP_ENTRY *entry = map->table[i];
		hash_map_entry_free(entry);
	}

	free(map->table);
	free(map);
}

static void hash_map_entry_free(HASH_MAP_ENTRY *entry)
{
	HASH_MAP_ENTRY *cur;
	do
	{
		cur = entry;
		entry = entry->next;
		free(entry->value);
		free(cur);
	} while (entry != NULL);
}
*/