///////////////////////////////////////////////////////////////////////////////
/**
 * @file prot_cpu.cpp
 * @brief BFWコマンド プロトコル生成関数(モニタCPU空間アクセス関連)
 * @author RSD Y.Minami, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/12/22
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/12/22 上田
	E2 Lite対応
*/
#include "prot_cpu.h"
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "ffw_sys.h"	// RevRxE2LNo141104-001 Append Line
#include "do_sys.h"	// RevRxE2LNo141104-001 Append Line


// ファイル内static変数の宣言

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWCmd_CPU_R/BFWCmd_CPU_R_EMLコマンドの発行
 *   pbyReadDataには、eadrReadAddr[], dwLength[]の配列要素番号0の領域に対応する
 *   リードデータから順に格納する。
 * @param eAccessSize アクセスサイズ(EBYTE_ACCESS/EWORD_ACCESS/ELWORD_ACCESS)
 * @param dwAreaNum リード領域数(1～0x4000)
 * @param eadrReadAddr[] リード開始アドレス
 * @param dwLength[] 1領域のリードアクセス回数(1～0x10000)
 * @param pbyReadData リードデータ格納バッファへのポインタ
 * @retval FFWERR_OK          正常終了
 * @retval FFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 */
//=============================================================================
FFWERR ProtCpuRead(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrReadAddr[],
				   const DWORD dwLength[], BYTE *const pbyReadData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;

	// エミュレータ種別、BFW動作モードに対応したCPU_Rコマンドを発行する。
	// エミュレータ種別はFFW内部変数を参照する。

	getEinfData(&einfData);

	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		ferr = PROT_CPU_R(eAccessSize, dwAreaNum, eadrReadAddr, dwLength, pbyReadData);

	} else {
		if (GetLevelEML()) {
			ferr = PROT_CPU_R_EML(eAccessSize, dwAreaNum, eadrReadAddr, dwLength, pbyReadData);
		} else {
			ferr = PROT_CPU_R(eAccessSize, dwAreaNum, eadrReadAddr, dwLength, pbyReadData);
		}
	}
	return ferr;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWCmd_CPU_W/BFWCmd_CPU_W_EMLコマンドの発行
 *   pbyWriteDataには、eadrWriteAddr[], dwLength[]の配列要素番号0の領域に対応する
 *   ライトデータから順に格納する。
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param dwAreaNum ライト領域数(1～0x4000)
 * @param eadrWriteAddr[] ライト開始アドレス
 * @param dwLength[] 1領域のライトアクセス回数(1～0x10000)
 * @param pbyWriteData ライトデータ格納バッファへのポインタ
 * @retval FFWERR_OK          正常終了
 * @retval FFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 */
//=============================================================================
FFWERR ProtCpuWrite(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrWriteAddr[],
					const DWORD dwLength[], const BYTE* pbyWriteData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;

	// エミュレータ種別、BFW動作モードに対応したCPU_Wコマンドを発行する。
	// エミュレータ種別はFFW内部変数を参照する。

	getEinfData(&einfData);

	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		ferr = PROT_CPU_W(eAccessSize, dwAreaNum, eadrWriteAddr, dwLength, pbyWriteData);

	} else {
		if (GetLevelEML()) {
			ferr = PROT_CPU_W_EML(eAccessSize, dwAreaNum, eadrWriteAddr, dwLength, pbyWriteData);
		} else {
			ferr = PROT_CPU_W(eAccessSize, dwAreaNum, eadrWriteAddr, dwLength, pbyWriteData);
		}
	}
	return ferr;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWCmd_CPU_FILL/BFWCmd_CPU_FILL_EMLコマンドの発行
 *   pbyWriteBuffには、eAccessSizeで指定したバイト数分のデータを格納する。
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param eadrWriteAddr FILLアドレス
 * @param dwLength アクセス回数(1～0x10000)
 * @param pbyWriteBuff FILLデータ格納領域へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ProtCpuFill(enum FFWENM_EACCESS_SIZE eAccessSize, EADDR eadrWriteAddr, DWORD dwLength, const BYTE* pbyWriteBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;

	// エミュレータ種別、BFW動作モードに対応したCPU_FILLコマンドを発行する。
	// エミュレータ種別はFFW内部変数を参照する。

	getEinfData(&einfData);

	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		ferr = PROT_CPU_FILL(eAccessSize, eadrWriteAddr, dwLength, pbyWriteBuff);

	} else {
		if (GetLevelEML()) {
			ferr = PROT_CPU_FILL_EML(eAccessSize, eadrWriteAddr, dwLength, pbyWriteBuff);
		} else {
			ferr = PROT_CPU_FILL(eAccessSize, eadrWriteAddr, dwLength, pbyWriteBuff);
		}
	}
	return ferr;
}
// RevRxE2LNo141104-001 Append End


