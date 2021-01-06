///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_closed.cpp
 * @brief 非公開コマンドの実装ファイル
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
#include "domcu_closed.h"
#include "protmcu_closed.h"
#include "mcu_flash.h"

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////

// 2008.8.28 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * JTAGのIRパスを実行
 * @param dwBitLength IRパスのビット数
 * @param pbyData IRパスでの送受信データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RXIR(DWORD dwBitLength, BYTE* pbyData)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	BYTE	byData = 0;
	DWORD	dwIrData[IR_BIT_NUM];	// 1024bit
	BYTE	*pbyOrgData;
	DWORD	dwCnt, dwCnt2;
	DWORD	dwBuffData;

	pbyOrgData = const_cast<BYTE*>(pbyData);

	ProtInit();

	ferr = PROT_MCU_RXIR(dwBitLength, pbyData);

	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	memset(&dwBuffData, 0, sizeof(dwBuffData));

	for (dwCnt = 0; dwCnt < (dwBitLength/IR_BIT_NUM); dwCnt++) {
		for (dwCnt2 = sizeof(DWORD); dwCnt2 > 0; dwCnt2--) {
			dwBuffData = (DWORD)byData & 0x000000ff;
			dwBuffData <<= (dwCnt2 - 1) * 8;								// 24bitシフト,16bitシフト,8bitシフト
			dwIrData[dwCnt] |= dwBuffData;
			byData++;
		}
		memcpy((DWORD*)pbyOrgData, &dwIrData[dwCnt], sizeof(dwIrData[0]));	// 返値をバッファへ
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * JTAGのDRパスを実行
 * @param dwBitLength DRパスのビット数
 * @param pbyData DRパスでの送受信データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RXDR(DWORD dwBitLength, BYTE* pbyData, DWORD dwPause)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_RXDR(dwBitLength, pbyData, dwPause);

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
 * JTAGのDRパスを実行(Getのみ)
 * @param dwBitLength DRパスのビット数
 * @param pbyData DRパスでの送受信データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RXGetDR(DWORD dwBitLength, BYTE* pbyData, DWORD dwPause)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_RXGetDR(dwBitLength, pbyData, dwPause);

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
// 2008.8.28 INSERT_END_E20RX600 }

//=============================================================================
/**
 * 命令コード供給への命令書き込み	
 * @param byLowrdLength 命令のデータのLWORDサイズ
 * @param pdwData 書き込むデータ
  * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RXSEMC(BYTE byLwordLength, DWORD* pdwData)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_RXSEMC(byLwordLength, pdwData);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * 命令コード供給実行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RXMAST(void)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_RXMAST();

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * 命令コード供給後C2E0に渡されたデータ取得
 * @param pdwData C2E0に渡されたデータ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RXGEMD(DWORD* pdwData)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_RXGEMD(pdwData);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * 命令コード供給にC2E0で渡すデータ
 * @param dwData C2E0で渡すデータ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RXSEMP(DWORD dwData)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_RXSEMP(dwData);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * シリアルアクセスレジスタ設定
 * @param byCommand		設定するシリアルアクセスレジスタ番号
 * @param eAccessSize	シリアルアクセスレジスタアクセスサイズ
 * @param dwWriteData	シリアルアクセスレジスタへ書き込むデータ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetRXSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteData)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_SetRXSAR(byCommand, eAccessSize, dwWriteData);
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * シリアルアクセスレジスタ参照
 * @param byCommand		参照するシリアルアクセスレジスタ番号
 * @param eAccessSize	シリアルアクセスレジスタアクセスサイズ
 * @param pdwReadData	シリアルアクセスレジスタから読み出したデータを格納する
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetRXSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD *pdwReadData)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();
	ferr = PROT_MCU_GetRXSAR(byCommand, eAccessSize, pdwReadData);
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	return ferr;
}
//=============================================================================
/**
 * フラッシュROMブロックをクリア(FCLRで登録したブロックのみ)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_FBER(void)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();
	//キャッシュのクリア
	ferr = EraseFlashRomBlock();
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * フラッシュROMキャッシュをクリア
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_FCCLR(void)
{

	InitFlashRomCacheMemFlg();

	return FFWERR_OK;
}

//=============================================================================
/**
 * 非公開コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Closed(void)
{
	return;
}
