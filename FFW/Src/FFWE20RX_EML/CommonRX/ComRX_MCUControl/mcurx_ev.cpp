////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcurx_ev.cpp
 * @brief RX共通イベント関連コマンドのソースファイル
 * @author RSO Y.Minami, H.Hashiguchi, K.Uemori, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/11/11
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120606-007	2012/07/13 橋口
  Class2 GPBのイベントを後ろから設定したときに、エラーが出る不具合修正
・RevNo110825-01 2012/07/13 橋口
　GPB実行イベントへのパスカウント設定不具合対応
・RevNo120131-001 2012/07/13 橋口
  GPB実行中の実行前、イベントブレーク不具合対応
・RevNo120606-002 2012/7/13 橋口
  イベントブレーク禁止中の実行中のイベントブレーク設定不具合対応
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121115-005 2012/11/15 橋口, 上田
　SetBreakEvEnable() オペランドアクセスブレーク設定処理修正。
  (GB,GPB内でSTEPをする時に、オペランドブレークを設定してもブレークしない不具合修正)
・RevRxNo121022-001	2012/11/21 SDS 岩田
  EZ-CUBE PROT_MCU_WRITE()分割処理対応
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001	2013/04/18 上田
	RX64M対応
・RevRxNo131101-006	2013/11/11 上田
	GPB実行中のパフォーマンス計測不具合修正
*/
#include "mcurx_ev.h"
#include "ocdspec.h"
#include "mcurx.h"
#include "ffwrx_ev.h"
#include "domcu_prog.h"
#include "ffwmcu_mcu.h"
#include "protmcu_mem.h"
#include "ffwrx_tim.h"		// RevRxNo120606-007 Append Line
#include "mcudef.h"	// RevRxNo130411-001 Append Line

static DWORD				s_dwEvSetData_RX[EV_ENABLE_REGNUM];	// イベント無効化前のイベント設定管理変数
// RevNo120131-001 Delete	s_dwBrkEvData_RX[EV_ENABLE_REGNUM],s_dwPreBrkEvData_RX[EV_ENABLE_REGNUM];

