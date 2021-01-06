///////////////////////////////////////////////////////////////////////////////
/**
 * @file hw_fpga.h
 * @brief FPGAアクセスのヘッダファイル
 * @author RSO H.Hashiguchi
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
#ifndef	__HW_FPGA_H__
#define	__HW_FPGA_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義

// グローバル関数の宣言
extern FFWERR WriteFpgaReg(EADDR eadrWriteAddr, WORD wData);
extern FFWERR ReadFpgaReg(EADDR eadrReadAddr, WORD* pwData);

extern void InitFpgaData(void);	///< E1E20共通FPGA制御用変数初期化

#endif	// __HW_FPGA_H__
