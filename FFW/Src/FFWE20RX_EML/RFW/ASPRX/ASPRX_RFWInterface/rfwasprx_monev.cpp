///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasprx_monev.h
 * @brief モニタイベントコマンドのソースファイル
 * @author REL M.Yamamoto, PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
 ///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/10/28 新規作成 M.Yamamoto
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#include <windows.h>
#include <stdio.h>


#include "rfwasprx_monev.h"
#include "doasprx_monev.h"
#include "do_syserr.h"
#include "emudef.h"
#include "rfwasp_RX.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"


///////////////////////////////////////////////////////////////////////////////
// RFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 拡張機能のモニタイベント条件の設定
 * @param byEventNo 
 * @param pAspMonitorEvent 
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetMonitorEvent)(BYTE byEventNo, const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent)
{
	DWORD	dwRet, dwRetTmp;

	/* ワーニング情報のクリア */
	ClrWarning();

	/* 排他制御のチェック */
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETMONITOREV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}


	/* 拡張機能が有効かチェック */
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}
	
	/* パラメータチェック */
	if (dwRet == RFWERR_OK) {
		if (byEventNo < RF_ASPMONEV_EVNO_MIN) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		if (pAspMonitorEvent == NULL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		if (pAspMonitorEvent->dwFuncMode == 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
		// RevRxE2No171004-001 Modify Line
		if (pAspMonitorEvent->byDetectType != RF_ASPMONEV_TYPE_SWBRK) {
				dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}
	
	/* イベント本数よりも多いイベント設定か */
	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetMonitorEvent(byEventNo, pAspMonitorEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}
		
	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETMONITOREV);
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
	return dwRet;
}

//=============================================================================
/**
 * 拡張機能のモニタイベント条件の参照
 * @param byEventNo 
 * @param pAspMonitorEvent 
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetMonitorEvent)(BYTE byEventNo, RFW_ASPMONEVCOND_DATA *const pAspMonitorEvent)
{
	DWORD	dwRet, dwRetTmp;
	
	/* ワーニング情報のクリア */
	ClrWarning();

	/* 排他制御のチェック */
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETMONITOREV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	/* 拡張機能が有効かチェック */
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}
	
	/* パラメータチェック */
	if (byEventNo < RF_ASPMONEV_EVNO_MIN) {			// RevRL78No170420-001-02
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}
	if (pAspMonitorEvent == NULL) {
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}
	
	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetMonitorEvent(byEventNo, pAspMonitorEvent);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETMONITOREV);
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

	return dwRet;
}

//=============================================================================
/**
 * 拡張機能のモニタイベント条件の消去
 * @param byChNo 
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrMonitorEvent)(BYTE byEventNo)
{
	DWORD dwRet, dwRetTmp;

	/* ワーニング情報のクリア */
	ClrWarning();

	/* 排他制御のチェック */
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRMONITOREV);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	/* 拡張機能が有効かチェック */
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}
	
	/* パラメータチェック */
	if (byEventNo < RF_ASPMONEV_EVNO_MIN) {		// RevRL78No170420-001-02
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrMonitorEvent(byEventNo);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRMONITOREV);
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


	return dwRet;
}

//=============================================================================
/**
 * モニタイベントのE2トリガ設定
 * @param byChNo 
 * @param byData
 * @param byMask
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_SetMonitorEventFactor)(BYTE byChNo, BYTE byData,	BYTE byMask)
{
	DWORD	dwRet, dwRetTmp;

	/* ワーニング情報のクリア */
	ClrWarning();

	/* 排他制御のチェック */
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETMONITOREVFACT);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	/* 拡張機能が有効かチェック */
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}
	
	/* パラメータチェック */
	if (byChNo > MONEV_TRG_CH6) {
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetMonitorEventFactor(byChNo, byData, byMask);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETMONITOREVFACT);
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



	return dwRet;
}

//=============================================================================
/**
 * モニタイベントのE2トリガ参照
 * @param byEventNo 
 * @param pbyData
 * @param pbyMask
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetMonitorEventFactor)(BYTE byChNo, BYTE *const pbyData, BYTE *const pbyMask)

{
	DWORD	dwRet, dwRetTmp;
	
	/* ワーニング情報のクリア */
	ClrWarning();

	/* 排他制御のチェック */
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETMONITOREVFACT);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	/* 拡張機能が有効かチェック */
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	/* パラメータチェック */
	if (byChNo > MONEV_TRG_CH6) {
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}
	if (pbyData == NULL) {
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}
	if (pbyMask == NULL) {
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetMonitorEventFactor(byChNo, pbyData, pbyMask);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETMONITOREVFACT);
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



	return dwRet;
}

//=============================================================================
/**
 * モニタイベントのE2トリガ消去
 * @param byChNo 
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_ClrMonitorEventFactor)(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	/* ワーニング情報のクリア */
	ClrWarning();

	/* 排他制御のチェック */
	dwRet = CmdExecutCheck_ASP(ASPCMD_CLRMONITOREVFACT);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	/* 拡張機能が有効かチェック */
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	/* パラメータチェック */
	if (byChNo > MONEV_TRG_CH6) {
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_ClrMonitorEventFactor(byChNo);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}
	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_CLRMONITOREVFACT);
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



	return dwRet;
}

//=============================================================================
/**
* モニタイベントの実行結果データを参照する。
* @param byDataType	結果データタイプ
* @param byEventNo	参照対象のモニタイベント番号
* @param pu64Data1	結果データ1
* @param pu64Data2	結果データ2(予約)
* @retval RFWERR_OK  正常動作
* @retval RFWERR_ERR エラーあり
*/
//=============================================================================
DECLARE(RFWASP_GetMonitorEventData)(BYTE byDataType, BYTE byEventNo, UINT64 *const pu64Data1, UINT64 *const pu64Data2)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	/* ワーニング情報のクリア */
	ClrWarning();

	/* 排他制御のチェック */
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETMONITOREVDATA);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	/* 拡張機能が有効かチェック */
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	/* パラメータチェック */
	if (byDataType > RF_MONEVDATA_PASSCOUNT) {
		dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetMonitorEventData(byDataType, byEventNo, pu64Data1, pu64Data2);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}
	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETMONITOREVDATA);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	return dwRet;
}