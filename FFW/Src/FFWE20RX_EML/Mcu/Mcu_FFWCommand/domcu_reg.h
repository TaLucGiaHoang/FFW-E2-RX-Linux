////////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_reg.h
 * @brief レジスタ操作コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/09/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo180625-001,PB18047-CD01-001 2018/06/25 PA 辻
	RX66T-H/RX72T対応
*/
#ifndef	__DOMCU_REG_H__
#define	__DOMCU_REG_H__


#include "ffw_typedef.h"
#include "ffwmcu_reg.h"
#include "mcudef.h"

//定義
//レジスタクリア処理
#define REGCLRCMD_CLR	0x0
#define REGCLRCMD_REST	0x1
#define REGCLRCMD_SREST	0x2
#define REGCLRCMD_PMOD	0x3

// プロトタイプ宣言（コマンド処理部）
extern FFWERR ClrFwRegData(BYTE byClrCmd);
void InitFfwCmdMcuData_Reg(void);	///< レジスタ操作コマンド用FFW内部変数の初期化
// RevRxNo180625-001 Append Start
extern FFWERR DO_SetREGBANK(const FFWRX_REGBANK_DATA* pRegBank);	///< レジスタ退避バンクの設定
extern FFWERR DO_GetREGBANK(FFWRX_REGBANK_DATA* pRegBank);			///< レジスタ退避バンクの参照
// RevRxNo180625-001 Append End

#endif	// __DOMCU_REG_H__
