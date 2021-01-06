////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcurx_ev.cpp
 * @brief RX共通イベント関連コマンドのソースファイル
 * @author RSO Y.Minami, H.Hashiguchi, H.Akashi, K.Uemori, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/04/12
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・Rev121017-004		2012/10/18 EM2 明石
	VS2008対応エラーコード WARNING C4554対策
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121022-001	2012/11/21 SDS 岩田
	EZ-CUBE PROT_MCU_WRITE()分割処理対応
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
*/
#include "mcurx_tim.h"
#include "mcurx.h"
#include "dorx_tim.h"
#include "ffwrx_tim.h"
#include "ffwrx_ev.h"
#include "ocdspec.h"
#include "ffwmcu_mcu.h"
#include "protmcu_mem.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line

//==============================================================================
/**
 * 時間測定関連レジスタを初期化する。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrPerfReg(void)
{
	FFWERR ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;		// 設定する領域へのアクセスサイズ
	DWORD		dwOcdData[1];
	BYTE*		pbyOcdData;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// RevRxNo121026-001 Append Start
	// wPPCが"0"の時、何もせずにreturnする。
	if (pMcuInfo->wPPC == RX_PPC_NON) {
		return ferr;
	}
	// RevRxNo121026-001 Append End

	pbyOcdData = reinterpret_cast<BYTE*>(dwOcdData);

	// パフォーマンス計測0開始用実行PC通過イベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVEPSPCP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPSPCP0,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// パフォーマンス計測0開始用オペランドアクセスイベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVEPSOP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPSOP0,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// パフォーマンス計測0終了用実行PC通過イベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVEPEPCP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPEPCP0,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// パフォーマンス計測0終了用オペランドアクセスイベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVEPEOP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPEOP0,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// パフォーマンス計測用イベントトリガ出力制御
	dwOcdData[0] = INIT_EV_PPC_TRG;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPTRG,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// V.1.02 No.38 Modify Start
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		// パフォーマンス計測1開始用実行PC通過イベント選択
		dwOcdData[0] = INIT_OCD_REG_EV_EVEPSPCP;
		ferr = SetMcuOCDReg(OCD_REG_EV_EVEPSPCP1,eAccessSize,pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// パフォーマンス計測1開始用オペランドアクセスイベント選択
		dwOcdData[0] = INIT_OCD_REG_EV_EVEPSOP;
		ferr = SetMcuOCDReg(OCD_REG_EV_EVEPSOP1,eAccessSize,pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// パフォーマンス計測1終了用実行PC通過イベント選択
		dwOcdData[0] = INIT_OCD_REG_EV_EVEPEPCP;
		ferr = SetMcuOCDReg(OCD_REG_EV_EVEPEPCP1,eAccessSize,pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// パフォーマンス計測1終了用オペランドアクセスイベント選択
		dwOcdData[0] = INIT_OCD_REG_EV_EVEPEOP;
		ferr = SetMcuOCDReg(OCD_REG_EV_EVEPEOP1 ,eAccessSize,pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

	}
	// V.1.02 No.38 Modify End

	return ferr;
}
//RevNo100715-028 Append Start
//==============================================================================
/**
 * パフォーマンスの再設定を行う。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetPpcRegData(void)
{
	FFWERR						ferr = FFWERR_OK;
	DWORD						dwData,dwBuff;
	DWORD						dwMpItemNum;
	DWORD						dwAddrCnt,dwDataCnt;
	DWORD						madrStartAddr[PPC_SET_REGNUM];
	DWORD						madrPCCSR[RX_CLS3_PPC_CH] = {OCD_REG_PPC_PCCSR0,OCD_REG_PPC_PCCSR1};
	BYTE						*pbyWriteData;
	DWORD						dwAreaNum,dwAccessCount;
	BOOL						bSameAccessSize,bSameAccessCount,bSameWriteData;	//すべてLWORDアクセス
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	enum FFWENM_VERIFY_SET eVerify = VERIFY_OFF;
	FFW_VERIFYERR_DATA VerifyErr;
	DWORD						dwPpcUsrCtrl;
	DWORD						dwRegCnt=0;
	BYTE						byEndian;
	WORD						i;
	FFWRX_PPCCTRL_DATA*			ePev;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// RevRxNo121026-001 Append Start
	// wPPCが"0"の時、何もせずにreturnする。
	if (pMcuInfo->wPPC == RX_PPC_NON) {
		return ferr;
	}
	// RevRxNo121026-001 Append End

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	pbyWriteData = new BYTE [PPC_SET_REGNUM*4];
	memset(pbyWriteData, 0, PPC_SET_REGNUM*4);

	dwAddrCnt = 0;
	dwDataCnt = 0;

	// パフォーマンス計測レジスタアクセスコントロールレジスタ
	dwPpcUsrCtrl = GetPerfUsrCtrl();
	if (dwPpcUsrCtrl == FFWRX_EVCOMBI_PPC_EML) {
		dwBuff = ((DWORD)dwPpcUsrCtrl << 30);
		dwBuff |= ((DWORD)(0x00000002 | dwPpcUsrCtrl));
	} else {
		dwBuff = ((DWORD)dwPpcUsrCtrl << 30);
		dwBuff |= (DWORD)dwPpcUsrCtrl;
	}

	madrStartAddr[dwAddrCnt] = OCD_REG_PPC_PAC;
	dwData = dwBuff;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;

	// パフォーマンス計測条件設定レジスタ0
	// 計測項目変換
	for(i=0;i<pMcuInfo->wPPC;i++){
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
		ePev = GetPpcCtrlData();
		GetItemNum2Cin(ePev->dwMpItemNum[i], &dwMpItemNum);
		dwBuff = ((dwMpItemNum							<< 16) & OCD_REG_PPC_PCCSR_CIN) |		// b26-16
		     ((ePev->bSetOnceMeasure[i]					<< 4) & OCD_REG_PPC_PCCSR_FOC) |		// b4
		     ((ePev->bSetConnect						<< 2) & OCD_REG_PPC_PCCSR_CCE) |		// b2
			 ((ePev->dwOvf[i]					        << 1) & OCD_REG_PPC_PCCSR_COS) |		// b1
	 		// RevNo110506-002 Modify Line
			// この関数を呼び出すのはブレーク中(REST,SREST後)なので、パフォーマンスは無効にしておく
			 ((ePev->bSetPerfFunc[i]	        ) & 0x00000000);		// b0 パフォーマンスは無効
		dwData = dwBuff;
		madrStartAddr[dwAddrCnt] = madrPCCSR[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
	}

	//Class2にはEVENSRがないので設定しない
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
		// カウントイベント設定レジスタ
		madrStartAddr[dwAddrCnt] = OCD_REG_PPC_EVENSR;
		dwData = GetPpcCntEvNum();
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
	}

	dwAreaNum = dwRegCnt;	//設定イベント個数
	bSameAccessSize = TRUE;		//すべてLWORDアクセス
	eAccessSize = MLWORD_ACCESS;	//LWORD
	bSameAccessCount = TRUE;
	dwAccessCount = 1;
	bSameWriteData = FALSE;
	byEndian = ENDIAN_LITTLE;

// RevRxNo121022-001 Append Start
	// ヘッダを含めて0x800hバイトを超える、PROT_MCU_WRITE()関数が呼ばれることはないので、BFWMCUCmd_WRITEコマンドの
	// ライトバイト数最大値でアクセスバイト数を分割してPROT_MCU_WRITE()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
	ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, madrStartAddr, bSameAccessSize, &eAccessSize,
					bSameAccessCount, &dwAccessCount, bSameWriteData, pbyWriteData, &VerifyErr,byEndian);

	delete [] pbyWriteData;

	// RevNo110302-01 Modify Line
	return ferr;
}
//RevNo100715-028 Append End

//==============================================================================
/**
 * パフォーマンスアクセス制限の設定をする。
 * @param ePpcUsrCtrl トリガの種類  FFWRX_EVCOMBI_PPC_EML:エミュレータからのみアクセス可能　FFWRX_EVCOMBI_PPC_USR:アクセス制限なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetPerfAccCtrl(enum FFWRX_EVCOMBI_PPC_USR_CTRL ePpcUsrCtrl)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwJoin[1];			// データ連結用
	BYTE*						pbyJoin;

	madrWriteAddr = OCD_REG_PPC_PAC;


	// データ連結処理
	//    31  30     ～      1   0
	//  ┌─┬─┬────┬───┐
	//  │  │  │        │      │
	//  └─┴─┴────┴───┘
	//  b30:カウントイベント番号設定レジスタへのアクセスコントロール
	//  b1,0:パフォーマンス計測条件設定レジスタ、パフォーマンス計測レジスタへのアクセスコントロール
	if (ePpcUsrCtrl == FFWRX_EVCOMBI_PPC_EML) {
		dwJoin[0] = OCD_REG_PPC_PAC_ESA0_EML | OCD_REG_PPC_PAC_UA_EML;		// 以前の式がわかりづらいので修正 RevNo120131-001	Modify Line
	} else {
		dwJoin[0] = OCD_REG_PPC_PAC_ESA0_USR | OCD_REG_PPC_PAC_UA_USR;		// 以前の式がわかりづらいので修正 RevNo120131-001	Modify Line
	}
	eAccessSize = MLWORD_ACCESS;
	pbyJoin = reinterpret_cast<BYTE*>(dwJoin);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyJoin);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}

//==============================================================================
/**
 * パフォーマンス設定を行う
 * @param dwPpcCh パフォーマンスチャンネル
 * @param dwPCCSR パフォーマンス設定値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetPpcCtrl(DWORD dwPpcCh,DWORD dwPCCSR){
	
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR	madrPCCSR[RX_CLS3_PPC_CH] = {OCD_REG_PPC_PCCSR0,OCD_REG_PPC_PCCSR1};
	BYTE*	pbyPCCSR;
	DWORD	dwData[1];

	eAccessSize = MLWORD_ACCESS;
	dwData[0] = dwPCCSR;
	pbyPCCSR = reinterpret_cast<BYTE*>(dwData);
	ferr = SetMcuOCDReg(madrPCCSR[dwPpcCh],eAccessSize,pbyPCCSR);

	return ferr;
}
//==============================================================================
/**
 * パフォーマンスを有効にする
 * @param bEnable TRUE 有効  FALSE 無効
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetPPCEnable(BOOL bEnable)
{
	FFWERR ferr = FFWERR_OK;
	DWORD dwData[1];
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;
	DWORD	madrPCCSR[RX_CLS3_PPC_CH] = {OCD_REG_PPC_PCCSR0,OCD_REG_PPC_PCCSR1};
	WORD	i;
	BYTE*	pbyOcdData;
	FFWRX_PPCCTRL_DATA*			ePev;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	// RevRxNo121026-001 Append Start
	// wPPCが"0"の時、何もせずにreturnする。
	if (pMcuInfo->wPPC == RX_PPC_NON) {
		return ferr;
	}
	// RevRxNo121026-001 Append End

	pbyOcdData = reinterpret_cast<BYTE*>(dwData);

	// ダイレクトユーザカウント機能の設定条件を確認 
	// イベントPPC使用宣言がされていない場合は、即リターン
	if (GetPerfUsrCtrl() == FFWRX_EVCOMBI_PPC_USR) {
		return FFWERR_OK;
	}

	// パフォーマンス設定値を取得
	ePev = GetPpcCtrlData();

	if(bEnable) {
		for(i=0;i<(pMcuInfo->wPPC);i++){
			if(ePev->bSetPerfFunc[i] == TRUE) {
				ferr = GetMcuOCDReg(madrPCCSR[i],eAccessSize,OCD_ACCESS_COUNT,pbyOcdData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				dwData[0] = dwData[0] | OCD_REG_PPC_PCCSR_PPCE;
				ferr = SetMcuOCDReg(madrPCCSR[i],eAccessSize,pbyOcdData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}

	} else {
		//RevNo100715-014 Modify Start
		//パフォーマンスONのときにOFFにする。
		for(i=0;i<(pMcuInfo->wPPC);i++){
			if(ePev->bSetPerfFunc[i] == TRUE) {
				ferr = GetMcuOCDReg(madrPCCSR[i],eAccessSize,OCD_ACCESS_COUNT,pbyOcdData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				dwData[0] = dwData[0] & ~OCD_REG_PPC_PCCSR_PPCE;
				ferr = SetMcuOCDReg(madrPCCSR[i],eAccessSize,pbyOcdData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}
		//RevNo100715-014 Modify End
	}

	return ferr;
}

//=============================================================================
/**
 * パフォーマンス内部管理変数をPPCコントロールレジスタ(PCCSR)に設定する値に変換 Class3用
 * @param なし
 * @retval なし
 */
