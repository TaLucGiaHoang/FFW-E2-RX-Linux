///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_closed.cpp
 * @brief 非公開コマンドの実装ファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/09/09 rfw_losed.cppからASP関連を移動 S.Nagai
*/
#include <windows.h>
#include <stdio.h>

#include "rfwasp_closed.h"
#include "doasp_closed.h"
#include "doasp_sample.h"
#include "do_syserr.h"
#include "emudef.h"
#include "asp_setting.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"

///////////////////////////////////////////////////////////////////////////////
// RFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ASP DTFIFO read
 * @param dwReadByte 読み込みバイト数(4byte単位)
 * @param pbyReadBuff リードデータ格納バッファアドレス
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFW_AspIn)(DWORD dwReadByte, DWORD *const pbyReadBuff)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_ASPIN);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	if (dwRet == RFWERR_OK) {
		if (dwReadByte % sizeof(DWORD)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspIn(dwReadByte, pbyReadBuff);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_ASPIN);
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
 * Asp DTFIFO write
 * @param dwReadByte 書込みバイト数(4byte単位)
 * @param pbyWriteBuff ライトデータ格納バッファアドレス
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFW_AspOut)(DWORD dwWriteByte, const DWORD* pbyWriteBuff)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_ASPOUT);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	if (dwRet == RFWERR_OK) {
		if (dwWriteByte % sizeof(DWORD)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspOut(dwWriteByte, pbyWriteBuff);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_ASPOUT);
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
 * VendorControl IN
 * @param req
 * @param val
 * @param index
 * @param pbyIn
 * @param len
 * @retval RFWERR_OK 正常終了
 * @retval RFWERR_ERR 送信失敗
 */
 //=============================================================================
DECLARE(RFW_VendorControlIn)(
    BYTE req,
    WORD val,
    WORD index,
    BYTE *pbyIn,
    WORD len)
{
	int rc;

	rc = COM_VendorControlIn(req, val, index, pbyIn, len);
	if (rc == TRUE) {
		return RFWERR_OK;
	} else {
		return RFWERR_ERR;
	}
}

//=============================================================================
/*
 * VendorControl Out
 * @param req
 * @param val
 * @param index
 * @retval RFWERR_OK 正常終了
 * @retval RFWERR_ERR 送信失敗
 */
 //=============================================================================
DECLARE(RFW_VendorControlOut)(
    BYTE req,
    WORD val,
    WORD index)
{
	int rc;

	rc = COM_VendorControlOut(req, val, index, 0, 0);
	if (rc == TRUE) {
		return RFWERR_OK;
	} else {
		return RFWERR_ERR;
	}
}


