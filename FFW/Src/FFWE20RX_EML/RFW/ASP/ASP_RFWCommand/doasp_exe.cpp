///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_exe.cpp
 * @brief 実行系拡張機能イベントリンクコマンドの実装ファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/10/20 新規作成 S.Nagai
*/
#include "rfw_bitops.h"
#include "doasp_exe.h"
#include "rfwasp_exe.h"
#include "emudef.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"

// ファイル内static変数の宣言

// ファイル内static関数の宣言
static BOOL s_bBreakEventEna = FALSE;		// ブレークイベント有効フラグ(TRUE:有効、FALSE:無効)
static BOOL s_bStopTraceEventEna = FALSE;	// トレース停止イベント有効フラグ(TRUE:有効、FALSE:無効)



///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* E2エミュレータ拡張機能のブレークイベントの設定
* @param u64FactorSingleEvent シングルE2トリガ
* @param dwFactorMultiEvent マルチE2トリガ
* @retval RFWERR_OK
* @retval RFWERR_N_BMCU_RUN
* @retval RFWERR_N_BASP_SGLLNK_ILLEGAL
* @retval RFWERR_N_BASP_FACT_NONSEL
* @retval RFWERR_N_BASP_FACT_NOSUPPORT
* @retval RFWERR_N_BASP_FACT_DISABLE
*/
//=============================================================================
DWORD DO_SetBreakEvent(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	dwFactorMultiEvent;	// warning回避

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun()) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, u64FactorSingleEvent, RFW_EVENTLINK_BOTH, RF_ASPE2ACTCH_BRKEV);
	}

	if (dwRet == RFWERR_OK) {
		
		// 指定シングルE2トリガのアクション情報にブレークイベントをリンク
		SaveSingleEvent(find_first_bit(u64FactorSingleEvent), (UINT64)((UINT64)1 << RF_ASPE2ACTCH_BRKEV));

		// E2アクションのテーブル設定
		SetE2ActTable((DWORD)RF_ASPE2ACTCH_BRKEV, RFW_EVENTLINK_BOTH, u64FactorSingleEvent);

		// ブレークイベント設定状態更新
		s_bBreakEventEna = TRUE;
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* E2エミュレータ拡張機能のブレークイベントの参照
* @param pu64FactorSingleEvent シングルE2トリガ
* @param pdwFactorMultiEvent マルチE2トリガ
* @retval RFWERR_OK
* @retval RFWERR_N_BASP_BRKEV_NOUSE
*/
//=============================================================================
DWORD DO_GetBreakEvent(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent)
{
	DWORD dwRet = RFWERR_OK;
	RFW_ASPE2ACT_INFO E2AspActInfo;

	ProtInit_ASP();
	
	if (!s_bBreakEventEna) {
		dwRet = RFWERR_N_BASP_BRKEV_NOUSE;
	}

	if (dwRet == RFWERR_OK) {
		GetE2ActTable(RF_ASPE2ACTCH_BRKEV, &E2AspActInfo);
		*pu64FactorSingleEvent = E2AspActInfo.u64LinkFact;
		*pdwFactorMultiEvent = 0;	// 予約変数
	}

	ProtEnd_ASP();

	return dwRet;

}

//=============================================================================
/**
* E2エミュレータ拡張機能のブレークイベントの削除
* @param なし
* @retval RFWERR_OK
* @retval RFWERR_N_BMCU_RUN
* @retval RFWERR_N_BASP_BRKEV_NOUSE
*/
//=============================================================================
DWORD DO_ClrBreakEvent(void)
{
	DWORD dwRet = RFWERR_OK;
	
	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun()) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// ブレークイベント無効でないか確認
	if (dwRet == RFWERR_OK) {
		if (!s_bBreakEventEna) {
			dwRet = RFWERR_N_BASP_BRKEV_NOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		ClrE2ActTable(RF_ASPE2ACTCH_BRKEV);

		// RFW_ASPE2TRG_INFO.u64LinkActにリンクされている本E2アクション情報を削除
		ClearSingleEvent((UINT64)((UINT64)1 << RF_ASPE2ACTCH_BRKEV));

		s_bBreakEventEna = FALSE;
	}

	ProtEnd_ASP();

	return dwRet;

}

///////////////////////////////////////////////////////////////////////////////
/**
* E2エミュレータ拡張機能のトレース停止イベントの設定
* @param u64FactorSingleEvent シングルE2トリガ
* @param dwFactorMultiEvent マルチE2トリガ
* @retval RFWERR_OK
* @retval RFWERR_N_BMCU_RUN
* @retval RFWERR_N_BASP_STOPTRCOFF
* @retval RFWERR_N_BASP_SGLLNK_ILLEGAL
* @retval RFWERR_N_BASP_FACT_NONSEL
* @retval RFWERR_N_BASP_FACT_NOSUPPORT
* @retval RFWERR_N_BASP_FACT_DISABLE
*/
//=============================================================================
DWORD DO_SetStopTraceEvent(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent)
{
	DWORD	dwRet = RFWERR_OK;
	RFW_ASPCONF_DATA AspConfData;

	ProtInit_ASP();

	dwFactorMultiEvent;	// warning回避

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if ((AspConfData.dwFunction3 & RF_ASPCONF_STOPTRC_EV) == 0) {
			dwRet = RFWERR_N_BASP_STOPTRCOFF;
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, u64FactorSingleEvent, RFW_EVENTLINK_BOTH, RF_ASPE2ACTCH_TRCEV);
	}

	if (dwRet == RFWERR_OK) {

		// 指定シングルE2トリガのアクション情報にトレース停止イベントをリンク
		SaveSingleEvent(find_first_bit(u64FactorSingleEvent), (UINT64)((UINT64)1 << RF_ASPE2ACTCH_TRCEV));

		// E2アクションのテーブル設定
		SetE2ActTable((DWORD)RF_ASPE2ACTCH_TRCEV, RFW_EVENTLINK_BOTH, u64FactorSingleEvent);

		// ブレークイベント設定状態更新
		s_bStopTraceEventEna = TRUE;
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* E2エミュレータ拡張機能のトレース停止イベントの参照
* @param pu64FactorSingleEvent シングルE2トリガ
* @param pdwFactorMultiEvent マルチE2トリガ
* @retval RFWERR_OK
* @retval RFWERR_N_BASP_STOPTRC_NOUSE
*/
//=============================================================================
DWORD DO_GetStopTraceEvent(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent)
{
	RFWERR dwRet = RFWERR_OK;
	RFW_ASPE2ACT_INFO E2AspActInfo;

	ProtInit_ASP();

	if (!s_bStopTraceEventEna) {
		dwRet = RFWERR_N_BASP_STOPTRC_NOUSE;
	}

	if (dwRet == RFWERR_OK) {
		GetE2ActTable(RF_ASPE2ACTCH_TRCEV, &E2AspActInfo);
		*pu64FactorSingleEvent = E2AspActInfo.u64LinkFact;
		*pdwFactorMultiEvent = 0;	// 予約変数
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
* E2エミュレータ拡張機能のトレース停止イベントの削除
* @param なし
* @retval RFWERR_OK
* @retval RFWERR_N_BMCU_RUN
* @retval RFWERR_N_BASP_STOPTRC_NOUSE
*/
//=============================================================================
DWORD DO_ClrStopTraceEvent(void)
{
	RFWERR dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	if (dwRet == RFWERR_OK) {
		if (!s_bStopTraceEventEna) {
			dwRet = RFWERR_N_BASP_STOPTRC_NOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		ClrE2ActTable(RF_ASPE2ACTCH_TRCEV);
		ClearSingleEvent((UINT64)1 << RF_ASPE2ACTCH_TRCEV);
		s_bStopTraceEventEna = FALSE;
	}

	ProtEnd_ASP();

	return dwRet;
}

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
/**
* E2エミュレータ拡張機能のブレークイベントの初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitBreakEvent(void)
{
	s_bBreakEventEna = FALSE;
}


//=============================================================================
/**
* E2エミュレータ拡張機能のブレークイベントアクション有効フラグの参照
* @param なし
* @retval TRUE:ブレークイベント有効   FALSE:無効
*/
//=============================================================================
BOOL IsBreakEventActEna(void)
{
	return s_bBreakEventEna;
}


//=============================================================================
/**
* E2エミュレータ拡張機能のブレークイベントのE2トリガ有効状態の参照
* @param なし
* @retval TRUE:ブレークイベント(E2トリガ)が何らかのE2アクションイベントに関連付けされている
* @retval FALSE: E2トリガとして未使用
*/
//=============================================================================
BOOL IsBreakEventTrgEna(void)
{
	BOOL bRet;
	RFW_ASPE2TRG_INFO AspE2TrgInfo;
	RFW_ASPE2ACT_INFO AspE2ActInfo;
	DWORD i;

	GetE2TrgTable(RF_E2ASP_TRG_CH_BRKEV, &AspE2TrgInfo);

	bRet = FALSE;

	/* ブレークイベント(E2トリガ条件)が何らかのE2アクションに割り付けられている場合、TRUEとする */
	if (AspE2TrgInfo.dwFactSw == RFW_EVENTLINK_SWON) {
		for (i = 0; i < RF_E2ACT_TABLE_MAX; i++) {
			GetE2ActTable(i, &AspE2ActInfo);
			if ( (AspE2ActInfo.u64LinkFact & (1 << RF_E2ASP_TRG_CH_BRKEV) ) != 0) {
				bRet = TRUE;
				break;
			}
		}
	}
	return bRet;
}


//=============================================================================
/**
* E2エミュレータ拡張機能のトレース停止イベントの初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitStopTraceEvent(void)
{
	s_bStopTraceEventEna = FALSE;
}


//=============================================================================
/**
* E2エミュレータ拡張機能のトレース停止イベントアクション有効フラグの参照
* @param なし
* @retval TRUE:トレース停止イベント有効   FALSE:無効
*/
//=============================================================================
BOOL IsStopTraceEventActEna(void)
{
	return s_bStopTraceEventEna;
}

