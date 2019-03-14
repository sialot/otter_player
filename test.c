
#include "test.h"


int main()
{
	/*
	BYTE_LIST *a1 = byte_list_create(1);
	BYTE_LIST *a2 = byte_list_create(2);
	BYTE_LIST *a3 = byte_list_create(3);
	BYTE_LIST *a4 = byte_list_create(4);
	BYTE_LIST *a5 = byte_list_create(5);
	BYTE_LIST *a6 = byte_list_create(6);
	BYTE_LIST *a7 = byte_list_create(7);

	BLOCK_QUEUE *queue = block_queue_create(5);
	int i = block_queue_push(queue, a1);
	printf("add  res = %d\n", i);
	i = block_queue_push(queue, a2);
	printf("add  res = %d\n", i);
	i = block_queue_push(queue, a3);
	printf("add  res = %d\n", i);
	i = block_queue_push(queue, a4);
	printf("add  res = %d\n", i);
	i = block_queue_push(queue, a5);
	printf("add  res = %d\n", i);
	i = block_queue_push(queue, a6);
	printf("add  res = %d\n", i);
	i = block_queue_push(queue, a7);
	printf("add  res = %d\n", i);

	for (int j = 0; j < 10; j++)
	{
		BYTE_LIST *item = block_queue_poll(queue);
		if (item != NULL)
		{
			printf("poll: %d \n", item->size);
		}
		else
		{
			printf("poll: NULL \n");
		}
	}

	int r = fileRead("C:\\1.ts"); */


	OTTER_PLAYER *p = create_player(1280,720);
	char media_url[128] = "http://10.0.9.229/pub/1.ts";
	set_media(p, media_url, 6519);

	play(p);

	play_by_time(p, 3000);

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
		//receive_ts_packet(pkt);
	}
	   	 
	if (fclose(tsFile))
	{
		printf("The file was not closed\n");
	}

	return 0;
}