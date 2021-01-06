////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_ev.cpp
 * @brief RX共通イベント関連コマンドのソースファイル
 * @author RSO Y.Minami, H.Hashiguchi, Y.Kawakami
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/06/27
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo110825-01 2012/07/13 橋口
	GPB実行イベントへのパスカウント設定不具合対応
・RevNo120131-001 2012/07/13 橋口
	GPB実行中の実行前、イベントブレーク不具合対応
・RevNo120606-002 2012/07/13 橋口
	BMの実行前、イベントブレーク禁止中の実行中のイベントブレーク設定不具合対応
・RevRxNo140515-002 2014/06/26 川上
	実行中のCOMBコマンド、ブレークイベント変更以外の変更を行った場合の処理改善
*/
#include "dorx_ev.h"
#include "ffwrx_ev.h"
#include "mcurx_ev.h"
#include "mcurx_tim.h"
#include "errchk.h"
#include "prot_common.h"
#include "domcu_prog.h"

#include "ffwrx_tim.h"
#include "ffwmcu_brk.h"							// RevNo120606-002 Append Line

//==============================================================================
/**
 * イベントの設定行う。
 * @param dwMode 設定イベント種別
 * @param byNo 設定イベント番号
 * @param ev 設定するイベント情報を格納するFFWRX_EV_DATA 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetRXEV(DWORD dwMode, BYTE byNo, FFWRX_EV_DATA* ev)
{

	FFWERR						ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	MADDR						madrSetAddr;		// 設定するイベントアドレス
	BOOL						bRet;
	// RevNo120131-001	Append Start
	DWORD						dwGpbEvNo;
	DWORD						dwGpbEvAddr;
	// RevNo120131-001	Append End

	ProtInit();

	// プログラム実行状態チェック
	bRet = IsMcuRun();
	if (bRet) {			// 実行中の場合
	// RevNo120131-001 Append Start
		if(GetMcuRunCmd() == PROGCMD_GPB){
			//GPB実行の場合
			// GPB実行イベント情報取得
			GetGpbPcEv(&dwGpbEvNo,&dwGpbEvAddr);
			if((dwMode == EV_MODE_ADDR) && (dwGpbEvNo == byNo)){
				//変更イベントがGPB設定イベントの場合
				ferrEnd = ProtEnd();
				return FFWERR_OK;							// 正常終了
			}
		}
		// RevNo120131-001 Append End
		// 動作許可指定をいったん無効化
		ferr = SetEventEnable(FALSE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	// イベント設定
	if (dwMode == EV_MODE_ADDR) {
		// 命令実行
		madrSetAddr = ev->evAddr.dwAddr;
		ferr = SetExecPcEv((DWORD)byNo,madrSetAddr);
		if(ferr != FFWERR_OK){
			if (bRet) {							// プログラム実行中の場合
				// 動作許可指定を有効化に戻す
				ferr = SetEventEnable(TRUE);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
		}
	// オペランドアクセス
	} else if (dwMode == EV_MODE_OPC_RANGE || dwMode == EV_MODE_OPC_MASK) {
		// レジスタ設定
		ferr = SetOpeAccEv((DWORD)byNo,ev);
		if(ferr != FFWERR_OK){
			if (bRet) {							// プログラム実行中の場合
				// 動作許可指定を有効化に戻す
				ferr = SetEventEnable(TRUE);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
		}
	} else {		// 上記以外(通らないはず)
		if (bRet) {							// プログラム実行中の場合
			// 動作許可指定を有効化に戻す
			ferr = SetEventEnable(TRUE);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
	}

	if (bRet) {							// プログラム実行中の場合
		// イベント組み合わせ/遷移回路初期化
		ferr = InitEveBrk();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// RevNo120131-001 Modify Start
		// GPB以外の判定削除　イベント有効化は常に実施
		// イベント動作許可レジスタをSetEventEnable(FALSE)内で退避された値を戻す。
		// (実行前PCブレーク動作許可、実行PC通過イベント動作許可、オペランドアクセス動作許可)
		// SetEV()内では、イベント動作許可レジスタの変更がないため、SetEventEnable(FALSE)内で退避された値を戻す。
		ferr = SetEventEnable(TRUE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevNo120131-001 Modify End
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;

}


//==============================================================================
/**
 * イベントカウントの設定する。
 * @param dwMode 設定イベント種別
 * @param byNo 設定イベント番号
 * @param evcnt 設定するイベント情報を格納するFFWRX_EVCNT_DATA 構造体のアドレスを指定する。
  * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetRXEVCNT(DWORD dwMode, BYTE byNo, const FFWRX_EVCNT_DATA* evcnt)
{

	FFWERR						ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	BOOL						bRet;
	// RevNo120131-001	Append Start
	DWORD						dwGpbEvNo;
	DWORD						dwGpbEvAddr;
	// RevNo120131-001	Append End

	//ワーニング対策
	dwMode;
	byNo;

	ProtInit();

	// プログラム実行状態チェック
	bRet = IsMcuRun();
	if (bRet) {			// 実行中の場合
		// RevNo120131-001 Append Start
		if(GetMcuRunCmd() == PROGCMD_GPB){
			//GPB実行の場合
			// GPB実行イベント情報取得
			GetGpbPcEv(&dwGpbEvNo,&dwGpbEvAddr);
			if((dwMode == EV_MODE_ADDR) && (dwGpbEvNo == byNo)){
				//変更イベントがGPB設定イベントの場合
				ferrEnd = ProtEnd();
				return FFWERR_OK;							// 正常終了
			}
		}
		// RevNo120131-001 Append End
		// 動作許可指定をいったん無効化
		ferr = SetEventEnable(FALSE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	//イベントカウントの設定
	ferr = SetEvCnt(evcnt);							//RevNo110825-01 Modify Line
	if (ferr != FFWERR_OK) {
		if (bRet) {							// プログラム実行中の場合
			// 動作許可指定を有効化に戻す
			ferr = SetEventEnable(TRUE);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
		ferrEnd = ProtEnd();
		return ferr;
	}

	if (bRet) {			// 実行中の場合
		// イベント組み合わせ/遷移回路初期化
		ferr = InitEveBrk();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// RevNo120131-001 Modify Start
		// GPB以外の判定削除　イベント有効化は常に実施
		// イベント動作許可レジスタをSetEventEnable(FALSE)内で退避された値を戻す。
		// (実行前PCブレーク動作許可、実行PC通過イベント動作許可、オペランドアクセス動作許可)
		// SetEVCNT()内では、イベント動作許可レジスタの変更がないため、SetEventEnable(FALSE)内で退避された値を戻す。
		ferr = SetEventEnable(TRUE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevNo120131-001 Modify End
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;

}

//==============================================================================
/**
 * イベント組み合わせ情報を設定する
 * @param  byEvKind        変更対象のイベントを指定
 * @param  dwCombi         イベントの組合せを指定
 * @param  evBrk           ブレークイベント情報を格納するFFWRX_COMB_BRK 構造体のアドレスを指定する。
 * @param  evTrc           トレースイベント情報を格納するFFWRX_COMB_TRC 構造体のアドレスを指定する。
 * @param  evPerfTime      パフォーマンス計測イベント情報を格納するFFWRX_COMB_PERFTIME 構造体のアドレスを指定する。
 * @param  ePpcUsrCtrl     パフォーマンス計測資源に対するアクセスコントロールを設定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetRXCOMB(BYTE byEvKind, DWORD dwCombi, FFWRX_COMB_BRK* evBrk, FFWRX_COMB_TRC* evTrc,
										FFWRX_COMB_PERFTIME* evPerfTime, enum FFWRX_EVCOMBI_PPC_USR_CTRL ePpcUsrCtrl)
{

	FFWERR						ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	BOOL						bRet;
	DWORD						dwSetEvNum = 0;		// PPC回数計測
	DWORD						dwPpcPcEv = 0;		// PPC回数計測 PC通過イベント格納変数
	DWORD						dwPpcOpcEv = 0;		// PPC回数計測 オペランドアクセスイベント格納変数
	DWORD						dwExecPcSet;
	DWORD						dwOpcSet;
	DWORD						dwBMode,dwBFactor;	// ブレークモード取得 // RevNo120606-002 Append Line
	DWORD						dwPreBrk;			// RevNo120606-002 Append Line
	DWORD						dwGpbEvNo,dwGpbEvAddr; // RevNo120131-001 Append Line

	FFWRX_COMB_BRK*				EvCombBrk_RX;
	FFWRX_COMB_TRC*				EvCombTrc_RX;
	FFWRX_COMB_PERFTIME*		EvCombPerf_RX;

	ProtInit();

	EvCombBrk_RX = GetEvCombBrkInfo();
	EvCombTrc_RX = GetEvCombTrcInfo();
	EvCombPerf_RX = GetEvCombPerfInfo();

	// プログラム実行状態チェック
	bRet = IsMcuRun();
	if (bRet) {			// 実行中の場合
		// イベント動作許可レジスタを無効化　設定値を内部変数に退避
		// SetCOMB()内でイベント動作許可レジスタの変更が発生するため、SetEventEnable(TRUE)ではなくSetRegEvEnable()で設定を行う。
		ferr = SetEventEnable(FALSE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	// ブレークモード取得
	GetBmMode(&dwBMode,&dwBFactor);		// RevNo120606-002 Append Line

	//-----------------
	// 各設定を実施
	//-----------------
	// 変更イベントがブレークの場合
	if ((byEvKind & COMB_EVKIND_BRK) == COMB_EVKIND_BRK) {
		if (memcmp((const void*)EvCombBrk_RX, (const void*)&evBrk, sizeof(FFWRX_COMB_BRK)) != 0) {
			// RevNo120606-002	Modify Start
			if((bRet == FALSE) 
			|| ((bRet == TRUE) && (GetMcuRunCmd() == PROGCMD_GB) && ((dwBMode & BMODE_EB_BIT) == BMODE_EB_BIT))){
				//ブレーク中もしくは、ブレークイベント有効時のGB実行
				ferr = SetEvBreakData(evBrk);
				if (ferr != FFWERR_OK) {
					if (bRet) {			// 実行中の場合
						ferr = SetEventEnable(TRUE);
						if (ferr != FFWERR_OK) {
							ferrEnd = ProtEnd();
							return ferr;
						}
					}
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
			// RevNo120606-002 Modify End
		}
	}
	// 変更イベントがトレースの場合
	if ((byEvKind & COMB_EVKIND_TRC) == COMB_EVKIND_TRC) {
		if (memcmp((const void*)&EvCombTrc_RX, (const void*)&evTrc, sizeof(FFWRX_COMB_TRC)) != 0) {
			ferr = SetEvTraceData(evTrc);
			if (ferr != FFWERR_OK) {
				if (bRet) {			// 実行中の場合
					// 動作許可指定を有効化に戻す
					ferr = SetEventEnable(TRUE);
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
				}
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
	}
	// 変更イベントがパフォーマンスの場合
	if ((byEvKind & COMB_EVKIND_PPC) != 0) {
		// 変更イベントがパフォーマンス0の場合
		if ((byEvKind & COMB_EVKIND_PPC_CH0) == COMB_EVKIND_PPC_CH0) {
			if (memcmp((const void*)&EvCombPerf_RX, (const void*)&evPerfTime, sizeof(FFWRX_COMB_PERFTIME)) != 0) {
				ferr = SetEvPerfData(0, evPerfTime);
				if (ferr != FFWERR_OK) {
					if (bRet) {			// 実行中の場合
						// 動作許可指定を有効化に戻す
						ferr = SetEventEnable(TRUE);
						if (ferr != FFWERR_OK) {
							ferrEnd = ProtEnd();
							return ferr;
						}
					}
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
		}
		// 変更イベントがパフォーマンス1の場合
		if ((byEvKind & COMB_EVKIND_PPC_CH1) == COMB_EVKIND_PPC_CH1)  {
			if (memcmp((const void*)&EvCombPerf_RX, (const void*)&evPerfTime, sizeof(FFWRX_COMB_PERFTIME)) != 0) {
				ferr = SetEvPerfData(1, evPerfTime);
				if (ferr != FFWERR_OK) {
					if (bRet) {			// 実行中の場合
						// 動作許可指定を有効化に戻す
						ferr = SetEventEnable(TRUE);
						if (ferr != FFWERR_OK) {
							ferrEnd = ProtEnd();
							return ferr;
						}
					}
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
		}
	}

	// パフォーマンス機能のユーザ利用を変更する場合
	if ((byEvKind & COMB_EVKIND_PPC_USR) == COMB_EVKIND_PPC_USR) {
		// パフォーマンス計測レジスタアクセスコントロールへの設定
		ferr = SetPerfAccCtrl(ePpcUsrCtrl);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevNo120131-001 Delete 	SetPerfUsrCtrl(ePpcUsrCtrl)　ここで設定値の変更はしない
	}

	// イベント組合せ指定レジスタへの設定
	// 回路の初期化
	ferr = InitCombEvent(dwCombi);						// 渡した引数で組み合わせ回路を設定する　RevNo120131-001 Modify Line
	if (ferr != FFWERR_OK) {
		if (bRet) {			// 実行中の場合
			// 動作許可指定を有効化に戻す
			ferr = SetEventEnable(TRUE);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevNo120131-001,RevNo120606-002 Modify Start 
	// 実行前PCイベントの設定値を取得
	if((bRet == FALSE) 
	|| ((bRet == TRUE) && (GetMcuRunCmd() == PROGCMD_GB) && ((dwBMode & BMODE_PRE_BIT) == BMODE_PRE_BIT))){
		//ブレーク中もしくは、実行前ブレークイベント有効時GB実行
		//内部変数値をそのまま設定
		//RevRxNo140515-002 Modify Start
		if ((byEvKind & COMB_EVKIND_BRK) == COMB_EVKIND_BRK){
			dwPreBrk = evBrk->dwPreBrk;
		} else {
			dwPreBrk = EvCombBrk_RX->dwPreBrk;
		}
		//RevRxNo140515-002 Modify End
	} else if((bRet == TRUE) && (GetMcuRunCmd() == PROGCMD_GPB)){
		// GPB実行時
		GetGpbPcEv(&dwGpbEvNo,&dwGpbEvAddr);
		dwPreBrk = 0x00000001 << dwGpbEvNo;
	} else {
		// その他は設定しない
		dwPreBrk = 0;
	}

	// パフォーマンスイベント回数計測時もEVEPCPEへイベント許可が必要
	dwSetEvNum = GetPpcCntEvNum();
	dwPpcPcEv = dwSetEvNum & 0x000000FF;
	dwPpcOpcEv = (dwSetEvNum & 0x000F0000) >> 16;

	dwExecPcSet = evBrk->dwBrkpe |
					evTrc->dwStartpe | evTrc->dwEndpe |
					evPerfTime->dwStartpe[0] | evPerfTime->dwEndpe[0] |
					evPerfTime->dwStartpe[1] | evPerfTime->dwEndpe[1] |
					dwPpcPcEv;

	dwOpcSet = evBrk->dwBrkde |
				evTrc->dwStartde | evTrc->dwEndde | evTrc->dwPickupde |
				evPerfTime->dwStartde[0] | evPerfTime->dwEndde[0] |
				evPerfTime->dwStartde[1] | evPerfTime->dwEndde[1] |
				dwPpcOpcEv;

	//RevNo100715-023 Modify Start
	// イベント動作許可レジスタ設定
	// SetCOMB()内でイベント動作許可レジスタの変更を反映する。
	ferr = SetRegEvEnable(dwPreBrk, dwExecPcSet, dwOpcSet);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	//RevNo100715-023 Modify End
	//RevNo120131-001,RevNo120606-002 Modify End
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;

}

//==============================================================================
/**
 * トレースパフォーマンスのトリガ設定をする。
 * @param byEvKind トリガの種類  bit0:トレース　bit1:PPC0 bit2:PPC1
 * @param evTrg トリガ設定情報
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetRXEVTRG(BYTE byEvKind, FFWRX_TRG_DATA evTrg)
{

	// V.1.02 RevNo110613-001 Modify Line
	FFWERR						ferr = FFWERR_OK;
	FFWERR						ferrEnd;

	ProtInit();

	//トリガ情報設定
	ferr = SetTrgEv(byEvKind, evTrg);
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

//=============================================================================
/**
 * イベント関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdRxData_Ev(void)
{

	return;

}
