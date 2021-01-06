#pragma once
///////////////////////////////////////////////////////////////////////////////
/**
* @file e2_asprx_fpga.h
* @brief FPGAのASPRX関連レジスタ定義
* @author REL H.Ohga
* @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
* @date 2017/10/23
*/
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2017/10/23 H.Ohga
*/

#include "e2_asp_fpga.h"

#ifndef __E2_ASPRX_FPGA_H__
#define __E2_ASPRX_FPGA_H__

#define	MONEV_MEOEN			(0x07000600)
#define	MONEV_MEOEN_BIT		(1<<0)

#define	DETS1				(0x07000604)
#define DETS2				(0x07000608)

#define	DETM1				(0x07000610)
#define DETM2				(0x07000614)

#define MRESCTRL         	(0x07000620)
#define RSTME				(1<<0)
#define RSTEC				(1<<1)
#define RSTMEREG			(1<<16)
#define RSTECREG			(1<<17)

#define	EVLACTST1			(0x07000630)
#define	EVL_TRSTOPST		(1<<0)
#define	EVL_BRKST			(1<<1)
#define	SRAM_STOP			(1<<2)
#define	SRAM_BREAK			(1<<3)

#define	EVLACTST2			(0x07000634)
#define	EVL_RST				(1<<0)

//Power Monitor
#define PWRM_MAXMIN			(0x070015E8)
#define PWRM_RNGMAX_MASK	(0x0FFF0000)
#define PWRM_RNGMIN_MASK	(0x00000FFF)
#define PWRM_MAXMIN_INIT	(0x00000FFF)

#endif	// __E2_ASPRX_FPGA_H__
