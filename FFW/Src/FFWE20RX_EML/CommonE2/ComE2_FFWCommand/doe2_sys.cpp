///////////////////////////////////////////////////////////////////////////////
/**
 * @file doe2_sys.cpp
 * @brief システムコマンドの実装ファイル
 * @author PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/07/24
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
*/
#include "doe2_sys.h"
#include "prote2_sys.h"
#include "prot_common.h"


///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ターゲットシステムへの電源供給(リニア指定)(E2用)
 * @param eExPwr2Mode 電源供給モード
 * @param byExPwr2Val 電源供給電圧
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_EXPWR2(enum FFWENM_EXPWR2_MODE eExPwr2Mode, BYTE byExPwr2Val)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	FFWENM_EXPWR2_SUPPLY	eExPwr2Sply;
	WORD	wExPwr2Val;

	ProtInit();

	// 二系統電源供給要否設定
	eExPwr2Sply = EXPWR2_SINGLE_POWER;	// RXでは電源供給モードにかかわらず、単一電源供給を設定する

	wExPwr2Val = (WORD)(byExPwr2Val * 10);	// 電源供給電圧設定値変換(FFW指定値->BFW LV0指定値)

	// ターゲットシステムへの電源供給(BFW LV0のEXTPWR2コマンド実行)
	ferr = PROT_EXPWR2(eExPwr2Mode, wExPwr2Val, eExPwr2Sply);
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
 * ターゲットシステムへの電源供給電圧の変更(リニア指定)(E2用)
 * @param byChgPwrVal 電源供給電圧
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_CHGPWR(BYTE byChgPwrVal)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	WORD	wChgPwrVal;
	DWORD	dwChgPwrWait;

	ProtInit();

	wChgPwrVal = (WORD)(byChgPwrVal * 10);	// 電源供給電圧設定値変換(FFW指定値->BFW(LV0)指定値)
	dwChgPwrWait = CHGPWR_WAIT_TIME;		// 電源供給電圧変更ウェイト時間設定

	// ターゲットシステムへの電源供給(BFW LV0のCHGPWRコマンド実行)
	ferr = PROT_CHGPWR(wChgPwrVal, dwChgPwrWait);
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
 * 電源供給ソース情報の取得(E2用)
 * @param pbSupply 電源供給ソース情報格納ポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetPwrSrc(BYTE* pbySupply)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	// 電源供給ソース情報の取得
	ferr = PROT_GetPwrSrc(pbySupply);
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

