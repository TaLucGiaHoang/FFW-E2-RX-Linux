///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_srm.h
 * @brief 指定ルーチン実行関連コマンドのヘッダファイル
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
#ifndef	__DOMCU_SRM_H__
#define	__DOMCU_SRM_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// グローバル関数の宣言
extern FFWERR DO_SetSRM(const FFW_SRM_DATA*); 	/// 指定ルーチン動作モードの設定処理
extern FFWERR ClrSRM(void);
extern void InitFfwCmdMcuData_Srm(void);

#endif	// __DOMCU_SRM_H__
