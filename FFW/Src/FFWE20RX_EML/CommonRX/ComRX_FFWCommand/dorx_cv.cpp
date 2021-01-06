////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_cv.cpp
 * @brief カバレッジ計測関連コマンドのソースファイル
 * @author RSO Y.Miyake
 * @author Copyright (C) 2013 Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/06/10
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo130308-001	2013/06/10 三宅
　カバレッジ開発対応
*/
#include "dorx_cv.h"
#include "ffwrx_cv.h"
#include "prot_common.h"
#include "hwrx_fpga_tra.h"
#include "hwrx_fpga_cv.h"
#include "mcu_extflash.h"
#include "ocdspec.h"
#include "mcurx.h"
#include "dorx_tra.h"
#include "ffwrx_tra.h"
#include "mcurx_tra.h"
#include "ffwmcu_mcu.h"
#include "ffwrx_ev.h"
#include "dorx_ev.h"

//=============================================================================
/**
 * カバレッジ機能時のトレースモードの設定処理
 * @param pProg カバレッジ機能時のトレースモード格納構造体アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetRXCVM(const FFWRX_CVM_DATA* pCoverageMode)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR			ferrEnd;
	FFWRX_RM_DATA	RmData_RX;
	FFWRX_RM_DATA*	pRmData_RX;	// 内部管理変数のポインタを格納
	BOOL	bRstf;
	BOOL	bLost_dOrTrerr;
	BYTE 	byEvKind;
	FFWRX_TRG_DATA evTrg;
	FFWRX_TRG_DATA* pevTrg;

	ProtInit();

	// TRCTL0のビット0(TREN)に"0"を設定
	ferr = SetTrcFpgaEnable(FALSE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// Min.32μsの待ち時間(FPGA内ライトバッファのトレースメモリ反映待ち)
	// RevRxNo130308-001-010 Modify Start
	COM_WaitMs(100);	// 100ms待ち(CVD0コマンドで1回なので100msでも体感速度に問題ない)
						// 注：COM_WaitMs()の関数仕様上、10msなど小さい値の引数は正常動作しない可能性あり。
	// RevRxNo130308-001-010 Modify End

	// CVCTLのビット0(CVEN)に"0"を設定
	ferr = SetFpgaCvEnable(FALSE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// FPGA内TRCTL0[8].RSTFが"1"の場合(CVCTL.TMP2を見る)	// RevRxNo130308-001-027 Modify Line
	ferr = GetFpgaCvctl_Tmp2(&bRstf);						// RevRxNo130308-001-027 Modify Line
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	if (bRstf == TRUE) {
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
	if (bLost_dOrTrerr == TRUE) {
		SetCoverageLostFlag(TRUE);	// "カバレッジのLOSTが発生したことを示すフラグ"に"TRUE"を設定
	}

	// マイコン内TBSRレジスタのビット6が"1(次の記録データにロストが記録される)"の場合
	// RevRxNo130308-001-021 Modify Line
	// LOST発生フラグは"TRUE"にしない。理由は、CVCL後CVMコマンドでLOST発生フラグを"TRUE"にしてしまわないように。

	// RevRxNo130308-001-027 Append Start
	// FPGA内のTRCTL0.B_LOSTビットに"1"を設定
		// 理由は、実行開始時にはF/WがFPGAにPC値をロードしているので、
		// 常にプログラム実行開始最初のLOSTは破棄したいため。
		// (なおTRCTL0.B_LOSTが立っているときにLOSTを破棄する条件は、TRCTL0.TRENが"0"後の1回目である。)
		// なお、BFWで、ブレーク直後の、TRCTL0.B_LOSTビットに"0"を設定する処理は削除。
	ferr = SetFpgaTrctl0_B_lost();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo130308-001-027 Append End

	// RmData_RX に、カバレッジ用固定設定値および引数pCoverageModeの指す内容を設定
	// RevRxNo130308-001-022 Modify Line
	if ((pCoverageMode->dwSetTRM & CVM_BIT_0_0_1MASK) == CVM_TRM_REALTIME) {	// CPU実行優先(リアルタイムトレース) の場合
		RmData_RX.dwSetMode = CVM_SETMODE_REALTIME;
	} else {																	// トレース出力優先(フルトレース) の場合
		RmData_RX.dwSetMode = CVM_SETMODE_TRCFULL;
	}

	// RevRxNo130308-001-022 Modify Line
	if ((pCoverageMode->dwSetTBW & CVM_BIT_3_0_1MASK) == CVM_TBW_DATA4_SYNC1) {			// DATA 4bit, SYNC 1bit の場合	// bit[31:4]は0マスクする。
		RmData_RX.dwSetInitial = CVM_SETINITIAL_DATA4_SYNC1;
	// RevRxNo130308-001-012 Modify Line	// RevRxNo130308-001-022 Modify Line
	} else if ((pCoverageMode->dwSetTBW & CVM_BIT_3_0_1MASK) == CVM_TBW_DATA8_SYNC2) {	// DATA 8bit, SYNC 2bit の場合	// bit[31:4]は0マスクする。
		RmData_RX.dwSetInitial = CVM_SETINITIAL_DATA8_SYNC2;
	} else {																	// DATA 8bit, SYNC 1bit の場合
		RmData_RX.dwSetInitial = CVM_SETINITIAL_DATA8_SYNC1;
	}
	RmData_RX.dwSetInitial = (RmData_RX.dwSetInitial & CVM_BIT_31_28_0MASK) | (pCoverageMode->dwSetTBW & ~CVM_BIT_31_28_0MASK);	// dwSetTBWのbit[31:28](DLYTGT)を反映。

	// RevRxNo130308-001-022 Modify Line
	if ((pCoverageMode->dwSetTRC & CVM_BIT_2_0_1MASK) == CVM_TRC_RATIO_1_1) {			// クロック比 1:1 の場合
		RmData_RX.dwSetInfo = CVM_SETINFO_RATIO_1_1;
	// RevRxNo130308-001-022 Modify Line
	} else if ((pCoverageMode->dwSetTRC & CVM_BIT_2_0_1MASK) == CVM_TRC_RATIO_2_1) {	// クロック比 2:1 の場合
		RmData_RX.dwSetInfo = CVM_SETINFO_RATIO_2_1;
	} else {																			// クロック比 4:1 の場合
		RmData_RX.dwSetInfo = CVM_SETINFO_RATIO_4_1;
	}

	RmData_RX.dwTrcSrcSel = CVM_TRCSRCSEL_CPU_SEL;	// RevRxNo130308-001-026 Modify Line
	RmData_RX.dwRomStartAddr = CVM_DEFAULT;
	RmData_RX.dwRomEndAddr = CVM_DEFAULT;
	RmData_RX.dwWinTrStartAddr = CVM_DEFAULT;
	RmData_RX.dwWinTrEndAddr = CVM_DEFAULT_FFFFFFFF;
	RmData_RX.eEmlTrcMemSize = EML_TRCMEM_64M;						// 64M×16bit (128Mbyte)

	// RmData_RX を内部管理変数へ渡す
	pRmData_RX = GetRmDataInfo();	// 内部管理変数のポインタ
	memcpy(pRmData_RX,&RmData_RX,sizeof(FFWRX_RM_DATA));

	// RevRxNo130308-001-008 Append Start
	// イベントトリガの引数生成
	byEvKind = EVTRG_EVKIND_TRC;		// EVTRGで設定変更機能は"トレース"
	evTrg.dwTrcTrg = EVETTRG_TRC_ALL;	// トレーストリガ全部"不使用"
	// RevRxNo130308-001-008 Append End

	ferrEnd = ProtEnd();			// 下の DO_SetRXRM() で、ProtInit()、ProtEnd()があるので、ここでProtEnd()しておく。
	if (ferrEnd != FFWERR_OK) {
		return ferrEnd;
	}

	ferr = DO_SetRXRM(&RmData_RX);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130308-001-008 Append Start
	ferr = DO_SetRXEVTRG(byEvKind, evTrg);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130308-001-008 Append End

	ProtInit();

	// RevRxNo130308-001-008 Append Start
	// 変更対象イベントであるトレースの、内部変数更新
	pevTrg = GetEvTrgInfo();
	pevTrg->dwTrcTrg = evTrg.dwTrcTrg;
	// RevRxNo130308-001-008 Append End

	// CVCTLのビット0(CVEN)に"1"を設定
	ferr = SetFpgaCvEnable(TRUE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo130308-001-003 Append Start
	// TRCTL0のビット9(RSTVC)に"1"を設定
	ferr = SetFpgaTrctl0_Rstvc();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo130308-001-003 Append End

	ferrEnd = ProtEnd();

	return ferrEnd;
}


//=============================================================================
/**
 * カバレッジ計測ベースアドレス設定処理
 * @param dwSetBlk カバレッジ計測領域を設定するブロック0～3をビットフィールドで指定
 * @param pCvbData カバレッジ計測領域を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetRXCVB(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvbData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR			ferrEnd;
	BOOL	bRstf;
	BOOL	bLost_dOrTrerr;
	// RevRxNo130308-001-006 Append Line
	BOOL	bCven;

	ProtInit();

	// TRCTL0のビット0(TREN)に"0"を設定
	ferr = SetTrcFpgaEnable(FALSE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// Min.32μsの待ち時間(FPGA内ライトバッファのトレースメモリ反映待ち)
	// RevRxNo130308-001-010 Modify Start
	COM_WaitMs(100);	// 100ms待ち(CVD0コマンドで1回なので100msでも体感速度に問題ない)
						// 注：COM_WaitMs()の関数仕様上、10msなど小さい値の引数は正常動作しない可能性あり。
	// RevRxNo130308-001-010 Modify End

	// RevRxNo130308-001-006 Append Start
	// CVCTLのビット0(CVEN)を参照
	ferr = GetFpgaCven(&bCven);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo130308-001-006 Append End

	// CVCTLのビット0(CVEN)に"0"を設定
	ferr = SetFpgaCvEnable(FALSE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// FPGA内TRCTL0[8].RSTFが"1"の場合(CVCTL.TMP2を見る)	// RevRxNo130308-001-027 Modify Line
	ferr = GetFpgaCvctl_Tmp2(&bRstf);						// RevRxNo130308-001-027 Modify Line
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	if (bRstf == TRUE) {
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
	if (bLost_dOrTrerr == TRUE) {
		SetCoverageLostFlag(TRUE);	// "カバレッジのLOSTが発生したことを示すフラグ"に"TRUE"を設定
	}

	// マイコン内TBSRレジスタのビット6が"1(次の記録データにロストが記録される)"の場合
	// RevRxNo130308-001-021 Modify Line
	// LOST発生フラグは"TRUE"にしない。理由は、CVCL後CVBコマンドでLOST発生フラグを"TRUE"にしてしまわないように。

	// ブロック開始アドレス設定
	ferr = SetCVBBlk(dwSetBlk, pCvbData);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// "カバレッジ機能有効ブロック選択"設定
	ferr = SetCVBEnable(dwSetBlk, pCvbData);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// TRCTL0のビット0(TREN)に"1"を設定
	ferr = SetTrcFpgaEnable(TRUE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// CVCTLのビット0(CVEN)に"1"を設定
	// RevRxNo130308-001-006 Append Line
	if (bCven == TRUE) {	// 関数先頭でリードしたCVENが"1"の場合のみ
		ferr = SetFpgaCvEnable(TRUE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	// RevRxNo130308-001-006 Append Line
	}

	ferrEnd = ProtEnd();

	return ferrEnd;
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
FFWERR DO_GetRXCVD0(enum FFWRXENM_CV_BLKNO eBlkNo, DWORD dwStart, DWORD dwEnd, DWORD dwCvData[])
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	BOOL	bRstf;
	BOOL	bLost_dOrTrerr;
	BOOL	bCvbf;
	DWORD	dwStartBlock, dwEndBlock;
	DWORD	dwRdSize;
	FFWERR	ferr1;
	DWORD	i;
	WORD*	wRd;
	// RevRxNo130308-001-006 Append Line
	BOOL	bCven;

	ProtInit();

	// TRCTL0のビット0(TREN)に"0"を設定
	ferr = SetTrcFpgaEnable(FALSE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// Min.32μsの待ち時間(FPGA内ライトバッファのトレースメモリ反映待ち)
	// RevRxNo130308-001-010 Modify Start
	COM_WaitMs(100);	// 100ms待ち(CVD0コマンドで1回なので100msでも体感速度に問題ない)
						// 注：COM_WaitMs()の関数仕様上、10msなど小さい値の引数は正常動作しない可能性あり。
	// RevRxNo130308-001-010 Modify End

	// RevRxNo130308-001-006 Append Start
	// CVCTLのビット0(CVEN)を参照
	ferr = GetFpgaCven(&bCven);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo130308-001-006 Append End

	// CVCTLのビット0(CVEN)に"0"を設定
	ferr = SetFpgaCvEnable(FALSE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// FPGA内TRCTL0[8].RSTFが"1"の場合(CVCTL.TMP2を見る)	// RevRxNo130308-001-027 Modify Line
	ferr = GetFpgaCvctl_Tmp2(&bRstf);						// RevRxNo130308-001-027 Modify Line
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	if (bRstf == TRUE) {
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
	if (bLost_dOrTrerr == TRUE) {
		SetCoverageLostFlag(TRUE);	// "カバレッジのLOSTが発生したことを示すフラグ"に"TRUE"を設定
	}

	// マイコン内TBSRレジスタのビット6が"1(次の記録データにロストが記録される)"の場合
	// RevRxNo130308-001-027 Modify Line
	// LOST発生フラグは"TRUE"にしない。ブレーク時にBFWがLOFからロストありか判定してCVCTL.TMPを立てているので。

	// eBlkNoに対応する、CVCTL.CVBFのビットが無効である場合
	ferr = GetFpgaCVBF(eBlkNo, &bCvbf);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	if (bCvbf == FALSE) {	// eBlkNoに対応するブロックのカバレッジ有効データが無い場合
		// CVCTLのビット0(CVEN)に"1"を設定
		// RevRxNo130308-001-006 Append Line
		if (bCven == TRUE) {	// 関数先頭でリードしたCVENが"1"の場合のみ
			ferr = SetFpgaCvEnable(TRUE);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		// RevRxNo130308-001-006 Append Line
		}
		// TRCTL0のビット0(TREN)に"1"を設定
		ferr = SetTrcFpgaEnable(TRUE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		ferrEnd = ProtEnd();
		return FFWERR_CVDATA_NON;	// "カバレッジデータがない"エラー
	}

	// トレースメモリからデータ取得のために、スタートブロック生成
	dwStartBlock = ((eBlkNo * CVD0_TRACE_MEM_BLOCK_SIZE) + (dwStart * 2)) >> CVD0_1K_SIFT;
	// トレースメモリからデータ取得のために、エンドブロック生成
	dwEndBlock   = ((eBlkNo * CVD0_TRACE_MEM_BLOCK_SIZE) + (dwEnd   * 2)) >> CVD0_1K_SIFT;
	// トレース内容を格納するワード配列の容量生成
	dwRdSize = (dwEndBlock - dwStartBlock + 1) * RD_TRC_BLOCK;
	// 配列を動的に確保
	wRd = new WORD[dwRdSize];
	// memset(wRd, 0, ((dwRdSize)*2));	// カバレッジ制御では、"0"で埋めることは不要。

	// トレースメモリデータを取得
	ferr = GetTrcFpgaMemBlock(dwStartBlock, dwEndBlock, wRd);
	if (ferr != FFWERR_OK) {	// GetTrcFpgaMemBlock()が正常終了でない場合
		delete [] wRd;
		// CVCTLのビット0(CVEN)に"1"を設定
		// RevRxNo130308-001-006 Append Line
		if (bCven == TRUE) {	// 関数先頭でリードしたCVENが"1"の場合のみ
			ferr1 = SetFpgaCvEnable(TRUE);
			//if (ferr1 != FFWERR_OK) {	// GetTrcFpgaMemBlock()のエラーの方が優先のため、コメントにする。
			//	ferrEnd = ProtEnd();
			//	return ferr1;
			//}
		// RevRxNo130308-001-006 Append Line
		}

		// TRCTL0のビット0(TREN)に"1"を設定
		ferr1 = SetTrcFpgaEnable(TRUE);
		//if (ferr1 != FFWERR_OK) {	// GetTrcFpgaMemBlock()のエラーの方が優先のため、コメントにする。
		//	ferrEnd = ProtEnd();
		//	return ferr1;
		//}
		ferrEnd = ProtEnd();
		return ferr;
	}

	// dwCvData[]への設定
	for (i = 0; i < (dwEnd - dwStart + 1); i++) {
		dwCvData[i] =  (static_cast<DWORD>(wRd[((dwStart & CVD0_MCU_BLOCK_REMAINDER) + i)*4 + 0])) | 
					  ((static_cast<DWORD>(wRd[((dwStart & CVD0_MCU_BLOCK_REMAINDER) + i)*4 + 2])) << 16);
	}

	// wRd を解放する。
	delete [] wRd;

	// TRCTL0のビット0(TREN)に"1"を設定
	ferr = SetTrcFpgaEnable(TRUE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// CVCTLのビット0(CVEN)に"1"を設定
	// RevRxNo130308-001-006 Append Line
	if (bCven == TRUE) {	// 関数先頭でリードしたCVENが"1"の場合のみ
		ferr = SetFpgaCvEnable(TRUE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	// RevRxNo130308-001-006 Append Line
	}

	if ((GetCoverageLostFlag() == TRUE) && (GetCoverageResetFlag() == TRUE)) {
		// "カバレッジのLOSTが発生したことを示すフラグ"および"ユーザリセットが発生したことを示すフラグ"が立っている場合
		
		return FFWERR_CV_LOST_MEASURE;	// "カバレッジ情報にLOSTが発生した、およびリセット発生によるカバレッジ計測異常発生の可能性あり(Warning)"を返して関数から抜ける
	} else if (GetCoverageLostFlag() == TRUE) {
		// "カバレッジのLOSTが発生したことを示すフラグ"のみ立っている場合
		
		return FFWERR_CV_LOST;			// "カバレッジ情報にLOSTが発生した(Warning)"を返して関数から抜ける
	} else if (GetCoverageResetFlag() == TRUE) {
		// "ユーザリセットが発生したことを示すフラグ"のみ立っている場合
		
		return FFWERR_CV_MEASURE;		// "リセット発生によるカバレッジ計測異常発生の可能性あり(Warning)"を返して関数から抜ける
	}

	ferrEnd = ProtEnd();

	return ferrEnd;
}


//=============================================================================
/**
 * カバレッジデータのクリア
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_ClrRXCVD(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	FFWRX_CVB_DATA*	pCvb_RX;
	// RevRxNo130308-001-006 Append Line
	BOOL	bCven;

	ProtInit();

	pCvb_RX = GetCvb_RXInfo();	// カバレッジ計測領域格納データ構造体のstatic変数のアドレスを取得

	// RevRxNo130308-001-016 Append Start
	// FPGA内のTRCTL0.B_LOSTビットに"1"を設定	// RevRxNo130308-001-035 Delete
	// RevRxNo130308-001-016 Append End

	// TRCTL0のビット0(TREN)に"0"を設定
	ferr = SetTrcFpgaEnable(FALSE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxNo130308-001-029 Append Start
	// Min.32μsの待ち時間(FPGA内ライトバッファのトレースメモリ反映待ち)
	COM_WaitMs(100);	// 100ms待ち(CVCLコマンドで1回なので100msでも体感速度に問題ない)
						// 注：COM_WaitMs()の関数仕様上、10msなど小さい値の引数は正常動作しない可能性あり。
	// RevRxNo130308-001-029 Append End

	// FPGA内のTRCTL0.TR_RSTビットに"1"を設定
	ferr = SetFpgaTrctl0_Tr_rst();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// FPGA内のTRCTL0[8].RSTF、TRCTL0[13].LOST_D、TRCTL0[12].TRERRをクリア
	ferr = ClrFpgaTrctl0_RstfLost_dTrerr();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
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

	// RevRxNo130308-001-006 Append Start
	// CVCTLのビット0(CVEN)を参照
	ferr = GetFpgaCven(&bCven);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo130308-001-006 Append End

	// CVCTLのビット0(CVEN)に"0"を設定
	ferr = SetFpgaCvEnable(FALSE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// FPGA内のCVCLRビットに"1"を設定
	ferr = SetFpgaCvctl_Cvclr();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// "カバレッジのLOSTが発生したことを示すフラグ"に"FALSE"設定
	SetCoverageLostFlag(FALSE);

	// "ユーザリセットが発生したことを示すフラグ"に"FALSE"設定
	SetCoverageResetFlag(FALSE);

	// カバレッジデータ(トレースメモリ)の初期化
	ferr = ClrCvData();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// ブロック開始アドレス設定	// RevRxNo130308-001-035 Delete

	// "カバレッジ機能有効ブロック選択"設定	// RevRxNo130308-001-035 Delete

	// TRCTL0のビット0(TREN)に"1"を設定
	ferr = SetTrcFpgaEnable(TRUE);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// CVCTLのビット0(CVEN)に"1"を設定
	// RevRxNo130308-001-006 Append Line
	if (bCven == TRUE) {	// 関数先頭でリードしたCVENが"1"の場合のみ
		ferr = SetFpgaCvEnable(TRUE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	// RevRxNo130308-001-006 Append Line
	}

	ferrEnd = ProtEnd();

	return ferrEnd;
}


//=============================================================================
/**
 * カバレッジ計測関連レジスタを初期化する
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ClrCvReg(void)
{
	FFWERR	ferr = FFWERR_OK;

	//トレースOCDレジスタのクリア（ClrTrcOCDReg()を流用。）
	ferr = ClrTrcOCDReg();
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// カバレッジデータ、カバレッジ用FPGAレジスタのクリア
	ferr = ClrCvFpgaReg();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}


// RevRxNo130308-001-007 Append Start
//=============================================================================
/**
 * カバレッジ機能選択時の実行前トレース機能設定(TBSR.TRFS設定)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetGoTrace_CV(void)
{
	FFWERR		ferr = FFWERR_OK;
	FFWRX_RM_DATA*				pRm;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	DWORD						dwBuff, dwData, dwJoinTrcBuffSet;
	DWORD						dwSetInitial;
	BYTE*						pbyTbsr;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;
	DWORD						dwTrcfrq = 0;
	FFWMCU_FWCTRL_DATA*			pFwCtrl;

	pRm = GetRmDataInfo();
	pFwCtrl = GetFwCtrlData();

	pMcuInfoData = GetMcuInfoDataRX();
	if(pFwCtrl->eTrcClkCtrl == RX_TRCLK_CTRL_RX630){
	// RX630系のクロック系の場合
		if(pMcuInfoData->eExTAL == TRC_EXTAL_DIS){
			// ExTALを使用しない場合は　ExTAL*1を選択
			dwBuff = (pRm->dwSetInitial & ~RM_SETINITIAL_TRFS);
			dwSetInitial = dwBuff;
		} else {
			// ExTALを使用する場合は　ExTAL*1/2を選択
			dwBuff = (pRm->dwSetInitial & ~RM_SETINITIAL_TRFS);
			dwSetInitial = dwBuff | RM_SETINITIAL_TRFS_1_2;
		}
	} else if(pFwCtrl->eTrcClkCtrl == RX_TRCLK_CTRL_RX610){
	// RX610系のクロック系の場合
		dwBuff = (pRm->dwSetInitial & RM_SETINITIAL_TRFS) >> 4;
		ferr = CheckTrcClkFrq(dwBuff, &dwTrcfrq);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		dwBuff = (pRm->dwSetInitial & ~RM_SETINITIAL_TRFS);
		dwSetInitial = dwBuff | (dwTrcfrq << 4);
	} else {
	// RX210系のクロック系の場合
		dwBuff = (pRm->dwSetInitial & ~RM_SETINITIAL_TRFS);
		dwSetInitial = dwBuff;
	}
	//
	SetRmInitialData(dwSetInitial);

	// トレースバッファ設定レジスタ
	//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
	//  ┌───────────────────────┬─┬─┬─┬─┐
	//  │                                              │              │
	//  └───────────────────────┴─┴─┴─┴─┘
	//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
	//  ┌─────┬─┬─────┬─┬───┬─┬─┬─┬─┬─┬─┐
	//  │          │  │          │  │      │  │  │      │  │  │
	//  └─────┴─┴─────┴─┴───┴─┴─┴─┴─┴─┴─┘
	//  b19-16:TRFS dwSetInitialのb7-4
	//  b12:RWE     dwSetInitialのb1
	//  b8:TMWM     dwSetModeのb1
	//  b5:OFF      dwSetInfoのb0
	//  b3-2:TDOS   dwSetModeのb3-2
	//  b0:TRPE     dwSetInitialのb0
	dwData = ((pRm->dwSetInitial << 12) & OCD_REG_RM_TBSR_TRFS) |		// b19-16
		     ((pRm->dwSetInitial << 11) & OCD_REG_RM_TBSR_RWE)  |		// b12
		     ((pRm->dwSetMode    << 7)  & OCD_REG_RM_TBSR_TMWM) |		// b8
		     ((pRm->dwSetInfo    << 5)  & OCD_REG_RM_TBSR_OFF)  |		// b5
		     ((pRm->dwSetMode    >> 0)  & OCD_REG_RM_TBSR_TDOS) |		// b3-2
		     ((pRm->dwSetInitial >> 0)  & OCD_REG_RM_TBSR_TRPE);		// b0
	dwJoinTrcBuffSet = dwData;

	// トレースバッファ設定レジスタ
	pbyTbsr = reinterpret_cast<BYTE*>(&dwJoinTrcBuffSet);
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, pbyTbsr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}
// RevRxNo130308-001-007 Append End


//=============================================================================
/**
 * カバレッジ計測関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Cv(void)
{
	return;
}
