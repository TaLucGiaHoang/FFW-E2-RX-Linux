///////////////////////////////////////////////////////////////////////////////
/**
 * @file hwrx_fpga_cv.h
 * @brief カバレッジ計測関連 E20 FPGAアクセスのヘッダファイル
 * @author RSO Y.Miyake
 * @author Copyright (C) 2013 Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/08/05
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo130308-001	2013/08/05 三宅
　カバレッジ開発対応
*/
#ifndef	__HWRX_FPGA_CV_H__
#define	__HWRX_FPGA_CV_H__

#include "ffw_typedef.h"
#include "mcudef.h"
#include "ffwrx_cv.h"

// define定義
// クリア後の初期値
#define REG_CV_DATA_INIT			0x00000000		// カバレッジ関連レジスタの初期値


// グローバル関数の宣言
//カバレッジFPGAへの設定入力
extern FFWERR SetFpgaCvEnable(BOOL bCven);
extern FFWERR SetCVBBlk(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvbData);
extern FFWERR SetCVBEnable(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvbData);
// RevRxNo130308-001-027 Delete		// GetFpgaRstf()
// RevRxNo130308-001-027 Append Line
extern FFWERR GetFpgaCvctl_Tmp2(BOOL* pbRstf);
// RevRxNo130308-001-013 Modify Line
extern FFWERR GetFpgaLost_dOrTrerrOrTmp(BOOL* pbLost_dOrTrerr);
extern FFWERR GetFpgaCVBF(enum FFWRXENM_CV_BLKNO eBlkNo, BOOL* pbCvbf);
extern FFWERR ClrFpgaTrctl0_RstfLost_dTrerr(void);
extern FFWERR ClrFpgaTrctl0_Rstf(void);		// RevRxNo130308-001-038 Append Line
extern FFWERR SetFpgaTrctl0_Tr_rst(void);
extern FFWERR SetFpgaCvctl_Cvclr(void);
extern FFWERR ClrCvFpgaReg(void);
extern FFWERR ClrCvData(void);
// RevRxNo130308-001-003 Append Line
extern FFWERR SetFpgaTrctl0_Rstvc(void);
// RevRxNo130308-001-006 Append Line
extern FFWERR GetFpgaCven(BOOL* pbCven);
// RevRxNo130308-001-013 Append Line
extern FFWERR SetFpgaCvctl_Tmp(BOOL bTmp);
// RevRxNo130308-001-027 Append Line
extern FFWERR SetFpgaCvctl_Tmp2(BOOL bTmp);
// RevRxNo130308-001-016 Append Line
extern FFWERR SetFpgaTrctl0_B_lost(void);

#endif	// __HWRX_FPGA_CV_H__
