///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_per.h
 * @brief 時間計測コマンドのヘッダファイル
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
#ifndef	__DOASP_PER_H__
#define	__DOASP_PER_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義
#define RF_ASPPER_MODE_TRG		(1<<0)
#define RF_ASPPER_MODE_TRG_PULSE	(0<<0)
#define RF_ASPPER_MODE_TRG_LEVEL	(1<<0)
#define RF_ASPPER_MODE_LMT		(1<<1)
#define RF_ASPPER_MODE_LMT_UPPER	(0<<1)
#define RF_ASPPER_MODE_LMT_LOWER	(1<<1)
#define RF_ASPPER_THRESHOLD_MAX	(0x8555555555555)

// グローバル関数の宣言
extern DWORD DO_SetAspPerCond(BYTE byChNo, const RFW_ASPPERCOND_DATA *pAspPerCond);
extern DWORD DO_GetAspPerCond(BYTE byChNo, RFW_ASPPERCOND_DATA *pAspPerCond);
extern DWORD DO_ClrAspPerCond(BYTE byChNo);
extern DWORD DO_GetAspPerData(BYTE byChNo, RFW_ASPPER_DATA *pAspPerformanceData);
extern DWORD DO_ClrAspPerData(BYTE byChNo);

#ifdef	__cplusplus
extern	"C" {
#endif
extern void InitAspPerCond(BYTE byChNo);
#ifdef	__cplusplus
};
#endif

extern BOOL IsAspPerformanceEna(void);
extern BOOL GetPerformanceEna(BYTE byChNo);
extern void GetPerformanceCondData(BYTE byChNo, RFW_ASPPERCOND_DATA *pAspPerCond);

#endif	// __DOASP_PER_H__
