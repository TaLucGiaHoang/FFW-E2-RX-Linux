///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_log.cpp
 * @brief
 * @author S.Nagai
 * @author Copyright (C) 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
	新規作成
*/
#include "rfw_typedef.h"
#include "rfwasp_family.h"

#if 0
extern "C" {
	int		DebugMsgFlgChk(ULONG);
	void	IniMsgBuf_RFW(int, DWORD);
	void	PostMsgBuf(void);
	void	SetMsgBufData1(int, USHORT, UINT64);
	void    SetMsgBufDataX(
	    int     DataNameId,
	    USHORT  DataType,
	    void *  DataP,
	    int     DataNum);
}

// マクロ定義
#define ASP_FUNC_GR		0x00008000	// 拡張機能関連I/F
#define CHARTYPE		1			/* 0000 0001    byte */
#define INTTYPE			2			/* 0000 0010    int */
#define LONGTYPE		4			/* 0000 0100    long */
#define UINT64TYPE		8			/* 0000 1000    uint64 */
#define STRTYPE			9
#define LONGTYPE_H_D	0xc4		/* 1100 0100    long */
#endif
// static変数

// static関数

#define LOGENA 0
#pragma warning(disable:4100)

//=============================================================================
/**
 * @brief ログ格納helper
 * @param msgid 名称ID
 * @param byData ログデータ
 */
//=============================================================================
void SetMsgBuf(int msgid, BYTE byData)
{
#if LOGENA
	SetMsgBufData1(msgid, CHARTYPE, (UINT64) byData);
#endif
}

//=============================================================================
/**
 * @brief ログ格納helper
 * @param msgid 名称ID
 * @param wData ログデータ
 */
//=============================================================================
void SetMsgBuf(int msgid, WORD wData)
{
#if LOGENA
	SetMsgBufData1(msgid, INTTYPE, (UINT64) wData);
#endif
}

//=============================================================================
/**
 * @brief ログ格納helper
 * @param msgid 名称ID
 * @param dwData ログデータ
 */
//=============================================================================
void SetMsgBuf(int msgid, DWORD dwData)
{
#if LOGENA
	SetMsgBufData1(msgid, LONGTYPE, (UINT64) dwData);
#endif
}

//=============================================================================
/**
 * @brief ログ格納helper
 * @param msgid 名称ID
 * @param u64Data ログデータ
 */
//=============================================================================
void SetMsgBuf(int msgid, UINT64 u64Data)
{
#if LOGENA
	SetMsgBufData1(msgid, UINT64TYPE, u64Data);
#endif
}

//=============================================================================
/**
 * @brief ログ格納helper
 * @param msgid 名称ID
 * @param u64Data ログデータ
 */
//=============================================================================
void SetMsgBuf(int msgid, INT64 i64Data)
{
#if LOGENA
	SetMsgBufData1(msgid, UINT64TYPE, i64Data);
#endif
}

//=============================================================================
/**
 * @brief ログ格納helper
 * @param msgid 名称ID
 * @param p ログデータ
 */
