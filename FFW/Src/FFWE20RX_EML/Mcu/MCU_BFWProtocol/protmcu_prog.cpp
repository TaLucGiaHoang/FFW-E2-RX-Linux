///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_prog.cpp
 * @brief BFWコマンド プロトコル生成関数(プログラム実行関連コマンド)
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, K.Uemori
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/06/18
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo140515-007 2014/06/18 植盛
	SSST, SSENコマンド追加による速度改善
*/
#include "protmcu_prog.h"
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "ffwmcu_prog.h"

#include "ffwmcu_mcu.h"
#include "do_sys.h"

//=============================================================================
/**
 * BFWMCUCmd_GOコマンドの発行
 * @param bMode	プログラム実行の動作モード
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxE2No171004-001 Modify Line
FFWERR PROT_MCU_GO(BYTE bMode)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;
	FFWE20_EINF_DATA	einfData;		// RevRxE2No171004-001 Append Line

	getEinfData(&einfData);

	wCmdCode = BFWCMD_GO;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxE2No171004-001 Append Start
	if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
		if (PutData1(bMode) != TRUE) {		// プログラム実行の動作モード設定
			return FFWERR_COM;
		}
	}
	// RevRxE2No171004-001 Append End

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}
//=============================================================================
/**
 * BFWRXCmd_GetPROGコマンドの発行
 * @param pProg     ユーザプログラム実行中の各種情報格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetRXPROG(FFWRX_PROG_DATA* pProg)
{
	FFWERR	ferr;
	WORD	wBuf;
	DWORD	dwBuf;
	WORD	wCmdCode;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	BOOL	bRet;

	bRet = getFirstConnectInfo();			// 初回起動時に接続した状態を取得
	if (bRet == FALSE) {
		return FFWERR_BMCU_NONCONNECT;		// MCUとの接続が未完了
	}

	pDbgData = GetDbgDataRX();

	wCmdCode = BFWCMD_GET_PROG;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData4(&dwBuf) != TRUE) {			// ターゲットプログラム実行状態情報受信
		return FFWERR_COM;
	}
	pProg->dwExecInfo = dwBuf;

	if (GetData4(&dwBuf) != TRUE) {			// プログラムカウンタ情報受信
		return FFWERR_COM;
	}
	pProg->dwPcInfo = dwBuf;

	if (GetData4(&dwBuf) != TRUE) {			// プロセッサステータスワード情報受信
		return FFWERR_COM;
	}
	pProg->dwPswInfo = dwBuf;

	if (GetData2(&wBuf) != TRUE) {			// C2Eバッファ情報受信
		return FFWERR_COM;
	}
	pProg->wC2eBuffSize = wBuf;

	if (GetData2(&wBuf) != TRUE) {			// E2Cバッファ情報受信
		return FFWERR_COM;
	}
	pProg->wE2cBuffSize = wBuf;

	if (GetData4(&dwBuf) != TRUE) {			// 全実行時間計測カウンタ情報受信
		return FFWERR_COM;
	}
	pProg->dwEmlMesInfo = dwBuf;

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWMCUCmd_STOPコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_STOP(void)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_STOP;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
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
 * BFWRXCmd_GetBRKFコマンドの発行
 * @param pBrkFact        ブレーク要因格納アドレス
 * @param eBrkTrcComb     イベントブレークの組合せ条件
 * @param byOrBrkFactEvPC PC通過イベント情報(イベントブレークでブレーク時)
 * @param byOrBrkFactEvOA オペランドアクセスイベント情報(イベントブレークでブレーク時)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_GetRXBRKF(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb, 
						  BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA)
{
	FFWERR	ferr;
	WORD	wBuf;
	DWORD	dwBuf;
	WORD	wCmdCode;

	BYTE	byBuf;

	wCmdCode = BFWCMD_GET_BRKF;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData4(&dwBuf) != TRUE) {			// ブレーク要因受信
		return FFWERR_COM;
	}

	*dwBreakFact = dwBuf;

	if (GetData1(&byBuf) != TRUE) {			// イベントブレークの組合せ条件受信
		return FFWERR_COM;
	}

	switch (byBuf) {
	case 0:
		*eBrkTrcComb = COMB_OR;
		break;
	case 1:
		*eBrkTrcComb = COMB_BRK_AND;
		break;
	case 2:
		*eBrkTrcComb = COMB_BRK_STATE;
		break;
	default:
		*eBrkTrcComb = COMB_OR;
		break;
	}

	if (GetData1(&byBuf) != TRUE) {			// PC通過イベント情報受信
		return FFWERR_COM;
	}

	*byOrBrkFactEvPC = byBuf;

	if (GetData1(&byBuf) != TRUE) {			// オペランドアクセスイベント情報受信
		return FFWERR_COM;
	}

	*byOrBrkFactEvOA = byBuf;

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWMCUCmd_STEP / BFWMCUCmd_STEPOVERコマンドの発行
 * @param pStep ステップ実行時のブレーク要因格納構造体アドレス
 * @param pbFfwStepReq FFWによるステップ再実行要求フラグ格納アドレス
 * @param pwOpecode 命令コード2バイト格納バッファアドレス
 * @param pbTgtReset RUN中のターゲットリセット発生状態格納アドレス
 * @param eStepCtrl シングルステップ制御(STEP_BFWCTRL/STEP_FFWCTRL)
 * @param eStepCmd ステップ実行コマンド
 * @param byEndian        実行アドレスのエンディアン
 * @param byMskSet        ステップ実行時のリセットマスク実施
 * @retval FFWエラーコード
 */
