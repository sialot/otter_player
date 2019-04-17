#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include "frame_data.h"

// 解码器定义
typedef struct DECODER
{
	int(*decode_frame)(void *, FRAME_DATA *, PRIORITY_QUEUE *);
	int display_width; // 显示宽
	int display_height; // 显示高
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

// 解码器管理员
typedef struct DECODER_MASTER
{
	PRIORITY_QUEUE *js_frame_queue;
	DECODER *aac_decoder;
	DECODER *h264_decoder;
	FRAME_DATA_POOL *audio_pool;
	FRAME_DATA_POOL *video_pool;
	int display_width; // 显示宽
	int display_height; // 显示高
} DECODER_MASTER;

// 创建对象
DECODER_MASTER * decoder_master_create(int display_width, int display_height, FRAME_DATA_POOL *audio_pool, FRAME_DATA_POOL *video_pool);

// 解码
int decode_frame(DECODER_MASTER *d, FRAME_DATA * f);

// 清空结果
void decode_queue_clean(DECODER_MASTER *d);

// 销毁对象
void decoder_master_destroy(DECODER_MASTER * d);

// 获取解码器，stream_type 流类型  h.264编码对应0x1b;aac编码对应0x0f;mp3编码对应0x03
DECODER * _get_decoder(DECODER_MASTER * d, unsigned stream_type);