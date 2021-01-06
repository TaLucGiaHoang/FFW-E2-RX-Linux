///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_rst.cpp
 * @brief リセット関連コマンドの実装ファイル
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, K.Uemori, S.Ueda, Y.Miyake, Y.Kawakami, SDS T.Iwata, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120727-001 2012/07/27 橋口
	内蔵ROM DUMP 高速化
・RevRxNo121022-001 2012/10/22 SDS 岩田
	DO_CLR()関数に、EZ-CUBE用処理を追加
・RevRxNo121022-001	2012/11/1 SDS 岩田
　　DO_CLR()関数のエミュレータ種別 EZ-CUBE判定処理の定義名を変更
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121206-001 2012/12/06 植盛
  ブートスワップ時のキャッシュ処理対応
・RevRxNo121211-001 2012/12/13 植盛
  低消費電力モード中のSREST不具合対応
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130308-001	2013/08/21 上田 (2013/06/27 三宅担当分マージ)
　カバレッジ開発対応
・RevRxNo130301-001 2013/09/06 上田
	RX64M対応
・RevRxNo130730-005 2013/11/11 上田
	内蔵ROM有効/無効判定処理改善
・RevRxNo130730-006	2013/11/13 上田
	E20トレースクロック遅延設定タイミング変更
・RevRxNo130411-001 2014/01/21 植盛
	enum FFWRXENM_MDE_TYPE型のメンバ名変更(RX_MDE_OSM→RX_MDE_FLASH)
・RevRxNo140515-007 2014/06/18 植盛
	SSST, SSENコマンド追加による速度改善
・RevRxNo130730-001 2014/07/23 植盛、大喜多
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140515-006 2014/06/24 植盛
	RX231対応
・RevRxNo140515-005 2014/06/26 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxE2LNo141104-001 2014/12/12 上田
	E2 Lite対応
・RevRxNo161003-001 2016/10/20 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#include "domcu_rst.h"
#include "mcurx_ev.h"
#include "domcu_prog.h"
#include "protmcu_rst.h"
#include "prot_common.h"
#include "do_sys.h"
//RevNo100715-028 Append Start
#include "dorx_tra.h"
#include "hwrx_fpga_tra.h"
#include "ffwrx_rrm.h"
#include "dorx_rrm.h"
#include "mcurx_tim.h"
//RevNo100715-028 Append End
#include "mcu_rst.h"
#include "domcu_dc.h"
#include "domcu_brk.h"
#include "domcu_srm.h"
#include "domcu_mcu.h"
#include "dodata.h"
#include "ffwdata.h"
#include "mcu_sfr.h"
#include "domcu_reg.h"
#include "mcuspec.h"
// RevRxNo121206-001 Append Line
#include "mcu_flash.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line
// RevRxNo130308-001 Append Start
#include "ffwrx_tra.h"
#include "dorx_cv.h"
#include "hwrx_fpga_cv.h"
#include "mcu_extflash.h"
#include "ocdspec.h"
#include "mcurx.h"
// RevRxNo130308-001 Append End
#include "ffwmcu_prog.h"	// RevRxNo140515-007 Append Line
// RevRxNo130730-001 Append Start
#include "ffwmcu_mcu.h"
#include "ffwmcu_dwn.h"
#include "domcu_dwn.h"
// RevRxNo130730-001 Append End
// RevRxNo140515-005 Append Start
#include "ffwmcu_srm.h"
#include "domcu_mem.h"
// RevRxNo140515-005 Append End
// static関数の宣言
static int s_nEndianState;		// エンディアン状態を記憶
//RevNo100715-028 Append Line
// RevRxNo170829-001 Append Start
#include "doasp_sys.h"
#include "doasprx_monev.h"
#include "doasp_trg.h"
#include "doasp_pwr.h"
#include "cpuspec.h"
// RevRxNo170829-001 Append End

static FFWERR s_ferrOsmFwriteWarning;				// RevRxNo130730-001 Append Line
static BOOL s_bSkipMdeMskResetOsmFwrite = FALSE;	// RevRxNo130730-001 Append Line

