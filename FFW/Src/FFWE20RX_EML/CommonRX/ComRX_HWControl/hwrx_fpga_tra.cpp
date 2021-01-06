///////////////////////////////////////////////////////////////////////////////
/**
 * @file hwrx_fpga_tra.cpp
 * @brief トレース関連 E20 FPGAアクセス実装ファイル
 * @author RSD H.Hashiguchi, S.Ueda, Y.Miyake
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/18
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120626-001 2012/07/12 橋口
　RX63TH トレース駆動能力対応 
   ・PROT_SetTrcBrk()→PROT_SetTRCMODE()に変更
・RevRxNo121227-001	2012/12/27 上田
　　ClrTrcFpgaStatus()追加。
・RevRxNo130308-001 2013/05/21 三宅
　カバレッジ開発対応
・RevRxNo130730-006	2013/11/25 上田
　E20トレースクロック遅延設定タイミング変更
・RevRxNo130408-001	2014/04/16 上田
	外部トレースデータ8ビット幅対応
・RevRxE2LNo141104-001 2014/11/18 上田
	E2 Lite対応
*/

#include "hwrx_fpga_tra.h"
#include "hw_fpga.h"
#include "fpga_tra.h"
#include "do_common.h"
#include "prot_cpu.h"
#include "dorx_tra.h"
#include "protrx_tra.h"
#include "ffwrx_tra.h"
// RevRxNo130308-001 Append Start
#include "ffwmcu_mcu.h"
#include "fpga_cv.h"
#include "dorx_cv.h"
// RevRxNo130308-001 Append End


// ファイル内static変数の宣言
// RevRxNo130730-006 Append Start
static BOOL s_bSetTrcE20FpgaAllRegFlg;	// E20 FPGAのトレース関連レジスタ設定処理(SetTrcE20FpgaData())での全レジスタ設定指示フラグ
static BOOL s_bNeedSetTrcFpgaHotplug;	// ホットプラグ動作中に設定されたトレース情報のFPGAレジスタ設定要否を示すフラグ
// RevRxNo130730-006 Append End


