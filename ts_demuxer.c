#include "ts_demuxer.h"

static const TS_PKT_LEN = 188;

// 全局表
static TS_PAT *GLOBAL_PAT = NULL;
static TS_PMT *GLOBAL_PMT = NULL; // 当前节目

// 全局当前节目号
static int CUR_PROGRAM_NUM = -1;

// 全局buffer map
static BUFFER_MAP GLOBAL_BUFFER_MAP = {NULL,0,0};

// 临时program/stream列表
static int temp_programs_count = 0; // 全局节目数
static TS_PAT_PROGRAM *temp_programs = NULL; // 全局节目数组指针
static int temp_streams_count = 0; // 全局流总数
static TS_PMT_STREAM *temp_streams = NULL; // 全局流数组指针

// 输入ts包数据
int receive_ts_packet(unsigned char * pTsBuf)
{
	TS_HEADER header = { sizeof(TS_HEADER),0 };
	if (read_ts_head(pTsBuf, &header) != 0)
	{
		return -1;
	}

	printf("PID %d\n", header.PID);

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

int receive_ts_packet_by_program_num(unsigned char * pTsBuf, int program_num)
{
	CUR_PROGRAM_NUM = program_num;
	receive_ts_packet(pTsBuf);
	return 0;
}

// 读取ts包头
static int read_ts_head(unsigned char * pTsBuf, TS_HEADER * pHeader)
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
static int read_adaption_field(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	if (pHeader->adaptation_field_control == 0x2 ||
		pHeader->adaptation_field_control == 0x3)
	{
		pHeader->adaptaion_field_length = pTsBuf[4];
	}
	return 0;
}
// 读取有效载荷
static int read_payload(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	int rs = -1;

	// 看是否为PAT信息
	if (pHeader->PID == 0x0)
	{
		rs = read_ts_PAT(pTsBuf, pHeader);
	}

	// 是否为 BAT/SDT 信息
	if (pHeader->PID == 0x11)
	{
		//TODO
	}

	//  看是否为PMT信息
	if (GLOBAL_PAT != NULL)
	{
		for (int i = 0; i < GLOBAL_PAT->program_count; i++) {
			if (GLOBAL_PAT->pPrograms[i].PID == pHeader->PID) {
				rs = read_ts_PMT(pTsBuf, pHeader);
				break;
			}
		}
	}

	// 看是否为PES信息
	if (GLOBAL_PMT != NULL)
	{
		for (int j = 0; j < GLOBAL_PMT->stream_count; j++) {
			if (GLOBAL_PMT->pStreams[j].elementary_PID == pHeader->PID) {
				rs = receive_pes_payload(pTsBuf, pHeader);
			}
		}
	}

	return rs;
}

// 解析PAT
static int read_ts_PAT(unsigned char * pTsBuf, TS_HEADER * pHeader)
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

	unsigned char *payload = pTsBuf + start;

	TS_PAT tempPat;
	tempPat.table_id = payload[0];
	tempPat.section_syntax_indicator = payload[1] >> 7 & 0x1;
	tempPat.zero = payload[1] >> 6 & 0x1;
	tempPat.reserved1 = payload[1] >> 4 & 0x3;
	tempPat.section_length = ((payload[1] & 0x0f) << 8) | payload[2];
	tempPat.transport_stream_id = (payload[3] << 8) | payload[4];
	tempPat.reserved2 = payload[5] >> 6 & 0x3;
	tempPat.version_number = payload[5] >> 1 & 0x1f;
	tempPat.current_next_indicator = payload[5] & 0x1;
	tempPat.section_number = payload[6];
	tempPat.last_section_number = payload[7];

	// current_next_indicator 当前包无效
	if (tempPat.current_next_indicator != 0x1)
	{
		return 0;
	}

	if (tempPat.section_syntax_indicator != 0x1)
	{
		printf("不支持 section_syntax_indicator 为 0 \n");
		return -1;
	}

	// 有效负载总长度
	int len = 3 + tempPat.section_length;
	tempPat.CRC = payload[len - 4] << 24 |
		payload[len - 3] << 16 |
		payload[len - 2] << 8 |
		payload[len - 1] & 0xFF;

	// 检测固定位
	if (tempPat.table_id != 0x00)
	{
		printf("pat parse error!table_id \n");
		return -1;
	}
	if (tempPat.section_syntax_indicator != 0x1)
	{
		printf("pat parse error!section_syntax_indicator \n");
		return -1;
	}
	if (tempPat.zero != 0x0)
	{
		printf("pat parse error!zero \n");
		return -1;
	}

	// 提取循环部分字节数组
	int loopStartPos = 8;
	int loopLength = tempPat.section_length - 9;

	// 循环数据数组起始
	unsigned char * pLoopData = payload + loopStartPos;

	BYTE_LIST * pat_loop_data_buffer = buffer_map_get(&GLOBAL_BUFFER_MAP, pHeader->PID);

	// 当前段不是第一个pat包，加载旧字节数组
	if (tempPat.section_number != 0x00)
	{
		if (pat_loop_data_buffer == NULL)  // buffer未初始化
		{
			pat_loop_data_buffer = byte_list_create(loopLength);
			buffer_map_put(&GLOBAL_BUFFER_MAP, pHeader->PID, pat_loop_data_buffer);
		}
	}
	else
	{
		if (pat_loop_data_buffer != NULL)   // 抛弃旧的buffer数据
		{
			byte_list_clean(pat_loop_data_buffer);
		}
		pat_loop_data_buffer = byte_list_create(loopLength);
		buffer_map_put(&GLOBAL_BUFFER_MAP, pHeader->PID, pat_loop_data_buffer);
	}
	byte_list_add_list(pat_loop_data_buffer, pLoopData, loopLength);

	// 当前为最后分段，解析循环数据
	if (tempPat.section_number == tempPat.last_section_number)
	{

		// 不为4的整数倍，数据有错误
		if ((pat_loop_data_buffer->used_len % 4) != 0)
		{
			printf("pat parse error!pat.loopData.length \n");
			return -1;
		}

		tempPat.program_count = pat_loop_data_buffer->used_len / 4;

		// 当临时空间不够时申请空间或者扩容，否则直接覆盖临时空间
		if(tempPat.program_count > temp_programs_count)
		{
			if (temp_programs != NULL) 
			{
				TS_PAT_PROGRAM * pProgramsNew = realloc(temp_programs, sizeof(TS_PAT_PROGRAM) * tempPat.program_count);
				if (pProgramsNew == NULL)
				{
					free(temp_programs);
					temp_programs = NULL;
					return -1;
				}
				temp_programs = pProgramsNew;
			}
			else
			{
				temp_programs = (TS_PAT_PROGRAM *)malloc(sizeof(TS_PAT_PROGRAM) * tempPat.program_count);
				if (temp_programs == NULL)
				{
					return -1;
				}
			}			
		}
		tempPat.pPrograms = temp_programs;

		for (int i = 0; i < pat_loop_data_buffer->used_len; i += 4)
		{

			int program_number = ((pat_loop_data_buffer->pBytes[i] & 0xff) << 8) | (pat_loop_data_buffer->pBytes[i + 1] & 0xff);

			// 0x00 是NIT
			if (program_number == 0x00)
			{
				tempPat.networkPID = ((pat_loop_data_buffer->pBytes[i + 2] & 0x1f) << 8) | (pat_loop_data_buffer->pBytes[i + 3] & 0xff);
			}
			else
			{
				TS_PAT_PROGRAM prg;
				prg.program_number = program_number;
				prg.reserved = pat_loop_data_buffer->pBytes[i + 2] >> 5 & 0x3;
				prg.PID = ((pat_loop_data_buffer->pBytes[i + 2] & 0x1f) << 8) | (pat_loop_data_buffer->pBytes[i + 3] & 0xff);
				tempPat.pPrograms[i / 4] = prg;
			}
		}
			   
		// 清空data
		byte_list_clean(pat_loop_data_buffer);
	}

	ts_pat_submit(tempPat);

	if ((CUR_PROGRAM_NUM == -1) && (GLOBAL_PAT->program_count >= 1)) {
		CUR_PROGRAM_NUM = GLOBAL_PAT->pPrograms[0].program_number;
	}

	/* TESTS
	for (int i = 0; i < GLOBAL_PAT->program_count; i++) {
		
		printf("program_number:%d,reserved:%d,PID:%d\n",
			GLOBAL_PAT->pPrograms[i].program_number,
			GLOBAL_PAT->pPrograms[i].reserved,
			GLOBAL_PAT->pPrograms[i].PID);
	
	}*/
	return 0;
}

