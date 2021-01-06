///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_converter.cpp
 * @brief ASP記録レコードからAPI構造体への変換(品種固有部)
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////

#include "rfwasp_converter.h"
#include "rfwasp_rec.h"

//=============================================================================
/**
 * @brief 品種固有のFPGAデータ->構造体変換
 * @param frametype 品種固有フレーム種別
 * @param pByCnv	変換先バッファ
 * @param frame		変換元frame
 */
//=============================================================================
void AspConverter::convert(int frametype, BYTE *pByCnv, const RecFrame &frame)
{
	if (frametype != 8)
		return;

	RFWASP_STRACE_RECORD *strace = reinterpret_cast<RFWASP_STRACE_RECORD*>(pByCnv);

	/*
	 * frame._revにはendian変換した生データが入っているので
	 * 構造体に変換する
	 * フォーマットはLLWEB-00006355-05_G3_TCU-SFT_Spec_main.pdf参照
	 */
	strace->tcode = bit_extract<BYTE, DWORD>(frame[0], 16, 2);
	strace->vmid = bit_extract<BYTE, DWORD>(frame[0], 16 + 2, 3);
	strace->vm = bit_extract<BYTE, DWORD>(frame[0], 16 + 5, 1);
	strace->htid = 0;
	if (strace->tcode == 0) {
		/* PC */
		strace->htid = bit_extract<BYTE, DWORD>(frame[0], 16 + 6, 6);
		strace->pc = bit_extract<WORD, DWORD>(frame[1], 16, 16) +
		             (bit_extract<WORD, DWORD>(frame[2], 16, 16) << 16);
	} else if (strace->tcode == 1) {
		/* TAG */
		strace->tag = bit_extract<DWORD, DWORD>(frame[0], 16 + 6, 10);
	} else if (strace->tcode == 2) {
		/* REG */
		strace->reg.no = bit_extract<WORD, DWORD>(frame[0], 16 + 6, 5);
		strace->htid = bit_extract<BYTE, DWORD>(frame[0], 16 + 11, 5);
		strace->reg.val = bit_extract<WORD, DWORD>(frame[1], 16, 16) +
		                  (bit_extract<WORD, DWORD>(frame[2], 16, 16) << 16);
	} else {
		/* OverFlow */
		strace->htid = bit_extract<BYTE, DWORD>(frame[0], 16 + 6, 6);
	}
}
