///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_brk.cpp
 * @brief BFWコマンド プロトコル生成関数(ブレーク関連)
 * @author RSO Y.Minami, H.Hashiguchi
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
#include "protmcu_brk.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "prot_common.h"

// 2008.11.6 INSERT_BEGIN_E20RX600(+1) {
#include "ffwmcu_mcu.h"
// 2008.11.6 INSERT_END_E20RX600 }
//=============================================================================
/**
 * ブレーク関連コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Brk(void)
{
	return;
}
