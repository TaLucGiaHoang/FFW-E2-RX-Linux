///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_dwn.cpp
 * @brief ダウンロード関連コマンドの実装ファイル
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, S.Ueda, K.Uemori, SDS T.Iwata, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/12/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-004 2012/11/21 三宅(2012/12/04 SDS 岩田 マージ)
　・FFWE20Cmd_DWNP_CLOSE()で、ワーニングのフラグs_bOfs1Lvd1renChangeFlgが
　　立っている場合、”ワーニングのフラグ”を寝かせてから、エラー(Warning)
　　FFWERR_WRITE_OFS1_LVD1REN_CHANGEを返すことを追加。
・RevRxNo120910-006	2012/11/12 三宅
  Start/Stop機能の高速化。
  ・FFWE20Cmd_DWNP_CLOSE()で、指定ルーチン用ワーク・プログラムのライト処理追加。
  ・ffwmcu_srm.h, domcu_mem.h のインクルード追加。
・RevRxNo120910-006	2012/11/21 三宅(2012/12/04 SDS 岩田 マージ)
  Start/Stop機能の高速化。
　・指定ルーチン用ワーク・プログラムのライト処理を DO_DWNP_CLOSE() へ移動。
・RevRxNo121122-005 2012/11/22 明石(2012/12/04 SDS 岩田 マージ)
  FFWMCUCmd_WRITEで、外部フラッシュメモリ　メーカIDデバイスID不一致エラー発生後、FFWCmd_GetErrorDetailでエラー詳細情報を
  取り込んだ後、FFWE20Cmd_DWNP_CLOSEを実行すると外部フラッシュメモリ メーカIDデバイスID不一致エラー発生するが、
  この時FFWCmd_GetErrorDetailでエラー詳細情報が取り込めないという不具合の修正
・RevRxNo140109-001 2014/01/23 植盛
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo140617-001 2014/06/26 大喜多
	TrustedMemory機能対応
・RevRxE2LNo141104-001 2014/12/22 上田
	E2 Lite対応
・RevRxNo150928-001 2015/09/28 PA 辻
	ROMキャッシュインバリデート Warning対応
	対応見送りのため、コメントアウト
・RevRxNo160527-001 2016/06/10 PA 辻
	RX651 正式対応
・RevRxNo161003-001 2016/12/01 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
*/
#include "domcu_dwn.h"
#include "ffw_sys.h"
#include "domcu_prog.h"
#include "errchk.h"
#include "ffwmcu_mcu.h"
#include "ffwmcu_dwn.h"
#include "domcu_mcu.h"
#include "mcu_extram.h"
#include "mcu_flash.h"
#include "mcu_extflash.h"
#include "mcu_mem.h"		// RevRxNo140109-001 Append Line
#include "ffwmcu_mem.h"		// RevRxNo140109-001 Append Line
#include "mcu_inram.h"		// RevRxE2LNo141104-001 Append Line
#include "do_sys.h"			// RevRxE2LNo141104-001 Append Line

// ファイル内static変数の宣言
static BOOL		s_bDwnpOpenFlg;			// DWNP実行中フラグ

// ファイル内static変数の宣言
// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Line
static FFWRX_FOVR_DATA	s_RxFovrData;		// フラッシュメモリ初期化ブロック格納変数

// ファイル内static変数の宣言
static BOOL s_bUserBootMatFclrFlg;			// ユーザーブートマット初期化管理変数 0：上書き、1：初期化

static FFW_UM_FCLR_DATA	s_UserFclrData;	
static FFW_DM_FCLR_DATA	s_DataFclrData;	

static FFW_FCLR_DATA	s_FclrData;		// フラッシュメモリ初期化ブロック格納変数


///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ユーザプログラムのダウンロード開始
 * @param なし
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_DWNP_OPEN(void)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	BOOL*	pbOfs1Lvd1renChangeFlg;	// フラグs_bOfs1Lvd1renChangeFlgのアドレスを格納	// RevRxNo140109-001 Append Line
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo140109-001 Append Line
	BOOL*	pbFawFsprChangeFlg;		// フラグs_bFawFsprChangeFlgのアドレスを格納		// RevRxNo160527-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	bHotPlugState = getHotPlugState();				// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo140109-001 Append Line

	// V.1.02 RevNo110523-001 Modyfy Line 
	// DO_DWNP_OPEN()後に発行していたがそれまでにエラーが発生した場合にMDE/UBコード書き換えワーニングも出してしまう
	// 場合があるためIsMcuRun()実行前に移動。
	// V.1.02 新デバイス対応 Append Line
	ClrMdeUbcodeChaneFlg();		// ダウンロード時にMDE/UBコード書き換えが有ったかどうかを格納する変数を初期化
	// RevRxNo161003-001 Append Line
	ClrBankmdBankselChaneFlg();	// ダウンロード時にBANKMDビット、BANKSWPビット書き換えが有ったかどうかを格納する変数を初期化
	
	// RevRxNo140109-001 Append Start
	ClrWriteErrFlg();			// WRITEコマンド処理実行時のエラー有無フラグを初期化

	pbOfs1Lvd1renChangeFlg = GetOfs1Lvd1renChangeFlgPtr();
	*pbOfs1Lvd1renChangeFlg = FALSE;			// LVD無効設定フラグを初期化
	// RevRxNo140109-001 Append End
	// RevRxNo160527-001 Append Start +2
	pbFawFsprChangeFlg = GetFawFsprChangeFlgPtr();
	*pbFawFsprChangeFlg = FALSE;				// FAW.FSPRビット差し替えフラグを初期化
	// RevRxNo160527-001 Append End


	// RevRxNo140109-001 Append Start
	if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
		ClrExistConfigDwnDataFlg();		// コンフィギュレーション設定領域へのダウンロードデータ有無フラグクリア
	}
	// RevRxNo140109-001 Append End

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}

