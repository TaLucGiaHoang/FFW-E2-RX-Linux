///////////////////////////////////////////////////////////////////////////////
/**
 * @file do_sys.cpp
 * @brief システムコマンドの実装ファイル
 * @author RSD Y.Minami, H.Hashiguchi, H.Akashi, S.Ueda, PA M.Tsuji, PA K.Tsumu
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120910-002	2012/10/30 明石
　　MONPコマンド ダウンロードデータサイズの奇数バイト対応
・RevNo121017-003	2012/11/12 明石
    VS2008対応エラーコード　WARNING C4996対応変更
・RevRxNo121106-001	2012/11/12 明石
    ソース記述改善
・RevRxNo131101-003	2013/12/11 上田
	5V電源供給時の制御改善
・RevRxE2LNo141104-001 2015/01/21 上田
	E2 Lite対応
	E1/E20 レベル0 旧バージョン(MP版以外)用関数削除。
・RevRxE2No170201-001 2017/02/01 PA 辻、PA 紡車
	E2エミュレータ対応
・RevRxNo170515-001 2017/05/15 PA 紡車
　　E2 OB-mini対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
  */
#include <stdio.h>
#include <stdlib.h>

#include "do_sys.h"
#include "ffw_sys.h"
#include "ffw_closed.h"	// RevRxE2LNo141104-001 Append Line
#include "prot_sys.h"
#include "prot_common.h"
#include "prot_cpu.h"
#include "comctrl.h"
#include "emudef.h"	// RevRxE2LNo141104-001 Append Line

// ファイル内static変数の宣言
static DWORD	s_dwTotalLength;		// FFWCmd_xxxx_SENDで送信するデータの総バイト数 (MONP, DCNF, JTGPで使用)
static DWORD	s_dwSendLength;			// FFWCmd_xxxx_SENDで送信したデータの総バイト数 (MONP, DCNF, JTGPで使用)

static FFWE20_EINF_DATA	s_EinfData;		// 起動時に取得したEINF情報を保持
static BOOL s_bSetEinfData;				// FFW内部で保持するエミュレータ情報(s_EinfData)の設定状況	// RevRxE2LNo141104-001 Append Line


static BOOL		s_bFirstConnect;		// 初回起動時に接続した状態を保持

static BOOL		s_bUnSetTmout;			// タイムアウト値未設定フラグ(TRUE:タイムアウト値未設定、FALSE:タイムアウト値設定済み)	// RevRxE2LNo141104-001 Append Line

// RevRxE2LNo141104-001 Modify Start
static BOOL		s_bNewBfwDataBuff;		// BFWコード格納用バッファ確保済みフラグ(TRUE:確保済み、FALSE:未確保)
static BYTE*	s_pbyBfwDataBuff;		// BFWコード格納用バッファポインタ
// RevRxE2LNo141104-001 Modify End

// RevRxE2No171004-001 Append Line
static DWORD	s_dwTransSize;			// USB転送サイズを保持用

