///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_pwr.cpp
 * @brief パワーモニタコマンドの実装ファイル
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

#include "rfwasp_pwr.h"
#include "doasp_pwr.h"
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
 * パワーモニタのサンプリング設定
 * @param pPowerMonitor パワーモニタサンプリング情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetPowerMonitor)(const RFW_PWRMONSAMP_DATA* pPowerMonitor)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetPowerMonitor_log_enter(pPowerMonitor);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETPWRMON);
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
		if (pPowerMonitor == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pPowerMonitor->bySamplingRate > RF_ASPPWR_REG_1M) {
			// 範囲外のサンプリングレート
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pPowerMonitor->bySamplingMode > RF_ASPPWR_EVLINK) {
			// 範囲外のサンプリングモード
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
		if (dwRet == RFWERR_OK) {
			/* フィルタ機能は削除された
			if (pPowerMonitor->wFilterMin > pPowerMonitor->wFilterMax) {
				// フィルタの大小関係がおかしい
				dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
			}*/
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetPwrMon(pPowerMonitor);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETPWRMON);
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

	RFWASP_SetPowerMonitor_log_leave(pPowerMonitor, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * パワーモニタのサンプリング設定参照
 * @param pPowerMonitor パワーモニタサンプリング情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetPowerMonitor)(RFW_PWRMONSAMP_DATA *const pPowerMonitor)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetPowerMonitor_log_enter(pPowerMonitor);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETPWRMON);
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
		if (pPowerMonitor == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetPwrMon(pPowerMonitor);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETPWRMON);
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

	RFWASP_GetPowerMonitor_log_leave(pPowerMonitor, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * パワーモニタのサンプリング設定消去
 * @param なし
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrPowerMonitor)(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrPowerMonitor_log_enter();

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRPWRMON);
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
		dwRet = DO_ClrPwrMon();
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRPWRMON);
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

	RFWASP_ClrPowerMonitor_log_leave(dwRet);
	return dwRet;
}

//=============================================================================
/**
 * パワーモニタイベント設定
 * @param pPowerMonitorEvent パワーモニタイベント情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetPowerMonitorEvent)(const RFW_PWRMONEV_DATA* pPowerMonitorEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetPowerMonitorEvent_log_enter(pPowerMonitorEvent);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETPWRMONEV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// 大小比較チェック(wPowerMin < wPowerMax)(wPowerWidthMin < wPowerWidthMax)
	if (dwRet == RFWERR_OK) {
		if (pPowerMonitorEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pPowerMonitorEvent->byDetectType > RF_ASPPWREV_PULSE_GTE) {
			// 検出タイプのチェック
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pPowerMonitorEvent->wPowerMin >= pPowerMonitorEvent->wPowerMax) {
			// 範囲指定の大小比較チェック
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pPowerMonitorEvent->byDetectType == RF_ASPPWREV_PULSE_LTE) {
			// PULSE_LTEのとき、
			if (pPowerMonitorEvent->wPulseWidthMin >= pPowerMonitorEvent->wPulseWidthMax) {
				dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
			}
		} else if (pPowerMonitorEvent->byDetectType == RF_ASPPWREV_PULSE_GTE) {
			if (pPowerMonitorEvent->wPulseWidthMax == 0) {
				dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
			}
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetPwrMonEvent(pPowerMonitorEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETPWRMONEV);
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

	RFWASP_SetPowerMonitorEvent_log_leave(pPowerMonitorEvent, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * パワーモニタイベント参照
 * @param pPowerMonitorEvent パワーモニタイベント情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetPowerMonitorEvent)(RFW_PWRMONEV_DATA *const pPowerMonitorEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetPowerMonitorEvent_log_enter(pPowerMonitorEvent);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETPWRMONEV);
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
		if (pPowerMonitorEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetPwrMonEvent(pPowerMonitorEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETPWRMONEV);
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

	RFWASP_GetPowerMonitorEvent_log_leave(pPowerMonitorEvent, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * パワーモニタイベント消去
 * @param  なし
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrPowerMonitorEvent)(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrPowerMonitorEvent_log_enter();

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRPWRMONEV);
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
		dwRet = DO_ClrPwrMonEvent();
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRPWRMONEV);
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

	RFWASP_ClrPowerMonitorEvent_log_leave(dwRet);
	return dwRet;
}

