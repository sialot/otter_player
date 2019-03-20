#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>

#include "decoder_master.h"

DECODER * aac_decoder_create();

FRAME_DATA * aac_decode_func(FRAME_DATA * pPesPkt);
