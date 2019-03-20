#include "aac_decoder.h"

DECODER * aac_decoder_create()
{
	DECODER *aac_decoder = malloc(sizeof(DECODER));
	if (aac_decoder == NULL)
	{
		return NULL;
	}
	aac_decoder->decode = aac_decode_func;
	return aac_decoder;
}

FRAME_DATA * aac_decode_func(FRAME_DATA * pPesPkt)
{
	printf("aac decoding\n");
	return pPesPkt;
}