//=============================================================================
/**
 * BFWCmd_CPU_Rコマンドの発行
 *   pbyReadDataには、eadrReadAddr[], dwLength[]の配列要素番号0の領域に対応する
 *   リードデータから順に格納する。
 * @param eAccessSize アクセスサイズ(EBYTE_ACCESS/EWORD_ACCESS/ELWORD_ACCESS)
 * @param dwAreaNum リード領域数(1～0x4000)
 * @param eadrReadAddr[] リード開始アドレス
 * @param dwLength[] 1領域のリードアクセス回数(1～0x10000)
 * @param pbyReadData リードデータ格納バッファへのポインタ
 * @retval FFWERR_OK          正常終了
 * @retval FFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 */
//=============================================================================
FFWERR PROT_CPU_R(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrReadAddr[],
		const DWORD dwLength[], BYTE *const pbyReadData)
{
	FFWERR	ferr;
	DWORD	dwCnt;
	DWORD	dwBlockLength;
	BYTE*	pbyBuff;
	WORD	wBuf;
	WORD	wCmdCode;

	if (dwAreaNum == 0) {
		return FFWERR_OK;
	}

	wCmdCode = BFWCMD_CPU_R;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData1(BFWDATA_RESERV) != TRUE) {	// リザーブデータ(1バイト)送信
		return FFWERR_COM;
	}
	if (PutData1(static_cast<BYTE>(eAccessSize)) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData4(dwAreaNum) != TRUE) {	// ブロック数送信
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (PutData4(dwLength[dwCnt]) != TRUE) {	// アクセス回数送信
			return FFWERR_COM;
		}
		if (PutData4(eadrReadAddr[dwCnt]) != TRUE) {	// リード開始アドレス送信
			return FFWERR_COM;
		}
	}

	ferr = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	pbyBuff = pbyReadData;

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		switch (eAccessSize) {
		case EBYTE_ACCESS:
			dwBlockLength = dwLength[dwCnt];
			break;
		case EWORD_ACCESS:
			dwBlockLength = dwLength[dwCnt] * 2;
			break;
		case ELWORD_ACCESS:
			dwBlockLength = dwLength[dwCnt] * 4;
			break;
		default:
			dwBlockLength = dwLength[dwCnt] * 4;
			break;
		}

		if (GetDataN(dwBlockLength, pbyBuff) != TRUE) {	// リードデータ受信
			return FFWERR_COM;
		}
		pbyBuff += dwBlockLength;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

// V.1.02 No.14,15 USB高速化対応 Append Start
//=============================================================================
/**
 * BFWCmd_CPU_R_EMLコマンドの発行
 *   pbyReadDataには、eadrReadAddr[], dwLength[]の配列要素番号0の領域に対応する
 *   リードデータから順に格納する。
 * @param eAccessSize アクセスサイズ(EBYTE_ACCESS/EWORD_ACCESS/ELWORD_ACCESS)
 * @param dwAreaNum リード領域数(1～0x4000)
 * @param eadrReadAddr[] リード開始アドレス
 * @param dwLength[] 1領域のリードアクセス回数(1～0x10000)
 * @param pbyReadData リードデータ格納バッファへのポインタ
 * @retval FFWERR_OK          正常終了
 * @retval FFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 */
