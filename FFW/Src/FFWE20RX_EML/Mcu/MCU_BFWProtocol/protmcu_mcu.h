///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_mcu.h
 * @brief BFWコマンド プロトコル生成関数ヘッダファイル(MCU依存コマンド)
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/10/09
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/09/11 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・PROT_MCU_GetRXSTAT()の関数宣言での、引数のbyStatKindをdwStatKindに変更。
*/
#ifndef	__PROTMCU_MCU_H__
#define	__PROTMCU_MCU_H__

#include "ffw_typedef.h"
#include "mcudef.h"
#include "ffwmcu_mcu.h"

/////////// define定義 ///////////
#define	STAT_EML_TRCINFO_TRCSTOP		0x00000000
#define	STAT_EML_TRCINFO_TRCRUN			0x00000001

// ID認証結果
#define IDCODE_RESULT_AUTH					0x1
#define IDCODE_RESULT_DEVICEID_ILLEGAL		0x2

/////////// 関数の宣言 ///////////
extern FFWERR PROT_MCU_SetRXMCU(const FFWRX_MCUAREA_DATA* pMcuArea, const FFWRX_MCUINFO_DATA* pMcuInfo);	///< BFWRXCmd_SetMCUコマンドの発行
extern FFWERR PROT_MCU_GetRXMCU(FFWRX_MCUAREA_DATA* pMcuArea, FFWRX_MCUINFO_DATA* pMcuInfo);
extern FFWERR PROT_MCU_SetRXDBG(const FFWMCU_DBG_DATA_RX* pDbg);	///< BFWRXCmd_SetDBGコマンドの発行
extern FFWERR PROT_MCU_GetRXDBG(FFWMCU_DBG_DATA_RX* pDbg);
// RevRxNo120910-001 Modify Line
extern FFWERR PROT_MCU_GetRXSTAT(DWORD dwStatKind, enum FFWRXENM_STAT_MCU* peStatMcu, FFWRX_STAT_SFR_DATA* pStatSFR,
								 FFWRX_STAT_JTAG_DATA* pStatJTAG, FFWRX_STAT_FINE_DATA* pStatFINE, FFWRX_STAT_EML_DATA* pStatEML);	///< BFWRXCmd_GetSTATコマンドの発行

// V.1.02 No.4 ユーザブートモード起動対応 Modify Lilne
extern FFWERR PROT_RXHPON(DWORD dwPlug, enum FFWRXENM_PMODE ePmode);
extern FFWERR PROT_E20SetHotPlug(void);
extern FFWERR PROT_RXIDCODE(BYTE*, BYTE*);

extern void InitProtMcuData_Mcu(void);	///< MCU依存コマンド用変数初期化


#endif	// __PROTMCU_MCU_H__
