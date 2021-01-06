///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_prog.h
 * @brief BFWコマンド プロトコル生成関数(プログラム実行関連)ヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, K.Uemori
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/06/18
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo140515-007 2014/06/18 植盛
	SSST, SSENコマンド追加による速度改善
*/
#ifndef	__PROTMCU_PROG_H__
#define	__PROTMCU_PROG_H__

#include "ffw_typedef.h"
#include "mcudef.h"
#include "FFWE20.h"
#include "domcu_prog.h"
#include "ffwmcu_prog.h"

/////////// 関数の宣言 ///////////
// RevRxE2No171004-001 Modify Line
extern FFWERR PROT_MCU_GO(BYTE bMode);				///< BFWMCUCmd_GOコマンドの発行
extern FFWERR PROT_MCU_STOP(void);					///< BFWMCUCmd_STOPコマンドの発行
//RevNo100715-013 Modify Line
extern FFWERR PROT_MCU_STEP(FFWE20_STEP_DATA* pStep, BOOL* pbFfwStepReq, WORD* pwOpecode, BOOL* pbTgtReset, enum FFWENM_STEPCTRL eStepCtrl,
							enum FFWENM_STEPCMD eStepCmd,BYTE byEndian, BYTE byMskSet);	///< BFWMCUCmd_STEPコマンドの発行

extern FFWERR PROT_GetRXPROG(FFWRX_PROG_DATA* pProg);	
													///< BFWRXCmd_GetPROGコマンドの発行
extern FFWERR PROT_MCU_GetRXBRKF(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb, 
								 BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA);
													///< BFWRXCmd_GetBRKFコマンドの発行
//RevNo100715-014 Modify Line
extern FFWERR PROT_MCU_GPBA(DWORD dwmadrBreakAddr, FFWE20_STEP_DATA* pStep, WORD* pwOpecode,BYTE byEndian);
													///< BFWMCUCmd_GPBAコマンドの発行

// RevRxNo140515-007 Append Start
FFWERR PROT_MCU_SetSSST(void);						///< PROT_MCU_SetSSSTコマンドの発行
FFWERR PROT_MCU_SetSSEN(void);						///< PROT_MCU_SetSSENコマンドの発行
// RevRxNo140515-007 Append End

extern void InitProtMcuData_Prog(void);				///< プログラム実行関連コマンド用変数初期化
#endif	// __PROTMCU_PROG_H__
