///////////////////////////////////////////////////////////////////////////////
/**
 * @file asp_setting.cpp
 * @brief ASP FPGA setting
 * @author TSSR M.Ogata, PA M.Tsuji
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2017/02/07 新規作成 TSSR M.Ogata
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#include "rfw_bitops.h"
#include "asp_setting.h"
#include "e2_asp_fpga.h"
#include "doasp_pwr.h"
#include "doasp_sample.h"
#include "doasp_sys.h"
#include "doasp_trg.h"
#include "rfwasp_trg.h"
#include "doasp_per.h"
#include "doasp_can.h"
#include "doasp_sys_family.h"
#include "rfwasp_recframe.h"

// ファイル内static変数の宣言
static DWORD s_dwSrc = 0;


// ファイル内static関数の宣言
WORD CalcADValue(DWORD);
BOOL IsProtocolAnalyzerStop(void);



//=============================================================================
/**
* E2拡張機能 記録対象の指定
* @param dwSrc:記録対象の機能
* @retval RFWERR_OK
* @retval RFWERR_N_BASP_ASPFUNCOFF
*/
//=============================================================================
DWORD DO_SetSamplingSource(DWORD dwSrc)
{
	DWORD dwRet = RFWERR_OK;

	RFW_ASPCONF_DATA AspConfData;

	ProtInit_ASP();

	GetAspConfData(&AspConfData);

	// パワーモニタ
	if (dwRet == RFWERR_OK) {
		if (dwSrc & RF_ASPSRC_POWER) {
			if (!(AspConfData.dwFunction3 & RF_ASPFUNC_PWRMON)) {
				dwRet = RFWERR_N_BASP_ASPFUNCOFF;
			}
		}
	}

	// 通信モニタch0
	if (dwRet == RFWERR_OK) {
		if (dwSrc & RF_ASPSRC_MON_CH0) {
			if (!(AspConfData.dwFunction0 & RF_ASPCONF_CAN_MON_CH0)) {
				dwRet = RFWERR_N_BASP_ASPFUNCOFF;
			}
		}
	}

	// 通信モニタch1
	if (dwRet == RFWERR_OK) {
		if (dwSrc & RF_ASPSRC_MON_CH1) {
			if (!(AspConfData.dwFunction1 & RF_ASPCONF_CAN_MON_CH1)) {
				dwRet = RFWERR_N_BASP_ASPFUNCOFF;
			}
		}
	}

	// ファミリ固有機能(いずれも無効の場合エラー)
	if (dwRet == RFWERR_OK) {
		if (dwSrc & RF_ASPSRC_FAMILY) {
			if (!(AspConfData.dwFunction3 & RF_ASPCONF_MON_EV) &&
				!(AspConfData.dwFunction3 & RF_ASPCONF_EXT_STRACE)) {
				dwRet = RFWERR_N_BASP_ASPFUNCOFF;
			}
		}
	}

	if (dwRet == RFWERR_OK) {
		s_dwSrc = dwSrc;
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
* ASP_MEMバスモード指定
* @param dwMode バスモード
* @retval なし
*/
//=============================================================================
void AspMemBusMode(DWORD dwMode)
{
	FPGA_SET_PAT(MEM_CTRL, dwMode);
}

//=============================================================================
/**
* ASP_MEMのリードモード指定
* @param dwRdMem リードモード
* @retval なし
*/
//=============================================================================
void AspMemRead(DWORD dwRdMem)
{
	/* USB_RDY_N(B2) ; BFW_RDY_N(B3) */
	DO_AspSetPort(0, 1);
	FPGA_SET_PAT(DTFIFO_CTRL, DTFIFOEN);

	FPGA_MDF_PAT(MEM_CTRL, MEM_CTRL_RDMEMEN, dwRdMem);
}

//=============================================================================
/**
* SFIFOの設定
* @param dwSrc 記録対象のSFIFO
* @retval なし
*/
//=============================================================================
void AspMemSfifoEn(DWORD dwSrc)
{
	DWORD dwMemSfifo = 0;
	if (dwSrc & RF_ASPSRC_POWER) {
		dwMemSfifo |= (0x10001 << 0);
	}
	if (dwSrc & RF_ASPSRC_MON_CH0) {
		dwMemSfifo |= (0x10001 << 1);
	}
	if (dwSrc & RF_ASPSRC_MON_CH1) {
		dwMemSfifo |= (0x10001 << 2);
	}
	if (dwSrc & RF_ASPSRC_EVL) {
		dwMemSfifo |= (0x10001 << 3);
	}
	if (dwSrc & RF_ASPSRC_FAMILY) {
		dwMemSfifo |= (0x10001 << 4);
	}
	FPGA_WRITE(SFIFO_CTRL, dwMemSfifo);
}

//=============================================================================
/**
* ASPFIFOの設定
* @param dwAspFifoEn ASPFIFOへの設定値
* @retval なし
*/
//=============================================================================
void AspMemAspFifoEn(DWORD dwAspFifoEn)
{
	FPGA_SET_PAT(ASPFIFO_CTRL, dwAspFifoEn);
}

//=============================================================================
/**
* ASP時間計測の開始ディレイ設定
* @param dwDelayCount ディレイ値
* @retval なし
*/
//=============================================================================
void SetAspTimeStampDelayCount(DWORD dwDelayCount)
{
	FPGA_WRITE(TS_DELAY, dwDelayCount);
}

//=============================================================================
/**
* E2拡張機能の動作開始設定
* @param  なし
* @retval RFWERR_OK
*/
//=============================================================================
DWORD Start_Asp(void)
{
	RFW_ASPCONF_DATA	pAspConfData;
	DWORD	dwDelay;

	// 拡張機能全体をリセット
	FPGA_SET_PAT(ASPCTRL, ASPCTRL_RSTEL | ASPCTRL_RSTPA | ASPCTRL_RSTCM | ASPCTRL_RSTMEM | ASPCTRL_RSTTIMER);
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTASPIF);

	// 拡張機能全体のレジスタをリセット
	FPGA_SET_PAT(ASPCTRL, ASPCTRL_RSTASPIFREGMSK);
	FPGA_SET_PAT(ASPCTRL, ASPCTRL_RSTASPFUNCREG);
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTASPFUNCREG);

	// ファミリ固有機能モジュールのリセットを行う
	Reset_Asp_family();

	// コンフィグ情報の取得
	GetAspConfData(&pAspConfData);

	if (IsAspPwrMonEna()) {
		// パワーモニタ有効時、拡張機能の設定を行う
		EnablePwrMon();
	}

	if (IsAspPerformanceEna()) {
		// 時間測定有効時、拡張機能の設定を行う
		EnablePerformance();
	}

	if (IsAspCanMonEna()) {
		// CANモニタ有効時、拡張機能の設定を行う
		EnableCanMon();
	}

	/* 外部トリガ入力 */
	EnableExtTrgIn();

	/* 外部トリガ出力は、RFW I/F関数呼び出し時にFPGAへ設定済み */
	
	// MCUファミリ固有の拡張機能の設定を行う
	Start_ASP_family();

	// イベントリンクの設定
	SetEventLink();
	
	/* TIME_STAMPのカウンタクリア */
	FPGA_SET_PAT(TS_STAT, TS_STAT_TSCLR);
	
	/* TIME_STAMPのDELAY設定 */
	dwDelay = GetAspDelay_family();
	SetAspTimeStampDelayCount(dwDelay);

	/* ASPCTRL RSTTIMER 解除 */
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTTIMER);

	/* TIME_STAMP有効 */
	FPGA_SET_PAT(TS_CTRL, TS_CTRL_TSEN);

	/* ストレージ切替 */
	switch (pAspConfData.dwStorage) {
	case RF_ASPSTORAGE_E2:
		/* E2ストレージ */
		AspMemBusMode(MEM_CTRL_MEMBUS_E2);
		/* リングモード */
		FPGA_WRITE(SRAM_MODE, SRAM_MODE_RING);
		break;
	case RF_ASPSTORAGE_E2_FULLSTOP:
	default:	// 記録しない場合もこの設定とする
		/* E2ストレージ */
		AspMemBusMode(MEM_CTRL_MEMBUS_E2);
		/* フルストップモード */
		FPGA_WRITE(SRAM_MODE, SRAM_MODE_FULLSTOP);
		break;
	case RF_ASPSTORAGE_E2_FULLBREAK:
		/* E2ストレージ */
		AspMemBusMode(MEM_CTRL_MEMBUS_E2);
		/* フルブレークモード */
		FPGA_WRITE(SRAM_MODE, SRAM_MODE_FULLBREAK);
		break;
	case RF_ASPSTORAGE_PC:
		/* PCストレージモード */
		AspMemBusMode(MEM_CTRL_MEMBUS_PC);
		break;
	}

	// ASP_MEMのリセット解除
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTMEM);

	/* DTFIFO 設定*/
	/* PCストレージの場合 */
	if (pAspConfData.dwStorage == RF_ASPSTORAGE_PC) {
		/* DTFIFOEN ON */
		FPGA_SET_PAT(DTFIFO_CTRL, DTFIFOEN);
	}
	
	/* SRAM有効 */
	/* E2ストレージの場合 */
	if (pAspConfData.dwStorage != RF_ASPSTORAGE_PC) {
		FPGA_SET_PAT(SRAM_CTRL, SRAM_CTRL_SRAMEN);
	}

	/* ASPFIFOをON */
	AspMemAspFifoEn(ASPFIFO_CTRL_ASPFIFOEN);
	
	/* 有効なSFIFOを設定 */
	if (pAspConfData.dwStorage != RF_ASPSTORAGE_NONE) {
		AspMemSfifoEn(s_dwSrc);
	} else {
		// 記録しない場合はSFIFOを止める。
		// 記録しない場合の対処はこれでよいか要検討
		AspMemSfifoEn(0);
	}

	/* MCUファミリで固有のタイミングで、拡張機能を動作開始する */
	AspRun_family();

	return RFWERR_OK;
}

