///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_sys_family.cpp
 * @brief
 * @author S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/04/27
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
	新規作成
*/
#include "rfw_typedef.h"
#include "doasp_sys_family.h"
#include "ex_inc2.h"
#include "getstat.h"
#include "socunit.h"
#include "doasprh_swtrc.h"
#include "asprh_setting.h"
#include "doasp_exe.h"
#include "doasp_sys.h"
#include "asp_setting.h"
#include "e2_asp_fpga.h"
#include "doasp_sample.h"
#include "rfwasp_recframe.h"

extern "C" {
	int		EnableSwTrc(void);
	BOOL	GetSaveSramRawDataFlag(void);
	UCHAR	GetExtPwrVal(void);
	BOOL	GetSftTrcEnable(void);
	BOOL	isSftTrcEnable(void);
	BOOL	GetPowerFlag(void);
	BOOL	GetPowerAspFlag(void);
	BOOL	IsSwTrcNotRecord(void);
	BOOL	IsSwTrcNotRecord_PreInfo(void);
}

// マクロ定義
#define	DELAYTIME_BFWPROC					(160)	// BFW aspRunセットからDBG_CTRLP送信までの処理時間(ns)
#define	DELAYTIME_DBGCTRLP_COMLENGTH		(57)	// DBG_CTRLP情報送信データ長(19bit×3LPDCLK)
#define	DELAYTIME_SFTTOLDU_CYCLE			(3)		// TCU_SFTからLDUまでの転送クロック数(3×DCUCLK)
// FPGAはソフトトレースデータの最終STOPビットの50%位置で検出(bit数を2倍で計算し計算結果を1/2にする)
#define	DELAYTIME_SWTRC_DBTAGLENGTH			(37)	// ソフトウェアトレースDBTAGデータ長(18.5bit×1回分(37×1/2))
#define	DELAYTIME_SWTRC_DBPUSH_DBCPLENGTH	(113)	// ソフトウェアトレースDBPUSH/DBCPデータ長(19bit×2回＋18.5bit×1回分(113×1/2))

// ソフトウェアレース通信完了からFPGA検出までの補正値
// LPD-4Pin計算式：①(LPDCLK×2.5)＋②(LPD内部CLK×3)＋④(120MHz×8)
// LPD-1Pin計算式：③(LPD内部CLK(66MHz)×4)＋④(120MHz×8)
//     ★以下に示すFPGA内の+α誤差成分は含めない
//         LPD内部CLK(66MHz)×1＋120MHz×1
#define	DELAYTIME_SWTRC_LPD4_FPGA_LPDCYCLE		(5)		// ①LPD-4Pin時のFPGA補正成分：LPDCLK×2.5(5×1/2)
#define	DELAYTIME_SWTRC_LPD4_FPGA_INT66CYCLE	(45)	// ②LPD-4Pin(20MHz/25MHz以外)時のFPGA補正成分固定値(LPD内部CLK(66MHz)×3≒45ns)
#define	DELAYTIME_SWTRC_LPD4_FPGA_INT80CYCLE	(38)	// ②LPD-4Pin(20MHz)時のFPGA補正成分固定値(LPD内部CLK(80MHz)×3≒38ns)
#define	DELAYTIME_SWTRC_LPD4_FPGA_INT100CYCLE	(30)	// ②LPD-4Pin(25MHz)時のFPGA補正成分固定値(LPD内部CLK(100MHz)×3≒30ns)
#define	DELAYTIME_SWTRC_LPD1_FPGA_INT66CYCLE	(61)	// ③LPD-1Pin時のFPGA補正成分固定値(LPD内部CLK(66MHz)×4≒61ns)(LPD 1Pin時は固定)
#define	DELAYTIME_SWTRC_FPGA_COMCYCLE			(67)	// ④LPD-4Pin/1Pin時のFPGA共通補正成分固定値(120MHz×8≒67ns)
#define	LPD4CLK_20MHZ	(20000)
#define	LPD4CLK_25MHZ	(25000)

// static変数

// static関数


