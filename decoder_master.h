#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include "frame_data.h"

// ����������
typedef struct DECODER
{
	int(*decode_frame)(void *, FRAME_DATA *, PRIORITY_QUEUE *);
	int display_width; // ��ʾ��
	int display_height; // ��ʾ��
	AVCodec *codec;
	AVCodecContext *context;
	AVCodecParserContext *parser;
	AVPacket *pkt;
	AVFrame *decoded_frame;
	struct SwsContext *swx_ctx;
	FRAME_DATA_POOL *frame_pool;
} DECODER;

#include "aac_decoder.h"
#include "h264_decoder.h"

// ����������Ա
typedef struct DECODER_MASTER
{
	PRIORITY_QUEUE *js_frame_queue;
	DECODER *aac_decoder;
	DECODER *h264_decoder;
	FRAME_DATA_POOL *audio_pool;
	FRAME_DATA_POOL *video_pool;
	int display_width; // ��ʾ��
	int display_height; // ��ʾ��
} DECODER_MASTER;

// ��������
DECODER_MASTER * decoder_master_create(int display_width, int display_height, FRAME_DATA_POOL *audio_pool, FRAME_DATA_POOL *video_pool);

// ����
int decode_frame(DECODER_MASTER *d, FRAME_DATA * f);

// ��ս��
void decode_queue_clean(DECODER_MASTER *d);

// ���ٶ���
void decoder_master_destroy(DECODER_MASTER * d);

// ��ȡ��������stream_type ������  h.264�����Ӧ0x1b;aac�����Ӧ0x0f;mp3�����Ӧ0x03
DECODER * _get_decoder(DECODER_MASTER * d, unsigned stream_type);