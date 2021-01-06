///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_prog.cpp
 * @brief プログラム実行関連コマンドの実装ファイル
 * @author RSD Y.Minami, H.Hashiguchi, Y.Miyake, H.Akashi, S.Ueda, K.Uemori, Y.Kawakami, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120606-001　2012/06/07 橋口
  ステップ実行開始アドレスのエンディアン判定処理修正 REG_NUM_xx定義削除
・RevNo111121-002	2012/07/11 橋口
  ・DO_GB()
    ・リセットベクタが示すアドレスにPB 設定して実行する場合、
      以下を実行するように変更。
      ・フラッシュ書き換え時のクロック切り替え設定が「禁止」の場合、
        フラッシュ書き換え前にクロック切り替え設定を「許可」にし、
        フラッシュ書き換え後に元に戻す処理を追加。
・RevNo110825-01 2012/07/13 橋口
　GPB実行イベントへのパスカウント設定不具合対応
・RevRxNo120910-001	2012/09/11 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・IsMcuRun(), IsMcuStateNormal()で、GetStatData()関数呼び出し時の引数の
    byStatKindをdwStatKindに変更。
・RevRxNo121029-003 2012/10/31 明石(橋口)
　ブレークポイントから実行時の時間計測改善
・RevRxNo121106-001	2012/11/14 上田
　・InitFfwCmdMcuData_Prog()にs_bPmodeInRomDisFlgの初期化処理を追加。
・RevRxNo121106-001	2012/11/15 上田
　・McuBreak_RX()のID認証結果設定状態を未設定にする処理を削除。
・RevRxNo121026-001 2012/12/13 植盛
  ブートスワップデバッグ時のSTEP処理変更
 ・RevRxEzNo121213-002 2012/12/18 橋口
  EZ-CUBE 性能向上対応
  IsMcuRun()内のSTAT呼び出しをSTAT_KIND_NONにしたが、IsMcuRun()の結果が正常な結果が
  返らないため、元に戻した。2012/12/14 (2012/12/21 橋口コメントのみ追加)
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130308-001 2013/04/23 三宅
　カバレッジ開発対応
・RevRxNo130730-005 2013/11/11 上田
　内蔵ROM有効/無効判定処理改善
・RevRxNo140515-007 2014/06/18 植盛
	SSST, SSENコマンド追加による速度改善
・RevRxNo140515-008 2014/06/24 植盛
	ステップ実行の高速化対応
・RevRxNo140515-006 2014/06/24 植盛
	RX231対応
・RevRxNo140515-012 2014/06/20 川上
	ホットプラグ起動中のカバレッジ機能エラー処理修正
・RevRxE2LNo141104-001 2015/01/20 上田
	E2 Lite対応
・RevRxNo170727-001 2017/07/27 佐々木(広)
	E2拡張対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#include "domcu_prog.h"
#include "ffwmcu_prog.h"
#include "ffwmcu_brk.h"
#include "mcurx_ev.h"
#include "domcu_runset.h"
#include "dorx_tra.h"
#include "domcu_brk.h"
#include "protmcu_prog.h"
#include "prot_common.h"
#include "protmcu_reg.h"
#include "mcu_flash.h"
#include "ffwrx_ev.h"
#include "domcu_rst.h"
#include "domcu_mcu.h"
#include "do_sys.h"
#include "protmcu_mcu.h"
#include "dorx_tim.h"
#include "mcurx_tim.h"
//RevNo100715-014 Append Line
#include "ffwrx_rrm.h"
#include "ffwrx_tra.h"
#include "mcu_brk.h"
#include "mcu_sfr.h"
#include "mcu_rst.h"						// RevNo111121-002 Append Line
#include "mcudef.h"	// RevRxNo130411-001 Append Line
// RevRxNo130308-001-007 Append Line
#include "dorx_cv.h"
// RevRxNo170727-001 Append Start
#include "doasp_sample.h"
#include "e2_asp_fpga.h"
// RevRxNo170727-001 Append End
// ファイル内static変数の宣言
// RevRxNo170727-001 Append Start
#include "doasp_sys_family.h"
#include "doasp_sys.h"
#include "asp_setting.h"
// RevRxE2No171004-001 Append Start
#include "doasprx_monev.h"
#include "ffwmcu_srm.h"
#include "domcu_srm.h"
// RevRxE2No171004-001 Append End
// RevRxNo170727-001 Append End
static BOOL		s_bMcuRunState;					// プログラム実行状態格納変数(TRUE:実行中/FALSE:停止中)
static enum FFWENM_PROGCMD_NO	s_eMcuRunCmd;	// プログラム実行コマンド種別格納変数
static BOOL		s_bStepRunState;				// シングルステップ実行からのユーザプログラム実行状態

//static BOOL	bFirstStepSequence = TRUE;
//RevNo100715-014 Append Line	
static DWORD s_dwGPBEvNo;
static DWORD s_dwGPBAddr;						// GPB実行時に設定したブレークアドレス RevNo110825-01 Append Line

/////////////////////////////////////////////////////////////////////
// Ver.1.01 2010/08/17 SDS T.Iwata
static BOOL		s_bPmodeInRomDisFlg;					// 内蔵ROM無効拡張モード状態格納変数(TRUE:内蔵ROM無効拡張モード/FALSE:内蔵ROM無効拡張モード以外)
/////////////////////////////////////////////////////////////////////

static BOOL	s_bReadPmodeInRomDisRegFlg;		// 内蔵ROM無効拡張モード状態参照用レジスタリードフラグ	// RevRxNo130730-005 Append Line
static BOOL s_bStepContinueStart;			// 連続ステップ開始フラグ // RevRxNo140515-007 Append Line

// static関数の宣言
static void setStepRunState(BOOL bState);
static BOOL getStepRunState(void);

static FFWERR mcuStep(FFWMCU_REG_DATA_RX* pRegData_RX, FFWE20_STEP_DATA* pStep, BOOL* pbFfwStepReq, WORD* pwOpecode, BOOL* pbTgtReset, enum FFWENM_STEPCTRL eStepCtrl, BYTE byMskSet);
static FFWERR mcuStepOver(FFWMCU_REG_DATA_RX* pRegData_RX, FFWE20_STEP_DATA* pStep, BOOL* pbFfwStepReq, WORD* pwOpecode, BOOL* pbTgtReset, enum FFWENM_STEPCTRL eStepCtrl, BYTE byMskSet);
static FFWERR mcuWaitBreak_RX(FFWRX_PROG_DATA* pProgData);

//実行前設定関数
static FFWERR setOcdBreakMode(DWORD dwBMode);
static FFWERR setSwBreakMode(DWORD dwNewBMode, enum FFWENM_PROGCMD_NO eProgCmd, DWORD dwPcAddr);
static FFWERR setGoPpc(enum FFWENM_PROGCMD_NO eProgCmd);
static FFWERR setGoAfterPpc(void);
static FFWERR setGoStepPpc(void);
static FFWERR setGoEvent(enum FFWENM_PROGCMD_NO eProgCmd);
static FFWERR setGoTrace(enum FFWENM_PROGCMD_NO eProgCmd);
static FFWERR setGoWriteFlash(enum FFWENM_PROGCMD_NO eProgCmd);
static FFWERR mcuBrkStep(enum FFWENM_PROGCMD_NO eProgCmd, FFWE20_STEP_DATA* pStep);
static FFWERR setGpbBrkEvent(MADDR madrBreakAddr);
static FFWERR getBreakStatus_RX(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb,
								BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA);
static BOOL isStepContinue(void);			// RevRxNo140515-007 Append Line
static void clrStepContinueStartFlg(void);	// RevRxNo140515-007 Append Line
extern void AspMemSfifoEn(DWORD dwSrc);
extern void AspMemAspFifoEn(DWORD dwAspFifoEn);
///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * プログラムのフリー実行
 * @param eStartAddrSet 実行開始アドレスの設定有無
 * @param madrStartAddr 実行開始アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GO(enum FFWENM_STARTADDR_SET eStartAddrSet, MADDR madrStartAddr)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	FFWMCU_REG_DATA_RX*	pRegDataRX;
	DWORD	dwNowBMode,dwNowBFactor,dwNewBMode;
#if defined(E2E2L)
	FFWE20_EINF_DATA	einfData;		// RevRxE2No171004-001 Append Line
#endif
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	ProtInit();

	UpdateRunsetData();	// FFW 内部変数を更新

	GetBmMode(&dwNowBMode, &dwNowBFactor);	// 現在のブレークモード取得

	// GOコマンド用ブレークモード設定
	MakeBreakMode(PROGCMD_GO, &dwNewBMode);

		// レジスタ設定
	pRegDataRX = GetRegData_RX();
	if (eStartAddrSet == ADDR_SETON) {
		pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
	}
	ferr = PROT_MCU_SetRXXREG(pRegDataRX);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// OCDブレーク設定入力
	ferr = setOcdBreakMode(dwNewBMode);	// ブレークモードのハードウェア設定
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// SWブレークテーブル更新
	ferr = setSwBreakMode(dwNewBMode,PROGCMD_GO,pRegDataRX->dwRegData[REG_NUM_RX_PC]);	// SWブレークの実行前処理
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo130308-001 Append Line
 	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	// RevRxNo130308-001 Append Line
	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// カバレッジ機能以外選択時
		// プログラム実行前のトレース動作設定
		ferr = setGoTrace(PROGCMD_GO);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	// RevRxNo130308-001-007 Append Start
	} else {										// カバレッジ機能選択時
		ferr = SetGoTrace_CV();		// TBSR.TRFS設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo130308-001-007 Append End

	// イベント有効/無効設定
	ferr = setGoEvent(PROGCMD_GO);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// 実行前パフォーマンス機能設定
	ferr = setGoPpc(PROGCMD_GO);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	//実行前フラッシュ書き込み実行
	ferr = setGoWriteFlash(PROGCMD_GO);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	getEinfData(&einfData);
	if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
		// ASP ON
		if (IsAspOn()) {
			// ユーザプログラム実行開始
			/* ASP機能ON */
			Start_Asp();
		}
	}
#endif
	// RevRxE2No171004-001 Append End

	// RevRxE2No171004-001 Modify Line
	ferr = PROT_MCU_GO(DISABLE_BREAK);		// BFWMCUCmd_GOコマンド発行(ブレークなし実行設定)
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
		if (ferrEnd != FFWERR_OK) {
			return ferrEnd;
		}

		SetMcuRunCmd(PROGCMD_GO);	// GOコマンドによるプログラム実行
		SetMcuRunState(TRUE);		// ユーザプログラム実行状態を"プログラム実行中"にする

	}

	return ferr;
}


