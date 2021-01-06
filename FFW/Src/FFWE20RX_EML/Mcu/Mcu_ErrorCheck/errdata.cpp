///////////////////////////////////////////////////////////////////////////////
/**
 * @file errdata.cpp
 * @brief エラー処理内部変数管理モジュール
 * @author RSO EM2 H.Hashiguchi
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
#include "errchk.h"


//=============================================================================
/**
 * INITコマンド発行時のエラー処理内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitErrorData(void)
{
	InitErrorData_Errchk();	// エラーチェック処理用変数初期化

	return;
}
