////////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_reg.h
 * @brief BFWコマンド プロトコル生成関数（レジスタ操作関連） ヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, PA M.Tsuji
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
#ifndef	__PROTMCU_REG_H__
#define	__PROTMCU_REG_H__

#include "ffw_typedef.h"
#include "mcudef.h"
#include "ffwmcu_reg.h"

// 定数定義


// プロトタイプ宣言
void InitProtMcuData_Reg(void);		//< レジスタ操作コマンド用変数初期化

// 2008.9.10 INSERT_BEGIN_E20RX600(+2) {
FFWERR PROT_MCU_SetRXXREG(const FFWMCU_REG_DATA_RX* pReg);			// レジスタ値の変更
FFWERR PROT_MCU_GetRXXREG(BYTE bRegNum, FFWMCU_REG_DATA_RX* pReg);	// レジスタ値の参照
// 2008.9.10 INSERT_END_E20RX600 }
// RevRxNo180625-001 Append Start
FFWERR PROT_MCU_SetREGBANK(const FFWRX_REGBANK_DATA* pRegBank);		// レジスタ退避バンクの設定
FFWERR PROT_MCU_GetREGBANK(FFWRX_REGBANK_DATA* pRegBank);			// レジスタ退避バンクの参照
// RevRxNo180625-001 Append End

#endif	//__PROTMCU_REG_H__
