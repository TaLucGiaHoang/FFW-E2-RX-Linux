///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_family.h
 * @brief
 * @author S.Nagai
 * @author Copyright (C) 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴

*/
#ifndef	__RFWASP_FAMILY_H__
#define	__RFWASP_FAMILY_H__

#include "rfw_typedef.h"
#include "mcudef.h"


// グローバル関数の宣言
// ログ関数の宣言
extern DWORD RFWASP_SetSWTraceEvent_log0(BYTE byChNo, const RFW_SWTRC_DATA* pSwTrcData);
extern DWORD RFWASP_SetSWTraceEvent_log1(BYTE byChNo, const RFW_SWTRC_DATA* pSwTrcData, DWORD dwRet);
extern DWORD RFWASP_GetSWTraceEvent_log0(BYTE byChNo);
extern DWORD RFWASP_GetSWTraceEvent_log1(BYTE byChNo, RFW_SWTRC_DATA* pSwTrcData, DWORD dwRet);
extern DWORD RFWASP_ClrSWTraceEvent_log0(BYTE byChNo);
extern DWORD RFWASP_ClrSWTraceEvent_log1(BYTE byChNo, DWORD dwRet);

extern DWORD RFW_AspIn_log_enter(DWORD dwReadByte, DWORD *const pbyReadBuff);
extern DWORD RFW_AspIn_log_leave(DWORD dwReadByte, DWORD *const pbyReadBuff, DWORD dwRet);
extern DWORD RFW_AspOut_log_enter(DWORD dwWriteByte, const DWORD* pbyWriteBuff);
extern DWORD RFW_AspOut_log_leave(DWORD dwWriteByte, const DWORD* pbyWriteBuff, DWORD dwRet);
extern DWORD RFW_VendorControlIn_log_enter(BYTE req, WORD val, WORD index, BYTE *pbyIn, WORD len);
extern DWORD RFW_VendorControlIn_log_leave(BYTE req, WORD val, WORD index, BYTE *pbyIn, WORD len, DWORD dwRet);
extern DWORD RFW_VendorControlOut_log_enter(BYTE req, WORD val, WORD index);
extern DWORD RFW_VendorControlOut_log_leave(BYTE req, WORD val, WORD index, DWORD dwRet);
extern DWORD RFWASP_SetAspSwitch_log_enter(BYTE byAspSwitch);
extern DWORD RFWASP_SetAspSwitch_log_leave(BYTE byAspSwitch, DWORD dwRet);
extern DWORD RFWASP_GetAspSwitch_log_enter(BYTE *const pbyAspSwitch);
extern DWORD RFWASP_GetAspSwitch_log_leave(BYTE *const pbyAspSwitch, DWORD dwRet);
extern DWORD RFWASP_GetAspFunction_log_enter(DWORD *const pdwAspComonFunction, DWORD *const pdwAspSeriesFunction, DWORD *const pdwAspStorage, DWORD *const pdwAspOption);
extern DWORD RFWASP_GetAspFunction_log_leave(DWORD *const pdwAspComonFunction, DWORD *const pdwAspSeriesFunction, DWORD *const pdwAspStorage, DWORD *const pdwAspOption, DWORD dwRet);
extern DWORD RFWASP_SetAspConfiguration_log_enter(const RFW_ASPCONF_DATA* pAspConfiguration);
extern DWORD RFWASP_SetAspConfiguration_log_leave(const RFW_ASPCONF_DATA* pAspConfiguration, DWORD dwRet);
extern DWORD RFWASP_GetAspConfiguration_log_enter(RFW_ASPCONF_DATA const* pAspConfiguration);
extern DWORD RFWASP_GetAspConfiguration_log_leave(RFW_ASPCONF_DATA const* pAspConfiguration, DWORD dwRet);
extern DWORD RFWASP_SetSamplingSource_log_enter(DWORD dwSrc);
extern DWORD RFWASP_SetSamplingSource_log_leave(DWORD dwSrc, DWORD dwRet);
extern DWORD RFWASP_SetSaveFileName_log_enter(const char *fname);
extern DWORD RFWASP_SetSaveFileName_log_leave(const char *fname, DWORD dwRet);
extern DWORD RFWASP_SaveFile_log_enter(void);
extern DWORD RFWASP_SaveFile_log_leave(DWORD dwRet);
extern DWORD RFWASP_LoadSample_log_enter(const char *fname);
extern DWORD RFWASP_LoadSample_log_leave(const char *fname, DWORD dwRet);
extern DWORD RFWASP_UnLoadSample_log_enter(void);
extern DWORD RFWASP_UnLoadSample_log_leave(DWORD dwRet);
extern DWORD RFWASP_GetSample_log_enter(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, void *pData, DWORD dwSize, DWORD *pdwStoreCnt, DWORD *pdwExistCnt, UINT64 *pu64Read);
extern DWORD RFWASP_GetSample_log_leave(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, void *pData, DWORD dwSize, DWORD *pdwStoreCnt, DWORD *pdwExistCnt, UINT64 *pu64Read, DWORD dwRet);
extern DWORD RFWASP_SetRealTimeBufferSize_log_enter(DWORD dwSize);
extern DWORD RFWASP_SetRealTimeBufferSize_log_leave(DWORD dwSize, DWORD dwRet);
extern DWORD RFWASP_SetRealTimeSamplingType_log_enter(DWORD dwType);
extern DWORD RFWASP_SetRealTimeSamplingType_log_leave(DWORD dwType, DWORD dwRet);
extern DWORD RFWASP_StopSample_log_enter(void);
extern DWORD RFWASP_StopSample_log_leave(DWORD dwRet);
extern DWORD RFWASP_RestartSample_log_enter(void);
extern DWORD RFWASP_RestartSample_log_leave(DWORD dwRet);
extern DWORD RFWASP_GetAvailableTime_log_enter(DWORD dwType, UINT64 *const pu64Start, UINT64 *const pu64End, UINT64 *const pu64Sample);
extern DWORD RFWASP_GetAvailableTime_log_leave(DWORD dwType, UINT64 *const pu64Start, UINT64 *const pu64End, UINT64 *const pu64Sample, DWORD dwRet);
extern DWORD RFWASP_GetSampleDataSize_log_enter(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, UINT64 *const pu64DataSize);
extern DWORD RFWASP_GetSampleDataSize_log_leave(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, UINT64 *const pu64DataSize, DWORD dwRet);
extern DWORD RFWASP_GetSampleRealTime_log_enter(DWORD dwType, DWORD dwKeyType, UINT64 u64Back, void *pData, DWORD dwSize, DWORD *const pdwStoreCnt, DWORD *const pdwExistCnt);
extern DWORD RFWASP_GetSampleRealTime_log_leave(DWORD dwType, DWORD dwKeyType, UINT64 u64Back, void *pData, DWORD dwSize, DWORD *const pdwStoreCnt, DWORD *const pdwExistCnt, DWORD dwRet);
extern DWORD RFWASP_GetSampleState_log_enter(DWORD *const pdwState, UINT64 *const pu64Min, UINT64 *const pu64Max);
extern DWORD RFWASP_GetSampleState_log_leave(DWORD *const pdwState, UINT64 *const pu64Min, UINT64 *const pu64Max, DWORD dwRet);
extern DWORD RFWASP_SetPowerMonitor_log_enter(const RFW_PWRMONSAMP_DATA* pPowerMonitor);
extern DWORD RFWASP_SetPowerMonitor_log_leave(const RFW_PWRMONSAMP_DATA* pPowerMonitor, DWORD dwRet);
extern DWORD RFWASP_GetPowerMonitor_log_enter(RFW_PWRMONSAMP_DATA *const pPowerMonitor);
extern DWORD RFWASP_GetPowerMonitor_log_leave(RFW_PWRMONSAMP_DATA *const pPowerMonitor, DWORD dwRet);
extern DWORD RFWASP_ClrPowerMonitor_log_enter(void);
extern DWORD RFWASP_ClrPowerMonitor_log_leave(DWORD dwRet);
extern DWORD RFWASP_SetPowerMonitorEvent_log_enter(const RFW_PWRMONEV_DATA* pPowerMonitorEvent);
extern DWORD RFWASP_SetPowerMonitorEvent_log_leave(const RFW_PWRMONEV_DATA* pPowerMonitorEvent, DWORD dwRet);
extern DWORD RFWASP_GetPowerMonitorEvent_log_enter(RFW_PWRMONEV_DATA *const pPowerMonitorEvent);
extern DWORD RFWASP_GetPowerMonitorEvent_log_leave(RFW_PWRMONEV_DATA *const pPowerMonitorEvent, DWORD dwRet);
extern DWORD RFWASP_ClrPowerMonitorEvent_log_enter(void);
extern DWORD RFWASP_ClrPowerMonitorEvent_log_leave(DWORD dwRet);
extern DWORD RFWASP_SetExTriggerIn_log_enter(BYTE byChNo, BYTE byDetectType);
extern DWORD RFWASP_SetExTriggerIn_log_leave(BYTE byChNo, BYTE byDetectType, DWORD dwRet);
extern DWORD RFWASP_GetExTriggerIn_log_enter(BYTE byChNo, BYTE *const pbyDetectType);
extern DWORD RFWASP_GetExTriggerIn_log_leave(BYTE byChNo, BYTE *const pbyDetectType, DWORD dwRet);
extern DWORD RFWASP_ClrExTriggerIn_log_enter(BYTE byChNo);
extern DWORD RFWASP_ClrExTriggerIn_log_leave(BYTE byChNo, DWORD dwRet);
extern DWORD RFWASP_SetExTriggerOut_log_enter(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut);
extern DWORD RFWASP_SetExTriggerOut_log_leave(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut, DWORD dwRet);
extern DWORD RFWASP_GetExTriggerOut_log_enter(BYTE byChNo, RFW_EXTRGOUT_DATA *const pExTriggerOut);
extern DWORD RFWASP_GetExTriggerOut_log_leave(BYTE byChNo, RFW_EXTRGOUT_DATA *const pExTriggerOut, DWORD dwRet);
extern DWORD RFWASP_ClrExTriggerOut_log_enter(BYTE byChNo);
extern DWORD RFWASP_ClrExTriggerOut_log_leave(BYTE byChNo, DWORD dwRet);
extern DWORD RFWASP_SetExTriggerOutLevel_log_enter(BYTE byChNo, BYTE byActiveLevel);
extern DWORD RFWASP_SetExTriggerOutLevel_log_leave(BYTE byChNo, BYTE byActiveLevel, DWORD dwRet);
extern DWORD RFWASP_GetExTriggerOutLevel_log_enter(BYTE byChNo, BYTE *const pbyActiveLevel);
extern DWORD RFWASP_GetExTriggerOutLevel_log_leave(BYTE byChNo, BYTE *const pbyActiveLevel, DWORD dwRet);
extern DWORD RFWASP_SetPerformanceInfo_log_enter(BYTE byChNo, const RFW_ASPPERCOND_DATA* pAspPerformance);
extern DWORD RFWASP_SetPerformanceInfo_log_leave(BYTE byChNo, const RFW_ASPPERCOND_DATA* pAspPerformance, DWORD dwRet);
extern DWORD RFWASP_GetPerformanceInfo_log_enter(BYTE byChNo, RFW_ASPPERCOND_DATA *const pAspPerformance);
extern DWORD RFWASP_GetPerformanceInfo_log_leave(BYTE byChNo, RFW_ASPPERCOND_DATA *const pAspPerformance, DWORD dwRet);
extern DWORD RFWASP_ClrPerformanceInfo_log_enter(BYTE byChNo);
extern DWORD RFWASP_ClrPerformanceInfo_log_leave(BYTE byChNo, DWORD dwRet);
extern DWORD RFWASP_GetPerformanceData_log_enter(BYTE byChNo, RFW_ASPPER_DATA *const pAspPerformanceData);
extern DWORD RFWASP_GetPerformanceData_log_leave(BYTE byChNo, RFW_ASPPER_DATA *const pAspPerformanceData, DWORD dwRet);
extern DWORD RFWASP_ClrPerformanceData_log_enter(BYTE byChNo);
extern DWORD RFWASP_ClrPerformanceData_log_leave(BYTE byChNo, DWORD dwRet);
extern DWORD RFWASP_SetCANMonitor_log_enter(BYTE byChNo, const RFW_CANMON_DATA* pCanMonitor);
extern DWORD RFWASP_SetCANMonitor_log_leave(BYTE byChNo, const RFW_CANMON_DATA* pCanMonitor, DWORD dwRet);
extern DWORD RFWASP_GetCANMonitor_log_enter(BYTE byChNo, RFW_CANMON_DATA *const pCanMonitor);
extern DWORD RFWASP_GetCANMonitor_log_leave(BYTE byChNo, RFW_CANMON_DATA *const pCanMonitor, DWORD dwRet);
extern DWORD RFWASP_ClrCANMonitor_log_enter(BYTE byChNo);
extern DWORD RFWASP_ClrCANMonitor_log_leave(BYTE byChNo, DWORD dwRet);
extern DWORD RFWASP_SetCANMonitorEvent_log_enter(BYTE byChNo, const RFW_CANMONEV_DATA* pCanMonitorEvent);
extern DWORD RFWASP_SetCANMonitorEvent_log_leave(BYTE byChNo, const RFW_CANMONEV_DATA* pCanMonitorEvent, DWORD dwRet);
extern DWORD RFWASP_GetCANMonitorEvent_log_enter(BYTE byChNo, RFW_CANMONEV_DATA *const pCanMonitorEvent);
extern DWORD RFWASP_GetCANMonitorEvent_log_leave(BYTE byChNo, RFW_CANMONEV_DATA *const pCanMonitorEvent, DWORD dwRet);
extern DWORD RFWASP_ClrCANMonitorEvent_log_enter(BYTE byChNo);
extern DWORD RFWASP_ClrCANMonitorEvent_log_leave(BYTE byChNo, DWORD dwRet);
extern DWORD RFWASP_SetBreakEvent_log_enter(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent);
extern DWORD RFWASP_SetBreakEvent_log_leave(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent, DWORD dwRet);
extern DWORD RFWASP_GetBreakEvent_log_enter(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent);
extern DWORD RFWASP_GetBreakEvent_log_leave(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent, DWORD dwRet);
extern DWORD RFWASP_ClrBreakEvent_log_enter(void);
extern DWORD RFWASP_ClrBreakEvent_log_leave(DWORD dwRet);
extern DWORD RFWASP_SetStopTraceEvent_log_enter(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent);
extern DWORD RFWASP_SetStopTraceEvent_log_leave(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent, DWORD dwRet);
extern DWORD RFWASP_GetStopTraceEvent_log_enter(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent);
extern DWORD RFWASP_GetStopTraceEvent_log_leave(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent, DWORD dwRet);
extern DWORD RFWASP_ClrStopTraceEvent_log_enter(void);
extern DWORD RFWASP_ClrStopTraceEvent_log_leave(DWORD dwRet);
extern DWORD RFWASP_GetTargetState_log_enter(DWORD *const pdwState);
extern DWORD RFWASP_GetTargetState_log_leave(DWORD *const pdwState, DWORD dwRet);
extern DWORD RFWASP_UartOpen_log_enter(DWORD dwRate, BYTE bySize, BYTE byParityType, BYTE byStopBits);
extern DWORD RFWASP_UartOpen_log_leave(DWORD dwRate, BYTE bySize, BYTE byParityType, BYTE byStopBits, DWORD dwRet);
extern DWORD RFWASP_UartClose_log_enter(void);
extern DWORD RFWASP_UartClose_log_leave(DWORD dwRet);
extern DWORD RFWASP_UartSend_log_enter(WORD wLen, const BYTE *pbyData);
extern DWORD RFWASP_UartSend_log_leave(WORD wLen, const BYTE *pbyData, DWORD dwRet);
extern DWORD RFWASP_UartReceive_log_enter(WORD wLen, BYTE *pbyData);
extern DWORD RFWASP_UartReceive_log_leave(WORD wLen, BYTE *pbyData, DWORD dwRet);
extern DWORD RFWASP_SetClkCalibration_log_enter(DWORD dwType, INT64 i64Calib);
extern DWORD RFWASP_SetClkCalibration_log_leave(DWORD dwType, INT64 i64Calib, DWORD dwRet);

#endif	// __RFWASP_FAMILY_H__
