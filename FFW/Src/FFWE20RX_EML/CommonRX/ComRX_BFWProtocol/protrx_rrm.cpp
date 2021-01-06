////////////////////////////////////////////////////////////////////////////////
/**
 * @file protrx_rrm.cpp
 * @brief BFWコマンド プロトコル生成関数（RAMモニタ関連） ソースファイル
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

#include "protrx_rrm.h"
#include "prot_common.h"
#include "prot.h"
#include "comctrl.h"


//==============================================================================
/**
 * FFWはコマンドコードBFWCMD_SET_RRMBを送信し、RAMモニタベースアドレスを設定する。
 * @param byBlk RAMモニタ全ブロック
 * @param dwInitBlk RAMモニタ設定ブロック　1が立っているビットに設定
 * @param dwEnableBlk RAMモニタ有効/無効ブロック情報 該当ビット 1:有効 0:無効
 * @param dwBlkEndian RAMモニタブロックエンディアン情報 該当ビット 1:BIG 0:LITTLE
 * @param pRrmb 設定ベースアドレス格納ポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
//RevNo100715-002 Modify Line
FFWERR PROT_E20_SetRrmb( BYTE byBlk,DWORD dwInitBlk,DWORD dwEnableBlk,DWORD dwBlkEndian, FFW_RRMB_DATA* pRrmb)
{
	int		eRet;
	BYTE	byData;
	WORD	wCmdCode;
	WORD	wData;
	DWORD	dwData;
	FFWERR	ferr;
	WORD*	pwBuf;

	// コマンドコード送信処理
	wCmdCode = BFWCMD_SET_RRMB;
	ferr = ProtSendCmd( wCmdCode );
	if( ferr != FFWERR_OK ){
		// エラーの場合は処理を中断
		return ferr;
	}

	// ブロック数送信処理
	byData = byBlk;
	eRet = PutData1( byData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// 初期化ブロック設定送信処理
	dwData = dwInitBlk;
	eRet = PutData4( dwData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// 動作禁止ブロック設定送信処理
	dwData = dwEnableBlk;
	eRet = PutData4( dwData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	//RevNo100715-002 Append Start
	// ブロックベースアドレスエンディアン送信
	dwData = dwBlkEndian;
	eRet = PutData4( dwData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}
	//RevNo100715-002 Append End

	// ブロックベースアドレス送信
	for( byData = 0;byData < byBlk;byData++ ){
		// ベースアドレスのコピー
		dwData = pRrmb->dwmadrBase[byData];
		eRet = PutData4( dwData );
		if( !eRet ) {
			// エラーの場合は処理を中断
			return FFWERR_COM;
		}	
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
 * FFWはコマンドコードBFWCMD_SET_RRMIを送信し、RAMモニタデータ、アクセス履歴を初期化する。
 * @param byBlk ブロック数
 * @param dwBlkSet 初期化ブロック
 * @param byBlkInit 初期化方法
 * @retval FFWエラーコード
 */
//==============================================================================
//RevNo100715-003 Modify Line
FFWERR PROT_E20_SetRrmi( BYTE byBlk,DWORD dwBlkSet,BYTE byBlkInit,DWORD dwEndian )
{
	int		eRet;
	BYTE	byData;
	WORD	wCmdCode;
	WORD	wData;
	DWORD	dwData;
	FFWERR	ferr;
	WORD*	pwBuf;

	// コマンドコード送信処理
	wCmdCode = BFWCMD_SET_RRMI;
	ferr = ProtSendCmd( wCmdCode );
	if( ferr != FFWERR_OK ){
		// エラーの場合は処理を中断
		return ferr;
	}

	// ブロック数送信処理
	byData = byBlk;
	eRet = PutData1( byData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// 初期化ブロック送信処理
	dwData = dwBlkSet;
	eRet = PutData4( dwData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// ブロック初期化送信処理
	byData = byBlkInit;
	eRet = PutData1( byData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	//RevNo100715-003 Append Start
	// 初期化ブロックエンディアン情報送信
	dwData = dwEndian;
	eRet = PutData4( dwData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}
	//RevNo100715-002 Append End

	// パケット受信処理（ステータスまで受信）
	ferr = ProtGetStatus( wCmdCode );
	if( ferr != FFWERR_OK ){
		return ferr;
	}

	// 処理中断コードを受信
	pwBuf = &wData;
	ferr = ProtRcvHaltCode( pwBuf );

	return	ferr;
}


//==============================================================================
/**
 * FFWはコマンドコードBFWCMD_GET_RRMBを送信し、RAMモニタベースアドレスを参照する。
 * @param byBlk 指定ブロック
 * @param wStartAddr 開始アドレス
 * @param wSize データ数
 * @param *pbyData 取得RAMモニタデータ格納ポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_E20_GetRrmd( BYTE byBlk,WORD wStartAddr,WORD wSize,BYTE *pbyData )
{
	int		eRet;
	BYTE	byData;
	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;
	WORD*	pwBuf;

	// コマンドコード送信処理
	wCmdCode = BFWCMD_GET_RRMD;
	ferr = ProtSendCmd( wCmdCode );
	if( ferr != FFWERR_OK ){
		// エラーの場合は処理を中断
		return ferr;
	}

	// ブロック数送信処理
	byData = byBlk;
	eRet = PutData1( byData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// 開始アドレス送信処理
	wData = wStartAddr;
	eRet = PutData2( wData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// データ数送信処理
	wData = wSize;
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

	// データ数受信
	eRet = GetData2( &wData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// RAMモニタデータ受信
	eRet = GetDataN( (wData * 2),pbyData );
	if( !eRet ) {
		// エラーの場合は処理を中断
		return FFWERR_COM;
	}

	// 処理中断コードを受信
	pwBuf = &wData;
	ferr = ProtRcvHaltCode( pwBuf );

	return	ferr;
}


//==============================================================================
/**
 * FFWはコマンドコードBFWCMD_CLR_RRMLを送信し、RAMモニタロスト発生をクリアする。
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_E20_ClrRrml( void )
{
	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;
	WORD*	pwBuf;

	// コマンドコード送信処理
	wCmdCode = BFWCMD_CLR_RRML;
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

	// 処理中断コードを受信
	pwBuf = &wData;
	ferr = ProtRcvHaltCode( pwBuf );

	return	ferr;
}


//=============================================================================
/**
 * RAMモニタコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtRxData_Rrm(void)
{
	return;
}
