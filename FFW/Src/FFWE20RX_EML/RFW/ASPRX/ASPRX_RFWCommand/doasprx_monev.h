///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasprl_monev.h
 * @brief
 * @author REL M.Yamamoto, PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・新規作成 M.Yamamoto
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#ifndef	__DOASPRL_MONEV_H__
#define	__DOASPRL_MONEV_H__

#include "rfw_typedef.h"
#include "mcudef.h"
#include "rfwasp_RX.h"

// define定義
#define MONEV_HWBRK_MAX 2
#define MONEV_SWBRK_MAX 16
#define MONEV_TRG_BASE_CH0 13

#define MONEV_TRG_SW_OFF	(0x00)
#define MONEV_TRG_SW_ON		(0x01)

#define MONEV_EV_MAX			0x0020
#define MONEV_HWBRKNO_START		0x7FF0
#define MONEV_HWBRKNO_END		0x7FF1
// RevRxE2No171004-001 Modify Start
#define MONEV_SWBRKNO_START		0xE0
#define MONEV_SWBRKNO_END		0xFF
// RevRxE2No171004-001 Modify End

#define monev_tblno(x) (x - RF_ASPMONEV_EVNO_MIN)

// RevRxE2No171004-001 Append Line
#define MONEV_WORKPROG_SIZE		0x100	// 通過ポイント用ワークプログラムサイズ

typedef	struct {
	BYTE byChNo;
	RFW_ASPMONEVCOND_DATA Cond;
	USHORT wExecBrkNo;
} RFW_ASPMONEVCOND_TABLE;


typedef	struct {
	BYTE bMonEvChSw;
	BYTE byMonEvCmpData;
	BYTE byMonEvMaskData;
} RFW_MONEV_FACTOR_TABLE;


typedef	struct {
	BYTE	byMonEvNo;
	DWORD	dwPassCount;
}RFW_ASPMONEV_PASSCOUNT;

// cppファイルで使用する関数のextern
#ifdef __cplusplus
extern "C" {
#endif

extern DWORD DO_SetMonitorEvent(BYTE byEventNo, const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent);
extern DWORD DO_GetMonitorEvent(BYTE byEventNo, RFW_ASPMONEVCOND_DATA *const pAspMonitorEvent);
extern DWORD DO_ClrMonitorEvent(BYTE byEventNo);

extern DWORD DO_SetMonitorEventFactor(BYTE byEventNo, BYTE byData, BYTE byMask);
extern DWORD DO_GetMonitorEventFactor(BYTE byEventNo, BYTE *pbyData, BYTE *pbyMask);
extern DWORD DO_ClrMonitorEventFactor(BYTE byEventNo);
extern DWORD DO_GetMonitorEventData(BYTE byDataType, BYTE byEventNo, UINT64 *const pu64Data1, UINT64 *const pu64Data2);

extern RFWERR SetMonEvData(BYTE, const RFW_ASPMONEVCOND_DATA*);
extern RFWERR GetMonEvData(BYTE, RFW_ASPMONEVCOND_TABLE *const);
extern RFWERR ClrMonEvData(BYTE);

// RevRxE2No171004-001 Append Start
extern RFWERR ChgFFWErrToRFWErr(DWORD dwFFWErr);
extern RFWERR SetSrModeMonEvUse(void);
// RevRxE2No171004-001 Append End

extern void UpdateMonEvMonitorCode(void);

extern RFW_ASPMONEVCOND_TABLE* GetMonEvCondTableInfo(void);
extern RFW_MONEV_FACTOR_TABLE* GetMonEvFactorTable(void);

extern BYTE GetMonEventSetNum(BYTE);
extern BYTE GetMonEventNumMax(BYTE);
extern WORD CreateExecBrkNo(BYTE byEventNo, BYTE byDetectType);
extern void DisableMonEvHwBrk(void);
extern void DisableMonEvSwBrk(void);
extern BOOL IsMonitorEventEna(void);
extern void InitAspMonEv(void);
extern RFWERR ChkMonEvNum(BYTE, const RFW_ASPMONEVCOND_DATA*);
// RevRxE2No171004-001 Modify Line
extern RFWERR IsSetAddrNopCode(UINT64, BOOL*);
extern BOOL IsMonEvSetAddrEna(UINT64);
extern BOOL IsFlashProgramDebugEna(void);
extern BOOL IsExclusiveFuncEna(void);
extern RFWERR ChkSetAddrBrkPoint(const RFW_ASPMONEVCOND_DATA*);
extern BOOL IsMonEvAddrSet(UINT64, BYTE);
extern BOOL IsMonEvAddrOverlap(BYTE, const RFW_ASPMONEVCOND_DATA*);
extern BYTE SearchMonEvNo(UINT64, BYTE);

extern RFWERR ChkMonEvData(BYTE, const RFW_ASPMONEVCOND_DATA*);
extern BOOL IsMonEvSurpportMcu(void);
extern void ClrAllMonData(void);
extern void SetTempSwBrkRunMode(BOOL bSetFlg);
extern BOOL IsTempSwBrkRunMode(void);
extern void GetMonEvPassCount(BYTE byEventNo, UINT64 *const pu64PassCnt);
#ifdef __cplusplus
};
#endif

// グローバル関数の宣言

#define MONEV_HWBRK_START_NO	0x07FF0
#define MONEV_HWBRK_END_NO		0x07FFF

#endif	// __DOASPRL_MONEV_H__
