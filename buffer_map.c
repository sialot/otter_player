#include "buffer_map.h"

BUFFER_MAP * buffer_map_create()
{
	BUFFER_MAP *map = malloc(sizeof(BUFFER_MAP));
	map->size = 0;
	map->array_len = 0;
	map->table = NULL;
	return map;
}

int buffer_map_put(BUFFER_MAP *map, int key, BYTE_LIST *value)
{
	int keyArrayIdx = get_table_index(map, key);
	if (keyArrayIdx != -1)
	{
		BUFFER_MAP_ENTRY *node = map->table[keyArrayIdx];

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
					BUFFER_MAP_ENTRY *entry = entry_create(key, value);
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
		BUFFER_MAP_ENTRY *entry = entry_create(key, value);
		if (entry == NULL)
			return -1;
		key_array_add(map, entry);
		map->size++;
	}

	return 0;
}

BYTE_LIST * buffer_map_get(BUFFER_MAP * map, int key)
{
	int hashcode = hash(key);
	int keyArrayIdx = get_table_index(map, key);
	if (keyArrayIdx != -1)
	{
		BUFFER_MAP_ENTRY *node = map->table[keyArrayIdx];

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

static int get_table_index(BUFFER_MAP *map, int key)
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

static BUFFER_MAP_ENTRY * entry_create(int key, BYTE_LIST *value)
{
	BUFFER_MAP_ENTRY *entry = (BUFFER_MAP_ENTRY *)malloc(sizeof(BUFFER_MAP_ENTRY));
	if (entry == NULL)
		return NULL;
	entry->hash = hash(key);
	entry->key = key;
	entry->next = NULL;
	entry->value = value;
	return entry;
}

static int key_array_add(BUFFER_MAP * map, BUFFER_MAP_ENTRY *entry)
{
	int idx = map->array_len;
	if (map->table == NULL)
	{
		BUFFER_MAP_ENTRY **tableNew = (BUFFER_MAP_ENTRY **)malloc(sizeof(BUFFER_MAP_ENTRY *) * (idx + 1));
		if (tableNew == NULL)
			return -1;
		map->table = tableNew;
	}
	else
	{
		BUFFER_MAP_ENTRY **tableNew = (BUFFER_MAP_ENTRY **)realloc(map->table, sizeof(BUFFER_MAP_ENTRY *)*(idx + 1));
		if (tableNew == NULL)
			return -1;
		map->table = tableNew;
	}

	map->table[idx] = entry;
	map->array_len++;
	return 0;
}

void buffer_map_free(BUFFER_MAP *map)
{
	if (map == NULL)
		return;

	for (int i = 0; i < map->array_len; i++)
	{
		BUFFER_MAP_ENTRY *entry = map->table[i];
		buffer_map_entry_free(entry);
	}

	free(map->table);
	free(map);
}

static void buffer_map_entry_free(BUFFER_MAP_ENTRY *entry)
{
	BUFFER_MAP_ENTRY *cur;
	do
	{
		cur = entry;
		entry = entry->next;
		byte_list_free(cur->value);
		free(cur);
	} while (entry != NULL);
}