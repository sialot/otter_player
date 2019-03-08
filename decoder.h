#pragma once

typedef VIDEO_FRAME
{
	unsigned long long PTS : 33; //PTS(presentation time stamp ��ʾʱ���ǩ)
	unsigned long long DTS : 33; //DTS(decoding time stamp ����ʱ���ǩ)��־λ

	// TODO
	unsigned char * frame_data;
} VIDEO_FRAME;

typedef AUDIO_FRAME
{
	unsigned long long PTS : 33; //PTS(presentation time stamp ��ʾʱ���ǩ)
	unsigned long long DTS : 33; //DTS(decoding time stamp ����ʱ���ǩ)��־λ

	// TODO
	unsigned char * frame_data;
} AUDIO_FRAME;

int decode();