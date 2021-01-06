///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfw_recdata.h
 * @brief ASP構造体定義
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef _RFW_RECDATA_H_
#define _RFW_RECDATA_H_

#include "rfw_bitops.h"

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

typedef struct {
	UINT64	time_nsec;
	DWORD	type;
}
RFWASP_REC_COMMON;

typedef struct {
	RFWASP_REC_COMMON common;
	WORD	data;
}
RFWASP_UART_RECORD;

typedef struct {
	RFWASP_REC_COMMON	common;
	DWORD		dwId;
	BYTE		bySrr;
	BYTE		byIde;
	BYTE		byRtr;
	BYTE		byDlc;
	BYTE		byData[8];
	WORD		wCrc;
	BYTE		byCrcDel;
	BYTE		byAck;
	BYTE		byAckDel;
	BYTE		byEof;
	BYTE		byItm;
	BYTE		byOlf;
	WORD		wErrorStatus;
}
RFWASP_CAN_RECORD;

typedef struct {
	RFWASP_REC_COMMON	common;
	union {
		struct {
			BYTE		rw;
			WORD		addr;
			BYTE		ack0;
			BYTE		ack1;
		} addr;
		struct {
			BYTE	data;
			BYTE	ack;
			BYTE	stop_cond;
		} data;
	};
}
RFWASP_I2C_RECORD;

typedef struct {
	RFWASP_REC_COMMON	common;
	BYTE		length;
	DWORD		data;
}
RFWASP_SPI_RECORD;

typedef struct {
	RFWASP_REC_COMMON	common;
	DWORD		dwUa;
}
RFWASP_POWER_RECORD;

typedef struct {
	RFWASP_REC_COMMON	common;
	DWORD		factor;
}
RFWASP_EVENT_RECORD;

typedef struct {
	RFWASP_REC_COMMON	common;
	BYTE		core;
	union {
		DWORD		pc;
		DWORD		mem;
	};
}
RFWASP_PC_MEM_RECORD;

typedef struct {
	RFWASP_REC_COMMON	common;
	union {
		BYTE		data[27];
		WORD		change_data;
	};
}
RFWASP_LOGIC_ANALYZER_RECORD;


typedef struct {
	RFWASP_REC_COMMON   common;		// 記録データ共通部
	BYTE			  byEventNo;	// 成立したモニタイベント条件番号
	BYTE			  byError;	// 通信エラー情報
}
RFWASP_MONITOR_EVENT;


typedef struct {
	RFWASP_REC_COMMON common;
	BYTE		tcode;
	BYTE		vmid;
	BYTE		vm;
	BYTE		htid;
	union {
		DWORD	pc;
		DWORD	tag;
		struct {
			WORD		no;
			DWORD		val;
		} reg;
	};
}
RFWASP_STRACE_RECORD;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

#endif