//=============================================================================
FFWERR PROT_CPU_R_EML(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrReadAddr[],
		const DWORD dwLength[], BYTE *const pbyReadData)
{
	FFWERR	ferr;
	DWORD	dwCnt;
	DWORD	dwBlockLength;
	BYTE*	pbyBuff;
	WORD	wBuf;
	WORD	wCmdCode;

	if (dwAreaNum == 0) {
		return FFWERR_OK;
	}

	wCmdCode = BFWCMD_CPU_R_EML;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData1(BFWDATA_RESERV) != TRUE) {	// リザーブデータ(1バイト)送信
		return FFWERR_COM;
	}
	if (PutData1(static_cast<BYTE>(eAccessSize)) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData4(dwAreaNum) != TRUE) {	// ブロック数送信
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (PutData4(dwLength[dwCnt]) != TRUE) {	// アクセス回数送信
			return FFWERR_COM;
		}
		if (PutData4(eadrReadAddr[dwCnt]) != TRUE) {	// リード開始アドレス送信
			return FFWERR_COM;
		}
	}

	ferr = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	pbyBuff = pbyReadData;

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		switch (eAccessSize) {
		case EBYTE_ACCESS:
			dwBlockLength = dwLength[dwCnt];
			break;
		case EWORD_ACCESS:
			dwBlockLength = dwLength[dwCnt] * 2;
			break;
		case ELWORD_ACCESS:
			dwBlockLength = dwLength[dwCnt] * 4;
			break;
		default:
			dwBlockLength = dwLength[dwCnt] * 4;
			break;
		}

		if (GetDataN(dwBlockLength, pbyBuff) != TRUE) {	// リードデータ受信
			return FFWERR_COM;
		}
		pbyBuff += dwBlockLength;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}
// V.1.02 No.14,15 USB高速化対応 Append End

//=============================================================================
/**
 * BFWCmd_CPU_Wコマンドの発行
 *   pbyWriteDataには、eadrWriteAddr[], dwLength[]の配列要素番号0の領域に対応する
 *   ライトデータから順に格納する。
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param dwAreaNum ライト領域数(1～0x4000)
 * @param eadrWriteAddr[] ライト開始アドレス
 * @param dwLength[] 1領域のライトアクセス回数(1～0x10000)
 * @param pbyWriteData ライトデータ格納バッファへのポインタ
 * @retval FFWERR_OK          正常終了
 * @retval FFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 */
//=============================================================================
FFWERR PROT_CPU_W(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrWriteAddr[],
		const DWORD dwLength[], const BYTE* pbyWriteData)
{
	FFWERR	ferr;
	DWORD	dwCnt;
	DWORD	dwBlockLength;
	BYTE*	pbyBuff;
	WORD	wCmdCode;

	if (dwAreaNum == 0) {
		return FFWERR_OK;
	}

	wCmdCode = BFWCMD_CPU_W;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData1(BFWDATA_RESERV) != TRUE) {	// リザーブデータ(1バイト)送信
		return FFWERR_COM;
	}
	if (PutData1(static_cast<BYTE>(eAccessSize)) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData4(dwAreaNum) != TRUE) {	// ブロック数送信
		return FFWERR_COM;
	}

	pbyBuff = const_cast<BYTE*>(pbyWriteData);

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (PutData4(dwLength[dwCnt]) != TRUE) {	// アクセス回数送信
			return FFWERR_COM;
		}

		if (PutData4(eadrWriteAddr[dwCnt]) != TRUE) {	// ライトアドレス送信
			return FFWERR_COM;
		}
		
		switch (eAccessSize) {
		case EBYTE_ACCESS:
			dwBlockLength = dwLength[dwCnt];
			break;
		case EWORD_ACCESS:
			dwBlockLength = dwLength[dwCnt] * 2;
			break;
		case ELWORD_ACCESS:
			dwBlockLength = dwLength[dwCnt] * 4;
			break;
		default:
			dwBlockLength = dwLength[dwCnt] * 4;
			break;
		}

		if (PutDataN(dwBlockLength, pbyBuff) != TRUE) {	// ライトデータ送信
			return FFWERR_COM;
		}
		pbyBuff += dwBlockLength;
	}

	ferr = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

// V.1.02 No.14,15 USB高速化対応 Append Start
//=============================================================================
/**
 * BFWCmd_CPU_W_EMLコマンドの発行
 *   pbyWriteDataには、eadrWriteAddr[], dwLength[]の配列要素番号0の領域に対応する
 *   ライトデータから順に格納する。
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param dwAreaNum ライト領域数(1～0x4000)
 * @param eadrWriteAddr[] ライト開始アドレス
 * @param dwLength[] 1領域のライトアクセス回数(1～0x10000)
 * @param pbyWriteData ライトデータ格納バッファへのポインタ
 * @retval FFWERR_OK          正常終了
 * @retval FFWERR_BFW_TIMEOUT BFW処理でタイムアウトが発生した
 */
