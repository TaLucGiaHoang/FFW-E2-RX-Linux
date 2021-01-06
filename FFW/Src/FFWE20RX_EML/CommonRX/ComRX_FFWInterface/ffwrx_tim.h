////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_tim.h
 * @brief RX共通実行時間測定関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, K.Uemori
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/11/07
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120606-007	2012/07/13 橋口
  Class2 GPBのイベントを後ろから設定したときに、エラーが出る不具合修正
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
*/
#ifndef	__FFWRX_TIM_H__
#define	__FFWRX_TIM_H__


#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義
#define PM_MPITEM_MCU_CYC		0x1		//パフォーマンス計測モード　経過サイクル
#define PM_MPITEM_EVMATCH		0xA		//パフォーマンス計測モード　イベントマッチ
#define PM_OVF_ENA				0x1		//パフォーマンス計測　オーバーフロー有効
#define RX_PPC_MAX_CH			0x2		//RXのPPC最大CH数
// RevRxNo121026-001 Append Start
#define RX_PPC_NON				0x0		//RXのPPC未実装定義
#define RX_PPC_1CH				0x1		//RXのPPC 1ch実装定義
// RevRxNo121026-001 Append End

#define PCCSR_SETPERFFUNC		0x00000001
#define PCCSR_SETPERFFUNC_ON	0x00000001

// プロトタイプ宣言（構造体変数定義、アクセス関数）

// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
FFWRX_PPCCTRL_DATA* GetPpcCtrlData(void);
// 2008.11.18 INSERT_END_E20RX600 }
extern DWORD GetPpcCntEvNum(void);


// プロトタイプ宣言（コマンド処理部）
void InitFfwIfRxData_Tim(void);	///< 実行時間測定関連コマンド用FFW内部変数の初期化

#endif	// __FFWRX_TIM_H__
