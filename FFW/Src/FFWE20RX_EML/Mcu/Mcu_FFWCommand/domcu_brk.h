////////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_brk.h
 * @brief ブレーク関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120606-002 2012/07/12 橋口
  BMの実行前、イベントブレーク禁止中の実行中のイベントブレーク設定不具合対応
・RevNo120720-002	2012/07/27 橋口
　ユーザプログラム実行中のS/Wブレークポイント解除処理修正
・RevRxE2No171004-001 2017/10/04 PA 辻
  E2拡張機能対応
*/
#ifndef	__DOMCU_BRK_H__
#define	__DOMCU_BRK_H__


#include "ffw_typedef.h"
#include "mcudef.h"


// プロトタイプ宣言（コマンド処理部）
extern FFWERR DO_SetPB(enum FFWENM_PB_SW eSw, DWORD dwmadrAddr, BYTE* pbyUsrCode, enum FFWENM_PB_FILLSTATE* peFillState, BOOL* pbClrPb);	// RevNo120720-002 Modify Line
extern FFWERR DO_ClrPB(void);
extern FFWERR DO_ClrPBAll(void);			// RevRxE2No171004-001 Append Line
extern FFWERR DO_SetBM(DWORD dwBMode);		// RevNo120606-002 Append Line

// プロトタイプ宣言（その他の関数）
extern BOOL SearchBrkPoint(DWORD dwmadrAddr, FFW_PB_DATA* pPb);	///< 対象アドレスにS/Wブレークが設定されているかを検索する
extern void InitFfwCmdMcuData_Brk(void);	///< ブレーク関連コマンド用FFW内部変数の初期化

#endif	// __DOMCU_BRK_H__
