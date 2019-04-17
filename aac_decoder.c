#include "aac_decoder.h"

// 创建解码器
DECODER * aac_decoder_create(FRAME_DATA_POOL *frame_pool)
{
	DECODER *aac_decoder = malloc(sizeof(DECODER));
	if (aac_decoder == NULL)
	{
		return NULL;
	}
	aac_decoder->decode_frame = aac_decode_func;
	aac_decoder->codec = NULL;
	aac_decoder->context = NULL;
	aac_decoder->decoded_frame = NULL;
	aac_decoder->swx_ctx = NULL;
	aac_decoder->display_height = 0;
	aac_decoder->display_width = 0;
	aac_decoder->frame_pool = frame_pool;

	/* find the AAC audio decoder */
	aac_decoder->codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
	if (!aac_decoder->codec)
	{
		aac_decode_destory(aac_decoder);
		printf("Codec not found! \n");
		return NULL;
	}

	// 创建上线文
	aac_decoder->context = avcodec_alloc_context3(aac_decoder->codec);
	if (!aac_decoder->context)
	{
		aac_decode_destory(aac_decoder);
		printf("Could not allocate audio codec context !\n");
		return NULL;
	}

	// 打开解码器
	if (avcodec_open2(aac_decoder->context, aac_decoder->codec, NULL) < 0)
	{
		aac_decode_destory(aac_decoder);
		printf("Could not allocate audio codec context !\n");
		return NULL;
	}

	// 初始化包
	aac_decoder->pkt = av_packet_alloc();

	// 初始化结果帧
	if (!aac_decoder->decoded_frame)
	{
		if (!(aac_decoder->decoded_frame = av_frame_alloc()))
		{
			aac_decode_destory(aac_decoder);
			printf("Could not allocate audio frame\n");
			return NULL;
		}
	}

	return aac_decoder;
}

// aac解码
int aac_decode_func(void * pDecoder, FRAME_DATA * pPesPkt, PRIORITY_QUEUE *queue)
{
	DECODER *d = (DECODER *)pDecoder;

	d->pkt->pts = pPesPkt->pts;
	d->pkt->dts = pPesPkt->dts;
	d->pkt->data = pPesPkt->data;
	d->pkt->size = pPesPkt->len;

	size_t sample_data_size = 0;
	int ret;
	ret = avcodec_send_packet(d->context, d->pkt);
	if (ret < 0) {
		printf("ACC_DECODE:Error submitting the packet to the decoder\n");
		return -1;
	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(d->context, d->decoded_frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			continue;
		else if (ret < 0) {
			printf("Error during decoding\n");
			return -1;
		}
		sample_data_size = av_get_bytes_per_sample(d->context->sample_fmt);
		if (sample_data_size < 0) {
			/* This should not occur, checking just for paranoia */
			printf("Failed to calculate data size\n");
			return -1;
		}

		// 准备入参
		FRAME_DATA *out_frame = frame_data_pool_borrow(d->frame_pool);
		if (out_frame == NULL) {
			printf("audio frame pool is empty.decode failed! \n");
			return -1;
		}

		for (int i = 0; i < d->decoded_frame->nb_samples; i++)
		{
			for (int ch = 0; ch < d->context->channels; ch++)
			{
				frame_data_set(out_frame, pPesPkt->av_type, 0x01, pPesPkt->dts, pPesPkt->pts, d->decoded_frame->data[ch] + sample_data_size * i, sample_data_size);
			}
		}

		out_frame->channels = d->context->channels;
		priority_queue_push(queue, out_frame, out_frame->pts / 90);
		av_frame_unref(d->decoded_frame);
	}
	return 0;
}

void aac_decode_destory(DECODER * d)
{
	if (d == NULL)
	{
		return;
	}
	if (d->context != NULL)
	{
		avcodec_free_context(&d->context);
	}
	if (d->decoded_frame != NULL)
	{
		av_frame_free(&d->decoded_frame);
	}
	if (d->pkt == NULL)
	{
		av_packet_free(&d->pkt);
	}
	free(d);
}
