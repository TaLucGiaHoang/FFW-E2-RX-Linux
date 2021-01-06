///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_prog.h
 * @brief プログラム実行関連コマンドのヘッダファイル
 * @author RSD Y.Minami, H.Hashiguchi, S.Ueda, K.Uemori, SDS T.Iwata, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120131-001 2012/07/13 橋口
  GPB実行中の実行前、イベントブレーク不具合対応
・RevRxNo130730-005 2013/11/13 上田
	内蔵ROM有効/無効判定処理改善
・RevRxNo140515-007 2014/06/18 植盛
	SSST, SSENコマンド追加による速度改善
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#ifndef	__DOMCU_PROG_H__
#define	__DOMCU_PROG_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// enum定義
enum FFWENM_PROGCMD_NO {	// プログラム実行コマンド種別
	PROGCMD_GO=0,				// GOコマンド用
	PROGCMD_GB,					// GBコマンド用
	PROGCMD_RSTG,				// RSTGコマンド用
	PROGCMD_GPB,				// GPBコマンド用
	PROGCMD_GPBA,				// GPBAコマンド用
	PROGCMD_STEP,				// STEPコマンド用
	PROGCMD_STEPOVER,			// STEPOVERコマンド用
	PROGCMD_STEPGPB,			// STEPGPBコマンド用

	PROGCMD_GB_BRKSTEP,			// GBコマンドのブレークポイントアドレスからのSTEP用
	PROGCMD_GPB_BRKSTEP,		// GPBコマンドのブレークポイントアドレスからのSTEP用
	PROGCMD_STEPGPB_BRKSTEP,	// STEPGPBコマンドのブレークポイントアドレスからのSTEP用

	PROGCMD_GB_BRKGB,			// GBコマンドのブレークポイントアドレスからのSTEP実行後のGB用
	PROGCMD_GPB_BRKGPB,			// GPBコマンドのブレークポイントアドレスからのSTEP実行後のGPB用
	PROGCMD_STEPGPB_BRKSTEPGPB	// STEPGPBコマンドのブレークポイントアドレスからのSTEP実行後のSTEPGPB用
};
enum FFWENM_STEPCTRL {	// シングルステップ制御
	STEP_BFWCTRL=0,				// BFW制御によるステップ実行
	STEP_FFWCTRL				// FFW制御によるステップ実行
};

// define 定義
// 2008.11.4 INSERT_BEGIN_E20RX600(+6) {
// RevRxE2No171004-001 Append Start
#define BRKF_BREAKFACT_SRAMFULL_BIT		0x00000080	// ブレーク要因 SRAMフルブレーク成立ビット
#define BRKF_BREAKFACT_E2ACTION_BIT		0x00000040	// ブレーク要因 E2アクションブレーク成立ビット
// RevRxE2No171004-001 Append End
#define BRKF_BREAKFACT_TRCFULL_BIT		0x00000020	// ブレーク要因 トレースフルブレーク成立ビット
#define BRKF_BREAKFACT_COMMAND_BIT		0x00000010	// ブレーク要因 コマンド通信成立ビット
#define BRKF_BREAKFACT_EVENT_BIT		0x00000008	// ブレーク要因 イベントブレーク成立ビット
#define BRKF_BREAKFACT_AFTER_BIT		0x00000004	// ブレーク要因 実行後PCブレーク成立ビット
#define BRKF_BREAKFACT_BEFORE_BIT		0x00000002	// ブレーク要因 実行前PCブレーク成立ビット
#define BRKF_BREAKFACT_SW_BIT			0x00000001	// ブレーク要因 ソフトウェアブレーク成立ビット
// 2008.11.4 INSERT_END_E20RX600 }

#define TMOUT_WAITBREAK	(3000)	// 3秒


#define TRMODE_NORMAL	TRMODE_SETDATA1_DATA
#define TRMODE_STEP		TRMODE_SETDATA2_DATA

// RevRxE2No171004-001 Append Start
#define DISABLE_BREAK	0		// ブレークなし実行
#define ENABLE_BREAK	1		// ブレーク付き実行
// RevRxE2No171004-001 Append End

// 構造体の定義


