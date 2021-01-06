///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_rst.h
 * @brief リセットコマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/07/23
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130301-001	2013/09/06 上田
	RX64M対応
・RevRxNo130730-001 2014/07/23 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
*/
#ifndef	__DOMCU_RST_H__
#define	__DOMCU_RST_H__

#include "ffw_typedef.h"
#include "mcudef.h"



// グローバル関数の宣言
extern FFWERR DO_REST(void);
extern FFWERR DO_CLR(void);
extern FFWERR DO_SREST(void);		///< システムリセット

extern FFWERR SysReSet(void);		///< システム再設定処理	// RevRxNo130301-001 Modify Line

extern void InitFfwCmdMcuData_Rst(void);	///< リセットコマンド用FFW内部変数の初期化

extern FFWERR SetEndianState(void);	///< リセット時のエンディアン状態を記憶
extern WORD GetEndianState(void);	///< リセット時のエンディアン状態を取得

extern void ClrOsmFwriteWarningFlg(void);		// RevRxNo130730-001 Append Line
extern FFWERR GetOsmFwriteWarningFlg(void);		// RevRxNo130730-001 Append Line
extern BOOL GetSkipMdeMskResetOsmFwrite(void);	// RevRxNo130730-001 Append Line

#endif	// __DOMCU_RST_H__
