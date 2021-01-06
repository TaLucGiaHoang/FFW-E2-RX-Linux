////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_tim.cpp
 * @brief RX共通実行時間測定関連コマンドのソースファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, K.Uemori, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/17
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
*/

#include "ffwrx_tim.h"
#include "ffwrx_ev.h"
#include "dorx_tim.h"
#include "errchk.h"
#include "ffwmcu_mcu.h"
#include "domcu_prog.h"

// FFW 内部変数
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
static FFWRX_PPCCTRL_DATA s_PpcCtrl;				// パフォーマンス計測イベントの設定情報
// 2008.11.18 INSERT_END_E20RX600 }
static DWORD s_dwSetEvNum;							// イベントカウント計測設定イベント情報

// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
// V.1.02 No.38 Modify Start
//==============================================================================
/**
 * PPC計測動作モードの設定
 * @param dwSetMp 区間時間測定イベントを設定するカウンタ番号
 * @param pPev    設定する区間時間測定イベントを格納するFFWRX_PPCCTRL_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_SetPPCCTRL(DWORD dwSetMp, const FFWRX_PPCCTRL_DATA* pPev)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	WORD	i;
	BOOL	bRet = FALSE;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo121026-001 Modify Line
	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// ダイレクトユーザカウント機能の設定条件を確認 
	// イベントPPC使用宣言がされていない
	// RevRxNo121026-001 Append Line
	// wPPCがあるMCUはユーザ使用かの判定を実施する。
	if (pMcuInfo->wPPC != RX_PPC_NON) {
		if (GetPerfUsrCtrl() == FFWRX_EVCOMBI_PPC_USR) {
			return FFWERR_PPC_USR_CTL;
		}
	}

	//設定チャネルがMCUに設定されているチャネルよりも大きいかどうかのチェック
	if(dwSetMp >= (DWORD)(0x00000001<<(pMcuInfo->wPPC))){
		return FFWERR_FFW_ARG;
	}

	// 実行中かどうかのフラグ取得
	bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line

	// RevRxNo121026-001 Append Line
	if (pMcuInfo->wPPC != RX_PPC_NON) {	// wPPCがあるMCUはPPCの設定を実施する。

		// OCD Class3の場合
		// RevRxNo130411-001 Modify Line
		if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {

			for(i=0;i<(pMcuInfo->wPPC);i++){
				if((dwSetMp & (0x1<<i)) != 0){
					//引数チェック 動作モード
					if (pPev->dwMpItemNum[i] > PM_MPITEM_EVMATCH) {
						return FFWERR_FFW_ARG;
					}

					//引数チェック カウンタオーバーフロー
					if (pPev->dwOvf[i] > PM_OVF_ENA) {
						return FFWERR_FFW_ARG;
					}
					
					//RUN中かつパフォーマンス計測中で、ONのまま設定を入れるのはエラー
					if ((bRet == TRUE) && (s_PpcCtrl.bSetPerfFunc[i] == TRUE) && (pPev->bSetPerfFunc[i] == TRUE) ) {
						return FFWERR_PPC_RUN;
					}
				}
			}

			//コネクタ連結モード設定 連結モード時に 0,1chで設定が違う場合はエラー
			if (pPev->bSetConnect == TRUE) {
				if (pPev->dwMpItemNum[0] !=  pPev->dwMpItemNum[1] ) {
					return FFWERR_PPC_CONNECT;
				}
				if (pPev->bSetOnceMeasure[0] !=  pPev->bSetOnceMeasure[1] ) {
					return FFWERR_PPC_CONNECT;
				}
				if (pPev->dwOvf[0] !=  pPev->dwOvf[1] ) {
					return FFWERR_PPC_CONNECT;
				}
				if (pPev->bSetPerfFunc[0] !=  pPev->bSetPerfFunc[1] ) {
					return FFWERR_PPC_CONNECT;
				}
			}
			ferr = DO_SetPPCCTRL(dwSetMp,pPev);
		} else {
			// OCD Class2の場合
			if (pPev->dwMpItemNum[0] != PM_MPITEM_MCU_CYC) {
				return FFWERR_PPC_UNSUPPORT;
			}
			//RUN中かつパフォーマンス計測中で、ONのまま設定を入れるのはエラー
			if ((bRet == TRUE) && (s_PpcCtrl.bSetPerfFunc[0] == TRUE) && (pPev->bSetPerfFunc[0] == TRUE) ) {
				return FFWERR_PPC_RUN;
			}

			ferr = DO_SetRXCls2PPCCTRL(dwSetMp,pPev);
		}
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	//内部管理変数更新
	for(i=0;i<(pMcuInfo->wPPC);i++){
		if((dwSetMp & (0x1<<i)) != 0){
			//Class2は結合機能がないためFLASEにする
			// RevRxNo130411-001 Modify Line
			if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
				s_PpcCtrl.bSetConnect =  pPev->bSetConnect;
				s_PpcCtrl.dwOvf[i] = pPev->dwOvf[i];
			// RevRxNo130411-001 Modify Line
			} else {
				s_PpcCtrl.bSetConnect =  FALSE;
				s_PpcCtrl.dwOvf[i] = 0;
			}
			s_PpcCtrl.bSetOnceMeasure[i] =  pPev->bSetOnceMeasure[i];
			s_PpcCtrl.bSetPerfFunc[i] = pPev->bSetPerfFunc[i];
			s_PpcCtrl.dwMpItemNum[i] = pPev->dwMpItemNum[i];
		}
	}

	return ferr;
}
// V.1.02 No.38 Modify End

//==============================================================================
/**
 * PPC計測動作モードの参照
 * @param pPev    設定する区間時間測定イベントを格納するFFWRX_PPCCTRL_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_GetPPCCTRL(FFWRX_PPCCTRL_DATA* pPev)
{
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	//INITで静的変数初期化(InitFfwIfRxCmdData_Tim())が呼ばれているので、Class2のときのGetのdwMpItemNumの値を1で返す。
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
		s_PpcCtrl.dwMpItemNum[0]			= 1;
	}
	memcpy(pPev,&s_PpcCtrl, sizeof(FFWRX_PPCCTRL_DATA));

	return FFWERR_OK;
}

// V.1.02 No.38 Modify Start
//==============================================================================
/**
 * PPC計測カウントイベントの設定
 * @param dwSetEvNum カウントイベント番号を設定
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_SetPPCEVCNT(DWORD dwSetEvNum)
{

	// V.1.02 RevNo110613-001 Apped Line
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (((pFwCtrl->eOcdCls == RX_OCD_CLS2) || (pMcuInfo->wPPC == RX_PPC_NON)) && (dwSetEvNum != 0)) {
		return FFWERR_PPC_UNSUPPORT;
	}
	// ダイレクトユーザカウント機能の設定条件を確認
	// イベントPPC使用宣言がされていない
	// RevRxNo121026-001 Append Line
	// wPPCがあるMCUはPPCカウントイベントの設定を実施する。
	if (pMcuInfo->wPPC != RX_PPC_NON) {
		if (GetPerfUsrCtrl() == FFWRX_EVCOMBI_PPC_USR) {
			return FFWERR_PPC_USR_CTL;
		}

		// RevNo110308-003 Modify Line
		//引数チェック 設定イベント番号
		if ((dwSetEvNum & 0x0000FFFF) >= (DWORD)(0x1 << (DWORD)(pMcuInfo->wPCEvent))) {
			return FFWERR_FFW_ARG;
		}

		// RevNo110308-003 Modify Line
		if ((dwSetEvNum & 0xFFFF0000) >= (DWORD)(0x00010000 << (DWORD)(pMcuInfo->wOAEvent))) {
			return FFWERR_FFW_ARG;
		}

		// V.1.02 RevNo110613-001 Modify Start
		ferr = DO_SetPPCEVCNT(dwSetEvNum);
		if(ferr == FFWERR_OK){
			ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
		}

		//内部管理変数更新
		s_dwSetEvNum = dwSetEvNum;
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}
// V.1.02 No.38 Modify End

//==============================================================================
/**
 * PPC計測カウントイベントの参照
 * @param pdwSetEvNum カウントイベント番号を設定
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_GetPPCEVCNT(DWORD* pdwSetEvNum)
{
	FFWERR ferr = FFWERR_OK;

	*pdwSetEvNum = s_dwSetEvNum;

	return ferr;
}

//==============================================================================
/**
 * PPC計測データの取得
 * @param dwGetMp 区間時間測定データを取得するカウンタ番号
 * @param pMpd    区間時間測定データを格納するFFWRX_MPD_DATA構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_GetPPCD(DWORD dwGetMp, FFWRX_MPD_DATA* pMpd)
{

	// V.1.02 RevNo110613-001 Append Line
	FFWERR ferr = FFWERR_OK;
	// V.1.02 No.38 Modify Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	
	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	
	//引数チェック 取得チャンネル 2ch以上は未対応なので、引数エラーにする
	if (dwGetMp >= (DWORD)(0x00000001<<(pMcuInfo->wPPC)) ) {
		return FFWERR_FFW_ARG;
	}
	// V.1.02 No.38 Modify End

	// RevRxNo121026-001 Append Line
	// wPPCがあるMCUはPPC計測データの取得を実施する。
	if (pMcuInfo->wPPC != RX_PPC_NON) {
		// V.1.02 RevNo110613-001 Modify Start
		ferr = DO_GetPPCD(dwGetMp, pMpd);
		if(ferr == FFWERR_OK){
			ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
		}
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}

//==============================================================================
/**
 * PPC計測データのクリア
 * @param dwClrMp 区間時間測定データをクリアするカウンタ番号を指定
  * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_ClrPPCD(DWORD dwClrMp)
{
	// V.1.02 RevNo110613-001 Append Line
	FFWERR ferr = FFWERR_OK;
	// V.1.02 No.38 Modify Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	// ダイレクトユーザカウント機能の設定条件を確認
	// イベントPPC使用宣言がされていない
	// RevRxNo121026-001 Append Line
	// wPPCがあるMCUはユーザ使用かの判定を実施する。
	if (pMcuInfo->wPPC != RX_PPC_NON) {
		if (GetPerfUsrCtrl() == FFWRX_EVCOMBI_PPC_USR) {
			return FFWERR_PPC_USR_CTL;
		}
	}

	//引数チェック 取得チャンネル　2ch以上は未対応なので、引数エラーにする
	if (dwClrMp >= (DWORD)(0x00000001<<(pMcuInfo->wPPC)) ) {
		return FFWERR_FFW_ARG;
	}
	// V.1.02 No.38 Modify End

	// RevRxNo121026-001 Append Line
	// wPPCがあるMCUはPPC計測データのクリアを実施する。
	if (pMcuInfo->wPPC != RX_PPC_NON) {
		// V.1.02 RevNo110613-001 Modify Start
		ferr = DO_ClrPPCD(dwClrMp);
		if(ferr == FFWERR_OK){
			ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
		}
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * パフォーマンス計測イベントの設定情報を取得する。
 * @param なし
 * @retval パフォーマンス計測イベントの設定情報格納領域のアドレス
 */
