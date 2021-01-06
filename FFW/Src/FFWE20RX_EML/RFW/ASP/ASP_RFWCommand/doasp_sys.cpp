///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_sys.cpp
 * @brief E2拡張機能システムコマンドの実装ファイル
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
#include "doasp_sys.h"
#include "rfwasp_sys.h"
#include "emudef.h"
#include "rfwasp_closed.h"
#include "doasp_per.h"
#include "doasp_exe.h"
#include "doasp_can.h"
#include "doasp_trg.h"
#include "doasp_sys_family.h"

#include "doasp_pwr.h"
#include "asp_setting.h"

// ファイル内static変数の宣言
static BYTE	s_byAspSwitch = RF_ASPSW_OFF;
static RFW_ASPCONF_DATA s_AspConfData;
static BYTE s_byAspConfFirstFlag;
static RFW_ASPE2TRG_INFO s_AspE2trgTable[RF_E2TRG_TABLE_MAX];
static RFW_ASPE2ACT_INFO s_AspE2actTable[RF_E2ACT_TABLE_MAX];

// ファイル内static関数の宣言
static void InitAspTrgAct(void);
static void InitAspSwitch(void);
static void InitAspConf(void);
static BOOL IsMultiBitDword(DWORD);
static BOOL IsMultiBitUint64(UINT64);


