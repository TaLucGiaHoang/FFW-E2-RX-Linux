///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_dc.h
 * @brief デバッグコンソールコマンドのヘッダファイル
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
#ifndef	__DOMCU_DC_H__
#define	__DOMCU_DC_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義


// グローバル関数の宣言

extern FFWERR DO_GetC2E(FFWRX_C2E_DATA* pC2E, BYTE* pbyData);		///< C2Eレジスタ内のデータを読み込む
extern FFWERR DO_SetE2C(FFWRX_E2C_DATA* pE2C);						///< E2Cレジスタにデータを書き込む
extern FFWERR DO_ClrC2EE2C(DWORD dwRegNo);							///< C2E, E2Cレジスタとバッファをクリア

extern void InitFfwCmdMcuData_Dc(void);	///< デバッグコンソールコマンド用変数初期化
#endif	// __DOMCU_DC_H__
