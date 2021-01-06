////////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_sci.cpp
 * @brief BFWコマンド プロトコル生成関数（シリアル関連)
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
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
#include "protmcu_sci.h"
#include "prot_common.h"
#include "prot.h"
#include "comctrl.h"
#include "errchk.h"
//==============================================================================
/**
 * FFWはコマンドコードBFWCMD_SET_SCIBRを送信し、E1/E20-MCU間のシリアル通信ボーレ
   ートを設定する。
 * @param wBaudId 通信ボーレート設定値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_SetSCIBR( DWORD dwBaudId )
{
	int		eRet;
	WORD	wCmdCode;
	DWORD	dwData;
	WORD	wData;
	FFWERR	ferr;

	WORD*	pwBuf;

	// コマンドコード送信処理
	wCmdCode = BFWCMD_SET_SCIBR;
	ferr = ProtSendCmd( wCmdCode );
	if( ferr != FFWERR_OK ){
		// エラーの場合は処理を中断
		return ferr;
	}

	// シリアル通信ボーレートID送信
	dwData = dwBaudId;
	eRet = PutData4( dwData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// パケット受信処理（ステータスまで受信）
	ferr = ProtGetStatus( wCmdCode );
	if( ferr != FFWERR_OK ){
		return ferr;
	}

	// 処理中断コードを受信
	pwBuf = &wData;
	ferr = ProtRcvHaltCode( pwBuf );

	return ferr;
}


//==============================================================================
/**
 * FFWはコマンドコードBFWCMD_SET_EXPORTDIRを送信し、ポート入出力方法を設定する。
 * @param wPortDir ポート入出力方向設定値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_GetSCIBR( DWORD *pdwBaudIdMin,DWORD *pdwBaudIdMax )
{
	int		eRet;
	WORD	wCmdCode;
	DWORD	dwData;
	WORD	wData;
	FFWERR	ferr;

	WORD*	pwBuf;

	// コマンドコード送信処理
	wCmdCode = BFWCMD_GET_SCIBR;
	ferr = ProtSendCmd( wCmdCode );
	if( ferr != FFWERR_OK ){
		// エラーの場合は処理を中断
		return ferr;
	}

	// パケット受信処理（ステータスまで受信）
	ferr = ProtGetStatus( wCmdCode );
	if( ferr != FFWERR_OK ){
		return ferr;
	}

	// パケット受信処理（ボーレートID Min受信）
	eRet = GetData4( &dwData );
	if( !eRet ){
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}
	// 受信したボーレートID Minをポインタ変数に格納
	*pdwBaudIdMin = dwData;

	// パケット受信処理（ボーレートID Max受信）
	eRet = GetData4( &dwData );
	if( !eRet ){
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}
	// 受信したボーレートID Maxをポインタ変数に格納
	*pdwBaudIdMax = dwData;
	
	// 処理中断コードを受信
	pwBuf = &wData;
	ferr = ProtRcvHaltCode( pwBuf );

	return ferr;
}


//==============================================================================
/**
 * FFWはコマンドコードBFWCMD_SET_SCICTRLを送信し、シリアル通信情報を設定する。
 * @param wSciCtrl シリアル通信制御情報
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_SetSCICTRL( WORD wSciCtrl )
{
	int		eRet;
	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;

	WORD*	pwBuf;

	// コマンドコード送信処理
	wCmdCode = BFWCMD_SET_SCICTRL;
	ferr = ProtSendCmd( wCmdCode );
	if( ferr != FFWERR_OK ){
		// エラーの場合は処理を中断
		return ferr;
	}

	// シリアル通信制御情報送信
	wData = wSciCtrl;
	eRet = PutData2( wData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// パケット受信処理（ステータスまで受信）
	ferr = ProtGetStatus( wCmdCode );
	if( ferr != FFWERR_OK ){
		return ferr;
	}

	// 処理中断コードを受信
	pwBuf = &wData;
	ferr = ProtRcvHaltCode( pwBuf );

	return ferr;
}
//==============================================================================
/**
 * FFWはコマンドコードBFWCMD_GET_SCICTRLを送信し、ポート入出力方法を参照する。
 * @param *wSciCtrl シリアル通信制御情報
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_GetSCICTRL( WORD *pwSciCtrl )
{
	int		eRet;
	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;

	WORD*	pwBuf;

	// コマンドコード送信処理
	wCmdCode = BFWCMD_GET_SCICTRL;
	ferr = ProtSendCmd( wCmdCode );
	if( ferr != FFWERR_OK ){
		// エラーの場合は処理を中断
		return ferr;
	}

	// パケット受信処理（ステータスまで受信）
	ferr = ProtGetStatus( wCmdCode );
	if( ferr != FFWERR_OK ){
		return ferr;
	}

	// パケット受信処理（シリアル通信制御受信）
	eRet = GetData2( &wData );
	if( !eRet ){
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}
	// 受信したシリアル通信制御情報をポインタ変数に格納
	*pwSciCtrl = wData;
	
	// 処理中断コードを受信
	pwBuf = &wData;
	ferr = ProtRcvHaltCode( pwBuf );

	return ferr;
}

//=============================================================================
/**
 * シリアルコマンド用変数初期化
 * @param なし
 * @return なし
 */
//=============================================================================
void InitProtMcuData_Sci(void)
{
	return;
}
