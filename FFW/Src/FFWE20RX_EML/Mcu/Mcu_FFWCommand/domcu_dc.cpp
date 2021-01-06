///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_dc.cpp
 * @brief デバッグコンソールコマンドの実装ファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#include "domcu_dc.h"
#include "prot_common.h"
#include "do_sys.h"
#include "protmcu_dc.h"


// static 関数宣言

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * C2Eレジスタ内のデータを読み込む
 * @param pC2E    C2Eレジスタ値を格納
 * @param pdwData C2Eレジスタ内のデータを格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetC2E(FFWRX_C2E_DATA* pC2E, BYTE* pbyData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_GetC2E(pC2E, pbyData);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();

	return ferrEnd;
}	

//=============================================================================
/**
 * E2Cレジスタにデータを書き込む
 * @param pE2C    E2Cレジスタ値を格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetE2C(FFWRX_E2C_DATA* pE2C)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_SetE2C(pE2C);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();

	return ferrEnd;
}	

//=============================================================================
/**
 * C2E, E2Cレジスタとバッファをクリア
 * @param dwRegNo   C2E,E2C Ch番号
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_ClrC2EE2C(DWORD dwRegNo)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	if (dwRegNo > 1) {
		return FFWERR_FFW_ARG;
	}

	ProtInit();

	ferr = PROT_ClrC2EE2C(dwRegNo);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();

	return ferrEnd;
}


//=============================================================================
/**
 * デバッグコンソールコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Dc(void)
{
	return;
}

