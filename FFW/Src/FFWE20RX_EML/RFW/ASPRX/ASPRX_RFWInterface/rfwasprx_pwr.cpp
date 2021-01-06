///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasprx_pwr.cpp
 * @brief パワーモニタコマンドの実装ファイル(RX固有部)
 * @author REL H.Ohga
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/20
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2017/10/20 新規作成 H.Ohga
*/
#include <windows.h>
#include <stdio.h>

#include "rfwasprx_pwr.h"
#include "doasprx_pwr.h"
#include "do_syserr.h"
#include "emudef.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"
#include "rfwasp_family.h"
///////////////////////////////////////////////////////////////////////////////
// RFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================

//=============================================================================
/**
 * パワーモニタのサンプリングデータ範囲参照
 * @param pdwUAmin パワーモニタのサンプリングデータの最小値情報
 * @param pdwUAmax パワーモニタのサンプリングデータの最大値情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFWASP_GetPowerMonitorDataRange)(DWORD *const pdwUAmin, DWORD *const pdwUAmax)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	*pdwUAmin = 0;
	*pdwUAmax = 0;

	RFWASP_GetPowerMonitorDataRange_log_enter(pdwUAmin, pdwUAmax);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETPWRMONRNG);
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
		dwRet = DO_GetPwrMonDataRange(pdwUAmin, pdwUAmax);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETPWRMONRNG);
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

	RFWASP_GetPowerMonitorDataRange_log_leave(pdwUAmin, pdwUAmax, dwRet);

	return dwRet;
}
