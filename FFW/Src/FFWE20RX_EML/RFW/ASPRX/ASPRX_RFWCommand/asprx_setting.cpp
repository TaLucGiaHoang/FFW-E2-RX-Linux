///////////////////////////////////////////////////////////////////////////////
/**
* @file asp_setting_family.cpp
* @brief
 * @author REL M.Yamamoto, PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////

/*
■改定履歴
・新規作成 M.Yamamoto
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/

#include "doasp_sys.h"
#include "doasp_exe.h"
#include "doasp_sys_family.h"
#include "e2_asprx_fpga.h"	// RevRxE2No171004-001 Modify Line
// RevRxE2No171004-001 Append Start
#include "doasprx_monev.h"
#include "doe2_asp.h"
#include "domcu_prog.h"
// RevRxE2No171004-001 Append End

BOOL s_bSramFullStopFlg = FALSE;


// RevRxE2No171004-001 Append Start
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
		byBfwSendData |= ASP_FUNC_MONEV;
	}

	/* ブレークイベント(トリガ条件)有効状態をBFWへ通知 */
	if (IsBreakEventTrgEna()) {
		byBfwSendData |= ASP_FUNC_BRKEV;
	}

	/* BFWへE2拡張機能の有効状態を通知 */
	if (FFWE2Cmd_SetAspConf(byBfwSendData | ASP_FUNC_ENABLE) != FFWERR_OK) {
		return;
	}

	/* BFWへ通過ポイントの設定点数、および、通過ポイント番号と通過ポイントアドレスを通知 */
	if (FFWE2Cmd_SetMonitorPointAddr() != FFWERR_OK) {
		return;
	}

	s_bSramFullStopFlg = FALSE;
}
// RevRxE2No171004-001 Append End


//=============================================================================
/**
* FPGAに対し、ASP機能の無効設定を行う
* @param なし
* @retval なし
*/
//=============================================================================
void Stop_ASP_family(void)
{
	// RevRxE2No171004-001 Append Start
	// モニタイベントの無効設定
	DisableMonEv();

	// ブレークイベントの無効設定
	DisableBrkEv();

	// ASP OFF
	DO_SetAspConf(0);
	// RevRxE2No171004-001 Append End
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
	/* 電流値計測テスト 暫定対応(本番はBFWでの実施を検討) */
	// FPGA_SET_PAT(ASPCTRL, ASPCTRL_ASPRUN);		// RevRxE2No171004-001 Delete Line
}


// RevRxE2No171004-001 Append Start
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
	FFWERR dwRet;
	DWORD dwBreakFact;
	enum FFWRX_COMB_PATTERN eBrkTrcComb;
	BYTE byOrBrkFactEvPC;
	BYTE byOrBrkFactEvOA;

	dwRet = FFWRXCmd_GetBRKF(&dwBreakFact, &eBrkTrcComb, &byOrBrkFactEvPC, &byOrBrkFactEvOA);

	if (dwRet == FFWERR_OK){
		// SRAMフルブレーク要求フラグ
		if ((dwBreakFact & BRKF_BREAKFACT_SRAMFULL_BIT) != 0) {
			return TRUE;
		}
	}

	if (s_bSramFullStopFlg) {
		return s_bSramFullStopFlg;
	}
	else if (FPGA_READ(EVLACTST1) & SRAM_STOP) {
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
// RevRxE2No171004-001 Append End
