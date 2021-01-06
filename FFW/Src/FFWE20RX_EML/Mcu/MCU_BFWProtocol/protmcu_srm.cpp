///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_srm.cpp
 * @brief BFWコマンド プロトコル生成関数(スタートストップファンクションコマンド)
 * @author RSO H.Hashiguchi, Y.Miyake
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/11/12
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-006 2012/11/12 三宅
    指定ルーチン実行機能の高速化。
*/
#include "protmcu_srm.h"
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
// RevRxNo120910-006 Append Line
#include "ffwmcu_srm.h"

//==============================================================================
/**
 * BFWMMCUCmd_SetSRM を送信し、指定ルーチンの動作設定を行う。
 * @param 指定ルーチンの動作設定情報を格納するFFWMCU_SRM_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_MCU_SetSRM(const FFW_SRM_DATA* pSrMode)
{
	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;

	wCmdCode = BFWCMD_SET_SRM;

	// コマンドコード送信
	ferr = ProtSendCmd(wCmdCode);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// [動作モード]の送信
	if (PutData2((WORD)pSrMode->dwSrMode) != TRUE) {	
		return FFWERR_COM;
	}

	// [ユーザプログラムブレーク中に繰り返し実行する指定ルーチン先頭アドレス]の送信
	if (PutData4(pSrMode->dwmadrBreakFunctionAddr) != TRUE) {	
		return FFWERR_COM;
	}

	// 現在は無効 [ユーザプログラムブレーク中に指定割込み先頭アドレス]の送信
	if (PutData4(0x0000000) != TRUE) {	
		return FFWERR_COM;
	}

	// [ユーザプログラムブレーク後に実行する指定ルーチン先頭アドレス]の送信
	if (PutData4(pSrMode->dwmadrAfterBreakFunctionAddr) != TRUE) {	
		return FFWERR_COM;
	}

	// [ユーザプログラム実行開始前に実行する指定ルーチン先頭アドレス]の送信
	if (PutData4(pSrMode->dwmadrBeforeRunFunctionAddr) != TRUE) {	
		return FFWERR_COM;
	}

	// [IPLレベル設定有無]の送信
	// RevRxNo120910-006 Modify Line
	if ((pSrMode->dwSrMode & SRM_INTERRUPT_FUNC_BIT) == SRM_INTERRUPT_FUNC_BIT) {
		// [IPLレベル設定有]の送信
		if (PutData2((WORD)0x01) != TRUE) {	
			return FFWERR_COM;
		}
	} else {
		// [IPLレベル設定無]の送信
		if (PutData2((WORD)0x00) != TRUE) {	
			return FFWERR_COM;
		}
	}

	// [ユーザプログラムブレーク中の割り込み優先レベル]の送信
	if (PutData2((WORD)pSrMode->dwBreakInterruptFunctionIPL) != TRUE) {	
		return FFWERR_COM;
	}

	// [指定ルーチン呼び出しのためのプログラムコードを配置する領域]の送信
	if (PutData4(pSrMode->dwmadrRomWorkAreaStartAddr) != TRUE) {
		return FFWERR_COM;
	}

	// [指定ルーチン実行で使用するスタック領域の開始アドレス]の送信
	if (PutData4(pSrMode->dwmadrRamWorkAreaStartAddr) != TRUE) {
		return FFWERR_COM;
	}

	// ステータス受信
	ferr = ProtGetStatus(wCmdCode);	
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 処理中断コードを受信
	ferr = ProtRcvHaltCode(&wData);

	return ferr;

}

//=============================================================================
/**
 * スタートストップファンクション関連コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Srm(void)
{
	return;
}

