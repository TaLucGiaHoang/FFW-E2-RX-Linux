////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_ev.h
 * @brief RX共通イベント関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, Y.Miyake, K.Uemori
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/05/20
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo110825-01 2012/07/13 橋口
　GPB実行イベントへのパスカウント設定不具合対応
・RevRxNo120910-001	2012/10/17 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・EV_BUSSEL_CPU_BUS、EV_BUSSEL_DMAC_DTC_BUS の定義追加。
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo130308-001 2013/04/26 三宅
　カバレッジ開発対応
*/
#ifndef	__FFWRX_EV_H__
#define	__FFWRX_EV_H__


#include "ffw_typedef.h"
#include "mcudef.h"



// 定数定義
#define EV_ENABLE_REGNUM	0x00000003			// イベント設定有効化/無効化設定対象レジスタ数

#define EVNO_PC_START		0x0	
#define EVNO_OP_START		0x10
#define EV_NO_0				0
//OCDイベントビット定義

//EVCNT
#define EVCNT_MIN			0x1					//イベントカウント最小値
#define EVCNT_MAX			0x100				//イベントカウント最大値

//COMB
#define COMB_EVKIND_COMB		0x01	//COMBで設定変更イベントが組み合わせの場合
#define COMB_EVKIND_BRK			0x02	//COMBで設定変更イベントがBREAKの場合
#define COMB_EVKIND_TRC			0x04	//COMBで設定変更イベントがTRCの場合
#define COMB_EVKIND_PPC			0x18	//COMBで設定変更イベントがPPCの場合
#define COMB_EVKIND_PPC_CH0		0x08	//COMBで設定変更イベントがPPC CH0の場合
#define COMB_EVKIND_PPC_CH1		0x10	//COMBで設定変更イベントがPPC CH1の場合
#define COMB_EVKIND_PPC_USR		0x20	//COMBで設定変更イベントがPPC USRの場合
#define COMB_EVKIND_ALL		0x3F		//COMBで設定変更イベントが全設定の場合
// RevRxNo121026-001 Append Start
#define COMB_EVKIND_PPC_MSK		0x38	//COMBのPPCユーザ利用、ch1,0マスク値
#define COMB_EVKIND_PPC_NON		0x00	//COMBで設定変更イベントがPPC未使用の場合
// RevRxNo121026-001 Append End

// EVETTRG
#define EVTRG_EVKIND_ALL	0x7			//EVTRGで設定変更機能すべて
#define EVTRG_EVKIND_TRC	0x1			//EVTRGで設定変更機能トレース
#define EVTRG_EVKIND_PPC_CH0	0x2			//EVTRGで設定変更機能パフォーマンス CH0
#define EVTRG_EVKIND_PPC_CH1	0x4			//EVTRGで設定変更機能パフォーマンス CH1
#define CLR_EVTRG_PPC_CH0		0xFFFFFFFC	//
#define CLR_EVTRG_PPC_CH1		0xFFFFFFF3	//
// RevRxNo121026-001 Append Start
#define EVTRG_EVKIND_PPC_MSK	0x06		//EVTRGでPPC ch1,0選択
#define EVTRG_EVKIND_PPC_NON	0x00		//EVTRGでPPC未使用
// RevRxNo121026-001 Append End

#define EVETTRG_TRC_START	0x00000001			//トレース開始トリガ
#define EVETTRG_TRC_END		0x00000002			//トレース終了トリガ
#define EVETTRG_TRC_PCKUP	0x00000004			//トレース抽出
#define EVETTRG_TRC_ALL		0x00000007			//トレーストリガ全部
// RevRxNo130308-001-008 Append Line
#define EVETTRG_TRC_NON		0x00000007			//トレーストリガ全て使用しない

// RevRxNo120910-001 Append Start
// EV
#define	EV_BUSSEL_CPU_BUS		0x00000001		// "オペランドアクセスイベントの検出対象バス"：CPUバス。
#define	EV_BUSSEL_DMAC_DTC_BUS	0x00000002		// "オペランドアクセスイベントの検出対象バス"：DMAC/DTCバス。
// RevRxNo120910-001 Append End

// 構造体宣言



// プロトタイプ宣言（構造体変数定義、アクセス関数）
enum FFWRX_EVCOMBI_PPC_USR_CTRL GetPerfUsrCtrl(void);
extern FFWERR ChkEvBreakData(DWORD dwCombi, FFWRX_COMB_BRK* evBrk);
extern FFWERR ChkEvTraceData(DWORD dwCombi, FFWRX_COMB_TRC* evTrc);
extern FFWERR ChkEvPerfData(DWORD dwPerfNo, DWORD dwCombi, FFWRX_COMB_PERFTIME* evPerfTime);

extern void EscExecPcBreakEv(BOOL bEscape);
extern void EscExecTrcEv(BOOL bEscape);
extern void EscExecPerfEv(BOOL bEscape);
extern void EscExecEv(DWORD byEvKind, FFWRX_COMB_BRK* evBrk, FFWRX_COMB_TRC* evTrc, FFWRX_COMB_PERFTIME* evPerfTime, BOOL bEscape);

extern void GetRegEvEnable(DWORD* pdwPreBrk, DWORD* pdwExecPcSet, DWORD* pdwOpcSet);

extern BOOL EscEvPreBrkPoint(BYTE byMode, DWORD* dwPreEvCombi);
//RevRxNo120713-001 Delete SetEveEXECPCSET(),SetEveOPCSET()

// プロトタイプ宣言　内部管理変数操作
extern void GetEvAddrInfo(DWORD dwEvNum, FFWRX_EV_ADDR* EvAddr);
extern FFWRX_EVCNT_DATA* GetEvCntInfo(void);
extern BOOL SearchEvCnt(enum FFWRX_EV eEvNum);				// RevNo110825-01 Append Line
extern DWORD GetCombiInfo(void);
extern FFWRX_COMB_BRK* GetEvCombBrkInfo(void);
extern void SetEvCombBrkInfo(FFWRX_COMB_BRK* evBrk);
extern FFWRX_COMB_TRC* GetEvCombTrcInfo(void);
extern void SetEvCombTrcInfo(FFWRX_COMB_TRC* evTrc);
extern FFWRX_COMB_PERFTIME* GetEvCombPerfInfo(void);
extern void SetEvCombPerfInfo(FFWRX_COMB_PERFTIME* evPerf);

// V.1.02 No.31 Class2 トレース対応 Append Line
extern FFWRX_EV_OPC* GetEvOpc(DWORD dwEvNum);
// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append Line
extern FFWRX_TRG_DATA* GetEvTrgInfo(void);
extern enum FFWRX_EVCOMBI_PPC_USR_CTRL GetPerfUsrCtrl(void);
extern void SetPerfUsrCtrl(enum FFWRX_EVCOMBI_PPC_USR_CTRL PpcUsrCtrl);

extern void InitFfwIfRxData_Ev(void);	///< イベント関連コマンド用FFW内部変数の初期化


#endif	// __FFWRX_EV_H__
