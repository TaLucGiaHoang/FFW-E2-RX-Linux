/**
 * @file rfwasp_converter_base.h
 * @brief ASP記録ファイル変換処理
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015-2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/11/16
 */
#ifndef __RFWASP_CONVERTER_BASE_H__
#define __RFWASP_CONVERTER_BASE_H__

#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include "rfw_bitops.h"
#include "rfwasp_recdata.h"
#include "rfwasp_recframe.h"
#include "rfwasp_converter.h"

/**
 * RecFameからAPI構造体への変換helper
 */
struct AspConverterBase {
	static BYTE *_cnv_buf;		/**< 変換先バッファ */
	static UINT64 _cnv_buf_max;	/**< 変換先バッファ最大バイト数 */
	static UINT64 _cnv_cnt;		/**< バッファ書込み済バイト数 */
	static DWORD _store_cnt;	/**< 変換個数 */
	static DWORD *_pexist_cnt;	/**< 変換個数(バッファに収まらない分も含む) */

	/**
	 * RecFrameからAPI structへの変換先バッファ設定
	 */
	static void set_cnv_buf(void *p, UINT64 cnt)
	{
		_cnv_buf = static_cast<BYTE*>(p);
		_cnv_buf_max = cnt;
		_cnv_cnt = 0;
	}

	/**
	 * Frameの変換
	 */
	static bool extract(const RecFrame &rec, UINT64 clk);

	/** FPGAのFrameTypeからRF_ASPTYPE_xxへの変換 */
	static DWORD to_asptype(FrameType type);

	/** RF_ASPTYPE_xx から FPGAのFrameTypeへの変換 */
	static FrameType to_frametype(DWORD type);

	static void ToApiRec(RFWASP_EVENT_RECORD *event, const RecFrame &frame, UINT64 clk);
	static void ToApiRec(RFWASP_POWER_RECORD *power, const RecFrame &frame, UINT64 clk);
	static void ToApiRec(RFWASP_CAN_RECORD *can, const RecFrame &frame, UINT64 clk);

	/** API 構造体共通部への変換 */
	static bool ToApiCommon(RFWASP_REC_COMMON *common, const RecFrame &frame, UINT64 clk);

	template <class T> static bool ToApiStruct(const RecFrame &frame, UINT64 clk)
	{
		if (_cnv_buf) {
			if (_cnv_cnt + sizeof(T) <= _cnv_buf_max) {
				ToApiRec(reinterpret_cast<T*>(&_cnv_buf[_cnv_cnt]), frame, clk);
				_cnv_cnt += sizeof(T);
				++_store_cnt;
			} else {
				return false;
			}
		} else {
			_cnv_cnt += sizeof(T);
		}
		return true;
	}

};

#endif
