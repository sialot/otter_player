#include "ts_demuxer.h"
#include "byte_list.h"

// 全局pat表 TODO
static TS_PAT *global_pat = NULL;

// pat数据包数据buffer
static BYTE_LIST *pat_loop_data_buffer = NULL;

// 节目号
static int current_program_num = -1;

// 输入ts包数据
int receive_ts_packet(unsigned char * pTsBuf)
{
	TS_HEADER header = { sizeof(TS_HEADER),0 };
	if (read_ts_head(pTsBuf, &header) != 0)
	{
		return -1;
	}

	//printf("PID %d\n", header.PID);
	//printf("continuity_counter %d\n", header.continuity_counter);

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
		pHeader->adpataion_field_length = pTsBuf[4];
	}
	return 0;
}
// 读取有效载荷
static int read_payload(unsigned char * pTsBuf, TS_HEADER * pHeader)
{
	// 看是否为PAT信息
	if (pHeader->PID == 0x0)
	{
		read_ts_PAT(pTsBuf, pHeader);
	}

	return 0;
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

	// 当前段不是第一个pat包，加载旧字节数组
	if (tempPat.section_number != 0x00)
	{
		if (pat_loop_data_buffer == NULL)  // buffer未初始化
		{
			pat_loop_data_buffer = byte_list_create(loopLength);
		}
	}
	else
	{
		if (pat_loop_data_buffer != NULL)   // 抛弃旧的buffer数据
		{
			byte_list_free(pat_loop_data_buffer);
			pat_loop_data_buffer = NULL;
		}
		pat_loop_data_buffer = byte_list_create(loopLength);
	}
	byte_list_add_list(pat_loop_data_buffer, pLoopData, loopLength);

	// 当前为最后分段，解析循环数据
	if (tempPat.section_number == tempPat.last_section_number)
	{

		// 不为4的整数倍，数据有错误
		if ((pat_loop_data_buffer->len % 4) != 0)
		{
			printf("pat parse error!pat.loopData.length");
			return -1;
		}
		
		tempPat.program_len = pat_loop_data_buffer->len / 4;			
		tempPat.pPrograms = (TS_PAT_PROGRAM *)malloc(sizeof(TS_PAT_PROGRAM) * tempPat.program_len);

		for (int i = 0; i < pat_loop_data_buffer->len; i += 4)
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
		byte_list_free(pat_loop_data_buffer);
		pat_loop_data_buffer = NULL;
	}

	ts_pat_submit(tempPat);

	if ((current_program_num == -1) && (global_pat->program_len == 1)) {
		current_program_num = global_pat->pPrograms[0].program_number;
	}


	// TESTS
	for (int i = 0; i < global_pat->program_len; i++) {
		
		printf("program_number:%d,reserved:%d,PID:%d\n",
			global_pat->pPrograms[i].program_number,
			global_pat->pPrograms[i].reserved,
			global_pat->pPrograms[i].PID);
	
	}
	return 0;
}

// 提交pat表
static int ts_pat_submit(TS_PAT pat)
{
	if (global_pat == NULL)
	{
		global_pat = (TS_PAT *)malloc(sizeof(TS_PAT));
		if (global_pat == NULL)
		{
			return -1;
		}
		global_pat->pPrograms = NULL;
		global_pat->program_len = 0;
	}

	global_pat->table_id = pat.table_id;
	global_pat->section_syntax_indicator = pat.section_syntax_indicator;
	global_pat->zero = pat.zero;
	global_pat->reserved1 = pat.reserved1;
	global_pat->section_length = pat.section_length;
	global_pat->transport_stream_id = pat.transport_stream_id;
	global_pat->reserved2 = pat.reserved2;
	global_pat->version_number = pat.version_number;
	global_pat->current_next_indicator = pat.current_next_indicator;
	global_pat->section_number = pat.section_number;
	global_pat->last_section_number = pat.last_section_number;
	global_pat->networkPID = pat.networkPID;
	global_pat->CRC = pat.CRC;

	// 新节目更多
	if (global_pat->program_len < pat.program_len)
	{	
		if (global_pat->pPrograms != NULL)
		{
			TS_PAT_PROGRAM * pProgramsNew = realloc(global_pat->pPrograms, sizeof(TS_PAT_PROGRAM) * pat.program_len);
			if (pProgramsNew == NULL)
			{
				free(global_pat);
				global_pat = NULL;
				return -1;
			}
			global_pat->pPrograms = pProgramsNew;
		}
		else
		{
			TS_PAT_PROGRAM * pProgramsNew = malloc(sizeof(TS_PAT_PROGRAM) * pat.program_len);
			if (pProgramsNew == NULL)
			{
				free(global_pat);
				global_pat = NULL;
				return -1;
			}
			global_pat->pPrograms = pProgramsNew;
		}		
	}
	global_pat->program_len = pat.program_len;

	memcpy(global_pat->pPrograms, pat.pPrograms, sizeof(TS_PAT_PROGRAM) * pat.program_len);

	free(pat.pPrograms);
	pat.pPrograms = NULL;
	return 0;
}
