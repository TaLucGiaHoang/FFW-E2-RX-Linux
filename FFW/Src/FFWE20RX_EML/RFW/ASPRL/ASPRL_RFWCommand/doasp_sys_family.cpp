///////////////////////////////////////////////////////////////////////////////
/**
* @file doasp_sys_family.cpp
* @brief E2拡張機能システムコマンド(MCUファミリ固有部)の実装ファイル
* @author RSD M.Yamamoto
* @author Copyright (C) 2016 Renesas Electronics Corporation and
* @author Renesas System Design Co., Ltd. All rights reserved.
* @date 2016/12/09
*/
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/12/09 新規作成 M.Yamamoto
*/
#include <math.h>
#include "doasp_sys.h"
#include "rfwasp_sys.h"
#include "emudef.h"
#include "rfwasp_closed.h"
#include "doasp_per.h"
#include "doasp_can.h"
#include "doasp_trg.h"
#include "doasp_sys_family.h"
#include "getstat.h" 
#include "extpower.h" 
#include "doasp_sample.h" 
#include "doasp_pwr.h"

extern "C" {
	BOOL	GetSaveSramRawDataFlag(void);
	BOOL	GetPwrMonCorrectValFlag(void);
	BOOL	GetPwrMonCorrectThreasholdFlag(void);
	extern  BOOL	WINAPI  get_LowVoltageAdapterMode(void);
}

// ファイル内static変数の宣言



// ファイル内static関数の宣言


///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// グローバル関数
///////////////////////////////////////////////////////////////////////////////


//=============================================================================
/**
* RUN状態を取得する関数
* @note ファミリ固有処理
* @param なし
* @retval TRUE ：RUN状態
* @retval FALSE：BRK状態
*/
//=============================================================================
BOOL IsStatRun(void)
{
	ULONG	dbg_stat;

	dbg_stat = getexestatruntrc(DBG_STAT_NOREAD1);

	if ((dbg_stat & ES_USRPRO_RUN) != 0) {
		return TRUE;
	}
	return FALSE;
}



