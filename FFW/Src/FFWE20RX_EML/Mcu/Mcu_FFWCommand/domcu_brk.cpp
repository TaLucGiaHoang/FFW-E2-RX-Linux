////////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_brk.cpp
 * @brief ブレーク関連コマンドのソースファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120606-002 2012/07/12 橋口
  BMの実行前、イベントブレーク禁止中の実行中のイベントブレーク設定不具合対応
・RevNo120727-001 2012/07/27 橋口
	内蔵ROM DUMP 高速化
・RevNo120720-002	2012/07/27 橋口
　ユーザプログラム実行中のS/Wブレークポイント解除処理修正
・RevRxE2No171004-001 2017/10/04 PA 辻
  E2拡張機能対応
 */
#include "ffwmcu_brk.h"
#include "domcu_brk.h"
#include "ffwmcu_mcu.h"
#include "protmcu_mem.h"
#include "prot_cpu.h"
#include "domcu_prog.h"
#include "prot_common.h"
#include "domcu_mem.h"
#include "errchk.h"
#include "mcu_brk.h"
#include "domcu_mcu.h"
// RevNo120606-002 Append Start
#include "ffwrx_ev.h"
#include "mcurx_ev.h"
// RevNo120606-002 Append End
// RevRxE2No171004-001 Append Start
#include "do_sys.h"
#include "doasp_sys.h"
#include "doasprx_monev.h"
// RevRxE2No171004-001 Append End
//==============================================================================
/**
 * FFW 内部変数のブレークポイント設定を設定／解除する。
 * @param  eSw         設定・解除の指定
 * @param  dwmadrAddr  ソフトウェアブレークを設定/解除するアドレス
 * @param  pbyUsrCode  退避させるユーザ命令コード
 * @param  peFillState ソフトウェアブレークの設定状態
 * @retval FFWエラーコード
 */
//==============================================================================
// RevNo120720-002 Modify Line
FFWERR DO_SetPB(enum FFWENM_PB_SW eSw, DWORD dwmadrAddr, BYTE* pbyUsrCode, enum FFWENM_PB_FILLSTATE* peFillState, BOOL* pbClrPb)
{

	FFWERR	ferr;
	FFWERR	ferrEnd;
	DWORD	dwBMode;					// 設定中のブレークモード格納バッファ
	DWORD	dwBFactor;					// 設定中のブレーク要因格納バッファ
	DWORD*	pdwBMode;
	DWORD*	pdwBFactor;
	BOOL	bRet;
	enum FFWENM_PROGCMD_NO eProgCmd;	// プログラム実行コマンド種別
	FFW_PB_DATA*	pPbData;
	BYTE*	pbyUsrCodeData;

	pPbData = GetPbData();
	pbyUsrCodeData = GetUsrCodeData();

	ProtInit();

	*pbClrPb = FALSE;	// RevNo120720-002 Append Line

	switch (eSw) {
	case PB_SET:		// S/Wブレークポイントを設定
		// プログラム実行状態をチェック
		// RevNo120727-001 Modify Line
		bRet = GetMcuRunState();
		if (bRet) {
					// プログラム実行中の場合
			/* プログラム実行コマンド種別をチェック */
			eProgCmd = GetMcuRunCmd();
			switch (eProgCmd) {		// プログラム実行コマンド種別
			case PROGCMD_GB:			// GB コマンド
			case PROGCMD_RSTG:			// RSTG コマンド
				pdwBMode = &dwBMode;
				pdwBFactor = &dwBFactor;
				GetBmMode(pdwBMode, pdwBFactor);
				if (dwBMode & BMODE_SWB_BIT) {
							// ブレークモードがS/W ブレーク許可
					/* ユーザ命令コードを退避し、BRK命令コードを書込む */
					ferr = WriteBrkCode(dwmadrAddr, pbyUsrCode);
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					*peFillState = PB_FILL_COMPLETE;
				} else {
					*peFillState = PB_FILL_SET;
				}
				break;

			default:					// 他のコマンド種別
				// 命令コードの退避のみ実施(埋め込みは実行時に実施)
				ferr = EscCmdData(dwmadrAddr, pbyUsrCode);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				*peFillState = PB_FILL_SET;
				break;
			}
		} else {
			// 命令コードの退避のみ実施(埋め込みは実行時に実施)
			ferr = EscCmdData(dwmadrAddr, pbyUsrCode);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			*peFillState = PB_FILL_SET;
		}
		break;

	case PB_CLR:		// S/Wブレークポイントを解除
		// プログラム実行状態をチェック
		// RevNo120727-001 Modify Line
		bRet = GetMcuRunState();
		if (bRet) {
					// プログラム実行中の場合
			/* プログラム実行コマンド種別をチェック */
			eProgCmd = GetMcuRunCmd();
			switch (eProgCmd) {		// プログラム実行コマンド種別
			case PROGCMD_GB:			// GB コマンド
			case PROGCMD_RSTG:			// RSTG コマンド
				pdwBMode = &dwBMode;
				pdwBFactor = &dwBFactor;
				GetBmMode(pdwBMode, pdwBFactor);
				if (dwBMode & BMODE_SWB_BIT) {
					/* ユーザコード復帰処理 */
					// WriteUserCode()内で復帰コードを検索
					ferr = WriteUserCode(dwmadrAddr);
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					*pbClrPb = TRUE;	// DBT命令→ユーザコードに復帰済み(RUN中のRAMへのPB解除時のみTRUEにする)	// RevNo120720-002 Append Line
				}
				break;

			default:					// 他のコマンド種別
				/* ユーザコード復帰処理 */
				// WriteUserCode()内で復帰コードを検索
				ferr = WriteUserCode(dwmadrAddr);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				// RevNo120720-002
				// ここには、PBアドレスにDBT命令埋め込み済みの状態でくることはないため、
				// *pbClrPbをTRUEにする処理は不要。
				break;
			}
		}else{
			/* ユーザコード復帰処理 */
			//WriteUserCode()内で復帰コードを検索
			ferr = WriteUserCode(dwmadrAddr);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			// RevNo120720-002
			// ROMアドレスのPB解除時はフラッシュ内容はDBT命令のままのため、*pbClrPbをTRUEにしない。
		}

		break;

	default:
		break;

	}

	ferrEnd = ProtEnd();

	return ferrEnd;

}


