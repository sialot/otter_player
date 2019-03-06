
#include "otter_player.h"
#include "buffer_map.h"
#include "byte_list.h"
int main()
{
	/*
	BUFFER_MAP *map = buffer_map_create();

	BYTE_LIST *value = byte_list_create(3);
	byte_list_add(value, 1);
	byte_list_add(value, 2);
	byte_list_add(value, 3);
	buffer_map_put(map, 1, value);

	value = byte_list_create(4);
	byte_list_add(value, 4);
	byte_list_add(value, 5);
	byte_list_add(value, 6);
	byte_list_add(value, 7);
	buffer_map_put(map, 2, value);

	value = byte_list_create(2);
	byte_list_add(value, 11);
	byte_list_add(value, 22);
	buffer_map_put(map, 1, value);


	value = byte_list_create(2);
	byte_list_add(value, 111);
	byte_list_add(value, 222);
	buffer_map_put(map, 11, value);

	value = byte_list_create(2);
	byte_list_add(value, 12);
	byte_list_add(value, 13);
	buffer_map_put(map, 21, value);
	byte_list_print(buffer_map_get(map, 1));
	byte_list_print(buffer_map_get(map, 2));

	byte_list_print(buffer_map_get(map,11));
	byte_list_print(buffer_map_get(map,21));*/
	
	//buffer_map_free(map);
	fileRead("C:\\1.ts"); 
	system("pause");
	return 0;
}

int fileRead(char *filePath) {
	printf("%s\n", filePath);
	FILE *tsFile = NULL;

	if ((tsFile = fopen(filePath, "rb")) == NULL)
	{
		printf("file not exist!\n");
	}

	unsigned char pkt[188];

	while (fread(pkt, 188, 1, tsFile) == 1) 
	{
		receive_ts_packet(pkt);
	}
	   	 
	if (fclose(tsFile))
	{
		printf("The file was not closed\n");
	}

	return 0;
}