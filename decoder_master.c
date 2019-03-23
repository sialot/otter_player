#include "decoder_master.h"

// ��󻺴�֡������Ƶ+��Ƶ
const int MAX_BUFFER_FRAME_COUNT = 500;

// ��СԤ����֡��
const int MAX_PREBUFFER_FRAME_COUNT = 50;

// ��������
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

// ����
int decode_frame(DECODER_MASTER * d, FRAME_DATA * f)
{

	// ��ȡ��Ӧ������
	DECODER *decoder = _get_decoder(d, f->stream_type);
	if (decoder == NULL)
	{
		printf("unsurported stream_type:%d !play failed! \n", f->stream_type);
		return -1;
	}

	// ���벢���js֡��չʾ����
	int ret = decoder->decode_frame(decoder, f, d->js_frame_queue);
	if (ret < 0)
	{
		printf("decode failed! \n");
		return -1;
	}
	return 0;
}

// ��ս��
void decode_queue_clean(DECODER_MASTER * d)
{
	priority_queue_clean(d->js_frame_queue);
}

// ���ٶ���
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

// ��ȡ��������stream_type ������  h.264�����Ӧ0x1b;aac�����Ӧ0x0f;mp3�����Ӧ0x03
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
