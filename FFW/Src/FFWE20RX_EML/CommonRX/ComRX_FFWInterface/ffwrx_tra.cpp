////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_tra.cpp
 * @brief RX共通トレース関連コマンドのヘッダファイル
 * @author RSD Y.Minami, H.Hashiguchi, Y.Miyake, K.Uemori, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2015) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2015/01/20
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/10/15 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・FFWRXCmd_GetRD(), FFWCmd_ClrRD(), FFWRXCmd_GetRCY()で
    DO_GetRXSTAT()関数呼び出し時の引数のbyStatKindをdwStatKindに変更。
  ・FFWRXCmd_SetRM()で、RX64x の場合pTraceMode->dwTrcSrcSelのb0,b1が
    同時に"1"のとき、引数エラーとする。
    InitFfwIfRxData_Tra()で、s_RmData_RX.dwTrcSrcSelに
    初期値 "1(CPUバス)" 設定を追加。
・RevRxNo121022-001	2012/10/22 SDS 岩田
　　chkRXRMArg()関数にEZ-CUBE用処理を追加
・RevRxNo121022-001	2012/11/1 SDS 岩田
　　chkRXRMArg()関数のエミュレータ種別 EZ-CUBE判定処理の定義名を変更
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130308-001 2013/08/21 上田 (2013/03/15 三宅担当分マージ)
　カバレッジ開発対応
・RevRxNo130301-001	2013/09/24 上田
	RX64M対応
・RevRxNo130730-006	2013/11/13 上田
	E20トレースクロック遅延設定タイミング変更
・RevRxE2LNo141104-001 2015/01/20 上田
	E2 Lite対応
*/

#include "ffwrx_tra.h"
#include "dorx_tra.h"
#include "domcu_prog.h"
#include "errchk.h"
// 2008.10.7 INSERT_BEGIN_E20RX600(+3) {
#include "prot_common.h"
// 2008.10.7 INSERT_END_E20RX600 }
#include "domcu_mcu.h"
#include "do_sys.h"
// RevRxNo121026-001 Append Line
#include "ffwrx_tim.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line
#include "hwrx_fpga_tra.h"	// RevRxNo130730-006 Append Line

// FFW 内部変数

// static関数宣言
// V.1.02 No.31 Class2トレース対応 Append Start
static FFWERR chkRXRMArg(const FFWRX_RM_DATA* pTraceMode);
static FFWERR chkRXCls2RMArg(const FFWRX_RM_DATA* pTraceMode);
// V.1.02 No.31 Class2トレース対応 Append End
static void setRmOldMode(void);	// RevRxNo130301-001 Append Line


static FFWRX_RM_DATA			s_RmData_RX;	// トレースモード設定の管理変数
static FFWRX_RM_OLDMODE_DATA	s_RmOldMode;	// 前回のトレースモード設定情報管理構造体変数	// RevRxNo130301-001 Append Line
static BOOL	s_bTraceDataClr;	// RCLコマンドによるトレースデータクリア実行有無(TRUE:実行済み/FALSE:未実行)	// RevRxE2LNo141104-001 Append Line


