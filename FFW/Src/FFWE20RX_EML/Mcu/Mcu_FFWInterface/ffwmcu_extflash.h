///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_extflash.h
 * @brief 外部Flashダウンロードのヘッダファイル
 * @author RSO K.Okita, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/06/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130301-001 2013/06/10 植盛
	RX64M対応
*/
#ifndef	__FFWMCU_EXTFLASH_H__
#define	__FFWMCU_EXTFLASH_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義
// RevRxNo130301-001 Append Start
#define	WORKRAM_INT	0x00	// ワークRAM種別が内蔵RAM
#define	WORKRAM_EXT	0x01	// ワークRAM種別が外部RAM
// RevRxNo130301-001 Append End

// グローバル関数の宣言
extern FFW_EXTF_DATA e_ExtfInfo;	// GUIから渡された外部フラッシュ情報保持用

extern void InitFfwIfMcuData_ExtFlash(void);
#endif	// __FFWMCU_EXTFLASH_H__
