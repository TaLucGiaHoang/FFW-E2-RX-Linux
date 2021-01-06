////////////////////////////////////////////////////////////////////////////////
/**
 * @file hwrx_fpga_cv.cpp
 * @brief カバレッジ計測関連 E20 FPGAアクセス実装ファイル
 * @author RSO Y.Miyake
 * @author Copyright (C) 2013 Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/08/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo130308-001	2013/08/05 三宅
　カバレッジ開発対応
*/
#include "hw_fpga.h"
#include "hwrx_fpga_cv.h"
#include "fpga_cv.h"
#include "fpga_tra.h"
#include "hwrx_fpga_tra.h"
#include "ffwrx_cv.h"
#include "mcu_extflash.h"

//==============================================================================
/**
 * E20カバレッジ機能を有効/無効にする
 * @param  bCven TRUE:有効 FLASE:無効
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetFpgaCvEnable(BOOL bCven)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_CV_CVCTL, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if(bCven == TRUE){
		//有効
		wBuff = static_cast<WORD>(wBuff | CVCTL_CVEN);
	} else {
		//無効
		wBuff = static_cast<WORD>(wBuff & ~CVCTL_CVEN);
	}

	 // RevRxNo130308-001-027 Modify Line	// bit12を"0"マスク
	wBuff = static_cast<WORD>(wBuff & ~CVCTL_BR_D);	// RevRxNo130308-001-034 Append Line
	ferr = WriteFpgaReg(REG_CV_CVCTL, wBuff);		// RevRxNo130308-001-034 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}


//==============================================================================
/**
 * カバレッジ計測ベースアドレスをFPGAレジスタに設定する
 * @param dwSetBlk カバレッジ計測領域を設定するブロック0～3をビットフィールドで指定
 * @param pCvbData カバレッジ計測領域を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetCVBBlk(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvbData)
{

	FFWERR	ferr = FFWERR_OK;
	int		i;
	DWORD	dwBlockBit;			// ブロックのビット位置を表す変数
	WORD	wBaseAddr = 0;

	for (i = 0; i < CV_RX_BLKNUM_MAX; i++) {
		dwBlockBit = CVB_BLOCK0_BIT << i;
		if ((dwSetBlk & dwBlockBit) != CVB_NON_BLOCK_BIT) { 	// 設定変更対象ブロックの場合
			wBaseAddr = static_cast<WORD>(pCvbData->dwmadrBase[i] >> 16);
			ferr = WriteFpgaReg(REG_CV_CVB0H + (4 * i), wBaseAddr);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}

	return ferr;
}


//==============================================================================
/**
 * FPGAレジスタCVCTLの"カバレッジ機能有効ブロック選択"ビットに設定する
 * @param dwSetBlk カバレッジ計測領域を設定するブロック0～3をビットフィールドで指定
 * @param pCvbData カバレッジ計測領域を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetCVBEnable(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvbData)
{

	FFWERR	ferr = FFWERR_OK;
	int		i;
	DWORD	dwBlockBit;			// ブロックのビット位置を表す変数
	WORD	wEnableBit;			// ブロックの許可/禁止ビット位置を表す変数
	WORD	wCvctl = 0;

	ferr = ReadFpgaReg(REG_CV_CVCTL, &wCvctl);	// CVCTLレジスタリード
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	for (i = 0; i < CV_RX_BLKNUM_MAX; i++) {
		dwBlockBit = CVB_BLOCK0_BIT << i;
		if ((dwSetBlk & dwBlockBit) != CVB_NON_BLOCK_BIT) { 	// 設定変更対象ブロックの場合
			wEnableBit = CVCTL_CVBS0 << i;
			if (pCvbData->eEnable[i] == RX_CVB_ENA) {
				wCvctl = wCvctl | wEnableBit;		// 対応するビットに"1"設定
			} else {
				wCvctl = wCvctl & (~wEnableBit);	// 対応するビットに"0"設定	// RevRxNo130308-001-004 Modify Line
			}
		}
	}

	 // RevRxNo130308-001-027 Modify Line	// bit12を"0"マスク
	wCvctl = static_cast<WORD>(wCvctl & ~CVCTL_BR_D);	// RevRxNo130308-001-034 Append Line
	ferr = WriteFpgaReg(REG_CV_CVCTL, wCvctl);	// CVCTLレジスタへライト	// RevRxNo130308-001-034 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}


// RevRxNo130308-001-027 Delete		// GetFpgaRstf()


// RevRxNo130308-001-027 Append Start
//==============================================================================
/**
 * FPGA内CVCTL[13].TMP2参照
 * @param  pbRstf TPM2が"1"かどうかのポインタ
 *                (TRUE:TMP2が"1" FLASE:TMP2が"0")
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetFpgaCvctl_Tmp2(BOOL* pbRstf)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_CV_CVCTL, &wBuff);	// CVCTLレジスタリード
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if ((wBuff & CVCTL_TMP2) == CVCTL_TMP2) {
		// TMP2が"1"
		*pbRstf = TRUE;
	} else {
		// TMP2が"0"
		*pbRstf = FALSE;
	}

	return ferr;
}
// RevRxNo130308-001-027 Append End


// RevRxNo130308-001-013 Modify Start
//==============================================================================
/**
 * FPGA内TRCTL0[13].LOST_DまたはTRCTL0[12].TRERRまたはCVCTL[14].TMP参照
 * @param  pbLost_dOrTrerr LOST_DまたはTRERRまたはTMPが"1"かどうかのポインタ
 *                (TRUE:LOST_DまたはTRERRまたはTMPが"1" FLASE:LOST_DもTRERRもTMPも"0")
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetFpgaLost_dOrTrerrOrTmp(BOOL* pbLost_dOrTrerr)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;
	WORD	wCvctl = 0;

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);	// TRCTL0レジスタリード
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	ferr = ReadFpgaReg(REG_CV_CVCTL, &wCvctl);	// CVCTLレジスタリード
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (((wBuff & TRCTL0_LOST_D) == TRCTL0_LOST_D) || ((wBuff & TRCTL0_TRERR) == TRCTL0_TRERR) ||
		((wCvctl & CVCTL_TMP) == CVCTL_TMP)) {
		// 「LOST 発生」または「FPGA内部で分岐情報がロスト発生」
		// または「CVCTL.TMP == "1"(ブレーク時に「FPGA内部で分岐情報がロスト発生」があった)」場合
		*pbLost_dOrTrerr = TRUE;
	} else {
		// 「LOST 発生」も「FPGA内部で分岐情報がロスト発生」もしていない、かつ
		// 「CVCTL.TMP == "0"(ブレーク時に「FPGA内部で分岐情報がロスト発生」がなかった)」場合
		*pbLost_dOrTrerr = FALSE;
	}

	return ferr;
}
// RevRxNo130308-001-013 Modify End


//==============================================================================
/**
 * eBlkNoブロックに対応するCVCTL.CVBF参照
 * @param  pbCvbf eBlkNoブロックに対応するCVCTL.CVBFが"1"かどうかのポインタ
 *                (TRUE:eBlkNoブロックに対応するCVCTL.CVBFが"1" FLASE:左記以外)
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetFpgaCVBF(enum FFWRXENM_CV_BLKNO eBlkNo, BOOL* pbCvbf)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;
	WORD	wCVBF;

	ferr = ReadFpgaReg(REG_CV_CVCTL, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wCVBF = CVCTL_CVBF0;
	wCVBF = wCVBF << eBlkNo;	// eBlkNoブロックに対応するCVBFビット位置
	if ((wBuff & wCVBF) == wCVBF) {	// eBlkNoブロックの有効データ有りの場合
		*pbCvbf = TRUE;
	} else {
		*pbCvbf = FALSE;
	}

	return ferr;
}


//==============================================================================
/**
 * FPGA内のTRCTL0[8].RSTF、TRCTL0[13].LOST_D、TRCTL0[12].TRERRをクリアする
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrFpgaTrctl0_RstfLost_dTrerr(void)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wBuff = static_cast<WORD>(wBuff | TRCTL0_RSTF | TRCTL0_LOST_D | TRCTL0_TRERR);

	ferr = WriteFpgaReg(REG_RM_TRCTL0, wBuff);	// TRCTL0[8].RSTF、TRCTL0[13].LOST_D、TRCTL0[12].TRERRビットに"1"設定することでクリア。
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}


// RevRxNo130308-001-038 Append Start
//==============================================================================
/**
 * FPGA内の TRCTL0[8].RSTF ビットをクリアする
 * (LOST_D(bit13)、TRERR(bit12)、TMFUL(bit10)、TM2FUL(bit7)ビットをクリアしない)
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrFpgaTrctl0_Rstf(void)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wBuff = static_cast<WORD>((wBuff & ~MSK_TRCTL0_FLAGS) | TRCTL0_RSTF);

	ferr = WriteFpgaReg(REG_RM_TRCTL0, wBuff);	// TRCTL0[8].RSTFビットに"1"設定することでクリア。
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}
// RevRxNo130308-001-038 Append End


//==============================================================================
/**
 * FPGA内のTRCTL0.TR_RSTビットに"1"を設定
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetFpgaTrctl0_Tr_rst(void)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wBuff = static_cast<WORD>(wBuff | TRCTL0_TR_RST);

	ferr = WriteFpgaReg(REG_RM_TRCTL0, wBuff);	// TRCTL0.TR_RSTビットに"1"設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}


//==============================================================================
/**
 * FPGA内のCVCLRビットに"1"を設定
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetFpgaCvctl_Cvclr(void)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_CV_CVCTL, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wBuff = static_cast<WORD>(wBuff | CVCTL_CVCLR);

	 // RevRxNo130308-001-027 Modify Line	// コメント追加
	ferr = WriteFpgaReg(REG_CV_CVCTL, wBuff);	// CVCTL.CVCLRビットに"1"設定	// BR_Dが立っていたら"0"クリア
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}


//==============================================================================
/**
 * カバレッジデータ、カバレッジ用FPGAレジスタのクリア(CLRコマンドで呼び出される)
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrCvFpgaReg(void)
{

	FFWERR	ferr = FFWERR_OK;
	FFWRX_CVB_DATA	Cvb_RX;
	WORD	wTrctl0;
	WORD	wBuff;
	int		i;

	// (1) FPGAのカバレッジ計測関係回路を初期化
	// TRCTL0のビット0(TREN)に"0"を設定
	ferr = SetTrcFpgaEnable(FALSE);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130308-001-029 Append Start
	// Min.32μsの待ち時間(FPGA内ライトバッファのトレースメモリ反映待ち)
	COM_WaitMs(100);	// 100ms待ち(CLRコマンドで1回なので100msでも体感速度に問題ない)
						// 注：COM_WaitMs()の関数仕様上、10msなど小さい値の引数は正常動作しない可能性あり。
	// RevRxNo130308-001-029 Append End

	// FPGA内のTRCTL0.TR_RSTビットに"1"を設定
	ferr = SetFpgaTrctl0_Tr_rst();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// FPGA内のTRCTL0[8].RSTF、TRCTL0[13].LOST_D、TRCTL0[12].TRERRをクリア
	ferr = ClrFpgaTrctl0_RstfLost_dTrerr();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130308-001-013 Append Start
	// CVCTLのビット14(TMP)に"0"を設定
	ferr = SetFpgaCvctl_Tmp(FALSE);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130308-001-013 Append End

	// RevRxNo130308-001-027 Append Start
	// CVCTLのビット13(TMP2)に"0"を設定
	ferr = SetFpgaCvctl_Tmp2(FALSE);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130308-001-027 Append End

	// CVCTLのビット0(CVEN)に"0"を設定
	ferr = SetFpgaCvEnable(FALSE);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// FPGA内のCVCLRビットに"1"を設定
	ferr = SetFpgaCvctl_Cvclr();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 注：FFWでの"カバレッジのLOSTが発生したことを示すフラグ"などは、
	// 　　DO_CLR()内のInitFfwIfData_CLR()でクリアされる。

	// (2)カバレッジ用FPGAのトレース関連レジスタ初期化
	// FPGAのトレース制御０レジスタ
	ferr = WriteFpgaReg(REG_RM_TRCTL0, INIT_TRCTL0);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	
	// FPGAのトレース制御１レジスタ
	ferr = WriteFpgaReg(REG_RM_TRCTL1, INIT_TRCTL1);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// FPGAのトレースメモリバッファ制御
	ferr = WriteFpgaReg(REG_RM_TMBCTL, INIT_TMBCTL);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// FPGAのトレースメモリ書込み用アドレスレジスタ
	ferr = WriteFpgaReg(REG_RD_TMWA, INIT_TMWA);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// FPGAのトレースメモリ読込み用アドレスレジスタ
	ferr = WriteFpgaReg(REG_RD_TMRA, INIT_TMRA);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 注：FPGAのSDRAMメモリサイズ制御レジスタは、カバレッジ用FPGAには無い。

	// トレースFPGAのリセット先頭アドレス上位側レジスタ
	ferr = WriteFpgaReg(REG_RM_RSTVH, REG_CV_DATA_INIT);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのリセット先頭アドレス下位側レジスタ
	ferr = WriteFpgaReg(REG_RM_RSTVL, REG_CV_DATA_INIT);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// (3)カバレッジデータ(トレースメモリ)の初期化
	// FPGAのトレースメモリフィルデータ(検査用)
	ferr = WriteFpgaReg(REG_RM_TMFILD, INIT_TMFILD);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレース制御０レジスタ(TMFILに1をセット)
	ferr = WriteFpgaReg(REG_RM_TRCTL0, TRCTL0_TMFIL);	// TMFILのみセット
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wTrctl0 = TRCTL0_TMFIL;
	while ((wTrctl0 & TRCTL0_TMFIL) == TRCTL0_TMFIL) {
		// トレースFPGAのトレース制御０レジスタ(TMFILのゼロ書き込み待ち)	// トレースメモリの初期化
		ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		wTrctl0 = wBuff;
	}

	// (4)カバレッジ用FPGAのカバレッジ関連レジスタ初期化
	// カバレッジブロック情報の初期値を生成
	for (i = 0; i < CV_RX_BLKNUM_MAX; i++) {
		Cvb_RX.eEnable[i] = RX_CVB_DIS;		// 動作禁止
		Cvb_RX.dwmadrBase[i] = CVB_BASE_ADDRESS_DEFAULT;	// "0"設定
	}
	// カバレッジ機能用ブロック#0～4開始アドレス上位側レジスタ設定
	ferr = SetCVBBlk(CVB_MAX_BLOCK_BIT, &Cvb_RX);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// "カバレッジ機能有効ブロック選択(カバレッジ機能制御レジスタ)"設定
	ferr = SetCVBEnable(CVB_MAX_BLOCK_BIT, &Cvb_RX);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// カバレッジ機能用実行開始アドレス上位側レジスタ
	ferr = WriteFpgaReg(REG_CV_CVSAH, REG_CV_DATA_INIT);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// カバレッジ機能用実行開始アドレス下位側レジスタ
	ferr = WriteFpgaReg(REG_CV_CVSAL, REG_CV_DATA_INIT);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// カバレッジ機能用終了アドレス上位側レジスタ
	ferr = WriteFpgaReg(REG_CV_CVEAH, REG_CV_DATA_INIT);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// カバレッジ機能用終了アドレス下位側レジスタ
	ferr = WriteFpgaReg(REG_CV_CVEAL, REG_CV_DATA_INIT);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 注：TRCTL0のビット0(TREN)は"0"のままである。
	// 注：CVCTL のビット0(CVEN)は"0"のままである。

	return ferr;
}


//==============================================================================
/**
 * カバレッジデータのクリア(CVCRコマンドで呼び出される)
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrCvData(void)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wTrctl0;
	WORD	wBuff;

	// FPGAのトレースメモリフィルデータ(検査用)
	ferr = WriteFpgaReg(REG_RM_TMFILD, INIT_TMFILD);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	wBuff = wBuff | TRCTL0_TMFIL;
	// トレースFPGAのトレース制御０レジスタ
	ferr = WriteFpgaReg(REG_RM_TRCTL0, wBuff);	// TMFILをセット
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	do {
		// トレースFPGAのトレース制御０レジスタ(TMFILのゼロ書き込み待ち)	// トレースメモリの初期化
		ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		wTrctl0 = wBuff;
	} while ((wTrctl0 & TRCTL0_TMFIL) == TRCTL0_TMFIL);

	return ferr;
}


// RevRxNo130308-001-003 Append Start
//==============================================================================
/**
 * FPGA内のTRCTL0.RSTVCビットに"1"を設定
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetFpgaTrctl0_Rstvc(void)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wBuff = static_cast<WORD>((wBuff & ~MSK_TRCTL0_FLAGS) | TRCTL0_RSTVC);

	ferr = WriteFpgaReg(REG_RM_TRCTL0, wBuff);	// TRCTL0.RSTVCビットに"1"設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}
// RevRxNo130308-001-003 Append End


// RevRxNo130308-001-006 Append Start
//==============================================================================
/**
 * FPGA内のCVCTL.CVENビットを参照
 * @param  pbCven CVENが"1"かどうかのポインタ
 *                (TRUE:CVENが"1" FLASE:CVENが"0")
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetFpgaCven(BOOL* pbCven)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_CV_CVCTL, &wBuff);	// CVCTLレジスタリード
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if ((wBuff & CVCTL_CVEN) == CVCTL_CVEN) {
		// CVENが"1"の場合
		*pbCven = TRUE;
	} else {
		// CVENが"0"の場合
		*pbCven = FALSE;
	}

	return ferr;
}
// RevRxNo130308-001-006 Append End


// RevRxNo130308-001-013 Append Start
//==============================================================================
/**
 * FPGA内 CVCTL.TMPに値を設定する
 * @param  bTmp TRUE:"1"設定 FLASE:"0"設定
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetFpgaCvctl_Tmp(BOOL bTmp)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_CV_CVCTL, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if(bTmp == TRUE){
		//有効
		wBuff = static_cast<WORD>(wBuff | CVCTL_TMP);
	} else {
		//無効
		wBuff = static_cast<WORD>(wBuff & ~CVCTL_TMP);
	}

	 // RevRxNo130308-001-028 Modify Line	// bit12を"0"マスク
	wBuff = static_cast<WORD>(wBuff & ~CVCTL_BR_D);	// RevRxNo130308-001-034 Append Line
	ferr = WriteFpgaReg(REG_CV_CVCTL, wBuff);		// RevRxNo130308-001-034 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}
// RevRxNo130308-001-013 Append End


// RevRxNo130308-001-027 Append Start
//==============================================================================
/**
 * FPGA内 CVCTL.TMP2に値を設定する
 * @param  bTmp TRUE:"1"設定 FLASE:"0"設定
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetFpgaCvctl_Tmp2(BOOL bTmp)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_CV_CVCTL, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if(bTmp == TRUE){
		//有効
		wBuff = static_cast<WORD>(wBuff | CVCTL_TMP2);
	} else {
		//無効
		wBuff = static_cast<WORD>(wBuff & ~CVCTL_TMP2);
	}

	 // RevRxNo130308-001-028 Modify Line	// bit12を"0"マスク
	wBuff = static_cast<WORD>(wBuff & ~CVCTL_BR_D);	// RevRxNo130308-001-034 Append Line
	ferr = WriteFpgaReg(REG_CV_CVCTL, wBuff);		// RevRxNo130308-001-034 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}
// RevRxNo130308-001-027 Append End


// RevRxNo130308-001-016 Append Start
//==============================================================================
/**
 * FPGA内のTRCTL0.B_LOSTビットに"1"を設定
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetFpgaTrctl0_B_lost(void)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wBuff = static_cast<WORD>((wBuff & ~MSK_TRCTL0_FLAGS) | TRCTL0_B_LOST);

	ferr = WriteFpgaReg(REG_RM_TRCTL0, wBuff);	// TRCTL0.B_LOSTビットに"1"設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}
// RevRxNo130308-001-016 Append End
