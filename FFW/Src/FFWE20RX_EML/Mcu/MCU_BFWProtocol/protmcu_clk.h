////////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_clk.h
 * @brief BFWコマンド プロトコル生成関数（クロック関連） ヘッダファイル
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
#ifndef	__PROTMCU_CLK_H__
#define	__PROTMCU_CLK_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義



// プロトタイプ宣言
FFWERR PROT_SetJtagClk(BYTE byJtagClk);
FFWERR PROT_GetJtagClk(BYTE *pbyJtagClk);
void InitProtMcuData_Clk(void);


#endif


