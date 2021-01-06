///////////////////////////////////////////////////////////////////////////////
/**
 * @file prote2_sys.cpp
 * @brief BFWコマンド プロトコル生成関数(システムコマンド)
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
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "doe2_sys.h"


//=============================================================================
/**
 * ターゲットシステムへの電源供給(リニア指定)(E2用)
 * @param eExPwr2Mode 電源供給モード
 * @param wExPwr2Val 電源供給電圧
 * @param eExPwr2Sply 二系統電源供給要否
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_EXPWR2(enum FFWENM_EXPWR2_MODE eExPwr2Mode, WORD wExPwr2Val, enum FFWENM_EXPWR2_SUPPLY eExPwr2Sply)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_EXTPWR2;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)eExPwr2Mode) != TRUE) {	// 電源供給モード
		return FFWERR_COM;
	}

	if (PutData2(wExPwr2Val) != TRUE) {			// 電源供給電圧値
		return FFWERR_COM;
	}

	if (PutData1((BYTE)eExPwr2Sply) != TRUE) {	// 二系統電源供給要否
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
 * ターゲットシステムへの電源供給電圧の変更(リニア指定)(E2用)
 * @param wChgPwrVal 電源供給電圧
 * @param dwChgPwrWait ウェイト時間
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_CHGPWR(WORD wChgPwrVal, DWORD dwChgPwrWait)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_CHGPWR;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData2(wChgPwrVal) != TRUE) {		// 電源供給電圧値
		return FFWERR_COM;
	}

	if (PutData4(dwChgPwrWait) != TRUE) {	// ウェイト時間
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);			// 処理中断コード受信

		
	return ferr;
}
//=============================================================================
/**
 * 電源供給ソース情報取得(E2用)
 * @param pbSupply 電源供給ソース情報格納ポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetPwrSrc(BYTE* pbySupply)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;
	BYTE	bySupply;

	wCmdCode = BFWCMD_GET_PWRSRC;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData1(&bySupply) != TRUE){		// 電源供給ソース受信
		return FFWERR_COM;
	}
	*pbySupply = bySupply;


	ferr = ProtRcvHaltCode(&wBuf);			// 処理中断コード受信

	return ferr;
}

