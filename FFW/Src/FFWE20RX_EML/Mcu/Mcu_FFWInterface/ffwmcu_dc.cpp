///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_dc.cpp
 * @brief デバッグコンソール関連コマンドの実装ファイル
 * @author RSO H.Hashiguchi
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
#include "ffwmcu_dc.h"
#include "domcu_dc.h"
#include "errchk.h"


///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * C2Eレジスタ内のデータを読み込む
 * @param pC2E    C2Eレジスタ値を格納
 * @param pdwData C2Eレジスタ内のデータを格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetC2E(FFWRX_C2E_DATA* pC2E, BYTE* pbyData)
{
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// CH0 以外は、引数エラーとする。
	if (pC2E->dwRegNo != C2E_CH0) {
		ferr = FFWERR_FFW_ARG;
	}
	

	if (pC2E->dwLength > C2E_BUFF_LEN) {
		ferr = FFWERR_FFW_ARG;
	}

	if (ferr == FFWERR_OK) {
		ferr = DO_GetC2E(pC2E, pbyData);
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * E2Cレジスタにデータを書き込む
 * @param pE2C    E2Cレジスタ値を格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_SetE2C(FFWRX_E2C_DATA* pE2C)
{
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// CH0 以外の指定は、引数エラーとする。
	if (pE2C->dwRegNo != E2C_CH0) {
		ferr = FFWERR_FFW_ARG;
	}

	if (pE2C->dwLength > E2C_BUFF_LEN) {
		ferr = FFWERR_FFW_ARG;
	}

	if (ferr == FFWERR_OK) {
		ferr = DO_SetE2C(pE2C);
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * C2E/E2Cレジスタの初期化とC2E/E2Cバッファの初期化。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_ClrC2EE2C(void)
{
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	ferr = DO_ClrC2EE2C(CLR_C2E_E2E_CH0);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * メモリ操作コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Dc(void)
{
	return;
}


