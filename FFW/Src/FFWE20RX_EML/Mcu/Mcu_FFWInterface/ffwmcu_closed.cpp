///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_closed.cpp
 * @brief 非公開コマンドの実装ファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/11/14
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120727-001 2012/07/27 橋口
	内蔵ROM DUMP 高速化 
*/
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#elif __linux__
#endif

#include "ffwmcu_closed.h"
#include "domcu_closed.h"

#include "ffwmcu_mcu.h"
#include "domcu_mcu.h"

//V.1.02 RevNo110613-001 Append Line
#include "errchk.h"
#include "domcu_prog.h"		// RevNo120727-001 Append Line

///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * JTAGのIRパスを実行
 * @param dwLength IRパスのビット数
 * @param pbyData IRパスでの送受信データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_IR(DWORD dwLength, BYTE* pbyData)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// 引数チェック
	if (dwLength > IR_LEN_MAX) {
		return FFWERR_FFW_ARG;
	}

	ferr = DO_RXIR(dwLength, pbyData);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * JTAGのDRパスを実行
 * @param dwLength DRパスのビット数
 * @param pbyData DRパスでの送受信データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_DR(DWORD dwLength, BYTE* pbyData, DWORD dwPause)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// 引数チェック
	if (dwLength > DR_LEN_MAX) {
		return FFWERR_FFW_ARG;
	}

	ferr = DO_RXDR(dwLength, pbyData, dwPause);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * JTAGのDRパスを実行(参照のみ)
 * @param dwLength DRパスのビット数
 * @param pbyData DRパスでの送受信データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetDR(DWORD dwLength, BYTE* pbyData, DWORD dwPause)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// 引数チェック
	if (dwLength > DR_LEN_MAX) {
		return FFWERR_FFW_ARG;
	}

	ferr = DO_RXGetDR(dwLength, pbyData, dwPause);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * 命令コード供給への命令書き込み	
 * @param byLength 命令のデータサイズ
 * @param pdwData 書き込むデータ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_SEMC(BYTE byLength, DWORD* pdwData)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	ferr = DO_RXSEMC(byLength, pdwData);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * 命令コード供給実行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_MAST(void)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	ferr = DO_RXMAST();

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * 命令コード供給後C2E0に渡されたデータ取得
 * @param pdwData C2E0に渡されたデータ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GEMD(DWORD* dwData)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	ferr = DO_RXGEMD(dwData);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * 命令コード供給にC2E0で渡すデータ
 * @param deData C2E0で渡すデータ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_SEMP(DWORD dwData)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	ferr = DO_RXSEMP(dwData);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * フラッシュROMキャッシュをクリア
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_FCCLR(void)
{
	FFWERR	ferr = FFWERR_OK;

	ferr = DO_FCCLR();	

	return ferr;
}

//=============================================================================
/**
 * フラッシュROMブロックをクリア(FCLRで登録したブロックのみ)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_FBER(void)
{
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevNo120727-001 Append Start
	// 内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevNo120727-001 Append End

	ferr = DO_FBER();

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

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
FFWE100_API	DWORD FFWRXCmd_SetSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteData)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// 引数エラーチェック
	if( byCommand < SAR_W_E2C0 || byCommand > SAR_W_MONCODE ){
		return FFWERR_FFW_ARG;
	}
	
	ferr = DO_SetRXSAR(byCommand, eAccessSize, dwWriteData);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

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
FFWE100_API	DWORD FFWRXCmd_GetSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD *pdwReadData)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// 引数エラーチェック
	if( byCommand < SAR_R_MCU_MR || byCommand > SAR_R_MCUPSW ){
		return FFWERR_FFW_ARG;
	}
		
	ferr = DO_GetRXSAR(byCommand, eAccessSize, pdwReadData);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * 非公開コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Closed(void)
{

	return;

}