//=============================================================================
FFWERR PROT_CPU_W_EML(enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwAreaNum, const EADDR eadrWriteAddr[],
		const DWORD dwLength[], const BYTE* pbyWriteData)
{
	FFWERR	ferr;
	DWORD	dwCnt;
	DWORD	dwBlockLength;
	BYTE*	pbyBuff;
	WORD	wCmdCode;

	if (dwAreaNum == 0) {
		return FFWERR_OK;
	}

	wCmdCode = BFWCMD_CPU_W_EML;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData1(BFWDATA_RESERV) != TRUE) {	// リザーブデータ(1バイト)送信
		return FFWERR_COM;
	}
	if (PutData1(static_cast<BYTE>(eAccessSize)) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData4(dwAreaNum) != TRUE) {	// ブロック数送信
		return FFWERR_COM;
	}

	pbyBuff = const_cast<BYTE*>(pbyWriteData);

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (PutData4(dwLength[dwCnt]) != TRUE) {	// アクセス回数送信
			return FFWERR_COM;
		}

		if (PutData4(eadrWriteAddr[dwCnt]) != TRUE) {	// ライトアドレス送信
			return FFWERR_COM;
		}
		
		switch (eAccessSize) {
		case EBYTE_ACCESS:
			dwBlockLength = dwLength[dwCnt];
			break;
		case EWORD_ACCESS:
			dwBlockLength = dwLength[dwCnt] * 2;
			break;
		case ELWORD_ACCESS:
			dwBlockLength = dwLength[dwCnt] * 4;
			break;
		default:
			dwBlockLength = dwLength[dwCnt] * 4;
			break;
		}

		if (PutDataN(dwBlockLength, pbyBuff) != TRUE) {	// ライトデータ送信
			return FFWERR_COM;
		}
		pbyBuff += dwBlockLength;
	}

	ferr = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}
// V.1.02 No.14,15 USB高速化対応 Append End

//=============================================================================
/**
 * BFWCmd_CPU_FILLコマンドの発行
 *   pbyWriteBuffには、eAccessSizeで指定したバイト数分のデータを格納する。
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param eadrWriteAddr FILLアドレス
 * @param dwLength アクセス回数(1～0x10000)
 * @param pbyWriteBuff FILLデータ格納領域へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_CPU_FILL(enum FFWENM_EACCESS_SIZE eAccessSize, EADDR eadrWriteAddr, DWORD dwLength, const BYTE* pbyWriteBuff)
{
	FFWERR	ferr;
	DWORD	dwSize;
	WORD	wCmdCode;

	if (dwLength == 0) {
		return FFWERR_OK;
	}

	wCmdCode = BFWCMD_CPU_FILL;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData1(BFWDATA_RESERV) != TRUE) {	// リザーブデータ(1バイト)送信
		return FFWERR_COM;
	}
	if (PutData1(static_cast<BYTE>(eAccessSize)) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData4(dwLength) != TRUE) {	// アクセス回数送信
		return FFWERR_COM;
	}
	if (PutData4(eadrWriteAddr) != TRUE) {	// アドレス送信
		return FFWERR_COM;
	}

	switch (eAccessSize) {
	case EBYTE_ACCESS:
		dwSize = 1;
		break;
	case EWORD_ACCESS:
		dwSize = 2;
		break;
	case ELWORD_ACCESS:
		dwSize = 4;
		break;
	default:
		dwSize = 4;
		break;
	}

	if (PutDataN(dwSize, pbyWriteBuff) != TRUE) {	// ライトデータ送信
		return FFWERR_COM;
	}

	ferr = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

// V.1.02 No.14,15 USB高速化対応 Append Start
//=============================================================================
/**
 * BFWCmd_CPU_FILL_EMLコマンドの発行
 *   pbyWriteBuffには、eAccessSizeで指定したバイト数分のデータを格納する。
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param eadrWriteAddr FILLアドレス
 * @param dwLength アクセス回数(1～0x10000)
 * @param pbyWriteBuff FILLデータ格納領域へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_CPU_FILL_EML(enum FFWENM_EACCESS_SIZE eAccessSize, EADDR eadrWriteAddr, DWORD dwLength, const BYTE* pbyWriteBuff)
{
	FFWERR	ferr;
	DWORD	dwSize;
	WORD	wCmdCode;

	if (dwLength == 0) {
		return FFWERR_OK;
	}

	wCmdCode = BFWCMD_CPU_FILL_EML;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData1(BFWDATA_RESERV) != TRUE) {	// リザーブデータ(1バイト)送信
		return FFWERR_COM;
	}
	if (PutData1(static_cast<BYTE>(eAccessSize)) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData4(dwLength) != TRUE) {	// アクセス回数送信
		return FFWERR_COM;
	}
	if (PutData4(eadrWriteAddr) != TRUE) {	// アドレス送信
		return FFWERR_COM;
	}

	switch (eAccessSize) {
	case EBYTE_ACCESS:
		dwSize = 1;
		break;
	case EWORD_ACCESS:
		dwSize = 2;
		break;
	case ELWORD_ACCESS:
		dwSize = 4;
		break;
	default:
		dwSize = 4;
		break;
	}

	if (PutDataN(dwSize, pbyWriteBuff) != TRUE) {	// ライトデータ送信
		return FFWERR_COM;
	}

	ferr = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWCmd_CPU_RMWFILLコマンドの発行
 *   dwMask, dwDataは、eAccessSizeで指定されたサイズのビット値のみ使用する。
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param eadrRmwAddr リードモディファイライトアドレス
 * @param dwLength アクセス回数(1～0x10000)
 * @param dwMask マスクデータ(ライトするビットに0を指定する)
 * @param dwData ライトデータ
 * @return FFWエラーコード
 */
