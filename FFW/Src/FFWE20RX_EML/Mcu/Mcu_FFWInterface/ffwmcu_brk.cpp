////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_brk.cpp
 * @brief ブレーク関連コマンドのソースファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120606-002 2012/7/12 橋口
  BMの実行前、イベントブレーク禁止中の実行中のイベントブレーク設定不具合対応
・RevNo120720-001	2012/07/27 橋口
　S/Wブレークポイント設定点数エラーチェック処理改善
・RevNo120720-002	2012/07/27 橋口
　ユーザプログラム実行中のS/Wブレークポイント解除処理修正
・RevRxE2No171004-001 2017/10/04 PA 辻
  E2拡張機能対応
*/

#include "ffwmcu_brk.h"
#include "domcu_brk.h"
#include "domcu_runset.h"
#include "domcu_prog.h"
#include "errchk.h"

#include "ffwmcu_mcu.h"

// RevRxE2No171004-001 Append Start
#include "do_sys.h"
#include "doasp_sys.h"
#include "doasprx_monev.h"
// RevRxE2No171004-001 Append End

// FFW 内部変数
static FFW_PB_DATA s_PbState;				// FFW S/Wブレーク管理変数
static FFW_PBCLR_DATA s_PbClrState;			// FFW S/Wブレークポイント埋め戻し管理変数
static BYTE s_UsrCode[PB_SETNUM_MAX];		// S/Wブレーク設定前のユーザ命令コード退避用変数
static DWORD s_BMode;						// FFW ブレークモード管理変数
static DWORD s_BFactor;						// FFW ブレーク要因管理変数

// プログラム実行中の連続コマンド対応用一時管理変数
static DWORD s_TmpBMode;					// FFW ブレークモード管理変数
static DWORD s_TmpBFactor;					// FFW ブレーク要因管理変数