//=============================================================================
/**
* E2拡張機能の動作停止設定
* @param  なし
* @retval なし
*/
//=============================================================================
void Stop_Asp(void)
{
	DWORD	dwSramStat;
	DWORD	dwSfifoStat;
	DWORD	dwAspfifoStat;
	UINT64	u64Timeout;

	// 拡張機能の停止
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_ASPRUN);

	// 未対応の記録対象を選択してしまった場合の暫定処置(1secで抜けるようにする)
	u64Timeout = GetTickCount64();

	/* SFIFO、ASPFIFOが空になるのを待つ */
	for (;;) {
		// 1secでタイムアウト
		if ((GetTickCount64() - u64Timeout) >= 1000) {
			break;
		}

		// プロトコルアナライザの解析完了待ち
		if (IsProtocolAnalyzerStop() == FALSE) {
			continue;
		}

		// 品種固有部のSRAMフル状態確認方法
		if (IsSramFullStat_family()) {
			break;
		}
		dwSramStat = FPGA_READ(SRAM_STAT);
		if ((dwSramStat & SRAM_STAT_SRAM_OVF) != 0) {
			// SRAMオーバーフロー発生時は待たない
			break;
		}

		// SFIFO→ASPFIFOの順でデータが流れるためチェックも同じ順で行う
		dwSfifoStat = FPGA_READ(SFIFO_STAT);
		if ((dwSfifoStat & SFIFO_STAT_EMPMASK) == SFIFO_STAT_EMPMASK) {
			// SFIFOが空
			dwAspfifoStat = FPGA_READ(ASPFIFO_STAT);
			if ((dwAspfifoStat & ASPFIFO_STAT_ASPFIFOEMP) != 0) {
				// ASPFIFOが空
				break;
			}
		}
	}

	// MCUファミリ固有の拡張機能の無効設定を行う
	Stop_ASP_family();
}

