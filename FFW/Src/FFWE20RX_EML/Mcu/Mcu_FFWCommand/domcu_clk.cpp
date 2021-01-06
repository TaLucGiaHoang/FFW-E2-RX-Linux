////////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_clk.cpp
 * @brief クロック関連コマンドのソースファイル
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
#include "errchk.h"
#include "protmcu_reg.h"
#include "prot_common.h"

//==============================================================================
/**
 * JTAGクロック周波数を設定する。
 * @param dwGenFreq エミュレータ生成クロックの周波数
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetJtagClk(BYTE byJtagClk)
{

	FFWERR 				ferr;
	FFWERR				ferrEnd;

	ProtInit();

	/* JTAGクロック周波数を設定 */
	ferr = PROT_SetJtagClk(byJtagClk);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();

	return ferrEnd;

}


//==============================================================================
/**
 * JTAGクロック周波数を参照する。
 * @param pdwGenFreq JTAGクロックの周波数を格納する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_GetJtagClk(BYTE *pbyJtagClk)
{

	FFWERR 			ferr;
	FFWERR			ferrEnd;

	ProtInit();

	/* JTAGクロック周波数を参照 */
	ferr = PROT_GetJtagClk(pbyJtagClk);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();

	return ferrEnd;

}

//=============================================================================
/**
 * クロック関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Clk(void)
{
	return;
}
