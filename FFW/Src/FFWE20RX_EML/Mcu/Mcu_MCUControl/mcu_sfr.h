///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_sfr.h
 * @brief ターゲットMCU SFR制御関数のヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, K.Uemori
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/07/18
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/09/11 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・GetStatData()の関数宣言での、引数のbyStatKindをdwStatKindに変更。
・RevRxNo121206-001 2012/12/07 植盛
  ブートスワップ時のキャッシュ処理対応
・RevRxNo130301-001 2013/06/10 植盛
	RX64M対応
・RevRxNo130730-001 2014/06/20 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140515-005 2014/07/18 大喜多
	RX71M対応(メモリウェイト必要領域対応)
*/
#ifndef	__MCU_SFR_H__
#define	__MCU_SFR_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義
#define OFS_ENDIAN_OTH		0x3		//OFSに書かれているエンディアン情報がBIGでもLITTLEでもない場合

#define SFR_ACCESS_COUNT	0x1		//Get/SetMcuSfrReg()使用時のアクセス回数

// RevRxNo121206-001 Append Start
#define	SWAP_SIZE_4KB			0x00001000	// ブートスワップの4KB値
#define	SWAP_SIZE_8KB			0x00002000	// ブートスワップの8KB値
#define	SWAP_SIZE_16KB			0x00004000	// ブートスワップの16KB値
// RevRxNo121206-001 Append End

// グローバル関数の宣言
extern FFWERR SetMcuSfrReg(MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE	eAccessSize, BYTE* pbyData);
extern FFWERR GetMcuSfrReg(MADDR madrReadAddr, enum FFWENM_MACCESS_SIZE	eAccessSize, DWORD dwLength, BYTE* pbyData);
// RevRxNo140515-005 Append Start(CPUアクセスレジスタ用)
extern FFWERR SetMcuSfrReg_CPU(MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE	eAccessSize, BYTE* pbyData);
extern FFWERR GetMcuSfrReg_CPU(MADDR madrReadAddr, enum FFWENM_MACCESS_SIZE	eAccessSize, DWORD dwLength, BYTE* pbyData);
// RevRxNo140515-005 Append End

extern FFWERR GetMcuRomExtBusStatInfo(BOOL* bRomEna,BOOL* bExtBus);
extern FFWERR GetMcuSckcrIckVal(DWORD* pdwIck);
extern FFWERR GetMcuUsrBootMode(BOOL* pbUbts);
extern FFWERR GetMcuEndian(enum FFWENM_ENDIAN* eMcuEndian);
extern FFWERR GetMcuOfs1(DWORD* pdwOfs1Val);	// RevRxNo130730-001 Append Line
extern FFWERR SetMcuPrcrProtect(WORD wPrcr);
extern FFWERR GetMcuPrcrProtect(WORD* pwPrcr);
extern FFWERR EscMcuPrcrProtect(BOOL bEsc,WORD wPrcr,BOOL* pbProtectFree);

// RevRxNo130301-001 Modify Line
extern FFWERR SetRegRamEnable(BOOL bEnable, BOOL* pbRamBlockEna);

// RevRxNo120910-001 Modify Line
extern FFWERR GetStatData(DWORD dwStatKind, enum FFWRXENM_STAT_MCU* peStatMcu, FFWRX_STAT_SFR_DATA* pStatSFR, 
						FFWRX_STAT_JTAG_DATA* pStatJTAG, FFWRX_STAT_FINE_DATA* pStatFINE, FFWRX_STAT_EML_DATA* pStatEML);			///< ターゲットMCUのステータス情報を取得する

extern void InitMcuData_Sfr(void);						///< ターゲットMCU制御関数用変数初期化

// RevRxNo121206-001 Append Start
extern FFWERR ChkBootSwapSizeInfo(void);
extern DWORD GetBootSwapSize(void);
extern FFWERR ChkBootSwap(BOOL* pbFlg);
// RevRxNo121206-001 Append End

#endif	// __MCU_SFR_H__