//=============================================================================
/**
* E2拡張機能のSRAM読み出し/Save処理
* @param  なし
* @retval RFWERR_OK
* @retval RFWERR_N_BASP_FPGA_START2
* @retval RFWERR_N_BASP_FPGA_SEQ
* @retval RFWERR_N_BASP_FPGA_SIZE
* @retval RFWERR_N_BASP_F_OPEN
* @retval RFWERR_N_BASP_F_SEEK
* @retval RFWERR_N_BASP_F_READ
* @retval RFWERR_N_BASP_F_WRITE
* @retval RFWERR_N_BASP_F_CLOSE
*/
//=============================================================================
DWORD Save_Asp(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwSramCount;

	// SRAMに入っているデータ数を保持する
	// DTFIFOイネーブルでカウントが減っていってしまうためSRAM読み出し設定の前で読み出す
	dwSramCount = (FPGA_READ(SRAM_STAT) & SRAM_STAT_SRAMCOUNT_MASK) * sizeof(WORD);

	if (dwSramCount > 0) {
		// SRAMが空で読み出し設定にするとリードモードから元に戻らないので
		// SRAMに入っている場合のみ、読み出し設定にする
		/* SRAM読み出し設定 */
		AspMemRead(MEM_CTRL_RDMEMEN);
	}

	/* SRAM読み出してsave */
	dwRet = AspSramSave(dwSramCount);

	return dwRet;
}

//=============================================================================
/**
 * パワーモニタ有効時のHW設定(実行時)
 * @param なし
 * @retval なし
 */
//=============================================================================
void EnablePwrMon(void)
{
	DWORD	dwSampf;
	RFW_PWRMONSAMP_DATA PwrMon;
	RFW_PWRMONEV_DATA PwrMonEvent;
	BYTE	byDetectType;
	BYTE	bySamplingMode;
	WORD	wPowerMin;
	WORD	wPowerMax;
	DWORD	dwWidth;
	RFW_ASPE2TRG_INFO AspE2TrgTable;

	// パワーモニタ設定情報の取得
	GetAspPwrMon((RFW_PWRMONSAMP_DATA *)&PwrMon);

	// パワーモニタの補正値算出
	CalcE2BaseCurrentVal();

	switch (PwrMon.bySamplingRate) {
	case RF_ASPPWR_FAST_HIGH:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_1U;
		break;
	case RF_ASPPWR_FAST_LOW:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_1U;
		break;
	case RF_ASPPWR_REG_2U:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_2U;
		break;
	case RF_ASPPWR_REG_5U:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_5U;
		break;
	case RF_ASPPWR_REG_10U:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_10U;
		break;
	case RF_ASPPWR_REG_20U:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_20U;
		break;
	case RF_ASPPWR_REG_50U:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_50U;
		break;
	case RF_ASPPWR_REG_100U:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_100U;
		break;
	case RF_ASPPWR_REG_200U:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_200U;
		break;
	case RF_ASPPWR_REG_500U:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_500U;
		break;
	case RF_ASPPWR_REG_1M:
	default:
		dwSampf = PWRMCTRL_SAMPF_REGVAL_1M;
		break;
	}
	// サンプリング周波数設定箇所が、PWRMCTRLレジスタの上位16ビットなので、
	// 16ビット左シフトする。
	dwSampf = dwSampf << 16;

	// サンプリングモード設定
	bySamplingMode = PwrMon.bySamplingMode << 1;
	FPGA_SET_PAT(PWRMCTRL, bySamplingMode);

	// サンプリング対象のチャンネル設定(通常分解能で固定)
	FPGA_SET_PAT(PWRMCTRL, PWRMCTRL_SAMP_CH2);

	// データ記録有効
	FPGA_SET_PAT(PWRMCTRL, PWRMCTRL_REC);

	// サンプリング周期の設定
	FPGA_CLR_PAT(PWRMCTRL, PWRMCTRL_SAMPF_MASK);
	FPGA_SET_PAT(PWRMCTRL, dwSampf);

	// パワーモニタイベント設定情報の取得
	GetAspPwrMonEv((RFW_PWRMONEV_DATA *)&PwrMonEvent);
	
	// パワーモニタイベントのE2トリガ検出種別の設定
	byDetectType = PwrMonEvent.byDetectType;
	if (byDetectType >= RF_ASPPWREV_RANGEIN_P) {
		byDetectType += 6;
	}
	FPGA_CLR_PAT(PWRM_EVLFACTFUNC, PWRM_FACTFUNC_MASK);
	FPGA_WRITE(PWRM_EVLFACTFUNC, (DWORD)byDetectType);

	// パワーモニタイベントの検出電流範囲の上限/下限の設定
	// RFW I/Fでは10μA単位なので、μA単位にするため10倍する
	// 電流値からAD変換値に変換したものをFPGAに設定する
	if (IsPwrMonCorrectThreasholdEna()) {
		wPowerMin = CalcPwrMonThreshold((PwrMonEvent.wPowerMin), CALC_MODE_MIN);
		wPowerMax = CalcPwrMonThreshold((PwrMonEvent.wPowerMax), CALC_MODE_MAX);
	}
	else {
		//補正無し
		wPowerMin = CalcADValue(PwrMonEvent.wPowerMin * 10);
		wPowerMax = CalcADValue(PwrMonEvent.wPowerMax * 10);	
	}

	// PWRM_EVLPの上位/下位16bitは、0xFFFが上限なので調整する
	if (wPowerMax > 0x0FFF) {
		wPowerMax = 0x0FFF;
	}
	if (wPowerMin > 0x0FFF) {
		wPowerMin = 0x0FFF;
	}

	FPGA_CLR_PAT(PWRM_EVLP, PWRM_PWRMAX_MASK|PWRM_PWRMIN_MASK);
	FPGA_WRITE(PWRM_EVLP, (DWORD)(((DWORD)wPowerMax << 16) | (DWORD)wPowerMin));

	// パワーモニタイベントのパルス検出幅の設定
	dwWidth = PwrMonEvent.wPulseWidthMin << 16 | PwrMonEvent.wPulseWidthMax;
	FPGA_WRITE(PWRM_EVLTP, dwWidth);

	if (PwrMon.bySamplingMode == RF_ASPPWR_EVLINK){
		FPGA_SET_PAT(PWRM_EVLCTRL, PWRM_EVLCTRL_ACTEN);
	}
	else {
		FPGA_CLR_PAT(PWRM_EVLCTRL, PWRM_EVLCTRL_ACTEN);
	}
	GetE2TrgTable(0x0A, &AspE2TrgTable);
	if (AspE2TrgTable.dwFactSw == RFW_EVENTLINK_SWON) {
		FPGA_SET_PAT(PWRM_EVLCTRL, PWRM_EVLCTRL_FACTEN);
	}
	else {
		GetE2TrgTable(0x0B, &AspE2TrgTable);
		if (AspE2TrgTable.dwFactSw == RFW_EVENTLINK_SWON) {
			FPGA_SET_PAT(PWRM_EVLCTRL, PWRM_EVLCTRL_FACTEN);
		}
		else {
			FPGA_CLR_PAT(PWRM_EVLCTRL, PWRM_EVLCTRL_FACTEN);
		}
	}

	// パワーモニタのリセット解除
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTCM);

	// パワーモニタ有効
	FPGA_SET_PAT(PWRMCTRL, PWRMCTRL_EN);
}


