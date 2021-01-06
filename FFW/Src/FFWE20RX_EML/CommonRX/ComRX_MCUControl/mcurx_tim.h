////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcurx_tim.h
 * @brief 実行時間計測関連コマンドのヘッダファイル
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
#ifndef	__MCURX_TIM_H__
#define	__MCURX_TIM_H__


#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義
#define INIT_OCD_REG_EV_EVEPSPCP	0	// パフォーマンス開始実行通過イベントクリア値
#define INIT_OCD_REG_EV_EVEPSOP		0	// パフォーマンス開始オペランドイベントクリア値
#define INIT_OCD_REG_EV_EVEPEPCP	0	// パフォーマンス終了実行通過イベントクリア値
#define INIT_OCD_REG_EV_EVEPEOP		0	// パフォーマンス終了オペランドイベントクリア値
#define INIT_EV_PPC_TRG				0	// パフォーマンストリガクリア値

#define PPC_USR_EMLUSE		0x00000003	//COMBで設定変更イベントがPPC をEMLが使用する場合

//RevNo100715-028 Append Line
#define PPC_SET_REGNUM 4				//設定が必要なPPCレジスタの最大値

// プロトタイプ宣言
extern FFWERR ClrPerfReg(void);
//RevNo100715-028 Append Line
extern FFWERR SetPpcRegData(void);
extern FFWERR SetPerfAccCtrl(enum FFWRX_EVCOMBI_PPC_USR_CTRL ePpcUsrCtrl);
extern FFWERR SetPpcCtrl(DWORD dwPpcCh,DWORD dwPPCSR);
extern FFWERR SetPPCEnable(BOOL bEnable);
extern void ConvPPCCTRL_DATA2PCCSRCls3(BYTE byPpcCh, FFWRX_PPCCTRL_DATA* pPev,DWORD* dwPPCSR);
extern void ConvPPCCTRL_DATA2PCCSRCls2(BYTE byPpcCh, FFWRX_PPCCTRL_DATA* pPev,DWORD* dwPPCSR);
extern void ClrPpcCtrlEnable(DWORD dwPPCSR,DWORD* dwClrPPCSR);
extern FFWERR GetPpcEnaInfo(DWORD dwPpcCh,BOOL* pbPpcRun,DWORD* pdwPpcMode);
extern FFWERR SetPpcCntEv(DWORD dwSetEvNum);
extern FFWERR GetPpcCnt(DWORD dwPpcCh,DWORD* pdwPpcCnt,BOOL* pbTimOvf);
extern FFWERR ClrPpcCnt(DWORD dwPpcCh);
extern void InitMcuRxData_Tim(void);	///< パフォーマンス関連コマンド用FFW内部変数の初期化

#endif	// __MCURX_TIM_H__