///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//==============================================================================
/**
 * トレース関連レジスタを初期化する。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcFpgaReg(void)
{
	FFWERR	ferr;
	WORD	wTrctl0 = 0;
	WORD	wBuff;
	WORD	wWriteData;
	
	// トレースバッファクリア
	ferr = ClrTrcFpgaBuf();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレース制御０レジスタ
	ferr = WriteFpgaReg(REG_RM_TRCTL0, INIT_TRCTL0);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	
	// トレースFPGAのトレース制御１レジスタ
	wBuff = INIT_TRCTL1;
	wWriteData = wBuff;
	ferr = WriteFpgaReg(REG_RM_TRCTL1, wWriteData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレースメモリバッファ制御
	ferr = WriteFpgaReg(REG_RM_TMBCTL, INIT_TMBCTL);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレースメモリ書込み用アドレスレジスタ
	ferr = WriteFpgaReg(REG_RD_TMWA, INIT_TMWA);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレースメモリ読込み用アドレスレジスタ
	ferr = WriteFpgaReg(REG_RD_TMRA, INIT_TMRA);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレースメモリ制御
	// Rev110303-001 Modify Start
	wBuff = EML_TRCMEM_16M;
	wWriteData = wBuff;
	ferr = WriteFpgaReg(REG_RM_TMSIZE, wWriteData);	// RevRxNo130408-001 Modify Line
	// Rev110303-001 Modify End
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレースメモリフィルデータ(検査用)
	ferr = WriteFpgaReg(REG_RM_TMFILD, INIT_TMFILD);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレース制御０レジスタ(TMFILに1をセット)
	wTrctl0 = TRCTL0_TMFIL;		// TMFILのみセット
	wWriteData = wTrctl0;
	ferr = WriteFpgaReg(REG_RM_TRCTL0, wWriteData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	while ((wTrctl0 & TRCTL0_TMFIL) == TRCTL0_TMFIL) {
		// トレースFPGAのトレース制御０レジスタ(TMFILのゼロ書き込み待ち)
		ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		wTrctl0 = wBuff;
	}

	// トレースFPGAのトレースコマンド取得(コマンド数クリア)
	wBuff = TC_CTL_TC_CLR;
	wWriteData = wBuff;
	ferr = WriteFpgaReg(REG_RM_TC_CTL, wWriteData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}
//==============================================================================
/**
 * トレースバッファのクリア。
 * @param  void
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcFpgaBuf(void) {

	FFWERR						ferr;
	BYTE*	pbyReadFpga;						// 参照データを格納する領域のアドレス(FPGA用)

	// E20トレースバッファクリア
	pbyReadFpga = new BYTE[RD_TMBUFFSIZE];
	memset(pbyReadFpga, 0, RD_TMBUFFSIZE);

	// RevRxE2LNo141104-001 Modify Line
	// エミュレータ種別、BFW動作モードに対応したCPU_FILLコマンドを発行(エミュレータ種別はFFW内部変数を参照)
	ferr = ProtCpuFill(EWORD_ACCESS, REG_RD_TMBUFF0, RD_TMBUFFSIZE,pbyReadFpga);

	delete [] pbyReadFpga;
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}


//==============================================================================
/**
 * トレースメモリのクリア。
 * @param  wClrMemSize クリアするトレースメモリのサイズ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcFpgaMem(WORD wClrMemSize) {

	FFWERR						ferr;
	WORD						wEscTrctl0 = 0;		// トレースFPGAのトレース制御０レジスタ(TRCTL0)退避用
	WORD						wTrctl0 = 0;		// トレースFPGAのトレース制御０レジスタ(TRCTL0)用
	WORD						wTmFil = 0;			// トレースFPGAのトレースメモリフィルデータ用
	WORD						wTcCtl = 0;			// トレースFPGAのトレースメモリフィルデータ用
	WORD						wTmwa = 0;			// トレースFPGAのトレースメモリ書き込み用アドレス
	WORD						wTmsize = 0;		//トレースFPGAメモリ容量制御レジスタ(TMSIZE)退避用
	WORD						wBuff;

	// トレースFPGAメモリフィルするときは、全エリアFILLする必要がある
	ferr = ReadFpgaReg(REG_RM_TMSIZE, &wTmsize);			//バックアップ取得	// RevRxNo130408-001 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wBuff = wClrMemSize;
	ferr = WriteFpgaReg(REG_RM_TMSIZE, wBuff);				//トレースメモリ容量をMAX(128MByte)にしておく	// RevRxNo130408-001 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレースメモリフィルデータ(0埋め)
	wTmFil = 0;
	ferr = WriteFpgaReg(REG_RM_TMFILD, wTmFil);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレース制御０レジスタ(TMFILに1をセット(TMFIL以外はクリア))
	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wTrctl0);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	
	wEscTrctl0 = wTrctl0;	// 読み込んだデータを退避
	wTrctl0 &= ~(TRCTL0_TREN);		// TRENのみクリア

	ferr = WriteFpgaReg(REG_RM_TRCTL0, wTrctl0);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wTrctl0 |= TRCTL0_TMFIL;		// TMFILのみセット
	ferr = WriteFpgaReg(REG_RM_TRCTL0, wTrctl0);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	while ((wTrctl0 & TRCTL0_TMFIL) == TRCTL0_TMFIL) {
		// トレースFPGAのトレース制御０レジスタ(TMFILのゼロ書き込み待ち)
		ferr = ReadFpgaReg(REG_RM_TRCTL0, &wTrctl0);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// トレースFPGAのトレースメモリ書き込み用アドレスクリア
	ferr = WriteFpgaReg(REG_RD_TMWA, wTmwa);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレースコマンド取得(上位側コマンド数、コマンドクリア))
	wTcCtl = TC_CTL_TC_CLR;
	ferr = WriteFpgaReg(REG_RM_TC_CTL, wTcCtl);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//メモリ0FILLが終わったので、トレースFPGAメモリの設定書き戻し
	ferr = WriteFpgaReg(REG_RM_TMSIZE, wTmsize);	// RevRxNo130408-001 Modify Line
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレース制御０レジスタ(退避データを復帰)
	ferr = WriteFpgaReg(REG_RM_TRCTL0, wTrctl0);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * E20トレースFPGAに設定
 * @param  なし
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWERR SetTrcE20FpgaData(void)
{

	FFWERR						ferr;
	WORD						wData;
	WORD						wJoinTrctl0 = 0;		// トレースFPGAのトレース制御０レジスタ(TRCTL0)のデータ連結用
	WORD						wJoinTrctl1 = 0;		// トレースFPGAのトレース制御０レジスタ(TRCTL0)のデータ連結用
	WORD						wTrcMemSize = 0;		// トレースFPGAのトレースメモリ制御レジスタ(TMSIZE)のデータ格納用
	//RevNo110309-004 Append Line
	WORD						wIn_Dly = 0;		// トレースFPGAのトレースメモリ制御レジスタ(TMCTL)のデータ格納用
	DWORD						madrStartAddr[TRC_SET_E20REGNUM+1];
	DWORD						*pdwCount;
	BYTE						*pbyWriteData;
	DWORD						dwAddrCnt,dwDataCnt;
	DWORD						dwAreaNum;
	enum FFWENM_EACCESS_SIZE	eAccessSize;
	FFWRX_RM_DATA*				RmData_RX;
	// RevRxNo130308-001 Append Start
	FFWMCU_DBG_DATA_RX*	pDbgData;
	WORD						wBuff = 0;

 	pDbgData = GetDbgDataRX();
	// RevRxNo130308-001 Append End

	RmData_RX = GetRmDataInfo();

	dwAddrCnt = 0;
	dwDataCnt = 0;
	pbyWriteData = new BYTE [(TRC_SET_E20REGNUM+1)*4];
	memset(pbyWriteData, 0, (TRC_SET_E20REGNUM+1)*4);
	pdwCount = new DWORD [(TRC_SET_E20REGNUM+1)*4];
	
	//---------------
	// データ連結処理
	//---------------

	// トレースFPGAのトレース制御０レジスタ
	//    8   7   6   5   4   3   2   1   0
	//  ┌─────────┬─┬─┬─┬─┐
	//  │                  │          │  │
	//  └─────────┴─┴─┴─┴─┘
	//  b3-1:FUNCS dwSetModeのb6-4
	//  b0:TREN '1'固定
	//  ※実際は上記並びではなく、以下のような並びになっているようだ。
	//    0x1234⇒0x3412
	//    b15-0⇒(b7-0|b15-8)
	wJoinTrctl0 = (WORD)((RmData_RX->dwSetMode << 5) & 0x0F00);	// FUNCS以外のビットをOFF

	// トレースFPGAのトレース制御１ジスタ
	//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
	//  ┌─────────┬───┬─┬─────┬─┬─┬─────┐
	//  │                  │      │  │          │  │  │          │
	//  └─────────┴───┴─┴─────┴─┴─┴─────┘
	//  b15-11:TRCON dwSetInfoのb10,dwSetInfoのb15,dwSetInfoのb22,dwSetInfoのb21,dwSetInfoのb19を連結
	//  b8:TRDDR '0'固定 ->s_RmData_RX.dwSetInitial 9bit目取得 
	//  b4:TRSW '0'固定
	//  b2-0:TRDW '010'固定
	//  ※実際は上記並びではなく、以下のような並びになっているようだ。
	//    0x1234⇒0x3412
	//    b15-0⇒(b7-0|b15-8)
	wJoinTrctl1 = (WORD)(((RmData_RX->dwSetInfo >> 3) & 0x00000080) |
						 ((RmData_RX->dwSetInfo >> 9) & 0x00000040) |
						 ((RmData_RX->dwSetInfo >> 17) & 0x00000020) |
						 ((RmData_RX->dwSetInfo >> 17) & 0x00000010) |
						 ((RmData_RX->dwSetInitial >> 9) & 0x00000001) |
						 ((RmData_RX->dwSetInfo >> 16) & 0x00000008));
	// RevRxNo130308-001-011, RevRxNo130408-001 Modify Start
	if ((RmData_RX->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D4S1) {	// DATA 4bit, SYNC 1bit の場合
		wData = REG_RM_TRCTL1_TRDW_4_DATA | REG_RM_TRCTL1_TRSW_1_DATA;

	} else if ((RmData_RX->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D8S2) {	// DATA 8bit, SYNC 2bit の場合
		wData = REG_RM_TRCTL1_TRDW_8_DATA | REG_RM_TRCTL1_TRSW_2_DATA;

	} else {	// DATA 8bit, SYNC 1bit の場合
		wData = REG_RM_TRCTL1_TRDW_8_DATA | REG_RM_TRCTL1_TRSW_1_DATA;
	}
	wData = (wData << 8) & 0xff00;
	wJoinTrctl1 |= wData;
	// RevRxNo130308-001-011, RevRxNo130408-001 Modify End

	// トレースメモリ容量制御レジスタ
	//    7   6   5   4   3   2   1   0　 7   6   5   4   3   2   1   0
	//  ┌─────────―─――──────────―─┬─┬─┬─┐
	//  │                                                  │  │  │  │
	//  └─────────────────────────┴─┴─┴─┘
	//  b2-0:TMSIZE s_wSetE20TrcMemSize
	//  ※実際は上記並びではなく、以下のような並びになっているようだ。
	//    0x1234⇒0x3412
	//    b15-0⇒(b7-0|b15-8)
	// RevRxNo130408-001 Modify Start
	wData = GetE20TrcMemSize();
	wTrcMemSize = (wData << 8) & 0xff00;
	// RevRxNo130408-001 Modify End

	//RevNo110309-004 Append Line
	// トレースクロック遅延の設定
	wIn_Dly = (WORD)((RmData_RX->dwSetInitial & RM_SETINITIAL_DLY) >> 16);

	//1.TRCTRL0:wJoinTrctlBuff0 TREN,FUNCSのみ0クリア値
	madrStartAddr[dwAddrCnt] = REG_RM_TRCTL0;
	wData = (WORD)(wJoinTrctl0 & 0xF0FF);
	memcpy(&pbyWriteData[dwDataCnt], &wData, sizeof(WORD));
	pdwCount[dwAddrCnt] = 0x00000001;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+2;


	// RevRxNo130730-006 Modify Start
	if ((s_bSetTrcE20FpgaAllRegFlg == TRUE) || (s_bNeedSetTrcFpgaHotplug == TRUE)) {
		// TRCTL1, IN_DLY, TMSIZEレジスタは、以下の場合のみ設定する。
		// (a)全レジスタ設定が指示されている場合
		// (b)ホットプラグ動作中に設定されたトレース情報のFPGAレジスタ設定が必要な場合

		//2.TRCTRL1:wJoinTrctlBuff1
		madrStartAddr[dwAddrCnt] = REG_RM_TRCTL1;
		wData = wJoinTrctl1;
		memcpy(&pbyWriteData[dwDataCnt], &wData, sizeof(WORD));
		pdwCount[dwAddrCnt] = 0x00000001;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+2;

		//3.IN_DLY:wIn_Dly
		//RevNo110309-004 Append Start
		madrStartAddr[dwAddrCnt] = REG_RM_IN_DYL;
		wData = wIn_Dly;
		memcpy(&pbyWriteData[dwDataCnt], &wData, sizeof(WORD));
		pdwCount[dwAddrCnt] = 0x00000001;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+2;
		//RevNo110309-004 Append End

		//4.TMSIZE:wTrcMemSize
		madrStartAddr[dwAddrCnt] = REG_RM_TMSIZE;	// RevRxNo130408-001 Modify Line
		wData = wTrcMemSize;
		memcpy(&pbyWriteData[dwDataCnt], &wData, sizeof(WORD));
		pdwCount[dwAddrCnt] = 0x00000001;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+2;
	}
	// RevRxNo130730-006 Modify End


	//5.TRCTRL0:wJoinTrctlBuff0 TREN1にしてすべて設定
	madrStartAddr[dwAddrCnt] = REG_RM_TRCTL0;
	// RevRxNo130308-001 Modify Line
	wJoinTrctl0 |= 0x0100;
	// RevRxNo130308-001-017 Append Start
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if ((wBuff & TRCTL0_B_LOST) == TRCTL0_B_LOST) {
			wJoinTrctl0 |= 0x0080;	// B_LOSTに相当する位置のビットを"1"にする。
		}
	}
	// RevRxNo130308-001-017 Append End
	wData = wJoinTrctl0;
	memcpy(&pbyWriteData[dwDataCnt], &wData, sizeof(WORD));
	pdwCount[dwAddrCnt] = 0x00000001;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+2;


	//データライト
	eAccessSize = EWORD_ACCESS;
	dwAreaNum = dwAddrCnt;	// RevRxNo130730-006 Modify Line

	// RevRxE2LNo141104-001 Modify Line
	// エミュレータ種別、BFW動作モードに対応したCPU_WRITEコマンドを発行(エミュレータ種別はFFW内部変数を参照)
	ferr = ProtCpuWrite(eAccessSize, dwAreaNum, madrStartAddr, pdwCount, pbyWriteData);
	
	delete [] pbyWriteData;
	delete [] pdwCount;

	// RevRxNo130730-006 Modify Line
	s_bNeedSetTrcFpgaHotplug = FALSE;	// ホットプラグ動作中に設定されたトレース情報のFPGAレジスタ設定要否フラグをクリア

	return ferr;
}
//RevNo100715-014 Append End
//==============================================================================
/**
 * E1/E20 BFWにトレースモードの設定をセットする。
 * @param  byTrcMode
 *         bit0 1:フルブレーク有効 0:フルブレーク無効
 *         bit1 1:内蔵トレース 0:外部トレース
 * @retval FFWエラーコード
 */