//==============================================================================
/**
 * ソフトウェアブレークポイントの設定と解除を行う。
 * @param eSw 設定／解除の指定
 * @param dwmadrAddr ソフトウェアブレークを設定/解除するアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_SetPB(enum FFWENM_PB_SW eSw, DWORD dwmadrAddr)
{

	FFW_PB_DATA*			pPb;			// S/Wブレークポイント設定情報領域のポインタ
	FFWMCU_MCUDEF_DATA* 	pMcuDef;		// MCU固有情報管理領域のポインタ
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR					ferr = FFWERR_OK;
	MADDR					madr;			// ブレークを設定／解除するアドレス
	DWORD					i;
	MADDR					dwmadrTmp1;		// ブレークポイントアドレスのソート用変数
	MADDR					dwmadrTmp2;		// ブレークポイントアドレスのソート用変数
	BYTE					byTmp1;			// ユーザコードのソート用変数
	BYTE					byTmp2;			// ユーザコードのソート用変数
	BOOL					bRet;
	BOOL					bFlg;			// 解除対象のブレークポイントアドレス検知フラグ
	BYTE					byUsrCode = 0;	// 退避／復帰するユーザ命令コード
	BYTE*					pbyUsrCode;
	enum FFWENM_PB_FILLSTATE eFillState;
	enum FFWENM_PB_FILLSTATE eFillStateTmp1;
	enum FFWENM_PB_FILLSTATE eFillStateTmp2;
	FFWMCU_DBG_DATA_RX*		pDbgData;
	BOOL					bClrPb;	// RevNo120720-002 Append Line
#if defined(E2E2L)
	FFWE20_EINF_DATA		einfData;		// RevRxE2No171004-001 Append Line
#endif

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

#if defined(E2E2L)
	getEinfData(&einfData);		// RevRxE2No171004-001 Append Line
#endif

	pDbgData = GetDbgDataRX();	// デバッグ情報取得
	pPb = GetPbData();			// S/Wブレークポイント設定を取得
	switch (eSw) {
	case PB_SET:		// S/Wブレークポイントを設定

		// RevNo120720-001 Modify Start
		// 最大点数設定状態でも、設定済みアドレスへのPB SET時はエラー(FFWERR_PBPOINT_OVER)にしないよう、
		// 設定済みアドレスの確認を設定済み点数チェックより先に実施する。
		bRet = SearchBrkPoint(dwmadrAddr, pPb);
		if (bRet) {		// 対象アドレスに既にブレークが設定されている
			// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
			if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
				if (IsAspOn()) {	// E2拡張機能が有効
					bRet = SearchMonEvNo((UINT64)dwmadrAddr, RF_ASPMONEV_TYPE_SWBRK);
					if (bRet != NOSET) {		// 対象アドレスに通過ポイントとしてS/Wブレークが設定されている場合は設定しない
						return FFWERR_PBADDR_OVERLAPP;	// 通過ポイントが設定されているアドレスへS/Wブレークポイントを設定することはできません
					}
				}
			}
#endif
			// RevRxE2No171004-001 Append End

			return FFWERR_OK;
		}
		// RevNo120720-001 Modify End

		if (pPb->dwSetNum >= PB_SETNUM_MAX) {		// 256点以上のS/Wブレークポイントが設定されている
			return FFWERR_PBPOINT_OVER;
		}

		break;
	case PB_CLR:		// S/Wブレークポイントを解除
		if (pPb->dwSetNum == 0) {		// S/Wブレークポイントが設定されていない
			return FFWERR_PBADDR_NOTSET;
		}

		bRet = SearchBrkPoint(dwmadrAddr, pPb);
		if (!bRet) {	// 対象アドレスにブレークが設定されていない
			return FFWERR_PBADDR_NOTSET;
		}

		// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			if (IsAspOn()) {	// E2拡張機能が有効
				bRet = SearchMonEvNo((UINT64)dwmadrAddr, RF_ASPMONEV_TYPE_SWBRK);
				if (bRet) {		// 対象アドレスに通過ポイントとしてS/Wブレークが設定されている場合は解除しない
					return FFWERR_PBADDR_NOTSET;
				}
			}
		}
#endif
		// RevRxE2No171004-001 Append End

		break;
	default:
		break;
	}

	madr = dwmadrAddr;
	pMcuDef = GetMcuDefData();
	if (madr > pMcuDef->madrMaxAddr) {
					// 指定アドレスが参照するメモリ空間の領域であるかをチェック
		return FFWERR_PBAREA_OUT;
	}

	// メモリアクセスが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ErrorChk_Pb(eSw, dwmadrAddr);	// MCUに依存するエラーチェック
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// V.1.02 ROM無効状態かどうかの判断処理削除
	// プログラム実行中にS/Wブレークを設定した場合、以下のようになる。
	// (1) 内蔵ROMへの設定
	//      wait命令をSTEP中でない場合
	//          ・GB/RSTGでRUN中の場合→FFWERR_PB_ROMAREA_MCURUN
	//          ・レジスタ設定の起動モードがROM無効モードの場合→FFWERR_PBAREA_OUT
	//          ・GO/GPB/GPBA/STEP/STEPOVERの場合→貼れたふりして次回実行時に反映される
	//      wait命令をStep中の場合
	//          ・DO_SetPB中の命令退避処理でSleep状態エラーが返る
	// (2) 内蔵RAMへの設定
	//      wait命令をSTEP中でない場合
	//          ・S/Wブレーク貼れる
	//      wait命令をSTEP中の場合
	//　　　    ・DO_SetPB()中のPROT_MCU_FILLでSleep状態エラーが返る
	// 結論：wait命令をStep中はどこにもS/Wブレークは貼れない。
	
	pbyUsrCode = &byUsrCode;
	ferr = DO_SetPB(eSw, dwmadrAddr, pbyUsrCode, &eFillState, &bClrPb);		// ブレークポイントを設定	// RevNo120720-002 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// FFW内の管理変数を更新
	switch (eSw) {
	case PB_SET:		// S/Wブレークポイントを設定
		if (s_PbState.dwSetNum == 0) {
			s_PbState.dwmadrAddr[0] = dwmadrAddr;
			s_PbState.byCmdData[0] = byUsrCode;
			s_PbState.eFillState[0] = eFillState;
		} else {
			dwmadrTmp1 = dwmadrAddr;
			byTmp1 = byUsrCode;
			eFillStateTmp1 = eFillState;
			for (i = 0; i < s_PbState.dwSetNum; ++i) {	// ブレークポイントをソートして設定
				dwmadrTmp2 = s_PbState.dwmadrAddr[i];
				byTmp2 = s_PbState.byCmdData[i];
				eFillStateTmp2 = s_PbState.eFillState[i];
				if (dwmadrTmp2 > dwmadrTmp1) {
					s_PbState.dwmadrAddr[i] = dwmadrTmp1;
					dwmadrTmp1 = dwmadrTmp2;
					s_PbState.byCmdData[i] = byTmp1;
					byTmp1 = byTmp2;
					s_PbState.eFillState[i] = eFillStateTmp1;
					eFillStateTmp1 = eFillStateTmp2;
				}
			}
			s_PbState.dwmadrAddr[i] = dwmadrTmp1;
			s_PbState.byCmdData[i] = byTmp1;
			s_PbState.eFillState[i] = eFillStateTmp1;
		}
		++s_PbState.dwSetNum;		// ブレークポイントの数を１増やす
		break;

	case PB_CLR:		// S/Wブレークポイントを解除
		bFlg = FALSE;
		for (i = 0; i < s_PbState.dwSetNum; ++i) {
			if (bFlg) {
				s_PbState.dwmadrAddr[i - 1] = s_PbState.dwmadrAddr[i];
				s_PbState.byCmdData[i - 1] = s_PbState.byCmdData[i];
				s_PbState.eFillState[i - 1] = s_PbState.eFillState[i];
			}
			if ((s_PbState.dwmadrAddr[i] == dwmadrAddr) && (s_PbState.eFillState[i] != PB_FILL_NON)) {	// 解除対象のアドレスを検知
				// 解除対象のアドレスがDBT命令を埋め込み済みの場合
				// RevNo120720-002 Modify Line: DO_SetPB()内でプログラム実行中にRAMアドレスを埋め戻した場合(bClrPb==TRUE)は、setPbClrTbl()を実行しない。
				if ((bClrPb == FALSE) && (s_PbState.eFillState[i] == PB_FILL_COMPLETE)) {
					SetPbClrTbl(s_PbState.dwmadrAddr[i], s_PbState.byCmdData[i]);					// 埋め戻し対象テーブルに登録
				}
				bFlg = TRUE;
			}
		}
		// 上記ループで設定項目を繰上げたので一番最後を初期状態に戻す
		s_PbState.dwmadrAddr[i-1] = 0x00000000;
		s_PbState.byCmdData[i-1] = 0x00;
		s_PbState.eFillState[i-1] = PB_FILL_NON;

		--s_PbState.dwSetNum;		// ブレークポイントの数を１減らす
		break;

	default:
		break;

	}

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End

}


//==============================================================================
/**
 * ソフトウェアブレークポイントの設定アドレスを取得する。
 * @param pPbAddr 現在設定されているS/Wブレークポイントを格納するFFWCmd_PB_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_GetPB(FFW_PB_DATA *pPbAddr)
{

	memcpy(pPbAddr, &s_PbState, sizeof(FFW_PB_DATA));

	return FFWERR_OK;

}


//==============================================================================
/**
 * ソフトウェアブレークポイントの全解除を行う。
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_ClrPB(void)
{

	FFWERR			ferr = FFWERR_OK;
	FFW_PB_DATA*	pPb;			// S/Wブレークポイント設定情報領域のポインタ

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pPb = GetPbData();				// S/Wブレークポイント設定を取得
	if (pPb->dwSetNum == 0) {
		return FFWERR_PBADDR_NOTSET;
	}

	ferr = ErrorChk_Pbcl();		// MCUに依存するエラーチェック
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = DO_ClrPB();			// S/Wブレーク設定を解除

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;

}


//==============================================================================
/**
 * ブレークモードを設定する。
 * @param dwBMode ブレークモード
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_SetBM(DWORD dwBMode)
{

	FFWERR	ferr = FFWERR_OK;					// RevNo120606-002 Append Line
	int		eCmdNum;							// コマンド番号

	// RevNo120606-002 Delete
	// ホットプラグ起動時のs_BModeへの反映処理は、実行中は常に実施。DO_SetBM()内へ移動

	// 引数チェック（ブレークモード）
	if (dwBMode > (BMODE_SWB_BIT | BMODE_EB_BIT | BMODE_PRE_BIT)) {
		return FFWERR_FFW_ARG;					// ブレークモードの指定が範囲外
	}
	// FFW 内部変数に値を設定
	s_TmpBMode = dwBMode;
	
	// コマンド呼び出しフラグを設定
	if (s_BMode != s_TmpBMode) {
		// イベントブレークの設定条件を変更する場合
		eCmdNum = RUNSET_CMDNUM_BM;
		SetEvCmdExeFlg(eCmdNum);
	}

	
	// RevNo120606-002 Modify Start
	// BM変更処理
	ferr = DO_SetBM(dwBMode);

	return ferr;
	// RevNo120606-002 Modify End

}


//==============================================================================
/**
 * ブレークモードを参照する。
 * @param dwBMode ブレークモードを格納する。
 * @retval FFWERR_OK 正常終了
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_GetBM(DWORD* pdwBMode)
{

	*pdwBMode = s_TmpBMode;

	return FFWERR_OK;

}


//==============================================================================
/**
 * FFW S/Wブレークポイント管理変数に設定されている値を更新する。
 * @param  madrStartAddr 更新対象アドレス
 * @param  bySet 埋め込み(1) or 復帰(0)
 * @retval なし
 */
