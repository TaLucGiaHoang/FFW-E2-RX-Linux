////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcurx_ev.h
 * @brief RX共通イベント関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/04/18
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo110825-01 2012/07/13 橋口
　GPB実行イベントへのパスカウント設定不具合対応
・RevRxNo130301-001 2013/04/18 上田
	RX64M対応
*/
#ifndef	__MCURX_EV_H__
#define	__MCURX_EV_H__


#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義

//EVCNT
#define MSK_EVCNT_MODE2CNTSEL			0x3		//
#define SIFT_EVCNT_MODE2CNTDEL			8		//

 //RevNo100715-014 Append Line, RevRxNo130301-001 Modify Line
#define EV_SET_REGNUM		55					//設定イベントレジスタの数
#define BRK_SET_REGNUM		3					//設定ブレークレジスタの数


//レジスタ初期値設定
#define INIT_OCD_REG_EV_EVEPCA		0		//実行PCイベントアドレスクリア値
#define INIT_OCD_REG_EV_EVEOPA		0		//オペランドイベントアドレスクリア値
#define INIT_OCD_REG_EV_EVEOPAM		0		//オペランドイベントアドレスマスククリア値
#define INIT_OCD_REG_EV_EVEOPD		0		//オペランドイベントデータクリア値
#define INIT_OCD_REG_EV_EVEOPDM		0		//オペランドイベントデータマスククリア値
// RevRxNo130301-001 Append Line
#define INIT_OCD_REG_EV_EVEOPSB		1		//オペランドアクセスイベントバス選択クリア値

#define INIT_OCD_REG_EV_EVEPCPREE	0		//実行前PCブレークイベント動作許可
#define INIT_OCD_REG_EV_EVEPCPE		0		//実行通過PCブレークイベント動作許可
#define INIT_OCD_REG_EV_EVEPCPFLG	0		//実行PC通過イベント成立結果
#define INIT_OCD_REG_EV_EVEOPE		0		//オペランドアクセスイベント動作許可
#define INIT_OCD_REG_EV_EVEOPFLG	0		//オペランドアクセスイベント成立結果
#define INIT_OCD_REG_EV_EVECNT		1		//イベント成立回数カウント
#define	INIT_OCD_REG_EV_EVECNTSEL	0		//成立回数指定イベント選択
#define INIT_OCD_REG_EV_EVEBRKTSC	0		//イベントブレーク/トレース開始イベント組合せ指定
#define INIT_OCD_REG_EV_EVEBRKPCP	0		//イベントブレーク用実行PC通過イベント選択
#define INIT_OCD_REG_EV_EVEBRKOP	0		//イベントブレーク用オペランドアクセスイベント選択

//ブレーク設定モード
#define OCDBRK_PRE			0x0001	//実行前PCブレーク設定
#define OCDBRK_EV			0x0002	//イベントブレーク設定

// プロトタイプ宣言（構造体変数定義、アクセス関数）
extern FFWERR SetEventEnable(BOOL bEnable);
extern FFWERR SetBreakEvEnable(BOOL bEnable);
extern FFWERR SetPreBreakEvEnable(BOOL bEnable);
extern FFWERR InitCombEvent(DWORD dwCombi);					// RevNo120131-001 Modify Line

extern FFWERR SetEvPpcCntEvEnable(DWORD dwPpcPcEv, DWORD dwPpcOpcEv);


// プロトタイプ宣言（コマンド処理部）

extern FFWERR SetEvBreakData(FFWRX_COMB_BRK* evBrk);
extern FFWERR SetEvTraceData(FFWRX_COMB_TRC* evTrc);
extern FFWERR SetEvPerfData(DWORD dwPerfNo, FFWRX_COMB_PERFTIME* evPerfTime);

extern FFWERR SetExecPcEv(DWORD dwEvNum, MADDR madrSetAddr);
extern FFWERR SetOpeAccEv(DWORD dwEvNum, FFWRX_EV_DATA* ev);
extern FFWERR SetEvCnt(const FFWRX_EVCNT_DATA* evcnt);			// RevNo110825-01 Modify Line
extern FFWERR SetTrgEv(BYTE byEvKind, FFWRX_TRG_DATA evTrg);

extern FFWERR SearchEvPreBrkPoint(DWORD dwmadrAddr,BOOL* pbBrkEna);
extern FFWERR DisableEvPreBrkPoint(BOOL* pbBrkEna, DWORD* pdwPreEvCombi);
extern FFWERR EnableEvPreBrkPoint(DWORD* dwPreEvCombi);

extern FFWERR SearchExecPcEvNum(BOOL* pbFindFlg,DWORD* dwEvNum);

extern FFWERR ClrEvReg(void);
extern FFWERR ClrBrkReg(void);
extern FFWERR InitEveBrk(void);

extern FFWERR SetEvPreBreakOn(FFWRX_COMB_BRK* evBrk);

extern FFWERR SetEvRegDataAll(void);
extern FFWERR SetEvRegDataBeforeGo(void);
extern FFWERR SetOcdBrkRegData(BYTE byOcdBrkMode);

extern FFWERR SetRegEvEnable(DWORD dwPreBrk, DWORD dwExecPcSet, DWORD dwOpcSet);
extern FFWERR GetEvExecPcCompInfo(DWORD* dwExecPc);
extern FFWERR GetEvOpcCompInfo(DWORD* dwOpc);

extern void InitMcuRxData_Ev(void);	///< イベント関連コマンド用FFW内部変数の初期化

#endif	// __MCURX_EV_H__
