///////////////////////////////////////////////////////////////////////////////
/**
 * @file protdata.cpp
 * @brief BFWプロトコル生成処理内部変数管理モジュール
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121227-002	2012/12/27 上田
　　InitProtData()にInitProtMcuData_Srm()呼び出しを追加。
・RevRxE2LNo141104-001 2014/11/10 上田
	E2 Lite対応
*/
#include "prot_common.h"
#include "prot_cpu.h"
#include "prot_sys.h"

#include "prote2_closed.h"	// RevRxE2LNo141104-001 Append Line

#include "protrx_tra.h"
#include "protrx_rrm.h"

#include "protmcu_rst.h"
#include "protmcu_prog.h"
#include "protmcu_clk.h"
#include "protmcu_mcu.h"
#include "protmcu_mem.h"
#include "protmcu_reg.h"
#include "protmcu_brk.h"
#include "protmcu_dc.h"
#include "protmcu_closed.h"
#include "protmcu_sci.h"
#include "protmcu_flash.h"
#include "protmcu_extflash.h"
#include "protmcu_srm.h"	// RevRxNo121227-002 Append Line


//=============================================================================
/**
 * INITコマンド発行時のBFWプロトコル生成処理内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtData(void)
{
	// 共通コマンド用変数初期化
	InitProtData_Common();	// BFWプロトコル生成部共通処理用変数初期化
	InitProtData_Cpu();		// モニタCPUアクセスコマンド用変数初期化
	InitProtData_Sys();		// システムコマンド用変数初期化

	// E2/E2 Lite共通コマンド用変数初期化
	InitProtE2Data_Closed();	// 非公開コマンド用変数初期化	// RevRxE2LNo141104-001 Append Line

	// RX共通コマンド用変数初期化
	InitProtRxData_Tra();		// トレース関連コマンド用変数初期化
	InitProtRxData_Rrm();		// RRM関連コマンド用変数初期化

	// 品種依存コマンド用変数初期化
	InitProtMcuData_Brk();	// ブレーク関連コマンド用変数初期化
	InitProtMcuData_Clk();		// クロック関連コマンド用変数初期化
	InitProtMcuData_Closed();		// クロック関連コマンド用変数初期化
	InitProtMcuData_Dc();	// デバッグコンソール関連コマンド用変数初期化
	InitProtMcuData_Mcu();	// MCU依存コマンド用変数初期化
	InitProtMcuData_Mem();	// メモリ操作コマンド用変数初期化
	InitProtMcuData_Prog();	// プログラム実行関連コマンド用変数初期化
	InitProtMcuData_Reg();	// レジスタ操作コマンド用変数初期化
	InitProtMcuData_Rst();		// リセットコマンド用変数初期化
	InitProtMcuData_Sci();	// レジスタ操作コマンド用変数初期化
	InitProtMcuData_Flash(); // 内部Flash操作コマンド用変数初期化
	InitProtMcuData_Extflash();	//外部Flashダウンロードコマンド用変数初期化
	InitProtMcuData_Srm();	// スタートストップファンクション関連コマンド用変数初期化	RevRxNo121227-002 Append Line

	return;
}