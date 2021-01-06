///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_srm.h
 * @brief BFWコマンド プロトコル生成関数(スタートストップファンクション)ヘッダファイル
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
#ifndef	__PROTMCU_SRM_H__
#define	__PROTMCU_SRM_H__

#include "ffw_typedef.h"
#include "mcudef.h"


/////////// 関数の宣言 ///////////
extern FFWERR PROT_MCU_SetSRM(const FFW_SRM_DATA*);	///< BFWMCUCmd_SetSRMコマンドの発行
extern void InitProtMcuData_Srm(void);				///< スタートストップファンクションコマンド用変数初期化

#endif	// __PROTMCU_SRM_H__