// V.1.02 No.31 Class2トレース対応 Modify Start
//==============================================================================
/**
 * トレースモードを設定する。
 * @param pTraceMode トレースモードを格納するFFWRX_RM_DATA 構造体のアドレス
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_SetRM(const FFWRX_RM_DATA* pTraceMode)
{
	BOOL	bRet = FALSE;
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RTT_UNSUPPORT;						// 「トレース機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	// Run中かの判定
	bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line
	if (bHotPlugState && bIdCodeResultState) {
		// RevNo110331-001 Delete
		// ホットプラグ起動中の場合、内部変数に設定値を渡すため、DO_SetRXRM()を実行させる
	} else {
		// 通常RUN中の場合
		if(bRet){
			return FFWERR_BMCU_RUN;
		}
	}

	pMcuInfoData = GetMcuInfoDataRX();


	// MCU情報取得
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if ((pFwCtrl->eOcdCls == RX_OCD_CLS2) || (pMcuInfoData->wPPC == RX_PPC_NON)) {
		// OCD Class 2の場合
		// 引数チェック
		ferr = chkRXCls2RMArg(pTraceMode);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// 内部管理変数更新前に、前回のトレースモード情報設定
		setRmOldMode();		// RevRxNo130301-001 Append Line

		//内部管理変数へ引数を渡す
		memcpy(&s_RmData_RX,pTraceMode,sizeof(FFWRX_RM_DATA));

		// トレース設定
		ferr = DO_SetRXCls2RM(pTraceMode);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	} else {
		// OCD Class 3の場合
		// 引数チェック
		ferr = chkRXRMArg(pTraceMode);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// 内部管理変数更新前に、前回のトレースモード情報設定
		setRmOldMode();		// RevRxNo130301-001 Append Line

		//内部管理変数へ引数を渡す
		memcpy(&s_RmData_RX,pTraceMode,sizeof(FFWRX_RM_DATA));
		//ハードウェアデバッグモードは設定禁止なので、強制的にソフトウェアデバッグモードにする
		s_RmData_RX.dwSetInfo = s_RmData_RX.dwSetInfo & ~RM_SETINFO_BSS;

		// E20トレースFPGA全レジスタ設定を指示
		SetTrcE20FpgaAllRegFlg(TRUE);	// RevRxNo130730-006 Append Line

		// トレース設定
		ferr = DO_SetRXRM(pTraceMode);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// E20トレースFPGA全レジスタ設定指示を解除
		SetTrcE20FpgaAllRegFlg(FALSE);	// RevRxNo130730-006 Append Line
	}

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End

}
// V.1.02 No.31 Class2トレース対応 Modify End

//==============================================================================
/**
 * トレースモードを参照する。
 * @param pTraceMode トレースモードを格納するFFWRX_RM_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_GetRM(FFWRX_RM_DATA* pTraceMode)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR						ferr = FFWERR_OK;
	// RevRxNo130308-001 Append Start
	FFWMCU_DBG_DATA_RX*	pDbgData;

 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RTT_UNSUPPORT;						// 「トレース機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	//内部管理変数へ引数を渡す
	memcpy(pTraceMode,&s_RmData_RX,sizeof(FFWRX_RM_DATA));

	return ferr;
	// V.1.02 RevNo110613-001 Modify End

}


// V.1.02 No.31 Class2トレース対応 Modify Start
// 2008.9.26 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * トレースメモリの内容を取得する。
 * @param dwStartCyc 開始サイクルの指定
 * @param dwEndCyc   終了サイクルの指定
 * @param pdwSetInfo トレース出力情報の設定状態を取得
 * @param rd         取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_GetRD(DWORD dwStartCyc, DWORD dwEndCyc, DWORD* pdwSetInfo, FFWRX_RD_DATA* rd)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR				ferr = FFWERR_OK;
	BOOL				bRet;				// プログラム実行状態
	FFWRX_RCY_DATA		rcy;
	BOOL				bRcyFlg;
	DWORD				dwCycCnt;

	// RevRxNo120910-001 Modify Line
	DWORD						dwStatKind;
	enum FFWRXENM_STAT_MCU		eStatMcu;
	FFWRX_STAT_SFR_DATA			pStatSFR;
	FFWRX_STAT_JTAG_DATA		pStatJTAG;
	FFWRX_STAT_FINE_DATA		pStatFINE;
	FFWRX_STAT_EML_DATA			pStatEML;
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RTT_UNSUPPORT;						// 「トレース機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line
	if (bRet) {
		// トレース動作状態を確認
		// RevRxNo120910-001 Modify Line
		dwStatKind = STAT_KIND_EML;		// EML情報のみ取得
		// RevRxNo120910-001 Modify Line
		ferr = DO_GetRXSTAT(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
		if (ferr != FFWERR_OK) {
			ProtEnd();
			return ferr;
		}

		// トレース計測中の場合
		if ( (pStatEML.dwTraceInfo & STAT_EML_TRCINFO_RUN ) == STAT_EML_TRCINFO_RUN) {
			return FFWERR_RTTSTAT_RECORDING;	// トレース動作中エラー
		}
	// RevNo110510-004 Modify Start
	} else {
		if ((GetMcuRunCmd() == PROGCMD_STEP) || (GetMcuRunCmd() == PROGCMD_STEPOVER)){
			// FFW内部変数が"ステップ実行によるブレーク成立"の場合ゴミが出る可能性があるので
			// サイクルがないエラーを返す
			return FFWERR_RTTDATA_NON;
		}
	}
	// RevNo110510-004 Modify End

	// RevRxE2LNo141104-001 Append Start
	if (s_bTraceDataClr) {	// RCLコマンドによるトレースデータクリア実行済みの場合
		return FFWERR_RTTDATA_NON;
	}
	// RevRxE2LNo141104-001 Append End

	// サイクル数の取得確認
	bRcyFlg = GetSetRcyFlg(&rcy);
	if ( bRcyFlg == FALSE ) {
		// サイクル数を取得
		ferr = DO_GetRXRCY(&rcy);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	
	// 開始サイクルチェック
	if (rcy.dwStartCyc > dwStartCyc) {
		return FFWERR_FFW_ARG;
	}
	
	// 終了サイクルチェック
	if (rcy.dwEndCyc < dwEndCyc) {
		return FFWERR_FFW_ARG;
	}

	// トレースデータ有無チェック
	dwCycCnt = rcy.dwEndCyc - rcy.dwStartCyc +1;

	// トレースデータがない場合 Start 0 End -1 サイクル数 0
	if ( (dwCycCnt == 0) && (rcy.dwStartCyc == 0) ) {
		return FFWERR_RTTDATA_NON;
	}

	// トレースメモリ内容を取得
	ferr = DO_GetRXRD(dwStartCyc, dwEndCyc, pdwSetInfo, rd);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}
// 2008.9.26 INSERT_END_E20RX600 }
// V.1.02 No.31 Class2トレース対応 Modify End

//==============================================================================
/**
 * トレースデータを取得する。
 * @param dwBlockStart トレースメモリ開始アドレス
 * @param dwBlockEnd   取得するサイクル数の指定
 * @param rd           取得したトレースデータを格納する
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWCmd_GetRD2_SPL(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* rd)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR		ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	ferr = DO_GetRD2_SPL(dwBlockStart, dwBlockEnd, rd);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;

}

//==============================================================================
/**
 * トレースデータをクリアする。
 * @param なし
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWE100_API DWORD FFWCmd_ClrRD(void)
{

	// V.1.02 RevNo110613-001 Modify Line
	FFWERR				ferr = FFWERR_OK;
	BOOL				bRet;				// プログラム実行状態

	DWORD						dwStatKind;
	enum FFWRXENM_STAT_MCU		eStatMcu;
	FFWRX_STAT_SFR_DATA			pStatSFR;
	FFWRX_STAT_JTAG_DATA		pStatJTAG;
	FFWRX_STAT_FINE_DATA		pStatFINE;
	FFWRX_STAT_EML_DATA			pStatEML;

	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;

	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RTT_UNSUPPORT;						// 「トレース機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line
		if (bRet) {
			// トレース動作状態を確認
			// RevRxNo120910-001 Modify Line
			dwStatKind = STAT_KIND_EML;		// EML情報のみ取得
			// RevRxNo120910-001 Modify Line
			ferr = DO_GetRXSTAT(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
			if (ferr != FFWERR_OK) {
				ProtEnd();
				return ferr;
			}

			// トレース計測中の場合
			if ( (pStatEML.dwTraceInfo & STAT_EML_TRCINFO_RUN ) == STAT_EML_TRCINFO_RUN) {
				return FFWERR_RTTSTAT_RECORDING;	// トレース動作中エラー
			}

			// 内蔵トレース時は実行中エラー
			if (GetTrcInramMode()) {
				return FFWERR_BMCU_RUN;
			}
		}
	} else {	//ホットプラグ中かつRUN中は何もしないでOKを返す
		bRet = IsMcuRun();
		if (bRet) {
			return FFWERR_OK;
		}
	}

	s_bTraceDataClr = TRUE;	// RCLコマンドによるトレースデータ実行済み // RevRxE2LNo141104-001 Append Line

	ferr = DO_ClrRD();

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;

}


// V.1.02 No.31 Class2トレース対応 Modify Start
// 2008.9.26 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * トレースメモリに格納されたデータの開始サイクルと終了サイクルの情報を取得する。
 * @param pRcy トレースメモリに格納されたトレースデータの開始サイクル、
 *             終了サイクルを格納するFFWRX_RCY_DATA 構造体のアドレス
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_GetRCY(FFWRX_RCY_DATA *pRcy)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR				ferr = FFWERR_OK;
	BOOL				bRet;				// プログラム実行状態

	// RevRxNo120910-001 Modify Line
	DWORD						dwStatKind;
	enum FFWRXENM_STAT_MCU		eStatMcu;
	FFWRX_STAT_SFR_DATA			pStatSFR;
	FFWRX_STAT_JTAG_DATA		pStatJTAG;
	FFWRX_STAT_FINE_DATA		pStatFINE;
	FFWRX_STAT_EML_DATA			pStatEML;
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RTT_UNSUPPORT;						// 「トレース機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line
		if (bRet) {
			// トレース動作状態を確認
			// RevRxNo120910-001 Modify Line
			dwStatKind = STAT_KIND_EML;		// EML情報のみ取得
			// RevRxNo120910-001 Modify Line
			ferr = DO_GetRXSTAT(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
			if (ferr != FFWERR_OK) {
				ProtEnd();
				return ferr;
			}

			// トレース計測中の場合
			if ( (pStatEML.dwTraceInfo & STAT_EML_TRCINFO_RUN ) == STAT_EML_TRCINFO_RUN) {
				return FFWERR_RTTSTAT_RECORDING;	// トレース動作中エラー
			}

	// RevNo110510-004 Modify Start
		} else {
			if ((GetMcuRunCmd() == PROGCMD_STEP) || (GetMcuRunCmd() == PROGCMD_STEPOVER)){
				// FFW内部変数が"ステップ実行によるブレーク成立"の場合ゴミが出る可能性があるので
				// サイクル数０で返す		
				pRcy->dwStartCyc = 0;
				pRcy->dwEndCyc = RCY_CYCLE_NON;
				return FFWERR_OK;
			}
	// RevNo110510-004 Modify End
		}

		// RevRxE2LNo141104-001 Append Start
		// RCLコマンドによるトレースデータクリア実行済みの場合「トレースサイクルなし」で正常終了する。
		if (s_bTraceDataClr) {
			pRcy->dwStartCyc = 0;
			pRcy->dwEndCyc = RCY_CYCLE_NON;
			return FFWERR_OK;
		}
		// RevRxE2LNo141104-001 Append End

	} else {	//ホットプラグ中は何もしないで、サイクルなしで返す。
		pRcy->dwStartCyc = 0;
		pRcy->dwEndCyc = RCY_CYCLE_NON;
		return FFWERR_OK;	
	}

	// トレースサイクル数を取得
	ferr = DO_GetRXRCY(pRcy);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}
// 2008.9.26 INSERT_END_E20RX600 }
// V.1.02 No.31 Class2トレース対応 Modify End

// 2008.10.29 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * トレース入力を停止する。
 * @param  eTrstp         トレース入力の停止/再開を指定
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_SetTRSTP(enum FFWENM_TRSTP eTrstp)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR		ferr = FFWERR_OK;

	//RevNo100715-007 Append Start
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RTT_UNSUPPORT;						// 「トレース機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if (bHotPlugState == TRUE && bIdCodeResultState == TRUE) {
		return FFWERR_RTTSTAT_DIS;
	}
	//RevNo100715-007 Append Start

	// 引数チェック
	if (eTrstp != TRSTP_RESTART && eTrstp != TRSTP_STOP_KEEP && eTrstp != TRSTP_STOP_CLR) {
		return FFWERR_FFW_ARG;
	}
	
	// RevNo110322-001	Modifiy Line
	ferr = DO_SetRXTRSTP(eTrstp,FALSE);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return FFWERR_OK;

}
// 2008.10.29 INSERT_END_E20RX600 }

// V.1.02 No.31 Class2トレース対応 Append Start
//==============================================================================
/**
 * FFWRXCmd_SetRM Class3用 引数チェック。
 * @param pTraceMode トレース設定値格納ポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR chkRXRMArg(const FFWRX_RM_DATA* pTraceMode){

	FFWMCU_MCUDEF_DATA	*pMcuDef;
	FFWE20_EINF_DATA				einfData;
	DWORD dwMode;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130301-001 Append Line
	
	pMcuDef = GetMcuDefData();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130301-001 Append Line

	// ROM空間開始アドレス設定
	// ROM空間開始アドレスがROM空間終了アドレスよりも大きい
	if (pTraceMode->dwRomStartAddr > pTraceMode->dwRomEndAddr ){
		return FFWERR_ROM_RANGE;
	}

	// ウィンドウトレースアドレスの範囲チェック
	if (pTraceMode->dwWinTrStartAddr > pTraceMode->dwWinTrEndAddr ){
		return FFWERR_WINTRC_RANGE;
	}
	if (pTraceMode->dwWinTrEndAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_WINTRC_RANGE;
	}

	if (((pTraceMode->dwWinTrStartAddr < 0x00000000) || (pTraceMode->dwWinTrStartAddr > 0xFFFFFF00)) ||
		((pTraceMode->dwWinTrEndAddr < 0x000000FF) || (pTraceMode->dwWinTrEndAddr > 0xFFFFFFFF))) {
		return FFWERR_WINTRC_RANGE;
	}


	// トレースメモリサイズ指定チェック
	if ((pTraceMode->eEmlTrcMemSize < EML_TRCMEM_64M) || (EML_TRCMEM_512K < pTraceMode->eEmlTrcMemSize)) {
		return FFWERR_FFW_ARG;
	}
	
	// トレースモードチェック
	dwMode = pTraceMode->dwSetMode >> 24;
	// トレースモードとピン数の関連チェック
	getEinfData(&einfData);						// エミュレータ情報取得

	if(dwMode > MODE10){
		return FFWERR_FFW_ARG;
	}
	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続以外の場合
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
		if ((dwMode == MODE2) || (dwMode == MODE3) || (dwMode == MODE6)) {
			return FFWERR_MODE_AND_PIN;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// RevRxNo130301-001 Append Start
	// RevRxNo120910-001 Append Start
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->bTraSrcSelEna == TRUE) {	// トレースソース選択があるMCUの場合
		// b0,b1が同時に"1"のとき、引数エラーとする。
		if ((pTraceMode->dwTrcSrcSel & (RM_TRCSRCSEL_MCU_BUS | RM_TRCSRCSEL_DMAC_DTC_BUS)) == 
									   (RM_TRCSRCSEL_MCU_BUS | RM_TRCSRCSEL_DMAC_DTC_BUS)     ) {
			return FFWERR_FFW_ARG;
		}

		// E20 38ピンケーブル接続時、トレースソースがDMAC/DTCトレースで、MODE2,4,6,10(TDOS=00かつTRM=1)の場合
		if ((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN)) {
			if ((pTraceMode->dwTrcSrcSel & RM_TRCSRCSEL_DMAC_DTC_BUS) == RM_TRCSRCSEL_DMAC_DTC_BUS) {
				if ((dwMode == MODE2) || (dwMode == MODE4) || (dwMode == MODE6) || (dwMode == MODE10)) {
					return FFWERR_RTTMODE_UNSUPRT;
				}
			}
		}
	}
	// RevRxNo120910-001 Append End
	// RevRxNo130301-001 Append End

	return FFWERR_OK;
}

//==============================================================================
/**
 * FFWRXCmd_SetRM Class2用 引数チェック。
 * @param トレース設定値格納ポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR chkRXCls2RMArg(const FFWRX_RM_DATA* pTraceMode){

	FFWERR	ferr = FFWERR_OK;
	DWORD dwMode;

	// RevRxNo121026-001 Append Start
	FFWRX_MCUINFO_DATA* pMcuInfo;

	pMcuInfo = GetMcuInfoDataRX();
	// RevRxNo121026-001 Append End

	// 分岐元とオペランドが有効(TBEとTDEが1)
	if((pTraceMode->dwSetInfo & 0x00060000) == 0x00060000) {
		return FFWERR_FFW_ARG;
	}

	// 分岐元と分岐先が有効かつタイムスタンプ出力(TBEとTDEが1)
	if((pTraceMode->dwSetInfo & 0x1C020000) == 0x14020000) {
		return FFWERR_FFW_ARG;
	}

	// トレースモードチェック
	dwMode = pTraceMode->dwSetMode >> 24;
	if(dwMode > MODE10){
		return FFWERR_FFW_ARG;
	}
	// MODE 2,3,6の場合は引数エラー
	if (dwMode == MODE2 || dwMode == MODE3 || dwMode == MODE6) {
		return FFWERR_FFW_ARG;
	}

	// RevRxNo121026-001 Append Start
	// PPC未実装MCUのTPEチェック
	if(pMcuInfo->wPPC == RX_PPC_NON) {
		if((pTraceMode->dwSetInfo & RM_SETINFO_TPE) == RM_SETINFO_CLS2_TMSTMP) {
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxNo121026-001 Append End

	return ferr;
}
// V.1.02 No.31 Class2トレース対応 Append End

// RevRxNo130301-001 Append Start
//==============================================================================
/**
 * 前回のトレースモード情報設定
 * @param なし
 * @retval なし
 */