//==============================================================================
// RevRxNo120626-001 Modify Start
FFWERR SetTrcFpgaTrcMode(BYTE byTrcMode){
	
	FFWERR				ferr = FFWERR_OK;

	ferr = PROT_SetTRCMODE(byTrcMode);

	return ferr;

}
// RevRxNo120626-001 Modify End

//==============================================================================
/**
 * FPGA トレースデータ入力が終わったことを確認
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR CheckTrcFpgaStop(void){

	FFWERR				ferr = FFWERR_OK;
	WORD				wTmwbaBuf = 0;
	WORD				wTmwba;
	DWORD				dwTrcEndCnt = 0;
	DWORD				i;

	// トレースFPGAデータ読み取り終了まで待つ TMWBA値が同じ値がリードできるまで
	ferr = ReadFpgaReg(REG_RD_TMWBA, &wTmwba);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	dwTrcEndCnt = TRC_END_WAIT_TIME;
	i = 0;
	while( (wTmwba != wTmwbaBuf) & (i < dwTrcEndCnt) ) {
		wTmwbaBuf = wTmwba;
		Sleep (1);
		ferr = ReadFpgaReg(REG_RD_TMWBA, &wTmwba);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		i ++ ;
	}
	if( wTmwba != wTmwbaBuf ) {
		return FFWERR_RTTSTOP_FAIL;
	}

	return ferr;
}

//==============================================================================
/**
 * トレースデータを取得する。
 * @param dwBlockStart トレースメモリ開始アドレス
 * @param dwBlockEnd   取得するサイクル数の指定
 * @param rd           取得したトレースデータを格納する
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetTrcFpgaMemBlock(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* rd)
{

	FFWERR				ferr = FFWERR_OK;

	/* トレースデータを取得 */
	ferr = PROT_GetRD2_SPL(dwBlockStart, dwBlockEnd, rd);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;

}

