#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include "priority_queue.h"

// ����������
typedef struct DECODER
{
	int(*decode_frame)(void *, FRAME_DATA *, PRIORITY_QUEUE *);
	AVCodec *codec;
	AVCodecContext *context;
	AVCodecParserContext *parser;
	AVPacket *pkt;
	AVFrame *decoded_frame;
} DECODER;

#include "aac_decoder.h"
#include "h264_decoder.h"

// ����������Ա
typedef struct DECODER_MASTER
{
	PRIORITY_QUEUE *js_frame_queue;
	DECODER *aac_decoder;
	DECODER *h264_decoder;
} DECODER_MASTER;

// ��������
DECODER_MASTER * decoder_master_create();

// ����
int decode_frame(DECODER_MASTER *d, FRAME_DATA * f);

// ���ٶ���
void decoder_master_destroy(DECODER_MASTER * d);

// ��ȡ��������stream_type ������  h.264�����Ӧ0x1b;aac�����Ӧ0x0f;mp3�����Ӧ0x03
DECODER * _get_decoder(DECODER_MASTER * d, unsigned stream_type);