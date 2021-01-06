///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_sci.cpp
 * @brief シリアル関連コマンドの実装ファイル
 * @author RSD K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/12
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/11/12 上田
	E2 Lite対応
*/
#include "ffwmcu_sci.h"
#include "domcu_sci.h"
// V.1.02 RevNo110613-001 Append Line
#include "errchk.h"
#include "do_sys.h"	// RevRxE2LNo141104-001 Append Line

//static変数宣言
static WORD s_wSciCtrl;

///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//==============================================================================
/**
 * ポート入出力方向の設定
 * @param wPort 入出力方向を指定するポート番号を設定する。
 * @param wPortDir 入出力方向を設定する。
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetEXPORTDIR(WORD wPort,WORD wPortDir,WORD wPortType)
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wPort;
	wPortDir;
	wPortType;

	return ferr;
}

//==============================================================================
/**
 * ポート入出力方向の参照
 * @param byPortNo 参照するポートの番号を指定
 * @param pbyDir   ポートの入出力方向を参照
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetEXPORTDIR( WORD wPort,WORD* pwPortDir,WORD* pwPortType )
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wPort;
	pwPortDir;
	pwPortType;

	return ferr;
}

//==============================================================================
/**
 * ポート出力レベルの設定
 * @param byPortNoLvl 指定ポートの番号と出力レベルを指定
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetEXPORTLVL( WORD wPort,WORD wPortLvl )
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wPort;
	wPortLvl;
	return ferr;
}


//==============================================================================
/**
 * ポート出力レベルの参照
 * @param byPortNo  参照するポートの番号を指定
 * @param pbyLvl    ポートの入力レベルを参照
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetEXPORTLVL( WORD wPort,WORD *pwPortLvl )
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wPort;
	pwPortLvl;

	return ferr;
}
//==============================================================================
/**
 * シリアル入出力用ボーレートの設定
 * @param wBaudrateVal 通信ボーレートの設定値を指定
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetSCIBR(DWORD dwBaudrateVal)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwBaudSet;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Modify Start
	if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20) || (einfData.wEmuStatus == EML_EZCUBE)) {
		// E1/E20/EZ-CUBEの場合
		// ボーレート設定レジスタへの設定値を算出
		// 設定値 = 99 × 10の6乗 / ボーレート値
		// 19200bpsの場合は5156.25で小数点切り捨てで5156が設定値
		dwBaudSet = 99000000 / dwBaudrateVal;

	} else {	// E1/E20/EZ-CUBE以外の場合
		dwBaudSet = dwBaudrateVal;
	}
	// RevRxE2LNo141104-001 Modify End

	// 通信ボーレートの設定
	ferr = DO_SetSCIBR(dwBaudSet);
	// V.1.02 RevNo110613-001 Modify Start
	if (ferr == FFWERR_OK) {
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}


//==============================================================================
/**
 * シリアル入出力用ボーレートの参照
 * @param pwBaudrateValMin 通信ボーレートの設定可能最小値を格納
 * @param pwBaudrateValMax 通信ボーレートの設定可能最大値を格納
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetSCIBR(DWORD* pdwBaudrateValMin,DWORD* pdwBaudrateValMax)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// 通信ボーレートの参照
	ferr = DO_GetSCIBR( pdwBaudrateValMin,pdwBaudrateValMax );
	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}


//==============================================================================
/**
 * シリアル通信情報の設定
 * @param wSciCtrl シリアル通信情報を設定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetSCICTRL(WORD wSciCtrl)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// シリアル通信情報の設定
	ferr = DO_SetSCICTRL( wSciCtrl );
	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	s_wSciCtrl = wSciCtrl;

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}


//==============================================================================
/**
 * シリアル通信情報の参照
 * @param *pwSciCtrl シリアル通信情報の参照値 
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetSCICTRL( WORD* pwSciCtrl )
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// シリアル通信情報の参照
	ferr = DO_GetSCICTRL( pwSciCtrl );
	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}

//==============================================================================
/**
 * ターゲットMCU情報番号の設定
 * @param wMcuInf ターゲットMCU情報番号を設定する。
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_TGTMCUINF( WORD wMcuInf )
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wMcuInf;

	return ferr;
}


//==============================================================================
/**
 * ターゲットMCUの起動
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_TGTMCUON( void )
{
	FFWERR	ferr = FFWERR_OK;

	return ferr;
}


//==============================================================================
/**
 * データのシリアル出力
 * @param dwLen   出力個数を格納
 * @param byData  シリアル出力する8ビットデータを指定
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetEXDATAOUT(DWORD dwLen, BYTE* byData)
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	dwLen;
	byData;
	
	return ferr;
}


//==============================================================================
/**
 * データのシリアル入力
 * @param dwLen   入力個数を格納
 * @param byData  シリアル入力した8ビットデータを指定
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetEXDATAIN(DWORD dwLen, BYTE* byData)
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	dwLen;
	byData;

	return ferr;
}

//==============================================================================
/**
 * ポートプルアップの設定
 * @param wPort プルアップを指定するポート番号を設定する。
 * @param wPortPup プルアップを設定する。
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetEXPORTPUP(WORD wPort,WORD wPortPup)
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wPort;
	wPortPup;

	return ferr;
}


//==============================================================================
/**
 * ポートプルアップの参照
 * @param wPort プルアップを指定するポート番号を設定する。
 * @param pwPortPup プルアップ参照値
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetEXPORTPUP( WORD wPort,WORD* pwPortPup )
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wPort;
	pwPortPup;

	return ferr;
}


//==============================================================================
/**
 * シリアルデータ送信間のウェイト設定
 * @param wFrmInterval シリアルデータ送信間のウェイト(us)
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetFRMINTERVAL( WORD wFrmInterval )
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wFrmInterval;

	return ferr;
}


//==============================================================================
/**
 * パケットデータ送信間のウェイト設定
 * @param wPktInterval パケットデータ送信間のウェイト(us)
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetPKTINTERVAL( WORD wPktInterval )
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	wPktInterval;

	return ferr;
}


//==============================================================================
/**
 * 入出力ポートのコンフィグレーション設定
 * @param byLen コンフィグレーションパターンの数
 * @param pwConfigData コンフィグレーションパターン
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中の為、コマンド処理を行うことができない
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetEXCONFIG( BYTE byLen,WORD *pwConfigData )
{
	FFWERR	ferr = FFWERR_OK;
	// コンパイル時のワーニング対策
	byLen;
	pwConfigData;

	return ferr;
}

//==============================================================================
/**
 * データのシリアル出力後、入力(未使用) R8Cで追加
 * @param 不明
 * @return FFWERR_OK
*/
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_EXDATAOUTIN(WORD wSendLen, WORD wRcvLen, BYTE* byData)
{
	// コンパイル時のワーニング対策
	wSendLen;
	wRcvLen;
	byData;
	return FFWERR_OK;
}

//=============================================================================
/**
 * SCIサポート関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Sci(void)
{
	s_wSciCtrl = 0;
	return;

}
