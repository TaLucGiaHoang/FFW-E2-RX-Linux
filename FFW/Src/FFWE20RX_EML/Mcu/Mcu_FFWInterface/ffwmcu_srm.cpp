///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_srm.cpp
 * @brief 指定ルーチン実行関連コマンドの実装ファイル
 * @author RSD H.Hashiguchi, Y.Miyake, S.Ueda, Y.Kawakami, K.Okita(PA K.Tsumu), SDS T.Iwata, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/02/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-006 2012/11/21 三宅(2012/12/04 SDS 岩田 マージ)
　・FFWMCUCmd_SetSRM() で、
　　・wSrmData.dwmadrRamWorkAreaStartAddr の引数エラー処理追加。
　　・指定ルーチン有効時に、指定ルーチン実行時のワーク・プログラム領域の
　　　ライト処理追加。
　・static変数 s_bySrmWorkProgramCode、s_bySrmWorkProgram を追加。
  ・GetSrmWorkProgramCode() の関数定義追加。
　・GetSrmData() の関数定義追加。
  ・domcu_mem.h のインクルード追加。
・RevRxNo120910-006 2012/12/11 三宅
  ・s_bySrmWorkProgram[] を、Stopルーチンのみ時もFrontC2E1に"1"設定追加に変更。
・RevRxNo120910-006 2012/12/21 三宅
  ・FFWMCUCmd_SetSRM()で、指定ルーチン用ワークRAMの引数エラー処理修正。
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001	2013/05/13 植盛
	RX64M対応
・RevRxNo130730-004 2013/11/26 川上
	Start/Stop高速化対応
・RevRxNo140515-005 2014/07/19 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxE2LNo141104-001 2014/11/20 上田
	E2 Lite対応
・RevRxE2No171004-001 2017/10/04 PA 紡車
　　E2拡張機能対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
*/
#include "ffwmcu_srm.h"
#include "ffwmcu_mcu.h"
#include "domcu_prog.h"
#include "domcu_srm.h"
#include "ffw_sys.h"
//V.1.02 RevNo110613-001 Append Line
#include "errchk.h"
//RevNo120802-001 Append Line
#include "domcu_mem.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line
#include "mcuspec.h"	//RevRxNo130730-012 Append Line
#include "mcu_mem.h"	// RevRxNo140515-005 Append Line

// RevRxE2No171004-001 Append Start
#include "do_sys.h"
#include "doasp_sys_family.h"
// RevRxE2No171004-001 Append End

