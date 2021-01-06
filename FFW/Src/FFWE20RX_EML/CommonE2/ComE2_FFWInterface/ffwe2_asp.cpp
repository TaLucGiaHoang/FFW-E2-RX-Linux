///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwe2_asp.cpp
 * @brief E2 拡張機能コマンドの実装ファイル
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
#include "ffwe2_asp.h"
#include "doe2_asp.h"
#include "do_sys.h"
#include "doasp_sys_family.h"
#include "domcu_prog.h"

// ファイル内static変数の宣言

///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * BFWへのE2拡張機能の有効状態の通知(E2用)
 * @param byBfwSendData E2拡張機能の有効状態の設定値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_SetAspConf(BYTE byBfwSendData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;

	getEinfData(&einfData);

	if (einfData.wEmuStatus != EML_E2) {		// E2エミュレータでない場合
		return FFWERR_FUNC_UNSUPORT;
	}

	if (GetMcuRunState()) {		// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}

	ferr = DO_SetAspConf(byBfwSendData);

	return ferr;
}

//=============================================================================
/**
 * BFWへの通過ポイントの設定条件の通知(E2用)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_SetMonitorPointAddr(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;

	getEinfData(&einfData);

	if (einfData.wEmuStatus != EML_E2) {		// E2エミュレータでない場合
		return FFWERR_FUNC_UNSUPORT;
	}

	if (GetMcuRunState()) {		// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}

	ferr = DO_SetMonitorPointAddr();

	return ferr;
}