//=============================================================================
FFWRX_PPCCTRL_DATA* GetPpcCtrlData(void)
{
	return &s_PpcCtrl;
}
// 2008.11.18 INSERT_END_E20RX600 }
//==============================================================================
/**
 * パフォーマンスイベントカウント　内部管理変数
 * @param なし
 * @retval s_dwSetEvNum             
 */
//==============================================================================
DWORD GetPpcCntEvNum(void){
	return s_dwSetEvNum;
}


//=============================================================================
/**
 * 実行時間測定関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfRxData_Tim(void)
{
	memset(&s_PpcCtrl, 0, sizeof(FFWRX_PPCCTRL_DATA));
	s_PpcCtrl.bSetConnect			= FALSE;
	s_PpcCtrl.dwMpItemNum[0]			= 0;
	s_PpcCtrl.dwMpItemNum[1]			= 0;
	s_PpcCtrl.bSetOnceMeasure[0]		= FALSE;
	s_PpcCtrl.bSetOnceMeasure[1]		= FALSE;
	s_PpcCtrl.dwOvf[0]				= 1;
	s_PpcCtrl.dwOvf[1]				= 1;
	s_PpcCtrl.bSetPerfFunc[0]		= FALSE;
	s_PpcCtrl.bSetPerfFunc[1]		= FALSE;

	s_dwSetEvNum = 0;
	return;

}