//=============================================================================
/**
 * 電流値をAD変換値に変換する
 * @param dwCurrent 電流値
 * @retval AD変換した値
 */
//=============================================================================
WORD CalcADValue(DWORD dwCurrent)
{
	WORD	wAd;
	double	dblAd;

	dblAd = (double)(dwCurrent / DENOMI_CALC_AD);
	dblAd += 0.5; // 四捨五入するため
	wAd = (WORD)dblAd;

	return wAd;
}

//=============================================================================
/**
 * パワーモニタ補正値算出のための強制AD変換のHW設定
 * @param dwSampCh サンプリング対象ch
 * @retval なし
 */
//=============================================================================
void ForcePwrMon(DWORD dwSampCh)
{
	DWORD dwSetData = 0;
	BYTE byWaitCnt = 0;

	// パワーモニタのリセット解除
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTCM);

	// サンプリング周期の設定
	FPGA_CLR_PAT(PWRMCTRL, PWRMCTRL_SAMPF_MASK);
	FPGA_SET_PAT(PWRMCTRL, PWRMCTRL_SAMPF_10US);

	// データ記録無効
	FPGA_CLR_PAT(PWRMCTRL, PWRMCTRL_REC);

	// サンプリング対象のチャンネル設定
	dwSetData = FPGA_READ(PWRMCTRL);
	dwSetData &= ~PWRMCTRL_SAMPCH_MASK;
	dwSetData |= dwSampCh;
	FPGA_WRITE(PWRMCTRL, dwSetData);

	// パワーモニタ有効
	FPGA_SET_PAT(PWRMCTRL, PWRMCTRL_EN);

	// 強制取得
	FPGA_SET_PAT(PWRMCTRL, PWRMCTRL_FORCE);

	// 変換終了までWait
	while (!(FPGA_READ(PWRMCTRL) & PWRMCTRL_READY)) {
		if (byWaitCnt < 10) {
			Sleep(1);
			byWaitCnt++;
		}
		else {
			break;
		}
	}
	
}

//=============================================================================
/**
 * パワーモニタの強制AD変換結果の取得
 * @param dwChNo　取得するチャネル
 * @retval AD変換結果
 */
//=============================================================================
DWORD GetForcePwrMonValue(DWORD dwChNo)
{
	DWORD	dwAdValue;

	switch (dwChNo) {
	case 1:
		dwAdValue = FPGA_READ(PWRMDBG0);
		dwAdValue &= PWRMON_AD_MASK;
		break;
	case 2:
		dwAdValue = FPGA_READ(PWRMDBG1);
		dwAdValue &= PWRMON_AD_MASK;
		break;
	case 3:
		dwAdValue = FPGA_READ(PWRMDBG2);
		dwAdValue &= PWRMON_AD_MASK;
		break;
	case 4:
		dwAdValue = FPGA_READ(PWRMDBG3);
		dwAdValue &= PWRMON_AD_MASK;
		break;
	default:
		dwAdValue = 0;
	}

	return dwAdValue;
}


