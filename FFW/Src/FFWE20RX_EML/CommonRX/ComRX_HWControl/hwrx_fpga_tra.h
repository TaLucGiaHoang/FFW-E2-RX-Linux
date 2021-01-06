///////////////////////////////////////////////////////////////////////////////
/**
 * @file hwrx_fpga_tra.h
 * @brief トレース E20 FPGAアクセスのヘッダファイル
 * @author RSO H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/11/15
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120626-001 2012/07/12 橋口
　RX63TH トレース駆動能力対応 外部/内蔵トレースビットの追加
・RevRxNo121227-001	2012/12/27 上田
　　ClrTrcFpgaStatus()関数extern宣言追加。
・RevRxNo130730-006	2013/11/15 上田
	E20トレースクロック遅延設定タイミング変更
*/
#ifndef	__HWRX_FPGA_TRA_H__
#define	__HWRX_FPGA_TRA_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義
#define TRC_SET_E20REGNUM		4

//トレースフルブレーク
#define	TRCBRK_OFF			0				// トレースブレークスイッチOFF
#define	TRCBRK_ON			1				// トレースブレークスイッチON
// RevRxNo120626-001 Append Start
// 外部/内蔵トレース設定
#define	IN_TRC				2				// 内蔵トレース設定	
#define	EXT_TRC				0				// 外部トレース設定	
// RevRxNo120626-001 Append End
#define TRC_FPGA_FUNCS_FULL	0x0001			// トレースFPGAの設定がトレースFULL

// TRCTL0
#define TRCTL0_TREN				0x0001
#define TRCTL0_FUNCS			0x000E
#define TRCTL0_TMFIL			0x0010
#define TRCTL0_TM2FUL			0x0080
#define TRCTL0_TMFUL			0x0400
#define TRCTL0_TMBUSY			0x0800
#define TRCTL0_TRERR			0x1000
#define TRCTL0_LOST_D			0x2000

// TC_CTL
#define TC_CTL_TC_GO				0x1000
#define TC_CTL_TC_CLR				0x2000
#define TC_CTL_TC_MD				0x4000
#define TC_CTL_TC_QH				0x07FF

// E20トレースバッファサイズ
#define RD_TMBUFF_BLKSIZE				0x07FF
#define RD_TMBUFFSIZE				0x1000
#define RD_TMBUFF_LAST_ADDR			0x07FE

//クリア後の初期値
#define INIT_TRCTL0					0x0000		//TRCTL0初期値
#define INIT_TRCTL1					0x0002		//TRCTL1初期値
#define INIT_TMBCTL					0x0000		//TMBCTL初期値
#define INIT_TMWA					0x0000		//TMWA初期値
#define INIT_TMRA					0x0000		//TMRA初期値
#define INIT_TMFILD					0x0000		//TMFILD初期値


// グローバル関数の宣言
//トレースFPGAへの設定入力
extern FFWERR ClrTrcFpgaReg(void);
extern FFWERR ClrTrcFpgaBuf(void);
extern FFWERR ClrTrcFpgaMem(WORD wClrMemSize);
extern FFWERR SetTrcFpgaEnable(BOOL);
//RevNo100715-014 Append Start
extern FFWERR SetTrcE20FpgaData(void);
//RevRxNo120626-001 Modify Line
extern FFWERR SetTrcFpgaTrcMode(BYTE byTrcMode);
//RevRxNo120626-001 Modify Line
extern FFWERR CheckTrcFpgaStop(void);
// RevRxNo121227-001 Append Line
extern FFWERR ClrTrcFpgaStatus(void);

//トレースメモリ
extern FFWERR GetTrcFpgaMemBlock(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* rd);

extern FFWERR GetTrcFpgaAllBlockCycleIndex(DWORD dwBlockEnd,DWORD* pdwTrcCmdCntBuf,DWORD* p_dwIndexNum);
extern DWORD GetTrcFpgaAllBlockCycle(void);

//トレースバッファ関数
extern FFWERR GetTrcFpgaBuffData(WORD wTmwba,WORD* pwRd, DWORD dwSetStart);
extern FFWERR SetTrcFpgaBuf2Mem(WORD* pwTmwa,WORD* pwTmwba);

//トレースレジスタリード関数
extern FFWERR GetTrcFpgaStatInfo(BOOL* pbTren,WORD* pwFuncs,BOOL* pbTmFull,BOOL* pbTm2Full);
extern FFWERR GetTrcE20MemTmwa(WORD* pwTmwa);
extern FFWERR GetTrcFpgaTmwba(WORD* pwTmwa);

// RevRxNo130730-006 Append Start
extern void SetTrcE20FpgaAllRegFlg(BOOL bFlg);
extern void SetNeedTrcE20FpgaHotplug(BOOL bFlg);
// RevRxNo130730-006 Append End

extern void InitHwRxFpgaData_Tra(void);	///< E20 トレースFPGAアクセス関数用変数初期化

#endif	// __HWRX_FPGA_TRA_H__