//==============================================================================
static void setRmOldMode(void)
{
	// 現在のRMコマンドで指定されているトレースモード(MODE1～MODE10)を格納
	s_RmOldMode.dwMode = (s_RmData_RX.dwSetMode >> 24) & 0x000000ff;

	// "前回のトレースモード設定済み"に設定
	s_RmOldMode.bSetMode = TRUE;

	return;
}
// RevRxNo130301-001 Append End


//==============================================================================
/**
 * トレース設定情報の取得
 * @param  なし
 * @retval s_RmData_RX 設定されているトレース情報格納ポインタ
 */
//==============================================================================
FFWRX_RM_DATA* GetRmDataInfo(void){
	return &s_RmData_RX;
}

//==============================================================================
/**
 * トレースモード情報の格納
 * @param  dwSetMode RM.dwSetMode情報
 * @retval なし
 */
//==============================================================================
void SetRmModeData(DWORD dwSetMode)
{
	s_RmData_RX.dwSetMode = dwSetMode;
	return;
}
//==============================================================================
/**
 * トレース RM Initial情報の設定しなおし
 * @param  dwSetInitial RM.dwSetInitial情報
 * @retval なし
 */
//==============================================================================
void SetRmInitialData(DWORD dwSetInitial)
{
	s_RmData_RX.dwSetInitial = dwSetInitial;
	return;
}

