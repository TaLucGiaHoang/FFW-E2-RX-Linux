///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_mem.h
 * @brief ターゲットMCU メモリ制御関数のヘッダファイル
 * @author RSO Y.Minami, K.Okita(PA Tsumu), H.Hashiguchi, K.Uemori, SDS T.Iwata, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2016/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121022-001	2012/11/21 SDS 岩田
	EZ-CUBE PROT_MCU_DUMP()分割処理対応
	EZ-CUBE PROT_MCU_WRITE()分割処理対応
・RevRxNo121022-001	2012/12/04 SDS 岩田
	EZ-CUBE PROT_MCU_CPUDUMP分割処理対応
・RevRxNo130301-001 2013/06/07 植盛
	RX64M対応
・RevRxNo140109-001 2014/01/17 植盛
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo140515-005 2014/07/18 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo150528-001	2015/06/02 PA 紡車
	メモリウェイト仕様追加対応
・RevRxNo161003-001	2016/10/04 PA 辻
	データフラッシュメモリアクセス周波数設定レジスタ対応
*/
#ifndef	__MCU_MEM_H__
#define	__MCU_MEM_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義

//MemDump,MemFill,MemWriteでアクセス時の終了アドレスへのプラス分
#define MBYTE_ACCESS_ADDR			0
#define MWORD_ACCESS_ADDR			1
#define MLWORD_ACCESS_ADDR			3

// RevRxNo140515-005 Append Start
// RevRxNo150528-001 Modify Line
#define NO_MEM_WAIT_CHG				0x00		// メモリウェイト設定変更必要なし
#define MEM_WAIT_ROM				0x01
#define MEM_WAIT_RAM				0x02
// RevRxNo140515-005 Append End

// グローバル関数の宣言
extern FFWERR MemDump(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff);
														///< ターゲットメモリのDUMP処理
extern FFWERR MemFill(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
					  enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
					  FFW_VERIFYERR_DATA* pVerifyErr);	///< ターゲットメモリのFILL処理
extern FFWERR MemWrite(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, enum FFWENM_MACCESS_SIZE eAccessSize,
					   const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);
														///< ターゲットメモリのライト処理
extern FFWERR MemCodeDump(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff);
														///< 命令コード並びのDUMP処理
extern FFWERR MemCodeWrite(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, enum FFWENM_MACCESS_SIZE eAccessSize,
					   const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);
														///< 命令コード並びのライト処理

extern FFWERR ChkVerifyWriteData(MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwLength, 
								const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr, BYTE byEndian);

extern enum FFWENM_VERIFY_SET GetVerifySet(void);
extern void SetVerifySet(enum FFWENM_VERIFY_SET eVerify);

extern void InitMcuData_Mem(void);						///< ターゲットMCU制御関数用変数初期化

// RevRxNo121022-001 Append Start
extern FFWERR McuWriteDivide(enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, MADDR madrWriteAddr, 
					  BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize, 
					  BOOL bSameAccessCount, DWORD dwAccessCount, 
					  BOOL bSameWriteData, BYTE* pbyWriteData, 
					  FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian, DWORD dwWriteMaxLength);

extern FFWERR McuDumpDivide(DWORD dwAreaNum, MADDR madrReadAddr, 
					 BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize,
					 BOOL bSameAccessCount, DWORD dwAccessCount, BYTE* pbyReadData,
					 BYTE byEndian, DWORD dwDumpMaxLength);

extern FFWERR McuCpuDumpDivide(DWORD dwAreaNum, MADDR madrReadAddr, 
					 BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize,
					 BOOL bSameAccessCount, DWORD dwAccessCount, BYTE* pbyReadData,
					 BYTE byEndian, DWORD dwDumpMaxLength);
// RevRxNo121022-001 Append End

// RevRxNo130301-001 Append Line
extern BOOL	ChkRamArea(MADDR madrStartAddr, MADDR madrEndAddr, BOOL* pbRamBlockEna);

// RevRxNo140109-001 Append Start
extern void ClrExistConfigDwnDataFlg(void);
extern BOOL GetExistConfigDwnDataFlg(void);
// RevRxNo140109-001 Append End

// RevRxNo140515-005 Append Start
extern void SetMemWaitChange(BOOL bMemWaitChange);
extern void SetOPCModeChange(BOOL bOPCModeChange);
extern FFWERR ChkMemWaitInsert(MADDR madrStartAddr, MADDR madrEndAddr, BYTE* pbyMemWaitInsert);
// RevRxNo150528-001 Append Line
extern FFWERR ChkMwAreaOverlap(MADDR madrStartAddr, MADDR madrEndAddr, BYTE* pbyMemWaitInsert);
extern FFWERR SetMemWait(BOOL bMemwaitSet);
extern FFWERR CalcUserICLK(float *pfUserIclk);
extern void ClrMemWaitInsertWarningFlg(void);
extern BYTE GetMemWaitInsertFlg(void);
// RevRxNo140515-005 Append End

// RevRxNo161003-001 Append Start +3
extern FFWERR SetEepfclk(BOOL bMode);
extern FFWERR CalcUserFCLK(BYTE *pbyFclk);
extern FFWERR CalcUserHOCO(float *pfHocoFreq);
// RevRxNo161003-001 Append End

#endif	// __MCU_MEM_H__