// 提交pat表
static int ts_pat_submit(TS_PAT pat)
{
	if (GLOBAL_PAT == NULL)
	{
		GLOBAL_PAT = (TS_PAT *)malloc(sizeof(TS_PAT));
		if (GLOBAL_PAT == NULL)
		{
			return -1;
		}
		GLOBAL_PAT->pPrograms = NULL;
		GLOBAL_PAT->program_count = 0;
	}

	GLOBAL_PAT->table_id = pat.table_id;
	GLOBAL_PAT->section_syntax_indicator = pat.section_syntax_indicator;
	GLOBAL_PAT->zero = pat.zero;
	GLOBAL_PAT->reserved1 = pat.reserved1;
	GLOBAL_PAT->section_length = pat.section_length;
	GLOBAL_PAT->transport_stream_id = pat.transport_stream_id;
	GLOBAL_PAT->reserved2 = pat.reserved2;
	GLOBAL_PAT->version_number = pat.version_number;
	GLOBAL_PAT->current_next_indicator = pat.current_next_indicator;
	GLOBAL_PAT->section_number = pat.section_number;
	GLOBAL_PAT->last_section_number = pat.last_section_number;
	GLOBAL_PAT->networkPID = pat.networkPID;
	GLOBAL_PAT->CRC = pat.CRC;

	// 新节目更多
	if (GLOBAL_PAT->program_count < pat.program_count)
	{	
		if (GLOBAL_PAT->pPrograms != NULL)
		{
			TS_PAT_PROGRAM * pProgramsNew = realloc(GLOBAL_PAT->pPrograms, sizeof(TS_PAT_PROGRAM) * pat.program_count);
			if (pProgramsNew == NULL)
			{
				free(GLOBAL_PAT);
				GLOBAL_PAT = NULL;
				return -1;
			}
			GLOBAL_PAT->pPrograms = pProgramsNew;
		}
		else
		{
			TS_PAT_PROGRAM * pProgramsNew = malloc(sizeof(TS_PAT_PROGRAM) * pat.program_count);
			if (pProgramsNew == NULL)
			{
				free(GLOBAL_PAT);
				GLOBAL_PAT = NULL;
				return -1;
			}
			GLOBAL_PAT->pPrograms = pProgramsNew;
		}		
	}
	GLOBAL_PAT->program_count = pat.program_count;

	memcpy(GLOBAL_PAT->pPrograms, pat.pPrograms, sizeof(TS_PAT_PROGRAM) * pat.program_count);
	return 0;
}

