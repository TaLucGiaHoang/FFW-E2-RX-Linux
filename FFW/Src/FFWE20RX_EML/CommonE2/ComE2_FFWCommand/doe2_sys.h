///////////////////////////////////////////////////////////////////////////////
/**
 * @file doe2_sys.h
 * @brief システム関連コマンドのヘッダファイル
 * @author PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/02/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
*/
#ifndef	__DOE2_SYS_H__
#define	__DOE2_SYS_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義
// CHGPWR用定義(E2用)
#define CHGPWR_WAIT_TIME		0x000003E8	// 電源供給電圧変更ウェイト時間(1000us(0x000003E8))

// enum定義
// EXPWR2(E2用)
enum FFWENM_EXPWR2_SUPPLY {		// 二系統電源供給要否を設定
	EXPWR2_SINGLE_POWER = 0,		// 単一電源供給
	EXPWR2_DOUBLE_POWER,			// 二系統電源供給
};

// グローバル関数の宣言
extern FFWERR DO_EXPWR2(enum FFWENM_EXPWR2_MODE eExPwr2Mode, BYTE byExPwr2Val);	///< ターゲットシステムへの電源供給(リニア指定)(E2用)
extern FFWERR DO_CHGPWR(BYTE byChgPwrVal);					///< ターゲットシステムへの電源供給電圧の変更(リニア指定)(E2用)
extern FFWERR DO_GetPwrSrc(BYTE* pbySupply);				///< 電源供給ソース情報取得(E2用)

#endif	// __DOE2_SYS_H__
