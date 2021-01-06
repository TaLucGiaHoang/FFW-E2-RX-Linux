///////////////////////////////////////////////////////////////////////////////
/**
 * @file doe2_asp.h
 * @brief E2拡張機能関連コマンドのヘッダファイル
 * @author PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2No171004-001 2017/10/04 PA 辻
	新規作成
	E2拡張機能対応
*/
#ifndef	__DOE2_ASP_H__
#define	__DOE2_ASP_H__

#include "ffw_typedef.h"


// define定義
// E2拡張機能用定義(E2用)
#define ASP_FUNC_ENABLE		(1<<0)		// E2拡張機能の有効
#define ASP_FUNC_MONEV		(1<<1)		// 通過ポイント有効
#define ASP_FUNC_BRKEV		(1<<2)		// ブレークイベント(トリガ条件)有効

// グローバル関数の宣言
extern FFWERR DO_SetAspConf(BYTE byBfwSendData);	///< BFWへのE2拡張機能の有効状態の通知(E2用)
extern FFWERR DO_SetMonitorPointAddr(void);			///< BFWへの通過ポイントの設定条件の通知(E2用)

#endif	// __DOE2_ASP_H__