//==============================================================================
/**
 * 全ブロックの識別子1を取得する。
 * @param  なし
 * @retval DWORD     識別子1の数
 */
//==============================================================================
DWORD GetTrcFpgaAllBlockCycle(void)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wTcCtl = 0;
	WORD	wTcQl = 0;
	WORD	wTmp = 0;
	DWORD	dwTrcCmdCnt = 0;

	// トレースFPGAのトレースコマンド数取得
	wTcCtl = TC_CTL_TC_CLR;
	wTmp = wTcCtl;
	ferr = WriteFpgaReg(REG_RM_TC_CTL, wTmp);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	wTcCtl = TC_CTL_TC_GO;
	wTmp = wTcCtl;
	ferr = WriteFpgaReg(REG_RM_TC_CTL, wTmp);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	while ((wTcCtl & TC_CTL_TC_GO) != 0x0000) {
		// トレースFPGAのトレースコマンド数取得完了待ち
		ferr = ReadFpgaReg(REG_RM_TC_CTL, &wTmp);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		wTcCtl = wTmp;
	}
	// トレースコマンド数(上位)取得

	// トレースFPGAのトレースコマンド数(下位)取得
	ferr = ReadFpgaReg(REG_RM_TC_QL, &wTmp);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	wTcQl = wTmp;

	// 全ブロックの識別子1の数を取得
	dwTrcCmdCnt = (DWORD)((wTcCtl & TC_CTL_TC_QH) * 0x10000) + wTcQl;

	return dwTrcCmdCnt;

}
//==============================================================================
/**
 * FPGA 使用している全ブロックの識別子1を取得する。
 * @param  dwBlockEnd 検索する最後のブロック
 * @param  pdwTrcCmdCntBuf 全サイクル数格納ポインタ
 * @param  p_dwIndexNum　各インデックスのサイクル数格納ポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetTrcFpgaAllBlockCycleIndex(DWORD dwBlockEnd,DWORD* pdwTrcCmdCntBuf,DWORD* p_dwIndexNum)
{
	DWORD	dwTrcCmdCntBuf = 0;
	BYTE*	p_byBuf;
	DWORD i;
	int j;
	int nIndexCnt = 0;
	FFWERR	ferr = FFWERR_OK;

	p_byBuf = new BYTE [RD_TMBUFFSIZE];
	memset(p_byBuf, 0, RD_TMBUFFSIZE);


	nIndexCnt = 0;
	// 全ブロックの識別子1の数を取り出す。
	for (i = 0 ; i <= (dwBlockEnd/TRCMEM_BLKDIV_SIZE); i++) {
		ferr = PROT_GetRCYCntN((WORD) (i*TRCMEM_BLKDIV_SIZE), TRCMEM_BLKDIV_SIZE*4, p_byBuf);
		if (ferr != FFWERR_OK) {
			delete [] p_byBuf;
			return ferr;
		}
		for (j = 0; j < TRCMEM_BLKDIV_SIZE*4 ; j = j+4) {
			p_dwIndexNum[nIndexCnt] =  (p_byBuf[j+3])|(p_byBuf[j+2]<<8) |(p_byBuf[j+1]<<16) | (p_byBuf[j]<<24);
			dwTrcCmdCntBuf = dwTrcCmdCntBuf + p_dwIndexNum[nIndexCnt];
			nIndexCnt ++ ;
		}
	}
	delete [] p_byBuf;


	*pdwTrcCmdCntBuf = dwTrcCmdCntBuf;

	return ferr;

}

//==============================================================================
/**
 * E20トレース入力を有効/無効にする
 * @param  bTren TRUE:有効 FLASE:無効
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetTrcFpgaEnable(BOOL bTren)
{

	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;
	WORD	wTrCtl = 0;
	// RevRxNo130308-001-002 Append Start
	FFWMCU_DBG_DATA_RX*	pDbgData;

 	pDbgData = GetDbgDataRX();
	// RevRxNo130308-001-002 Append End

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//1ライトで無効になるフラグ位置を0にしておく
	wTrCtl = wBuff;
	if(bTren == TRUE){
		//有効
		//TREN 無効→有効でTMFUL,TRERR,LOST_Dはクリアするが、意図的に1ライトしてクリアをする
		wTrCtl = (WORD)(wTrCtl | (TRCTL0_TMFUL|TRCTL0_TRERR|TRCTL0_LOST_D));
		wTrCtl = (WORD)(wTrCtl | TRCTL0_TREN);
	} else {
		//無効
		// RevRxNo130308-001-002 Modify Start
		if (pDbgData->eTrcFuncMode != EML_TRCFUNC_CV) {	// "カバレッジ用に利用"以外の場合
			//TMFUL,TRERR,LOST_Dはクリアしないようにするため、0ライト
			wTrCtl = (WORD)(wTrCtl & ~(TRCTL0_TMFUL|TRCTL0_TRERR|TRCTL0_LOST_D));
		} else {
			//LOST_D(bit13)、TRERR(bit12)、TMFUL(bit10)、RSTF(bit8)、TM2FUL(bit7)はクリアしないようにするため、0ライト
			wTrCtl = (WORD)(wTrCtl & ~MSK_TRCTL0_FLAGS);
		}
		// RevRxNo130308-001-002 Modify End
		wTrCtl = (WORD)(wTrCtl & ~TRCTL0_TREN);
	}
	wBuff = wTrCtl;

	ferr = WriteFpgaReg(REG_RM_TRCTL0, wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}


//==============================================================================
/**
 * E20トレースバッファのデータ取得
 * @param  wTmwba E20トレースバッファアドレスレジスタ(TMWBA)値
 * @param  pwRd トレースメモリデータ格納領域アドレス
 * @param  dwSetStart pwRdの格納トレースメモリデータ格納開始位置
 * @retval FFWエラーコード
 *
 * ※pwRdに指定するトレースメモリデータ格納領域のサイズは、
 * 　トレースデータ出力端子幅により異なる。
 */
