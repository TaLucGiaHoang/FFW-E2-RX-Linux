///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_srm.cpp
 * @brief 指定ルーチン実行関連コマンドの実装ファイル
 * @author RSO H.Hashiguchi, K.Okita
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo140515-005 2014/06/26 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxE2No171004-001 2017/10/04 PA 紡車
	E2拡張機能対応
*/
#include "ffwmcu_srm.h"
#include "domcu_srm.h"
#include "protmcu_srm.h"
#include "prot_common.h"
#include "domcu_mem.h"	// RevRxNo140515-005 Append Line


//=============================================================================
/**
 * 指定ルーチン動作モードの設定処理
 * @param pProg 指定ルーチンのユーザプログラム実行中の各種情報格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetSRM(const FFW_SRM_DATA* pSrMode) 
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR			ferrEnd;

	// RevRxNo140515-005 Append Start	
	// RevRxNo120910-006 Append Start
	FFW_SRM_DATA* pSrmData;
	BYTE*	pbyRwBuff;
	MADDR	dwmadrWorkProgramStartAddr;				// ワーク・プログラム スタートアドレス
	MADDR	dwmadrWorkProgramEndAddr;				// ワーク・プログラム エンドアドレス
	FFW_VERIFYERR_DATA	VerifyErr;
	// RevRxNo120910-006 Append End
	// RevRxNo140515-005 Append End	

	ProtInit();

	ferr = PROT_MCU_SetSRM(pSrMode);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo140515-005 Append Start	
	// メモリウェイト対応のためFFWMCUCmd_SetSRM()から移動。メモリウェイト処理はDO_CWRITE()中で実施のため単純に移動しただけとなった。
	// RevRxNo120910-006 Append Start
	pSrmData = GetSrmData();
	// Startルーチン有効、またはStopルーチン有効時
	if(((pSrmData->dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((pSrmData->dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)){
		pbyRwBuff = GetSrmWorkProgramCode(pSrmData);
		dwmadrWorkProgramStartAddr = pSrmData->dwmadrRamWorkAreaStartAddr + WORKPROGRAMSTARTADDR;
		dwmadrWorkProgramEndAddr   = dwmadrWorkProgramStartAddr + SRM_WORK_PROGRAM_LENGTH - 0x00000001;
		ferr = DO_CWRITE(dwmadrWorkProgramStartAddr, dwmadrWorkProgramEndAddr, VERIFY_OFF, pbyRwBuff, &VerifyErr);
	}
	// RevRxNo120910-006 Append End
	// RevRxNo140515-005 Append End

	// RevRxE2No171004-001 Append Start
	if ((pSrmData->dwSrMode & SRM_MON_POINT_FUNC_BIT) == SRM_MON_POINT_FUNC_BIT) {	// 通過ポイントの場合
		pbyRwBuff = GetMonitorPointProgramCode(pSrmData);
		dwmadrWorkProgramStartAddr = pSrmData->dwmadrRamWorkAreaStartAddr + MON_PROGR_START_ADDR;
		dwmadrWorkProgramEndAddr = dwmadrWorkProgramStartAddr + MON_POINT_PROGRAM_LENGTH - 0x00000001;
		ferr = DO_CWRITE(dwmadrWorkProgramStartAddr, dwmadrWorkProgramEndAddr, VERIFY_OFF, pbyRwBuff, &VerifyErr);
	}
	// RevRxE2No171004-001 Append End

	ferrEnd = ProtEnd();

	return ferrEnd;

}

//=============================================================================
/**
 * 指定ルーチン動作モードクリア
 * @param pProg 指定ルーチンのユーザプログラム実行中の各種情報格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ClrSRM(void) 
{
	FFWERR	ferr = FFWERR_OK;
	FFW_SRM_DATA SrMode;

	SrMode.dwSrMode = INIT_SRM_MODE;
	SrMode.dwmadrBeforeRunFunctionAddr = INIT_BRUN_FUNCADDR;
	SrMode.dwmadrAfterBreakFunctionAddr = INIT_ABREAK_FUNCADDR;
	SrMode.dwmadrBreakFunctionAddr = INIT_BREAK_FUNCADDR;
	SrMode.dwBreakInterruptFunctionIPL = INIT_INT_IPL;
	SrMode.dwmadrRomWorkAreaStartAddr = INIT_WORKROM_ADDR;
	SrMode.dwmadrRamWorkAreaStartAddr = INIT_WORKRAM_ADDR;

	ferr = PROT_MCU_SetSRM(&SrMode);

	return ferr;

}

//=============================================================================
/**
 * スタートストップ関連コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Srm(void)
{
	return;
}




