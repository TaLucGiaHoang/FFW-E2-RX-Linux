////////////////////////////////////////////////////////////////////////////////
/**
 * @file hwdata.cpp
 * @brief 品種依存部 H/W制御モジュール ソースファイル
 * @author RSO H.Hashiguchi
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
#include "hwdata.h"
#include "hw_fpga.h"
#include "hwrx_fpga_tra.h"


//==============================================================================
/**
 * INITコマンド発行時のHW品種依存部制御モジュールの初期化
 * @param なし
 * @retval なし
 */
//==============================================================================
void InitHwData(void)
{

	//E1/E20共通コマンド用変数初期化
	InitFpgaData();
	//RX依存部共通コマンド用変数初期化
	InitHwRxFpgaData_Tra();
	
	return ;
}
