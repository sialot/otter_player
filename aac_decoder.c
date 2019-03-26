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
	aac_decoder->swr_ctx = NULL;
	aac_decoder->src_data = NULL;
	aac_decoder->dst_data = NULL;

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

	// 解析数据获得一个Packet， 从输入的数据流中分离出一帧一帧的压缩编码数据
	int ret = av_parser_parse2(d->parser, d->context, &d->pkt->data, &d->pkt->size,
		data, data_size,
		pPesPkt->dtime, pPesPkt->ptime, 0);

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
				return 0;
			else if (ret < 0) {
				printf("Error during decoding\n");
				return -1;
			}
			data_size = av_get_bytes_per_sample(d->context->sample_fmt);
			if (data_size < 0) {
				/* This should not occur, checking just for paranoia */
				printf("Failed to calculate data size\n");
				return -1;
			}

			RESAMPLE_RESULT rs = aac_audio_resample(d);
			if (rs.length < 0)
			{
				return -1;
			}

			FRAME_DATA *out_frame = frame_data_create(pPesPkt->av_type, 0x01, pPesPkt->dtime, pPesPkt->ptime, rs.data, rs.length);
			out_frame->channels = d->context->channels;
			priority_queue_push(queue, out_frame, out_frame->ptime);

			///printf("get pcm data.nb_samples:%d, channels:%d, data_size:%d, total_size:%d \n ", d->decoded_frame->nb_samples, d->context->channels, (int)data_size, js_frame_data_len);
		}
	}

	return 0;
}

RESAMPLE_RESULT aac_audio_resample(DECODER * d)
{
	RESAMPLE_RESULT error_rs = { NULL, -1 };
	int data_size = 0;
	int ret = 0;

	int64_t src_ch_layout = (d->context->channel_layout &&
		d->context->channels ==
		av_get_channel_layout_nb_channels(d->context->channel_layout)) ?
		d->context->channel_layout :
		av_get_default_channel_layout(d->context->channels);
	int	src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);
	int src_linesize;
	enum AVSampleFormat src_sample_fmt = d->context->sample_fmt;
	int src_nb_samples = d->decoded_frame->nb_samples;
	int src_sample_rate = d->context->sample_rate;

	int64_t dst_ch_layout = AV_CH_LAYOUT_STEREO; //这里设定ok
	int dst_nb_channels = 2;
	int dst_linesize;
	enum AVSampleFormat dst_sample_fmt = AV_SAMPLE_FMT_FLTP;
	int dst_nb_samples = 0;
	int dst_sample_rate = 44100;

	int max_dst_nb_samples = 0;
	int resampled_data_size = 0;
	int dst_bufsize = 0;

	d->swr_ctx = swr_alloc();
	if (!d->swr_ctx)
	{
		printf("swr_alloc error \n");
		return error_rs;
	}

	/* set options */
	av_opt_set_int(d->swr_ctx, "in_channel_layout", src_ch_layout, 0);
	av_opt_set_int(d->swr_ctx, "in_sample_rate", src_sample_rate, 0);
	av_opt_set_sample_fmt(d->swr_ctx, "in_sample_fmt", src_sample_fmt, 0);

	av_opt_set_int(d->swr_ctx, "out_channel_layout", dst_ch_layout, 0);
	av_opt_set_int(d->swr_ctx, "out_sample_rate", dst_sample_rate, 0);
	av_opt_set_sample_fmt(d->swr_ctx, "out_sample_fmt", dst_sample_fmt, 0);
	swr_init(d->swr_ctx);

	// src空间开辟
	ret = av_samples_alloc_array_and_samples(&d->src_data, &src_linesize, src_nb_channels,
		src_nb_samples, src_sample_fmt, 0);

	if (ret < 0) {
		printf("Could not allocate source samples\n");
		return error_rs;
	}

	// 估算输出样本数
	max_dst_nb_samples = dst_nb_samples =
		av_rescale_rnd(src_nb_samples, dst_sample_rate, src_sample_rate, AV_ROUND_UP);
	if (max_dst_nb_samples <= 0)
	{
		printf("av_rescale_rnd error \n");
		return error_rs;
	}

	// dst空间开辟
	ret = av_samples_alloc_array_and_samples(&d->dst_data, &dst_linesize, dst_nb_channels,
		dst_nb_samples, dst_sample_fmt, 0);
	if (ret < 0)
	{
		printf("av_samples_alloc_array_and_samples error \n");
		return error_rs;
	}

	// 准备入参
	int per_sample_size = av_get_bytes_per_sample(d->context->sample_fmt);
	int in_pcm_data_len = d->decoded_frame->nb_samples * d->context->channels * per_sample_size;

	for (int i = 0; i < d->decoded_frame->nb_samples; i++)
	{
		for (int ch = 0; ch < d->context->channels; ch++)
		{
			memcpy(d->src_data[0] + (per_sample_size * (i * 2 + ch)), d->decoded_frame->data[ch] + per_sample_size * i, per_sample_size);
		}
	}

	// 估算输出样本数
	dst_nb_samples = av_rescale_rnd(swr_get_delay(d->swr_ctx, src_sample_rate) +
		src_nb_samples, dst_sample_rate, src_sample_rate, AV_ROUND_UP);

	if (dst_nb_samples > max_dst_nb_samples)
	{
		av_free(&d->dst_data[0]);
		ret = av_samples_alloc(d->dst_data, &dst_linesize, dst_nb_channels,
			dst_nb_samples, dst_nb_samples, 1);
		max_dst_nb_samples = dst_nb_samples;
	}
	
	// 转换
	ret = swr_convert(d->swr_ctx, d->dst_data, dst_nb_samples,
		(const uint8_t **)d->src_data, src_nb_samples);
	if (ret < 0)
	{
		printf("swr_convert error \n");

		return error_rs;
	}
	
	dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
		ret, dst_sample_fmt, 1);
	if (dst_bufsize <= 0)
	{
		printf("av_samples_get_buffer_size error \n");
		return error_rs;
	}
	
	uint8_t * out_data = malloc(sizeof(unsigned char) * dst_bufsize);
	memcpy(out_data, d->dst_data[0], dst_bufsize);

	RESAMPLE_RESULT rs = { out_data,dst_bufsize };
	if (d->swr_ctx != NULL)
	{
		swr_free(&d->swr_ctx);
	}

	return rs;
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
	av_freep(&d->src_data[0]);
	av_freep(&d->src_data);

	av_freep(&d->dst_data[0]);
	av_freep(&d->dst_data);

}
