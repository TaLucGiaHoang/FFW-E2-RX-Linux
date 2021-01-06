///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_dc.h
 * @brief デバッグコンソールコマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
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
#ifndef	__FFWMCU_DC_H__
#define	__FFWMCU_DC_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義
#define C2E_CH0			0x01		// C2Eチャネル番号 Ch0
#define E2C_CH0			0x01		// E2Cチャネル番号 Ch0
#define CLR_C2E_E2E_CH0			0	// C2E/E2C クリア時のチャネル番号　Ch0 
// グローバル関数の宣言
extern void InitFfwIfMcuData_Dc(void);	///< デバッグコンソールコマンド用変数初期化


#endif	// __FFWMCU_DC_H__
