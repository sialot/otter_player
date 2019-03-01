#include "ts_demuxer.h"
#include <stdio.h>
#include <stdlib.h>

// 输入ts包数据
int ireceive_ts_packet(unsigned char * pTsBuf)
{

	TS_HEADER *pHeader = malloc(sizeof(TS_HEADER));

	if (read_ts_head(pTsBuf, pHeader) != 0) {
		return -1;
	}

	printf("PID %d\n", pHeader->PID);
	printf("continuity_counter %d\n", pHeader->continuity_counter);
	free(pHeader);

	return 0;
}

// 读取ts包头
int read_ts_head(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	pHeader->sync_byte = pTsBuf[0];
	if (pHeader->sync_byte != 0x47) {
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

void test_icld()
{
	printf("ts_demuxer !");
}