// 解析PMT
int read_ts_PMT(unsigned char * pTsBuf, TS_HEADER * pHeader)
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

	unsigned char *payload = pTsBuf + start;

	TS_PMT tempPmt;
	tempPmt.table_id = payload[0];
	tempPmt.section_syntax_indicator = payload[1] >> 7 & 0x1;
	tempPmt.zero = payload[1] >> 6 & 0x1;
	tempPmt.reserved1 = payload[1] >> 4 & 0x3;
	tempPmt.section_length = ((payload[1] & 0x0f) << 8) | payload[2];
	tempPmt.program_number = (payload[3] << 8) | payload[4];
	tempPmt.reserved2 = payload[5] >> 6 & 0x3;
	tempPmt.version_number = payload[5] >> 1 & 0x1f;
	tempPmt.current_next_indicator = payload[5] & 0x1;
	tempPmt.section_number = payload[6];
	tempPmt.last_section_number = payload[7];
	tempPmt.reserved3 = payload[8] >> 5 & 0x7;
	tempPmt.PCR_PID = ((payload[8] & 0x1f) << 8) | payload[9];
	tempPmt.reserved4 = payload[10] >> 4 & 0xf;
	tempPmt.program_info_length = ((payload[10] & 0xf) << 8) | payload[11];

	// 不是当前节目的pmt包，舍弃
	if (CUR_PROGRAM_NUM != tempPmt.program_number)
	{
		return 0;
	}

	// current_next_indicator 当前包无效
	if (tempPmt.current_next_indicator != 0x1)
	{
		return 0;
	}

	if (tempPmt.section_syntax_indicator != 0x1) {
		printf("不支持 section_syntax_indicator 为 0 \n");
		return -1;
	}

	// 有效负载总长度
	int len = 3 + tempPmt.section_length;
	tempPmt.CRC = payload[len - 4] << 24 |
		payload[len - 3] << 16 |
		payload[len - 2] << 8 |
		payload[len - 1] & 0xFF;

	// 检测固定位
	if (tempPmt.section_syntax_indicator != 0x1)
	{
		printf("pat parse error!section_syntax_indicator \n");
		return -1;
	}
	if (tempPmt.zero != 0x0)
	{
		printf("pat parse error!zero \n");
		return -1;
	}

	// programInfo提取
	if (tempPmt.program_info_length != 0x0)
	{
		int key = 0x1 << 13 | pHeader->PID;
		BYTE_LIST *pmt_program_info_buffer = buffer_map_get(&GLOBAL_BUFFER_MAP, key);

		if (pmt_program_info_buffer == NULL)
		{
			pmt_program_info_buffer = byte_list_create(tempPmt.program_info_length);
			buffer_map_put(&GLOBAL_BUFFER_MAP, key, pmt_program_info_buffer);
		}
		else
		{
			byte_list_clean(pmt_program_info_buffer);
		}

		unsigned char *pPrgInfo = pTsBuf + 12;
		byte_list_add_list(pmt_program_info_buffer, pPrgInfo, tempPmt.program_info_length);
	}
	
	// 提取循环部分字节数组
	int loopStartPos = 12 + tempPmt.program_info_length;
	int loopLength = tempPmt.section_length - 13 - tempPmt.program_info_length;

	// 循环数据数组起始
	unsigned char * pLoopData = payload + loopStartPos;
	
	BYTE_LIST *pmt_loop_data_buffer = buffer_map_get(&GLOBAL_BUFFER_MAP, pHeader->PID);

	// 当前段不是第一个pat包，加载旧字节数组
	if (tempPmt.section_number != 0x00)
	{
		if (pmt_loop_data_buffer == NULL)  // buffer未初始化
		{
			pmt_loop_data_buffer = byte_list_create(loopLength);
			buffer_map_put(&GLOBAL_BUFFER_MAP,pHeader->PID,pmt_loop_data_buffer);
		}
	}
	else
	{
		if (pmt_loop_data_buffer != NULL)   // 抛弃旧的buffer数据
		{
			byte_list_clean(pmt_loop_data_buffer);
		}
		pmt_loop_data_buffer = byte_list_create(loopLength);
		buffer_map_put(&GLOBAL_BUFFER_MAP, pHeader->PID, pmt_loop_data_buffer);
	}
	byte_list_add_list(pmt_loop_data_buffer, pLoopData, loopLength);

	// 当前为最后分段，解析循环数据
	if (tempPmt.section_number == tempPmt.last_section_number)
	{
		int pos = 0;
		int streamcount = 0;

		// 获取有多少流
		for (; pos < pmt_loop_data_buffer->used_len;)
		{
			int es_info_length = ((pmt_loop_data_buffer->pBytes[pos + 3] & 0xf) << 8) | pmt_loop_data_buffer->pBytes[pos + 4];
			if (es_info_length > 0) {
				pos += es_info_length;
			}
			pos += 5;
			streamcount++;
		}

		tempPmt.stream_count = streamcount;

		// 当临时空间不够时申请空间或者扩容，否则直接覆盖临时空间
		if (streamcount > temp_streams_count)
		{
			if (temp_streams != NULL)
			{
				TS_PMT_STREAM * pStreamsNew = realloc(temp_streams, sizeof(TS_PMT_STREAM) * tempPmt.stream_count);
				if (pStreamsNew == NULL)
				{
					free(temp_streams);
					temp_streams = NULL;
					return -1;
				}
				temp_streams = pStreamsNew;
			}
			else
			{
				temp_streams = (TS_PMT_STREAM *)malloc(sizeof(TS_PMT_STREAM) * tempPmt.stream_count);
				if (temp_streams == NULL)
				{
					return -1;
				}
			}
		}
		tempPmt.pStreams = temp_streams;

		pos = 0;
		streamcount = 0;
		for (; pos < pmt_loop_data_buffer->used_len;)
		{

			TS_PMT_STREAM s;
			s.stream_type = pmt_loop_data_buffer->pBytes[pos];
			s.reserved1 = (pmt_loop_data_buffer->pBytes[pos + 1] >> 5) & 0x7;
			s.elementary_PID = ((pmt_loop_data_buffer->pBytes[pos + 1] & 0x1f) << 8) | pmt_loop_data_buffer->pBytes[pos + 2];
			s.reserved2 = (pmt_loop_data_buffer->pBytes[pos + 3] >> 4) & 0xf;
			s.ES_info_length = ((pmt_loop_data_buffer->pBytes[pos + 3] & 0xf) << 8) | pmt_loop_data_buffer->pBytes[pos + 4];

			if (s.ES_info_length > 0) {

				// TODO 暂未解析
				pos += s.ES_info_length;
			}
			tempPmt.pStreams[streamcount] = s;
			pos += 5;
			streamcount++;
		}
		
		// 清空data
		byte_list_clean(pmt_loop_data_buffer);
	}
	ts_pmt_submit(tempPmt);
	
	/*for (int i = 0; i < GLOBAL_PMT->stream_count; i++) {

		printf("stream_type:%d,elementary_PID:%d,ES_info_length:%d\n",
			GLOBAL_PMT->pStreams[i].stream_type,
			GLOBAL_PMT->pStreams[i].elementary_PID,
			GLOBAL_PMT->pStreams[i].ES_info_length);
	}*/

	return 0;
}

