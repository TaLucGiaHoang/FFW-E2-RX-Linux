///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_pwr.cpp
 * @brief パワーモニタコマンドの実装ファイル
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
#include <math.h>
#include "rfw_bitops.h"
#include "doasp_pwr.h"
#include "emudef.h"
#include "doasp_sys.h"
#include "asp_setting.h"
#include "doasp_sys_family.h"
#include "e2_asp_fpga.h"

// ファイル内static変数の宣言
static BOOL		s_bPwrMonEna;
static RFW_PWRMONSAMP_DATA s_PwrMon;
static double	s_dblCorrectValueCh1 = 0;
static double	s_dblCorrectValueCh4 = 0;
static BOOL		s_bPwrMonEvSw;
static RFW_PWRMONEV_DATA s_PwrMonEvData;
static DWORD	s_dwTrgCh;

// 補正用の変数
#if ASP_RL
static WORD		s_wE2BaseCurrentVal[PWRMON_BASECURRENT_NUM_MAX] = { 0x001F, 0x0030, 0x005C, 0x085, 0x096 };	// 1st対応のRL78用暫定値を使用
#elif ASP_RX
static WORD		s_wE2BaseCurrentVal[PWRMON_BASECURRENT_NUM_MAX] = { 0x000D, 0x001A, 0x0036, 0x04F, 0x059 };	// 1st対応のRX用暫定値を使用
#else
static WORD		s_wE2BaseCurrentVal[PWRMON_BASECURRENT_NUM_MAX] = { 0x001F, 0x0031, 0x005C, 0x085, 0x096 };	// RH850が対応する場合は、#elifを追加する。このデフォルトはLPD(1pin)の場合。
#endif
static double	s_dblE2BaseCurrentResult = 0;
static double	s_dblCabSlope = 0;		// キャリブレーション補正の傾き

static double	s_dblPwrMonIncCurrentSlope = 0;


// ファイル内static関数の宣言
void GetPwrMonCalibrationVal(void);
static DWORD ClrPwrMonEvMain(void);

///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * パワーモニタのサンプリング設定
 * @param pPowerMonitor パワーモニタサンプリング情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PWRMONOFF　パワーモニタが無効状態
 * @retval RFWERR_N_BASP_FACT_NONSEL　E2トリガが指定されていない
 * @retval RFWERR_N_BASP_SGLLNK_ILLEGAL　シングルE2トリガに複数個の要因は設定できない
 * @retval RFWERR_N_BASP_FACT_DISABLE　指定のE2トリガは無効状態
 * @retval RFWERR_N_BASP_FACT_NOSUPPORT　指定のE2トリガは未サポート
 */