//=============================================================================
/**
* 計測時間の参照
* @param byChNo チャネル番号
* @param pAspPerformanceData 時間計測結果
* @retval RFWERR_OK
* @retval RFWERR_N_BASP_TSTAMP_OVER
*/
//=============================================================================
DWORD GetAspPerData(BYTE byChNo, RFW_ASPPER_DATA *pAspPerformanceData)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwTmStat;
	UINT64	u64StartEvent;
	UINT64	u64EndEvent;
	UINT64	u64LevelEvent;
	RFW_ASPPER_DATA	AspPerformanceDataRaw;
	RFW_ASPPERCOND_DATA AspPerCond;

	if (byChNo == 0) {
		dwTmStat = FPGA_READ(TS_STAT);
		if ((dwTmStat & TS_STAT_TSOVF) == TS_STAT_TSOVF) {
			// オーバーフロー発生のエラー
			dwRet = RFWERR_N_BASP_TSTAMP_OVER;
			pAspPerformanceData->u64MinCount = 0;
			pAspPerformanceData->u64MaxCount = 0;
			pAspPerformanceData->u64SumCount = 0;
			pAspPerformanceData->dwPassCount = 0;
		} else {
			AspPerformanceDataRaw.u64MinCount = (UINT64)FPGA_READ(TM_MINH0) << 32;
			AspPerformanceDataRaw.u64MinCount |= FPGA_READ(TM_MINL0);
			if (AspPerformanceDataRaw.u64MinCount == 0xFFFFFFFFFFFF) {
				// 初期値の場合は0を返す(未計測)
				AspPerformanceDataRaw.u64MinCount = 0;
			}
			AspPerformanceDataRaw.u64MinCount = clk_to_nsec(AspPerformanceDataRaw.u64MinCount, OFF);

			AspPerformanceDataRaw.u64MaxCount = (UINT64)FPGA_READ(TM_MAXH0) << 32;
			AspPerformanceDataRaw.u64MaxCount |= FPGA_READ(TM_MAXL0);
			AspPerformanceDataRaw.u64MaxCount = clk_to_nsec(AspPerformanceDataRaw.u64MaxCount, OFF);

			AspPerformanceDataRaw.u64SumCount = (UINT64)FPGA_READ(TM_CUMH0) << 32;
			AspPerformanceDataRaw.u64SumCount |= FPGA_READ(TM_CUML0);
			AspPerformanceDataRaw.u64SumCount = clk_to_nsec(AspPerformanceDataRaw.u64SumCount, OFF);

			AspPerformanceDataRaw.dwPassCount = FPGA_READ(TM_CNT0);

			// 時間計測条件の取得
			GetPerformanceCondData(byChNo, &AspPerCond);

			if ((AspPerCond.byMode & RF_ASPPER_MODE_TRG) == RF_ASPPER_MODE_TRG_LEVEL) {
				// レベル型
				u64StartEvent = 0;
				u64EndEvent = 0;
				u64LevelEvent = AspPerCond.u64SectionEvent;
			} else {
				// パルス型
				u64StartEvent = AspPerCond.u64StartEvent;
				u64EndEvent = AspPerCond.u64EndEvent;
				u64LevelEvent = 0;
			}
			AspPerDataCalibration(AspPerformanceDataRaw, pAspPerformanceData, u64StartEvent, u64EndEvent, u64LevelEvent);
		}
	} else {
		dwTmStat = FPGA_READ(TS_STAT);
		if ((dwTmStat & TS_STAT_TSOVF) == TS_STAT_TSOVF) {
			// オーバーフロー発生のエラー
			dwRet = RFWERR_N_BASP_TSTAMP_OVER;
			pAspPerformanceData->u64MinCount = 0;
			pAspPerformanceData->u64MaxCount = 0;
			pAspPerformanceData->u64SumCount = 0;
			pAspPerformanceData->dwPassCount = 0;
		} else {
			AspPerformanceDataRaw.u64MinCount = (UINT64)FPGA_READ(TM_MINH1) << 32;
			AspPerformanceDataRaw.u64MinCount |= FPGA_READ(TM_MINL1);
			if (AspPerformanceDataRaw.u64MinCount == 0xFFFFFFFFFFFF) {
				// 初期値の場合は0を返す(未計測)
				AspPerformanceDataRaw.u64MinCount = 0;
			}
			AspPerformanceDataRaw.u64MinCount = clk_to_nsec(AspPerformanceDataRaw.u64MinCount, OFF);

			AspPerformanceDataRaw.u64MaxCount = (UINT64)FPGA_READ(TM_MAXH1) << 32;
			AspPerformanceDataRaw.u64MaxCount |= FPGA_READ(TM_MAXL1);
			AspPerformanceDataRaw.u64MaxCount = clk_to_nsec(AspPerformanceDataRaw.u64MaxCount, OFF);

			AspPerformanceDataRaw.u64SumCount = (UINT64)FPGA_READ(TM_CUMH1) << 32;
			AspPerformanceDataRaw.u64SumCount |= FPGA_READ(TM_CUML1);
			AspPerformanceDataRaw.u64SumCount = clk_to_nsec(AspPerformanceDataRaw.u64SumCount, OFF);

			AspPerformanceDataRaw.dwPassCount = FPGA_READ(TM_CNT1);

			// 時間計測条件の取得
			GetPerformanceCondData(byChNo, &AspPerCond);

			if ((AspPerCond.byMode & RF_ASPPER_MODE_TRG) == RF_ASPPER_MODE_TRG_LEVEL) {
				// レベル型
				u64StartEvent = 0;
				u64EndEvent = 0;
				u64LevelEvent = AspPerCond.u64SectionEvent;
			} else {
				// パルス型
				u64StartEvent = AspPerCond.u64StartEvent;
				u64EndEvent = AspPerCond.u64EndEvent;
				u64LevelEvent = 0;
			}
			AspPerDataCalibration(AspPerformanceDataRaw, pAspPerformanceData, u64StartEvent, u64EndEvent, u64LevelEvent);
		}
	}
	return dwRet;
}


//=============================================================================
/**
* 計測時間のクリア
* @param byChNo チャネル番号
* @retval なし
*/
//=============================================================================
void ClrAspPerData(BYTE byChNo)
{
	FPGA_SET_PAT(TM_STAT, TM_STAT_TMCLRCH0 << byChNo);
}


//=============================================================================
/**
* E2トリガアクションのFPGA設定
* @param なし
* @retval なし
*/
//=============================================================================
void SetEventLink(void)
{
	DWORD	dwChNo;
	RFW_ASPE2ACT_INFO AspE2actInfo;
	DWORD	fpgareg_sevl_ctrl;

	// FPGAの持っているチャネル分設定する
	// FWでは32bit以降も設定可能だが、FPGAはないので省略
	for(dwChNo = 0; dwChNo < RFW_EVENTLINK_CH_MAX; dwChNo++) {
		fpgareg_sevl_ctrl = SEVL_CTRL0 + ((dwChNo / 4) * 4);
		GetE2ActTable(dwChNo, &AspE2actInfo);
		if (AspE2actInfo.dwActSw == RFW_EVENTLINK_SWON) {
			// アクション側の設定がありの場合、要因を設定する
			FPGA_CLR_PAT(fpgareg_sevl_ctrl, RFW_EVENTLINK_FACT_MASK << ((dwChNo % 4) * 8));
			FPGA_SET_PAT(fpgareg_sevl_ctrl, (find_first_bit(AspE2actInfo.u64LinkFact) << (((dwChNo % 4) * 8) + 1)));
		}
	}

	// イベントリンクのリセット解除
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTEL);

	for (dwChNo = 0; dwChNo < RFW_EVENTLINK_CH_MAX; dwChNo++) {
		fpgareg_sevl_ctrl = SEVL_CTRL0 + ((dwChNo / 4) * 4);
		GetE2ActTable(dwChNo, &AspE2actInfo);
		if (AspE2actInfo.dwActSw == RFW_EVENTLINK_SWON) {
			// アクション側の設定がありの場合、イネーブルにする
			FPGA_SET_PAT(fpgareg_sevl_ctrl, 1 << ((dwChNo % 4) * 8));
		}
	}
}


//=============================================================================
/**
 * 外部トリガ入力機能の設定
 * @param byChNo チャネル番号
 * @param byDetectType トリガ入力検出条件
 * @retval なし
 */
