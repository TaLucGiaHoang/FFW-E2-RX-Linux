///////////////////////////////////////////////////////////////////////////////
/**
 * @file prot_cpu.h
 * @brief BFWコマンド プロトコル生成関数(モニタCPU空間アクセス関連)ヘッダファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/18
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/11/18 上田
	E2 Lite対応
*/
#ifndef	__PROT_CPU_H__
#define	__PROT_CPU_H__

#include "ffw_typedef.h"
#include "mcudef.h"

/////////// define定義 ///////////
#define PROT_CPU_R_BLK_MAX	0x4000	// BFWCmd_CPU_Rで指定可能なブロック総数最大値
#define PROT_CPU_R_LENGTH_MAX		0x10000	// BFWCmd_CPU_Rで指定可能なリード回数最大値
#define PROT_CPU_W_LENGTH_MAX		0x10000	// BFWCmd_CPU_Wで指定可能なライト回数最大値
#define PROT_CPU_FILL_LENGTH_MAX	0x10000	// BFWCmd_CPU_FILLで指定可能なライト回数最大値
#define PROT_CPU_RMWFILL_LENGTH_MAX	0x10000	// BFWCmd_CPU_RMFILLで指定可能なライト回数最大値	// RevRxE2LNo141104-001 Append Line
#define REG_ALL_BIT_OFF		0x00000000		// レジスタ全ビットOFF 用定義
#define REG_ALL_BIT_ON		0xFFFFFFFF		// レジスタ全ビットON 用定義


/////////// 関数の宣言 ///////////
// RevRxE2LNo141104-001 Append Start
extern FFWERR ProtCpuRead(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrReadAddr[], 
						  const DWORD dwLength[], BYTE *const pbyReadData);
														///< BFWCmd_CPU_R/BFWCmd_CPU_R_EMLコマンドの発行
extern FFWERR ProtCpuWrite(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrWriteAddr[],
						   const DWORD dwLength[], const BYTE* pbyWriteData);
														///< BFWCmd_CPU_W/BFWCmd_CPU_W_EMLコマンドの発行
extern FFWERR ProtCpuFill(enum FFWENM_EACCESS_SIZE eAccessSize, EADDR eadrWriteAddr, DWORD dwLength, 
						  const BYTE* pbyWriteBuff);	///< BFWCmd_CPU_FILL/BFWCmd_CPU_FILL_EMLコマンドの発行
// RevRxE2LNo141104-001 Append End
extern FFWERR PROT_CPU_R(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrReadAddr[], 
						 const DWORD dwLength[], BYTE *const pbyReadData);
														///< BFWCmd_CPU_Rコマンドの発行
extern FFWERR PROT_CPU_W(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrWriteAddr[],
						 const DWORD dwLength[], const BYTE* pbyWriteData);
														///< BFWCmd_CPU_Wコマンドの発行
extern FFWERR PROT_CPU_FILL(enum FFWENM_EACCESS_SIZE eAccessSize, EADDR eadrWriteAddr, DWORD dwLength, 
							const BYTE* pbyWriteBuff);	///< BFWCmd_CPU_FILLコマンドの発行
// RevRxE2LNo141104-001 Append Start
extern FFWERR PROT_CPU_RMWFILL(enum FFWENM_EACCESS_SIZE eAccessSize, EADDR eadrRmwAddr, DWORD dwLength, 
							   DWORD dwMask, DWORD dwData);
														///< BFWCmd_CPU_RMWFILLコマンドの発行
// RevRxE2LNo141104-001 Append End
extern FFWERR PROT_BATEND_RecvErrCodeReservData(WORD wCmdCode);	///< BFWCmd_BATENDコマンド送信後のエラーコードとリザーブデータ受信

void InitProtData_Cpu(void);	///< モニタCPUアクセスコマンド用変数初期化

// V.1.02 No.14,15 USB高速化対応 Append Start
extern FFWERR PROT_CPU_R_EML(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrReadAddr[], 
						 const DWORD dwLength[], BYTE *const pbyReadData);
														///< BFWCmd_CPU_R_EMLコマンドの発行
extern FFWERR PROT_CPU_W_EML(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrWriteAddr[],
						 const DWORD dwLength[], const BYTE* pbyWriteData);
														///< BFWCmd_CPU_W_EMLコマンドの発行
extern FFWERR PROT_CPU_FILL_EML(enum FFWENM_EACCESS_SIZE eAccessSize, EADDR eadrWriteAddr, DWORD dwLength, 
							const BYTE* pbyWriteBuff);	///< BFWCmd_CPU_FILLコマンドの発行
// V.1.02 No.14,15 USB高速化対応 Append End

#endif	// __PROT_CPU_H__
