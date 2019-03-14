#include "ts_loader.h"

TS_LOADER * create_ts_loader(char * mediaUrl, int duration, BLOCK_QUEUE *ts_pkt_queue)
{
	TS_LOADER *loader = malloc(sizeof(TS_LOADER));

	if (loader == NULL)
	{
		printf("create_ts_loader failed.can't init mem!\n");
		return NULL;
	}

	loader->media_file_size = -1;
	loader->duration = duration;
	strcpy(loader->media_url, mediaUrl);
	printf("create_ts_loader>> url:%s\n", loader->media_url);
	return loader;
}

int ts_loader_seek_and_trans(TS_LOADER_PARAM *p)
{
	printf("ts_load_thread start!\n");

	TS_LOADER *l = p->l;

	if (p->pCurrentPlayTime >= l->duration)
	{
		printf("ts_loader_seek_and_trans error>> pCurrentPlayTime >= l->duration !\n");
		return -1;
	}
	if (p->pCurrentPlayTime < 0)
	{
		printf("ts_loader_seek_and_trans error>> pCurrentPlayTime < 0 !\n");
		return -1;
	}

	if (l->media_file_size == -1)
	{
		_get_file_size(l);
	}

	double wishSize = ((double)p->pCurrentPlayTime / (double)l->duration) * l->media_file_size;
	int rangeStart = floor(wishSize / 188.0) * 188;
	printf("ts_loader_seek_and_trans>> seek start: %d/%d \n", rangeStart, l->media_file_size);

	return 0;
}

int ts_loader_destroy(TS_LOADER * l)
{
	printf("ts_loader_destroy\n");
	// TODO
	free(l);
	return 0;
}

static void _get_file_size(TS_LOADER * l)
{







	l->media_file_size = 2037353368;
}
