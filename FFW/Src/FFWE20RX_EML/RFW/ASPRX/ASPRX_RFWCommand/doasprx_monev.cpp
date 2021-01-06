///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasprl_monev.cpp
 * @brief
 * @author REL M.Yamamoto, PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・新規作成 M.Yamamoto
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/

#include <stdlib.h>
#include "rfwasprx_monev.h"
#include "doasprx_monev.h"
#include "emudef.h"
#include "rfwasp_RX.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"
// RevRxE2No171004-001 Append Start
#include "mcu_brk.h"
#include "ffwmcu_srm.h"
#include "ffwmcu_mcu.h"
#include "ffwmcu_brk.h"
#include "domcu_brk.h"
// RevRxE2No171004-001 Append End

// ファイル内static変数の宣言

/* 共通部 */
/* モニタイベント条件 管理テーブル(両ブレーク方式共通) */
static RFW_ASPMONEVCOND_TABLE s_MonEvTable[MONEV_EV_MAX+1];	 // 削除用に+1する

/* モニタイベントのE2トリガ条件管理テーブル */
static RFW_MONEV_FACTOR_TABLE s_MonEvFactTable[MONEV_TRG_MAX+1];

/* 品種依存部(RL78) */
/* モニタイベント設定アドレスへのテンポラリブレーク付き実行フラグ */
static BOOL s_bTempBrkRunModeFlg = FALSE;

// ファイル内static関数の宣言
static void GenMonEvCode(DWORD *pdwMonEvSwBrkInfo, WORD *pwMonEvHwBrkInfo);