// static関数の宣言
static FFWERR setUsbTransferSize(DWORD dwTransSize);
static FFWERR checkBfwTmoutCmdEnable(BOOL* pbBfwCmdEnable);	// RevRxE2LNo141104-001 Append Line
static void SplitFpgaHdrData(const BYTE* pbyDcsfBuff, DWORD *posData, DWORD *dwDataCnt);
static void SplitFpgaHdrData_E2(const BYTE* pbyDcsfBuff, DWORD *posData, DWORD *dwDataCnt);

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * INIT処理
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_INIT(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = SetUsbTransferSize_INIT(INIT_USBTRANS_SIZE);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * USB転送サイズの設定
 * @param dwTransSize Communi.DLLのCOM_ReceiveBlockH関数でBFWに要求するデータバイト数
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetUSBTS(DWORD dwTransSize)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = SetUsbTransferSize_USBTS(dwTransSize);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * USB転送サイズの参照
 * @param dwTransSize Communi.DLLのCOM_ReceiveBlockH関数でBFWに要求するデータバイト数
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetUSBTS(DWORD* pdwTransSize)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_GetUSBT(pdwTransSize);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * FFW, BFWのタイムアウト時間の設定
 *   wFfwTimeOutをCommuni.DLLに、wBfwTimeOutをBFWに設定する。
 * @param wFfwTimeOut FFWがBFWからデータを受信する際の受信タイムアウト時間
 * @param wBfwTimeOut BFWがEFW実行要求を出してからEFW処理実行完了確認までのタイムアウト時間
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetTMOUT(WORD wFfwTimeOut, WORD wBfwTimeOut)
{
	FFWERR ferr;
	FFWERR ferrEnd;
	BOOL bBfwCmdEnable;	// RevRxE2LNo141104-001 Append Line

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	// BFW TMOUTコマンド発行可否判定
	ferr = checkBfwTmoutCmdEnable(&bBfwCmdEnable);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	if (bBfwCmdEnable) {	// BFW TMOUTコマンド発行可の場合
		ferr = SetCOMTimeOut(wFfwTimeOut);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		ferr = PROT_SetTMOUT(wBfwTimeOut);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	} else {
		s_bUnSetTmout = TRUE;
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}


//=============================================================================
/**
 * BFWのタイムアウト時間の参照
 * @param pwBfwTimeOut BFWがEFW実行要求を出してからEFW処理実行完了確認までのタイムアウト時間の格納アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetTMOUT(WORD* pwBfwTimeOut)
{
	FFWERR ferr;
	FFWERR ferrEnd;
	BOOL bBfwCmdEnable;	// RevRxE2LNo141104-001 Append Line

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	// BFW TMOUTコマンド発行可否判定
	ferr = checkBfwTmoutCmdEnable(&bBfwCmdEnable);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	if (bBfwCmdEnable) {	// BFW TMOUTコマンド発行可の場合
		ferr = PROT_GetTMOUT(pwBfwTimeOut);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * エミュレータ情報を取得する。(RX用)
 * @param pEinf エミュレータハードウェア情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetE20EINF(FFWE20_EINF_DATA* pEinf)
{
	FFWERR	ferr;
	FFWERR	ferr2;		// RevRxNo170515-001 Append Line
	FFWERR	ferrEnd;
	WORD	wData;
	BYTE	byData;

	ProtInit();

	// V.1.02 No.14,15 USB高速化対応 Append & Modify Start
	if (GetLevelEML()) {	// レベルEML動作中の場合
		// RevRxE2LNo141104-001 Modify Start
		// 起動時に参照したEINF情報からエミュレータ種別を判定
		if ((s_EinfData.wEmuStatus == EML_E2) || (s_EinfData.wEmuStatus == EML_E2LITE)) {
			ferr = PROT_GetE20EINF(pEinf);
		} else {
			ferr = PROT_GetE20EINF_EML(pEinf);
		}
		// RevRxE2LNo141104-001 Modify End

	} else {	// レベル0動作中の場合
		ferr = PROT_GetE20EINF(pEinf);
	}
	// V.1.02 No.14,15 USB高速化対応 Append & Modify End


	// USBバスパワー(A/D変換値→電圧値への変換)
	// RevRxE2LNo141104-001 Modify Line
	pEinf->flNowVbus = ConvVbusVal(pEinf->wVbusVal, pEinf->wEmuStatus);


	// ユーザI/F接続形態
	// RevRxE2LNo141104-001 Modify Start
	if (pEinf->wEmuStatus != EML_E20) {
		pEinf->eStatUIF = CONNECT_IF_RESERVE;		// RESERVED
	} else {	// E20の場合
		wData = pEinf->wTagetCable;
		wData = (WORD)(wData >> 6);					// 上位10ビットが有効のため、6ビット右シフト。

		if ( wData >= 0x03C1 && wData <= 0x046C) {
			pEinf->eStatUIF = CONNECT_IF_NON;			// ケーブル未接続(3.10V以上3.65V以下)
		}
		else if ( wData >= 0x0383 ) {
			pEinf->eStatUIF = CONNECT_IF_38PIN;			// 38pinケーブル接続(2.90V以上3.10V未満)
		}
		else if ( wData >= 0x0326 ) {
			pEinf->eStatUIF = CONNECT_IF_14PIN;			// 38pinケーブル＋38pin→14pin変換コネクタ接続(2.60V以上2.90V未満)
		}
		else if ( wData >= 0x02C9 ) {
			pEinf->eStatUIF = CONNECT_IF_RESERVE;		// RESERVED(2.30V以上2.60V未満)
		}
		else if ( wData >= 0x027F ) {
			pEinf->eStatUIF = CONNECT_IF_SELF;			// 38pinケーブル＋セルフチェックコネクタ接続(2.06V以上2.30V未満)
		}												// ループバック状態
		else if ( wData >= 0x024D ) {
			pEinf->eStatUIF = CONNECT_IF_LOOPBACK;		// 38pinケーブル＋38pin→14pin変換コネクタ＋14pin→38pin変換コネクタ＋セルフチェックコネクタ接続(1.90V以上2.06V未満)
		}												// ループバック状態
		else if ( wData >= 0x020F ) {
			pEinf->eStatUIF = CONNECT_IF_RESERVE;		// RESERVED(1.70V以上1.90V未満)
		}
		else if ( wData >= 0x01B2 ) {
			pEinf->eStatUIF = CONNECT_IF_RESERVE;		// RESERVED(1.40V以上1.70V未満)
		}
		else {
			pEinf->eStatUIF = CONNECT_IF_RESERVE;		// RESERVED(1.4V未満)
		}
	}
	// RevRxE2LNo141104-001 Modify End


	// ユーザシステム電圧(A/D変換値→電圧値への変換)
	// RevRxE2LNo141104-001 Modify Line
	pEinf->flNowUVCC = ConvUvccVal(pEinf->wUvccVal, pEinf->wEmuStatus);

	// ユーザシステム電源状態
	// RevRxE2No170201-001 Modify Start
	if (pEinf->wEmuStatus == EML_E2) {
		if (pEinf->flNowUVCC < UVCC_1_20V) {	// UVCCが1.20Vより低い場合
			//電源OFF
			pEinf->eStatUVCC = UVCC_OFF;
		} else {								// UVCCが1.20V以上の場合
			//電源ON
			pEinf->eStatUVCC = UVCC_ON;
		}
	} else {
		// RevRxE2LNo141104-001 Modify Start
		if (pEinf->flNowUVCC < UVCC_1_49V) {	// UVCCが1.49Vより低い場合
			//電源OFF
			pEinf->eStatUVCC = UVCC_OFF;
		} else {								// UVCCが1.49V以上の場合
			//電源ON
			pEinf->eStatUVCC = UVCC_ON;
		}
		// RevRxE2LNo141104-001 Modify End
	}
	// RevRxE2No170201-001 Modify End

	// 通信制御FPGAデバッグインタフェース種別
	byData = pEinf->byFpga1DbgIf;
	switch (byData) {
	case SCI_DEBUG_RESERVE:
		pEinf->eDebugIF = SCI_DEBUG_RESERVE;
		break;
	case SCI_DEBUG_RX_JTAG:
		pEinf->eDebugIF = SCI_DEBUG_RX_JTAG;
		break;
	case SCI_DEBUG_SCI:
		pEinf->eDebugIF = SCI_DEBUG_SCI;
		break;
	default:
		break;	// ここは通らない
	}

	// トレース対象MCU種別
	byData = pEinf->byFpga2DbgIf;
	switch (byData) {
	case TRACE_MCU_RESERVE:
		pEinf->eMCUType = TRACE_MCU_RESERVE;
		break;
	case TRACE_MCU_RX610:
		pEinf->eMCUType = TRACE_MCU_RX610;
		break;
	default:
		break;	// ここは通らない
	}

	// EINF情報をs_EinfDataに設定
	memcpy(&s_EinfData, pEinf, sizeof(FFWE20_EINF_DATA));
	s_bSetEinfData = TRUE;	// RevRxE2LNo141104-001 Append Line

	// エミュレータ固有情報設定
	// RevRxNo170515-001 Modify Line
	ferr2 = SetEmuDefData();	// RevRxE2LNo141104-001 Append Line

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		// RevRxNo170515-001 Modify Start
		if (ferr2 == FFWERR_OK) {
			ferr = ferrEnd;
		} else {
			ferr = ferr2;
		}
		// RevRxNo170515-001 Modify End
	}

	return ferr;
}


// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * USBバスパワー電圧のA/D変換値→電圧値への変換
 * @param wVbusVal USBバスパワー電圧A/D変換値
 * @param wEmuStatus エミュレータ種別
 * @retval USBバスパワー電圧値([V])
 */
//=============================================================================
float ConvVbusVal(WORD wVbusVal, WORD wEmuStatus)
{
	float	fVbus = 0;
	WORD	wData;
	float	fData;
	BYTE	byTmpH, byTmpL;
	char	buffer[256];
	int		ierr;

	// RevRxE2LNo141104-001 Modify Start
	switch (wEmuStatus) {
	case EML_E1:	// no break
	case EML_E20:	// no break
	case EML_EZCUBE:
		wData = wVbusVal;
		// RevRxNo121106-001	Modify Line
		wData = (WORD)(wData >> 6);		// 上位10ビットが有効のため、6ビット右シフト。(改善ソースではBYTEにキャストしていた。)
		fData = (float)wData;
		fData /= 1024;
		fData *= (float)3.3;
		fData = fData * 80 / 47;		// 基板上の抵抗分割によりA/D入力値は47/80されている。

		wData = (WORD)((int)fData);
		fData = (fData - wData) * 100;
		wData = (WORD)((wData << 8) + (BYTE)fData);

		byTmpH = (BYTE)((wData >> 8) & 0x00FF);					   
		byTmpL = (BYTE)(wData & 0x00FF);
		// RevNo121017-003	Modify Line
		ierr = sprintf_s(buffer, _countof(buffer), "%2d.%2d", byTmpH, byTmpL);

		fVbus = (float)atof(buffer);
		break;

	case EML_E2:		// RevRxE2No170201-001 Append Line
	case EML_E2LITE:
		fData = (float)wVbusVal;
		fData /= 4096;
		fData *= (float)3.3;
		fData = fData * 2;				// 基板上の抵抗分割によりA/D入力値は1/2されている。
		fVbus = fData;
		break;
	default:
		break;	// ここは通らない
	}
	// RevRxE2LNo141104-001 Modify End

	return fVbus;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * ユーザシステム電圧のA/D変換値→電圧値への変換
 * @param wUvccVal ユーザシステム電圧A/D変換値
 * @param wEmuStatus エミュレータ種別
 * @retval ユーザシステム電圧値([V])
 */
//=============================================================================
float ConvUvccVal(WORD wUvccVal, WORD wEmuStatus)
{
	float	fUvcc = 0;
	WORD	wData;
	float	fData;
	BYTE	byTmpH, byTmpL;
	char	buffer[256];
	int		ierr;

	// RevRxE2LNo141104-001 Modify Start
	switch (wEmuStatus) {
	case EML_E1:	// no break
	case EML_E20:	// no break
	case EML_EZCUBE:
		wData = wUvccVal;
		wData = (WORD)(wData >> 6);					// 上位10ビットが有効のため、6ビット右シフト。
		fData = (float)wData;
		fData /= 1024;
		fData *= (float)3.3;
		fData *= 2;

		wData = (WORD)((int)fData);
		fData = (fData - wData) * 100;
		wData = (WORD)((wData << 8) + (BYTE)fData);

		byTmpH = (BYTE)((wData >> 8) & 0x00FF);
		byTmpL = (BYTE)(wData & 0x00FF);
		// RevNo121017-003	Modify Line
		ierr = sprintf_s(buffer, _countof(buffer), "%2d.%2d", byTmpH, byTmpL);

		fUvcc = (float)atof(buffer);
		break;
	case EML_E2:		// RevRxE2No170201-001 Append Line
	case EML_E2LITE:
		fData = (float)wUvccVal;
		fData /= 4096;
		fData *= (float)3.3;
		fData *= 2;
		fUvcc = fData;
		break;
//	case EML_E2:	// E2未対応
//		break;
	default:
		break;	// ここは通らない
	}
	// RevRxE2LNo141104-001 Modify End

	return fUvcc;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * FFW内部で保持するエミュレータ情報を取得する。(RX用)
 * ※レベルEML動作中などEINF情報がFFW内部変数に格納されていることが明らかな場合
 *   のみ使用可能である。
 *   レベル0動作中やEINF情報がFFW内部変数に格納されていない可能性がある場合に
 *   エミュレータ種別を参照する場合は、BFWからEINF情報を取得すること。
 * @param pEinf エミュレータハードウェア情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR getEinfData(FFWE20_EINF_DATA* pEinf)
{
	memcpy(pEinf, &s_EinfData, sizeof(FFWE20_EINF_DATA));

	return FFWERR_OK;
}

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * FFW内部で保持するエミュレータ情報の設定状況を取得する。
 * @param pEinf エミュレータハードウェア情報を格納する構造体のアドレス
 * @retval TRUE s_EinfDataにエミュレータ情報設定済み
 * @retval FALSE s_EinfDataにエミュレータ情報未設定
 */
//=============================================================================
BOOL GetEinfDataSetFlg(void)
{
	return s_bSetEinfData;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * モニタプログラム(BFW EML)データの送信開始を通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONP_OPEN(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;

	// ワーニング対策
	dwTotalLength;

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	// MONPコマンドを指定して、MONP OPEN処理実行
	eBfwLoadCmd = BFWLOAD_CMD_MONP;
	ferr = MonpOpen(eBfwLoadCmd);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * モニタプログラム(BFW EML)データの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x0000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONP_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	// RevRxE2LNo141104-001 Modify Start
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
	// RevRxE2LNo141104-001 Modify End

	return ferr;
}

//=============================================================================
/**
 * モニタプログラム(BFW EML)データの送信終了を通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONP_CLOSE(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	// MONPコマンドを指定して、MONP CLOSE処理実行
	eBfwLoadCmd = BFWLOAD_CMD_MONP;
	ferr = MonpClose(eBfwLoadCmd);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}


//=============================================================================
/**
 * モニタプログラム(BFW FDT/PRG)データの送信開始を通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPFDT_OPEN(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;

	// ワーニング対策
	dwTotalLength;

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	// MONPFDT/MONPPRGコマンドを指定して、MONP OPEN処理実行
	eBfwLoadCmd = BFWLOAD_CMD_MONPFDT;
	ferr = MonpOpen(eBfwLoadCmd);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * モニタプログラム(BFW FDT/PRG)データの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x0000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPFDT_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	// MONP SEND処理実行(MONP, MONPFDT, MONPCOM, MONPALL共通)
	ferr = MonpSend(dweadrAddr, dwLength, pbyBfwBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * モニタプログラム(BFW FDT/PRG)データの送信終了を通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPFDT_CLOSE(void)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	// MONPFDTコマンドを指定して、MONP CLOSE処理実行
	eBfwLoadCmd = BFWLOAD_CMD_MONPFDT;
	ferr = MonpClose(eBfwLoadCmd);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

// 2008.11.13 INSERT_BEGIN_E20RX600(+14) {
//=============================================================================
/**
 * DCSFコンフィギュレーションデータを通知(E20用)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_E20DCNF_OPEN(DWORD dwTotalLength, DWORD dwType)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_DCNF_OPEN(dwType);

	s_dwTotalLength = dwTotalLength;
	s_dwSendLength = 0;

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
// 2008.11.13 INSERT_END_E20RX600 }
//=============================================================================
/**
* FPGA Configデータのヘッダ部を削除
* @param なし
* @retval FFWエラーコード
*/
//=============================================================================
static void SplitFpgaHdrData(const BYTE* pbyDcsfBuff, DWORD *posData, DWORD *dwDataCnt)
{
	DWORD   j;
	DWORD	dwCnt;
	char	byFpga1FileName[256];
	char	byFpga1FileType[256];
	char	byFpga1FileDate[256];
	char	byFpga1FileTime[256];
	DWORD	posStart;
	DWORD	dwCntU, dwCntD;
	DWORD	dwCntUU, dwCntDD;

	posStart = (long)pbyDcsfBuff;

	//ファイル名情報位置を検索
	while (*pbyDcsfBuff != ASCI_a/*'a'*/) {
		//	while(*pbyDcsfBuff != 0x61/*'a'*/) {
		pbyDcsfBuff++;
	}

	pbyDcsfBuff++;
	dwCntU = *pbyDcsfBuff;
	pbyDcsfBuff++;
	dwCntD = *pbyDcsfBuff;
	dwCnt = ((dwCntU) << 8) | (dwCntD);
	//ファイル名情報を取得
	for (j = 0; j<dwCnt; j++) {
		byFpga1FileName[j] = *pbyDcsfBuff;
		pbyDcsfBuff++;
	}

	//FPGA種類情報位置を検索
	while (*pbyDcsfBuff != ASCI_b/*'b'*/) {
		//	while(*pbyDcsfBuff != 0x62/*'b'*/) {
		pbyDcsfBuff++;
	}

	pbyDcsfBuff++;
	dwCntU = *pbyDcsfBuff;
	pbyDcsfBuff++;
	dwCntD = *pbyDcsfBuff;
	dwCnt = ((dwCntU) << 8) | (dwCntD);
	//FPGA種類情報を取得
	for (j = 0; j<dwCnt; j++) {
		byFpga1FileType[j] = *pbyDcsfBuff;
		pbyDcsfBuff++;
	}
	//ファイル生成日情報位置を検索
	while (*pbyDcsfBuff != ASCI_c/*'c'*/) {
		//	while(*pbyDcsfBuff != 0x63/*'c'*/) {
		pbyDcsfBuff++;
	}

	pbyDcsfBuff++;
	dwCntU = *pbyDcsfBuff;
	pbyDcsfBuff++;
	dwCntD = *pbyDcsfBuff;
	dwCnt = ((dwCntU) << 8) | (dwCntD);
	//ファイル生成日情報取得
	for (j = 0; j<dwCnt; j++) {
		byFpga1FileDate[j] = *pbyDcsfBuff;
		pbyDcsfBuff++;
	}
	//ファイル生成時間情報位置を検索
	while (*pbyDcsfBuff != ASCI_d/*'d'*/) {
		//	while(*pbyDcsfBuff != 0x64/*'d'*/) {
		pbyDcsfBuff++;
	}

	pbyDcsfBuff++;
	dwCntU = *pbyDcsfBuff;
	pbyDcsfBuff++;
	dwCntD = *pbyDcsfBuff;
	dwCnt = ((dwCntU) << 8) | (dwCntD);
	//ファイル生成時間情報取得
	for (j = 0; j<dwCnt; j++) {
		byFpga1FileTime[j] = *pbyDcsfBuff;
		pbyDcsfBuff++;
	}
	//コンフィグレーションデータ開始位置を検索
	while (*pbyDcsfBuff != ASCI_e/*'e'*/) {
		//	while(*pbyDcsfBuff != 0x65/*'e'*/) {
		pbyDcsfBuff++;
	}

	pbyDcsfBuff++;
	dwCntUU = *pbyDcsfBuff;
	pbyDcsfBuff++;
	dwCntU = *pbyDcsfBuff;
	pbyDcsfBuff++;
	dwCntD = *pbyDcsfBuff;
	pbyDcsfBuff++;
	dwCntDD = *pbyDcsfBuff;
	dwCnt = (dwCntUU << 24) | (dwCntU << 16) | (dwCntD << 8) | (dwCntDD);

	*dwDataCnt = dwCnt;
	*posData = (long)pbyDcsfBuff - posStart;

}

// RevRxE2No170201-001 Modify Start
//=============================================================================
/**
* FPGA Configデータのヘッダ部を削除
* @param pbyFpgaBuff FPGAデータのバッファ
* @param posData コンフィグレーションデータ開始位置
* @param dwDataCnt コンフィグレーションデータサイズ
* @retval RFWERR_OK  正常動作
* @retval RFWERR_ERR エラーあり
*/
//=============================================================================
static void SplitFpgaHdrData_E2(const BYTE* pbyFpgaBuff, DWORD *posData, DWORD *dwDataCnt)
{
	DWORD	i;
	DWORD   j;
	DWORD	dwCnt;
	char	byFpga1FileName[256];
	char	byFpga1FileVersion[8];
	char	byFpga1FileSize[4];
	DWORD	posStart;
	char	byLength[2];
	WORD	wLength;

	posStart = (long)pbyFpgaBuff;

	//Field3先頭位置(ファイル名と追加情報の先頭位置)を検索
	for (;;) {
		if (*pbyFpgaBuff == ASCI_a/*'a'*/) {
			pbyFpgaBuff++;
			break;
		}
		pbyFpgaBuff++;
	}

	//Field3のレングスを取得
	for (j = 0; j < 2; j++) {
		byLength[j] = *pbyFpgaBuff;
		pbyFpgaBuff++;
	}
	wLength = ((byLength[0] << 8) & 0xff00) |
		(byLength[1] & 0x00ff);

	//ファイル名情報を取得
	for (j = 0; j < wLength; j++) {
		byFpga1FileName[j] = *pbyFpgaBuff;
		pbyFpgaBuff++;
		if (*pbyFpgaBuff == ASCI_SEMICOLON/* ; */) {
			j++;
			break;
		}
	}
	//ファイルバージョン情報位置を検索
	for (;;) {
		pbyFpgaBuff++;
		j++;
		if (*pbyFpgaBuff == ASCI_I/*'I'*/) {
			pbyFpgaBuff++;
			j++;
			if (*pbyFpgaBuff == ASCI_D/*'D'*/) {
				pbyFpgaBuff++;
				j++;
				if (*pbyFpgaBuff == ASCI_EQUAL/*'='*/) {
					pbyFpgaBuff++;
					j++;
					break;
				}
			}
		}
	}
	//ファイルバージョン情報を取得
	for (i = 0; i < 8; i++) {
		byFpga1FileVersion[i] = *pbyFpgaBuff;
		pbyFpgaBuff++;
		j++;
	}
	//Field3通過
	for (; j < wLength; j++) {
		pbyFpgaBuff++;
	}

	//pbyFpgaBuffはField4のkey"b"の位置
	pbyFpgaBuff++;
	//Field4のレングスを取得
	for (j = 0; j < 2; j++) {
		byLength[j] = *pbyFpgaBuff;
		pbyFpgaBuff++;
	}
	wLength = ((byLength[0] << 8) & 0xff00) |
		(byLength[1] & 0x00ff);
	//Field4通過
	for (j = 0; j < wLength; j++) {
		pbyFpgaBuff++;
	}

	//pbyFpgaBuffはField5のkey"c"の位置
	pbyFpgaBuff++;
	//Field5のレングスを取得
	for (j = 0; j < 2; j++) {
		byLength[j] = *pbyFpgaBuff;
		pbyFpgaBuff++;
	}
	wLength = ((byLength[0] << 8) & 0xff00) |
		(byLength[1] & 0x00ff);
	//Field5通過
	for (j = 0; j < wLength; j++) {
		pbyFpgaBuff++;
	}

	//pbyFpgaBuffはField6のkey"d"の位置
	pbyFpgaBuff++;
	//Field6のレングスを取得
	for (j = 0; j < 2; j++) {
		byLength[j] = *pbyFpgaBuff;
		pbyFpgaBuff++;
	}
	wLength = ((byLength[0] << 8) & 0xff00) |
		(byLength[1] & 0x00ff);
	//Field6通過
	for (j = 0; j < wLength; j++) {
		pbyFpgaBuff++;
	}

	//Field7位置(書き込みデータサイズ情報位置)を検索
	//pbyFpgaBuffの現在の位置がField7のkey"e"の位置
	for (;;) {
		if (*pbyFpgaBuff == ASCI_e/*'e'*/) {
			break;
		}
		pbyFpgaBuff++;
	}
	//pbyFpgaBuffはField7のkey"e"の位置
	pbyFpgaBuff++;
	//書き込みデータサイズを取得
	for (j = 0; j < 4; j++) {
		byFpga1FileSize[j] = *pbyFpgaBuff;
		pbyFpgaBuff++;
	}
	dwCnt = ((byFpga1FileSize[0] << 24) & 0xff000000) |
		((byFpga1FileSize[1] << 16) & 0x00ff0000) |
		((byFpga1FileSize[2] << 8) & 0x0000ff00) |
		(byFpga1FileSize[3] & 0x000000ff);

	//コンフィグレーションデータ開始位置を検索
	*dwDataCnt = dwCnt;
	*posData = (long)pbyFpgaBuff - posStart;

}
// RevRxE2No170201-001 Modify End

//=============================================================================
/**
 * DCSFコンフィギュレーションデータの送信
 * @param dwLength DCSF論理データのバイト数
 * @param pbyDcsfBuff DCSF論理データの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_DCNF_SEND(DWORD dwLength, const BYTE* pbyDcsfBuff)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	DWORD	dwCount;
	DWORD	posData;
//	DWORD	dwDataCnt;
	FFWE20_EINF_DATA	einfData;	// RevRxE2No170201-001 Append Line

	ProtInit();

	getEinfData(&einfData);			// RevRxE2No170201-001 Append Line

	//FPGAデータはバイナリで提供される。
	//ヘッダ情報を保持し、実データのみをBFWに送信する。

	if ((einfData.wEmuStatus == EML_E2)) {		// E2の場合
		SplitFpgaHdrData_E2(pbyDcsfBuff, &posData, &dwLength);
	}
	else {		// E1/E20の場合
		SplitFpgaHdrData(pbyDcsfBuff, &posData, &dwLength);
	}
	pbyDcsfBuff = pbyDcsfBuff + posData;
	// RevRxE2No170201-001 Append Line
	if ((einfData.wEmuStatus == EML_E2)) {		// E2の場合
		dwLength = dwLength - posData; //実データ数のみ	(ヘッダとデータ数を減らす)
	}
	//ferr = FFWERR_OK;
	//return ferr;

	ferr = PROT_DCNF_SEND(dwLength, pbyDcsfBuff);

	// 進捗ゲージカウンタの更新
	s_dwSendLength += dwLength;
	dwCount = s_dwTotalLength / s_dwSendLength;
	dwCount = static_cast<DWORD>(GAGE_COUNT_MAX) / dwCount;
	SetGageCount(dwCount);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * DCSFコンフィギュレーションの終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_DCNF_CLOSE()
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = FFWERR_OK;

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
//=============================================================================
/**
 * ターゲットシステムへの電源供給(固定値指定)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_EXPWR(enum FFWENM_EXPWR_ENABLE eExPwrEnable, enum FFWENM_EXPWR_VAL eExPwrVal)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	// RevRxNo131101-003 Append Start
	FFWERR	ferr2;
	BOOL	bLevelEml;
	BYTE	byBfwTransMode;
	// RevRxNo131101-003 Append End
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	ProtInit();

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxNo131101-003 Modify Start
	// レベルEML動作状態取得
	bLevelEml = GetLevelEML();

	// RevRxE2LNo141104-001 Modify Start
	if ((einfData.wEmuStatus != EML_E1) && (einfData.wEmuStatus != EML_E20) && (einfData.wEmuStatus != EML_EZCUBE)) {
		// E1/E20/EZ-CUBE以外の場合
		// ターゲットシステムへの電源供給(BFW レベル0のEXTPWRコマンド実行)
		ferr = PROT_EXPWR(eExPwrEnable, eExPwrVal);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

	} else {	// E1/E20/EZ-CUBEの場合
		if (bLevelEml != TRUE) {
			// レベルEMLでない(レベル0)場合、レベルEMLへ遷移させる
			byBfwTransMode = TRANS_MODE_EML;
			ferr = PROT_E20TRANS(byBfwTransMode);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}

		// ターゲットシステムへの電源供給(BFW レベルEMLのEXTPWRコマンド実行)
		ferr = PROT_EXPWR_EML(eExPwrEnable, eExPwrVal);
		if (ferr != FFWERR_OK) {
			if (bLevelEml != TRUE) {
				byBfwTransMode = TRANS_MODE_LV0;
				ferr2 = PROT_E20TRANS_EML(byBfwTransMode);
			}
			ferrEnd = ProtEnd();
			return ferr;
		}

		// レベル0からレベルEMLへ遷移させた場合、レベル0に戻す
		if (bLevelEml != TRUE) {
			byBfwTransMode = TRANS_MODE_LV0;
			ferr = PROT_E20TRANS_EML(byBfwTransMode);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
		// RevRxNo131101-003 Modify End
	}
	// RevRxE2LNo141104-001 Modify End


	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

// 2009.6.22 INSERT_BEGIN_E20RX600(+NN) {

//=============================================================================
/**
 * BFW のLEVEL遷移
 * @param byBfwTransMode BFW動作モード(TRANS_MODE_LV0/TRANS_MODE_EML/TRANS_MODE_FDT)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_E20Trans(BYTE byBfwTransMode)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	WORD	wFfwTimeOut;	// RevRxE2LNo141104-001 Append Line
	WORD	wBfwTimeOut;	// RevRxE2LNo141104-001 Append Line

	ProtInit();

	// V.1.02 No.14,15 USB高速化対応 Append & Modify Start
	if (GetLevelEML()) {
		// RevRxE2LNo141104-001 Modify Start
		if ((s_EinfData.wEmuStatus == EML_E2) || (s_EinfData.wEmuStatus == EML_E2LITE)) {	// E2/E2 Liteの場合
			ferr = PROT_E20TRANS(byBfwTransMode);
		} else {	// E1/E20/EZ-CUBEの場合
			ferr = PROT_E20TRANS_EML(byBfwTransMode);
		}
		// RevRxE2LNo141104-001 Modify End
	} else {
		ferr = PROT_E20TRANS(byBfwTransMode);
	}
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// V.1.02 No.14,15 USB高速化対応 Append & Modify End

	// RevRxE2LNo141104-001 Append Start
	// E2/E2 LiteでEML動作モードへ遷移した場合、TMOUTコマンド処理を実行する。
	if (byBfwTransMode == TRANS_MODE_EML) {
		if ((s_EinfData.wEmuStatus == EML_E2) || (s_EinfData.wEmuStatus == EML_E2LITE)) {
			if (s_bUnSetTmout) {
				wFfwTimeOut = GetFfwTimeOut();
				ferr = SetCOMTimeOut(wFfwTimeOut);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				wBfwTimeOut = GetBfwTimeOut();
				ferr = PROT_SetTMOUT(wBfwTimeOut);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				s_bUnSetTmout = FALSE;
			}
		}
	}
	// RevRxE2LNo141104-001 Append End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
//=============================================================================
/**
 * BFW のLEVEL0への遷移　内部関数
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR E20TransLV0(void)
{
	FFWERR	ferr;
	BYTE byBfwTransMode;

	byBfwTransMode = TRANS_MODE_LV0;
	// V.1.02 RevNo110506-004 Modify Line
	ferr = DO_E20Trans(byBfwTransMode);
	SetLevelEML(FALSE);

	return ferr;
}

//=============================================================================
/**
 * Level0 + 1およびデータフラッシュ領域のサムチェックおよびLevel0領域のサム値を取得
 * @param pSum Level0のサム値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_E20SUMCHECK(FFWE20_SUM_DATA* pSum)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();
	// サム値取得コマンド送信
	ferr = PROT_E20SUMCHECK( pSum );

	ferrEnd = ProtEnd();
	if( ferrEnd != FFWERR_OK ){
		return( ferrEnd );
	}

	// 基本的にはPROT_E20SUMCHECK()で実行したステータス(サム値)を返す。
	return ferr;
}


// RevRxE2LNo141104-001 Modify Start
// FFWE20Cmd_SetSELID(), DO_E20SetSELID_OPEN(), DO_E20SetSELID_SEND(), DO_E20SetSELID_CLOSE()処理を
// DO_E20_SetSELID()で実施するよう変更。
//=============================================================================
/**
 * シリアル番号の設定(E1/E20用)
 * @param dwNum シリアル番号バイト数
 * @param cNumber[] シリアル番号格納配列
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_E20_SetSELID(DWORD dwNum, const char cNumber[])
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	FFW_EMUDEF_DATA* pEmuDef;
	BYTE	byData[SELID_NUM_MAX + 1];
	BYTE	bySumData[4];
	BYTE	bySumValue;
	DWORD	dwSetNum;
	DWORD	dwSetAddr;
	DWORD	dwCnt;

	ProtInit();

	pEmuDef = GetEmuDefData();

	// シリアル番号の先頭にレングスを設定する。
	dwSetNum = 0;
	byData[dwSetNum] = (BYTE)dwNum;
	dwSetNum++;
	bySumData[3] = (BYTE)dwNum;	// SUM値設定

	for (dwCnt = 0; dwCnt < dwNum; dwCnt++) {
		byData[dwSetNum] = cNumber[dwCnt];
		dwSetNum++;
		// SUM値算出
		bySumData[3] = (BYTE)(bySumData[3] + cNumber[dwCnt]);
	}

	// NULLターミネータを付加
	byData[dwSetNum] = 0x00;
	dwSetNum++;

	// 製品シリアル番号設定開始
	ferr = PROT_E20SetSELID_OPEN();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// 製品シリアル番号設定
	dwSetAddr = pEmuDef->dwSelIdStartAdr;
	ferr = PROT_E20SetSELID_SEND(dwSetAddr, dwSetNum, &byData[0]);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// 製品シリアル番号のチェックサム確認コード設定
	// SetSELIDのデータフラッシュ領域の2の補数値書込み処理
	// F00FFCh～F0FFEhのライトデータの初期化
	bySumData[0] = 0xff;
	bySumData[1] = 0xff;
	bySumData[2] = 0xff;

	// データフラッシュ領域の2の補数値の作成
	bySumValue = (BYTE)(((pEmuDef->dwSelIdSize - 1) - dwSetNum) * 0xFF);
	bySumValue = (BYTE)(bySumValue + bySumData[3]);
	bySumData[3] = (BYTE)((~bySumValue) + 1);

	// 2の補数値の書き込み
	dwSetAddr = pEmuDef->dwSelIdStartAdr + pEmuDef->dwSelIdSize - 4;
	ferr = PROT_E20SetSELID_SEND(dwSetAddr, 4, &bySumData[0]);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// 製品シリアル番号設定終了
	ferr = PROT_E20SetSELID_CLOSE();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferrEnd != FFWERR_OK) {
		return ferrEnd;
	}
	return	ferr;
}
// RevRxE2LNo141104-001 Modify End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * シリアル番号の設定(E2/E2 Lite用)
 * @param dwNum シリアル番号バイト数
 * @param cNumber[] シリアル番号格納配列
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_E2_SetSELID(DWORD dwNum, const char cNumber[])
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwSize;
	BYTE*	byData;
	BYTE*	byReadData;
	DWORD	dwReadAddr;
	DWORD	dwSetCnt;
	DWORD	dwCnt;
	enum FFWENM_EACCESS_SIZE eAccessSize;
	DWORD	dwAreaNum;


	ProtInit();

	pEmuDef = GetEmuDefData();

	// 製品シリアル番号設定データの生成
	dwSize = pEmuDef->dwSelIdSize;
	byData = new BYTE [dwSize];	// 製品シリアル番号設定領域サイズ分のバッファ確保

	dwSetCnt = 0;
	byData[0] = (BYTE)dwNum;	// byData[0]にシリアル番号のバイト数を格納
	dwSetCnt++;

	for (dwCnt = 0; dwCnt < dwNum; dwCnt++) {
		byData[dwSetCnt] = cNumber[dwCnt];	// シリアル番号を格納
		dwSetCnt++;
	}

	byData[dwSetCnt] = 0x00;	// NULLターミネータを格納
	dwSetCnt++;

	while (dwSetCnt < (dwSize - 4)) {
		byData[dwSetCnt] = SELID_AREA_NONDATA_E2;	// シリアル番号のない領域に0xFFを格納
		dwSetCnt++;
	}
	while (dwSetCnt < dwSize) {
		byData[dwSetCnt] = SELID_AREA_ENDCODE_E2;	// 最終4バイトに終了コードを格納
		dwSetCnt++;
	}


	// 製品シリアル番号設定
	ferr = PROT_E2_SetSELID(dwSize, &byData[0]);
	if (ferr != FFWERR_OK) {
		delete [] byData;
		ferrEnd = ProtEnd();
		return ferr;
	}

	// 書き換え領域のベリファイ
	byReadData = new BYTE [dwSize];	// 製品シリアル番号設定領域サイズ分のバッファ確保(ベリファイ用)
	eAccessSize = EBYTE_ACCESS;
	dwAreaNum = 1;
	dwReadAddr = pEmuDef->dwSelIdStartAdr;
	ferr = PROT_CPU_R(eAccessSize, dwAreaNum, &dwReadAddr, &dwSize, byReadData);
	if (ferr != FFWERR_OK) {
		delete [] byData;
		delete [] byReadData;
		ferrEnd = ProtEnd();
		return ferr;
	}
	for (dwSetCnt = 0; dwSetCnt < dwSize; dwSetCnt++) {
		if (byData[dwSetCnt] != byReadData[dwSetCnt]) {
			ferr = FFWERR_BFWFROM_VERIFY;
			break;
		}
	}

	delete [] byData;
	delete [] byReadData;

	ferrEnd = ProtEnd();
	if (ferrEnd != FFWERR_OK) {
		return ferrEnd;
	}
	return	ferr;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * シリアル番号を取得
 * @param byLen 取得するシリアル番号のレングス
 * @param pbyNumber シリアル番号
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_E20GetSELID(DWORD* pdwNum,char* pbyNumber)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();
	// シリアル番号取得コマンド送信
	ferr = PROT_E20GetSELID( pdwNum,pbyNumber );

	ferrEnd = ProtEnd();
	if( ferrEnd != FFWERR_OK ){
		return( ferrEnd );
	}

	return ferr;
}
// 2009.6.22 INSERT_END_E20RX600 }

///////////////////////////////////////////////////////////////////////////////
// FFW内部関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * FFWCmd_INIT時のUSB転送サイズ設定
 * @param dwTransSize USB転送サイズ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetUsbTransferSize_INIT(DWORD dwTransSize)
{
	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_USB_SPEED eUsbSpeedType;
	FFW_WINVER_DATA*	pWinVer;
	BOOL	bInit;

	// 現在のUSB転送種別(Full-Speed/Hi-Speed)を取得
	// 起動後最初のBFWコマンド通信ができるかどうかの確認のため、
	// Windows2000以外の場合もBFWCmd_GetUSBSを発行する。
	bInit = TRUE;	// FFWCmd_INITからの発行
	ferr = PROT_GetUSBS(&eUsbSpeedType, bInit);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	pWinVer = GetWinVersion();

	if (pWinVer->dwPlatformId == WIN_VER_PLATFORM_WIN32_WINDOWS) {
		//Win 9x系
		dwTransSize = WIN2KFULL_USBTRANS_SIZE;
	} else if ((pWinVer->dwPlatformId == WIN_VER_PLATFORM_WIN32_NT) && (pWinVer->dwMajorVersion <= 4)) {
		//Win NT 3.51, NT 4.0
		dwTransSize = WIN2KFULL_USBTRANS_SIZE;
	} else if ((pWinVer->dwPlatformId == WIN_VER_PLATFORM_WIN32_NT) && (pWinVer->dwMajorVersion == 5) && (pWinVer->dwMinorVersion == 0)){
		//Win 2000
		if (eUsbSpeedType == USB_SPEED_FULL) {
			// Full-Speed
			dwTransSize = WIN2KFULL_USBTRANS_SIZE;
		} else {	// Hi-Speed
			dwTransSize = WIN2KHI_USBTRANS_SIZE;
		}
	}

	// USB転送サイズを、Communi.dll,BFWに設定する
	ferr = setUsbTransferSize(dwTransSize);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}

//=============================================================================
/**
 * FFWCmd_SetUSBTS時のUSB転送サイズ設定
 *   Windows2000 Hi-speedの場合、指定サイズを設定する。
 *   Windows2000 Hi-speed以外の場合、何もせず正常終了する。
 * @param dwTransSize USB転送サイズ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetUsbTransferSize_USBTS(DWORD dwTransSize)
{
	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_USB_SPEED eUsbSpeedType;
	FFW_WINVER_DATA*	pWinVer;
	BOOL	bUsbtsSet;
	BOOL	bInit;

	pWinVer = GetWinVersion();

// OSに関係なく処理を行うように変更
#if 0
	bUsbtsSet = FALSE;

	// Windows2000の場合
	if ((pWinVer->dwPlatformId == WIN_VER_PLATFORM_WIN32_NT) && (pWinVer->dwMajorVersion == 5) && (pWinVer->dwMinorVersion == 0)){

		// 現在のUSB転送種別(Full-Speed/Hi-Speed)を取得
		bInit = FALSE;	// FFWCmd_INIT以外からの発行
		ferr = PROT_GetUSBS(&eUsbSpeedType, bInit);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (eUsbSpeedType == USB_SPEED_HIGH) {	// Hi-speedの場合
			bUsbtsSet = TRUE;
		}
	}

	if (bUsbtsSet == TRUE) {
		// USB転送サイズを、Communi.dll,BFWに設定する
		ferr = setUsbTransferSize(dwTransSize);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
#else
	// 現在のUSB転送種別(Full-Speed/Hi-Speed)を取得
	bInit = FALSE;	// FFWCmd_INIT以外からの発行
	ferr = PROT_GetUSBS(&eUsbSpeedType, bInit);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (eUsbSpeedType == USB_SPEED_HIGH) {	// Hi-speedの場合
		bUsbtsSet = TRUE;
	}

	// USB転送サイズを、Communi.dll,BFWに設定する
	ferr = setUsbTransferSize(dwTransSize);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

#endif
	return ferr;
}

//=============================================================================
/**
 * 初回起動時に接続した状態を取得
 * @param  なし
 * @retval 現在の接続状態(TRUE:接続、FALSE：未接続)
 */
//=============================================================================
BOOL getFirstConnectInfo(void)
{
	return s_bFirstConnect;
}

//=============================================================================
/**
 * 初回起動時に接続した状態を設定
 * @param  接続状態(TRUE:接続、FALSE：未接続)
 * @retval なし
 */
//=============================================================================
void setFirstConnectInfo(BOOL bSet)
{
	s_bFirstConnect = bSet;

	return;
}

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * MONPxxx OPEN処理実行(MONP, MONPFDT, MONPCOM, MONPALL共通)
 * @param eBfwLoadCmd BFWコマンド種別
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR MonpOpen(enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd)
{
	FFWERR	ferr = FFWERR_OK;

	ferr = PROT_MONP_OPEN(eBfwLoadCmd);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// BFWコード格納用バッファの確保と初期化
	NewBfwDataBuff();

	return ferr;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * MONPxxx SEND処理実行(MONP, MONPFDT, MONPCOM, MONPALL共通)
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x0000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR MonpSend(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE*	pbyBfwCodeBuf;
	DWORD	dwCount;

	if (s_bNewBfwDataBuff) {	// BFWコード格納用バッファを確保済みの場合のみ実施
		// BFWコード格納用バッファにBFWコードデータを格納
		pbyBfwCodeBuf = GetBfwCodeBuffPtr(dweadrAddr);
		for (dwCount = 0 ; dwCount < dwLength; dwCount++) {
			*pbyBfwCodeBuf++ = *pbyBfwBuff++;
		}
	}

	return ferr;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * MONPxxx CLOSE処理実行(MONP, MONPFDT, MONPCOM, E1/E20 MONPALL用)
 * @param eBfwLoadCmd BFWコマンド種別
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR MonpClose(enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwSendCount;		// 送信済みデータ数(進捗ゲージカウンタ用)
	DWORD	dwTotalSendCount;	// 総送信データ数
	DWORD	dwSendStart;		// 書き換え領域開始アドレス
	DWORD	dwCpuAddr;			// 制御CPUアドレス
	DWORD	dwCount;			// 進捗ゲージ用カウンタ
	BYTE*	pbyBfwCodeBuf;		// BFWコード格納用バッファポインタ
	DWORD	dwLength;
	DWORD	dwTmpLength;
	BYTE*	pbyReadData;
	BYTE*	pbyData;
	enum FFWENM_EACCESS_SIZE eAccessSize;
	DWORD	dwAreaNum;
	DWORD	dwAccCount;

	pEmuDef = GetEmuDefData();

	// 書き換え開始アドレス、総データ送信カウント数設定
	switch (eBfwLoadCmd) {
	case BFWLOAD_CMD_MONP:
		dwSendStart = pEmuDef->dwBfwEmlStartAdr;	// EML領域開始アドレス
		dwTotalSendCount = pEmuDef->dwBfwEmlSize;	// EML領域サイズ
		break;
	case BFWLOAD_CMD_MONPFDT:
		dwSendStart = pEmuDef->dwBfwPrgStartAdr;	// FDT/PRG領域開始アドレス
		dwTotalSendCount = pEmuDef->dwBfwPrgSize;	// FDT/PRG領域サイズ
		break;
	case BFWLOAD_CMD_MONPCOM:
		dwSendStart = pEmuDef->dwBfwComStartAdr;	// COM領域開始アドレス
		dwTotalSendCount = pEmuDef->dwBfwComSize;	// COM領域サイズ
		break;
	case BFWLOAD_CMD_MONPALL:
		dwSendStart = pEmuDef->dwBfwAreaStartAdr;	// BFW領域開始アドレス
		dwTotalSendCount = pEmuDef->dwBfwAreaSize;	// BFW領域サイズ
		break;
	default:	// ここは通らない
		dwSendStart = pEmuDef->dwBfwEmlStartAdr;
		dwTotalSendCount = pEmuDef->dwBfwEmlSize;
		break;
	}


	dwCpuAddr = dwSendStart;
	dwLength = dwTotalSendCount;
	pbyBfwCodeBuf = GetBfwCodeBuffPtr(dwCpuAddr);	// BFWコード格納バッファポインタ取得

	dwSendCount = 0;	// 送信済みデータ数(進捗ゲージカウンタ用)初期化

	// BFWコード送信処理
	while (dwLength > 0) {
		// 最大FFW_SEND_UNITCNT(0x2000)単位でBFWコマンドを発行する
		if (dwLength > FFW_SEND_UNITCNT) {
			dwTmpLength = FFW_SEND_UNITCNT;
		} else {
			dwTmpLength = dwLength;
		}
		ferr = PROT_MONP_SEND(dwCpuAddr, dwTmpLength, pbyBfwCodeBuf, eBfwLoadCmd);
		if (ferr != FFWERR_OK) {
			break;	// エラー発生時もPROT_MONP_CLOSE()を実行する
		}
		dwCpuAddr += dwTmpLength;
		dwLength -= dwTmpLength;
		pbyBfwCodeBuf += dwTmpLength;
		dwSendCount += dwTmpLength;

		// 進捗ゲージカウンタ更新処理
		if (dwSendCount == 0) {
			dwCount = 0;
		} else {
			dwCount = dwTotalSendCount / dwSendCount;
			if (0 < dwCount) {
				dwCount = static_cast<DWORD>(GAGE_COUNT_MAX) / dwCount;
			}
		}
		SetGageCount(dwCount);
	}


	// BFW書き換え終了通知(エラー発生時も必ず実行する)
	ferr = PROT_MONP_CLOSE(eBfwLoadCmd);


	// E2/E2 Liteの場合、書き換え領域のベリファイ
	if ((ferr == FFWERR_OK) && ((s_EinfData.wEmuStatus == EML_E2) || (s_EinfData.wEmuStatus == EML_E2LITE))) {
		pbyReadData = new BYTE [PROT_CPU_R_LENGTH_MAX];	// リードデータ格納バッファ確保
		eAccessSize = ELWORD_ACCESS;
		dwAreaNum = 1;

		dwCpuAddr = dwSendStart;
		dwLength = dwTotalSendCount;
		while (dwLength > 0) {
			// 最大PROT_CPU_R_LENGTH_MAX単位でBFWコマンドを発行する
			if (dwLength > PROT_CPU_R_LENGTH_MAX) {
				dwTmpLength = PROT_CPU_R_LENGTH_MAX;
			} else {
				dwTmpLength = dwLength;
			}

			dwAccCount = dwTmpLength / 4;
			ferr = PROT_CPU_R(eAccessSize, dwAreaNum, &dwCpuAddr, &dwAccCount, pbyReadData);
			if (ferr != FFWERR_OK) {
				delete [] pbyReadData;
				DeleteBfwDataBuff();
				return ferr;
			}

			pbyBfwCodeBuf = GetBfwCodeBuffPtr(dwCpuAddr);	// BFWコード格納バッファポインタ取得
			pbyData = pbyReadData;

			for (dwCount = 0; dwCount < dwTmpLength; dwCount++) {
				if (*pbyBfwCodeBuf != *pbyData) {
					ferr = FFWERR_BFWFROM_VERIFY;
					delete [] pbyReadData;
					DeleteBfwDataBuff();
					return ferr;
				}
				pbyBfwCodeBuf++;
				pbyData++;
			}

			dwCpuAddr += dwTmpLength;
			dwLength -= dwTmpLength;
			pbyBfwCodeBuf += dwTmpLength;
		}

		delete [] pbyReadData;	// リードデータ格納バッファ解放
	}

	// BFWコード格納用バッファの解放
	DeleteBfwDataBuff();

	return ferr;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWコード格納用バッファの確保と初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void NewBfwDataBuff(void)
{
	FFW_EMUDEF_DATA* pEmuDef;

	pEmuDef = GetEmuDefData();

	if (s_bNewBfwDataBuff == FALSE) {
		s_pbyBfwDataBuff = new BYTE [pEmuDef->dwBfwAreaSize];
		memset(s_pbyBfwDataBuff, FFW_MONP_BUFDEFAULT, pEmuDef->dwBfwAreaSize);
		s_bNewBfwDataBuff = TRUE;	// BFWコード格納用バッファ確保済み
	}

	return;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWコード格納用バッファの解放
 * @param なし
 * @retval なし
 */
//=============================================================================
void DeleteBfwDataBuff(void)
{
	if (s_bNewBfwDataBuff) {
		delete [] s_pbyBfwDataBuff;
		s_bNewBfwDataBuff = FALSE;
	}

	return;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWコード格納用バッファポインタの取得
 * @param dwAddr BFWコード格納アドレス(制御CPU空間アドレス)
 * @retval 指定アドレスに対応するBFWコード格納バッファポインタ
 */
//=============================================================================
BYTE* GetBfwCodeBuffPtr(DWORD dwAddr)
{
	BYTE*	pbyBuf;
	FFW_EMUDEF_DATA* pEmuDef;

	pEmuDef = GetEmuDefData();

	pbyBuf = s_pbyBfwDataBuff + (dwAddr -  pEmuDef->dwBfwAreaStartAdr);

	return pbyBuf;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * システムコマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdData_Sys(void)
{
	int	i;

	s_dwTotalLength = 0;				// FFWCmd_xxxx_SENDで送信するデータの総バイト数 (MONP, DCNF, JTGPで使用)
	s_dwSendLength = 0;					// FFWCmd_xxxx_SENDで送信したデータの総バイト数 (MONP, DCNF, JTGPで使用)

	s_bSetEinfData = FALSE;	// RevRxE2LNo141104-001 Append Line
	s_EinfData.wEmuStatus = 0;
	s_EinfData.wBoardRev = 0;
	s_EinfData.eEmlPwrSupply = EML_PWR_OFF;
	s_EinfData.wVbusVal = 0;
	s_EinfData.flNowVbus = 0.0;

	s_EinfData.wTagetCable = CONNECT_IF_RESERVE;
	s_EinfData.eStatUIF = CONNECT_IF_RESERVE;
	s_EinfData.eStatUCON = TRGT_EML_NCON;
	s_EinfData.wUvccVal = 0;
	s_EinfData.flNowUVCC = 0.0;
	s_EinfData.eStatUVCC = UVCC_OFF;

	s_EinfData.dwBfwLV0Ver = 0;
	s_EinfData.dwBfwEMLVer = 0;
	s_EinfData.wEmlTargetCpu = 0;
	s_EinfData.wEmlTargetCpuSub = 0;
	for (i = 0; i < 16; i++) {
		s_EinfData.cEmlTgtMucId[i] = 0;
	}
	s_EinfData.dwEmlTargetLv0Ver = 0;
	s_EinfData.dwBfwFdtVer = 0;
	for (i = 0; i < 16; i++) {
		s_EinfData.cFdtTgtMucId[i] = 0;
	}
	s_EinfData.dwFdtTargetLv0Ver = 0;

	s_EinfData.byFpga1Status = 0;
	s_EinfData.byFpga1DbgIf = 0;
	s_EinfData.eDebugIF = SCI_DEBUG_RESERVE;
	s_EinfData.byFpga1Rev = 0;
	s_EinfData.byFpga2DbgIf = 0;
	s_EinfData.eMCUType = TRACE_MCU_RESERVE;
	s_EinfData.byFpga2Rev = 0;
	s_EinfData.byFpgaVer = 0;
	s_EinfData.byTraceFlg = 0;

	s_bFirstConnect = FALSE;

	s_bUnSetTmout = FALSE;	// RevRxE2LNo141104-001 Append Line

	s_bNewBfwDataBuff = FALSE;	// RevRxE2LNo141104-001 Append Line

	s_dwTransSize = INIT_USBTRANS_SIZE;	// RevRxE2No171004-001 Append Line
}

// RevRxE2No171004-001 Append Start
//=============================================================================
/**
* USB転送サイズを取得
* @param なし
* @retval USB転送サイズ
*/
//=============================================================================
DWORD GetUsbTransferSize(void)
{
	return s_dwTransSize;
}
// RevRxE2No171004-001 Append End

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * USB転送サイズを、Communi.dll、BFWに設定する
 * @param dwTransSize USB転送サイズ
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setUsbTransferSize(DWORD dwTransSize)
{
	FFWERR	ferr = FFWERR_OK;

	// Communi.dllに転送サイズを設定
	SetCOMTransferSize(dwTransSize);

	// BFWに転送サイズを通知
	ferr = PROT_SetUSBT(dwTransSize);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// USB転送サイズを保持
	s_dwTransSize = dwTransSize;	// RevRxE2No171004-001 Append Line

	return ferr;
}

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFW TMOUTコマンド発行可否状態の確認
 * @param pbSetEnable BFWタイムアウト時間の設定/参照可否状態格納変数のポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR checkBfwTmoutCmdEnable(BOOL* pbBfwCmdEnable)
{
	FFWERR	ferr = FFWERR_OK;
	BOOL	bBfwCmdEnable;
	FFWE20_EINF_DATA	einfData;
	BOOL	bLevelEml;
	WORD	wEmu;


	bLevelEml = GetLevelEML();

	// 現在接続されているエミュレータ種別を取得する
	if (bLevelEml == FALSE) {
		// レベルEMLでない(レベル0)場合、BFWからエミュレータ種別取得
		ferr = PROT_GetE20EINF(&einfData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		wEmu = einfData.wEmuStatus;

	} else {
		// レベルEML場合、FFW内部変数からエミュレータ種別取得
		wEmu = s_EinfData.wEmuStatus;
	}

	// BFW TMOUTコマンド発行可否判定
	bBfwCmdEnable = TRUE;
	if ((wEmu == EML_E2) || (wEmu == EML_E2LITE)) {
		if (bLevelEml == FALSE) {
			bBfwCmdEnable = FALSE;	// E2/E2 Liteでレベル0動作中の場合、TMOUTコマンド発行不可
		}
	}
	*pbBfwCmdEnable = bBfwCmdEnable;

	return ferr;
}
// RevRxE2LNo141104-001 Append End

