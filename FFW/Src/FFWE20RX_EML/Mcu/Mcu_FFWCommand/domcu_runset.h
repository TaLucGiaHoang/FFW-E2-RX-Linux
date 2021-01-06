////////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_runset.h
 * @brief プログラム実行中の設定変更コマンドのヘッダファイル
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
#ifndef	__DOMCU_RUNSET_H__
#define	__DOMCU_RUNSET_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// 定数定義
#define INIT_CMDEXE_FLG					0x00000000		// 各コマンド呼び出し確認フラグ
#define RUNSET_CMDNUM_BM				3				// コマンド番号(BM)

// プロトタイプ宣言（コマンド処理部）

// プロトタイプ宣言（その他の関数）
extern void SetEvCmdExeFlg(int eCmdNum);
extern BOOL GetEvCmdExeFlg(int eCmdNum);
extern void UpdateRunsetData(void);
extern void InitFfwCmdMcuData_Runset(void);	///< プログラム実行中の設定変更コマンド用FFW内部変数の初期化

#endif	// __DOMCU_RUNSET_H__
