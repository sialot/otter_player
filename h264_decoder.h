#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "decoder_master.h"

// 创建解码器
DECODER * h264_decoder_create(int display_width, int display_height);

// 解码
int h264_decode_func(void * pDecoder, FRAME_DATA * pPesPkt, PRIORITY_QUEUE *queue);

// 销毁解码器
void h264_decode_destory(DECODER *d);