//=============================================================================
void SetMsgBuf(int msgid, const char* p)
{
#if LOGENA
	SetMsgBufDataX(msgid, STRTYPE, (void*)p, 0);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// RFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////


#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetAspSwitch_log_enter(BYTE byAspSwitch)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetAspSwitch_log_leave(BYTE byAspSwitch, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETASPSW, dwRet);

	SetMsgBuf(587, byAspSwitch);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetAspSwitch_log_enter(BYTE *const pbyAspSwitch)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetAspSwitch_log_leave(BYTE *const pbyAspSwitch, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETASPSW, dwRet);

	SetMsgBuf(588, *pbyAspSwitch);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetAspFunction_log_enter(DWORD *const pdwAspComonFunction, DWORD *const pdwAspSeriesFunction, DWORD *const pdwAspStorage, DWORD *const pdwAspOption)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetAspFunction_log_leave(DWORD *const pdwAspComonFunction, DWORD *const pdwAspSeriesFunction, DWORD *const pdwAspStorage, DWORD *const pdwAspOption, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETASPFUNC, dwRet);

	if (dwRet == RFWERR_OK) {
		SetMsgBuf(589, *pdwAspComonFunction);
		SetMsgBuf(590, *pdwAspSeriesFunction);
		SetMsgBuf(591, *pdwAspStorage);
		SetMsgBuf(592, *pdwAspOption);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetAspConfiguration_log_enter(const RFW_ASPCONF_DATA* pAspConfiguration)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetAspConfiguration_log_leave(const RFW_ASPCONF_DATA* pAspConfiguration, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETASPCONFIG, dwRet);

	if (pAspConfiguration) {
		SetMsgBuf(593, pAspConfiguration->dwStorage);
		SetMsgBuf(594, pAspConfiguration->dwCnt);
		SetMsgBuf(595, pAspConfiguration->dwMaxStorage);
		SetMsgBuf(596, pAspConfiguration->dwFunction0);
		SetMsgBuf(597, pAspConfiguration->dwFunction1);
		SetMsgBuf(598, pAspConfiguration->dwFunction2);
		SetMsgBuf(599, pAspConfiguration->dwFunction3);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetAspConfiguration_log_enter(RFW_ASPCONF_DATA const* pAspConfiguration)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetAspConfiguration_log_leave(RFW_ASPCONF_DATA const* pAspConfiguration, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETASPCONFIG, dwRet);

	if (dwRet == RFWERR_OK) {
		SetMsgBuf(600, pAspConfiguration->dwStorage);
		SetMsgBuf(601, pAspConfiguration->dwCnt);
		SetMsgBuf(602, pAspConfiguration->dwMaxStorage);
		SetMsgBuf(603, pAspConfiguration->dwFunction0);
		SetMsgBuf(604, pAspConfiguration->dwFunction1);
		SetMsgBuf(605, pAspConfiguration->dwFunction2);
		SetMsgBuf(606, pAspConfiguration->dwFunction3);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetSamplingSource_log_enter(DWORD dwSrc)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetSamplingSource_log_leave(DWORD dwSrc, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETSAMPSRC, dwRet);

	SetMsgBuf(607, dwSrc);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetSaveFileName_log_enter(const char *fname)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetSaveFileName_log_leave(const char *fname, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETSAVEFILENAME, dwRet);

	SetMsgBuf(629, fname);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SaveFile_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SaveFile_log_leave(DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SAVEFILE, dwRet);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_LoadSample_log_enter(const char *fname)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_LoadSample_log_leave(const char *fname, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_LOADSAMPLE, dwRet);

	SetMsgBuf(630, fname);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_UnLoadSample_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_UnLoadSample_log_leave(DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_UNLOADSAMPLE, dwRet);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetSample_log_enter(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, void *pData, DWORD dwSize, DWORD *pdwStoreCnt, DWORD *pdwExistCnt, UINT64 *pu64Read)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetSample_log_leave(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, void *pData, DWORD dwSize, DWORD *pdwStoreCnt, DWORD *pdwExistCnt, UINT64 *pu64Read, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETSAMP, dwRet);

	SetMsgBuf(612, dwType);
	SetMsgBuf(613, dwKeyType);
	SetMsgBuf(614, u64Start);
	SetMsgBuf(615, u64End);
	SetMsgBuf(616, (UINT64)pData);
	SetMsgBuf(617, dwSize);
	if (pdwStoreCnt) {
		SetMsgBuf(618, *pdwStoreCnt);
	}
	if (pdwExistCnt) {
		SetMsgBuf(619, *pdwExistCnt);
	}
	if (pu64Read) {
		SetMsgBuf(620, *pu64Read);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if 0
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetRealTimeBufferSize_log_enter(DWORD dwSize)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetRealTimeBufferSize_log_leave(DWORD dwSize, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETBUFSIZE, dwRet);

	SetMsgBuf(617, dwSize);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetRealTimeSamplingType_log_enter(DWORD dwType)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetRealTimeSamplingType_log_leave(DWORD dwType, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETSAMPTYPE, dwRet);

	SetMsgBuf(621, dwType);

	PostMsgBuf();
#endif
	return dwRet;
}
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_StopSample_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_StopSample_log_leave(DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_STOPSAMP, dwRet);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_RestartSample_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_RestartSample_log_leave(DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_RESTARTSAMP, dwRet);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetAvailableTime_log_enter(DWORD dwType, UINT64 *const pu64Start, UINT64 *const pu64End, UINT64 *const pu64Sample)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetAvailableTime_log_leave(DWORD dwType, UINT64 *const pu64Start, UINT64 *const pu64End, UINT64 *const pu64Sample, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETAVAILTIME, dwRet);

	SetMsgBuf(608, dwType);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(609, *pu64Start);
		SetMsgBuf(610, *pu64End);
		SetMsgBuf(611, *pu64Sample);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetSampleDataSize_log_enter(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, UINT64 *const pu64DataSize)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetSampleDataSize_log_leave(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, UINT64 *const pu64DataSize, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETSAMPSIZE, dwRet);

	SetMsgBuf(621, dwType);
	SetMsgBuf(622, dwKeyType);
	SetMsgBuf(623, u64Start);
	SetMsgBuf(624, u64End);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(625, *pu64DataSize);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if 0
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetSampleRealTime_log_enter(DWORD dwType, DWORD dwKeyType, UINT64 u64Back, void *pData, DWORD dwSize, DWORD *const pdwStoreCnt, DWORD *const pdwExistCnt)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetSampleRealTime_log_leave(DWORD dwType, DWORD dwKeyType, UINT64 u64Back, void *pData, DWORD dwSize, DWORD *const pdwStoreCnt, DWORD *const pdwExistCnt, DWORD dwRet)
{
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETSAMPNOW, dwRet);

	SetMsgBuf(820, dwType);
	SetMsgBuf(821, dwKeyType);
	SetMsgBuf(822, u64Back);
	SetMsgBuf(823, (UINT64)pData);
	SetMsgBuf(824, dwSize);
	if (dwRet == RFWERR_OK && pdwStoreCnt) {
		SetMsgBuf(825, *pdwStoreCnt);
	}
	if (dwRet == RFWERR_OK && pdwExistCnt) {
		SetMsgBuf(825, *pdwExistCnt);
	}

	PostMsgBuf();
	return dwRet;
}
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetSampleState_log_enter(DWORD *const pdwState, UINT64 *const pu64Min, UINT64 *const pu64Max)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetSampleState_log_leave(DWORD *const pdwState, UINT64 *const pu64Min, UINT64 *const pu64Max, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETSAMPSTAT, dwRet);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(626, *pdwState);
		SetMsgBuf(627, *pu64Min);
		SetMsgBuf(628, *pu64Max);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetPowerMonitor_log_enter(const RFW_PWRMONSAMP_DATA* pPowerMonitor)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetPowerMonitor_log_leave(const RFW_PWRMONSAMP_DATA* pPowerMonitor, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETPWRMON, dwRet);

	if (pPowerMonitor) {
		SetMsgBuf(723, pPowerMonitor->bySamplingRate);
		SetMsgBuf(724, pPowerMonitor->bySamplingMode);
		SetMsgBuf(725, pPowerMonitor->u64FactorSingleEvent);
		SetMsgBuf(726, pPowerMonitor->dwFactorMultiEvent);
		SetMsgBuf(727, pPowerMonitor->wFilterMin);
		SetMsgBuf(728, pPowerMonitor->wFilterMax);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetPowerMonitor_log_enter(RFW_PWRMONSAMP_DATA *const pPowerMonitor)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetPowerMonitor_log_leave(RFW_PWRMONSAMP_DATA *const pPowerMonitor, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETPWRMON, dwRet);

	if (dwRet == RFWERR_OK) {
		SetMsgBuf(729, pPowerMonitor->bySamplingRate);
		SetMsgBuf(730, pPowerMonitor->bySamplingMode);
		SetMsgBuf(731, pPowerMonitor->u64FactorSingleEvent);
		SetMsgBuf(732, pPowerMonitor->dwFactorMultiEvent);
		SetMsgBuf(733, pPowerMonitor->wFilterMin);
		SetMsgBuf(734, pPowerMonitor->wFilterMax);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrPowerMonitor_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrPowerMonitor_log_leave(DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLRPWRMON, dwRet);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetPowerMonitorEvent_log_enter(const RFW_PWRMONEV_DATA* pPowerMonitorEvent)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetPowerMonitorEvent_log_leave(const RFW_PWRMONEV_DATA* pPowerMonitorEvent, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETPWRMONEV, dwRet);

	if (pPowerMonitorEvent) {
		SetMsgBuf(735, pPowerMonitorEvent->byDetectType);
		SetMsgBuf(736, pPowerMonitorEvent->wPowerMin);
		SetMsgBuf(737, pPowerMonitorEvent->wPowerMax);
		SetMsgBuf(738, pPowerMonitorEvent->wPulseWidthMin);
		SetMsgBuf(739, pPowerMonitorEvent->wPulseWidthMax);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetPowerMonitorEvent_log_enter(RFW_PWRMONEV_DATA *const pPowerMonitorEvent)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetPowerMonitorEvent_log_leave(RFW_PWRMONEV_DATA *const pPowerMonitorEvent, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETPWRMONEV, dwRet);

	if (dwRet == RFWERR_OK) {
		SetMsgBuf(740, pPowerMonitorEvent->byDetectType);
		SetMsgBuf(741, pPowerMonitorEvent->wPowerMin);
		SetMsgBuf(742, pPowerMonitorEvent->wPowerMax);
		SetMsgBuf(743, pPowerMonitorEvent->wPulseWidthMin);
		SetMsgBuf(744, pPowerMonitorEvent->wPulseWidthMax);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrPowerMonitorEvent_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrPowerMonitorEvent_log_leave(DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLRPWRMONEV, dwRet);

	PostMsgBuf();
#endif
	return dwRet;
}



#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetPowerMonitorDataRange_log_enter(DWORD *const pdwUAmin, DWORD *const pdwUAmax)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetPowerMonitorDataRange_log_leave(DWORD *const pdwUAmin, DWORD *const pdwUAmax, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETPWRMONRNG, dwRet);

	if (dwRet == RFWERR_OK) {
		SetMsgBuf(998, *pdwUAmin); //998,999は適当に入れている数値
		SetMsgBuf(999, *pdwUAmax); //→後で要調整
	}

	PostMsgBuf();
#endif
	return dwRet;
}



#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetExTriggerIn_log_enter(BYTE byChNo, BYTE byDetectType)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetExTriggerIn_log_leave(BYTE byChNo, BYTE byDetectType, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETEXTRGIN, dwRet);

	SetMsgBuf(633, byChNo);
	SetMsgBuf(634, byDetectType);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetExTriggerIn_log_enter(BYTE byChNo, BYTE *const pbyDetectType)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetExTriggerIn_log_leave(BYTE byChNo, BYTE *const pbyDetectType, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETEXTRGIN, dwRet);

	SetMsgBuf(635, byChNo);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(636, *pbyDetectType);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrExTriggerIn_log_enter(BYTE byChNo)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrExTriggerIn_log_leave(BYTE byChNo, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLREXTRGIN, dwRet);
	SetMsgBuf(637, byChNo);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetExTriggerOut_log_enter(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetExTriggerOut_log_leave(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETEXTRGOUT, dwRet);

	SetMsgBuf(638, byChNo);
	if (pExTriggerOut) {
		SetMsgBuf(639, pExTriggerOut->byOutputType);
		SetMsgBuf(640, pExTriggerOut->wPulseWidth);
		SetMsgBuf(641, pExTriggerOut->u64FactorSingleEvent);
		SetMsgBuf(642, pExTriggerOut->dwFactorMultiEvent);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetExTriggerOut_log_enter(BYTE byChNo, RFW_EXTRGOUT_DATA *const pExTriggerOut)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetExTriggerOut_log_leave(BYTE byChNo, RFW_EXTRGOUT_DATA *const pExTriggerOut, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETEXTRGOUT, dwRet);

	SetMsgBuf(643, byChNo);
	if (pExTriggerOut) {
		SetMsgBuf(644, pExTriggerOut->byOutputType);
		SetMsgBuf(645, pExTriggerOut->wPulseWidth);
		SetMsgBuf(646, pExTriggerOut->u64FactorSingleEvent);
		SetMsgBuf(647, pExTriggerOut->dwFactorMultiEvent);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrExTriggerOut_log_enter(BYTE byChNo)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrExTriggerOut_log_leave(BYTE byChNo, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLREXTRGOUT, dwRet);

	SetMsgBuf(648, byChNo);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetExTriggerOutLevel_log_enter(BYTE byChNo, BYTE byActiveLevel)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetExTriggerOutLevel_log_leave(BYTE byChNo, BYTE byActiveLevel, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETEXTRGOUTLV, dwRet);

	SetMsgBuf(747, byChNo);
	SetMsgBuf(748, byActiveLevel);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetExTriggerOutLevel_log_enter(BYTE byChNo, BYTE *const pbyActiveLevel)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetExTriggerOutLevel_log_leave(BYTE byChNo, BYTE *const pbyActiveLevel, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETEXTRGOUTLV, dwRet);

	SetMsgBuf(749, byChNo);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(750, *pbyActiveLevel);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetPerformanceInfo_log_enter(BYTE byChNo, const RFW_ASPPERCOND_DATA* pAspPerformance)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetPerformanceInfo_log_leave(BYTE byChNo, const RFW_ASPPERCOND_DATA* pAspPerformance, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETPERINFO, dwRet);

	SetMsgBuf(694, byChNo);
	if (pAspPerformance) {
		SetMsgBuf(695, pAspPerformance->byMode);
		SetMsgBuf(696, pAspPerformance->u64StartEvent);
		SetMsgBuf(697, pAspPerformance->u64EndEvent);
		SetMsgBuf(698, pAspPerformance->u64SectionEvent);
		SetMsgBuf(699, pAspPerformance->u64Threshold);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetPerformanceInfo_log_enter(BYTE byChNo, RFW_ASPPERCOND_DATA *const pAspPerformance)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetPerformanceInfo_log_leave(BYTE byChNo, RFW_ASPPERCOND_DATA *const pAspPerformance, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETPERINFO, dwRet);

	SetMsgBuf(700, byChNo);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(701, pAspPerformance->byMode);
		SetMsgBuf(702, pAspPerformance->u64StartEvent);
		SetMsgBuf(703, pAspPerformance->u64EndEvent);
		SetMsgBuf(704, pAspPerformance->u64SectionEvent);
		SetMsgBuf(705, pAspPerformance->u64Threshold);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrPerformanceInfo_log_enter(BYTE byChNo)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrPerformanceInfo_log_leave(BYTE byChNo, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLRPERINFO, dwRet);

	SetMsgBuf(706, byChNo);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetPerformanceData_log_enter(BYTE byChNo, RFW_ASPPER_DATA *const pAspPerformanceData)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetPerformanceData_log_leave(BYTE byChNo, RFW_ASPPER_DATA *const pAspPerformanceData, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETPERDATA, dwRet);

	SetMsgBuf(707, byChNo);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(708, pAspPerformanceData->u64MinCount);
		SetMsgBuf(709, pAspPerformanceData->u64MaxCount);
		SetMsgBuf(710, pAspPerformanceData->dwPassCount);
		SetMsgBuf(711, pAspPerformanceData->u64SumCount);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrPerformanceData_log_enter(BYTE byChNo)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrPerformanceData_log_leave(BYTE byChNo, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLRPERDATA, dwRet);

	SetMsgBuf(712, byChNo);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetCANMonitor_log_enter(BYTE byChNo, const RFW_CANMON_DATA* pCanMonitor)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetCANMonitor_log_leave(BYTE byChNo, const RFW_CANMON_DATA* pCanMonitor, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETCANMON, dwRet);

	SetMsgBuf(649, byChNo);
	if (pCanMonitor) {
		SetMsgBuf(650, pCanMonitor->byCanMode);
		SetMsgBuf(651, pCanMonitor->wBaudrate);
		SetMsgBuf(652, pCanMonitor->wFastBaudrate);
		SetMsgBuf(653, pCanMonitor->bySamplingPoint);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetCANMonitor_log_enter(BYTE byChNo, RFW_CANMON_DATA *const pCanMonitor)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetCANMonitor_log_leave(BYTE byChNo, RFW_CANMON_DATA *const pCanMonitor, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETCANMON, dwRet);
	SetMsgBuf(654, byChNo);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(655, pCanMonitor->byCanMode);
		SetMsgBuf(656, pCanMonitor->wBaudrate);
		SetMsgBuf(657, pCanMonitor->wFastBaudrate);
		SetMsgBuf(658, pCanMonitor->bySamplingPoint);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrCANMonitor_log_enter(BYTE byChNo)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrCANMonitor_log_leave(BYTE byChNo, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLRCANMON, dwRet);
	SetMsgBuf(659, byChNo);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetCANMonitorEvent_log_enter(BYTE byChNo, const RFW_CANMONEV_DATA* pCanMonitorEvent)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetCANMonitorEvent_log_leave(BYTE byChNo, const RFW_CANMONEV_DATA* pCanMonitorEvent, DWORD dwRet)
{
#if LOGENA
	RFW_CANMONEV_DATA CanMonEv;

	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETCANMONEV, dwRet);

	SetMsgBuf(660, byChNo);
	if (pCanMonitorEvent) {
		memcpy((void*)&CanMonEv, pCanMonitorEvent, sizeof(RFW_CANMONEV_DATA));

		SetMsgBuf(661, pCanMonitorEvent->byDetectType);
		SetMsgBuf(662, pCanMonitorEvent->byDelayCount);
		SetMsgBuf(663, pCanMonitorEvent->byCanMode);
		SetMsgBuf(664, pCanMonitorEvent->byDLC);
		SetMsgBuf(665, pCanMonitorEvent->byRTR);
		SetMsgBuf(666, pCanMonitorEvent->bySRR);
		SetMsgBuf(667, pCanMonitorEvent->dwID);
		SetMsgBufDataX(668, LONGTYPE_H_D, CanMonEv.dwData, 16);
		SetMsgBuf(669, pCanMonitorEvent->dwIDMask);
		SetMsgBufDataX(670, LONGTYPE_H_D, CanMonEv.dwMask, 16);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetCANMonitorEvent_log_enter(BYTE byChNo, RFW_CANMONEV_DATA *const pCanMonitorEvent)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetCANMonitorEvent_log_leave(BYTE byChNo, RFW_CANMONEV_DATA *const pCanMonitorEvent, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETCANMONEV, dwRet);

	SetMsgBuf(671, byChNo);
	if (dwRet == RFWERR_OK) {
		SetMsgBuf(672, pCanMonitorEvent->byDetectType);
		SetMsgBuf(673, pCanMonitorEvent->byDelayCount);
		SetMsgBuf(674, pCanMonitorEvent->byCanMode);
		SetMsgBuf(675, pCanMonitorEvent->byDLC);
		SetMsgBuf(676, pCanMonitorEvent->byRTR);
		SetMsgBuf(677, pCanMonitorEvent->bySRR);
		SetMsgBuf(678, pCanMonitorEvent->dwID);
		SetMsgBufDataX(679, LONGTYPE_H_D, pCanMonitorEvent->dwData, 16);
		SetMsgBuf(680, pCanMonitorEvent->dwIDMask);
		SetMsgBufDataX(681, LONGTYPE_H_D, pCanMonitorEvent->dwMask, 16);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrCANMonitorEvent_log_enter(BYTE byChNo)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrCANMonitorEvent_log_leave(BYTE byChNo, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLRCANMONEV, dwRet);

	SetMsgBuf(682, byChNo);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetBreakEvent_log_enter(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetBreakEvent_log_leave(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETBRKEV, dwRet);

	SetMsgBuf(713, u64FactorSingleEvent);
	SetMsgBuf(714, dwFactorMultiEvent);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetBreakEvent_log_enter(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetBreakEvent_log_leave(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETBRKEV, dwRet);

	if (dwRet == RFWERR_OK) {
		SetMsgBuf(715, *pu64FactorSingleEvent);
		SetMsgBuf(716, *pdwFactorMultiEvent);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrBreakEvent_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrBreakEvent_log_leave(DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLRBRKEV, dwRet);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetStopTraceEvent_log_enter(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetStopTraceEvent_log_leave(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETSTOPTRCEV, dwRet);

	SetMsgBuf(718, u64FactorSingleEvent);
	SetMsgBuf(719, dwFactorMultiEvent);

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetStopTraceEvent_log_enter(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetStopTraceEvent_log_leave(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_GETSTOPTRCEV, dwRet);

	if (dwRet == RFWERR_OK) {
		SetMsgBuf(720, *pu64FactorSingleEvent);
		SetMsgBuf(721, *pdwFactorMultiEvent);
	}

	PostMsgBuf();
#endif
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_ClrStopTraceEvent_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_ClrStopTraceEvent_log_leave(DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_CLRSTOPTRCEV, dwRet);

	PostMsgBuf();
#endif
	return dwRet;
}

#if 0
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_GetTargetState_log_enter(DWORD *const pdwState)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_GetTargetState_log_leave(DWORD *const pdwState, DWORD dwRet)
{
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_XXX, dwRet);

	SetMsgBufData1(685, CHARTYPE, byChNo);

	PostMsgBuf();
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_UartOpen_log_enter(DWORD dwRate, BYTE bySize, BYTE byParityType, BYTE byStopBits)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_UartOpen_log_leave(DWORD dwRate, BYTE bySize, BYTE byParityType, BYTE byStopBits, DWORD dwRet)
{
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_XXX, dwRet);

	SetMsgBufData1(685, CHARTYPE, byChNo);

	PostMsgBuf();
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_UartClose_log_enter(void)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_UartClose_log_leave(DWORD dwRet)
{
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_XXX, dwRet);

	SetMsgBufData1(685, CHARTYPE, byChNo);

	PostMsgBuf();
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_UartSend_log_enter(WORD wLen, const BYTE *pbyData)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_UartSend_log_leave(WORD wLen, const BYTE *pbyData, DWORD dwRet)
{
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_XXX, dwRet);

	SetMsgBufData1(685, CHARTYPE, byChNo);

	PostMsgBuf();
	return dwRet;
}

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_UartReceive_log_enter(WORD wLen, BYTE *pbyData)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_UartReceive_log_leave(WORD wLen, BYTE *pbyData, DWORD dwRet)
{
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_XXX, dwRet);

	SetMsgBufData1(685, CHARTYPE, byChNo);

	PostMsgBuf();
	return dwRet;
}
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif
DWORD RFWASP_SetClkCalibration_log_enter(DWORD dwType, INT64 i64Calib)
{
	return RFWERR_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

DWORD RFWASP_SetClkCalibration_log_leave(DWORD dwType, INT64 i64Calib, DWORD dwRet)
{
#if LOGENA
	if (!DebugMsgFlgChk(ASP_FUNC_GR)) {
		return RFWERR_OK;
	}
	IniMsgBuf_RFW(ASPCMD_SETCLKCALIBRATION, dwRet);

	SetMsgBuf(745, dwType);
	SetMsgBuf(746, i64Calib);

	PostMsgBuf();
#endif
	return dwRet;
}


