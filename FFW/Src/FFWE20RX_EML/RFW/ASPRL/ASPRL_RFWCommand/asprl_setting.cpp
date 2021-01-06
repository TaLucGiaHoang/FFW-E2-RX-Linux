///////////////////////////////////////////////////////////////////////////////
/**
* @file asp_setting_family.cpp
* @brief
* @author M.Yamamoto
* @author Copyright (C) 2016 Renesas Electronics Corporation and
* @author Renesas System Design Co., Ltd. All rights reserved.
* @date 2016/12/02
*/
///////////////////////////////////////////////////////////////////////////////

/*
■改定履歴
新規作成
*/

#include "doasp_sys.h"
#include "asp_setting_family.h"
#include "rfwasprl_monev.h"
#include "doasprl_monev.h"
#include "doasp_exe.h"
#include "getstat.h"
extern "C" {
	int e2_CPU_R(ULONG addr, ULONG len, UCHAR *data);
	int e2_CPU_W(ULONG addr, ULONG len, UCHAR *data);
}
#include "e2_asprl_fpga.h"
#include "doasp_sys_family.h"
#include "ocdusbcmd.h"

BOOL s_bSramFullStopFlg = FALSE;


//=============================================================================
/**
* FPGAに対し、ASP機能の動作設定を行う
* @param なし
* @retval なし
*/
//=============================================================================
void Start_ASP_family(void)
{
	RFW_MONEV_FACTOR_TABLE *pTable;
	BYTE bySetCmpData[MONEV_TRG_MAX] = { 0 };
	DWORD dwSetData;
	BYTE byBfwSendData;

	byBfwSendData = 0;

	/* RL78固有部のリセット解除 */
	FPGA_CLR_PAT(MRESCTRL, RSTEC | RSTME);

	if (IsMonitorEventEna()) {
		/*モニタイベント有効時  */
		pTable = GetMonEvFactorTable();
		
		dwSetData = 0;
		// RevRL78No170420-001-02 Top
		dwSetData |= (DWORD)((0x000000FF) & (pTable[0].byMonEvCmpData << 0));
		dwSetData |= (DWORD)((0x0000FF00) & (pTable[1].byMonEvCmpData << 8));
		dwSetData |= (DWORD)((0x00FF0000) & (pTable[2].byMonEvCmpData << 16));
		dwSetData |= (DWORD)((0xFF000000) & (pTable[3].byMonEvCmpData << 24));

		FPGA_WRITE(DETS1, dwSetData);

		dwSetData = 0;
		dwSetData |= (DWORD)((0x000000FF) & (pTable[4].byMonEvCmpData << 0));
		dwSetData |= (DWORD)((0x0000FF00) & (pTable[5].byMonEvCmpData << 8));
		// RevRL78No170420-001-02 Bottom
		
		FPGA_WRITE(DETS2, dwSetData);


		/* マスク機能は未サポート */
		dwSetData = 0;
		FPGA_WRITE(DETM1, dwSetData);
		FPGA_WRITE(DETM2, dwSetData);

		/* モニタイベント有効 */
		FPGA_SET_PAT(MONEV_MEOEN, MONEV_MEOEN_BIT);

		/* BFWへの通知データ(モニタイベント有効) */
		byBfwSendData |= RL78_ASP_FUNC_MONEV;
	}

	/* ブレークイベント(トリガ条件)有効状態をBFWへ通知 */
	if (IsBreakEventTrgEna()) {
		byBfwSendData |= RL78_ASP_FUNC_BRKEV;
	}

	/* BFWへE2拡張機能の有効状態を通知 */
	if (ocd_SetAspFuncMode(byBfwSendData | RL78_ASP_FUNC_ENABLE) == FALSE) {
		return;
	}
	s_bSramFullStopFlg = FALSE;
}


//=============================================================================
/**
* FPGAに対し、ASP機能の無効設定を行う
* @param なし
* @retval なし
*/
//=============================================================================
void Stop_ASP_family(void)
{
	// モニタイベントの無効設定
	DisableMonEv();

	// ブレークイベントの無効設定
	DisableBrkEv();

	// ASP OFF
	ocd_SetAspFuncMode(0);
}


//=============================================================================
/**
* FPGAに対し、ASP機能の動作開始設定を行う
* @param なし
* @retval なし
*/
//=============================================================================
void AspRun_family(void)
{
}


//=============================================================================
/**
* モニタイベントのHW設定(ブレーク時)
* @param なし
* @retval なし
*/
//=============================================================================
void DisableMonEv(void)
{
	/* モニタイベント無効 */
	FPGA_CLR_PAT(MONEV_MEOEN, MONEV_MEOEN_BIT);
}



//=============================================================================
/**
* ブレークイベントのHW設定(ブレーク時)
* @param なし
* @retval なし
*/
//=============================================================================
void DisableBrkEv(void)
{
	/* ダミーリードで無効化 */
	FPGA_READ(EVLACTST1);
}


//=============================================================================
/**
* SRAMフル状態を確認(MCU固有部を見る確認)
* @param なし
* @retval TRUE:SRAMフル状態、　FALSE:SRAMフルでない
*/
//=============================================================================
BOOL IsSramFullStat_family(void)
{
	DWORD dwReadData;

	dwReadData = getbrkstat(DBG_STAT_NOREAD0);

	/* SRAMフルブレーク要求フラグ */
	if ((dwReadData & ES_EXPFUNCFULL_BK) != 0) {
		return TRUE;
	}

	if (s_bSramFullStopFlg) {
		return s_bSramFullStopFlg;
	} else if (FPGA_READ(EVLACTST1) & SRAM_STOP) {
		s_bSramFullStopFlg = TRUE;
		return TRUE;
	}
	else {
		// ありえない
	}

	return FALSE;
}

//=============================================================================
/**
* SRAMフルストップ状態フラグを初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitSramFullStopFlg(void)
{
	s_bSramFullStopFlg = FALSE;
}


//=============================================================================
/**
* ファミリ固有機能のモジュールリセットとレジスタリセットを行う
* @param なし
* @retval なし
*/
//=============================================================================
void Reset_Asp_family(void)
{
	/* RL78固有部のレジスタリセット */
	FPGA_SET_PAT(MRESCTRL, RSTEC | RSTME);
	FPGA_SET_PAT(MRESCTRL, RSTECREG | RSTMEREG);
	FPGA_CLR_PAT(MRESCTRL, RSTECREG | RSTMEREG);
}
