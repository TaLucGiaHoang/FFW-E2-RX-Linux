///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_sys.cpp
 * @brief E2拡張機能システムコマンドの実装ファイル
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

#include "rfwasp_sys.h"
#include "doasp_sys.h"
#include "do_syserr.h"
#include "emudef.h"
#include "doasp_sys_family.h"
#include "rfwasp_family.h"

///////////////////////////////////////////////////////////////////////////////
// RFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * E2エミュレータ拡張機能のスイッチ設定
 * @param byAspSwitch 拡張機能のON/OFF情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetAspSwitch)(BYTE byAspSwitch)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetAspSwitch_log_enter(byAspSwitch);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETASPSW);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if ((byAspSwitch != RF_ASPSW_OFF)
		&&	(byAspSwitch != RF_ASPSW_ON)){
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetAspSw(byAspSwitch);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETASPSW);
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

	RFWASP_SetAspSwitch_log_leave(byAspSwitch, dwRet);

	return dwRet;
}

//=============================================================================
/**
 * E2エミュレータ拡張機能のスイッチの参照
 * @param pbyAspSwitch 拡張機能のON/OFF情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetAspSwitch)(BYTE *const pbyAspSwitch)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetAspSwitch_log_enter(pbyAspSwitch);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETASPSW);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (pbyAspSwitch == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetAspSw(pbyAspSwitch);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETASPSW);
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

	RFWASP_GetAspSwitch_log_leave(pbyAspSwitch, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * E2エミュレータ拡張機能サポート状況の参照
 * @param pdwAspComonFunction 共通機能のサポート状況
 * @param pdwAspSeriesFunction ファミリ固有のサポート状況
 * @param pdwAspStorage ストレージ関連のサポート状況
 * @param pdwAspOption オプションボード接続状況
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetAspFunction)(
	DWORD *const pdwAspComonFunction,
	DWORD *const pdwAspSeriesFunction,
	DWORD *const pdwAspStorage,
	DWORD *const pdwAspOption
){
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetAspFunction_log_enter(pdwAspComonFunction, pdwAspSeriesFunction, pdwAspStorage, pdwAspOption);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETASPFUNC);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if ((pdwAspComonFunction == NULL)
		||	(pdwAspSeriesFunction == NULL)
		||	(pdwAspStorage == NULL)
		||	(pdwAspOption == NULL)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetAspFunc(pdwAspComonFunction, pdwAspSeriesFunction, pdwAspStorage, pdwAspOption);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETASPFUNC);
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

	RFWASP_GetAspFunction_log_leave(pdwAspComonFunction, pdwAspSeriesFunction, pdwAspStorage, pdwAspOption, dwRet);
	return dwRet;
}


//=============================================================================
/*
* SetAspConfiguration
* @param pAspConfiguration 拡張機能のコンフィグ情報
* @retval RFWERR_OK
* @retval RFWERR_ERR
*/
//=============================================================================
DECLARE(RFWASP_SetAspConfiguration)(const RFW_ASPCONF_DATA* pAspConfiguration)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetAspConfiguration_log_enter(pAspConfiguration);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETASPCONFIG);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (!pAspConfiguration) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if ((pAspConfiguration->dwStorage != RF_ASPSTORAGE_NONE)
			&& (pAspConfiguration->dwStorage != RF_ASPSTORAGE_E2)
			&& (pAspConfiguration->dwStorage != RF_ASPSTORAGE_E2_FULLSTOP)
			&& (pAspConfiguration->dwStorage != RF_ASPSTORAGE_E2_FULLBREAK)
			&& (pAspConfiguration->dwStorage != RF_ASPSTORAGE_PC)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if ((pAspConfiguration->dwFunction0 & ~RF_ASPCONF0_MASK) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if ((pAspConfiguration->dwFunction1 & ~RF_ASPCONF1_MASK) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if ((pAspConfiguration->dwFunction2 & ~RF_ASPCONF2_MASK) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if ((pAspConfiguration->dwFunction3 & ~RF_ASPCONF3_MASK) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetAspConfiguration(pAspConfiguration);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETASPCONFIG);
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

	RFWASP_SetAspConfiguration_log_leave(pAspConfiguration, dwRet);
	return dwRet;
}

//=============================================================================
/*
* GetAspConfiguration
* @param pAspConfiguration 拡張機能のコンフィグ情報
* @return RFWERR_OK
* @return RFWERR_ERR
*/
//=============================================================================
DECLARE(RFWASP_GetAspConfiguration)(RFW_ASPCONF_DATA const* pAspConfiguration)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetAspConfiguration_log_enter(pAspConfiguration);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETASPCONFIG);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	if (dwRet == RFWERR_OK) {
		if (!pAspConfiguration) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetAspConfiguration(pAspConfiguration);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETASPCONFIG);
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

	RFWASP_GetAspConfiguration_log_leave(pAspConfiguration, dwRet);
	return dwRet;
}

