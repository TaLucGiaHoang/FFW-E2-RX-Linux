////////////////////////////////////////////////////////////////////////////////
/**
 * @file hwdata.h
 * @brief 品種依存部 H/W制御モジュール ヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/05/20
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130308-001	2013/05/20 三宅
　カバレッジ開発対応
  ・ヘッダ内のファイル名を「hw_data_po.h」から「hwdata.h」に修正。
*/
#ifndef	__HWDATA_H__
#define	__HWDATA_H__


#include "ffw_typedef.h"
#include "mcudef.h"


// 定数定義

// 関数プロトタイプ

extern void InitHwData(void);		///

#endif	// __HWDATA_H__
