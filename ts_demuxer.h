#pragma once

typedef struct TS_HEADER
{
	unsigned sync_byte                       :8; // 同步字节：固定为0x47;
	unsigned transport_error_indicator       :1; // 传输错误标志：‘1’表示在相关的传输包中至少有一个不可纠正的错误位。
	unsigned payload_unit_start_indicator    :1; // 负载起始标志：在前4个字节之后会有一个调整字节，其的数值为后面调整字段的长度length。
	unsigned transport_priority              :1; // 传输优先级标志
	unsigned PID                             :13;// PID
	unsigned transport_scrambling_control    :2; // 加扰控制标志：表示TS流分组有效负载的加密模式。空包为‘00’
	unsigned adaptation_field_control        :2; // 适配域控制标志‘00’为ISO/IEC未来使用保留；
	unsigned continuity_counter              :4; // 连续性计数器
	unsigned adpataion_field_length          :8; // 适配域长度
} TS_HEADER;

// 输入ts包数据
int ireceive_ts_packet(unsigned char *pTsBuf);

// 读取ts包头
int read_ts_head(unsigned char *pTsBuf,TS_HEADER *pHeader);

void test_icld();