//=============================================================================
/**
 * プログラムのブレーク付き実行
 * @param eStartAddrSet 実行開始アドレスの設定有無
 * @param madrStartAddr 実行開始アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GB(enum FFWENM_STARTADDR_SET eStartAddrSet, MADDR madrStartAddr)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	DWORD*	pdwNowBMode;
	DWORD*	pdwNowBFactor;
	DWORD	dwNowBMode,dwNewBMode;
	DWORD	dwNowBFactor;
	FFW_PB_DATA*	pPb;
	DWORD	dwPreBrkCombi;

	FFWMCU_REG_DATA_RX*	pRegDataRX;
	enum FFWENM_PROGCMD_NO eGoCmd;
	FFWE20_STEP_DATA pStep;

	BYTE	byEscMode;
	BOOL	bEvPreBrk;
// RevNo111121-002 Append Start
	FFWERR	ferr2 = FFWERR_OK;
	enum FFWRXENM_PMODE			ePmode;
	FFWMCU_MCUAREA_DATA_RX*		pMcuArea;
	DWORD						dwRestVectSetAddr;  // リセットベクタが示すアドレス
	enum FFWRXENM_CLKCHANGE_ENA	eClkChangeEna;
	MADDR						madrGbAddr;
	BOOL						bStep = FALSE;
	BOOL						bClkChange = FALSE;
	BOOL						bFlashWrite = FALSE;
	FFWMCU_DBG_DATA_RX*	pDbgData;
// RevNo111121-002 Append End

	// RevRxE2No171004-001 Append Start
	BOOL	bDoStep = TRUE;		// S/Wブレークからの実行の場合にシングルステップするかどうか判断用
								// E2エミュレータで使用(それ以外のエミュレータでは常にTRUE(シングルステップする))
#if defined(E2E2L)
	BOOL	bRet;
	FFWE20_EINF_DATA	einfData;
	FFW_SRM_DATA*		pSrmData;
#endif
	// RevRxE2No171004-001 Append End

	ProtInit();

	UpdateRunsetData();	// FFW 内部変数を更新

	pdwNowBMode = &dwNowBMode;
	pdwNowBFactor = &dwNowBFactor;
	GetBmMode(pdwNowBMode, pdwNowBFactor);	// 現在のブレークモード取得
	dwNowBMode = *pdwNowBMode;

	pPb = GetPbData();						// S/Wブレークポイント設定を取得

// RevNo111121-002 Append Start
	ePmode = GetPmodeDataRX();
	pMcuArea = GetMcuAreaDataRX();
	pDbgData = GetDbgDataRX();								// デバッグ情報取得
// RevNo111121-002 Append End

	// 以下の場合、実行開始アドレスをシングルステップ実行してから、GB実行する。
	// (a)ソフトウェアブレーク許可で、実行開始アドレスがPBアドレスの場合
	// (b)アドレス一致ブレーク許可で、実行開始アドレスがアドレス一致ブレークアドレスの場合
	//     →R8C/Tinyではアドレス一致使用しないため、(b)は該当しない

	pRegDataRX = GetRegData_RX();											// レジスタ情報取得
	if (eStartAddrSet == ADDR_SETON) {
		pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
	}else {
		madrStartAddr = pRegDataRX->dwRegData[REG_NUM_RX_PC];
	}
	madrGbAddr = madrStartAddr;		// RevNo111121-002 Append Line

	ferr = PROT_MCU_SetRXXREG(pRegDataRX);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// GBコマンド用ブレークモード設定
	MakeBreakMode(PROGCMD_GB, &dwNewBMode);

	// OCDブレーク設定入力
	ferr = setOcdBreakMode(dwNewBMode);	// ブレークモードのハードウェア設定
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo130308-001 Append Line
	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// カバレッジ機能以外選択時
		// プログラム実行前のトレース動作設定
		ferr = setGoTrace(PROGCMD_GB);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	// RevRxNo130308-001-007 Append Start
	} else {										// カバレッジ機能選択時
		ferr = SetGoTrace_CV();		// TBSR.TRFS設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo130308-001-007 Append End

	// イベント有効/無効設定
	ferr = setGoEvent(PROGCMD_GB);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo121029-003	Append Start
	// 実行前パフォーマンス機能設定
	ferr = setGoPpc(PROGCMD_GB);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo121029-003	Append End

#if defined(E2E2L)
	getEinfData(&einfData);		// RevRxE2No171004-001 Append Line
#endif

	if (dwNowBMode & BMODE_SWB_BIT) {
		// ソフトブレークからの実行の場合
		if (SearchBrkPoint(madrStartAddr, pPb) == TRUE) {
			// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
			if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
				if (IsAspOn()) {	// E2拡張機能が有効
					bRet = SearchMonEvNo((UINT64)madrStartAddr, RF_ASPMONEV_TYPE_SWBRK);
					if (bRet != NOSET) {		// 対象アドレスにモニタイベントが設定されている
						//実行開始PCに実行前PCブレークが設定されているか確認
						ferr = SearchEvPreBrkPoint(madrStartAddr, &bEvPreBrk);
						if (ferr != FFWERR_OK) {
							ferrEnd = ProtEnd();
							return ferr;
						}
						if (bEvPreBrk == FALSE) {	// 実行前PCブレークが設定されていない
							bDoStep = FALSE;		// モニタイベントのみの場合はシングルステップしない
						}
					}
				}
			}
#endif

			if (bDoStep == TRUE) {
			// RevRxE2No171004-001 Append End
				// RevRxNo121029-003	Modify Start
				//シングルステップ実行
				//RevNo100715-013 Append Line
				eGoCmd = PROGCMD_GB;
				ferr = mcuBrkStep(eGoCmd, &pStep);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				// RevRxNo121029-003	Modify End

				//STEP実行が終了せずに実行してしまった場合はGB処理終了
				if (pStep.eGoToRunState == STEP_GOTO_RUNSTATE) {
					ferrEnd = ProtEnd();
					return ferr;
				}

				// STEP実行が正常終了した場合
				// 今のPCにブレークが設定されていないか確認
				madrStartAddr = pStep.dwPc;
				ferr = SearchEvPreBrkPoint(madrStartAddr, &bEvPreBrk);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				if ((SearchBrkPoint(madrStartAddr, pPb) == TRUE) || (bEvPreBrk == TRUE)) {
					//ブレークが設定されていた場合はそのまま終了
					pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
					ferr = PROT_MCU_SetRXXREG(pRegDataRX);
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					SetMcuRunCmd(PROGCMD_GB_BRKSTEP);
					// ここで終了する場合は、PPC後処理が必要
					ferr = setGoAfterPpc();
					ferrEnd = ProtEnd();
					return ferr;
				}
				pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
				ferr = PROT_MCU_SetRXXREG(pRegDataRX);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				// RevRxNo121029-003	Append Start
				// STEP後継続して実行する場合、PPC処理を実施
				ferr = setGoStepPpc();
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				// RevRxNo121029-003	Append End

				bStep = TRUE;	// RevNo111121-002 Append Line
			}		// RevRxE2No171004-001 Append Line
		}
	}

	//実行開始PCにイベントブレークが設定されているか確認
	ferr = SearchEvPreBrkPoint(madrStartAddr,&bEvPreBrk);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	if (bEvPreBrk == TRUE) {
		ferr = DisableEvPreBrkPoint(&bEvPreBrk,&dwPreBrkCombi);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// RevRxNo121029-003	Append Start
		//シングルステップ実行
		//RevNo100715-013 Append Line
		eGoCmd = PROGCMD_GB;
		ferr = mcuBrkStep(eGoCmd, &pStep);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxNo121029-003	Append End

		byEscMode = 0x2;	//初期化
		EscEvPreBrkPoint(byEscMode, &dwPreBrkCombi);
		if (pStep.eGoToRunState == STEP_GOTO_RUNSTATE) {
			byEscMode = 0x0;	//退避
			EscEvPreBrkPoint(byEscMode, &dwPreBrkCombi);
			ferrEnd = ProtEnd();
			return ferr;
		}
		ferr = EnableEvPreBrkPoint(&dwPreBrkCombi);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		madrStartAddr = pStep.dwPc;
		
		ferr = SearchEvPreBrkPoint(madrStartAddr,&bEvPreBrk);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if ((SearchBrkPoint(madrStartAddr, pPb) == TRUE) || (bEvPreBrk == TRUE)) {
			SetMcuRunCmd(PROGCMD_GB_BRKSTEP);
			// ここで終了する場合は、PPC後処理が必要
			ferr = setGoAfterPpc();
			ferrEnd = ProtEnd();
			return ferr;
		}
		pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
		ferr = PROT_MCU_SetRXXREG(pRegDataRX);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxNo121029-003	Append Start
		// STEP後継続して実行する場合、PPC処理を実施
		ferr = setGoStepPpc();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxNo121029-003	Append End
		
		bStep = TRUE;	// RevNo111121-002 Append Line
	}

	// SWブレークテーブル更新
	ferr = setSwBreakMode(dwNewBMode,PROGCMD_GB,madrStartAddr);	// SWブレークの実行前処理
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

// RevNo111121-002 Append Start
	bFlashWrite = GetStateFlashRomCache();
	if (bFlashWrite == TRUE) {
		//内蔵Flash ROMキャッシュに変更がある場合
			if (bStep == TRUE) {
			// 実行開始アドレスをシングルステップした場合

				eClkChangeEna = pDbgData->eClkChangeEna;
				if (eClkChangeEna == RX_CLKCHANGE_DIS) {
					// フラッシュ書き換え時のクロック切り替え設定が「禁止」の場合

					// リセットベクタが示すアドレスをリード
					ferr = GetMcuRestAddr((BYTE)RESTVECT_CACHE_DATA,&dwRestVectSetAddr);
					if (ferr != FFWERR_OK) {
						return ferr;
					}

					// リセットベクタが示すアドレスが実行開始アドレスと同じかをチェック
					if (dwRestVectSetAddr == madrGbAddr) {
						// リセットベクタが示すアドレスが実行開始アドレスと同じである場合

						// フラッシュ書き換え時のクロック切り替えを一時的に「許可」に設定
						ferr = SetDbgClkChg(RX_CLKCHANGE_ENA);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						bClkChange = TRUE;
					}
				}
			}
		}
	
// RevNo111121-002 Append End

	//実行前フラッシュ書き込み実行
	ferr = setGoWriteFlash(PROGCMD_GB);
// RevNo111121-002 Modify Start
	if(bClkChange == TRUE){
		// フラッシュ書き換え時のクロック切り替え設定を変更した場合
		// フラッシュ書き換え時のクロック切り替えを「禁止」に戻す
		ferr2 = SetDbgClkChg(RX_CLKCHANGE_DIS);
		if (ferr2 != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr2;
		} else if(ferr != FFWERR_OK){
			ferrEnd = ProtEnd();
			return ferr;
		}
	} else if(ferr != FFWERR_OK){
		// フラッシュ書き換え時のクロック切り替え設定を変更しなかった、かつferrがエラーの場合
			ferrEnd = ProtEnd();
			return ferr;
	} 
// RevNo111121-002 Modify End

	// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
		// ASP ON
		if (IsAspOn()) {
			// ユーザプログラム実行開始
			/* ASP機能ON */
			Start_Asp();
			// 通過ポイント用モニタプログラムを書き込むためSetSRMを実行
			pSrmData = GetSrmData();		// 指定ルーチン実行情報格納構造体情報取得
			ferr = DO_SetSRM(pSrmData);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
	}
#endif
	// RevRxE2No171004-001 Append End

	// RevRxE2No171004-001 Modify Line
	ferr = PROT_MCU_GO(ENABLE_BREAK);			// BFWMCUCmd_GOコマンド発行(ブレーク付き実行設定)
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	SetMcuRunState(TRUE);						// ユーザプログラム実行状態を"プログラム実行中"にする

	SetMcuRunCmd(PROGCMD_GB);	// GBコマンドによるプログラム実行

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * プログラムのリセット付き実行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RSTG(void)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	// RESTコマンド実行
	ferr = DO_REST();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// GBコマンド実行
	ferr = DO_GB(ADDR_SETOFF, 0);
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
 * プログラムのプログラムブレークポイント付き実行
 * @param eStartAddrSet 実行開始アドレスの設定有無
 * @param madrStartAddr 実行開始アドレス
 * @param madrBreakAddr 実行停止アドレス
 * @param bStepOverExec ステップオーバ処理からの実行情報
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo140515-008 Modify Line
FFWERR DO_GPB(enum FFWENM_STARTADDR_SET eStartAddrSet, MADDR madrStartAddr,	MADDR madrBreakAddr, BOOL bStepOverExec)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	DWORD*	pdwNowBMode;
	DWORD*	pdwNowBFactor;
	DWORD	dwNowBMode,dwNewBMode;
	DWORD	dwNowBFactor;
	FFW_PB_DATA*	pPb;

	FFWMCU_REG_DATA_RX*	pRegDataRX;
	enum FFWENM_PROGCMD_NO eGoCmd;
	FFWE20_STEP_DATA pStep;

	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;
#if defined(E2E2L)
	FFWE20_EINF_DATA	einfData;		// RevRxE2No171004-001 Append Line
#endif

	ProtInit();

	UpdateRunsetData();	// FFW 内部変数を更新

	pdwNowBMode = &dwNowBMode;
	pdwNowBFactor = &dwNowBFactor;
	GetBmMode(pdwNowBMode, pdwNowBFactor);	// 現在のブレークモード取得
	dwNowBMode = *pdwNowBMode;

	pPb = GetPbData();						// S/Wブレークポイント設定を取得

	pRegDataRX = GetRegData_RX();											// レジスタ情報取得
	if (eStartAddrSet == ADDR_SETON) {
		pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
	}else {
		madrStartAddr = pRegDataRX->dwRegData[REG_NUM_RX_PC];
	}

	// RevRxNo140515-008 Modify Start
	// GPBコマンド用ブレークモード設定
	MakeBreakMode(PROGCMD_GPB, &dwNewBMode);

	if (bStepOverExec == FALSE) {	// ステップオーバからの実行ではない場合
		ferr = PROT_MCU_SetRXXREG(pRegDataRX);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// OCDブレーク設定入力
		ferr = setOcdBreakMode(dwNewBMode);	// ブレークモードのハードウェア設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo140515-008 Modify End

	// SWブレークテーブル更新
	ferr = setSwBreakMode(dwNewBMode,PROGCMD_GPB,madrStartAddr);	// SWブレークの実行前処理
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo130308-001 Append Line
 	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	// RevRxNo130308-001 Append Line
	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// カバレッジ機能以外選択時
		// プログラム実行前のトレース動作設定
		ferr = setGoTrace(PROGCMD_GPB);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	// RevRxNo130308-001-007 Append Start
	} else {										// カバレッジ機能選択時
		ferr = SetGoTrace_CV();		// TBSR.TRFS設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo130308-001-007 Append End

	// イベント有効/無効設定
	ferr = setGoEvent(PROGCMD_GPB);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo140515-007 Modify Start
	if (isStepContinue() == FALSE) {	// 連続ステップ中でない場合
		// RevRxNo121029-003	Append Start
		// 実行前パフォーマンス機能設定
		ferr = setGoPpc(PROGCMD_GPB);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxNo121029-003	Append End
	}
	// RevRxNo140515-007 Modify End


	// 実行開始アドレスと実行停止アドレスが同一の場合
	if (madrStartAddr == madrBreakAddr) {
		// RevRxNo121029-003	Modify Start
		//シングルステップ実行
		//RevNo100715-013 Append Line
		eGoCmd = PROGCMD_GPB;
		ferr = mcuBrkStep(eGoCmd, &pStep);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxNo121029-003	Modify End

		if (pStep.eGoToRunState == STEP_GOTO_RUNSTATE) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		madrStartAddr = pStep.dwPc;
		//RevNo110426-01 Modify Line
		// GPB停止アドレスがSTEP後のアドレスと一致した場合は終了
		if (madrStartAddr == madrBreakAddr) {
			pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
			ferr = PROT_MCU_SetRXXREG(pRegDataRX);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			SetMcuRunCmd(PROGCMD_GPB_BRKSTEP);
			// ここで終了する場合は、PPC後処理が必要
			ferr = setGoAfterPpc();
			ferrEnd = ProtEnd();
			return ferr;
		}
		pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
		ferr = PROT_MCU_SetRXXREG(pRegDataRX);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxNo121029-003	Modify Start
		// STEP後継続して実行する場合、PPC処理を実施
		ferr = setGoStepPpc();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxNo121029-003	Modify End
	}

	//GPBで使用するブレークイベントを設定
	ferr = setGpbBrkEvent(madrBreakAddr);
	if(ferr != FFWERR_OK){
		ferrEnd = ProtEnd();
		return ferr;
	}

	//実行前フラッシュ書き込み実行
	ferr = setGoWriteFlash(PROGCMD_GPB);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	getEinfData(&einfData);
	if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
		// ASP ON
		if (IsAspOn()) {
			// ユーザプログラム実行開始
			/* ASP機能ON */
			Start_Asp();
		}
	}
