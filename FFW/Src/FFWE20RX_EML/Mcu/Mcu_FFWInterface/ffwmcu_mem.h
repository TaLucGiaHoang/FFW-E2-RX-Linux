///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_mem.h
 * @brief メモリ操作コマンドのヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, S.Ueda, K.Uemori
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/01/23
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo121017-002	2012/10/31 上田
　・WRITE_LENGTH_MAXのdefine定義追加  <VS2008 ERROR C2148対策>
・RevRxNo140109-001 2014/01/23 植盛
	RX64Mオプション設定メモリへのダウンロード対応
*/
#ifndef	__FFWMCU_MEM_H__
#define	__FFWMCU_MEM_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義
#define MDATASIZE_1BYTE	1	// データサイズ=1バイト
#define MDATASIZE_2BYTE	2	// データサイズ=2バイト
#define MDATASIZE_4BYTE	4	// データサイズ=4バイト
#define MDATASIZE_8BYTE	8	// データサイズ=8バイト

#define MSER_DSIZE_MIN	1	// メモリ検索データサイズの最小値
#define MSER_DSIZE_MAX	64	// メモリ検索データサイズの最大値

// RevNo121017-002 Append Line
#define WRITE_LENGTH_MAX	0x8000000	// WRITEコマンドレングスの最大値(128MB)
#define MEM_MAX_RANGE		0x1000000	// メモリ確保の最大値(16MB)

// グローバル関数の宣言
extern void InitFfwIfMcuData_Mem(void);	///< メモリ操作コマンド用変数初期化

// RevRxNo140109-001 Append Start
extern void ClrWriteErrFlg(void);		// WRITE処理実行時のエラー有無フラグクリア
extern BOOL GetWriteErrFlg(void);		// WRITE処理実行時のエラー有無フラグ値取得
// RevRxNo140109-001 Append End

#endif	// __FFWMCU_MEM_H__
