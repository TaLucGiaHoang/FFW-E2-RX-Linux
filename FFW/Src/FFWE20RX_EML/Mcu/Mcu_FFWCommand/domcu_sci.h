///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_sci.h
 * @brief シリアル関連コマンドのヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
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
#ifndef	__DOMCU_SCI_H__
#define	__DOMCU_SCI_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// プロトタイプ宣言（コマンド処理部）
extern FFWERR	DO_SetSCIBR( DWORD dwBaudId );							///< シリアル通信ボーレート設定コマンド
extern FFWERR	DO_GetSCIBR( DWORD *pdwBaudIdMin,DWORD *pdwBaudIdMax );	///< シリアル通信ボーレート参照コマンド
extern FFWERR	DO_SetSCICTRL( WORD wSciCtrl );							///< シリアル通信情報設定コマンド
extern FFWERR DO_GetSCICTRL( WORD *wSciCtrl );

// グローバル関数の宣言
extern void InitFfwCmdMcuData_Sci(void);	///< FDTサポートコマンド用変数初期化


#endif	// __DOMCU_SCI_H__