#endif
	// RevRxE2No171004-001 Append End

	// RevRxE2No171004-001 Modify Line
	ferr = PROT_MCU_GO(DISABLE_BREAK);			// BFWMCUCmd_GOコマンド発行(ブレークなし実行設定)
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	SetMcuRunState(TRUE);						// ユーザプログラム実行状態を"プログラム実行中"にする

	SetMcuRunCmd(PROGCMD_GPB);	// GPBコマンドによるプログラム実行

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * プログラムの実行後ブレークポイント付き実行
 * @param eStartAddrSet 実行開始アドレスの設定有無
 * @param madrStartAddr 実行開始アドレス
 * @param madrBreakAddr 実行停止アドレス
 * @param pStep ステップ実行時のブレーク要因格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GPBA(enum FFWENM_STARTADDR_SET eStartAddrSet, MADDR madrStartAddr, MADDR madrBreakAddr, FFWE20_STEP_DATA* pStep)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	DWORD*	pdwNowBMode;
	DWORD*	pdwNowBFactor;
	DWORD	dwNowBMode,dwNewBMode;
	DWORD	dwNowBFactor;
	FFW_PB_DATA*	pPb;
	WORD	wOpecode;

	FFWMCU_REG_DATA_RX*	pRegDataRX;

	FFWRX_INST_DATA InstResult;

	//RevNo100715-014 Append Start
	BYTE	byEndian;
	//RevNo100715-014 Append End

	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;
#if defined(E2E2L)
	FFWE20_EINF_DATA	einfData;		// RevRxE2No171004-001 Append Line
#endif

	ProtInit();

	UpdateRunsetData();	// FFW 内部変数を更新

	pdwNowBMode = &dwNowBMode;
	pdwNowBFactor = &dwNowBFactor;
	GetBmMode(pdwNowBMode, pdwNowBFactor);	// 現在のブレークモード取得
	dwNowBMode = *pdwNowBMode;

	pPb = GetPbData();						// S/Wブレークポイント設定を取得

	pRegDataRX = GetRegData_RX();			// レジスタ情報取得

	// RevRxNo140515-007 Modify Start
	// GPBAコマンド用ブレークモード設定
	MakeBreakMode(PROGCMD_GPBA, &dwNewBMode);

	if (isStepContinue() == FALSE) {	// 連続ステップ中でない場合

		if (eStartAddrSet == ADDR_SETON) {
			pRegDataRX->dwRegData[REG_NUM_RX_PC] = madrStartAddr;
		}else {
			madrStartAddr = pRegDataRX->dwRegData[REG_NUM_RX_PC];
		}

		ferr = PROT_MCU_SetRXXREG(pRegDataRX);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// OCDブレーク設定入力
		ferr = setOcdBreakMode(dwNewBMode);	// ブレークモードのハードウェア設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// 実行前パフォーマンス機能設定
		ferr = setGoPpc(PROGCMD_GPBA);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	// SWブレークテーブル更新
	ferr = setSwBreakMode(dwNewBMode,PROGCMD_GPBA,madrStartAddr);	// SWブレークの実行前処理
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo130308-001 Append Line
 	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	// RevRxNo130308-001 Append Line
	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// カバレッジ機能以外選択時
		// プログラム実行前のトレース動作設定
		ferr = setGoTrace(PROGCMD_GPBA);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	// RevRxNo130308-001-007 Append Start
	} else {										// カバレッジ機能選択時
		ferr = SetGoTrace_CV();		// TBSR.TRFS設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo130308-001-007 Append End

	// イベント有効/無効設定
	ferr = setGoEvent(PROGCMD_GPBA);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo140515-007 Modify End

	//実行前フラッシュ書き込み実行
	ferr = setGoWriteFlash(PROGCMD_GPBA);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	//RevNo100715-014 Append Line
	ferr = GetEndianType2(pRegDataRX->dwRegData[REG_NUM_RX_PC],&byEndian);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	getEinfData(&einfData);
	if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
		// ASP ON
		if (IsAspOn()) {
			// ユーザプログラム実行開始
			/* ASP機能ON */
			Start_Asp();
		}
	}
