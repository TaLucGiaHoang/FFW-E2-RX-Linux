///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_cv.cpp
 * @brief カバレッジ関連コマンドの実装ファイル
 * @author RSD Y.Miyake, S.Ueda
 * @author Copyright (C) 2013(2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/20
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo130308-001 2013/07/19 三宅
　カバレッジ開発対応
・RevRxNo130910-002 2013/09/10 上田
　CVBコマンド引数エラーチェック修正
・RevRxNo130730-006	2013/11/13 上田
　E20トレースクロック遅延設定タイミング変更
・RevRxE2LNo141104-001 2014/11/20 上田
	E2 Lite対応
*/
#include "ffwrx_cv.h"
#include "ffwmcu_mcu.h"
#include "mcudef.h"
#include "errchk.h"
#include "dorx_cv.h"
#include "domcu_prog.h"
#include "hwrx_fpga_tra.h"	// RevRxNo130730-006 Append Line


// static変数
static FFWRX_CVM_DATA	s_CoverageMode_RX;		// カバレッジ機能時のトレースモード格納データ
static FFWRX_CVB_DATA	s_Cvb_RX;				// カバレッジ計測領域格納データ
static BOOL				s_bCoverageLostFlag;	// カバレッジのLOSTが発生したことを示すフラグ
												//　TRUE： LOSTが発生した
												//　FALSE：LOSTが発生していない(DEFAULT)
static BOOL				s_bCoverageResetFlag;	// ユーザリセットが発生したことを示すフラグ
												//　TRUE： ユーザリセットが発生した
												//　FALSE：ユーザリセットが発生していない(DEFAULT)


///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * カバレッジ機能のモード設定
 * @param pCoverageMode カバレッジ機能時のトレースモードを格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_SetCVM(const FFWRX_CVM_DATA* pCoverageMode)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

 	pDbgData = GetDbgDataRX();

	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		return FFWERR_CV_UNSUPPORT;						// 「カバレッジ機能はサポートしていない。」エラーを返す。
	}

	if (GetMcuRunState()) {	// プログラム実行中の場合 // RevRxE2LNo141104-001 Modify Line
		return FFWERR_BMCU_RUN;		// 「ユーザプログラム実行中のためコマンド処理を実行できない」エラーを返す。
	}

	// RevRxNo130308-001-024 Append Start
	// 引数エラーチェック
	// トレースデータ出力端子幅について
	// RevRxNo130308-001-022 Modify Start
	if (((pCoverageMode->dwSetTBW & CVM_BIT_3_0_1MASK) != CVM_TBW_DATA4_SYNC1) && 
		((pCoverageMode->dwSetTBW & CVM_BIT_3_0_1MASK) != CVM_TBW_DATA8_SYNC2) && 
		((pCoverageMode->dwSetTBW & CVM_BIT_3_0_1MASK) != CVM_TBW_DATA8_SYNC1)) {
		return FFWERR_FFW_ARG;
	}
	// トレース情報取得モードについて
	if (((pCoverageMode->dwSetTRM & CVM_BIT_0_0_1MASK) != CVM_TRM_REALTIME) && 
		((pCoverageMode->dwSetTRM & CVM_BIT_0_0_1MASK) != CVM_TRM_TRCFULL)) {
		return FFWERR_FFW_ARG;
	}
	// トレースクロック比について
	if (((pCoverageMode->dwSetTRC & CVM_BIT_2_0_1MASK) != CVM_TRC_RATIO_1_1) && 
		((pCoverageMode->dwSetTRC & CVM_BIT_2_0_1MASK) != CVM_TRC_RATIO_2_1) && 
		((pCoverageMode->dwSetTRC & CVM_BIT_2_0_1MASK) != CVM_TRC_RATIO_4_1)) {
		return FFWERR_FFW_ARG;
	}
	// RevRxNo130308-001-022 Modify End
	// RevRxNo130308-001-024 Append End

	// E20トレースFPGA全レジスタ設定を指示
	SetTrcE20FpgaAllRegFlg(TRUE);	// RevRxNo130730-006 Append Line

	// カバレッジ機能のモード設定実施
	ferr = DO_SetRXCVM(pCoverageMode);
	if(ferr != FFWERR_OK) {
		return ferr;
	}

	// E20トレースFPGA全レジスタ設定指示を解除
	SetTrcE20FpgaAllRegFlg(FALSE);	// RevRxNo130730-006 Append Line

	// static変数 s_CoverageMode_RX に、pCoverageModeの内容を保存する。
	memcpy(&s_CoverageMode_RX, pCoverageMode, sizeof(FFWRX_CVM_DATA));

	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
}