//=============================================================================
FFWERR PROT_CPU_RMWFILL(enum FFWENM_EACCESS_SIZE eAccessSize, EADDR eadrRmwAddr, DWORD dwLength, DWORD dwMask, DWORD dwData)
{
	FFWERR	ferr;
	WORD	wCmdCode;

	if (dwLength == 0) {
		return FFWERR_OK;
	}

	wCmdCode = BFWCMD_CPU_RMWFILL;
	if (PutCmd(wCmdCode) != TRUE) {	// コマンドコード送信
		return FFWERR_COM;
	}
	if (PutData1(BFWDATA_RESERV) != TRUE) {	// リザーブデータ(1バイト)送信
		return FFWERR_COM;
	}
	if (PutData1(static_cast<BYTE>(eAccessSize)) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData4(dwLength) != TRUE) {	// アクセス回数送信
		return FFWERR_COM;
	}
	if (PutData4(eadrRmwAddr) != TRUE) {	// アドレス送信
		return FFWERR_COM;
	}

	dwData &= ~dwMask;	// ライト対象外のビットを0にマスクする。
	if (eAccessSize == EBYTE_ACCESS) {
		dwMask &= 0x000000ff;
		dwData &= 0x000000ff;
	} else if (eAccessSize == EWORD_ACCESS) {
		dwMask &= 0x0000ffff;
		dwData &= 0x0000ffff;
	} else {
		// 何もしない
	}
	if (PutData4(dwMask) != TRUE) {	// マスクデータ送信
		return FFWERR_COM;
	}
	if (PutData4(dwData) != TRUE) {	// ライトデータ送信
		return FFWERR_COM;
	}

	ferr = PROT_BATEND_RecvErrCodeReservData(wCmdCode);	// エラーコード、リザーブデータ受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}
// RevRxE2LNo141104-001 Append End

// V.1.02 No.14,15 USB高速化対応 Append End
//=============================================================================
/**
 * BFWCmd_BATENDコマンド送信後のエラーコードとリザーブデータ受信
 * @param wCmdCode コマンドコード
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_BATEND_RecvErrCodeReservData(WORD wCmdCode)
{	
	WORD	wBuf;
	FFWERR	ferr;

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData2(&wBuf) != TRUE) {	// reserve data
		return FFWERR_COM;
	}

	return ferr;
}
//=============================================================================
/**
 * モニタCPUアクセスコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtData_Cpu(void)
{

	return;
}

