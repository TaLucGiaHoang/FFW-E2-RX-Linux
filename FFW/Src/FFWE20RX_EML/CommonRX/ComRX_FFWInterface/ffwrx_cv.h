////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_cv.h
 * @brief カバレッジ計測関連コマンドのヘッダファイル
 * @author RSO Y.Miyake
 * @author Copyright (C) 2013 Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/03/22
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo130308-001	2013/03/22 三宅
　カバレッジ開発対応
*/
#ifndef	__FFWRX_CV_H__
#define	__FFWRX_CV_H__


#include "ffw_typedef.h"
#include "mcudef.h"
#include "dorx_tra.h"

// 定数定義
#define CVM_TBW_DATA4_SYNC1	0x00000000			// DATA 4bit, SYNC 1bit(デフォルト)
#define CVM_TBW_DATA8_SYNC2	0x00000001			// DATA 8bit, SYNC 2bit
#define CVM_TBW_DATA8_SYNC1	0x00000004			// DATA 8bit, SYNC 1bit
#define CVM_TRM_REALTIME	0x00000000			// CPU実行優先(リアルタイムトレース)(デフォルト)
#define CVM_TRM_TRCFULL		0x00000001			// トレース出力優先(フルトレース)
#define CVM_TRC_RATIO_1_1	0x00000000			// クロック比 1:1(デフォルト)
#define CVM_TRC_RATIO_2_1	0x00000001			// クロック比 2:1
#define CVM_TRC_RATIO_4_1	0x00000002			// クロック比 4:1

#define CVB_MAX_BLOCK_BIT	0x0000000F			// カバレッジ計測最大ブロックビット配置
#define CVB_NON_BLOCK_BIT	0x00000000			// カバレッジ計測ブロックが無い場合のビット配置
#define CVB_BLOCK0_BIT		0x00000001			// カバレッジ計測ブロック0のビット配置
#define CVB_BIT_CLR_21_0	0xFFC00000			// ビット21～0を"0"
#define CVB_BASE_ADDRESS_DEFAULT	0x00000000	// カバレッジ計測ブロックのベースアドレス(デフォルト)

#define RX_STAT_CV_EXEC		0x00000001			// "カバレッジ計測"ビットが「カバレッジ計測中」
#define RX_STAT_CV_LOST		0x00000002			// "ロスト発生状態"ビットが「ロスト発生」
#define RX_STAT_CV_RESET	0x00000004			// "カバレッジ計測異常発生状態"ビットが
												// 「リセット発生によるカバレッジ計測異常発生の可能性あり」
#define CVD0_MAX_ADDRESS	0x003FFFFF			// カバレッジブロックの最大オフセット・アドレス

// 初期化用定義
#define INIT_CVM_TBW		CVM_TBW_DATA4_SYNC1	// DATA 4bit, SYNC 1bit
#define INIT_CVM_TRM		CVM_TRM_REALTIME	// CPU実行優先(リアルタイムトレース)
#define INIT_CVM_TRC		CVM_TRC_RATIO_1_1	// クロック比 1：1


// プロトタイプ宣言（構造体変数定義、アクセス関数）


// プロトタイプ宣言（コマンド処理部）
extern FFWRX_CVB_DATA* GetCvb_RXInfo(void);					///< カバレッジ計測領域格納データ構造体のstatic変数のアドレスを取得
extern FFWRX_CVM_DATA* GetCoverageMode_RXInfo(void);		///< カバレッジ機能時のトレースモード格納データ構造体のstatic変数のアドレスを取得
extern BOOL GetCoverageLostFlag(void);						///< カバレッジのLOSTが発生したことを示すフラグを取得する
extern void SetCoverageLostFlag(BOOL bCoverageLostFlag);	///< カバレッジのLOSTが発生したことを示すフラグに設定する
extern BOOL GetCoverageResetFlag(void);						///< ユーザリセットが発生したことを示すフラグを取得する
extern void SetCoverageResetFlag(BOOL bCoverageResetFlag);	///< ユーザリセットが発生したことを示すフラグに設定する
extern void InitFfwIfMcuData_Cv(void);						///< レジスタ操作コマンド用FFW内部変数の初期化(RX用)

#endif	// __FFWRX_CV_H__