//==============================================================================
/**
 * ブレークイベントの設定を行う。
 * @param  evBrk           ブレークイベント情報を格納するFFWRX_COMB_BRK 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetEvBreakData(FFWRX_COMB_BRK* evBrk)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwWorkBrk[1];			// レジスタ書き込み時の退避用
	BYTE*						pbyWorkBrk;

	// RevNo120606-002 Delete
	// 内部変数にセットはこの関数内ではやらない

	// レジスタ設定用
	eAccessSize = MLWORD_ACCESS;

	// RevNo120606-002 Delete
	// プログラム実行状態チェックはこの関数内ではやらない

	// レジスタ設定
	// イベントブレーク用実行PCイベント選択への設定
	dwWorkBrk[0] = evBrk->dwBrkpe;
	madrWriteAddr = OCD_REG_EV_EVEBRKPCP;
	pbyWorkBrk = reinterpret_cast<BYTE*>(dwWorkBrk);
	// RevNo120606-002 Modify Start
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkBrk);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevNo120606-002 Modify End

	// イベントブレーク用オペランドイベント選択への設定
	dwWorkBrk[0] = evBrk->dwBrkde;
	madrWriteAddr = OCD_REG_EV_EVEBRKOP;
	// RevNo120606-002 Modify Start
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkBrk);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevNo120606-002 Modify End

	return FFWERR_OK;
}

//==============================================================================
/**
 * トレースイベントの設定を行う。
 * @param  evTrc           トレースイベント情報を格納するFFWRX_COMB_TRC 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetEvTraceData(FFWRX_COMB_TRC* evTrc)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwWorkTrc[1];			// レジスタ書き込み時の退避用
	BYTE*						pbyWorkTrc;

	// RevNo120131-001 Delete
	// 内部変数にセットはこの関数内ではやらない

	// レジスタ設定用
	eAccessSize = MLWORD_ACCESS;
	pbyWorkTrc = reinterpret_cast<BYTE*>(dwWorkTrc);

	// レジスタ設定
	// トレース開始用実行PCイベント選択への設定
	dwWorkTrc[0] = evTrc->dwStartpe ;
	madrWriteAddr = OCD_REG_EV_EVETSPCP;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkTrc);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレース開始用オペランドイベント選択への設定
	dwWorkTrc[0] = evTrc->dwStartde;
	madrWriteAddr = OCD_REG_EV_EVETSOP;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkTrc);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// トレース終了用実行PCイベント選択への設定
	dwWorkTrc[0] = evTrc->dwEndpe;
	madrWriteAddr = OCD_REG_EV_EVETEPCP;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkTrc);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// トレース終了用オペランドイベント選択への設定
	dwWorkTrc[0] = evTrc->dwEndde;
	madrWriteAddr = OCD_REG_EV_EVETEOP;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkTrc);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// トレース抽出用オペランドイベント選択への設定
	dwWorkTrc[0] = evTrc->dwPickupde;
	madrWriteAddr = OCD_REG_EV_EVETXOP;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkTrc);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * パフォーマンスイベントの設定を行う。
 * @param  dwPerfNo        パフォーマンス計測Noを指定
 * @param  evPerfTime      パフォーマンス計測イベント情報を格納するFFWRX_COMB_PERFTIME 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetEvPerfData(DWORD dwPerfNo, FFWRX_COMB_PERFTIME* evPerfTime)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwWorkPerf[1];			// レジスタ書き込み時の退避用
	BYTE*						pbyWorkPerf;

	// RevNo120131-001 Delete
	// 内部変数にセットはこの関数ないでやらない

	// レジスタ設定用
	eAccessSize = MLWORD_ACCESS;
	pbyWorkPerf = reinterpret_cast<BYTE*>(dwWorkPerf);

	dwWorkPerf[0] = evPerfTime->dwStartpe[dwPerfNo];
	// パフォーマンスn開始用実行PCイベント選択への設定
	madrWriteAddr = OCD_REG_EV_EVEPSPCP0 + (dwPerfNo * 4);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkPerf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	dwWorkPerf[0] = evPerfTime->dwStartde[dwPerfNo];
	// パフォーマンスn開始用オペランドアクセスイベント選択への設定
	madrWriteAddr = OCD_REG_EV_EVEPSOP0 + (dwPerfNo * 4);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkPerf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	dwWorkPerf[0] = evPerfTime->dwEndpe[dwPerfNo];
	// パフォーマンスn終了用実行PCイベント選択への設定
	madrWriteAddr = OCD_REG_EV_EVEPEPCP0 + (dwPerfNo * 4);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkPerf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	dwWorkPerf[0] = evPerfTime->dwEndde[dwPerfNo];
	// パフォーマンスn終了用オペランドアクセスイベント選択への設定
	madrWriteAddr = OCD_REG_EV_EVEPEOP0 + (dwPerfNo * 4);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkPerf);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * イベント設定の有効化/無効化を設定する。
 * @param  bEnable     TRUE:イベント設定有効化, FALSE:イベント設定無効化
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetEventEnable(BOOL bEnable)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrReadAddr;		// 参照するレジスタのアドレス
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwEnable[1];
	BYTE*						pbyEnable;
	DWORD						dwRegAddr[] = {OCD_REG_EV_EVEPCPREE, OCD_REG_EV_EVEPCPE, OCD_REG_EV_EVEOPE};
	int							i;
	DWORD						dwBuff[1];
	BYTE*						pbyOcdData;
	DWORD						dwEvSetData[1];
	BYTE*						pbyEvSetData;
	DWORD						dwClrData[1];
	BYTE*						pbyClrData;

	// レジスタ設定
	eAccessSize = MLWORD_ACCESS;

	if (bEnable == TRUE) {			// 有効化の場合
		for (i = 0; i < (sizeof(dwRegAddr) / sizeof(DWORD)); i++) {
			// レジスタに書き込み
			madrWriteAddr = dwRegAddr[i];
			dwEvSetData[0] = s_dwEvSetData_RX[i];
			pbyEvSetData = reinterpret_cast<BYTE*>(dwEvSetData);
			ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize, pbyEvSetData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

	} else {						// 無効化の場合
		dwEnable[0] = 0x00000000;
		pbyEnable = reinterpret_cast<BYTE*>(dwEnable);
		for (i = 0; i < (sizeof(dwRegAddr) / sizeof(DWORD)); i++) {
			// レジスタの読み出し
			madrReadAddr = dwRegAddr[i];
			pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
			ferr = GetMcuOCDReg(madrReadAddr, eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_dwEvSetData_RX[i] = dwBuff[0];

			// レジスタに書き込み
			madrWriteAddr = dwRegAddr[i];
			ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize, pbyEnable);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

		// 実行PC通過イベント成立結果フラグクリア
		dwClrData[0] = 0;
		pbyClrData = reinterpret_cast<BYTE*>(dwClrData);
		ferr = SetMcuOCDReg(OCD_REG_EV_EVEPCPFLG,eAccessSize, pbyClrData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// オペランドアクセスイベント成立結果フラグクリア
		ferr = SetMcuOCDReg(OCD_REG_EV_EVEOPFLG,eAccessSize, pbyClrData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * 通過イベントブレーク設定の有効化/無効化を設定する。
 * @param  bEnable     TRUE:イベント設定有効化, FALSE:イベント設定無効化
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetBreakEvEnable(BOOL bEnable)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwEnable[1];
	BYTE*						pbyEnable;
	DWORD						dwRegAddr[] = {OCD_REG_EV_EVEBRKPCP, OCD_REG_EV_EVEBRKOP};
	DWORD						dwBrkEvData[1];
	BYTE*						pbyBrkEvData;
	FFWRX_COMB_BRK*				evBrk;

	// 内部管理変数値取得
	evBrk=GetEvCombBrkInfo();
	// レジスタ設定
	eAccessSize = MLWORD_ACCESS;

	if (bEnable == TRUE) {			// 有効化の場合
	//RevNo120131-001 Modify Start
	//FALSEで0レジスタ値は0になっているはずなので、s_dwBrkEvData_RX[]が0以外の場合はレジスタに値を入れる必要がある
	// 0: 実行後ブレーク 1:オペランドブレーク
	//RevNo100715-014 Modify Start
		if(evBrk->dwBrkpe!=OCD_REG_EV_EVEBRKPCP_ALL_DIS){
			//実行通過PCブレークがすべて無効ではない場合
			madrWriteAddr = dwRegAddr[0];
			dwBrkEvData[0] = evBrk->dwBrkpe;
			pbyBrkEvData = reinterpret_cast<BYTE*>(dwBrkEvData);
			ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyBrkEvData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		//RevRxNo121115-005 Modify Line
		if(evBrk->dwBrkde!=OCD_REG_EV_EVEBRKOP_ALL_DIS){
			//オペランドアクセスブレークがすべて無効ではない場合
			madrWriteAddr = dwRegAddr[1];
			dwBrkEvData[0] = evBrk->dwBrkde;
			pbyBrkEvData = reinterpret_cast<BYTE*>(dwBrkEvData);
			ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyBrkEvData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	//RevNo100715-014 Modify End
	//RevNo120131-001 Modify End
	} else {						// 無効化の場合
		dwEnable[0] = OCD_REG_EV_EVEBRKPCP_ALL_DIS;
		pbyEnable = reinterpret_cast<BYTE*>(dwEnable);
		//RevNo120131-001 Modify Start
		// s_dwBrkEvData_RX[]で退避復帰をすると、実行中の内部管理変数の変更が難しくなるため
		// 管理はs_EvCombBrk_RXで一元化する
		//RevNo100715-014 Modify Start
		//実行後PCブレーク
		if(evBrk->dwBrkpe != OCD_REG_EV_EVEBRKPCP_ALL_DIS){
		//RevNo120131-001 Modify End
			// レジスタに書き込み
			madrWriteAddr = dwRegAddr[0];
			ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyEnable);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

		//オペランドアクセスブレーク
		//RevNo120131-001 Modify Start
		// s_dwBrkEvData_RX[]で退避復帰をすると、実行中の内部管理変数の変更が難しくなるため
		// 管理はs_EvCombBrk_RXで一元化する
		if(evBrk->dwBrkde != OCD_REG_EV_EVEBRKOP_ALL_DIS){
		//RevNo120131-001 Modify End
			// レジスタに書き込み
			madrWriteAddr = dwRegAddr[1];
			ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyEnable);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		//RevNo100715-014 Modify End
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * 実行前PCブレーク設定の有効化/無効化を設定する。
 * @param  bEnable     TRUE:イベント設定有効化, FALSE:イベント設定無効化
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetPreBreakEvEnable(BOOL bEnable)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwPreBrkEvData[1];
	BYTE*						pbyPreBrkEvData;
	DWORD						dwEnable[1];
	BYTE*						pbyEnable;
	DWORD						dwRegAddr[] = {OCD_REG_EV_EVEPCPREE};
	FFWRX_COMB_BRK*				evBrk;

	// 内部管理変数値取得
	evBrk=GetEvCombBrkInfo();

	// レジスタ設定
	eAccessSize = MLWORD_ACCESS;
	pbyPreBrkEvData = reinterpret_cast<BYTE*>(dwPreBrkEvData);

	if (bEnable == TRUE) {			// 有効化の場合
		//RevNo100715-014 Modify Start
		//FALSEで0レジスタ値は0になっているはずなので、s_dwBrkEvData_RX[]が0以外の場合はレジスタに値を入れる必要がある
		//実行前PCブレーク
		if(evBrk->dwPreBrk != OCD_REG_EV_EVEPCPREE_ALL_DIS){				// RevNo120131-001 Modify Line
			// レジスタに書き込み
			madrWriteAddr = dwRegAddr[0];
			dwPreBrkEvData[0] = evBrk->dwPreBrk;							// RevNo120131-001 Modify Line
			ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyPreBrkEvData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		//RevNo100715-014 Modify End
	} else {						// 無効化の場合
		dwEnable[0] = OCD_REG_EV_EVEPCPREE_ALL_DIS;
		pbyEnable = reinterpret_cast<BYTE*>(dwEnable);
		// RevNo120131-001 Modify Start
		// s_dwPreBrkEvData_RXを使うと実行中の実行前PCブレークイベント変更対応で内部変数管理が複雑になるので
		// s_EvCombBrk_RX.dwPreBrkのみで管理を一元化
		//RevNo100715-014 Modify Start
		//実行前PCブレーク
		if(evBrk->dwPreBrk != OCD_REG_EV_EVEPCPREE_ALL_DIS){
		// RevNo120131-001 Modify End
			// レジスタに書き込み
			madrWriteAddr = dwRegAddr[0];
			ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyEnable);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		//RevNo100715-014 Modify End
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * 組合せイベント設定の回路を初期化する。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
//RevNo120131-001　Modify Line
FFWERR InitCombEvent(DWORD dwCombi)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwInitStart[1];		// イベント組合せ設定の初期化開始
	BYTE*						pbyInitStart;
	DWORD						dwInitEnd[1];			// イベント組合せ設定の初期化終了
	BYTE*						pbyInitEnd;
	DWORD						dwCombi_RX = 0;		// イベント組み合わせレジスタ設定値

	// RevNo120131-001 Delete 内部管理変数値取得 GetCombiInfo();

	// レジスタ設定用
	eAccessSize = MLWORD_ACCESS;


	// イベント組合せ指定レジスタへの設定
	// 回路の初期化
	//    8   7   6   5   4   3   2   1   0
	//  ┌─┬─────────┬────┐
	//  │  │                  │        │
	//  └─┴─────────┴────┘
	//  b8:回路の初期化
	//  b2-0:イベント組合せ指定
	// レジスタ設定値と引数値が異なるため、コンバート
	switch(dwCombi) {
	case COMB_OR:
		dwCombi_RX = BRKTSC_OR;
		break;
	case COMB_BRK_AND:
		dwCombi_RX = BRKTSC_BRK_AND;
		break;
	case COMB_BRK_STATE:
		dwCombi_RX = BRKTSC_BRK_STATE;
		break;
	case COMB_TRC_AND:
		dwCombi_RX = BRKTSC_TRC_AND;
		break;
	case COMB_TRC_STATE:
		dwCombi_RX = BRKTSC_TRC_STATE;
		break;
	default:
		dwCombi_RX = BRKTSC_OR;
		break;
	}

	dwInitStart[0] = dwCombi_RX  | OCD_REG_EV_EVEBRKTSC_BTCTC;
	dwInitEnd[0] = dwCombi_RX;
	pbyInitStart = reinterpret_cast<BYTE*>(dwInitStart);
	pbyInitEnd = reinterpret_cast<BYTE*>(dwInitEnd);

	madrWriteAddr = OCD_REG_EV_EVEBRKTSC;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyInitStart);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// 組合せ指定(同じレジスタに設定)
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyInitEnd);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

// 2008.11.6 INSERT_END_E20RX600 }

//==============================================================================
/**
 * 対象アドレスにイベントの実行前PCブレークが設定されているかを検索する。
 * @param dwmadrAddr ブレーク検索対象アドレス
 * @param pbBrkEna	TRUE:見つかった　FALSE:見つからなかった 
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SearchEvPreBrkPoint(DWORD dwmadrAddr,BOOL* pbBrkEna)
{

	DWORD		i;
	DWORD	dwAddrPreBrkCombi;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	FFWERR			ferr = FFWERR_OK;
	DWORD			dwCombiData[1];
	BYTE*			pbyCombiData;
	DWORD			dwAddr[1];
	BYTE*			pbyAddr;

	*pbBrkEna = FALSE;
	dwAddrPreBrkCombi = OCD_REG_EV_EVEPCPREE;
	eAccessSize = MLWORD_ACCESS;

	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	pbyCombiData = reinterpret_cast<BYTE*>(dwCombiData);
	ferr = GetMcuOCDReg(dwAddrPreBrkCombi,eAccessSize,OCD_ACCESS_COUNT, pbyCombiData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (dwCombiData[0] == OCD_REG_EV_EVEPCPREE_ALL_DIS) {
		*pbBrkEna = FALSE;
		return ferr;
	}
	for(i = 0; i < (pMcuInfo->wPCEvent) ; i++) {
		if(((dwCombiData[0] >> i) & 0x1) == 0x1) {
			dwAddrPreBrkCombi = OCD_REG_EV_EVEPCA + (i * 4);
			pbyAddr = reinterpret_cast<BYTE*>(dwAddr);
			ferr = GetMcuOCDReg(dwAddrPreBrkCombi,eAccessSize,OCD_ACCESS_COUNT, pbyAddr);
			if (ferr != FFWERR_OK) {
				*pbBrkEna = FALSE;
				return ferr;
			}
			if(dwmadrAddr == dwAddr[0]) {
				*pbBrkEna = TRUE;
				return ferr;
			}
		}
	}
	return ferr;

}

//==============================================================================
/**
 * イベントの実行前PCブレーク組合せが指定されていれば無効化する。
 * @param pbBrkEna	TRUE:無効化した　FALSE:無効化しなかった 
 * @param pdwPreEvCombi	組合せ設定されている値 
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DisableEvPreBrkPoint(BOOL* pbBrkEna, DWORD* pdwPreEvCombi)
{

	DWORD	dwAddrPreBrkCombi;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	FFWERR			ferr = FFWERR_OK;
	DWORD						dwData[1];
	BYTE*						pbyOcdData;
	DWORD						dwCombi[1];
	BYTE*						pbyCombi;

	*pbBrkEna = TRUE;
	dwAddrPreBrkCombi = OCD_REG_EV_EVEPCPREE;
	eAccessSize = MLWORD_ACCESS;

	pbyCombi = reinterpret_cast<BYTE*>(dwCombi);
	ferr = GetMcuOCDReg(dwAddrPreBrkCombi,eAccessSize,OCD_ACCESS_COUNT, pbyCombi);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (dwCombi[0] == OCD_REG_EV_EVEPCPREE_ALL_DIS) {
		*pbBrkEna = FALSE;
		return ferr;
	}
	dwData[0] = OCD_REG_EV_EVEPCPREE_ALL_DIS;
	pbyOcdData = reinterpret_cast<BYTE*>(dwData);
	ferr = SetMcuOCDReg(dwAddrPreBrkCombi,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	*pdwPreEvCombi = dwCombi[0];
	
	return ferr;

}

//==============================================================================
/**
 * イベントの実行前PCブレーク組合せを有効化する。
 * @param pdwPreEvCombi	無効化時の組合せ設定値 
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR EnableEvPreBrkPoint(DWORD* pdwPreEvCombi)
{
	DWORD	dwAddrPreBrkCombi;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	DWORD			dwPreEvCombi[1];
	BYTE*			pbyPreEvCombi;
	FFWERR			ferr;

	dwAddrPreBrkCombi = OCD_REG_EV_EVEPCPREE;
	eAccessSize = MLWORD_ACCESS;
	dwPreEvCombi[0] = *pdwPreEvCombi;
	pbyPreEvCombi = reinterpret_cast<BYTE*>(dwPreEvCombi);

	ferr = SetMcuOCDReg(dwAddrPreBrkCombi,eAccessSize,pbyPreEvCombi);

	return  ferr;
}


//==============================================================================
/**
 * 実行PCイベント番号の空き番号を検索する。
 * @param  pbFindFlg 空きイベント有無格納変数のポインタ
 * @param  pdwEvNum 空きイベント番号格納変数のポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
// RevRxNo131101-006 Modify Line
FFWERR SearchExecPcEvNum(BOOL* pbFindFlg, DWORD* pdwEvNum)
{
	FFWERR						ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	DWORD						dwRegAddr[] = { OCD_REG_EV_EVEBRKPCP, OCD_REG_EV_EVETSPCP, OCD_REG_EV_EVETEPCP, 
												OCD_REG_EV_EVEPSPCP0, OCD_REG_EV_EVEPEPCP0, OCD_REG_EV_EVEPSPCP1, OCD_REG_EV_EVEPEPCP1};		// RevRxNo120606-007 Modify Line
	DWORD						dwBuffData[] = {0, 0, 0, 0, 0, 0, 0};
	DWORD						dwRegAddrCls3;		// RevRxNo131101-006 Append Line
	int							i,nReadRegEnd;		// RevRxNo120606-007 Modify Line;
	DWORD						dwEmptyBit = 0;
	DWORD						dwOcdData[1];
	BYTE*						pbyOcdData;
	FFWMCU_FWCTRL_DATA*			pFwCtrl;	// RevRxNo131101-006 Append Line
	// V.1.02 No.10 Class2 イベント対応 Append Start
	FFWRX_MCUINFO_DATA* pMcuInfo;

	*pbFindFlg = FALSE;

	pMcuInfo = GetMcuInfoDataRX();
	// V.1.02 No.10 Class2 イベント対応 Append End
	pFwCtrl = GetFwCtrlData();	// RevRxNo131101-006 Append Line

	// レジスタ設定
	eAccessSize = MLWORD_ACCESS;
	
	// RevRxNo120606-007 Modify Start
	// 開始と終了イベントがあるので、PPC最大ch*2分引いておく
	nReadRegEnd = (sizeof(dwBuffData) / sizeof(DWORD))-(RX_PPC_MAX_CH*2);
	// サポートPPC ch*2分を足す
	nReadRegEnd += (pMcuInfo->wPPC*2);
	
	for (i = 0; i < nReadRegEnd; i++) {
	// RevRxNo120606-007 modify End
		// レジスタの読み出し
		pbyOcdData = reinterpret_cast<BYTE*>(dwOcdData);
		ferr = GetMcuOCDReg(dwRegAddr[i],eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		dwBuffData[i] = dwOcdData[0];
		dwEmptyBit |= dwBuffData[i];
	}

	// RevRxNo131101-006 Append Start
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		//OCD Class3の場合
		dwRegAddrCls3 = OCD_REG_PPC_EVENSR;
		pbyOcdData = reinterpret_cast<BYTE*>(dwOcdData);
		ferr = GetMcuOCDReg(dwRegAddrCls3, eAccessSize, OCD_ACCESS_COUNT, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		dwOcdData[0] &= OCD_REG_PPC_EVENSR_PCE;	// 実行PCイベント設定ビット以外を0マスク
		dwEmptyBit |= dwOcdData[0];
	}
	// RevRxNo131101-006 Append End

	// 空き番号の検索
	// V.1.02 No.10 Class2 イベント対応 Modify Line
	for (i=0;i< pMcuInfo->wPCEvent; i++){
		if (((dwEmptyBit >> i) & 0x00000001) == FALSE) {
			(*pdwEvNum) = i;		// 空き番号あり	// RevRxNo131101-006 Modify Line
			*pbFindFlg = TRUE;
			return ferr;
		}
	}

	return ferr;
}

//==============================================================================
/**
 * 命令実行イベントを設定する。
 * @param  dwEvNum     イベント番号
 * @param  madrSetAddr 設定アドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetExecPcEv(DWORD dwEvNum, MADDR madrSetAddr)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwSetAddr[1];
	BYTE*						pbySetAddr;

	// レジスタ設定
	eAccessSize = MLWORD_ACCESS;
	dwSetAddr[0] = madrSetAddr;
	pbySetAddr = reinterpret_cast<BYTE*>(dwSetAddr);

	// レジスタ設定
	madrWriteAddr = OCD_REG_EV_EVEPCA + (dwEvNum * 4);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbySetAddr);

	return ferr;
}

//==============================================================================
/**
 * オペランドアクセスイベントを設定する。
 * @param  dwEvNum     イベント番号
 * @param  ev 設定情報
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetOpeAccEv(DWORD dwEvNum, FFWRX_EV_DATA* ev)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwJoin[1];
	BYTE*						pbyJoin;
	DWORD						dwDataMask[1];
	BYTE*						pbyDataMask;
	DWORD						dwAddrStart[1];
	BYTE*						pbyAddrStart;
	DWORD						dwAddrEnd[1];
	BYTE*						pbyAddrEnd;
	DWORD						dwDataValue[1];
	BYTE*						pbyDataValue;
	// RevRxNo130301-001 Append Start
	DWORD						dwBusSel[1];
	BYTE*						pbyBusSel;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;
	// RevRxNo130301-001 Append End

	pFwCtrl = GetFwCtrlData();	// RevRxNo130301-001 Append Line

	// レジスタ設定
	eAccessSize = MLWORD_ACCESS;
	// 開始アドレス
	madrWriteAddr = OCD_REG_EV_EVEOPA + (dwEvNum * 4);
	dwAddrStart[0] = ev->evOpc.dwAddrStart;
	pbyAddrStart = reinterpret_cast<BYTE*>(dwAddrStart);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyAddrStart);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
		
	// 終了アドレス
	madrWriteAddr = OCD_REG_EV_EVEOPAM + (dwEvNum * 4);
	dwAddrEnd[0] = ev->evOpc.dwAddrEnd;
	pbyAddrEnd = reinterpret_cast<BYTE*>(dwAddrEnd);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyAddrEnd);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// データ
	madrWriteAddr = OCD_REG_EV_EVEOPD + (dwEvNum * 4);
	dwDataValue[0] = ev->evOpc.dwDataValue;
	pbyDataValue = reinterpret_cast<BYTE*>(dwDataValue);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyDataValue);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// データサイズによってマスク値を変更
	if (ev->evOpc.byDataSize == EV_DATASIZE_BYTE) {
		dwDataMask[0] = (ev->evOpc.dwDataMask & 0x000000FF);
	} else if (ev->evOpc.byDataSize == EV_DATASIZE_WORD) {
		dwDataMask[0] = (ev->evOpc.dwDataMask & 0x0000FFFF);
	} else {
		dwDataMask[0] = ev->evOpc.dwDataMask;
	}

	// データマスク値
	madrWriteAddr = OCD_REG_EV_EVEOPDM + (dwEvNum * 4);
	pbyDataMask = reinterpret_cast<BYTE*>(dwDataMask);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyDataMask);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 検出条件
	madrWriteAddr = OCD_REG_EV_EVEOPCD + (dwEvNum * 4);
	// データ連結処理
	//    9   8   7   6   5   4   3   2   1   0
	//  ┌───┬─┬─┬─┬─┬───┬───┐
	//  │      │0 │0 │0 │  │      │      │
	//  └───┴─┴─┴─┴─┴───┴───┘
	//  b9,8:データサイズ
	//  b4　:データ比較条件
	//  b3,2:アドレス比較条件
	//  b1,0:アクセス属性
	dwJoin[0] = ((((DWORD)ev->evOpc.byDataSize & 0x00000003) << 8) |
				(((DWORD)ev->evOpc.byDataCmp & 0x00000001) << 4) |
				(((DWORD)ev->evOpc.byAddrCmp & 0x00000003) << 2) |
				((DWORD)ev->evOpc.byAccess & 0x00000003));
	pbyJoin = reinterpret_cast<BYTE*>(dwJoin);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyJoin);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130301-001 Append Start
	// バス選択
	if (pFwCtrl->bEvOaBusSelEna == TRUE) {
		madrWriteAddr = OCD_REG_EV_EVEOPSB + (dwEvNum * 4);
		dwBusSel[0] = ev->evOpc.dwBusSel;
		pbyBusSel = reinterpret_cast<BYTE*>(dwBusSel);
		ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyBusSel);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo130301-001 Append End

	return FFWERR_OK;
}

//==============================================================================
/**
 * イベントカウント機能を設定する。
 * @param  evcnt イベントカウント情報格納ポインタ
 * @retval FFWエラーコード
 */
