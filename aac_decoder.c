#include "aac_decoder.h"

// 创建解码器
DECODER * aac_decoder_create()
{
	DECODER *aac_decoder = malloc(sizeof(DECODER));
	if (aac_decoder == NULL)
	{
		return NULL;
	}
	aac_decoder->decode_frame = aac_decode_func;
	aac_decoder->codec = NULL;
	aac_decoder->context = NULL;
	aac_decoder->parser = NULL;
	aac_decoder->decoded_frame = NULL;
	aac_decoder->swx_ctx = NULL;
	aac_decoder->display_height = 0;
	aac_decoder->display_width = 0;

	/* find the AAC audio decoder */
	aac_decoder->codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
	if (!aac_decoder->codec)
	{
		aac_decode_destory(aac_decoder);
		printf("Codec not found! \n");
		return NULL;
	}

	// 获取parser
	aac_decoder->parser = av_parser_init(aac_decoder->codec->id);
	if (!aac_decoder->parser)
	{
		aac_decode_destory(aac_decoder);
		printf("Parser not found! \n");
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

	// 输入数据，输入长度
	uint8_t *data = (uint8_t *)pPesPkt->data;
	size_t data_size = (size_t)pPesPkt->len;
	size_t sample_data_size = 0;

	while (data_size > 0) {

		// 解析数据获得一个Packet， 从输入的数据流中分离出一帧一帧的压缩编码数据
		int ret = av_parser_parse2(d->parser, d->context, &d->pkt->data, &d->pkt->size,
			data, data_size,
			pPesPkt->pts, pPesPkt->dts, 0);

		if (ret < 0) {
			printf("Error while parsing\n");
			return -1;
		}
		data += ret;
		data_size -= ret;

		if (d->pkt->size)
		{
			int ret;
			ret = avcodec_send_packet(d->context, d->pkt);
			if (ret < 0) {
				printf("Error submitting the packet to the decoder\n");
				return -1;
			}

			/* read all the output frames (in general there may be any number of them */
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
				int out_len = d->decoded_frame->nb_samples * d->context->channels * sample_data_size;
				unsigned char * out_data = malloc(sizeof(unsigned char) * out_len);

				for (int i = 0; i < d->decoded_frame->nb_samples; i++)
				{
					for (int ch = 0; ch < d->context->channels; ch++)
					{
						memcpy(out_data + (sample_data_size * (i * 2 + ch)), d->decoded_frame->data[ch] + sample_data_size * i, sample_data_size);
					}
				}

				FRAME_DATA *out_frame = frame_data_create(pPesPkt->av_type, 0x01, pPesPkt->dts, pPesPkt->pts, out_data, out_len);
				out_frame->channels = d->context->channels;
				priority_queue_push(queue, out_frame, out_frame->ptime);

				///printf("get pcm data.nb_samples:%d, channels:%d, data_size:%d, total_size:%d \n ", d->decoded_frame->nb_samples, d->context->channels, (int)data_size, js_frame_data_len);
			}
		}
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
	if (d->parser != NULL)
	{
		av_parser_close(d->parser);
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
