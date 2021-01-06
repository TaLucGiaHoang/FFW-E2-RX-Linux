///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwe2_closed.h
 * @brief 非公開コマンドのヘッダファイル
 * @author RSD S.Ueda
 * @author Copyright (C) 2014 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/12/24
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/12/24 上田
	E2 Lite対応
*/
#ifndef	__FFWE2_CLOSED_H__
#define	__FFWE2_CLOSED_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義
// ACTLEDコマンド
#define ACTLED_OFF	0x00000000		// ACTLED 消灯
#define ACTLED_ON	0x00000001		// ACTLED 点灯

// LIDコマンド
#define LID_AREA_SIZE	0x00000800		// ライセンス情報設定領域サイズ
#define LID_LENGTH_MIN	0x00000001		// ライセンス情報のデータバイト数最小値
#define LID_LENGTH_MAX	LID_AREA_SIZE	// ライセンス情報のデータバイト数最大値


// グローバル関数の宣言
extern void InitFfwIfE2Data_Closed(void);	///< 非公開コマンド用変数初期化

#endif	// __FFWE2_CLOSED_H__
