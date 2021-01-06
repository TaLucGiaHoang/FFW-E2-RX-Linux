////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_clk.cpp
 * @brief クロック関連コマンドのソースファイル
 * @author RSD Y.Minami, H.Hashiguchi, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/02/01
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2 対応
*/

#include "ffwmcu_clk.h"
#include "domcu_clk.h"
#include "errchk.h"
#include "domcu_prog.h"
#include "do_sys.h"		// RevRxE2No170201-001 Append Line

// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * JTAGテストクロックの設定
 * @param byJtagClk JTAGテストクロック(TCLK)を指定
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWE20Cmd_SetJTAGCLK(BYTE byJtagClk)
{
	FFWERR	ferr;
	BOOL	bRet;
	FFWE20_EINF_DATA	einfData;	// RevRxE2No170201-001 Modify Line

	getEinfData(&einfData);			// RevRxE2No170201-001 Modify Line

	bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line
	if (bRet) {		// プログラム実行状態をチェック
		return FFWERR_BMCU_RUN;
	}

	// 引数チェック
	// RevRxE2No170201-001 Modify Start
	if (einfData.wEmuStatus == EML_E2) {	// E2の場合
		if (byJtagClk < TCLK_50_0MHZ || byJtagClk > TCLK_22_222MHZ) {
			 return FFWERR_FFW_ARG;
		}
	} else {								// E1/E20/E2 Liteの場合
		if (byJtagClk < TCLK_50_0MHZ || byJtagClk > TCLK_16_6MHZ) {
			 return FFWERR_FFW_ARG;
		}
	}
	// RevRxE2No170201-001 Modify End

	// クロック周波数設定処理
	ferr = DO_SetJtagClk(byJtagClk);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * JTAGテストクロックの参照
 * @param pbyJtagClk JTAGテストクロック(TCLK)を指定
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWE20Cmd_GetJTAGCLK(BYTE* pbyJtagClk)
{
	FFWERR	ferr;

	/* クロック周波数を参照 */
	ferr = DO_GetJtagClk(pbyJtagClk);

	return ferr;
}
// 2008.11.18 INSERT_END_E20RX600 }

//=============================================================================
/**
 * クロック関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Clk(void)
{

	return;

}
