#include "h264_decoder.h"

DECODER * h264_decoder_create()
{
	DECODER *h264_decoder = malloc(sizeof(DECODER));
	if (h264_decoder == NULL)
	{
		return NULL;
	}
	h264_decoder->decode = h264_decode_func;
	return h264_decoder;
}

FRAME_DATA * h264_decode_func(FRAME_DATA * pPesPkt)
{
	printf("h264 decoding\n");
	return pPesPkt;
}