//=============================================================================
//RevNo100715-013 Modify Line
FFWERR PROT_MCU_STEP(FFWE20_STEP_DATA* pStep, BOOL* pbFfwStepReq, WORD* pwOpecode, BOOL* pbTgtReset, enum FFWENM_STEPCTRL eStepCtrl,
					 enum FFWENM_STEPCMD eStepCmd, BYTE byEndian, BYTE byMskSet)
{
	FFWERR	ferr;
	WORD	wBuf;
	DWORD	dwBuf;
	WORD	wData;
	WORD	wCmdCode;
	//ワーニング対策
	pbTgtReset;
	pbFfwStepReq;

	if (eStepCmd == STEPCMD_STEP) {	// STEPコマンド実行時
		wCmdCode = BFWCMD_STEP;
	} else {
		wCmdCode = BFWCMD_STEPOVER;	// STEPOVERコマンド実行時
	}
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wData = static_cast<WORD>(eStepCtrl);
	if (PutData2(wData) != TRUE) {	// ステップ実行制御種別送信
		return FFWERR_COM;
	}

	//RevNo100715-013 Append Start
	if (PutData1(byEndian) != TRUE) {	// 実行PC位置のエンディアン情報
		return FFWERR_COM;
	}

	if (PutData1(byMskSet) != TRUE) {	// リセット/NMI/割り込みマスク設定
		return FFWERR_COM;
	}
	//RevNo100715-013 Append End

	//WAIT命令をステップ実行すると、RUNSTATE移行するため
	ferr = ProtGetStatus(wCmdCode);					// ステータス受信
	if (ferr == FFWERR_OK) {						// 実行停止中
		pStep->eGoToRunState = STEP_NOT_GOTO_RUNSTATE;
	} else if (ferr == FFWERR_COMDATA_ERRCODE) {	// 実行状態
		pStep->eGoToRunState = STEP_GOTO_RUNSTATE;
		return FFWERR_OK;
	} else {
		return ferr;
	}

	if (GetData4(&dwBuf) != TRUE) {	// ブレーク要因受信
		return FFWERR_COM;
	}
	//	BFWパラメータ							FFWパラメータ
	//  
	//	dwBuf	b20: RUN中のリセット発生状態	->	*pbTgtReset (FFW内部処理変数)
	//			b19: サブルーチンコール命令		->	pStep->eSubCallCodeStepFlag: サブルーチンコール命令
	//			b18: RUN状態移行フラグ			->	pStep->eGoToRunState: RUN状態移行フラグ
	//			b17: リターンサブルーチン命令	->	pStep->eReturnCodeStepFlag: リターンサブルーチン命令
	//			b16: ステップ再実行要求			->	*pbFfwStepReq (FFW内部処理変数)
	//			b15: システムリセット入力		->	
	//			b14: ユーザ割り込み				->	
	//			b0-8:例外検出結果				->	pStep->dwExFact	b0-8:例外検出結果