//=============================================================================
/**
 * カバレッジ機能のモード参照
 * @param pCoverageMode カバレッジ機能時のトレースモードを格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetCVM(FFWRX_CVM_DATA *const pCoverageMode)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_DBG_DATA_RX*	pDbgData;

 	pDbgData = GetDbgDataRX();

	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		return FFWERR_CV_UNSUPPORT;						// 「カバレッジ機能はサポートしていない。」エラーを返す。
	}

	// 引数へ、FFW内部管理構造体変数から内容を渡す。
	memcpy( (void*)pCoverageMode, (const void*)&s_CoverageMode_RX, sizeof(FFWRX_CVM_DATA) );

	return ferr;
}


//=============================================================================
/**
 * カバレッジ計測ベースアドレス設定
 * @param dwSetBlk カバレッジ計測領域を設定するブロック0～3をビットフィールドで指定
 * @param pCvb     カバレッジ計測領域を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_SetCVB(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvb)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	FFWRX_CVB_DATA		CvbData;
	DWORD				dwBlockBit, dwBlockBit2;	// ブロックのビット位置を表す変数
	INT					i, j;

	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

 	pDbgData = GetDbgDataRX();
	pMcuDef = GetMcuDefData();

	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		return FFWERR_CV_UNSUPPORT;						// 「カバレッジ機能はサポートしていない。」エラーを返す。
	}

	if (GetMcuRunState()) {	// プログラム実行中の場合 // RevRxE2LNo141104-001 Modify Line
		return FFWERR_BMCU_RUN;		// 「ユーザプログラム実行中のためコマンド処理を実行できない」エラーを返す。
	}

	// RevRxNo130910-002 Modify Start
	// 引数エラーチェック
	// カバレッジ計測領域を設定するブロックが4つを超える場合
	// RevRxNo130308-001-004 Modify Line
	if ((dwSetBlk & ~CVB_MAX_BLOCK_BIT) != CVB_NON_BLOCK_BIT) {
		return FFWERR_FFW_ARG;
	}

	// カバレッジ計測領域を設定するブロックが無い場合
	// RevRxNo130308-001-004 Modify Line
	if ((dwSetBlk & CVB_MAX_BLOCK_BIT) == CVB_NON_BLOCK_BIT) {
		return FFWERR_OK;	// 何もせず正常終了
	}
	// RevRxNo130910-002 Modify End

	memcpy( (void*)&CvbData, (const void*)pCvb, sizeof(FFWRX_CVB_DATA) );
	for (i = 0; i < CV_RX_BLKNUM_MAX; i++) {
		dwBlockBit = CVB_BLOCK0_BIT << i;
		if ((dwSetBlk & dwBlockBit) != CVB_NON_BLOCK_BIT) { 	// 設定変更対象ブロックの場合
			if (CvbData.eEnable[i] == RX_CVB_ENA) { 		// 指定ブロックのカバレッジ動作が許可指定の場合
				if (CvbData.dwmadrBase[i] > pMcuDef->madrMaxAddr) {	// 最大アドレスを超える場合
					return FFWERR_FFW_ARG;								// 引数エラー
				}
				CvbData.dwmadrBase[i] = CvbData.dwmadrBase[i] & CVB_BIT_CLR_21_0;	// 21～0bitのアドレスをクリア
			} else {										// 指定ブロックのカバレッジ動作が禁止指定の場合
				CvbData.dwmadrBase[i] = CVB_BASE_ADDRESS_DEFAULT;					// "0"を設定（GetCVBをしたときに、"0"が見えるようにするため。）
			}
		}
	}

	// 設定変更対象で、かつカバレッジ動作が許可指定のカバレッジベースアドレスについて、重複していた場合
	for (i = 0; i < CV_RX_BLKNUM_MAX; i++) {
		dwBlockBit = CVB_BLOCK0_BIT << i;
		if (((dwSetBlk & dwBlockBit) != CVB_NON_BLOCK_BIT) && 	// 設定変更対象ブロックで、かつ
			(CvbData.eEnable[i] == RX_CVB_ENA)) { 		// 指定ブロックのカバレッジ動作が許可指定の場合
			for (j = i + 1; j < CV_RX_BLKNUM_MAX; j++) {
				dwBlockBit2 = CVB_BLOCK0_BIT << j;
				if (((dwSetBlk & dwBlockBit2) != CVB_NON_BLOCK_BIT) && 	// 設定変更対象ブロックで、かつ
					(CvbData.eEnable[j] == RX_CVB_ENA)) { 			// 指定ブロックのカバレッジ動作が許可指定の場合
					if (CvbData.dwmadrBase[i] == CvbData.dwmadrBase[j]) {	// 重複していた場合
						return FFWERR_CVAREA_SAME;								// エラー「カバレッジ計測設定領域が重複している」
					}
				}
			}
		}
	}

	// 設定変更対象で、かつカバレッジ動作が許可指定のカバレッジベースアドレスについて、
	// 設定変更対象でない、かつ現在動作許可状態ブロックのベースアドレスと重複していた場合
	for (i = 0; i < CV_RX_BLKNUM_MAX; i++) {
		dwBlockBit = CVB_BLOCK0_BIT << i;
		if (((dwSetBlk & dwBlockBit) != CVB_NON_BLOCK_BIT) && 	// 設定変更対象ブロックで、かつ
			(CvbData.eEnable[i] == RX_CVB_ENA)) { 		// 指定ブロックのカバレッジ動作が許可指定の場合
			// RevRxNo130308-001-014 Modify Start
			for (j = 0; j < CV_RX_BLKNUM_MAX; j++) {
				if (j != i) {
					dwBlockBit2 = CVB_BLOCK0_BIT << j;
					if (((dwSetBlk & dwBlockBit2) == CVB_NON_BLOCK_BIT) && 	// 設定変更対象ブロックでなく、かつ
						(s_Cvb_RX.eEnable[j] == RX_CVB_ENA)) { 			// 現在動作許可状態の場合
						if (CvbData.dwmadrBase[i] == s_Cvb_RX.dwmadrBase[j]) {		// 重複していた場合
							return FFWERR_CVAREA_SAME;								// エラー「カバレッジ計測設定領域が重複している」
						}
					}
				}
			}
			// RevRxNo130308-001-014 Modify End
		}
	}

	// カバレッジ計測領域設定実施
	ferr = DO_SetRXCVB(dwSetBlk, &CvbData);
	if(ferr != FFWERR_OK) {
		return ferr;
	}

	// static変数 s_Cvb_RX に、設定変更対象ブロックについての内容を保存する。
	for (i = 0; i < CV_RX_BLKNUM_MAX; i++) {
		dwBlockBit = CVB_BLOCK0_BIT << i;
		if ((dwSetBlk & dwBlockBit) != CVB_NON_BLOCK_BIT) { 	// 設定変更対象ブロックについて
			s_Cvb_RX.eEnable[i]    = CvbData.eEnable[i];		// s_Cvb に許可/不許可設定
			s_Cvb_RX.dwmadrBase[i] = CvbData.dwmadrBase[i];		// s_Cvb_RX にベースアドレス設定
		}
	}

	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
}


//=============================================================================
/**
 * カバレッジ計測ベースアドレス参照
 * @param pCvb カバレッジ計測領域を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetCVB(FFWRX_CVB_DATA *const pCvb)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_DBG_DATA_RX*	pDbgData;

 	pDbgData = GetDbgDataRX();

	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		return FFWERR_CV_UNSUPPORT;						// 「カバレッジ機能はサポートしていない。」エラーを返す。
	}

	// 引数へ、FFW内部管理構造体変数から内容を渡す。
	memcpy( (void*)pCvb, (const void*)&s_Cvb_RX, sizeof(FFWRX_CVB_DATA) );

	return ferr;
}


//=============================================================================
/**
 * C0カバレッジデータの取得
 * @param eBlkNo  C0カバレッジデータを取得するブロック番号
 * @param dwStart C0カバレッジデータを取得する領域の開始アドレス
 * @param dwEnd   C0カバレッジデータを取得する領域の終了アドレス
 * @param dwCvData[]     カバレッジデータを格納する配列のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetCVD0(enum FFWRXENM_CV_BLKNO eBlkNo, 
								   DWORD dwStart, DWORD dwEnd, DWORD dwCvData[])
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

 	pDbgData = GetDbgDataRX();

	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		return FFWERR_CV_UNSUPPORT;						// 「カバレッジ機能はサポートしていない。」エラーを返す。
	}

	if (GetMcuRunState()) {	// プログラム実行中の場合 // RevRxE2LNo141104-001 Modify Line
		return FFWERR_BMCU_RUN;		// 「ユーザプログラム実行中のためコマンド処理を実行できない」エラーを返す。
	}

	// 引数エラーチェック
	// RevRxNo130308-001-024 Append Start	//RevRxNo130308-001-015 Modify Line
	if ((eBlkNo >= CV_RX_BLKNUM_MAX) || (eBlkNo < 0)) {	// ブロック番号が"最大ブロック数"以上の場合
		return FFWERR_FFW_ARG;		// 引数エラー
	}
	// RevRxNo130308-001-024 Append End
	if (s_Cvb_RX.eEnable[eBlkNo] == RX_CVB_DIS) {	// カバレッジの動作禁止ブロックの場合
		return FFWERR_CVBLK_DIS;	// 「指定されたカバレッジ計測ブロックは動作禁止状態である」エラーを返す。
	}
	if (dwStart > CVD0_MAX_ADDRESS) {	// 0x003FFFFF より大きい場合
		return FFWERR_FFW_ARG;		// 引数エラー
	}
	if (dwEnd > CVD0_MAX_ADDRESS) {	// 0x003FFFFF より大きい場合
		return FFWERR_FFW_ARG;		// 引数エラー
	}
	if (dwStart > dwEnd) {	// 開始アドレスの方が終了アドレスより大きい場合
		return FFWERR_FFW_ARG;		// 引数エラー
	}

	// C0カバレッジデータの取得実施
	ferr = DO_GetRXCVD0(eBlkNo, dwStart, dwEnd, dwCvData);
	if(ferr != FFWERR_OK) {
		return ferr;
	}

	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
}


//=============================================================================
/**
 * カバレッジデータのクリア
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_ClrCVD(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

 	pDbgData = GetDbgDataRX();

	if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
		return FFWERR_CV_UNSUPPORT;						// 「カバレッジ機能はサポートしていない。」エラーを返す。
	}

	if (GetMcuRunState()) {	// プログラム実行中の場合 // RevRxE2LNo141104-001 Modify Line
		return FFWERR_BMCU_RUN;		// 「ユーザプログラム実行中のためコマンド処理を実行できない」エラーを返す。
	}

	// カバレッジデータのクリア実施
	ferr = DO_ClrRXCVD();
	if(ferr != FFWERR_OK) {
		return ferr;
	}

	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
}


//=============================================================================
/**
 * カバレッジ計測領域格納データ構造体のstatic変数のアドレスを取得
 * @param  なし
 * @retval カバレッジ計測領域格納データ構造体のstatic変数のアドレス
 */