//==============================================================================
/**
 * FFW 内部変数に設定されているブレークポイント設定を解除する。
 * E2の場合は、通過ポイント用のS/Wブレークは解除しない。
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_ClrPB(void)
{
	FFWERR			ferr;
	FFWERR			ferrEnd;
	FFW_PB_DATA*	pPb;			// S/Wブレークポイント設定情報領域のポインタ
	DWORD			i;
	// RevRxE2No171004-001 Append Start
	FFW_PB_DATA		sTmpPb;			// 通過ポイント用S/Wブレークポイント設定情報の保存領域
#if defined(E2E2L)
	FFWE20_EINF_DATA	einfData;
	BOOL			bRet;
#endif
	// RevRxE2No171004-001 Append End

	ProtInit();

	pPb = GetPbData();				// S/Wブレークポイント設定を取得

	// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	getEinfData(&einfData);
#endif
	sTmpPb.dwSetNum = INIT_PB_NUM;
	// RevRxE2No171004-001 Append End

	for (i = 0; i < pPb->dwSetNum; ++i) {
		// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			if (IsAspOn()) {	// E2拡張機能が有効
				bRet = SearchMonEvNo((UINT64)pPb->dwmadrAddr[i], RF_ASPMONEV_TYPE_SWBRK);
				if (bRet) {		// 対象アドレスに通過ポイントとしてS/Wブレークが設定されている場合は解除しないため一旦保存
					sTmpPb.byCmdData[sTmpPb.dwSetNum] = pPb->byCmdData[i];
					sTmpPb.dwmadrAddr[sTmpPb.dwSetNum] = pPb->dwmadrAddr[i];
					sTmpPb.eFillState[sTmpPb.dwSetNum] = pPb->eFillState[i];
					sTmpPb.dwSetNum++;
					pPb->byCmdData[i] = 0x00;
					pPb->dwmadrAddr[i] = 0x00000000;
					pPb->eFillState[i] = PB_FILL_NON;
					continue;
				}
			}
		}
#endif
		// RevRxE2No171004-001 Append End

		if (pPb->eFillState[i] == PB_FILL_COMPLETE) {				// 埋め込み済みの場合
			SetPbClrTbl(pPb->dwmadrAddr[i], pPb->byCmdData[i]);		// 埋め戻し対象テーブルに登録
			ferr = WriteUserCode(pPb->dwmadrAddr[i]);	// 命令コード復帰　WriteUserCode()内で復帰コードを検索
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
		pPb->byCmdData[i] = 0x00;
		pPb->dwmadrAddr[i] = 0x00000000;
		pPb->eFillState[i] = PB_FILL_NON;
	}

	// RevRxE2No171004-001 Modify Start
	if (sTmpPb.dwSetNum != INIT_PB_NUM) {	// S/Wブレークポイント管理変数に通過ポイント用S/Wブレークを再設定
		for (i = 0; i < sTmpPb.dwSetNum; i++) {
			pPb->byCmdData[i] = sTmpPb.byCmdData[i];
			pPb->dwmadrAddr[i] = sTmpPb.dwmadrAddr[i];
			pPb->eFillState[i] = sTmpPb.eFillState[i];
		}
		pPb->dwSetNum = sTmpPb.dwSetNum;			// S/Wブレークポイント数初期化(通過ポイント用S/Wブレークは解除しない)
	} else {
		pPb->dwSetNum = INIT_PB_NUM;				// S/Wブレークポイント数初期化
	}
	// RevRxE2No171004-001 Modify End

	// RevNo120720-002 Append Start
	if (IsMcuRun() == TRUE) {
		// プログラム実行中の場合(RAMアドレスのS/Wブレークポイントクリア時)は、
		// 埋め戻し対象のS/Wブレークポイントをクリアする
		ClrPbClrTbl();
	}
	// RevNo120720-002 Append End

	ferrEnd = ProtEnd();

	return ferrEnd;

}

// RevRxE2No171004-001 Append Start
//==============================================================================
/**
* FFW 内部変数に設定されているブレークポイント設定を解除する。
* E2の場合は、通過ポイント用のS/Wブレークポイントの解除とRFWの通過ポイント設定を削除する。
* @param なし
* @retval FFWエラーコード
*/
//==============================================================================
FFWERR DO_ClrPBAll(void)
{
	FFWERR			ferr;
	FFWERR			ferrEnd;
	FFW_PB_DATA*	pPb;			// S/Wブレークポイント設定情報領域のポインタ
	DWORD			i;
#if defined(E2E2L)
	FFWE20_EINF_DATA		einfData;
	BOOL			bRet;
	DWORD			dwTblNo;
	RFW_ASPMONEVCOND_TABLE* pMonEvTable = NULL;	// 初期化されていない可能性のある変数使用の警告回避のためNULLを代入
#endif

	ProtInit();

	pPb = GetPbData();				// S/Wブレークポイント設定を取得

#if defined(E2E2L)
	getEinfData(&einfData);
	if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
		if (IsAspOn()) {	// E2拡張機能が有効
			pMonEvTable = GetMonEvCondTableInfo();
		}
	}
