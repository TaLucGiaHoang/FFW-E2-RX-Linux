///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwdata.cpp
 * @brief FFW内部変数管理モジュール
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130308-001 2013/05/20 三宅
　カバレッジ開発対応
・RevRxE2LNo141104-001 2014/11/10 上田
	E2 Lite対応
*/

#include "ffw_sys.h"
#include "ffw_closed.h"
#include "do_common.h"

#include "ffwe2_closed.h"	// RevRxE2LNo141104-001 Append Line

#include "ffwrx_ev.h"
#include "ffwrx_rrm.h"
#include "ffwrx_tra.h"
#include "ffwrx_tim.h"

#include "ffwmcu_clk.h"
#include "ffwmcu_mcu.h"
#include "ffwmcu_rst.h"
#include "ffwmcu_dwn.h"
#include "ffwmcu_brk.h"
#include "ffwmcu_prog.h"
#include "ffwmcu_srm.h"
#include "ffwmcu_reg.h"
#include "ffwmcu_mem.h"
#include "ffwmcu_closed.h"
#include "ffwmcu_extflash.h"
#include "ffwmcu_sci.h"
#include "ffwmcu_dc.h"
// RevRxNo130308-001 Append Line
#include "ffwrx_cv.h"

//=============================================================================
/**
 * INITコマンド発行時のFFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfData(void)
{
	// 共通コマンド用変数初期化
	InitFfwIfData_Sys();		// システムコマンド用変数初期化
	InitFfwIfData_Closed();		// 非公開コマンド用変数初期化

	// E2/E2 Lite共通コマンド用変数初期化
	InitFfwIfE2Data_Closed();	// 非公開コマンド用変数初期化	// RevRxE2LNo141104-001 Append Line

	// RX用品種依存コマンド用変数初期化
	InitFfwIfRxData_Ev();			// イベント関連コマンド用変数初期化
	InitFfwIfRxData_Rrm();		// RAMモニタ関連コマンド用変数初期化
	InitFfwIfRxData_Tra();		// トレース関連コマンド用変数初期化
	InitFfwIfRxData_Tim();		// 時間測定関連コマンド用変数初期化

	// 品種依存コマンド用変数初期化
	InitFfwIfMcuData_Brk();		// ブレーク関連コマンド用変数初期化
	InitFfwIfMcuData_Clk();		// クロック関連コマンド用変数初期化
	InitFfwIfMcuData_Closed();		// 非公開コマンド用変数初期化
	InitFfwIfMcuData_Dwn();		// プログラムダウンロード用変数初期化
	InitFfwIfMcuData_Dc();		// デバッグコンソール用変数初期化
	InitFfwIfMcuData_ExtFlash();	// 外部Flashダウンロード用変数初期化
	InitFfwIfMcuData_Mcu();		// MCU依存コマンド用変数
	InitFfwIfMcuData_Mem();		// メモリ操作コマンド用変数初期化
	InitFfwIfMcuData_Prog();	// プログラム実行関連コマンド用変数初期化
	InitFfwIfMcuData_Reg();	// レジスタ操作コマンド用変数初期化(RX用)
	InitFfwIfMcuData_Rst();		// リセットコマンド用変数初期化
	InitFfwIfMcuData_Sci();			// シリアル通信用変数初期化
	InitFfwIfMcuData_Srm();		// 指定ルーチン実行関連コマンド用変数の初期化
	// RevRxNo130308-001 Append Line
	InitFfwIfMcuData_Cv();		// カバレッジ計測関連コマンド用変数初期化
}

//=============================================================================
/**
 * CLRコマンド発行時のFFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfData_CLR(void)
{
	// 共通コマンド用変数初期化
	InitFfwIfData_Closed();		// 非公開コマンド用変数初期化

	// RX用品種依存コマンド用変数初期化
	InitFfwIfRxData_Ev();			// イベント関連コマンド用変数初期化
	InitFfwIfRxData_Rrm();		// RAMモニタ関連コマンド用変数初期化
	InitFfwIfRxData_Tra();		// トレース関連コマンド用変数初期化
	InitFfwIfRxData_Tim();		// 時間測定関連コマンド用変数初期化

	// 品種依存コマンド用変数初期化
	InitFfwIfMcuData_Rst();		// リセットコマンド用変数初期化
	InitFfwIfMcuData_Brk();		// ブレーク関連コマンド用変数初期化
	InitFfwIfMcuData_Prog();	// プログラム実行関連コマンド用変数初期化
	InitFfwIfMcuData_Srm();		// 指定ルーチン実行関連コマンド用変数の初期化
	InitFfwIfMcuData_Mem();		// メモリ操作コマンド用変数初期化
// 2008.9.9 INSERT_BEGIN_E20RX600(+1) {
	InitFfwIfMcuData_Reg();	// レジスタ操作コマンド用変数初期化(RX用)
// 2008.9.9 INSERT_END_E20RX600 }
	// RevRxNo130308-001 Append Line
	InitFfwIfMcuData_Cv();		// カバレッジ計測関連コマンド用変数初期化
}