//==============================================================================
void UpdatePbData(MADDR madrStartAddr, BYTE bySet)
{
	DWORD i;

	for (i = 0; i < s_PbState.dwSetNum; i++) {
		if (madrStartAddr == s_PbState.dwmadrAddr[i]) {
			if (bySet == 0) {
				if (s_PbState.eFillState[i] == PB_FILL_COMPLETE) {		// 埋め込み済みの場合、埋め込み設定待ちに更新
					s_PbState.eFillState[i] = PB_FILL_SET;
				}
			} else {
				if (s_PbState.eFillState[i] == PB_FILL_SET) {			// 埋め込み設定待ちの場合、埋め込み済みに更新
					s_PbState.eFillState[i] = PB_FILL_COMPLETE;
				}
			}
			break;
		}
	}

	return;
}

//==============================================================================
/**
 * FFW S/Wブレークポイント管理変数に設定されている値を取得する。
 * @param なし
 * @retval FFW S/Wブレークポイント管理変数のポインタ
 */
//==============================================================================
FFW_PB_DATA* GetPbData(void)
{

	FFW_PB_DATA*	pRet;

	pRet = &s_PbState;

	return pRet;

}

//==============================================================================
/**
 * FFW S/Wブレークポイント埋め戻し管理変数に設定されている値を取得する。
 * @param なし
 * @retval FFW S/Wブレークポイント埋め戻し管理変数のポインタ
 */
