///////////////////////////////////////////////////////////////////////////////
/**
 * @file protasp_cpu.cpp
 * @brief BFWコマンド プロトコル生成関数(制御CPU空間アクセス関連)
 * @author RSD S.Nagai, REL K.Sasaki
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/10/26
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/09/09 新規作成
・2017/10/26 RXのFFW向けにcppソース用extern宣言を追加
*/
#include "protasp_cpu.h"
#include "prot.h"
#include "do_syserr.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"

// ファイル内static変数の宣言
#ifdef __cplusplus
extern int flashProtDataRcvDataN(DWORD dwSize);
#else
extern "C" int flashProtDataRcvDataN(DWORD dwSize);
#endif
//=============================================================================
/**
 * BFWCmd_AspInコマンドの発行
 * @param dwReadByte  リードバイト数
 * @param pbyReadData リードデータ格納バッファへのポインタ
 * @retval RFWERR_OK
 * @retval RFWERR_N_BFW_CMD
 */
//=============================================================================
DWORD ProtAspIn(DWORD dwReadByte, DWORD *const pbyReadData)
{
	DWORD	dwRet = RFWERR_ERR;

	// AspInコマンドを発行する。
	// エミュレータ種別はE2以外はエラーとする

	if (IsEmlE2() == TRUE) {
		dwRet = PROT_AspIn(dwReadByte, pbyReadData);
	} else {
		dwRet = RFWERR_N_BFW_CMD;
	}
	return dwRet;
}


//=============================================================================
/**
 * BFWCmd_AspOutコマンドの発行
 * @param dwWriteByte  ライトバイト数
 * @param pbyWriteData ライトデータ格納バッファへのポインタ
 * @retval RFWERR_OK
 * @retval RFWERR_N_BFW_CMD
 */
//=============================================================================
DWORD ProtAspOut(DWORD dwWriteByte, const DWORD* pbyWriteData)
{
	DWORD	dwRet = RFWERR_OK;

	// AspOutコマンドを発行する。
	// エミュレータ種別はE2以外はエラーとする

	if (IsEmlE2() == TRUE) {
		dwRet = PROT_AspOut(dwWriteByte, pbyWriteData);
	} else {
		dwRet = RFWERR_N_BFW_CMD;
	}

	return dwRet;
}

//=============================================================================
/**
 * BFWCmd_AspSetPortコマンドの発行
 * @param USB_RDY_N  USB_RDY_Nの設定値
 * @param BFW_RDY_N  BFW_RDY_Nの設定値
 * @retval RFWERR_OK
 * @retval RFWERR_N_BFW_CMD
 */
//=============================================================================
DWORD ProtAspSetPort(BYTE byUsbRdyN, BYTE byBfwRdyN)
{
	DWORD	dwRet = RFWERR_ERR;

	// AspSetPortコマンドを発行する。
	// エミュレータ種別はE2以外はエラーとする

	if (IsEmlE2() == TRUE) {
		dwRet = PROT_AspSetPort(byUsbRdyN, byBfwRdyN);
	} else {
		dwRet = RFWERR_N_BFW_CMD;
	}
	return dwRet;
}


//=============================================================================
/**
 * BFWCmd_AspInコマンドの発行
 *   リードデータから順に格納する。
 * @param dwReadByte リードバイト数
 * @param pbyReadData リードデータ格納バッファへのポインタ
 * @retval RFWERR_OK          正常終了
 * @retval RFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DWORD PROT_AspIn(DWORD dwReadByte, DWORD *const pbyReadData)
{
	DWORD	dwRet = RFWERR_OK;
	WORD	wCmdCode;
//	WORD	wBuf;

	wCmdCode = BFWCMD_ASP_IN;
	if (PutCmd(wCmdCode) == FALSE) {			// コマンドコード送信
		return RFWERR_D_COM;
	}
	if (PutData4(dwReadByte) == FALSE) {		// アクセス回数送信
		return RFWERR_D_COM;
	}
	FlashProtDataForced();

	if (GetDataN_H(dwReadByte, (BYTE * const)pbyReadData) == FALSE) {
		return RFWERR_D_COM;
	}

	flashProtDataRcvDataN(6);	// 予め6byte読んでおかないと動作しない...
	dwRet = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (dwRet != RFWERR_OK) {
		return dwRet;
	}

//	ProtRcvHaltCode(&wBuf);	// 処理中断コードを受信する

	return dwRet;
}


//=============================================================================
/**
 * BFWCmd_AspOutコマンドの発行
 * @param dwWriteByte ライトバイト数
 * @param pbyWriteData ライトデータ格納バッファへのポインタ
 * @retval RFWERR_OK          正常終了
 * @retval RFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DWORD PROT_AspOut(DWORD dwWriteByte, const DWORD* pbyWriteData)
{
	DWORD	dwRet = RFWERR_OK;
	WORD	wCmdCode;
	DWORD	wResCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_ASP_OUT;
	if (PutCmd(wCmdCode) == FALSE) {				// コマンドコード送信
		return RFWERR_D_COM;
	}
	if (PutData4(dwWriteByte) == FALSE) {
		return RFWERR_D_COM;
	}
	FlashProtDataForced();

	/* FIFO切替完了待ち */
	dwRet = GetData4(&wResCode);
	if (wResCode != (BFWCMD_FIFO_READY << 16)) {
		return RFWERR_D_COM;
	}

	SendBlock_ASP((const BYTE *) pbyWriteData, dwWriteByte);

	dwRet = ProtGetStatus(wCmdCode);	// エラーコード受信
	if (dwRet != RFWERR_OK) {
		return dwRet;
	}

	ProtRcvHaltCode(&wBuf);	// 処理中断コードを受信する

	return RFWERR_OK;
}

//=============================================================================
/**
 * BFWCmd_AspSetPortコマンドの発行
 *   リードデータから順に格納する。
 * @param byUsbRdyN  USB_RDY_Nの設定値
 * @param byBfwRdyN  BFW_RDY_Nの設定値
 * @retval RFWERR_OK          正常終了
 * @retval RFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DWORD PROT_AspSetPort(BYTE byUsbRdyN, BYTE byBfwRdyN)
{
	DWORD	dwRet = RFWERR_OK;
	WORD	wCmdCode;
//	WORD	wBuf;

	wCmdCode = BFWCMD_ASP_SETPORT;
	if (PutCmd(wCmdCode) == FALSE) {			// コマンドコード送信
		return RFWERR_D_COM;
	}
	if (PutData1(byUsbRdyN) == FALSE) {
		return RFWERR_D_COM;
	}
	if (PutData1(byBfwRdyN) == FALSE) {
		return RFWERR_D_COM;
	}
	FlashProtDataForced();

	flashProtDataRcvDataN(6);	// 予め6byte読んでおかないと動作しない...
	dwRet = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (dwRet != RFWERR_OK) {
		return dwRet;
	}

	//ProtRcvHaltCode(&wBuf);	// 処理中断コードを受信する

	return dwRet;
}