//=============================================================================
FFWRX_CVB_DATA* GetCvb_RXInfo(void)
{
	return &s_Cvb_RX;
}


//=============================================================================
/**
 * カバレッジ機能時のトレースモード格納データ構造体のstatic変数のアドレスを取得
 * @param  なし
 * @retval カバレッジ機能時のトレースモード格納データ構造体のstatic変数のアドレス
 */
//=============================================================================
FFWRX_CVM_DATA* GetCoverageMode_RXInfo(void)
{
	return &s_CoverageMode_RX;
}


//=============================================================================
/**
 * カバレッジのLOSTが発生したことを示すフラグを取得する。
 * @param なし
 * @retval TRUE  LOSTが発生した
 * @retval FALSE LOSTが発生していない
 */
//=============================================================================
BOOL GetCoverageLostFlag(void)
{
	return s_bCoverageLostFlag;
}


//=============================================================================
/**
 * カバレッジのLOSTが発生したことを示すフラグに設定する。
 * @param BOOL bCoverageLostFlag
 * @retval なし
 */
//=============================================================================
void SetCoverageLostFlag(BOOL bCoverageLostFlag)
{
	s_bCoverageLostFlag = bCoverageLostFlag;
}


//=============================================================================
/**
 * ユーザリセットが発生したことを示すフラグを取得する。
 * @param なし
 * @retval TRUE  ユーザリセットが発生した
 * @retval FALSE ユーザリセットが発生していない
 */
