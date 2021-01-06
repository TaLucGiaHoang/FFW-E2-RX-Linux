///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffw_closed.h
 * @brief 非公開コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/12
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/11/12 上田
	E2 Lite対応
*/
#ifndef	__FFW_CLOSED_H__
#define	__FFW_CLOSED_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義
#define	BFW_MONP_AREA_NON	0x00		// MONPALL 領域指定なし
#define BFW_MONP_AREA_LV0	0x01		// MONPALL レベル0領域書き換え指定
#define	BFW_MONP_AREA_EML	0x02		// MONPALL EML領域書き換え指定
#define BFW_MONP_AREA_FDT	0x04		// MONPALL FDT/PRG領域書き換え指定
#define BFW_MONP_AREA_COM	0x08		// MONPALL COM領域書き換え指定

#define SELID_NUM_MAX		32			// シリアル番号のバイト数最大値

// グローバル関数の宣言
extern BYTE GetMonpAreaFlg(void);		///< MONP書き込み領域フラグの参照
extern void InitFfwIfData_Closed(void);	///< 非公開コマンド用変数初期化

#endif	// __FFW_CLOSED_H__
