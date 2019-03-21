#include "h264_decoder.h"

DECODER * h264_decoder_create()
{
	DECODER *h264_decoder = malloc(sizeof(DECODER));
	if (h264_decoder == NULL)
	{
		return NULL;
	}
	h264_decoder->decode_frame = h264_decode_func;
	return h264_decoder;
}

int h264_decode_func(void * pDecoder, FRAME_DATA * pPesPkt, PRIORITY_QUEUE *queue)
{
	printf("h264 decoding\n");
	return pPesPkt;
}
void h264_decode_destory(DECODER *d)
{

	free(d);
	return;
}