// Ver.1.01 2010/08/17 SDS T.Iwata
	// メモリアクセス、内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = DO_DWNP_OPEN();

	if (ferr == FFWERR_OK) {
		SetDwnpOpenData(TRUE);
	}

	SetGageCount(0);		// 進捗ゲージに初期値セット
	SetMsgGageCount("");	// 進捗ダイアログ表示内容を消去

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned Start
	ExtramDwnpTop = NULL;
	SetExtramDwnpNum(0x00);
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * ユーザプログラムのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_DWNP_CLOSE(FFW_DWNP_CHKSUM_DATA* chksum)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo150928-001 Append Start +2
#if 0
	FFWERR	romc_ferr = FFWERR_OK;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
#endif
	// RevRxNo150928-001 Append End

	// RevRxNo160527-001 Append Line
	FFWERR	faw_ferr = FFWERR_OK;

	DWORD	dwCount;
	// RevRxNo120910-004 Append Line
	BOOL*	pbOfs1Lvd1renChangeFlg;	// フラグs_bOfs1Lvd1renChangeFlgのアドレスを格納
	// RevRxNo121122-005	Append Line
	FFW_ERROR_DETAIL_INFO*	pFerdiData;				// デバイスID メーカID不一致エラーの詳細情報テーブルのポインタ
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo140109-001 Append Line
	BOOL*	pbFawFsprChangeFlg;		// フラグs_bFawFsprChangeFlgのアドレスを格納		// RevRxNo160527-001 Append Line

	pFwCtrl = GetFwCtrlData();	// RevRxNo140109-001 Append Line

	// RevRxNo150928-001 Append Line
#if 0
	pMcuInfo = GetMcuInfoDataRX();
#endif

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

// V.1.02 CLOSEでのプログラム実行中エラーチェックは不要のため削除

// ExtFlashModule_002b Append Line
	ClrExistLockSectFlg();	// ロックされているセクタがあったかを示すフラグをクリア

	ferr = DO_DWNP_CLOSE(chksum);

	// RevRxE2LNo141104-001 Append Start
	if (ferr == FFWERR_MCURAM_VERIFY) {
		pFerdiData = GetInRamVerifyErrorDetailInfo();
		SetErrorDetailInfo(pFerdiData, INRAMVRFY_ERRD_NUM);
	// RevRxE2LNo141104-001 Append End

	// RevRxNo121122-005	Append Start
	} else if (ferr == FFWERR_EXTROM_ID) {	// RevRxE2LNo141104-001 Modify Line
		pFerdiData = GetExtFlashErrorDetailInfo();
		SetErrorDetailInfo(pFerdiData,EFLASHERR_MAX);

	} else if (ferr == FFWERR_EXTRAM_VERIFY) {	// RevRxE2LNo141104-001 Modify Line
	// RevRxNo121122-005	Append End
	// RevRxE2LNo141104-001 Append Start
		pFerdiData = GetExtRamVerifyErrorDetailInfo();
		SetErrorDetailInfo(pFerdiData, EXTRAMVRFY_ERRD_NUM);
	} else {
		// 何もしない
	}
	// RevRxE2LNo141104-001 Append End

	dwCount = GAGE_COUNT_MAX;
	SetGageCount(dwCount);
	SetDwnpOpenData(FALSE);

	// RevRxNo120910-004 Append Line
	if (GetWriteErrFlg() == FALSE) {	// WRITEコマンド処理中にエラーは発生しなかった場合
	// V.1.02 新デバイス対応 Append Start
		if (ferr == FFWERR_OK) {
			ferr = WarningChk_FlashWriteErr();		// 内蔵フラッシュダウンロード時のWarning確認
		}
	// V.1.02 新デバイス対応 Append End

		// RevRxNo140617-001 Modify Start
			// TM機能対応に伴いワーニング表示に優先順位を見直した。
			// FFWERR_DWN_OSMDATA_NONの優先順位がFFWERR_WRITE_OFS1_LVDAS_CHANGEより上になった。
		// RevRxNo140109-001 Modify Start
		// RevRxNo120910-004 Append Start
		// コンフィギュレーション設定領域へのダウンロードデータワーニング確認
		if(ferr == FFWERR_OK){
			if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
				if(ferr == FFWERR_OK){						// ワーニングが発生していない場合
					if( GetPmodeInRomDisFlg() == FALSE) {	// ROM有効の場合
						if (GetExistConfigDwnDataFlg() == FALSE) {	// 
							// コンフィギュレーション設定領域へのダウンロードデータがない場合
								ferr = FFWERR_DWN_OSMDATA_NON;		// エラー：オプション設定メモリへのダウンロードデータがない(Warning)
						}
					}
				}
			}
		}

		// RevRxNo150928-001 Append Start +8
#if 0
		// ROMキャッシュ動作を禁止に変更後インバリデート実施し、ROMキャッシュ動作を許可に戻したワーニング確認
		if(ferr == FFWERR_OK){
			if ((pMcuInfo->dwSpc[0] & SPC_ROMC_EXIST) == SPC_ROMC_EXIST) {	// ROMキャッシュ搭載品種の場合
				romc_ferr = WarningChk_BFW();
				if (romc_ferr == FFWERR_ROMC_BAN_INVALIDATE_PERMIT) {		// ROMキャッシュ動作を禁止に変更後インバリデート実施し、ROMキャッシュ動作を許可に戻した場合
					ferr = FFWERR_ROMC_BAN_INVALIDATE_PERMIT;				// ROMキャッシュ動作を禁止に変更後インバリデート実施し、ROMキャッシュ動作を許可に戻した(Warning)
				}
			}
		}
