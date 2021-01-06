///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_can.cpp
 * @brief CANモニタコマンドの実装ファイル
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

#include "rfwasp_can.h"
#include "doasp_can.h"
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
 * CANモニタのサンプリング設定
 * @param byChNo チャネル番号
 * @param pCanMonitor CANモニタサンプリング情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetCANMonitor)(BYTE byChNo, const RFW_CANMON_DATA* pCanMonitor)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetCANMonitor_log_enter(byChNo, pCanMonitor);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETCANMON);
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
		if (byChNo > RF_ASPCANMON_CH1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if (pCanMonitor == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if ((pCanMonitor->byCanMode != RF_ASPCAN_STANDARD_ID)
			&&	 (pCanMonitor->byCanMode != RF_ASPCAN_EXTENDED_ID)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if ((pCanMonitor->wBaudrate != RF_ASPCAN_1MBPS)
			&&	 (pCanMonitor->wBaudrate != RF_ASPCAN_500KBPS)
			&&	 (pCanMonitor->wBaudrate != RF_ASPCAN_250KBPS)
			&&	 (pCanMonitor->wBaudrate != RF_ASPCAN_125KBPS)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else if ((pCanMonitor->bySamplingPoint < 1) || (pCanMonitor->bySamplingPoint > 100)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		else {
			// これ以上のパラメータチェックはなし
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetCanMon(byChNo, pCanMonitor);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETCANMON);
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

	RFWASP_SetCANMonitor_log_leave(byChNo, pCanMonitor, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * CANモニタのサンプリング設定参照
 * @param byChNo チャネル番号
 * @param pCanMonitor CANモニタサンプリング情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetCANMonitor)(BYTE byChNo, RFW_CANMON_DATA *const pCanMonitor)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetCANMonitor_log_enter(byChNo, pCanMonitor);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETCANMON);
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
		if (byChNo > RF_ASPCANMON_CH1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pCanMonitor == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetCanMon(byChNo, pCanMonitor);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETCANMON);
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

	RFWASP_GetCANMonitor_log_leave(byChNo, pCanMonitor, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * CANモニタのサンプリング設定消去
 * @param byChNo チャネル番号
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrCANMonitor)(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrCANMonitor_log_enter(byChNo);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRCANMON);
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
		if (byChNo > RF_ASPCANMON_CH1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrCanMon(byChNo);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRCANMON);
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

	RFWASP_ClrCANMonitor_log_leave(byChNo, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * CANモニタイベントの設定
 * @param byChNo チャネル番号
 * @param pCanMonitorEvent CANモニタイベント情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetCANMonitorEvent)(BYTE byChNo, const RFW_CANMONEV_DATA* pCanMonitorEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetCANMonitorEvent_log_enter(byChNo, pCanMonitorEvent);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETCANMONEV);
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
		if (byChNo > RF_ASPCANMON_CH1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pCanMonitorEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pCanMonitorEvent->byDetectType != RF_ASPCAN_MATCH_P) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pCanMonitorEvent->byDelayCount == 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if ((pCanMonitorEvent->byCanMode != RF_ASPCAN_STANDARD_ID)
			&& (pCanMonitorEvent->byCanMode != RF_ASPCAN_EXTENDED_ID)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if ((pCanMonitorEvent->byDLC & 0xF0) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if ((pCanMonitorEvent->byRTR & 0xFE) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if ((pCanMonitorEvent->bySRR & 0xFE) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetCanMonEvent(byChNo, pCanMonitorEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}
	
	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETCANMONEV);
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

	RFWASP_SetCANMonitorEvent_log_leave(byChNo, pCanMonitorEvent, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * CANモニタイベントの参照
 * @param byChNo チャネル番号
 * @param pCanMonitorEvent CANモニタイベント情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetCANMonitorEvent)(BYTE byChNo, RFW_CANMONEV_DATA *const pCanMonitorEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetCANMonitorEvent_log_enter(byChNo, pCanMonitorEvent);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETCANMONEV);
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
		if (byChNo > RF_ASPCANMON_CH1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (pCanMonitorEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetCanMonEvent(byChNo, pCanMonitorEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETCANMONEV);
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

	RFWASP_GetCANMonitorEvent_log_leave(byChNo, pCanMonitorEvent, dwRet);
	return dwRet;
}

//=============================================================================
/**
 * CANモニタイベントの消去
 * @param byChNo チャネル番号
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrCANMonitorEvent)(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_ClrCANMonitorEvent_log_enter(byChNo);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRCANMONEV);
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
		if (byChNo > RF_ASPCANMON_CH1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrCanMonEvent(byChNo);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRCANMONEV);
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

	RFWASP_ClrCANMonitorEvent_log_leave(byChNo, dwRet);
	return dwRet;
}

