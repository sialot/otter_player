
#include <stdio.h>
#include <stdlib.h>

#include <crtdbg.h>  

//#include <libavcodec/avcodec.h>
#include "ts_demuxer.h"
//#include "block_queue.h"
#include "player.h"

int fileRead(char *filePath);
long long my_atoll(char *instr);
#ifdef _DEBUG  
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif  

void EnableMemLeakCheck()
{
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpFlag);
}

int main()
{
	EnableMemLeakCheck();
	//_CrtSetBreakAlloc(136393);
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

	/*
	OTTER_PLAYER *p = create_player(1280,720);
	char media_url[128] = "http://10.0.9.229/pub/1.ts";
	set_media(p, media_url, 6519);

	play(p);

	play_by_time(p, 3000);
	*/

	//fileRead("C:\\1.ts");

	long long media_file_size = 34359738368000000;
	printf("%lld\n", media_file_size);
	long long current_range = 0;
	int duration = 7200;  
	int start_time = 72;

	long long wishSize = ((double)start_time / (double)duration) * media_file_size;
	printf("%lld\n", wishSize);
	current_range = (long long)floor(wishSize / 188.0) * 188;
	printf("%lld\n", current_range);

	char test[30];
	sprintf(test,"%lld", current_range);

	printf(">> %s \n ", test);

	long long t = my_atoll(test);

	printf(">>>>> %lld \n ", t);


	system("pause");
	return 0;
}
long long my_atoll(char *instr)
{
	long long retval;
	int i;

	retval = 0;
	for (; *instr; instr++) {
		retval = 10 * retval + (*instr - '0');
	}
	return retval;
}
int fileRead(char *filePath) {
	printf("%s\n", filePath);
	FILE *tsFile = NULL;

	if ((tsFile = fopen(filePath, "rb")) == NULL)
	{
		printf("file not exist!\n");
	}

	size_t rs = 0;
	TS_DEMUXER *d = ts_demuxer_create(512);
	do {
		unsigned char *pkt = malloc(sizeof(unsigned char) * 188);
		rs = fread(pkt, 188, 1, tsFile);
		if(rs > 0)
			demux_ts_pkt(d, pkt);
		free(pkt);
	} while (rs != 0);

	ts_demuxer_destroy(d);

	if (fclose(tsFile))
	{
		printf("The file was not closed\n");
	}

	return 0;
}