//=============================================================================
/**
* RFW I/F関数を跨いだUSB通信データパケットの結合を防止するための前処理
* @param なし
* @retval なし
*/
//=============================================================================
void ProtInit_ASP(void)
{
	ProtInit();
}

//=============================================================================
/**
* RFW I/F関数を跨いだUSB通信データパケットの結合を防止するための後処理
* @param なし
* @retval FFWERR_OK(EX_NOERROR/RFWERR_OK)
*/
//=============================================================================
DWORD ProtEnd_ASP(void)
{
	return ProtEnd();
}


//=============================================================================
/**
* RFW I/F関連処理におけるメイン処理呼び出し前のコマンドチェック関数
* @param byCommandID 拡張機能のコマンドID
* @retval RFWエラーコード
*/
//=============================================================================
DWORD CmdExecutCheck_ASP(BYTE byCommandID)
{
	int	nExecErr;

	nExecErr = CmdExecutCheck(byCommandID);

	return TransErrExec2Rfw(nExecErr);
}


//=============================================================================
/**
* RFW I/F関連処理におけるメイン処理呼び出し後のコマンドチェック関数
* @param byCommandID 拡張機能のコマンドID
* @retval RFWエラーコード
*/
//=============================================================================
DWORD EndOfCmdExecut_ASP(BYTE byCommandID)
{
	int	nExecErr;

	nExecErr = EndOfCmdExecut(byCommandID, RFWERR_OK);

	return TransErrExec2Rfw(nExecErr);
}

//=============================================================================
/**
* EXECエラーコードからRFWエラーコードに変換する関数
* @param nExecErr	EXECエラーコード
* @retval dwRfwErr	RFWエラーコード
*/
//=============================================================================
DWORD TransErrExec2Rfw(int nExecErr)
{
	DWORD	dwRfwErr;

	switch (nExecErr) {
	case EX_NOERROR:
		dwRfwErr = RFWERR_OK;
		break;
	case EX_FE_COMM:
		dwRfwErr = RFWERR_N_BMCU_NONCONNECT;
		break;
	case EX_FE_USBFAIL:
		dwRfwErr = RFWERR_D_COM;
		break;
	case EX_FE_SENDOUT:
		dwRfwErr = RFWERR_SU_MULTI_RFW_REQ_TIMEOUT;
		break;
	case EX_SE_TRGTOFF:
		dwRfwErr = RFWERR_N_BTARGET_POWER;
		break;
	case EX_DE_M_DCUACCESS:
		dwRfwErr = RFWERR_SU_LPD_TRANSFER;
		break;
	default:
		// 上記以外は未定義のエラーとする
		dwRfwErr = 0xFFFFFFFF;
		break;
	}
	return dwRfwErr;
}


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
	if (GetIceIfInf() == ICEIF_E2) {
		return TRUE;
	} else {
		return FALSE;
	}
}


int SendBlock_ASP(const BYTE* pData, DWORD size)
{
	// タイムアウト時間は使用しないので0にしておく
	return usb_ServerSend((UCHAR *) pData, size, 0);
}


//=============================================================================
/**
 * ファミリ固有機能の取得情報を返す関数
 * RHはソフトトレースのみ
 * @param なし
 * @retval ソフトトレースのみ
 */
//=============================================================================
DWORD GetAspSeriesFunc(void)
{
	return RF_ASPFUNC_RH_SWTRC;
}


//=============================================================================
/**
* ファミリ固有のサポートしているE2トリガ情報を返す
* @param なし
* @retval RF_ASPFACT_SUPORT_RH
*/
//=============================================================================
UINT64 GetAspSeriesFact(void)
{
	return RF_ASPFACT_SUPORT_RH;
}


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

	if (IsAspRun()) {
		// 拡張機能有効のRUN中はフラグ判定で終了
		return TRUE;
	}

	dbg_stat = getexestatruntrc(DBG_STAT_NOREAD1);

	if ((dbg_stat & ES_USRPRO_RUN) != 0) {
		return TRUE;
	}
	return FALSE;
}


