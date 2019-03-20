#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>

#include "decoder_master.h"

DECODER * h264_decoder_create();

FRAME_DATA * h264_decode_func(FRAME_DATA * pPesPkt);
