///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_sci.h
 * @brief SCIサポート関連コマンドのヘッダファイル
 * @author RSO EM2 K.Okita H.Hashiguchi
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
#ifndef	__FFWMCU_SCI_H__
#define	__FFWMCU_SCI_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義
#define BAUD_ID_MIN		0x0000			// 通信ボーレートIDのMIN値
#define	BAUD_ID_MAX		0x000C			// 通信ボーレートIDのMAX値
#define PORT_MAX		0x0FFF			// 指定ポートのMAX値
#define DIR_MAX			0x0FFF			// ポート入出力方向のMAX値		

#define TYPE_MAX		0x0FFF			// ポート出力形式のMAX値		
#define PUP_MAX			0x0FFF			// ポートプルアップのMAX値		
// プロトタイプ宣言（上記以外の関数）
void InitFfwIfMcuData_Sci(void);		///< シリアル通信コマンド用FFW内部変数の初期化


#endif	// __FFWMCU_SCI_H__