//=============================================================================
void ConvPPCCTRL_DATA2PCCSRCls3(BYTE byPpcCh, FFWRX_PPCCTRL_DATA* pPev,DWORD* dwPCCSR){

	DWORD dwData;

	// PCCSR0レジスタ値設定
	//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
	//  ┌─────────┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
	//  │                                                              │
	//  └─────────┴─┴─┴─┴─┴─┴───┴─┴─┴─┴─┘
	//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
	//  ┌───────────┬─┬─┬───┬─┬─┬─┬─┬─┬─┐
	//  │                      │  │  │      │  │  │  │  │  │  │
	//  └───────────┴─┴─┴───┴─┴─┴─┴─┴─┴─┘
	//  b26-16:CIN  dwMplItemNum b10-b0
	//  b4:FOC      bSetOnceMeasure TRUE = 1
	//  b2:CCE      bSetConnect TRUE = 1
	//  b1:COS      dwOvf b0
	//  b0:PPCE     1
	dwData = (((pPev->dwMpItemNum[byPpcCh]				<< 16) & OCD_REG_PPC_PCCSR_CIN) |		// b26-16
			     ((pPev->bSetOnceMeasure[byPpcCh]		<< 4) & OCD_REG_PPC_PCCSR_FOC) |		// b4
			     ((pPev->bSetConnect					<< 2) & OCD_REG_PPC_PCCSR_CCE) |		// b2
				 ((pPev->dwOvf[byPpcCh]		            << 1) & OCD_REG_PPC_PCCSR_COS) |		// b1
				 ((pPev->bSetPerfFunc[byPpcCh]		        ) & OCD_REG_PPC_PCCSR_PPCE));		// b0

	*dwPCCSR = dwData;
	return;
}

