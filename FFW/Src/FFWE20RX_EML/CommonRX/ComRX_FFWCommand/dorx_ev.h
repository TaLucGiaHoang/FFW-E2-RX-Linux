////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_ev.h
 * @brief RX共通イベント関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#ifndef	__DORX_EV_H__
#define	__DORX_EV_H__


#include "ffw_typedef.h"
#include "mcudef.h"

//define 定義


// プロトタイプ宣言（コマンド処理部）
extern FFWERR DO_SetRXEV(DWORD dwMode, BYTE byNo, FFWRX_EV_DATA* ev);
extern FFWERR DO_SetRXEVCNT(DWORD dwMode, BYTE byNo, const FFWRX_EVCNT_DATA* evcnt);
extern FFWERR DO_SetRXCOMB(BYTE byEvKind, DWORD dwCombi, FFWRX_COMB_BRK* evBrk, FFWRX_COMB_TRC* evTrc,	FFWRX_COMB_PERFTIME* evPerfTime, enum FFWRX_EVCOMBI_PPC_USR_CTRL ePpcUsrCtrl);
extern FFWERR DO_SetRXEVTRG(BYTE byEvKind, FFWRX_TRG_DATA evTrg);

extern void InitFfwCmdRxData_Ev(void);	///< イベント関連コマンド用FFW内部変数の初期化



#endif	// __DORX_EV_H__
