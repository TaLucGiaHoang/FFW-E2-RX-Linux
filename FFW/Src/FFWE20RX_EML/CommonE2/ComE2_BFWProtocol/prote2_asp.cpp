///////////////////////////////////////////////////////////////////////////////
/**
 * @file prote2_asp.cpp
 * @brief BFWコマンド プロトコル生成関数(E2 拡張機能コマンド)
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
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "ffwmcu_mcu.h"
#include "doasprx_monev.h"


//=============================================================================
/**
 * BFWへのE2拡張機能の有効状態の通知(E2用)
 * @param byBfwSendData E2拡張機能の有効状態の設定値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_SET_ASPCONF(BYTE byBfwSendData)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_SET_ASPCONF;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1(byBfwSendData) != TRUE) {	// E2拡張機能の有効状態の設定値
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);				// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);				// 処理中断コード受信

		
	return ferr;
}

//=============================================================================
/**
 * BFWへの通過ポイントの設定条件の通知(E2用)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_SET_MONITORPOINTADDR(void)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	enum FFWENM_ENDIAN	eMcuEndian;
	RFW_ASPMONEVCOND_TABLE AspMonEvTable = { 0 };
#if defined(E2E2L)
	BYTE	byCount;
	BYTE	byEventNo;
	BYTE	byDataCount;
	RFWERR	dwRet = RFWERR_OK;
#endif

	wCmdCode = BFWCMD_SET_MONITORPOINTADDR;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	eMcuEndian = GetEndianDataRX();	// エンディアン情報取得
	if (PutData1((BYTE)eMcuEndian) != TRUE) {	// エンディアン情報送信
		return FFWERR_COM;
	}

#if defined(E2E2L)
	byCount = GetMonEventSetNum(RF_ASPMONEV_TYPE_SWBRK);
	if (PutData1(byCount) != TRUE) {	// 通過ポイント設定点数送信
		return FFWERR_COM;
	}

	for (byDataCount = 0, byEventNo = MONEV_SWBRKNO_START; byDataCount < MONEV_EV_MAX; byDataCount++, byEventNo++) {
		dwRet = GetMonEvData(byEventNo, &AspMonEvTable);
		if (dwRet == RFWERR_OK) {	// 設定されている
			if (AspMonEvTable.Cond.byDetectType == RF_ASPMONEV_TYPE_SWBRK) {	// ソフトウェアブレーク方式
				if (PutData1(AspMonEvTable.byChNo) != TRUE) {	// 通過ポイント番号送信
					return FFWERR_COM;
				}
				if (PutData4((DWORD)AspMonEvTable.Cond.u64EventAddr) != TRUE) {	// 通過ポイントアドレス送信
					return FFWERR_COM;
				}
			}
		}
	}
#endif

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);			// 処理中断コード受信

		
	return ferr;
}
