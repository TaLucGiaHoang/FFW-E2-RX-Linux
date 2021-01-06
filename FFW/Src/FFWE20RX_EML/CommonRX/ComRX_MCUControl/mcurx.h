////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcurx.h
 * @brief イベント関連コマンドのヘッダファイル
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
#ifndef	__MCURX_H__
#define	__MCURX_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義

#define OCD_ACCESS_COUNT	0x1		//Get/SetOCDReg()使用時のアクセス回数

// プロトタイプ宣言（コマンド処理部）
extern FFWERR SetMcuOCDReg(MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyData);
extern FFWERR GetMcuOCDReg(MADDR madrReadAddr, enum FFWENM_MACCESS_SIZE	eAccessSize, DWORD dwReadCnt, BYTE* pbyData);
extern void InitMcuRxData_Mcu(void);	///< OCD関連コマンド用FFW内部変数の初期化


#endif	// __MCURX_H__
