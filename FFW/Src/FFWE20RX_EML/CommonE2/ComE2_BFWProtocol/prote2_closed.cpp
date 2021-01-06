///////////////////////////////////////////////////////////////////////////////
/**
 * @file prote2_closed.cpp
 * @brief BFWコマンド プロトコル生成関数(非公開関連)
 * @author RSD S.Ueda
 * @author Copyright (C) 2014 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/10 上田
	新規作成
*/
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"


//=============================================================================
/**
 * BFWCmd_SetActLedコマンドの発行
 * @param dwActLedControl ACT-LED点灯/消灯指定
 * @retval FFWERR_OK  正常終了
 * @retval FFWERR_COM 通信異常
 */
//=============================================================================
FFWERR PROT_SetACTLED(DWORD dwActLedControl)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	BYTE	byData;

	wCmdCode = BFWCMD_SET_ACTLED;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	byData = static_cast<BYTE>(dwActLedControl);	// ACT-LED ON/OFF指定
	if (PutData1(byData) != TRUE) {
		return FFWERR_COM;
	}
	if (PutData1(BFWDATA_RESERV) != TRUE) {	// リザーブデータ(1バイト)送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_SetLIDコマンドの発行
 * @param dwOffsetAddr ライセンス情報設定開始アドレス(オフセットアドレス)
 * @param dwLength ライセンス情報設定データのバイト数
 * @param pbyData ライセンス情報設定データの格納バッファアドレス
 * @retval FFWERR_OK  正常終了
 * @retval FFWERR_BFWFROM_ERASE BFW格納用 フラッシュROMのイレーズエラー
 * @retval FFWERR_BFWFROM_WRITE BFW格納用 フラッシュROMの書き込みエラー
 * @retval FFWERR_BFWFROM_VERIFY BFW格納用 フラッシュROMのベリファイエラー
 * @retval FFWERR_COM 通信異常
 */
//=============================================================================
FFWERR PROT_SetLID(DWORD dwOffsetAddr, DWORD dwLength, const BYTE* pbyData)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_SET_LID;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData4(dwOffsetAddr) != TRUE) {	// ライセンス情報設定開始アドレス(オフセットアドレス)送信
		return FFWERR_COM;
	}
	if (PutData4(dwLength) != TRUE) {	// ライセンス情報設定データのバイト数送信
		return FFWERR_COM;
	}
	if (PutDataN(dwLength, pbyData) != TRUE) {	// ライセンス情報設定データ送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_GetLIDコマンドの発行
 * @param dwOffsetAddr ライセンス情報参照開始アドレス(オフセットアドレス)
 * @param dwLength ライセンス情報参照データのバイト数
 * @param pbyData ライセンス情報参照データの格納バッファアドレス
 * @retval FFWERR_OK  正常終了
 * @retval FFWERR_LID_UNDEFINE ライセンスIDが定義されていない
 * @retval FFWERR_COM 通信異常
 */
//=============================================================================
FFWERR PROT_GetLID(DWORD dwOffsetAddr, DWORD dwLength, BYTE *const pbyData)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_GET_LID;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData4(dwOffsetAddr) != TRUE) {	// ライセンス情報設定開始アドレス(オフセットアドレス)送信
		return FFWERR_COM;
	}
	if (PutData4(dwLength) != TRUE) {	// ライセンス情報設定データのバイト数送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetDataN(dwLength, pbyData) != TRUE) {	// ライセンス情報データ受信
		return FFWERR_COM;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}


//=============================================================================
/**
 * 非公開コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtE2Data_Closed(void)
{

	return;
}
