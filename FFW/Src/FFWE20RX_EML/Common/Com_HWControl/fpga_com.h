////////////////////////////////////////////////////////////////////////////////
/**
 * @file fpga_com.h
 * @brief FPGA共通部のレジスタヘッダファイル
 * @author RSO H.Hashiguchi
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
#ifndef	__FPGA_COM_H__
#define	__FPGA_COM_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義
// FPGA時間計測レジスタ//
#define REG_TIM_TMCTRL				0x00400050		// FPGA 時間計測制御レジスタ//
#define REG_TIM_TMCNTH				0x00400052		// FPGA 時間計測カウンタレジスタ H//
#define REG_TIM_TMCNTL				0x00400054		// FPGA 時間計測カウンタレジスタ L//


#endif	// __FPGA_COM_H__