//=============================================================================
void SetExTrgIn(BYTE byChNo, BYTE byDetectType)
{
	DWORD	dwFactFuncValue;
	DWORD	dwPortFuncValue;

	// ASPP_EVLFACTFUNC設定(E2トリガ検出条件選択)
	dwFactFuncValue = FPGA_READ(ASPP_EVLFACTFUNC);
	if (byChNo == RF_ASPTRGIN_CH0) {
		dwFactFuncValue &= 0x000000F0;
		dwFactFuncValue |= (DWORD)byDetectType & 0x0000000F;
	} else {
		dwFactFuncValue &= 0x0000000F;
		dwFactFuncValue |= (((DWORD)byDetectType) << ASPP_EVLFACTFUNC_TRGI1_ACTFUNC_BIT) & 0x000000F0;
	}
	FPGA_WRITE(ASPP_EVLFACTFUNC, dwFactFuncValue);

	// ASPPFSEL1設定(トリガ機能選択)
	dwPortFuncValue = FPGA_READ(ASPPFSEL1);
	if (byChNo == RF_ASPTRGIN_CH0) {
		dwPortFuncValue &= 0x0000F0FF;
		dwPortFuncValue |= (((DWORD)0x1) << ASPPFSEL1_ASP_D10_SEL_BIT) & 0x00000F00;
	} else {
		dwPortFuncValue &= 0x00000FFF;
		dwPortFuncValue |= (((DWORD)0x1) << ASPPFSEL1_ASP_D11_SEL_BIT) & 0x0000F000;
	}
	FPGA_WRITE(ASPPFSEL1, dwPortFuncValue);
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTASPIF);
}

//=============================================================================
/**
 * 外部トリガ入力機能の解除
 * @param byChNo チャネル番号
 * @retval なし
 */
//=============================================================================
void ClrExTrgIn(BYTE byChNo)
{
	DWORD	dwPortFuncValue;
	DWORD	dwFactFuncValue;

	// ASPPFSEL1設定(Normal Input/Output選択)
	dwPortFuncValue = FPGA_READ(ASPPFSEL1);
	if (byChNo == RF_ASPTRGIN_CH0) {
		dwPortFuncValue &= 0xFFFFF0FF;
	} else {
		dwPortFuncValue &= 0xFFFF0FFF;
	}
	FPGA_WRITE(ASPPFSEL1, dwPortFuncValue);

	// ASPP_EVLFACTFUNC設定(E2アクション出力条件選択)
	dwFactFuncValue = FPGA_READ(ASPP_EVLFACTFUNC);
	if (byChNo == RF_ASPTRGOUT_CH0) {
		dwFactFuncValue &= 0xFFFFFFF0;
	} else {
		dwFactFuncValue &= 0xFFFFFF0F;
	}
	FPGA_WRITE(ASPP_EVLFACTFUNC, dwFactFuncValue);
}


//=============================================================================
/**
 * 外部トリガ出力機能の設定
 * @param byChNo チャネル番号
 * @param pExTriggerOut 外部トリガ出力情報
 * @retval なし
 */
//=============================================================================
void SetExTrgOut(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut)
{
	DWORD	dwActFuncAddr;
	DWORD	dwActFuncValue;

	if (byChNo == RF_ASPTRGOUT_CH0) {
		dwActFuncAddr = ASPP_EVLACTFUNC0;
	} else {
		dwActFuncAddr = ASPP_EVLACTFUNC1;
	}

	dwActFuncValue = FPGA_READ(dwActFuncAddr);

	// bit0以外はマスク(bit0でアクティブレベルが決まる)
	dwActFuncValue &= 0x00000001;

	if (pExTriggerOut->byOutputType == RF_ASPTRGOUT_PULSE) {
		dwActFuncValue |= ASPP_EVLACTFUNC_PULSE;
	}

	dwActFuncValue |= (((DWORD)pExTriggerOut->wPulseWidth) << ASPP_EVLACTFUNC_TRGOWCNT_BIT) & 0xFFFF0000;

	FPGA_WRITE(dwActFuncAddr, dwActFuncValue);
}

//=============================================================================
/**
 * 外部トリガ出力機能の解除
 * @param byChNo チャネル番号
 * @retval なし
 */
//=============================================================================
void ClrExTrgOut(BYTE byChNo)
{
	// ASPP_EVLACTFUNC設定(E2アクション出力条件選択)
	// アクティブレベル以外を初期値に戻す
	if (byChNo == RF_ASPTRGOUT_CH0) {
		FPGA_MDF_PAT(ASPP_EVLACTFUNC0, 0x00010000, 0xFFFFFFFE);
	} else {
		FPGA_MDF_PAT(ASPP_EVLACTFUNC1, 0x00010000, 0xFFFFFFFE);
	}
}


//=============================================================================
/**
* 外部トリガ出力のアクティブレベル設定
* @param byChNo チャネル番号
* @param byActiveLevel 外部トリガ出力アクティブレベル
* @retval なし
*/
//=============================================================================
void SetExTrgOutLevel(BYTE byChNo, BYTE byActiveLevel)
{
	DWORD	dwPortFuncValue;
	DWORD	dwMaskValue;
	BYTE	bySiftValue;
	UINT64	u64Timeout;

	// 直前のトリガパルス出力が終わるのを待つため、100msウェイト
	// パルス出力の最大は約65.5msecのため余裕をもって100msec
	u64Timeout = GetTickCount64();
	for (;;) {
		if ((GetTickCount64() - u64Timeout) >= 100) {
			break;
		}
	}

	// E2アクション動作を無効化
	FPGA_CLR_PAT(ASPP_EVLCTRL, ASPP_EVLCTRL_ACTEN);

	if (byChNo == RF_ASPTRGOUT_CH0) {
		if (byActiveLevel == RF_ASPTRGOUT_HACTIVE) {
			FPGA_SET_PAT(ASPP_EVLACTFUNC0, ASPP_EVLACTFUNC_HACTIVE);
		} else {
			FPGA_CLR_PAT(ASPP_EVLACTFUNC0, ASPP_EVLACTFUNC_HACTIVE);
		}
	} else {
		if (byActiveLevel == RF_ASPTRGOUT_HACTIVE) {
			FPGA_SET_PAT(ASPP_EVLACTFUNC1, ASPP_EVLACTFUNC_HACTIVE);
		} else {
			FPGA_CLR_PAT(ASPP_EVLACTFUNC1, ASPP_EVLACTFUNC_HACTIVE);
		}
	}

	// ASPPFSEL1設定(トリガ機能選択)
	dwPortFuncValue = FPGA_READ(ASPPFSEL1);

	if (byChNo == RF_ASPTRGOUT_CH0) {
		dwMaskValue = 0x0000000F;
		bySiftValue = ASPPFSEL1_ASP_D8_SEL_BIT;
	} else {
		dwMaskValue = 0x000000F0;
		bySiftValue = ASPPFSEL1_ASP_D9_SEL_BIT;
	}
	dwPortFuncValue &= ~dwMaskValue;
	dwPortFuncValue |= (((DWORD)0x1) << bySiftValue) & dwMaskValue;
	FPGA_WRITE(ASPPFSEL1, dwPortFuncValue);
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTASPIF);

	// E2アクション動作を有効化
	FPGA_SET_PAT(ASPP_EVLCTRL, ASPP_EVLCTRL_ACTEN);
}


