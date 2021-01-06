///////////////////////////////////////////////////////////////////////////////
/**
 * @file errdata.h
 * @brief エラー処理内部変数管理モジュールのヘッダファイル
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
#ifndef	__ERRDATA_H__
#define	__ERRDATA_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// グローバル関数の宣言
extern void InitErrorData(void);	///< INITコマンド発行時のエラー処理内部変数の初期化


#endif	// __ERRDATA_H__
