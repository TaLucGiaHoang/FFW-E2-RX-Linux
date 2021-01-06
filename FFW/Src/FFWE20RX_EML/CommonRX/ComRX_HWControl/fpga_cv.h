////////////////////////////////////////////////////////////////////////////////
/**
 * @file fpga_cv.h
 * @brief カバレッジ用FPGAのレジスタヘッダファイル
 * @author RSO Y.Miyake
 * @author Copyright (C) 2013 Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/06/10
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo130308-001	2013/06/10 三宅
　カバレッジ開発対応
*/
#ifndef	__FPGA_CV_H__
#define	__FPGA_CV_H__


#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義
// カバレッジ設定関連レジスタ//
#define REG_CV_CVCTL				0x00400130		// カバレッジ機能制御レジスタ
	#define CVCTL_CVEN					0x0001			// CVENビット位置
	#define CVCTL_CVBS0					0x0010			// CVBS[0]ビット位置
	#define CVCTL_CVBF0					0x0100			// CVBF[0]ビット位置
	// RevRxNo130308-001-028 Append Line
	#define CVCTL_BR_D					0x1000			// BR_Dビット位置
	// RevRxNo130308-001-027 Append Line
	#define CVCTL_TMP2					0x2000			// TMP2ビット位置(TMP2は、BFWが、ブレーク時にTRCTL0.RSTFが"1"のとき"1"にするビット)
	// RevRxNo130308-001-013 Append Line
	#define CVCTL_TMP					0x4000			// TMPビット位置(TMPは、BFWが、ブレーク時にTRCTL0.TRERRが"1"のとき
														// または、TRCTL0.LOST_Dが"1"のとき														// RevRxNo130308-001-027 Append Line
														// または「「LOF=="1"」かつ、「分岐トレースが出た(BR_D=="1")」」のとき"1"にするビット)	// RevRxNo130308-001-027 Append Line
	#define CVCTL_CVCLR					0x8000			// CVCLRビット位置
#define REG_CV_CVSAH				0x00400132		// カバレッジ機能用実行開始アドレス上位側レジスタ
#define REG_CV_CVSAL				0x00400134		// カバレッジ機能用実行開始アドレス下位側レジスタ
#define REG_CV_CVEAH				0x0040013A		// カバレッジ機能用終了アドレス上位側レジスタ
#define REG_CV_CVEAL				0x0040013C		// カバレッジ機能用終了アドレス下位側レジスタ
#define REG_CV_CVB0H				0x00400140		// カバレッジ機能用ブロック#0開始アドレス上位側レジスタ
#define REG_CV_CVB1H				0x00400144		// カバレッジ機能用ブロック#1開始アドレス上位側レジスタ
#define REG_CV_CVB2H				0x00400148		// カバレッジ機能用ブロック#2開始アドレス上位側レジスタ
#define REG_CV_CVB3H				0x0040014C		// カバレッジ機能用ブロック#3開始アドレス上位側レジスタ

// TRCTL0
	#define TRCTL0_RSTF					0x0100			// RSTFビット位置
	#define TRCTL0_TR_RST				0x0040			// TR_RSTビット位置
	// RevRxNo130308-001-003 Append Line
	#define TRCTL0_RSTVC				0x0200			// RSTVCビット位置
	// RevRxNo130308-001-002 Append Line
	#define MSK_TRCTL0_FLAGS			0x3580			// ビット位置(LOST_D(bit13)、TRERR(bit12)、TMFUL(bit10)、RSTF(bit8)、TM2FUL(bit7))
	// RevRxNo130308-001-016 Append Line
	#define TRCTL0_B_LOST				0x8000			// B_LOSTビット位置

#endif	// __FPGA_CV_H__
