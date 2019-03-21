#include "aac_decoder.h"

DECODER * aac_decoder_create()
{
	DECODER *aac_decoder = malloc(sizeof(DECODER));
	if (aac_decoder == NULL)
	{
		return NULL;
	}
	aac_decoder->decode_frame = aac_decode_func;
	return aac_decoder;
}

int aac_decode_func(void * d, FRAME_DATA * pPesPkt, PRIORITY_QUEUE *queue)
{
	printf("aac decoding\n");
	return pPesPkt;
}