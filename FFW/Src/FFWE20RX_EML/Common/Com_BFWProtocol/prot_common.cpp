///////////////////////////////////////////////////////////////////////////////
/**
 * @file prot_common.cpp
 * @brief BFWコマンド プロトコル生成関数(共通部)
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

#include "prot_common.h"
#include "comctrl.h"
#include "prot_cpu.h"
#include "prot.h"
#include "ffw_sys.h"
#include "errchk.h"
#include "BFWE20ERR.h"


// ファイル内static変数の宣言

// static関数の宣言
static BOOL chkBatCmd(WORD wCmdCode);


//=============================================================================
/**
 * BFWコマンド送信処理の初期化
 *   連続コマンド送信開始指示フラグを初期化する。
 * @param なし
 * @retval なし
 */
//=============================================================================
void ProtInit(void)
{
// ProtInit()は、DO_XXXX()関数の先頭で発行される。
// FFW処理内で、連続コマンド送信中にDO_XXXX()を呼び出す場合があるため、
// 連続コマンド送信開始指示フラグを初期化しないよう変更。
// なお、連続コマンド送信開始指示フラグは、連続コマンド送信終了時と
// INITコマンド発行時に初期化されるため、問題ない。
// ProtInitの処理は、システムリセット検出、処理中断処理実装時に
// 再検討する。

//	ClrBatStartFlg();	// 連続コマンド送信開始指示フラグを初期化する。
}

