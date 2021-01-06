/**
 * @file rfwasp_converter.cpp
 * @brief ASP記録レコードからAPI構造体への変換(品種固有部)
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015-2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/11/16
 */

#include "rfwasp_converter.h"
#include "rfwasp_rec.h"
//#include "doasprl_monev.h"

void AspConverter::convert(int frametype, BYTE *pByCnv, const RecFrame &frame)
{
	if (frametype != 9) {
		return;
	}

	RFWASP_MONITOR_EVENT *monevent = reinterpret_cast<RFWASP_MONITOR_EVENT*>(pByCnv);
	monevent->common.time_nsec;
	monevent->byEventNo = bit_extract<BYTE, DWORD>(frame[0], 16, 8);	// RevRL78No170420-001-02
	monevent->byError = bit_extract<BYTE, DWORD>(frame[0], 24, 2);
	
	/*
	 * frame[0]..frame[n]にはendian変換した生データが入っているので
	 * 構造体に変換する
	 */
}
