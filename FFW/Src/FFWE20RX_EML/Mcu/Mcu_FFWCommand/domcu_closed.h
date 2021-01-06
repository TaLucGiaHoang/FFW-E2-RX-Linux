///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_closed.h
 * @brief 非公開コマンドのヘッダファイル 品種依存コマンド用
 * @author RSO Y.Minami, H.Hashiguchi
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
#ifndef	__DOMCU_CLOSED_H__
#define	__DOMCU_CLOSED_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義
#define IR_BIT_NUM	32
#define DR_BIT_NUM	32

// グローバル関数の宣言

// 2008.8.28 INSERT_BEGIN_E20RX600(+2) {
extern FFWERR DO_RXIR(DWORD dwBitLength, BYTE* pbyData);
extern FFWERR DO_RXDR(DWORD dwBitLength, BYTE* pbyData, DWORD dwPause);
extern FFWERR DO_RXGetDR(DWORD dwBitLength, BYTE* pbyData, DWORD dwPause);
// 2008.8.28 INSERT_END_E20RX600 }
extern FFWERR DO_RXSEMC(BYTE byLwordLength, DWORD* pbyData);				///< 命令コードの供給
extern FFWERR DO_RXMAST(void);										///< 供給命令コードの実行
extern FFWERR DO_RXGEMD(DWORD *dwData);								///< 供給命令コード実行による結果(C2E_0)内容の取得
extern FFWERR DO_RXSEMP(DWORD dwData);								///< 命令供給コードへのパラメータ渡し(E2C_0)に設定

extern FFWERR DO_SetRXSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteData);
extern FFWERR DO_GetRXSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD *pdwReadData);

extern FFWERR DO_FBER(void);
extern FFWERR DO_FCCLR(void);

extern void InitFfwCmdMcuData_Closed(void);

#endif	// __DOMCU_CLOSED_H__
