///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_exe.cpp
 * @brief 実行系拡張機能イベントリンクコマンドの実装ファイル
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

#include "rfwasp_exe.h"
#include "doasp_exe.h"
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
 * E2エミュレータ拡張機能のブレークイベントの設定
 * @param u64FactorSingleEvent シングルE2トリガ
 * @param dwFactorMultiEvent マルチE2トリガ
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetBreakEvent)(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetBreakEvent_log_enter(u64FactorSingleEvent, dwFactorMultiEvent);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETBRKEV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetBreakEvent(u64FactorSingleEvent, dwFactorMultiEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETBRKEV);
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

	RFWASP_SetBreakEvent_log_leave(u64FactorSingleEvent, dwFactorMultiEvent, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * E2エミュレータ拡張機能のブレークイベントの参照
 * @param pu64FactorSingleEvent シングルE2トリガ
 * @param pdwFactorMultiEvent マルチE2トリガ
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetBreakEvent)(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetBreakEvent_log_enter(pu64FactorSingleEvent, pdwFactorMultiEvent);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETBRKEV);
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
		if (pu64FactorSingleEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pdwFactorMultiEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetBreakEvent(pu64FactorSingleEvent, pdwFactorMultiEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETBRKEV);
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

	RFWASP_GetBreakEvent_log_leave(pu64FactorSingleEvent, pdwFactorMultiEvent, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * E2エミュレータ拡張機能のブレークイベントの消去
 * @param なし
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrBreakEvent)(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrBreakEvent_log_enter();

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRBRKEV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrBreakEvent();
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRBRKEV);
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

	RFWASP_ClrBreakEvent_log_leave(dwRet);
	return dwRet;
}

//=============================================================================
/**
 * E2エミュレータ拡張機能のトレース停止イベントの設定
 * @param u64FactorSingleEvent シングルE2トリガ
 * @param dwFactorMultiEvent マルチE2トリガ
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetStopTraceEvent)(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetStopTraceEvent_log_enter(u64FactorSingleEvent, dwFactorMultiEvent);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETSTOPTRCEV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetStopTraceEvent(u64FactorSingleEvent, dwFactorMultiEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETSTOPTRCEV);
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

	RFWASP_SetStopTraceEvent_log_leave(u64FactorSingleEvent, dwFactorMultiEvent, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * E2エミュレータ拡張機能のトレース停止イベントの参照
 * @param pu64FactorSingleEvent シングルE2トリガ
 * @param pdwFactorMultiEvent マルチE2トリガ
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetStopTraceEvent)(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetStopTraceEvent_log_enter(pu64FactorSingleEvent, pdwFactorMultiEvent);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETSTOPTRCEV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// ダミー関数
	if (dwRet == RFWERR_OK) {
		if (pu64FactorSingleEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pdwFactorMultiEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetStopTraceEvent(pu64FactorSingleEvent, pdwFactorMultiEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETSTOPTRCEV);
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

	RFWASP_GetStopTraceEvent_log_leave(pu64FactorSingleEvent, pdwFactorMultiEvent, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * E2エミュレータ拡張機能のトレース停止イベントの消去
 * @param なし
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrStopTraceEvent)(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrStopTraceEvent_log_enter();

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRSTOPTRCEV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrStopTraceEvent();
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRSTOPTRCEV);
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

	RFWASP_ClrStopTraceEvent_log_leave(dwRet);
	return dwRet;
}

