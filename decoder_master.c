#include "decoder_master.h"

const int MAX_BUFFER_FRAME_COUNT = 4000;
const int MAX_PREBUFFER_FRAME_COUNT = 1500;

// 创建对象
DECODER_MASTER * decoder_master_create()
{
	DECODER_MASTER *d = (DECODER_MASTER *)malloc(sizeof(DECODER_MASTER));
	if (d == NULL)
	{
		printf("decoder_master_create failed! \n");
		return NULL;
	}

	PRIORITY_QUEUE *js_frame_queue = priority_queue_create(MAX_BUFFER_FRAME_COUNT, MAX_PREBUFFER_FRAME_COUNT);
	if (js_frame_queue == NULL)
	{
		printf("js_frame_queue create failed! \n");
		return NULL;
	}
	d->js_frame_queue = js_frame_queue;
	d->aac_decoder = NULL;
	d->h264_decoder = NULL;
	return d;
}

// 解码
int decode_frame(DECODER_MASTER * d, FRAME_DATA * f)
{
	DECODER *decoder = _get_decoder(d, f->stream_type);
	if (decoder == NULL)
	{
		printf("unsurported stream_type!play failed! \n");
		return -1;
	}

	FRAME_DATA *js_frame = decoder->decode(f);
	if (js_frame == NULL)
	{
		printf("decode failed!play failed! \n");
		return -1;
	}
	priority_queue_push(d->js_frame_queue, js_frame, js_frame->PTS);
	return 0;
}

// 销毁对象
void decoder_master_destroy(DECODER_MASTER * d)
{
	if (d == NULL)
	{
		return;
	}
	if (d->js_frame_queue != NULL)
	{
		priority_queue_destroy(d->js_frame_queue);
	}
	free(d);
}

// 获取解码器，stream_type 流类型  h.264编码对应0x1b;aac编码对应0x0f;mp3编码对应0x03
DECODER * _get_decoder(DECODER_MASTER * d, unsigned stream_type)
{
	switch (stream_type)
	{
	case 0x1b: 
		if (d->h264_decoder == NULL)
		{
			d->h264_decoder = h264_decoder_create();
		}
		return d->h264_decoder;
	case 0x0f:
		if (d->aac_decoder == NULL)
		{
			d->aac_decoder = aac_decoder_create();
		}
		return d->aac_decoder;
	}
	return NULL;
}