#endif

	for (i = 0; i < pPb->dwSetNum; ++i) {
		if (pPb->eFillState[i] == PB_FILL_COMPLETE) {				// 埋め込み済みの場合
			SetPbClrTbl(pPb->dwmadrAddr[i], pPb->byCmdData[i]);		// 埋め戻し対象テーブルに登録
			ferr = WriteUserCode(pPb->dwmadrAddr[i]);	// 命令コード復帰　WriteUserCode()内で復帰コードを検索
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
#if defined(E2E2L)
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			if (IsAspOn()) {	// E2拡張機能が有効
				bRet = SearchMonEvNo((UINT64)pPb->dwmadrAddr[i], RF_ASPMONEV_TYPE_SWBRK);
				if (bRet) {		// 対象アドレスに通過ポイントとしてS/Wブレークポイントが設定されている場合は通過ポイントを削除
					/* 当該テーブルを初期化 */
					dwTblNo = monev_tblno(bRet);
					memset(&pMonEvTable[dwTblNo], 0x00, sizeof(RFW_ASPMONEVCOND_TABLE));
				}
			}
		}
#endif
		pPb->byCmdData[i] = 0x00;
		pPb->dwmadrAddr[i] = 0x00000000;
		pPb->eFillState[i] = PB_FILL_NON;
	}

	pPb->dwSetNum = INIT_PB_NUM;				// S/Wブレークポイント数初期化

	// RevNo120720-002 Append Start
	if (IsMcuRun() == TRUE) {
		// プログラム実行中の場合(RAMアドレスのS/Wブレークポイントクリア時)は、
		// 埋め戻し対象のS/Wブレークポイントをクリアする
		ClrPbClrTbl();
	}
	// RevNo120720-002 Append End

	ferrEnd = ProtEnd();

	return ferrEnd;

}
// RevRxE2No171004-001 Append End

//==============================================================================
/**
 * 対象アドレスにS/Wブレークが設定されているかを検索する。
 * @param dwmadrAddr ブレーク検索対象アドレス
 * @param pPb S/Wブレークポイント設定情報領域のポインタ
 * @retval TRUE ブレークが設定されている
 * @retval FALSE ブレークが設定されていない
 */
//==============================================================================
BOOL SearchBrkPoint(DWORD dwmadrAddr, FFW_PB_DATA* pPb)
{

	DWORD	i;

	for (i = 0; i < pPb->dwSetNum; i++) {
		if (pPb->dwmadrAddr[i] == dwmadrAddr && (pPb->eFillState[i] == PB_FILL_SET || pPb->eFillState[i] == PB_FILL_COMPLETE)) {
					// ブレークが設定されている
			return TRUE;
		}
	}

	return FALSE;

}

// RevNo120606-002 Append Start
//==============================================================================
/**
 * BMコマンド処理実行
 * 実行中のBM変更発生時に、OCDブレーク設定を変更する。
 * @param dwBMode BMで渡されたBMモード
 */