#include "FFWE20.h"
#include "ffwmcu_prog.h"
// グローバル関数の宣言
extern FFWERR DO_GO(enum FFWENM_STARTADDR_SET eStartAddrSet, MADDR madrStartAddr);	///< プログラムのフリー実行
extern FFWERR DO_GB(enum FFWENM_STARTADDR_SET eStartAddrSet, MADDR madrStartAddr);	///< プログラムのブレーク付き実行
extern FFWERR DO_RSTG(void);							///< プログラムのリセット付き実行
// RevRxNo140515-008 Modify Line
extern FFWERR DO_GPB(enum FFWENM_STARTADDR_SET eStartAddrSet, MADDR madrStartAddr,	MADDR madrBreakAddr, BOOL bStepOverExec);	///< プログラムのプログラムブレークポイント付き実行
extern FFWERR DO_GPBA(enum FFWENM_STARTADDR_SET eStartAddrSet, MADDR madrStartAddr,	
					  MADDR madrBreakAddr, FFWE20_STEP_DATA* pStep);				///< プログラムの実行後ブレークポイント付き実行
extern FFWERR DO_STOP(void);							///< プログラム実行停止
// 2008.11.19 MODIFY_BEGIN_E20RX600(-NN,+NN) {
extern FFWERR DO_STEP(FFWE20_STEP_DATA* pStep, enum FFWENM_STEPCMD eStepCmd);	///< プログラムのステップ実行
// 2008.11.19 MODIFY_END_E20RX600 }

extern FFWERR DO_GetRXPROG(FFWRX_PROG_DATA* pProg);								///< プログラム実行中の情報入手
extern FFWERR DO_GetRXBRKF(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb, 
						   BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA);		///< ブレーク要因の参照

extern FFWERR DO_SetSSST(void);							///< 連続ステップ開始処理	// RevRxNo140515-007 Append Line
extern FFWERR DO_SetSSEN(void);							///< 連続ステップ終了処理	// RevRxNo140515-007 Append Line

extern BOOL IsMcuRun(void);								///< ユーザプログラム実行状態の取得
extern void SetMcuRunState(BOOL bState);				///< ユーザプログラム実行状態の設定
extern BOOL GetMcuRunState(void);						///< ユーザプログラム実行状態の取得
extern enum FFWENM_PROGCMD_NO GetMcuRunCmd(void);		///< プログラム実行コマンド種別の取得
extern void SetMcuRunCmd(enum FFWENM_PROGCMD_NO eRunCmd);	///< プログラム実行コマンド種別の設定
// RevRxE2LNo141104-001 Delete Line: IsMcuStateNormal()

extern void MakeBreakMode(enum FFWENM_PROGCMD_NO eRunCmd, DWORD* pdwNewBMode);

extern FFWERR McuBreak_RX(void);						///< プログラム実行停止後の処理(RX用)

extern void SetReadPmodeInRomDisRegFlg(BOOL bFlg);	///< 内蔵ROM無効拡張モード状態参照レジスタリードフラグの設定	// RevRxNo130730-005 Append Line

/////////////////////////////////////////////////////////////////////
// Ver.1.01 2010/08/17 SDS T.Iwata
extern FFWERR SetPmodeInRomReg2Flg(void);			///< 内蔵ROM無効拡張モード状態の取得
extern void SetPmodeInRomDisFlg(BOOL bState);				///< 内蔵ROM無効拡張モードフラグの設定
extern BOOL GetPmodeInRomDisFlg(void);						///< 内蔵ROM無効拡張モードフラグの取得
/////////////////////////////////////////////////////////////////////

extern void GetGpbPcEv(DWORD* pdwEvNo, MADDR* pmaddrEvAddr);		// GPBのイベント情報取得 //RevNo120131-001 Append Line

extern void SetStepContinueStartFlg(void);				///< 連続ステップ開始フラグ設定		// RevRxNo140515-007 Append Line
extern BOOL GetStepContinueStartFlg(void);				///< 連続ステップ開始フラグ値取得	// RevRxNo140515-007 Append Line

extern void InitFfwCmdMcuData_Prog(void);				///< プログラム実行関連コマンド用変数初期化

#endif	// __DOMCU_PROG_H__