// RevRxNo130730-001 Append Line	// RevRxNo161003-001 Modify Line
static FFWERR beforeResetOsm(BOOL* pbSrestFlg, BOOL* pbFwriteFlg, FFWERR* pferrWarning, BOOL* pbBootSwap, BOOL* pbBankselChangeFlg);

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//==============================================================================
/**
 * ターゲットMCU をH/W リセットする。リセット後、MCU の各レジスタを初期化する。
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_REST(void)
{

	FFWERR						ferr;
	FFWERR						ferrEnd;
	FFWERR						ferrWarning = FFWERR_OK;	// RevRxNo130730-001 Append Line
	FFWMCU_MCUAREA_DATA_RX*		pMcuArea;			// ROMレス品判断のためMCU領域情報が必要
	// V.1.02 RevNo110425-001 Append Start
	FFW_RRMB_DATA*		pSetRrmb;		// 設定中のRAM モニタ設定情報
	FFWE20_EINF_DATA	einfData;
	DWORD				dwSetBlk;
	// V.1.02 RevNo110425-001 Append End
	// RevRxNo121206-001 Append Line
	BOOL				bBootSwap;
	BOOL				bSysReset;		// RevRxNo130730-001 Append Line
	BOOL				bOsmFwrite;		// RevRxNo130730-001 Append Line
	enum FFWRXENM_PMODE	ePmode;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	// RevRxNo130308-001 Append Start
	FFWMCU_DBG_DATA_RX*	pDbgData;
	BOOL	bRstf;
	BOOL	bLost_dOrTrerr;
	FFWRX_CVM_DATA*	pCoverageMode_RX;
	FFWRX_CVB_DATA*	pCvb_RX;
	// RevRxNo130308-001 Append End
	BOOL bSetOcdFpgaReg;	// RevRxNo130730-006 Append Line
	// RevRxNo161003-001 Append Start
	enum FFWRXENM_BANKSEL	eBankSel;
	BOOL	bBankselChangeFlg = FALSE;
	// RevRxNo161003-001 Append End

	ePmode = GetPmodeDataRX();
	pMcuArea = GetMcuAreaDataRX();
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line


	ProtInit();

	// RevRxNo130730-006 Append Start
	// ユーザブート起動時、以下に該当する場合、OCDとFPGAレジスタを再設定する必要がある。
	// (a)リセット処理でBFWのHPONコマンドを発行する(BFW のHPONコマンド内でFPGAを初期化しているため)。
	// (b)ブートプログラム内でOCDレジスタを使用する。
	// ここでは、MCUのリセット処理後にOCDとFPGAレジスタの再設定が必要であるかを確認する。
	bSetOcdFpgaReg = FALSE;
	if ((ePmode == RX_PMODE_USRBOOT) && (pFwCtrl->bSetFpgaOcdUsrBootReset == TRUE)) {
		bSetOcdFpgaReg = TRUE;
	}
	// RevRxNo130730-006 Append End

	//リセット時にトレースデータがゴミを出力するときがあるので、トレースFPGAの入力をOFFにする
	// RevRxE2LNo141104-001 Modify Start
	if (GetTrcInramMode() == FALSE) {
		ferr = SetTrcFpgaEnable(FALSE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		// Min.32μsの待ち時間(FPGA内ライトバッファのトレースメモリ反映待ち)
		// RevRxNo130308-001-010 Modify Start
		COM_WaitMs(100);	// 100ms待ち(CVD0コマンドで1回なので100msでも体感速度に問題ない)
							// 注：COM_WaitMs()の関数仕様上、10msなど小さい値の引数は正常動作しない可能性あり。
		// RevRxNo130308-001-010 Modify End

		// FPGA内TRCTL0[8].RSTFが"1"の場合(CVCTL.TMP2を見る)	// RevRxNo130308-001-027 Modify Line
		ferr = GetFpgaCvctl_Tmp2(&bRstf);						// RevRxNo130308-001-027 Modify Line
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (bRstf) {
			SetCoverageResetFlag(TRUE);	// "ユーザリセットが発生したことを示すフラグ"に"TRUE"を設定
		}
		// RevRxNo130308-001-013 Modify Start
		// FPGA内TRCTL0[13].LOST_DまたはTRCTL0[12].TRERRまたはCVCTL[14].TMP が"1"の場合
		ferr = GetFpgaLost_dOrTrerrOrTmp(&bLost_dOrTrerr);
		// RevRxNo130308-001-013 Modify End
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (bLost_dOrTrerr) {
			SetCoverageLostFlag(TRUE);	// "カバレッジのLOSTが発生したことを示すフラグ"に"TRUE"を設定
		}

		// マイコン内TBSRレジスタのビット6が"1(次の記録データにロストが記録される)"の場合
		// RevRxNo130308-001-021 Modify Line
		// LOST発生フラグは"TRUE"にしない。理由は、CVCL後RESTコマンドでLOST発生フラグを"TRUE"にしてしまわないように。
	}
	// RevRxNo130308-001 Append End

	// RevRxNo121206-001 Append Start
	bBootSwap = FALSE;

	// RevRxNo130411-001 Modify Line, RevRxNo140515-006 Modify Line
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
		// フラッシュがMF3またはSC32用MF3の場合

		ferr = ChkBootSwap(&bBootSwap);			// IOレジスタからブートスワップの設定状態を確認する。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo121206-001 Append End

	// RevRxNo140515-007 Append Start
	if (GetSsstState()) {	// SSSTコマンド発行状態の場合
		ferr = DO_SetSSEN();		// 連続ステップ処理の終了処理を実施
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-007 Append End

	// RevRxNo130730-001 Append Start
	bSysReset = FALSE;
	bOsmFwrite = FALSE;
	if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {
		// プログラムROM のフラッシュ書き換えデバッグが「有効」の場合
		// (OFS1レジスタおよびMDEレジスタがプログラムROM領域に配置されているため)

		// オプション設定メモリ書き戻し判定および処理を実施
		// bBootSwapがTRUE且つbeforeResetOsm()内でシステムリセットを実施した場合、
		// beforeResetOsm()でシステムリセット後にクラスタ0と1のキャッシュを入れ替えて、
		// bBootSwapをFALSEに設定する(システムリセット後にフラッシュを書き換えるため)。
		// 上記の場合、DO_REST()でキャッシュの入れ替えを実施しないことになる。
		// RevRxNo161003-001 Modify Line
		ferr = beforeResetOsm(&bSysReset, &bOsmFwrite, &ferrWarning, &bBootSwap, &bBankselChangeFlg);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo130730-001 Append End

	// RevRxNo161003-001 Append Start
	if (GetBankSelChangeFlg() || bBankselChangeFlg) {	// ダウロード処理でBANKSWPビットを書き換えた、または、セルフプログラミングでBANKSWPビット書き換えた
		// FFWキャッシュメモリクリア
		InitCacheDataSetFlg(MAREA_USER);		// ユーザマットのキャッシュ取得情報クリア
		// 現在の起動バンク情報を取得し反対の情報を設定
		eBankSel = GetBankSelDataRX();
		if (eBankSel == RX_BANKSEL_BANK0) {				// 現在の起動バンクが0の場合は1に変わる
			eBankSel = RX_BANKSEL_BANK1;
		} else if (eBankSel == RX_BANKSEL_BANK1) {		// 現在の起動バンクが1の場合は0に変わる
			eBankSel = RX_BANKSEL_BANK0;
		} else {
			// ここには来ない
		}
		SetBankSelDataRX(eBankSel);				// s_eBankSelに設定
	}
	// RevRxNo161003-001 Append End

	//リセット処理
	ferr = McuRest();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	
	// RevRxNo121206-001 Append Start
	if (bBootSwap) {				// MCUのリセットによりブートスワップする→しないになった場合
		SwapFlashRomCacheData();		// クラスタ0と1のキャッシュを入れ替える
		bBootSwap = FALSE;				// キャッシュを入れ替えたため、フラグをFALSEに初期化	// RevRxNo130730-001 Append Line
	}
	// RevRxNo121206-001 Append End

	// 汎用レジスタ値の初期化設定( FINEでもJTAGでも必要 )
	ferr = ClrFwRegData(REGCLRCMD_REST);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevNo110422-001 Append Start


	// リセット時のエンディアン状態を記憶
	ferr = SetEndianState();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	//トレースFPGAの入力を有効にする
	// RevRxE2LNo141104-001 Modify Start
	if (GetTrcInramMode() == FALSE) {
		ferr = SetTrcFpgaEnable(TRUE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// トレースFPGAの入力をOFFにすると、トレースFPGAのRRM伸張ベースアドレスがクリアされるので、
	// RRM有効ステップ時にOCDとトレースFPGAのアドレスを一致させるために、OCD RAMにダミーデータ処理を行う
	// RevNo120727-001 Modify Line
	if ((GetTrcInramMode() == FALSE) && (GetMcuRunState() == FALSE)) {
		// RevRxNo130308-001 Append Line
		if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合

			// RevRxNo130730-006 Append Start
			if (bSetOcdFpgaReg) {
				SetTrcE20FpgaAllRegFlg(TRUE);	// SetTrace()内でのE20トレースFPGA全レジスタ設定を指示
			}
			// RevRxNo130730-006 Append End

			ferr = SetTrace(TRC_BRK_ENABLE);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			// RevRxNo130730-006 Append Start
			if (bSetOcdFpgaReg) {
				SetTrcE20FpgaAllRegFlg(FALSE);	// SetTrace()内でのE20トレースFPGA全レジスタ設定指示を解除
			}
			// RevRxNo130730-006 Append End
		// RevRxNo130308-001 Append Line
		}
	}

	// V.1.02 RevNo110425-001 Append Start
	// 以下に該当する場合、OCDとFPGAレジスタを再設定する。
	// (a)ユーザブート起動時、リセット処理でBFWのHPONコマンドを発行する(BFW のHPONコマンド内でFPGAを初期化しているため)。
	// (b)ユーザブート起動時、ブートプログラム内でOCDレジスタを使用する。
	// (c)オプション設定メモリ書き戻し処理内でシステムリセットを実行した。
	// RevRxNo130411-001, RevRxNo130730-006 Modify Line
	// RevRxNo130730-001 Modify Line
	if (bSetOcdFpgaReg || bSysReset) {

		// RevRxNo130730-006 Append Line
		SetTrcE20FpgaAllRegFlg(TRUE);	// SetTrace(),DO_E20_SetRRMB()内でのE20トレースFPGA全レジスタ設定を指示

		// RevNo110506-002 Append Start
		// 内蔵トレースの場合は上のsetTrace()を通らないので、ここで呼ぶ必要がある
		if (GetTrcInramMode()) {
			// RevRxNo130308-001 Append Line
			if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
				ferr = SetTrace(TRC_BRK_ENABLE);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			// RevRxNo130308-001 Append Line
			}
		}
		// RevNo110506-002 Append End
		// RRM再設定
		// E20 38PIN接続時設定
		getEinfData(&einfData);		// エミュレータ情報取得
		if ((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN)) {
			pSetRrmb = GetRrmbData();
			dwSetBlk = MAX_BLOCK_NUM;
			ferr = DO_E20_SetRRMB(dwSetBlk, pSetRrmb);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}

		// RevRxNo130730-006 Append Line
		SetTrcE20FpgaAllRegFlg(FALSE);	// SetTrace(),DO_E20_SetRRMB()内でのE20トレースFPGA全レジスタ設定指示を解除


		// パフォーマンス再設定
		ferr = SetPpcRegData();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// 全イベント再設定
		ferr = SetEvRegDataAll();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// RevRxNo130308-001 Append Start
		// カバレッジ再設定
		// 注意：DO_REST()の最初でフラグの更新をしているので、ここ以前でTRENを"0"→"1"にすることがあっても問題ない。
		if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
			// RevRxNo130730-006 Append Line
			SetTrcE20FpgaAllRegFlg(TRUE);	// DO_SetRXCVM()内でのE20トレースFPGA全レジスタ設定を指示

			pCoverageMode_RX = GetCoverageMode_RXInfo();	// カバレッジ機能時のトレースモード格納データ構造体のstatic変数のアドレスを取得
			ferr = DO_SetRXCVM(pCoverageMode_RX);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// RevRxNo130730-006 Append Line
			SetTrcE20FpgaAllRegFlg(FALSE);	// DO_SetRXCVM()内でのE20トレースFPGA全レジスタ設定指示を解除

			pCvb_RX = GetCvb_RXInfo();	// カバレッジ計測領域格納データ構造体のstatic変数のアドレスを取得
			ferr = DO_SetRXCVB(CVB_MAX_BLOCK_BIT, pCvb_RX);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// RevRxNo130308-001 Append End
	}
	// V.1.02 RevNo110425-001 Append End

	// RevRxNo130730-001 Modify Start
	if (ferrWarning != FFWERR_OK) {
		s_ferrOsmFwriteWarning = ferrWarning;		// オプション設定メモリ書き戻し時のWarning情報を覚えておく
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	
	return ferr;
	// RevRxNo130730-001 Modify End
}


//==============================================================================
/**
 * エミュレータ内部設定を初期化する。
 * @param なし
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWERR DO_CLR(void)
{

	FFWERR				ferr;
	FFWERR				ferrEnd;
	FFWMCU_REG_DATA_RX		RegData;
	FFWMCU_REG_DATA_RX*	pReg;
	FFWE20_EINF_DATA	einfData;
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;
	// RevRxNo130411-001 Append Line
	FFWMCU_FWCTRL_DATA*	pFwCtrl;

	BOOL	bRet = FALSE;
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// V.1.02 No.4 ユーザブート/USBブート対応 Append Start
	FFWMCU_MCUAREA_DATA_RX*		pMcuArea;			// ROMレス品判断のためMCU領域情報が必要
	enum FFWRXENM_PMODE	ePmode;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	pMcuInfo = GetMcuInfoDataRX();
	ePmode = GetPmodeDataRX();
	pMcuArea = GetMcuAreaDataRX();
	// V.1.02 No.4 ユーザブート/USBブート対応 Append End

	// RevRxNo130308-001 Append Line
 	pDbgData = GetDbgDataRX();
	// RevRxNo130301-001 Append Line
	pFwCtrl = GetFwCtrlData();

	ProtInit();

	getEinfData(&einfData);						// エミュレータ情報取得

	// RevRxNo130301-001 Append Start
	// ホットプラグ起動中でない場合
	if (bHotPlugState == FALSE) {
		// 外部トレース切り替え時にデバッグモジュールリセットが必要なMCUの場合
		// デバッグモジュールをリセットする
		if (pFwCtrl->bResetDebugModuleExtTrc == TRUE) {
			// OCDのデバッグモジュールリセット
			ferr = OcdDbgModuleReset();
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
	}
	// RevRxNo130301-001 Append End

	// イベント関連レジスタ初期化
	ferr = ClrEvReg();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// S/Wブレーク初期化
	ferr = DO_ClrPBAll();		// RevRxE2No171004-001 Modify Line
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// ブレーク関連レジスタ初期化
	ferr = ClrBrkReg();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// パフォーマンス関連レジスタ初期化
	ferr = ClrPerfReg();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	/* 指定ルーチン実行設定クリア */
	ferr = ClrSRM();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	/* C2E/E2Cバッファクリア */
	ferr = DO_ClrC2EE2C(0);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続時
	if ((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN)) {
		// RAMモニタ関連レジスタ初期化
		ferr = ClrRramReg();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// RevRxNo130308-001 Append Line
	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		// ダミーデータ設定があるため、RAMモニタ初期化の後に移動
		// トレース関連レジスタ初期化 E1/E20の切り分け処理はClrTrcReg()で行う
		ferr = ClrTrcReg();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	// RevRxNo130308-001 Append Line
	}

	// RevRxNo130308-001 Append Line
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		ferr = ClrCvReg();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo130308-001 Append End

	//ホットプラグ中にFFW実行中フラグはクリアしない
	if ((bHotPlugState == TRUE) && (bIdCodeResultState == TRUE)) {
		bRet = GetMcuRunState();
	}

	/* FFW 内部変数を初期化 */
	InitFfwIfData_CLR();
	InitFfwCmdData_CLR();

	if ((bHotPlugState == TRUE) && (bIdCodeResultState == TRUE)) {
		SetMcuRunState(bRet);
	}

	/* MCU レジスタ値の取得 */
	//ホットプラグ中はレジスタ値をクリアしない
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {

		// 内蔵ROM有効/無効状態が変わっている可能性があるため、SetPmodeInRomReg2Flg()を発行する。
		ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// V.1.02 No.4 ユーザブート/USBブート対応 Modify Start
		// レジスタ値を更新
		ferr = ClrFwRegData(REGCLRCMD_CLR);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

	} else {
		//ホットプラグ
		//DO_CLR()はエンディアン情報はクリアしないようにしなければならない。
		ferr = SetEndianState();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		if(GetMcuRunState() == TRUE){		//実行中はレジスタの初期化のみ行っておく
			pReg = &RegData;
			memset(pReg, 0, sizeof(FFWMCU_REG_DATA_RX));
		}
		//ブレーク中はなにもしない　コードに埋め込まれているソフトブレークによるブレーク時のPC取得のため
	}

// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	if (einfData.wEmuStatus == EML_E2) {
		InitAspPinConf();
		InitAspMonEv();
		InitAsp();
		InitAspExTrgOutLevel();
		InitAspPwrMonCorrect();
	}
#endif
// RevRxE2No171004-001 Append End

	ferrEnd = ProtEnd();

	return ferrEnd;

}

//=============================================================================
/**
 * システムリセット
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
//RevNo100715-028 Modify Start
FFWERR DO_SREST(void)
{
	FFWERR	ferr;
	FFWERR			ferrEnd;
	FFWERR				ferrWarning = FFWERR_OK;	// RevRxNo130730-001 Append Line
	BYTE	byResult;
	// RevRxNo121206-001 Append Line
	BOOL				bBootSwap;
	BOOL				bSysReset;		// RevRxNo130730-001 Append Line
	BOOL				bOsmFwrite;		// RevRxNo130730-001 Append Line

	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	enum FFWRXENM_PMODE	ePmode;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;	// RevRxNo130730-001 Append Line

	// RevRxNo140515-005 Append Start	
	// RevRxNo120910-006 Append Start
	FFW_SRM_DATA* pSrmData;
	BYTE*	pbyRwBuff;
	MADDR	dwmadrWorkProgramStartAddr;				// ワーク・プログラム スタートアドレス
	MADDR	dwmadrWorkProgramEndAddr;				// ワーク・プログラム エンドアドレス
	FFW_VERIFYERR_DATA	VerifyErr;
	// RevRxNo120910-006 Append End
	// RevRxNo140515-005 Append End	
	// RevRxNo161003-001 Append Start
	enum FFWRXENM_BANKSEL	eBankSel;
	BOOL	bBankselChangeFlg = FALSE;
	// RevRxNo161003-001 Append End

	pMcuInfo = GetMcuInfoDataRX();
	ePmode = GetPmodeDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	pDbgData = GetDbgDataRX();		// RevRxNo130730-001 Append Line


	ProtInit();

	// RevRxNo121206-001 Append Start
	bBootSwap = FALSE;
	bSysReset = FALSE;		// RevRxNo130730-001 Append Line
	bOsmFwrite = FALSE;		// RevRxNo130730-001 Append Line

	// RX100において、FISRレジスタによるブートスワップの対策処理。
	// SREST前にスワップしていた場合、SREST後にキャッシュをスワップする。
	// ただし、低消費電力モード中はChkBootSwap()内のFISR DUMPでエラーになってしまう。
	// エラーが発生してもSREST処理を実行するため、ChkBootSwap()の戻り値確認および
	// returnはしない。
	// RevRxNo130411-001 Modify Line, RevRxNo140515-006 Modify Line
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
		// フラッシュがMF3またはSC32用MF3の場合

		ferr = ChkBootSwap(&bBootSwap);			// IOレジスタからブートスワップの設定状態を確認する。
		// RevRxNo121211-001 Delete
	}
	// RevRxNo121206-001 Append End

	// RevRxNo140515-007 Append Start
	// エラーが発生してもSREST処理を実行するため、DO_SetSSEN()の戻り値確認および
	// returnはしない。
	if (GetSsstState() == TRUE) {	// SSSTコマンド発行状態の場合
		ferr = DO_SetSSEN();		// 連続ステップ処理の終了処理を実施
	}
	// RevRxNo140515-007 Append End

	ferr = PROT_MCU_SREST(&byResult);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo121206-001 Append Start
	if(bBootSwap == TRUE) {				// MCUのリセットによりブートスワップする→しないになった場合
		SwapFlashRomCacheData();		// クラスタ0と1のキャッシュを入れ替える
		bBootSwap = FALSE;				// キャッシュを入れ替えたため、フラグをFALSEに初期化	// RevRxNo130730-001 Append Line
	}
	// RevRxNo121206-001 Append End

	if(byResult == 0x01) {
		// 再接続完了
		// エンディアン状態を記憶(未リセット時に対応)
		ferr = SetEndianState();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	} else if(byResult == 0x02) {
		ferrEnd = ProtEnd();
		return FFWERR_ILLEGAL_DEVICEID;
	}else{
		ferrEnd = ProtEnd();
		return FFWERR_ILLEGAL_IDCODE;
	}

	// MCUをリセット後、SetPmodeInRomReg2Flg()発行前に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグを、TRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// MCUリセットにより内蔵ROM有効/無効状態が変化する可能性があるため、
	// SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End

	// RevNo110506-003 Append Line
	// SRESTが成功したので、IsMcuRun()で内部変数をブレーク状態にする
	SetMcuRunState(FALSE);		// ユーザプログラム実行状態を"プログラム停止中"にする

	// V.1.02 No.4 Modify Start
	// リセットベクタをユーザブートベクタに設定( FINEでもJTAGでも必要 )
	ferr = ClrFwRegData(REGCLRCMD_SREST);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo130730-001 Append Start
	if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {
		// プログラムROM のフラッシュ書き換えデバッグが「有効」の場合
		// (OFS1レジスタおよびMDEレジスタがプログラムROM領域に配置されているため)

		// オプション設定メモリ書き戻し判定および処理を実施
		ferr = beforeResetOsm(&bSysReset, &bOsmFwrite, &ferrWarning, &bBootSwap, &bBankselChangeFlg);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo130730-001 Append End

	// RevRxNo161003-001 Append Start
	if (GetBankSelChangeFlg() || bBankselChangeFlg) {	// ダウロード処理でBANKSWPビットを書き換えた、または、セルフプログラミングでBANKSWPビット書き換えた
		// FFWキャッシュメモリクリア
		InitCacheDataSetFlg(MAREA_USER);		// ユーザマットのキャッシュ取得情報クリア
		// 現在の起動バンク情報を取得し反対の情報を設定
		eBankSel = GetBankSelDataRX();
		if (eBankSel == RX_BANKSEL_BANK0) {				// 現在の起動バンクが0の場合は1に変わる
			eBankSel = RX_BANKSEL_BANK1;
		} else if (eBankSel == RX_BANKSEL_BANK1) {		// 現在の起動バンクが1の場合は0に変わる
			eBankSel = RX_BANKSEL_BANK0;
		} else {
			// ここには来ない
		}
		SetBankSelDataRX(eBankSel);				// s_eBankSelに設定
	}
	// RevRxNo161003-001 Append End

	// RevRxNo130301-001 Modify Start
	// RevNo110405-001 Append Start
	// 以下の場合、再リセットする。
	// (a)FINE接続の場合
	// 　　リセットを入れなおさないとdDMAが設定エンディアンで動作しないため。
	// 　　ユーザブートモード時は2回目のリセットは不要。
	// (b)RV40Fの場合
	// 　　BFW SREST処理内のFCUファーム転送によりRV40FはECCエラーが発生するため、
	// 　　リセットによりクリアする。
	// (c)オプション設定メモリを書き戻した場合
	// 　　オプション設定メモリを書き戻した後、リセットを入力しないと書き込んだ値が
	// 　　有効にならないため。
	// RevNo110506-002 Modify Line
	// RevRxNo130730-001 Append Line
	if (((pMcuInfo->byInterfaceType == IF_TYPE_1) && (ePmode != RX_PMODE_USRBOOT)) || (pFwCtrl->eFlashType == RX_FLASH_RV40F) || (bOsmFwrite == TRUE)) {
		ferr = McuRest();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// RevRxNo130730-001 Append Start
		// リセットによりブートプログラムが動作してCPUレジスタの値が変更される場合がある。
		// そのため、リセット後にはCPUレジスタ初期値をF/Wが設定する。
		// システムリセット処理中であるため、CPUレジスタ初期値指定はREGCLRCMD_SRESTとする。
		ferr = ClrFwRegData(REGCLRCMD_SREST);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxNo130730-001 Append End

		// RevRxNo130308-001-033 Delete // RSTFをクリアする処理を、McuRest()内へ移動のため。
	}
	// RevNo110405-001 Append End
	// RevRxNo130301-001 Modify End

	// RevRxNo130301-001 Modify Start
	// RV40F、ユーザブートモード時は上記McuRest()でOCDレジスタが初期化される。
	// このため、FPGA, OCDレジスタを再設定する関数SysReSet()は、McuRest()後に実行する。
	ferr = SysReSet();	// FFWでのシステム再設定処理実行
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo130301-001 Modify End

	// RevRxNo140515-005 Append Start	
	// メモリウェイト対応のためFFWMCUCmd_SREST()から移動。メモリウェイト処理はDO_CWRITE()中で実施のため単純に移動しただけとなった。
	// RevRxNo120910-006 Append Start
	pSrmData = GetSrmData();
	// Startルーチン有効、またはStopルーチン有効時
	if(((pSrmData->dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((pSrmData->dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)){
		pbyRwBuff = GetSrmWorkProgramCode(pSrmData);
		dwmadrWorkProgramStartAddr = pSrmData->dwmadrRamWorkAreaStartAddr + WORKPROGRAMSTARTADDR;
		dwmadrWorkProgramEndAddr   = dwmadrWorkProgramStartAddr + SRM_WORK_PROGRAM_LENGTH - 0x00000001;
		ferr = DO_CWRITE(dwmadrWorkProgramStartAddr, dwmadrWorkProgramEndAddr, VERIFY_OFF, pbyRwBuff, &VerifyErr);
	}
	// RevRxNo120910-006 Append End
	// RevRxNo140515-005 Append End

	// RevRxNo130730-001 Modify Start
	if (ferrWarning != FFWERR_OK) {
		s_ferrOsmFwriteWarning = ferrWarning;		// オプション設定メモリ書き戻し時のWarning情報を覚えておく
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	
	return ferr;
	// RevRxNo130730-001 Modify End
}
//RevNo100715-028 Modify End

//=============================================================================
/**
 * システム再設定処理
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
//RevNo100715-028 Modify Start
FFWERR SysReSet(void)	// RevRxNo130301-001 Modify Line
{
	FFWERR	ferr;
	FFW_RRMB_DATA*		pSetRrmb;		// 設定中のRAM モニタ設定情報
	FFWE20_EINF_DATA	einfData;
	DWORD				dwSetBlk;
	// RevRxNo130308-001 Append Start
	FFWMCU_DBG_DATA_RX*	pDbgData;
	FFWRX_CVM_DATA*	pCoverageMode_RX;
	FFWRX_CVB_DATA*	pCvb_RX;			// RevRxNo130308-001-035 Append Line

 	pDbgData = GetDbgDataRX();
	// RevRxNo130308-001 Append End

	//OCDレジスタ情報がクリアされるので、OCDレジスタならびにトレース情報などを再設定
	// トレース再設定
	// RevRxNo130308-001 Append Line
	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		ferr = SetTrace(TRC_BRK_ENABLE);					// トレースFPGAへの初期設定
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	// RevRxNo130308-001 Append Line
	}
	// 内部トレースはデータが残ってしまうため、OCDトレースクロック供給後にトレースデータクリア
	// RevRxNo130308-001 Append Line
	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		ferr = ClrTrcMemData();	// RevRxE2LNo141104-001 Modify Line
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	// RevRxNo130308-001 Append Line
	}
	
	// RRM再設定
	// E20 38PIN接続時設定
	getEinfData(&einfData);		// エミュレータ情報取得
	if ((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN)) {
		pSetRrmb = GetRrmbData();
		dwSetBlk = MAX_BLOCK_NUM;
		// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
		// DO_E20_SetRRMB()でメモリアクセスする前にSetPmodeInRomReg2Flg()を発行しておく。
		ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
		ferr = DO_E20_SetRRMB(dwSetBlk, pSetRrmb);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// パフォーマンス再設定
	ferr = SetPpcRegData();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// イベント再設定
	ferr = SetEvRegDataAll();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130308-001 Append Start
	// カバレッジ再設定
	// RevRxNo130308-001-020 Append Line
	// 注意：ここで、トレースメモリに格納されたカバレッジデータの"0"クリアも実施する。
	// 注意：ここで、"カバレッジのLOSTが発生したことを示すフラグ"、"ユーザリセットが発生したことを示すフラグ"
	// RevRxNo130308-001-020 Modify Line
	//       のクリアをする。
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		pCoverageMode_RX = GetCoverageMode_RXInfo();	// カバレッジ機能時のトレースモード格納データ構造体のstatic変数のアドレスを取得
		ferr = DO_SetRXCVM(pCoverageMode_RX);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevRxNo130308-001-035 Append Start
		pCvb_RX = GetCvb_RXInfo();	// カバレッジ計測領域格納データ構造体のstatic変数のアドレスを取得
		ferr = DO_SetRXCVB(CVB_MAX_BLOCK_BIT, pCvb_RX);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevRxNo130308-001-035 Append End
		// RevRxNo130308-001-020 Modify Line
		ferr = DO_ClrRXCVD();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo130308-001 Append End

	// V.1.02 覚書 No.36 Append Line
	//C2E/E2Cバッファクリア
	ferr = DO_ClrC2EE2C(0);

	return ferr;
}
//RevNo100715-028 Modify End

// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * リセット前のオプション設定メモリ書き戻し処理
 * @param  pbSrestFlg システムリセット実行有無フラグのポインタ
 * 			(TRUE：システムリセットを実行した FALSE：システムリセットを実行していない)
 * @param  pbFwriteFlg オプション設定メモリ書き戻し実施フラグのポインタ
 * 			(TRUE：オプション設定メモリの書き戻しを実施した FALSE：オプション設定メモリの書き戻しを実施していない)
 * @param  pferrWarning Warning情報格納変数のポインタ
 * @param  pbBootSwap セットによるスワップ有無フラグのポインタ
 * 			(TRUE：リセットでスワップする FALSE：リセットでスワップしない)
 * @param	pbBankselChangeFlg	BANKSELレジスタBANKSWPビット書き換え有無フラグのポインタ
 *			(TRUE：BANKSEL書き換えあり、FALSE：BANKSEL書き換えなし)
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR beforeResetOsm(BOOL* pbSrestFlg, BOOL* pbFwriteFlg, FFWERR* pferrWarning, BOOL* pbBootSwap, BOOL* pbBankselChangeFlg)
{
	FFWERR				ferr = FFWERR_OK;
	FFWERR				ferrEnd;
	BOOL				bEndianChange;		// エンディアンの変更がいるかどうかを示す
	// RevRxNo161003-001 Append Start
	BOOL				bBankmdChange;		// MDEレジスタBANKMDビットの変更がいるかどうかを示す
	BOOL				bBankselChange;		// BANKSELレジスタの変更がいるかどうかを示す
	// RevRxNo161003-001 Append End
	BOOL				bUbcodeChange;		// UBコードの変更がいるかどうかを示す
	BOOL				bDebugCont;			// デバッグ継続モード設定必要かどうかを示す
	BOOL				bExtraChange;		// Extra領域の変更が必要かを示す
	BOOL				bLvdChange;			// 電圧監視リセット無効設定が必要かどうかを示す
	BOOL				bPmodCmd;			// PMODコマンドから実行するかを示す
	BYTE				byResult;
	WORD				wReadData[1];		// 参照データ格納領域
	WORD				wMaskFentryr;
	MADDR				madrBlkStartAddrCluster0;
	MADDR				madrBlkEndAddrCluster0;
	MADDR				madrReadAddrCluster0;
	DWORD				dwBlkNoCluster0;
	DWORD				dwBlkSizeCluster0;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;
	enum FFWRXENM_PMODE	ePmode;				// 起動時に指定された端子設定起動モード
	FFWENM_ENDIAN		eMcuEndian;
	FFWRX_UBCODE_DATA	*pUbcode;
	FFWRX_UBAREA_CHANGEINFO_DATA	*pUbAreaChange;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	USER_ROMCACHE_RX	*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ

	pFwCtrl = GetFwCtrlData();
	ePmode = GetPmodeDataRX();
	eMcuEndian = GetEndianDataRX();			// エンディアン情報取得
	pUbcode = GetUBCodeDataRX();
	pUbAreaChange = GetUBCodeDataAreaChangeRX();
	pDbgData = GetDbgDataRX();								// デバッグ情報取得
	pMcuDef = GetMcuDefData();
	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得

	*pbSrestFlg = FALSE;
	*pbFwriteFlg = FALSE;

	// フラッシュ書き換え判断フラグ初期化
	bEndianChange = FALSE;				// エンディアン書き換えしない
	// RevRxNo161003-001 Append Start
	bBankmdChange = FALSE;				// バンクモード書き換えしない
	bBankselChange = FALSE;				// 起動バンク書き換えしない
	// RevRxNo161003-001 Append End
	bUbcodeChange = FALSE;				// UBコード書き換えしない
	bDebugCont = FALSE;					// デバッグ継続モード設定しない
	bExtraChange = FALSE;				// Extra領域書き換えしない
	bLvdChange = FALSE;					// 電圧監視リセットを無効設定しない
	bPmodCmd = FALSE;					// PMODコマンド以外からの発行

	ferr = GetFentryrRegData(&wReadData[0]);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	wMaskFentryr = MCU_REG_FLASH_FENTRYR_USERMAT_PEMODE | MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE;
	if ((wReadData[0] & wMaskFentryr) == MCU_REG_FLASH_FENTRYR_NO_PEMODE) {
		// P/Eモードではない場合のみ、書き戻し処理を実施する

		if (pFwCtrl->eFlashType != RX_FLASH_RV40F) {	// フラッシュがRV40F以外の場合
				ferr = CheckOsm_RC03F_MF3(ePmode, eMcuEndian, pUbcode, pUbAreaChange, &bEndianChange, &bUbcodeChange, &bDebugCont, &bLvdChange, pferrWarning, bPmodCmd);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
		} else {	// フラッシュがRV40Fの場合
				// RevRxNo161003-001 Modify Line
				ferr = CheckOsm_RV40F(ePmode, eMcuEndian, pUbcode, pUbAreaChange, &bEndianChange, &bBankmdChange, &bBankselChange, &bUbcodeChange, &bDebugCont, &bExtraChange, &bLvdChange, pferrWarning, bPmodCmd);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
		}

		ClrMdeUbcodeChaneFlg();		// ダウンロード時にMDE/UBコード書き換えが有ったかどうかを格納する変数を初期化
		// RevRxNo161003-001 Append Line
		ClrBankmdBankselChaneFlg();	// ダウンロード時にBANKMDビット、BANKSWPビット書き換えが有ったかどうかを格納する変数を初期化

		// RevRxNo161003-001 Modify Line
		if (bEndianChange || bBankmdChange || bBankselChange || bDebugCont || bExtraChange || bLvdChange) {
			// フラッシュ書き換えする場合
			// RevRxNo161003-001 Append Start
			if (bBankselChange) {	// BANKSELレジスタを書き換える場合
				*pbBankselChangeFlg = TRUE;
			}
			// RevRxNo161003-001 Append End

			if (pDbgData->eClkChangeEna == RX_CLKCHANGE_DIS) {	// クロック切り替えが無効の場合
				if(*pbBootSwap == TRUE) {			// MCUのリセットによりブートスワップする→しないになった場合
					if(GetNewCacheMem(MAREA_USER) == TRUE){		// ユーザマットのキャッシュを作成している場合
						// リセットによりスワップする場合、本関数を実行する時はクラスタ1のキャッシュは取得済みの状態となる。
						// SREST後にキャッシュデータをスワップする場合、リセット前のクラスタ0用キャッシュを確保する必要がある。
						madrReadAddrCluster0 = MCU_OSM_OFS1_START;
						GetFlashRomBlockInf(MAREA_USER, madrReadAddrCluster0, &madrBlkStartAddrCluster0, &madrBlkEndAddrCluster0, &dwBlkNoCluster0, &dwBlkSizeCluster0);

						if(um_ptr->pbyCacheSetFlag[dwBlkNoCluster0] == FALSE){
							ferr = UpdateFlashRomCache(MAREA_USER, dwBlkNoCluster0, madrBlkStartAddrCluster0, madrBlkEndAddrCluster0, dwBlkSizeCluster0);
							if(ferr != FFWERR_OK){
								return ferr;
							}
						}
					}
				}

				ferr = PROT_MCU_SREST(&byResult);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				*pbSrestFlg = TRUE;		// システムリセットを実行した

				if(*pbBootSwap == TRUE) {			// MCUのリセットによりブートスワップする→しないになった場合
					ferr = SwapFlashWriteFlag();	// クラスタ0と1のブロック書き換えフラグ設定を入れ替える
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					SwapFlashRomCacheData();		// クラスタ0と1のキャッシュを入れ替える
					*pbBootSwap = FALSE;			// キャッシュを入れ替えたため、フラグをFALSEに初期化
				}
			}

			if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40F以外の場合
				// リセットベクタを含むブロックのデータをキャッシュに取得
				ferr = GetFlashCacheRestVect(ePmode);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			}

			// リセット時のオプション設定メモリ書き戻しでフラッシュを書き換える時、
			// WriteFlashExec()内のMDE/UBコードのマスク処理を省略する。
			// WriteFlashExec()前後で省略用のフラグを設定する。
			s_bSkipMdeMskResetOsmFwrite = TRUE;
			ferr = WriteFlashExec(VERIFY_OFF);
			s_bSkipMdeMskResetOsmFwrite = FALSE;

			if (ferr != FFWERR_OK) {
				SetDwnpOpenData(FALSE);
				ferrEnd = ProtEnd();
				return ferr;
			}
			*pbFwriteFlg = TRUE;	//オプション設定メモリの書き戻しを実施した

			ClrFlashDataClearFlg();	// フラッシュROMブロック初期化情報を"未初期化"に設定
		}
	}

	return ferr;
}
// RevRxNo130730-001 Append End

//=============================================================================
/**
 * リセット時のエンディアン状態を記憶
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetEndianState(void)
{
	FFWERR						ferr = FFWERR_OK;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWENM_ENDIAN				eMcuEndian;
	enum FFWENM_ENDIAN			eGetMcuEndian;

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Modify Start
	// RevRxNo130411-001 Modify Line
	if ((pFwCtrl->eMdeType == RX_MDE_FLASH) || (pFwCtrl->eMdeType == RX_MDE_EXTRA)) {
			// MDEをフラッシュROMまたはExtra領域で設定するMCUの場合
		eMcuEndian = GetEndianDataRX();
		if( eMcuEndian == ENDIAN_LITTLE ){
			s_nEndianState = ENDIAN_LITTLE;
		}else{
			s_nEndianState = ENDIAN_BIG;
		}

	} else {	// MDEをMD端子で設定するMCUの場合
		// MCUからエンディアン情報を取得
		ferr = GetMcuEndian(&eGetMcuEndian);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		s_nEndianState = eGetMcuEndian;
	}
	// RevRxNo130411-001 Modify End

	return ferr;
}

//=============================================================================
/**
 * リセット時のエンディアン状態を取得
 * @param なし
 * @retval エンディアン状態
 */
