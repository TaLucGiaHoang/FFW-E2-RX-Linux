///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_mem.h
 * @brief BFWコマンド プロトコル生成関数(メモリ操作関連)ヘッダファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/12/02
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120606-004 2012/07/12 橋口
  ・MPU領域リード/ライト対応
・RevRxNo121022-001   2012/11/20 SDS Iwata
  ・EZ-CUBE用のリードバイト数最大値定義、ライトバイト数最大値定義を追加
・RevRxE2LNo141104-001 2014/12/02 上田
	E2 Lite対応
*/
#ifndef	__PROTMCU_MEM_H__
#define	__PROTMCU_MEM_H__

#include "ffw_typedef.h"
#include "mcudef.h"

/////////// define ///////////
// BFWMCUCmd_XXXXコマンド発行単位定義
// E1/E20/E2/E2 Lite/EZ-CUBE共通
// V.1.02 新デバイス対応 Modify Line　
// 低速クロックでFILLサイズが大きいとBFWタイムアウトエラーが出てしまうため 0x10000→0x2000単位に小分けにする
#define PROT_FILL_LENGTH_MAX			0x02000		// BFWMCUCmd_FILLコマンドのFILL回数最大値

// E1/E20/E2/E2 Lite用
#define PROT_DUMP_LENGTH_MAX			0x200		// BFWMCUCmd_DUMPコマンドのリード回数最大値(E1/E20用)
#define PROT_DUMP_LENGTH_MAX_E2			0x2000		// BFWMCUCmd_DUMPコマンドのリード回数最大値(E2/E2 Lite用)	// RevRxE2LNo141104-001 Append Line
#define PROT_WRITE_LENGTH_MAX			0x200		// BFWMCUCmd_WRITEコマンドのライトバイト数最大値(E1/E20/E2/E2 Lite用)
// V.1.02 No.14,15 ダウンロード高速化対応 Append Line
#define PROT_EXT_WRITE_LENGTH_MAX		0x10000		// BFWMCUCmd_WRITEコマンドのライトバイト数最大値(外部RAMダウンロード用)(E1/E20/E2/E2 Lite用)
#define PROT_INRAM_WRITE_LENGTH_MAX		0x200		// BFWMCUCmd_WRITEコマンドのライトバイト数最大値(内部RAMダウンロード用)(E1/E20用)	// RevRxE2LNo141104-001 Append Line
#define PROT_INRAM_WRITE_LENGTH_MAX_E2	0x10000		// BFWMCUCmd_WRITEコマンドのライトバイト数最大値(内部RAMダウンロード用)(E2/E2 Lite用)	// RevRxE2LNo141104-001 Append Line

// EZ-CUBE用
// RevRxNo121022-001 Append Start
#define PROT_DUMP_LENGTH_MAX_EZ			0x200	// BFWMCUCmd_DUMPコマンドのリードバイト数最大値(EZ-CUBE用)
#define PROT_WRITE_LENGTH_MAX_EZ		0x200	// BFWMCUCmd_WRITEコマンドのライトバイト数最大値(EZ-CUBE用)
#define PROT_EXT_WRITE_LENGTH_MAX_EZ	0x200	// BFWMCUCmd_WRITEコマンドのライトバイト数最大値(外部RAMダウンロード用)(EZ-CUBE用)
// RevRxNo121022-001 Append End
#define PROT_INRAM_WRITE_LENGTH_MAX_EZ	0x200	// BFWMCUCmd_WRITEコマンドのライトバイト数最大値(内部RAMダウンロード用)(E2/E2 Lite用)	// RevRxE2LNo141104-001 Append Line


/////////// 関数の宣言 ///////////
extern FFWERR PROT_MCU_DUMP(DWORD dwAreaNum, const MADDR madrReadAddr[], 
							BOOL bSameAccessSize, const enum FFWENM_MACCESS_SIZE eAccessSize[],
							BOOL bSameAccessCount, const DWORD dwAccessCount[], BYTE *const pbyReadData, BYTE byEndian);
																					///< BFWMCUCmd_DUMPコマンドの発行

// RevRxNo120606-004 Append Start																			
extern FFWERR PROT_MCU_CPUDUMP(DWORD dwAreaNum, const MADDR madrReadAddr[], 
							BOOL bSameAccessSize, const enum FFWENM_MACCESS_SIZE eAccessSize[],
							BOOL bSameLength, const DWORD dwLength[], BYTE *const pbyReadData, BYTE byEndian);
// RevRxNo120606-004 Append End	

// V.1.02 No.3 起動時エンディアン&デバッグ継続モード処理 Append Line
extern FFWERR PROT_MCU_DDUMP(DWORD dwAreaNum, const MADDR madrReadAddr[], 
							BOOL bSameAccessSize, const enum FFWENM_MACCESS_SIZE eAccessSize[],
							BOOL bSameAccessCount, const DWORD dwAccessCount[], BYTE *const pbyReadData);
																					///< BFWMCUCmd_DDUMPコマンドの発行
extern FFWERR PROT_MCU_FILL(enum FFWENM_VERIFY_SET eVerify, MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE eAccessSize,
							DWORD dwAccessCount, DWORD dwWriteDataLength, const BYTE* pbyWriteBuff, 
							FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian);	///< BFWMCUCmd_FILLコマンドの発行
// RevRxNo120606-004 Append Start
extern FFWERR PROT_MCU_CPUFILL(enum FFWENM_VERIFY_SET eVerify, MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE eAccessSize,
							DWORD dwLength, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff, 
							FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian);	///< BFWMCUCmd_FILLコマンドの発行
// RevRxNo120606-004 Append End
extern FFWERR PROT_MCU_WRITE(enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, const MADDR madrWriteAddr[], 
							 BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize[], 
							 BOOL bSameAccessCount, const DWORD dwAccessCount[], 
							 BOOL bSameWriteData, const BYTE* pbyWriteData, 
							 FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian);	///< BFWMCUCmd_WRITEコマンドの発行
// RevRxNo120606-004 Append Start
extern FFWERR PROT_MCU_CPUWRITE(enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, const MADDR madrWriteAddr[], 
							 BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize[], 
							 BOOL bSameLength, const DWORD dwLength[], 
							 BOOL bSameWriteData, const BYTE* pbyWriteData, 
							 FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian);	///< BFWMCUCmd_WRITEコマンドの発行
// RevRxNo120606-004 Append End
extern void InitProtMcuData_Mem(void);	///< メモリ操作コマンド用変数初期化

#endif	// __PROTMCU_MEM_H__
