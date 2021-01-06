////////////////////////////////////////////////////////////////////////////////
/**
 * @file do_common.h
 * @brief FFW 共通関数 ヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#ifndef	__DO_COMMON_H__
#define	__DO_COMMON_H__


#include "ffw_typedef.h"
#include "mcudef.h"



// define 定義
#define CONV_ENDIAN_L_BIT0		0x000000FF		// LWORD データ エンディアン変換対象ビット(bit0 - 7)
#define CONV_ENDIAN_L_BIT1		0x0000FF00		// LWORD データ エンディアン変換対象ビット(bit8 - 15)
#define CONV_ENDIAN_L_BIT2		0x00FF0000		// LWORD データ エンディアン変換対象ビット(bit16 - 23)
#define CONV_ENDIAN_L_BIT3		0xFF000000		// LWORD データ エンディアン変換対象ビット(bit24 - 31)
#define CONV_ENDIAN_W_BIT0		0x00FF			// WORD データ エンディアン変換対象ビット(bit0 - 7)
#define CONV_ENDIAN_W_BIT1		0xFF00			// WORD データ エンディアン変換対象ビット(bit8 - 15)



// enum 定義


// 構造体宣言


// プロトタイプ宣言（FFW 共通関数）
DWORD ConvEndian_L(DWORD dwConv);
WORD ConvEndian_W(WORD wConv);
void InitFfwCmdData_Common(void);

#endif	// __DO_COMMON_H__