#endif
		// RevRxNo150928-001 Append End

		// OFS1レジスタ電圧監視リセット無効設定ワーニング確認
		if(ferr == FFWERR_OK){
			pbOfs1Lvd1renChangeFlg = GetOfs1Lvd1renChangeFlgPtr();
			if (*pbOfs1Lvd1renChangeFlg == TRUE) {
				// OFS1レジスタの電圧監視リセットを無効設定に変更したフラグがTRUEの場合
				
				if (pFwCtrl->eLvdErrCodeType == RX_LVDERR_CODE_LVD1REN) {		// FFWERR_WRITE_OFS1_LVD1REN_CHANGEを返すMCUの場合
					ferr = FFWERR_WRITE_OFS1_LVD1REN_CHANGE;	// エラー：OFS1レジスタへのライトデータを起動時電圧監視1リセット無効に変更した(Warning)
				} else if (pFwCtrl->eLvdErrCodeType == RX_LVDERR_CODE_LVDAS) {	// FFWERR_WRITE_OFS1_LVDAS_CHANGEを返すMCUの場合
					ferr = FFWERR_WRITE_OFS1_LVDAS_CHANGE;		// エラー：OFS1レジスタへのライトデータをリセット後電圧監視リセット無効に変更した(Warning)
				}
			}
		}
		// RevRxNo140617-001 Modify End

		// RevRxNo150928-001 Append Start +7
#if 0
		// ROMキャッシュインバリデートを実施したワーニング確認
		if(ferr == FFWERR_OK){
			if ((pMcuInfo->dwSpc[0] & SPC_ROMC_EXIST) == SPC_ROMC_EXIST) {	// ROMキャッシュ搭載品種の場合
				if (romc_ferr == FFWERR_ROMC_INVALIDATE) {					// ROMキャッシュインバリデートを実施した場合
					ferr = FFWERR_ROMC_INVALIDATE;							// ROMキャッシュインバリデートを実施した(Warning)
				}
			}
		}
#endif
		// RevRxNo150928-001 Append End
	}

	// RevRxNo140617-001 Append Start
	if(ferr == FFWERR_OK){		// ワーニング未発生
		if (GetDwnpDataIncludeTMArea() == TRUE) {	// TM領域へのダウンロードデータあり
			ferr = FFWERR_DWN_TMAREA;				// エラー：Trusted Memory領域にはダウンロードデータを書き込んでいません(Warning)
		}
	}
	// RevRxNo140617-001 Append End

	// RevRxNo161003-001 Append Start
	// BANKSELレジスタBANKSWPビット書き換え発生時ワーニング処理
	if(ferr == FFWERR_OK){
		if (GetBankSelChangeFlg()) {		// BANKSWPビットを書き換えた
			ferr = FFWERR_WRITE_BANKSWP;
		}
		if (GetBankSelNotChangeFlg()) {		// BANKSWPビットを書き換えた
			ferr = FFWERR_WRITE_BANKSEL_NOT_CHANGE;
		}
	}
	// RevRxNo161003-001 Append End

	// RevRxNo160527-001 Append Start +16
	// FAWレジスタFSPRビットによるプロテクトがすでに設定されているワーニング確認
	if(ferr == FFWERR_OK){
		if (pFwCtrl->bFawExist) {		// FAWレジスタありの場合
			faw_ferr = WarningChk_BFW();
			if (faw_ferr == FFWERR_FAW_FSPR_ALREADY_PROTECT) {		// FAWレジスタFSPRビットによるプロテクトがすでに設定されている場合
				ferr = FFWERR_FAW_FSPR_ALREADY_PROTECT;				// FAWレジスタFSPRビットによるプロテクトがすでに設定されている(Warning)
			}
		}
	}

	if(ferr == FFWERR_OK){		// ワーニング未発生
		if (pFwCtrl->bFawExist) {		// FAWレジスタありの場合
			pbFawFsprChangeFlg = GetFawFsprChangeFlgPtr();
			if (*pbFawFsprChangeFlg == TRUE) {
				ferr = FFWERR_WRITE_FAW_FSPR_CHANGE;				// エラー：FAWレジスタFSPRビットへのライトデータをプロテクトなしに変更した(Warning)
			}
		}
	}
	// RevRxNo160527-001 Appned End

// ExtFlashModule_002b Append Start
	// 外部フラッシュダウンロード時のワーニング確認
	if (ferr == FFWERR_OK) {					// 内蔵フラッシュへのダウンロードでWarningがなかったら外部のWarningチェックする
		ferr = WarningChk_ExtFlashWriteErr();	// 外部フラッシュダウンロード時のWarning確認
	}
// ExtFlashModule_002b Append End
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned Line
	DeleteExtramCacheMem();	// 外部RAMダウンロード領域解放

	// RevRxNo140109-001 Modify End

	DeleteInRamCacheMem();	// 内部RAMダウンロード領域解放	// RevRxE2LNo141104-001 Append Line


	// V.1.02 RevNo110613-001 Apped Start
	// BFWワーニング確認
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

