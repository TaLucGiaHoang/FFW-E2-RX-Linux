////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_tim.h
 * @brief RX共通実行時間測定関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/10/09
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#ifndef	__DORX_TIM_H__
#define	__DORX_TIM_H__


#include "ffw_typedef.h"
#include "mcudef.h"

// V.1.02 No.38 Append Start
#define RX_CLS3_PPC_CH	2
#define RX_CLS2_PPC_CH	1
// V.1.02 No.38 Append End


// プロトタイプ宣言（コマンド処理部）

extern FFWERR DO_SetPPCCTRL(DWORD dwSetMp, const FFWRX_PPCCTRL_DATA* pPev);
// V.1.02 No.38 Append Line
extern FFWERR DO_SetRXCls2PPCCTRL(DWORD dwSetMp, const FFWRX_PPCCTRL_DATA* pPev);
extern FFWERR DO_SetPPCEVCNT(DWORD dwSetEvNum);
extern FFWERR DO_GetPPCD(DWORD dwGetMp, FFWRX_MPD_DATA* pMpd);
extern FFWERR DO_ClrPPCD(DWORD dwClrMp);

extern void GetItemNum2Cin(DWORD dwMpItem, DWORD* pCinMode);
extern FFWERR ClrPPCDPreRun(void);

extern void InitFfwCmdRxData_Tim(void);	///< 実行時間測定関連コマンド用FFW内部変数の初期化

#endif	// __DORX_TIM_H__
