///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_brk.h
 * @brief BFWコマンド プロトコル生成関数(ブレーク関連)ヘッダファイル
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
#ifndef	__PROTMCU_BRK_H__
#define	__PROTMCU_BRK_H__

#include "ffw_typedef.h"
#include "mcudef.h"


/////////// define ///////////

/////////// 関数の宣言 ///////////
extern void InitProtMcuData_Brk(void);					///< ブレーク関連コマンド用変数初期化


#endif	// __PROTMCU_BRK_H__