//==============================================================================
FFWERR GetTrcFpgaBuffData(WORD wTmwba, WORD* pwRd, DWORD dwSetStart)
{
	FFWERR	ferr = FFWERR_OK;
	EADDR	eadrReadAddr[1];					// 参照するレジスタのアドレス
	BYTE*	pbyReadFpga;						// 参照データを格納する領域のアドレス(FPGA用)
	DWORD	dwLength;							// 取得するトレースバッファデータバイトサイズ
	DWORD	dwReadCnt;							// CPU_Rでリードする回数
	DWORD	i;
	// RevRxNo130408-001 Append Start
	FFWRX_RM_DATA*	pRm;
	BOOL	bConv;
	DWORD	dwRdBuffSize;
	WORD	wTmp;
	BYTE*	pbyData;
	// RevRxNo130408-001 Append End

	pRm = GetRmDataInfo();	// RevRxNo130408-001 Append Line

	// RevRxNo130408-001 Append Start
	// トレースデータ出力端子幅がDATA 8ビット、SYNC 2ビットの場合
	if ((pRm->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D8S2) {
		bConv = TRUE;	// トレースメモリデータフォーマット変換必要
	} else {
		bConv = FALSE;	// トレースメモリデータフォーマット変換不要
	}
	// RevRxNo130408-001 Append End


	// E20トレースバッファリード情報設定
	if ((wTmwba & REG_RD_TMWBA_TMWBS)  != REG_RD_TMWBA_TMWBS) {
		eadrReadAddr[0] = REG_RD_TMBUFF0;	// バッファ#0
	} else {
		eadrReadAddr[0] = REG_RD_TMBUFF1;	// バッファ#1
	}
	dwLength = (DWORD)((wTmwba & REG_RD_TMWBA_TMWBA) << 2);		// リードバイト数
	dwReadCnt = dwLength / 2;		// ワードアクセスでのリード回数

	// E20トレースバッファデータ格納用バッファ確保
	pbyReadFpga = new BYTE [dwLength];

	// RevRxE2LNo141104-001 Modify Line
	// エミュレータ種別、BFW動作モードに対応したCPU_Rコマンドを発行(エミュレータ種別はFFW内部変数を参照)
	ferr = ProtCpuRead(EWORD_ACCESS, 1, eadrReadAddr, &dwReadCnt, pbyReadFpga);
	if (ferr != FFWERR_OK) {
		delete [] pbyReadFpga;
		return ferr;
	}

	// RevRxNo130408-001 Modify Start
	// E20トレースバッファのトレースメモリデータ格納バッファpwRd[]サイズ取得
	dwRdBuffSize = GetRdBufSizeTraceBuffer(wTmwba);

	pbyData = pbyReadFpga;
	if (bConv == FALSE) {	// フォーマット変換不要の場合
		for (i = 0 ; i < dwRdBuffSize; i = i + 2) {
			// *pbyData, *(pbyData + 1)のデータをpwRd[dwSetStart+i]に格納
			wTmp = static_cast<WORD>(*pbyData);
			pwRd[dwSetStart + i] = ((wTmp << 8) & 0xff00) | static_cast<WORD>(*(pbyData + 1));
			pbyData += 2;
		}

	} else {	// フォーマット変換必要の場合
		for (i = 0 ; i < dwRdBuffSize; i = i + 4) {
			// DATA 8ビット、SYNC 2ビットフォーマットを、DATA 4ビット、SYNC 1ビットフォーマットに変換して、
			// pwRd[dwSetStart+i], pwRd[dwSetStart+i+2]へ格納
			wTmp = static_cast<WORD>(*pbyData);
			wTmp = ((wTmp << 8) & 0xff00) | static_cast<WORD>(*(pbyData + 1));
			pwRd[dwSetStart + i] =     ((wTmp << 4) & 0x7000)		// トレース情報種別-0
										| ((wTmp >> 1) & 0x0400)	// コマンド/データ識別信号-0
										| (wTmp & 0x000f);			// コマンド/データ-0
			pwRd[dwSetStart + i + 2] = (wTmp & 0x7000)				// トレース情報種別-1
										| ((wTmp >> 5) & 0x0400)	// コマンド/データ識別信号-1
										| ((wTmp >> 4) & 0x000f);	// コマンド/データ-1
			pbyData += 2;
		}
	}
	// RevRxNo130408-001 Modify End

	delete [] pbyReadFpga;

	return ferr;

}


//==============================================================================
/**
 * FPGA トレースバッファからトレースメモリへデータを押し出す処理
 * @param  pwTmwa トレースメモリ入力ポインタアドレス
 * @param  pwTmwba トレースバッファ入力ポインタアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetTrcFpgaBuf2Mem(WORD* pwTmwa,WORD* pwTmwba){

	FFWERR ferr = FFWERR_OK;
	EADDR	eadrReadAddr[1];
	WORD	wBuf;
	BYTE	byFillFpga[2];
	
	//データ押し出し
	//トレースバッファ0の最後のアドレスにライト
	eadrReadAddr[0] = REG_RD_TMBUFF0;
	ferr = WriteFpgaReg(eadrReadAddr[0]+RD_TMBUFF_LAST_ADDR, 0);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wBuf = (WORD)(wBuf & TRCTL0_TMBUSY);
	while(wBuf != 0) {
		ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuf);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		wBuf = (WORD)(wBuf & TRCTL0_TMBUSY);
	}

	//以下再取得
	// トレースFPGAのトレースメモリ書込み用アドレスレジスタ
	ferr = ReadFpgaReg(REG_RD_TMWA, pwTmwa);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//念のため、バッファを0でクリアしておく
	memset(&byFillFpga[0], 0, sizeof(WORD));

	// RevRxE2LNo141104-001 Modify Line
	// エミュレータ種別、BFW動作モードに対応したCPU_FILLコマンドを発行(エミュレータ種別はFFW内部変数を参照)
	ferr = ProtCpuFill(EWORD_ACCESS, eadrReadAddr[0], RD_TMBUFF_BLKSIZE,&byFillFpga[0]);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// トレースFPGAのトレースメモリ書込み用バッファアドレスレジスタ
	*pwTmwba = 0;

	return ferr;
}

// RevRxNo121227-001 Append Start
//==============================================================================
/**
 * E20トレースFPGAのトレース状態クリア(TRENは0に設定する)
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcFpgaStatus(void)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff = 0;

	// LOST_D, TRERR, TMFUL, TM2FULフラグをクリア
	// これらは"1"ライトでクリアされるため、リード値をライトする。

	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// TRENを0に設定
	wBuff = wBuff & (WORD)(~TRCTL0_TREN);

	ferr = WriteFpgaReg(REG_RM_TRCTL0, wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}
// RevRxNo121227-001 Append End

//==============================================================================
/**
 * FPGA トレースFPGAの状態取得
 * @param  pbTren　TRUE:トレース有効 FALSE:トレース無効
 * @param  pwFuncs フル/フリートレースモード
 * @param  pbTmFull　TRUE:トレースフルビット1 FALSE:トレースフルビット0
 * @param  pbTm2Full TRUE:トレースフル2ビット1 FALSE:トレースフル2ビット0
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetTrcFpgaStatInfo(BOOL* pbTren,WORD* pwFuncs,BOOL* pbTmFull,BOOL* pbTm2Full){

	FFWERR ferr = FFWERR_OK;
	WORD	wTrCtl;


	*pbTmFull = FALSE;
	*pbTm2Full = FALSE;
	*pbTren = FALSE;

	// トレースFPGAのトレース制御０レジスタ
	// (b10:TMFUL (0:トレース未フル状態、1:トレースフル状態) オーバーフローフラグとして使えないのでTM2FULがある)
	// (b7:TM2FUL (0:トレース未フル状態、1:トレースフル状態))
	// (b3-1:FUNCS (0:フリートレースモード、1:フルトレースモード))
	ferr = ReadFpgaReg(REG_RM_TRCTL0, &wTrCtl);

	if((wTrCtl & TRCTL0_TREN) == TRCTL0_TREN){
		*pbTren = TRUE;
	}
	if((wTrCtl & TRCTL0_TMFUL) == TRCTL0_TMFUL){
		*pbTmFull = TRUE;
	}
	if((wTrCtl & TRCTL0_TM2FUL) == TRCTL0_TM2FUL){
		*pbTm2Full = TRUE;
	}
	*pwFuncs = (WORD)((wTrCtl & TRCTL0_FUNCS) >> 1);		// FUNCS

	return ferr;


}
//==============================================================================
/**
 * FPGA トレースFPGAメモリの入力ポインタ値の取得
 * @param  pwTmwa　入力ポインタ格納
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetTrcE20MemTmwa(WORD* pwTmwa){

	FFWERR	ferr = FFWERR_OK;

	ferr = ReadFpgaReg(REG_RD_TMWA, pwTmwa);

	return ferr;

}
//==============================================================================
/**
 * FPGA トレースFPGAバッファの入力ポインタ値の取得
 * @param  pwTmwa　入力ポインタ格納
 * @retval FFWERR_OK  正常終了
 */
//==============================================================================
FFWERR GetTrcFpgaTmwba(WORD* pwTmwa){

	FFWERR	ferr = FFWERR_OK;

	ferr = ReadFpgaReg(REG_RD_TMWBA, pwTmwa);

	return ferr;

}

// RevRxNo130730-006 Append Start
//==============================================================================
/**
 * E20 FPGAのトレース関連全レジスタ設定指示フラグの設定
 *	SetTrcE20FpgaData()で、設定対象の全レジスタを設定するかを指定する。
 * @param  bFlg E20 FPGAトレース関連全レジスタ設定指示フラグの設定値
 *				　TRUE：全レジスタ(TRCTL0,TRCTL1,IN_DLT,TMSIZE)を設定
 *				　FALSE：一部レジスタ(TRCTL0)のみ設定
 * @retval なし
 */
//==============================================================================
void SetTrcE20FpgaAllRegFlg(BOOL bFlg)
{
	s_bSetTrcE20FpgaAllRegFlg = bFlg;

	return;
}
// RevRxNo130730-006 Append End

// RevRxNo130730-006 Append Start
//==============================================================================
/**
 * ホットプラグ動作中に設定されたトレース情報のFPGAレジスタ設定要否を示すフラグの設定
 * @param  bFlg ホットプラグ動作中に設定されたトレース情報のFPGAレジスタ設定要否を
 *         示すフラグの設定値
 * @retval なし
 */
//==============================================================================
void SetNeedTrcE20FpgaHotplug(BOOL bFlg)
{
	s_bNeedSetTrcFpgaHotplug = bFlg;

	return;
}
// RevRxNo130730-006 Append End


///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * E20トレースFPGA制御関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitHwRxFpgaData_Tra(void)
{
	// RevRxNo130730-006 Append Start
	s_bSetTrcE20FpgaAllRegFlg = TRUE;
	s_bNeedSetTrcFpgaHotplug = FALSE;
	// RevRxNo130730-006 Append End

	return;
}


