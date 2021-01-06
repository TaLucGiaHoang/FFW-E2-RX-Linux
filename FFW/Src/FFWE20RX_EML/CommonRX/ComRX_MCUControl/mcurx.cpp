////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcurx.cpp
 * @brief OCD関連コマンドのソースファイル
 * @author RSO H.Hashiguchi, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/11/28
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121022-001	2012/11/28 SDS 岩田
	EZ-CUBE PROT_MCU_DUMP()分割処理対応
*/
#include "mcurx.h"
#include "do_common.h"

#include "ffwmcu_mem.h"
#include "protmcu_mem.h"
#include "domcu_mcu.h"

#include "do_sys.h"		// RevRxNo121022-001 Append Line
#include "mcu_mem.h"	// RevRxNo121022-001 Append Line

//==============================================================================
/**
 * OCDレジスタへの書き込み(mcurx_xx.cppからのみの呼び出し)
 * @param madrWriteAddr 書き込み対象アドレス
 * @param pbyData        書き込みデータ格納ポインタ
 * @param eAccessSize   アクセスサイズ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetMcuOCDReg(MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyData)
{
	FFWERR						ferr;
	enum FFWENM_VERIFY_SET		eVerify;				// ベリファイON/OFF
	FFW_VERIFYERR_DATA			VerifyErr;				// ベリファイ結果格納構造体
	BYTE						byWriteData[4];			// 設定データを格納する領域のアドレス
	DWORD						dwWriteDataSize;
	DWORD						dwLength;
	BYTE						byEndian;

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	//エンディアン取得
	byEndian = ENDIAN_LITTLE;
	eVerify = VERIFY_OFF;

	// レジスタ設定
	dwLength = 1;
	switch(eAccessSize){
	case MBYTE_ACCESS:
		dwWriteDataSize = MDATASIZE_1BYTE;
		break;
	case MWORD_ACCESS:
		dwWriteDataSize = MDATASIZE_2BYTE;
		break;
	case MLWORD_ACCESS:
		dwWriteDataSize = MDATASIZE_4BYTE;
		break;
	default:
		dwWriteDataSize = MDATASIZE_4BYTE;
		break;
	}

	memcpy(&byWriteData, pbyData, dwWriteDataSize);

	ferr = PROT_MCU_FILL(eVerify, madrWriteAddr, eAccessSize, dwLength, dwWriteDataSize, byWriteData, &VerifyErr,byEndian);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}

//==============================================================================
/**
 * OCDレジスタからの読み込み(mcurx_xx.cppからのみの呼び出し)
 * @param madrReadAddr 読み込み対象アドレス
 * @param eAccessSize 読み込み対象アクセスサイズ
 * @param dwReadCnt リード回数
 * @param pbyData      読み込みデータ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetMcuOCDReg(MADDR madrReadAddr, enum FFWENM_MACCESS_SIZE	eAccessSize, DWORD dwReadCnt, BYTE* pbyData)
{
	FFWERR						ferr;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwAreaNum;
	DWORD						dwSize;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BYTE						byEndian;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
// RevRxNo121022-001 Append Start

	// レジスタ設定
	dwAreaNum = 1;
	bSameAccessSize = TRUE;
	bSameAccessCount = TRUE;
	//エンディアン取得
	byEndian = ENDIAN_LITTLE;

	switch(eAccessSize){
	case MBYTE_ACCESS:
		dwSize = MDATASIZE_1BYTE;
		break;
	case MWORD_ACCESS:
		dwSize = MDATASIZE_2BYTE;
		break;
	case MLWORD_ACCESS:
		dwSize = MDATASIZE_4BYTE;
		break;
	default:
		dwSize = MDATASIZE_4BYTE;
		break;
	}
	pbyReadData = new BYTE [dwSize*dwReadCnt];

	memset(pbyReadData, 0, dwSize);
	
// RevRxNo121022-001 Append Start
	if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
		ferr = McuDumpDivide(dwAreaNum, madrReadAddr, bSameAccessSize, eAccessSize, bSameAccessCount, dwReadCnt, pbyReadData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
	} else{
		ferr = PROT_MCU_DUMP(dwAreaNum, &madrReadAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwReadCnt, pbyReadData, byEndian);
	}
// RevRxNo121022-001 Append End

	if (ferr != FFWERR_OK) {
		delete [] pbyReadData;
		return ferr;
	}

	//ビッグエンディアンの場合はスワップさせる
	if(byEndian == FFWRX_ENDIAN_BIG){
		ReplaceEndian(pbyReadData, eAccessSize, dwReadCnt);
	}
	memcpy(pbyData, pbyReadData, dwSize*dwReadCnt);		// 設定内容を記憶

	delete [] pbyReadData;

	return ferr;
}

//=============================================================================
/**
 * ターゲットMCU制御関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuRxData_Mcu(void)
{
	return;
}
