///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_closed.cpp
 * @brief BFWコマンド プロトコル生成関数(非公開コマンド)
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/09/13
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#include <stdio.h>
#include <stdlib.h>

#include "prot_sys.h"
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "prot_cpu.h"
#include "errchk.h"

// 2008.9.2 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * BFWCmd_RXIRコマンドの発行
 * @param dwLength IRパスのビット数
 * @param pbyData IRパスでの送受信データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_RXIR(DWORD dwLength, BYTE* pbyData)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	DWORD	dwRcvData;
	DWORD	dwData;

	wCmdCode = BFWCMD_IR;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)dwLength) != TRUE) {		// ビット数送信
		return FFWERR_COM;
	}

	dwData = (*(pbyData) << 0) | (*(pbyData+1) << 8) | (*(pbyData+2) << 16) | (*(pbyData+3) << 24);
	if (PutData4(dwData) != TRUE) {		// ビット数送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	GetData4(&dwRcvData);
	*(pbyData++) = (BYTE)((dwRcvData >> 0)  & 0xFF);
	*(pbyData++) = (BYTE)((dwRcvData >> 8)  & 0xFF);
	*(pbyData++) = (BYTE)((dwRcvData >> 16) & 0xFF);
	*(pbyData++) = (BYTE)((dwRcvData >> 24) & 0xFF);

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信
 		
	return ferr;
}

//=============================================================================
/**
 * BFWCmd_RXDRコマンドの発行
 * @param dwLength DRパスのビット数
 * @param pbyData DRパスでの送受信データ
 * @param dwPause Pause-DR状態でのウェイト有無
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_RXDR(DWORD dwLength, BYTE* pbyData, DWORD dwPause)
{
	FFWERR	ferr;

	WORD	wCmdCode;
	BYTE*	pbyBuff;
	WORD	wBuf;
	DWORD	dwRcvData;
	DWORD	dwData;

	//ワーニング対策
	dwPause;

	wCmdCode = BFWCMD_DR;
	ferr = ProtSendCmd(wCmdCode);							// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)dwLength) != TRUE) {					// ビット数送信
		return FFWERR_COM;
	}

	dwData = (*(pbyData) << 0) | (*(pbyData+1) << 8) | (*(pbyData+2) << 16) | (*(pbyData+3) << 24);
	if (PutData4(dwData) != TRUE) {		// ビット数送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);							// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	pbyBuff = const_cast<BYTE*>(pbyData);

	GetData4(&dwRcvData);
	*(pbyData++) = (BYTE)((dwRcvData >> 0)  & 0xFF);
	*(pbyData++) = (BYTE)((dwRcvData >> 8)  & 0xFF);
	*(pbyData++) = (BYTE)((dwRcvData >> 16) & 0xFF);
	*(pbyData++) = (BYTE)((dwRcvData >> 24) & 0xFF);

	ferr = ProtRcvHaltCode(&wBuf);							// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_RXDRコマンドの発行 (Getのみ)
 * @param dwLength DRパスのビット数
 * @param pbyData DRパスでの送受信データ
 * @param dwPause Pause-DR状態でのウェイト有無
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_RXGetDR(DWORD dwLength, BYTE* pbyData, DWORD dwPause)
{
	FFWERR	ferr;

	WORD	wCmdCode;
	BYTE*	pbyBuff;
	WORD	wBuf;
	DWORD	dwRcvData;
	DWORD	dwData;

	//ワーニング対策
	dwPause;

	wCmdCode = BFWCMD_DR_GETONLY;
	ferr = ProtSendCmd(wCmdCode);							// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)dwLength) != TRUE) {					// ビット数送信
		return FFWERR_COM;
	}

	dwData = (*(pbyData) << 0) | (*(pbyData+1) << 8) | (*(pbyData+2) << 16) | (*(pbyData+3) << 24);
	if (PutData4(dwData) != TRUE) {		// ビット数送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);							// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	pbyBuff = const_cast<BYTE*>(pbyData);

	GetData4(&dwRcvData);
	*(pbyData++) = (BYTE)(((dwRcvData >> 0))  & 0xFF);
	*(pbyData++) = (BYTE)(((dwRcvData >> 8))  & 0xFF);
	*(pbyData++) = (BYTE)(((dwRcvData >> 16)) & 0xFF);
	*(pbyData++) = (BYTE)(((dwRcvData >> 24)) & 0xFF);

	ferr = ProtRcvHaltCode(&wBuf);							// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_RXSEMCコマンドの発行
 * @param dwLength 命令コードデータサイズ
 * @param pdwData 命令データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_RXSEMC(BYTE byLength, DWORD* pdwData)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	i;
	WORD	wBuf;

	wCmdCode = BFWCMD_SEMC;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)byLength) != TRUE) {		// ビット数送信
		return FFWERR_COM;
	}

	for(i=0; i<byLength; i++) {
		if (PutData4(*pdwData) != TRUE) {		// ビット数送信
			return FFWERR_COM;
		}
		pdwData++;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

		
	return ferr;
}

//=============================================================================
/**
 * BFWCmd_RXMASTコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_RXMAST(void)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_MAST;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

		
	return ferr;
}

//=============================================================================
/**
 * BFWCmd_RXGEMDコマンドの発行
 * @param pdwData C2E0データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_RXGEMD(DWORD* pdwData)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	DWORD	dwRcvData;

	wCmdCode = BFWCMD_GEMD;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	GetData4(&dwRcvData);
	*pdwData = dwRcvData;

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

		
	return ferr;
}

//=============================================================================
/**
 * BFWCmd_RXSEMPコマンドの発行
 * @param dwData C2E0への設定データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_RXSEMP(DWORD dwData)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_SEMP;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData4(dwData) != TRUE) {		// ビット数送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

		
	return ferr;
}

//=============================================================================
/**
 * BFWRXCmd_SetSARコマンドの発行
 * @param byCommand		設定するシリアルアクセスレジスタ番号
 * @param eAccessSize	シリアルアクセスレジスタアクセスサイズ
 * @param dwWriteData	シリアルアクセスレジスタへ書き込むデータ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_SetRXSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteData)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_SET_SAR;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)eAccessSize) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}

	if (PutData1(byCommand) != TRUE) {		// コマンド番号送信
		return FFWERR_COM;
	}

	if (PutData4(dwWriteData) != TRUE) {	// ライトデータ送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);			// 処理中断コード受信

	return ferr;
}


//=============================================================================
/**
 * BFWRXCmd_GetSARコマンドの発行
 * @param byCommand		設定するシリアルアクセスレジスタ番号
 * @param eAccessSize	シリアルアクセスレジスタアクセスサイズ
 * @param pdwReadData	シリアルアクセスレジスタから読み出したデータ格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_GetRXSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD *pdwReadData)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;
	DWORD	dwBuf;

	wCmdCode = BFWCMD_GET_SAR;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)eAccessSize) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}

	if (PutData1(byCommand) != TRUE) {		// コマンド番号送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData4(&dwBuf) != TRUE) {			// リードデータ受信
		return FFWERR_COM;
	}
	*pdwReadData = dwBuf;

	ferr = ProtRcvHaltCode(&wBuf);			// 処理中断コード受信

	return ferr;
}



//=============================================================================
/**
 * システムコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Closed(void)
{
	return;
}