// ファイル内static変数の宣言
static FFW_SRM_DATA	s_SrmData;	// 指定ルーチン実行情報格納構造体変数
// RevRxNo120910-006 Append Start
static BYTE s_bySrmWorkProgramCode[SRM_WORK_PROGRAM_LENGTH];	// レジスタのある/なしを反映後のコード
// RevRxNo120910-006 Modify Start, RevRxNo130730-004 Modify Line
static const BYTE s_bySrmWorkProgram[SRM_WORK_PROGRAM_LENGTH] = 	// FPSWあり、ACCあり、CPENありの場合
   {                    0xFB,0xF2,0xD4,0x40,0x08,0x00,0xF8,0xF6,0x01,0xFB,0xF2,0x02,
	0x40,0xFF,0xFF,0x7F,0x1F,0xFB,0xF2,0x04,0x30,0x00,0x00,0xED,0xF2,0x01,0xED,0xF3,
	0x02,0xED,0xF4,0x03,0xED,0xF5,0x04,0xED,0xF6,0x05,0xED,0xF7,0x06,0xED,0xF8,0x07,
	0xED,0xF9,0x08,0xED,0xFA,0x09,0xED,0xFB,0x0A,0xED,0xFC,0x0B,0xED,0xFD,0x0C,0xED,
	0xFE,0x0D,0xED,0xF1,0x0F,0xFD,0x68,0x1A,0xED,0xF1,0x10,0xFD,0x68,0x12,0xED,0xF1,
	0x11,0xFD,0x68,0x1C,0xED,0xF1,0x12,0xFD,0x68,0x19,0xED,0xF1,0x13,0xFD,0x68,0x18,
	0xED,0xF1,0x14,0xFD,0x69,0x19,0xED,0xF1,0x15,0xFD,0x69,0x18,0xED,0xF1,0x16,0xFD,
	0x68,0x1B,0xED,0xF1,0x17,0xFD,0x68,0x13,0xED,0xF1,0x18,0xFD,0x68,0x14,0xED,0xF1,
	0x19,0xFD,0x17,0x01,0xED,0xF1,0x1A,0xFD,0x17,0x11,0x03,0x03,0x03,0x03,0x03,0x03,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0xED,0xF1,0x00,0xFB,0xF2,0x84,0x40,0x08,
	0x00,0xF8,0xF6,0x01,0xFD,0x6A,0x7F,0x7F,0x94,0xFB,0xF2,0xD4,0x40,0x08,0x00,0xF8,
	0xF6,0x01,0xFB,0xF2,0x84,0x40,0x08,0x00,0xF8,0xF6,0x01,0xFD,0x6A,0x7F,0x7F,0x94,
	0xFD,0x6A,0x71,0xFD,0x68,0xF7,0xFB,0xF2,0x04,0x30,0x00,0x00,0xE7,0xF1,0x00,0xE7,
	0xF2,0x01,0xE7,0xF3,0x02,0xE7,0xF4,0x03,0xE7,0xF5,0x04,0xE7,0xF6,0x05,0xE7,0xF7,
	0x06,0xE7,0xF8,0x07,0xE7,0xF9,0x08,0xE7,0xFA,0x09,0xE7,0xFB,0x0A,0xE7,0xFC,0x0B,
	0xE7,0xFD,0x0C,0xE7,0xFE,0x0D,0xFD,0x6A,0x71,0xE7,0xF1,0x0E,0xFD,0x6A,0xA1,0xE7,
	0xF1,0x0F,0xFD,0x6A,0x21,0xE7,0xF1,0x10,0xFD,0x6A,0xC1,0xE7,0xF1,0x11,0xFD,0x6A,
	0x91,0xE7,0xF1,0x12,0xFD,0x6A,0x81,0xE7,0xF1,0x13,0xFD,0x6B,0x91,0xE7,0xF1,0x14,
	0xFD,0x6B,0x81,0xE7,0xF1,0x15,0xFD,0x6A,0xB1,0xE7,0xF1,0x16,0xFD,0x6A,0x31,0xE7,
	0xF1,0x17,0xFD,0x6A,0x41,0xE7,0xF1,0x18,0xFD,0x1F,0x01,0xE7,0xF1,0x19,0xFD,0x1F,
	0x21,0x6D,0x01,0xE7,0xF1,0x1A,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x03,0x03,0x03,0x03,0x03,0x03,0xFD,0x68,0xFA,0xFB,0x12,0x00,0x00,0x00,0x00,0xFD,
	0x68,0x10,0xED,0xF1,0x00,0xFB,0xF2,0x02,0x40,0xFF,0xFF,0x7F,0x1F,0xFB,0xF2,0xD4,
	0x40,0x08,0x00,0xF8,0xF6,0x00,0xFB,0xF2,0x2C,0x32,0x00,0x00,0xF8,0xF4,0x01,0xFB,
	0xF2,0x00,0x40,0x08,0x00,0x7F,0x0F,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03};
// RevRxNo120910-006 Modify End
// RevRxNo130730-004 Append Start
static const BYTE s_bySrmWorkProgramRxv2[SRM_WORK_PROGRAM_RXV2] =		// RXV2アーキテクチャの場合
	{
	0xED,0xF1,0x19,0xFD,0x17,0x31,0xED,0xF1,0x1A,0xFD,0x17,0x01,0xED,0xF1,0x1B,0xFD,
	0x17,0x11,0xED,0xF1,0x1C,0xFD,0x17,0xB1,0xED,0xF1,0x1D,0xFD,0x17,0x81,0xED,0xF1,
	0x1E,0xFD,0x17,0x91,0xED,0xF1,0x1F,0xFD,0x68,0x1D,0xFD,0x1F,0x31,0xE7,0xF1,0x19,
	0xFD,0x1F,0x01,0xE7,0xF1,0x1A,0xFD,0x1F,0x11,0xE7,0xF1,0x1B,0xFD,0x1F,0xB1,0xE7,
	0xF1,0x1C,0xFD,0x1F,0x81,0xE7,0xF1,0x1D,0xFD,0x1F,0x91,0xE7,0xF1,0x1E,0xFD,0x6A,
	0xD1,0xE7,0xF1,0x1F
	};