///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
* エミュレータがE2か取得する関数(RH専用)
* @param なし
* @retval TRUE ：E2
* @retval FALSE：E2以外
*/
//=============================================================================
BOOL IsEmlE2(void)
{
	if (ChkDllTypeEmu(E2)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//=============================================================================
/**
* 共通部で使用する関数(RLでは処理不要)
* @param なし
* @retval なし
*/
//=============================================================================
void ProtInit_ASP(void)
{
}


//=============================================================================
/**
* 共通部で使用する関数(RLでは処理不要)
* @param なし
* @retval なし
*/
//=============================================================================
DWORD ProtEnd_ASP(void)
{
	return RFWERR_OK;	// ProtEnd()はEXEC本体でも正常終了を返すのみ
}



//=============================================================================
/**
* RL固有機能の取得情報を返す関数
* @param なし
* @retval モニタイベントのみ
*/
//=============================================================================
BOOL IsSupplyPower(void)
{

	ULONG	ulVout = 0;
	USHORT  ushMode = 0;


	/* 低電圧アダプタ使用時は供給なし */
	if (get_LowVoltageAdapterMode()) {
		return FALSE;
	}

	/* ex_extpower()で電源供給設定した場合 */
	if ((GetExtPwrFunc()) == EXTPWR_FUNC_EXTPWR) {
		ushMode = GetExtPwrMode();
		if (ushMode >= EX_EXTPWR_USERIF) {
			return TRUE;
		}
	}
	/* ex_initexec()で電源供給設定した場合 */
	else {
		/* E2のVoutで判断する */
		e2_GetVout(&ulVout);
		/* E2からの電源供給時、最低3.3V供給するが、マージン含め2.7Vで判断する */
		if (ulVout >= 0x010E) {
			return TRUE;
		}
	}
	return FALSE;
}


//=============================================================================
/**
* ASP I/Fへの電源供給状態の取得
* @note ファミリ固有
* @param なし
* @retval TRUE ：供給中
* @retval FALSE：未供給
*/
//=============================================================================
BOOL IsSupplyPowerASP(void)
{
	BOOL bRet = FALSE;
	
	switch (GetExtPwrMode()) {
	case EX_EXTPWR_TARGET_ASPIF:
	case EX_EXTPWR_HOTPLUG_ASPIF:
	case EX_EXTPWR_ASPIF:
	case EX_EXTPWR_USERIF_ASPIF:
		bRet = TRUE;
		break;
	default:
		bRet = FALSE;
		break;
	}
	return bRet;

}


//=============================================================================
/**
* 非同期モードかを返す関数
* @note RH専用の関数
* @note RH以外ではFALSEを返す
* @param なし
* @retval TRUE ：非同期モード
* @retval FALSE：同期モード
*/
//=============================================================================
BOOL IsAsyncMode(void)
{
	return FALSE;
}


//=============================================================================
/**
* E2拡張機能(共通)でサポートする機能の情報を返す関数
* @param なし
* @retval モニタイベントのみ
*/
//=============================================================================
DWORD GetAspCommonFunc(void)
{
	DWORD dwFunc = 0;

	// 通信モニタ、記録停止、RESETZ、PCサンプリング、LOGGER、疑似ターゲット、RUN検出/要求はRL78では未サポート

	// パワーモニタ
	dwFunc |= RF_ASPFUNC_PWRMON;

	// パフォーマンス
	dwFunc |= RF_ASPFUNC_PERFORMANCE;

	// 外部トリガ
	dwFunc |= RF_ASPFUNC_EXTRG;

	// ブレーク検出
	dwFunc |= RF_ASPFUNC_FACTBRK;

	// ブレーク要求
	dwFunc |= RF_ASPFUNC_ACTBRK;

	return dwFunc;
}

//=============================================================================
/**
* RL固有機能の取得情報を返す関数
* @param なし
* @retval モニタイベントのみ
*/
//=============================================================================
DWORD GetAspSeriesFunc(void)
{
	return RF_ASPFUNC_RL_MONEV;
}


//=============================================================================
/**
* E2拡張機能(共通)でサポートする機能の情報を返す関数
* @param なし
* @retval
*/
//=============================================================================
DWORD GetAspStorage(void)
{
	DWORD dwStorage = 0;

	// E2ストレージ、リングモード
	dwStorage = RF_ASPFUNC_E2_RING;

	// E2ストレージ、フルストップモード
	dwStorage |= RF_ASPFUNC_E2_FULLSTOP;

	// E2ストレージ、フルブレークモード
	dwStorage |= RF_ASPFUNC_E2_FULLBRK;

	return dwStorage;
}

// RevRL78No170420-001-01 Top
//=============================================================================
/**
* aspRun=1にしてから、実際にサンプリングが動き出すまでdelay値の参照
* @note delay値の対象は固定時間のみとし変動時間については対象外とする
* @param なし
* @retval delay値
*/
//=============================================================================
DWORD GetAspDelay_family(void)
{
	RFW_ASPE2TRG_INFO AspE2TrgInfo;
	RFW_PWRMONSAMP_DATA PowerMonitor;

	// パワーモニタの動作モード取得
	GetAspPwrMon(&PowerMonitor);

	// モニタイベントは通信時間分(10usec固定)で減算補正する
	DO_SetClkCalibration(RF_ASPTYPE_MON_EV, -1200);

	// パワーモニタは通常計測モードの場合は補正しない
	DO_SetClkCalibration(RF_ASPTYPE_POWER, 0);

	// パワーモニタはイベントリンクモードでモニタイベントによる開始/終了を行う場合は
	// モニタイベントと同様、通信時間分(10usec固定)で減算補正する
	if (PowerMonitor.bySamplingMode == RF_ASPPWR_EVLINK) {
		GetE2TrgTable(MONEV_TRG_BASE_CH0 + 6, &AspE2TrgInfo);
		if (AspE2TrgInfo.dwFactSw == RFW_EVENTLINK_SWON) {
			if (AspE2TrgInfo.u64LinkAct & ((UINT64)1 << RF_ASPE2ACTCH_PWRMON)) {
				DO_SetClkCalibration(RF_ASPTYPE_POWER, -1200);
			}
		}
	}

	// Delay値はFPGA(120MHz)カウント単位で設定(記録開始のディレイはなし)
	// GOコマンド分の通信時間のディレイは入れないものとする。
	return (DWORD)0;
}
// RevRL78No170420-001-01 Bottom


//=============================================================================
/**
* 時間計測結果を補正する関数
* @note 計測条件によって補正有無や補正値が変わる。
* @param AspPerformanceDataRaw 計測結果の生データ
* @param pAspPerformanceData   補正した計測結果を返す
* @param u64StartEvent 計測開始イベント
* @param u64EndEvent   計測終了イベント
* @param u64LevelEvent レベル型イベント
* @retval なし
*/
//=============================================================================
void AspPerDataCalibration(RFW_ASPPER_DATA AspPerformanceDataRaw, RFW_ASPPER_DATA* pAspPerformanceData, UINT64 u64StartEvent, UINT64 u64EndEvent, UINT64 u64LevelEvent)
{

	/* RL78では時間計測機能の補正は未対応 */
	AspPerformanceDataRaw;
	pAspPerformanceData;
	u64StartEvent;
	u64EndEvent;
	u64LevelEvent;

	// 補正なしの状態で初期化
	memcpy((void*)pAspPerformanceData, &AspPerformanceDataRaw, sizeof(AspPerformanceDataRaw));
}



//=============================================================================
/**
* 時間計測の閾値を補正する関数
* @note 計測条件によって補正有無や補正値が変わる。
* @param u64ThresholdRaw 閾値の生データ
* @param pu64Threshold   補正した閾値を返す
* @param u64StartEvent 計測開始イベント
* @param u64EndEvent   計測終了イベント
* @param u64LevelEvent レベル型イベント
* @retval なし
*/
//=============================================================================
void AspPerThresholdCalibration(UINT64 u64ThresholdRaw, UINT64* pu64Threshold, UINT64 u64StartEvent, UINT64 u64EndEvent, UINT64 u64LevelEvent)
{

	/* RL78では時間計測機能の補正は未対応 */
	u64ThresholdRaw;
	pu64Threshold;
	u64StartEvent;
	u64EndEvent;
	u64LevelEvent;

	// 補正なしの状態で初期化
	*pu64Threshold = u64ThresholdRaw;
}



//=============================================================================
/**
* RLのサポートしているE2トリガ情報を返す
* @param なし
* @retval RF_ASPFACT_SUPORT_RL
*/
//=============================================================================
UINT64 GetAspSeriesFact(void)
{
	return RF_ASPFACT_SUPORT_RL;
}


int SendBlock_ASP(const BYTE* pData, DWORD size)
{
	// タイムアウト時間は使用しないので0にしておく
	return usb_ServerSend((UCHAR *)pData, size, 0);
}


//=============================================================================
/**
* RL固有の拡張機能の初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitAsp_family(void)
{
	/* モニタイベントの初期化 */
	if (IsMonitorEventEna()) {
		ClrAllMonData();
	}
	InitAspMonEv();
	InitSramFullStopFlg();
}


//=============================================================================
/**
* SRAM生データをデバッグ用に保存するかどうかを返す
* note iniファイル等に保存する/しないの情報を持ち、起動時にそれをフラグに保存しておく
* @param なし
* @retval TRUE:保存する、　FALSE:保存しない
*/
//=============================================================================
BOOL IsSaveSramRawData(void)
{
	return GetSaveSramRawDataFlag();
}

//=============================================================================
/**
* E2から供給している電源電圧値を返す。
* @param なし
* @retval 電源電圧値(0.1V単位)
*/
//=============================================================================
BYTE GetE2SupplyPwrVal(void)
{
	return GetSupplyPwrVal();
}

//=============================================================================
/**
* ファミリ固有機能の有効状態を確認
* @param byFunc
* @retval TRUE:指定した機能は有効(使用中)、FALSE:指定した機能は無効(未使用)
*/
//=============================================================================
BOOL IsFamilyAspFuncEna(BYTE byFunc)
{
	BOOL bRet = FALSE;

	switch (byFunc) {
	case RF_ASPFUNC_MONEV:
		bRet = IsMonitorEventEna();
		break;
	default:
		bRet = FALSE;
		break;
	}
	return bRet;
}

//=============================================================================
/**
* パワーモニタの測定結果補正の有効/無効チェック
* @param byFunc
* @retval TRUE:パワーモニタの測定結果を補正する、FALSE:補正しない
*/
//=============================================================================
BOOL IsPwrMonCorrectValEna(void)
{
	return GetPwrMonCorrectValFlag();
}

//=============================================================================
/**
* パワーモニタの検出条件(閾値)への補正有効/無効チェック
* @param byFunc
* @retval TRUE:パワーモニタの検出条件(閾値)を補正する、FALSE:補正しない
*/
//=============================================================================
BOOL IsPwrMonCorrectThreasholdEna(void)
{
	return GetPwrMonCorrectThreasholdFlag();
}


//=============================================================================
/**
* RFWAsp_GetAvailableTimeのエラーチェック処理
* @param dwType 記録データ種別
* @retval RFWエラー(RFWERR_OK)
*/
//=============================================================================
RFWERR checkGetAvailableTime(DWORD dwType)
{
	dwType;	// Warning除去

	// RL78はチェックしない
	return RFWERR_OK;
}
//=============================================================================
/**
* RFWAsp_GetSampleのエラーチェック処理
* @param dwType 記録データ種別
* @param dwKeyType 0:時間指定 1:変化点数指定
* @param u64Start 開始時間/開始変化点
* @param u64End 終了時間/終了変化点
* @retval RFWエラー(RFWERR_OK)
*/
//=============================================================================
RFWERR checkGetSample(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End)
{
	dwType;		// Warning除去
	dwKeyType;	// Warning除去
	u64Start;	// Warning除去
	u64End;		// Warning除去

	// RL78はチェックしない
	return RFWERR_OK;
}

//=============================================================================
/**
* RFWAsp_SampleDataSizeのエラーチェック処理
* @param dwType 記録データ種別
* @param dwKeyType 0:時間指定 1:変化点数指定
* @param u64Start 開始時間/開始変化点
* @param u64End 終了時間/終了変化点
* @retval RFWエラー(RFWERR_OK)
*/
//=============================================================================
RFWERR checkGetSampleDataSize(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End)
{
	dwType;		// Warning除去
	dwKeyType;	// Warning除去
	u64Start;	// Warning除去
	u64End;		// Warning除去

	// RL78はチェックしない
	return RFWERR_OK;
}



//=============================================================================
/**
* RFWAsp_GetSampleRealTimeのエラーチェック処理
* @param dwType 記録データ種別
* @param dwKeyType 0:時間指定 1:変化点数指定
* @param u64Back 現在の時間から遡る時間/変化点
* @param pData 格納領域
* @param dwSize 格納領域のサイズ
* @retval RFWエラー(RFWERR_OK)
*/
//=============================================================================
RFWERR checkGetSampleRealTime(DWORD dwType, DWORD dwKeyType, UINT64 u64Back)
// ソフトウェア・トレース(LPD)は記録できない状態を継続中かをチェック
{
	dwType;		// Warning除去
	dwKeyType;	// Warning除去
	u64Back;	// Warning除去

	// RL78はチェックしない
	return RFWERR_OK;
}

//=============================================================================
/**
* RFWAsp_SetClkCalibrationのエラーチェック
* @param dwType 記録種別
* @param i64Calib 補正値
* @retval RFWエラー(RFWERR_OK)
*/
//=============================================================================
RFWERR checkSetClkCalibration(DWORD dwType, INT64 i64Calib)
{
	dwType;		// Warning除去
	i64Calib;	// Warning除去

	// RL78はチェックしない
	return RFWERR_OK;
}

