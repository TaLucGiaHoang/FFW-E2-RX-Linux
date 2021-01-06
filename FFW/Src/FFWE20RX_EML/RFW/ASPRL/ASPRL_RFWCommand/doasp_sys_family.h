///////////////////////////////////////////////////////////////////////////////
/**
* @file doasp_sys_family.h
* @brief E2拡張機能システムコマンド(MCUファミリ固有部)のヘッダファイル
* @author RSD M.Yamamoto
* @author Copyright (C) 2016 Renesas Electronics Corporation and
* @author Renesas System Design Co., Ltd. All rights reserved.
* @date 2016/12/09
*/
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/12/09 新規作成 M.Yamamoto
*/
#ifndef	__DOASP_SYS_FAMILY_H__
#define	__DOASP_SYS_FAMILY_H__

#include "rfw_typedef.h"
#include "mcudef.h"

#include "doasprl_monev.h"


// define定義

#define RF_ASPFACT_SUPORT_RL 0x00000000000FFC0F
// bit0		時間計測ch0				サポート
// bit1		時間計測ch1				サポート
// bit2		外部トリガ入力ch0		サポート
// bit3		外部トリガ入力ch1		サポート
// bit4		RESETZ端子の監視		未
// bit5		EVTOZ端子の監視			未
// bit6		通信モニタch0			未
// bit7		通信モニタch0			未
// bit8		通信モニタch1			未
// bit9		通信モニタch1			未
// bit10	パワーモニタ			サポート
// bit11	パワーモニタ			サポート
// bit12	ステータス				サポート
// bit13	モニタイベントch0		サポート
// bit14	モニタイベントch1		サポート
// bit15	モニタイベントch2		サポート
// bit16	モニタイベントch3		サポート
// bit17	モニタイベントch4		サポート
// bit18	モニタイベントch5		サポート
// bit19	モニタイベントch6		サポート
// bit20	外部ソフトトレースch0	未
// bit21	外部ソフトトレースch1	未
// bit22	外部ソフトトレースch2	未
// bit23	外部ソフトトレースch3	未
// bit24-63	


// グローバル関数の宣言


extern void ProtInit_ASP(void);
extern DWORD ProtEnd_ASP(void);


extern DWORD GetAspCommonFunc(void);
extern DWORD GetAspSeriesFunc(void);
extern DWORD GetAspStorage(void);
extern BOOL IsStatRun(void);
extern BOOL IsAsyncMode(void);
extern BOOL IsSupplyPower(void);
extern BOOL IsSupplyPowerASP(void);
extern UINT64 GetAspSeriesFact(void);

extern void InitAsp_family(void);

extern void AspRun_family(void);

extern void Start_ASP_family(void);
extern void Stop_ASP_family(void);
extern void DisableMonEv(void);
extern void DisableBrkEv(void);

extern BOOL IsSramFullStat_family(void);
extern BOOL IsPwrMonCorrectValEna(void);
extern BOOL IsPwrMonCorrectThreasholdEna(void);
extern void Reset_Asp_family(void);
extern DWORD GetAspDelay_family(void);
extern void AspPerDataCalibration(RFW_ASPPER_DATA AspPerformanceDataRaw, RFW_ASPPER_DATA* pAspPerformanceData, UINT64 u64StartEvent, UINT64 u64EndEvent, UINT64 u64LevelEvent);
extern void AspPerThresholdCalibration(UINT64 u64ThresholdRaw, UINT64* pu64Threshold, UINT64 u64StartEvent, UINT64 u64EndEvent, UINT64 u64LevelEvent);
extern RFWERR checkGetAvailableTime(DWORD dwType);
extern RFWERR checkGetSample(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End);
extern RFWERR checkGetSampleDataSize(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End);
extern RFWERR checkSetClkCalibration(DWORD dwType, INT64 i64Calib);
extern RFWERR checkGetSampleRealTime(DWORD dwType, DWORD dwKeyType, UINT64 u64Back);

#ifdef __cplusplus
extern "C" {
#endif

	int		PutCmd(WORD wCmdCode);
	int		PutData4(DWORD dwData);
	int		PutData1(BYTE byData);
	int		GetData2(WORD *const pwData);
	int		GetData4(DWORD *const pdwData);
	int		GetDataN_H(DWORD dwSize, BYTE *const pbyReadData);

	extern FFWERR ProtGetStatus(WORD wCmdCode);

	int FlashProtDataForced(void);
	extern FFWERR ProtRcvHaltCode(WORD *const pwBuf);	///< 処理中断コードの受信


	extern int e2_CPU_R(ULONG addr, ULONG len, UCHAR *data);
	extern int e2_CPU_W(ULONG addr, ULONG len, UCHAR *data);

	// Communi.DLL用
	extern int COM_Open(long, char *, long);
	extern int COM_Close(void);
	extern int SetCOMTimeOut(WORD);
	extern int COM_SendBlock(char *, long);
	extern int SetCOMTransferSize(DWORD);
	extern int COM_ReceiveBlockH(char *, long *);
	extern int COM_GetLastError(void);
	extern int COM_VendorControlIn(BYTE req, WORD val, WORD index, BYTE *pbyIn, WORD len);
	extern int COM_VendorControlOut(BYTE req, WORD val, WORD index, const BYTE *pbyOut, WORD len);

	extern BOOL IsEmlE2(void);
	extern BOOL usb_ServerSend(unsigned char *sbuf, int spacksiz, DWORD stimeout);
	extern FFWERR PROT_BATEND_RecvErrCodeReservData(WORD wCmdCode);
	extern int SendBlock_ASP(const BYTE* pData, DWORD size);
	extern BOOL IsSaveSramRawData(void);
	extern BYTE GetE2SupplyPwrVal(void);
	extern BOOL IsFamilyAspFuncEna(BYTE byFunc);
	extern void InitSramFullStopFlg(void);
#ifdef __cplusplus
}
#endif

#define RF_ASPFUNC_MONEV	0
#define RF_ASPFUNC_STRACE	1


#endif	// __DOASP_SYS_H__
