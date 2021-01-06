////////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_sci.h
 * @brief BFWコマンド プロトコル生成関数（シリアル関連） ヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
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
#ifndef	__PROTMCU_SCI_H__
#define	__PROTMCU_SCI_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義

// プロトタイプ宣言
extern FFWERR PROT_SetSCIBR( DWORD wBaudId );								//< シリアル通信ボーレート設定コマンド
extern FFWERR PROT_GetSCIBR( DWORD *pdwBaudIdMin,DWORD *pdwBaudIdMax );	//< シリアル通信ボーレート参照コマンド
extern FFWERR PROT_SetSCICTRL( WORD wSciCtrl );							//< シリアル通信制御情報の設定コマンド
extern FFWERR PROT_GetSCICTRL( WORD *pwSciCtrl );

extern void InitProtMcuData_Sci(void);				//< シリアルサポート関連コマンド用変数初期化

#endif //__PROTMCU_SCI__
