/**
 * @file rfwasp_converter.h
 * @brief ASP記録レコードからAPI構造体への変換
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015-2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/11/16
 */
#ifndef __RFWASP_CONVERTER_H_
#define __RFWASP_CONVERTER_H_

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
#include "rfwasp_fio.h"
#include "rfwasp_recdata.h"
#include "rfwasp_converter_base.h"
#include "RFW_min.h"
#include "RFWASP.h"

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

/* RL用モニタイベント構造体 */

#if defined(_MSC_VER) | defined(__GNUC__)
#pragma pack(pop)
#endif


/* 品種固有構造体変換 */
struct AspConverter : public AspConverterBase {
	/*
	 * 構造体のサイズを返す
	 * @param frametype FPGAデータフォーマットのFrameType(品種固有のFrameTypeのみ呼び出される)
	 * @return 変換先の構造体サイズ 対応しないFrameTypeでは0を返すこと
	 */
	static UINT64 convert_size(int frametype)
	{
		if (frametype == 9) {
			return sizeof(RFWASP_MONITOR_EVENT);
		} else {
			return 0;
		}
	}

	/*
	 * 構造体への変換 convert_sizeで0以外を返した場合に呼ばれる
	 * 構造体共通部分は変換済なので固有部の変換を行う
	 * @param frametype
	 * @param pByCnv 変換先ポインタ
	 * @param frame[0]...frame[n]にrawデータが入っている
	 */
	static void convert(int frametype, BYTE *pByCnv, const RecFrame &frame);

	/**
	 * 固有部のRF_ASPTYPE_XXからFPGAのFrameTypeへの変換
	 * 共通部で処理しないRF_ASPTYPE_XXが指定された場合に呼び出される
	 * @param DWORD
	 * @return FrameTypeのうちSPECIFIC_8からSPECIFIC_Eのどれか
	 *         変換できない場合はMAX_FRAME_TYPEを返すこと
	 */
	static FrameType AspTypeToFpgaFrameType(DWORD type)
	{
		if (type == RF_ASPTYPE_MON_EV) {
			return SPECIFIC_9;
		} else {
			return MAX_FRAME_TYPE;
		}
	}

	/**
	 * FPGAのFrameTypeから固有部のRF_ASPTYPE_XXへの変換
	 * 共通部で処理しないFPGAのFrameTypeの場合に呼び出される
	 * @param FrameType(SPECIFIC_8からSPECIFIC_Eのどれか)
	 * @return 固有部で定義したRF_ASPTYPE_xx
	 */
	static DWORD FpgaFrameTypeToAspType(FrameType type)
	{
		if (type == SPECIFIC_9) {
			return RF_ASPTYPE_MON_EV;
		} else {
			return 0;
		}
	}

};

#endif