//==============================================================================
FFW_PBCLR_DATA* GetPbClrData(void)
{

	FFW_PBCLR_DATA*	pRet;

	pRet = &s_PbClrState;

	return pRet;

}

//==============================================================================
/**
 * FFW S/Wブレーク設定前のユーザ命令コード退避用変数のポインタを取得する。
 * @param なし
 * @retval FFW S/Wブレークポイント管理変数のポインタ
 */
//==============================================================================
BYTE* GetUsrCodeData(void)
{

	BYTE*	pRet;

	pRet = &s_UsrCode[0];

	return pRet;

}


//==============================================================================
/**
 * FFW ブレークモード／ブレーク要因管理変数に設定されている値を取得する。
 * @param pdwBMode 設定されているブレークモードを格納するアドレス
 * @param pdwBFactor 設定されているブレーク要因を格納するアドレス
 * @retval なし
 */
//==============================================================================
void GetBmMode(DWORD* pdwBMode, DWORD* pdwBFactor)
{

	*pdwBMode = s_BMode;
	*pdwBFactor = s_BFactor;

	return;

}


//==============================================================================
/**
 * FFWCmd_SetBM コマンドで使用するFFW 内部変数を設定する。
 * @param なし
 * @retval なし
 */
//==============================================================================
void SetBmData(void)
{

	s_BMode = s_TmpBMode;
	if (s_BMode & BMODE_EB_BIT) {
				// イベントブレークを有効にする場合
		s_BFactor = s_TmpBFactor;
	}

	return;

}

