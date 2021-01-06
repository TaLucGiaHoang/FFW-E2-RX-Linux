///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_prog.cpp
 * @brief プログラム実行関連コマンドの実装ファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, S.Ueda, K.Uemori
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/20
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120621-002 2012/07/12 橋口
  HotPlug起動中の実行系IFで実行中エラー判定を実施するように修正
・RevRxNo120910-001	2012/10/15 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・FFWRXCmd_GetPROG の引数dwSizeOfProgStruct追加。
    FFWRXCmd_GetPROG で、
  　V.2.00.00(本仕様の初期バージョン)時点では、"プログラム実行中の各種情報を
  　格納する構造体サイズ"が構造体サイズより小さい場合、引数エラーとする。
  　"プログラム実行中の各種情報を格納する構造体サイズ"が構造体サイズより大きい場合、
  　エラー「FFWに設定可能なパラメータサイズを超えていた(Warning)」を返すことを追加。
　・FFWMCUCmd_SetSSST()、FFWMCUCmd_SetSSEN() の空の関数定義追加。
・RevRxNo130809-001 2013/08/20 上田
	PROGコマンドのパラメータ可変長対応処理改善
・RevRxNo130730-005 2013/11/21 上田
	内蔵ROM有効/無効判定処理改善
・RevRxNo140515-007 2014/06/18 植盛
	SSST, SSENコマンド追加による速度改善
・RevRxNo140515-008 2014/06/18 植盛
	ステップ実行の高速化対応
・RevRxNo140617-001	2014/06/17 大喜多
	TrustedMemory機能対応
・RevRxNo130730-001 2014/07/22 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxE2LNo141104-001 2014/11/20 上田
	E2 Lite対応
*/

#include "ffwmcu_prog.h"
#include "domcu_prog.h"
#include "errchk.h"
#include "ffwmcu_mcu.h"
#include "ffwmcu_reg.h"		// RevRxNo140617-001 Appned Line
#include "domcu_mcu.h"		// RevRxNo140617-001 Appned Line
#include "domcu_rst.h"		// RevRxNo130730-001 Append Line

// static変数
static BOOL		s_bSsstState;		// SSSTコマンド発行状態フラグ

