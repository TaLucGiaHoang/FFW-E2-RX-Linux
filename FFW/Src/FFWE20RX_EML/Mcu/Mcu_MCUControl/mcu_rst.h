////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_rst.h
 * @brief リセット関連MCUへの処理のヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/09/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130301-001	2013/09/05 上田
	RX64M対応
*/
#ifndef	__MCU_RST_H__
#define	__MCU_RST_H__

#include "ffw_typedef.h"
#include "mcudef.h"

//リセットベクタ取得時にメモリの値を取得するか
#define RESTVECT_MEM_DATA		0
#define RESTVECT_CACHE_DATA		1

// プロトタイプ宣言（コマンド処理部）
extern FFWERR McuRest(void);
extern FFWERR OcdDbgModuleReset(void);	// RevRxNo130301-001 Append Line
extern FFWERR GetMcuRestAddr(BYTE byRestVect,DWORD* pdwRestVect);

void InitMcuData_Rst(void);	///< リセット関連コマンド用FFW内部変数の初期化



#endif	// __MCU_RST_H__