// V.1.02 フラッシュブロック初期化指定変更対応 Append Start
//=============================================================================
/**
 * フラッシュメモリ上書きブロックの設定
 * @param pFovr ダウンロード時に上書きするフラッシュメモリ情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_SetFOVR(const FFWRX_FOVR_DATA* pFovr)
{
	FFWERR	ferr;
	DWORD	dwCnt;
	DWORD	dwMcuRomPtCnt, dwMcuRomBlkCnt;
	FFWMCU_MCUDEF_DATA	*pMcuDef;
	FFWMCU_MCUAREA_DATA_RX	*pMcuArea;
	BOOL	bAreaFlgS;
	BOOL	bAreaFlgE;
	BOOL	bBlockCheckFlgS;
	BOOL	bBlockCheckFlgE;
	BOOL	bBlockFlgS;
	BOOL	bBlockFlgE;
	MADDR	dwmadrAreaStart, dwmadrAreaEnd;
	MADDR	dwmadrBlockStart, dwmadrBlockEnd;

	MADDR	dwmadrUserStart = 0xFFFFFFFF;
	MADDR	dwmadrUserEnd = 0x00000000;
	MADDR	dwmadrDataStart = 0xFFFFFFFF;
	MADDR	dwmadrDataEnd = 0x00000000;
	
	BOOL	bUserAreaCheckFlg = FALSE;
	BOOL	bDataAreaCheckFlg = FALSE;
	BOOL	bUserBootAreaCheckFlg = FALSE;

	BOOL	bOverBlockFlg = FALSE;
	
	FFW_UM_FCLR_DATA *pUserFclr;	
	FFW_DM_FCLR_DATA *pDataFclr;	

	pMcuDef = GetMcuDefData();
	pMcuArea = GetMcuAreaDataRX();
	
	pUserFclr = GetUserFclrData();
	pDataFclr = GetDataFclrData();

	ferr = FFWERR_OK;

	// 引数エラーチェック
	if (pFovr->dwNum > FOVR_RX_BLKNUM_MAX) {	// フラッシュメモリ初期化ブロック数のチェック
		return FFWERR_FFW_ARG;
	}

	for (dwCnt = 0; dwCnt < pFovr->dwNum; dwCnt++) {
		if (pFovr->dwmadrFlashAreaStart[dwCnt] > pMcuDef->madrMaxAddr) {	// MCUアドレス空間の範囲内かチェック
			return FFWERR_FFW_ARG;
		}
		if(pFovr->dwmadrFlashAreaStart[dwCnt] > pFovr->dwmadrFlashAreaEnd[dwCnt]){	// 上書き先頭アドレスが上書き終了アドレスより大きいかチェック
			return FFWERR_FFW_ARG;
		}

		// 上書き領域が重複していないか確認
		for (DWORD i = 0; i < pFovr->dwNum; i++) {
			if(dwCnt != i){
				if(pFovr->dwmadrFlashAreaStart[dwCnt] <= pFovr->dwmadrFlashAreaStart[i] && pFovr->dwmadrFlashAreaStart[i] <= pFovr->dwmadrFlashAreaEnd[dwCnt]){
					return FFWERR_FFW_ARG;
				}
				if(pFovr->dwmadrFlashAreaStart[dwCnt] <= pFovr->dwmadrFlashAreaEnd[i] && pFovr->dwmadrFlashAreaStart[i] <= pFovr->dwmadrFlashAreaEnd[dwCnt]){	// 上書き終了アドレスが重複していないか確認
					return FFWERR_FFW_ARG;
				}
			}
		}
	}

	// ユーザーマット領域の先頭アドレス、終了アドレスを算出
	if(pMcuArea->dwFlashRomPatternNum == 0){
		dwmadrUserStart = 0;	// ユーザーマット領域パターン先頭アドレス算出	
		dwmadrUserEnd = 0;		// ユーザーマット領域パターン終了アドレス算出
	}
	else{
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwFlashRomPatternNum; dwMcuRomPtCnt++) {
			// ユーザーマット領域パターン先頭アドレス算出
			if(dwmadrUserStart > pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt]){	
				dwmadrUserStart = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt];	
			}
			// ユーザーマット領域パターン終了アドレス算出
			if(dwmadrUserEnd < pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + (pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]) -1){
				dwmadrUserEnd = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + (pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]) -1;
			}
		}
	}
	// データマット領域の先頭アドレス、終了アドレスを算出
	if(pMcuArea->dwDataFlashRomPatternNum == 0){
		dwmadrDataStart = 0;	// データマット領域パターン先頭アドレス算出
		dwmadrDataEnd = 0;		// データマット領域パターン終了アドレス算出
	}
	else{
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwDataFlashRomPatternNum; dwMcuRomPtCnt++) {
			// データマット領域パターン先頭アドレス算出
			if(dwmadrDataStart > pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt]){	
				dwmadrDataStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt];	
			}
			// データマット領域パターン終了アドレス算出
			if(dwmadrDataEnd < pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + (pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]) -1){
				dwmadrDataEnd = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + (pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]) -1;
			}
		}
	}

	// 上書き領域アドレスチェック
	for (dwCnt = 0; dwCnt < pFovr->dwNum; dwCnt++) {

		bAreaFlgS = FALSE;
		bAreaFlgE = FALSE;
		bBlockCheckFlgS = FALSE;
		bBlockCheckFlgE = FALSE;
		bBlockFlgS = FALSE;
		bBlockFlgE = FALSE;
		
		// ユーザーマット領域のアドレスチェック
		if(dwmadrUserStart <= pFovr->dwmadrFlashAreaStart[dwCnt] && pFovr->dwmadrFlashAreaEnd[dwCnt] <= dwmadrUserEnd){
			for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwFlashRomPatternNum; dwMcuRomPtCnt++) {
				// ユーザーマット領域パターン先頭アドレス算出
				dwmadrAreaStart = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt];	
				// ユーザーマット領域パターン終了アドレス算出
				dwmadrAreaEnd = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + (pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]) - 1;

				// 上書き先頭アドレスがユーザーマット領域パターンの範囲内であるか確認				
				if (dwmadrAreaStart <= pFovr->dwmadrFlashAreaStart[dwCnt] && pFovr->dwmadrFlashAreaStart[dwCnt] <= dwmadrAreaEnd) {
					bAreaFlgS = TRUE;
				}
				// 上書き終了アドレスがユーザーマット領域パターンの範囲内であるか確認				
				if (dwmadrAreaStart <= pFovr->dwmadrFlashAreaEnd[dwCnt] && pFovr->dwmadrFlashAreaEnd[dwCnt] <= dwmadrAreaEnd) {
					bAreaFlgE = TRUE;
				}
				if(bAreaFlgS == TRUE || bAreaFlgE == TRUE){
					for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt]; dwMcuRomBlkCnt++) {
						dwmadrBlockStart = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + (dwMcuRomBlkCnt * pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]);
						dwmadrBlockEnd = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + ((dwMcuRomBlkCnt + 1)*pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]) - 1;

						if(bAreaFlgS == TRUE && bBlockCheckFlgS == FALSE){
							if(dwmadrBlockStart <= pFovr->dwmadrFlashAreaStart[dwCnt] && pFovr->dwmadrFlashAreaStart[dwCnt] <= dwmadrBlockEnd){
								if(pFovr->dwmadrFlashAreaStart[dwCnt] == dwmadrBlockStart){
									bBlockFlgS = TRUE;
								}
								bBlockCheckFlgS = TRUE;
							}
						}
						if(bAreaFlgE == TRUE && bBlockCheckFlgE == FALSE){
							if(dwmadrBlockStart <= pFovr->dwmadrFlashAreaEnd[dwCnt] && pFovr->dwmadrFlashAreaEnd[dwCnt] <= dwmadrBlockEnd){
								if(pFovr->dwmadrFlashAreaEnd[dwCnt] == dwmadrBlockEnd){
									bBlockFlgE = TRUE;
								}
								bBlockCheckFlgE = TRUE;
							}
						}
						if((bBlockCheckFlgS == TRUE && bBlockCheckFlgE == TRUE)){
							break;
						}
					}
				}
				if((bBlockCheckFlgS == TRUE && bBlockCheckFlgE == TRUE)){
					break;
				}
			}
			if(bBlockFlgS == TRUE && bBlockFlgE == TRUE){
				bUserAreaCheckFlg = TRUE;
			}
			else{
				return FFWERR_FFW_ARG;
			}
		}


		// データマット領域のアドレスチェック
		else if(dwmadrDataStart <= pFovr->dwmadrFlashAreaStart[dwCnt] && pFovr->dwmadrFlashAreaEnd[dwCnt] <= dwmadrDataEnd){
			for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwDataFlashRomPatternNum; dwMcuRomPtCnt++) {
				// データマット領域パターン先頭アドレス算出
				dwmadrAreaStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt];	
				// データマット領域パターン終了アドレス算出
				dwmadrAreaEnd = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + (pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]) - 1;

				// 上書き先頭アドレスがデータマット領域パターンの範囲内であるか確認				
				if (dwmadrAreaStart <= pFovr->dwmadrFlashAreaStart[dwCnt] && pFovr->dwmadrFlashAreaStart[dwCnt] <= dwmadrAreaEnd) {
					bAreaFlgS = TRUE;
				}
				// 上書き終了アドレスがデータマット領域パターンの範囲内であるか確認				
				if (dwmadrAreaStart <= pFovr->dwmadrFlashAreaEnd[dwCnt] && pFovr->dwmadrFlashAreaEnd[dwCnt] <= dwmadrAreaEnd) {
					bAreaFlgE = TRUE;
				}
				if(bAreaFlgS == TRUE || bAreaFlgE == TRUE){
					for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt]; dwMcuRomBlkCnt++) {
						dwmadrBlockStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + (dwMcuRomBlkCnt * pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]);
						dwmadrBlockEnd = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + ((dwMcuRomBlkCnt + 1)*pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]) - 1;

						if(bAreaFlgS == TRUE && bBlockCheckFlgS == FALSE){
							if(dwmadrBlockStart <= pFovr->dwmadrFlashAreaStart[dwCnt] && pFovr->dwmadrFlashAreaStart[dwCnt] <= dwmadrBlockEnd){
								if(pFovr->dwmadrFlashAreaStart[dwCnt] == dwmadrBlockStart){
									bBlockFlgS = TRUE;
								}
								bBlockCheckFlgS = TRUE;
							}
						}
						if(bAreaFlgE == TRUE && bBlockCheckFlgE == FALSE){
							if(dwmadrBlockStart <= pFovr->dwmadrFlashAreaEnd[dwCnt] && pFovr->dwmadrFlashAreaEnd[dwCnt] <= dwmadrBlockEnd){
								if(pFovr->dwmadrFlashAreaEnd[dwCnt] == dwmadrBlockEnd){
									bBlockFlgE = TRUE;
								}
								bBlockCheckFlgE = TRUE;
							}
						}
						if((bBlockCheckFlgS == TRUE && bBlockCheckFlgE == TRUE)){
							break;
						}
					}
				}
				if((bBlockCheckFlgS == TRUE && bBlockCheckFlgE == TRUE)){
					break;
				}
			}
			if(bBlockFlgS == TRUE && bBlockFlgE == TRUE){
				bDataAreaCheckFlg = TRUE;
			}
			else{
				return FFWERR_FFW_ARG;
			}
		}
		
		// ユーザーブートマット領域のアドレスチェック
		else if(pMcuArea->dwmadrUserBootStart <= pFovr->dwmadrFlashAreaStart[dwCnt] && pFovr->dwmadrFlashAreaEnd[dwCnt] <= pMcuArea->dwmadrUserBootEnd){
			// V.1.02 RevNo110228-001 Modify Line
			if(!(pMcuArea->dwmadrUserBootStart == 0 && pMcuArea->dwmadrUserBootEnd == 0)){		// ユーザブート領域がある場合
				if(pMcuArea->dwmadrUserBootStart == pFovr->dwmadrFlashAreaStart[dwCnt] && pFovr->dwmadrFlashAreaEnd[dwCnt] == pMcuArea->dwmadrUserBootEnd){
					bUserBootAreaCheckFlg = TRUE;
				}
				else{
					return FFWERR_FFW_ARG;
				}
			// V.1.02 RevNo110228-001 Append Start
			}else{		// ユーザブート領域がない場合( 上書き指定が正しくない(開始0、終了0が該当) )
				return FFWERR_FFW_ARG;
			// V.1.02 RevNo110228-001 Append End
			}
		}	
		else{
			return FFWERR_FFW_ARG;
		}
	}

	// FFW内部管理構造体変数の更新
	memcpy( (void*)&s_RxFovrData, (const void*)pFovr, sizeof(FFWRX_FOVR_DATA) );

	// 初期化ブロック情報初期化
	memset(pUserFclr, 0, sizeof(FFW_UM_FCLR_DATA));	// ユーザーマット初期化管理構造体初期化 
	memset(pDataFclr, 0, sizeof(FFW_DM_FCLR_DATA)); // データマット初期化管理構造体初期化 
	SetUserBootMatFclr(FALSE);						// ユーザーブートマット初期化管理フラグを初期化

	// RevNo110216-001 Modify Start
	// ユーザーマット領域初期化ブロック情報設定
	if(bUserAreaCheckFlg == TRUE){		// ユーザマット領域に上書き領域指定があった場合
		pUserFclr->dwNum = 0;	// ユーザーマット初期化ブロック設定数を0に設定
		// MCUユーザーマット領域パターン数分以下の処理を行う
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwFlashRomPatternNum; dwMcuRomPtCnt++) {
			// V.1.02 RevNo110414-001 Delete Line 20110413
			// bOverBlockFlg = FALSE;	// 上書きブロックフラグをクリア
			// 該当領域パターンのブロック数分以下の処理を行う
			for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt]; dwMcuRomBlkCnt++) {
				// ブロック先頭アドレス算出
				dwmadrBlockStart = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + (dwMcuRomBlkCnt * pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]);
				// ブロック終了アドレス算出
				dwmadrBlockEnd = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + ((dwMcuRomBlkCnt + 1)*pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]) - 1;
				// V.1.02 RevNo110414-001 Append Line 20110413
				bOverBlockFlg = FALSE;	// 上書きブロックフラグをクリア
				// 該当ブロックが上書き領域に含まれるか検索
				for (dwCnt = 0; dwCnt < pFovr->dwNum; dwCnt++) {
					// 該当ブロックが上書き領域に含まれている場合
					if(pFovr->dwmadrFlashAreaStart[dwCnt] <= dwmadrBlockStart && dwmadrBlockEnd <= pFovr->dwmadrFlashAreaEnd[dwCnt]){
						// 上書きブロックフラグをセット
						bOverBlockFlg = TRUE;
						// ループから抜ける
						break;
					}
				}
				// 該当ブロックが上書き領域に含まれるか検索した結果、含まれない場合
				if(bOverBlockFlg == FALSE){
					// MCUブロックが上書き領域に含まれていない場合、初期化情報を登録
					pUserFclr->dwmadrBlkStart[pUserFclr->dwNum] = dwmadrBlockStart;	// MCUブロック先頭アドレスを初期化ブロック先頭アドレスに登録
					pUserFclr->dwmadrBlkEnd[pUserFclr->dwNum] = dwmadrBlockEnd;		// MCUブロック終了アドレスを初期化ブロック終了アドレスに登録
					pUserFclr->dwNum++;		// 初期化ブロック設定数を更新
				}
			}
		}
	}else{								// ユーザマット領域に上書き領域指定がなかった場合
		if(pMcuArea->dwFlashRomPatternNum > 0){
			SetAllBlockFclrData(pFovr,MAREA_USER);		//ユーザーマットの全ブロックを初期化ブロック情報に設定
		}
	}

	// データマット領域初期化ブロック情報設定
	if(bDataAreaCheckFlg == TRUE){		// データマット領域に上書き領域指定があった場合
		pDataFclr->dwNum = 0;	//データマット初期化ブロック設定数を0に設定
		// MCUデータマット領域パターン数分以下の処理を行う
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwDataFlashRomPatternNum; dwMcuRomPtCnt++) {
			// V.1.02 RevNo110414-001 Delete Line 20110413
			// bOverBlockFlg = FALSE;	// 上書きブロックフラグをクリア
			// 該当領域パターンのブロック数分以下の処理を行う
			for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt]; dwMcuRomBlkCnt++) {
				// ブロック先頭アドレス算出
				dwmadrBlockStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + (dwMcuRomBlkCnt * pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]);
				// ブロック終了アドレス算出
				dwmadrBlockEnd = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + ((dwMcuRomBlkCnt + 1)*pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]) - 1;
				// V.1.02 RevNo110414-001 Append Line 20110413
				bOverBlockFlg = FALSE;	// 上書きブロックフラグをクリア
				// 該当ブロックが上書き領域に含まれるか検索
				for (dwCnt = 0; dwCnt < pFovr->dwNum; dwCnt++) {
					// 該当ブロックが上書き領域に含まれている場合
					if(pFovr->dwmadrFlashAreaStart[dwCnt] <= dwmadrBlockStart && dwmadrBlockEnd <= pFovr->dwmadrFlashAreaEnd[dwCnt]){
						// 上書きブロックフラグをセット
						bOverBlockFlg = TRUE;
						// ループから抜ける
						break;
					}
				}
				// 該当ブロックが上書き領域に含まれるか検索した結果、含まれない場合
				if(bOverBlockFlg == FALSE){
					// MCUブロックが上書き領域に含まれていない場合、初期化情報を登録
				pDataFclr->dwmadrBlkStart[pDataFclr->dwNum] = dwmadrBlockStart;	// MCUブロック先頭アドレスを初期化ブロック先頭アドレスに登録
					pDataFclr->dwmadrBlkEnd[pDataFclr->dwNum] = dwmadrBlockEnd;		// MCUブロック終了アドレスを初期化ブロック終了アドレスに登録
					pDataFclr->dwNum++;		// 初期化ブロック設定数を更新
				}
			}
		}
	}else{								// データマット領域に上書き領域指定がなかった場合
		if(pMcuArea->dwDataFlashRomPatternNum > 0){
			SetAllBlockFclrData(pFovr,MAREA_DATA);		//データマットの全ブロックを初期化ブロック情報に設定
		}
	}
	// ユーザーブートマット領域初期化ブロック情報設定
	bOverBlockFlg = FALSE;	// 上書きブロックフラグをクリア
	if(bUserBootAreaCheckFlg == TRUE){		// ユーザブートマット領域に上書き領域指定があった場合
		for (dwCnt = 0; dwCnt < pFovr->dwNum; dwCnt++) {
			if(pFovr->dwmadrFlashAreaStart[dwCnt] <= pMcuArea->dwmadrUserBootStart && pMcuArea->dwmadrUserBootEnd <= pFovr->dwmadrFlashAreaEnd[dwCnt]){
				// 上書きブロックフラグをセット
				bOverBlockFlg = TRUE;
			}

		// V.1.02 RevNo110228-001 Append Line  全上書き領域をチェックしてから初期化ブロック設定をしないといけない
		}
		// 該当ブロックが上書き領域に含まれるか検索した結果、含まれない場合
		if(bOverBlockFlg == FALSE){
			// ユーザーブートモード起動時
			if(GetPmodeDataRX() == RX_PMODE_USRBOOT){
				// ユーザーブートマット初期化管理変数を初期化に設定する。 
				SetUserBootMatFclr(TRUE);
			}
		}
	}else{									// ユーザブートマット領域に上書き領域指定がなかった場合
		// V.1.02 RevNo110228-001 Modify Line
		if(!(pMcuArea->dwmadrUserBootStart == 0 && pMcuArea->dwmadrUserBootEnd == 0)){		// ユーザブート領域がある場合
			// V.1.02 RevNo110228-001 Append Line  ユーザブート起動時のみ初期化ブロック設定をすべき
			if(GetPmodeDataRX() == RX_PMODE_USRBOOT){		// ユーザーブートモード起動時
				SetAllBlockFclrData(pFovr,MAREA_USERBOOT);	//ユーザーブートマットの初期化ブロック情報を設定
			// V.1.02 RevNo110228-001 Append Line
			}
		}
	}
	// RevNo110216-001 Modify End

	return ferr;
}
// V.1.02 フラッシュブロック初期化指定変更対応 Append End

//=============================================================================
/**
 * フラッシュメモリ上書きブロックの参照
 * @param pFovr ダウンロード時に上書きするフラッシュメモリ情報を格納する構造体のアドレス
 * @retval FFWERR_OK
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetFOVR(FFWRX_FOVR_DATA* pFovr)
{
	memcpy( (void*)pFovr, (const void*)&s_RxFovrData, sizeof(FFWRX_FOVR_DATA) );

	return FFWERR_OK;
}



//=============================================================================
/**
 * 書き込みプログラムのロード
 * @param eWtrType 書き込みプログラムの種別
 * @param dwDataSize 書き込みプログラムのバイト数
 * @param pbyDataBuff 書き込みプログラムデータ格納バッファへのポインタ
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_WTRLOAD(enum FFWRXENM_WTRTYPE eWtrType, DWORD dwDataSize, const BYTE* pbyDataBuff)
{

	// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;

	FFWRX_MCUINFO_DATA	*pMcuInfoData;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuInfoData = GetMcuInfoDataRX();	// MCU空間情報を取得


	bHotPlugState = getHotPlugState();				// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}	
	
	if(eWtrType != WTRTYPE_0){	// WTRプログラム種別の引数チェック
		return FFWERR_FFW_ARG;
	}

	if(dwDataSize > ( pMcuInfoData->dwsizWorkRam + 0x10 - 0x100 )){	//WTRプログラムデータサイズの引数チェック
		return FFWERR_FFW_ARG;
	}

	ferr = DO_WTRLOAD(eWtrType, dwDataSize, pbyDataBuff);
	
	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 No.22 書き込みプログラムダウンロード化 Append End
	return ferr;
	// V.1.02 RevNo110613-001 Modify End
	
}

//=============================================================================
/**
 * 書き込みプログラムのバージョン参照
 * @param eWtrType 書き込みプログラムの種別
 * @param psWtrVer 書き込みプログラムバージョン格納バッファへのポインタ
 * @retval FFWERR_OK
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetWTRVER(enum FFWRXENM_WTRTYPE eWtrType, char *const psWtrVer)
{
	
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	
	if(eWtrType != WTRTYPE_0){	// WTRプログラム種別の引数チェック
		return FFWERR_FFW_ARG;
	}

	ferr = DO_WTRVER(eWtrType, psWtrVer);

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}

//=============================================================================
/**
 * フラッシュメモリ初期化ブロックの設定
 * @param pFclr ダウンロード時に初期化するフラッシュメモリ情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWCmd_SetFCLR(const FFW_FCLR_DATA* pFclr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUAREA_DATA_RX	*pMcuArea;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	pMcuArea = GetMcuAreaDataRX();
	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		return FFWERR_FUNC_UNSUPORT;
	}
	// RevRxE2LNo141104-001 Append End

	// 引数エラーチェック
	if (pFclr->dwNum > FCLR_BLKNUM_MAX) {	// フラッシュメモリ初期化ブロック数のチェック
		return FFWERR_FFW_ARG;
	}

	DO_SetFCLR();

	// FFW内部管理構造体変数の更新
	memcpy( (void*)&s_FclrData, (const void*)pFclr, sizeof(FFW_FCLR_DATA) );

	return ferr;
}

//=============================================================================
/**
 * フラッシュメモリ初期化ブロックの参照
 * @param pFclr ダウンロード時に初期化するフラッシュメモリ情報を格納する構造体のアドレス
 * @retval FFWERR_OK
 */
