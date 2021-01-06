///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_rst.h
 * @brief BFWコマンド プロトコル生成関数ヘッダファイル(リセットコマンド)
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/09/05
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130301-001	2013/09/05 上田
	RX64M対応
*/
#ifndef	__PROTMCU_RST_H__
#define	__PROTMCU_RST_H__

#include "ffw_typedef.h"
#include "mcudef.h"


/////////// 関数の宣言 ///////////
//RevNo100715-028 Modify Line

// プロトタイプ宣言
extern FFWERR PROT_MCU_REST(void);			///< ターゲットMCU のリセット
extern FFWERR PROT_MCU_SREST(BYTE* byResult);	///< BFWMCUCCmd_SRESTコマンドの発行	// RevRxNo130301-001 Append Line
extern FFWERR PROT_MCU_DBGREST(void);		///< BFWRXCCmd_DBGRESTのデバッグモジュールリセット

extern void InitProtMcuData_Rst(void);	///< リセット関連コマンド用変数初期化


#endif	// __PROTMCU_RST_H__