//=============================================================================
/**
 * パフォーマンス内部管理変数をPPCコントロールレジスタ(PCCSR)に設定する値に変換 Class2用
 * @param なし
 * @retval なし
 */
//=============================================================================
void ConvPPCCTRL_DATA2PCCSRCls2(BYTE byPpcCh, FFWRX_PPCCTRL_DATA* pPev,DWORD* dwPCCSR){


	// PCCSR0レジスタ値設定
	//    31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16
	//  ┌─────────┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
	//  │                                                              │
	//  └─────────┴─┴─┴─┴─┴─┴───┴─┴─┴─┴─┘
	//    15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
	//  ┌───────────┬─┬─┬───┬─┬─┬─┬─┬─┬─┐
	//  │                      │  │  │      │  │  │  │  │  │  │
	//  └───────────┴─┴─┴───┴─┴─┴─┴─┴─┴─┘
	//  b26-16:CIN  dwMplItemNum b10-b0 00010100001(固定 Rのみ)
	//  b4:FOC      bSetOnceMeasure TRUE = 1
	//  b0:PPCE     1
	*dwPCCSR =	(((PPC_CYC_PASS							<< 16) & OCD_REG_PPC_PCCSR_CIN) |		// b26-16
				 ((pPev->bSetOnceMeasure[byPpcCh]		<< 4) & OCD_REG_PPC_PCCSR_FOC) |		// b4
				 ((pPev->bSetPerfFunc[byPpcCh]		        ) & OCD_REG_PPC_PCCSR_PPCE));		// b0
	return;
}


