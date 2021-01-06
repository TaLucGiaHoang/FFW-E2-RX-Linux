///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_dc.cpp
 * @brief BFWコマンド プロトコル生成関数(デバッグコンソール)
 * @author RSO H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#include "protmcu_dc.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "prot_common.h"
#include "ffw_sys.h"

//=============================================================================
/**
 * C2Eレジスタ内のデータを読み込む
 * @param pC2E    C2Eレジスタ値を格納
 * @param pbyData C2Eレジスタ内のデータを格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetC2E(FFWRX_C2E_DATA* pC2E, BYTE* pbyData)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	BYTE	byData;
	WORD	wData;

	FFWENM_EACCESS_SIZE	eAccessSize;

	wCmdCode = BFWCMD_C2E;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wData = 0x0000;
	if (PutData2(wData) != TRUE) {					// リザーブ送信
		return FFWERR_COM;
	}

	switch (pC2E->dwRegNo & 0x03) {
	case 0x01:
		byData = 0;
		break;
	case 0x02:
		byData = 1;
		break;
	default:
		// あり得ない
		byData = 1;
		break;
	}
	if (PutData1(byData) != TRUE) {				// C2EのCh No.
		return FFWERR_COM;
	}

	eAccessSize = EBYTE_ACCESS;
	byData = static_cast<BYTE>(eAccessSize);
	if (PutData1(byData) != TRUE) {				// アクセスサイズ
		return FFWERR_COM;
	}

	if (PutData4(pC2E->dwLength) != TRUE) {		// 取得サイズ
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// データ受信
	if (GetDataN(pC2E->dwLength, pbyData) != TRUE) {
		return FFWERR_COM;
	}

	ferr = ProtRcvHaltCode(&wData);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * E2Cレジスタにデータを書き込む
 * @param pE2C    E2Cレジスタ値を格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_SetE2C(FFWRX_E2C_DATA* pE2C)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	BYTE	byData;
	WORD	wData;

	FFWENM_EACCESS_SIZE	eAccessSize;

	wCmdCode = BFWCMD_E2C;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wData = 0x0000;
	if (PutData2(wData) != TRUE) {					// リザーブ送信
		return FFWERR_COM;
	}

	switch (pE2C->dwRegNo & 0x0F) {
	case 0x01:
		byData = 0;
		break;
	case 0x02:
		byData = 1;
		break;
	default:
		// あり得ない
		byData = 0;
		break;
	}
	if (PutData1(byData) != TRUE) {				// C2EのCh No.
		return FFWERR_COM;
	}

	eAccessSize = EBYTE_ACCESS;
	byData = static_cast<BYTE>(eAccessSize);
	if (PutData1(byData) != TRUE) {				// アクセスサイズ
		return FFWERR_COM;
	}

	if (PutData4(pE2C->dwLength) != TRUE) {		// 設定データ数
		return FFWERR_COM;
	}

	// データ送信
	if (PutDataN(pE2C->dwLength, pE2C->pbyData) != TRUE) {
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);				// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wData);	// 処理中断コード受信

	return ferr;
}


//=============================================================================
/**
 * C2E, E2Cレジスタとバッファをクリア
 * @param dwRegNo   C2E,E2C Ch番号
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_ClrC2EE2C(DWORD dwRegNo)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	BYTE	byData;
	WORD	wData;

	FFWENM_EACCESS_SIZE	eAccessSize;

	wCmdCode = BFWCMD_C2E2CCLR;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wData = 0x0000;
	if (PutData2(wData) != TRUE) {					// リザーブ送信
		return FFWERR_COM;
	}

	byData = static_cast<BYTE>(dwRegNo);
	if (PutData1(byData) != TRUE) {				// C2EのCh No.
		return FFWERR_COM;
	}

	eAccessSize = ELWORD_ACCESS;
	byData = static_cast<BYTE>(eAccessSize);
	if (PutData1(byData) != TRUE) {				// アクセスサイズ
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);				// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wData);	// 処理中断コード受信

	return ferr;

}

//=============================================================================
/**
 * デバッグコンソールコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Dc(void)
{
	return;
}
