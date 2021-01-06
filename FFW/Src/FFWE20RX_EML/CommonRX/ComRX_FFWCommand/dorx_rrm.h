////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_rrm.h
 * @brief RAMモニタ関連コマンドのヘッダファイル
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
#ifndef	__DORX_RRM_H__
#define	__DORX_RRM_H__


#include "ffw_typedef.h"
#include "mcudef.h"


// プロトタイプ宣言（コマンド処理部）
// 2009.04.30 INSERT_BEGIN_E20 RAMモニタ対応 SDS Ueda {
FFWERR DO_E20_SetRRMB(DWORD dwSetBlk, FFW_RRMB_DATA *pRrmbData);
//RevNo100715-003 Modify Line
FFWERR DO_E20_SetRRMI(DWORD dwInitBlk, const DWORD dwInitInfo[], BOOL bInitFlg0, BOOL bInitFlg1,FFW_RRMB_DATA *pRrmbData);
FFWERR DO_E20_GetRRMD(DWORD dwGetBlk, const FFW_RRMD_DATA rrmd[], FFW_RRMD_GET_DATA ramMonData[]);
FFWERR DO_E20_ClrRRML(void);
// 2009.04.30 INSERT_END_E20 RAMモニタ対応 SDS Ueda }
extern FFWERR ClrRramReg(void);

extern void InitFfwCmdRxData_Rrm(void);	///< RAMモニタ関連コマンド用FFW内部変数の初期化

#endif	// __DORX_RRM_H__