	pStep->eSubCallCodeStepFlag = STEP_NONSUBCALLCODE;
	pStep->eGoToRunState = STEP_NOT_GOTO_RUNSTATE;
	pStep->eRetunCodeStepFlag = STEP_NONRTSCODE;

	if (GetData4(&dwBuf) != TRUE) {	// PC受信
		return FFWERR_COM;
	}
	pStep->dwPc = dwBuf;

	if (GetData2(&wBuf) != TRUE) {	// 命令コード受信
		return FFWERR_COM;
	}
	*pwOpecode = wBuf;

	if (GetData2(&wBuf) != TRUE) {	// サブルーチンコール命令コード長受信
		return FFWERR_COM;
	}
	pStep->dwSubCallCodeSize = static_cast<DWORD>(wBuf);

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWMCUCmd_GPBAコマンドの発行
 * @param dwmadrBreakAddr ブレークアドレス
 * @param pStep           ステップ実行時のブレーク要因格納構造体アドレス
 * @param pwOpecode       命令コード2バイト格納バッファアドレス
 * @param byEndian        実行アドレスのエンディアン
 * @retval FFWエラーコード
 */
//=============================================================================
//RevNo100715-014 Modify Line
FFWERR PROT_MCU_GPBA(DWORD dwmadrBreakAddr, FFWE20_STEP_DATA* pStep, WORD* pwOpecode,BYTE byEndian)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;
	DWORD	dwBuf;

	wCmdCode = BFWCMD_GPBA;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData4(dwmadrBreakAddr) != TRUE) {	// ブレークアドレス送信
		return FFWERR_COM;
	}

	//RevNo100715-014 Append Start
	if (PutData1(byEndian) != TRUE) {	// 実行PC位置のエンディアン情報
		return FFWERR_COM;
	}
	//RevNo100715-014 Append End
	
	ferr = ProtGetStatus(wCmdCode);					// ステータス受信
	if (ferr == FFWERR_OK) {						// 実行停止中
		pStep->eGoToRunState = STEP_NOT_GOTO_RUNSTATE;
	} else if (ferr == FFWERR_COMDATA_ERRCODE) {	// 実行状態
		pStep->eGoToRunState = STEP_GOTO_RUNSTATE;
		return FFWERR_OK;
	} else {
		return ferr;
	}
	
	if (GetData4(&dwBuf) != TRUE) {				// ブレーク要因受信
		return FFWERR_COM;
	}

	if (GetData4(&dwBuf) != TRUE) {	// PC受信
		return FFWERR_COM;
	}
	pStep->dwPc = dwBuf;

	if (GetData2(&wBuf) != TRUE) {	// 命令コード受信
		return FFWERR_COM;
	}
	*pwOpecode = wBuf;

	if (GetData2(&wBuf) != TRUE) {	// サブルーチンコール命令コード長受信
		return FFWERR_COM;
	}
	pStep->dwSubCallCodeSize = static_cast<DWORD>(wBuf);

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

// RevRxNo140515-007 Append Start
//==============================================================================
/**
 * PROT_MCU_SetSSSTコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_MCU_SetSSST(void)
{

	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_SET_SSST;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	/* 処理中断コードを受信 */
	pwBuf = &wData;
	ferr = ProtRcvHaltCode(pwBuf);

	return ferr;

}
// RevRxNo140515-007 Append End

// RevRxNo140515-007 Append Start
//==============================================================================
/**
 * PROT_MCU_SetSSENコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR PROT_MCU_SetSSEN(void)
{

	WORD	wCmdCode;
	WORD	wData;
	FFWERR	ferr;
	WORD*	pwBuf;

	wCmdCode = BFWCMD_SET_SSEN;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	/* 処理中断コードを受信 */
	pwBuf = &wData;
	ferr = ProtRcvHaltCode(pwBuf);

	return ferr;

}
// RevRxNo140515-007 Append End

//=============================================================================
/**
 * プログラム実行関連コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Prog(void)
{
	return;
}

