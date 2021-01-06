////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_tra.h
 * @brief RX共通トレース関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, Y.Miyake, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/17
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/10/15 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・RM_TRCSRCSEL_MCU_BUS、RM_TRCSRCSEL_DMAC_DTC_BUS の定義追加。
・RevRxNo130301-001	2013/09/05 上田
	RX64M対応
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
*/
#ifndef	__FFWRX_TRA_H__
#define	__FFWRX_TRA_H__


#include "ffw_typedef.h"
#include "mcudef.h"


// RevRxNo120910-001 Append Start
// 定数定義
// EV
#define	RM_TRCSRCSEL_MCU_BUS		0x00000001		// オペランドアクセス時のトレースソース選択情報：CPUバス。
#define	RM_TRCSRCSEL_DMAC_DTC_BUS	0x00000002		// オペランドアクセス時のトレースソース選択情報：DMAC/DTCバス。
// RevRxNo120910-001 Append End

// 構造体定義
// RevRxNo130301-001 Append Start
typedef	struct {
	BOOL	bSetMode;	// 前回のトレースモード情報設定有無
	DWORD	dwMode;		// 前回のトレースモード
} FFWRX_RM_OLDMODE_DATA;
// RevRxNo130301-001 Append End



// プロトタイプ宣言（構造体変数定義、アクセス関数）
extern FFWRX_RM_DATA* GetRmDataInfo(void);
extern FFWRX_RM_OLDMODE_DATA* GetRmOldMode(void);	// RevRxNo130301-001 Append Line
extern void SetRmModeData(DWORD dwSetMode);
extern void SetRmInitialData(DWORD dwSetInitial);
extern void ClrTraceDataClrFlg(void);	// RevRxE2LNo141104-001 Append Line

// プロトタイプ宣言（その他の関数）
void InitFfwIfRxData_Tra(void);	///< トレース関連コマンド用FFW内部変数の初期化


#endif	// __FFWRX_TRA_H__