//==============================================================================
// RevNo110825-01 Modify Line
FFWERR SetEvCnt(const FFWRX_EVCNT_DATA* evcnt)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwWorkCnt[1];
	BYTE*						pbyWorkCnt;
	DWORD						dwJoin[1];
	BYTE*						pbyJoin;
	// RevNo110825-01 Append Start
	DWORD						dwMode;
	DWORD						dwEvNum = 0;
	// RevNo110825-01 Append End

	// レジスタ設定
	eAccessSize = MLWORD_ACCESS;

	// イベント成立回数
	// イベント回数は1～0x100まで設定可能　0x100の場合は0を入れるので、0xFFでマスク
	dwWorkCnt[0] = (evcnt->dwCount & 0x000000ff);

	madrWriteAddr = OCD_REG_EV_EVECNT;
	pbyWorkCnt = reinterpret_cast<BYTE*>(dwWorkCnt);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkCnt);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// イベント選択
	madrWriteAddr = OCD_REG_EV_EVECNTSEL;
	// RevNo110825-01 Append Start
	//EVECNTSELへの設定
	if(evcnt->bEnable == FALSE){
	//イベント設定しない場合
		dwMode =EVCNT_NON;
	} else {
		if((FFWRX_PE0<=evcnt->eEvNo) && (evcnt->eEvNo<FFWRX_DE0)){
			//実行PCイベントに設定する場合
			dwMode =EVCNT_ADDR;
			dwEvNum = (DWORD)(evcnt->eEvNo);
		} else {
			//オペランドイベントに設定する場合
			dwMode =EVCNT_OPC;
			dwEvNum = (DWORD)(evcnt->eEvNo-FFWRX_DE0);
		}
	}
	// RevNo110825-01 Append End
	// データ連結処理
	//    9   8   7   6   5   4   3   2   1   0
	//  ┌───┬─┬─┬─┬─────────┐
	//  │      │0 │0 │0 │                  │
	//  └───┴─┴─┴─┴─────────┘
	//  b9,8:イベント種類
	//  b4,0:イベント番号
	dwJoin[0] = (((dwMode & MSK_EVCNT_MODE2CNTSEL) << SIFT_EVCNT_MODE2CNTDEL) | (dwEvNum & OCD_REG_EV_EVECNTSEL_SEL));
	pbyJoin = reinterpret_cast<BYTE*>(dwJoin);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyJoin);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	return FFWERR_OK;
}

