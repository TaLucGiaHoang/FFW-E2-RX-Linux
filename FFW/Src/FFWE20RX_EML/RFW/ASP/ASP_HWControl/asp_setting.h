///////////////////////////////////////////////////////////////////////////////
/**
* @file asp_setting.h
* @brief ASP FPGA setting
* @author TSSR M.Ogata
* @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
* @author Renesas System Design Co., Ltd. All rights reserved.
* @date 2017/03/28
*/
///////////////////////////////////////////////////////////////////////////////
#ifndef _ASP_SETTING_H_
#define _ASP_SETTING_H_

#include "rfw_typedef.h"
#include "mcudef.h"


#define RFW_EVENTLINK_CH_MAX		32
#define RFW_EVENTLINK_FACT_MASK		0x0000003E
#define RFW_EVENTLINK_ACT_CLEAR		0x0000003F

#ifdef __cplusplus
extern "C" {
#endif
	extern DWORD Start_Asp(void);
	extern void Stop_Asp(void);
	extern DWORD Save_Asp(void);
	extern DWORD DO_SetSamplingSource(DWORD dwSrc);
#ifdef __cplusplus
}
#endif

extern DWORD GetAspPerData(BYTE byChNo, RFW_ASPPER_DATA *pAspPerformanceData);
extern void ClrAspPerData(BYTE byChNo);
extern DWORD Start_Asp(void);
extern void Stop_Asp(void);
extern DWORD Save_Asp(void);
extern DWORD DO_SetSamplingSource(DWORD dwSrc);
extern void EnablePwrMon(void);
extern void ForcePwrMon(DWORD dwSampCh);
extern DWORD GetForcePwrMonValue(DWORD dwChNo);
extern void SetEventLink(void);
extern void SetExTrgIn(BYTE byChNo, BYTE byDetectType);
extern void ClrExTrgIn(BYTE byChNo);
extern void SetExTrgOut(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut);
extern void ClrExTrgOut(BYTE byChNo);
extern void SetExTrgOutLevel(BYTE byChNo, BYTE byActiveLevel);
extern void EnablePerformance(void);
extern void EnableCanMon(void);
extern void EnableExtTrgIn(void);

#endif
