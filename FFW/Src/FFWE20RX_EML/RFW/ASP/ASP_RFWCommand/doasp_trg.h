///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_trg.h
 * @brief 外部トリガコマンドのヘッダファイル
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
#ifndef	__DOASP_TRG_H__
#define	__DOASP_TRG_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義
/* 外部トリガ入出力機能割り当てビット位置 */
#define RF_ASPTRGOUT_CH0_BIT	(1 << 7)	// TRGO0(9番ピン)機能割り当てビット7
#define RF_ASPTRGOUT_CH1_BIT	(1 << 8)	// TRGO1(10番ピン)機能割り当てビット8
#define RF_ASPTRGIN_CH0_BIT		(1 << 9)	// TRGI0(11番ピン)機能割り当てビット9
#define RF_ASPTRGIN_CH1_BIT		(1 << 10)	// TRGI1(12番ピン)機能割り当てビット10

/* 外部トリガ入出力 */
#define RF_ASPTRGIN				(0)			// 外部トリガ入力
#define RF_ASPTRGOUT			(1)			// 外部トリガ出力

// グローバル関数の宣言
extern DWORD DO_SetExTriggerIn(BYTE byChNo, BYTE byDetectType);
extern DWORD DO_GetExTriggerIn(BYTE byChNo, BYTE *const pbyDetectType);
extern DWORD DO_ClrExTriggerIn(BYTE byChNo);
extern DWORD DO_SetExTriggerOut(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut);
extern DWORD DO_GetExTriggerOut(BYTE byChNo, RFW_EXTRGOUT_DATA *const pExTriggerOut);
extern DWORD DO_ClrExTriggerOut(BYTE byChNo);
extern DWORD DO_SetExTriggerOutLevel(BYTE byChNo, BYTE byActiveLevel);
extern DWORD DO_GetExTriggerOutLevel(BYTE byChNo, BYTE *const pbyActiveLevel);
extern void InitAspExTrgIn(void);
extern void InitAspExTrgOut(BYTE byChNo);
extern void InitAspExTrg(void);
extern BOOL IsAspExTrgOutEna(BYTE byChNo);
extern BOOL IsAspExTrgInEna(BYTE byChNo);

#ifdef	__cplusplus
extern	"C" {
#endif
extern void InitAspExTrgOutLevel(void);
#ifdef	__cplusplus
};
#endif

#endif	// __DOASP_TRG_H__
