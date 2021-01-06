////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_tim.cpp
 * @brief RX共通実行時間測定関連コマンドのソースファイル
 * @author RSO H.Hashiguchi, H.Akashi, K.Uemori
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/11/07
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo121017-004 	2012/10/18 明石
　VS2008対応エラーコード　WARNING C4554対策
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
*/
#include "ffwrx_tim.h"
#include "dorx_tim.h"
#include "mcurx_tim.h"
#include "mcurx_ev.h"
#include "errchk.h"
#include "prot_common.h"

#include "do_common.h"
#include "domcu_prog.h"
#include "dorx_tra.h"
#include "ffwrx_ev.h"
//RevNo100715-028 Append Line
#include "protmcu_mem.h"

#include "domcu_mem.h"
#include "ffwmcu_mcu.h"

#include "mcurx.h"
#include "ocdspec.h"

static DWORD s_dwPCCSR[RX_CLS3_PPC_CH];			//PPCレジスタ PCCSR0,1に現在設定している値の格納変数

//==============================================================================
/**
 * PPC計測動作モードを設定する。
 * @param dwSetMp PPC設定チャンネル
 * @param pPev PPC計測動作モードを格納するFFWRX_PPCCTRL_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetPPCCTRL(DWORD dwSetMp, const FFWRX_PPCCTRL_DATA* pPev)
{

	FFWERR	ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	DWORD	dwBuff = 0;				//PCCSRレジスタに設定する値
	DWORD	dwPCCSR[RX_CLS3_PPC_CH];	//PCCSRレジスタに設定する値
	DWORD	dwSetPCCSR;	
	BOOL bRet = FALSE;
	BOOL	bSetConnect = FALSE;
	BOOL	bSetPerfFunc[RX_CLS3_PPC_CH];
	DWORD	dwClrMp = 0;
	DWORD	dwMpItemNum;
	WORD	i;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWRX_PPCCTRL_DATA pPevTmp;

	ProtInit();

	// RUN中か確認
	bRet = GetMcuRunState();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	// V.1.02 No.38 PPC Class2 対応 Modify Start
	for(i=0;i<pMcuInfo->wPPC;i++){
		// カウンタch i設定更新
		if((dwSetMp & (0x00000001<<i)) != 0 ) {

			//設定値を入れる
			if(i==0){
				bSetConnect								= pPev->bSetConnect;
			} else {
				bSetConnect								= FALSE;
			}

			//RUN中はbSetPerfFuncの設定値をそのまま入れる。Break中はbSetPerfFunc=FALSEとする
			if(bRet == TRUE) {
				//RUN中
				bSetPerfFunc[i] = pPev->bSetPerfFunc[i];
			} else {
				//Break中
				bSetPerfFunc[i] = FALSE;
			}

			// 計測項目変換
			GetItemNum2Cin(pPev->dwMpItemNum[i], &dwMpItemNum);

			// 内部変数をPPCコントロールレジスタに設定する値に変換
		    pPevTmp.dwMpItemNum[i] = dwMpItemNum;
		    pPevTmp.bSetOnceMeasure[i] = pPev->bSetOnceMeasure[i];
			pPevTmp.bSetConnect = bSetConnect;
			pPevTmp.dwOvf[i] = pPev->dwOvf[i];
			pPevTmp.bSetPerfFunc[i] = pPev->bSetPerfFunc[i];
			ConvPPCCTRL_DATA2PCCSRCls3((BYTE)i,&pPevTmp,&dwPCCSR[i]);

			//PPCコントロールレジスタに設定
			//前回の設定値と異なる場合はクリアを実施　クリアする場合は動作OFFにしておく
			if((s_dwPCCSR[i]&(~PCCSR_SETPERFFUNC)) != (dwPCCSR[i]&(~PCCSR_SETPERFFUNC))){
				dwClrMp |= (0x1<<i);
				ClrPpcCtrlEnable(dwPCCSR[i],&dwBuff);
				ferr = SetPpcCtrl((DWORD)i,dwBuff);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
		}	
	}

	//以前の設定と異なる設定を行った場合は、カウンタ値のクリアを実施
	if(dwClrMp != 0){
		ferr = DO_ClrPPCD(dwClrMp);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	//PCCSRに値を設定
	for(i=0;i<pMcuInfo->wPPC;i++){
		// カウンタch i設定更新
		if((dwSetMp & (0x00000001<<i)) != 0 ) {
			dwSetPCCSR = (dwPCCSR[i] & (~PCCSR_SETPERFFUNC))|bSetPerfFunc[i];
			ferr = SetPpcCtrl((DWORD)i,dwSetPCCSR);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			s_dwPCCSR[i] = dwPCCSR[i];
		}
	}
	// V.1.02 No.38 PPC Class2 対応 Modify End
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;

}
// V.1.02 No.38 PPC Class2 対応 Append Start
//==============================================================================
/**
 * PPC計測動作モードを設定する。
 * @param dwSetMp PPC設定チャンネル
 * @param pPev PPC計測動作モードを格納するFFWRX_PPCCTRL_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetRXCls2PPCCTRL(DWORD dwSetMp, const FFWRX_PPCCTRL_DATA* pPev)
{

	FFWERR	ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	DWORD	dwPCCSR[RX_CLS2_PPC_CH];			//PCCSR0レジスタに設定する値
	BOOL bRet = FALSE;
	BOOL	bSetPerfFunc[RX_CLS2_PPC_CH];
	DWORD	dwSetPCCSR;
	BOOL	dwClrMp = 0;
	DWORD	dwMpItemNum;
	WORD	i;
	DWORD	dwBuff;
	FFWRX_PPCCTRL_DATA pPevTmp;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	ProtInit();

	// RUN中か確認
	bRet = GetMcuRunState();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	for(i=0;i<pMcuInfo->wPPC;i++){
		// カウンタch i設定更新
		if((dwSetMp & (0x00000001<<i)) != 0 ) {

			//RUN中はbSetPerfFuncの設定値をそのまま入れる。Break中はbSetPerfFunc=FALSEとする
			if(bRet == TRUE) {
				//RUN中
				bSetPerfFunc[i] = pPev->bSetPerfFunc[i];
			} else {
				//Break中
				bSetPerfFunc[i] = FALSE;
			}
			// 計測項目変換
			GetItemNum2Cin(pPev->dwMpItemNum[i], &dwMpItemNum);

			// 内部変数をPPCコントロールレジスタに設定する値に変換
		    pPevTmp.bSetOnceMeasure[i] = pPev->bSetOnceMeasure[i];
			pPevTmp.bSetConnect = FALSE;
			pPevTmp.dwOvf[i] = 0;
			pPevTmp.bSetPerfFunc[i] = pPev->bSetPerfFunc[i];
			ConvPPCCTRL_DATA2PCCSRCls2((BYTE)i,&pPevTmp,&dwPCCSR[i]);

			//前回の設定値と異なる場合はクリアを実施　クリアする場合は動作OFFにしておく
			if((s_dwPCCSR[i]&~PCCSR_SETPERFFUNC) != (dwPCCSR[i]&~PCCSR_SETPERFFUNC)){
				dwClrMp |= (0x1<<i);
				ClrPpcCtrlEnable(dwPCCSR[i],&dwBuff);
				ferr = SetPpcCtrl((DWORD)i,dwBuff);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			}

		}
	}

	//以前の設定と異なる設定を行った場合は、カウンタ値のクリアを実施
	if(dwClrMp != 0){
		ferr = DO_ClrPPCD(dwClrMp);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	//PCCSRに値を設定
	for(i=0;i<pMcuInfo->wPPC;i++){
		dwSetPCCSR = (dwPCCSR[i] & ~PCCSR_SETPERFFUNC)|bSetPerfFunc[i];
		ferr = SetPpcCtrl((DWORD)i,dwSetPCCSR);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		s_dwPCCSR[i] = dwPCCSR[i];
	}
	
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	return ferr;

}
// V.1.02 No.38 PPC Class2 対応 Append End
//==============================================================================
/**
 * PPC計測イベントマッチに関する設定を行う。
 * @param  dwSetEvNum カウントイベント番号
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetPPCEVCNT(DWORD dwSetEvNum)
{
	BOOL bRet = FALSE;
	FFWERR						ferr;
	FFWERR						ferrEnd;
	DWORD						dwPpcPcEv = 0;		// PPC回数計測 PC通過イベント格納変数
	DWORD						dwPpcOpcEv = 0;		// PPC回数計測 オペランドアクセスイベント格納変数
	int							i;
	BOOL						bPpcRun;
	DWORD						dwPpcMode;

	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	ProtInit();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	// RUN中か確認
	bRet = GetMcuRunState();

	// カウント測定中確認
	for(i=0;i<pMcuInfo->wPPC;i++){
		ferr = GetPpcEnaInfo(i,&bPpcRun,&dwPpcMode);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// イベント計測中
		if (dwPpcMode  == PPC_CNT_EVT){
			// パフォーマンス計測有効かつRUN中
			if ( (bPpcRun  == TRUE) && (bRet==TRUE) ) {
				ferrEnd = ProtEnd();
				return FFWERR_PPC_RUN;
			}
		}
	}


	// EVENSRレジスタ
	ferr = SetPpcCntEv(dwSetEvNum);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// 実行PC/オペランドアクセスイベント許可の設定
	dwPpcPcEv = dwSetEvNum & 0x000000FF;
	dwPpcOpcEv = (dwSetEvNum & 0x000F0000) >> 16;
	// パフォーマンスで使う実行通過PC/オペランドイベント有効レジスタ設定
	ferr = SetEvPpcCntEvEnable(dwPpcPcEv,dwPpcOpcEv);
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

//==============================================================================
/**
 * PPC計測データを取得する。
 * @param dwGetMp 取得チャンネル
 * @param  pMpd PPC情報格納ポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_GetPPCD(DWORD dwGetMp, FFWRX_MPD_DATA* pMpd)
{
	FFWERR		ferr = FFWERR_OK;
	FFWERR		ferrEnd;
	DWORD		dwPCR = 0;
	WORD		i;
	BOOL		bTimOvf;

	ProtInit();

	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	for(i=0;i<RX_CLS3_PPC_CH;i++){
		// RevNo121017-004	Modify Line
		if(((dwGetMp>>i) & 0x1) == TRUE){
			pMpd->bTimOvf[i] = FALSE;
			pMpd->dwTotal[i] = 0;
		}
	}

	for(i=0;i<(pMcuInfo->wPPC);i++){
		//PPC計測値取得
		if (((dwGetMp >> i) & 0x00000001) == TRUE) {

			// カウンタ値取得
			ferr = GetPpcCnt((DWORD)i, &dwPCR,&bTimOvf);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			pMpd->bTimOvf[i] = bTimOvf;
			pMpd->dwTotal[i] = dwPCR;
		}
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
// V.1.02 No.38 PPC Class2 対応 Modify Start
//==============================================================================
/**
 * PPC計測データをクリアする。
 * @param  dwClrMp クリアチャンネル
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_ClrPPCD(DWORD dwClrMp)
{
	FFWERR						ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	BOOL						bRet = FALSE;
	WORD						i;
	BOOL						bPpcRun;
	DWORD						dwPpcMode;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	ProtInit();
	
	// RUN中か確認
	bRet = GetMcuRunState();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	for(i=0;i<(pMcuInfo->wPPC);i++){
		//PPCレジスタ値クリア
		if ((dwClrMp & (0x00000001 << i)) != 0) {
			
			// カウント測定中確認
			ferr = GetPpcEnaInfo(i,&bPpcRun,&dwPpcMode);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			// パフォーマンス計測有効かつRUN中
			if ( (bPpcRun  == TRUE) && (bRet==TRUE) ) {
				ferrEnd = ProtEnd();
				return FFWERR_PPC_RUN;
			}

			// カウントクリア処理
			ferr = ClrPpcCnt(i);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
// V.1.02 No.38 PPC Class2 対応 Modify End

//==============================================================================
/**
 * パフォーマンス動作モードをOCDへの設定値に変換関数
 * @param  dwMpItem FFW IFで渡されるパフォーマンス動作モード
 * @param  pCinMode OCDに設定するパフォーマンス動作モード
 * @retval なし
 */