// RevRxNo130730-004 Append End
// RevRxNo120910-006 Append End

// RevRxE2No171004-001 Append Start
static BYTE s_byMonitorPointProgramCode[MON_POINT_PROGRAM_LENGTH];	// ワークRAMアドレス反映後のコード
// 通過ポイント用モニタプログラム
static const BYTE s_byMonitorPointProgram[MON_POINT_PROGRAM_LENGTH] =
{0xFB,0xF2,0xD4,0x40,0x08,0x00,0xF8,0xF6,0x01,0xFD,0x6A,0x7F,0x7F,0x94,0xFD,0x6A,
0x71,0xFD,0x68,0xF7,0x6E,0x15,0xFB,0xF2,0x55,0x30,0x00,0x00,0xF4,0xF0,0x21,0x4A,
0xFB,0xF2,0x00,0x49,0x08,0x00,0xF4,0xF1,0x20,0x40,0xFD,0x6A,0x91,0x60,0x11,0xFB,
0x22,0x00,0x30,0x00,0x00,0xEF,0x25,0xEC,0x23,0x62,0x42,0x66,0x04,0x06,0x84,0x21,
0x20,0x0C,0x62,0x42,0x62,0x14,0x47,0x43,0x21,0xF5,0x2E,0x1E,0x71,0x44,0x44,0x4B,
0x54,0xCC,0x41,0xFB,0x2A,0x00,0xFF,0x57,0x21,0xFB,0x22,0x80,0x40,0x08,0x00,0xE3,
0x21,0x6F,0x15,0xFD,0x6A,0x7F,0x7F,0x94,0xFB,0xF2,0xD4,0x40,0x08,0x00,0xF8,0xF6,
0x00,0xFB,0x1A,0x00,0xFF,0xFB,0x22,0x80,0x40,0x08,0x00,0xE3,0x21,0x6F,0x15,0xFB,
0xF2,0x00,0x40,0x08,0x00,0x7F,0x0F,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03};
// RevRxE2No171004-001 Append End


