///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_mem.cpp
 * @brief BFWコマンド プロトコル生成関数(メモリ操作関連)
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/11/20
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120606-004 2012/07/12 橋口
  ・MPU領域リード/ライト対応
    →修正　2012/09/14
・RevRxNo121116-001 2012/11/20 橋口, 上田
　・PROT_MCU_CPUWRITE() ベリファイエラー結果受信削除。
　・PROT_MCU_CPUFILL(), PROT_MCU_CPUWRITE()
　　　BFW送信パラメータのベリファイ指定を予約に変更。
　・PROT_MCU_CPUFILL(), PROT_MCU_WRITE(), PROT_MCU_CPUWRITE()
　　　ベリファイ結果の設定処理追加。
*/
#include "protmcu_mem.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "prot_common.h"
#include "ffw_sys.h"
#include <malloc.h>

#include "ffwmcu_mcu.h"
#include "domcu_mcu.h"

//=============================================================================
/**
 * BFWMCUCmd_DUMPコマンドの発行
 *	bSameAccessSizeがTRUEの場合、全領域eAccessSize[0]のアクセスサイズでアクセスする。
 *	bSameLengthがTRUEの場合、全領域dwLength[0]のアクセスサイズでアクセスする。
 * @param dwAreaNum リード領域数(1～0x1000)
 * @param madrReadAddr[] リード開始アドレス
 * @param bSameAccessSize 同一アクセスサイズ指定フラグ
 * @param eAccessSize[] アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameAccessCount 同一リードアクセス回数指定フラグ
 * @param dwAccessCount[] 1領域のリードアクセス回数(1～0x10000)
 * @param pbyReadData リードデータ格納用バッファへのポインタ
 * @param byEndian エンディアン指定(0:リトル、1:ビッグ)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_DUMP(DWORD dwAreaNum, const MADDR madrReadAddr[], 
					 BOOL bSameAccessSize, const enum FFWENM_MACCESS_SIZE eAccessSize[],
					 BOOL bSameAccessCount, const DWORD dwAccessCount[], BYTE *const pbyReadData, BYTE byEndian)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wProtAreaNum;
	DWORD	dwCnt;
	DWORD	dwBlockLength;		//受信データバイト長
	BYTE*	pbyBuff;
	BYTE	byData;
	DWORD	dwData;
	WORD	wCmdCode;
	enum FFWENM_MACCESS_SIZE eAccessSizeTmp;
	DWORD	dwAccessCountTmp;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	pDbgData = GetDbgDataRX();

	if (dwAreaNum == 0) {
		return FFWERR_OK;
	}

	// BFWMCUCmd_DUMP送信
	wCmdCode = BFWCMD_DUMP;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo120606-004 Append Start
	if (PutData1(DDMA_ACCESS) != TRUE) {		// アクセス方法送信
		return FFWERR_COM;
	}
	// RevRxNo120606-004 Append End

	if (PutData1(byEndian) != TRUE) {		// エンディアン指定送信
		return FFWERR_COM;
	}

	wProtAreaNum = static_cast<WORD>(dwAreaNum);

	if (PutData2(wProtAreaNum) != TRUE) {	// リード領域数送信
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			byData = static_cast<BYTE>(eAccessSize[0]);
		} else {
			byData = static_cast<BYTE>(eAccessSize[dwCnt]);
		}
		if (PutData1(byData) != TRUE) {	// アクセスサイズ送信
			return FFWERR_COM;
		}

		if (bSameAccessCount == TRUE) {
			dwData = dwAccessCount[0];
		} else {
			dwData = dwAccessCount[dwCnt];
		}
		if (PutData3(dwData) != TRUE) {	// レングス(アクセス回数)送信
			return FFWERR_COM;
		}

		if (PutData4(madrReadAddr[dwCnt]) != TRUE) {	// リード開始アドレス送信
			return FFWERR_COM;
		}
	}

	// ステータス受信
	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	if (GetData2(&wBuf) != TRUE) {	// reserve data
		return FFWERR_COM;
	}

	pbyBuff = pbyReadData;

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			eAccessSizeTmp = eAccessSize[0];
		} else {
			eAccessSizeTmp = eAccessSize[dwCnt];
		}
		if (bSameAccessCount == TRUE) {
			dwAccessCountTmp = dwAccessCount[0];
		} else {
			dwAccessCountTmp = dwAccessCount[dwCnt];
		}

		switch (eAccessSizeTmp) {
		case MBYTE_ACCESS:
			dwBlockLength = dwAccessCountTmp;
			break;
		case MWORD_ACCESS:
			dwBlockLength = dwAccessCountTmp * 2;
			break;
		case MLWORD_ACCESS:
			dwBlockLength = dwAccessCountTmp * 4;
			break;
		default:
			dwBlockLength = dwAccessCountTmp * 4;
			break;
		}

		if (GetDataN(dwBlockLength, pbyBuff) != TRUE) {
			return FFWERR_COM;
		}
		pbyBuff += dwBlockLength;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return	ferr;
}

// RevRxNo120606-004 Append Start
//=============================================================================
/**
 * BFWMCUCmd_CPUDUMPコマンドの発行
 *	bSameAccessSizeがTRUEの場合、全領域eAccessSize[0]のアクセスサイズでアクセスする。
 *	bSameLengthがTRUEの場合、全領域dwLength[0]のアクセスサイズでアクセスする。
 * @param dwAreaNum リード領域数(1～0x1000)
 * @param madrReadAddr[] リード開始アドレス
 * @param bSameAccessSize 同一アクセスサイズ指定フラグ
 * @param eAccessSize[] アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameLength 同一リードアクセス回数指定フラグ
 * @param dwLength[] 1領域のリードアクセス回数(1～0x10000)
 * @param pbyReadData リードデータ格納用バッファへのポインタ
 * @param byEndian エンディアン指定(0:リトル、1:ビッグ)
 * @return FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_CPUDUMP(DWORD dwAreaNum, const MADDR madrReadAddr[], 
					 BOOL bSameAccessSize, const enum FFWENM_MACCESS_SIZE eAccessSize[],
					 BOOL bSameLength, const DWORD dwLength[], BYTE *const pbyReadData, BYTE byEndian)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wProtAreaNum;
	DWORD	dwCnt;
	DWORD	dwBlockLength;
	BYTE*	pbyBuff;
	BYTE	byData;
	DWORD	dwData;
	WORD	wCmdCode;
	enum FFWENM_MACCESS_SIZE eAccessSizeTmp;
	DWORD	dwLengthTmp;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	pDbgData = GetDbgDataRX();

	if (dwAreaNum == 0) {
		return FFWERR_OK;
	}

	// BFWMCUCmd_DUMP送信
	wCmdCode = BFWCMD_DUMP;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1(CPU_ACCESS) != TRUE) {		// アクセス方法送信
		return FFWERR_COM;
	}


	if (PutData1(byEndian) != TRUE) {		// エンディアン指定送信
		return FFWERR_COM;
	}

	wProtAreaNum = static_cast<WORD>(dwAreaNum);

	if (PutData2(wProtAreaNum) != TRUE) {	// リード領域数送信
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			byData = static_cast<BYTE>(eAccessSize[0]);
		} else {
			byData = static_cast<BYTE>(eAccessSize[dwCnt]);
		}
		if (PutData1(byData) != TRUE) {	// アクセスサイズ送信
			return FFWERR_COM;
		}

		if (bSameLength == TRUE) {
			dwData = dwLength[0];
		} else {
			dwData = dwLength[dwCnt];
		}
		if (PutData3(dwData) != TRUE) {	// レングス(アクセス回数)送信
			return FFWERR_COM;
		}

		if (PutData4(madrReadAddr[dwCnt]) != TRUE) {	// リード開始アドレス送信
			return FFWERR_COM;
		}
	}

	// ステータス受信
	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData2(&wBuf) != TRUE) {	// reserve data
		return FFWERR_COM;
	}

	pbyBuff = pbyReadData;

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			eAccessSizeTmp = eAccessSize[0];
		} else {
			eAccessSizeTmp = eAccessSize[dwCnt];
		}
		if (bSameLength == TRUE) {
			dwLengthTmp = dwLength[0];
		} else {
			dwLengthTmp = dwLength[dwCnt];
		}

		switch (eAccessSizeTmp) {
		case MBYTE_ACCESS:
			dwBlockLength = dwLengthTmp;
			break;
		case MWORD_ACCESS:
			dwBlockLength = dwLengthTmp * 2;
			break;
		case MLWORD_ACCESS:
			dwBlockLength = dwLengthTmp * 4;
			break;
		default:
			dwBlockLength = dwLengthTmp * 4;
			break;
		}

		if (GetDataN(dwBlockLength, pbyBuff) != TRUE) {
			return FFWERR_COM;
		}
		pbyBuff += dwBlockLength;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return	ferr;
}
// RevRxNo120606-004 Append End

// V.1.02 No.3 起動時エンディアン&デバッグ継続モード処理 Append Start
//=============================================================================
/**
 * BFWMCUCmd_DDUMPコマンドの発行
 *	bSameAccessSizeがTRUEの場合、全領域eAccessSize[0]のアクセスサイズでアクセスする。
 *	bSameLengthがTRUEの場合、全領域dwLength[0]のアクセスサイズでアクセスする。
 * @param dwAreaNum リード領域数(1～0x1000)
 * @param madrReadAddr[] リード開始アドレス
 * @param bSameAccessSize 同一アクセスサイズ指定フラグ
 * @param eAccessSize[] アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameAccessCount 同一リードアクセス回数指定フラグ
 * @param dwAccessCount[] 1領域のリードアクセス回数(1～0x10000)
 * @param pbyReadData リードデータ格納用バッファへのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_DDUMP(DWORD dwAreaNum, const MADDR madrReadAddr[], 
					 BOOL bSameAccessSize, const enum FFWENM_MACCESS_SIZE eAccessSize[],
					 BOOL bSameAccessCount, const DWORD dwAccessCount[], BYTE *const pbyReadData)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wProtAreaNum;
	DWORD	dwCnt;
	DWORD	dwBlockLength;	//受信データバイト長
	BYTE*	pbyBuff;
	BYTE	byData;
	DWORD	dwData;
	WORD	wCmdCode;
	enum FFWENM_MACCESS_SIZE eAccessSizeTmp;
	DWORD	dwAccessCountTmp;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	pDbgData = GetDbgDataRX();

	if (dwAreaNum == 0) {
		return FFWERR_OK;
	}

	// BFWMCUCmd_DUMP送信
	wCmdCode = BFWCMD_DDUMP;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wProtAreaNum = static_cast<WORD>(dwAreaNum);

	if (PutData2(wProtAreaNum) != TRUE) {	// リード領域数送信
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			byData = static_cast<BYTE>(eAccessSize[0]);
		} else {
			byData = static_cast<BYTE>(eAccessSize[dwCnt]);
		}
		if (PutData1(byData) != TRUE) {	// アクセスサイズ送信
			return FFWERR_COM;
		}

		if (bSameAccessCount == TRUE) {
			dwData = dwAccessCount[0];
		} else {
			dwData = dwAccessCount[dwCnt];
		}
		if (PutData3(dwData) != TRUE) {	// レングス(アクセス回数)送信
			return FFWERR_COM;
		}

		if (PutData4(madrReadAddr[dwCnt]) != TRUE) {	// リード開始アドレス送信
			return FFWERR_COM;
		}
	}

	// ステータス受信
	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData2(&wBuf) != TRUE) {	// reserve data
		return FFWERR_COM;
	}

	pbyBuff = pbyReadData;

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			eAccessSizeTmp = eAccessSize[0];
		} else {
			eAccessSizeTmp = eAccessSize[dwCnt];
		}
		if (bSameAccessCount == TRUE) {
			dwAccessCountTmp = dwAccessCount[0];
		} else {
			dwAccessCountTmp = dwAccessCount[dwCnt];
		}

		switch (eAccessSizeTmp) {
		case MBYTE_ACCESS:
			dwBlockLength = dwAccessCountTmp;
			break;
		case MWORD_ACCESS:
			dwBlockLength = dwAccessCountTmp * 2;
			break;
		case MLWORD_ACCESS:
			dwBlockLength = dwAccessCountTmp * 4;
			break;
		default:
			dwBlockLength = dwAccessCountTmp * 4;
			break;
		}

		if (GetDataN(dwBlockLength, pbyBuff) != TRUE) {
			return FFWERR_COM;
		}
		pbyBuff += dwBlockLength;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return	ferr;
}
// V.1.02 No.3 起動時エンディアン&デバッグ継続モード処理 Append End


//=============================================================================
/**
 * BFWMCUCmd_FILLコマンドの発行
 *   BFWMCUCmd_FILLコマンドを送信し、DDMA経由でターゲットMCU空間へのメモリFILLを行う。
 * @param eVerify ベリファイ指定(VERIFY_OFF/VERIFY_ON)(BFWではベリファイをしない)
 * @param madrWriteAddr FILL開始アドレス
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param dwAccessCount FILLアクセス回数(1～0x10000)
 * @param dwWriteDataLength FILLデータバイトサイズ(MDATASIZE_1BYTE/MDATASIZE_2BYTE/MDATASIZE_4BYTE/MDATASIZE_8BYTE)
 * @param pbyWriteBuff FILLデータ格納領域へのポインタ
 * @param pVerifyErrorInfo ベリファイ結果格納領域へのポインタ(BFWではベリファイをしない)
 * @param byEndian エンディアン指定(0:リトル、1:ビッグ)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_FILL(enum FFWENM_VERIFY_SET eVerify, MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE eAccessSize,
					 DWORD dwAccessCount, DWORD dwWriteDataLength, const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian)
{
	FFWERR	ferr;
	BYTE	byData;
	WORD	wData;
	WORD	wCmdCode;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	//ワーニング対策
	eVerify;

	pDbgData = GetDbgDataRX();

	if (dwAccessCount == 0) {
		return FFWERR_OK;
	}

	// BFWMCUCmd_FILL送信
	wCmdCode = BFWCMD_FILL;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo120606-004 Append Start
	if (PutData1(DDMA_ACCESS) != TRUE) {		// アクセス方法送信
		return FFWERR_COM;
	}
	// RevRxNo120606-004 Append End

	if (PutData1(byEndian) != TRUE) {		// エンディアン指定送信
		return FFWERR_COM;
	}

	byData = 0x00;
	if (PutData1(byData) != TRUE) {			// 予約
		return FFWERR_COM;
	}
	byData = static_cast<BYTE>(eAccessSize);
	if (PutData1(byData) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData3(dwAccessCount) != TRUE) {	// レングス(FILLアクセス回数)送信
		return FFWERR_COM;
	}
	if (PutData4(madrWriteAddr) != TRUE) {	// FILL開始アドレス送信
		return FFWERR_COM;
	}
	wData = static_cast<WORD>(dwWriteDataLength);
	if (PutData2(wData) != TRUE) {	// ライトデータサイズ送信
		return FFWERR_COM;
	}
	if (PutDataN(dwWriteDataLength, pbyWriteBuff) != TRUE) {	// ライトデータ送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	//メモリエラー対応
	pVerifyErrorInfo->eErrorFlag = VERIFY_OK;

	ferr = ProtRcvHaltCode(&wData);	// 処理中断コード受信

	return ferr;
}

// RevRxNo120606-004 Append Start
//=============================================================================
/**
 * BFWMCUCmd_FILLコマンドの発行
 *   BFWMCUCmd_FILLコマンドを送信し、CPU経由でターゲットMCU空間へのメモリFILLを行う。
 * @param eVerify ベリファイ指定(VERIFY_OFF/VERIFY_ON)
 * @param madrWriteAddr FILL開始アドレス
 * @param eAccessSize アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param dwLength FILLアクセス回数(1～0x10000)
 * @param dwWriteDataSize FILLデータサイズ(MDATASIZE_1BYTE/MDATASIZE_2BYTE/MDATASIZE_4BYTE/MDATASIZE_8BYTE)
 * @param pbyWriteBuff FILLデータ格納領域へのポインタ
 * @param pVerifyErrorInfo ベリファイ結果格納領域へのポインタ(BFWではベリファイをしない)
 * @param byEndian エンディアン指定(0:リトル、1:ビッグ)
 * @return FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_CPUFILL(enum FFWENM_VERIFY_SET eVerify, MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE eAccessSize,
					 DWORD dwLength, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian)
{
	FFWERR	ferr;
	BYTE	byData;
	WORD	wData;
	WORD	wCmdCode;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	//ワーニング対応
	// RevRxNo121116-001 Modify Line
	eVerify;

	pDbgData = GetDbgDataRX();

	if (dwLength == 0) {
		return FFWERR_OK;
	}

	// BFWMCUCmd_FILL送信
	wCmdCode = BFWCMD_FILL;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1(CPU_ACCESS) != TRUE) {		// アクセス方法送信
		return FFWERR_COM;
	}

	if (PutData1(byEndian) != TRUE) {		// エンディアン指定送信
		return FFWERR_COM;
	}

	// RevRxNo121116-001 Modify Line
	byData = 0x00;
	if (PutData1(byData) != TRUE) {	// 予約
		return FFWERR_COM;
	}
	byData = static_cast<BYTE>(eAccessSize);
	if (PutData1(byData) != TRUE) {	// アクセスサイズ送信
		return FFWERR_COM;
	}
	if (PutData3(dwLength) != TRUE) {	// レングス(FILLアクセス回数)送信
		return FFWERR_COM;
	}
	if (PutData4(madrWriteAddr) != TRUE) {	// FILL開始アドレス送信
		return FFWERR_COM;
	}
	wData = static_cast<WORD>(dwWriteDataSize);
	if (PutData2(wData) != TRUE) {	// ライトデータサイズ送信
		return FFWERR_COM;
	}
	if (PutDataN(dwWriteDataSize, pbyWriteBuff) != TRUE) {	// ライトデータ送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wData);	// 処理中断コード受信

	//メモリエラー対応
	// RevRxNo121116-001 Append Line
	pVerifyErrorInfo->eErrorFlag = VERIFY_OK;

	return ferr;
}
// RevRxNo120606-004 Append End

//=============================================================================
/**
 * BFWMCUCmd_WRITEコマンドの発行　dDMAアクセス
 *   pbyWriteData[]には、madrWriteAddr[], eAccessSize[], dwLength[]の
 *   配列要素番号0の領域に対応するライトデータから順に格納する。
 *   bSameAccessSizeがTRUEの場合、全領域eAccessSize[0]のアクセスサイズでアクセスする。
 *   bSameLengthがTRUEの場合、全領域dwLength[0]のアクセスサイズでアクセスする。
 *   bSameWriteDataがTRUEの場合、全領域pbyWriteDataの先頭ライトデータをライトする。
 * @param eVerify べリファイ指定(VERIFY_OFF/VERIFY_ON)(BFWではベリファイをしない)
 * @param dwAreaNum ライト領域数
 * @param madrWriteAddr[] ライト開始アドレス
 * @param bSameAccessSize 同一アクセスサイズ指定フラグ
 * @param eAccessSize[] アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameAccessCount 同一ライトアクセス回数指定フラグ
 * @param dwAccessCount[] 1領域のライトアクセス回数(1～0x10000)
 * @param bSameWriteData 同一ライトデータ指定フラグ
 * @param pbyWriteData ライトデータ格納用バッファへのポインタ
 * @param pVerifyErrorInfo ベリファイ結果格納用バッファアドレス(BFWではベリファイをしない)
 * @param byEndian エンディアン指定(0:リトル、1:ビッグ)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_WRITE(enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, const MADDR madrWriteAddr[], 
					  BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize[], 
					  BOOL bSameAccessCount, const DWORD dwAccessCount[], 
					  BOOL bSameWriteData, const BYTE* pbyWriteData, 
					  FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian)
{
	FFWERR	ferr;
	BYTE	byData;
	WORD	wBuf;
	WORD	wData;
	DWORD	dwCnt;
	BYTE*	pbyData;
	enum FFWENM_MACCESS_SIZE eSendAcc;
	DWORD	dwSendLength;				// 送信WRITEデータのバイト長
	DWORD	dwSendAccessCount;			// 送信WRITEアクセス回数
	WORD	wCmdCode;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// ワーニング対策
	// RevRxNo121116-001 Delete
	eVerify;

	pDbgData = GetDbgDataRX();

	// BFWMCUCmd_WRITE送信
	wCmdCode = BFWCMD_WRITE;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo120606-004 Append Start
	if (PutData1(DDMA_ACCESS) != TRUE) {		// アクセス方法送信
		return FFWERR_COM;
	}
	// RevRxNo120606-004 Append End

	if (PutData1(byEndian) != TRUE) {		// エンディアン指定送信
		return FFWERR_COM;
	}

	byData = 0x00;
	if (PutData1(byData) != TRUE) {			// 予約
		return FFWERR_COM;
	}
	if (PutData2(BFWDATA_RESERV) != TRUE) {
		return FFWERR_COM;
	}
	wData = static_cast<WORD>(dwAreaNum);
	if (PutData2(wData) != TRUE) {	// ブロック数送信
		return FFWERR_COM;
	}

	pbyData = const_cast<BYTE*>(pbyWriteData);

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			eSendAcc = eAccessSize[0];
		} else {
			eSendAcc = eAccessSize[dwCnt];
		}
		byData = static_cast<BYTE>(eSendAcc);
		if (PutData1(byData) != TRUE) {		// アクセスサイズ送信
			return FFWERR_COM;
		}

		if (bSameAccessCount == TRUE) {
			dwSendAccessCount = dwAccessCount[0];
		} else {
			dwSendAccessCount = dwAccessCount[dwCnt];
		}
		if (PutData3(dwSendAccessCount) != TRUE) {	// レングス送信
			return FFWERR_COM;
		}

		if (PutData4(madrWriteAddr[dwCnt]) != TRUE) {	// アドレス送信
			return FFWERR_COM;
		}

		if (eSendAcc == MWORD_ACCESS) {
			//ワードアクセス
			dwSendLength = dwSendAccessCount * 2;
		}else if (eSendAcc == MLWORD_ACCESS) {
			//Lワードアクセス
			dwSendLength = dwSendAccessCount * 4;
		} else {
			//バイトアクセス
			dwSendLength = dwSendAccessCount;
		}
		if (PutDataN(dwSendLength, pbyData) != TRUE) {	// ライトデータ送信
			return FFWERR_COM;
		}

		if (bSameWriteData == FALSE) {
			pbyData += dwSendLength;	// ライトデータ格納バッファアドレス更新
		}
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	//メモリエラー対応
	// RevRxNo121116-001 Append Line
	pVerifyErrorInfo->eErrorFlag = VERIFY_OK;

	return ferr;
}
// RevRxNo120606-004 Append Start
//=============================================================================
/**
 * BFWMCUCmd_WRITEコマンドの発行 CPUアクセス
 *   pbyWriteData[]には、madrWriteAddr[], eAccessSize[], dwLength[]の
 *   配列要素番号0の領域に対応するライトデータから順に格納する。
 *   bSameAccessSizeがTRUEの場合、全領域eAccessSize[0]のアクセスサイズでアクセスする。
 *   bSameLengthがTRUEの場合、全領域dwLength[0]のアクセスサイズでアクセスする。
 *   bSameWriteDataがTRUEの場合、全領域pbyWriteDataの先頭ライトデータをライトする。
 * @param eVerify べリファイ指定(VERIFY_OFF/VERIFY_ON)
 * @param dwAreaNum ライト領域数
 * @param madrWriteAddr[] ライト開始アドレス
 * @param bSameAccessSize 同一アクセスサイズ指定フラグ
 * @param eAccessSize[] アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameLength 同一ライトアクセス回数指定フラグ
 * @param dwLength[] 1領域のライトアクセス回数(1～0x10000)
 * @param bSameWriteData 同一ライトデータ指定フラグ
 * @param pbyWriteData ライトデータ格納用バッファへのポインタ
 * @param pVerifyErrorInfo ベリファイ結果格納用バッファアドレス(BFWではベリファイをしない)
 * @param byEndian エンディアン指定(0:リトル、1:ビッグ)
 * @return FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_CPUWRITE(enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, const MADDR madrWriteAddr[], 
					  BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize[], 
					  BOOL bSameLength, const DWORD dwLength[], 
					  BOOL bSameWriteData, const BYTE* pbyWriteData, 
					  FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian)
{
	FFWERR	ferr;
	BYTE	byData;
	WORD	wBuf;
	WORD	wData;
	DWORD	dwCnt;
	BYTE*	pbyData;
	enum FFWENM_MACCESS_SIZE eSendAcc;
	DWORD	dwSendLength;
	WORD	wCmdCode;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// ワーニング対策
	// RevRxNo121116-001 Appned Line
	eVerify;

	pDbgData = GetDbgDataRX();

	// BFWMCUCmd_WRITE送信
	wCmdCode = BFWCMD_WRITE;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1(CPU_ACCESS) != TRUE) {		// アクセス方法送信
		return FFWERR_COM;
	}

	if (PutData1(byEndian) != TRUE) {		// エンディアン指定送信
		return FFWERR_COM;
	}

	// RevRxNo121116-001 Modify Line
	byData = 0x00;
	if (PutData1(byData) != TRUE) {	// 予約
		return FFWERR_COM;
	}
	if (PutData2(BFWDATA_RESERV) != TRUE) {
		return FFWERR_COM;
	}
	wData = static_cast<WORD>(dwAreaNum);
	if (PutData2(wData) != TRUE) {	// ブロック数送信
		return FFWERR_COM;
	}

	pbyData = const_cast<BYTE*>(pbyWriteData);

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			eSendAcc = eAccessSize[0];
		} else {
			eSendAcc = eAccessSize[dwCnt];
		}
		byData = static_cast<BYTE>(eSendAcc);
		if (PutData1(byData) != TRUE) {		// アクセスサイズ送信
			return FFWERR_COM;
		}

		if (bSameLength == TRUE) {
			dwSendLength = dwLength[0];
		} else {
			dwSendLength = dwLength[dwCnt];
		}
		if (PutData3(dwSendLength) != TRUE) {	// レングス送信
			return FFWERR_COM;
		}

		if (PutData4(madrWriteAddr[dwCnt]) != TRUE) {	// アドレス送信
			return FFWERR_COM;
		}

		if (eSendAcc == MWORD_ACCESS) {
			dwSendLength = dwSendLength * 2;
		}else if (eSendAcc == MLWORD_ACCESS) {
			dwSendLength = dwSendLength * 4;
		}
		if (PutDataN(dwSendLength, pbyData) != TRUE) {	// ライトデータ送信
			return FFWERR_COM;
		}

		if (bSameWriteData == FALSE) {
			pbyData += dwSendLength;	// ライトデータ格納バッファアドレス更新
		}
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo121116-001 Delete
	// ベリファイエラー結果受信処理削除

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	//メモリエラー対応
	// RevRxNo121116-001 Append Line
	pVerifyErrorInfo->eErrorFlag = VERIFY_OK;

	return ferr;
}
// RevRxNo120606-004 Append End

//=============================================================================
/**
 * メモリ操作コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Mem(void)
{
	return;
}