//=============================================================================
/**
* 電源供給状態の取得
* @note ファミリ固有
* @param なし
* @retval TRUE ：供給中
* @retval FALSE：未供給
*/
//=============================================================================
BOOL IsSupplyPower(void)
{
	return GetPowerFlag();
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
	return GetPowerAspFlag();
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
	if (GetDebugModeInfo() == 0) {
		return FALSE;
	}
	return TRUE;
}


//=============================================================================
/**
* RUN開始直前のRH固有のFPGA設定処理呼び出し
* @note
* @param なし
* @retval なし
*/
//=============================================================================
void Start_ASP_family(void)
{
	if (IsAspSwTrcEvEna()) {
		// ソフトトレースイベント有効時、拡張機能の設定を行う
		EnableSwTrcEv();
	}

	// コマンドボックスのブレーク要求コマンドを設定する
	// ブレーク要求は強制ブレークでも使用するため必ず設定する
	EnableBrkEv();

	if (!IsAspSwTrcEna() || !GetSftTrcEnable() || !isSftTrcEnable() || (IsSwTrcNotRecord_PreInfo() != FALSE) ) {
		// ソフトトレースが無効の場合
		// ステータスチェックをコマンドボックスで行う
		EnableDetectBrk_CMDBOX();
	} else {
		// ソフトトレース有効の場合
		// ブレークチェックはソフトトレース側で設定しているためここでは何も行わない
	}

	// ソフトウェアトレースのモジュールリセット解除
	// コマンドボックスを動かすにも必要なため、無条件で解除する
	SwTrcResetRelease();

	if (IsStopTraceEventActEna()) {
		// 内蔵トレース停止コマンドが有効の場合
		EnableStopTrcEv();
	}

	if ((IsAspSwTrcEna()) && (IsSwTrcNotRecord_PreInfo() == FALSE)) {
		// ソフトトレース有効時、拡張機能の設定を行う
		// ソフトトレース取得側の有効無効の判断も込み
		// 有効の場合、これ以降DCUアクセスは禁止
		EnableSwTrc();
	}
}


//=============================================================================
/**
* FPGAに対し、RH固有のASP機能の無効設定を行う
* @param なし
* @retval なし
*/
//=============================================================================
void Stop_ASP_family(void)
{
	// RUNフラグのクリア
	SetAspRunFlag(FALSE);
}


//=============================================================================
/**
* RUN開始処理(ファミリ固有で行う)
* @note
* @param なし
* @retval なし
*/
//=============================================================================
void AspRun_family(void)
{
	AspRun_RH();

	// RUNフラグのセット
	SetAspRunFlag(TRUE);
}


//=============================================================================
/**
* 管理用変数初期化のファミリ固有処理
* @note
* @param なし
* @retval なし
*/
//=============================================================================
void InitAsp_family(void)
{
	InitAspSwTrcEv(0xff);
	SetAspRunFlag(FALSE);
}


