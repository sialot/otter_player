#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "decoder_master.h"

// ����������
DECODER * h264_decoder_create(int display_width, int display_height);

// ����
int h264_decode_func(void * pDecoder, FRAME_DATA * pPesPkt, PRIORITY_QUEUE *queue);

// ���ٽ�����
void h264_decode_destory(DECODER *d);
