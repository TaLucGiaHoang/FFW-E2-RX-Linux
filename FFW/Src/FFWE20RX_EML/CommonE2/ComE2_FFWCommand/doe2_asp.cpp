///////////////////////////////////////////////////////////////////////////////
/**
 * @file doe2_asp.cpp
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
#include "doe2_asp.h"
#include "prote2_asp.h"
#include "prot_common.h"

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * BFWへのE2拡張機能の有効状態の通知(E2用)
 * @param byBfwSendData E2拡張機能の有効状態の設定値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetAspConf(BYTE byBfwSendData)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	// E2拡張機能の有効状態通知
	ferr = PROT_SET_ASPCONF(byBfwSendData);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * BFWへの通過ポイントの設定条件の通知(E2用)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetMonitorPointAddr(void)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	// 通過ポイントの設定点数、および、通過ポイント番号と通過ポイントアドレス通知
	ferr = PROT_SET_MONITORPOINTADDR();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

