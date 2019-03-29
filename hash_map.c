#include "hash_map.h"

HASH_MAP * hash_map_create()
{
	HASH_MAP *map = malloc(sizeof(HASH_MAP));
	map->size = 0;
	map->array_len = 0;
	map->table = NULL;
	return map;
}

int hash_map_put(HASH_MAP *map, int key, BYTE_LIST *value)
{
	int keyArrayIdx = get_table_index(map, key);
	if (keyArrayIdx != -1)
	{
		HASH_MAP_ENTRY *node = map->table[keyArrayIdx];

		do
		{
			if (node->key == key)
			{
				// �滻����
				if (node->value != value)
				{
					byte_list_destroy(node->value);
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

BYTE_LIST * hash_map_get(HASH_MAP * map, int key)
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

int hash_map_destory_and_remove(HASH_MAP * map, int key)
{
	BYTE_LIST *item = hash_map_get(map, key);
	if (item == NULL)
	{
		return 0;
	}
	hash_map_put(map, key, NULL);
	return 0;
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

static HASH_MAP_ENTRY * entry_create(int key, BYTE_LIST *value)
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
void hash_map_destroy(HASH_MAP * map)
{
	if (map == NULL)
		return;

	for (int i = 0; i < map->array_len; i++)
	{
		HASH_MAP_ENTRY *cur;
		HASH_MAP_ENTRY *entry = map->table[i];
		do
		{
			cur = entry;
			entry = entry->next;
			byte_list_destroy(cur->value);
			free(cur);
		} while (entry != NULL);
	}

	free(map->table);
	free(map);
}
