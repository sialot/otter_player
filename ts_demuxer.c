#include "ts_demuxer.h"
#include <stdio.h>
#include <stdlib.h>

// 全局pat表
static TS_PAT PAT = { sizeof(TS_PAT),0 };

// pat数据包数据buffer
static unsigned char *pat_loop_data_buffer;

// 输入ts包数据
int receive_ts_packet(unsigned char * pTsBuf)
{
	TS_HEADER header = { sizeof(TS_HEADER),0 };
	if (read_ts_head(pTsBuf, &header) != 0) 
	{
		return -1;
	}

	printf("PID %d\n", header.PID);
	printf("continuity_counter %d\n", header.continuity_counter);

	// 获取适配域
	if (read_adaption_field(pTsBuf, &header) != 0) 
	{
		return -1;
	}

	// 解析净荷
	if (read_payload(pTsBuf, &header) != 0) 
	{
		return -1;
	}

	return 0;
}

// 读取ts包头
int read_ts_head(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	pHeader->sync_byte = pTsBuf[0];
	if (pHeader->sync_byte != 0x47) 
	{
		printf("sync_byte != 0x47\n");
		return -1;
	}

	pHeader->transport_error_indicator = pTsBuf[1] >> 7;
	pHeader->payload_unit_start_indicator = pTsBuf[1] >> 6 & 0x01;
	pHeader->transport_priority = pTsBuf[1] >> 5 & 0x01;
	pHeader->PID = (pTsBuf[1] & 0x1f) << 8 | pTsBuf[2];
	pHeader->transport_scrambling_control = pTsBuf[3] >> 6;
	pHeader->adaptation_field_control = pTsBuf[3] >> 4 & 0x03;
	pHeader->continuity_counter = pTsBuf[3] & 0x0f;
	return 0;
}

// 读取适配域
int read_adaption_field(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	if (pHeader->adaptation_field_control == 0x2 ||
		pHeader->adaptation_field_control == 0x3) 
	{
		pHeader->adpataion_field_length = pTsBuf[4];
	}
	return 0;
}
// 读取有效载荷
int read_payload(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	// 看是否为PAT信息
	if (pHeader->PID == 0x0) 
	{
		read_ts_PAT(pTsBuf, pHeader);
	}

	return 0;
}

// 解析PAT
int read_ts_PAT(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	int start = 4;
	int adpataion_field_length = pTsBuf[4];

	// 同时又负载和适配域
	if (pHeader->adaptation_field_control == 0x3)
	{
		start = start + 1 + pHeader->adaptation_field_control;
	}

	// 有效载荷开始的位置应再偏移1+[length]个字节。
	if (pHeader->payload_unit_start_indicator == 0x01)
	{
		start = start + 1 + pTsBuf[start];
	}

	unsigned char *pPatBuf = pTsBuf + start;

	// todo
}