//=============================================================================
/**
* 時間測定有効時のHW設定(実行時)
* @param なし
* @retval なし
*/
//=============================================================================
void EnablePerformance(void)
{
	RFW_ASPPERCOND_DATA AspPerCond;
	BYTE	byCnt;
	UINT64	u64CmpData;
	UINT64	u64StartEvent;
	UINT64	u64EndEvent;
	UINT64	u64LevelEvent;

	for (byCnt = 0; byCnt < 2; byCnt++) {
		// 時間計測機能の設定
		if (GetPerformanceEna(byCnt)) {
			GetPerformanceCondData(byCnt, &AspPerCond);

			if ((AspPerCond.byMode & RF_ASPPER_MODE_TRG) == RF_ASPPER_MODE_TRG_LEVEL) {
				// レベル型
				u64StartEvent = 0;
				u64EndEvent = 0;
				u64LevelEvent = AspPerCond.u64SectionEvent;
			} else {
				// パルス型
				u64StartEvent = AspPerCond.u64StartEvent;
				u64EndEvent = AspPerCond.u64EndEvent;
				u64LevelEvent = 0;
			}

			// 比較データの設定
			AspPerThresholdCalibration(AspPerCond.u64Threshold, &u64CmpData, u64StartEvent, u64EndEvent, u64LevelEvent);
			// nsec単位から120MHzカウント値に変更
			u64CmpData = nsec_to_clk(u64CmpData, OFF);
			FPGA_WRITE(TM_CMPL0 + (byCnt * 8), (DWORD)(u64CmpData));
			FPGA_WRITE(TM_CMPH0 + (byCnt * 8), (DWORD)(u64CmpData >> 32));

			// 上限、下限の設定
			if ((AspPerCond.byMode & RF_ASPPER_MODE_LMT) == RF_ASPPER_MODE_LMT_UPPER) {
				// 上限の場合
				FPGA_SET_PAT(TM_FACT, TM_FACT_UPPERLMT << (byCnt * 4));
			} else {
				// 下限の場合
				FPGA_CLR_PAT(TM_FACT, TM_FACT_UPPERLMT << (byCnt * 4));
			}

			// 計測許可設定
			FPGA_SET_PAT(TM_CTRL, (TM_CTRL_TMENCH0 | TM_CTRL_TMFCTENCH0 | TM_CTRL_TMACTENCH0) << byCnt);
		}
	}
}


