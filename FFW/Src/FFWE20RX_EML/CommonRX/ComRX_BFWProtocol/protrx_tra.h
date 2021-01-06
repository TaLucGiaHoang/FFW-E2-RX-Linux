////////////////////////////////////////////////////////////////////////////////
/**
 * @file protrx_tra.h
 * @brief BFWコマンド プロトコル生成関数（トレース関連） ヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/12
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120626-001 2012/07/12 橋口
　RX63TH トレース駆動能力対応 PROT_SetTrcBrk()→PROT_SetTRCMODE()に変更
*/
#ifndef	__PROTRX_TRA_H__
#define	__PROTRX_TRA_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// 定数定義

// プロトタイプ宣言
extern FFWERR PROT_GetRD2_SPL(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* rd);

extern FFWERR PROT_GetRCYCntN(WORD wBlockNum, WORD wBlockCnt, BYTE *p_byPktCnt);
extern FFWERR PROT_SetTRCMODE(BYTE byTrcMode);				// RevRxNo120626-001 Modify Line
//RevNo100715-014 Append Line
extern FFWERR PROT_TRC_SetDummyData(void);
void InitProtRxData_Tra(void);



#endif


