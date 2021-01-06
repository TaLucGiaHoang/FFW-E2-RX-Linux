////////////////////////////////////////////////////////////////////////////////
/**
 * @file protrx_tra.cpp
 * @brief BFWコマンド プロトコル生成関数（トレース関連） ソースファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/04/17
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120626-001 2012/07/12 橋口
　RX63TH トレース駆動能力対応 PROT_SetTrcBrk()→PROT_SetTRCMODE()に変更
・RevRxNo130408-001	2014/04/17 上田
	外部トレースデータ8ビット幅対応
*/

#include "protrx_tra.h"
#include "do_common.h"
#include "prot.h"
#include "comctrl.h"
#include "errchk.h"
#include "prot_common.h"
#include "mcurx_tra.h"
#include "ffwrx_tra.h"
#include "ffwmcu_mcu.h"
#include "dorx_tra.h"

//==============================================================================
/**
 * FFW はコマンドコードBFWCmd_GetRD_SPL を送信し、トレースデータを取得する。
 * @param dwBlockStart トレースメモリ開始ブロック
 * @param dwBlockEnd   トレースメモリ終了ブロック
 * @param wRd          トレースメモリデータ格納領域アドレス
 * @retval FFWエラーコード
 *
 * ※wRdに指定するトレースメモリデータ格納領域のサイズは、
 * 　トレース機能/カバレッジ機能有効設定およびトレースデータ出力端子幅により異なる。
 */
//==============================================================================
FFWERR PROT_GetRD2_SPL(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* wRd)
{
	int		eRet;
	WORD	wCmdCode;			// 送信コマンドコード
	FFWERR	ferr;
	WORD	wData;
	DWORD	i, j;	// RevRxNo130408-001 Modify Line
	BYTE*	pbyTraData;			// トレースデータをリードする領域のアドレス
	BYTE*	pbyData;
	WORD	wTmp;
	// RevRxNo130408-001 Append Start
	FFWRX_RM_DATA*	pRm;
	FFWMCU_DBG_DATA_RX*	pDbg;
	DWORD	dwBlockNum;
	DWORD	dwRdBlockSize;
	DWORD	dwRcvDataSize;
	DWORD	dwRdBlockStart;
	BOOL	bConv;
	// RevRxNo130408-001 Append End

	pRm = GetRmDataInfo();	// RevRxNo130408-001 Append Line
	pDbg = GetDbgDataRX();	// RevRxNo130408-001 Append Line

	/* コマンドコードを送信 */
	wCmdCode = BFWCMD_GET_RD_SPL;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	/* リザーブ送信 */
	wData = 0x0000;
	eRet = PutData2(wData);
	if (!eRet) {
		return FFWERR_COM;
	}

	//開始ブロック
	eRet = PutData4(dwBlockStart);

	//終了ブロック
	eRet = PutData4(dwBlockEnd);


	/* エラーコードを受信 */
	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	// RevRxNo130408-001 Modify Start
	// トレース機能有効で、DATA 8ビット、SYNC 2ビットの場合
	if ((pDbg->eTrcFuncMode == EML_TRCFUNC_TRC) && ((pRm->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D8S2)) {
		bConv = TRUE;	// トレースメモリデータフォーマット変換必要
	} else {
		bConv = FALSE;	// トレースメモリデータフォーマット変換不要
	}

	// 1トレースメモリブロック分のwRd[]バッファサイズ取得
	dwRdBlockSize = GetRdBufSizeTraceBlock();

	// 1トレースメモリブロック分のデータ受信用バッファ確保
	dwRcvDataSize = RD_TRC_BLOCK;
	pbyTraData = new BYTE[dwRcvDataSize];

	dwBlockNum = dwBlockEnd - dwBlockStart + 1;
	for (i = 0 ; i < dwBlockNum; i++) {
		// wRd[]のトレースメモリデータ格納位置設定
		dwRdBlockStart = i * dwRdBlockSize;

		// 1ブロック分のトレースメモリデータを受信
		pbyData = pbyTraData;
		eRet = GetDataN(dwRcvDataSize, pbyData);
		if (!eRet) {
			delete [] pbyTraData;
			return FFWERR_COM;
		}

		if (bConv == FALSE) {	// フォーマット変換不要の場合
			for (j = 0; j < dwRdBlockSize; j+=4) {
				// *pbyData ～ *(pbyData + 3)のデータをwRd[j], wRd[j+2]に格納
				wTmp = static_cast<WORD>(*pbyData);
				wRd[dwRdBlockStart + j + 2] = ((wTmp << 8) & 0xff00) | static_cast<WORD>(*(pbyData + 1));
				pbyData += 2;

				wTmp = static_cast<WORD>(*pbyData);
				wRd[dwRdBlockStart + j]     = ((wTmp << 8) & 0xff00) | static_cast<WORD>(*(pbyData + 1));
				pbyData += 2;
			}

		} else {	// フォーマット変換必要の場合
			// DATA 8ビット、SYNC 2ビットフォーマットを、DATA 4ビット、SYNC 1ビットフォーマットに変換して、
			// wRd[j], wRd[j+2], wRd[j+4], wRd[j+6]へ格納
			for (j = 0; j < dwRdBlockSize; j+=8) {
				// *pbyData ～ *(pbyData + 3)のDATA 8ビット、SYNC 2ビットフォーマットデータを
				// DATA 4ビット、SYNC 1ビットフォーマットに変換してwRd[j], wRd[j+2], wRd[j+4], wRd[j+6]に格納
				wTmp = static_cast<WORD>(*pbyData);
				wTmp = ((wTmp << 8) & 0xff00) | static_cast<WORD>(*(pbyData + 1));
				wRd[dwRdBlockStart + j + 4] = ((wTmp << 4) & 0x7000)		// トレース情報種別-2
												| ((wTmp >> 1) & 0x0400)	// コマンド/データ識別信号-2
												| (wTmp & 0x000f);			// コマンド/データ-2
				wRd[dwRdBlockStart + j + 6] = (wTmp & 0x7000)				// トレース情報種別-3
												| ((wTmp >> 5) & 0x0400)	// コマンド/データ識別信号-3
												| ((wTmp >> 4) & 0x000f);	// コマンド/データ-3
				pbyData += 2;

				wTmp = static_cast<WORD>(*pbyData);
				wTmp = ((wTmp << 8) & 0xff00) | static_cast<WORD>(*(pbyData + 1));
				wRd[dwRdBlockStart + j] =     ((wTmp << 4) & 0x7000)		// トレース情報種別-0
												| ((wTmp >> 1) & 0x0400)	// コマンド/データ識別信号-0
												| (wTmp & 0x000f);			// コマンド/データ-0
				wRd[dwRdBlockStart + j + 2] = (wTmp & 0x7000)				// トレース情報種別-1
												| ((wTmp >> 5) & 0x0400)	// コマンド/データ識別信号-1
												| ((wTmp >> 4) & 0x000f);	// コマンド/データ-1
				pbyData += 2;
			}
		}
	}

	delete [] pbyTraData;
	// RevRxNo130408-001 Modify End

	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}

	return FFWERR_OK;

}