//=============================================================================
/**
* SRAMフル状態を確認(MCU固有部を見る確認)
* note RH未対応(FALSEを返すだけ)
* @param なし
* @retval TRUE:SRAMフル状態、　FALSE:SRAMフルでない
*/
//=============================================================================
BOOL IsSramFullStat_family(void)
{
	return FALSE;
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
* E2拡張機能(共通)でサポートする機能の情報を返す関数
* @param なし
* @retval
*/
//=============================================================================
DWORD GetAspCommonFunc(void)
{
	DWORD dwFunc = 0;

	// CANモニタ
	dwFunc |= RF_ASPFUNC_MON_CAN;

	// パフォーマンス
	dwFunc |= RF_ASPFUNC_PERFORMANCE;

	// 外部トリガ
	dwFunc |= RF_ASPFUNC_EXTRG;

	// ブレーク検出
	dwFunc |= RF_ASPFUNC_FACTBRK;

	// ブレーク要求
	dwFunc |= RF_ASPFUNC_ACTBRK;

	// トレース停止
	dwFunc |= RF_ASPFUNC_STOPTRC;

	return dwFunc;
}


//=============================================================================
/**
* E2拡張機能(共通)でサポートするストレージモードを返す関数
* @param なし
* @retval dwStorage
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


//=============================================================================
/**
* ファミリ固有のE2拡張機能の使用状況を取得する関数
* @param byFunc ファンクション番号
* @retval TRUE(使用中)
* @retval FALSE(使用していない)
*/
//=============================================================================
BOOL IsFamilyAspFuncEna(BYTE byFunc)
{
	BOOL	sFlag;
	sFlag = FALSE;
	if (byFunc == RF_ASPFUNC_STRACE) {
		sFlag = IsAspSwTrcEvEna();
	}
	return sFlag;
}


//=============================================================================
/**
* パワーモニタで測定した電流値に補正処理を適用するか否かを取得する関数
* @note RHではパワーモニタをサポートしないためFALSE固定で返す
* @param なし
* @retval FALSE(適用しない)
*/
//=============================================================================
BOOL IsPwrMonCorrectValEna(void)
{
	return FALSE;
}


//=============================================================================
/**
* パワーモニタイベントの下限/上限値に補正処理を適用するか否かを取得する関数
* @note RHではパワーモニタをサポートしないためFALSE固定で返す
* @param なし
* @retval FALSE(適用しない)
*/
//=============================================================================
BOOL IsPwrMonCorrectThreasholdEna(void)
{
	return FALSE;
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
	return GetExtPwrVal();
}


//=============================================================================
/**
* E2フルブレークモードかどうかを返す
* @param なし
* @retval TRUE：フルブレークモード
* @retval FALSE：上記以外
*/
//=============================================================================
BOOL IsE2FullBrkMode(void)
{
	RFW_ASPCONF_DATA	AspConfData;
	GetAspConfData(&AspConfData);
	if (AspConfData.dwStorage == RF_ASPSTORAGE_E2_FULLBREAK) {
		return TRUE;
	}
	return FALSE;
}


//=============================================================================
/**
* SRAMがフル状態かどうかを返す
* @param なし
* @retval TRUE：フル状態
* @retval FALSE：上記以外
*/
//=============================================================================
BOOL IsE2FullStat(void)
{
	if ((FPGA_READ(SRAM_STAT) & SRAM_STAT_SRAM_LIMIT) != 0) {
		return TRUE;
	}
	return FALSE;
}


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
	unsigned long	ulIf_Clock;
	unsigned long	ulIf_Tck;
	UINT64			u64If_Clock_Delay;
	UINT64			u64Dcu_Clock_Delay;
	INT64			i64Etc_Delay;
	INT64			i64Swtrc_Delay;

	// 起動時の情報取得
	ulIf_Clock = get_lpdoptinfodata(3);	// LPD I/Fクロック
	ulIf_Tck   = get_lpdoptinfodata(4);	// デバッグモジュールクロック

	// Delay値 = DBG_CTRLP情報分の通信時間＋BFW処理時間(AspRun設定～送信開始設定まで)
	u64If_Clock_Delay  = ((DELAYTIME_DBGCTRLP_COMLENGTH * 1000000) / ulIf_Clock) + DELAYTIME_BFWPROC;
	// ソフトウェアトレースタイムスタンプ値追加補正値 = TCU_SFT～LDU転送時間
	u64Dcu_Clock_Delay = ((DELAYTIME_SFTTOLDU_CYCLE * 1000000) / ulIf_Tck);

	// ns単位->FPGA(120MHz)カウント単位の変換
	i64Etc_Delay = (INT64)nsec_to_clk(u64If_Clock_Delay, OFF);
	i64Swtrc_Delay = (INT64)nsec_to_clk((u64If_Clock_Delay+u64Dcu_Clock_Delay), OFF);

	// 記録データタイムスタンプの補正値設定
	DO_SetClkCalibration(RF_ASPTYPE_CAN_CH0_RX, -i64Etc_Delay);
	DO_SetClkCalibration(RF_ASPTYPE_CAN_CH1_RX, -i64Etc_Delay);
	DO_SetClkCalibration(RF_ASPTYPE_STRACE, -i64Swtrc_Delay);

	// Delay値はFPGA(120MHz)カウント単位で設定
	return (DWORD)i64Etc_Delay;
}


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
	unsigned long	ulIf_Clock;
	unsigned long	ulIf_Tck;
	unsigned long	ulIf_Pin;
	UINT64			u64If_Clock_Delay;
	UINT64			u64FPGA_Clock_Delay;
	UINT64			u64Swtrc_Length;
	UINT64			u64StartEvent_chk;
	UINT64			u64EndEvent_chk;
	RFW_SWTRC_DATA	SwTrcData;
	BYTE			byChNo;

	// 起動時のLPD I/F情報取得
	ulIf_Pin   = get_lpdoptinfodata(0);	// LPD I/F
	ulIf_Clock = get_lpdoptinfodata(3);	// LPD I/Fクロック
	ulIf_Tck   = get_lpdoptinfodata(4);	// DCUクロック
	// ワーニング対策のため初期化
	u64If_Clock_Delay   = 0;
	u64FPGA_Clock_Delay = 0;
	u64Swtrc_Length     = 0;
	u64StartEvent_chk   = 0;
	u64EndEvent_chk     = 0;
	byChNo = 0;
	memset((void*)&SwTrcData, 0, sizeof(RFW_SWTRC_DATA));

	// 一旦補正なしの状態で初期化
	memcpy((void*)pAspPerformanceData, &AspPerformanceDataRaw, sizeof(AspPerformanceDataRaw));

	u64StartEvent_chk = (1<<RF_ASPE2TRGCH_EXTRGOUTCH0) | (1<<RF_ASPE2TRGCH_EXTRGOUTCH1) | (1<<RF_ASPE2TRGCH_COMUMONCH0_MATCH) | (1<<RF_ASPE2TRGCH_COMUMONCH1_MATCH);
	u64EndEvent_chk   = (1<<RF_ASPE2TRGCH_SWTRCCH0) | (1<<RF_ASPE2TRGCH_SWTRCCH1) | (1<<RF_ASPE2TRGCH_SWTRCCH2) | (1<<RF_ASPE2TRGCH_SWTRCCH3);
	if ((u64StartEvent & u64StartEvent_chk) && (u64EndEvent & u64EndEvent_chk)) {
	// 開始条件が外部トリガ/通信モニタでかつ終了条件がソフトウェアトレースの場合、種別ごとに補正する
		// Ch番号取得
		switch (u64EndEvent & u64EndEvent_chk) {
			case (1<<RF_ASPE2TRGCH_SWTRCCH0):
				byChNo = 0;
				break;
			case (1<<RF_ASPE2TRGCH_SWTRCCH1):
				byChNo = 1;
				break;
			case (1<<RF_ASPE2TRGCH_SWTRCCH2):
				byChNo = 2;
				break;
			case (1<<RF_ASPE2TRGCH_SWTRCCH3):
			default:
				byChNo = 3;
				break;
		}
		GetSwTrcData(byChNo, &SwTrcData);
		// 注意：実行命令が[DBTAG(PC値付き)],[DBPUSH(PC値付き)],[DBPUSH(レジスタ複数指定)]の場合、正確な補正ができない
		switch (SwTrcData.byTraceType) {
			case RF_ASPSWTRC_PC:
			case RF_ASPSWTRC_REG:
				u64Swtrc_Length = DELAYTIME_SWTRC_DBPUSH_DBCPLENGTH;
				break;
			case RF_ASPSWTRC_TAG:
			default:
				u64Swtrc_Length = DELAYTIME_SWTRC_DBTAGLENGTH;
				break;
		}
		// デバイスのTCU_SFT～LDU転送時間)/LPD通信時間/FPGA内処理時間の分を補正
		if (ulIf_Pin == 3) {
		// LPD-4Pin
			// FPGA内部補正時間
			switch (ulIf_Clock) {
			case LPD4CLK_20MHZ:
				u64FPGA_Clock_Delay = DELAYTIME_SWTRC_LPD4_FPGA_INT80CYCLE;
				break;
			case LPD4CLK_25MHZ:
				u64FPGA_Clock_Delay = DELAYTIME_SWTRC_LPD4_FPGA_INT100CYCLE;
				break;
			default:
				u64FPGA_Clock_Delay = DELAYTIME_SWTRC_LPD4_FPGA_INT66CYCLE;
				break;
			}
			u64FPGA_Clock_Delay = (((DELAYTIME_SWTRC_LPD4_FPGA_LPDCYCLE * 1000000) / ulIf_Clock) >> 1) + u64FPGA_Clock_Delay + DELAYTIME_SWTRC_FPGA_COMCYCLE;
		} else {
		// LPD-1Pin
			// FPGA内部補正時間
			u64FPGA_Clock_Delay = DELAYTIME_SWTRC_LPD1_FPGA_INT66CYCLE + DELAYTIME_SWTRC_FPGA_COMCYCLE;
		}
		// LPD通信時間
		u64If_Clock_Delay = ((u64Swtrc_Length * 1000000) / ulIf_Clock) >> 1;
		// LPD通信時間＋FPGA内部補正時間の合計値
		u64If_Clock_Delay = u64If_Clock_Delay + u64FPGA_Clock_Delay;
		// TCU_SFT～LDU転送時間
		ulIf_Tck = (DELAYTIME_SFTTOLDU_CYCLE * 1000000) / ulIf_Tck;

		// 計測結果を補正
		// パラメータ単位で補正値より計測結果が短い場合は0とする
		if (AspPerformanceDataRaw.u64MaxCount <= (u64If_Clock_Delay+ulIf_Tck)) {
			pAspPerformanceData->u64MaxCount = 0;
		} else {
			pAspPerformanceData->u64MaxCount = AspPerformanceDataRaw.u64MaxCount - (u64If_Clock_Delay+ulIf_Tck);
		}
		if (AspPerformanceDataRaw.u64MinCount <= (u64If_Clock_Delay+ulIf_Tck)) {
			pAspPerformanceData->u64MinCount = 0;
		} else {
			pAspPerformanceData->u64MinCount = AspPerformanceDataRaw.u64MinCount - (u64If_Clock_Delay+ulIf_Tck);
		}
		pAspPerformanceData->dwPassCount = AspPerformanceDataRaw.dwPassCount;
		if (AspPerformanceDataRaw.u64SumCount <= ((u64If_Clock_Delay+ulIf_Tck) * AspPerformanceDataRaw.dwPassCount)) {
			pAspPerformanceData->u64SumCount = 0;
		} else {
			pAspPerformanceData->u64SumCount = AspPerformanceDataRaw.u64SumCount - ((u64If_Clock_Delay+ulIf_Tck) * AspPerformanceDataRaw.dwPassCount);
		}
	}
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
	unsigned long	ulIf_Clock;
	unsigned long	ulIf_Tck;
	unsigned long	ulIf_Pin;
	UINT64			u64If_Clock_Delay;
	UINT64			u64FPGA_Clock_Delay;
	UINT64			u64Swtrc_Length;
	UINT64			u64StartEvent_chk;
	UINT64			u64EndEvent_chk;
	RFW_SWTRC_DATA	SwTrcData;
	BYTE			byChNo;

	// 起動時のLPD I/F情報取得
	ulIf_Pin = get_lpdoptinfodata(0);	// LPD I/F
	ulIf_Clock = get_lpdoptinfodata(3);	// LPD I/Fクロック
	ulIf_Tck   = get_lpdoptinfodata(4);	// DCUクロック
	// ワーニング対策のため初期化
	u64If_Clock_Delay   = 0;
	u64FPGA_Clock_Delay = 0;
	u64Swtrc_Length     = 0;
	u64StartEvent_chk   = 0;
	u64EndEvent_chk     = 0;
	byChNo = 0;
	memset((void*)&SwTrcData, 0, sizeof(RFW_SWTRC_DATA));

	// 一旦補正なしの状態で初期化
	*pu64Threshold = u64ThresholdRaw;

	u64StartEvent_chk = (1<<RF_ASPE2TRGCH_EXTRGOUTCH0) | (1<<RF_ASPE2TRGCH_EXTRGOUTCH1) | (1<<RF_ASPE2TRGCH_COMUMONCH0_MATCH) | (1<<RF_ASPE2TRGCH_COMUMONCH1_MATCH);
	u64EndEvent_chk   = (1<<RF_ASPE2TRGCH_SWTRCCH0) | (1<<RF_ASPE2TRGCH_SWTRCCH1) | (1<<RF_ASPE2TRGCH_SWTRCCH2) | (1<<RF_ASPE2TRGCH_SWTRCCH3);
	if ((u64StartEvent & u64StartEvent_chk) && (u64EndEvent & u64EndEvent_chk)) {
	// 開始条件が外部トリガ/通信モニタでかつ終了条件がソフトウェアトレースの場合、ソフトウエアトレース種別ごとに補正する
		// Ch番号取得
		switch (u64EndEvent & u64EndEvent_chk) {
			case (1<<RF_ASPE2TRGCH_SWTRCCH0):
				byChNo = 0;
				break;
			case (1<<RF_ASPE2TRGCH_SWTRCCH1):
				byChNo = 1;
				break;
			case (1<<RF_ASPE2TRGCH_SWTRCCH2):
				byChNo = 2;
				break;
			case (1<<RF_ASPE2TRGCH_SWTRCCH3):
			default:
				byChNo = 3;
				break;
		}
		GetSwTrcData(byChNo, &SwTrcData);
		// 注意：実行命令が[DBTAG(PC値付き)],[DBPUSH(PC値付き)],[DBPUSH(レジスタ複数指定)]の場合、正確な補正ができない
		switch (SwTrcData.byTraceType) {
			case RF_ASPSWTRC_PC:
			case RF_ASPSWTRC_REG:
				u64Swtrc_Length = DELAYTIME_SWTRC_DBPUSH_DBCPLENGTH;
				break;
			case RF_ASPSWTRC_TAG:
			default:
				u64Swtrc_Length = DELAYTIME_SWTRC_DBTAGLENGTH;
				break;
		}
		// デバイスのTCU_SFT～LDU転送時間)/LPD通信時間/FPGA内処理時間の分を補正
		if (ulIf_Pin == 3) {
		// LPD-4Pin
			// FPGA内部補正時間
			switch (ulIf_Clock) {
			case LPD4CLK_20MHZ:
				u64FPGA_Clock_Delay = DELAYTIME_SWTRC_LPD4_FPGA_INT80CYCLE;
				break;
			case LPD4CLK_25MHZ:
				u64FPGA_Clock_Delay = DELAYTIME_SWTRC_LPD4_FPGA_INT100CYCLE;
				break;
			default:
				u64FPGA_Clock_Delay = DELAYTIME_SWTRC_LPD4_FPGA_INT66CYCLE;
				break;
			}
			u64FPGA_Clock_Delay = (((DELAYTIME_SWTRC_LPD4_FPGA_LPDCYCLE * 1000000) / ulIf_Clock) >> 1) + u64FPGA_Clock_Delay + DELAYTIME_SWTRC_FPGA_COMCYCLE;
		} else {
		// LPD-1Pin
			// FPGA内部補正時間
			u64FPGA_Clock_Delay = DELAYTIME_SWTRC_LPD1_FPGA_INT66CYCLE + DELAYTIME_SWTRC_FPGA_COMCYCLE;
		}
		// LPD通信時間
		u64If_Clock_Delay = ((u64Swtrc_Length * 1000000) / ulIf_Clock) >> 1;
		// LPD通信時間＋FPGA内部補正時間の合計値
		u64If_Clock_Delay = u64If_Clock_Delay + u64FPGA_Clock_Delay;
		// TCU_SFT～LDU転送時間
		ulIf_Tck = (DELAYTIME_SFTTOLDU_CYCLE * 1000000) / ulIf_Tck;

		// 閾値を補正(48bitで丸める)
		*pu64Threshold = (u64ThresholdRaw + (u64If_Clock_Delay+ulIf_Tck)) & 0x0000FFFFFFFFFFFF;
	}
}



