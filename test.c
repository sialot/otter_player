
#include <stdio.h>
#include <stdlib.h>

#include <crtdbg.h>  

//#include <libavcodec/avcodec.h>
#include "ts_demuxer.h"
#include "decoder_master.h"
//#include "block_queue.h"
#include "player.h"

int fileRead(char *filePath, char *outPath);
long long my_atoll(char *instr);
void MySaveBmp(const char *filename, unsigned char *rgbbuf, int width, int height);
#ifdef _DEBUG  
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#endif  

void EnableMemLeakCheck()
{
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpFlag);
}
typedef struct         BITMAPFILEHEADER              /**** BMP file header structure ****/
{
	unsigned int   bfSize;           /* Size of file */
	unsigned short bfReserved1;      /* Reserved */
	unsigned short bfReserved2;      /* ... */
	unsigned int   bfOffBits;        /* Offset to bitmap data */
} MyBITMAPFILEHEADER;
typedef struct      MyBITMAPINFOHEADER                 /**** BMP file info structure ****/
{
	unsigned int   biSize;           /* Size of info header */
	int            biWidth;          /* Width of image */
	int            biHeight;         /* Height of image */
	unsigned short biPlanes;         /* Number of color planes */
	unsigned short biBitCount;       /* Number of bits per pixel */
	unsigned int   biCompression;    /* Type of compression to use */
	unsigned int   biSizeImage;      /* Size of image data */
	int            biXPelsPerMeter;  /* X pixels per meter */
	int            biYPelsPerMeter;  /* Y pixels per meter */
	unsigned int   biClrUsed;        /* Number of colors used */
	unsigned int   biClrImportant;   /* Number of important colors */
} MyBITMAPINFOHEADER;
typedef struct tagRGBQUAD {
	unsigned char rgbBlue;//蓝色的亮度（值范围为0-255)  
	unsigned char rgbGreen;//绿色的亮度（值范围为0-255)  
	unsigned char rgbRed;//红色的亮度（值范围为0-255)  
	unsigned char rgbReserved;//保留，必须为0  
}RGBQUAD;
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
	play_or_seek(p, 0);
	*/

	fileRead("C:\\2.ts","D:\\1111.aac");

	/*long long media_file_size = 34359738368000000;
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

	*/
	system("pause");
	return 0;
}
long long my_atoll(char *instr)
{
	long long retval;

	retval = 0;
	for (; *instr; instr++) {
		retval = 10 * retval + (*instr - '0');
	}
	return retval;
}
int fileRead(char *filePath, char *outPath) {
	printf("%s\n", filePath);
	FILE *tsFile = NULL;
	if ((tsFile = fopen(filePath, "rb")) == NULL)
	{
		printf("file not exist!\n");
	}

	FILE *outfile;
	printf("%s\n", outPath);
	outfile = fopen(outPath, "wb");
	if (!outfile) {
		printf("file not exist!\n");
		return -1;
	}

	size_t rs = 0;

	FRAME_DATA_POOL *audio_pool = frame_data_pool_create(5000);
	FRAME_DATA_POOL *video_pool = frame_data_pool_create(5000);

	TS_DEMUXER *d = ts_demuxer_create(audio_pool, video_pool);
	DECODER_MASTER *m = decoder_master_create(240, 160, audio_pool, video_pool);
	unsigned char *pkt = malloc(sizeof(unsigned char) * 188);

	int ts_num = 0;
	int x = 0;
	do {
		
		rs = fread(pkt, 188, 1, tsFile);
		if (rs > 0)
		{

			demux_ts_pkt(d, pkt);

			while (!is_pes_queue_empty(d))
			{
				FRAME_DATA *pes = poll_pes_pkt(d);

				decode_frame(m, pes);

			
				while (!is_priority_queue_empty(m->js_frame_queue)) {

					FRAME_DATA *f = priority_queue_poll(m->js_frame_queue);

					if (f->av_type == 0)
					{

						//printf("<<<<< PCM:%d \n", f->pts / 90);
						//fwrite(f->data, 1, f->len, outfile);

						frame_data_pool_return(m->audio_pool, f);
					}
					else
					{
						char outPath1[1024];

						sprintf(outPath1, "D:\\out\\out_pic_%d.bmp", f->pts / 90);

						FILE *outfile1;
						//printf("%s\n", outPath1);
					//	MySaveBmp(outPath1, f->data, 240, 160);

						//fwrite(f->data, 1, f->len, outfile1);
						//printf("<<<<< RGB:%d \n", f->pts /90);
						frame_data_pool_return(m->video_pool, f);
					}
					x++;

				}

				if (pes->av_type == AUDIO)
				{
					frame_data_pool_return(d->audio_pool, pes);
				}
				else
				{
					frame_data_pool_return(d->video_pool, pes);
				}

			
			}
		}

	} while (rs != 0);

	decoder_master_destroy(m);
	ts_demuxer_destroy(d);
	frame_data_pool_destroy(audio_pool);
	frame_data_pool_destroy(video_pool);
	free(pkt);

	if (fclose(tsFile))
	{
		printf("The file was not closed\n");
	}

	return 0;
}

void MySaveBmp(const char *filename, unsigned char *rgbbuf, int width, int height)
{
	MyBITMAPFILEHEADER bfh;
	MyBITMAPINFOHEADER bih;
	/* Magic number for file. It does not fit in the header structure due to alignment requirements, so put it outside */
	unsigned short bfType = 0x4d42;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfSize = 2 + sizeof(MyBITMAPFILEHEADER) + sizeof(MyBITMAPFILEHEADER) + width * height * 3;
	bfh.bfOffBits = 0x36;

	bih.biSize = sizeof(MyBITMAPINFOHEADER);
	bih.biWidth = width;
	bih.biHeight = height;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = 0;
	bih.biSizeImage = 0;
	bih.biXPelsPerMeter = 5000;
	bih.biYPelsPerMeter = 5000;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	FILE *file = fopen(filename, "wb");
	if (!file)
	{
		printf("Could not write file\n");
		return;
	}

	/*Write headers*/
	fwrite(&bfType, sizeof(bfType), 1, file);
	fwrite(&bfh, sizeof(bfh), 1, file);
	fwrite(&bih, sizeof(bih), 1, file);

	fwrite(rgbbuf, width*height * 3, 1, file);
	fclose(file);
}