///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 指定ルーチン実行のモード設定
 * @param pSrMode 指定ルーチン実行情報格納構造体のアドレス
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_SetSRM(const FFW_SRM_DATA* pSrMode)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_SRM_DATA wSrmData;
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	// RevRxNo120910-006 Append Start
	FFWMCU_MCUDEF_DATA* pMcuDef;
	// RevRxNo140515-005 Delete Start/Stopファンクション用ワークRAMの設定処理をDO_SetSRM()に移動 
	FFWRX_MCUAREA_DATA* pMcuArea;
	DWORD	dwCnt;
	BOOL	bSrmWorkRamInRam = FALSE;				// 指定ルーチン用ワークRAMが内蔵RAM領域内かのフラグ(TRUE:RAM領域内、FALSE:RAM領域外もあり)
	// RevRxNo120910-006 Append End
	// RevRxNo120910-006 Append Start
	MADDR	dwmadrRamWorkStart;						// 指定ルーチン用ワークRAMが内蔵RAM領域内かの判定時に使用する。
	MADDR	dwmadrRamWorkEnd;						// 指定ルーチン用ワークRAMが内蔵RAM領域内かの判定時に使用する。
	// RevRxNo120910-006 Append End
	FFWE20_EINF_DATA Einf;		// RevRxE2No171004-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	getEinfData(&Einf);		// RevRxE2No171004-001 Append Line

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得


   // Work RAMエリアを内部処理で取得するため、ローカルの構造体を用意
	memcpy( (void*)&wSrmData, (const void*)pSrMode, sizeof(FFW_SRM_DATA) );

	//ホットプラグ実行中はRUN中エラーを返さない
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}

	// ユーザブレーク中の指定ルーチン実行、割り込み実行 設定無効
	// RevRxNo120910-006 Modify Line
	if((wSrmData.dwSrMode & ~(SRM_START_FUNC_BIT | SRM_STOP_FUNC_BIT | SRM_MON_POINT_FUNC_BIT)) != 0x00000000) {	// RevRxE2No171004-001 Modify Line
		return FFWERR_FFW_ARG;
	}
	// RevRxNo120910-006 Append Start
	// RevRxE2No171004-001 Modify Line
	if(((wSrmData.dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((wSrmData.dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)
		|| ((wSrmData.dwSrMode & SRM_MON_POINT_FUNC_BIT) == SRM_MON_POINT_FUNC_BIT)){
		// Startルーチン指定、Stopルーチン指定、通過ポイント指定のいずれかが立っている場合はワークRAMの確認を実施する
		pMcuDef = GetMcuDefData();
		if (wSrmData.dwmadrRamWorkAreaStartAddr > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		if ((wSrmData.dwmadrRamWorkAreaStartAddr % 4) != 0) {	// 4で割り切れない場合
			return FFWERR_FFW_ARG;
		}
		// 指定ルーチン用ワークRAMが、RAM領域内かの確認
		pMcuArea = GetMcuAreaDataRX();
		// RevRxNo120910-006 Modify Start
		dwmadrRamWorkStart = wSrmData.dwmadrRamWorkAreaStartAddr;
		// RevRxE2No171004-001 Modify Start
		if ((wSrmData.dwSrMode & SRM_MON_POINT_FUNC_BIT) == SRM_MON_POINT_FUNC_BIT) {
			dwmadrRamWorkEnd = wSrmData.dwmadrRamWorkAreaStartAddr + MON_POINT_WORKRAM_LENGTH - 1;
		} else {
			dwmadrRamWorkEnd = wSrmData.dwmadrRamWorkAreaStartAddr + SRM_WORKRAM_LENGTH - 1;
		}
		// RevRxE2No171004-001 Modify End
		dwCnt = 0;
		for(;;){
			if (dwCnt == pMcuArea->dwRamAreaNum) {	// 内蔵RAMの配列番号を超えた場合
				break;		// "指定ルーチン用ワークRAMが内蔵RAM領域外もあり"と判定される。
			}
			if ((pMcuArea->dwmadrRamStartAddr[dwCnt] <= dwmadrRamWorkStart) && 
				(dwmadrRamWorkStart <= pMcuArea->dwmadrRamEndAddr[dwCnt])      ) {	
				// 指定ルーチンワークRAM先頭がRAMブロックに入る場合
				
				if (dwmadrRamWorkEnd <= pMcuArea->dwmadrRamEndAddr[dwCnt]) { 
					// 指定ルーチンワークRAM最終がそのRAMブロックに入る場合
					
					bSrmWorkRamInRam = TRUE;	// フラグに、"指定ルーチン用ワークRAMが内蔵RAM領域内"を設定
					break;
				} else {
					// 指定ルーチンワークRAM最終がそのRAMブロックに入らない場合（指定ルーチンワークRAM先頭はそのRAMブロックに入っている）
					
					dwmadrRamWorkStart = pMcuArea->dwmadrRamEndAddr[dwCnt] + 1;		// 指定ルーチンワークRAM先頭を、「そのRAMブロックの最終 + 1」に更新。
					dwCnt = 0;	// カウンタを "0" に戻す。"0"から再度回すため。
				}
			} else {	
				// 指定ルーチンワークRAM先頭がRAMブロックに入らない場合
				
				dwCnt++;	// インクリメント
			}
		}
		// RevRxNo120910-006 Modify End
		if (bSrmWorkRamInRam == FALSE) {	// "指定ルーチン用ワークRAMが内蔵RAM領域外もあり"の場合
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxNo120910-006 Append End
	// RevRxE2No171004-001 Append Start
	if (Einf.wEmuStatus != EML_E2) {		// E2エミュレータ以外の場合
		// dwSrMode.bit4が'1'の場合エラー
		if ((wSrmData.dwSrMode & SRM_MON_POINT_FUNC_BIT) == SRM_MON_POINT_FUNC_BIT) {
			return FFWERR_FFW_ARG;
		}
	}
#if defined(E2E2L)
	if (Einf.wEmuStatus == EML_E2) {		// E2エミュレータの場合
		// dwSrMode.bit4が'1'で、bit0またはbit1が'1'の場合
		if ((wSrmData.dwSrMode & SRM_MON_POINT_FUNC_BIT) == SRM_MON_POINT_FUNC_BIT) {
			if ((wSrmData.dwSrMode & (SRM_START_FUNC_BIT | SRM_STOP_FUNC_BIT)) != 0) {
				return FFWERR_SRM_USE_WORKRAM;
			}
		}
		// 通過ポイントが登録された状態(※)で指定ルーチン有効指定が来た場合
		if (IsFamilyAspFuncEna(RF_ASPFUNC_MONEV)) {
			if ((wSrmData.dwSrMode & (SRM_START_FUNC_BIT | SRM_STOP_FUNC_BIT)) != 0) {
				return FFWERR_SRM_USE_WORKRAM;
			}
		}
	}
#endif
	// RevRxE2No171004-001 Append End

	// 使用しない設定領域には、0を設定
	wSrmData.dwmadrBreakFunctionAddr = 0;
	wSrmData.dwBreakInterruptFunctionIPL = 0;
	wSrmData.dwmadrRomWorkAreaStartAddr = 0;

	// FFW内部管理構造体変数の更新
	memcpy( (void*)&s_SrmData, (void*)&wSrmData, sizeof(FFW_SRM_DATA) );

	// SetSrmの実行
	ferr = DO_SetSRM(&wSrmData);

	// RevRxNo140515-005 Delete Start/Stopファンクション用ワークRAMの設定処理をDO_SetSRM()に移動 

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * 指定ルーチン実行のモード参照
 * @param pSrMode 指定ルーチン実行情報格納構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_GetSRM(FFW_SRM_DATA* pSrMode)
{
	FFWERR	ferr = FFWERR_OK;

	if (ferr == FFWERR_OK) {
		// FFW内部管理構造体変数の更新
		memcpy( (void*)pSrMode, (const void*)&s_SrmData, sizeof(FFW_SRM_DATA) );
	}

	return ferr;
}


//=============================================================================
/**
 * 指定ルーチン実行関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Srm(void)
{
	// 指定ルーチン実行情報格納構造体変数の初期化
	s_SrmData.dwSrMode = INIT_SRM_MODE;
	s_SrmData.dwmadrBeforeRunFunctionAddr = INIT_BRUN_FUNCADDR;
	s_SrmData.dwmadrAfterBreakFunctionAddr = INIT_ABREAK_FUNCADDR;
	s_SrmData.dwmadrBreakFunctionAddr = INIT_BREAK_FUNCADDR;
	s_SrmData.dwBreakInterruptFunctionIPL = INIT_INT_IPL;
	s_SrmData.dwmadrRomWorkAreaStartAddr = INIT_WORKROM_ADDR;
	s_SrmData.dwmadrRamWorkAreaStartAddr = INIT_WORKRAM_ADDR;
	
	return;
}


// RevRxNo120910-006 Append Start
//=============================================================================
/**
 * FFW内で定義している指定ルーチン用ワーク・プログラムコードの配列のポインタを返送する。
 * @param pSrMode 指定ルーチン実行情報格納構造体のアドレス
 * @retval 指定ルーチン用ワーク・プログラムコードの配列へのポインタ
 */
//=============================================================================
BYTE* GetSrmWorkProgramCode(const FFW_SRM_DATA* pSrMode)
{
	// RevRxNo130730-012 Modify Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	DWORD	dwmadrWorkRamRegAddr;		// ワークRAM内のレジスタ格納スタートアドレス
	DWORD	dwmadrWorkRamStopFlagAddr;	// ワークRAM内のStopルーチン完了フラグのアドレス
	DWORD	dwLen;
	DWORD	dwOrgNum = 0;				// コピー元ワークプログラムs_bySrmWorkProgram[]の要素番号
	DWORD	dwRxv2Num = 0;				// コピー元RXV2用プログラムs_bySrmWorkProgramRxv2[]の要素番号
	DWORD	dwCodeNum = 0;				// コピー先ワークプログラムs_bySrmWorkProgramCode[]の要素番号
	BYTE	byBraSrcCode;

	pMcuInfo = GetMcuInfoDataRX();		// MCU情報取得
	// 先頭からFINTV復帰までをコピー先ワークプログラム配列に設定
	dwLen = SRM_COMMON_PROG1_LENGTH;
	memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);

	dwOrgNum += dwLen;
	dwCodeNum += dwLen;

	// RevRxNo130411-001 Modify Line
	dwLen = SRM_RET_FPU_LENGTH;
	if (pMcuInfo->eFpuSet == RX_FPU_ON) {	// FPUありの場合
		// FPU復帰プログラムをコピー先ワークプログラム配列に設定
		memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);
		dwCodeNum += dwLen;	
	}
	dwOrgNum += dwLen;

	dwLen = SRM_RET_CPEN_LENGTH;
	if (pMcuInfo->eCoProSet == COPRO_SET_ON) {	// CPENありの場合
		// CPEN復帰プログラムをコピー先ワークプログラム配列に設定
		memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);
		dwCodeNum += dwLen;
	}
	dwOrgNum += dwLen;

	if (pMcuInfo->eAccSet == ACC_SET_ON) {		// ACCありの場合
		// RevRxNo180228-001 Modify Start
		if (pMcuInfo->eCpuIsa == RX_ISA_RXV1) {	// RX V1アーキテクチャの場合
			dwLen = SRM_RET_ACC_LENGTH;
			// ACC復帰プログラムをコピー先ワークプログラム配列に設定
			memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);
			dwCodeNum += dwLen;
		} else {	// RX V2/RX V3アーキテクチャの場合
			dwLen = SRM_RET_ACC_RXV2_LENGTH;
			// ACC0,1復帰プログラムをコピー先ワークプログラム配列に設定
			memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgramRxv2[dwRxv2Num], dwLen);
			dwCodeNum += dwLen;
			dwRxv2Num += dwLen;
		}
		// RevRxNo180228-001 Modify End
	}
	dwOrgNum +=	SRM_RET_ACC_LENGTH;		// ACC復帰プログラムの次に処理が追加になった場合を考慮し、
										// RXV1,RXV2に関係なくdwOrgNumを加算する

	dwLen = SRM_RET_EXTB_RXV2_LENGTH;
	// RevRxNo180228-001 Modify Line
	if (pMcuInfo->eCpuIsa != RX_ISA_RXV1) {	// RX V2/RX V3アーキテクチャの場合
		// EXTB復帰プログラムをコピー先ワークプログラム配列に設定
		memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgramRxv2[dwRxv2Num], dwLen);
		dwCodeNum += dwLen;
		dwRxv2Num += dwLen;
	}

	// STARTルーチン用プログラム(2)へのBRA設定
	byBraSrcCode = static_cast<BYTE>(SRM_START_PROG2_NUM - dwCodeNum);
	// 2バイト以上の空きがある場合のみBRA.B srcコードを設定する
	if (byBraSrcCode >= 2) {
		s_bySrmWorkProgramCode[dwCodeNum] = MCU_INST_CODE_BRA_B;
		dwCodeNum++;
		s_bySrmWorkProgramCode[dwCodeNum] = byBraSrcCode;
		dwCodeNum++;
	}
	// 未使用領域にNOPを設定
	while (dwCodeNum < SRM_START_PROG2_NUM) {
		s_bySrmWorkProgramCode[dwCodeNum] = MCU_INST_CODE_NOP;
		dwCodeNum++;
	}
	dwOrgNum = SRM_START_PROG2_NUM;

	// STARTルーチン用プログラム(2)～FINTV退避までをコピー先ワークプログラム配列に設定
	dwLen = SRM_COMMON_PROG2_LENGTH;
	memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);
	dwCodeNum += dwLen;
	dwOrgNum += dwLen;

	dwLen = SRM_SAVE_FPU_LENGTH;
	if (pMcuInfo->eFpuSet == RX_FPU_ON) {	// FPUありの場合
		// FPU退避プログラムをコピー先ワークプログラム配列に設定
		memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);
		dwCodeNum += dwLen;	
	}
	dwOrgNum += dwLen;

	dwLen = SRM_SAVE_CPEN_LENGTH;
	if (pMcuInfo->eCoProSet == COPRO_SET_ON) {	// CPENありの場合
		// CPEN退避プログラムをコピー先ワークプログラム配列に設定
		memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);
		dwCodeNum += dwLen;
	}
	dwOrgNum += dwLen;

	if (pMcuInfo->eAccSet == ACC_SET_ON) {		// ACCありの場合
		// RevRxNo180228-001 Modify Start
		if (pMcuInfo->eCpuIsa == RX_ISA_RXV1) {	// RX V1アーキテクチャの場合
			dwLen = SRM_SAVE_ACC_LENGTH;
			// ACC退避プログラムをコピー先ワークプログラム配列に設定
			memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);
			dwCodeNum += dwLen;
		} else {	// RX V2/RX V3アーキテクチャの場合
			dwLen = SRM_SAVE_ACC_RXV2_LENGTH;
			// ACC0,1退避プログラムをコピー先ワークプログラム配列に設定
			memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgramRxv2[dwRxv2Num], dwLen);
			dwCodeNum += dwLen;
			dwRxv2Num += dwLen;
		}
		// RevRxNo180228-001 Modify End
	}
	dwOrgNum += SRM_SAVE_ACC_LENGTH;	// ACC退避プログラムの次に処理が追加になった場合を考慮し、
										// RXV1,RXV2に関係なくdwOrgNumを加算する

	dwLen = SRM_SAVE_EXTB_RXV2_LENGTH;
	// RevRxNo180228-001 Modify Line
	if (pMcuInfo->eCpuIsa != RX_ISA_RXV1) {	// RX V2/RX V3アーキテクチャの場合
		// EXTB退避プログラムをコピー先ワークプログラム配列に設定
		memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgramRxv2[dwRxv2Num], dwLen);
		dwCodeNum += dwLen;
		dwRxv2Num += dwLen;
	}

	// STOPルーチン用プログラム(2)へのBRAを設定
	byBraSrcCode = static_cast<BYTE>(SRM_STOP_PROG2_NUM - dwCodeNum);
	// 2バイト以上の空きがある場合のみBRA.B srcコードを設定する
	if (byBraSrcCode >= 2) {
		s_bySrmWorkProgramCode[dwCodeNum] = MCU_INST_CODE_BRA_B;
		dwCodeNum++;
		s_bySrmWorkProgramCode[dwCodeNum] = byBraSrcCode;
		dwCodeNum++;
	}
	// 未使用領域をNOPでうめる
	while (dwCodeNum < SRM_STOP_PROG2_NUM) {
		s_bySrmWorkProgramCode[dwCodeNum] = MCU_INST_CODE_NOP;
		dwCodeNum++;
	}
	dwOrgNum = SRM_STOP_PROG2_NUM;

	// STOPルーチン用プログラム(2)をコピー先ワークプログラム配列に設定
	dwLen = SRM_COMMON_PROG3_LENGTH;
	memcpy(&s_bySrmWorkProgramCode[dwCodeNum], &s_bySrmWorkProgram[dwOrgNum], dwLen);

	// RevRxNo130730-004 Modify End

	// Startルーチン先頭アドレス設定
	s_bySrmWorkProgramCode[0x00b] = static_cast<BYTE>(pSrMode->dwmadrBeforeRunFunctionAddr);
	s_bySrmWorkProgramCode[0x00c] = static_cast<BYTE>(pSrMode->dwmadrBeforeRunFunctionAddr >> 8);
	s_bySrmWorkProgramCode[0x00d] = static_cast<BYTE>(pSrMode->dwmadrBeforeRunFunctionAddr >> 16);
	s_bySrmWorkProgramCode[0x00e] = static_cast<BYTE>(pSrMode->dwmadrBeforeRunFunctionAddr >> 24);
	// Stopルーチン先頭アドレス設定
	s_bySrmWorkProgramCode[0x183] = static_cast<BYTE>(pSrMode->dwmadrAfterBreakFunctionAddr);
	s_bySrmWorkProgramCode[0x184] = static_cast<BYTE>(pSrMode->dwmadrAfterBreakFunctionAddr >> 8);
	s_bySrmWorkProgramCode[0x185] = static_cast<BYTE>(pSrMode->dwmadrAfterBreakFunctionAddr >> 16);
	s_bySrmWorkProgramCode[0x186] = static_cast<BYTE>(pSrMode->dwmadrAfterBreakFunctionAddr >> 24);
	// 指定ルーチン用ワークRAM先頭アドレス + 004h 設定
	dwmadrWorkRamRegAddr = pSrMode->dwmadrRamWorkAreaStartAddr + WORKRAMREGADDR;
	s_bySrmWorkProgramCode[0x013] = static_cast<BYTE>(dwmadrWorkRamRegAddr);
	s_bySrmWorkProgramCode[0x014] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 8);
	s_bySrmWorkProgramCode[0x015] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 16);
	s_bySrmWorkProgramCode[0x016] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 24);
	s_bySrmWorkProgramCode[0x0d4] = static_cast<BYTE>(dwmadrWorkRamRegAddr);
	s_bySrmWorkProgramCode[0x0d5] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 8);
	s_bySrmWorkProgramCode[0x0d6] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 16);
	s_bySrmWorkProgramCode[0x0d7] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 24);
	// 指定ルーチン用Stopルーチン完了フラグアドレス(ワークRAM先頭アドレス + 21Ch) 設定
	dwmadrWorkRamStopFlagAddr = pSrMode->dwmadrRamWorkAreaStartAddr + WORKRAMSTOPFLAGADDR;
	s_bySrmWorkProgramCode[0x194] = static_cast<BYTE>(dwmadrWorkRamStopFlagAddr);
	s_bySrmWorkProgramCode[0x195] = static_cast<BYTE>(dwmadrWorkRamStopFlagAddr >> 8);
	s_bySrmWorkProgramCode[0x196] = static_cast<BYTE>(dwmadrWorkRamStopFlagAddr >> 16);
	s_bySrmWorkProgramCode[0x197] = static_cast<BYTE>(dwmadrWorkRamStopFlagAddr >> 24);
	
	return	s_bySrmWorkProgramCode;
}