// 提交pmt表
int ts_pmt_submit(TS_PMT pmt)
{
	if (GLOBAL_PMT == NULL)
	{
		GLOBAL_PMT = (TS_PMT *)malloc(sizeof(TS_PMT));
		if (GLOBAL_PMT == NULL)
		{
			return -1;
		}
		GLOBAL_PMT->pStreams = NULL;
		GLOBAL_PMT->stream_count = 0;
	}

	GLOBAL_PMT->table_id = pmt.table_id;
	GLOBAL_PMT->section_syntax_indicator = pmt.section_syntax_indicator;
	GLOBAL_PMT->zero = pmt.zero;
	GLOBAL_PMT->reserved1 = pmt.reserved1;
	GLOBAL_PMT->section_length = pmt.section_length;
	GLOBAL_PMT->program_number = pmt.program_number;
	GLOBAL_PMT->reserved2 = pmt.reserved2;
	GLOBAL_PMT->version_number = pmt.version_number;
	GLOBAL_PMT->current_next_indicator = pmt.current_next_indicator;
	GLOBAL_PMT->section_number = pmt.section_number;
	GLOBAL_PMT->last_section_number = pmt.last_section_number;
	GLOBAL_PMT->reserved3 = pmt.reserved3;
	GLOBAL_PMT->PCR_PID = pmt.PCR_PID;
	GLOBAL_PMT->reserved4 = pmt.reserved4;
	GLOBAL_PMT->program_info_length = pmt.program_info_length;

	// 新的流变多
	if (GLOBAL_PMT->stream_count < pmt.stream_count)
	{
		if (GLOBAL_PMT->pStreams != NULL)
		{
			TS_PMT_STREAM * pStreamsNew = realloc(GLOBAL_PMT->pStreams, sizeof(TS_PMT_STREAM) * pmt.stream_count);
			if (pStreamsNew == NULL)
			{
				free(GLOBAL_PMT);
				GLOBAL_PMT = NULL;
				return -1;
			}
			GLOBAL_PMT->pStreams = pStreamsNew;
		}
		else
		{
			TS_PMT_STREAM * pStreamsNew = malloc(sizeof(TS_PMT_STREAM) * pmt.stream_count);
			if (pStreamsNew == NULL)
			{
				free(GLOBAL_PMT);
				GLOBAL_PMT = NULL;
				return -1;
			}
			GLOBAL_PMT->pStreams = pStreamsNew;
		}
	}
	GLOBAL_PMT->stream_count = pmt.stream_count;

	memcpy(GLOBAL_PMT->pStreams, pmt.pStreams, sizeof(TS_PMT_STREAM) * pmt.stream_count);
	return 0;
}

