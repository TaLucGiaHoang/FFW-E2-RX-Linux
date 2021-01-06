///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_dc.h
 * @brief BFWコマンド プロトコル生成関数(デバッグコンソール)ヘッダファイル
 * @author RSO H.Hashiguchi
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
#ifndef	__PROTMCU_DC_H__
#define	__PROTMCU_DC_H__

#include "ffw_typedef.h"
#include "mcudef.h"

extern FFWERR PROT_GetC2E(FFWRX_C2E_DATA* pC2E, BYTE* pbyData);						///< BFWMCUCmd_GetC2Eコマンドの発行
extern FFWERR PROT_SetE2C(FFWRX_E2C_DATA* pE2C);									///< BFWMCUCmd_SetE2Cコマンドの発行
extern FFWERR PROT_ClrC2EE2C(DWORD dwRegNo);										///< BFWMCUCmd_ClrC2EE2Cコマンドの発行
extern void InitProtMcuData_Dc(void);
#endif	// __PROTMCU_DC_H__
