///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_extflash.h
 * @brief BFWコマンド プロトコル生成関数(外部フラッシュダウンロード関連)ヘッダファイル
 * @author RSO K.Okita, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#ifndef	__PROTMCU_EXTFLASH_H__
#define	__PROTMCU_EXTFLASH_H__

#include "ffw_typedef.h"
#include "mcudef.h"

/////////// define ///////////
#define PROT_EFWRITE_LENGTH		0x100		// BFWMCUCmd_EFWRITEコマンドで送信するライトバイト数

/////////// 関数の宣言 ///////////
extern FFWERR PROT_MCU_EFWRITESTART(BYTE byEndian, BYTE byConnect, enum FFWENM_MACCESS_SIZE eAccessSize, MADDR madrProgStartAddr, MADDR madrBuffStartAddr, DWORD dwBuffSize, MADDR madrSectAddr,
									WORD Com8_1, WORD Com8_2, WORD Com16_1, WORD Com16_2, WORD Com32_1, WORD Com32_2);
// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数にpwEraseStatusを追加 )
extern FFWERR PROT_MCU_EFWRITE(BYTE byMode, WORD, DWORD dwMakerID, DWORD dwDeviceID, MADDR madrStartAddr, const BYTE* pbyWriteData, WORD* pwEraseStatus);
extern FFWERR PROT_MCU_EFWRITEEND(void);

extern void InitProtMcuData_Extflash(void);	///< 外部Flashメモリ操作コマンド用変数初期化

#endif	// __PROTMCU_EXTFLASH_H__
