///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_sys_family.h
 * @brief
 * @author S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴

*/
#ifndef	__DOASP_SYS_FAMILY_H__
#define	__DOASP_SYS_FAMILY_H__

#include "rfw_typedef.h"
#include "mcudef.h"


// define定義
#define RF_ASPFACT_SUPORT_RH 0x0000000000F0114F
// bit0		時間計測ch0				サポート
// bit1		時間計測ch1				サポート
// bit2		外部トリガ入力ch0		サポート
// bit3		外部トリガ入力ch1		サポート
// bit4		RESETZ端子の監視		未
// bit5		EVTOZ端子の監視			未
// bit6		通信モニタch0			サポート
// bit7		予約					未
// bit8		通信モニタch1			サポート
// bit9		予約					未
// bit10	パワーモニタ			未
// bit11	パワーモニタ			未
// bit12	ステータス				サポート
// bit13	モニタイベントch0		未
// bit14	モニタイベントch1		未
// bit15	モニタイベントch2		未
// bit16	モニタイベントch3		未
// bit17	モニタイベントch4		未
// bit18	モニタイベントch5		未
// bit19	モニタイベント			未
// bit20	外部ソフトトレースch0	サポート
// bit21	外部ソフトトレースch1	サポート
// bit22	外部ソフトトレースch2	サポート
// bit23	外部ソフトトレースch3	サポート
// bit24-63	

#define RF_ASPFUNC_MONEV	0
#define RF_ASPFUNC_STRACE	1


// グローバル関数の宣言
extern void ProtInit_ASP(void);
extern DWORD ProtEnd_ASP(void);
extern DWORD CmdExecutCheck_ASP(BYTE byCommandID);
extern DWORD EndOfCmdExecut_ASP(BYTE byCommandID);
extern DWORD TransErrExec2Rfw(int nExecErr);
extern BOOL IsEmlE2(void);
extern int SendBlock_ASP(const BYTE* pData, DWORD size);
extern DWORD GetAspCommonFunc(void);
extern DWORD GetAspSeriesFunc(void);
extern DWORD GetAspStorage(void);
extern BOOL IsStatRun(void);
extern BOOL IsAsyncMode(void);
extern BOOL IsSupplyPower(void);
extern BOOL IsSupplyPowerASP(void);
extern UINT64 GetAspSeriesFact(void);
extern void Start_ASP_family(void);
extern void Stop_ASP_family(void);
extern void AspRun_family(void);
extern void InitAsp_family(void);
extern BOOL IsSramFullStat_family(void);
extern BOOL IsSaveSramRawData(void);
extern BOOL IsFamilyAspFuncEna(BYTE byFunc);
extern BOOL IsPwrMonCorrectValEna(void);
extern BOOL IsPwrMonCorrectThreasholdEna(void);
extern BYTE GetE2SupplyPwrVal(void);
extern void Reset_Asp_family(void);
extern DWORD GetAspDelay_family(void);
extern void AspPerDataCalibration(RFW_ASPPER_DATA AspPerformanceDataRaw, RFW_ASPPER_DATA* pAspPerformanceData, UINT64 u64StartEvent, UINT64 u64EndEvent, UINT64 u64LevelEvent);
extern void AspPerThresholdCalibration(UINT64 u64ThresholdRaw, UINT64* pu64Threshold, UINT64 u64StartEvent, UINT64 u64EndEvent, UINT64 u64LevelEvent);
extern RFWERR checkGetAvailableTime(DWORD dwType);
extern RFWERR checkGetSample(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End);
extern RFWERR checkGetSampleDataSize(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End);
extern RFWERR checkSetClkCalibration(DWORD dwType, INT64 i64Calib);
extern RFWERR checkGetSampleRealTime(DWORD dwType, DWORD dwKeyType, UINT64 u64Back);



// cppファイルで使用する関数のextern
#ifdef __cplusplus
extern "C" {
#endif
	int		PutCmd(WORD wCmdCode);
	int		PutData4(DWORD dwData);
	int		PutData1(BYTE byData);
	int		GetData2(WORD *const pwData);
	int		GetData4(DWORD *const pdwData);
	int		GetDataN_H(DWORD dwSize, BYTE *const pbyReadData);
	int		FlashProtDataForced(void);
	// ULONG	ProtGetStatus(WORD wCmdCode);
	FFWERR	PROT_BATEND_RecvErrCodeReservData(WORD wCmdCode);
	// FFWERR	ProtRcvHaltCode(WORD *const pwBuf);
	BOOL	usb_ServerSend(unsigned char *sbuf, int spacksiz, DWORD stimeout);
	USHORT	GetIceIfInf(void);
	// void	ProtInit(void);
	// FFWERR	ProtEnd(void);
	int		CmdExecutCheck(unsigned char);
	int		EndOfCmdExecut(unsigned char, int);
	int 	COM_VendorControlIn(
	    BYTE req, WORD val, WORD index, BYTE *pbyIn, WORD len);
	int 	COM_VendorControlOut(
	    BYTE req, WORD val, WORD index, const BYTE *pbyOut, WORD len);
	BOOL IsE2FullBrkMode(void);
	BOOL IsE2FullStat(void);
	BOOL IsReleaseBrkCmd(void);
	unsigned long  get_lpdoptinfodata( int id);
#ifdef __cplusplus
}
#endif

// Cファイルで定義しているdefineの再定義
#define ICEIF_E2 0x0002


#endif	// __DOASP_SYS_FAMILY_H__
