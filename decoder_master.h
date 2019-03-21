#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include "priority_queue.h"

// 解码器定义
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

// 解码器管理员
typedef struct DECODER_MASTER
{
	PRIORITY_QUEUE *js_frame_queue;
	DECODER *aac_decoder;
	DECODER *h264_decoder;
} DECODER_MASTER;

// 创建对象
DECODER_MASTER * decoder_master_create();

// 解码
int decode_frame(DECODER_MASTER *d, FRAME_DATA * f);

// 销毁对象
void decoder_master_destroy(DECODER_MASTER * d);

// 获取解码器，stream_type 流类型  h.264编码对应0x1b;aac编码对应0x0f;mp3编码对应0x03
DECODER * _get_decoder(DECODER_MASTER * d, unsigned stream_type);