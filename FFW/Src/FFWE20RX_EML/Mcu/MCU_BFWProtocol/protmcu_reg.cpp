////////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_reg.cpp
 * @brief BFWコマンド プロトコル生成関数（レジスタ操作関連） ソースファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/09/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/10/30 上田
  FFW I/F仕様変更に伴うFFWソース変更。
  ・PROT_MCU_SetRXXREG(), PROT_MCU_GetRXXREG() 設定/一括参照時のレジスタ数変更
・RevRxNo180625-001,PB18047-CD01-001 2018/06/25 PA 辻
	RX66T-H/RX72T対応
*/

#include "protmcu_reg.h"
#include "prot_common.h"
#include "prot.h"
#include "comctrl.h"
#include "errchk.h"

// 2008.9.10 INSERT_BEGIN_E20RX600(+1) {
#include "ffwmcu_mcu.h"
// 2008.9.10 INSERT_END_E20RX600 }


// 2008.9.10 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * BFWRXCmd_SetXREG を送信し、レジスタ値の変更を行う。
 * @param pReg 設定・参照する全レジスタ値を格納するFFWMCU_REG_DATA_RX 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_MCU_SetRXXREG(const FFWMCU_REG_DATA_RX* pReg)
{
	int		eRet;
	int		i;
	WORD	wCmdCode;
	WORD	wData;
	DWORD	dwData;
	FFWERR	ferr;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_SET_XREG;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 設定するレジスタ値を送信
	// RevRxNo120910-001 Modify Line
	for (i = 0; i <= REG_NUM_RX_EXTB; ++i) {
		dwData = pReg->dwRegData[i];
		eRet = PutData4(dwData);
		if (!eRet) {
			return FFWERR_COM;
		}
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
 * BFWRXCCmd_GetXREG を送信し、レジスタ値の参照を行う。
 * @param bRegNum 取得するレジスタ番号
 * @param pReg 設定・参照する全レジスタ値を格納するFFWMCU_REG_DATA_RX 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_MCU_GetRXXREG(BYTE bRegNum, FFWMCU_REG_DATA_RX* pReg)
{
	int		eRet;
	int		i;
	WORD	wCmdCode;
	WORD	wData;
	DWORD	dwData;
	FFWERR	ferr;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_GET_XREG;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wData = static_cast<WORD>(bRegNum);
	eRet = PutData2(wData);						// レジスタ番号送信
	if (!eRet) {
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);				// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// レジスタ参照値受信
	if (bRegNum != REG_NUM_RX_ALL) {			// 個別参照
		eRet = GetData4(&dwData);
		if (!eRet) {
			return FFWERR_COM;
		}
		pReg->dwRegData[bRegNum] = dwData;
	} else {									// 一括参照
		// RevRxNo120910-001 Modify Line
		for (i = 0; i <= REG_NUM_RX_EXTB; ++i) {
			eRet = GetData4(&dwData);
			if (!eRet) {
				return FFWERR_COM;
			}
			pReg->dwRegData[i] = dwData;
		}
	}

	/* 処理中断コードを受信 */
	pwBuf = &wData;
	ferr = ProtRcvHaltCode(pwBuf);

	return ferr;

}
// 2008.9.10 INSERT_END_E20RX600 }
//=============================================================================
/**
 * レジスタ操作コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Reg(void)
{
	return;
}

// RevRxNo180625-001 Append Start
//==============================================================================
/**
* BFWRXCmd_SetREGBANK を送信し、レジスタ退避バンクの設定を行う。
* @param pRegBank 設定するレジスタ退避バンク情報を格納するFFWRX_REGBANK_DATA構造体アドレス
* @retval FFWエラーコード
*/
//==============================================================================
FFWERR PROT_MCU_SetREGBANK(const FFWRX_REGBANK_DATA* pRegBank)
{
	int		eRet;
	int		i;
	WORD	wCmdCode;
	BYTE	byData;
	WORD	wData;
	DWORD	dwData;
	FFWERR	ferr;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_SET_REGBANK;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 設定対象のバンク番号を送信
	byData = pRegBank->byRegBankNum;
	eRet = PutData1(byData);
	if (!eRet) {
		return FFWERR_COM;
	}

	// 設定するレジスタ退避バンクのレジスタ値を送信
	for (i = REGBANK_REG_NUM_R1; i <= REGBANK_REG_NUM_ACC1L; i++) {
		dwData = pRegBank->dwRegBankData[i];
		eRet = PutData4(dwData);
		if (!eRet) {
			return FFWERR_COM;
		}
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
* BFWRXCCmd_GetREGBANK を送信し、レジスタ退避バンクの参照を行う。
* @param pReg 参照するレジスタ退避バンク情報を格納するFFWRX_REGBANK_DATA構造体アドレス
* @retval FFWエラーコード
*/
//==============================================================================
FFWERR PROT_MCU_GetREGBANK(FFWRX_REGBANK_DATA* pRegBank)
{
	int		eRet;
	int		i;
	WORD	wCmdCode;
	BYTE	byData;
	WORD	wData;
	DWORD	dwData;
	FFWERR	ferr;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_GET_REGBANK;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 設定対象のバンク番号を送信
	byData = pRegBank->byRegBankNum;
	eRet = PutData1(byData);
	if (!eRet) {
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);				// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// レジスタ退避バンクのレジスタ参照値受信
	for (i = REGBANK_REG_NUM_R1; i <= REGBANK_REG_NUM_ACC1L; i++) {
		eRet = GetData4(&dwData);
		if (!eRet) {
			return FFWERR_COM;
		}
		pRegBank->dwRegBankData[i] = dwData;
	}

	/* 処理中断コードを受信 */
	pwBuf = &wData;
	ferr = ProtRcvHaltCode(pwBuf);

	return ferr;
}
// RevRxNo180625-001 Append End