//==============================================================================
/**
 * 埋め戻し対象のS/Wブレークポイントをテーブルに登録する。
 * @param  dwmadrAddr 埋め戻し対象アドレス
 * @param  byCmdData  埋め戻す命令コード
 * @retval なし
 */
//==============================================================================
void SetPbClrTbl(DWORD dwmadrAddr, BYTE byCmdData)
{
	DWORD	dwIndex;

	dwIndex = s_PbClrState.dwSetNum;
	s_PbClrState.dwmadrAddr[dwIndex] = dwmadrAddr;
	s_PbClrState.byCmdData[dwIndex] = byCmdData;
	++s_PbClrState.dwSetNum;	// ブレークポイントの数を１増やす

	return;
}

//==============================================================================
/**
 * 埋め戻し対象のS/Wブレークポイントをテーブルからクリアする。
 * (※プログラム実行時にまとめて埋め戻す為、登録内容を全てクリアする)
 * @param  なし
 * @retval なし
 */
//==============================================================================
void ClrPbClrTbl(void)
{
	DWORD	i;

	for (i = 0; i < s_PbClrState.dwSetNum; i++) {
		s_PbClrState.dwmadrAddr[i] = 0x00000000;
		s_PbClrState.byCmdData[i] = 0x00;
	}

	s_PbClrState.dwSetNum = 0;

	return;
}
//=============================================================================
/**
 * ブレーク関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Brk(void)
{

	int		i;

	/* S/W ブレークポイントの数を初期化 */
	s_PbState.dwSetNum = INIT_PB_NUM;
	s_PbClrState.dwSetNum = INIT_PB_NUM;

	for (i = 0; i < PB_SETNUM_MAX; i++) {
		s_PbState.byCmdData[i] = 0x00;
		s_PbState.dwmadrAddr[i] = 0x00000000;
		s_PbState.eFillState[i] = PB_FILL_NON;
		s_PbClrState.byCmdData[i] = 0x00;
		s_PbClrState.dwmadrAddr[i] = 0x00000000;
	}

	/* S/W ブレーク設定前のユーザ命令コード退避領域を初期化 */
	for (i = 0; i < PB_SETNUM_MAX; ++i) {
		s_UsrCode[i] = 0x00000000;
	}

	/* ブレークモードを初期化 */
	s_BMode = INIT_BM_MODE;
	s_TmpBMode = INIT_BM_MODE;

	/* ブレークモードを初期化 */
	s_BFactor = INIT_BM_FACTOR;
	s_TmpBFactor = INIT_BM_FACTOR;

	// FFW ブレークモード管理変数
	s_TmpBMode = INIT_BM_MODE;
	// FFW ブレーク要因管理変数
	s_TmpBFactor = INIT_BM_MODE;

	return;

}