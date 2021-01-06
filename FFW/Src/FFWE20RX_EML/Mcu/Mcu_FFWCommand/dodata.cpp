///////////////////////////////////////////////////////////////////////////////
/**
 * @file dodata.cpp
 * @brief FFW内部変数管理モジュール
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130308-001	2013/05/20 三宅
　カバレッジ開発対応
・RevRxE2LNo141104-001 2014/11/10 上田
	E2 Lite対応
*/
#include "do_sys.h"
#include "do_closed.h"
#include "do_common.h"

#include "doe2_closed.h"	// RevRxE2LNo141104-001 Append Line

#include "dorx_ev.h"
#include "dorx_rrm.h"
#include "dorx_tra.h"
#include "dorx_tim.h"

#include "domcu_clk.h"
#include "domcu_mcu.h"
#include "domcu_rst.h"
#include "domcu_dwn.h"
#include "domcu_brk.h"
#include "domcu_prog.h"
#include "domcu_reg.h"
#include "domcu_mem.h"
#include "domcu_dc.h"
#include "domcu_extflash.h"
#include "domcu_sci.h"
#include "domcu_srm.h"
#include "domcu_closed.h"
#include "domcu_runset.h"
// RevRxNo130308-001 Append Line
#include "dorx_cv.h"



//=============================================================================
/**
 * INITコマンド発行時のFFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdData(void)
{
	// 共通コマンド用変数初期化
	InitFfwCmdData_Sys();		// システムコマンド用変数初期化
	InitFfwCmdData_Closed();	// 非公開コマンド用変数初期化
	InitFfwCmdData_Common();	// FFW 共通関数用変数初期化

	// E2/E2 Lite共通コマンド用変数初期化
	InitFfwCmdE2Data_Closed();	// 非公開コマンド用変数初期化	// RevRxE2LNo141104-001 Append Line

	// // RX共通コマンド用変数初期化
	InitFfwCmdRxData_Rrm();		// RAMモニタ関連コマンド用変数初期化
	InitFfwCmdRxData_Tra();		// トレース関連コマンド用変数初期化
	InitFfwCmdRxData_Tim();		// 時間測定関連コマンド用変数初期化
	InitFfwCmdRxData_Ev();		// イベント関連コマンド用変数初期化

	// 品種依存コマンド用変数初期化
	InitFfwCmdMcuData_Brk();	// ブレーク関連コマンド用変数初期化
	InitFfwCmdMcuData_Clk();		// クロック関連コマンド用変数初期化
	InitFfwCmdMcuData_Closed();		// 非公開コマンド用変数初期化
	InitFfwCmdMcuData_Dc();		// デバッグコンソールコマンド用変数初期化
	InitFfwCmdMcuData_Dwn();	// プログラムダウンロード用変数初期化
	InitFfwCmdMcuData_Extflash();	// 外部フラッシュコマンド用変数初期化
	InitFfwCmdMcuData_Mcu();	// MCU依存コマンド用変数初期化
	InitFfwCmdMcuData_Mem();	// メモリ操作コマンド用変数初期化
	InitFfwCmdMcuData_Prog();	// プログラム実行関連コマンド用変数初期化
	InitFfwCmdMcuData_Reg();	// レジスタ操作コマンド用変数初期化
	InitFfwCmdMcuData_Rst();	// リセットコマンド用変数初期化
	InitFfwCmdMcuData_Runset();	// プログラム実行中の設定変更コマンド用変数初期化
	InitFfwCmdMcuData_Sci();	// シリアルコマンド用変数初期化
	InitFfwCmdMcuData_Srm();	// スタートストップルーチンコマンド用変数初期化
	// RevRxNo130308-001 Append Line
	InitFfwCmdMcuData_Cv();		// カバレッジ計測関連コマンド用変数初期化
}

//=============================================================================
/**
 * CLRコマンド発行時のFFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdData_CLR(void)
{
	// 共通コマンド用変数初期化
	InitFfwCmdData_Closed();	// 非公開コマンド用変数初期化
	InitFfwCmdData_Common();	// FFW 共通関数用変数初期化

	// RX共通コマンド用変数初期化
	InitFfwCmdRxData_Ev();		// イベント関連コマンド用変数初期化
	InitFfwCmdRxData_Rrm();		// RAMモニタ関連コマンド用変数初期化
	InitFfwCmdRxData_Tra();		// トレース関連コマンド用変数初期化
	InitFfwCmdRxData_Tim();		// 時間測定関連コマンド用変数初期化

	// 品種依存コマンド用変数初期化
	InitFfwCmdMcuData_Rst();	// リセットコマンド用変数初期化
	InitFfwCmdMcuData_Brk();	// ブレーク関連コマンド用変数初期化
	InitFfwCmdMcuData_Prog();	// プログラム実行関連コマンド用変数初期化
	InitFfwCmdMcuData_Reg();	// レジスタ操作コマンド用変数初期化
	InitFfwCmdMcuData_Mem();	// メモリ操作コマンド用変数初期化
	InitFfwCmdMcuData_Dc();		// デバッグコンソールコマンド用変数初期化
	InitFfwCmdMcuData_Runset();	// プログラム実行中の設定変更コマンド用変数初期化
	// RevRxNo130308-001 Append Line
	InitFfwCmdMcuData_Cv();		// カバレッジ計測関連コマンド用変数初期化
}
