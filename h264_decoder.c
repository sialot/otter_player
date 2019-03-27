#include "h264_decoder.h"

DECODER * h264_decoder_create(int display_width, int display_height)
{
	DECODER *h264_decoder = malloc(sizeof(DECODER));
	if (h264_decoder == NULL)
	{
		return NULL;
	}
	h264_decoder->decode_frame = h264_decode_func;
	h264_decoder->codec = NULL;
	h264_decoder->context = NULL;
	h264_decoder->parser = NULL;
	h264_decoder->decoded_frame = NULL;
	h264_decoder->swx_ctx = NULL;
	h264_decoder->display_height = display_height;
	h264_decoder->display_width = display_width;

	/* find the h264 audio decoder */
	h264_decoder->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!h264_decoder->codec)
	{
		h264_decode_destory(h264_decoder);
		printf("Codec not found! \n");
		return NULL;
	}

	// 获取parser
	h264_decoder->parser = av_parser_init(h264_decoder->codec->id);
	if (!h264_decoder->parser)
	{
		h264_decode_destory(h264_decoder);
		printf("Parser not found! \n");
		return NULL;
	}

	// 创建上线文
	h264_decoder->context = avcodec_alloc_context3(h264_decoder->codec);
	if (!h264_decoder->context)
	{
		h264_decode_destory(h264_decoder);
		printf("Could not allocate audio codec context !\n");
		return NULL;
	}

	//初始化参数，下面的参数应该由具体的业务决定  AV_PIX_FMT_YUV420P;
	h264_decoder->context->time_base.num = 1;
	h264_decoder->context->time_base.den = 25;//帧率 
	h264_decoder->context->frame_number = 1; //每包一个视频帧  
	h264_decoder->context->codec_type = AVMEDIA_TYPE_VIDEO;
	h264_decoder->context->pix_fmt = AV_PIX_FMT_YUVJ420P;
	h264_decoder->context->color_range = AVCOL_RANGE_MPEG;

	// 打开解码器
	if (avcodec_open2(h264_decoder->context, h264_decoder->codec, NULL) < 0)
	{
		h264_decode_destory(h264_decoder);
		printf("Could not allocate audio codec context !\n");
		return NULL;
	}

	// 初始化包
	h264_decoder->pkt = av_packet_alloc();

	// 初始化结果帧
	if (!h264_decoder->decoded_frame)
	{
		if (!(h264_decoder->decoded_frame = av_frame_alloc()))
		{
			h264_decode_destory(h264_decoder);
			printf("Could not allocate audio frame\n");
			return NULL;
		}
	}

	return h264_decoder;
}

int h264_decode_func(void * pDecoder, FRAME_DATA * pPesPkt, PRIORITY_QUEUE *queue)
{
	DECODER *d = (DECODER *)pDecoder;

	// 输入数据，输入长度
	uint8_t *data = (uint8_t *)pPesPkt->data;
	size_t data_size = (size_t)pPesPkt->len;

	while (data_size > 0)
	{

		// 解析数据获得一个Packet， 从输入的数据流中分离出一帧一帧的压缩编码数据
		int ret = av_parser_parse2(d->parser, d->context, &d->pkt->data, &d->pkt->size,
			data, data_size, pPesPkt->pts, pPesPkt->dts, 0);

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

			while (ret >= 0) {
				ret = avcodec_receive_frame(d->context, d->decoded_frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					continue;
				}
				else if (ret < 0) {
					printf("Error during decoding\n");
					return -1;
				}
				
				uint8_t *dst_data[4];
				int dst_linesize[4], ret;

				// 初始化缩放
				if (d->swx_ctx == NULL)
				{
					d->swx_ctx = sws_getContext(d->decoded_frame->width, d->decoded_frame->height, 
						d->context->pix_fmt, d->display_width, d->display_height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
				}

				// 准备缩放空间
				if ((ret = av_image_alloc(dst_data, dst_linesize, d->display_width, d->display_height, AV_PIX_FMT_RGB24, 1)) < 0) {
					printf("Could not allocate destination image\n");
					return -1;
				}

				// 缩放
				sws_scale(d->swx_ctx, (const uint8_t *const *)d->decoded_frame->data, d->decoded_frame->linesize, 0,
					d->decoded_frame->height, dst_data, dst_linesize);

				unsigned char * out_data = malloc(sizeof(unsigned char) * ret);
				memcpy(out_data, dst_data[0], ret);
				av_freep(&dst_data[0]);
				FRAME_DATA *out_frame = frame_data_create(pPesPkt->av_type, 0x02, (unsigned long long)d->decoded_frame->pkt_dts, (unsigned long long)d->decoded_frame->pts, out_data, ret);
				priority_queue_push(queue, out_frame, out_frame->ptime);
			}
		}
	}

	return 0;
}

void h264_decode_destory(DECODER *d)
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
	if (d->pkt != NULL)
	{
		av_packet_free(&d->pkt);
	}

	if (d->swx_ctx != NULL)
	{
		sws_freeContext(d->swx_ctx);
	}

	free(d);
	return;
}