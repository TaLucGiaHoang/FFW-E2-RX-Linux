///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_sci.cpp
 * @brief シリアル関連コマンドの実装ファイル
 * @author RSD Y.Minami, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/20
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/11/20 上田
	E2 Lite対応
*/
#include "domcu_sci.h"
#include "protmcu_sci.h"
#include "prot_common.h"
#include "ffw_sys.h"
#include "domcu_prog.h"
#include "ffwmcu_mcu.h"

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////

//==============================================================================
/**
 * E1/E20-MCU間シリアル通信ボーレートを設定する。
 * @param wBaudId 通信ボーレート設定値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetSCIBR( DWORD dwBaudId )
{
	FFWERR			ferr;
	FFWERR			ferrEnd;

	ProtInit();

	// SetSCIBRプロトコル送信
	ferr = PROT_SetSCIBR( dwBaudId );
	if (ferr != FFWERR_OK) {
		// エラー発生時は処理を中断
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	return ferrEnd;
}


//==============================================================================
/**
 * E1/E20-MCU間シリアル通信ボーレートを参照する。
 * @param wBaudIdMin 通信ボーレート設定可能最小値
 * @param wBaudIdMax 通信ボーレート設定可能最大値
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWERR DO_GetSCIBR( DWORD *dwBaudIdMin,DWORD *dwBaudIdMax )
{
	FFWERR			ferr;
	FFWERR			ferrEnd;
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;

	bHotPlugState = getHotPlugState();				// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	ProtInit();

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}

	// SetSCIBRプロトコル送信
	ferr = PROT_GetSCIBR( dwBaudIdMin,dwBaudIdMax );
	if (ferr != FFWERR_OK) {
		// エラー発生時は処理を中断
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	return ferrEnd;
}


//==============================================================================
/**
 * シリアル通信情報を設定する。
 * @param wSciCtrl シリアル通信機能情報設定値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetSCICTRL( WORD wSciCtrl )
{
	FFWERR			ferr;
	FFWERR			ferrEnd;

	ProtInit();

	// SetSCICTRLプロトコル送信
	ferr = PROT_SetSCICTRL( wSciCtrl );
	if (ferr != FFWERR_OK) {
		// エラー発生時は処理を中断
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	return ferrEnd;
}

//==============================================================================
/**
 * シリアル通信情報を参照する。
 * @param wSciCtrl シリアル通信機能情報参照値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_GetSCICTRL( WORD *wSciCtrl )
{
	FFWERR			ferr;
	FFWERR			ferrEnd;

	ProtInit();

	// GetSCICTRLプロトコル送信
	ferr = PROT_GetSCICTRL( wSciCtrl );
	if (ferr != FFWERR_OK) {
		// エラー発生時は処理を中断
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	return ferrEnd;
}


//=============================================================================
/**
 * シリアル関連コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Sci(void)
{
	return;
}
