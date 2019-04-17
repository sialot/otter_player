#include "h264_decoder.h"

DECODER * h264_decoder_create(int display_width, int display_height, FRAME_DATA_POOL *frame_pool)
{
	DECODER *h264_decoder = malloc(sizeof(DECODER));
	if (h264_decoder == NULL)
	{
		return NULL;
	}
	h264_decoder->decode_frame = h264_decode_func;
	h264_decoder->codec = NULL;
	h264_decoder->context = NULL;
	h264_decoder->decoded_frame = NULL;
	h264_decoder->swx_ctx = NULL;
	h264_decoder->display_height = display_height;
	h264_decoder->display_width = display_width;
	h264_decoder->frame_pool = frame_pool;

	/* find the h264 audio decoder */
	h264_decoder->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!h264_decoder->codec)
	{
		h264_decode_destory(h264_decoder);
		printf("Codec not found! \n");
		return NULL;
	}

	// ����������
	h264_decoder->context = avcodec_alloc_context3(h264_decoder->codec);
	if (!h264_decoder->context)
	{
		h264_decode_destory(h264_decoder);
		printf("Could not allocate audio codec context !\n");
		return NULL;
	}

	//��ʼ������������Ĳ���Ӧ���ɾ����ҵ�����  AV_PIX_FMT_YUV420P;
	h264_decoder->context->time_base.num = 1;
	h264_decoder->context->time_base.den = 25;//֡�� 
	h264_decoder->context->frame_number = 1; //ÿ��һ����Ƶ֡  
	h264_decoder->context->codec_type = AVMEDIA_TYPE_VIDEO;
	h264_decoder->context->pix_fmt = AV_PIX_FMT_YUVJ420P;
	h264_decoder->context->color_range = AVCOL_RANGE_MPEG;

	// �򿪽�����
	if (avcodec_open2(h264_decoder->context, h264_decoder->codec, NULL) < 0)
	{
		h264_decode_destory(h264_decoder);
		printf("Could not allocate audio codec context !\n");
		return NULL;
	}

	// ��ʼ����
	h264_decoder->pkt = av_packet_alloc();

	// ��ʼ�����֡
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

	d->pkt->pts = pPesPkt->pts;
	d->pkt->dts = pPesPkt->dts;
	d->pkt->data = pPesPkt->data;
	d->pkt->size = pPesPkt->len;

	int ret;
	ret = avcodec_send_packet(d->context, d->pkt);
	if (ret < 0) {
		printf("H264_DECODE:Error submitting the packet to the decoder\n");
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

		// ��ʼ������
		if (d->swx_ctx == NULL)
		{
			d->swx_ctx = sws_getContext(d->decoded_frame->width, d->decoded_frame->height, 
				d->context->pix_fmt, d->display_width, d->display_height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		}

		// ׼�����ſռ�
		if ((ret = av_image_alloc(dst_data, dst_linesize, d->display_width, d->display_height, AV_PIX_FMT_RGB24, 1)) < 0) {
			printf("Could not allocate destination image\n");
			return -1;
		}

		// ����
		sws_scale(d->swx_ctx, (const uint8_t *const *)d->decoded_frame->data, d->decoded_frame->linesize, 0,
			d->decoded_frame->height, dst_data, dst_linesize);

		// ��֡
		FRAME_DATA *out_frame = frame_data_pool_borrow(d->frame_pool);
		if (out_frame == NULL) {
			printf("video frame pool is empty.decode failed! \n");
			return -1;
		}

		// ��������
		frame_data_set(out_frame, pPesPkt->av_type, 0x02, (unsigned long long)d->decoded_frame->pkt_dts, (unsigned long long)d->decoded_frame->pts, dst_data[0], ret);
		priority_queue_push(queue, out_frame, out_frame->pts / 90);

		// ������ʱ����
		av_frame_unref(d->decoded_frame);
		av_freep(&dst_data[0]);
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