//=============================================================================
/**
* RFWAsp_GetAvailableTimeのエラーチェック処理
* @param dwType 記録データ種別
* @retval RFWエラー(RFWERR_N_BASP_SWT_NOREC/RFWERR_OK)
*/
//=============================================================================
RFWERR checkGetAvailableTime(DWORD dwType) 
{
	// ソフトウェア・トレース(LPD)は記録できない状態を継続中かをチェック
	if ((IsSwTrcNotRecord() != FALSE) && (dwType == RF_ASPTYPE_STRACE)) {
		return RFWERR_N_BASP_SWT_NOREC;
	} else {
		return RFWERR_OK;
	}
}

//=============================================================================
/**
* RFWAsp_GetSampleのエラーチェック処理
* @param dwType 記録データ種別
* @param dwKeyType 0:時間指定 1:変化点数指定
* @param u64Start 開始時間/開始変化点
* @param u64End 終了時間/終了変化点
* @retval RFWエラー(RFWERR_N_BASP_SWT_NOREC/RFWERR_OK)
*/
//=============================================================================
RFWERR checkGetSample(DWORD dwType, DWORD dwKeyType, UINT64 u64Start,UINT64 u64End)
{
	// ソフトウェア・トレース(LPD)は記録できない状態を継続中かをチェック
	if ((IsSwTrcNotRecord() != FALSE) && (dwType == RF_ASPTYPE_STRACE)) {
		return RFWERR_N_BASP_SWT_NOREC;
	}
	else {
		return RFWERR_OK;
	}
}