///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* モニタイベント条件の設定
* @param byChNo
* @param pAspMonitorEvent
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
* @retval RFWERR_N_EV_OVER  イベント本数よりも多いイベントが設定された
*/
//=============================================================================
DWORD DO_SetMonitorEvent(BYTE byEventNo, const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent)
{
	DWORD dwRet = RFWERR_OK;
	RFW_ASPCONF_DATA AspConfData;

	//BOOL bSetMonitorEna = IsMonitorEventEna();		// RevRxE2No171004-001 Delete Line

	ProtInit_ASP();

	/* ユーザプログラム実行中かチェック */
	if (IsStatRun()) {
		dwRet = RFWERR_N_BMCU_RUN;
	}

	/* モニタイベントが無効状態かチェック */
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if (!(AspConfData.dwFunction3 & RF_ASPCONF_MON_EV)) {
			dwRet = RFWERR_N_BASP_MONEVOFF;
		}
	}

	/* ターゲットMCUのファミリ依存になる部分のエラーチェック */
	if (dwRet == RFWERR_OK) {
		dwRet = ChkMonEvData(byEventNo, pAspMonitorEvent);
	}

	if (dwRet == RFWERR_OK) {
		dwRet = SetMonEvData(byEventNo, pAspMonitorEvent);
	}

	// RevRxE2No171004-001 Append Start
	/* 通過ポイント使用許可設定 */
	if (dwRet == RFWERR_OK) {
		dwRet = SetSrModeMonEvUse();
	}
	// RevRxE2No171004-001 Append End

	// RXは、通過ポイント用モニタコードを指定ルーチン機能側で作成するためここでは不要
	// RevRxE2No171004-001 Delete Start
	//if (dwRet == RFWERR_OK) {
	//	if (!bSetMonitorEna && IsMonitorEventEna()) {
	//	//	UpdateMonEvMonitorCode();
	//	//	K0RMQB2_UpdateMonitor();
	//	}
	//}
	// RevRxE2No171004-001 Delete End

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* モニタイベント条件の参照
* @param byChNo
* @param pAspMonitorEvent
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_MONEV_NOTSET  指定したモニタイベント条件番号が設定されていない
*/
//=============================================================================
DWORD DO_GetMonitorEvent(BYTE byEventNo, RFW_ASPMONEVCOND_DATA *const pAspMonitorEvent)
{
	RFWERR dwRet = RFWERR_OK;
	RFW_ASPCONF_DATA AspConfData = { 0 };
	RFW_ASPMONEVCOND_TABLE AspMonEvTable = { 0 };

	ProtInit_ASP();

	/* モニタイベントが無効状態かチェック */
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if (!(AspConfData.dwFunction3 & RF_ASPCONF_MON_EV)) {
			dwRet = RFWERR_N_BASP_MONEVOFF;
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = GetMonEvData(byEventNo, &AspMonEvTable);
	}

	if (dwRet == RFWERR_OK) {
		memcpy(pAspMonitorEvent, &AspMonEvTable.Cond, sizeof(RFW_ASPMONEVCOND_DATA));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* モニタイベント条件の消去
* @param byChNo
* @param pAspMonitorEvent
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_MONEV_NOTSET  指定したモニタイベント条件番号が設定されていない
*/
//=============================================================================
DWORD DO_ClrMonitorEvent(BYTE byEventNo)
{
	RFW_ASPCONF_DATA AspConfData = { 0 };

	RFWERR dwRet = RFWERR_OK;

	ProtInit_ASP();

	/* ユーザプログラム実行中かチェック */
	if (IsStatRun()) {
		dwRet = RFWERR_N_BMCU_RUN;
	}

	/* モニタイベントが無効状態かチェック */
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if (!(AspConfData.dwFunction3 & RF_ASPCONF_MON_EV)) {
			dwRet = RFWERR_N_BASP_MONEVOFF;
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = ClrMonEvData(byEventNo);
	}
	// RevRxE2No171004-001 Delete Start
	//if (dwRet == RFWERR_OK) {
	//	/* この消去によって設定数0になった場合は、モニタプログラムに遷移しないよう更新 */
	//	if (!IsMonitorEventEna()) {
	//	//	K0RMQB2_UpdateMonitor();
	//	}
	//}
	// RevRxE2No171004-001 Delete End
	ProtEnd_ASP();

	return dwRet;

}

//=============================================================================
/**
* モニタイベントのE2トリガ条件設定
* @param byChNo
* @param byData
* @param byMask
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
*/
//=============================================================================
DWORD DO_SetMonitorEventFactor(BYTE byChNo, BYTE byData, BYTE byMask)
{
	DWORD dwEventLinkType = 0;
	RFW_ASPCONF_DATA AspConfData = { 0 };
	RFWERR dwRet = RFWERR_OK;

	ProtInit_ASP();

	byMask;	// Warning対策

	/* ユーザプログラム実行中かチェック */
	if (IsStatRun()) {
		dwRet = RFWERR_N_BMCU_RUN;
	}

	/* モニタイベントが無効状態かチェック */
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if (!(AspConfData.dwFunction3 & RF_ASPCONF_MON_EV)) {
			dwRet = RFWERR_N_BASP_MONEVOFF;
		}
	}

	if (dwRet == RFWERR_OK) {

		s_MonEvFactTable[byChNo].bMonEvChSw = MONEV_TRG_SW_ON;

		if (byChNo == MONEV_TRG_CH6) {
			s_MonEvFactTable[byChNo].byMonEvCmpData = MONEV_CH6_ON;	/* ch6の場合は01でON、00でOFFとする */
			dwEventLinkType = RFW_EVENTLINK_LEVEL;	/* ch6のみレベル型 */
		}
		else {
			s_MonEvFactTable[byChNo].byMonEvCmpData = byData;			/* ch0～5の場合は設定値を保持 */
			dwEventLinkType = RFW_EVENTLINK_PULSE;	/* ch0～5はパルス型 */
		}
		s_MonEvFactTable[byChNo].byMonEvMaskData = 0;


		SetE2TrgTable(MONEV_TRG_BASE_CH0 + byChNo, dwEventLinkType);
	}
	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
* モニタイベントのE2トリガ条件参照
* @param byChNo
* @param pbyData
* @param pbyMask
* @retval RFWERR_OK  正常動作
*/
//=============================================================================
DWORD DO_GetMonitorEventFactor(BYTE byChNo, BYTE *pbyData, BYTE *pbyMask)
{
	RFW_ASPCONF_DATA AspConfData = { 0 };
	RFWERR dwRet = RFWERR_OK;

	ProtInit_ASP();

	/* モニタイベントが無効状態かチェック */
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if (!(AspConfData.dwFunction3 & RF_ASPCONF_MON_EV)) {
			dwRet = RFWERR_N_BASP_MONEVOFF;
		}
	}

	/* 対象のE2トリガが設定されているかチェック */
	if (dwRet == RFWERR_OK) {
		if (s_MonEvFactTable[byChNo].bMonEvChSw == MONEV_TRG_SW_OFF) {
			dwRet = RFWERR_N_BASP_CHNOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		*pbyData = s_MonEvFactTable[byChNo].byMonEvCmpData;
		*pbyMask = 0;
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* モニタイベントのE2トリガ条件削除
* @param byChNo
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
*/
//=============================================================================
DWORD DO_ClrMonitorEventFactor(BYTE byChNo)
{
	RFW_ASPCONF_DATA AspConfData = { 0 };
	RFWERR dwRet = RFWERR_OK;

	ProtInit_ASP();

	/* ユーザプログラム実行中かチェック */
	if (IsStatRun()) {
		dwRet = RFWERR_N_BMCU_RUN;
	}

	/* モニタイベントが無効状態かチェック */
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if (!(AspConfData.dwFunction3 & RF_ASPCONF_MON_EV)) {
			dwRet = RFWERR_N_BASP_MONEVOFF;
		}
	}

	/* 対象のE2トリガが設定されているかチェック */
	if (dwRet == RFWERR_OK) {
		if (s_MonEvFactTable[byChNo].bMonEvChSw == MONEV_TRG_SW_OFF) {
			dwRet = RFWERR_N_BASP_CHNOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		s_MonEvFactTable[byChNo].byMonEvCmpData = 0;
		s_MonEvFactTable[byChNo].byMonEvMaskData = 0;
		s_MonEvFactTable[byChNo].bMonEvChSw = MONEV_TRG_SW_OFF;
		ClrE2TrgTable(MONEV_TRG_BASE_CH0 + byChNo);
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
* モニタイベントの結果データ取得
* @param byDataType	結果データタイプ
* @param byEventNo	モニタイベント番号
* @param pu64Data1	結果データ1
* @param pu64Data2	結果データ2(予約)
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
*/
//=============================================================================
DWORD DO_GetMonitorEventData(BYTE byDataType, BYTE byEventNo, UINT64 *const pu64Data1, UINT64 *const pu64Data2)
{
	RFWERR dwRet = RFWERR_OK;

	ProtInit_ASP();

	/* ユーザプログラム実行中かチェック(PCストレージ対応時に条件追加必要) */
	if (IsStatRun()) {
		dwRet = RFWERR_N_BMCU_RUN;
	}

	byDataType;	// Warning回避
	byEventNo;	// Warning回避

	/* 予約変数には0固定を入れる */
	*pu64Data1 = 0;
	*pu64Data2 = 0;

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
* モニタイベント関連の管理変数を初期化
* @param byDetectType
* @param pbyMaxTableNum
* @param pTable
* @retval なし
*/
//=============================================================================
void InitAspMonEv(void)
{
	memset(s_MonEvTable, NOSET, (sizeof(RFW_ASPMONEVCOND_TABLE) * (MONEV_EV_MAX + 1)));
	memset(s_MonEvFactTable, NOSET, (sizeof(RFW_MONEV_FACTOR_TABLE) * MONEV_TRG_MAX));
	s_bTempBrkRunModeFlg = FALSE;
}

//=============================================================================
/**
* モニタイベント関連の設定状態を全削除する(ブレーク設定含む)
* @param なし
* @retval なし
*/
//=============================================================================
void ClrAllMonData(void)
{
	int i;
	FFW_SRM_DATA* pSrmData;		// RevRxE2No171004-001 Append Line

	for (i = 0; i < MONEV_EV_MAX + 1; i++) {
		/* Clr処理で管理テーブルを前詰めにしているので、常に先頭を削除 */
		if (s_MonEvTable[i].byChNo != NOSET) {			// RevRL78No170420-001-02
			ClrMonEvData(s_MonEvTable[i].byChNo);		// RevRL78No170420-001-02
		}
	}
	// モニタを更新
	//K0RMQB2_UpdateMonitor();

	// RevRxE2No171004-001 Append Start
	pSrmData = GetSrmData();	// 指定ルーチン実行情報格納構造体情報取得

	if ((pSrmData->dwSrMode & SRM_MON_POINT_FUNC_BIT) == SRM_MON_POINT_FUNC_BIT) {	// 通過ポイント使用許可
		pSrmData->dwSrMode = INIT_SRM_MODE;
		pSrmData->dwmadrRamWorkAreaStartAddr = INIT_WORKRAM_ADDR;
		FFWMCUCmd_SetSRM(pSrmData);		// 指定ルーチン実行機能を初期化
	}
	// RevRxE2No171004-001 Append End
}


//=============================================================================
/**
* 指定のブレーク方式におけるモニタイベント条件の設定数を取得する
* @param byDetectType
* @retval byRetNum
*/
//=============================================================================
BYTE GetMonEventSetNum(BYTE byDetectType)
{
	int i;
	BYTE byRetNum = 0;

	for (i = 0; i < MONEV_EV_MAX; i++) {
		if (s_MonEvTable[i].Cond.byDetectType == byDetectType) {
			if (s_MonEvTable[i].byChNo != NOSET) {
				byRetNum++;
			}
		}
	}
	return byRetNum;
}
//=============================================================================
/**
* モニタイベント有効/無効の参照
* @param なし
* @retval TRUE  モニタイベント有効
* @retval FALSE モニタイベント無効
*/
//=============================================================================
BOOL IsMonitorEventEna(void)
{

	BYTE byCount = 0;

	byCount += GetMonEventSetNum(RF_ASPMONEV_TYPE_SWBRK);
//	byCount += GetMonEventSetNum(RF_ASPMONEV_TYPE_HWBRK);		// RevRxE2No171004-001 Delete Line

	/* S/Wブレーク方式とH/Wブレーク方式で、1つもモニタイベントが設定されていなければ、FALSE */
	if (byCount == 0) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}


//=============================================================================
/**
* モニタイベント設定値保持とEXECへのブレーク設定
* @param byEventNo
* @param pAspMonitorEvent
* @retval RFWエラー
*/
//=============================================================================
RFWERR SetMonEvData(BYTE byEventNo, const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent)
{
	RFWERR dwRet = RFWERR_OK;

	// RevRxE2No171004-001 Modify Start
	/* 上書きの場合を考慮し、指定のイベント番号で設定済みのものを削除する */
	ClrMonEvData(byEventNo);

	dwRet = FFWMCUCmd_SetPB(PB_SET, (DWORD)pAspMonitorEvent->u64EventAddr);
	dwRet = ChgFFWErrToRFWErr(dwRet);
	// RevRxE2No171004-001 Modify End

	if (dwRet == RFWERR_OK) {
		// RevRL78No170420-001-02 top
		s_MonEvTable[monev_tblno(byEventNo)].byChNo = byEventNo;
		s_MonEvTable[monev_tblno(byEventNo)].wExecBrkNo = 0;		//	RX用の暫定処置
		memcpy(&s_MonEvTable[monev_tblno(byEventNo)].Cond, pAspMonitorEvent, sizeof(RFW_ASPMONEVCOND_DATA));
		// RevRL78No170420-001-02 Bottom
	}

	return dwRet;

}


// RevRxE2No171004-001 Append Start
//=============================================================================
/**
* FFWエラーコードをRFWエラーコードに変換
* @param FFWエラーコード
* @retval RFWエラーコード
*/
//=============================================================================
RFWERR ChgFFWErrToRFWErr(DWORD dwFFWErr)
{
	RFWERR dwRFWErr;

	switch (dwFFWErr) {
	case FFWERR_OK:
		dwRFWErr = RFWERR_OK;					// 正常終了
		break;
	case FFWERR_FFW_ARG:						// 引数エラー
		dwRFWErr = RFWERR_N_RFW_ARG;
		break;
	case FFWERR_PBPOINT_OVER:					// ソフトウェアブレークの設定ポイントが最大設定点数(256点)を超えている
		dwRFWErr = RFWERR_N_SBPOINT_OVER;
		break;
	case FFWERR_PBADDR_NOTSET:					// ソフトウェアブレークが設定されていない(解除時のみ送信)
		dwRFWErr = RFWERR_N_SBADDR_NOTSET;
		break;
	case FFWERR_PBAREA_OUT:						// 指定領域はソフトウェアブレークポイント設定不可領域である
		dwRFWErr = RFWERR_N_SBAREA_OUT;
		break;
	case FFWERR_PBADDR_OVERLAPP:				// 通過ポイントが設定されているアドレスへS/Wブレークポイントを設定することはできません
		dwRFWErr = RFWERR_N_BASP_MONEV_OVERLAPP;
		break;
	case FFWERR_PB_ROMAREA_FLASHDEBUG:			// フラッシュ書き替えデバッグ中にMCU内部ROM領域へのソフトウェアブレークは設定できない
		dwRFWErr = RFWERR_N_SB_ROMAREA_FLASHDEBUG;
		break;
	case FFWERR_PB_ROMAREA_MCURUN:				// ユーザプログラム実行中にMCU内蔵ROM領域へのソフトウェアブレークは設定/解除できない
		dwRFWErr = RFWERR_N_SB_ROMAREA_MCURUN;
		break;
	case FFWERR_CLKCHG_DMB_CLR:					// メモリアクセス中のクロック切り替えなどによる通信エラーが発生したためリカバリした(Warning)
		dwRFWErr = RFWERR_W_CLKCHG_DMB_CLR;
		break;
	case FFWERR_BTARGET_NOCONNECT:				// ターゲットシステムと接続されていない
		dwRFWErr = RFWERR_N_BTARGET_NOCONNECT;
		break;
	case FFWERR_BTARGET_POWER:					// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない
		dwRFWErr = RFWERR_N_BTARGET_POWER;
		break;
	case FFWERR_BMCU_RESET:						// MCUがリセット状態のためコマンド処理を実行できない
		dwRFWErr = RFWERR_N_BMCU_RESET;
		break;
	case FFWERR_BMCU_STANBY:					// MCUのスタンバイモード中で内部クロックが停止状態である
		dwRFWErr = RFWERR_N_BMCU_STANBY;
		break;
	case FFWERR_BMCU_SLEEP:						// MCUのスリープモード中で内部クロックが停止状態である
		dwRFWErr = RFWERR_N_BMCU_SLEEP;
		break;
	case FFWERR_BMCU_AUTH_DIS:					// MCU内部リセットが発生したため、コマンド処理を実行できない
		dwRFWErr = RFWERR_N_BMCU_AUTH_DIS;
		break;
	case FFWERR_EL3_BMCU_AUTH_DIS:				// 認証切れが発生したため、コマンド処理が実行できない
		dwRFWErr = RFWERR_SU_BMCU_AUTH_DIS;
		break;
	case FFWERR_AMCU_RESET:						// BFW内でタイムアウト発生。MCUがリセット状態である
		dwRFWErr = RFWERR_SU_AMCU_RESET;
		break;
	case FFWERR_AMCU_STANBY:					// タイムアウトが発生した。MCUのスタンバイモード中で内部クロックが停止状態である
		dwRFWErr = RFWERR_SU_AMCU_STANBY;
		break;
	case FFWERR_AMCU_SLEEP:						// タイムアウトが発生した。MCUのスリープモード中で内部クロックが停止状態である
		dwRFWErr = RFWERR_SU_AMCU_SLEEP;
		break;
	case FFWERR_ATARGET_NOCONNECT:				// タイムアウトが発生した。ターゲットシステムと接続されていない
		dwRFWErr = RFWERR_SU_ATARGET_NOCONNECT;
		break;
	case FFWERR_ATARGET_POWER:					// タイムアウトが発生した。ターゲットシステム上のMCU供給電源がOFF状態である
		dwRFWErr = RFWERR_SU_ATARGET_POWER;
		break;
	case FFWERR_EL3_AMCU_AUTH_DIS:				// タイムアウトが発生した。認証切れが発生したため、コマンド処理が実行できなかった
		dwRFWErr = RFWERR_SU_AMCU_AUTH_DIS;
		break;
	case FFWERR_BFW_TIMEOUT:					// BFW処理でタイムアウトが発生した
		dwRFWErr = RFWERR_SU_BFW_TIMEOUT;
		break;
	case FFWERR_MEMORY_NOT_SECURED:				// PC上メモリを確保できなかった
		dwRFWErr = RFWERR_N_MEMORY_NOT_SECURED;
		break;
	case FFWERR_RTTMODE_UNSUPRT:				// サポートしていないトレースモードである
		dwRFWErr = RFWERR_N_RTTMODE_UNSUPRT;
		break;
	case FFWERR_RTT_UNSUPPORT:					// トレース機能はサポートしていない
		dwRFWErr = RFWERR_N_RTTSTAT_DIS;
		break;
	case FFWERR_N_UNSUPPORT:					// 指定の機能はサポートしていない
		dwRFWErr = RFWERR_N_UNSUPPORT;
		break;
	case FFWERR_BMCU_RUN:						// ユーザプログラム実行中のためコマンド処理を実行できない
		dwRFWErr = RFWERR_N_BMCU_RUN;
		break;
	case FFWERR_RAM_WAIT_TMP_CHANGE:			// RAMのウェイト設定を一時的に変更してコマンドを実行した(Warning)
		dwRFWErr = RFWERR_W_RAM_WAIT_TMP_CHANGE;
		break;
	case FFWERR_ROM_WAIT_TMP_CHANGE:			// ROMのウェイト設定を一時的に変更してコマンドを実行した(Warning)
		dwRFWErr = RFWERR_W_ROM_WAIT_TMP_CHANGE;
		break;
	case FFWERR_SRM_USE_WORKRAM:				// 指定ルーチン実行機能用ワークRAMを別の用途で使用している
		dwRFWErr = RFWERR_N_SRM_USE_WORKRAM;
		break;
	case FFWERR_COM:							// 致命的な通信異常が発生した
		dwRFWErr = RFWERR_D_COM;
		break;
	default:									// エラーメッセージ未登録のエラーが発生しました
		dwRFWErr = RFWERR_N_NOMESSAGE_ERROR;
		break;
	}

	return dwRFWErr;
}


//=============================================================================
/**
* 指定ルーチンの動作モードとして通過ポイント使用許可を設定
* @param なし
* @retval RFWエラーコード
*/
//=============================================================================
RFWERR SetSrModeMonEvUse(void)
{
	FFW_SRM_DATA* pSrmData;
	FFWRX_MCUAREA_DATA* pMcuArea;
	RFWERR dwRet = RFWERR_OK;

	pSrmData = GetSrmData();	// 指定ルーチン実行情報格納構造体情報取得

	if ((pSrmData->dwSrMode & SRM_MON_POINT_FUNC_BIT) == SRM_MON_POINT_FUNC_BIT) {	// 通過ポイント使用許可
		return dwRet;
	}

	pMcuArea = GetMcuAreaDataRX();
	
	if (pMcuArea->dwRamAreaNum != 0) {
		// 指定ルーチン実行で使用するワークRAM領域の開始アドレスの初期値設定
		pSrmData->dwmadrRamWorkAreaStartAddr = pMcuArea->dwmadrRamEndAddr[0] - MONEV_WORKPROG_SIZE + 1;	// RAM最終アドレス(複数ブロックならば1個目の最後) - ワークプログラムコードのサイズ分 + 1
	}
	pSrmData->dwSrMode = pSrmData->dwSrMode | SRM_MON_POINT_FUNC_BIT;					// 通過ポイント使用許可

	dwRet = FFWMCUCmd_SetSRM(pSrmData);		// 指定ルーチン実行機能を設定
	dwRet = ChgFFWErrToRFWErr(dwRet);

	return dwRet;
}
// RevRxE2No171004-001 Append End


//=============================================================================
/**
* モニタイベント用モニタコードデータ更新
* @param なし
* @retval なし
*/
//=============================================================================
void UpdateMonEvMonitorCode(void)
{
#if 0
	int i, j, k;
	BYTE byMaxNum = 0;
	BYTE byOffset;
	BYTE byHwBrk[2] = { 0 };
	BYTE byBuff[3] = { 0 };
	BYTE byMonmod1 = 0;
	DWORD dwHwBrkAddr1 = 0;
	DWORD dwHwBrkAddr2 = 0;

	DWORD dwChkAddr = 0;
#endif
	DWORD dwMonEvSwBrkInfo[MONEV_SWBRK_MAX] = { 0 };
	WORD wMonEvHwBrkInfo[MONEV_HWBRK_MAX] = { 0 };
#if 0
	byOffset = 0;
	
	byMaxNum = (BYTE)( (GetIRomSize() - 1) >> 16 );
	byMaxNum++;

	k = 0;
	
	/* S/Wブレーク設定用データ生成 */
	for (i = 0; i < byMaxNum; i++) {
		dwChkAddr = (DWORD)(i << 16);
		for (j = 0; j < MONEV_EV_MAX; j++) {
			if (s_MonEvTable[j].Cond.byDetectType == RF_ASPMONEV_TYPE_SWBRK) {
				if (((DWORD)s_MonEvTable[j].Cond.u64EventAddr & 0xF0000) == dwChkAddr) {
					dwMonEvSwBrkInfo[k] = 0;
					dwMonEvSwBrkInfo[k] |= (DWORD)( (s_MonEvTable[j].Cond.u64EventAddr + 1) & 0x0FFFFF);
					dwMonEvSwBrkInfo[k] |= (DWORD)((s_MonEvTable[j].byChNo) << 24);
					k++;
				}
			}
		}
	}

	/* H/Wブレーク設定状態をMCUから確認 */
	if (GetMonEventSetNum(RF_ASPMONEV_TYPE_HWBRK) != 0) {
		ocd_ReadMemory(MONMOD_ADDR, 1, byBuff);
		if ((byBuff[0] & 0xF0) == 0) {					// 実行後ブレーク0時
			ocd_ReadMemory(BPAL0_ADDR, 3, byBuff);
			dwHwBrkAddr1 = (0x0FFFFF) & (DWORD)((byBuff[2] << 16) | (byBuff[1] << 8) | (byBuff[0]));
			ocd_ReadMemory(MONMOD1_ADDR, 1, &byMonmod1);
		}
	}
	
	/* H/Wブレーク設定用データ生成(iは使わない) */
	for (j = 0; j < MONEV_EV_MAX; j++) {
		if (s_MonEvTable[j].Cond.byDetectType == RF_ASPMONEV_TYPE_HWBRK &&
			s_MonEvTable[j].byChNo != NOSET) {
			if ((DWORD)s_MonEvTable[j].Cond.u64EventAddr == dwHwBrkAddr1) {
				// トレース開始イベント用でないこと
				if ((byMonmod1 & 0x01) == 0) {
					wMonEvHwBrkInfo[0] |= (WORD)(0x0001);
					wMonEvHwBrkInfo[0] |= (WORD)((s_MonEvTable[j].byChNo) << 8);
				}
			} else {
				// ここに来る時点で、2点以上設定できるRL78である
				ocd_ReadMemory(BPAL1_ADDR, 3, byBuff);
				dwHwBrkAddr2 = (0x0FFFFF) & (DWORD)((byBuff[2] << 16) | (byBuff[1] << 8) | (byBuff[0]));

				if ((DWORD)s_MonEvTable[j].Cond.u64EventAddr == dwHwBrkAddr2) {
					// トレース終了イベント用でないこと
					if ((byMonmod1 & 0x02) == 0) {
						wMonEvHwBrkInfo[1] |= (WORD)(0x0001);
						wMonEvHwBrkInfo[1] |= (WORD)((s_MonEvTable[j].byChNo) << 8);
					}
				}
			}
		}
	}
#endif
	/* モニタプログラムを生成 */
	GenMonEvCode(dwMonEvSwBrkInfo, wMonEvHwBrkInfo);

}

//=============================================================================
/**
* モニタイベントのH/Wブレーク方式無効化(モニタコードデータ更新)
* @param なし
* @retval なし
*/
//=============================================================================
void DisableMonEvHwBrk(void)
{
#if 0
	if (!ChkDllTypeEmu(E2)) {
		return;
	}
	memset(monitoerevent_monitor_code + 5, NOSET, 6);
#endif
}

//=============================================================================
/**
* モニタイベントのS/Wブレーク方式無効化(モニタコードデータ更新)
* @param なし
* @retval なし
*/
//=============================================================================
void DisableMonEvSwBrk(void)
{
#if 0
	if (!ChkDllTypeEmu(E2)) {
		return;
	}
	memset(monitoerevent_monitor_code + 2, NOSET, 3);
#endif

}

//=============================================================================
/**
* モニタイベントのE2トリガ条件管理テーブルのポインタ取得
* @param なし
* @retval s_MonEvFactTableのポインタ
*/
//=============================================================================
RFW_MONEV_FACTOR_TABLE* GetMonEvFactorTable(void)
{
	return &s_MonEvFactTable[0];
}

//=============================================================================
/**
* モニタイベント条件の参照
* @param byEventNo
* @param pAspMonEvTable
* @retval RFWエラー
*/
//=============================================================================
RFWERR GetMonEvData(BYTE byEventNo, RFW_ASPMONEVCOND_TABLE *const pAspMonEvTable)
{
	RFWERR dwRet = RFWERR_OK;

	// RevRL78No170420-001-02 Top
	if (s_MonEvTable[monev_tblno(byEventNo)].byChNo != NOSET) {
		memcpy(pAspMonEvTable, &s_MonEvTable[monev_tblno(byEventNo)], sizeof(RFW_ASPMONEVCOND_TABLE));
	}
	else {
		/* 未設定で見つからなかった場合 */
		dwRet = RFWERR_N_BASP_MONEV_NOTSET;
	}
	// RevRL78No170420-001-02 Bottom


	return dwRet;
}

//=============================================================================
/**
* モニタイベント条件の削除
* @param byEventNo
* @retval RFWエラー
*/
//=============================================================================
RFWERR ClrMonEvData(BYTE byEventNo)
{
	RFWERR dwRet = RFWERR_OK;
	RFW_ASPMONEVCOND_TABLE sTmpMonEvTable;

	BYTE byTblNo = monev_tblno(byEventNo);

	if (s_MonEvTable[byTblNo].byChNo == NOSET) {
		dwRet = RFWERR_N_BASP_MONEV_NOTSET;
	}

	if (dwRet == RFWERR_OK) {
		// RevRxE2No171004-001 Modify Start
		sTmpMonEvTable = s_MonEvTable[byTblNo];	// S/Wブレークの解除でエラーになった場合にデータを元に戻すため保存しておく

		/* 当該テーブルを初期化 */
		memset(&s_MonEvTable[byTblNo], 0x00, sizeof(RFW_ASPMONEVCOND_TABLE));

		dwRet = FFWMCUCmd_SetPB(PB_CLR, (DWORD)sTmpMonEvTable.Cond.u64EventAddr);
		dwRet = ChgFFWErrToRFWErr(dwRet);

		if (dwRet != RFWERR_OK) {		// S/Wブレークが解除できなかった
			s_MonEvTable[byTblNo] = sTmpMonEvTable;		// 保存していたデータを元に戻す
		}
		// RevRxE2No171004-001 Modify End
	}

	return dwRet;
}



//=============================================================================
/**
* モニタイベントのイベント通知番号の生成/予約
* @param byDetectType
* @retval イベント通知番号
*/
//=============================================================================
WORD CreateExecBrkNo(BYTE byEventNo, BYTE byDetectType)
{
//	int i, j;
	WORD wRetVal = 0;

	byEventNo;		// Warning回避(RX用暫定処置)
	byDetectType;	// Warning回避(RX用暫定処置)
#if 0
	/* RevRL78No170420-001-02 top */
	if (byDetectType == RF_ASPMONEV_TYPE_SWBRK) {
		/* S/Wブレーク方式はモニタイベント番号の個数分ブレーク番号が確保できるので専用割り付け */
		wRetVal = MONEV_SWBRKNO_START + (byEventNo - RF_ASPMONEV_EVNO_MIN);
	}
	else {
		if (GetMonEventSetNum(RF_ASPMONEV_TYPE_HWBRK) == 0) {
			/* 1点も設定していなかったら開始番号を割り付け */
			wRetVal = MONEV_HWBRKNO_START;
		}
		else {
			/* 1点以上設定していたら未使用番号を探し、割り付け */
			for (i = 0; i < MONEV_HWBRK_MAX; i++) {
				wRetVal = (WORD)(MONEV_HWBRKNO_START + i);
				for (j = 0; j < MONEV_EV_MAX; j++) {
					if (s_MonEvTable[j].Cond.byDetectType == RF_ASPMONEV_TYPE_HWBRK) {
						if (s_MonEvTable[j].wExecBrkNo == wRetVal) {
							break;
						}
					}
				}
				if (j == MONEV_EV_MAX) {
					break;
				}
			}
		}
	}
	/* RevRL78No170420-001-02 bottom */
#endif
	return wRetVal;
}


//=============================================================================
/**
* モニタイベント設定条件のチェック処理(ファミリ依存部)
* @param byChNo
* @retval RFWERR_OK  正常動作
* @retval RFWERR_ERR エラーあり
*/
//=============================================================================
RFWERR ChkMonEvData(BYTE byEventNo, const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent)
{
	RFWERR dwRet = RFWERR_OK;
	// RevRxE2No171004-001 Append Line
	BOOL bChkNop;		// 指定したアドレスがNOP命令か判断

	/* 以下のエラーチェックを行うこと */
	/* RFWERR_N_BASP_ROMWRITE_DISABLE */
	/* RFWERR_N_EV_UNSUPPORT_ADDR */
	/* RFWERR_N_BASP_FUNCEXCLUSIVE */
	/* RFWERR_N_BASP_MONEV_NONOP */
	/* RFWERR_N_BASP_MONEV_BREAKADDR */
	/* RFWERR_N_BASP_MONEV_OVERLAPP */
	/* RFWERR_N_EV_OVER */
	
	/* モニタイベントをサポートするMCUか */
	if (!IsMonEvSurpportMcu()) {
		return RFWERR_N_EV_UNSUPPORT;
	}

	/* 排他関係にある機能が既に有効でないか */
	if (dwRet == RFWERR_OK) {
		if (IsExclusiveFuncEna()) {
			dwRet = RFWERR_N_BASP_FUNCEXCLUSIVE;
		}
	}

	/* ソフトウェアブレーク方式の場合、指定したアドレスがNOP命令か */
	if (dwRet == RFWERR_OK) {
		if (pAspMonitorEvent->byDetectType == RF_ASPMONEV_TYPE_SWBRK) {
			// RevRxE2No171004-001 Modify Start
			dwRet = IsSetAddrNopCode(pAspMonitorEvent->u64EventAddr, &bChkNop);
			if (dwRet == RFWERR_OK) {
				if (!bChkNop) {
					dwRet = RFWERR_N_BASP_MONEV_NONOP;
				}
			}
			// RevRxE2No171004-001 Modify End
		}
	}

	/* デバッグ機能のブレークポイントが既に設定されていないか(S/Wブレークのみ済み) */
	if (dwRet == RFWERR_OK) {
		dwRet = ChkSetAddrBrkPoint(pAspMonitorEvent);
	}

	/* 他のモニタイベント条件番号に指定済みのアドレスか */
	if (dwRet == RFWERR_OK) {
		if (IsMonEvAddrOverlap(byEventNo, pAspMonitorEvent)) {
			dwRet = RFWERR_N_BASP_MONEV_OVERLAPP;
		}
	}


	/* モニタイベントに空きがあるか確認 */
	if (dwRet == RFWERR_OK) {
		dwRet = ChkMonEvNum(byEventNo, pAspMonitorEvent);
	}

	return dwRet;

}

/////////////////////* 共通関数 */////////////////////

//=============================================================================
/**
* 指定のブレーク方式で設定可能なモニタイベント最大点数を取得
* @param   byDetectType
* @retval  MONEV_SWBRK_MAX or MONEV_HWBRK_MAX
*/
//=============================================================================
BYTE GetMonEventNumMax(BYTE byDetectType)
{

	BYTE byReturnNum = 0;

	if (byDetectType == RF_ASPMONEV_TYPE_SWBRK) {
		byReturnNum = MONEV_SWBRK_MAX;
	}
	else {
		byReturnNum = MONEV_HWBRK_MAX;	// RX用暫定処置
	}

	return MONEV_SWBRK_MAX;	// 暫定的にソフトウエアブレーク方式としておく
}


//=============================================================================
/**
* モニタイベント設定数のチェック
* @param   byEventNo
* @param   pAspMonitorEvent
* @retval  RFWエラー
*/
//=============================================================================
RFWERR ChkMonEvNum(BYTE byEventNo, const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent)
{
	int i;
	RFWERR dwRet;
	BYTE byMonEvSetNum, byMonEvMaxNum;
	byMonEvSetNum = GetMonEventSetNum(pAspMonitorEvent->byDetectType);	/* 設定済みイベント設定数 */
	byMonEvMaxNum = GetMonEventNumMax(pAspMonitorEvent->byDetectType);	/* モニタイベント設定可能最大点数 */

	dwRet = RFWERR_OK;

	/* イベント番号チェック */
	for (i = 0; i < MONEV_EV_MAX; i++) {
		/* 番号重複を確認(上書き設定) */
		if ((s_MonEvTable[i].byChNo == byEventNo)) {
			/* 異なるブレーク方式への変更設定 */
			if (pAspMonitorEvent->byDetectType != s_MonEvTable[i].Cond.byDetectType) {
				/* 変更先が既にMAX点数設定されていないか */
				if (byMonEvSetNum >= byMonEvMaxNum) {
					dwRet = RFWERR_N_EV_OVER;
				}
			}
			break;
		}
		else if ((s_MonEvTable[i].byChNo == NOSET)) {
			/* 管理テーブルに空きを発見(0は未設定値) */
			if (byMonEvSetNum >= byMonEvMaxNum) {
				/* 管理テーブルに空きがあってもブレーク方式の設定点数としては空きがない */
				dwRet = RFWERR_N_EV_OVER;
			}
			break;
		}
		else {
			/* 何もせずに処理続行もしくは、処理終了 */
		}

	}
	if (dwRet == RFWERR_OK) {
		if (i == MONEV_EV_MAX) {
			/* 管理テーブル的にも空きがない(全ブレーク方式が満杯) */
			dwRet = RFWERR_N_EV_OVER;
		}
	}

	return dwRet;
}


///////////////////* 品種依存関数 *///////////////////

// RevRxE2No171004-001 Modify Start
//=============================================================================
/**
* 指定設定アドレスにNOPコードが埋め込まれているか確認
* @param   u64MonEvSetAddress
* @param   bChkNop
*		 	TRUE  NOPコードあり
*			FALSE NOPコードなし
* @retval  RFWエラー
* @note    S/Wブレーク方式指定時のみ使用すること。
*/
//=============================================================================
RFWERR IsSetAddrNopCode(UINT64 u64MonEvSetAddress, BOOL* bChkNop)
{
	BYTE	byNopCmd = 0x03;	// NOP
	BYTE	byCmdData;
	RFWERR	dwRet = FFWERR_OK;

	// S/Wブレーク設定アドレスに現在書き込まれている内容を取得
	dwRet = GetPbSetCmdData((MADDR)u64MonEvSetAddress, (MADDR)u64MonEvSetAddress, &byCmdData);
	dwRet = ChgFFWErrToRFWErr(dwRet);
	if (dwRet == RFWERR_OK) {
		// NOP命令かどうか判断
		if (byCmdData != byNopCmd) {
			*bChkNop = FALSE;
		} else {
			*bChkNop = TRUE;
		}
	}

	return dwRet;
}
// RevRxE2No171004-001 Modify End


//=============================================================================
/**
* 接続中のターゲットMCUがモニタイベント機能をサポートしているか確認
* @param   なし
* @retval  TRUE  サポートしている
* @retval  FALSE サポートしていない
* @note    S/Wブレーク方式指定時のみ使用すること。
*/
//=============================================================================
BOOL IsMonEvSurpportMcu (void)
{
#if 0
	if (GetOcdMacroKindUpper() != RL78_OCD_MACRO_KIND_G10) {
		return TRUE;
	}
	else {
		return FALSE;	// RL78/G10系(8bit)は未サポート
	}
#endif

	return TRUE;	// 暫定的にTRUEを返す
}


//=============================================================================
/**
* モニタイベントと排他関係にある機能が有効でないか確認
* @param   なし
* @retval  TRUE  モニタイベントと排他関係にある機能が有効である
* @retval  FALSE モニタイベントと排他関係にある機能が有効でない(無効)
* @note
*/
//=============================================================================
BOOL IsExclusiveFuncEna(void)
{
	// RevRxE2No171004-001 Modify Start
	BOOL bRet = FALSE;
	FFW_SRM_DATA* pSrmData;

	// Start/Stop機能
	pSrmData = GetSrmData();
	// Startルーチン有効、またはStopルーチン有効時
	if (((pSrmData->dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((pSrmData->dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)) {
		bRet = TRUE;
	}

	// ホットプラグ起動
	if (!bRet) {
		if (getHotPlugState()) {
			bRet = TRUE;
		}
	}

	return bRet;
// RevRxE2No171004-001 Modify End
}

//=============================================================================
/**
* モニタイベントに指定するアドレスにデバッグ機能のブレークポイントが設定されていないか確認
* @param   pAspMonitorEvent
* @retval  RFWエラー
*/
//=============================================================================
RFWERR ChkSetAddrBrkPoint(const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent)
{
	// RevRxE2No171004-001 Modify Start
	FFW_PB_DATA*			pPb;			// S/Wブレークポイント設定情報領域のポインタ
	BOOL					bRet;
	RFWERR					dwRet = RFWERR_OK;

	pPb = GetPbData();			// S/Wブレークポイント設定を取得
	bRet = SearchBrkPoint((DWORD)pAspMonitorEvent->u64EventAddr, pPb);
	if (bRet) {		// 対象アドレスに既にブレークが設定されている
		if (SearchMonEvNo(pAspMonitorEvent->u64EventAddr, RF_ASPMONEV_TYPE_SWBRK) == NOSET) {
			dwRet = RFWERR_N_BASP_MONEV_BREAKADDR;
		}
	}

	return dwRet;
	// RevRxE2No171004-001 Modify End
}

//=============================================================================
/**
* 指定アドレスおよびブレーク方式で設定されているモニタイベント番号を取得
* @param   u64Addr
* @param   byDetectType
* @retval  モニタイベント番号(NOSETの場合は未設定)
*/
//=============================================================================
BYTE SearchMonEvNo(UINT64 u64Addr, BYTE byDetectType)
{
	int i;
	BYTE byMonEvNo = NOSET;

	if (!IsEmlE2()) {
		return NOSET;
	}

	if (!IsMonitorEventEna()) {
		return NOSET;
	}

	for (i = 0; i < MONEV_EV_MAX; i++) {
		if (s_MonEvTable[i].byChNo != NOSET) {
			if (s_MonEvTable[i].Cond.u64EventAddr == u64Addr) {
				if (s_MonEvTable[i].Cond.byDetectType == byDetectType) {
					byMonEvNo = s_MonEvTable[i].byChNo;
					break;
				}
			}
		}
	}

	return byMonEvNo;
}


//=============================================================================
/**
* 異なるモニタイベント番号で同じアドレスに設定されていないか確認
* @param   byEventNo
* @param   pAspMonitorEvent
* @retval  TRUE  設定済み
* @retval  FALSE 未設定
*/
//=============================================================================
BOOL IsMonEvAddrOverlap(BYTE byEventNo, const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent)
{
	BOOL bRet = FALSE;
	BYTE byMonEvNo = NOSET;

	byMonEvNo = SearchMonEvNo(pAspMonitorEvent->u64EventAddr, RF_ASPMONEV_TYPE_SWBRK);
	if (byMonEvNo != NOSET) {
		if (byEventNo != byMonEvNo) {
			bRet = TRUE;
		}
	}

	return bRet;

}


//=============================================================================
/**
* モニタイベントのE2トリガ条件管理テーブルのポインタ取得
* @param なし
* @retval s_MonEvTableのポインタ
*/
//=============================================================================
RFW_ASPMONEVCOND_TABLE* GetMonEvCondTableInfo(void)
{
	return &s_MonEvTable[0];
}


//=============================================================================
/**
* テンポラリブレーク(S/Wブレーク方式)付き実行モードフラグの設定
* @param TRUE: 実行モードはテンポラリブレーク付き実行である
* @param FALSE:実行モードはテンポラリブレーク付き実行でない
* @retval なし
*/
//=============================================================================
void SetTempSwBrkRunMode(BOOL bSetFlg)
{
	s_bTempBrkRunModeFlg = bSetFlg;
}

//=============================================================================
/**
* テンポラリブレーク(S/Wブレーク方式)付き実行モードフラグの参照
* @param なし
* @retval s_bTempBrkRunModeFlgの値
*/
//=============================================================================
BOOL IsTempSwBrkRunMode(void)
{
	return s_bTempBrkRunModeFlg;
}

//=============================================================================
/**
* モニタイベントのモニタプログラムコード生成
* @param mon_adr
* @param mon_hwbrk
* @param pbyMonCode
* @retval なし
*/
//=============================================================================
static void GenMonEvCode(DWORD *pdwMonEvSwBrkInfo, WORD *pwMonEvHwBrkInfo)
{
	pdwMonEvSwBrkInfo;	// Warning回避(RX用暫定処置)
	pwMonEvHwBrkInfo;	// Warning回避(RX用暫定処置)

#if 0
	int i, j;						//カウント用変数
	WORD wTemp;						//ソート用変数
	BYTE total_search = 0;
	BYTE total_search_core = 0;
	BYTE usearch_count = 0;
	BYTE usearch_count_core = 0;
	BYTE total_search_upper = 0;
	BYTE cmpuper_end_offset = 0;
	BYTE monev_trans_offset = 0;
	BYTE *moncore;

	moncore = monitoerevent_monitor_code;

	//検索すべきアドレスに対して、同じ上位アドレスがそれぞれ何個あるかを把握するための変数
	BYTE u0 = 0;
	BYTE u1 = 0;
	BYTE u2 = 0;
	BYTE u3 = 0;
	BYTE u4 = 0;
	BYTE u5 = 0;
	BYTE u6 = 0;
	BYTE u7 = 0;

	//拡張モニタプログラムメイン部
	BYTE mon_main[15] = { 0x8E, 0xAE, 0x31, 0x43, 0x28, 0x31, 0x03, 0x07, 0x31, 0x53, 0x08, 0xEC, 0xCA, 0xFF, 0x0E, };

	//モニタイベントで設定されたハードウェアブレークのイベント番号を設定する
	BYTE breaknum_set_hwbrk[6] = { 0x51,0xFF,0xEF,0x02,0x51,0xFF };

	//イベント番号の送信とユーザプログラムへの復帰部
	BYTE breaknum_send_run[24] = { 0x31,0xF2,0xAF,0xFC,0x71,0x7A,0xAF,0x00,0x00,0x00,0x71,0x4A,0xAF,0x9E,0xAD,0xAF,0xF8,0x07,
		0x16,0xAF,0xF6,0x07,0x61,0xFC };

	//ソフトウェアブレークのイベント番号検索部の準備
	BYTE breaknum_search_swbrk_pre[9] = { 0xAE,0xF8,0xC3,0x16,0xAB,0x12,0x8C,0x02,0x76 };

	//上位アドレス一つ分検索のテンプレート
	//cmpuper_pre[1]=検索する上位アドレス
	//cmpuper_pre[4]=中位下位アドレスの検索ルーチン飛び先オフセット値
	BYTE cmpuper_pre[6] = { 0x4C,0xFF,0x13,0xDD,0xFF,0x66 };

	//上位アドレスの検索部コード(最大52byte)
	BYTE cmpuper[48];
	memset(cmpuper, 0x0FF, sizeof(cmpuper));

	//上位アドレスの検索部で何も見つからなかった時のBFA復帰処理のテーブルへ飛ぶBR命令
	BYTE cmpuper_end[3] = { 0xEE,0xFF,0xFF };

	//中位下位アドレスの検索部コード(最大128byte)
	BYTE cmplower[128];
	memset(cmplower, 0x0FF, sizeof(cmplower));
	//
	BYTE cmplower_pre[7] = { 0x56,0xFF,0x44,0xFF,0xFF,0xDD,0xFF };
	//
	BYTE cmplower_end[2] = { 0xEF,0xFF };

	//全部NOP命令に書き換える
	BYTE cmplower_dummy[6];
	memset(cmplower_dummy, NOP_CODE, sizeof(cmplower_dummy));

	//モニタコード末尾部
	BYTE monend[10] = { 0xC2, 0x66, 0xEE, 0xFF, 0xFF, 0xC2, 0xEC, 0xCA, 0xFF, 0x0E };

	//SWBKが0個の時のBFA部リターン処理
	BYTE retbfa[5] = { 0xC2, 0xEC, 0xCA, 0xFF, 0x0E };

	//ソート管理用配列
	//上位1バイトが、検索アドレスの上位アドレスを示す。
	//下位1バイトが、検索アドレスの上位アドレス毎に何個のモニタイベントが設定されているかを示す。
	WORD search_sort[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };

	//各上位ビット毎に振り分ける用の配列
	DWORD uper0_adr[16];
	DWORD uper1_adr[16];
	DWORD uper2_adr[16];
	DWORD uper3_adr[16];
	DWORD uper4_adr[16];
	DWORD uper5_adr[16];
	DWORD uper6_adr[16];
	DWORD uper7_adr[16];
	DWORD dummy_adr[16];

	memset(uper0_adr, 0x0FFFFF, sizeof(uper0_adr));
	memset(uper1_adr, 0x0FFFFF, sizeof(uper1_adr));
	memset(uper2_adr, 0x0FFFFF, sizeof(uper2_adr));
	memset(uper3_adr, 0x0FFFFF, sizeof(uper3_adr));
	memset(uper4_adr, 0x0FFFFF, sizeof(uper4_adr));
	memset(uper5_adr, 0x0FFFFF, sizeof(uper5_adr));
	memset(uper6_adr, 0x0FFFFF, sizeof(uper6_adr));
	memset(uper7_adr, 0x0FFFFF, sizeof(uper7_adr));
	memset(dummy_adr, 0x0FFFFF, sizeof(dummy_adr));

	//ハードウェアブレークに設定されたモニタイベント番号を
	//生成するバイナリデータに反映させる
	breaknum_set_hwbrk[1] = (BYTE)(pwMonEvHwBrkInfo[0] >> 8);
	breaknum_set_hwbrk[5] = (BYTE)(pwMonEvHwBrkInfo[1] >> 8);

	/* S/Wブレーク方式のテンポラリブレーク付き実行でモニタイベント設定ポイントまでGOする際、 */
	/* ブレーク時にモニプログラムタがモニタイベント成立と誤認するため、一時的に無効化する */
	if (IsTempSwBrkRunMode()) {
		memset(mon_main + 2, NOP_CODE, 3);
		SetTempSwBrkRunMode(FALSE);
	}

	//モニタイベントにハードウェアブレークが設定されているか確認する
	for (i = 0; i < 2; i++) {
		//モニタイベントにハードウェアブレークが設定されていなければ
		//MONSTA0レジスタの参照部をNOP命令に置換えて
		//イベント番号設定部に飛ばないようにする
		if ((char)pwMonEvHwBrkInfo[i] == 0) {
			memset(mon_main + (5 + (i * 3)), NOP_CODE, 3);
		}
	}


	//上位ビット毎に振り分ける
	//同時に各上位ビットで何個のアドレスが積まれたかカウントしておく
	for (i = 0; i<16; i++) {
		//モニタイベント番号が0x00ならば何もしない
		if (((pdwMonEvSwBrkInfo[i] & 0xFF000000)) != 0x00000000) {
			switch (pdwMonEvSwBrkInfo[i] & 0x000F0000) {
			case 0x000000:
				uper0_adr[u0] = pdwMonEvSwBrkInfo[i];
				u0++;
				break;
			case 0x010000:
				uper1_adr[u1] = pdwMonEvSwBrkInfo[i];
				u1++;
				break;
			case 0x020000:
				uper2_adr[u2] = pdwMonEvSwBrkInfo[i];
				u2++;
				break;
			case 0x030000:
				uper3_adr[u3] = pdwMonEvSwBrkInfo[i];
				u3++;
				break;
			case 0x040000:
				uper4_adr[u4] = pdwMonEvSwBrkInfo[i];
				u4++;
				break;
			case 0x050000:
				uper5_adr[u5] = pdwMonEvSwBrkInfo[i];
				u5++;
				break;
			case 0x060000:
				uper6_adr[u6] = pdwMonEvSwBrkInfo[i];
				u6++;
				break;
			case 0x070000:
				uper7_adr[u7] = pdwMonEvSwBrkInfo[i];
				u7++;
				break;
			default://上位アドレスが0～7以外ならば何も処理をしない
				break;
			}
		}
	}

	search_sort[0] += (0x00FF & (WORD)u0);
	search_sort[1] += (0x00FF & (WORD)u1);
	search_sort[2] += (0x00FF & (WORD)u2);
	search_sort[3] += (0x00FF & (WORD)u3);
	search_sort[4] += (0x00FF & (WORD)u4);
	search_sort[5] += (0x00FF & (WORD)u5);
	search_sort[6] += (0x00FF & (WORD)u6);
	search_sort[7] += (0x00FF & (WORD)u7);

	//上位アドレスが同じポイントの個数が多い順に並べ直す
	//バブルソート
	for (i = 0; i<7; i++) {
		for (j = 0; j<7; j++) {
			if ((char)search_sort[j] < (char)search_sort[j + 1]) {
				wTemp = search_sort[j];
				search_sort[j] = search_sort[j + 1];
				search_sort[j + 1] = wTemp;
			}
		}


	}

	usearch_count = 0;

	//モニタイベントが設定されたアドレスが存在する上位ビットのアドレスは何個あるか？
	for (i = 0; i<8; i++) {
		if ((char)(search_sort[i]) != 0x00) {
			usearch_count++;
		}
	}

	total_search = u0 + u1 + u2 + u3 + u4 + u5 + u6 + u7;//SWBKで設定されているモニタイベントの総数

	//ここで、SWBKのアドレス検索部前までの固定部をバッファに展開
	memcpy(moncore, mon_main, 15);
	memcpy(moncore + 15, breaknum_set_hwbrk, sizeof(breaknum_set_hwbrk));
	memcpy(moncore + 21, breaknum_send_run, sizeof(breaknum_send_run));
	memcpy(moncore + 45, breaknum_search_swbrk_pre, sizeof(breaknum_search_swbrk_pre));//ここまで固定部(合計52byte)
	if (total_search == 0) {//SWBKに設定されているモニタイベントが0だった場合
		memcpy(moncore + 54, retbfa, sizeof(retbfa));//固定部のすぐ後にBFA復帰処理を書いて終了するようにバッファを展開
		return;
	}

	total_search_core = total_search;//モニタイベントの総数tmp値
	usearch_count_core = usearch_count;//モニタイベントが存在する上位ビットアドレスの総数tmp値


									   //上位アドレス検索のテンプレートに値を設定する
									   //検索の高速化のため、検索対象アドレスが多いものからプログラムに配置する
	for (i = 0; i<8; i++) {
		usearch_count--;
		//モニタイベントが設定されているアドレスで、存在しない上位アドレスの
		//検索ルーチンはモニタプログラムに書き込まない。(bz命令のアウトオブレンジ対策とする)
		if ((char)(search_sort[i]) != 0x00) {
			switch (search_sort[i] & 0xFF00) {
			case 0x0000:
				cmpuper_pre[1] = 0x00;//上位アドレス
									  //上位検索テンプレート(6byte)*(残りの上位検索部) + 未検出時のBR命令(3byte) + 自検索部の後ろ1byte + (下位検索テンプレート(7byte)*想定下位配置済み個数) + BR命令(2byte)*想定配置済み個数
				cmpuper_pre[4] = (BYTE)(((6 * usearch_count) + 3 + 1) + 7 * (total_search_upper)+2 * i); //BZ命令での飛び先オフセット
				total_search_upper += u0;
				break;
			case 0x0100:
				cmpuper_pre[1] = 0x01;
				cmpuper_pre[4] = (BYTE)(((6 * usearch_count) + 3 + 1) + 7 * (total_search_upper)+2 * i); //BZ命令での飛び先オフセット
				total_search_upper += u1;
				break;
			case 0x0200:
				cmpuper_pre[1] = 0x02;
				cmpuper_pre[4] = (BYTE)(((6 * usearch_count) + 3 + 1) + 7 * (total_search_upper)+2 * i); //BZ命令での飛び先オフセット
				total_search_upper += u2;
				break;
			case 0x0300:
				cmpuper_pre[1] = 0x03;
				cmpuper_pre[4] = (BYTE)(((6 * usearch_count) + 3 + 1) + 7 * (total_search_upper)+2 * i); //BZ命令での飛び先オフセット
				total_search_upper += u3;
				break;
			case 0x0400:
				cmpuper_pre[1] = 0x04;
				cmpuper_pre[4] = (BYTE)(((6 * usearch_count) + 3 + 1) + 7 * (total_search_upper)+2 * i); //BZ命令での飛び先オフセット
				total_search_upper += u4;
				break;
			case 0x0500:
				cmpuper_pre[1] = 0x05;
				cmpuper_pre[4] = (BYTE)(((6 * usearch_count) + 3 + 1) + 7 * (total_search_upper)+2 * i); //BZ命令での飛び先オフセット
				total_search_upper += u5;
				break;
			case 0x0600:
				cmpuper_pre[1] = 0x06;
				cmpuper_pre[4] = (BYTE)(((6 * usearch_count) + 3 + 1) + 7 * (total_search_upper)+2 * i); //BZ命令での飛び先オフセット
				total_search_upper += u6;
				break;
			case 0x0700:
				cmpuper_pre[1] = 0x07;
				cmpuper_pre[4] = (BYTE)(((6 * usearch_count) + 3 + 1) + 7 * (total_search_upper)+2 * i); //BZ命令での飛び先オフセット
				total_search_upper += u7;
				break;
			default://ありえない
				break;
			}
			memcpy(cmpuper + 6 * i, cmpuper_pre, 6);
		}
	}

	total_search = total_search_core;//モニタイベントの総数tmp値(復帰)
	usearch_count = usearch_count_core;//モニタイベントが存在する上位ビットアドレスの総数tmp値(復帰)

	for (i = 0; i <= usearch_count_core - 1; i++) {
		//モニタイベントが設定されているアドレスで、存在しない上位アドレスの
		//検索ルーチンはモニタプログラムに書き込まないでbz命令のアウトオブレンジ対策とする
		if ((char)(search_sort[i]) != 0x00) {
			switch ((char)(search_sort[i] >> 8)) {
				//中位下位アドレスの検索をモニタプログラムを配置する
			case 0x00:
				for (j = 0; j < u0; j++) {
					total_search--;
					cmplower_pre[1] = (BYTE)(uper0_adr[j] >> 24);//モニタイベント番号
					cmplower_pre[3] = (BYTE)(uper0_adr[j]);//下位
					cmplower_pre[4] = (BYTE)(uper0_adr[j] >> 8);//中位
					cmplower_pre[6] = (BYTE)(7 * (total_search)+2 * usearch_count);//BZ命令のオフセット値
					memcpy(cmplower + (7 * (total_search_core - total_search - 1) + 2 * i), cmplower_pre, 7);
				}
				break;
			case 0x01:
				for (j = 0; j < u1; j++) {
					total_search--;
					cmplower_pre[1] = (BYTE)(uper1_adr[j] >> 24);//モニタイベント番号
					cmplower_pre[3] = (BYTE)(uper1_adr[j]);//下位
					cmplower_pre[4] = (BYTE)(uper1_adr[j] >> 8);//中位
					cmplower_pre[6] = (BYTE)(7 * (total_search)+2 * usearch_count);//BZ命令のオフセット値
					memcpy(cmplower + (7 * (total_search_core - total_search - 1) + 2 * i), cmplower_pre, 7);
				}
				break;
			case 0x02:
				for (j = 0; j < u2; j++) {
					total_search--;
					cmplower_pre[1] = (BYTE)(uper2_adr[j] >> 24);//モニタイベント番号
					cmplower_pre[3] = (BYTE)(uper2_adr[j]);//下位
					cmplower_pre[4] = (BYTE)(uper2_adr[j] >> 8);//中位
					cmplower_pre[6] = (BYTE)(7 * (total_search)+2 * usearch_count);//BZ命令のオフセット値
					memcpy(cmplower + (7 * (total_search_core - total_search - 1) + 2 * i), cmplower_pre, 7);
				}
				break;
			case 0x03:
				for (j = 0; j < u3; j++) {
					total_search--;
					cmplower_pre[1] = (BYTE)(uper3_adr[j] >> 24);//モニタイベント番号
					cmplower_pre[3] = (BYTE)(uper3_adr[j]);//下位
					cmplower_pre[4] = (BYTE)(uper3_adr[j] >> 8);//中位
					cmplower_pre[6] = (BYTE)(7 * (total_search)+2 * usearch_count);//BZ命令のオフセット値
					memcpy(cmplower + (7 * (total_search_core - total_search - 1) + 2 * i), cmplower_pre, 7);
				}
				break;
			case 0x04:
				for (j = 0; j < u4; j++) {
					total_search--;
					cmplower_pre[1] = (BYTE)(uper4_adr[j] >> 24);//モニタイベント番号
					cmplower_pre[3] = (BYTE)(uper4_adr[j]);//下位
					cmplower_pre[4] = (BYTE)(uper4_adr[j] >> 8);//中位
					cmplower_pre[6] = (BYTE)(7 * (total_search)+2 * usearch_count);//BZ命令のオフセット値
					memcpy(cmplower + (7 * (total_search_core - total_search - 1) + 2 * i), cmplower_pre, 7);
				}
				break;
			case 0x05:
				for (j = 0; j < u5; j++) {
					total_search--;
					cmplower_pre[1] = (BYTE)(uper5_adr[j] >> 24);//モニタイベント番号
					cmplower_pre[3] = (BYTE)(uper5_adr[j]);//下位
					cmplower_pre[4] = (BYTE)(uper5_adr[j] >> 8);//中位
					cmplower_pre[6] = (BYTE)(7 * (total_search)+2 * usearch_count);//BZ命令のオフセット値
					memcpy(cmplower + (7 * (total_search_core - total_search - 1) + 2 * i), cmplower_pre, 7);
				}
				break;
			case 0x06:
				for (j = 0; j < u6; j++) {
					total_search--;
					cmplower_pre[1] = (BYTE)(uper6_adr[j] >> 24);//モニタイベント番号
					cmplower_pre[3] = (BYTE)(uper6_adr[j]);//下位
					cmplower_pre[4] = (BYTE)(uper6_adr[j] >> 8);//中位
					cmplower_pre[6] = (BYTE)(7 * (total_search)+2 * usearch_count);//BZ命令のオフセット値
					memcpy(cmplower + (7 * (total_search_core - total_search - 1) + 2 * i), cmplower_pre, 7);
				}
				break;
			case 0x07:
				for (j = 0; j < u7; j++) {
					total_search--;
					cmplower_pre[1] = (BYTE)(uper7_adr[j] >> 24);//モニタイベント番号
					cmplower_pre[3] = (BYTE)(uper7_adr[j]);//下位
					cmplower_pre[4] = (BYTE)(uper7_adr[j] >> 8);//中位
					cmplower_pre[6] = (BYTE)(7 * (total_search)+2 * usearch_count);//BZ命令のオフセット値
					memcpy(cmplower + (7 * (total_search_core - total_search - 1) + 2 * i), cmplower_pre, sizeof(cmplower_pre));
				}
				break;
			default://ありえない
				break;
			}
		}
		cmplower_end[1] = 7 * (total_search)+2 * usearch_count + 3;
		memcpy(cmplower + (7 * (total_search_core - total_search) + 2 * i), cmplower_end, sizeof(cmplower_end));//ブレーク処理ルーチン差し戻しのBR命令
		usearch_count--;
}

	//最後に書き込み用配列へ整形した各配列をコピーする
   //上位ビットアドレス検索部
	memcpy(moncore + 54, cmpuper, usearch_count_core * 6);

	//上位ビットアドレス未一致時のBR命令オフセット値
	//モニタイベント設定数*中位下位検索部(7byte) + 上位ビットアドレス存在数*BR命令(2byte) + 送信部ジャンプテーブル(4byte)
	cmpuper_end_offset = total_search_core * 7 + usearch_count_core * 2 + 5;
	cmpuper_end[1] = cmpuper_end_offset;
	cmpuper_end[2] = (BYTE)0x00; //256byteを超えることはない
	memcpy(moncore + 54 + usearch_count_core * 6, cmpuper_end, sizeof(cmpuper_end));

	cmpuper_end_offset -= 5;//実際の中位下位検索部のサイズはジャンプテーブル5byteを除いたサイズ
							//中位下位ビットアドレス検索部
	memcpy(moncore + 54 + usearch_count_core * 6 + 3, cmplower, cmpuper_end_offset);

	//送信部へのジャンプテーブルと検索しても何も見つからなかった時のBFA戻り処理部(monend)
	//送信部へのBR命令オフセット値
	//中位下位検索部 + 上位検索部 + 上位未一致BR命令 + ジャンプテーブルの番号コピー処理MOV命令 + 固定部の末尾からデータ送信部先頭までのオフセット値 + 自BR命令分
	monev_trans_offset -= (cmpuper_end_offset + usearch_count_core * 6 + 3 + 4 + 31 + 3);
	monend[3] = (char)monev_trans_offset;
	monend[4] = 0xFF;//データ送信部に戻るので、オフセット値はマイナスとなる。
	memcpy(moncore + 54 + usearch_count_core * 6 + 3 + cmpuper_end_offset, monend, sizeof(monend));

#endif
}

//=============================================================================
/**
* モニタイベントのパスカウント値取得
* @param byEventNo		モニタイベント番号
* @param *pu64PassCnt	パスカウント値
* @retval なし
*/
//=============================================================================
void GetMonEvPassCount(BYTE byEventNo, UINT64 *const pu64PassCnt)
{	
	byEventNo;
	*pu64PassCnt = 0;

}

