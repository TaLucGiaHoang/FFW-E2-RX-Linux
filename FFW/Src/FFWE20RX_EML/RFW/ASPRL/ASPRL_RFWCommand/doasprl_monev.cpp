///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasprl_monev.cpp
 * @brief
 * @author M.Yamamoto
 * @author Copyright (C) 2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/10/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
*/

#include <stdlib.h>
#include "rfwasprl_monev.h"
#include "doasprl_monev.h"
#include "emudef.h"
#include "rfwasp_RL.h"
#include "doasp_sys.h"
#include "ex_inc2.h"
#include "swbrcond.h"
#include "k0r_ocd.h"
#include "k0ocd.h"
#include "usermap.h"
#include "doasp_sys_family.h"
#include "memacc.h"
#include "socunit.h"
#include "swbrcond.h"
#include "ocdusbcmd.h"


// ファイル内static変数の宣言

/* 共通部 */
/* モニタイベント条件 管理テーブル(両ブレーク方式共通) */
static RFW_ASPMONEVCOND_TABLE s_MonEvTable[MONEV_EV_MAX+1];	 // 削除用に+1する

/* モニタイベントのE2トリガ条件管理テーブル */
static RFW_MONEV_FACTOR_TABLE s_MonEvFactTabele[MONEV_TRG_MAX+1];

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

	
	BOOL bSetMonitorEna = IsMonitorEventEna();

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


	if (dwRet == RFWERR_OK) {
		if (!bSetMonitorEna && IsMonitorEventEna()) {
			UpdateMonEvMonitorCode();
			K0RMQB2_UpdateMonitor();
		}
	}

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
* モニタイベント条件の参照
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
	if (dwRet == RFWERR_OK) {
		/* この消去によって設定数0になった場合は、モニタプログラムに遷移しないよう更新 */
		if (!IsMonitorEventEna()) {
			K0RMQB2_UpdateMonitor();
		}
	}
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

		s_MonEvFactTabele[byChNo].bMonEvChSw = MONEV_TRG_SW_ON;

		if (byChNo == MONEV_TRG_CH6) {
			s_MonEvFactTabele[byChNo].byMonEvCmpData = MONEV_CH6_ON;	/* ch6の場合は01でON、00でOFFとする */
			dwEventLinkType = RFW_EVENTLINK_LEVEL;	/* ch6のみレベル型 */
		}
		else {
			s_MonEvFactTabele[byChNo].byMonEvCmpData = byData;			/* ch0～5の場合は設定値を保持 */
			dwEventLinkType = RFW_EVENTLINK_PULSE;	/* ch0～5はパルス型 */
		}
		s_MonEvFactTabele[byChNo].byMonEvMaskData = 0;


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
		if (s_MonEvFactTabele[byChNo].bMonEvChSw == MONEV_TRG_SW_OFF) {
			dwRet = RFWERR_N_BASP_CHNOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		*pbyData = s_MonEvFactTabele[byChNo].byMonEvCmpData;
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
		if (s_MonEvFactTabele[byChNo].bMonEvChSw == MONEV_TRG_SW_OFF) {
			dwRet = RFWERR_N_BASP_CHNOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		s_MonEvFactTabele[byChNo].byMonEvCmpData = 0;
		s_MonEvFactTabele[byChNo].byMonEvMaskData = 0;
		s_MonEvFactTabele[byChNo].bMonEvChSw = MONEV_TRG_SW_OFF;
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

#if 0
	if (dwRet == RFWERR_OK) {
		switch (byDataType) {
		case RF_MONEVDATA_PASSCOUNT:
			GetMonEvPassCount(byEventNo, pu64Data1);
			break;
		default:	// 通らない
			break;
		}
	}
#endif
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
	memset(s_MonEvFactTabele, NOSET, (sizeof(RFW_MONEV_FACTOR_TABLE) * MONEV_TRG_MAX));
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
	for (i = 0; i < MONEV_EV_MAX + 1; i++) {
		/* Clr処理で管理テーブルを前詰めにしているので、常に先頭を削除 */
		if (s_MonEvTable[i].byChNo != NOSET) {			// RevRL78No170420-001-02
			ClrMonEvData(s_MonEvTable[i].byChNo);		// RevRL78No170420-001-02
		}
	}
	// モニタを更新
	K0RMQB2_UpdateMonitor();
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
	byCount += GetMonEventSetNum(RF_ASPMONEV_TYPE_HWBRK);

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
	EXHWBRK HwBrkSet = { 0 };
	HwBrkSet.num = 1;

	WORD wEvNo[10] = { 0 };
	HwBrkSet.event = wEvNo;

	EXEVCOND2 EventData = { 0 };
	EXSWBRK SwBrkSet = { 0 };
	WORD wSetNum = 1;
	WORD wExecBrkNo = 0;

	/* 上書きの場合を考慮し、指定のイベント番号で設定済みのものを削除する */
	ClrMonEvData(byEventNo);

	/* EXEC I/Fに設定するブレーク番号を抽出 */
	// RevRL78No170420-001-02 Top
	wExecBrkNo = CreateExecBrkNo(byEventNo, pAspMonitorEvent->byDetectType);		// RevRL78No170420-001-02
		
	/* H/Wブレーク方式 */
	if (pAspMonitorEvent->byDetectType == RF_ASPMONEV_TYPE_HWBRK) {
		EventData.stat = EX_MONEV_STS;
		EventData.saddr = (DWORD)(pAspMonitorEvent->u64EventAddr);
		if (exe_ex_evncond2(EX_SET, wExecBrkNo, &EventData) == EX_PE_EVNOVER) {
			/* H/Wブレークがデバッグ機能として使用されている */
			/* モニタイベント自体の設定点数オーバーは事前にチェック済み */
			dwRet = RFWERR_N_EV_USEFUNC;

		}
		// RevRL78No170420-001-02 Bottom
		if (dwRet == RFWERR_OK) {
			HwBrkSet.event[0] = wExecBrkNo;
			exe_ex_hwbrkevent(EX_SET, &HwBrkSet);
		}
	}
	else {
		/* S/Wブレーク方式 */
		SwBrkSet.sbno = wExecBrkNo;
		SwBrkSet.sbaddr = (DWORD)(pAspMonitorEvent->u64EventAddr);
		SwBrkSet.dummy[0] = 'E';	// MagicCode
		SwBrkSet.dummy[1] = '2';	// MagicCode
		exe_ex_swbrkcond(EX_SET, &wSetNum, &SwBrkSet);
		if (SwBrkSet.sberr == EX_SB_POINTOVER) {
			dwRet = RFWERR_N_SBPOINT_OVER;
		}
	}
	if (dwRet == RFWERR_OK) {
		// RevRL78No170420-001-02 top
		s_MonEvTable[monev_tblno(byEventNo)].byChNo = byEventNo;
		s_MonEvTable[monev_tblno(byEventNo)].wExecBrkNo = wExecBrkNo;
		memcpy(&s_MonEvTable[monev_tblno(byEventNo)].Cond, pAspMonitorEvent, sizeof(RFW_ASPMONEVCOND_DATA));
		// RevRL78No170420-001-02 Bottom
	}

	return dwRet;

}


//=============================================================================
/**
* モニタイベント用モニタコードデータ更新
* @param なし
* @retval なし
*/
//=============================================================================
void UpdateMonEvMonitorCode(void)
{
	int i, j, k;
	BYTE byMaxNum = 0;
	BYTE byOffset;
	BYTE byHwBrk[2] = { 0 };
	BYTE byBuff[3] = { 0 };
	BYTE byMonmod1 = 0;
	DWORD dwHwBrkAddr1 = 0;
	DWORD dwHwBrkAddr2 = 0;

	DWORD dwChkAddr = 0;

	DWORD dwMonEvSwBrkInfo[MONEV_SWBRK_MAX] = { 0 };
	WORD wMonEvHwBrkInfo[MONEV_HWBRK_MAX] = { 0 };
	
	
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
	if (!ChkDllTypeEmu(E2)) {
		return;
	}
	memset(monitoerevent_monitor_code + 5, NOSET, 6);

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
	if (!ChkDllTypeEmu(E2)) {
		return;
	}
	memset(monitoerevent_monitor_code + 2, NOSET, 3);

}

//=============================================================================
/**
* モニタイベントのE2トリガ条件管理テーブルのポインタ取得
* @param なし
* @retval s_MonEvFactTabeleのポインタ
*/
//=============================================================================
RFW_MONEV_FACTOR_TABLE* GetMonEvFactorTable(void)
{
	return &s_MonEvFactTabele[0];
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

	RFW_ASPMONEVCOND_TABLE MonEvSetOld = { 0 };
	WORD wDelNum = 1;

	EXHWBRK HwBrkSet = { 0 };
	HwBrkSet.num = wDelNum;
	WORD wEvNo[10] = { 0 };
	HwBrkSet.event = wEvNo;
	EXEVCOND2 EventData = { 0 };
	EXSWBRK SwBrkSet = { 0 };

	BYTE byTblNo = monev_tblno(byEventNo);

	// RevRL78No170420-001-02 top
	if (s_MonEvTable[byTblNo].byChNo == NOSET) {
		dwRet = RFWERR_N_BASP_MONEV_NOTSET;
	}

	if (dwRet == RFWERR_OK) {
		if (s_MonEvTable[byTblNo].Cond.byDetectType == RF_ASPMONEV_TYPE_HWBRK) {
			/* H/Wブレーク方式の削除 */ 
			HwBrkSet.event[0] = s_MonEvTable[byTblNo].wExecBrkNo;
			exe_ex_hwbrkevent(EX_DEL, &HwBrkSet);
			EventData.stat = EX_MONEV_STS;
			exe_ex_evncond2(EX_DEL, s_MonEvTable[byTblNo].wExecBrkNo, &EventData);
		}
		else {
			/* S/Wブレーク方式の削除 */
			SwBrkSet.sbno = s_MonEvTable[byTblNo].wExecBrkNo;
			SwBrkSet.dummy[0] = 'E';
			SwBrkSet.dummy[1] = '2';
			exe_ex_swbrkcond(EX_DEL, &wDelNum, &SwBrkSet);
		}

		/* 当該テーブルを初期化 */
		memset(&s_MonEvTable[byTblNo], 0x00, sizeof(RFW_ASPMONEVCOND_TABLE));
	}
	// RevRL78No170420-001-02 Bottom

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
	int i, j;
	WORD wRetVal = 0;
		
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

	/* 指定されたアドレス範囲内か */
	if (dwRet == RFWERR_OK) {
		if (!IsMonEvSetAddrEna(pAspMonitorEvent->u64EventAddr)) {
			dwRet = RFWERR_N_EV_UNSUPPORT_ADDR;
		}
	}

	/* フラッシュ書き換え禁止デバッグ中か */
	if (dwRet == RFWERR_OK) {
		if (!IsFlashProgramDebugEna()) {
			dwRet = RFWERR_N_BASP_ROMWRITE_DISABLE;
		}
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
			if (!IsSetAddrNopCode(pAspMonitorEvent->u64EventAddr)) {
				dwRet = RFWERR_N_BASP_MONEV_NONOP;
			}
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
		// OCDマクロの種類により設定可能なH/Wブレーク点数が異なる
		if (GetOcdMacroKind() >= RL78_OCD_MACRO_KIND_V3) {
			byReturnNum = MONEV_HWBRK_MAX;
		}
		else {
			byReturnNum = MONEV_HWBRK_MAX - 1;
		}
	}

	return byReturnNum;
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

//=============================================================================
/**
* 指定設定アドレスにNOPコードが埋め込まれているか確認
* @param   u64MonEvSetAddress
* @retval  TRUE  NOPコードあり
* @retval  FALSE NOPコードなし
* @note    S/Wブレーク方式指定時のみ使用すること。
*/
//=============================================================================
BOOL IsSetAddrNopCode(UINT64 u64MonEvSetAddress)
{
	EXMEM work = { 0 };
	BOOL bRet = FALSE;
	BYTE byData[2] = { 0xFF };

	work.data_1 = byData;
	work.acc_size = EX_DATA_SIZE_1;
	work.number = 1;
	work.addr_1 = (DWORD)u64MonEvSetAddress;

	/* この時点ではS/Wブレークコードが差し替えられて読み出していてもよい */
	Memory_Read(&work);
	if (work.data_1[0] != NOP_CODE) {
		bRet = FALSE;
	}
	else {
		bRet = TRUE;
	}
	return bRet;
}


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
	if (GetOcdMacroKindUpper() != RL78_OCD_MACRO_KIND_G10) {
		return TRUE;
	}
	else {
		return FALSE;	// RL78/G10系(8bit)は未サポート
	}
}

//=============================================================================
/**
* モニタイベントに指定可能なアドレスか確認
* @param   u64SetAddr
* @retval  TRUE  モニタイベントに指定可能なアドレスである(ROM/RAM)
* @retval  FALSE モニタイベントに指定可能なアドレスでない(DataFlash/SFR/予約など)
* @note
*/
//=============================================================================
BOOL IsMonEvSetAddrEna(UINT64 u64SetAddr)
{
	DWORD dwSetAddr = (DWORD)u64SetAddr;
	BOOL bRet = FALSE;

	/* ユーザ公開のROM領域内？ */
	if ((GetIRomSize() - 0x200) > dwSetAddr) {	/* モニタプログラムとリセットフックモニタにより、ROM最終アドレス付近は512byte固定で占有 */
		bRet = TRUE;
	}

	return bRet;
}


//=============================================================================
/**
* フラッシュ書き換え可能なデバッグモードか確認
* @param   なし
* @retval  TRUE  フラッシュ書き換え可能なデバッグモードである
* @retval  FALSE フラッシュ書き換え禁止なデバッグモードである
* @note
*/
//=============================================================================
BOOL IsFlashProgramDebugEna(void)
{

	BOOL bRet = FALSE;

	if (GetFlashProgramDisableFlag() != 0) {
		bRet = FALSE;
	}
	else {
		bRet = TRUE;
	}

	return bRet;
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
	DWORD dwStartStopFuncSw, dwDummy1, dwDummy2;
	BOOL bRet = FALSE;


	/* 疑似RRM/DMM機能 */
	if (GetRrmDmmSw() || GetDummyRrmDmmSw()) {
		bRet = TRUE;
	}

	/* Start/Stop機能 */
	if (!bRet) {
		GetStartStopFuncInfo(&dwStartStopFuncSw, &dwDummy1, &dwDummy2);
		if (dwStartStopFuncSw != 0) {
			bRet = TRUE;
		}
	}

	/* HotPlug-in or dDTC方式のライブデバッグ */
	if (!bRet) {
		if (GetLiveDebugSw()) {
			bRet = TRUE;
		}
	}
	return bRet;

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
	RFWERR dwRet;
	BYTE byCodeData[10];
	WORD wResult;
	int i;
	WORD wBrkNo[2] = {};
	EXEVCOND2 HwBrkCond = {};
	EXHWBRK HwBrkData;
	HwBrkData.event = wBrkNo;
	EXEVCOND2 EvCondData = {};

	dwRet = RFWERR_OK;

	wResult = ChkSbAddr((DWORD)pAspMonitorEvent->u64EventAddr, byCodeData);
	if (wResult & 0x0001) {
		if (SearchMonEvNo(pAspMonitorEvent->u64EventAddr, RF_ASPMONEV_TYPE_SWBRK) == NOSET) {
			dwRet = RFWERR_N_BASP_MONEV_BREAKADDR;
		}
	}
	if (dwRet == RFWERR_OK) {

		/* H/Wブレークに設定されたイベント番号を取得 */
		if ((exe_ex_hwbrkevent(EX_REF, &HwBrkData)) != EX_SE_IRGEVNSET) {
			for (i = 0; i < HwBrkData.num ; i++) {
				/* イベントブレーク機能用の番号(モニタイベント番号ではない場合) */
				if (HwBrkData.event[i] < MONEV_HWBRK_START_NO) {
					/* 単体イベント設定情報を取得 */
					exe_ex_evncond2(EX_REF, HwBrkData.event[i], &EvCondData);
					if (EvCondData.saddr == (DWORD)pAspMonitorEvent->u64EventAddr) {
						dwRet = RFWERR_N_BASP_MONEV_BREAKADDR;
					}
				}
			}
		}
	}

	return dwRet;
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
	else {
		byMonEvNo = SearchMonEvNo(pAspMonitorEvent->u64EventAddr, RF_ASPMONEV_TYPE_HWBRK);
		if (byMonEvNo != NOSET) {
			if (byEventNo != byMonEvNo) {
				bRet = TRUE;
			}
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

