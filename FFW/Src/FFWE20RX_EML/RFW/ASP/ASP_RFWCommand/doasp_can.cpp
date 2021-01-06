///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_can.cpp
 * @brief CANモニタコマンドの実装ファイル
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
#include "doasp_can.h"
#include "emudef.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"

// ファイル内static変数の宣言
static BOOL	s_bCanMonSw[2];
static BOOL s_bCanMonEvSw[2];
static RFW_CANMON_DATA s_CanMonData[2];
static RFW_CANMONEV_DATA s_CanMonEvData[2];

// ファイル内static関数の宣言
static DWORD ClrCanMonEvMain(BYTE byChNo);


///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* CANモニタサンプリングの設定
* @param byChNo チャネル番号
* @param pCanMonData CANモニタサンプリング情報
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
* @retval RFWERR_N_BASP_CANMONOFF  CANモニタが有効化されていない
*/
//=============================================================================
DWORD DO_SetCanMon(BYTE byChNo, const RFW_CANMON_DATA *pCanMonData)
{
	DWORD	dwRet = RFWERR_OK;
	RFW_ASPCONF_DATA	AspConfData;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// SetConfigurationでCANを使う設定になっているかチェック
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if (byChNo == RF_ASPCANMON_CH0) {
			if ((AspConfData.dwFunction0 & RF_ASPCONF_CAN_MON_CH0) == 0) {
				dwRet = RFWERR_N_BASP_CANMONOFF;
			}
		} else {
			if ((AspConfData.dwFunction1 & RF_ASPCONF_CAN_MON_CH1) == 0) {
				dwRet = RFWERR_N_BASP_CANMONOFF;
			}
		}
	}

	// CANモニタサンプリング条件の保存
	if (dwRet == RFWERR_OK) {
		s_bCanMonSw[byChNo] = TRUE;
		memcpy((void*)&s_CanMonData[byChNo], pCanMonData, sizeof(s_CanMonData[byChNo]));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* CANモニタのサンプリング設定の取得
* @param byChNo チャネル番号
* @param pCanMonData CANモニタサンプリング情報
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_CANMON_NOUSE　指定したチャネルは無効
*/
//=============================================================================
DWORD DO_GetCanMon(BYTE byChNo, RFW_CANMON_DATA *pCanMonData)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	if (!s_bCanMonSw[byChNo]) {
		// 指定したチャネルは無効
		dwRet = RFWERR_N_BASP_CANMON_NOUSE;
	} else {
		memcpy((void*)pCanMonData, &s_CanMonData[byChNo], sizeof(s_CanMonData[byChNo]));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* CANモニタのサンプリング設定の削除
* @param byChNo チャネル番号
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
* @retval RFWERR_N_BASP_CANMON_NOUSE　指定したチャネルは無効
* @retval RFWERR_N_BASP_FACT_CANTCLR　CANモニタイベントとして使用中
*/
//=============================================================================
DWORD DO_ClrCanMon(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 指定したチャネルが有効かチェック
	if (dwRet == RFWERR_OK) {
		if (!s_bCanMonSw[byChNo]) {
			// 指定したチャネルは無効
			dwRet = RFWERR_N_BASP_CANMON_NOUSE;
		}
	}

	// 指定したチャネルがCANモニタイベントとして使用されていないかチェック
	if (dwRet == RFWERR_OK) {
		if (s_bCanMonEvSw[byChNo]) {
			dwRet = RFWERR_N_BASP_FACT_CANTCLR;
		}
	}

	if (dwRet == RFWERR_OK) {
		// CANモニタサンプリング条件管理用変数の初期化
		InitAspCanMon(byChNo);
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* CANモニタイベントの設定
* @param byChNo チャネル番号
* @param pCanMonitorEvent CANモニタイベント情報
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
* @retval RFWERR_N_BASP_CANMON_NOUSE　指定したチャネルは使用していない
*/
//=============================================================================
DWORD DO_SetCanMonEvent(BYTE byChNo, const RFW_CANMONEV_DATA * pCanMonitorEvent)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwTrgCh;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 指定したチャネルが設定されているかチェック
	if (dwRet == RFWERR_OK) {
		if (!s_bCanMonSw[byChNo]) {
			// 指定したチャネルは無効
			dwRet = RFWERR_N_BASP_CANMON_NOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		// CANモニタイベント条件の保存
		s_bCanMonEvSw[byChNo] = TRUE;
		memcpy((void*)&s_CanMonEvData[byChNo], pCanMonitorEvent, sizeof(s_CanMonEvData[byChNo]));

		// E2トリガのテーブルへの保存
		// ->チャネルは統一することとした
		if (byChNo == RF_ASPCANMON_CH0) {
			dwTrgCh = RF_ASPE2TRGCH_COMUMONCH0_MATCH;
		} else {
			dwTrgCh = RF_ASPE2TRGCH_COMUMONCH1_MATCH;
		}
		SetE2TrgTable(dwTrgCh, RFW_EVENTLINK_PULSE);
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* CANモニタイベント設定の参照
* @param byChNo チャネル番号
* @param pCanMonitorEvent CANモニタイベント情報
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_CANMON_NOUSE　指定したチャネルは使用していない
*/
//=============================================================================
DWORD DO_GetCanMonEvent(BYTE byChNo, RFW_CANMONEV_DATA * pCanMonitorEvent)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	if (!s_bCanMonEvSw[byChNo]) {
		// 指定したチャネルは無効
		dwRet = RFWERR_N_BASP_CANMON_NOUSE;
	} else {
		memcpy((void*)pCanMonitorEvent, &s_CanMonEvData[byChNo], sizeof(s_CanMonEvData[byChNo]));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* CANモニタイベント設定の削除
* @param byChNo チャネル番号
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
* @retval RFWERR_N_BASP_CANMON_NOUSE　指定したチャネルは使用していない
* @retval RFWERR_N_BASP_FACT_CANTCLR　E2アクションにリンクしているためクリアできない
*/
//=============================================================================
DWORD DO_ClrCanMonEvent(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 指定したチャネルが設定されているかチェック
	if (dwRet == RFWERR_OK) {
		if (!s_bCanMonEvSw[byChNo]) {
			// 指定したチャネルは無効
			dwRet = RFWERR_N_BASP_CANMON_NOUSE;
		}
	}

	// 削除のメイン処理
	if (dwRet == RFWERR_OK) {
		dwRet = ClrCanMonEvMain(byChNo);
	}

	ProtEnd_ASP();

	return dwRet;
}


///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
* CANモニタイベント削除のメイン処理
* @param byChNo チャネル番号
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_FACT_CANTCLR　E2アクションにリンクしているためクリアできない
*/
//=============================================================================
static DWORD ClrCanMonEvMain(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	UINT64	u64SingleEvent;

	if (byChNo == RF_ASPCANMON_CH0) {
		u64SingleEvent = (UINT64)((UINT64)0x1 << RF_ASPE2TRGCH_COMUMONCH0_MATCH);
	} else {
		u64SingleEvent = (UINT64)((UINT64)0x1 << RF_ASPE2TRGCH_COMUMONCH1_MATCH);
	}

	// アクションとリンクされていないかチェック
	dwRet = CheckSingleEvent(RFW_ASPFUNC_CLR, u64SingleEvent, 0, 0);

	if (dwRet == RFWERR_OK) {
		// CANモニタイベント管理用変数の初期化
		InitAspCanMonEv(byChNo);

		// E2トリガのテーブル初期化
		ClrE2TrgTable(find_first_bit(u64SingleEvent));
	}

	return dwRet;
}



///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* CANモニタ用内部管理変数の初期化
* @note 初期化するch番号を指定する。0xFFを指定した場合は全chを初期化する。
* @param byChNo チャネル番号
* @retval なし
*/
//=============================================================================
void InitAspCanMon(BYTE byChNo)
{
	BYTE	byData;
	if (byChNo == 0xFF) {
		for (byData = 0; byData < 2; byData++) {
			s_bCanMonSw[byData] = FALSE;
			s_CanMonData[byData].byCanMode = RF_ASPCAN_STANDARD_ID;
			s_CanMonData[byData].wBaudrate = RF_ASPCAN_1MBPS;
			s_CanMonData[byData].bySamplingPoint = 1;
			s_CanMonData[byData].wFastBaudrate = 0;
		}
	} else if (byChNo == 0) {
		s_bCanMonSw[0] = FALSE;
		s_CanMonData[0].byCanMode = RF_ASPCAN_STANDARD_ID;
		s_CanMonData[0].wBaudrate = RF_ASPCAN_1MBPS;
		s_CanMonData[0].bySamplingPoint = 1;
		s_CanMonData[0].wFastBaudrate = 0;
	} else {
		s_bCanMonSw[1] = FALSE;
		s_CanMonData[1].byCanMode = RF_ASPCAN_STANDARD_ID;
		s_CanMonData[1].wBaudrate = RF_ASPCAN_1MBPS;
		s_CanMonData[1].bySamplingPoint = 1;
		s_CanMonData[0].wFastBaudrate = 0;
	}
}


//=============================================================================
/**
* CANモニタイベント用内部管理変数の初期化
* @note 初期化するch番号を指定する。0xFFを指定した場合は全chを初期化する。
* @param byChNo チャネル番号
* @retval なし
*/
//=============================================================================
void InitAspCanMonEv(BYTE byChNo)
{
	BYTE	byData;
	if (byChNo == 0xFF) {
		for (byData = 0; byData < 2; byData++) {
			s_bCanMonEvSw[byData] = FALSE;
			memset((void*)&s_CanMonEvData[byData], 0x00, sizeof(s_CanMonEvData[byData]));
		}
	} else if (byChNo == 0) {
		s_bCanMonEvSw[0] = FALSE;
		memset((void*)&s_CanMonEvData[0], 0x00, sizeof(s_CanMonEvData[0]));
	} else {
		s_bCanMonEvSw[1] = FALSE;
		memset((void*)&s_CanMonEvData[1], 0x00, sizeof(s_CanMonEvData[1]));
	}
}


///////////////////////////////////////////////////////////////////////////////
// グローバル関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* CANモニタ有効/無効の参照
* @param なし
* @retval TRUE  CANモニタ有効
* @retval FALSE CANモニタ無効
*/
//=============================================================================
BOOL IsAspCanMonEna(void)
{
	if (s_bCanMonSw[RF_ASPCANMON_CH0] || s_bCanMonSw[RF_ASPCANMON_CH1]) {
		return TRUE;
	}

	return FALSE;
}


//=============================================================================
/**
* ch指定のCANモニタ有効/無効の取得
* @param byChNo チャネル番号
* @retval 0 CANモニタ有効
* @retval 1 CANモニタ無効
*/
//=============================================================================
BOOL GetCanMonEna(BYTE byChNo)
{
	return s_bCanMonSw[byChNo];
}


//=============================================================================
/**
* ch指定のCANモニタ有効/無効の取得
* @param byChNo チャネル番号
* @retval 0 CANモニタトリガ有効
* @retval 1 CANモニタトリガ無効
*/
//=============================================================================
BOOL GetCanMonEvEna(BYTE byChNo)
{
	return s_bCanMonEvSw[byChNo];
}


//=============================================================================
/**
* CANモニタのサンプリング設定の取得
* @param byChNo チャネル番号
* @param pCanMonData
* @retval なし
*/
//=============================================================================
void GetCanMonData(BYTE byChNo, RFW_CANMON_DATA *pCanMonData)
{
	memcpy((void*)pCanMonData, &s_CanMonData[byChNo], sizeof(s_CanMonData[byChNo]));
}


//=============================================================================
/**
* CANモニタイベント設定の取得
* @param byChNo チャネル番号
* @param pCanMonEvData
* @retval なし
*/
//=============================================================================
void GetCanMonEvData(BYTE byChNo, RFW_CANMONEV_DATA *pCanMonEvData)
{
	memcpy((void*)pCanMonEvData, &s_CanMonEvData[byChNo], sizeof(s_CanMonEvData[byChNo]));
}