//=============================================================================
/**
* CANモニタ有効時のHW設定(実行時)
* @param なし
* @retval なし
*/
//=============================================================================
void EnableCanMon(void)
{
	RFW_CANMON_DATA		CanMon;
	RFW_CANMONEV_DATA	CanMonEv;
	BYTE	byCnt;
	DWORD	dwPactrlData;
	DWORD	dwAsppfsel0Data;
	DWORD	dwAsppfsel0Mask;
	DWORD	dwSampPoint;
	DWORD	dwData;
	DWORD	dwMask;

	for (byCnt = 0; byCnt < 2; byCnt++) {
		// CANモニタの設定
		if (GetCanMonEna(byCnt)) {
			GetCanMonData(byCnt, &CanMon);

			// 記録有効、種別はCAN
			dwPactrlData = PAnCTRL_REC | PAnCTRL_TYPE_CAN;

			// フォーマットの指定
			if (CanMon.byCanMode == RF_ASPCAN_STANDARD_ID) {
				dwPactrlData |= PAnCTRL_MODE_CAN_STD;
			} else {
				dwPactrlData |= PAnCTRL_MODE_CAN_EXT;
			}

			// プロトコルアナライザの設定
			FPGA_WRITE(PA0CTRL + (0x100 * byCnt), dwPactrlData);

			// ボーレートの設定
			FPGA_WRITE(PA0BAUDRATE + (0x100 * byCnt), (DWORD)CanMon.wBaudrate);

			// サンプリングポイントの設定
			dwSampPoint = (DWORD)(((float)CanMon.wBaudrate * (float)CanMon.bySamplingPoint) / 100);
			if (dwSampPoint < 0x5) {
				// サンプリングポイントの設定可能な最小値は5
				dwSampPoint = 5;
			}
			FPGA_WRITE(PA0SAMPPNT + (0x100 * byCnt), dwSampPoint);

			dwAsppfsel0Data = ((ASPPFSEL0_ASP_CAN << ASPPFSEL0_ASP_D0_SEL_BIT)
				| (ASPPFSEL0_ASP_CAN << ASPPFSEL0_ASP_D1_SEL_BIT)
				| (ASPPFSEL0_ASP_CAN << ASPPFSEL0_ASP_D2_SEL_BIT)
				| (ASPPFSEL0_ASP_CAN << ASPPFSEL0_ASP_D3_SEL_BIT));

			dwAsppfsel0Data <<= (byCnt * 16);
			dwAsppfsel0Mask = ASPPFSEL0_ASP_MASK << (byCnt * 16);

			// ASP I/Fの端子をCANに設定
			FPGA_MDF_PAT(ASPPFSEL0, dwAsppfsel0Data, dwAsppfsel0Mask);
		}

		// CANモニタイベントの設定
		if (GetCanMonEvEna(byCnt)) {
			// 設定情報の取得
			GetCanMonEvData(byCnt, &CanMonEv);

			if ((CanMonEv.byCanMode & RF_ASPCAN_EXTENDED_ID) == RF_ASPCAN_EXTENDED_ID) {
				// 拡張IDの場合
				dwData = CanMonEv.dwID & 0x000007FF;			// bit10-0 : ID
				dwData |= (CanMonEv.bySRR << 11);				// bit11   : SRR
				dwData |= (1 << 12);							// bit12   : IDE
				dwData |= ((CanMonEv.dwID & 0x1FFFF800) << 2);	// bit30-13: ExtendedID
				dwData |= (CanMonEv.byRTR << 31);				// bit31   : RTR
			} else {
				// 標準IDの場合
				dwData = CanMonEv.dwID & 0x000007FF;			// bit10-0 : ID
				dwData |= (CanMonEv.byRTR << 31);				// bit31   : RTR
			}

			FPGA_WRITE(PA0_EVLP00 + (0x100 * byCnt), dwData);

			dwData = CanMonEv.byDLC;							// bit3-0  : DLC
			dwData |= (CanMonEv.dwData[0] << 6);				// bit31-6 : Data4[1:0], Data3, Data2, Data1

			FPGA_WRITE(PA0_EVLP01 + (0x100 * byCnt), dwData);

			dwData = ((CanMonEv.dwData[0] >> 26) & 0x0000003F);	// bit5-0  : Data4[7:2]
			dwData |= (CanMonEv.dwData[1] << 6);				// bit31-6 : Data8[1:0], Data7, Data6, Data5

			FPGA_WRITE(PA0_EVLP02 + (0x100 * byCnt), dwData);

			dwData = ((CanMonEv.dwData[1] >> 26) & 0x0000003F);	// bit5-0  : Data8[7:2]

			FPGA_WRITE(PA0_EVLP03 + (0x100 * byCnt), dwData);

			FPGA_WRITE(PA0_EVLP04 + (0x100 * byCnt), 0);

			// マスクデータの設定
			if ((CanMonEv.byCanMode & RF_ASPCAN_EXTENDED_ID) == RF_ASPCAN_EXTENDED_ID) {
				// 拡張IDの場合
				dwMask = CanMonEv.dwIDMask & 0x000007FF;			// bit10-0 : ID
				dwMask |= ((CanMonEv.dwIDMask & 0x1FFFF800) << 2);	// bit30-13: ExtendedID
				// SRR, IDE, RTRはマスクしない
			} else {
				// 標準IDの場合
				dwMask = 0x7FFFE000;								// bit30-13: マスク対象
				dwMask |= CanMonEv.dwIDMask & 0x000007FF;			// bit10-0 : ID
				// SRR, IDE, RTRはマスクしない
			}

			FPGA_WRITE(PA0_EVLPMSK00 + (0x100 * byCnt), dwMask);

			dwMask = (CanMonEv.dwMask[0] << 6);						// bit31-6 : Data4[1:0], Data3, Data2, Data1
			// DLCはマスクしない

			FPGA_WRITE(PA0_EVLPMSK01 + (0x100 * byCnt), dwMask);

			dwMask = ((CanMonEv.dwMask[0] >> 26) & 0x0000003F);		// bit5-0  : Mask4[7:2]
			dwMask |= (CanMonEv.dwMask[1] << 6);					// bit31-6 : Mask8[1:0], Mask7, Mask6, Mask5

			FPGA_WRITE(PA0_EVLPMSK02 + (0x100 * byCnt), dwMask);

			dwMask = 0xFFFFFFC0;
			dwMask |= ((CanMonEv.dwMask[1] >> 26) & 0x0000003F);	// bit5-0  : Mask8[7:2]

			FPGA_WRITE(PA0_EVLPMSK03 + (0x100 * byCnt), dwMask);

			FPGA_WRITE(PA0_EVLPMSK04 + (0x100 * byCnt), 0xFFFFFFFF);

			// ディレイカウンタの設定
			FPGA_WRITE(PA0_EVLFACTCNT + (0x100 * byCnt), CanMonEv.byDelayCount);

			// 一致のみサポート
			FPGA_WRITE(PA0_EVLFACTFUNC + (0x100 * byCnt), PAn_EVLFACTFUNC_MATCHP);

			// トリガの有効化
			FPGA_WRITE(PA0_EVLCTRL + (0x100 * byCnt), PAn_EVLCTRL_FACTEN);
		}
	}
	// プロトコルアナライザのリセット解除
	FPGA_CLR_PAT(ASPCTRL, ASPCTRL_RSTPA);

	for (byCnt = 0; byCnt < 2; byCnt++) {
		// CANモニタの設定
		if (GetCanMonEna(byCnt)) {
			// プロトコルアナライザの設定
			FPGA_SET_PAT(PA0CTRL + (0x100 * byCnt), PAnCTRL_EN);
		}
	}
}


//=============================================================================
/**
* 外部トリガ入力機能のHW設定(実行開始時)
* @param なし
* @retval なし
*/
//=============================================================================
void EnableExtTrgIn(void)
{
	/* 外部トリガ入力を使う場合 */
	if ((IsAspExTrgInEna(RF_ASPTRGIN_CH0)) || (IsAspExTrgInEna(RF_ASPTRGIN_CH1))) {
		// E2トリガ検出を有効化
		FPGA_SET_PAT(ASPP_EVLCTRL, ASPP_EVLCTRL_FACTEN);
	} else {
		// E2トリガ検出を無効化
		FPGA_CLR_PAT(ASPP_EVLCTRL, ASPP_EVLCTRL_FACTEN);
	}
}

//=============================================================================
/**
* プロトコルアナライザの解析完了チェック
* @param なし
* @retval TRUE  解析完了
* @retval FALSE 解析中
*/
//=============================================================================
BOOL IsProtocolAnalyzerStop(void)
{
	DWORD	dwPa0ctrl;
	DWORD	dwPa1ctrl;
	BOOL	bEndFlag = FALSE;

	if (GetCanMonEna(0) || GetCanMonEna(1)) {
		// CANモニタが有効
		dwPa0ctrl = FPGA_READ(PA0CTRL);
		dwPa1ctrl = FPGA_READ(PA1CTRL);

		if (((dwPa0ctrl & PAnCTRL_BUSY) == 0)
		&& ((dwPa1ctrl & PAnCTRL_BUSY) == 0)) {
			// 2chとも解析完了でTRUEとする
			bEndFlag = TRUE;
		}
	} else {
		// CANモニタが無効の場合は解析完了とする
		bEndFlag = TRUE;
	}

	return bEndFlag;
}
