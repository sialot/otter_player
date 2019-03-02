#pragma once

// ts 头
typedef struct TS_HEADER
{
	unsigned sync_byte                       : 8; // 同步字节：固定为0x47;
	unsigned transport_error_indicator       : 1; // 传输错误标志：‘1’表示在相关的传输包中至少有一个不可纠正的错误位。
	unsigned payload_unit_start_indicator    : 1; // 负载起始标志：在前4个字节之后会有一个调整字节，其的数值为后面调整字段的长度length。
	unsigned transport_priority              : 1; // 传输优先级标志
	unsigned PID                             : 13;// PID
	unsigned transport_scrambling_control    : 2; // 加扰控制标志：表示TS流分组有效负载的加密模式。空包为‘00’
	unsigned adaptation_field_control        : 2; // 适配域控制标志‘00’为ISO/IEC未来使用保留；
	unsigned continuity_counter              : 4; // 连续性计数器
	unsigned adpataion_field_length          : 8; // 适配域长度
} TS_HEADER;

// pat 节目 
typedef struct TS_PAT_PROGRAM
{
	unsigned program_number : 16; // 节目号
	unsigned reserved       : 3; // 保留字段，固定为111
	unsigned PID            : 13; // 节目号对应内容的PID值
} TS_PAT_PROGRAM;

// pat表
typedef struct TS_PAT
{
	unsigned table_id                        : 8; // PAT表固定为0x00
	unsigned section_syntax_indicator        : 1; // 段语法标志位，固定为1
	unsigned zero                            : 1; // 固定为0
	unsigned reserved1                       : 2; //  保留字段，固定为11
	unsigned section_length                  : 12; // 表示这个字节后面数据的长度,包括 CRC信息
	unsigned transport_stream_id             : 16; // 该传输流的ID
	unsigned reserved2                       : 2; // 保留字段，固定为11
	unsigned version_number                  : 5; //  版本号，固定为00000,有变化则版本号加1
	unsigned current_next_indicator          : 1; // PAT是否有效,固定为1，表示这个PAT表可以用，如果为0则要等待下一个PAT表
	unsigned section_number                  : 8; // 分段号码,最多256个分段
	unsigned last_section_number             : 8; // 最后一个分段的号码
	unsigned networkPID                      : 16; // 网络PID
	unsigned CRC                             : 32; // CRC校验码
	unsigned program_len                     : 8; // 节目数量
	TS_PAT_PROGRAM programs[];
} TS_PAT;

// 输入ts包数据
int receive_ts_packet(unsigned char *pTsBuf);

// 读取ts包头
int read_ts_head(unsigned char *pTsBuf,TS_HEADER *pHeader);

// 读取适配域
int read_adaption_field(unsigned char *pTsBuf, TS_HEADER *pHeader);

// 读取有效载荷
int read_payload(unsigned char *pTsBuf, TS_HEADER *pHeader);

// 解析PAT
int read_ts_PAT(unsigned char * pTsBuf, TS_HEADER * pHeader);