//==============================================================================
void GetItemNum2Cin(DWORD dwMpItem, DWORD* pCinMode)
{
	switch(dwMpItem){
	case 1:
		*pCinMode = (DWORD)PPC_CYC_PASS;
		break;
	case 2:
		*pCinMode = (DWORD)PPC_CYC_PASS_SP;
		break;
	case 3:
		*pCinMode = (DWORD)PPC_CYC_EXC_INT;
		break;
	case 4:
		*pCinMode = (DWORD)PPC_CYC_EXC;
		break;
	case 5:
		*pCinMode = (DWORD)PPC_CYC_INT;
		break;
	case 6:
		*pCinMode = (DWORD)PPC_CNT_VALINST;
		break;
	case 7:
		*pCinMode = (DWORD)PPC_CNT_EXC_INT;
		break;
	case 8:
		*pCinMode = (DWORD)PPC_CNT_EXC;
		break;
	case 9:
		*pCinMode = (DWORD)PPC_CNT_INT;
		break;
	case 0xA:
		*pCinMode = (DWORD)PPC_CNT_EVT;
		break;
	default:
		*pCinMode = (DWORD)PPC_NON;
		break;
	}
	return;

}

//==============================================================================
/**
 * 実行前に一度だけ計測の場合、測定値クリア。
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrPPCDPreRun(void)
{
	DWORD dwClrMp = 0;
	FFWERR		ferr = FFWERR_OK;
	WORD		i;
	FFWRX_PPCCTRL_DATA*			pPerf;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pPerf = GetPpcCtrlData();

	// RevRxNo121026-001 Append Start
	// wPPCが"0"の時、何もせずにreturnする。
	if (pMcuInfo->wPPC == RX_PPC_NON) {
		return ferr;
	}
	// RevRxNo121026-001 Append End

	// ユーザが使用している場合はクリアしない
	if (GetPerfUsrCtrl() == FFWRX_EVCOMBI_PPC_USR) {
		return FFWERR_OK;
	}

	// トレース計測中の場合、エラー →停止中しか呼ばれないので削除

	//一度だけ計測のとき、実行前にカウンタクリア
	dwClrMp = 0;
	for(i=0;i<(pMcuInfo->wPPC);i++){
		if(pPerf->bSetOnceMeasure[i] == TRUE) {
			dwClrMp |= (0x00000001<<i);
		}
	}

	if (dwClrMp != 0) {
		ferr = DO_ClrPPCD(dwClrMp);
	}

	return ferr;

}
//=============================================================================
/**
 * 実行時間測定関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdRxData_Tim(void)
{

	// RevNo110224-003 Append Start
	s_dwPCCSR[0] = 0;
	s_dwPCCSR[1] = 0;
	// RevNo110224-003 Append End
	return;

}