//=============================================================================
/**
 * BFWコマンド送信処理の終了
 *   連続コマンド送信中の場合、BATENDを送信して連続コマンド処理を終了する。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ProtEnd(void)
{
	FFWERR	ferr;

	ferr = FFWERR_OK;

//	ClrBatStartFlg();	// 連続コマンド終了
	return	ferr;
}


//=============================================================================
/**
 * BFWコマンドコードの送信
 * @param wCmdCode コマンドコード
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ProtSendCmd(WORD wCmdCode)
{

	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}

	return FFWERR_OK;
}

//=============================================================================
/**
 * BFWコマンドのエラーコード受信
 * @param wCmdCode コマンドコード
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ProtGetStatus(WORD wCmdCode)
{
	WORD	wBuf;
	BFWERR	berr;
	FFWERR	ferr;

	if (GetData2(&wBuf) != TRUE) {	// コマンドコード受信
		return FFWERR_COM;
	}
	if (wBuf != wCmdCode) {
		return FFWERR_COMDATA_CMDCODE;	// コマンドコードが異常である
	}

	if (GetData2(&wBuf) != TRUE) {	// error code
		return FFWERR_COM;
	}
	berr = static_cast<BFWERR>(wBuf);


	if (berr == BFWERR_COMEXE_STOP) {	// コマンド処理が中断された場合、続けて処理中断コードを取得する。

		if (GetData2(&wBuf) != TRUE) {	// 処理中断コード受信
			return FFWERR_COM;
		}

		switch (wBuf) {
		case HALT_BFWHALT_NORMAL:	// BFWCmd_HALTにより処理を中断した(BFW,EFWはコマンド待機状態)
			ferr = FFWERR_TRGHALT_OK;
			break;

		case HALT_BFWHALT_EFWRUN:	// BFWCmd_HALTにより処理を中断した(EFWはコマンド処理継続状態)
			ferr = FFWERR_TRGHALT_NGEFWRUN;
			break;

		default:
			ferr = FFWERR_COMDATA_ERRCODE;	// エラーコードが異常である
			break;
		}

	} else {
		ferr = TransError(berr);
		// V.1.02 RevNo110613-001 Modify Start
		if (ferr != FFWERR_OK) {
			// ワーニング判定
			if(SetBFWWarning(ferr) == TRUE){
				// ワーニングの場合は正常終了で返る
				ferr = FFWERR_OK;
			} else {
				// エラーの場合
				ProtRcvHaltCode(&wBuf);	// 処理中断コードを受信する
			}
		}
		// V.1.02 RevNo110613-001 Modify End
	}

	return ferr;
}

//=============================================================================
/**
 * FFW起動後、最初のBFWコマンドのエラーコード受信
 * @param wCmdCode コマンドコード
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ProtGetStatus_1st(WORD wCmdCode)
{
	WORD	wBuf;
	BYTE	byBuf;
	BFWERR	berr;
	FFWERR	ferr;
	BOOL	bRcv1;
	BOOL	bRcv2;
	BYTE	byData1;
	BYTE	byData2;
	WORD	wData;


	wData = (WORD)((wCmdCode >> 8) & 0x00ff);
	byData1 = static_cast<BYTE>(wData);
	
	wData = (WORD)(wCmdCode & 0x00ff);
	byData2 = static_cast<BYTE>(wData);


	bRcv1 = FALSE;
	bRcv2 = FALSE;
	while (bRcv2 == FALSE) {
		if (GetData1(&byBuf) != TRUE) {
			return FFWERR_COM;
		}

		if (byBuf == byData1) {	// 受信データが1バイト目のデータ(スタートコード)の場合
			bRcv1 = TRUE;

		} else {	// 受信データが1バイト目のデータ(スタートコード)でない場合

			if (bRcv1 == TRUE) {	// 前回のデータが1バイト目のデータ(スタートコード)だった場合
				if (byBuf == byData2) {
					bRcv2 = TRUE;
				} else {
					bRcv1 = FALSE;
				}
			} else {
				bRcv1 = FALSE;
			}
		}
	}


	if (GetData2(&wBuf) != TRUE) {	// error code
		return FFWERR_COM;
	}
	berr = static_cast<BFWERR>(wBuf);


	if (berr == BFWERR_COMEXE_STOP) {	// コマンド処理が中断された場合、続けて処理中断コードを取得する。

		if (GetData2(&wBuf) != TRUE) {	// 処理中断コード受信
			return FFWERR_COM;
		}

		switch (wBuf) {
		case HALT_BFWHALT_NORMAL:	// BFWCmd_HALTにより処理を中断した(BFW,EFWはコマンド待機状態)
			ferr = FFWERR_TRGHALT_OK;
			break;

		case HALT_BFWHALT_EFWRUN:	// BFWCmd_HALTにより処理を中断した(EFWはコマンド処理継続状態)
			ferr = FFWERR_TRGHALT_NGEFWRUN;
			break;

		default:
			ferr = FFWERR_COMDATA_ERRCODE;	// エラーコードが異常である
			break;
		}

	} else {
		ferr = TransError(berr);
		if (ferr != FFWERR_OK) {
			ProtRcvHaltCode(&wBuf);	// 処理中断コードを受信する
		}
	}

	return ferr;
}

//=============================================================================
/**
 * BFWコマンドのエラーコード受信
 * @param wCmdCode コマンドコード
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ProtGetStatus_SetUSBT(WORD wCmdCode)
{
	WORD	wBuf;
	BFWERR	berr;
	FFWERR	ferr;

	// 送信時と受信時でエラーコードを区別するため、ここでBFWにデータを送信する。
	if (FlashProtDataForced() != TRUE) {
		return FFWERR_COM;
	}


	// 以降、受信処理で通信エラーが発生した場合、FFWERR_COM_USBT_CHANGEを返送する。

	if (GetData2(&wBuf) != TRUE) {	// コマンドコード受信
		return FFWERR_COM_USBT_CHANGE;
	}
	if (wBuf != wCmdCode) {
		return FFWERR_COM_USBT_CHANGE;	// コマンドコードが異常である
	}

	if (GetData2(&wBuf) != TRUE) {	// error code
		return FFWERR_COM_USBT_CHANGE;
	}
	berr = static_cast<BFWERR>(wBuf);


	if (berr == BFWERR_COMEXE_STOP) {	// コマンド処理が中断された場合、続けて処理中断コードを取得する。

		if (GetData2(&wBuf) != TRUE) {	// 処理中断コード受信
			return FFWERR_COM_USBT_CHANGE;
		}

		switch (wBuf) {
		case HALT_BFWHALT_NORMAL:	// BFWCmd_HALTにより処理を中断した(BFW,EFWはコマンド待機状態)
			ferr = FFWERR_TRGHALT_OK;
			break;

		case HALT_BFWHALT_EFWRUN:	// BFWCmd_HALTにより処理を中断した(EFWはコマンド処理継続状態)
			ferr = FFWERR_TRGHALT_NGEFWRUN;
			break;

		default:
			ferr = FFWERR_COM_USBT_CHANGE;	// エラーコードが異常である
			break;
		}

	} else {
		ferr = TransError(berr);
		if (ferr != FFWERR_OK) {
			ProtRcvHaltCode(&wBuf);	// 処理中断コードを受信する
		}
	}

	return ferr;
}

//=============================================================================
/**
 * 処理中断コード受信
 * @param pwBuf 
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ProtRcvHaltCode(WORD *const pwBuf)
{	
	WORD	wBuf;
	FFWERR	ferr;

	pwBuf;	//ワーニング対策

	if (GetData2(&wBuf) != TRUE) {		// 処理中断コード受信
		return FFWERR_COM;
	}

	switch (wBuf) {
	case HALT_NON:				// 処理中断なし
		ferr = FFWERR_OK;
		break;

	case HALT_BFWHALT_NORMAL:	// BFWCmd_HALTにより処理を中断した(BFW,EFWはコマンド待機状態)
		ferr = FFWERR_TRGHALT_OK;
		break;

	case HALT_BFWHALT_EFWRUN:	// BFWCmd_HALTにより処理を中断した(EFWはコマンド処理継続状態)
		ferr = FFWERR_TRGHALT_NGEFWRUN;
		break;

	case HALT_EFW_TIMEOUT:			// EFWのタイムアウトを検出し、処理を中断した
		//RevNo100715-028 Modify Line
		ferr = FFWERR_OFW_TIMEOUT;
		break;

	case HALT_BFW_TIMEOUT:			// BFWのタイムアウトを検出し、処理を中断した
		ferr = FFWERR_BFW_TIMEOUT;
		break;

	case BFWERR_BMCU_RESET:		// MCUがリセット状態のためコマンド処理を実行できない
		ferr = FFWERR_BMCU_RESET;
		break;

	case BFWERR_BTARGET_POWER:	// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない
		ferr = FFWERR_BTARGET_POWER;
		break;

	case BFWERR_BTARGET_NOCONNECT:	// ターゲットシステムと接続されていない
		ferr = FFWERR_BTARGET_NOCONNECT;
		break;

	default:
		ferr = TransError(wBuf);		// エラーコードを検索 dDMAリードはここでエラーを返す場合がある。
		// V.1.02 RevNo110613-001 Modify Start
		// DUMPはここでエラー、ワーニングを返す場合があるため、ここにエラー判定処理を入れる。
		if (ferr != FFWERR_OK) {
			if(SetBFWWarning(ferr) == TRUE){
				// ワーニングの場合は正常終了
				ferr = FFWERR_OK;
			}
		}
		// V.1.02 RevNo110613-001 Modify End
		break;
	}

	return ferr;
}

//=============================================================================
/**
 * BFWプロトコル生成部共通処理用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtData_Common(void)
{
	return;
}


///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
