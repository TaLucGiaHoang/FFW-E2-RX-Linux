///////////////////////////////////////////////////////////////////////////////
/**
 * @file doe2_closed.cpp
 * @brief 非公開コマンドの実装ファイル
 * @author RSD S.Ueda
 * @author Copyright (C) 2014 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/11
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/11 上田
	新規作成
*/
#include "doe2_closed.h"
#include "prote2_closed.h"
#include "prot_common.h"
#include "prot_sys.h"


///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * モニタプログラム(BFW COM)データの送信開始を通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPCOM_OPEN(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;

	// ワーニング対策
	dwTotalLength;

	ProtInit();

	// MONPCOMコマンドを指定して、MONP OPEN処理実行
	eBfwLoadCmd = BFWLOAD_CMD_MONPCOM;
	ferr = MonpOpen(eBfwLoadCmd);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * モニタプログラム(BFW COM)データの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x0000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPCOM_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	// MONP SEND処理実行(MONP, MONPFDT, MONPCOM, MONPALL共通)
	ferr = MonpSend(dweadrAddr, dwLength, pbyBfwBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * モニタプログラム(BFW COM)データの送信終了を通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPCOM_CLOSE(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;

	ProtInit();

	// MONPCOMコマンドを指定して、MONP CLOSE処理実行
	eBfwLoadCmd = BFWLOAD_CMD_MONPCOM;
	ferr = MonpClose(eBfwLoadCmd);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}


//=============================================================================
/**
 * エミュレータACT-LED制御
 * @param dwActLedControl ACT-LED点灯/消灯指定
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetACTLED(DWORD dwActLedControl)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_SetACTLED(dwActLedControl);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * ライセンス情報の設定
 * @param dwOffsetAddr ライセンス情報設定開始アドレス(オフセットアドレス)
 * @param dwLength ライセンス情報設定データのバイト数
 * @param pbyData ライセンス情報設定データの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetLID(DWORD dwOffsetAddr, DWORD dwLength, const BYTE* pbyData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_SetLID(dwOffsetAddr, dwLength, pbyData);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * ライセンス情報の参照
 * @param dwOffsetAddr ライセンス情報参照開始アドレス(オフセットアドレス)
 * @param dwLength ライセンス情報参照データのバイト数
 * @param pbyData ライセンス情報参照データの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetLID(DWORD dwOffsetAddr, DWORD dwLength, BYTE *const pbyData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_GetLID(dwOffsetAddr, dwLength, pbyData);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}


///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 非公開コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdE2Data_Closed(void)
{
	return;
}