// トリガアクションのサポートテーブル
static BYTE s_byTrgAct[RFW_EVENTLINK_CH_MAX][RFW_EVENTLINK_CH_MAX] = {
//			 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
			{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// 0
			{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 1
			{ 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 2
			{ 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 3
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 4
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 5
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 6
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 7
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 8
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 9
			{ 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 10
			{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 11
			{ 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 12
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 13
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 14
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 15
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 16
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 17
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 18
			{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 19
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 20
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 21
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 22
			{ 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 23
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 24
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 25
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 26
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 28
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 29
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 30
			{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };// 31


///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * E2エミュレータ拡張機能のスイッチ設定
 * @param byAspSwitch 拡張機能のON/OFF情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
 * @retval RFWERR_N_ASP_DISABLE_EMU  E2以外のエミュレータは有効化できない
 * @retval RFWERR_N_ASP_DISABLE_ASYNC  非同期デバッグモード中は有効化できない
 */
//=============================================================================
DWORD DO_SetAspSw(BYTE byAspSwitch)
{
	DWORD	dwRet = RFWERR_OK;
	BYTE byAspSwitchOld;

	ProtInit_ASP();

	byAspSwitchOld = s_byAspSwitch;

	// エミュレータがE2かチェック
	if (dwRet == RFWERR_OK) {
		if (IsEmlE2() == FALSE) {
			dwRet = RFWERR_N_ASP_DISABLE_EMU;
		}
	}

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// [RH限定]
	// 非同期デバッグモードかチェック
	if (dwRet == RFWERR_OK) {
		if ((IsAsyncMode() == TRUE)
		&&  (byAspSwitch == RF_ASPSW_ON)) {
			dwRet = RFWERR_N_ASP_DISABLE_ASYNC;
		}
	}

	if (dwRet == RFWERR_OK) {
		s_byAspSwitch = byAspSwitch;

		/* E2拡張機能が無効に切り替わった場合、全機能の管理変数を初期化する */
		if (byAspSwitchOld == RF_ASPSW_ON && s_byAspSwitch == RF_ASPSW_OFF) {
			InitAsp();
		}
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
 * E2エミュレータ拡張機能のスイッチ設定参照
 * @param pbyAspSwitch 拡張機能のON/OFF情報
 * @retval RFWERR_OK  正常動作
 */
//=============================================================================
DWORD DO_GetAspSw(BYTE *pbyAspSwitch)
{
	DWORD dwRet = RFWERR_OK;

	ProtInit_ASP();

	*pbyAspSwitch = s_byAspSwitch;

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
 * E2エミュレータ拡張機能サポート状況の参照
 * @param pdwAspComonFunction 共通機能のサポート状況
 * @param pdwAspSeriesFunction ファミリ固有のサポート状況
 * @param pdwAspStorage ストレージ関連のサポート状況
 * @param pdwAspOption オプションボード接続状況
 * @retval RFWERR_OK  正常動作
 */
//=============================================================================
DWORD DO_GetAspFunc(DWORD *pdwAspCommonFunction, DWORD *pdwAspSeriesFunction, DWORD *pdwAspStorage, DWORD *pdwAspOption)
{
	ProtInit_ASP();

	// 共通機能のサポート状況
	*pdwAspCommonFunction = GetAspCommonFunc();

	// ファミリ固有のサポート機能
	*pdwAspSeriesFunction = GetAspSeriesFunc();

	// ストレージのサポート状況
	*pdwAspStorage = GetAspStorage();

	// オプションボード接続状況
	*pdwAspOption = 0;

	ProtEnd_ASP();

	return RFWERR_OK;
}

//=============================================================================
/**
* SetAspConfiguration
* @param pAspConfiguration 拡張機能のコンフィグ情報
* @retval RFWERR_OK
* @retval RFWERR_N_BMCU_RUN
* @retval RFWERR_N_BASP_FUNCEXCLUSIVE
* @retval RFWERR_N_BASP_PINEXCLUSIVE
* @retval RFWERR_N_BASP_PWRMON_E2PWROFF
* @retval RFWERR_N_BASP_ASPIF_PWROFF
* @retval RFWERR_N_RFW_ARG
* @retval RFWERR_N_BASP_FUNC_USED
*/
//=============================================================================
DWORD DO_SetAspConfiguration(const RFW_ASPCONF_DATA *pAspConfiguration)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 端子1-4の機能割り当て排他チェック
	if (dwRet == RFWERR_OK) {
		// 機能割り当て排他チェック。複数bitが1だとエラー
		if (IsMultiBitDword(pAspConfiguration->dwFunction0)) {
			dwRet = RFWERR_N_BASP_PINEXCLUSIVE;
		}
	}

	// 端子5-8の機能割り当て排他チェック
	if (dwRet == RFWERR_OK) {
		// 機能割り当て排他チェック。複数bitが1だとエラー
		if (IsMultiBitDword(pAspConfiguration->dwFunction1)) {
			dwRet = RFWERR_N_BASP_PINEXCLUSIVE;
		}
	}

	// 端子9-12はの機能割り当て排他チェック
	if (dwRet == RFWERR_OK) {
		// 外部トリガとロジアナの同時使用は禁止
		if (((pAspConfiguration->dwFunction2 & RF_ASPCONF_TRIGGER_MASK) != 0)
		&& ((pAspConfiguration->dwFunction2 & RF_ASPCONF_LOGICANALY_MASK) != 0)) {
			dwRet = RFWERR_N_BASP_PINEXCLUSIVE;
		}
	}

	// ASP I/Fを使用しない機能
	// 排他チェック
	if (dwRet == RFWERR_OK) {
		if (((pAspConfiguration->dwFunction3 & RF_ASPCONF_EXT_STRACE) != 0)
		&& ((pAspConfiguration->dwFunction3 & RF_ASPCONF_LOGGER) != 0)) {
			// ソフトトレースとロガーは排他
			dwRet = RFWERR_N_BASP_FUNCEXCLUSIVE;

		} else if (((pAspConfiguration->dwFunction3 & RF_ASPCONF_EXT_STRACE) != 0)
		&& ((pAspConfiguration->dwFunction3 & RF_ASPCONF_PC_SAMPLING) != 0)) {
			// ソフトトレースとPCサンプリングは排他
			dwRet = RFWERR_N_BASP_FUNCEXCLUSIVE;

		} else if (((pAspConfiguration->dwFunction3 & RF_ASPCONF_EXT_STRACE) != 0)
		&& ((pAspConfiguration->dwFunction3 & RF_ASPCONF_MON_EV) != 0)) {
			// ソフトトレースとモニタイベントは排他
			dwRet = RFWERR_N_BASP_FUNCEXCLUSIVE;

		} else if (((pAspConfiguration->dwFunction3 & RF_ASPCONF_EXT_STRACE) != 0)
		&& ((pAspConfiguration->dwFunction3 & RF_ASPCONF_STOPTRC_EV) != 0)) {
			// ソフトトレースとトレース停止機能は排他
			dwRet = RFWERR_N_BASP_FUNCEXCLUSIVE;

		} else if (((pAspConfiguration->dwFunction3 & RF_ASPCONF_MON_EV) != 0)
		&& ((pAspConfiguration->dwFunction3 & RF_ASPCONF_STOPTRC_EV) != 0)) {
			// モニタイベントとトレース停止機能は排他
			dwRet = RFWERR_N_BASP_FUNCEXCLUSIVE;

		} else {
			// ASP I/Fを使用しない機能のチェックは以上
		}
	}

	// 全体の排他機能チェック
	if (dwRet == RFWERR_OK) {
		// PCストレージ使用の疑似ターゲット時は、すべての機能と排他
		if (((pAspConfiguration->dwFunction0 & RF_ASPCONF_PSEUDO_MASK) != 0)
			&& (pAspConfiguration->dwStorage == RF_ASPSTORAGE_PC)) {
			if ((pAspConfiguration->dwFunction1 != 0)
				|| (pAspConfiguration->dwFunction2 != 0)
				|| (pAspConfiguration->dwFunction3 != 0)) {
				dwRet = RFWERR_N_BASP_FUNCEXCLUSIVE;
			}
		}
	}

	// サポート状況の確認
	if (dwRet == RFWERR_OK) {
		dwRet = ChkSupportAspFunction(pAspConfiguration);
	}

	// 電源供給のチェック
	if (dwRet == RFWERR_OK) {
		// パワーモニタは電源供給しないと計測できないためエラー
		if (((pAspConfiguration->dwFunction3 & RF_ASPCONF_POWER_MON) != 0) && (IsSupplyPower() == FALSE)) {
			dwRet = RFWERR_N_BASP_PWRMON_E2PWROFF;

		// 通信モニタ、疑似ターゲットは電源供給が必要
		} else if ((pAspConfiguration->dwFunction0 != 0) && (IsSupplyPowerASP() == FALSE)) {
			dwRet = RFWERR_N_BASP_ASPIF_PWROFF;

		// 通信モニタは電源供給が必要
		} else if ((pAspConfiguration->dwFunction1 != 0) && (IsSupplyPowerASP() == FALSE)) {
			dwRet = RFWERR_N_BASP_ASPIF_PWROFF;

		// 外部トリガ、ロジアナは電源供給が必要
		} else if ((pAspConfiguration->dwFunction2 != 0) && (IsSupplyPowerASP() == FALSE)) {
			dwRet = RFWERR_N_BASP_ASPIF_PWROFF;

		} else {
			// 上記以外は電源供給に関係なく使用可
		}
	}

	// 無効設定する機能の使用状態確認
	if (dwRet == RFWERR_OK) {
		dwRet = ChkInvalidAspFunction(pAspConfiguration);
	}

	if (dwRet == RFWERR_OK) {
		/* ブレークイベント(E2トリガ検出)の有効/無効設定 */
		if ((pAspConfiguration->dwFunction3 & RF_ASPCONF_BRK_EV) != 0) {
			SetE2TrgTable(RF_E2ASP_TRG_CH_BRKEV, RFW_EVENTLINK_PULSE);
		}
		else {
			ClrE2TrgTable(RF_E2ASP_TRG_CH_BRKEV);
		}

		// 管理用変数に格納
		if (s_byAspConfFirstFlag) {
			// 初回のみすべての設定を反映
			memcpy(&s_AspConfData, pAspConfiguration, sizeof(s_AspConfData));
			s_byAspConfFirstFlag = FALSE;
		} else {
			// 2回目以降は端子を使用する機能の変更は不可
			s_AspConfData.dwStorage = pAspConfiguration->dwStorage;
			s_AspConfData.dwCnt = pAspConfiguration->dwCnt;
			s_AspConfData.dwMaxStorage = pAspConfiguration->dwMaxStorage;
			s_AspConfData.dwFunction3 = pAspConfiguration->dwFunction3;
		}
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
* GetAspConfiguration
* @param pAspConfiguration RFW_ASPCONF_DATA *
* @retval RFWERR_OK
*/
//=============================================================================
DWORD DO_GetAspConfiguration(RFW_ASPCONF_DATA const *pAspConfiguration)
{
	DWORD dwRet = RFWERR_OK;

	ProtInit_ASP();

	memcpy((void*)pAspConfiguration, &s_AspConfData, sizeof(s_AspConfData));

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* 複数bitが1になっているかチェックする関数(DWORD版)
* note 2つ以上のbitが1だとTRUEを返す
* @param dwData
* @retval TRUE  複数のbitが1になっている
* @retval FALSE 上記以外
*/
//=============================================================================
static BOOL IsMultiBitDword(DWORD dwData)
{
	dwData = (dwData & 0x55555555) + (dwData >> 1 & 0x55555555);
	dwData = (dwData & 0x33333333) + (dwData >> 2 & 0x33333333);
	dwData = (dwData & 0x0f0f0f0f) + (dwData >> 4 & 0x0f0f0f0f);
	dwData = (dwData & 0x00ff00ff) + (dwData >> 8 & 0x00ff00ff);
	dwData = (dwData & 0x0000ffff) + (dwData >> 16 & 0x0000ffff);
	if (dwData > 1) {
		return TRUE;
	} else {
		return FALSE;
	}
}


//=============================================================================
/**
* 複数bitが1になっているかチェックする関数(UINT64版)
* note 2つ以上のbitが1だとTRUEを返す
* @param u64Data
* @retval TRUE  複数のbitが1になっている
* @retval FALSE 上記以外
*/
//=============================================================================
static BOOL IsMultiBitUint64(UINT64 u64Data)
{
	u64Data = (u64Data & 0x5555555555555555) + (u64Data >> 1 & 0x5555555555555555);
	u64Data = (u64Data & 0x3333333333333333) + (u64Data >> 2 & 0x3333333333333333);
	u64Data = (u64Data & 0x0f0f0f0f0f0f0f0f) + (u64Data >> 4 & 0x0f0f0f0f0f0f0f0f);
	u64Data = (u64Data & 0x00ff00ff00ff00ff) + (u64Data >> 8 & 0x00ff00ff00ff00ff);
	u64Data = (u64Data & 0x0000ffff0000ffff) + (u64Data >> 16 & 0x0000ffff0000ffff);
	u64Data = (u64Data & 0x00000000ffffffff) + (u64Data >> 32 & 0x00000000ffffffff);
	if (u64Data > 1) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}


//=============================================================================
/**
* E2トリガのチェック関数
* note 
* @param dwFunc 設定時(0)or削除時(1)
* @param u64SingleEvent シングルE2トリガ情報
* @param dwActType アクションタイプ
* @param dwActChNo アクションのチャネル番号
* @retval RFWERR_N_BASP_SGLLNK_ILLEGAL
* @retval RFWERR_N_BASP_FACT_NONSEL
* @retval RFWERR_N_BASP_FACT_NOSUPPORT
* @retval RFWERR_N_BASP_FACT_DISABLE
* @retval RFWERR_N_BASP_LFACT_CANTSET
* @retval RFWERR_N_BASP_PFACT_CANTSET
* @retval RFWERR_N_BASP_FACT_CANTCLR
* @retval RFWERR_OK
*/
//=============================================================================
DWORD CheckSingleEvent(DWORD dwFunc, UINT64 u64SingleEvent, DWORD dwActType, DWORD dwActChNo)
{
	RFW_ASPE2TRG_INFO AspE2TrgInfo;
	DWORD	dwChNo;


	if (dwFunc == RFW_ASPFUNC_SET) {
		if (IsMultiBitUint64(u64SingleEvent)) {
			// 複数の要因は指定できない
			return RFWERR_N_BASP_SGLLNK_ILLEGAL;
		}

		if (u64SingleEvent == 0) {
			// E2トリガが指定されていない
			return RFWERR_N_BASP_FACT_NONSEL;
		}

		// イベント情報からトリガチャネル番号を算出
		dwChNo = find_first_bit(u64SingleEvent);
		GetE2TrgTable(dwChNo, &AspE2TrgInfo);

		if ((u64SingleEvent & ~(GetAspSeriesFact())) != 0) {
			// 指定したE2トリガは未サポート
			return RFWERR_N_BASP_FACT_NOSUPPORT;
		}

		if (s_byTrgAct[dwChNo][dwActChNo] == 0) {
			// サポートしていないトリガとアクションの組み合わせ
			return RFWERR_N_BASP_COMB_NOSUPPORT;
		}

		if ((AspE2TrgInfo.dwFactType != dwActType)
		&& (AspE2TrgInfo.dwFactType != RFW_EVENTLINK_NONSEL)) {
			if (dwActType == RFW_EVENTLINK_PULSE) {
				// トリガがレベル型でアクションがパルス型なので設定できない
				return RFWERR_N_BASP_LFACT_CANTSET;
			} else if (dwActType == RFW_EVENTLINK_LEVEL) {
				// トリガがパルス型でアクションがレベル型なので設定できない
				return RFWERR_N_BASP_PFACT_CANTSET;
			} else {
				// どちらでも可なアクションのためエラーなし
			}
		}

		if (AspE2TrgInfo.dwFactSw == 0) {
			// 指定したE2トリガは無効
			return RFWERR_N_BASP_FACT_DISABLE;
		}
	} else {
		// イベント情報からトリガチャネル番号を算出
		dwChNo = find_first_bit(u64SingleEvent);

		GetE2TrgTable(dwChNo, &AspE2TrgInfo);
		if (AspE2TrgInfo.u64LinkAct != 0) {
			// アクションが設定済みのためクリア不可
			return RFWERR_N_BASP_FACT_CANTCLR;
		}
	}
	return RFWERR_OK;
}


//=============================================================================
/**
* シングルイベント情報の保存
* note
* @param dwFactChNo E2トリガチャネル番号
* @param u64ActCh E2アクションのbit情報
* @retval なし
*/
//=============================================================================
void SaveSingleEvent(DWORD dwFactChNo, UINT64 u64ActCh)
{
	s_AspE2trgTable[dwFactChNo].u64LinkAct |= u64ActCh;
}


//=============================================================================
/**
* シングルイベント情報の削除
* note
* @param u64ActCh E2アクションのbit情報
* @retval なし
*/
//=============================================================================
void ClearSingleEvent(UINT64 u64ActCh)
{
	int	count;

	// すべてのE2トリガ情報から指定のアクションbitを0にする
	for (count = 0; count < RFW_EVENTLINK_CH_MAX; count++) {
		s_AspE2trgTable[count].u64LinkAct &= ~u64ActCh;
	}
}


//=============================================================================
/**
* コンフィグレーション設定の取得
* note
* @param pAspConfiguration 拡張機能のコンフィグ情報
* @retval なし
*/
//=============================================================================
void GetAspConfData(RFW_ASPCONF_DATA const *pAspConfiguration)
{
	memcpy((void*)pAspConfiguration, &s_AspConfData, sizeof(s_AspConfData));
}


///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// グローバル関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * E2拡張機能の有効/無効状態を取得する関数
 * @param なし
 * @retval TRUE ：E2拡張機能有効
 * @retval FALSE：E2拡張機能無効
 */
//=============================================================================
BOOL IsAspOn(void)
{
	if (s_byAspSwitch == RF_ASPSW_ON) {
		return TRUE;
	}
	return FALSE;
}


//=============================================================================
/**
* 指定したチャネルのE2トリガ情報の取得
* @param dwChNo チャネル番号
* @param pAspE2TrgInfo E2トリガ情報
* @retval なし
*/
//=============================================================================
void GetE2TrgTable(DWORD dwChNo, RFW_ASPE2TRG_INFO *pAspE2TrgInfo)
{
	memcpy((void*)pAspE2TrgInfo, &s_AspE2trgTable[dwChNo], sizeof(*pAspE2TrgInfo));
}


//=============================================================================
/**
* 指定したチャネルのE2トリガ情報の設定
* @param dwChNo チャネル番号
* @param dwFactType E2トリガのタイプ(パルス or レベル)
* @retval なし
*/
//=============================================================================
void SetE2TrgTable(DWORD dwChNo, DWORD dwFactType)
{
	s_AspE2trgTable[dwChNo].dwFactSw = RFW_EVENTLINK_SWON;
	s_AspE2trgTable[dwChNo].dwFactType = dwFactType;
	// この時点でアクションは変更しない
}


//=============================================================================
/**
* 指定したチャネルのE2トリガ情報の削除
* @param dwChNo チャネル番号
* @retval なし
*/
//=============================================================================
void ClrE2TrgTable(DWORD dwChNo)
{
	s_AspE2trgTable[dwChNo].dwFactSw = RFW_EVENTLINK_SWOFF;
	s_AspE2trgTable[dwChNo].dwFactType = RFW_EVENTLINK_NONSEL;
	// この時点でアクションは変更しない
}


//=============================================================================
/**
* 指定したチャネルのE2アクション情報の取得
* @param dwChNo チャネル番号
* @param pAspE2actInfo E2アクション情報
* @retval なし
*/
//=============================================================================
void GetE2ActTable(DWORD dwChNo, RFW_ASPE2ACT_INFO *pAspE2actInfo)
{
	memcpy((void*)pAspE2actInfo, &s_AspE2actTable[dwChNo], sizeof(*pAspE2actInfo));
}


//=============================================================================
/**
* 指定したチャネルのE2アクション情報の設定
* @param dwChNo チャネル番号
* @param dwType E2アクションのタイプ(パルス or レベル or 両方)
* @param u64FactEvent リンクするE2トリガチャネルのbit情報
* @retval なし
*/
//=============================================================================
void SetE2ActTable(DWORD dwChNo, DWORD dwType, UINT64 u64FactEvent)
{
	s_AspE2actTable[dwChNo].dwActSw = RFW_EVENTLINK_SWON;
	s_AspE2actTable[dwChNo].dwActType = dwType;
	s_AspE2actTable[dwChNo].u64LinkFact = u64FactEvent;
}


//=============================================================================
/**
* 指定したチャネルのE2アクション情報の削除
* @param dwChNo チャネル番号
* @retval なし
*/
//=============================================================================
void ClrE2ActTable(DWORD dwChNo)
{
	s_AspE2actTable[dwChNo].dwActSw = RFW_EVENTLINK_SWOFF;
	s_AspE2actTable[dwChNo].dwActType = RFW_EVENTLINK_NONSEL;
	s_AspE2actTable[dwChNo].u64LinkFact = 0;
}


///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * E2エミュレータ拡張機能の初期化
 * note 接続処理で呼び出すこと
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitAsp(void)
{
	// メインスイッチの初期化
	InitAspSwitch();

	// 拡張機能の構成を初期化
	InitAspConf();

	// パワーモニタの初期化
	InitAspPwrMon();

	// パワーモニタイベントの初期化
	InitAspPwrMonEv();

	// 時間測定の初期化
	InitAspPerCond(0xFF);

	// 外部トリガ入出力の初期化
	InitAspExTrg();

	// ブレークイベントの初期化
	InitBreakEvent();

	// トレース停止イベントの初期化
	InitStopTraceEvent();

	// CANモニタの初期化
	InitAspCanMon(0xFF);

	// CANモニタイベントの初期化
	InitAspCanMonEv(0xFF);

	// ファミリ依存のASP機能の初期化
	InitAsp_family();

	// E2トリガアクションの初期化
	InitAspTrgAct();

}


//=============================================================================
/**
 * E2拡張機能の有効/無効状態を初期化する関数
 * @param なし
 * @retval なし
 */
//=============================================================================
static void InitAspSwitch(void)
{
	s_byAspSwitch = RF_ASPSW_OFF;
}


//=============================================================================
/**
* 拡張機能の構成初期化
* @param なし
* @retval なし
*/
//=============================================================================
static void InitAspConf(void)
{
	s_AspConfData.dwStorage = 0;	// 記録しない
	s_AspConfData.dwCnt = 0;
	s_AspConfData.dwMaxStorage = 0;
	s_AspConfData.dwFunction3 = 0;	// ASP I/Fを使用しない拡張機能の機能割り当てなし
}


//=============================================================================
/**
* 拡張機能の構成初期化(端子機能)
* @note ターゲット接続時のみ初期化する
* @note ASP ONO/OFFでは初期化しないので分離
* @param なし
* @retval なし
*/
//=============================================================================
void InitAspPinConf(void)
{
	s_byAspConfFirstFlag = TRUE;
	s_AspConfData.dwFunction0 = 0;	// 端子1-4の機能割り当てなし
	s_AspConfData.dwFunction1 = 0;	// 端子5-8の機能割り当てなし
	s_AspConfData.dwFunction2 = 0;	// 端子9-12の機能割り当てなし
}


//=============================================================================
/**
* E2トリガ・アクション管理用変数の初期化
* @param なし
* @retval なし
*/
//=============================================================================
static void InitAspTrgAct(void)
{
	memset(s_AspE2trgTable, 0x00, sizeof(s_AspE2trgTable));
	memset(s_AspE2actTable, 0x00, sizeof(s_AspE2actTable));
}

//=============================================================================
/**
* 無効化するE2拡張機能が使用されていないかを確認する
* @param pAspConfiguration 拡張機能のコンフィグ情報
* @retval RFWERR_OK
* @retval RFWERR_N_BASP_FUNC_USED
*/
//=============================================================================
DWORD ChkInvalidAspFunction(const RFW_ASPCONF_DATA *pAspConfiguration)
{
	DWORD dwRet = RFWERR_OK;

	/* 端子1-4 */
	/* CANモニタch0 */
	if ((pAspConfiguration->dwFunction0 & RF_ASPCONF_CAN_MON_CH0) == 0) {
		if ((s_AspConfData.dwFunction0 & RF_ASPCONF_CAN_MON_CH0) != 0) {
			if (GetCanMonEna(RF_ASPCANMON_CH0)) {
				return RFWERR_N_BASP_FUNC_USED;
			}
		}
	}
	/* 他の端子1-4機能は省略(未サポート)*/


	/* 端子5-8 */
	/* CANモニタch1 */
	if ((pAspConfiguration->dwFunction1 & RF_ASPCONF_CAN_MON_CH1) == 0) {
		if ((s_AspConfData.dwFunction1 & RF_ASPCONF_CAN_MON_CH1) != 0) {
			if (GetCanMonEna(RF_ASPCANMON_CH1)) {
				return RFWERR_N_BASP_FUNC_USED;
			}
		}
	}
	/* 他の端子5-8機能は省略(未サポート)*/

	/* 端子9-12 */
	/* 外部トリガ出力ch0 */
	if ((pAspConfiguration->dwFunction2 & RF_ASPCONF_TRIGGER_OUT_CH0) == 0) {
		if ((s_AspConfData.dwFunction2 & RF_ASPCONF_TRIGGER_OUT_CH0) != 0) {
			if (IsAspExTrgOutEna(RF_ASPTRGOUT_CH0)) {
				return RFWERR_N_BASP_FUNC_USED;
			}
		}
	}

	/* 外部トリガ出力ch1 */
	if ((pAspConfiguration->dwFunction2 & RF_ASPCONF_TRIGGER_OUT_CH1) == 0) {
		if ((s_AspConfData.dwFunction2 & RF_ASPCONF_TRIGGER_OUT_CH1) != 0) {
			if (IsAspExTrgOutEna(RF_ASPTRGOUT_CH1)) {
				return RFWERR_N_BASP_FUNC_USED;
			}
		}
	}

	/* 外部トリガ入力ch0 */
	if ((pAspConfiguration->dwFunction2 & RF_ASPCONF_TRIGGER_IN_CH0) == 0) {
		if ((s_AspConfData.dwFunction2 & RF_ASPCONF_TRIGGER_IN_CH0) != 0) {
			if (IsAspExTrgInEna(RF_ASPTRGIN_CH0)) {
				return RFWERR_N_BASP_FUNC_USED;
			}
		}
	}
	/* 外部トリガ入力ch1 */
	if ((pAspConfiguration->dwFunction2 & RF_ASPCONF_TRIGGER_IN_CH1) == 0) {
		if ((s_AspConfData.dwFunction2 & RF_ASPCONF_TRIGGER_IN_CH1) != 0) {
			if (IsAspExTrgInEna(RF_ASPTRGIN_CH1)) {
				return RFWERR_N_BASP_FUNC_USED;
			}
		}
	}
	/* 他の端子9-12機能は省略(未サポートor予約)*/

	/* ASP I / Fを使用しない機能 */
	/* ブレークイベント(E2トリガ) */
	if ((pAspConfiguration->dwFunction3 & RF_ASPCONF_BRK_EV) == 0) {
		if ((s_AspConfData.dwFunction3 & RF_ASPCONF_BRK_EV) != 0) {
			if (CheckSingleEvent(RFW_ASPFUNC_CLR, ((UINT64)1 << RF_E2ASP_TRG_CH_BRKEV), 0, 0) != RFWERR_OK) {
				return RFWERR_N_BASP_FUNC_USED;
			}
		}
	}


	/* モニタイベント */
	if ((pAspConfiguration->dwFunction3 & RF_ASPCONF_MON_EV) == 0) {
		if ((s_AspConfData.dwFunction3 & RF_ASPCONF_MON_EV) != 0) {
			if (IsFamilyAspFuncEna(RF_ASPFUNC_MONEV)) {
				return RFWERR_N_BASP_FUNC_USED;
			}

		}
	}

	/* パワーモニタ */
	if ((pAspConfiguration->dwFunction3 & RF_ASPCONF_POWER_MON) == 0) {
		if ((s_AspConfData.dwFunction3 & RF_ASPCONF_POWER_MON) != 0) {
			if (IsAspPwrMonEna()) {
				return RFWERR_N_BASP_FUNC_USED;
			}
		}
	}

	/* 外部ソフトトレース */
	if ((pAspConfiguration->dwFunction3 & RF_ASPCONF_EXT_STRACE) == 0) {
		if ((s_AspConfData.dwFunction3 & RF_ASPCONF_EXT_STRACE) != 0) {
			if (IsFamilyAspFuncEna(RF_ASPFUNC_STRACE)) {
				return RFWERR_N_BASP_FUNC_USED;
			}

		}
	}

	/* トレース停止 */
	if ((pAspConfiguration->dwFunction3 & RF_ASPCONF_STOPTRC_EV) == 0) {
		if ((s_AspConfData.dwFunction3 & RF_ASPCONF_STOPTRC_EV) != 0) {
			if (IsStopTraceEventActEna()) {
				return RFWERR_N_BASP_FUNC_USED;
			}

		}
	}
	/* 他のASP I/Fを使用しない機能は省略(未サポートor予約)*/

	return dwRet;
}


//=============================================================================
/**
* GetAspFuncの戻り値から未サポートの機能を有効設定していないか確認する
* @param pAspConfiguration 拡張機能のコンフィグ情報
* @retval RFWERR_OK
* @retval RFWERR_N_BASP_NOSUPPORT
*/
//=============================================================================
DWORD ChkSupportAspFunction(const RFW_ASPCONF_DATA *pAspConfiguration)
{
	DWORD dwRet = RFWERR_OK;
	DWORD dwAspCommonFunc = 0;
	DWORD dwAspSeriesFunc = 0;
	DWORD dwAspStorage = 0;

	dwAspCommonFunc = GetAspCommonFunc();
	dwAspSeriesFunc = GetAspSeriesFunc();
	dwAspStorage = GetAspStorage();

	// ストレージモード
	// E2ストレージ、リングモード
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwStorage == RF_ASPSTORAGE_E2) {
			if ((dwAspStorage & RF_ASPFUNC_E2_RING) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// E2ストレージ、フルストップモード
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwStorage == RF_ASPSTORAGE_E2_FULLSTOP) {
			if ((dwAspStorage & RF_ASPFUNC_E2_FULLSTOP) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// E2ストレージ、フルブレークモード
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwStorage == RF_ASPSTORAGE_E2_FULLBREAK) {
			if ((dwAspStorage & RF_ASPFUNC_E2_FULLBRK) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// PCストレージ、リングモード
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwStorage == RF_ASPSTORAGE_PC) {
			if ((dwAspStorage & RF_ASPFUNC_PC_RING) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 端子1-4
	// 通信モニタch0(CAN)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction0 & RF_ASPCONF_CAN_MON_CH0) {
			if ((dwAspCommonFunc & RF_ASPFUNC_MON_CAN) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 通信モニタch0(UART)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction0 & RF_ASPCONF_UART_MON_CH0) {
			if ((dwAspCommonFunc & RF_ASPFUNC_MON_UART) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 通信モニタch0(SPI)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction0 & RF_ASPCONF_SPI_MON_CH0) {
			if ((dwAspCommonFunc & RF_ASPFUNC_MON_SPI) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 通信モニタch0(I2C)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction0 & RF_ASPCONF_I2C_MON_CH0) {
			if ((dwAspCommonFunc & RF_ASPFUNC_MON_I2C) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 疑似ターゲット(UART)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction0 & RF_ASPCONF_PSEUDO_UART) {
			if ((dwAspCommonFunc & RF_ASPFUNC_PSEUDO_UART) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 疑似ターゲット(SPI)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction0 & RF_ASPCONF_PSEUDO_SPI) {
			if ((dwAspCommonFunc & RF_ASPFUNC_PSEUDO_SPI) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 疑似ターゲット(I2C)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction0 & RF_ASPCONF_PSEUDO_I2C) {
			if ((dwAspCommonFunc & RF_ASPFUNC_PSEUDO_I2C) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 端子5-8
	// 通信モニタch1(CAN)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction1 & RF_ASPCONF_CAN_MON_CH1) {
			if ((dwAspCommonFunc & RF_ASPFUNC_MON_CAN) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}
	// 通信モニタch1(UART)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction1 & RF_ASPCONF_UART_MON_CH1) {
			if ((dwAspCommonFunc & RF_ASPFUNC_MON_UART) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 通信モニタch1(SPI)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction1 & RF_ASPCONF_SPI_MON_CH1) {
			if ((dwAspCommonFunc & RF_ASPFUNC_MON_SPI) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 通信モニタch1(I2C)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction1 & RF_ASPCONF_I2C_MON_CH1) {
			if ((dwAspCommonFunc & RF_ASPFUNC_MON_I2C) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 端子9-12
	// 外部トリガ
	if (dwRet == RFWERR_OK) {
		if ((pAspConfiguration->dwFunction2 & RF_ASPCONF_TRIGGER_MASK) != 0) {
			if ((dwAspCommonFunc & RF_ASPFUNC_EXTRG) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// ロジアナch0, ch1
	if (dwRet == RFWERR_OK) {
		if ((pAspConfiguration->dwFunction2 & RF_ASPCONF_LOGICANALY_MASK) != 0) {
			if ((dwAspCommonFunc & RF_ASPFUNC_LOGIC) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// ASP I/Fを使用しない機能

	// ステータス(ブレークイベント E2トリガ条件)
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_BRK_EV) {
			if ((dwAspCommonFunc & RF_ASPFUNC_FACTBRK) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// モニタイベント[RL]
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_MON_EV) {
			if ((dwAspSeriesFunc & RF_ASPFUNC_RL_MONEV) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// LOGGER
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_LOGGER) {
			if ((dwAspCommonFunc & RF_ASPFUNC_RAMMON) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// PCサンプリング
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_PC_SAMPLING) {
			if ((dwAspCommonFunc & RF_ASPFUNC_PCSAMP) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// パワーモニタ
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_POWER_MON) {
			if ((dwAspCommonFunc & RF_ASPFUNC_PWRMON) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// RESETZ
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_RESETZ) {
			if ((dwAspCommonFunc & RF_ASPFUNC_RESETZ) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// EVTOZ
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_EVTOZ) {
			if ((dwAspSeriesFunc & RF_ASPFUNC_RH_EVTOZ) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// 外部ソフトトレース
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_EXT_STRACE) {
			if ((dwAspSeriesFunc & RF_ASPFUNC_RH_SWTRC) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	// トレース停止
	if (dwRet == RFWERR_OK) {
		if (pAspConfiguration->dwFunction3 & RF_ASPCONF_STOPTRC_EV) {
			if ((dwAspCommonFunc & RF_ASPFUNC_STOPTRC) == 0) {
				dwRet = RFWERR_N_BASP_NOSUPPORT;
			}
		}
	}

	return dwRet;
}
