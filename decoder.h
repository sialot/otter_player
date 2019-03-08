#pragma once

typedef VIDEO_FRAME
{
	unsigned long long PTS : 33; //PTS(presentation time stamp 显示时间标签)
	unsigned long long DTS : 33; //DTS(decoding time stamp 解码时间标签)标志位

	// TODO
	unsigned char * frame_data;
} VIDEO_FRAME;

typedef AUDIO_FRAME
{
	unsigned long long PTS : 33; //PTS(presentation time stamp 显示时间标签)
	unsigned long long DTS : 33; //DTS(decoding time stamp 解码时间标签)标志位

	// TODO
	unsigned char * frame_data;
} AUDIO_FRAME;

int decode();