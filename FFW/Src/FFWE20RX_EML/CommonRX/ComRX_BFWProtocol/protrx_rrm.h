////////////////////////////////////////////////////////////////////////////////
/**
 * @file protrx_rrm.h
 * @brief BFWコマンド プロトコル生成関数（RAMモニタ関連） ヘッダファイル
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
#ifndef	__PROTRX_RRM_H__
#define	__PROTRX_RRM_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義

// プロトタイプ宣言
//RevNo100715-002 Modify Line
FFWERR PROT_E20_SetRrmb( BYTE byBlk,DWORD dwInitBlk,DWORD dwEnableBlk,DWORD dwBlkEndian, FFW_RRMB_DATA* pRrmb );	//< RAMモニタベースアドレス設定コマンド
//RevNo100715-003 Modify Line
FFWERR PROT_E20_SetRrmi( BYTE byBlk,DWORD dwBlkSet,BYTE byBlkInit ,DWORD dwEndian);				//< RAMモニタ初期化コマンド
FFWERR PROT_E20_GetRrmd( BYTE byBlk,WORD wStartAddr,WORD wSize,BYTE *pbyData );		//< RAMモニタデータ参照コマンド
FFWERR PROT_E20_ClrRrml( void );												//< RAMモニタロスト発生クリアコマンド
void InitProtRxData_Rrm(void);													//< RAMモニタ関連コマンド用変数初期化


#endif


