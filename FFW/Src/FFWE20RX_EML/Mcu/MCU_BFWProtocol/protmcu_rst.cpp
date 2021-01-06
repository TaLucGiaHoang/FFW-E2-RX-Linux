///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_rst.cpp
 * @brief BFWコマンド プロトコル生成関数(リセットコマンド)
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/09/05
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130301-001	2013/09/05 上田
	RX64M対応
*/
#include "protmcu_mcu.h"
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "ffwmcu_mcu.h"
//==============================================================================
/**
 * FFWはコマンドコードBFWMCUCmd_RESTを送信し、ターゲットMCUのH/Wリセットを行う。
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_MCU_REST(void)
{

	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_REST;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	/* 処理中断コードを受信 */
	pwBuf = &wData;
	ferr = ProtRcvHaltCode(pwBuf);

	return ferr;

}
//=============================================================================
/**
 * BFWMCUCmd_SRESTコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
//RevNo100715-028 Modify Line
FFWERR PROT_MCU_SREST(BYTE* byResult)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_SREST;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	
	//RevNo100715-028 Append Start
	if (GetData1(byResult) != TRUE) {		// 認証結果受信
		return FFWERR_COM;
	}
	//RevNo100715-028 Append End

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

// RevRxNo130301-001 Append Start
//==============================================================================
/**
 * BFWRXCmd_DBGRESTコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_MCU_DBGREST(void)
{

	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_DBGREST;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	/* 処理中断コードを受信 */
	pwBuf = &wData;
	ferr = ProtRcvHaltCode(pwBuf);

	return ferr;

}
// RevRxNo130301-001 Append End

//=============================================================================
/**
 * リセットコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Rst(void)
{
	return;
}