//=============================================================================
BOOL GetCoverageResetFlag(void)
{
	return s_bCoverageResetFlag;
}


//=============================================================================
/**
 * ユーザリセットが発生したことを示すフラグに設定する。
 * @param BOOL bCoverageResetFlag
 * @retval なし
 */
//=============================================================================
void SetCoverageResetFlag(BOOL bCoverageResetFlag)
{
	s_bCoverageResetFlag = bCoverageResetFlag;
}


//=============================================================================
/**
 * カバレッジ計測関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Cv(void)
{
	int	i;

	// カバレッジ計測関連情報格納構造体変数の初期化
	s_CoverageMode_RX.dwSetTBW = INIT_CVM_TBW;	// DATA 4bit, SYNC 1bit
	s_CoverageMode_RX.dwSetTRM = INIT_CVM_TRM;	// CPU実行優先(リアルタイムトレース)
	s_CoverageMode_RX.dwSetTRC = INIT_CVM_TRC;	// クロック比 1：1
	for (i = 0; i < CV_RX_BLKNUM_MAX; i++) {
		s_Cvb_RX.eEnable[i] = RX_CVB_DIS;					// 動作禁止
		s_Cvb_RX.dwmadrBase[i] = CVB_BASE_ADDRESS_DEFAULT;	// "0"設定
	}
	s_bCoverageLostFlag = FALSE;				// LOSTが発生していない
	s_bCoverageResetFlag = FALSE;				// ユーザリセットが発生していない

	return;
}


