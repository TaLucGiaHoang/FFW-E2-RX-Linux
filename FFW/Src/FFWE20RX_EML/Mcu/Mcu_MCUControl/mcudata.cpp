///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcudata.cpp
 * @brief ターゲットMCU制御関数内部変数管理モジュール
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/17
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121227-002	2012/12/27 上田
　　InitMcuData()にInitMcuRxData_Mcu()呼び出しを追加。
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
*/
#include "mcurx.h"
#include "mcurx_ev.h"
#include "mcurx_tim.h"
#include "mcurx_tra.h"
#include "mcu_brk.h"
#include "mcu_rst.h"
#include "mcu_extflash.h"
#include "mcu_flash.h"
#include "mcu_mem.h"
#include "mcu_inram.h"	// RevRxE2LNo141104-001 Append Line
#include "mcu_extram.h"
#include "mcu_sfr.h"

//=============================================================================
/**
 * INITコマンド発行時のターゲットMCU制御関数内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData(void)
{

	// RX依存コマンド用変数初期化
	InitMcuRxData_Ev();
	InitMcuRxData_Tim();
	InitMcuRxData_Tra();
	InitMcuRxData_Mcu();	// RevRxNo121227-002 Append Line

	// 品種依存コマンド用変数初期化
	InitMcuData_Brk();			// ターゲットMCUブレーク制御関数用変数初期化
	InitMcuData_Rst();			// ターゲットMCUリセット制御関数用変数初期化
	InitMcuData_Extflash();		// ターゲットMCU外部フラッシュ制御関数用変数初期化
	InitMcuData_Flash();		// ターゲットMCUフラッシュ制御関数用変数初期化
	InitMcuData_Mem();			// ターゲットMCUメモリ操作制御関数用変数初期化
	InitMcuData_Inram();		// ターゲットMCU内部RAMダウンロード制御関数用変数初期化	// RevRxE2LNo141104-001 Append Line
	InitMcuData_Extram();		// ターゲットMCU外部RAM制御関数用変数初期化
	InitMcuData_Sfr();			// ターゲットMCUSFRレジスタ制御関数用変数初期化

	return;
}