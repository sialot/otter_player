#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>

#include "decoder_master.h"

DECODER * aac_decoder_create();

int aac_decode_func(void * d, FRAME_DATA * pPesPkt, PRIORITY_QUEUE *queue);