//=============================================================================
/**
 * トレース識別子1の数をカウントコマンド
 * @param wBlockNum カウント開始ブロック
 * @param wBlockCnt カウントするブロック数
 * @param pbyPktCn  取得した識別子1の数をカウントするポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetRCYCntN(WORD wBlockNum, WORD wBlockCnt, BYTE *pbyPktCnt)
{
	int		eRet;
	WORD	wCmdCode;			// 送信コマンドコード
	WORD	wData;
	WORD	wBuf;
	FFWERR	ferr;

	/* コマンドコードを送信 */
	wCmdCode = BFWCMD_GETRCYN;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	/* リザーブ送信 */
	wData = 0x0000;
	eRet = PutData2(wData);
	if (!eRet) {
		return FFWERR_COM;
	}

	/* 取得トレースブロック指定 */
	wData = wBlockNum;
	eRet = PutData2(wData);
	if (!eRet) {
		return FFWERR_COM;
	}

	/* 取得トレースブロック数 */
	wData = WORD(wBlockCnt/4);
	eRet = PutData2(wData);
	if (!eRet) {
		return FFWERR_COM;
	}

	/* データ取得 */
	eRet = GetDataN(wBlockCnt,pbyPktCnt);
	if (!eRet) {
		return FFWERR_COM;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * トレースブレークフラグ設定
 * @param byTrcMode 
 *         bit0 1:フルブレーク有効 0:フルブレーク無効
 *         bit1 1:内蔵トレース 0:外部トレース
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_SetTRCMODE(BYTE byTrcMode)				// RevRxNo120626-001 Modify Line
{
	int		eRet;
	WORD	wCmdCode;			// 送信コマンドコード
	BYTE	byData;
	WORD	wData;
	WORD	wBuf;
	FFWERR	ferr;

	/* コマンドコードを送信 */
	wCmdCode = BFWCMD_SET_TRCMODE;			// RevRxNo120626-001 Modify Line
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	/* リザーブ送信 */
	wData = 0x0000;
	eRet = PutData2(wData);
	if (!eRet) {
		return FFWERR_COM;
	}

	// トーレスモード設定 
	byData = byTrcMode;						// RevRxNo120626-001 Modify Line
	eRet = PutData1(byData);
	if (!eRet) {
		return FFWERR_COM;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}
//RevNo100715-014 Append Start
//=============================================================================
/**
 * トレースブレークフラグ設定
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_TRC_SetDummyData(void)
{
	int		eRet;
	WORD	wCmdCode;			// 送信コマンドコード
	WORD	wData;
	WORD	wBuf;
	FFWERR	ferr;

	/* コマンドコードを送信 */
	wCmdCode = BFWCMD_TRC_SETDMY;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	/* リザーブ送信 */
	wData = 0x0000;
	eRet = PutData2(wData);
	if (!eRet) {
		return FFWERR_COM;
	}

	/* BTR2 1送信 */
	eRet = PutData4(0);
	if (!eRet) {
		return FFWERR_COM;
	}
	/* BTR2 2送信 */
	eRet = PutData4(0);
	if (!eRet) {
		return FFWERR_COM;
	}
	/* DTW1 1送信 */
	eRet = PutData4(0);
	if (!eRet) {
		return FFWERR_COM;
	}
	/* DTW2 1送信 */
	eRet = PutData4(DUMMY_DTW_ADDRESS);
	if (!eRet) {
		return FFWERR_COM;
	}


	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}
//RevNo100715-014 Append End


//=============================================================================
/**
 * トレース関連コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtRxData_Tra(void)
{
	return;
}