//=============================================================================
/**
 * パフォーマンス計測実行ビットのクリア
 * @param dwPCCSR PCCSRレジスタ設定値
 * @param pdwClrPCCSR PCCSRレジスタのPPCEをクリアした値格納ポインタ
 * @retval なし
 */
//=============================================================================
void ClrPpcCtrlEnable(DWORD dwPCCSR,DWORD* dwClrPCCSR){

	*dwClrPCCSR = dwPCCSR & ~OCD_REG_PPC_PCCSR_PPCE;
	return;
}

//=============================================================================
/**
 * パフォーマンス計測が有効
 * @param dwPpcCh 確認CH CH0:0 CH1:1
 * @param pbPpcRun TRUE:計測有効　FALSE計測無効
 * @param pdwPpcMode　計測モードを格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetPpcEnaInfo(DWORD dwPpcCh,BOOL* pbPpcRun,DWORD* pdwPpcMode){
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	DWORD						madrPCCSR[RX_CLS3_PPC_CH] = {OCD_REG_PPC_PCCSR0,OCD_REG_PPC_PCCSR1};
	DWORD						dwPCCSR[1];
	BYTE*						pbyPCCSR;

	eAccessSize = MLWORD_ACCESS;
	*pbPpcRun = FALSE;
	*pdwPpcMode = PPC_CYC_PASS;

	pbyPCCSR = reinterpret_cast<BYTE*>(dwPCCSR);
	ferr = GetMcuOCDReg(madrPCCSR[dwPpcCh],eAccessSize,OCD_ACCESS_COUNT, pbyPCCSR);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//計測中
	if ( (dwPCCSR[0] & OCD_REG_PPC_PCCSR_PPCE) == OCD_REG_PPC_PCCSR_PPCE) {
		*pbPpcRun = TRUE;
	}

	//計測モード
	*pdwPpcMode = (dwPCCSR[0] & OCD_REG_PPC_PCCSR_CIN) >> 16;

	return ferr;

}
//=============================================================================
/**
 * パフォーマンスイベントカウンタ　イベント設定
 * @param dwSetEvNum 設定イベント0～7bit:実行PCイベント0～7 8～11bit:オペランドイベント0～3
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetPpcCntEv(DWORD dwSetEvNum){

	FFWERR						ferr;
	DWORD						dwData[1];
	BYTE*						pbySetEvNum;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	eAccessSize = MLWORD_ACCESS;
	
	dwData[0] = dwSetEvNum;
	pbySetEvNum = reinterpret_cast<BYTE*>(dwData);

	ferr = SetMcuOCDReg(OCD_REG_PPC_EVENSR,eAccessSize,pbySetEvNum);

	return ferr;
}

//=============================================================================
/**
 * パフォーマンスカウンタ　カウント値取得
 * @param dwPpcCh カウンタチャネル
 * @param *pdwPpcCnt カウンタ値
 * @param *bTimOvf オーバーフローフラグ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetPpcCnt(DWORD dwPpcCh,DWORD* pdwPpcCnt,BOOL* pbTimOvf)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	DWORD	madrPCCSR[RX_CLS3_PPC_CH] = {OCD_REG_PPC_PCCSR0,OCD_REG_PPC_PCCSR1};
	DWORD	madrPCR[RX_CLS3_PPC_CH] = {OCD_REG_PPC_PCR0,OCD_REG_PPC_PCR1};
	DWORD	dwBuff[1];
	BYTE*	pbyOcdData;
	
	eAccessSize = MLWORD_ACCESS;

	// オーバーフローフラグ取得
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = GetMcuOCDReg(madrPCCSR[dwPpcCh],eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	*pbTimOvf = (dwBuff[0] & OCD_REG_PPC_PCCSR_COF) >> 9;

	// PPC値取得
	ferr = GetMcuOCDReg(madrPCR[dwPpcCh],eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	*pdwPpcCnt = dwBuff[0];

	return ferr;

}

//=============================================================================
/**
 * パフォーマンスカウンタ　クリア処理
 * @param dwPpcCh カウンタチャネル
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ClrPpcCnt(DWORD dwPpcCh){

	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	DWORD	madrPCCSR[RX_CLS3_PPC_CH] = {OCD_REG_PPC_PCCSR0,OCD_REG_PPC_PCCSR1};
	DWORD	dwPCCSR[1];
	BYTE*	pbyPCCSR;
	DWORD	dwPCCSR_bkup[1];
	BYTE*	pbyPCCSR_bkup;
	DWORD	dwBuff[1];
	BYTE*	pbyOcdData;

	eAccessSize = MLWORD_ACCESS;

	// PCCSRの設定値を取得
	pbyPCCSR = reinterpret_cast<BYTE*>(dwPCCSR);
	ferr = GetMcuOCDReg(madrPCCSR[dwPpcCh],eAccessSize,OCD_ACCESS_COUNT, pbyPCCSR);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	dwPCCSR_bkup[0] = dwPCCSR[0];

	//クリア
	dwPCCSR[0] = (DWORD)((dwPCCSR[0] | OCD_REG_PPC_PCCSR_PRC) & ~OCD_REG_PPC_PCCSR_PPCE);
	ferr = SetMcuOCDReg(madrPCCSR[dwPpcCh],eAccessSize,pbyPCCSR);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = GetMcuOCDReg(madrPCCSR[dwPpcCh],eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// PCCSR0.PRCが0になるまで待つ
	// RevNo121017-004	Modify Line
	if ((dwBuff[0] & OCD_REG_PPC_PCCSR_PRC) == OCD_REG_PPC_PCCSR_PRC) {
		ferr = GetMcuOCDReg(madrPCCSR[dwPpcCh],eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	//書き戻し
	pbyPCCSR_bkup = reinterpret_cast<BYTE*>(dwPCCSR_bkup);
	ferr = SetMcuOCDReg(madrPCCSR[dwPpcCh],eAccessSize,pbyPCCSR_bkup);

	return ferr;

}
//=============================================================================
/**
 * パフォーマンス関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuRxData_Tim(void)
{

	return;

}
