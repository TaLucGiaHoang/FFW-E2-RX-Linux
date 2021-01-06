///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_can.h
 * @brief CANモニタコマンドのヘッダファイル
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
#ifndef	__DOASP_CAN_H__
#define	__DOASP_CAN_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義

// グローバル関数の宣言
extern DWORD DO_SetCanMon(BYTE byChNo, const RFW_CANMON_DATA *pCanMonData);
extern DWORD DO_GetCanMon(BYTE byChNo, RFW_CANMON_DATA *pCanMonData);
extern DWORD DO_ClrCanMon(BYTE byChNo);
extern DWORD DO_SetCanMonEvent(BYTE byChNo, const RFW_CANMONEV_DATA *pCanMonitorEvent);
extern DWORD DO_GetCanMonEvent(BYTE byChNo, RFW_CANMONEV_DATA *pCanMonitorEvent);
extern DWORD DO_ClrCanMonEvent(BYTE byChNo);

extern void InitAspCanMon(BYTE byChNo);
extern void InitAspCanMonEv(BYTE byChNo);
extern BOOL IsAspCanMonEna(void);
extern BOOL GetCanMonEna(BYTE byChNo);
extern BOOL GetCanMonEvEna(BYTE byChNo);
extern void GetCanMonData(BYTE byChNo, RFW_CANMON_DATA *pCanMonData);
extern void GetCanMonEvData(BYTE byChNo, RFW_CANMONEV_DATA *pCanMonEvData);

#endif	// __DOASP_CAN_H__
