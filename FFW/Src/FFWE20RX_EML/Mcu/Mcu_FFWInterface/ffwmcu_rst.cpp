///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_rst.cpp
 * @brief リセット関連コマンドの実装ファイル
 * @author RSD Y.Minami, H.Hashiguchi, Y.Miyake, S.Ueda, K.Okita
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/17
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-006	2012/11/01 三宅
  Start/Stop機能の高速化。
  ・FFWMCUCmd_SREST()で、指定ルーチン用ワーク・プログラムのライト処理追加。
  ・ffwmcu_srm.h, domcu_mem.h のインクルード追加。
・RevRxNo130730-006	2013/11/13 上田
	E20トレースクロック遅延設定タイミング変更
・RevRxNo140515-005 2014/07/19 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo130730-001 2014/07/22 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
*/
#include "ffwmcu_rst.h"
#include "domcu_rst.h"
#include "ffwmcu_mcu.h"
#include "domcu_prog.h"
//V.1.02 RevNo110613-001 Append Line
#include "errchk.h"
// RevRxNo120910-006 Append Start
#include "ffwmcu_srm.h"
#include "domcu_mem.h"
// RevRxNo120910-006 Append End
#include "hwrx_fpga_tra.h"	// RevRxNo130730-006 Append Line
#include "mcu_mem.h"		// RevRxNo140515-005 Append Line

// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * ターゲットMCU をH/W リセットする。リセット後、MCU の各レジスタを初期化する。
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_REST(void)
{

	// V.1.02 RevNo110613-001 Modify Line
	DWORD	dwRet = FFWERR_OK;
	BOOL	bRet;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrOsmFwriteWarningFlg();	// RevRxNo130730-001 Append Line

	bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line
	if (bRet) {		// プログラム実行状態をチェック
		return FFWERR_BMCU_RUN;
	}

	dwRet = DO_REST();

	// RevRxNo130730-001 Append Start
	if(dwRet == FFWERR_OK){
		dwRet = WarningChk_OsmFwrite();			// オプション設定メモリ書き戻し時のWarning発生確認
	}
	// RevRxNo130730-001 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(dwRet == FFWERR_OK){
		dwRet = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return dwRet;

}


//==============================================================================
/**
 * エミュレータ内部設定を初期化する。
 * @param なし
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_CLR(void)
{

	// V.1.02 RevNo110613-001 Modify Line
	FFWERR		ferr = FFWERR_OK;
	BOOL		bRet;
	BOOL		bHotPlugState = FALSE;
	BOOL		bIdCodeResultState = FALSE;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	
	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		/* プログラム実行状態をチェック */
		bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line
		if (bRet) {
					// プログラム実行中の場合
			return FFWERR_BMCU_RUN;
		}
	}

	// E20トレースFPGA全レジスタ設定を指示
	SetTrcE20FpgaAllRegFlg(TRUE);	// RevRxNo130730-006 Append Line

	ferr = DO_CLR();

	// E20トレースFPGA全レジスタ設定指示を解除
	SetTrcE20FpgaAllRegFlg(FALSE);	// RevRxNo130730-006 Append Line

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;

}
// 2008.11.18 INSERT_END_E20RX600 }

//==============================================================================
/**
 * システムリセット
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_SREST(void)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo140515-005 Delete Start/Stopファンクション用ワークRAMの設定処理をDO_SREST()に移動 

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line
	ClrOsmFwriteWarningFlg();		// RevRxNo130730-001 Append Line

	// E20トレースFPGA全レジスタ設定を指示
	SetTrcE20FpgaAllRegFlg(TRUE);	// RevRxNo130730-006 Append Line

	ferr = DO_SREST();

	// E20トレースFPGA全レジスタ設定指示を解除
	SetTrcE20FpgaAllRegFlg(FALSE);	// RevRxNo130730-006 Append Line

	// RevRxNo140515-005 Delete Start/Stopファンクション用ワークRAMの設定処理をDO_SREST()に移動 

	// RevRxNo130730-001 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_OsmFwrite();			// オプション設定メモリ書き戻し時のWarning発生確認
	}
	// RevRxNo130730-001 Append End

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;

}


//=============================================================================
/**
 * リセット関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Rst(void)
{

	return;

}
