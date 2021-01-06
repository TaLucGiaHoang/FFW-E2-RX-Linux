////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcurx_tra.cpp
 * @brief RX共通トレース関連コマンドのソースファイル
 * @author RSD Y.Minami, H.Hashiguchi, S.Ueda, SDS T.Iwata, K.Okita, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/10/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RX220 WS対応 RX210でRX220対応 2012/07/11 橋口
・RevRxNo120606-006 2012/07/12 橋口
　RX Class2トレース RCYする前にRCLしてもサイクル数が0にならない
・RevRxNo121022-001	2012/11/21 SDS 岩田
　EZ-CUBE PROT_MCU_WRITE()分割処理対応
・RevRxNo130830-002 2013/08/30 三宅
　　トレースのBTR1掃出し処理のRX64M用変更
・RevRxNo130905-001	2013/09/05 上田
	トレース設関連OCDレジスタ設定処理改善
・RevRxNo130301-001	2013/09/10 上田
	RX64M対応
・RevRxNo140515-005 2014/08/08 大喜多
	RX71M対応
・RevRxE2LNo141104-001 2014/11/28 上田
	E2 Lite対応
・RevRxNo161003-001	2016/10/05 PA 辻
	ROMデータトレースアクセスタイプ出力対応
*/
#include "mcurx_tra.h"
#include "dorx_tra.h"
#include "mcurx.h"
#include "ocdspec.h"
#include "ffwrx_tra.h"

#include "ffwmcu_mcu.h"
#include "mcu_sfr.h"
#include "protmcu_mem.h"