//=============================================================================
WORD GetEndianState(void)
{
	if (s_nEndianState < 0) {
		SetEndianState();
	}

	return (WORD)s_nEndianState;
}

//=============================================================================
/**
 * リセットコマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Rst(void)
{
	s_nEndianState = -1;
	s_ferrOsmFwriteWarning = FFWERR_OK;		// RevRxNo130730-001 Append Line
	s_bSkipMdeMskResetOsmFwrite = FALSE;	// RevRxNo130730-001 Append Line

	return;
}

// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * オプション設定メモリ書き戻しWarning情報初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrOsmFwriteWarningFlg(void)
{
	s_ferrOsmFwriteWarning = FFWERR_OK;

	return;
}
// RevRxNo130730-001 Append End

// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * オプション設定メモリ書き戻しWarning情報取得
 * @param なし
 * @retval FFWERR_OK					正常終了
 * @retval FFWERR_ROM_MDE_CHANGE		MDEレジスタを指定された内容に書き換えた
 * @retval FFWERR_ROM_OFS1_CHANGE		OFS1レジスタを書き換えた
 * @retval FFWERR_ROM_MDE_OFS1_CHANGE	MDEレジスタおよびOFS1レジスタを書き換えた
 */
//=============================================================================
FFWERR GetOsmFwriteWarningFlg(void)
{
	return s_ferrOsmFwriteWarning;
}
// RevRxNo130730-001 Append End

// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * オプション設定メモリ書き戻し時のMDEマスク処理省略フラグ取得
 * @param なし
 * @retval s_ferrOsmFwriteWarning
 * 			(TRUE：リセットによるフラッシュ書き換え時のMDEマスク処理を省略する
 * 			 FALSE：リセットによるフラッシュ書き換え時のMDEマスク処理を省略しない)
 */
//=============================================================================
BOOL GetSkipMdeMskResetOsmFwrite(void)
{
	return s_bSkipMdeMskResetOsmFwrite;
}
// RevRxNo130730-001 Append End