//=============================================================================
/**
* FFW内で定義している通過イベント用モニタプログラムコードの配列のポインタを返送する。
* @param pSrMode 指定ルーチン実行情報格納構造体のアドレス
* @retval 通過イベント用モニタプログラムコードの配列へのポインタ
*/
//=============================================================================
BYTE* GetMonitorPointProgramCode(const FFW_SRM_DATA* pSrMode)
{
	DWORD	dwmadrWorkRamRegAddr;		// ワークRAM内のレジスタ格納スタートアドレス
	DWORD	dwLen;

	dwLen = MON_POINT_PROGRAM_LENGTH;
	memcpy(&s_byMonitorPointProgramCode[0], &s_byMonitorPointProgram[0], dwLen);

	// 指定ルーチン用ワークRAM先頭アドレス設定
	dwmadrWorkRamRegAddr = pSrMode->dwmadrRamWorkAreaStartAddr;

	s_byMonitorPointProgramCode[WORKRAM_TOP_ADDR_SET] = static_cast<BYTE>(dwmadrWorkRamRegAddr);
	s_byMonitorPointProgramCode[WORKRAM_TOP_ADDR_SET+1] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 8);
	s_byMonitorPointProgramCode[WORKRAM_TOP_ADDR_SET+2] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 16);
	s_byMonitorPointProgramCode[WORKRAM_TOP_ADDR_SET+3] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 24);

	// 指定ルーチン用ワークRAM先頭アドレス + 55h 設定
	dwmadrWorkRamRegAddr = pSrMode->dwmadrRamWorkAreaStartAddr + MON_PROGRAM_STOP_ADDR;

	s_byMonitorPointProgramCode[COMP_FLG_ADDR_SET] = static_cast<BYTE>(dwmadrWorkRamRegAddr);
	s_byMonitorPointProgramCode[COMP_FLG_ADDR_SET+1] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 8);
	s_byMonitorPointProgramCode[COMP_FLG_ADDR_SET+2] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 16);
	s_byMonitorPointProgramCode[COMP_FLG_ADDR_SET+3] = static_cast<BYTE>(dwmadrWorkRamRegAddr >> 24);


	return	s_byMonitorPointProgramCode;
}


//=============================================================================
/**
 * 指定ルーチン実行情報格納構造体変数のポインタを返送する。
 * @param なし
 * @retval 指定ルーチン実行情報格納構造体変数のポインタ
 */
//=============================================================================
FFW_SRM_DATA* GetSrmData(void)
{
	return	&s_SrmData;
}
// RevRxNo120910-006 Append End