// 输入pes载荷
int receive_pes_payload(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	int start = 4;
	int adpataion_field_length = pTsBuf[4];
	if (pHeader->adaptation_field_control == 0x3)
	{
		start = start + 1 + pHeader->adaptaion_field_length;
	}
	unsigned char *payload = pTsBuf + start;
	int payload_len = TS_PKT_LEN - start;
	
	// ts包中含有新pes包头
	if (pHeader->payload_unit_start_indicator == 0x1) {

		unsigned PES_packet_length = payload[4] << 8 | payload[5] ;
		unsigned finish_len = 0;
		if (PES_packet_length != 0)
		{
			finish_len = PES_packet_length + 6;
		}

		// 取出旧pes包缓存数据
		BYTE_LIST *pesBuffer = buffer_map_get(&GLOBAL_BUFFER_MAP, pHeader->PID);

		if (pesBuffer != NULL) {

			if (pesBuffer->used_len > 0)
			{
				// 获得PES包数据
				TS_PES_PACKET *pesPkt = read_pes(pesBuffer);

				// TODO
				//tsDecoder.feedbackPes(pesPkt, p.PID);

				// 清空但不释放空间
				byte_list_clean(pesBuffer);
			}
		}
		else
		{
			pesBuffer = byte_list_create(finish_len);
			buffer_map_put(&GLOBAL_BUFFER_MAP, pHeader->PID, pesBuffer);
		}
		pesBuffer->finish_len = finish_len;

		// 缓存中追加新数据
		byte_list_add_list(pesBuffer, payload, payload_len);

		// 调试信息打印
		int pesStartPrefix = (payload[0] << 16) | (payload[1] << 8) | (payload[2]);
		int stream_id = payload[3];
		//printf("%d (%d) > pre: %#X  stream_id: %d  PES_packet_length:%d  data: %d \n",	pHeader->PID, pHeader->payload_unit_start_indicator, pesStartPrefix, stream_id, PES_packet_length, payload_len);
	}
	else {

		// 取出旧pes包缓存数据
		BYTE_LIST *pesBuffer = buffer_map_get(&GLOBAL_BUFFER_MAP, pHeader->PID);
		if (pesBuffer != NULL)
		{
			// 缓存中追加新数据
			byte_list_add_list(pesBuffer, payload, payload_len);

			/*System.out.println(p.PID +"(" + p.payload_unit_start_indicator + ")" + "> data:" + + tsPayload.length + " loading:" +
					olderBuffer.buffer.length + "/" + (olderBuffer.PES_packet_length + 6));*/

			if (is_byte_list_finish(pesBuffer)) {

				// 获得PES包数据
				TS_PES_PACKET *pesPkt = read_pes(pesBuffer);

				// TODO
				//tsDecoder.feedbackPes(pesPkt, p.PID);

				// 清空但不释放空间
				byte_list_clean(pesBuffer);
			}
		}
		else {
			printf("receive_pes_payload:can't append data. \n");
			return -1;
		}
	}

	return 0;
}

// 解析pes包
TS_PES_PACKET * read_pes(BYTE_LIST * pPesByteList)
{
	printf("read pes: buffer:  %d/%d\n", pPesByteList->used_len, pPesByteList->finish_len);
	return 0;
}