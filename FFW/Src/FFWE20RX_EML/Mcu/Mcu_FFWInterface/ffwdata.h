///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwdata.h
 * @brief FFW内部変数管理モジュールのヘッダファイル
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
#ifndef	__FFWDATA_H__
#define	__FFWDATA_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// グローバル関数の宣言
void InitFfwIfData(void);			///< INITコマンド発行時のFFW内部変数の初期化
void InitFfwIfData_CLR(void);		///< CLRコマンド発行時のFFW内部変数の初期化

#endif	// __FFWDATA_H__
