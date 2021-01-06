///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_pwr.h
 * @brief パワーモニタコマンドのヘッダファイル
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
#ifndef	__DOASP_PWR_H__
#define	__DOASP_PWR_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義
#define PWRMON_MASK		0x0FFF0000
#define PWRMON_CHINFO	0x80000000
#define PWRMONEV_TRG_LEVEL (10)
#define PWRMONEV_TRG_PULSE (11)

#define	PWRMCTRL_SAMPF_REGVAL_1U	0x000F
#define	PWRMCTRL_SAMPF_REGVAL_2U	0x001D
#define	PWRMCTRL_SAMPF_REGVAL_5U	0x004A
#define	PWRMCTRL_SAMPF_REGVAL_10U	0x0095
#define	PWRMCTRL_SAMPF_REGVAL_20U	0x012B
#define	PWRMCTRL_SAMPF_REGVAL_50U	0x02ED
#define	PWRMCTRL_SAMPF_REGVAL_100U	0x05DB
#define	PWRMCTRL_SAMPF_REGVAL_200U	0x0BB7
#define	PWRMCTRL_SAMPF_REGVAL_500U	0x1D4B
#define	PWRMCTRL_SAMPF_REGVAL_1M	0x3A97

#define BASE18V			0
#define BASE20V			1
#define BASE33V			2
#define BASE45V			3
#define BASE50V			4
#define CALC_MODE_MIN 0
#define CALC_MODE_MAX 1
#define PWRMON_BASECURRENT_NUM_MAX	5


#define	E2TRG_BIT_PWRMON_LEVEL	0x0400
#define	E2TRG_BIT_PWRMON_PULSE	0x0800

#define	DENOMI_CALC_AD	((((3.3 / 4096) / 50)/(double)0.2)*1000000)

// グローバル関数の宣言
#ifdef	__cplusplus
extern	"C" {
#endif
extern DWORD DO_SetPwrMon(const RFW_PWRMONSAMP_DATA* pPowerMonitor);
extern DWORD DO_GetPwrMon(RFW_PWRMONSAMP_DATA *const pPowerMonitor);
extern DWORD DO_ClrPwrMon(void);
extern DWORD DO_SetPwrMonEvent(const RFW_PWRMONEV_DATA* pPowerMonitorEvent);
extern DWORD DO_GetPwrMonEvent(RFW_PWRMONEV_DATA *const pPowerMonitorEvent);
extern DWORD DO_ClrPwrMonEvent(void);
extern void InitAspPwrMon(void);
extern void InitAspPwrMonEv(void);
extern void InitAspPwrMonCorrect(void);
extern BOOL IsAspPwrMonEna(void);
extern void GetAspPwrMon(RFW_PWRMONSAMP_DATA * pPowerMonitor);
extern void GetAspPwrMonEv(RFW_PWRMONEV_DATA * pPowerMonitorEvent);
extern DWORD CalcAd2Current(DWORD dwAd);
extern void CalcE2BaseCurrentVal(void);
extern WORD CalcPwrMonThreshold(WORD wPower, BYTE byMode);
extern void SetE2BaseCurrentVal(double dblSlope, WORD wData0, WORD wData1, WORD wData2, WORD wData3, WORD wData4);
#ifdef	__cplusplus
};
#endif

#endif	// __DOASP_PWR_H__
