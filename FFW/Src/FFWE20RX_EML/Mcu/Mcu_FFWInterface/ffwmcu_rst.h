///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_rst.h
 * @brief リセット関連コマンドのヘッダファイル
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
#ifndef	__FFWMCU_RST_H__
#define	__FFWMCU_RST_H__

#include "ffw_typedef.h"
#include "mcudef.h"


void InitFfwIfMcuData_Rst(void);	///< リセット関連コマンド用FFW内部変数の初期化



#endif	// __FFWMCU_RST_H__