//==============================================================================
/**
 * トレースOCD関連レジスタを初期化する。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcOCDReg(void)
{
	FFWERR	ferr;
	DWORD	dwTbsr[1];
	BYTE*	pbyTbsr;
	DWORD	dwOcdData[1];
	BYTE*	pbyOcdData;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130301-001 Append Line

	pFwCtrl = GetFwCtrlData();	// RevRxNo130301-001 Append Line
	pbyOcdData = reinterpret_cast<BYTE*>(dwOcdData);

	// トレース開始用実行PC通過イベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVETSPCP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVETSPCP,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース開始用オペランドアクセスイベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVETSOP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVETSOP,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース終了用実行PC通過イベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVETEPCP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVETEPCP,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース終了用オペランドアクセスイベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVETEOP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVETEOP,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース抽出用オペランドアクセスイベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVETXOP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVETXOP,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース用イベントトリガ出力制御
	dwOcdData[0] = INIT_OCD_REG_EV_EVETTRG;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVETTRG,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130905-001 Append Start
	// トレース動作中コントロールレジスタ(トレースバッファへの入力停止)
	dwOcdData[0] = TRSTP_STOP_KEEP;
	ferr = SetMcuOCDReg(OCD_REG_RM_TACR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Append End

	// RevRxNo130905-001 Modify Start
	// トレースコントロールレジスタ(トレースクロック供給)
	dwOcdData[0] = INIT_OCD_REG_RM_TRCR | OCD_REG_RM_TRCR_TRE;
	ferr = SetMcuOCDReg(OCD_REG_RM_TRCR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースコントロールレジスタ(トレースクロックリソース有効)
	dwOcdData[0] = INIT_OCD_REG_RM_TRCR | OCD_REG_RM_TRCR_TRE | OCD_REG_RM_TRCR_TRV;
	ferr = SetMcuOCDReg(OCD_REG_RM_TRCR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	// トレースデータ圧縮方式設定
	dwOcdData[0] = INIT_OCD_REG_RM_TRCS;
	ferr = SetMcuOCDReg(OCD_REG_RM_TRCS,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースデータ命令選別レジスタ
	dwOcdData[0] = INIT_OCD_REG_RM_TISR;
	ferr = SetMcuOCDReg(OCD_REG_RM_TISR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// ROM空間先頭アドレス設定レジスタ
	dwOcdData[0] = INIT_OCD_REG_RM_RSAR;
	ferr = SetMcuOCDReg(OCD_REG_RM_RSAR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// ROM空間最終アドレス設定レジスタ
	dwOcdData[0] = INIT_OCD_REG_RM_REAR;
	ferr = SetMcuOCDReg(OCD_REG_RM_REAR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// ウインドウトレース用先頭アドレス設定レジスタ
	dwOcdData[0] = INIT_OCD_REG_RM_WSAR;
	ferr = SetMcuOCDReg(OCD_REG_RM_WSAR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// ウインドウトレース用最終アドレス設定レジスタ
	dwOcdData[0] = INIT_OCD_REG_RM_WEAR;
	ferr = SetMcuOCDReg(OCD_REG_RM_WEAR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130301-001 Append Start
	// トレースソース選択レジスタ
	if (pFwCtrl->bTraSrcSelEna == TRUE) {
		dwOcdData[0] = INIT_OCD_REG_RM_TSSR;
		ferr = SetMcuOCDReg(OCD_REG_RM_TSSR,eAccessSize, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo130301-001 Append Start

	// トレースバッファ入力/出力ポインタを初期化
	pbyTbsr = reinterpret_cast<BYTE*>(dwTbsr);

	// トレースバッファ設定レジスタ取得
	ferr = GetMcuOCDReg(OCD_REG_RM_TBSR, eAccessSize, OCD_ACCESS_COUNT,pbyTbsr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースバッファ設定レジスタ(いったんRWEビットのみ有効化)
	dwTbsr[0] = (dwTbsr[0] & ~OCD_REG_RM_TBSR_OFF) | OCD_REG_RM_TBSR_RWE;	// RevRxNo130905-001 Modify Line
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR, eAccessSize, pbyTbsr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースバッファ出力ポインタレジスタ
	// RevRxNo130905-001 Modify Start
	// トレースバッファをリードすることでTBPAを設定する
	ferr = GetMcuOCDReg(OCD_BUF_RM_TRC0_H, eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	// トレースバッファ入力ポインタレジスタ
	// RevRxNo130905-001 Modify Start
	// トレースバッファへライトすることでTBIPを設定する
	dwOcdData[0] = 0;
	ferr = SetMcuOCDReg(OCD_BUF_RM_TRC0_H, eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	// トレースバッファ設定レジスタ(全ビット無効化)
	dwOcdData[0] = 0;
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR, eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130905-001 Modify Start
	// トレースコントロールレジスタ(トレースクロック停止)
	dwOcdData[0] = INIT_OCD_REG_RM_TRCR;
	ferr = SetMcuOCDReg(OCD_REG_RM_TRCR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース動作中コントロールレジスタ
	dwOcdData[0] = INIT_OCD_REG_RM_TACR;
	ferr = SetMcuOCDReg(OCD_REG_RM_TACR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	return ferr;
}


//RevNo100715-014 Append Start
//==============================================================================
/**
 * トレースOCDのクリア処理 Class3用
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcOcdMemCls3(void){

	FFWERR						ferr = FFWERR_OK;
	DWORD						dwTbsr[1];			// モニタCPUのバッファ内データアクセス方式用
	BYTE*						pbyTbsr;
	DWORD						dwEscTbsr[1];		// モニタCPUのバッファ内データアクセス方式用(退避用)
	BYTE*						pbyEscTbsr;
	DWORD						dwTbpa[1];	// トレースバッファ出力ポインタ用
	DWORD						dwTbip[1];	// トレースバッファ入力ポインタ用
	DWORD						dwBuff[1];
	BYTE*						pbyOcdData;
	DWORD						dwEscTrcr[1];	// トレースコントロールレジスタ値退避用		// RevRxNo130905-001 Append Line
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;


	// RevRxNo130905-001 Append Start
	// トレースコントロールレジスタ値取得
	pbyOcdData = reinterpret_cast<BYTE*>(dwEscTrcr);
	ferr = GetMcuOCDReg(OCD_REG_RM_TRCR, eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースクロック供給
	dwBuff[0] = dwEscTrcr[0] | OCD_REG_RM_TRCR_TRE;
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = SetMcuOCDReg(OCD_REG_RM_TRCR, eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースクロックリソース有効
	dwBuff[0] = dwBuff[0] | OCD_REG_RM_TRCR_TRV;
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = SetMcuOCDReg(OCD_REG_RM_TRCR, eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Append End

	// モニタCPUのバッファ内データアクセス方式をいったん有効化
	pbyTbsr = reinterpret_cast<BYTE*>(dwTbsr);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,OCD_ACCESS_COUNT, pbyTbsr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	dwEscTbsr[0] = dwTbsr[0];

	// トレースバッファアクセス可能にする バッファにデータを書き込むときは、TBSR.TDOS=0にする必要がある
	dwTbsr[0] = dwTbsr[0] & ~OCD_REG_RM_TBSR_TDOS;
	ferr = OpenTrcBuff(dwTbsr[0]);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxE2LNo141104-001 Delete: 性能向上のためOCDトレースバッファのクリア処理削除

	// トレースバッファ入力ポインタ初期化 入力ポインタを先にする
	// RevRxNo130905-001 Modify Start
	// トレースバッファへライトすることでTBIPを設定する
	dwTbip[0] = OCD_BUF_RM_TRC0_H;
	dwBuff[0] = 0;
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = SetMcuOCDReg(dwTbip[0], eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	// トレースバッファ出力ポインタ初期化
	// RevRxNo130905-001 Modify Start
	// トレースバッファをリードすることでTBPAを設定する
	dwTbpa[0] = OCD_BUF_RM_TRC0_H;
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = GetMcuOCDReg(dwTbpa[0], eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	// モニタCPUのバッファ内データアクセス方式を元に戻す
	pbyEscTbsr = reinterpret_cast<BYTE*>(dwEscTbsr);
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, pbyEscTbsr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130905-001 Append Start
	// トレースコントロールレジスタ値を元に戻す
	pbyOcdData = reinterpret_cast<BYTE*>(dwEscTrcr);
	ferr = SetMcuOCDReg(OCD_REG_RM_TRCR, eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Append End

	return ferr;
}

// RevRxE2LNo141104-001 Delete:OCDトレースメモリクリア関数 clrTrcOcdTrcMemCls3() 削除

//RevNo100715-014 Append Start
//==============================================================================
/**
 * RX600トレースOCDに設定
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetTrcOcdData(void)
{

	FFWERR						ferr;
	DWORD						dwData,dwDataCnv;
	DWORD						dwJoinCtrl = 0;			// トレースコントロールレジスタ(TRCR)のデータ連結用
	DWORD						dwJoinCompress = 0;		// トレースデータ圧縮方式設定(TRCS)のデータ連結用
	DWORD						dwJoinInstruct = 0;		// トレースデータ命令選別レジスタ(TISR)のデータ連結用
	DWORD						dwJoinTrcBuffSet = 0;	// トレースバッファ設定レジスタ(TBSR)のデータ連結用
	DWORD						madrStartAddr[TRC_SET_OCDREGNUM+3];		// RevRxNo130905-001 Modify Line
	BYTE						*pbyWriteData;
	DWORD						dwAddrCnt,dwDataCnt;
	DWORD						dwAreaNum,dwAccessCount;
	BOOL						bSameAccessSize,bSameAccessCount,bSameWriteData;	//すべてLWORDアクセス
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	enum FFWENM_VERIFY_SET eVerify = VERIFY_OFF;
	FFW_VERIFYERR_DATA VerifyErr;
	BYTE						byEndian = ENDIAN_LITTLE;

	FFWRX_RM_DATA*				RmData_RX;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130301-001 Append Line

	RmData_RX = GetRmDataInfo();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130301-001 Append Line

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	pbyWriteData = new BYTE [(TRC_SET_OCDREGNUM+3)*4];	// RevRxNo130905-001 Modify Line
	memset(pbyWriteData, 0, (TRC_SET_OCDREGNUM+3)*4);	// RevRxNo130905-001 Modify Line

	dwAddrCnt = 0;
	dwDataCnt = 0;

	//---------------
	// データ連結処理
	//---------------
	// トレースコントロールレジスタ
	//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
	//  ┌─┬─────┬─┬─┬─┬─┬───────┬─┬─┬─┬─┐
	//  │  │          │              │              │          │  │
	//  └─┴─────┴─┴─┴─┴─┴───────┴─┴─┴─┴─┘
	//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
	//  ┌───┬─┬───┬─┬─┬─┬───┬─┬─┬─┬─┬─┬─┐
	//  │      │  │      │  │  │  │      │  │  │  │  │  │  │
	//  └───┴─┴───┴─┴─┴─┴───┴─┴─┴─┴─┴─┴─┘
	//  b31:TRV    dwSetInitialのb16
	//  b27-24:TBW dwSetInitialのb15-12
	//  b19-17:TRC dwSetInfoのb25-23
	//  b16:TRO    dwSetInitialのb9
	//  b15-14:TSD dwSetInfoのb30-29	// RevRxNo130301-001 Append Line
	//  b13:TSE    dwSetInfoのb22
	//  b10:TDA    dwSetInfoのb21
	//  b9:TMO     dwSetInfoのb20
	//  b8:TRI     dwSetInfoのb19
	//  b5:TOI     ---
	//  b4:TDE     dwSetInfoのb18
	//  b3:TBE     dwSetInfoのb17
	//  b1:TRM     dwSetModeのb0
	//  b0:TRE     dwSetInitialのb8
	dwData = ((RmData_RX->dwSetInitial << 15) & OCD_REG_RM_TRCR_TRV) |	// b31
			 ((RmData_RX->dwSetInitial << 12) & OCD_REG_RM_TRCR_TBW) |	// b27-24
			 ((RmData_RX->dwSetInfo    >> 6)  & OCD_REG_RM_TRCR_TRC) |	// b19-17
			 ((RmData_RX->dwSetInitial << 7)  & OCD_REG_RM_TRCR_TRO) |	// b16
			 ((RmData_RX->dwSetInfo    >> 9)  & OCD_REG_RM_TRCR_TSE) |	// b13
			 ((RmData_RX->dwSetInfo    >> 11) & OCD_REG_RM_TRCR_TDA) |	// b10
			 ((RmData_RX->dwSetInfo    >> 11) & OCD_REG_RM_TRCR_TMO) |	// b9
			 ((RmData_RX->dwSetInfo    >> 11) & OCD_REG_RM_TRCR_TRI) |	// b8
			 ((RmData_RX->dwSetInfo    >> 14) & OCD_REG_RM_TRCR_TDE) |	// b4
			 ((RmData_RX->dwSetInfo    >> 14) & OCD_REG_RM_TRCR_TBE) |	// b3
			 ((RmData_RX->dwSetMode    << 1)  & OCD_REG_RM_TRCR_TRM) |	// b1
			 ((RmData_RX->dwSetInitial >> 8)  & OCD_REG_RM_TRCR_TRE);	// b0

	// RevRxNo130301-001 Append Start
	// トレースタイムスタンプカウンタ分周設定があるMCUの場合
	if (pFwCtrl->bTraTimeDivEna == TRUE) {
		dwData |= ((RmData_RX->dwSetInfo    >> 15) & OCD_REG_RM_TRCR_TSD);	// b15-14
	}
	// RevRxNo130301-001 Append End

	dwJoinCtrl = dwData;

#if 0	// クロック分周比最適化処理を入れると、トレースクロックが30MHzを超えることがあることが発覚したため、
		// トレースクロック分周比最適化処理はコメントアウトする。
		// EXTAL * 逓倍数 が100MHzを超えると、トレースクロックは100MHzを超えたままになる。　
	// RevRxNo140515-005 Append Start
	pMcuInfoData = GetMcuInfoDataRX();
	if( pMcuInfoData->byMcuType != MCU_TYPE_0 ){		// MCU_TYPE_0でない場合のみクロック分周比最適化
		if (pMcuInfoData->fClkIclk <= OCD_REG_RM_TRCR_TRC_60MHz) {			// 60MHz以下の場合
			dwJoinCtrl = dwJoinCtrl & ~OCD_REG_RM_TRCR_TRC;					// トレースクロック分周比を1:1(TRCR.bit19-17を000b)にする
		} else if (pMcuInfoData->fClkIclk <= OCD_REG_RM_TRCR_TRC_120MHz){	// 120MHz以下の場合
			dwJoinCtrl = (dwJoinCtrl & ~OCD_REG_RM_TRCR_TRC) | OCD_REG_RM_TRCR_TRC_2_1;		// トレースクロック分周比を2:1(TRCR.bit19-17を001b)にする
		} else {		// 120MHzを超えている
			dwJoinCtrl = (dwJoinCtrl & ~OCD_REG_RM_TRCR_TRC) | OCD_REG_RM_TRCR_TRC_4_1;		// トレースクロック分周比を4:1(TRCR.bit19-17を010b)にする
		}
	}
	// RevRxNo140515-005 Append End
#endif

	// RevRxNo161003-001 Modify Start +20
	if (pFwCtrl->bRomDataTrcAccessTypeExist) {	// ROMデータトレース アクセスタイプ出力ビットあり
		// トレースデータ圧縮方式設定
		//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
		//  ┌─┬───────────┬─┬─────────────┬─┐
		//  │  │                      │  │                          │  │
		//  └─┴───────────┴─┴─────────────┴─┘
		//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
		//  ┌─────────────────────┬─┬─┬─┬─┬─┐
		//  │                                          │  │  │  │  │  │
		//  └─────────────────────┴─┴─┴─┴─┴─┘
		//  b24:BSS  dwSetInfoのb15
		//  b16:BCE  dwSetInfoのb14
		//  b5:RODTE dwSetInfoのb31
		//  b4:DAE   dwSetInfoのb13
		//  b3:DISE  dwSetInfoのb12
		//  b2:TSCE  dwSetInfoのb11
		//  b1:RODE  dwSetInfoのb10
		//  b0:DCE   dwSetInfoのb9
		dwData = ((RmData_RX->dwSetInfo << 9) & OCD_REG_RM_TRCS_BSS) |		// b24
				 ((RmData_RX->dwSetInfo << 2) & OCD_REG_RM_TRCS_BCE) |		// b16
				 ((RmData_RX->dwSetInfo >> 26) & OCD_REG_RM_TRCS_RODTE) |	// b5
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_DAE) |		// b4
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_DISE) |		// b3
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_TSCE) |		// b2
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_RODE) |		// b1
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_DCE);		// b0
		dwJoinCompress = dwData;
	} else {
		// トレースデータ圧縮方式設定
		//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
		//  ┌─────────────┬─┬─────────────┬─┐
		//  │                          │  │                          │  │
		//  └─────────────┴─┴─────────────┴─┘
		//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
		//  ┌─────────────────────┬─┬─┬─┬─┬─┐
		//  │                                          │  │  │  │  │  │
		//  └─────────────────────┴─┴─┴─┴─┴─┘
		//  b24:BSS dwSetInfoのb15
		//  b16:BCE dwSetInfoのb14
		//  b4:DAE  dwSetInfoのb13
		//  b3:DISE dwSetInfoのb12
		//  b2:TSCE dwSetInfoのb11
		//  b1:RODE dwSetInfoのb10
		//  b0:DCE  dwSetInfoのb9
		dwData = ((RmData_RX->dwSetInfo << 9) & OCD_REG_RM_TRCS_BSS) |	// b24
				 ((RmData_RX->dwSetInfo << 2) & OCD_REG_RM_TRCS_BCE) |	// b16
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_DAE) |	// b4
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_DISE) |	// b3
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_TSCE) |	// b2
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_RODE) |	// b1
				 ((RmData_RX->dwSetInfo >> 9) & OCD_REG_RM_TRCS_DCE);	// b0
		dwJoinCompress = dwData;
	}
	// RevRxNo161003-001 Modify End

	// トレースデータ命令選別レジスタ
	//     7   6   5   4   3   2   1   0
	//  ┌─┬─┬─┬─┬─┬─┬─┬─┐
	//  │  │  │  │  │  │  │  │  │
	//  └─┴─┴─┴─┴─┴─┴─┴─┘
	//  b7:DEXS dwSetInfoのb8
	//  b6:DFPS dwSetInfoのb7
	//  b5:DBTS dwSetInfoのb6
	//  b4:DLOS dwSetInfoのb5
	//  b3:DARS dwSetInfoのb4
	//  b2:DSRS dwSetInfoのb3
	//  b1:DSTS dwSetInfoのb2
	//  b0:DTRS dwSetInfoのb1
	dwData = ((RmData_RX->dwSetInfo >> 1) & OCD_REG_RM_TISR_DEXS) |	// b7
			 ((RmData_RX->dwSetInfo >> 1) & OCD_REG_RM_TISR_DFPS) |	// b6
			 ((RmData_RX->dwSetInfo >> 1) & OCD_REG_RM_TISR_DBTS) |	// b5
			 ((RmData_RX->dwSetInfo >> 1) & OCD_REG_RM_TISR_DLOS) |	// b4
			 ((RmData_RX->dwSetInfo >> 1) & OCD_REG_RM_TISR_DARS) |	// b3
			 ((RmData_RX->dwSetInfo >> 1) & OCD_REG_RM_TISR_DSRS) |	// b2
			 ((RmData_RX->dwSetInfo >> 1) & OCD_REG_RM_TISR_DSTS) |	// b1
			 ((RmData_RX->dwSetInfo >> 1) & OCD_REG_RM_TISR_DTRS);	// b0
	dwJoinInstruct = dwData;

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
	dwData = ((RmData_RX->dwSetInitial << 12) & OCD_REG_RM_TBSR_TRFS) |		// b19-16
		     ((RmData_RX->dwSetInitial << 11) & OCD_REG_RM_TBSR_RWE)  |		// b12
		     ((RmData_RX->dwSetMode    << 7)  & OCD_REG_RM_TBSR_TMWM) |		// b8
		     ((RmData_RX->dwSetInfo    << 5)  & OCD_REG_RM_TBSR_OFF)  |		// b5
		     ((RmData_RX->dwSetMode    >> 0)  & OCD_REG_RM_TBSR_TDOS) |		// b3-2
		     ((RmData_RX->dwSetInitial >> 0)  & OCD_REG_RM_TBSR_TRPE);		// b0
	dwJoinTrcBuffSet = dwData;


	// RevRxNo130905-001 Modify Start
	// トレース動作中コントロールレジスタ(トレースバッファへの入力停止)
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TACR;
	dwData = TRSTP_STOP_KEEP;
	dwDataCnv = dwData;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレースバッファ設定レジスタ
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TBSR;
	dwDataCnv = dwJoinTrcBuffSet;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレースコントロールレジスタ(トレースクロック供給ビットのみ)
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TRCR;
	dwDataCnv = OCD_REG_RM_TRCR_TRE;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレースコントロールレジスタ(トレースクロックリソース有効)
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TRCR;
	dwDataCnv = OCD_REG_RM_TRCR_TRE | OCD_REG_RM_TRCR_TRV;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;
	// RevRxNo130905-001 Modify End

	// トレースデータ圧縮方式設定
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TRCS;
	dwDataCnv = dwJoinCompress;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレースデータ命令選別レジスタ
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TISR;
	dwDataCnv = dwJoinInstruct;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// ROM空間先頭アドレス
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_RSAR;
	dwDataCnv = RmData_RX->dwRomStartAddr;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// ROM空間最終アドレス
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_REAR;
	dwDataCnv = RmData_RX->dwRomEndAddr;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// ウィンドウトレース先頭アドレス
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_WSAR;
	dwDataCnv = RmData_RX->dwWinTrStartAddr;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// ウィンドウトレース最終アドレス
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_WEAR;
	dwDataCnv = RmData_RX->dwWinTrEndAddr;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// RevRxNo130301-001 Append Start
	// トレースソース選択アドレス
	if (pFwCtrl->bTraSrcSelEna == TRUE) {
		madrStartAddr[dwAddrCnt] = OCD_REG_RM_TSSR;
		dwDataCnv = RmData_RX->dwTrcSrcSel;
		memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}
	// RevRxNo130301-001 Append End

	// RevRxNo130905-001 Modify Start
	// トレースコントロールレジスタ
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TRCR;
	memcpy(&pbyWriteData[dwDataCnt], &dwJoinCtrl, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース動作中コントロールレジスタセット
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TACR;
	dwDataCnv = TRSTP_RESTART;
	memcpy(&pbyWriteData[dwDataCnt], &dwDataCnv, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;
	// RevRxNo130905-001 Modify End

	//OCDライト
	dwAreaNum = dwAddrCnt;		//設定イベント個数	// RevRxNo130905-001 Modify Line
	bSameAccessSize = TRUE;		//すべてLWORDアクセス
	eAccessSize = MLWORD_ACCESS;	//LWORD
	bSameAccessCount = TRUE;
	dwAccessCount = 1;
	bSameWriteData = FALSE;

// RevRxNo121022-001 Append Start
	// ヘッダを含めて0x800hバイトを超える、PROT_MCU_WRITE()関数が呼ばれることはないので、BFWMCUCmd_WRITEコマンドの
	// ライトバイト数最大値でアクセスバイト数を分割してPROT_MCU_WRITE()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
	ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, madrStartAddr, bSameAccessSize, &eAccessSize,
					bSameAccessCount, &dwAccessCount, bSameWriteData, pbyWriteData, &VerifyErr,byEndian);

	delete [] pbyWriteData;

	return ferr;
}

//==============================================================================
/**
 * OCD トレース トレースメモリへの入出力有効無効状態確認
 * @param pdwTrstp TRSTP値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetTrcOcdMemInputInfo(DWORD* pdwTrstp){

	FFWERR	ferr = FFWERR_OK;
	BYTE*	pbyTrstp;
	DWORD	dwTrstp[1];
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ

	pbyTrstp = reinterpret_cast<BYTE*>(dwTrstp);
	ferr = GetMcuOCDReg(OCD_REG_RM_TACR,eAccessSize,OCD_ACCESS_COUNT, pbyTrstp);
	*pdwTrstp = dwTrstp[0];

	return ferr;

}
//==============================================================================
/**
 * OCD トレースメモリへのデータ入力再開処理
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetTrcOcdRst(void){

	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ
	DWORD	dwTrstp[1];
	BYTE*	pbyTrstp;
	
	dwTrstp[0] = TRSTP_RESTART;

	pbyTrstp = reinterpret_cast<BYTE*>(dwTrstp);
	ferr = SetMcuOCDReg(OCD_REG_RM_TACR,eAccessSize, pbyTrstp);
	return ferr;

}

//==============================================================================
/**
 * OCD トレースメモリへのデータ入力停止処理
 * @param dwRst      TRSTP_STOP_KEEP:イベント成立保持　TRSTP_STOP_CLR:イベント成立クリア
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetTrcOcdStp(enum FFWENM_TRSTP eTrstp){

	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ
	DWORD	dwData[1];
	BYTE*	pbyRst;

	dwData[0] = (DWORD)eTrstp;
	pbyRst = reinterpret_cast<BYTE*>(dwData);
	ferr = SetMcuOCDReg(OCD_REG_RM_TACR,eAccessSize, pbyRst);
	return ferr;

}

//==============================================================================
/**
 * OCD トレース停止を確認
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR CheckTrcOcdStop(void){

	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ
	DWORD	i;
	DWORD	dwToi,dwTrcEndCnt;
	DWORD	dwBuff[1];
	BYTE*	pbyOcdData;

	// TRCR.TOIビットの状態確認 トレースをはき終わっていたら、中断処理
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = GetMcuOCDReg(OCD_REG_RM_TRCR, eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	dwToi = ((dwBuff[0] & OCD_REG_RM_TRCR_TOI ) >> 5);

	i = 0;
	//トレース終了タイムアウト値取得　戻り値はms
	dwTrcEndCnt = TRC_END_WAIT_TIME;
	// RevRxNo130301-001 Modify Line
	while ( (dwToi == TRUE) && (i < dwTrcEndCnt) ) {
		Sleep (1);
		ferr = GetMcuOCDReg(OCD_REG_RM_TRCR, eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		dwToi = ((dwBuff[0] & OCD_REG_RM_TRCR_TOI ) >> 5);
		//カウントアップ
		i++;
	}
		
	if( dwToi == TRUE ) {
		// バッファ入出力ポインタ追い越し現象発生時は復帰
		ClrTOIBit();
		return FFWERR_RTTSTOP_FAIL;
	}

	return ferr;
}
//==============================================================================
/**
 * トレースモジュール動作クロックチェック
 * 100MHz以下かつMCUクロック(ICK)以上である必要がある
 * @param  dwTrcfrq 補正前TBSR.TRFS設定値
 * @param  pdwTrcfrq 補正後TBSR.TRFS設定格納ポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR CheckTrcClkFrq(DWORD dwTrcfrq, DWORD* pdwTrfs)
{
	DWORD	dwTrfs = 0;
	DWORD	dwIck = 0;
	FFWERR	ferr = FFWERR_OK;
	float fExTAL,fExTALdiv;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報

	// ExTAL 100MHz以下?
	dwTrfs = dwTrcfrq;
	pMcuInfoData = GetMcuInfoDataRX();
	// 12.5MHz以下の場合は、設定値のままで100MHz以下
	fExTAL = pMcuInfoData->fClkExtal;
	fExTALdiv = 100/fExTAL;
	if( (fExTALdiv < 8) && (fExTALdiv >= 4) ) {
		if(dwTrfs == 0){
		// x4に丸め込み
			dwTrfs = 1;
		}
	} else if ((fExTALdiv < 4) && (fExTALdiv >= 2) ) {
		if(dwTrfs < 2){
		// x2に丸め込み
			dwTrfs = 2;
		}
	} else if ((fExTALdiv < 2) && (fExTALdiv >= 1) ) {
		if(dwTrfs < 3){
		// x1に丸め込み
			dwTrfs = 3;
		}
	} else if ((fExTALdiv < 1) && (fExTALdiv >= 0.5) ) {
		if(dwTrfs < 4){
		// x1/2に丸め込み
			dwTrfs = 4;
		}
	} else if (fExTALdiv < 0.5) {
		if(dwTrfs < 5){
		// x1/2に丸め込み
			dwTrfs = 5;
		}
	}

	// ICK以下になっていない?
	ferr = GetMcuSckcrIckVal(&dwIck);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (dwIck < dwTrfs) {
		dwTrfs = dwIck;
	}

	*pdwTrfs = dwTrfs;

	return ferr;
}


//==============================================================================
/**
 * トレース ブレーク時BTR1出力対策
 * BTR1パケットがトレースバッファにたまっているかもしれないので、掃きだし
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetTrcOCDDummyDataBTR1(void)
{

	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTbsr = 0;
	DWORD	dwTbsr_bkup[1];
	BYTE*	pbyTbsr_bkup;
	DWORD	dwTrcBuf[1];
	BYTE*	pbyTrcBuf;
	DWORD	dwBuff[1];
	BYTE*	pbyOcdData;
	DWORD	dwTrcEndCnt;
	DWORD	i;
	DWORD	dwPSW_U;
	BOOL	bOutputEnd;		// RevRxNo130301-001 Append Line
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ

	//一旦トレース出力を止める
	// OCDトレースバッファへのデータ入力 停止
	ferr = SetTrcOcdStp(TRSTP_STOP_KEEP);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130301-001 Modify Start
	//OCDのトレース出力停止を確認
	ferr = CheckTrcOcdStop();
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130301-001 Modify End

	// トレースバッファ設定レジスタ(バックアップデータ取得)
	pbyTbsr_bkup = reinterpret_cast<BYTE*>(dwTbsr_bkup);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,OCD_ACCESS_COUNT, pbyTbsr_bkup);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースバッファアクセス可能にする
	dwTbsr = dwTbsr_bkup[0];
	ferr = OpenTrcBuff(dwTbsr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// MODEをあわせる
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBPA, eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	dwTrcBuf[0] = dwBuff[0];
	// RevRxNo130301-001 Modify Start
	if (dwTrcBuf[0] == OCD_BUF_RM_TRC0_H) {
		dwTrcBuf[0] = OCD_BUF_RM_TRC255_H;
	} else {
		dwTrcBuf[0] = dwTrcBuf[0] - 0x10;
	}
	// RevRxNo130301-001 Modify End

	ferr = GetMcuOCDReg(dwTrcBuf[0], eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	dwTrcBuf[0] = dwBuff[0];
	dwPSW_U = dwTrcBuf[0] & TRCRM_HDR_MODE;

	//BTRダミーデータセット ヘッダ
	dwBuff[0] = DUMMY_BTR_HEADER;
	dwTrcBuf[0] = dwBuff[0] | dwPSW_U;
	pbyTrcBuf = reinterpret_cast<BYTE*>(dwTrcBuf);
	ferr = SetMcuOCDReg(OCD_BUF_RM_TRC0_H,eAccessSize, pbyTrcBuf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	//BTRダミーデータセット 分岐先アドレス
	dwTrcBuf[0] = DUMMY_BTR_DSTADDR;
	ferr = SetMcuOCDReg(OCD_BUF_RM_TRC0_D1,eAccessSize, pbyTrcBuf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	//BTRダミーデータセット 分岐元アドレス
	dwTrcBuf[0] = DUMMY_BTR_SRCADDR;
	ferr = SetMcuOCDReg(OCD_BUF_RM_TRC0_D2,eAccessSize, pbyTrcBuf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130301-001 Delete
	// OCDトレースバッファへのデータ入力再開は、トレースバッファ設定レジスタ値書き戻し後に移動

	// トレース出力バッファポインタ
	// RevRxNo130830-002 Modify Start
	// RX64Mでは、TBPAレジスタに85000hをライトしても、正常にBTR1を掃出さないので、
	// 85000h番地をリードすることで、TBPAを"85000h"にする。
	ferr = GetMcuOCDReg(OCD_BUF_RM_TRC0_H, eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
	// RevRxNo130830-002 Modify End
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース入力バッファポインタ
	// RevRxNo130905-001 Modify Start
	// トレースバッファへライトすることでTBIPを設定する
	dwTrcBuf[0] = 0;
	ferr = SetMcuOCDReg(OCD_BUF_RM_TRC1_H,eAccessSize, pbyTrcBuf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	// トレースバッファ設定レジスタ 値の書き戻し
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, pbyTbsr_bkup);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130301-001 Append Start
	// OCDトレースバッファへのデータ入力 再開
	ferr = SetTrcOcdRst();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// ダミーデータ出力待ち
	bOutputEnd = FALSE;	
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	i = 0;
	dwTrcEndCnt = TRC_END_WAIT_TIME;
	while (bOutputEnd == FALSE) {
		// トレース出力バッファポインタ値リード
		ferr = GetMcuOCDReg(OCD_REG_RM_TBPA, eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (dwBuff[0] == OCD_BUF_RM_TRC1_H) {
			bOutputEnd = TRUE;	// ダミーデータ出力完了
		}
		Sleep(1);
		i++;
		if (i >= dwTrcEndCnt) {	// タイムアウト発生
			break;
		}
	}
	if (bOutputEnd == FALSE) {
		return FFWERR_RTTSTOP_FAIL;
	}


	///// ダミーデータ書き込み後のTBIP, TBPA初期化処理 /////
	// OCDトレースバッファへのデータ入力 停止
	ferr = SetTrcOcdStp(TRSTP_STOP_KEEP);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//OCDのトレース出力停止を確認
	ferr = CheckTrcOcdStop();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースバッファアクセス可能にする
	dwTbsr = dwTbsr_bkup[0];
	ferr = OpenTrcBuff(dwTbsr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース入力バッファポインタ
	// RevRxNo130905-001 Modify Start
	// トレースバッファへライトすることでTBIPを設定する
	dwTrcBuf[0] = 0;
	ferr = SetMcuOCDReg(OCD_BUF_RM_TRC0_H,eAccessSize, pbyTrcBuf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	// トレース出力バッファポインタ
	// RevRxNo130905-001 Modify Start
	// トレースバッファをリードすることでTBPAを設定する
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = GetMcuOCDReg(OCD_BUF_RM_TRC0_H, eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130905-001 Modify End

	// トレースバッファ設定レジスタ 値の書き戻し
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, pbyTbsr_bkup);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// OCDトレースバッファへのデータ入力 再開
	ferr = SetTrcOcdRst();
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130301-001 Append End

	return ferr;
}

//==============================================================================
/**
 * トレースバッファ入出力ポインタ　追い越し現象クリア(RX610 1stCutでのみ発生するはず)
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTOIBit(void)
{
	FFWERR	ferr = FFWERR_OK;	// 戻り値
	DWORD	dwTbpa[1];			// バッファ出力ポインタ値格納変数
	BYTE*	pbyTbpa;
	DWORD	dwTbip[1];			// バッファ入力ポインタ値格納変数
	BYTE*	pbyTbip;
	DWORD	dwTbsr = 0;			// トレースバッファ設定レジスタ格納変数
	DWORD	dwTbsrEsc[1];		// トレースバッファ設定レジスタバックアップ変数
	BYTE*	pbyTbsrEsc;
	DWORD	dwToi = 0;			// TBSR.TOIビット値格納変数
	DWORD	dwTrcEndCnt = 0;	// タイムアウト設定変数
	DWORD	dwBuff[1];			// リードレジスタ値格納変数
	BYTE*	pbyOcdData;
	DWORD	i;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ

	// 出力ポインタ
	pbyTbpa = reinterpret_cast<BYTE*>(dwTbpa);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBPA,eAccessSize,OCD_ACCESS_COUNT, pbyTbpa);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// 入力ポインタ
	pbyTbip = reinterpret_cast<BYTE*>(dwTbip);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBIP,eAccessSize,OCD_ACCESS_COUNT, pbyTbip);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 出力ポインタが入力ポインタを追い越してしまう
	if (dwTbpa[0] != dwTbip[0]) {
		pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
		ferr = GetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		dwTbsrEsc[0] = dwBuff[0];
		dwTbsr = dwBuff[0];
		ferr = OpenTrcBuff(dwTbsr);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		
		// 最初に今の出力バッファポインタを入力バッファポインタ値に合わせる
		ferr = GetMcuOCDReg(OCD_REG_RM_TBIP,eAccessSize,OCD_ACCESS_COUNT, pbyTbip);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		ferr = SetMcuOCDReg(OCD_REG_RM_TBPA,eAccessSize, pbyTbip);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// 入出力ポインタ値を比較
		// 出力ポインタ
		ferr = GetMcuOCDReg(OCD_REG_RM_TBPA,eAccessSize,OCD_ACCESS_COUNT, pbyTbpa);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// 入力ポインタ
		ferr = GetMcuOCDReg(OCD_REG_RM_TBIP,eAccessSize,OCD_ACCESS_COUNT, pbyTbip);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		if (dwTbpa[0] != dwTbip[0]) {
			// 次に今の入力バッファポインタを出力バッファポインタ値に合わせる
			ferr = GetMcuOCDReg(OCD_REG_RM_TBPA,eAccessSize,OCD_ACCESS_COUNT, pbyTbpa);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = SetMcuOCDReg(OCD_REG_RM_TBIP,eAccessSize, pbyTbpa);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

		// トレースバッファ設定レジスタ差し戻し
		pbyTbsrEsc = reinterpret_cast<BYTE*>(dwTbsrEsc);
		ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, pbyTbsrEsc);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// TRCR.TOIビットの状態確認 TOIが0であることを確認
		ferr = GetMcuOCDReg(OCD_REG_RM_TRCR,eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		dwToi = ((dwBuff[0] & OCD_REG_RM_TRCR_TOI ) >> 5);

		i = 0;
		//トレース終了タイムアウト値取得　戻り値はms
		dwTrcEndCnt = TRC_END_WAIT_TIME;
		// RevRxNo130301-001 Modify Line
		while ( (dwToi == TRUE) && (i < dwTrcEndCnt) ) {
			Sleep (1);
			ferr = GetMcuOCDReg(OCD_REG_RM_TRCR,eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			dwToi = ((dwBuff[0] & OCD_REG_RM_TRCR_TOI ) >> 5);
			//カウントアップ
			i++;
		}

		if( dwToi == TRUE ) {
			return FFWERR_RTTSTOP_FAIL;
		}
	}

	return ferr;
}

/////////////////////////////////////////////////////////////////////////////////
///////////   内蔵トレース用
////////////////////////////////////////////////////////////////////////////////
// V.1.02 No.31 Class2 トレース対応 Modify Start
//==============================================================================
/**
 * RX Class2 トレースOCD に設定
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetTrcCls2OcdData(void)
{

	FFWERR						ferr;
	DWORD						dwData;
	DWORD						dwTRCR = 0;			// トレースコントロールレジスタ(TRCR)のデータ連結用
	DWORD						dwTRCS = 0;		// トレースデータ圧縮方式設定(TRCS)のデータ連結用
	DWORD						dwTBSR = 0;	// トレースバッファ設定レジスタ(TBSR)のデータ連結用
	DWORD						madrStartAddr[TRC_SET_CLASS2_OCDREGNUM];
	BYTE						*pbyWriteData;
	BYTE						byEndian;
	DWORD						dwAddrCnt,dwDataCnt;
	DWORD						dwAreaNum,dwLength;
	BOOL						bSameAccessSize,bSameLength,bSameWriteData;	//すべてLWORDアクセス
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	enum FFWENM_VERIFY_SET eVerify = VERIFY_OFF;
	FFW_VERIFYERR_DATA VerifyErr;
	FFWRX_RM_DATA*				RmData_RX;

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	RmData_RX = GetRmDataInfo();
	pbyWriteData = new BYTE [(TRC_SET_CLASS2_OCDREGNUM)*4];
	memset(pbyWriteData, 0, (TRC_SET_CLASS2_OCDREGNUM)*4);

	dwAddrCnt = 0;
	dwDataCnt = 0;

	//---------------
	// データ連結処理
	//---------------
	// TBSR トレースコントロールレジスタ
	//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
	//  ┌─┬─────┬─┬─┬─┬─┬───────┬─┬─┬─┬─┐
	//  │  │          │              │              │          │  │
	//  └─┴─────┴─┴─┴─┴─┴───────┴─┴─┴─┴─┘
	//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
	//  ┌───┬─┬───┬─┬─┬─┬───┬─┬─┬─┬─┬─┬─┐
	//  │      │  │      │  │  │  │      │  │  │  │  │  │  │
	//  └───┴─┴───┴─┴─┴─┴───┴─┴─┴─┴─┴─┴─┘
	//  b31:TRV    dwSetInitialのb16
	//  b11-12:TPE dwSetInfoのb26,27
	//  b4:TDE     dwSetInfoのb18
	//  b3:TBE     dwSetInfoのb17
	dwTRCR = ((RmData_RX->dwSetInitial << 15) & OCD_REG_RM_TRCR_TRV) |	// b31
			 ((RmData_RX->dwSetInfo    >> 15) & OCD_REG_RM_TRCR_TPE) |	// b11-12
			 ((RmData_RX->dwSetInfo    >> 14) & OCD_REG_RM_TRCR_TDE) |	// b4
			 ((RmData_RX->dwSetInfo    >> 14) & OCD_REG_RM_TRCR_TBE);	// b3

	// トレースデータ圧縮方式設定
	//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
	//  ┌─────────────┬─┬─────────────┬─┐
	//  │                          │  │                          │  │
	//  └─────────────┴─┴─────────────┴─┘
	//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
	//  ┌─────────────────────┬─┬─┬─┬─┬─┐
	//  │                                          │  │  │  │  │  │
	//  └─────────────────────┴─┴─┴─┴─┴─┘
	//  b20:BTA dwSetInfoのb28
	dwTRCS = ((RmData_RX->dwSetInfo >>8 ) & OCD_REG_RM_TRCS_BTA); 	// b20

	// TBSR トレースバッファ設定レジスタ
	//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
	//  ┌───────────────────────┬─┬─┬─┬─┐
	//  │                                              │              │
	//  └───────────────────────┴─┴─┴─┴─┘
	//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
	//  ┌─────┬─┬─────┬─┬───┬─┬─┬─┬─┬─┬─┐
	//  │          │  │          │  │      │  │  │      │  │  │
	//  └─────┴─┴─────┴─┴───┴─┴─┴─┴─┴─┴─┘
	//  b12:RWE     dwSetInitialのb1
	//  b8:TMWM     dwSetModeのb1
	//  b3-2:TDOS   dwSetModeのb3-2
	dwTBSR = ((RmData_RX->dwSetInitial << 11) & OCD_REG_RM_TBSR_RWE)  |		// b12
		     ((RmData_RX->dwSetMode    << 7)  & OCD_REG_RM_TBSR_TMWM) |		// b8
		     ((RmData_RX->dwSetMode    >> 0)  & OCD_REG_RM_TBSR_TDOS);		// b3-2

	// TRCR トレースコントロールレジスタ
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TRCR;
	memcpy(&pbyWriteData[dwDataCnt], &dwTRCR, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース動作中コントロールレジスタセット
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TACR;
	dwData = TRSTP_RESTART;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレースデータ圧縮方式設定
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TRCS;
	memcpy(&pbyWriteData[dwDataCnt], &dwTRCS, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレースバッファ設定レジスタ
	madrStartAddr[dwAddrCnt] = OCD_REG_RM_TBSR;
	memcpy(&pbyWriteData[dwDataCnt], &dwTBSR, sizeof(DWORD));
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	//OCDライト
	dwAreaNum = TRC_SET_CLASS2_OCDREGNUM;	//設定イベント個数
	bSameAccessSize = TRUE;			//すべてLWORDアクセス
	eAccessSize = MLWORD_ACCESS;	//LWORD
	bSameLength = TRUE;
	dwLength = 1;
	bSameWriteData = FALSE;
	byEndian = ENDIAN_LITTLE;

// RevRxNo121022-001 Append Start
	// ヘッダを含めて0x800hバイトを超える、PROT_MCU_WRITE()関数が呼ばれることはないので、BFWMCUCmd_WRITEコマンドの
	// ライトバイト数最大値でアクセスバイト数を分割してPROT_MCU_WRITE()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
	ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, madrStartAddr, bSameAccessSize, &eAccessSize,
					bSameLength, &dwLength, bSameWriteData, pbyWriteData, &VerifyErr,byEndian);

	delete [] pbyWriteData;

	return ferr;
}
// V.1.02 No.31 Class2 トレース対応 Modify End

//==============================================================================
/**
 * トレースOCDのクリア処理 Class3用
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcOcdMemCls2(void){

	FFWERR						ferr = FFWERR_OK;
	DWORD						dwTbsr[1];			// モニタCPUのバッファ内データアクセス方式用
	BYTE*						pbyTbsr;
	DWORD						dwEscTbsr[1];		// モニタCPUのバッファ内データアクセス方式用(退避用)
	BYTE*						pbyEscTbsr;
	DWORD						dwTbip[1];	// トレースバッファ入力ポインタ用
	BYTE*						pbyTbip;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;

	// トレースポインタ値を0x00085000に戻す
	// TBSR トレースバッファ設定レジスタ値リード
	pbyEscTbsr = reinterpret_cast<BYTE*>(dwEscTbsr);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,OCD_ACCESS_COUNT,pbyEscTbsr);
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// TBSR.OFF = 0 TBSR.RWE = 1 にする
	// RevRxNo120606-006 Modify Start
	dwEscTbsr[0] = dwEscTbsr[0] & ~OCD_REG_RM_TBSR_OFF;
	dwTbsr[0] = dwEscTbsr[0] | OCD_REG_RM_TBSR_RWE;
	// RevRxNo120606-006 Modify End
	pbyTbsr = reinterpret_cast<BYTE*>(dwTbsr);
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,pbyTbsr);
	if(ferr != FFWERR_OK){
		return ferr;
	}
	
	//TBIPに0x00085000をライト
	dwTbip[0] = OCD_BUF_RM_TRC0_H;
	pbyTbip = reinterpret_cast<BYTE*>(dwTbip);
	ferr = SetMcuOCDReg(OCD_REG_RM_TBIP,eAccessSize,pbyTbip);
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// TBSRを元に戻す
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,pbyEscTbsr);

	return ferr;
}

//==============================================================================
/**
 * 内蔵トレースバッファに格納された全トレースデータを取得
 * @param  pdwRd      トレースデータ格納バッファポインタ pdwRd[0]に0サイクル目を入れる
 * @param  dwRdCount  トレースメモリをLWORDでリードする回数
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetAllTrcBuffDataTrbf(DWORD* pdwRd,DWORD dwRdCount)
{
	FFWERR	ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;
	DWORD						dwTBSR_bkup[1];
	BYTE*						pbyTBSR_bkup;
	DWORD						dwTBSR[1];
	BYTE*						pbyTBSR;
	DWORD						dwTBIP[1];
	BYTE*						pbyTBIP;
	DWORD*						pdwBuff;
	BYTE*						pbyBuff;
	DWORD						dwStartAddr,dwStartBlock;
	DWORD						dwReadCnt1,dwReadCnt2;
	DWORD						i;
	BOOL						bOff;
	//RX220WS対応 Appned Start
	DWORD						dwData;
	DWORD						dwRdCountTmp;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	pMcuInfoData = GetMcuInfoDataRX();
	//RX220WS対応 Appned End
	// RevRxNo130905-001 Append Start
	DWORD						dwOcdData[1];
	BYTE*						pbyOcdData;
	FFWMCU_FWCTRL_DATA*			pFwCtrl;
	// RevRxNo130905-001 Append End

	pFwCtrl = GetFwCtrlData();	// RevRxNo130905-001 Append Line

	// トレースバッファアクセス有効
	pbyTBSR_bkup = reinterpret_cast<BYTE*>(dwTBSR_bkup);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,OCD_ACCESS_COUNT,pbyTBSR_bkup);
	if(ferr != FFWERR_OK){
		return ferr;
	}
	// OFF = 0, RWE = 1
	dwTBSR_bkup[0] &= ~OCD_REG_RM_TBSR_OFF;
	dwTBSR[0] = dwTBSR_bkup[0] | OCD_REG_RM_TBSR_RWE;
	pbyTBSR = reinterpret_cast<BYTE*>(dwTBSR);
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,pbyTBSR);
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// トレースバッファポインタ取得
	pbyTBIP = reinterpret_cast<BYTE*>(dwTBIP);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBIP,eAccessSize,OCD_ACCESS_COUNT,pbyTBIP);
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// 内蔵トレースデータ取得
	//RX220WS対応 Modify Start
	if((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_WS) == SPC_TRC_RX220_BUFF64_32CYC){
		//RX220 64段バッファの場合は64サイクル分データを取得
		dwRdCountTmp = CLS2_TRC_BUF_MAX/4;
	} else {
		dwRdCountTmp = dwRdCount;
	}
	//RX220WS対応 Modify End
	pdwBuff = new DWORD [dwRdCountTmp];
	memset(pdwBuff,0,dwRdCountTmp);
	pbyBuff = new BYTE [dwRdCountTmp*4];
	memset(pbyBuff,0,dwRdCountTmp*4);

	// トレースバッファデータ取得
	ferr = GetMcuOCDReg(OCD_BUF_RM_TRC0_H,eAccessSize,dwRdCountTmp,pbyBuff);
	if(ferr != FFWERR_OK){
		delete [] pdwBuff;
		delete [] pbyBuff;
		return ferr;
	}
	memcpy(pdwBuff,pbyBuff,dwRdCount*4);

	// RevRxNo130905-001 Append Start
	// トレースバッファ入力ポインタ初期化
	// トレースバッファへライトすることでTBIPを設定する
	dwOcdData[0] = 0;
	pbyOcdData = reinterpret_cast<BYTE*>(dwOcdData);
	ferr = SetMcuOCDReg(OCD_BUF_RM_TRC0_H, eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		// トレースバッファ出力ポインタレジスタ
		// トレースバッファをリードすることでTBPAを設定する
		pbyOcdData = reinterpret_cast<BYTE*>(dwOcdData);
		ferr = GetMcuOCDReg(OCD_BUF_RM_TRC0_H, eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo130905-001 Append End

	// トレースメモリバッファアクセス禁止
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,pbyTBSR_bkup);
	if(ferr != FFWERR_OK){
		delete [] pdwBuff;
		delete [] pbyBuff;
		return ferr;
	}

	bOff = GetTrcOffSwFlg();
	// メモリデータ並び替え
	//RX220WS対応 Modify Start
	if((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_WS) == (SPC_TRC_RX220_BUFF64_32CYC)){
		//RX220 64段バッファの場合
		if((GetTrcOffSwFlg()==TRUE) && (GetTrcFreeMode() == TRUE)){
			// フリーとレースでオーバーフローしている
			//スタート位置を見つける
			//バッファの半分より大きいところにいるときは32サイクル分戻したところがスタートになる
			if(dwTBIP[0] > OCD_BUF_RM_TRC32_H_CLS2){
				dwStartAddr = dwTBIP[0]-0x100;
				dwStartBlock = (dwStartAddr & 0x00000FFF) >> 2;
				dwReadCnt1 = (GetTrcCycMaxCls2()*2);
			} else {
				dwData = dwTBIP[0] & 0x00000FFF;
				dwStartAddr = OCD_BUF_RM_TRC33_H_CLS2 + dwData;
				dwStartBlock = (dwStartAddr & 0x00000FFF) >> 2;
				dwReadCnt1 = ((CLS2_TRC_BUF_MAX/4)-dwStartBlock);
			}

			for(i=0;i<dwReadCnt1;i++){
				pdwRd[i] = pdwBuff[i+dwStartBlock]; 
			}

			// メモリ最初から最後のサイクルまで
			//　サイクル数の計算
			dwReadCnt2 = (GetTrcCycMaxCls2()*2) - dwReadCnt1;
			for(i=0;i<dwReadCnt2;i++){
				pdwRd[i+dwReadCnt1] = pdwBuff[i]; 
			}
		} else if((GetTrcOffSwFlg()==FALSE) && (GetTrcFreeMode() == TRUE)){
			// オーバーフローしていない
			//スタート位置を見つける
			if(dwTBIP[0] > OCD_BUF_RM_TRC33_H_CLS2){
				//バッファの半分より大きいところにいるときは32サイクル分戻したところがスタートになる
				dwData = dwTBIP[0] - OCD_BUF_RM_TRC33_H_CLS2;
				dwStartAddr = (OCD_BUF_RM_TRC0_H + dwData);
				dwStartBlock = (dwStartAddr & 0x00000FFF) >> 2;
				dwReadCnt1 = ((OCD_BUF_RM_TRC33_H_CLS2 & 0x00000FFF)>>2);
			} else {
				//バッファの半分以下の場合は0からスタート
				dwStartAddr = OCD_BUF_RM_TRC0_H;
				dwStartBlock = 0;
    			dwReadCnt1 = (dwTBIP[0] & 0x00000FFF)>>2;
			}
			for(i=0;i<dwReadCnt1;i++){
				pdwRd[i] = pdwBuff[i+dwStartBlock]; 
			}

		} else if((GetTrcOffSwFlg()==TRUE) && (GetTrcFreeMode() != TRUE)){
			// フルトレースの場合でメモリ満杯の場合
			dwStartAddr = OCD_BUF_RM_TRC0_H;
			dwStartBlock = 0;
			dwReadCnt1 = (GetTrcCycMaxCls2()*2);
			for(i=0;i<dwReadCnt1;i++){
				pdwRd[i] = pdwBuff[i]; 
			}

		} else {
			// フルトレースの場合メモリラップアラウンドしていない
			//スタートサイクルは0サイクル目
			dwStartAddr = OCD_BUF_RM_TRC0_H;
			dwStartBlock = 0;
			if(dwTBIP[0] > OCD_BUF_RM_TRC32_H_CLS2){
				//バッファの半分より大きいところにいるときは32サイクル分取得
				dwReadCnt1 = (GetTrcCycMaxCls2()*2);
			} else {
				//バッファの半分以下の場合は、ある分だけ取得
				dwReadCnt1 = (dwTBIP[0] & 0x00000FFF)>>2;
			}
			for(i=0;i<dwReadCnt1;i++){
				pdwRd[i] = pdwBuff[i]; 
			}
		}
	} else {
		//通常
		if(bOff==TRUE) {
		// オーバーフローしている
			// サイクル0からメモリ最後まで
			dwStartAddr = dwTBIP[0];
			dwStartBlock = (dwStartAddr & 0x00000FFF) >> 2;
			dwReadCnt1 = (dwRdCount-dwStartBlock);
			for(i=0;i<dwReadCnt1;i++){
				pdwRd[i] = pdwBuff[i+dwStartBlock]; 
			}
			// メモリ最初から最後のサイクルまで
			//　サイクル数の計算
			if(dwStartBlock == 0){
				dwReadCnt2 = 0;
			} else {
				dwReadCnt2 = ((dwStartAddr & 0x00000FFF))>>2;
			}
			for(i=0;i<dwReadCnt2;i++){
				pdwRd[i+dwReadCnt1] = pdwBuff[i]; 
			}
		} else {
			// メモリラップアラウンドしていない
			dwStartAddr = OCD_BUF_RM_TRC0_H;
			dwStartBlock = 0;
			dwReadCnt1 = (dwTBIP[0] & 0x00000FFF)>>2;
			for(i=0;i<dwReadCnt1;i++){
				pdwRd[i] = pdwBuff[i]; 
			}
		}
	}
	//RX220WS対応 Modify End

	// メモリ開放
	delete [] pdwBuff;
	delete [] pbyBuff;

	return FFWERR_OK;

}

//==============================================================================
/**
 * 内蔵トレースバッファにアクセスするための設定変更
 * @param  dwTbsr  Tbsr値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR OpenTrcBuff(DWORD dwTbsr)
{

	FFWERR	ferr;
	DWORD	dwIck;
	DWORD	dwTbsrSet[1];
	BYTE*	pbyTbsrSet;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ

	// システムクロック ICLK値取得 ExTALに対しCPUクロックが何逓倍かチェック
	pMcuInfoData = GetMcuInfoDataRX();
	// FPGAのときは、クロックコントロールレジスタを触ると動作異常になるため、定数をいれる
	ferr = GetMcuSckcrIckVal(&dwIck);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースバッファ設定レジスタ TRFS(ExTAL x1) :0011 RWE :1
	dwTbsrSet[0] = dwTbsr & ~(OCD_REG_RM_TBSR_TRFS | OCD_REG_RM_TBSR_RWE);
	dwTbsrSet[0] = dwTbsrSet[0] | OCD_REG_RM_TBSR_RWE | (dwIck << 16);
	pbyTbsrSet = reinterpret_cast<BYTE*>(dwTbsrSet);
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, pbyTbsrSet);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;

}
//=============================================================================
/**
 * 内蔵トレースオーバフロービット取得
 * @param pbOff TRUE:OCDメモリのオーバーフロー発生
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetTrcOcdOffFlg(BOOL* pbOff){
	
	FFWERR  ferr;
	DWORD	dwTBSR[1];
	BYTE*	pbyTBSR;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ

	pbyTBSR = reinterpret_cast<BYTE*>(dwTBSR);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize,OCD_ACCESS_COUNT, pbyTBSR);

	*pbOff = (dwTBSR[0] & OCD_REG_RM_TBSR_OFF) >> 5;

	return ferr;
}

//=============================================================================
/**
 * OCD トレース入力ポインタ値取得 Class2用
 * @param dwTbip:トレースメモリ入力ポインタ値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetTrcOcdMemInptPntCls2(DWORD* pdwTbip){

	FFWERR  ferr;
	DWORD	dwTBSR[1];
	DWORD	dwTbsrEsc;
	BYTE*	pbyTBSR;
	DWORD	dwBuff[1];
	BYTE*	pbyOcdData;
	DWORD	dwTbip[1];
	BYTE*	pbyTbip;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;	//OCDレジスタアクセスサイズ

	pbyTBSR = reinterpret_cast<BYTE*>(dwTBSR);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, OCD_ACCESS_COUNT,pbyTBSR);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	dwTbsrEsc = dwTBSR[0];

	// TBSR.RWE=1 TBSR.OFF=0
	dwTBSR[0] &= ~OCD_REG_RM_TBSR_OFF;
	dwBuff[0] = dwTBSR[0] | OCD_REG_RM_TBSR_RWE;
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースバッファ入力ポインタアドレス値取得
	pbyTbip = reinterpret_cast<BYTE*>(dwTbip);
	ferr = GetMcuOCDReg(OCD_REG_RM_TBIP,eAccessSize,OCD_ACCESS_COUNT, pbyTbip);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	*pdwTbip = dwTbip[0];

	// トレースバッファレジスタ値書き戻し TBSR
	dwTBSR[0] = dwTbsrEsc;
	ferr = SetMcuOCDReg(OCD_REG_RM_TBSR,eAccessSize, pbyTBSR);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;

}
//=============================================================================
/**
 * トレース関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuRxData_Tra(void)
{

	return;

}