//=============================================================================
FFWE100_API	DWORD FFWCmd_GetFCLR(FFW_FCLR_DATA* pFclr)
{
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		return FFWERR_FUNC_UNSUPORT;
	}
	// RevRxE2LNo141104-001 Append End

	memcpy( (void*)pFclr, (const void*)&s_FclrData, sizeof(FFW_FCLR_DATA) );

	return FFWERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
// FFW内部関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * DWNPコマンド実行中フラグの参照
 * @param なし
 * @retval TRUE		DWNP実行中である(FFWMCUCmd_DWNP_OPEN発行後、FFWMCUCmd_DWNP_CLOSE発行前まで)
 * @retval FALSE	DWNP実行中でない
 */
//=============================================================================
BOOL GetDwnpOpenData(void)
{
	return s_bDwnpOpenFlg;
}

//=============================================================================
/**
 * DWNPコマンド実行中フラグの設定
 * @param BOOL  bDwnpOpenFlg	DWNP実行中指定
 * @retval なし
 */
//=============================================================================
void SetDwnpOpenData(BOOL	bDwnpOpenFlg)
{
	s_bDwnpOpenFlg = bDwnpOpenFlg;
}

// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Start
//=============================================================================
/**
 * ダウンロード時のフラッシュ初期化ブロック情報の参照
 * @param なし
 * @retval フラッシュ初期化ブロック情報管理構造体へのポインタ
 */
