
#include "otter_player.h"


int main()
{

	/*BYTE_LIST *a1 = byte_list_create(1);
	BYTE_LIST *a2 = byte_list_create(2);
	BYTE_LIST *a3 = byte_list_create(3);
	BYTE_LIST *a4 = byte_list_create(4);
	BYTE_LIST *a5 = byte_list_create(5);
	BYTE_LIST *a6 = byte_list_create(6);
	BYTE_LIST *a7 = byte_list_create(7);

	ES_QUEUE *queue = es_queue_create(5);
	int i = es_queue_add(queue, a1);
	printf("add  res = %d\n", i);
	i = es_queue_add(queue, a2);
	printf("add  res = %d\n", i);
	i = es_queue_add(queue, a3);
	printf("add  res = %d\n", i);
	i = es_queue_add(queue, a4);
	printf("add  res = %d\n", i);
	i = es_queue_add(queue, a5);
	printf("add  res = %d\n", i);
	i = es_queue_add(queue, a6);
	printf("add  res = %d\n", i);
	i = es_queue_add(queue, a7);
	printf("add  res = %d\n", i);

	for (int j = 0; j < 10; j++)
	{
		BYTE_LIST *item = es_queue_poll(queue);
		if (item != NULL)
		{
			printf("poll: %d \n", item->size);
		}
		else
		{
			printf("poll: NULL \n");
		}
	}*/

	int r = fileRead("C:\\1.ts"); 
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

	ES_QUEUE *queue = es_queue_create(1024);

	while (fread(pkt, 188, 1, tsFile) == 1) 
	{
		receive_ts_packet(pkt, queue);

		TS_PES_PACKET *pes = es_queue_poll(queue);
		if (pes != NULL)
		{
			printf("pes poll> dts:%d, pts:%d \n", pes->DTS, pes->PTS);
		}
	}
	   	 
	if (fclose(tsFile))
	{
		printf("The file was not closed\n");
	}

	return 0;
}