// RevRxNo130301-001 Append Start
//==============================================================================
/**
 * 前回のトレースモード設定情報の取得
 * @param  なし
 * @retval 前回のトレースモード設定情報管理構造体変数のポインタ
 */
//==============================================================================
extern FFWRX_RM_OLDMODE_DATA* GetRmOldMode(void)
{
	return &s_RmOldMode;
}
// RevRxNo130301-001 Append End


// RevRxE2LNo141104-001 Append Start
//==============================================================================
/**
 * RCLコマンドによるトレースデータクリア実行有無フラグクリア
 * @param  なし
 * @retval なし
 */
//==============================================================================
void ClrTraceDataClrFlg(void)
{
	s_bTraceDataClr = FALSE;
	return;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * トレース関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfRxData_Tra(void)
{
	
	memset(&s_RmData_RX, 0, sizeof(FFWRX_RM_DATA));
	s_RmData_RX.dwSetMode = MODE1;
	// RevRxNo120910-001 Append Line
	s_RmData_RX.dwTrcSrcSel = RM_TRCSRCSEL_MCU_BUS;
	s_RmData_RX.dwRomStartAddr = 0xFE000000;
	s_RmData_RX.dwRomEndAddr = 0xFFFFFFFF;
	s_RmData_RX.dwWinTrStartAddr = 0x00000000;
	s_RmData_RX.dwWinTrEndAddr = 0xFFFFFFFF;
	// Rev110303-001 Modify Line
	s_RmData_RX.eEmlTrcMemSize = EML_TRCMEM_16M;

	s_RmOldMode.bSetMode = FALSE;	// RevRxNo130301-001 Append Line

	s_bTraceDataClr = FALSE;	// RevRxE2LNo141104-001 Append Line

	return;
}