//==============================================================================
FFWERR DO_SetBM(DWORD dwBMode){

	FFWERR			ferr = FFWERR_OK;
	FFWERR			ferrEnd;
	FFWRX_COMB_BRK*	pevBrk;
	FFWRX_COMB_BRK	evNewBrk;
	DWORD			dwBefBMode,dwBefBFactor;
	DWORD			dwPreBrkTmp,dwExecPcSet,dwOpcSet;
	BOOL			bNeedSetPreEv = FALSE;
	BOOL			bNeedSetEv = FALSE;
	
	ProtInit();

	// GB,RSTG実行中はイベントブレークを有効/無効にする
	if((GetMcuRunState() == TRUE) && (GetMcuRunCmd() == PROGCMD_GB)){

		// 前回までの設定を取得
		GetBmMode(&dwBefBMode,&dwBefBFactor);
		// イベントブレーク設定情報を取得
		pevBrk = GetEvCombBrkInfo();

		// 今回設定するブレーク情報設定変数を0で初期化
		memset(&evNewBrk,0,sizeof(FFWRX_COMB_BRK));

		// 実行前PCブレークモード変更
		if(((dwBefBMode & BMODE_PRE_BIT) == BMODE_PRE_BIT) &&  ((dwBMode & BMODE_PRE_BIT) != BMODE_PRE_BIT)){
			//有効→無効設定時
			if(pevBrk->dwPreBrk != 0){
				bNeedSetPreEv = TRUE;
			}
		} else if(((dwBefBMode & BMODE_PRE_BIT) != BMODE_PRE_BIT) &&  ((dwBMode & BMODE_PRE_BIT) == BMODE_PRE_BIT)){
			//無効→有効設定時
			if(pevBrk->dwPreBrk != 0){
				bNeedSetPreEv = TRUE;
				evNewBrk.dwPreBrk = pevBrk->dwPreBrk;
			}
		} else if((dwBefBMode & BMODE_PRE_BIT) == BMODE_PRE_BIT) {
			// 有効→有効の場合
			evNewBrk.dwPreBrk = pevBrk->dwPreBrk;
		}

		// イベントブレークモード変更
		if(((dwBefBMode & BMODE_EB_BIT) == BMODE_EB_BIT) &&  ((dwBMode & BMODE_EB_BIT) != BMODE_EB_BIT)){
			//有効→無効設定時
			if(pevBrk->dwBrkpe != 0){
				bNeedSetEv = TRUE;
			}
			if(pevBrk->dwBrkde != 0){
				bNeedSetEv = TRUE;
			}
		} else if(((dwBefBMode & BMODE_EB_BIT) != BMODE_EB_BIT) &&  ((dwBMode & BMODE_EB_BIT) == BMODE_EB_BIT)){
			//無効→有効設定時
			if(pevBrk->dwBrkpe != 0){
				bNeedSetEv = TRUE;
				evNewBrk.dwBrkpe = pevBrk->dwBrkpe;
			}
			if(pevBrk->dwBrkde != 0){
				bNeedSetEv = TRUE;
				evNewBrk.dwBrkde = pevBrk->dwBrkde;
			}
		} else if((dwBefBMode & BMODE_EB_BIT) == BMODE_EB_BIT){
			//有効→有効設定時
			evNewBrk.dwBrkpe = pevBrk->dwBrkpe;
			evNewBrk.dwBrkde = pevBrk->dwBrkde;
		}

		//ブレーク設定実施
		if(bNeedSetPreEv || bNeedSetEv){
			// 実行前PC,実行通過PC,オペランドイベント動作許可レジスタを無効
			// 実行前PCイベントを変更する可能性があるため、SetEventEnable(TRUE)ではなくSetRegEvEnable()で復帰
			ferr = SetEventEnable(FALSE);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if(bNeedSetEv){
				//イベントブレーク設定が必要な場合は、実行通過PC,オペランドイベントブレーク選択レジスタ設定
				ferr = SetEvBreakData(&evNewBrk);
				if(ferr != FFWERR_OK){
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
			//実行通過PC,オペランドイベント動作許可情報取得
			GetRegEvEnable(&dwPreBrkTmp,&dwExecPcSet,&dwOpcSet);
			//実行前PC,実行通過PC,オペランドイベント動作許可レジスタ設定
			ferr = SetRegEvEnable(evNewBrk.dwPreBrk, dwExecPcSet, dwOpcSet);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}

	}
	if(GetMcuRunState() == TRUE) {
		// 実行中はBMを更新
		SetBmData();
	}

	ferrEnd = ProtEnd();
	return ferr;
}
// RevNo120606-002 Append End

//=============================================================================
/**
 * ブレーク関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Brk(void)
{
	return;

}