//==============================================================================
/**
 * トレース/パフォーマンスのトリガ設定をする。
 * @param byEvKind トリガの種類  bit0:トレース　bit1:PPC0 bit2:PPC1
 * @param evTrg トリガ設定情報
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetTrgEv(BYTE byEvKind, FFWRX_TRG_DATA evTrg)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwWorkTrg[1];
	BYTE*						pbyWorkTrg;
	DWORD						dwSetPf0,dwSetPf1;
	FFWRX_TRG_DATA*				EvTrgData_RX;

	EvTrgData_RX = GetEvTrgInfo();

	// レジスタ設定用
	eAccessSize = MLWORD_ACCESS;
	pbyWorkTrg = reinterpret_cast<BYTE*>(dwWorkTrg);

	// 変更対象イベントがトレースの場合
	if ((byEvKind & EVTRG_EVKIND_TRC) == EVTRG_EVKIND_TRC) {
		// トレース用トリガの設定
		dwWorkTrg[0] = evTrg.dwTrcTrg;

		madrWriteAddr = OCD_REG_EV_EVETTRG;
		ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkTrg);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// 変更対象イベントがパフォーマンス0の場合
	if ((byEvKind & EVTRG_EVKIND_PPC_CH0) == EVTRG_EVKIND_PPC_CH0) {
		dwSetPf0 = EvTrgData_RX->dwPerfTrg & CLR_EVTRG_PPC_CH0;		// 前の設定値の設定対象のビットOFF
		dwWorkTrg[0] = dwSetPf0 | evTrg.dwPerfTrg;	// 今回の設定値を格納
		madrWriteAddr = OCD_REG_EV_EVEPTRG;
		ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkTrg);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// 変更対象イベントがパフォーマンス1の場合
	if ((byEvKind & EVTRG_EVKIND_PPC_CH1) == EVTRG_EVKIND_PPC_CH1) {
		dwSetPf1 = EvTrgData_RX->dwPerfTrg & CLR_EVTRG_PPC_CH1;		// 前の設定値の設定対象のビットOFF
		dwWorkTrg[0] = dwSetPf1 | evTrg.dwPerfTrg;	// 今回の設定値を格納
		madrWriteAddr = OCD_REG_EV_EVEPTRG;
		ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkTrg);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	return FFWERR_OK;
}

// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append End
//==============================================================================
/**
 * イベント関連レジスタを初期化する。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrEvReg(void)
{
	FFWERR	ferr;
	// V.1.02 No.10 Class2 イベント対応 Modify Line
	int		i,j;
	DWORD	dwJoin[1];
	BYTE*	pbyJoin;
	DWORD	dwDataSize;
	DWORD	dwDataCmp;
	DWORD	dwAddrCmp;
	DWORD	dwAccess;
	DWORD	dwOcdData[1];
	BYTE*	pbyOcdData;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	// V.1.02 No.10 Class2 イベント対応 Append Start
	FFWRX_MCUINFO_DATA* pMcuInfo;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	//イベント情報取得
	pMcuInfo = GetMcuInfoDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// V.1.02 No.10 Class2 イベント対応 Append End
	eAccessSize = MLWORD_ACCESS;
	pbyOcdData = reinterpret_cast<BYTE*>(dwOcdData);
	pbyJoin = reinterpret_cast<BYTE*>(dwJoin);

	// 実行PCイベントiアドレス（i=0～7）
	// V.1.02 No.10 Class2 イベント対応 Modify Line
	for (i=0;i< pMcuInfo->wPCEvent; i++){
		dwOcdData[0] = INIT_OCD_REG_EV_EVEPCA;
		ferr = SetMcuOCDReg((OCD_REG_EV_EVEPCA + (i * 4)),eAccessSize, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// オペランドアクセスイベントjアドレス（j=0～3）
	// V.1.02 No.10 Class2 イベント対応 Modify Line
	for (i=0;i< pMcuInfo->wOAEvent; i++){
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
			j = i+1;
		} else {
			j = i;
		}
		dwOcdData[0] = INIT_OCD_REG_EV_EVEOPA;
		ferr = SetMcuOCDReg((OCD_REG_EV_EVEOPA + (j * 4)),eAccessSize, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// オペランドアクセスイベントjアドレスマスク（j=0～3）
	// V.1.02 No.10 Class2 イベント対応 Modify Start
	for (i=0;i< pMcuInfo->wOAEvent; i++){
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
			j = i+1;
		} else {
			j = i;
		}
		dwOcdData[0] = INIT_OCD_REG_EV_EVEOPAM;
		ferr = SetMcuOCDReg((OCD_REG_EV_EVEOPAM + (j * 4)),eAccessSize, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// オペランドアクセスイベントjデータ（j=0～3）
	for (i=0;i< pMcuInfo->wOAEvent; i++){
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
			j = i+1;
		} else {
			j = i;
		}
		dwOcdData[0] = INIT_OCD_REG_EV_EVEOPD;
		ferr = SetMcuOCDReg((OCD_REG_EV_EVEOPD + (j * 4)),eAccessSize, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// オペランドアクセスイベントjデータマスク（j=0～3）
	for (i=0;i< pMcuInfo->wOAEvent; i++){
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
			j = i+1;
		} else {
			j = i;
		}
		dwOcdData[0] = INIT_OCD_REG_EV_EVEOPDM;
		ferr = SetMcuOCDReg((OCD_REG_EV_EVEOPDM + (j * 4)),eAccessSize, pbyOcdData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// オペランドアクセスイベントj検出条件指定（j=0～3）
	for (i=0;i< pMcuInfo->wOAEvent; i++){
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
			j = i+1;
		} else {
			j = i;
		}
		dwDataSize = EV_DATASIZE_BYTE;
		dwDataCmp = EV_CMPMASK_AGREE;
		dwAddrCmp = EV_CMPMASK_AGREE;
		dwAccess = EV_ACCESS_W;
		dwJoin[0] = (((dwDataSize & 0x00000003) << 8) |
					((dwDataCmp & 0x00000001) << 4) |
					((dwAddrCmp & 0x00000003) << 2) |
					(dwAccess & 0x00000003));
		ferr = SetMcuOCDReg((OCD_REG_EV_EVEOPCD + (j * 4)) ,eAccessSize,pbyJoin);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// V.1.02 No.10 Class2 イベント対応 Modify End

	// RevRxNo130301-001 Append Start
	// オペランドアクセスイベントjバス選択（j=0～3）
	if (pFwCtrl->bEvOaBusSelEna == TRUE) {
		for (i=0;i< pMcuInfo->wOAEvent; i++){
			if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
				j = i+1;
			} else {
				j = i;
			}
			dwOcdData[0] = INIT_OCD_REG_EV_EVEOPSB;
			ferr = SetMcuOCDReg((OCD_REG_EV_EVEOPSB + (j * 4)),eAccessSize, pbyOcdData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// RevRxNo130301-001 Append End

	return FFWERR_OK;
}

//==============================================================================
/**
 * ブレーク関連レジスタを初期化する。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrBrkReg(void)
{
	FFWERR	ferr;
	DWORD	dwOcdData[1];
	BYTE*	pbyOcdData;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ

	eAccessSize = MLWORD_ACCESS;
	pbyOcdData = reinterpret_cast<BYTE*>(dwOcdData);

	// 実行前PCブレークイベント動作許可
	dwOcdData[0] = INIT_OCD_REG_EV_EVEPCPREE;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPCPREE ,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 実行PC通過イベント動作許可
	dwOcdData[0] = INIT_OCD_REG_EV_EVEPCPE;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPCPE ,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 実行PC通過イベント成立結果
	dwOcdData[0] = INIT_OCD_REG_EV_EVEPCPFLG;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPCPFLG ,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// オペランドアクセスイベント動作許可
	dwOcdData[0] = INIT_OCD_REG_EV_EVEOPE;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEOPE ,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// オペランドアクセスイベント成立結果
	dwOcdData[0] = INIT_OCD_REG_EV_EVEOPFLG;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEOPFLG ,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// イベント成立回数カウントii（ii=0）
	dwOcdData[0] = INIT_OCD_REG_EV_EVECNT;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVECNT ,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 成立回数指定イベント選択ii（ii=0）
	dwOcdData[0] = INIT_OCD_REG_EV_EVECNTSEL;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVECNTSEL,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// イベントブレーク/トレース開始イベント組合せ指定
	dwOcdData[0] = INIT_OCD_REG_EV_EVEBRKTSC;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEBRKTSC,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// イベントブレーク用実行PC通過イベント選択
	dwOcdData[0] = INIT_OCD_REG_EV_EVEBRKPCP;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEBRKPCP,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// イベントブレーク用オペランドアクセスイベント選択
	dwOcdData[0] = 0;
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEBRKOP,eAccessSize,pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * イベント組み合わせ/遷移回路初期化。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR InitEveBrk(void)
{
	FFWERR	ferr;
	DWORD	dwBuff[1];
	DWORD	dwWriteData[1];
	BYTE*	pbyWriteData;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	BYTE*	pbyOcdData;

	eAccessSize = MLWORD_ACCESS;
	pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);

	// イベントブレーク/トレース開始イベント組み合わせ指定レジスタ
	pbyOcdData = reinterpret_cast<BYTE*>(dwBuff);
	ferr = GetMcuOCDReg(OCD_REG_EV_EVEBRKTSC,eAccessSize,OCD_ACCESS_COUNT, pbyOcdData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 初期化開始
	dwWriteData[0] = (DWORD)(dwBuff[0] | OCD_REG_EV_EVEBRKTSC_BTCTC);
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEBRKTSC,eAccessSize,pbyWriteData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 初期化終了
	dwWriteData[0] = (DWORD)(dwBuff[0] & ~OCD_REG_EV_EVEBRKTSC_BTCTC);
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEBRKTSC,eAccessSize,pbyWriteData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * 実行前PCブレークイベントの動作許可設定を行う。
 * @param  evBrk           ブレークイベント情報を格納するFFWRX_COMB_BRK 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetEvPreBreakOn(FFWRX_COMB_BRK* evBrk)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwWorkBrk[1];			// レジスタ書き込み時の退避用
	BYTE*						pbyWorkBrk;

	//RevNo120131-001 Delete
	// 内部変数にセット　内部変数操作はCOMB処理内でのみ実施するため、ここではセットしない

	// レジスタ設定用
	eAccessSize = MLWORD_ACCESS;

	// レジスタ設定
	// 実行前PCブレーク
	dwWorkBrk[0] = evBrk->dwPreBrk;					// RevNo120131-001 Modify Line
	madrWriteAddr = OCD_REG_EV_EVEPCPREE;
	pbyWorkBrk = reinterpret_cast<BYTE*>(dwWorkBrk);
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkBrk);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * 全イベントレジスタ関係の再設定を行う。(ブレークも含む)
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
// V.1.02 No.10 Modify Start
//RevNo100715-014 Modify Start
FFWERR SetEvRegDataAll(void)
{
	FFWERR						ferr;
	DWORD						dwEvNo;
	DWORD						dwData;
	int							i;
	DWORD						dwAddrCnt,dwDataCnt,dwCombi_RX;
	DWORD						madrStartAddr[EV_SET_REGNUM];
	BYTE						*pbyWriteData;
	DWORD						dwAreaNum,dwAccessCount;
	BOOL						bSameAccessSize,bSameAccessCount,bSameWriteData;	//すべてLWORDアクセス
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	enum FFWENM_VERIFY_SET eVerify = VERIFY_OFF;
	FFW_VERIFYERR_DATA VerifyErr;
	BYTE						byEndian = ENDIAN_LITTLE;
	DWORD						dwRegCnt;
	// RevNo110506-002 Append Line
	int							nDeStart,nDeEnd;
	FFWRX_COMB_TRC*				EvCombTrc_RX;
	FFWRX_COMB_PERFTIME*				EvCombPerf_RX;
	FFWRX_TRG_DATA*				EvTrgData_RX;
	DWORD						dwPreBrk, dwExecPcSet,dwOpcSet;
	FFWRX_EV_ADDR				EvAddr;
	FFWRX_EV_OPC*				EvOpc;
	DWORD						dwEvNum;
	FFWRX_EVCNT_DATA*			EvCount_RX;
	DWORD						dwCombi;
	FFWRX_COMB_BRK*				EvCombBrk_RX;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	//内部管理変数取得
	EvCombBrk_RX = GetEvCombBrkInfo();
	EvCombTrc_RX = GetEvCombTrcInfo();
	EvCombPerf_RX = GetEvCombPerfInfo();
	EvTrgData_RX = GetEvTrgInfo();
	GetRegEvEnable(&dwPreBrk, &dwExecPcSet, &dwOpcSet);

	pbyWriteData = new BYTE [EV_SET_REGNUM*4];
	memset(pbyWriteData, 0, EV_SET_REGNUM*4);

	dwAddrCnt = 0;
	dwDataCnt = 0;
	dwRegCnt = 0;

	//実行前PCブレークイベント動作許可
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPCPREE;
	dwData = EvCombBrk_RX->dwPreBrk;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;

	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	
	// 実行PC通過イベント動作許可
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPCPE;
	dwData = dwExecPcSet;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;

	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// 実行PCイベントアドレス設定
	for (i=0;i<(int)(pMcuInfo->wPCEvent);i++) {
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPCA+(i*4);
		GetEvAddrInfo((DWORD)i, &EvAddr);
		dwData = EvAddr.dwAddr;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;

		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}

	// オペランドアクセスイベント動作許可
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPE;
	dwData = dwOpcSet;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;
	
	// RevNo110506-002 Modify Start
	// Class2の場合はDE1からはじめる
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
		nDeStart = 1;
	} else {
		nDeStart = 0;
	}
	nDeEnd = (int)(pMcuInfo->wOAEvent) + nDeStart;

	// オペランドイベント設定
	for (i=nDeStart;i<nDeEnd;i++) {
	// RevNo110506-002 Modify End
		//オペランドアドレス
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPA+(i*4);
		switch(i){
		case DE0:
			dwEvNum = DE0;
			break;
		case DE1:
			dwEvNum = DE1;
			break;
		case DE2:
			dwEvNum = DE2;
			break;
		case DE3:
			dwEvNum = DE3;
			break;
		default:
			dwEvNum = (DWORD)i;
			break;
		}
		EvOpc = GetEvOpc(dwEvNum);
		dwData = EvOpc->dwAddrStart;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//オペランドマスクもしくは終了
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPAM+(i*4);
		dwData = EvOpc->dwAddrEnd;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//オペランドデータ
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPD+(i*4);
		dwData = EvOpc->dwDataValue;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//オペランドデータマスク
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPDM+(i*4);
		dwData = EvOpc->dwDataMask;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//オペランド条件指定
		dwData = ((((DWORD)EvOpc->byDataSize & 0x00000003) << 8) |
				(((DWORD)EvOpc->byDataCmp & 0x00000001) << 4) |
				(((DWORD)EvOpc->byAddrCmp & 0x00000003) << 2) |
				((DWORD)EvOpc->byAccess & 0x00000003));
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPCD+(i*4);
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		// RevRxNo130301-001 Append Start
		// オペランドアクセスイベントバス選択
		if (pFwCtrl->bEvOaBusSelEna == TRUE) {
			madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPSB+(i*4);
			dwData = EvOpc->dwBusSel;
			memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
			dwRegCnt++;
			dwAddrCnt++;
			dwDataCnt = dwDataCnt+4;
		}
		// RevRxNo130301-001 Append End

	}

	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		//イベント回数指定
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVECNT;
		EvCount_RX = GetEvCntInfo();
		dwData = EvCount_RX->dwCount;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//イベント回数指定あり
		dwEvNo = 0;
		if(EvCount_RX->bEnable == TRUE){
			if(EvCount_RX->eEvNo >= 0x10 ) {
				//オペランド
				dwEvNo = OCD_REG_EV_EVECNTSEL_CNTK_OPC | (EvCount_RX->eEvNo & 0xF);
			} else {
				//命令実行
				dwEvNo = OCD_REG_EV_EVECNTSEL_CNTK_EXEC | (EvCount_RX->eEvNo & 0xF);
			}
		}
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVECNTSEL;
		dwData = dwEvNo;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}

	// 組み合わせ指定
	dwCombi = GetCombiInfo();
	switch(dwCombi) {
	case COMB_OR:
		dwCombi_RX = BRKTSC_OR;
		break;
	case COMB_BRK_AND:
		dwCombi_RX = BRKTSC_BRK_AND;
		break;
	case COMB_BRK_STATE:
		dwCombi_RX = BRKTSC_BRK_STATE;
		break;
	case COMB_TRC_AND:
		dwCombi_RX = BRKTSC_TRC_AND;
		break;
	case COMB_TRC_STATE:
		dwCombi_RX = BRKTSC_TRC_STATE;
		break;
	default:
		dwCombi_RX = BRKTSC_OR;
		break;
	}

	// イベントブレーク/トレース開始イベント組み合わせ指定レジスタ
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEBRKTSC;
	dwData = dwCombi_RX;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// 実行PC通過ブレークイベント許可
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEBRKPCP;
	dwData = EvCombBrk_RX->dwBrkpe;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// オペランドブレークイベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEBRKOP;
	dwData = EvCombBrk_RX->dwBrkde;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース開始用実行PC通過イベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETSPCP;
	dwData = EvCombTrc_RX->dwStartpe;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース開始用オペランドアクセスイベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETSOP;
	dwData = EvCombTrc_RX->dwStartde;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース終了用実行PC通過イベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETEPCP;
	dwData = EvCombTrc_RX->dwEndpe;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース終了用オペランドアクセスイベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETEOP;
	dwData = EvCombTrc_RX->dwEndde;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース抽出用オペランドアクセスイベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETXOP;
	dwData = EvCombTrc_RX->dwPickupde;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース用イベントトリガ出力制御
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETTRG;
	dwData = EvTrgData_RX->dwTrcTrg;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// パフォーマンスイベント選択
	for (i=0;i<(int)(pMcuInfo->wPPC);i++) {
		// パフォーマンス開始用実行PC通過イベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPSPCP0+(i*4);
		dwData = EvCombPerf_RX->dwStartpe[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
		// パフォーマンス開始用オペランドアクセスイベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPSOP0+(i*4);
		dwData = EvCombPerf_RX->dwStartde[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
		// パフォーマンス終了用実行PC通過イベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPEPCP0+(i*4);
		dwData = EvCombPerf_RX->dwEndpe[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
		// パフォーマンス終了用オペランドアクセスイベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPEOP0+(i*4);
		dwData = EvCombPerf_RX->dwEndde[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}

	// RevRxNo121026-001 Append Line
	if ((pMcuInfo->wPPC != RX_PPC_NON)) {	// PPCが実装されているMCUの場合
		// パフォーマンス用イベントトリガ出力制御
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPTRG;
		dwData = EvTrgData_RX->dwPerfTrg;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
	}

	dwAreaNum = dwRegCnt;	//設定イベント個数
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

	// RevNo110302-01 Modify Line
	return ferr;
}
//RevNo100715-014 Modify End

//==============================================================================
/**
 * 実行前にイベントの再設定を行う。(ブレークは除く)
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
// V.1.02 No.10 Modify Start
//RevNo100715-014 Modify Start
FFWERR SetEvRegDataBeforeGo(void)
{
	FFWERR						ferr;
	DWORD						dwEvNo;
	DWORD						dwData;
	int							i;
	DWORD						dwAddrCnt,dwDataCnt,dwCombi_RX;
	DWORD						madrStartAddr[EV_SET_REGNUM];
	BYTE						*pbyWriteData;
	DWORD						dwAreaNum,dwAccessCount;
	BOOL						bSameAccessSize,bSameAccessCount,bSameWriteData;	//すべてLWORDアクセス
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	enum FFWENM_VERIFY_SET eVerify = VERIFY_OFF;
	FFW_VERIFYERR_DATA VerifyErr;
	BYTE						byEndian = ENDIAN_LITTLE;
	DWORD						dwRegCnt;
	// RevNo110506-002 Append Line
	int							nDeStart,nDeEnd;
	FFWRX_COMB_TRC*				EvCombTrc_RX;
	FFWRX_COMB_PERFTIME*				EvCombPerf_RX;
	FFWRX_TRG_DATA*				EvTrgData_RX;
	DWORD						dwPreBrk, dwExecPcSet,dwOpcSet;
	FFWRX_EV_ADDR				EvAddr;
	FFWRX_EV_OPC*				EvOpc;
	DWORD						dwEvNum;
	FFWRX_EVCNT_DATA*			EvCount_RX;
	DWORD						dwCombi;
	BYTE						byEvRegNum;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	//内部管理変数取得
	EvCombTrc_RX = GetEvCombTrcInfo();
	EvCombPerf_RX = GetEvCombPerfInfo();
	EvTrgData_RX = GetEvTrgInfo();
	GetRegEvEnable(&dwPreBrk, &dwExecPcSet, &dwOpcSet);

	byEvRegNum = EV_SET_REGNUM-BRK_SET_REGNUM;

	pbyWriteData = new BYTE [byEvRegNum*4];
	memset(pbyWriteData, 0, byEvRegNum*4);

	dwAddrCnt = 0;
	dwDataCnt = 0;
	dwRegCnt = 0;

	// 実行PC通過イベント動作許可
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPCPE;
	dwData = dwExecPcSet;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;

	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// 実行PCイベントアドレス設定
	for (i=0;i<(int)(pMcuInfo->wPCEvent);i++) {
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPCA+(i*4);
		GetEvAddrInfo((DWORD)i, &EvAddr);
		dwData = EvAddr.dwAddr;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;

		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}

	// オペランドアクセスイベント動作許可
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPE;
	dwData = dwOpcSet;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;
	
	// RevNo110506-002 Modify Start
	// Class2の場合はDE1からはじめる
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
		nDeStart = 1;
	} else {
		nDeStart = 0;
	}
	nDeEnd = (int)(pMcuInfo->wOAEvent) + nDeStart;

	// オペランドイベント設定
	for (i=nDeStart;i<nDeEnd;i++) {
	// RevNo110506-002 Modify End
		//オペランドアドレス
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPA+(i*4);
		switch(i){
		case DE0:
			dwEvNum = DE0;
			break;
		case DE1:
			dwEvNum = DE1;
			break;
		case DE2:
			dwEvNum = DE2;
			break;
		case DE3:
			dwEvNum = DE3;
			break;
		default:
			dwEvNum = (DWORD)i;
			break;
		}
		EvOpc = GetEvOpc(dwEvNum);
		dwData = EvOpc->dwAddrStart;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//オペランドマスクもしくは終了
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPAM+(i*4);
		dwData = EvOpc->dwAddrEnd;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//オペランドデータ
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPD+(i*4);
		dwData = EvOpc->dwDataValue;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//オペランドデータマスク
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPDM+(i*4);
		dwData = EvOpc->dwDataMask;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//オペランド条件指定
		dwData = ((((DWORD)EvOpc->byDataSize & 0x00000003) << 8) |
				(((DWORD)EvOpc->byDataCmp & 0x00000001) << 4) |
				(((DWORD)EvOpc->byAddrCmp & 0x00000003) << 2) |
				((DWORD)EvOpc->byAccess & 0x00000003));
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPCD+(i*4);
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		// RevRxNo130301-001 Append Start
		// オペランドアクセスイベントバス選択
		if (pFwCtrl->bEvOaBusSelEna == TRUE) {
			madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEOPSB+(i*4);
			dwData = EvOpc->dwBusSel;
			memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
			dwRegCnt++;
			dwAddrCnt++;
			dwDataCnt = dwDataCnt+4;
		}
		// RevRxNo130301-001 Append End

	}

	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		//イベント回数指定
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVECNT;
		EvCount_RX = GetEvCntInfo();
		dwData = EvCount_RX->dwCount;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		//イベント回数指定あり
		dwEvNo = 0;
		if(EvCount_RX->bEnable == TRUE){
			if(EvCount_RX->eEvNo >= 0x10 ) {
				//オペランド
				dwEvNo = OCD_REG_EV_EVECNTSEL_CNTK_OPC | (EvCount_RX->eEvNo & 0xF);
			} else {
				//命令実行
				dwEvNo = OCD_REG_EV_EVECNTSEL_CNTK_EXEC | (EvCount_RX->eEvNo & 0xF);
			}
		}
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVECNTSEL;
		dwData = dwEvNo;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}

	// 組み合わせ指定
	dwCombi = GetCombiInfo();
	switch(dwCombi) {
	case COMB_OR:
		dwCombi_RX = BRKTSC_OR;
		break;
	case COMB_BRK_AND:
		dwCombi_RX = BRKTSC_BRK_AND;
		break;
	case COMB_BRK_STATE:
		dwCombi_RX = BRKTSC_BRK_STATE;
		break;
	case COMB_TRC_AND:
		dwCombi_RX = BRKTSC_TRC_AND;
		break;
	case COMB_TRC_STATE:
		dwCombi_RX = BRKTSC_TRC_STATE;
		break;
	default:
		dwCombi_RX = BRKTSC_OR;
		break;
	}

	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEBRKTSC;
	dwData = dwCombi_RX;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース開始用実行PC通過イベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETSPCP;
	dwData = EvCombTrc_RX->dwStartpe;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース開始用オペランドアクセスイベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETSOP;
	dwData = EvCombTrc_RX->dwStartde;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース終了用実行PC通過イベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETEPCP;
	dwData = EvCombTrc_RX->dwEndpe;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース終了用オペランドアクセスイベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETEOP;
	dwData = EvCombTrc_RX->dwEndde;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース抽出用オペランドアクセスイベント選択
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETXOP;
	dwData = EvCombTrc_RX->dwPickupde;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// トレース用イベントトリガ出力制御
	madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVETTRG;
	dwData = EvTrgData_RX->dwTrcTrg;
	memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwDataCnt = dwDataCnt+4;

	// パフォーマンスイベント選択
	for (i=0;i<(int)(pMcuInfo->wPPC);i++) {
		// パフォーマンス開始用実行PC通過イベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPSPCP0+(i*4);
		dwData = EvCombPerf_RX->dwStartpe[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
		// パフォーマンス開始用オペランドアクセスイベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPSOP0+(i*4);
		dwData = EvCombPerf_RX->dwStartde[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
		// パフォーマンス終了用実行PC通過イベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPEPCP0+(i*4);
		dwData = EvCombPerf_RX->dwEndpe[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
		// パフォーマンス終了用オペランドアクセスイベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPEOP0+(i*4);
		dwData = EvCombPerf_RX->dwEndde[i];
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}

	// RevRxNo121026-001 Append Line
	if ((pMcuInfo->wPPC != RX_PPC_NON)) {	// PPCが実装されているMCUの場合
		// パフォーマンス用イベントトリガ出力制御
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPTRG;
		dwData = EvTrgData_RX->dwPerfTrg;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
	}

	dwAreaNum = dwRegCnt;	//設定イベント個数
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

	// RevNo110302-01 Modify Line
	return ferr;
}
//RevNo100715-014 Modify End
//==============================================================================
/**
 * 実行前にブレークの再設定を行う。
 * @param  byOcdBrkMode OCDBRK_PRE(bit0):1 実行前PCブレーク設定 OCDBRK_EV(bit1):1 イベントブレーク設定
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetOcdBrkRegData(BYTE byOcdBrkMode)
{
	FFWERR						ferr;
	DWORD						dwData;
	DWORD						madrStartAddr[BRK_SET_REGNUM];
	BYTE						*pbyWriteData;
	DWORD						dwAreaNum,dwAccessCount;
	BOOL						bSameAccessSize,bSameAccessCount,bSameWriteData;	//すべてLWORDアクセス
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	enum FFWENM_VERIFY_SET eVerify = VERIFY_OFF;
	FFW_VERIFYERR_DATA VerifyErr;
	BYTE						byEndian = ENDIAN_LITTLE;
	DWORD						dwRegCnt,dwAddrCnt,dwDataCnt;
	// RevNo110506-002 Append Line
	FFWRX_COMB_BRK*				EvCombBrk_RX;

	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	//内部管理変数取得
	EvCombBrk_RX = GetEvCombBrkInfo();

	pbyWriteData = new BYTE [BRK_SET_REGNUM*4];
	memset(pbyWriteData, 0, BRK_SET_REGNUM*4);

	dwAddrCnt = 0;
	dwDataCnt = 0;
	dwRegCnt = 0;

	//実行前PCブレークイベント動作許可
	if((byOcdBrkMode & OCDBRK_PRE) == OCDBRK_PRE){
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEPCPREE;
		dwData = EvCombBrk_RX->dwPreBrk;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;

		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}

	if((byOcdBrkMode & OCDBRK_EV) == OCDBRK_EV){
		// 実行PC通過ブレークイベント許可
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEBRKPCP;
		dwData = EvCombBrk_RX->dwBrkpe;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;

		// オペランドブレークイベント選択
		madrStartAddr[dwAddrCnt] = OCD_REG_EV_EVEBRKOP;
		dwData = EvCombBrk_RX->dwBrkde;
		memcpy(&pbyWriteData[dwDataCnt], &dwData, sizeof(DWORD));
		dwRegCnt++;
		dwAddrCnt++;
		dwDataCnt = dwDataCnt+4;
	}

	dwAreaNum = dwRegCnt;	//設定イベント個数
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
// V.1.02 No.10 Modify End

//==============================================================================
/**
 * イベント動作許可レジスタを設定する。
 * @param  dwPreBrk     実行前PCブレークイベント動作許可レジスタへの設定値
 * @param  dwExecPcSet  実行PC通過イベント動作許可レジスタへの設定値
 * @param  dwOpcSet     オペランドアクセスイベント動作許可レジスタへの設定値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetRegEvEnable(DWORD dwPreBrk, DWORD dwExecPcSet, DWORD dwOpcSet)
{
	FFWERR	ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrWriteAddr;		// 設定するレジスタのアドレス
	DWORD						dwWorkReg[1];			// レジスタ書き込み時の退避用
	BYTE*						pbyWorkReg;

	// レジスタ設定用
	eAccessSize = MLWORD_ACCESS;
	pbyWorkReg = reinterpret_cast<BYTE*>(dwWorkReg);

	// RevNo120606-002 Modify Start
	//RevNo100715-023 Modify Start
	// 実行前PCブレーク指定レジスタへの設定
	// 実行中のGBのときのみ反映させる→実行中の判定をこの関数内では実施しない。
	dwWorkReg[0] = dwPreBrk;
	madrWriteAddr = OCD_REG_EV_EVEPCPREE;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkReg);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	//RevNo100715-023 Modify End
	// RevNo120606-002 Modify End

	// 実行PCイベント指定レジスタへの設定
	dwWorkReg[0] = dwExecPcSet;
	madrWriteAddr = OCD_REG_EV_EVEPCPE;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkReg);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// オペランドアクセスイベント指定レジスタへの設定
	dwWorkReg[0] = dwOpcSet;
	madrWriteAddr = OCD_REG_EV_EVEOPE;
	ferr = SetMcuOCDReg(madrWriteAddr,eAccessSize,pbyWorkReg);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;

}
//==============================================================================
/**
 * パフォーマンス　イベントカウントで使用するイベント動作許可レジスタ(実行通過PCとオペランド)を設定する。 * @param  dwPpcPcEv	実行PC通過イベント動作許可レジスタへの設定値
 * @param  dwPpcPcEv    実行通過イベント動作許可レジスタへの設定値
 * @param  dwPpcOpcEv   オペランドアクセスイベント動作許可レジスタへの設定値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetEvPpcCntEvEnable(DWORD dwPpcPcEv, DWORD dwPpcOpcEv)
{
	FFWERR	ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	DWORD						dwExecPcSet[1];
	DWORD						dwOpcSet[1];
	BYTE*						pbyExecPcSet;
	BYTE*						pbyOpcSet;
	FFWRX_COMB_BRK* evBrk;
	FFWRX_COMB_TRC* evTrc;
	FFWRX_COMB_PERFTIME* evPerf;

	evBrk = GetEvCombBrkInfo();
	evTrc = GetEvCombTrcInfo();
	evPerf = GetEvCombPerfInfo();

	dwExecPcSet[0] = evBrk->dwBrkpe |
					evTrc->dwStartpe | evTrc->dwEndpe |
					evPerf->dwStartpe[0] | evPerf->dwEndpe[0] |
					evPerf->dwStartpe[1] | evPerf->dwEndpe[1] |
					dwPpcPcEv;

	dwOpcSet[0] = evBrk->dwBrkde |
				evTrc->dwStartde | evTrc->dwEndde | evTrc->dwPickupde |
				evPerf->dwStartde[0] | evPerf->dwEndde[0] |
				evPerf->dwStartde[1] | evPerf->dwEndde[1] |
				dwPpcOpcEv;

	// レジスタ設定用
	eAccessSize = MLWORD_ACCESS;
	pbyExecPcSet = reinterpret_cast<BYTE*>(dwExecPcSet);
	pbyOpcSet = reinterpret_cast<BYTE*>(dwOpcSet);

	// EVEPCPEレジスタ
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEPCPE,eAccessSize,pbyExecPcSet);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	//RevNo100715-014 Append Line
	// RevRxNo120713-001 Delete ここで内部管理変数に設定はしない。SetEveEXECPCSET(dwExecPcSet[0])削除

	// EVEOPEレジスタ
	ferr = SetMcuOCDReg(OCD_REG_EV_EVEOPE,eAccessSize,pbyOpcSet);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	//RevNo100715-014 Append Line
	// RevRxNo120713-001 Delete ここで内部管理変数に設定はしない。SetEveOPCSET(dwOpcSet[0])削除

	return FFWERR_OK;

}

//==============================================================================
/**
 * 実行通過PCイベントの状態確認
 * @param  dwExecPc		実行通過PCイベントの成立状態 0～7bit:PE0～7
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetEvExecPcCompInfo(DWORD* pdwExecPc)
{
	FFWERR	ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;		// 設定する領域へのアクセスサイズ
	BYTE*	pbyExecPc;
	DWORD	dwData[1];

	//PCイベント成立状態を取得
	pbyExecPc = reinterpret_cast<BYTE*>(dwData);
	ferr = GetMcuOCDReg(OCD_REG_EV_EVEPCPFLG,eAccessSize,OCD_ACCESS_COUNT,pbyExecPc);
	*pdwExecPc = dwData[0];

	return ferr;

}

//==============================================================================
/**
 * オペランドイベントの成立状態確認
 * @param  pdwOpc	オペランドイベントの成立状態 0～3bit:DE0～3
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetEvOpcCompInfo(DWORD* pdwOpc)
{
	FFWERR	ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize = MLWORD_ACCESS;		// 設定する領域へのアクセスサイズ
	BYTE*	pbyOpc;
	DWORD	dwData[1];

	//オペランドイベント成立状態を取得
	pbyOpc = reinterpret_cast<BYTE*>(dwData);
	ferr = GetMcuOCDReg(OCD_REG_EV_EVEOPFLG,eAccessSize,OCD_ACCESS_COUNT,pbyOpc);
	*pdwOpc = dwData[0];

	return ferr;

}


//=============================================================================
/**
 * イベント関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
void InitMcuRxData_Ev(void)
{

	int		i;

	// イベント無効化前のイベント設定管理変数
	for (i = 0; i < EV_ENABLE_REGNUM; i++) {
		s_dwEvSetData_RX[i] = 0;
	}

	// RevNo120131-001 Delete	s_dwBrkEvData_RX[i],s_dwPreBrkEvData_RX[i]初期化

	return;

}