//=============================================================================
/**
* RFWAsp_SampleDataSizeのエラーチェック処理
* @param dwType 記録データ種別
* @param dwKeyType 0:時間指定 1:変化点数指定
* @param u64Start 開始時間/開始変化点
* @param u64End 終了時間/終了変化点
* @retval RFWエラー(RFWERR_N_BASP_SWT_NOREC/RFWERR_OK)
*/
//=============================================================================
RFWERR checkGetSampleDataSize(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End)
{
	// ソフトウェア・トレース(LPD)は記録できない状態を継続中かをチェック
	if ((IsSwTrcNotRecord() != FALSE) && (dwType == RF_ASPTYPE_STRACE)) {
		return RFWERR_N_BASP_SWT_NOREC;
	}
	else {
		return RFWERR_OK;
	}
}



//=============================================================================
/**
* RFWAsp_GetSampleRealTimeのエラーチェック処理
* @param dwType 記録データ種別
* @param dwKeyType 0:時間指定 1:変化点数指定
* @param u64Back 現在の時間から遡る時間/変化点
* @param pData 格納領域
* @param dwSize 格納領域のサイズ
* @retval RFWエラー(RFWERR_N_BASP_SWT_NOREC/RFWERR_OK)
*/
//=============================================================================
RFWERR checkGetSampleRealTime(DWORD dwType, DWORD dwKeyType, UINT64 u64Back)
// ソフトウェア・トレース(LPD)は記録できない状態を継続中かをチェック
{
	if ((IsSwTrcNotRecord() != FALSE) && (dwType == RF_ASPTYPE_STRACE)) {
		return RFWERR_N_BASP_SWT_NOREC;
	} else {
		return RFWERR_OK;
	}
}

//=============================================================================
/**
* RFWAsp_SetClkCalibrationのエラーチェック
 * @param dwType 記録種別
 * @param i64Calib 補正値
* @retval RFWエラー(RFWERR_N_BASP_SWT_NOREC/RFWERR_OK)
*/
//=============================================================================
RFWERR checkSetClkCalibration(DWORD dwType, INT64 i64Calib)
{
// ソフトウェア・トレース(LPD)は記録できない状態を継続中かをチェック
	if ((IsSwTrcNotRecord() != FALSE) && (dwType == RF_ASPTYPE_STRACE)) {
		return RFWERR_N_BASP_SWT_NOREC;
	} else {
		return RFWERR_OK;
	}
}