//=============================================================================
FFWRX_FOVR_DATA* GetRxFovrData(void)
{
	return &s_RxFovrData;
}

// V.1.02 No.8 フラッシュROMブロック細分化対応 Append End

//=============================================================================
/**
 * ユーザーブート領域初期化フラグの参照
 * @param なし
 * @retval TRUE		初期化
 * @retval FALSE	上書き
 */
//=============================================================================
BOOL GetUserBootMatFclr(void)
{
	return s_bUserBootMatFclrFlg;
}


//=============================================================================
/**
 * ユーザーブート領域初期化フラグの設定
 * @param BOOL  bUserBootFclrFlg
 *	初期化指定 TRUE：初期化、FALSE、上書き
 * @retval なし
 */
//=============================================================================
void SetUserBootMatFclr(BOOL bUserBootFclrFlg)
{
	s_bUserBootMatFclrFlg = bUserBootFclrFlg;
}


//=============================================================================
/**
 * 初期化ブロック情報の設定
 * @param pFovr　上書き領域情報格納構造体のアドレス
 * @param eAreaType Flash領域の種別情報
 * @retval なし
 */
//=============================================================================
// RevNo110224-004 Modify Line
void SetAllBlockFclrData(const FFWRX_FOVR_DATA* pFovr,enum FFWRXENM_MAREA_TYPE eAreaType)
{

	DWORD	dwMcuRomPtCnt;
	DWORD	dwMcuRomBlkCnt;
	MADDR	dwmadrBlockStart;
	MADDR	dwmadrBlockEnd;

	FFWMCU_MCUAREA_DATA_RX	*pMcuArea;

	FFW_UM_FCLR_DATA *pUserFclr;
	FFW_DM_FCLR_DATA *pDataFclr;
	
	//ワーニング対策
	pFovr;

	pMcuArea = GetMcuAreaDataRX();
	
	pUserFclr = GetUserFclrData();
	pDataFclr = GetDataFclrData();

	// RevNo110224-004 Append Line
	if(eAreaType == MAREA_USER){
		// ユーザーマット領域初期化ブロック情報設定
		pUserFclr->dwNum = 0;	// FOVR NONの場合
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwFlashRomPatternNum; dwMcuRomPtCnt++) {
			for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt]; dwMcuRomBlkCnt++) {
				dwmadrBlockStart = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + (dwMcuRomBlkCnt * pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]);
				dwmadrBlockEnd = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + ((dwMcuRomBlkCnt + 1)*pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]) - 1;

				pUserFclr->dwmadrBlkStart[pUserFclr->dwNum] = dwmadrBlockStart;
				pUserFclr->dwmadrBlkEnd[pUserFclr->dwNum] = dwmadrBlockEnd;
				pUserFclr->dwNum++;
			}
		}
	// RevNo110224-004 Append Line
	} else if(eAreaType == MAREA_DATA){
		// データマット領域初期化ブロック情報設定
		pDataFclr->dwNum = 0;
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwDataFlashRomPatternNum; dwMcuRomPtCnt++) {
			for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt]; dwMcuRomBlkCnt++) {
				dwmadrBlockStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + (dwMcuRomBlkCnt * pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]);
				dwmadrBlockEnd = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + ((dwMcuRomBlkCnt + 1)*pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]) - 1;

				pDataFclr->dwmadrBlkStart[pDataFclr->dwNum] = dwmadrBlockStart;
				pDataFclr->dwmadrBlkEnd[pDataFclr->dwNum] = dwmadrBlockEnd;
				pDataFclr->dwNum++;
			}
		}
	// RevNo110224-004 Append Line
	}else{
		// ユーザーブートマット領域初期化ブロック情報設定
		if(pMcuArea->dwmadrUserBootStart != 0 && pMcuArea->dwmadrUserBootEnd != 0){
			// V.1.02 RevNo110324-003 Modify Line
			if(GetPmodeDataRX() == RX_PMODE_USRBOOT){	// ユーザブート起動の場合のみ初期化ブロックとする
				SetUserBootMatFclr(TRUE);
			}
			else{
				SetUserBootMatFclr(FALSE);
			}
		}
	// RevNo110224-004 Append Line
	}

}