///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * プログラムのフリー実行
 * @param eStartAddrSet 実行開始アドレスの設定有無
 * @param dwmadrStartAddr 実行開始アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_GO(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuDef = GetMcuDefData();

	if (eStartAddrSet == ADDR_SETON) {	// 実行開始アドレス指定時
		if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {	// 実行開始アドレスの引数チェック
			return FFWERR_FFW_ARG;
		}
	}

	// RevRxNo120621-002 Modify Start
	// RevRxNo140515-008 Modify Line
	if (GetMcuRunState() == TRUE) {	// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}
	// RevRxNo120621-002 Modify End

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// 内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End

	// RevRxNo140515-007 Append Start
	if (s_bSsstState == TRUE) {	// SSSTコマンド発行状態の場合
		ferr = DO_SetSSEN();		// 連続ステップ処理の終了処理を実施
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-007 Append End

	ferr = DO_GO(eStartAddrSet, dwmadrStartAddr);


	// ユーザプログラムを実行するFFW I/F発行時、ユーザプログラム実行処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグをTRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * プログラムのブレーク付き実行
 * @param eStartAddrSet 実行開始アドレスの設定有無
 * @param dwmadrStartAddr 実行開始アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_GB(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuDef = GetMcuDefData();

	if (eStartAddrSet == ADDR_SETON) {	// 実行開始アドレス指定時
		if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {	// 実行開始アドレスの引数チェック
			return FFWERR_FFW_ARG;
		}
	}

	// RevRxNo120621-002 Modify Start
	// RevRxNo140515-008 Modify Line
	if (GetMcuRunState() == TRUE) {	// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}
	// RevRxNo120621-002 Modify End

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// 内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End

	// RevRxNo140515-007 Append Start
	if (s_bSsstState == TRUE) {	// SSSTコマンド発行状態の場合
		ferr = DO_SetSSEN();		// 連続ステップ処理の終了処理を実施
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-007 Append End

	ferr = DO_GB(eStartAddrSet, dwmadrStartAddr);

	// ユーザプログラムを実行するFFW I/F発行時、ユーザプログラム実行処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグをTRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * プログラムのリセット付き実行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_RSTG(void)
{
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrOsmFwriteWarningFlg();	// RevRxNo130730-001 Append Line

	// RevRxNo120621-002 Modify Start
	// RevRxNo140515-008 Modify Line
	if (GetMcuRunState() == TRUE) {	// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}
	// RevRxNo120621-002 Modify End

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// 内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End

	// RevRxNo140515-007 Append Start
	if (s_bSsstState == TRUE) {	// SSSTコマンド発行状態の場合
		ferr = DO_SetSSEN();		// 連続ステップ処理の終了処理を実施
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-007 Append End

	ferr = DO_RSTG();

	// RevRxNo130730-005 Delete

	// ユーザプログラムを実行するFFW I/F発行時、ユーザプログラム実行処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグをTRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// RevRxNo130730-001 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_OsmFwrite();			// オプション設定メモリ書き戻し時のWarning発生確認
	}
	// RevRxNo130730-001 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * プログラムのプログラムブレークポイント付き実行
 * @param eStartAddrSet 実行開始アドレスの設定有無
 * @param dwmadrStartAddr 実行開始アドレス
 * @param dwmadrBreakAddr 実行停止アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_GPB(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr,	DWORD dwmadrBreakAddr)
{
	FFWERR	ferr= FFWERR_OK;
	BOOL	bStepOverExec;			// RevRxNo140515-008 Append Line
	FFWMCU_MCUDEF_DATA* pMcuDef;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuDef = GetMcuDefData();

	if (eStartAddrSet == ADDR_SETON) {	// 実行開始アドレス指定時
		if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {	// 実行開始アドレスの引数チェック
			return FFWERR_FFW_ARG;
		}
	}
	if (dwmadrBreakAddr > pMcuDef->madrMaxAddr) {	// 実行停止アドレスの引数チェック
		return FFWERR_FFW_ARG;
	}

	// RevRxNo120621-002 Modify Start
	// RevRxNo140515-008 Modify Line
	if (GetMcuRunState() == TRUE) {	// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}
	// RevRxNo120621-002 Modify End

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// 内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End

	// RevRxNo140515-007 Append Start
	if (s_bSsstState == TRUE) {	// SSSTコマンド発行状態の場合
		ferr = DO_SetSSEN();		// 連続ステップ処理の終了処理を実施
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-007 Append End

	// RevRxNo140515-008 Modify Start
	bStepOverExec = FALSE;	// GPBコマンドからの実行のため、実行情報をFALSEに設定
	ferr = DO_GPB(eStartAddrSet, dwmadrStartAddr, dwmadrBreakAddr, bStepOverExec);
	// RevRxNo140515-008 Modify End

	// ユーザプログラムを実行するFFW I/F発行時、ユーザプログラム実行処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグを、TRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * プログラムの実行後ブレークポイント付き実行
 * @param eStartAddrSet 実行開始アドレスの設定有無
 * @param dwmadrStartAddr 実行開始アドレス
 * @param dwmadrBreakAddr 実行停止アドレス
 * @param pStep ステップ実行時のブレーク要因格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_GPBA(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr, 
								 DWORD dwmadrBreakAddr, FFWE20_STEP_DATA* pStep)
{
	FFWERR	ferr= FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	FFWMCU_REG_DATA_RX*	pRegDataRX;		// RevRxNo140617-001 Append Line	

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuDef = GetMcuDefData();

	if (eStartAddrSet == ADDR_SETON) {	// 実行開始アドレス指定時
		if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {	// 実行開始アドレスの引数チェック
			return FFWERR_FFW_ARG;
		}
	}
	if (dwmadrBreakAddr > pMcuDef->madrMaxAddr) {	// 実行停止アドレスの引数チェック
		return FFWERR_FFW_ARG;
	}

	// RevRxNo120621-002 Modify Start
	// RevRxNo140515-008 Modify Line
	if (GetMcuRunState() == TRUE) {	// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}
	// RevRxNo120621-002 Modify End

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// 内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo140617-001 Append Start
	if (GetPmodeInRomDisFlg() == FALSE) {	// 内蔵ROM無効拡張モードでない場合
		if (GetTMEnable() == TRUE) {		// TM機能有効の場合
			pRegDataRX = GetRegData_RX();					// レジスタ情報取得
			if (ChkTmArea(pRegDataRX->dwRegData[REG_NUM_RX_PC]) == TRUE) {		// 現在のPC値がTM領域内の場合
				return FFWERR_STEPADDR_TMAREA;
			}
		}
	}
	// RevRxNo140617-001 Append End

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_GPBA(eStartAddrSet, dwmadrStartAddr, dwmadrBreakAddr, pStep);

	// ユーザプログラムを実行するFFW I/F発行時、ユーザプログラム実行処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグを、TRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * プログラム実行中の情報入手
 * @param pProg     ユーザプログラム実行中の各種情報格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo120910-001 Modify Line
FFWE100_API	DWORD FFWRXCmd_GetPROG(DWORD dwSizeOfProgStruct, FFWRX_PROG_DATA* pProg)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo130809-001 Append Start
	FFWRX_PROG_DATA	progDataTmp;		// ユーザプログラム実行中各種情報の一時格納用変数
	BOOL	bOverStructSize;			// 構造体サイズが超えているかを示すフラグ
	DWORD	dwSetSize;
	// RevRxNo130809-001 Append End

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// 引数チェック

	// RevRxNo130809-001 Modify Start
	bOverStructSize = FALSE;	// 構造体サイズが超えているかを示すフラグの初期化

	// RevRxNo120910-001 Append Start
	if (dwSizeOfProgStruct < sizeof(FFWRX_PROG_DATA)) {
		return FFWERR_FFW_ARG;
	}
	// RevRxNo120910-001 Append End
	if (dwSizeOfProgStruct > sizeof(FFWRX_PROG_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}
	// RevRxNo130809-001 Modify End

	ferr = DO_GetRXPROG(&progDataTmp);	// RevRxNo130809-001 Modify Line

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		// RevRxNo130809-001 Append Start
		// FFW I/Fで設定された変数に一時変数の内容を格納
		if (dwSizeOfProgStruct < sizeof(FFWRX_PROG_DATA)) {
			dwSetSize = dwSizeOfProgStruct;
		} else {
			dwSetSize = sizeof(FFWRX_PROG_DATA);
		}
		memcpy((void*)pProg, (const void*)&progDataTmp, dwSetSize);
		// RevRxNo130809-001 Append End

		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	// RevRxNo120910-001 Append Start
	// "プログラム実行中の各種情報を格納するFFWRX_PROG_DATA構造体サイズズ"の引数エラーチェック（越える場合）について
	// dwSizeOfProgStruct が構造体FFWRX_PROG_DATAのサイズを越える場合、
	// 通常動作をして、かつ以下のエラー(Warning)を返す。
	//     FFWERR_FFWPARA_OVER：FFWに設定可能なパラメータサイズを超えていた(Warning)
	if(ferr == FFWERR_OK){
		if (bOverStructSize == TRUE) {	// RevRxNo130809-001 Modify Line
			return FFWERR_FFWPARA_OVER;		// FFWに設定可能なパラメータサイズを超えていた(Warning)
		}
	}
	// RevRxNo120910-001 Append End

	return ferr;
}

//=============================================================================
/**
 * プログラム実行停止
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_STOP(void)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	if (IsMcuRun() == FALSE) {	// プログラム停止中エラー確認
		return FFWERR_BMCU_STOP;
	}

	ferr = DO_STOP();

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * ブレーク要因の参照
 * @param dwBreakFact     ブレーク要因格納アドレス
 * @param eBrkTrcComb     イベントブレークの組合せ条件
 * @param byOrBrkFactEvPC PC通過イベント情報(イベントブレークでブレーク時)
 * @param byOrBrkFactEvOA オペランドアクセスイベント情報(イベントブレークでブレーク時)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetBRKF(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb,
								   BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
		return FFWERR_BMCU_RUN;
	}

	ferr = DO_GetRXBRKF(dwBreakFact, eBrkTrcComb, byOrBrkFactEvPC, byOrBrkFactEvOA);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * プログラムのインストラクションステップ実行
 * @param pStep ステップ実行時のブレーク要因格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_STEP(FFWE20_STEP_DATA* pStep)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_STEPCMD	eStepCmd;
	FFWMCU_REG_DATA_RX*	pRegDataRX;		// RevRxNo140617-001 Append Line	

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo120621-002 Modify Start
	// RevRxNo140515-008 Modify Line
	if (GetMcuRunState() == TRUE) {	// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}
	// RevRxNo120621-002 Modify End

	eStepCmd = STEPCMD_STEP;
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// 内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo140617-001 Append Start
	if (GetPmodeInRomDisFlg() == FALSE) {	// 内蔵ROM無効拡張モードでない場合
		if (GetTMEnable() == TRUE) {		// TM機能有効の場合
			pRegDataRX = GetRegData_RX();					// レジスタ情報取得
			if (ChkTmArea(pRegDataRX->dwRegData[REG_NUM_RX_PC]) == TRUE) {		// 現在のPC値がTM領域内の場合
				return FFWERR_STEPADDR_TMAREA;
			}
		}
	}
	// RevRxNo140617-001 Append End

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_STEP(pStep, eStepCmd);

	// ユーザプログラムを実行するFFW I/F発行時、ユーザプログラム実行処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグを、TRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * プログラムの関数単位ステップ実行
 * @param pStepOver ステップ実行時のブレーク要因格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_STEPOVER(FFWE20_STEP_DATA* pStepOver)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_STEPCMD	eStepCmd;
	FFWMCU_REG_DATA_RX*	pRegDataRX;		// RevRxNo140617-001 Append Line	

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo120621-002 Modify Start
	// RevRxNo140515-008 Modify Line
	if (GetMcuRunState() == TRUE) {	// プログラム実行中エラー確認
		return FFWERR_BMCU_RUN;
	}
	// RevRxNo120621-002 Modify End

	eStepCmd = STEPCMD_STEPOVER;
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// 内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo140617-001 Append Start
	if (GetPmodeInRomDisFlg() == FALSE) {	// 内蔵ROM無効拡張モードでない場合
		if (GetTMEnable() == TRUE) {		// TM機能有効の場合
			pRegDataRX = GetRegData_RX();					// レジスタ情報取得
			if (ChkTmArea(pRegDataRX->dwRegData[REG_NUM_RX_PC]) == TRUE) {		// 現在のPC値がTM領域内の場合
				return FFWERR_STEPADDR_TMAREA;
			}
		}
	}
	// RevRxNo140617-001 Append End

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_STEP(pStepOver, eStepCmd);

	// ユーザプログラムを実行するFFW I/F発行時、ユーザプログラム実行処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグを、TRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}
// 2008.11.18 MODIFY_END_E20RX600 }

// RevRxNo140515-007 Modify Start
// RevRxNo120910-001 Append Start
//=============================================================================
/**
 * 連続ステップ処理の開始通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_SetSSST(void)
{
	FFWERR	ferr = FFWERR_OK;

	if (GetMcuRunState() == TRUE) {		// プログラム実行中の場合、エラー終了
		return FFWERR_BMCU_RUN;
	}

	if (s_bSsstState == TRUE) {		// SSSTコマンド発行状態の場合
		ferr = DO_SetSSEN();		// 連続ステップ処理の終了処理を実施

	} else {						// 連続ステップ処理の開始通知未発行の場合
		s_bSsstState = TRUE;		// SSSTコマンド発行状態に設定
		ferr = DO_SetSSST();		// 連続ステップ処理の開始処理を実施
	}

	return ferr;
}
// RevRxNo140515-007 Modify End

// RevRxNo140515-007 Modify Start
//=============================================================================
/**
 * 連続ステップ処理の終了通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_SetSSEN(void)
{
	FFWERR	ferr = FFWERR_OK;

	if (GetMcuRunState() == TRUE) {		// プログラム実行中の場合、エラー終了
		return FFWERR_BMCU_RUN;
	}

	if (s_bSsstState == TRUE) {		// SSSTコマンド発行状態の場合
		ferr = DO_SetSSEN();		// 連続ステップ処理の終了処理を実施
	}

	return ferr;
}
// RevRxNo120910-001 Append End
// RevRxNo140515-007 Modify End

// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ発行状態フラグ設定
 * @param	なし
 * @retval	なし
 */
//=============================================================================
void SetSsstState(void)
{
	s_bSsstState = TRUE;		// SSSTコマンド発行状態
}
// RevRxNo140515-007 Append End

// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ発行状態フラグクリア
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrSsstState(void)
{
	s_bSsstState = FALSE;		// SSSTコマンド未発行状態
}
// RevRxNo140515-007 Append End

// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ発行状態フラグ値取得
 * @param なし
 * @retval TRUE :SSSTコマンド発行状態
 * @retval FALSE:SSSTコマンド未発行状態
 */
//=============================================================================
BOOL GetSsstState(void)
{
	return s_bSsstState;
}
// RevRxNo140515-007 Append End

//=============================================================================
/**
 * プログラム実行関連コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Prog(void)
{
	s_bSsstState = FALSE;		// SSSTコマンド未発行状態	// RevRxNo140515-007 Append Line

	return;
}


