////////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_clk.cpp
 * @brief BFWコマンド プロトコル生成関数（クロック関連） ソースファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#include "protmcu_clk.h"
#include "prot_common.h"
#include "prot.h"
#include "comctrl.h"
#include "errchk.h"

//==============================================================================
/**
 * FFWはコマンドコードBFWCmd_SetJtagClkを送信し、JTAGクロック周波数を設定する。
 * @param byJtagClk 設定するTCK周波数
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM BFW 通信異常発生
 */
//==============================================================================
FFWERR PROT_SetJtagClk(BYTE byJtagClk)
{

	int		eRet;
	WORD	wCmdCode;
	FFWERR	ferr;
	BYTE	byData;
	WORD	wData;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_SET_JTAGCLK;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	byData = byJtagClk;
	eRet = PutData1(byData);				// 周波数設定値送信
	if (!eRet) {
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	/* 処理中断コードを受信 */
	pwBuf = &wData;
	ferr = ProtRcvHaltCode(pwBuf);

	return ferr;

}


//==============================================================================
/**
 * FFWはコマンドコードBFWCmd_GetJtagCLKを送信し、エミュレータ生成クロック周波数を取得する。
 * @param pbyJtagClk 取得したTCK周波数
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM BFW 通信異常発生
 */
//==============================================================================
FFWERR PROT_GetJtagClk(BYTE *pbyJtagClk)
{

	int		eRet;
	WORD	wCmdCode;
	FFWERR	ferr;
	WORD	wData;
	WORD*	pwBuf;
	BYTE	byData;

	wCmdCode = BFWCMD_GET_JTAGCLK;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	eRet = GetData1(&byData);				// クロック周波数受信
	if (!eRet) {
		return FFWERR_COM;
	}
	*pbyJtagClk = byData;

	/* 処理中断コードを受信 */
	pwBuf = &wData;
	ferr = ProtRcvHaltCode(pwBuf);

	return ferr;

}

//=============================================================================
/**
 * クロック関連コマンド用変数初期化
 * @param なし
 * @return なし
 */
//=============================================================================
void InitProtMcuData_Clk(void)
{
	return;
}

