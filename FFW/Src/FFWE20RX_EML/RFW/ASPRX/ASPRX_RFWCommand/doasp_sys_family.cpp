///////////////////////////////////////////////////////////////////////////////
/**
* @file doasp_sys_family.cpp
* @brief E2拡張機能システムコマンド(MCUファミリ固有部)の実装ファイル
 * @author REL M.Yamamoto, PA M.Tsuji
 * @author Copyright (C) 2017(2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/09/05
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/12/09 新規作成 M.Yamamoto
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
・RevRxE2No180720-001,PB18047-CD05-001 2018/07/20 PA 辻
	E2拡張機能の制限解除対応
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
#include "doasp_sample.h" 
#include "doasp_pwr.h"
#include "doasprx_monev.h"		// RevRxE2No171004-001 Append Line

#include "do_sys.h"
#include "domcu_prog.h"
#include "ffwe2_sys.h"
#include "doe2_sys.h"

#include "ffwmcu_mcu.h"
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
	return GetMcuRunState();
}



///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
* エミュレータがE2か取得する関数
* @param なし
* @retval TRUE ：E2
* @retval FALSE：E2以外
*/
//=============================================================================
BOOL IsEmlE2(void)
{
	FFWE20_EINF_DATA	einfData;

	getEinfData(&einfData);

	if (einfData.wEmuStatus == EML_E2) {
		return TRUE;
	}
	else {
		return FALSE;
	}
	
}

//=============================================================================
/**
* 共通部で使用する関数(RXでも暫定で処理不要とする)
* @param なし
* @retval なし
*/
//=============================================================================
void ProtInit_ASP(void)
{
}


//=============================================================================
/**
* 共通部で使用する関数(RXでも暫定で処理不要とする)
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
* E2からの電源供給状態の取得
* @param なし
* @retval FALSE：外部電源から電源供給
* @retval TRUE ：E2、または、ASP I/Fから電源供給
*/
//=============================================================================
BOOL IsSupplyPower(void)
{
	// RevRxE2No180720-001 Modify Start
	enum FFWENM_EXPWR2_MODE eNowExPwr2Mode;

	eNowExPwr2Mode = GetExPwr2Mode();

	if (eNowExPwr2Mode == EXPWR2_USER || eNowExPwr2Mode == EXPWR2_ASP || eNowExPwr2Mode == EXPWR2_USER_ASP) {
		return TRUE;
	} else {
		return FALSE;
	}
	// RevRxE2No180720-001 Modify End
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
	enum FFWENM_EXPWR2_MODE eNowExPwr2Mode;
	eNowExPwr2Mode = GetExPwr2Mode();
	if (eNowExPwr2Mode == EXPWR2_ASP || eNowExPwr2Mode == EXPWR2_TARGET_ASPIN || eNowExPwr2Mode == EXPWR2_USER_ASP) {
		return TRUE;
	}
	else {
		return FALSE;
	}
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

	// 通信モニタ、記録停止、RESETZ、PCサンプリング、LOGGER、疑似ターゲット、RUN検出/要求はRL78/RXでは未サポート

	// パワーモニタ
	dwFunc |= RF_ASPFUNC_PWRMON;

	// パフォーマンス
	// dwFunc |= RF_ASPFUNC_PERFORMANCE;	// RevRxE2No171004-001 Delete Line

	// 外部トリガ
	dwFunc |= RF_ASPFUNC_EXTRG;

	// ブレーク検出
	dwFunc |= RF_ASPFUNC_FACTBRK;

	// ブレーク要求
	dwFunc |= RF_ASPFUNC_ACTBRK;

	// トレース停止
	// dwFunc |= RF_ASPFUNC_STOPTRC;		// RevRxE2No171004-001 Delete Line

	return dwFunc;
}

//=============================================================================
/**
* RX固有機能の取得情報を返す関数
* @param なし
* @retval 0(なし)
*/
//=============================================================================
DWORD GetAspSeriesFunc(void)
{
	DWORD dwFunc = 0;

	dwFunc |= RF_ASPFUNC_RL_MONEV;	// RXは通過ポイントをサポートするため、RL78用のRF_ASPFUNC_RL_MONEVのdefine定義を使用する

	return dwFunc;
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
* RXのサポートしているE2トリガ情報を返す(試作段階ではRLと一緒にする)
* @param なし
* @retval RF_ASPFACT_SUPORT_RX
*/
//=============================================================================
UINT64 GetAspSeriesFact(void)
{
	return RF_ASPFACT_SUPORT_RX;
}


int SendBlock_ASP(const BYTE* pData, DWORD size)
{
	pData;		//　Warning回避
	size;		//　Warning回避
	// USB送信機能は拡張機能では未使用のため、実質空関数とする
//	return usb_ServerSend((UCHAR *)pData, size, 0);
	return 0;
}


//=============================================================================
/**
* RX固有の拡張機能の初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitAsp_family(void)
{
	// RevRxE2No171004-001 Append Start
	/* モニタイベントの初期化 */
	if (IsMonitorEventEna()) {
		ClrAllMonData();
	}
	InitAspMonEv();
	InitSramFullStopFlg();
	// RevRxE2No171004-001 Append End
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
	return FALSE;		// RevRxE2No171004-001 Modify Line
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
	// RevRxE2No171004-001 Modify Start
	BYTE	byPwrVal;

	byPwrVal = GetExPwr2Val();

	return byPwrVal;
	// RevRxE2No171004-001 Modify End
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
	// RevRxE2No171004-001 Modify Start
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
	// RevRxE2No171004-001 Modify End
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
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報

	pMcuInfoData = GetMcuInfoDataRX();

	// FINEの場合は、電流値補正を行う
	if( pMcuInfoData->byInterfaceType == IF_TYPE_1 ){
		return TRUE;		// RevRxE2No171004-001 Modify Line
	} else { 
		return FALSE;
	}
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
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報

	pMcuInfoData = GetMcuInfoDataRX();

	// FINEの場合は、電流値補正を行う
	if( pMcuInfoData->byInterfaceType == IF_TYPE_1 ){
		return TRUE;		// RevRxE2No171004-001 Modify Line
	} else { 
		return FALSE;
	}
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

	// RL78, RXはチェックしない
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

	// RL78, RXはチェックしない
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

	// RL78, RXはチェックしない
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

	// RL78, RXはチェックしない
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

	// RL78, RXはチェックしない
	return RFWERR_OK;
}

