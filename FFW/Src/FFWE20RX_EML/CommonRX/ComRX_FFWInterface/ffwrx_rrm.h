////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_rrm.h
 * @brief RAMモニタ関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
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
#ifndef	__FFWRX_RRM_H__
#define	__FFWRX_RRM_H__


#include "ffw_typedef.h"
#include "mcudef.h"



// 定数定義
#define CHECK_INIT_INFO		0xFFFFFFFC			// RAM モニタ領域の初期化内容チェック値
//RevNo100715-004 Modify Start
#define MIN_BLOCK_NUM		0x00				// RAM モニタブロック番号の最小値
#define MAX_BLOCK_NUM		0x0F				// RAM モニタブロック番号の最大値
//RevNo100715-004 Modify End
#define MAX_AREA_NUM		512					// 1 ブロック内に設定する初期化抜け検出領域の最大数
#define RRM_OFFSET_ADDR		0x1FF				// RAMモニタ領域の開始／終了アドレスまでの最大オフセット

#define RRM_E20_OFFSET_ADDR	0x3FF				// E20 RAMモニタ領域の開始／終了アドレスまでの最大オフセット

#define E20_MAX_AREA_SIZE		1024				// E20 RAM モニタ領域の1 ブロックの最大サイズ

#define INIT_BIT0			0x00000001			// 初期化対象指定ビット（RAM モニタデータ、最終アクセス履歴）
#define INIT_BIT1			0x00000002			// 初期化対象指定ビット（リードアクセス履歴、ライトアクセス履歴、
												// リード→ライトアクセス履歴の初期化指定）
// 初期化用定義
#define INIT_RRMB_DATA		RRMB_DIS			// RAM モニタ領域の動作設定
#define INIT_INIA_NUM		0					// 1 ブロック内に設定する初期化抜け検出領域の個数

// 構造体定義

// プロトタイプ宣言（構造体変数定義、アクセス関数）
FFW_RRMB_DATA* GetRrmbData(void);

// プロトタイプ宣言（コマンド処理部）
void InitFfwIfRxData_Rrm(void);	///< RAMモニタ関連コマンド用FFW内部変数の初期化


#endif	// __FFWRX_RRM_H__
