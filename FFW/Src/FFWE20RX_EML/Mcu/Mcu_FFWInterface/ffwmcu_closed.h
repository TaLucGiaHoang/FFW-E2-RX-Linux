///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_closed.h
 * @brief 非公開コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
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
#ifndef	__FFWMCU_CLOSED_H__
#define	__FFWMCU_CLOSED_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義

//JTAG用 定義
#define IR_LEN_MAX	1024
#define DR_LEN_MAX	1024

// シリアルアクセスレジスタ(ライト用)
#define	SAR_W_E2C0			0x84
#define	SAR_W_E2C1			0x85
#define	SAR_W_DSETUP		0x88
#define	SAR_W_DMAADDR		0x8A
#define	SAR_W_DMATYPE		0x8B
#define	SAR_W_DMAWRITE		0x8C
#define	SAR_W_MONEXEC		0x8E
#define	SAR_W_MONCODE		0x8F

// シリアルアクセスレジスタ(リード用)
#define	SAR_R_MCU_MR		0xC0	
#define	SAR_R_MCU_FR		0xC1	
#define	SAR_R_MCU_DR		0xC2	
#define	SAR_R_MCU_VR		0xC3	
#define	SAR_R_C2E0			0xC4	
#define	SAR_R_C2E1			0xC5	
#define	SAR_R_MCUSTAT		0xC6	
#define	SAR_R_BAUDC			0xC7
#define	SAR_R_DSETUP		0xC8	
#define	SAR_R_DMAREAD		0xCC	
#define	SAR_R_DMARDAT		0xCD	
#define	SAR_R_MCUPC			0xCE	
#define	SAR_R_MCUPSW		0xCF	


// グローバル関数の宣言

extern void InitFfwIfMcuData_Closed(void);

#endif	// __FFWMCU_CLOSED_H__
