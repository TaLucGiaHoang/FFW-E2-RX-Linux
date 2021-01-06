///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_per.cpp
 * @brief 時間計測コマンドの実装ファイル
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

#include "rfwasp_per.h"
#include "doasp_per.h"
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
 * 時間計測条件の設定
 * @param byChNo チャネル番号
 * @param pAspPerformance 時間計測条件
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetPerformanceInfo)(BYTE byChNo, const RFW_ASPPERCOND_DATA* pAspPerformance)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetPerformanceInfo_log_enter(byChNo, pAspPerformance);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETPERINFO);
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
		if (byChNo > 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pAspPerformance == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if ((pAspPerformance->byMode & 0xFC) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pAspPerformance->u64Threshold > RF_ASPPER_THRESHOLD_MAX) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetAspPerCond(byChNo, pAspPerformance);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETPERINFO);
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

	RFWASP_SetPerformanceInfo_log_leave(byChNo, pAspPerformance, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 時間計測条件の参照
 * @param byChNo チャネル番号
 * @param pAspPerformance 時間計測条件
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetPerformanceInfo)(BYTE byChNo, RFW_ASPPERCOND_DATA *const pAspPerformance)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetPerformanceInfo_log_enter(byChNo, pAspPerformance);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETPERINFO);
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
		if (byChNo > 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pAspPerformance == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetAspPerCond(byChNo, pAspPerformance);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETPERINFO);
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

	RFWASP_GetPerformanceInfo_log_leave(byChNo, pAspPerformance, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 時間計測条件の消去
 * @param byChNo チャネル番号
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrPerformanceInfo)(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrPerformanceInfo_log_enter(byChNo);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRPERINFO);
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
		if (byChNo > 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrAspPerCond(byChNo);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRPERINFO);
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

	RFWASP_ClrPerformanceInfo_log_leave(byChNo, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 時間計測結果の参照
 * @param byChNo チャネル番号
 * @param pAspPerformanceData 時間計測結果
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetPerformanceData)(BYTE byChNo, RFW_ASPPER_DATA *const pAspPerformanceData)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetPerformanceData_log_enter(byChNo, pAspPerformanceData);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETPERDATA);
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
		if (byChNo > 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pAspPerformanceData == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetAspPerData(byChNo, pAspPerformanceData);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETPERDATA);
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

	RFWASP_GetPerformanceData_log_leave(byChNo, pAspPerformanceData, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * 時間計測結果の消去
 * @param byChNo チャネル番号
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrPerformanceData)(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrPerformanceData_log_enter(byChNo);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRPERDATA);
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
		if (byChNo > 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrAspPerData(byChNo);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRPERDATA);
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

	RFWASP_ClrPerformanceData_log_leave(byChNo, dwRet);
	return dwRet;
}