//=============================================================================
DWORD DO_SetPwrMon(const RFW_PWRMONSAMP_DATA* pPowerMonitor)
{
	DWORD	dwRet = RFWERR_OK;
	RFW_ASPCONF_DATA	AspConfData;

	ProtInit_ASP();

	// コンフィグ情報の取得
	GetAspConfData(&AspConfData);

	// コンフィグでパワーモニタを有効化しているかチェック
	if (dwRet == RFWERR_OK) {
		if ((AspConfData.dwFunction3 & RF_ASPFUNC_PWRMON) == 0) {
			dwRet = RFWERR_N_BASP_PWRMONOFF;
		}
	}

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// イベントリンクモードの要因イベントチェック
	if (dwRet == RFWERR_OK) {
		if (pPowerMonitor->bySamplingMode == RF_ASPPWR_EVLINK) {
			dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, pPowerMonitor->u64FactorSingleEvent, RFW_EVENTLINK_LEVEL, RF_ASPE2ACTCH_PWRMON);
			if (dwRet == RFWERR_OK) {
				// E2アクションのテーブル設定
				SaveSingleEvent(find_first_bit(pPowerMonitor->u64FactorSingleEvent), (UINT64)((UINT64)1 << RF_ASPE2ACTCH_PWRMON));
				SetE2ActTable((DWORD)RF_ASPE2ACTCH_PWRMON, RFW_EVENTLINK_LEVEL, pPowerMonitor->u64FactorSingleEvent);
			}
		} else { // RF_ASPPWR_NORMAL
			ClrE2ActTable(RF_ASPE2ACTCH_PWRMON);
			ClearSingleEvent((UINT64)((UINT64)1 << RF_ASPE2ACTCH_PWRMON));
		}
	}

	// パワーモニタ設定状態の保存
	if (dwRet == RFWERR_OK) {
		s_bPwrMonEna = TRUE;
		memcpy(&s_PwrMon, pPowerMonitor, sizeof(*pPowerMonitor));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
 * パワーモニタのサンプリング設定参照
 * @param pPowerMonitor パワーモニタサンプリング情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BASP_PWRMON_NOUSE パワーモニタは使用されていません
 */
//=============================================================================
DWORD DO_GetPwrMon(RFW_PWRMONSAMP_DATA *const pPowerMonitor)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// パワーモニタが設定されているかチェック
	if (dwRet == RFWERR_OK) {
		if (s_bPwrMonEna == FALSE) {
			dwRet = RFWERR_N_BASP_PWRMON_NOUSE;
		}
	}

	// パワーモニタ設定状態の保存
	if (dwRet == RFWERR_OK) {
		memcpy((void*)pPowerMonitor, &s_PwrMon, sizeof(s_PwrMon));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
 * パワーモニタのサンプリング設定消去
 * @param なし
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PWRMON_NOUSE パワーモニタは使用されていません
 * @retval RFWERR_N_BASP_FACT_CANTCLR パワーモニタイベントが有効なためクリアできない
 */
//=============================================================================
DWORD DO_ClrPwrMon(void)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// パワーモニタが設定されているかチェック
	if (dwRet == RFWERR_OK) {
		if (s_bPwrMonEna == FALSE) {
			dwRet = RFWERR_N_BASP_PWRMON_NOUSE;
		}
	}

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 指定したチャネルがパワーモニタイベントとして使用されていないかチェック
	// エラーは要検討。今はCANモニタと処理を合わせている。
	if (dwRet == RFWERR_OK) {
		if (s_bPwrMonEvSw) {
			dwRet = RFWERR_N_BASP_FACT_CANTCLR;
		}
	}

	// パワーモニタ設定の消去
	if (dwRet == RFWERR_OK) {
		ClearSingleEvent((UINT64)((UINT64)1 << RF_ASPE2ACTCH_PWRMON));
		ClrE2ActTable(RF_ASPE2ACTCH_PWRMON);
		InitAspPwrMon();
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * パワーモニタイベントの設定
 * @param pPowerMonitor パワーモニタイベント情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PWRMON_NOUSE パワーモニタは使用されていません
 * @retval RFWERR_N_BASP_LFACT_CANTCHG E2トリガがレベル型のとき、LEVEL→PULSEに変更しようとした
 * @retval RFWERR_N_BASP_PFACT_CANTCHG E2トリガがパルス型のとき、PULSE→LEVELに変更しようとした
 */
//=============================================================================
DWORD DO_SetPwrMonEvent(const RFW_PWRMONEV_DATA* pPowerMonitorEvent)
{
	DWORD	dwRet = RFWERR_OK;
	UINT64	u64PwrMonBit;
	int		i;
	RFW_ASPE2ACT_INFO	AspE2ActInfo;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// パワーモニタ機能が使用されているかチェック
	if (dwRet == RFWERR_OK) {
		if (s_bPwrMonEna == FALSE) {
			dwRet = RFWERR_N_BASP_PWRMON_NOUSE;
		}
	}

	// 別I/FでパワーモニタがE2トリガとして設定されているときに、E2アクションのタイプと
	// トリガ検出レベルの種類が正しいかチェック
	if (dwRet == RFWERR_OK) {
		if ((pPowerMonitorEvent->byDetectType == RF_ASPPWREV_RANGEIN_L) ||
		    (pPowerMonitorEvent->byDetectType == RF_ASPPWREV_RANGEOUT_L)) {
			u64PwrMonBit = E2TRG_BIT_PWRMON_PULSE; //0x0800
		} else {
			u64PwrMonBit = E2TRG_BIT_PWRMON_LEVEL; //0x0400
		}
		for (i = 0; i < RFW_EVENTLINK_CH_MAX; i++) {
			GetE2ActTable((DWORD)i, &AspE2ActInfo);
			if ((AspE2ActInfo.u64LinkFact & u64PwrMonBit) == u64PwrMonBit) {
				// E2アクションのタイプチェック
				if (AspE2ActInfo.dwActType == RFW_EVENTLINK_PULSE) {
					// パルス型の場合
					if (pPowerMonitorEvent->byDetectType <= RF_ASPPWREV_RANGEOUT_L) {
						dwRet = RFWERR_N_BASP_PFACT_CANTCHG;
						break;
					}
				} else if (AspE2ActInfo.dwActType == RFW_EVENTLINK_LEVEL) {
					// レベル型の場合
					if (pPowerMonitorEvent->byDetectType >= RF_ASPPWREV_RANGEIN_P &&
						pPowerMonitorEvent->byDetectType <= RF_ASPPWREV_PULSE_GTE) {
						dwRet = RFWERR_N_BASP_LFACT_CANTCHG;
						break;
					}
				} else {
					// どちらでも可なアクションの場合、そのサポート状況を確認する

					// レベル型
					if (pPowerMonitorEvent->byDetectType <= RF_ASPPWREV_RANGEOUT_L) {
						dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, (UINT64)0x0A, RFW_EVENTLINK_LEVEL, i);
					}
					else {
						// パルス型は通らないが、念のためチェックする
						dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, (UINT64)0x0B, RFW_EVENTLINK_PULSE, i);
					}
					if (dwRet != RFWERR_OK) {
						break;
					}
				}
			}
		}
	}

	// パワーモニタ設定状態の保存
	if (dwRet == RFWERR_OK) {
		s_bPwrMonEvSw = TRUE;
		memcpy(&s_PwrMonEvData, pPowerMonitorEvent, sizeof(*pPowerMonitorEvent));

		if ((pPowerMonitorEvent->byDetectType == RF_ASPPWREV_RANGEIN_L) ||
		    (pPowerMonitorEvent->byDetectType == RF_ASPPWREV_RANGEOUT_L)) {
			s_dwTrgCh = PWRMONEV_TRG_LEVEL;
			SetE2TrgTable(s_dwTrgCh, RFW_EVENTLINK_LEVEL);
			ClrE2TrgTable(PWRMONEV_TRG_PULSE); //パルス型の要因を削除
		} else {
			s_dwTrgCh = PWRMONEV_TRG_PULSE;
			SetE2TrgTable(s_dwTrgCh, RFW_EVENTLINK_PULSE);
			ClrE2TrgTable(PWRMONEV_TRG_LEVEL); //レベル型の要因を削除
		}
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
 * パワーモニタイベントの設定参照
 * @param pPowerMonitorEvent パワーモニタイベント情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BASP_PWRMON_NOUSE パワーモニタは使用されていません
 */
//=============================================================================
DWORD DO_GetPwrMonEvent(RFW_PWRMONEV_DATA *const pPowerMonitorEvent)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// パワーモニタイベントが設定されているかチェック
	if (dwRet == RFWERR_OK) {
		if (s_bPwrMonEvSw == FALSE) {
			dwRet = RFWERR_N_BASP_PWRMON_NOUSE;
		}
	}

	// パワーモニタイベント設定状態の保存
	if (dwRet == RFWERR_OK) {
		memcpy((void*)pPowerMonitorEvent, &s_PwrMonEvData, sizeof(s_PwrMonEvData));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
 * パワーモニタイベントの設定消去
 * @param なし
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PWRMON_NOUSE パワーモニタは使用されていません
 */
//=============================================================================
DWORD DO_ClrPwrMonEvent(void)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// パワーモニタイベントが設定されているかチェック
	if (dwRet == RFWERR_OK) {
		if (s_bPwrMonEvSw == FALSE) {
			dwRet = RFWERR_N_BASP_PWRMON_NOUSE;
		}
	}

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 削除のメイン処理
	if (dwRet == RFWERR_OK) {
		dwRet = ClrPwrMonEvMain();
	}

	ProtEnd_ASP();

	return dwRet;
}



///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * パワーモニタ補正値の参照
 * @note パワーモニタ機能有効状態でのユーザプログラム実行開始のたびに行う
 * @param なし
 * @retval なし
 */
//=============================================================================
void GetPwrMonCalibrationVal(void)
{
	// 補正用の値を算出
	// 強制AD変換を行う。
	// CH1とCH4で2回ずつ行い、2回目のデータ4つの平均を算出する。
	
	ForcePwrMon(PWRMCTRL_SAMP_CH1);	// CH1(1回目)
	ForcePwrMon(PWRMCTRL_SAMP_CH1);	// CH1(2回目)
	s_dblCorrectValueCh1 = 0;
	s_dblCorrectValueCh1 += GetForcePwrMonValue(1);
	s_dblCorrectValueCh1 += GetForcePwrMonValue(2);
	s_dblCorrectValueCh1 += GetForcePwrMonValue(3);
	s_dblCorrectValueCh1 += GetForcePwrMonValue(4);
	s_dblCorrectValueCh1 /= 4;

	ForcePwrMon(PWRMCTRL_SAMP_CH4);	// CH4(1回目)
	ForcePwrMon(PWRMCTRL_SAMP_CH4);	// CH4(2回目)
	s_dblCorrectValueCh4 = 0;
	s_dblCorrectValueCh4 += GetForcePwrMonValue(1);
	s_dblCorrectValueCh4 += GetForcePwrMonValue(2);
	s_dblCorrectValueCh4 += GetForcePwrMonValue(3);
	s_dblCorrectValueCh4 += GetForcePwrMonValue(4);
	s_dblCorrectValueCh4 /= 4;
	
	// 本番用のCH2でダミー変換を行う(実施しないと、最初のAD0の値が不正になる)
	ForcePwrMon(PWRMCTRL_SAMP_CH2);	// CH2
	GetForcePwrMonValue(2);	// ダミーで読み出し
}

///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
* パワーモニタの初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitAspPwrMon(void)
{
	s_bPwrMonEna = FALSE;
	s_PwrMon.bySamplingMode = 0;
	s_PwrMon.bySamplingRate = 0;
	s_PwrMon.dwFactorMultiEvent = 0;
	s_PwrMon.u64FactorSingleEvent = 0;
	s_PwrMon.wFilterMin = 0x0000;
	s_PwrMon.wFilterMax = 0xFFFF;

}

//=============================================================================
/**
* パワーモニタ補正関連の初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitAspPwrMonCorrect(void)
{
	// s_wE2BaseCurrentVal[], s_dblPwrMonIncCurrentSlopeはSetE2BaseCurrentVal()で初期設定する。
	s_dblCabSlope = 0;
	s_dblCorrectValueCh1 = 0;
	s_dblCorrectValueCh4 = 0;
	s_dblE2BaseCurrentResult = 0;
}


//=============================================================================
/**
* パワーモニタイベントの初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitAspPwrMonEv(void)
{
	s_bPwrMonEvSw = FALSE;
	s_PwrMonEvData.byDetectType = RF_ASPPWREV_RANGEIN_L;
	s_PwrMonEvData.wPowerMin = 0x0000;
	s_PwrMonEvData.wPowerMax = 0xFFFF;
	s_PwrMonEvData.wPulseWidthMin = 0x0000;
	s_PwrMonEvData.wPulseWidthMax = 0x0001;
	s_dwTrgCh = PWRMONEV_TRG_LEVEL;

}
//=============================================================================
/**
* パワーモニタのE2ベース電流値算出
* @param なし
* @retval なし
*/
//=============================================================================
void CalcE2BaseCurrentVal(void)
{
	double dblVal, dblSlope, dblIntercept;
	double dblAvarage;

	/* 強制A/D変換によるキャリブレーション補正値取得 */
	GetPwrMonCalibrationVal();

	/* 1.8V, 3.3V, 5.0Vの場合は暫定値をそのまま使用する */
	switch (GetE2SupplyPwrVal()) {
	case 50:
		s_dblE2BaseCurrentResult = (double)s_wE2BaseCurrentVal[BASE50V];
		break;
	case 33:
		s_dblE2BaseCurrentResult = (double)s_wE2BaseCurrentVal[BASE33V];
		break;
	case 18:
		s_dblE2BaseCurrentResult = (double)s_wE2BaseCurrentVal[BASE18V];
		break;
	default:
		/* 1.9V-4.9Vの時 無負荷時のE2のベース電流（AD変換相当）を求める */
		/* ①BASE平均を求める */
		dblAvarage = 0;
		dblAvarage += s_wE2BaseCurrentVal[BASE20V];
		dblAvarage += s_wE2BaseCurrentVal[BASE33V];
		dblAvarage += s_wE2BaseCurrentVal[BASE45V];
		dblAvarage /= 3;

		/* ②共分散を求める */
		dblVal = (-1.26 * (s_wE2BaseCurrentVal[BASE20V] - dblAvarage) + 0.033333 * (s_wE2BaseCurrentVal[BASE33V] - dblAvarage) + 1.233333 * (s_wE2BaseCurrentVal[BASE45V] - dblAvarage)) / 3;

		/* ③ 近似直線の傾きを求める */
		dblSlope = dblVal / 1.042222;

		/* ④ 近似直線の切片 */
		dblIntercept = dblAvarage - (dblSlope * 3.266667);

		/* ⑤ 供給電圧設定値から、無負荷時のE2のベース電流（AD変換相当 */
		dblVal = (((double)GetE2SupplyPwrVal()) / 10); // 0.1V単位なので10で割る
		s_dblE2BaseCurrentResult = (dblVal * dblSlope + dblIntercept);
		break;
	}

	/* ⑧キャリブレーション 傾き */
	if (s_dblCorrectValueCh1 != s_dblCorrectValueCh4) {
		s_dblCabSlope = ((3282 - 814) / ((double)s_dblCorrectValueCh1 - (double)s_dblCorrectValueCh4));
	}
	else {
		// ありえないが、ゼロ除算を回避する
		// 補正後の計算結果が最悪でもマイナスにはならないように暫定値を入れる
		s_dblCabSlope = 1;	
	}
}

///////////////////////////////////////////////////////////////////////////////
// グローバル関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
 * パワーモニタ有効/無効の参照
 * @param なし
 * @retval TRUE  パワーモニタ有効
 * @retval FALSE パワーモニタ無効
 */
//=============================================================================
BOOL IsAspPwrMonEna(void)
{
	return s_bPwrMonEna;
}

//=============================================================================
/**
 * パワーモニタの設定値読み出し
 * @param pPowerMonitor パワーモニタサンプリング情報
 * @retval なし
 */
//=============================================================================
void GetAspPwrMon(RFW_PWRMONSAMP_DATA * pPowerMonitor)
{
	memcpy((void*)pPowerMonitor, &s_PwrMon, sizeof(s_PwrMon));
}

//=============================================================================
/**
 * パワーモニタイベントの設定値読み出し
 * @param pPowerMonitorEvent パワーモニタイベント情報
 * @retval なし
 */
//=============================================================================
void GetAspPwrMonEv(RFW_PWRMONEV_DATA * pPowerMonitorEvent)
{
	memcpy((void*)pPowerMonitorEvent, &s_PwrMonEvData, sizeof(s_PwrMonEvData));
}

//=============================================================================
/**
 * AD変換結果から電流値を算出し、補正をかける
 * note パワーモニタのFPGAフォーマットのまま代入すること
 * note bit16-27がAD変換値、bit31がch情報
 * @param dwAd AD変換値
 * @retval 電流値換算した結果[uA]
 */
//=============================================================================
DWORD CalcAd2Current(DWORD dwAd)
{
	double	dblData;
	DWORD	dwData;

	// AD変換値のみにする
	dwData = dwAd & PWRMON_MASK;
	dwData >>= 16;

	if (IsPwrMonCorrectValEna()) {

		/* (キャリブレーション補正傾き - 電流増に伴う傾き) × (AD変換結果 - 補正するBASE電流) */
		dblData = (s_dblCabSlope - s_dblPwrMonIncCurrentSlope) * ((double)dwData - s_dblE2BaseCurrentResult);

		/* 四捨五入(+0.5して切り捨て) */
		dblData += 0.5; 
		dblData = floor(dblData);
	}
	else {
		dblData = (double)dwData;
	}

	/* 補正結果が負の値になる場合があるので、0を入れる。 */
	/* 以後の計算処理はdblDataに対する乗除算しかないので、実行されても0のままとなり問題ない */
	if (dblData < 0) {
		dblData = 0;
	}

	dblData *= (double)3.3;	// 基準電圧
	dblData /= 4096;		// 12bitのADC
	dblData /= 50;			// 計装アンプのゲイン
	dblData /= (double)0.2;	// 抵抗値

	/* 現在はch2のみでパワーモニタを動作させている*/
	/* 高分解能(ch3)対応する際に以下の処理を有効化する */
#if 0 
	if (dwAd & PWRMON_CHINFO) {
		// ch3はゲインがch2の4倍している
		fData /= (double)4;
	}
#endif

	dblData *= 1000000;		// [A]->[uA]
	
	return (DWORD)dblData;
}

//=============================================================================
/**
* 指定の電流値から、FPGAへ設定する上限/下限設定値を補正算出する
* @param wPower 変換元電流値[10uA]
* @param byMode 下限値(0)or上限値(1)
* @retval 補正算出した上限/下限設定値[10uA]
*/
//=============================================================================
WORD CalcPwrMonThreshold(WORD wPower, BYTE byMode)
{
	double dblPower = 0;
	
	// AD変換値へ変換
	dblPower = ((((double)wPower * 10) / 1000000) * 10) / 3.3;
	dblPower *= 4096;

	/* 補正処理 */
	dblPower /= (s_dblCabSlope - s_dblPwrMonIncCurrentSlope);
	dblPower += s_dblE2BaseCurrentResult;
	
	if (byMode == CALC_MODE_MAX) {
		// 上限値指定の場合、切り捨て
		dblPower = floor(dblPower);
	}
	else {
		// 下限値指定の場合、切り上げ
		dblPower = ceil(dblPower);
	}
	
	return (WORD)(dblPower);
}

//=============================================================================
/**
* パワーモニタイベント削除のメイン処理
* @param なし
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_FACT_CANTCLR  パワーモニタイベントがE2アクションにリンクされている
*/
//=============================================================================
static DWORD ClrPwrMonEvMain(void)
{
	DWORD	dwRet = RFWERR_OK;
	UINT64	u64SingleEvent = 1;

	u64SingleEvent <<= s_dwTrgCh;

	// アクションとリンクされていないかチェック
	dwRet = CheckSingleEvent(RFW_ASPFUNC_CLR, u64SingleEvent, 0, 0);

	if (dwRet == RFWERR_OK) {
		// E2トリガのテーブル初期化
		ClrE2TrgTable(s_dwTrgCh);

		// パワーモニタイベント管理用変数の初期化
		InitAspPwrMonEv();
	}

	return dwRet;
}

//=============================================================================
/**
* パワーモニタE2個体データ設定
* @param wData0:	1.8V供給時の補正データ
* @param wData1:	2.0V供給時の補正データ 
* @param wData2:	3.3V供給時の補正データ
* @param wData3:	4.5V供給時の補正データ
* @param wData4:	5.0V供給時の補正データ
* @retval 
*/
//=============================================================================
void SetE2BaseCurrentVal(double dblSlope, WORD wData0, WORD wData1, WORD wData2, WORD wData3, WORD wData4)
{

	s_dblPwrMonIncCurrentSlope = dblSlope;
	s_wE2BaseCurrentVal[BASE18V] = wData0;
	s_wE2BaseCurrentVal[BASE20V] = wData1;
	s_wE2BaseCurrentVal[BASE33V] = wData2;
	s_wE2BaseCurrentVal[BASE45V] = wData3;
	s_wE2BaseCurrentVal[BASE50V] = wData4;
}