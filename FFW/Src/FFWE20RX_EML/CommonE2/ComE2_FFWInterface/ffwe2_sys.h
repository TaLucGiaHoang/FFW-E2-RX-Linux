///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwe2_sys.h
 * @brief システム関連コマンドのヘッダファイル
 * @author PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#ifndef	__FFWE2_SYS_H__
#define	__FFWE2_SYS_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// enum定義
// EXPWR2(E2用)
enum BFWENM_POWER_SOURCE {		// 電源供給ソース情報
	EML_POWER = 0,					// エミュレータからの電源供給
	EXT_POWER,						// 外部電源からの電源供給
};

// グローバル関数の宣言
// RevRxE2No171004-001 Modify Line
extern void SaveExPwr2Setting(enum FFWENM_EXPWR2_MODE eExPwr2Mode, BYTE byExPwr2Val);	// 電源供給設定を保存(E2用)
extern enum FFWENM_EXPWR2_MODE GetExPwr2Mode(void);			// 電源供給モード設定を取得(E2用)
// RevRxE2No171004-001 Append Line
extern BYTE GetExPwr2Val(void);								// 電源供給電圧設定を取得(E2用)

#endif	// __FFWE2_SYS_H__