#endif
	// RevRxE2No171004-001 Append End

	// BFWのGPBAコマンド発行(実行後PCブレーク設定)
	//RevNo100715-014 Modify Line
	ferr = PROT_MCU_GPBA(madrBreakAddr, pStep, &wOpecode,byEndian);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo140515-007 Append Start
	if (GetSsstState() == TRUE) {		// SSSTコマンドが発行されている場合
		SetStepContinueStartFlg();		// 連続ステップ開始フラグをTRUE設定
	}
	// RevRxNo140515-007 Append Start

	// プログラム停止中の場合
	if (pStep->eGoToRunState == STEP_NOT_GOTO_RUNSTATE) {
		SetMcuRunState(FALSE);						// ユーザプログラム実行状態を"プログラム停止中"にする

		// PC位置の命令コード解析
		ferr = DO_GetRXINSTCODE(madrBreakAddr, &InstResult);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// ブレークアドレス位置の命令コード解析実施
		if (InstResult.branch_type == BRANCH_SUB2) {
			pStep->eRetunCodeStepFlag = STEP_RTSCODE;
		} else {
			pStep->eRetunCodeStepFlag = STEP_NONRTSCODE;
		}
		if (InstResult.branch_type == BRANCH_SUB1 || InstResult.branch_type == BRANCH_BRK) {
			pStep->eSubCallCodeStepFlag = STEP_SUBCALLCODE;
		} else {
			pStep->eSubCallCodeStepFlag = STEP_NONSUBCALLCODE;
		}
		pStep->dwSubCallCodeSize = (DWORD)InstResult.byCmdLen;

		// RevRxNo140515-007 Modify Start
		pRegDataRX->dwRegData[REG_NUM_RX_PC] = pStep->dwPc;	// PC値を更新

		if (isStepContinue() == FALSE) {	// 連続ステップ中でない場合
			// PC値更新の為、レジスタ値取得
			ferr = PROT_MCU_GetRXXREG(REG_NUM_RX_ALL, pRegDataRX);	// RevNo120606-001 Modify Line
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			ferr = SetBreakEvEnable(TRUE);					// GPBA前の通過イベントブレーク設定を復帰
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			ferr = SetPreBreakEvEnable(TRUE);				// GPBA前の実行前PCブレーク設定を復帰
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			// PPC リセット中にパフォーマンスが動いてしまう対策
			ferr = setGoAfterPpc();						// 停止中なのでパフォーマンス無効
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
		// RevRxNo140515-007 Modify End

	} else {
		SetMcuRunState(TRUE);						// ユーザプログラム実行状態を"プログラム実行中"にする
		SetMcuRunCmd(PROGCMD_GPBA);					// GPBAコマンドによるプログラム実行
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * プログラム実行中の情報入手
 * @param pProg     ユーザプログラム実行中の各種情報格納構造体アドレス
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
FFWERR DO_GetRXPROG(FFWRX_PROG_DATA* pProg)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	FFWMCU_REG_DATA_RX*	pRegDataRX;

	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
#if defined(E2E2L)
	FFWE20_EINF_DATA	einfData;		// RevRxE2No171004-001 Append Line
#endif

	ProtInit();

	ferr = PROT_GetRXPROG(pProg);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// PROG取得では停止中、FFW内部では実行中
	if ((pProg->dwExecInfo == 0x00) && (GetMcuRunState() == TRUE)) {
		SetMcuRunState(FALSE);										// ユーザプログラム実行状態を"プログラム停止中"にする
		ferr = McuBreak_RX();										// ユーザプログラム停止後の処理を実行
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// リセット中のPPCが動くため、PPCをOFFにしておく PPC後処理実施
		ferr = setGoAfterPpc();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// PROG取得では停止中、FFW内部では停止中
	else if ((pProg->dwExecInfo == 0x00) && (GetMcuRunState() == FALSE)) {
		pRegDataRX = GetRegData_RX();
		pProg->dwPcInfo = pRegDataRX->dwRegData[REG_NUM_RX_PC];		// PC値をレジスタ値で上書き
	}
	// PROG取得では実行中、FFW内部では実行中
	else if ((pProg->dwExecInfo == 0x01) && (GetMcuRunState() == TRUE)) {
		// ホットプラグ設定状態,ID認証結果状態を取得
		bHotPlugState = getHotPlugState();
		bIdCodeResultState = getIdCodeResultState();

		// ホットプラグ設定状態でID認証結果設定状態の場合
		if (bHotPlugState == TRUE && bIdCodeResultState == TRUE) {
			SetMcuRunCmd(PROGCMD_GB);	// GBコマンドによるプログラム実行を設定
		}
	}
	// PROG取得では実行中、FFW内部では停止中
	else if ((pProg->dwExecInfo == 0x01) && (GetMcuRunState() == FALSE)) {
		SetMcuRunState(TRUE);										// ユーザプログラム実行状態を"プログラム実行中"にする
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	// ユーザプログラムが停止しているので、ユーザプログラム実行状態を"プログラム停止中"にする
	if (pProg->dwExecInfo == 0x00) {
		SetMcuRunState(FALSE);
		// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
		getEinfData(&einfData);
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			if (IsAspOn()) {	// E2でASP ONの場合
				Stop_Asp();			// E2拡張機能の動作停止
			}
		}
#endif
		// RevRxE2No171004-001 Append End
	}

	return ferr;
}

//=============================================================================
/**
 * プログラム実行停止
 *	DO_STOP関数内で、ユーザプログラム停止後の処理を行う。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_STOP(void)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
#if defined(E2E2L)
	FFWE20_EINF_DATA	einfData;		// RevRxE2No171004-001 Append Line
#endif

	ProtInit();

	ferr = PROT_MCU_STOP();
	if (ferr != FFWERR_OK) {	// ユーザプログラムが正常に停止しなかった場合
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	getEinfData(&einfData);
	if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
		if (IsAspOn()) {	// E2でASP ONの場合
			Stop_Asp();			// E2拡張機能の動作停止
		}
	}
#endif
	// RevRxE2No171004-001 Append End

	// ユーザプログラムが正常に停止した場合
	SetMcuRunState(FALSE);		// ユーザプログラム実行状態を"プログラム停止中"にする

	ferr = McuBreak_RX();	// ユーザプログラム停止後の処理を実行
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// リセット中のPPCが動くため、PPCをOFFにしておく PPC後処理実施
	ferr = setGoAfterPpc();
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
 * ブレーク要因の参照
 * @param dwBreakFact     ブレーク要因格納アドレス
 * @param eBrkTrcComb     イベントブレークの組合せ条件
 * @param byOrBrkFactEvPC PC通過イベント情報(イベントブレークでブレーク時)
 * @param byOrBrkFactEvOA オペランドアクセスイベント情報(イベントブレークでブレーク時)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetRXBRKF(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb, 
					BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	//RevNo100715-025 Append Start
	FFW_PB_DATA*	pPb;
	FFWMCU_REG_DATA_RX*	pRegDataRX;
	DWORD  dwBreakFactBuff;
	enum FFWENM_PROGCMD_NO eProgCmd;	// プログラム実行コマンド種別
	BOOL	bEvPreBrk;
	//RevNo100715-025 Append End
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	pMcuInfo = GetMcuInfoDataRX();	// MCU情報取得

	ProtInit();

	ferr = getBreakStatus_RX(dwBreakFact, eBrkTrcComb, byOrBrkFactEvPC, byOrBrkFactEvOA);

	//RevNo100715-025 Append Start
	// ブレーク要因の解析
	dwBreakFactBuff = *dwBreakFact;
	eProgCmd = GetMcuRunCmd();
	if(dwBreakFactBuff == BRKF_BREAKFACT_SW_BIT){
		// ブレーク要因がS/Wブレークの場合、F/Wで認識するS/Wブレーク以外は"Unknow Break"とする
		pRegDataRX = GetRegData_RX();
		pPb = GetPbData();						// S/Wブレークポイント設定を取得
		if(SearchBrkPoint(pRegDataRX->dwRegData[REG_NUM_RX_PC], pPb) == FALSE){
			dwBreakFactBuff &= ~(BRKF_BREAKFACT_SW_BIT);
			*dwBreakFact = dwBreakFactBuff;
		}
	}
	//RevNo100715-025 Append End

	//RevNo100715-024 Append Start
	//GBもしくはGPB実行後のブレーク要因が実行後PCブレークの場合
	//RevNo110426-01 Modify Line
	//GPBでS/Wブレークもしくは実行前PCブレークで停止することはないので、GBだけブレーク要因変更処理実施
	if ((dwBreakFactBuff == BRKF_BREAKFACT_AFTER_BIT) && (eProgCmd == PROGCMD_GB_BRKSTEP)){
		pRegDataRX = GetRegData_RX();
		pPb = GetPbData();						// S/Wブレークポイント設定を取得
		ferr = SearchEvPreBrkPoint(pRegDataRX->dwRegData[REG_NUM_RX_PC],&bEvPreBrk);
		if(ferr != FFWERR_OK){
			return ferr;
		}
		if(bEvPreBrk == TRUE) {
		// ブレークPC番地にPCブレークがある場合は、ブレーク要因を実行後ブレーク修正
			*dwBreakFact = BRKF_BREAKFACT_BEFORE_BIT;
		} else if(SearchBrkPoint(pRegDataRX->dwRegData[REG_NUM_RX_PC], pPb) == TRUE){
		// ブレークPC番地にS/Wブレークがある場合は、ブレーク要因をS/Wブレーク修正
			*dwBreakFact = BRKF_BREAKFACT_SW_BIT;
		}
	}
	//RevNo100715-024 Append End
	//RevNo110426-01 Append Start
	if ((dwBreakFactBuff == BRKF_BREAKFACT_AFTER_BIT) && (eProgCmd == PROGCMD_GPB_BRKSTEP)){
		*dwBreakFact = BRKF_BREAKFACT_BEFORE_BIT;
	}

	// RevNo110322-002	Append Start
	// FINEの場合ブレーク時に強制ブレークアサートするため、ブレーク要因に強制ブレークアサートが立ってしまう
	// 強制ブレークアサート以外の要因がある場合は、強制ブレークアサートを寝かせておく
	if(pMcuInfo->byInterfaceType == IF_TYPE_1){
		if((*dwBreakFact & ~BRKF_BREAKFACT_COMMAND_BIT) != 0){
			*dwBreakFact &= ~BRKF_BREAKFACT_COMMAND_BIT;
		}
	}
	// RevNo110322-002	Append End
	
	//RevNo110426-01 Append End
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * プログラムのステップ実行
 * @param pStep ステップ実行時のブレーク要因格納構造体アドレス
 * @param eStepCmd ステップ実行コマンド
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_STEP(FFWE20_STEP_DATA* pStep, enum FFWENM_STEPCMD eStepCmd)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	BOOL	bFfwStepReq;
	BOOL	bTgtReset;
	WORD	wOpecode;
	enum FFWENM_PROGCMD_NO eRunCmd;
	FFWMCU_REG_DATA_RX*	pRegDataRX;

	DWORD			dwPcAddr;
	DWORD			dwNewBMode;
	//RevNo100715-013 Append Line
	BYTE	byMskSet;
	// RevRxNo130308-001-007 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	UpdateRunsetData();						// FFW 内部変数を更新

	pRegDataRX = GetRegData_RX();			// レジスタ情報取得

	// RevRxNo140515-007 Modify Start
	ProtInit();

	UpdateRunsetData();						// FFW 内部変数を更新

	//RevNo100715-013 Append Line
	byMskSet = 0;
	if (eStepCmd == STEPCMD_STEP) {			// STEPコマンドによるステップ実行時
		eRunCmd = PROGCMD_STEP;
	//RevNo100715-013 Append Start
	} else if (eStepCmd == STEPCMD_STEP_MSK){	// STEPコマンドによるステップ実行時
		eRunCmd = PROGCMD_STEP;
		byMskSet = 1;
	//RevNo100715-013 Append End
	} else {								// STEPOVERコマンドによるステップ実行時
		eRunCmd = PROGCMD_STEPOVER;
	}

	SetMcuRunCmd(eRunCmd);					// STEP/STEPOVERコマンドによるプログラム実行

	dwPcAddr = pRegDataRX->dwRegData[REG_NUM_RX_PC];	// 現在のPCアドレス取得

	// STEPコマンド用ブレークモード設定
	MakeBreakMode(eRunCmd, &dwNewBMode);

	if (isStepContinue() == FALSE) {	// 連続ステップ中でない場合

		ferr = PROT_MCU_SetRXXREG(pRegDataRX);	// レジスタ情報設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// OCDブレーク設定入力
		ferr = setOcdBreakMode(dwNewBMode);	// ブレークモードのハードウェア設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// 実行前パフォーマンス機能設定
		ferr = setGoPpc(eRunCmd);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo140515-007 Modify End

	// SWブレークテーブル更新
	ferr = setSwBreakMode(dwNewBMode,PROGCMD_STEP,pRegDataRX->dwRegData[REG_NUM_RX_PC]);	// SWブレークの実行前処理
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo130308-001-007 Append Start
 	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// カバレッジ機能選択時
		ferr = SetGoTrace_CV();		// TBSR.TRFS設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo130308-001-007 Append End

	if (eRunCmd == PROGCMD_STEP) {
		//RevNo100715-013 Modify Line
		ferr = mcuStep(pRegDataRX, pStep, &bFfwStepReq, &wOpecode, &bTgtReset, STEP_BFWCTRL, byMskSet);		// シングルステップ実行
	} else {
		//RevNo100715-013 Modify Line
		ferr = mcuStepOver(pRegDataRX, pStep, &bFfwStepReq, &wOpecode, &bTgtReset, STEP_BFWCTRL,byMskSet);	// オーバステップ実行
	}
	if (ferr != FFWERR_OK) {
		ProtEnd();
		return ferr;
	}

	// RevRxNo140515-007 Append Start
	if (GetSsstState() == TRUE) {		// SSSTコマンドが発行されている場合
		SetStepContinueStartFlg();		// 連続ステップ開始フラグをTRUE設定
	}
	// RevRxNo140515-007 Append Start


	if (pStep->eGoToRunState == STEP_NOT_GOTO_RUNSTATE) {	// ステップ実行が完了した場合

		// RevRxNo140515-007 Modify Start
		if (eRunCmd == PROGCMD_STEP) {						// シングルステップ実行の場合
			setStepRunState(FALSE);							// シングルステップ実行からRUN状態への移行なし
		}
		pRegDataRX = GetRegData_RX();											// レジスタ情報取得
		pRegDataRX->dwRegData[REG_NUM_RX_PC] = pStep->dwPc;

		if (isStepContinue() == FALSE) {	// 連続ステップ中でない場合

			ferr = SetBreakEvEnable(TRUE);					// STEP前の通過イベントブレーク設定を復帰
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			ferr = SetPreBreakEvEnable(TRUE);				// STEP前の実行前PCブレーク設定を復帰
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			// リセット中PPCが動くため、PPCをOFFにしておく PPC後処理実施
			ferr = setGoAfterPpc();
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			//ステップ実行後に汎用レジスタを毎回取得
			ferr = PROT_MCU_GetRXXREG(REG_NUM_RX_ALL, pRegDataRX);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			SetRegData_RX(pRegDataRX);
		}
		// RevRxNo140515-007 Modify End

	} else {												// ステップ実行が完了しなかった場合(RUN状態へ移行した場合)
		if (eRunCmd == PROGCMD_STEP) {						// シングルステップ実行の場合
			setStepRunState(TRUE);							// シングルステップ実行後RUN状態へ移行したことを指示
		}
		SetMcuRunState(TRUE);								// ユーザプログラム実行状態を"プログラム実行中"にする
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
// 2008.11.19 MODIFY_END_E20RX600 }


// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ開始処理
 * @param	なし
 * @retval	FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetSSST(void)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ferr = PROT_MCU_SetSSST();	// 連続ステップ終了通知
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	return ferr;
}
// RevRxNo140515-007 Append End

// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ終了処理
 * @param	なし
 * @retval	FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetSSEN(void)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	FFWMCU_REG_DATA_RX*	pRegDataRX;

	pRegDataRX = GetRegData_RX();

	// SSSTコマンド未発行状態に設定
	ClrSsstState();

	ferr = PROT_MCU_SetSSEN();	// 連続ステップ終了通知
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	if (GetMcuRunState() == FALSE) {	// プログラム停止中の場合
		// 連続ステップ無効設定に伴い、OCDレジスタ設定値を復帰する

		ferr = setGoAfterPpc();				// パフォーマンス機能無効設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		ferr = SetBreakEvEnable(TRUE);		// ステップ処理前の通過イベントブレーク設定を復帰
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		ferr = SetPreBreakEvEnable(TRUE);	// ステップ処理前の実行前PCブレーク設定を復帰
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// 連続ステップ中はR1、R15、PC値しかMCUから取得しないため、
		// 連続ステップ無効時に全CPUレジスタを取得する
		ferr = PROT_MCU_GetRXXREG(REG_NUM_RX_ALL, pRegDataRX);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

	}

	clrStepContinueStartFlg();	// 連続ステップ実行開始フラグをクリアする

	return ferr;
}
// RevRxNo140515-007 Append End

///////////////////////////////////////////////////////////////////////////////
// FFW内部関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ユーザプログラム実行状態の取得
 * ※MCUの実行状態とFFW内部変数のズレもここで修正
 * @param なし
 * @retval TRUE ユーザプログラム実行中
 * @retval FALSE ユーザプログラム停止中
 */
//=============================================================================
BOOL IsMcuRun(void)
{
	FFWERR					ferr;
	FFWRX_PROG_DATA			ProgData;
	enum FFWRXENM_STAT_MCU	eStatMcu;
	BOOL					bRet;
	// RevRxNo120910-001 Modify Line
	DWORD					dwStatKind = 0;
	FFWRX_STAT_SFR_DATA		pStatSFR;
	FFWRX_STAT_JTAG_DATA	pStatJTAG;
	FFWRX_STAT_FINE_DATA	pStatFINE;
	FFWRX_STAT_EML_DATA		pStatEML;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
#if defined(E2E2L)
	FFWE20_EINF_DATA Einf;		// RevRxE2No171004-001 Append Line
#endif

	ProtInit();
#if defined(E2E2L)
	getEinfData(&Einf);			// RevRxE2No171004-001 Append Line
#endif

	// V.1.02 新デバイス対応 Append Line
	pMcuInfoData = GetMcuInfoDataRX();

	bRet = getFirstConnectInfo();			// 初回起動時に接続した状態を取得
	if (bRet == FALSE) {
		return	s_bMcuRunState;
	}

	// STATコマンドで実行状態をチェック
	// V.1.02 新デバイス対応 Modify Start
	if( pMcuInfoData->byInterfaceType == IF_TYPE_0 ){			// JTAG指定時
		// RevRxNo120910-001 Modify Line
		dwStatKind = STAT_KIND_JTAG;		// JTAG情報のみ取得
	}else if( pMcuInfoData->byInterfaceType == IF_TYPE_1 ){		// FINE指定時
		// RevRxNo120910-001 Modify Line
		dwStatKind = STAT_KIND_FINE;		// FINE情報のみ取得
	}
	// RevRxNo120910-001 Modify Line
	ferr = GetStatData(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
	// V.1.02 新デバイス対応 Modify End
	if (ferr != FFWERR_OK) {
		return FALSE;
	}

	// PROGコマンドで実行状態をチェック
	// ※PROT_MCU_GetRXSTAT()呼び出しによってブレークを検出し、
	//   McuBreak_RX()によってブレーク後の処理が呼ばれる。
	//   この時、PC値を取得するが、S/Wブレークでブレークしたときだけは
	//   BFWのPROGコマンド発行によって、PC値を-1する必要あり
	//   そのために、 ここでPROT_GetRXPROG()を呼び出す。
	//   DO_GetRXPROG()の中では、SetMcuRunState(FALSE)の呼び出しによって
	//   すでにPC値取得済みとみなされ、-1されていないままのPC値が適用されてしまっていた。

	ferr = PROT_GetRXPROG(&ProgData);
	if (ferr != FFWERR_OK) {
		return FALSE;
	}

	// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	if (Einf.wEmuStatus == EML_E2) {		// E2エミュレータの場合
		if (IsFamilyAspFuncEna(RF_ASPFUNC_MONEV)) {		// 通過ポイントが登録された状態の場合
			// GetStatData()で取得したプログラム実行状態と PROT_GetRXPROG()で取得したプログラム実行状態が異なる場合がある
			// そのため、プログラム実行状態であるPROT_GetRXPROG()の結果をpStatJTAG.eStatExecに反映する必要がある。
			// ただし、GetStatData()が実行状態でPROT_GetRXPROG()が停止状態ということは今までにもあったはずなので、
			// GetStatData()が停止状態でPROT_GetRXPROG()が実行状態の場合のみ反映するようにする。
			if (ProgData.dwExecInfo == 0x01) {			// プログラム実行状態
				if (pMcuInfoData->byInterfaceType == IF_TYPE_0) {		// JTAG指定時
					pStatJTAG.eStatExec = FFWRX_STAT_EXEC_RUN;			// プログラム実行状態に再設定
				}
				else if (pMcuInfoData->byInterfaceType == IF_TYPE_1) {	// FINE指定時
					pStatFINE.eStatExec = FFWRX_STAT_EXEC_RUN;			// プログラム実行状態に再設定
				}
			}
		}
	}
#endif
	// RevRxE2No171004-001 Append End

	if( pMcuInfoData->byInterfaceType == IF_TYPE_0 ){			// JTAG指定時
		if ((pStatJTAG.eStatExec == 0x00) && (s_bMcuRunState == TRUE)) {			// STAT取得では停止中、FFW内部では実行中
			SetMcuRunState(FALSE);													// ユーザプログラム実行状態を"プログラム停止中"にする
			ferr = McuBreak_RX();													// ユーザプログラム停止後の処理を実行
			if (ferr != FFWERR_OK) {
				ProtEnd();
				return ferr;
			}
		} else if ((pStatJTAG.eStatExec == 0x00) && (s_bMcuRunState == FALSE)) {	// STAT取得では停止中、FFW内部では停止中
			// 何もしない

		} else if ((pStatJTAG.eStatExec == 0x01) && (s_bMcuRunState == TRUE)) {	// STAT取得では実行中、FFW内部では実行中
			// 累積AND、状態遷移の状態チェック

		} else if ((pStatJTAG.eStatExec == 0x01) && (s_bMcuRunState == FALSE)) {	// STAT取得では実行中、FFW内部では停止中
			SetMcuRunState(TRUE);													// ユーザプログラム実行状態を"プログラム実行中"にする
		}

		if (pStatJTAG.eStatExec == 0x00) {		// ユーザプログラムが停止しているので、ユーザプログラム実行状態を"プログラム停止中"にする
			SetMcuRunState(FALSE);
		}
	}else if( pMcuInfoData->byInterfaceType == IF_TYPE_1 ){		// FINE指定時
		if ((pStatFINE.eStatExec == 0x00) && (s_bMcuRunState == TRUE)) {			// STAT取得では停止中、FFW内部では実行中
			SetMcuRunState(FALSE);													// ユーザプログラム実行状態を"プログラム停止中"にする
			ferr = McuBreak_RX();													// ユーザプログラム停止後の処理を実行
			if (ferr != FFWERR_OK) {
				ProtEnd();
				return ferr;
			}
		} else if ((pStatFINE.eStatExec == 0x00) && (s_bMcuRunState == FALSE)) {	// STAT取得では停止中、FFW内部では停止中
			// 何もしない

		} else if ((pStatFINE.eStatExec == 0x01) && (s_bMcuRunState == TRUE)) {	// STAT取得では実行中、FFW内部では実行中
			// 累積AND、状態遷移の状態チェック

		} else if ((pStatFINE.eStatExec == 0x01) && (s_bMcuRunState == FALSE)) {	// STAT取得では実行中、FFW内部では停止中
			SetMcuRunState(TRUE);													// ユーザプログラム実行状態を"プログラム実行中"にする
		}

		if (pStatFINE.eStatExec == 0x00) {		// ユーザプログラムが停止しているので、ユーザプログラム実行状態を"プログラム停止中"にする
			SetMcuRunState(FALSE);
		}
	}
	return	s_bMcuRunState;

}

// RevRxE2LNo141104-001 Delete:IsMcuStateNormal()

//=============================================================================
/**
 * ブレークモードの設定
 * @param eRunCmd プログラム実行コマンド種別
 * @param pdwNewBMode 新規設定ブレークモード格納変数へのポインタ
 * @retval なし
 */
//=============================================================================
void MakeBreakMode(enum FFWENM_PROGCMD_NO eRunCmd, DWORD* pdwNewBMode)
{
	DWORD	dwNowBMode;		// 現在設定されているブレークモード格納変数
	DWORD	dwNowBFactor;

	GetBmMode(&dwNowBMode, &dwNowBFactor);	// 現在設定されているブレークモード、ブレーク要因を取得

	*pdwNewBMode = 0x0000;

	switch (eRunCmd) {
	case PROGCMD_GO:	// GOコマンドの場合
	case PROGCMD_GPB:	// GPBコマンドの場合
	case PROGCMD_GPBA:	// GPBAコマンドの場合
	case PROGCMD_STEP:	// STEPコマンドの場合
	case PROGCMD_STEPOVER:	// STEPOVERコマンドの場合
		*pdwNewBMode &= (DWORD)(~BMODE_SWB_BIT);	// ソフトウェアブレーク禁止
		*pdwNewBMode &= (DWORD)(~BMODE_EB_BIT);		// イベントブレーク禁止
		*pdwNewBMode &= (DWORD)(~BMODE_PRE_BIT);	// オンチップブレーク禁止
		break;

	case PROGCMD_GB:	// GBコマンドの場合
//	case PROGCMD_RSTG:	// RSTGコマンドの場合　→RSTG自体はDO_REST()→DO_GB()のためコメントにする。
		*pdwNewBMode |= (dwNowBMode & BMODE_SWB_BIT);			// ソフトウェアブレーク条件設定(BMコマンド内容が有効)
		*pdwNewBMode |= (dwNowBMode & BMODE_EB_BIT);			// イベントブレーク条件設定(BMコマンド内容が有効)
		*pdwNewBMode |= (dwNowBMode & BMODE_OCB_BIT);			// オンチップブレーク条件設定(BMコマンド内容が有効)
		break;
	}

	return;
}

//=============================================================================
/**
 * ユーザプログラム実行状態の設定
 * @param ユーザプログラム実行状態(TRUE:ユーザプログラム実行中/FALSE:ユーザプログラム停止中)
 * @retval なし 
 */
//=============================================================================
void SetMcuRunState(BOOL bState)
{
	s_bMcuRunState = bState;
}

//=============================================================================
/**
 * ユーザプログラム実行状態の取得
 * ※MCUの実行状態とFFW内部変数のズレは修正しない
 * @param なし
 * @retval TRUE ユーザプログラム実行中
 * @retval FALSE ユーザプログラム停止中
 */
//=============================================================================
BOOL GetMcuRunState(void)
{
	return	s_bMcuRunState;
}

//=============================================================================
/**
 * プログラム実行コマンド種別の取得
 *   最後に実行したプログラム実行コマンドを取得する。
 * @param なし
 * @retval プログラム実行コマンド種別
 */
//=============================================================================
enum FFWENM_PROGCMD_NO GetMcuRunCmd(void)
{
	return s_eMcuRunCmd;
}

//=============================================================================
/**
 * プログラム実行コマンド種別の設定
 * @param eRunCmd プログラム実行コマンド種別
 * @retval なし
 */
//=============================================================================
void SetMcuRunCmd(enum FFWENM_PROGCMD_NO eRunCmd)
{
	s_eMcuRunCmd = eRunCmd;
}


//=============================================================================
// Ver.1.01 2010/08/17 SDS T.Iwata
/**
 * 内蔵ROM無効拡張モードの状態の取得
 * (関数名を変更 IsPmodeExMemDis()→SetPmodeInRomReg2Flg())
 * ※プロセッサモード状態を取得し、内蔵ROM無効拡張モードフラグを設定する。
 * 　本関数は、以下のタイミングで発行する必要がある。
 * 　・メモリアクセスが発生するFFW I/Fの先頭
 * 　・内蔵フラッシュの書き換えが発生するFFW I/Fの先頭
 * 　・内蔵ROM有効/無効状態が変化するタイミング
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetPmodeInRomReg2Flg(void)
{

	// V.1.02 覚書 No.25　SetPmodeInRomReg2Flg()処理修正 Modify Start
	FFWERR	ferr = FFWERR_OK;
	BOOL	bRomEna,bExtBus;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;	// RevRxNo130730-005 Append Line

	pMcuInfo = GetMcuInfoDataRX();	// RevRxNo130730-005 Append Line

	// RevRxNo130730-005 Modify Start
	if ((s_bReadPmodeInRomDisRegFlg == TRUE) || (GetMcuRunState() == TRUE) || (pMcuInfo->byInterfaceType == IF_TYPE_1)) {
		// 以下(a)～(c)のいずれかに該当する場合、内蔵ROM無効状態を確認する。
		// (a)内蔵ROM無効拡張モード状態参照レジスタリードフラグがTRUE
		// (b)ユーザプログラム実行中
		// (c)FINE I/F
		ferr = GetMcuRomExtBusStatInfo(&bRomEna,&bExtBus);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// V.1.02 覚書 No.25　SetPmodeInRomReg2Flg()処理修正 Modify End

		// 内蔵ROM無効拡張モードの場合、内蔵ROM無効拡張モードフラグをセット
		// V.1.02 覚書 No.25　SetPmodeInRomReg2Flg()処理修正 Modify Line
		//RevNo110316-001 Modify Line
		if (bRomEna == FALSE){
			SetPmodeInRomDisFlg( TRUE );
		}
		// 内蔵ROM無効拡張モード以外の場合、内蔵ROM無効拡張モードフラグをクリア
		else{
			SetPmodeInRomDisFlg( FALSE );
		}

		if (GetMcuRunState() == FALSE) {
			// プログラム停止中の場合、内蔵ROM無効拡張モード状態参照レジスタリードフラグをクリア
			s_bReadPmodeInRomDisRegFlg = FALSE;
		}
	}
	// RevRxNo130730-005 Modify End

	return	FFWERR_OK;

}

// RevRxNo130730-005 Append Start
//=============================================================================
/**
 * 内蔵ROM無効拡張モード状態参照レジスタリードフラグの設定
 * @param 内蔵ROM無効拡張モード状態参照レジスタリードフラグ値
 *			TRUE：内蔵ROM無効拡張モード状態参照用レジスタリード必要
 *			FALSE：内蔵ROM無効拡張モード状態参照用レジスタリード不要
 * @retval なし 
 */
//=============================================================================
void SetReadPmodeInRomDisRegFlg(BOOL bFlg)
{
	s_bReadPmodeInRomDisRegFlg = bFlg;
}
// RevRxNo130730-005 Append End

//=============================================================================
// Ver.1.01 2010/08/17 SDS T.Iwata
/**
 * 内蔵ROM無効拡張モードフラグの設定
 * 関数名変更 SetPmodeInRomFlg() → SetPmodeInRomDisFlg()
 * @param 内蔵ROM無効拡張モード状態(TRUE:内蔵ROM無効/FALSE:内蔵ROM有効)
 * @retval なし 
 */
//=============================================================================
void SetPmodeInRomDisFlg(BOOL bState)
{
	s_bPmodeInRomDisFlg = bState;
}

//=============================================================================
// Ver.1.01 2010/08/17 SDS T.Iwata
/**
 * 内蔵ROM無効拡張モードフラグの取得
 * 関数名変更 GetPmodeExMemDis()→GetPmodeInRomDisFlg()
 * @param なし
 * @retval TRUE 内蔵ROM無効の場合
 * @retval FALSE 内蔵ROM有効の場合
 */
//=============================================================================
BOOL GetPmodeInRomDisFlg(void)
{
	return	s_bPmodeInRomDisFlg;
}

//=============================================================================
/**
 * プログラム実行関連コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Prog(void)
{
	s_bMcuRunState = FALSE;				// プログラム実行状態格納変数(停止中)

	s_bStepRunState = FALSE;			// シングルステップ実行からのユーザプログラム実行状態

	//RevNo100715-014 Append Line
	s_dwGPBEvNo = 0;

	//RevRxNo121106-001 Append Line
	s_bPmodeInRomDisFlg = FALSE;

	// RevRxNo130730-005 Append Line
	s_bReadPmodeInRomDisRegFlg = TRUE;

	// RevRxNo140515-007 Append Line
	s_bStepContinueStart = FALSE;		// 連続ステップ実行を開始していない(1回目のステップを実行していない)
}

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * シングルステップ実行からのユーザプログラム実行状態の設定
 * @param シングルステップ実行からのユーザプログラム実行状態
 *			(TRUE:シングルステップ実行からユーザプログラム実行状態へ移行した
 *			 FALSE:シングルステップ実行からユーザプログラム実行状態へ移行していない)
 * @retval なし 
 */
//=============================================================================
static void setStepRunState(BOOL bState)
{
	s_bStepRunState = bState;
}

//=============================================================================
/**
 * シングルステップ実行からのユーザプログラム実行状態の取得
 * @param なし
 * @retval TRUE シングルステップ実行からユーザプログラム実行状態へ移行した
 * @retval FALSE シングルステップ実行からユーザプログラム実行状態へ移行していない
 */
//=============================================================================
static BOOL getStepRunState(void)
{
	return s_bStepRunState;
}

//=============================================================================
/**
 * MCUのインストラクションステップ実行
 * @param pRegData レジスタ情報格納構造体アドレス
 * @param pStep ステップ実行時のブレーク要因格納構造体アドレス
 * @param pbFfwStepReq FFWによるステップ再実行要求フラグ格納アドレス
 * @param pwOpecode 命令コード2バイト格納バッファアドレス
 * @param pbTgtReset RUN中のターゲットリセット発生状態格納アドレス
 * @param eStepCtrl シングルステップ制御(STEP_BFWCTRL/STEP_FFWCTRL)
 * @retval FFWエラーコード
 */
//=============================================================================
//RevNo100715-013 Modify Line
static FFWERR mcuStep(FFWMCU_REG_DATA_RX* pRegData, FFWE20_STEP_DATA* pStep, BOOL* pbFfwStepReq, WORD* pwOpecode, BOOL* pbTgtReset, enum FFWENM_STEPCTRL eStepCtrl,BYTE byMskSet)
{
	FFWERR	ferr;
	enum FFWENM_STEPCMD	eStepCmd;

	FFWRX_INST_DATA InstResult;

	//RevNo100715-013 Append Line
	BYTE	byEndian;

	ferr = setGoWriteFlash(PROGCMD_STEP);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//RevNo100715-013 Append Line
	//実行PCアドレスのエンディアン情報取得
	ferr = GetEndianType2(pRegData->dwRegData[REG_NUM_RX_PC],&byEndian);		// RevNo120606-001 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	eStepCmd = STEPCMD_STEP;
	//RevNo100715-013 Modify Line
	ferr = PROT_MCU_STEP(pStep, pbFfwStepReq, pwOpecode, pbTgtReset, eStepCtrl, eStepCmd, byEndian, byMskSet);	// BFWMCUCmd_STEPコマンド発行
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// 実行停止中の場合は命令解析を実施
	if (pStep->eGoToRunState == STEP_NOT_GOTO_RUNSTATE) {
		// PC位置の命令コード解析
		ferr = DO_GetRXINSTCODE(pRegData->dwRegData[REG_NUM_RX_PC], &InstResult);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		if (InstResult.branch_type == BRANCH_SUB2) {
			pStep->eRetunCodeStepFlag = STEP_RTSCODE;
		} else {
			pStep->eRetunCodeStepFlag = STEP_NONRTSCODE;
		}
		if (InstResult.branch_type == BRANCH_SUB1 || InstResult.branch_type == BRANCH_BRK) {
			pStep->eSubCallCodeStepFlag = STEP_SUBCALLCODE;
		} else {
			pStep->eSubCallCodeStepFlag = STEP_NONSUBCALLCODE;
		}
		pStep->dwSubCallCodeSize = (DWORD)InstResult.byCmdLen;
	}
	
	return FFWERR_OK;
}

//=============================================================================
/**
 * MCUのオーバステップ実行
 * @param pRegData レジスタ情報格納構造体アドレス
 * @param pStep ステップ実行時のブレーク要因格納構造体アドレス
 * @param pbFfwStepReq FFWによるステップ再実行要求フラグ格納アドレス
 * @param pwOpecode 命令コード2バイト格納バッファアドレス
 * @param pbTgtReset RUN中のターゲットリセット発生状態格納アドレス
 * @param eStepCtrl シングルステップ制御(STEP_BFWCTRL/STEP_FFWCTRL)
 * @retval FFWエラーコード
 */
//=============================================================================
//RevNo100715-013 Modify Line
static FFWERR mcuStepOver(FFWMCU_REG_DATA_RX* pRegData, FFWE20_STEP_DATA* pStep, BOOL* pbFfwStepReq, WORD* pwOpecode, BOOL* pbTgtReset, enum FFWENM_STEPCTRL eStepCtrl,BYTE byMskSet)
{
	FFWERR	ferr;
	enum FFWENM_STEPCMD	eStepCmd;
	// RevRxNo140515-008 Delete
	FFWRX_INST_DATA InstResult;
	DWORD	dwCmdLen;
	DWORD	dwStopAddr;
	BOOL	bStepOverExec;				// RevRxNo140515-008 Append Line
	FFWRX_PROG_DATA	ProgData;
	FFWMCU_REG_DATA_RX*	pRegDataRX;		// RevRxNo140515-008 Append Line

	pRegDataRX = GetRegData_RX();		// RevRxNo140515-008 Append Line

	//RevNo100715-013 Modify Line
	BYTE	byEndian;

	// PC位置の命令コード解析
	ferr = DO_GetRXINSTCODE(pRegData->dwRegData[REG_NUM_RX_PC], &InstResult);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	dwCmdLen = (DWORD)InstResult.byCmdLen;										// 命令コードのサイズ取得

	// サブルーチン分岐命令の場合
	if (InstResult.branch_type == BRANCH_SUB1) {
		dwStopAddr = pRegData->dwRegData[REG_NUM_RX_PC] + dwCmdLen;					// 停止アドレスをセット

		// RevRxNo140515-008 Delete

		// RevRxNo140515-008 Modify Start
		bStepOverExec = TRUE;	// ステップオーバからの実行のため、実行情報をTRUEに設定
		ferr = DO_GPB(ADDR_SETON, pRegData->dwRegData[REG_NUM_RX_PC], dwStopAddr, bStepOverExec);	// GPBコマンド発行
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevRxNo140515-008 Modify End
		ferr = mcuWaitBreak_RX(&ProgData);											// プログラム停止待ち
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		if (ProgData.dwExecInfo == 0x00) {											// プログラムが停止した場合
			// RevRxNo140515-008 Modify Start
			ferr = McuBreak_RX();				// ユーザプログラム停止後の処理を実行
			if (ferr != FFWERR_OK) {
				ProtEnd();
				return ferr;
			}

			// ステップオーバー実行結果をセット
			pStep->dwPc = pRegDataRX->dwRegData[REG_NUM_RX_PC];
			pStep->eRetunCodeStepFlag = STEP_NONRTSCODE;
			pStep->eSubCallCodeStepFlag = STEP_SUBCALLCODE;
			pStep->dwSubCallCodeSize = dwCmdLen;
			pStep->eGoToRunState = STEP_NOT_GOTO_RUNSTATE;
			SetMcuRunState(FALSE);				// ユーザプログラム実行状態を"プログラム停止中"にする
			// RevRxNo140515-008 Modify End

		} else {
			pStep->eGoToRunState = STEP_GOTO_RUNSTATE;
		}
	}
	// サブルーチン分岐命令でない場合、シングルステップ
	else {
		// フラッシュROMキャッシュ変更ありの場合、フラッシュROMへの書込み実施(サブルーチン分岐命令時はDO_GPB内で実施)
		ferr = setGoWriteFlash(PROGCMD_STEP);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		//RevNo100715-013 Append Line
		ferr = GetEndianType2(pRegData->dwRegData[REG_NUM_RX_PC],&byEndian);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		eStepCmd = STEPCMD_STEP;
		//RevNo100715-013 Modify Line
		ferr = PROT_MCU_STEP(pStep, pbFfwStepReq, pwOpecode, pbTgtReset, eStepCtrl, eStepCmd, byEndian, byMskSet);	// BFWMCUCmd_STEPコマンド発行
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (InstResult.branch_type == BRANCH_SUB2) {
			pStep->eRetunCodeStepFlag = STEP_RTSCODE;
		} else {
			pStep->eRetunCodeStepFlag = STEP_NONRTSCODE;
		}
		if (InstResult.branch_type == BRANCH_BRK) {
			pStep->eSubCallCodeStepFlag = STEP_SUBCALLCODE;
		} else {
			pStep->eSubCallCodeStepFlag = STEP_NONSUBCALLCODE;
		}
		pStep->dwSubCallCodeSize = dwCmdLen;
	}
	
	return FFWERR_OK;
}

// 2008.9.19 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * プログラム実行停止後の処理(RX用)
 * @param なし
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
FFWERR McuBreak_RX(void)
{
	FFWERR	ferr;
	FFWMCU_REG_DATA_RX	regData;
	FFWRX_EV_ADDR EvAddr[EV_ADDR_MAX_RX];
	DWORD*	pdwNowBMode;
	DWORD*	pdwNowBFactor;
	DWORD	dwNowBMode;
	DWORD	dwNowBFactor;
	FFW_PB_DATA*	pPb;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	DWORD dwPreBrk = 0;
	DWORD dwExecPcSet = 0;
	DWORD dwOpcSet = 0;

	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;

	BYTE	byMode;
	DWORD	dwPreBrkCombi;

	// RevNo110825-01 Append line
	FFWRX_EVCNT_DATA* pevCnt;

	// レジスタ値取得
	ferr = PROT_MCU_GetRXXREG(REG_NUM_RX_ALL, &regData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	SetRegData_RX(&regData);

	pPb = GetPbData();									// S/Wブレークポイント設定を取得

	pDbgData = GetDbgDataRX();							// デバッグ情報取得

	if (GetMcuRunCmd() == PROGCMD_GPBA) {				// GPBAコマンドでの実行の場合
		ferr = SetBreakEvEnable(TRUE);					// GPB前の通過イベントブレーク設定を復帰
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// RevNo120131-001 Delete
		// 実行前PCブレーク設定を復帰 
		// →GetRegEvEnable()内でもEVEPCPREE復帰を実施するので削除

		// イベントブレークを有効
		GetRegEvEnable(&dwPreBrk, &dwExecPcSet, &dwOpcSet);
		ferr = SetRegEvEnable(dwPreBrk, dwExecPcSet, dwOpcSet);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

	}

	if (GetMcuRunCmd() == PROGCMD_GPB) {				// GPBコマンドでの実行の場合
		// RevNo120131-001 Delete
		// 設定情報を復帰 → 内部管理変数管理が複雑になるので削除

		//RevNo100715-014 Modify Start
		//変更した分だけ元に戻す
		GetEvAddrInfo(s_dwGPBEvNo, &EvAddr[s_dwGPBEvNo]);
		ferr = SetExecPcEv(s_dwGPBEvNo, EvAddr[s_dwGPBEvNo].dwAddr);	// 命令実行イベントの設定情報を復帰
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		//RevNo100715-014 Modify End

		// RevNo110825-01 Append Start
		// イベントカウント回数指定の復帰
		if(SearchEvCnt((enum FFWRX_EV)s_dwGPBEvNo)){
			pevCnt = GetEvCntInfo();
			if(pevCnt->bEnable == TRUE){
				ferr = SetEvCnt(pevCnt);
				if(ferr != FFWERR_OK){
					return ferr;
				}
			}
		}
		// RevNo110825-01 Append End

		// RevNo120131-001 Delete
		// 組み合わせチェック削除 COMB実行時点でチェックしているので、不要

		// RevNo120131-001 Delete
		// SetEvBreakData()はSetBreakEvEnable()と同じことをしているので削除


		ferr = SetBreakEvEnable(TRUE);					// GPB前の通過イベントブレーク設定を復帰
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// RevNo120131-001 Delete
		// 	実行前PCブレーク設定を復帰 
		// →GetRegEvEnable()内でもEVEPCPREE復帰を実施するので削除
		
		GetRegEvEnable(&dwPreBrk, &dwExecPcSet, &dwOpcSet);
		ferr = SetRegEvEnable(dwPreBrk, dwExecPcSet, dwOpcSet);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	if (GetMcuRunCmd() == PROGCMD_GB) {					// GBコマンドでの実行の場合
		// ホットプラグ設定状態,ID認証結果状態を取得
		bHotPlugState = getHotPlugState();
		bIdCodeResultState = getIdCodeResultState();

		// RevNo120131-001 Modify Start
		// ホットプラグ設定状態でID認証結果設定状態の場合
		if ((bHotPlugState == TRUE) && (bIdCodeResultState == TRUE)) {
			setHotPlugState(FALSE);			// ホットプラグ設定状態を未設定に変更
			// RevRxNo140515-012 Append Line
			setHotPlgStartFlg(FALSE);		// ホットプラグ起動フラグをFALSEに設定
			// RevRxNo121106-001 Delete(ID認証結果設定状態を未設定にする処理を削除)
		}
		
		pdwNowBMode = &dwNowBMode;
		pdwNowBFactor = &dwNowBFactor;
		GetBmMode(pdwNowBMode, pdwNowBFactor);			// 現在のブレークモード取得
		dwNowBMode = *pdwNowBMode;

		if ((dwNowBMode & BMODE_EB_BIT) != BMODE_EB_BIT) {		// 通過イベントブレーク無効時
			ferr = SetBreakEvEnable(TRUE);						// GB前の通過イベントブレーク設定を復帰
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		if ((dwNowBMode & BMODE_PRE_BIT) != BMODE_PRE_BIT) {	// 実行前PCブレーク無効時
			ferr = SetPreBreakEvEnable(TRUE);					// GB前の実行前PCブレーク設定を復帰
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// RevNo120131-001 Modify End
	}

	if (GetMcuRunCmd() == PROGCMD_GO) {					// GOコマンドでの実行の場合

		ferr = SetBreakEvEnable(TRUE);					// GO前の通過イベントブレーク設定を復帰
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevNo120131-001 Delete
		// 実行前PCブレーク設定を復帰 
		// →GetRegEvEnable()内でもEVEPCPREE復帰を実施するので削除

		// イベント有効化(EVEPCPREE、EVEPCPE、EVEOPE)
		GetRegEvEnable(&dwPreBrk, &dwExecPcSet, &dwOpcSet);
		ferr = SetRegEvEnable(dwPreBrk, dwExecPcSet, dwOpcSet);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

	}

	// シングルステップ実行時に2秒経過してRUN状態へ遷移した場合、ブレーク時にレジスタ値復帰処理を行う。
	if (getStepRunState() == TRUE) {
		byMode = 0x01;	//取得モード
		if(EscEvPreBrkPoint(byMode, &dwPreBrkCombi) == TRUE) {
			// 実行前PCブレーク組合せを退避済、退避値を設定		
			ferr = EnableEvPreBrkPoint(&dwPreBrkCombi);
			if (ferr != FFWERR_OK) {
				return ferr;
			}

		}

		setStepRunState(FALSE);
	}

	return ferr;
}
// 2008.9.19 INSERT_END_E20RX600 }

//=============================================================================
/**
 * ユーザプログラムブレーク待ち処理(RX用)
 * @param pProgData ユーザプログラム実行中の各種情報格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR mcuWaitBreak_RX(FFWRX_PROG_DATA* pProgData)
{
	FFWERR	ferr;
	DWORD	dwTimeout;
	DWORD	dwTimeStart;
	DWORD	dwTimeNow;

	dwTimeout = TMOUT_WAITBREAK;

	dwTimeStart = GetTickCount();	// ベース時間取得

	for (;;) {
		ferr = PROT_GetRXPROG(pProgData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		if (pProgData->dwExecInfo  == 0x00) {	// ターゲットプログラム停止時
			break;
		}

		dwTimeNow = GetTickCount();	// 現在の時間取得
		if ((dwTimeNow - dwTimeStart) > dwTimeout) {	// 一定時間経過した場合
			break;
		}
	}

	return FFWERR_OK;
}

//=============================================================================
/**
 * ブレーク要因の取得
 * @param dwBreakFact     ブレーク要因格納アドレス
 * @param eBrkTrcComb     イベントブレークの組合せ条件
 * @param byOrBrkFactEvPC PC通過イベント情報(イベントブレークでブレーク時)
 * @param byOrBrkFactEvOA オペランドアクセスイベント情報(イベントブレークでブレーク時)
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR getBreakStatus_RX(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb,
								BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA)
{
	FFWERR	ferr;
	DWORD	dwFact;
	enum FFWRX_COMB_PATTERN eComb;
	BYTE byEvPC;
	BYTE byEvOA;

	*dwBreakFact = 0x00000000;	// ブレーク要因初期化
	*eBrkTrcComb = COMB_OR;
	*byOrBrkFactEvPC = 0;
	*byOrBrkFactEvOA = 0;

	// ブレーク要因の取得
	ferr = PROT_MCU_GetRXBRKF(&dwFact, &eComb, &byEvPC, &byEvOA);

	// ブレーク要因格納
	*dwBreakFact = dwFact;
	*eBrkTrcComb = eComb;
	*byOrBrkFactEvPC = byEvPC;
	*byOrBrkFactEvOA = byEvOA;

	return ferr;
}

//=============================================================================
/**
 * ブレークモードのハードウェア設定
 * @param dwBMode BMで設定するブレークモード
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setOcdBreakMode(DWORD dwBMode)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwBModeTmp;
	
	dwBModeTmp = dwBMode & (BMODE_PRE_BIT | BMODE_EB_BIT);

	if(dwBModeTmp == (BMODE_PRE_BIT | BMODE_EB_BIT)){
		//実行前PCブレークとイベントブレークを設定
		ferr = SetOcdBrkRegData(OCDBRK_PRE| OCDBRK_EV);		// 実行前PCブレークとイベントブレークを設定
	} else if (dwBModeTmp == BMODE_PRE_BIT) {
		//実行前PCブレークのみの設定
		ferr = SetOcdBrkRegData(OCDBRK_PRE);					// 実行前PCブレークを設定
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		ferr = SetBreakEvEnable(FALSE);							// イベントブレーク設定を無効化
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	} else if (dwBModeTmp == BMODE_EB_BIT) {
		ferr = SetPreBreakEvEnable(FALSE);					// 実行前PCブレーク設定を無効化
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		//イベントブレークのみの設定
		ferr = SetOcdBrkRegData(OCDBRK_EV);				// イベントブレーク設定
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	} else {
		ferr = SetPreBreakEvEnable(FALSE);					// 実行前PCブレーク設定を無効化
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		ferr = SetBreakEvEnable(FALSE);							// イベントブレーク設定を無効化
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	
	return ferr;
}


//=============================================================================
/**
 * ブレークモードのSWブレーク設定
 * @param dwBMode ブレークモード
 * @param eProgCmd 実行処理コマンド
 * @param dwPcAddr 実行開始アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setSwBreakMode(DWORD dwNewBMode, enum FFWENM_PROGCMD_NO eProgCmd, DWORD dwPcAddr)
{

	FFWERR ferr;
	FFW_PBCLR_DATA*	pPbClr;
	FFW_PB_DATA*	pPb;
	BOOL			bPcCmp,bPbFillComp;
	DWORD			i;

	switch(eProgCmd){
	case PROGCMD_STEP:
		bPcCmp = TRUE;
		break;
	default:
		bPcCmp = FALSE;
		break;
	}


	// S/Wブレーク命令コード置換
	ferr = ReplaceCmdData();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	pPbClr = GetPbClrData();						// S/Wブレーク埋め戻しデータ取得
	pPb = GetPbData();							// S/Wブレークポイント設定を取得

	if ((dwNewBMode & BMODE_SWB_BIT) != BMODE_SWB_BIT) {
		// S/Wブレーク無効設定の場合

		for (i = 0; i < pPbClr->dwSetNum; i++) {
			ferr = RetCmdData(pPbClr->dwmadrAddr[i]);	// 命令コード復帰
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		for (i = 0; i < pPb->dwSetNum; i++) {
			// S/Wブレーク埋め込み済みフラグ(bPbFillComp)の設定
			if(bPcCmp == TRUE){
				// 実行開始アドレスとS/Wブレークの比較が必要な場合
				// STEPの場合はSTEPするPCにS/Wブレークが設定されている場合のみ書き戻しを実施
				if((pPb->eFillState[i] == PB_FILL_COMPLETE) && (dwPcAddr == pPb->dwmadrAddr[i] )){
					//S/Wブレークが実行PCと一致する場合
					bPbFillComp = TRUE;		//S/Wブレークが埋め込み済み
				} else {
					bPbFillComp = FALSE;	//S/Wブレークが埋め込みみ
				}
			} else {
				// 実行開始アドレスとS/Wブレークの比較が不要な場合
				// STEP以外の場合はS/Wブレークが設定されている場合、書き戻しを実施
				bPbFillComp = pPb->eFillState[i] == PB_FILL_COMPLETE;
			}
			if ( bPbFillComp == TRUE) {
				ferr = RetCmdData(pPb->dwmadrAddr[i]);		// 命令コード復帰
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				UpdatePbData(pPb->dwmadrAddr[i], 0);	// S/Wブレーク設定状態更新
			}
		}
	} else {
		// S/Wブレーク有効設定の場合
		for (i = 0; i < pPbClr->dwSetNum; i++) {
			ferr = RetCmdData(pPbClr->dwmadrAddr[i]);	// 命令コード復帰
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		for (i = 0; i < pPb->dwSetNum; i++) {
			if (pPb->eFillState[i] == PB_FILL_SET) {		// DBT命令埋め込み待ち
				ferr = FillCmdData(pPb->dwmadrAddr[i]);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				UpdatePbData(pPb->dwmadrAddr[i], 1);	// S/Wブレーク設定状態更新
			}
		}
	}

	return ferr;
}

//=============================================================================
/**
 * 実行前パフォーマンス機能設定
 * @param eProgCmd 実行コマンド
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setGoPpc(enum FFWENM_PROGCMD_NO eProgCmd)
{

	FFWERR		ferr = FFWERR_OK;
	BOOL		bPPCEnable = FALSE;
	BOOL		bClrPPCDOnce = FALSE;

	switch(eProgCmd){
	case PROGCMD_GO:
//	case PROGCMD_RSTG:					//RSTGはDO_REST()→DO_GB()のためコメントのみ
	case PROGCMD_GB:
	case PROGCMD_GPB:
	case PROGCMD_GPBA:
		bPPCEnable = TRUE;
		bClrPPCDOnce = TRUE;
		break;
	case PROGCMD_STEP:
	case PROGCMD_STEPOVER:
		bPPCEnable = TRUE;
		bClrPPCDOnce = FALSE;
		break;
	default:
		break;
	}

	if(bPPCEnable == TRUE){
		//実行前パフォーマンス
		ferr = SetPPCEnable(TRUE);							// 実行前にパフォーマンスを有効
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	if(bClrPPCDOnce == TRUE){
		ferr = ClrPPCDPreRun();								// 実行前パフォーマンス1回測定クリア
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	return ferr;
}

//=============================================================================
/**
 * 実行後のパフォーマンス機能後処理
 * @param eProgCmd 実行コマンド
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setGoAfterPpc(void)
{

	FFWERR		ferr = FFWERR_OK;

	//PPC無効
	ferr = SetPPCEnable(FALSE);

	return ferr;
}

//=============================================================================
/**
 * 実行処理内のStep後のパフォーマンス機能設定(GB,GPB用)
 * @param eProgCmd 実行コマンド
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setGoStepPpc(void)
{

	FFWERR		ferr = FFWERR_OK;

	//Step完了後に実行する場合、一度だけ計測の場合パフォーマンスカウンタをクリアする
	//クリアすると、実行処理時のパフォーマンス計測になる。クリアしない場合、STEPのパフォーマンス計測になる。
	ferr = ClrPPCDPreRun();								// 実行前パフォーマンス1回測定クリア

	return ferr;
}

//=============================================================================
/**
 * 実行前イベント機能設定
 * @param eProgCmd 実行コマンド
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setGoEvent(enum FFWENM_PROGCMD_NO eProgCmd)
{

	FFWERR		ferr = FFWERR_OK;
	BOOL		bEvEnable = FALSE;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	pDbgData = GetDbgDataRX();								// デバッグ情報取得

	switch(eProgCmd){
	case PROGCMD_GO:
	case PROGCMD_GB:
//	case PROGCMD_RSTG:					//RSTGはDO_REST()→DO_GB()のためコメントのみ
	case PROGCMD_GPB:
		bEvEnable = TRUE;
		break;
	case PROGCMD_GPBA:
	case PROGCMD_STEP:
	case PROGCMD_STEPOVER:
		bEvEnable = FALSE;
		break;
	default:
		break;
	}

	if(bEvEnable == TRUE){
		//プログラム実行前設定ありの場合
		if (pDbgData->eBfrRunRegSet == EML_RUNREGSET_USE) {
			ferr = SetEvRegDataBeforeGo();
			if(ferr != FFWERR_OK){
				return ferr;
			}
		}
	}

	return ferr;
}
//=============================================================================
/**
 * 実行前トレース機能設定
 * @param eProgCmd 実行コマンド
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setGoTrace(enum FFWENM_PROGCMD_NO eProgCmd)
{

	FFWERR		ferr = FFWERR_OK;
	BOOL		bClrTrc = FALSE;
	BOOL		bSetTrc = FALSE;
	BOOL		bTrcFullBrk = FALSE;
	enum FFWENM_TRCBRK_MODE eTrcBrk;
	FFWRX_RM_DATA*  RmData_RX;
	DWORD		dwMode;
	FFW_RRMB_DATA*	pRrmb;
	int			i;

	FFWMCU_DBG_DATA_RX*	pDbgData;
	pDbgData = GetDbgDataRX();								// デバッグ情報取得

	switch(eProgCmd){
	case PROGCMD_GO:
		bClrTrc = TRUE;
		bSetTrc = TRUE;
		bTrcFullBrk = FALSE;
		break;
//	case PROGCMD_RSTG:					//RSTGはDO_REST()→DO_GB()のためコメントのみ
	case PROGCMD_GB:
		bClrTrc = TRUE;
		bSetTrc = TRUE;
		bTrcFullBrk = TRUE;
		break;
	case PROGCMD_GPB:
		bClrTrc = TRUE;
		bSetTrc = TRUE;
		bTrcFullBrk = FALSE;
		break;
	case PROGCMD_GPBA:
		RmData_RX = GetRmDataInfo();
		dwMode = (RmData_RX->dwSetMode & 0xFF000000) >> 24;
		if((dwMode == MODE3) || (dwMode == MODE4) || (dwMode == MODE8)){
			bSetTrc = TRUE;
		}
		// RRM有効無効確認
		pRrmb = GetRrmbData();
		for(i=0;i<RRM_BLKNUM_MAX;i++){
			if(pRrmb->eEnable[i] == RRMB_ENA){
				bSetTrc = TRUE;
			}
		}
		bClrTrc = FALSE;
		bTrcFullBrk = FALSE;
		break;
	case PROGCMD_STEP:
	case PROGCMD_STEPOVER:
		bClrTrc = FALSE;
		bSetTrc = FALSE;
		bTrcFullBrk = FALSE;
		break;
	default:
		break;
	}

	if(bClrTrc == TRUE){
		//プログラム実行前設定ありの場合
		if (pDbgData->eBfrRunRegSet == EML_RUNREGSET_USE) {
			//トレースクリアが必要
			ferr = ClrTrcMemData();	// トレースデータクリア // RevRxE2LNo141104-001 Modify Line
			if(ferr != FFWERR_OK){
				return ferr;
			}
		}
	}
	if(bSetTrc == TRUE){
		//プログラム実行前設定ありの場合
		if (pDbgData->eBfrRunRegSet == EML_RUNREGSET_USE) {
			//トレースデータセットを行う
			if(bTrcFullBrk == TRUE){
				eTrcBrk = TRC_BRK_ENABLE;
			} else {
				eTrcBrk = TRC_BRK_DISABLE;
			}
			ferr = SetTrace(eTrcBrk);							// トレースFPGAへの初期設定
			// RevNo110125-01 (V.1.02 覚書 No.27) トレースメモリクリア不具合 Append Line
			SetClrTrcMemSize();									// クリアトレースメモリサイズ更新
			if(ferr != FFWERR_OK){
				return ferr;
			}
		}
	}

	ClrTraceDataClrFlg();	// RCLコマンドによるトレースデータクリア未実行

	return ferr;

}
//=============================================================================
/**
 * 実行前Flash書き込み動作
 * @param eProgCmd 実行コマンド
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setGoWriteFlash(enum FFWENM_PROGCMD_NO eProgCmd)
{
	FFWERR		ferr = FFWERR_OK;
	FFWERR		ferr2 = FFWERR_OK;
	BOOL		bRecvrPb = FALSE;
	BOOL		bCacheClr = FALSE;
	BOOL		bFlashWrite;
	DWORD		i;
	FFW_PB_DATA*	pPb;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	pDbgData = GetDbgDataRX();								// デバッグ情報取得

	switch(eProgCmd){
	case PROGCMD_GO:
	case PROGCMD_GPB:
	case PROGCMD_GPBA:
		bRecvrPb = FALSE;
		bCacheClr = TRUE;
		break;
//	case PROGCMD_RSTG:					//RSTGはDO_REST()→DO_GB()のためコメントのみ
	case PROGCMD_GB:
		bRecvrPb = TRUE;
		bCacheClr = TRUE;
		break;
	case PROGCMD_STEP:
	case PROGCMD_STEPOVER:
		bRecvrPb = FALSE;
		bCacheClr = FALSE;
		break;
	default:
		break;
	}

	// RevRxNo121026-001 Append Start
	// RevRxNo130411-001 Modify Line
	// RevRxNo140515-006 Modify Start
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {	// フラッシュがMF3またはSC32用MF3の場合
		if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {		// ユーザマットのフラッシュ書き換えデバッグありの場合
			bCacheClr = TRUE;
		}
	}
	// RevRxNo140515-006 Modify End
	// RevRxNo121026-001 Append End

	// フラッシュROMキャッシュ変更ありの場合、フラッシュROMへの書込み実施
	bFlashWrite = GetStateFlashRomCache();
	if (bFlashWrite == TRUE) {
		// フラッシュROMへのライト処理実施
		ferr = WriteFlashExec(VERIFY_OFF);
		if (ferr != FFWERR_OK) {
			if(bRecvrPb == FALSE){
				return ferr;
			} else {
				// RevNo110415-01 Appned Start
				pPb = GetPbData();							// S/Wブレークポイント設定を取得
				// S/WブレークステートをPB_FILL_COMPLETE → PB_FILL_SET
				for (i = 0; i < pPb->dwSetNum; i++) {
					if (pPb->eFillState[i] == PB_FILL_COMPLETE) {
						ferr2 = RetCmdData(pPb->dwmadrAddr[i]);		// 命令コード復帰
						if (ferr2 != FFWERR_OK) {
							return ferr2;
						}
						UpdatePbData(pPb->dwmadrAddr[i], 0);		// S/Wブレーク設定状態更新
					}
				}
				// RevNo110415-01 Appned Start
				return ferr;
			}
		}

	}

	// 埋め戻し対象のS/Wブレークポイントをクリア(FWRITE成功後に実施する)
	ClrPbClrTbl();

	if(bCacheClr == TRUE){
		// データマット領域 キャッシュメモリデータ設定フラグをクリア
		InitCacheDataSetFlg(MAREA_DATA);
	
		// フラッシュ書き換えデバッグありの場合
		if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {
			// ユーザーマット領域 キャッシュメモリデータ設定フラグをクリア
			InitCacheDataSetFlg(MAREA_USER);
		}
	}

	return ferr;
}
//=============================================================================
/**
 * ブレークポイントから実行時のインストラクションステップ実行
 * @param pStep ステップ実行時のブレーク要因格納構造体アドレス
 * @param eStepCmd ステップ実行コマンド
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR mcuBrkStep(enum FFWENM_PROGCMD_NO eProgCmd, FFWE20_STEP_DATA* pStep)
{
	FFWERR	ferr;
	BOOL	bFfwStepReq;
	BOOL	bTgtReset;
	WORD	wOpecode;
	enum FFWENM_PROGCMD_NO eRunCmd;
	FFWMCU_REG_DATA_RX*	pRegDataRX;

	FFW_PB_DATA*	pPb;
	FFW_PBCLR_DATA*	pPbClr;
	DWORD			i;
	DWORD			dwPcAddr;

	BYTE	byMskSet = 0;


	pRegDataRX = GetRegData_RX();			// レジスタ情報取得

	eRunCmd = PROGCMD_STEP;

	SetMcuRunCmd(eRunCmd);					// STEP/STEPOVERコマンドによるプログラム実行


	dwPcAddr = pRegDataRX->dwRegData[REG_NUM_RX_PC];	// 現在のPCアドレス取得
	pPb = GetPbData();									// S/Wブレークポイント設定を取得

	//GBの場合は必要であるがGPBは不要
	if(eProgCmd == PROGCMD_GB){
		// S/Wブレーク命令コード置換
		ferr = ReplaceCmdData();
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		pPbClr = GetPbClrData();						// S/Wブレーク埋め戻しデータ取得
		for (i = 0; i < pPbClr->dwSetNum; i++) {
			ferr = RetCmdData(pPbClr->dwmadrAddr[i]);	// 命令コード復帰
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		for (i = 0; i < pPb->dwSetNum; i++) {
			// 埋め込み済みの場合
			if (dwPcAddr == pPb->dwmadrAddr[i] && pPb->eFillState[i] == PB_FILL_COMPLETE) {
				ferr = RetCmdData(dwPcAddr);		// 命令コード復帰
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				UpdatePbData(pPb->dwmadrAddr[i], 0);	// S/Wブレーク設定状態更新
				break;
			}
		}

		// 通過イベントブレーク設定を無効化
		ferr = SetBreakEvEnable(FALSE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// 実行前PCブレーク設定を無効化
		ferr = SetPreBreakEvEnable(FALSE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}


	//STEP実行実施
	ferr = mcuStep(pRegDataRX, pStep, &bFfwStepReq, &wOpecode, &bTgtReset, STEP_BFWCTRL, byMskSet);		// シングルステップ実行
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (pStep->eGoToRunState == STEP_NOT_GOTO_RUNSTATE) {	// ステップ実行が完了した場合

		ferr = SetBreakEvEnable(TRUE);					// STEP前の通過イベントブレーク設定を復帰
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		ferr = SetPreBreakEvEnable(TRUE);				// STEP前の実行前PCブレーク設定を復帰
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		if (eRunCmd == PROGCMD_STEP) {						// シングルステップ実行の場合
			setStepRunState(FALSE);							// シングルステップ実行からRUN状態への移行なし
		}
		pRegDataRX = GetRegData_RX();											// レジスタ情報取得
		pRegDataRX->dwRegData[REG_NUM_RX_PC] = pStep->dwPc;

		//テップ実行後に汎用レジスタを毎回取得したらどうなるか
		ferr = PROT_MCU_GetRXXREG(REG_NUM_RX_ALL, pRegDataRX);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		SetRegData_RX(pRegDataRX);
	} else {												// ステップ実行が完了しなかった場合(RUN状態へ移行した場合)
		if (eRunCmd == PROGCMD_STEP) {						// シングルステップ実行の場合
			setStepRunState(TRUE);							// シングルステップ実行後RUN状態へ移行したことを指示
		}
		SetMcuRunState(TRUE);								// ユーザプログラム実行状態を"プログラム実行中"にする
	}


	return ferr;
}

//=============================================================================
/**
 * GPBで使用するOCDブレークポイント設定
 * @param madrBreakAddr GPBのブレークアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setGpbBrkEvent(MADDR madrBreakAddr)
{
	FFWERR			ferr;
	BOOL			bExecPcEv;
	DWORD			dwEvNum;
	DWORD			dwCombi;
	FFWRX_COMB_BRK	evBrk;
	// RevNo110825-01 Append Start
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	enum FFWRX_EV				eEvNum;
	FFWRX_EVCNT_DATA			evCnt;
	BOOL						bEvCntFlg;
	// RevNo110825-01 Append End

	// 停止アドレスを設定
	memset(&evBrk, 0, sizeof(FFWRX_COMB_BRK));

	//イベントに空きがあるかチェック
	ferr = SearchExecPcEvNum(&bExecPcEv,&dwEvNum);
	if(ferr != FFWERR_OK){
		return ferr;
	}

	//イベントが足りなかった場合　エラーで返す
	if(bExecPcEv == FALSE){
		return FFWERR_GPB_NOT_EVNUM;
	}

	// V.1.02 覚書 No.12　GPB実行時のイベント数 Delete

	//RevNo100715-014 Append Line
	//GPBで使用するイベント番号を退避
	s_dwGPBEvNo = dwEvNum;
	// GPBでブレークするアドレスを退避
	s_dwGPBAddr = madrBreakAddr;		// RevNo120131-001 Append Line

	// RevNo110825-01 Append Start
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	// RX OCD Class3の場合
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		// 指定イベントにイベント回数指定が設定されているかチェック
		eEvNum = (enum FFWRX_EV)(dwEvNum);
		bEvCntFlg = SearchEvCnt(eEvNum);
		if(bEvCntFlg == TRUE){
			//GPBイベントに回数指定が設定されている場合
			evCnt.bEnable = FALSE;		// イベントカウント無効
			evCnt.dwCount = 1;			// カウント回数 1
			evCnt.eEvNo = eEvNum;
			//イベントカウント設定を行う
			ferr = SetEvCnt(&evCnt);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// RevNo110825-01 Append End

	// RevNo120131-001 Delete
	// 設定情報を退避をしていたが、内部管理変数の更新が複雑になるので削除

	// レジスタ設定
	ferr = SetExecPcEv(dwEvNum, madrBreakAddr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	evBrk.dwPreBrk = 0x00000001 << dwEvNum;
	dwCombi = GetCombiInfo();

	//ブレークイベントが使用可能なイベントかチェックする
	ferr = ChkEvBreakData(dwCombi, &evBrk);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//実行前ブレークイベントセット
	ferr = SetEvBreakData(&evBrk);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// GPBで使用する実行前PCブレークを有効にする
	ferr = SetEvPreBreakOn(&evBrk);

	return ferr;
}
// RevNo120131-001 Append Start
//=============================================================================
/**
 * GPBイベント情報取得
 * @param pdwEvNo　GPBイベント設定イベント番号
 * @param pdwEvAddr　GPBイベント設定アドレス
 * @retval なし
 */
//=============================================================================
void GetGpbPcEv(DWORD* pdwEvNo, DWORD* pdwEvAddr){
	*pdwEvNo = s_dwGPBEvNo;
	*pdwEvAddr = s_dwGPBAddr;
	return;
}
// RevNo120131-001 Append End

// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ実行状態確認処理
 * @param	なし
 * @retval	TRUE  SSSTコマンド発行状態でステップ実行中である
 * @retval	FALSE SSSTコマンド未発行またはSSSTコマンド発行状態でステップ未実行である
 */
//=============================================================================
static BOOL isStepContinue(void)
{
	BOOL	bFlg;

	if (GetSsstState() == FALSE) {		// SSSTコマンドが発行されていない場合
		bFlg = FALSE;

	} else if (GetStepContinueStartFlg() == FALSE) {	// 連続ステップ実行を開始していない場合
		bFlg = FALSE;

	} else {
		bFlg = TRUE;
	}

	return bFlg;
}
// RevRxNo140515-007 Append End

// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ開始フラグ設定
 * @param	なし
 * @retval	なし
 */
//=============================================================================
void SetStepContinueStartFlg(void)
{
	s_bStepContinueStart = TRUE;	// 連続ステップ実行を開始した(1回目のステップを実行した)
}
// RevRxNo140515-007 Append End

// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ開始フラグクリア
 * @param なし
 * @retval なし
 */
//=============================================================================
static void clrStepContinueStartFlg(void)
{
	s_bStepContinueStart = FALSE;	// 連続ステップ実行を開始していない(1回目のステップを実行していない)
}
// RevRxNo140515-007 Append End

// RevRxNo140515-007 Append Start
//=============================================================================
/**
 * 連続ステップ開始フラグ値取得
 * @param なし
 * @retval TRUE :連続ステップ実行を開始した(1回目のステップを実行した)
 * @retval FALSE:連続ステップ実行を開始していない(1回目のステップを実行していない)
 */
//=============================================================================
BOOL GetStepContinueStartFlg(void)
{
	return s_bStepContinueStart;
}
// RevRxNo140515-007 Append End


