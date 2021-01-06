///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_trg.cpp
 * @brief 外部トリガコマンドの実装ファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/10/20 新規作成 S.Nagai
*/
#include <windows.h>
#include <stdio.h>

#include "rfwasp_trg.h"
#include "doasp_trg.h"
#include "do_syserr.h"
#include "emudef.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"
#include "rfwasp_family.h"

///////////////////////////////////////////////////////////////////////////////
// RFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 外部トリガ入力の要因イベント設定
 * @param byChNo チャネル番号
 * @param byDetectType トリガ入力検出条件
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetExTriggerIn)(BYTE byChNo, BYTE byDetectType)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetExTriggerIn_log_enter(byChNo, byDetectType);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETEXTRGIN);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (byChNo > RF_ASPTRGIN_CH1) {
			// 範囲外のチャネル番号
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (byDetectType > RF_ASPTRGIN_BEDGE) {
			// 範囲外の検出条件
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetExTriggerIn(byChNo, byDetectType);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETEXTRGIN);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_SetExTriggerIn_log_leave(byChNo, byDetectType, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ入力の要因イベント参照
 * @param byChNo チャネル番号
 * @param pbyDetectType トリガ入力検出条件
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetExTriggerIn)(BYTE byChNo, BYTE *const pbyDetectType)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetExTriggerIn_log_enter(byChNo, pbyDetectType);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETEXTRGIN);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (byChNo > RF_ASPTRGIN_CH1) {
			// 範囲外のチャネル番号
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pbyDetectType == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetExTriggerIn(byChNo, pbyDetectType);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETEXTRGIN);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_GetExTriggerIn_log_leave(byChNo, pbyDetectType, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ入力の要因イベント削除
 * @param byChNo チャネル番号
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrExTriggerIn)(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrExTriggerIn_log_enter(byChNo);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLREXTRGIN);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (byChNo > RF_ASPTRGIN_CH1) {
			// 範囲外のチャネル番号
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrExTriggerIn(byChNo);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLREXTRGIN);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_ClrExTriggerIn_log_leave(byChNo, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ出力の動作設定
 * @param byChNo チャネル番号
 * @param pExTriggerOut 外部トリガ出力情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetExTriggerOut)(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetExTriggerOut_log_enter(byChNo, pExTriggerOut);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETEXTRGOUT);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (byChNo > RF_ASPTRGOUT_CH1) {
			// 範囲外のチャネル番号
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pExTriggerOut == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pExTriggerOut->byOutputType > RF_ASPTRGOUT_PULSE) {
			// 範囲外の出力信号タイプ
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pExTriggerOut->byOutputType == RF_ASPTRGOUT_PULSE) {
			if (pExTriggerOut->wPulseWidth < RF_ASPTRGPLS_MIN) {
				// 範囲外のパルス幅
				dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
			}
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetExTriggerOut(byChNo, pExTriggerOut);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETEXTRGOUT);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_SetExTriggerOut_log_leave(byChNo, pExTriggerOut, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ出力の動作参照
 * @param byChNo チャネル番号
 * @param pExTriggerOut 外部トリガ出力情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetExTriggerOut)(BYTE byChNo, RFW_EXTRGOUT_DATA *const pExTriggerOut)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetExTriggerOut_log_enter(byChNo, pExTriggerOut);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETEXTRGOUT);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (byChNo > RF_ASPTRGOUT_CH1) {
			// 範囲外のチャネル番号
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pExTriggerOut == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetExTriggerOut(byChNo, pExTriggerOut);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETEXTRGOUT);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_GetExTriggerOut_log_leave(byChNo, pExTriggerOut, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ出力の動作削除
 * @param byChNo チャネル番号
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrExTriggerOut)(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrExTriggerOut_log_enter(byChNo);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLREXTRGOUT);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (byChNo > RF_ASPTRGOUT_CH1) {
			// 範囲外のチャネル番号
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrExTriggerOut(byChNo);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLREXTRGOUT);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_ClrExTriggerOut_log_leave(byChNo, dwRet);
	return dwRet;
}


//=============================================================================
/**
* 外部トリガ出力のアクティブレベル設定
* @param byChNo チャネル番号
* @param byActiveLevel 外部トリガ出力アクティブレベル
* @retval RFWERR_OK  正常動作
* @retval RFWERR_ERR エラーあり
*/
//=============================================================================
DECLARE(RFWASP_SetExTriggerOutLevel)(BYTE byChNo, BYTE byActiveLevel)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetExTriggerOutLevel_log_enter(byChNo, byActiveLevel);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETEXTRGOUTLV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (byChNo > RF_ASPTRGOUT_CH1) {
			// 範囲外のチャネル番号
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (byActiveLevel > RF_ASPTRGOUT_LACTIVE) {
			// 範囲外のアクティブレベル
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetExTriggerOutLevel(byChNo, byActiveLevel);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETEXTRGOUTLV);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_SetExTriggerOutLevel_log_leave(byChNo, byActiveLevel, dwRet);
	return dwRet;
}

//=============================================================================
/**
* 外部トリガ出力のアクティブレベル参照
* @param byChNo チャネル番号
* @param pbyActiveLevel 外部トリガ出力アクティブレベル
* @retval RFWERR_OK  正常動作
* @retval RFWERR_ERR エラーあり
*/
//=============================================================================
DECLARE(RFWASP_GetExTriggerOutLevel)(BYTE byChNo, BYTE *const pbyActiveLevel)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetExTriggerOutLevel_log_enter(byChNo, pbyActiveLevel);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETEXTRGOUTLV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (byChNo > RF_ASPTRGOUT_CH1) {
			// 範囲外のチャネル番号
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pbyActiveLevel == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetExTriggerOutLevel(byChNo, pbyActiveLevel);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETEXTRGOUTLV);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_GetExTriggerOutLevel_log_leave(byChNo, pbyActiveLevel, dwRet);
	return dwRet;
}


