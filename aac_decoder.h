#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>

#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

#include "decoder_master.h"

// 创建解码器
DECODER * aac_decoder_create();

// 解码
int aac_decode_func(void * pDecoder, FRAME_DATA * pPesPkt, PRIORITY_QUEUE *queue);

typedef struct RESAMPLE_RESULT
{
	uint8_t * data;
	int length;
} RESAMPLE_RESULT;

// 重采样
RESAMPLE_RESULT aac_audio_resample(DECODER * d);

// 销毁解码器
void aac_decode_destory(DECODER *d);