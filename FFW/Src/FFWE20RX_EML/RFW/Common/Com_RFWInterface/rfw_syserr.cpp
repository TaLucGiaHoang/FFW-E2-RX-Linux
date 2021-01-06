////////////////////////////////////////////////////////////////////////////////
/**
 * @file rfw_syserr.cpp
 * @brief システムコマンドの実装ファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/09/09
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/09/09
*/
#include <windows.h>
#include <stdio.h>
#include <string.h>


#include "do_syserr.h"
//#include "rfw_log.h"

// ファイル内static変数の宣言



///////////////////////////////////////////////////////////////////////////////
// RFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * エラー番号、メッセージの取得
 * @param byMsgLanguage メッセージ文字列の言語
 * @param pdwErrType エラーメッセージの種別
 * @param pErrMsgInfo エラー番号、エラーメッセージ文字列を格納
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DECLARE(RFW_GetErrInfo)(BYTE byMsgLanguage, DWORD *const pdwErrType, RFW_ERRMSGINFO_DATA *const pErrMsgInfo)
{
	// entry log.
//	RFW_GetErrInfo_Log0(byMsgLanguage);

	DWORD dwRet = RFWERR_OK;
//	DWORD dwRetTmp = RFWERR_OK;
	DWORD dwCnt;
	DWORD* pdwRfwErrNoCore;			//エラー番号格納変数(コア個別エラー格納用)ポインタ
	DWORD* pdwRfwErrCoreInfo;		// エラー発生コア番号格納変数ポインタの取得

	RFW_ERRMSGINFO_DATA * pErrMsgInfoData;
	pErrMsgInfoData = pErrMsgInfo;
	WCHAR* swzRfwErrMsg;
	swzRfwErrMsg = new WCHAR [ERRMSGINFO_MSGSIZE_MAX + 1];

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
//	dwRet = CmdExecutCheck();
//	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
//		dwRet = SetCommErrNo(dwRet);
//	}

	if (dwRet == RFWERR_OK) {
		// 出力パラメータの値設定
		*pdwErrType = GetCommErrType();
		// 共通エラー
		if (*pdwErrType == RF_ERR_COMMON) {
			pErrMsgInfoData->dwCoreInfo = RF_PE1;
			pErrMsgInfoData->dwRfwErrNo = GetCommErrNo();
			GetCommErrMsg(pErrMsgInfoData->dwRfwErrNo, byMsgLanguage, swzRfwErrMsg);
			wcscpy_s(pErrMsgInfoData->swzRfwErrMsg, swzRfwErrMsg);
			// コア個別エラー
		} else if (*pdwErrType == RF_ERR_CORE) {

			pdwRfwErrNoCore = GetRfwErrNoCorePtr();		//エラー番号格納変数(コア個別エラー格納用)ポインタ
			pdwRfwErrCoreInfo = GetRfwErrCoreInfoPtr();	// エラー発生コア番号格納変数ポインタの取得

			// コア個別エラーの情報を登録
			for (dwCnt = 0; dwCnt < GetRfwErrNoCoreCnt(); dwCnt++) {
				pErrMsgInfoData->dwCoreInfo  = *(pdwRfwErrCoreInfo + dwCnt);
				pErrMsgInfoData->dwRfwErrNo  = *(pdwRfwErrNoCore + dwCnt);
				GetCommErrMsg(pErrMsgInfoData->dwRfwErrNo, byMsgLanguage, swzRfwErrMsg);
				wcscpy_s(pErrMsgInfoData->swzRfwErrMsg, swzRfwErrMsg);
				pErrMsgInfoData++;
			}
		} else {
			;
		}
	}

	// 排他制御のための終了制御
//	dwRetTmp = EndOfCmdExecut();
//	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
//		dwRet = SetCommErrNo(dwRetTmp);
//	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	// exit log.
//	RFW_GetErrInfo_Log1(dwRet, byMsgLanguage, pdwErrType, pErrMsgInfo);

	delete [] swzRfwErrMsg;

	return dwRet;
}