//=============================================================================
/**
 * ユーザーマット領域FCLR情報の取得
 * @param なし
 * @retval ユーザーマット領域FCLR情報管理構造体へのポインタ
 */
//=============================================================================
FFW_UM_FCLR_DATA* GetUserFclrData(void)
{
	return &s_UserFclrData;
}

//=============================================================================
/**
 * データマット領域FCLR情報の取得
 * @param なし
 * @retval データマット領域FCLR情報管理構造体へのポインタ
 */
//=============================================================================
FFW_DM_FCLR_DATA* GetDataFclrData(void)
{
	return &s_DataFclrData;
}

//=============================================================================
/**
 * プログラムダウンロードコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Dwn(void)
{
	s_bDwnpOpenFlg = FALSE;	// DWNP実行中フラグ

	s_RxFovrData.dwNum = INIT_FCLR_RX_NUM;
	// V.1.02 フラッシュブロック初期化指定変更対応 Append Start
	s_UserFclrData.dwNum = INIT_FCLR_RX_NUM;
	s_DataFclrData.dwNum = INIT_FCLR_RX_NUM;
	SetUserBootMatFclr(FALSE);		// 上書き領域として扱う
	// V.1.02 フラッシュブロック初期化指定変更対応 Append End

	s_FclrData.dwNum = INIT_FCLR_RX_NUM;

	return;

}

