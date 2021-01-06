///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_prog.h
 * @brief プログラム実行関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda, K.Uemori
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/06/18
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130809-001 2013/08/19 上田
	PROGコマンドのパラメータ可変長対応処理改善
・RevRxNo140515-007 2014/06/18 植盛
	SSST, SSENコマンド追加による速度改善
*/
#ifndef	__FFWMCU_PROG_H__
#define	__FFWMCU_PROG_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// enum定義
enum FFWENM_STEPCMD {	// ステップ実行コマンド
	STEPCMD_STEP=0,				// STEPコマンドによるステップ実行
	//RevNo100715-013 Modify Line
	STEPCMD_STEPOVER,			// STEPOVERコマンドによるステップ実行
	//RevNo100715-013 Append Line
	STEPCMD_STEP_MSK			// STEPコマンドによるステップ実行 リセット/NMI/割り込みにリセットはしたまま
};


// define 定義
// PROGコマンド 構造体サイズ定義
#define FFWRX_PROG_DATA_SIZE_V20000	0x00000014	// V.2.00.00のFFWRX_PROG_DATA構造体サイズ	// RevRxNo130809-001 Append Line


// グローバル関数の宣言
// RevRxNo140515-007 Append Start
extern void SetSsstState(void);
extern void ClrSsstState(void);
extern BOOL GetSsstState(void);
// RevRxNo140515-007 Append End

extern void InitFfwIfMcuData_Prog(void);				///< プログラム実行関連コマンド用変数初期化



#endif	// __FFWMCU_PROG_H__
