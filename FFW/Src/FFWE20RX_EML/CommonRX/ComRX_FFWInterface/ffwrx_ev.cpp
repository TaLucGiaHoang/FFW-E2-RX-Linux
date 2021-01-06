////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_ev.cpp
 * @brief RX共通イベント関連コマンドのソースファイル
 * @author RSD Y.Minami, H.Hashiguchi, Y.Miyake, K.Uemori, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/17
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
・RevRxNo120713-001 2012/07/13 橋口
　GetRegEvEnable()処理でs_dwExecPcSet,s_dwExecOpcSetを使わないように修正
・RevRxNo120910-001	2012/10/17 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・FFWRXCmd_SetEV()で、RX64x の場合ev->evOpc.dwBusSelのb0,b1が同時に"1"のとき、
    引数エラーとする。
    InitFfwIfRxData_Ev()で、s_EvOpc[i].dwBusSelに初期値 "1(CPUバス)" 設定を追加。
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121126-002　2012/11/28 上田(2012/12/04 SDS 岩田 マージ)
　COMB設定エラー時に、COMBの設定格納変数(s_EvComb～)が書き換わっている不具合修正
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001	2013/06/13 上田
	RX64M対応
・RevRxNo130308-001 2013/08/21 上田 (2013/06/13 三宅担当分マージ)
　カバレッジ開発対応
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
*/

#include "ffwrx_ev.h"
#include "dorx_ev.h"
#include "ffwrx_tim.h"
#include "errchk.h"
#include "ffwmcu_mcu.h"
#include "domcu_prog.h"
#include "protmcu_mem.h"
#include "ffwrx_tim.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line

// FFW 内部変数

static FFWRX_EV_ADDR		s_EvAddr[EV_ADDR_MAX_RX];			// 命令実行イベントの管理変数
static FFWRX_EV_OPC			s_EvOpc[EV_OPC_MAX_RX];				// オペランドアクセスイベントの管理変数
static FFWRX_EVCNT_DATA		s_EvCount_RX;						// イベント成立回数の管理変数
static FFWRX_TRG_DATA		s_EvTrgData_RX;						// イベントトリガの設定管理変数
static DWORD				s_dwCombi_RX;						// イベント組合せの設定管理変数
static FFWRX_COMB_BRK		s_EvCombBrk_RX;						// ブレークイベントの設定管理変数
static FFWRX_COMB_TRC		s_EvCombTrc_RX;						// トレースイベントの設定管理変数
static FFWRX_COMB_PERFTIME	s_EvCombPerf_RX;					// パフォーマンス計測イベントの設定管理変数
static enum FFWRX_EVCOMBI_PPC_USR_CTRL	s_PpcUsrCtrl;			// パフォーマンス計測アクセスコントロールの管理変数

static FFWRX_EV_ADDR		s_EscEvAddr[EV_ADDR_MAX_RX];		// 退避/復帰用の命令実行イベント管理変数
static FFWRX_COMB_BRK		s_EscEvCombBrk_RX;					// 退避/復帰用のブレークイベント設定管理変数
static FFWRX_COMB_TRC		s_EscEvCombTrc_RX;					// 退避/復帰用のトレースイベントの設定管理変数
static FFWRX_COMB_PERFTIME	s_EscEvCombPerf_RX;					// 退避/復帰用のパフォーマンス計測イベントの設定管理変数

// RevRxNo120713-001 Delete s_dwExecPcSet,s_dwExecOpcSet

static DWORD				s_dwPreEvCombi;						// 退避用実行前PCブレーク組み合わせ管理変数

// 2008.11.6 INSERT_BEGIN_E20RX600(+NN) {
// V.1.02 No.10 Modify Start
//==============================================================================
/**
 * イベント単体の設定を行う。
 * @param  eEvNo           設定するイベント種別に応じたイベント番号
 * @param  ev              設定するイベント情報を格納するFFWRX_EV_DATA 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_SetEV(FFWRX_EV eEvNo, FFWRX_EV_DATA* ev)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR						ferr = FFWERR_OK;
	DWORD						dwMode;
	BYTE						byNo;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// イベント番号チェック
	if ((FFWRX_PE0 <= eEvNo) && (eEvNo < pMcuInfo->wPCEvent)) {
		dwMode = EV_MODE_ADDR;			// 命令実行
		byNo = (BYTE)eEvNo;
	// RevRxNo130411-001 Modify Line
	} else if ((FFWRX_DE0 <= eEvNo) && (eEvNo <= FFWRX_DE3) && (pFwCtrl->eOcdCls == RX_OCD_CLS3)) {
		// OCD Class3 オペランドイベント
		if (ev->evOpc.byKindSelect == EV_MODE_OPC_RANGE) {
			if (FFWRX_DE1 != eEvNo) {		// DE1以外は範囲指定の設定不可
				return FFWERR_FFW_ARG;
			}
			dwMode = EV_MODE_OPC_RANGE;		// オペランドアクセス(範囲指定)
		} else if (ev->evOpc.byKindSelect == EV_MODE_OPC_MASK) {
			dwMode = EV_MODE_OPC_MASK;		// オペランドアクセス(マスク指定)
		} else {
			return FFWERR_FFW_ARG;
		}
		byNo = (BYTE)(eEvNo - FFWRX_DE0);
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	} else if ((FFWRX_DE1 <= eEvNo) && (eEvNo < (FFWRX_DE1+(pMcuInfo->wOAEvent))) && (pFwCtrl->eOcdCls == RX_OCD_CLS2)) {
		// OCD Class2 オペランドイベント
		if (ev->evOpc.byKindSelect == EV_MODE_OPC_RANGE) {
			//RX Class2はないのでエラー
			return FFWERR_FFW_ARG;
		} else if (ev->evOpc.byKindSelect == EV_MODE_OPC_MASK) {
			dwMode = EV_MODE_OPC_MASK;		// オペランドアクセス(マスク指定)
		} else {
			return FFWERR_FFW_ARG;
		}
		byNo = (BYTE)(eEvNo - FFWRX_DE0);
	} else {
		return FFWERR_FFW_ARG;
	}

	if (dwMode == EV_MODE_ADDR) {
		// 実行イベントの場合
		if (memcmp((const void*)&s_EvAddr[byNo], (const void*)&ev->evAddr, sizeof(FFWRX_EV_ADDR)) == 0) {
			//設定値が変更されない場合は正常終了
			return FFWERR_OK;
		}
	} else if ((dwMode == EV_MODE_OPC_RANGE) || (dwMode == EV_MODE_OPC_MASK)) {
	//オペランドイベント設定の場合の引数チェック
		if (memcmp((const void*)&s_EvOpc[byNo], (const void*)&ev->evOpc, sizeof(FFWRX_EV_OPC)) != 0) {
			//値が更新されている場合
			if (dwMode == EV_MODE_OPC_RANGE) {			// 範囲指定の場合
				// 開始アドレスが終了アドレスより大きい場合
				if (ev->evOpc.dwAddrStart > ev->evOpc.dwAddrEnd) {
					return FFWERR_FFW_ARG;
				}
				// アドレス比較条件指定チェック
				if ((ev->evOpc.byAddrCmp != EV_CMPRANGE_OUT) && (ev->evOpc.byAddrCmp != EV_CMPRANGE_IN) &&
					(ev->evOpc.byAddrCmp != EV_CMPMASK_AGREE) && (ev->evOpc.byAddrCmp != EV_CMPMASK_DISAGREE)) {
					return FFWERR_FFW_ARG;
				}
			} else if (dwMode == EV_MODE_OPC_MASK) {		// マスク指定の場合
				// アドレス比較条件指定チェック
				if ((ev->evOpc.byAddrCmp != EV_CMPMASK_AGREE) && (ev->evOpc.byAddrCmp != EV_CMPMASK_DISAGREE)) {
					return FFWERR_FFW_ARG;
				}
			}

			// アクセス属性指定チェック
			if ((ev->evOpc.byAccess != EV_ACCESS_NON) && (ev->evOpc.byAccess != EV_ACCESS_R) &&
				(ev->evOpc.byAccess != EV_ACCESS_W) && (ev->evOpc.byAccess != EV_ACCESS_RW)) {
				return FFWERR_FFW_ARG;
			}
			// データサイズ指定チェック
			if ((ev->evOpc.byDataSize != EV_DATASIZE_NON) && (ev->evOpc.byDataSize != EV_DATASIZE_BYTE) &&
				(ev->evOpc.byDataSize != EV_DATASIZE_WORD) && (ev->evOpc.byDataSize != EV_DATASIZE_LWORD)) {
				return FFWERR_FFW_ARG;
			}
			// データ比較条件指定チェック
			if ((ev->evOpc.byDataCmp != EV_CMPMASK_AGREE) && (ev->evOpc.byDataCmp != EV_CMPMASK_DISAGREE)) {
				return FFWERR_FFW_ARG;
			}
			// RevRxNo130301-001 Modify Start
			// RevRxNo120910-001 Append Start
			// RevRxNo130411-001 Modify Line
			if (pFwCtrl->bEvOaBusSelEna == TRUE) {	// オペランドアクセスイベントのバス選択があるMCUの場合
				// b0,b1が同時に"1"のとき、引数エラーとする。
				if ((ev->evOpc.dwBusSel & (EV_BUSSEL_CPU_BUS | EV_BUSSEL_DMAC_DTC_BUS)) == 
										  (EV_BUSSEL_CPU_BUS | EV_BUSSEL_DMAC_DTC_BUS)) {
					return FFWERR_FFW_ARG;
				}
			}
			// RevRxNo120910-001 Append End
			// RevRxNo130301-001 Modify End
		} else {
			//設定値が変更されない場合は正常終了
			return FFWERR_OK;
		}
	}

	//OCDレジスタに値を設定
	ferr = DO_SetRXEV(dwMode,byNo, ev);
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// イベント設定値を内部変数に保存
	if (dwMode == EV_MODE_ADDR) {
	// 実行イベントの場合
		if (memcmp((const void*)&s_EvAddr[byNo], (const void*)&ev->evAddr, sizeof(FFWRX_EV_ADDR)) != 0) {
			// 値が更新されていたら内部変数にセット
			s_EvAddr[byNo].dwAddr = ev->evAddr.dwAddr;		
		}
	// オペランドアクセス
	} else if (dwMode == EV_MODE_OPC_RANGE || dwMode == EV_MODE_OPC_MASK) {
		if (memcmp((const void*)&s_EvOpc[byNo], (const void*)&ev->evOpc, sizeof(FFWRX_EV_OPC)) != 0) {
			//値が更新されている場合
			// 内部変数にセット
			s_EvOpc[byNo].byKindSelect = ev->evOpc.byKindSelect;
			s_EvOpc[byNo].dwAddrStart = ev->evOpc.dwAddrStart;
			s_EvOpc[byNo].dwAddrEnd = ev->evOpc.dwAddrEnd;
			s_EvOpc[byNo].dwDataValue = ev->evOpc.dwDataValue;
			s_EvOpc[byNo].byAccess = ev->evOpc.byAccess;
			s_EvOpc[byNo].byDataSize = ev->evOpc.byDataSize;
			s_EvOpc[byNo].byAddrCmp = ev->evOpc.byAddrCmp;
			s_EvOpc[byNo].byDataCmp = ev->evOpc.byDataCmp;
			s_EvOpc[byNo].dwBusSel = ev->evOpc.dwBusSel;	// RevRxNo130301-001 Append Line
			// データサイズによってマスク値を変更
			if (ev->evOpc.byDataSize == EV_DATASIZE_BYTE) {
				s_EvOpc[byNo].dwDataMask = (ev->evOpc.dwDataMask & 0x000000FF);
			} else if (ev->evOpc.byDataSize == EV_DATASIZE_WORD) {
				s_EvOpc[byNo].dwDataMask = (ev->evOpc.dwDataMask & 0x0000FFFF);
			} else {
				s_EvOpc[byNo].dwDataMask = ev->evOpc.dwDataMask;
			}
		}
	}

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modiy End
}
// V.1.02 No.10 Modify End

//==============================================================================
/**
 * イベント設定内容の参照を行う。
 * @param  eEvNo     参照するイベント種別に応じたイベント番号
 * @param  ev        参照するイベント情報を格納するFFWRX_EV_DATA 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_GetEV(FFWRX_EV eEvNo, FFWRX_EV_DATA* ev)
{

	if ((FFWRX_PE0 <= eEvNo) && (eEvNo <= FFWRX_PE7)) {
		memcpy(&ev->evAddr, &s_EvAddr[eEvNo], sizeof(FFWRX_EV_ADDR));
	} else if ((FFWRX_DE0 <= eEvNo) && (eEvNo <= FFWRX_DE3)) {
		memcpy(&ev->evOpc, &s_EvOpc[eEvNo-0x10], sizeof(FFWRX_EV_OPC));
	} else {
		return FFWERR_FFW_ARG;
	}
	return FFWERR_OK;
}

//==============================================================================
/**
 * イベント成立回数の設定を行う。
 * @param  evcnt           設定するイベント成立可否情報を格納するFFWRX_EVCNT_DATA 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_SetEVCNT(FFWRX_EVCNT_DATA* evcnt)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR						ferr = FFWERR_OK;
	DWORD						dwMode;
	BYTE						byNo;
	DWORD						dwCombi;			// イベント組合せ設定情報
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*			pFwCtrl;	// RevRxNo130411-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// V.1.02 No.10 Append Start
	//Class2はカウント機能がないため、有効設定の場合エラーを返す
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if((evcnt->bEnable == TRUE) && (pFwCtrl->eOcdCls == RX_OCD_CLS2)){
		return FFWERR_EV_UNSUPPORT;
	}
	// V.1.02 No.10 Append End

	// イベント番号チェック
	if ((EVNO_PC_START <= evcnt->eEvNo) && (evcnt->eEvNo < pMcuInfo->wPCEvent)) {
		byNo = (BYTE)(evcnt->eEvNo);
		// 接続有無チェック
		if (evcnt->bEnable == TRUE) {
			dwMode = EVCNT_ADDR;	// 命令実行
		} else {
			dwMode = EVCNT_NON;		// 接続しない
		}
	} else if ((EVNO_OP_START <= evcnt->eEvNo) && (evcnt->eEvNo < (EVNO_OP_START+pMcuInfo->wOAEvent))) {
		byNo = (BYTE)(evcnt->eEvNo - EVNO_OP_START);
		// 接続有無チェック
		if (evcnt->bEnable == TRUE) {
			dwMode = EVCNT_OPC;		// オペランドアクセス
		} else {
			dwMode = EVCNT_NON;		// 接続しない
		}
	} else {
		return FFWERR_FFW_ARG;
	}

	// 状態遷移指定有無チェック(接続する場合のみ)※状態遷移指定時はPE0とDE0にイベント回数指定できない
	if (evcnt->bEnable == TRUE) {
		dwCombi = GetCombiInfo();
		if (byNo == EV_NO_0 && (dwCombi == COMB_BRK_STATE || dwCombi == COMB_TRC_STATE)) {
			return FFWERR_FFW_ARG;
		}
	}

	// イベント成立回数チェック(接続する場合のみ)
	if (evcnt->bEnable == TRUE) {
		if ((evcnt->dwCount > EVCNT_MAX) || (evcnt->dwCount < EVCNT_MIN)) {
			return FFWERR_FFW_ARG;
		}
	} else {
		evcnt->dwCount = s_EvCount_RX.dwCount;	// 接続しない場合、内部変数をセット
	}

	//OCDに設定
	ferr = DO_SetRXEVCNT(dwMode,byNo,evcnt);
	if(ferr != FFWERR_OK){
		return ferr;
	}
	
	// 内部変数にセット
	s_EvCount_RX.bEnable = evcnt->bEnable;
	s_EvCount_RX.eEvNo = evcnt->eEvNo;

	// イベント成立回数の指定が最大値の場合
	if (evcnt->dwCount == EVCNT_MAX) {
		s_EvCount_RX.dwCount = 0x00000000;	// 内部的に0hをセットする
	} else {
		s_EvCount_RX.dwCount = evcnt->dwCount;
	}

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}

//==============================================================================
/**
 * イベント成立回数の参照を行う。
 * @param  evcnt           設定するイベント成立可否情報を格納するFFWRX_EVCNT_DATA 構造体のアドレスを指定する。
 * @retval FFWERR_OK       正常終了
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_GetEVCNT(FFWRX_EVCNT_DATA* evcnt)
{
	memcpy(evcnt, &s_EvCount_RX, sizeof(FFWRX_EVCNT_DATA));

	return FFWERR_OK;
}

//==============================================================================
/**
 * イベント組合せの設定を行う。
 * @param  byEvKind        変更対象のイベントを指定
 * @param  dwCombi         イベントの組合せを指定
 * @param  evBrk           ブレークイベント情報を格納するFFWRX_COMB_BRK 構造体のアドレスを指定する。
 * @param  evTrc           トレースイベント情報を格納するFFWRX_COMB_TRC 構造体のアドレスを指定する。
 * @param  evPerfTime      パフォーマンス計測イベント情報を格納するFFWRX_COMB_PERFTIME 構造体のアドレスを指定する。
 * @param  ePpcUsrCtrl     パフォーマンス計測資源に対するアクセスコントロールを設定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_SetCOMB(BYTE byEvKind, DWORD dwCombi, FFWRX_COMB_BRK* evBrk, FFWRX_COMB_TRC* evTrc,
										FFWRX_COMB_PERFTIME* evPerfTime, enum FFWRX_EVCOMBI_PPC_USR_CTRL ePpcUsrCtrl)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR						ferr = FFWERR_OK;
	// RevRxNo120713-001 Delete	dwSetEvNum,dwPpcPcEv,dwPpcOpcEv
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得

	// イベント組合せ指定の内容チェック
	if ((dwCombi < COMB_OR) || (dwCombi > COMB_TRC_STATE)) {
		return FFWERR_FFW_ARG;
	}

	// 変更対象イベント指定の内容チェック
	if ((byEvKind & COMB_EVKIND_ALL) < COMB_EVKIND_COMB || ((byEvKind & COMB_EVKIND_ALL) > COMB_EVKIND_ALL)) {
		return FFWERR_FFW_ARG;
	}

	//-----------------
	// 各設定内容チェック
	//-----------------
	// 組合せを変更しない場合は、前の設定値を渡す
	if ((byEvKind & COMB_EVKIND_COMB) != COMB_EVKIND_COMB) {
		dwCombi = s_dwCombi_RX;
	}

	// 組み合わせイベントチェックのため設定変更イベントを一時的にstatic変数に移す
	EscExecEv(byEvKind, evBrk, evTrc, evPerfTime, TRUE);		// 設定情報を退避

	// 変更イベントがブレークの場合
	if ((byEvKind & COMB_EVKIND_BRK) == COMB_EVKIND_BRK) {
		//引数チェック
		ferr = ChkEvBreakData(dwCombi, evBrk);
		if (ferr != FFWERR_OK) {
			EscExecEv(byEvKind, evBrk, evTrc, evPerfTime, FALSE);		// 設定情報を復帰
			return ferr;
		}
	}

	// 変更イベントがトレースの場合
	if ((byEvKind & COMB_EVKIND_TRC) == COMB_EVKIND_TRC) {
		//組み合わせチェック
		ferr = ChkEvTraceData(dwCombi, evTrc);
		if (ferr != FFWERR_OK) {
			EscExecEv(byEvKind, evBrk, evTrc, evPerfTime, FALSE);		// 設定情報を復帰
			return ferr;
		}
	}

	// 設定変更イベントを戻す
	EscExecEv(byEvKind, evBrk, evTrc, evPerfTime, FALSE);		// 設定情報を復帰

	// RevRxNo121026-001 Append Start
	// wPPCがないMCUでパフォーマンスへの設定はエラー
	if((pMcuInfo->wPPC == RX_PPC_NON) && ((byEvKind & COMB_EVKIND_PPC_MSK) != COMB_EVKIND_PPC_NON)){
		return FFWERR_EV_UNSUPPORT;
	}
	// RevRxNo121026-001 Append End

	// パフォーマンス機能のユーザ利用を変更する場合
	if ((byEvKind & COMB_EVKIND_PPC_USR) == COMB_EVKIND_PPC_USR) {
		if ((ePpcUsrCtrl != FFWRX_EVCOMBI_PPC_USR) && (ePpcUsrCtrl != FFWRX_EVCOMBI_PPC_EML)) {
			return FFWERR_FFW_ARG;
		}
	}

	// 変更イベントがパフォーマンス0の場合
	if ((byEvKind & COMB_EVKIND_PPC_CH0) == COMB_EVKIND_PPC_CH0) {
		ferr = ChkEvPerfData(0, dwCombi, evPerfTime);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// 変更イベントがパフォーマンス1の場合
	// Class2はパフォーマンスカウンタが1chしかない
	if ((byEvKind & COMB_EVKIND_PPC_CH1) == COMB_EVKIND_PPC_CH1) {
		// V.1.02 No.10 Append Start
		// RevRxNo121026-001 Modify
		// PPCのチャネル数が1chの場合、パフォーマンスch1への設定はエラー
		if(pMcuInfo->wPPC == RX_PPC_1CH) {
			return FFWERR_EV_UNSUPPORT;
		}
		// V.1.02 No.10 Append End
		ferr = ChkEvPerfData(1, dwCombi, evPerfTime);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// RevNo120606-002,RevNo120131-001 Delete 引数を内部管理変数の更新はDO_SetRXCOMB()実施後

	//OCDへ設定
	ferr = DO_SetRXCOMB(byEvKind, dwCombi,  evBrk,  evTrc, evPerfTime, ePpcUsrCtrl);
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// RevRxNo120713-001 Delete s_dwExecPcSet,s_dwExecOpcSet
	// 引数を内部管理変数へ渡す。RevNo120606-002,RevNo120131-001 Append Start
	// 組合せを変更する場合
	if ((byEvKind & COMB_EVKIND_COMB) == COMB_EVKIND_COMB) {
		s_dwCombi_RX = dwCombi;
	}
	// 変更イベントがブレークの場合
	if ((byEvKind & COMB_EVKIND_BRK) == COMB_EVKIND_BRK) {
		// 内部変数にセット
		s_EvCombBrk_RX.dwPreBrk = evBrk->dwPreBrk;
		s_EvCombBrk_RX.dwBrkpe = evBrk->dwBrkpe;
		s_EvCombBrk_RX.dwBrkde = evBrk->dwBrkde;
	}
	// 変更イベントがトレースの場合
	if ((byEvKind & COMB_EVKIND_TRC) == COMB_EVKIND_TRC) {
		// 内部変数にセット
		s_EvCombTrc_RX.dwStartpe = evTrc->dwStartpe;
		s_EvCombTrc_RX.dwEndpe = evTrc->dwEndpe;
		s_EvCombTrc_RX.dwStartde = evTrc->dwStartde;
		s_EvCombTrc_RX.dwEndde = evTrc->dwEndde;
		s_EvCombTrc_RX.dwPickupde = evTrc->dwPickupde;
	}
	// 変更イベントがパフォーマンスの場合
	if ((byEvKind & COMB_EVKIND_PPC) != 0) {
		// 変更イベントがパフォーマンス0の場合
		if ((byEvKind & COMB_EVKIND_PPC_CH0) == COMB_EVKIND_PPC_CH0) {
			// 内部変数にセット
				// 内部変数にセット
			s_EvCombPerf_RX.dwStartpe[0] = evPerfTime->dwStartpe[0];
			s_EvCombPerf_RX.dwEndpe[0] = evPerfTime->dwEndpe[0];
			s_EvCombPerf_RX.dwStartde[0] = evPerfTime->dwStartde[0];
			s_EvCombPerf_RX.dwEndde[0] = evPerfTime->dwEndde[0];
		}
		// 変更イベントがパフォーマンス1の場合
		if ((byEvKind & COMB_EVKIND_PPC_CH1) == COMB_EVKIND_PPC_CH1)  {
			// 内部変数にセット
			s_EvCombPerf_RX.dwStartpe[1] = evPerfTime->dwStartpe[1];
			s_EvCombPerf_RX.dwEndpe[1] = evPerfTime->dwEndpe[1];
			s_EvCombPerf_RX.dwStartde[1] = evPerfTime->dwStartde[1];
			s_EvCombPerf_RX.dwEndde[1] = evPerfTime->dwEndde[1];
		}
	}

	// パフォーマンス機能のユーザ利用を変更する場合
	if ((byEvKind & COMB_EVKIND_PPC_USR) == COMB_EVKIND_PPC_USR) {
		s_PpcUsrCtrl = ePpcUsrCtrl;
	}
	// RevNo120606-002,RevNo120131-001 Append End

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}

//==============================================================================
/**
 * イベント組合せの参照を行う。
 * @param  dwCombi         イベントの組合せを指定
 * @param  evBrk           ブレークイベント情報を格納するFFWRX_COMB_BRK 構造体のアドレスを指定する。
 * @param  evTrc           トレースイベント情報を格納するFFWRX_COMB_TRC 構造体のアドレスを指定する。
 * @param  evPerfTime      パフォーマンス計測イベント情報を格納するFFWRX_COMB_PERFTIME 構造体のアドレスを指定する。
 * @param  ePpcUsrCtrl     パフォーマンス計測資源に対するアクセスコントロールを設定する。
 * @retval FFWERR_OK       正常終了
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_GetCOMB(DWORD* dwCombi, FFWRX_COMB_BRK* evBrk, FFWRX_COMB_TRC* evTrc,
										FFWRX_COMB_PERFTIME* evPerfTime, enum FFWRX_EVCOMBI_PPC_USR_CTRL* ePpcUsrCtrl)
{
	(*dwCombi) = s_dwCombi_RX;
	memcpy(evBrk, &s_EvCombBrk_RX, sizeof(FFWRX_COMB_BRK));
	memcpy(evTrc, &s_EvCombTrc_RX, sizeof(FFWRX_COMB_TRC));
	memcpy(evPerfTime, &s_EvCombPerf_RX, sizeof(FFWRX_COMB_PERFTIME));
	(*ePpcUsrCtrl) = s_PpcUsrCtrl;

	return FFWERR_OK;
}

//==============================================================================
/**
 * イベントトリガの設定を行う。
 * @param  byEvKind        変更対象イベントを指定
 * @param  evTrg           設定するイベントトリガ情報を格納するFFWRX_TRG_DATA 構造体
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_SetEVTRG(BYTE byEvKind, FFWRX_TRG_DATA evTrg)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR						ferr = FFWERR_OK;
	DWORD						dwSetPf0,dwSetPf1;
	// RevNo110412-001 Append Line
	BOOL						bRet=FALSE;
	// RevNo110421-003 Append Start
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	// RevNo110421-003 Append End
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;		// MCU情報構造体のポインタ
	// RevRxNo130308-001-008 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	// RevNo110421-003 Append Start
	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得
	// RevNo110421-003 Append End

	// RevNo110412-001 Append Start
	bRet = GetMcuRunState();	// RevRxE2LNo141104-001 Modify Line
	// RevNo110421-003 Modfy Start
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		// 通常RUN中の場合
		if(bRet){
			return FFWERR_BMCU_RUN;
		}
	}
	// RevNo110421-003 Modfy End
	// RevNo110412-001 Append End


	// 変更対象イベント指定の内容チェック
	if (byEvKind  > EVTRG_EVKIND_ALL) {
		return FFWERR_FFW_ARG;
	}

	// V.1.02 No.10 Append Start
	// Class2のパフォーマンスch1への設定はエラー
	if((pMcuInfo->wPPC == RX_PPC_1CH) && ((byEvKind & EVTRG_EVKIND_PPC_CH1) == EVTRG_EVKIND_PPC_CH1)){
		return FFWERR_EV_UNSUPPORT;
	}
	// V.1.02 No.10 Append End

	// RevRxNo121026-001 Append Start
	// wPPCがないMCUでパフォーマンスへの設定はエラー
	if((pMcuInfo->wPPC == RX_PPC_NON) && ((byEvKind & EVTRG_EVKIND_PPC_MSK) != EVTRG_EVKIND_PPC_NON)){
		return FFWERR_EV_UNSUPPORT;
	}
	// RevRxNo121026-001 Append End

	// 変更対象イベントがトレースの場合
	if ((byEvKind & EVTRG_EVKIND_TRC) == EVTRG_EVKIND_TRC) {
		// トレース用トリガの設定チェック
		if (evTrg.dwTrcTrg > EVETTRG_TRC_ALL) {
			return FFWERR_FFW_ARG;
		}
	}

	// RevRxNo130308-001-008 Append Start
	// カバレッジ機能選択の場合、トレース抽出用、トレース終了用、トレース開始用イベントトリガは未使用のみ可。
 	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// カバレッジ機能選択時
		if ((byEvKind & EVTRG_EVKIND_TRC) == EVTRG_EVKIND_TRC) {
			// トレース用トリガの設定チェック
			if ((evTrg.dwTrcTrg & EVETTRG_TRC_ALL) != EVETTRG_TRC_NON) {
				return FFWERR_RTT_UNSUPPORT;		// エラー「トレース機能はサポートしていない。」
			}
		}
	}
	// RevRxNo130308-001-008 Append End

	//OCDへの設定 HotPlug動作中は設定しない
	if (bHotPlugState == FALSE || bIdCodeResultState == FALSE) {
		ferr = DO_SetRXEVTRG(byEvKind, evTrg);
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}
	
	//内部変数更新
	// 変更対象イベントがトレースの場合
	if ((byEvKind & EVTRG_EVKIND_TRC) == EVTRG_EVKIND_TRC) {
		s_EvTrgData_RX.dwTrcTrg = evTrg.dwTrcTrg;
	}

	// 変更対象イベントがパフォーマンス0の場合
	if ((byEvKind & EVTRG_EVKIND_PPC_CH0) == EVTRG_EVKIND_PPC_CH0) {
		dwSetPf0 = s_EvTrgData_RX.dwPerfTrg & CLR_EVTRG_PPC_CH0;		// 設定対象のビットOFF
		s_EvTrgData_RX.dwPerfTrg = dwSetPf0 | evTrg.dwPerfTrg;	// 今回の設定値を格納
	}
	// 変更対象イベントがパフォーマンス1の場合
	if ((byEvKind & 0x04) == 0x04) {
		dwSetPf1 = s_EvTrgData_RX.dwPerfTrg & CLR_EVTRG_PPC_CH1;		// 設定対象のビットOFF
		s_EvTrgData_RX.dwPerfTrg = dwSetPf1 | evTrg.dwPerfTrg;	// 今回の設定値を格納
	}

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End
}

//==============================================================================
/**
 * イベントトリガの参照を行う。
 * @param  pEvTrg          設定するイベントトリガ情報を格納するFFWRX_TRG_DATA 構造体
 * @retval FFWERR_OK       正常終了
 */
//==============================================================================
FFWE100_API DWORD FFWRXCmd_GetEVTRG(FFWRX_TRG_DATA* pEvTrg)
{
	memcpy(pEvTrg, &s_EvTrgData_RX, sizeof(FFWRX_TRG_DATA));

	return FFWERR_OK;
}

// V.1.02 No.10 Modify Start
//==============================================================================
/**
 * ブレークイベントの設定内容をチェックする。
 * @param  dwCombi         イベントの組合せを指定
 * @param  evBrk           ブレークイベント情報を格納するFFWRX_COMB_BRK 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ChkEvBreakData(DWORD dwCombi, FFWRX_COMB_BRK* evBrk)
{
	DWORD					dwShiftData;		// 状態遷移時のイベント設定チェック用
	DWORD					dwEvBuffData;		// 状態遷移時のイベント設定チェック用
	BOOL					bStateEvSet;		// 状態遷移時のイベント設定チェック用
	int						i;
	WORD					j;
	FFWMCU_DBG_DATA_RX*		pDbgData;
	FFWMCU_MCUINFO_DATA_RX*	pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*		pFwCtrl;	// RevRxNo130411-001 Append Line

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pDbgData = GetDbgDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	//実行前ブレーク有効イベントチェック
	if(evBrk->dwPreBrk >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wPCEvent))){
		return FFWERR_FFW_ARG;
	}

	//実行ブレークイベントチェック
	if(evBrk->dwBrkpe >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wPCEvent))){
		return FFWERR_FFW_ARG;
	}

	// Rev110303-002 Modify Start
	// オペランドイベントはClass3とClass2でことなるためチェックを修正
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		//オペランドイベントチェック
		if(evBrk->dwBrkde >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent))){
			return FFWERR_FFW_ARG;
		}
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	} else {
		//オペランドイベントチェック
		if((evBrk->dwBrkde  >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent+1))) 
			|| ((evBrk->dwBrkde  & 0x00000001) == 0x00000001) ){
			return FFWERR_FFW_ARG;
		}
	}
	// Rev110303-002 Modify End

	// 実行前PCブレークとの重複チェック
	if ((evBrk->dwPreBrk & evBrk->dwBrkpe) != 0x00000000) {
		if (pDbgData->eOcdArgErrChk == OCD_ARGERRCHK_USE) {		// 組合せ制限エラー検出ありの場合
			return FFWERR_FFW_REPEAT_ADDR_ADDR;
		}
	}
	// 累積AND,状態遷移指定時のチェック
	if ((dwCombi == COMB_BRK_AND) || (dwCombi == COMB_BRK_STATE)) {
		if (pDbgData->eOcdArgErrChk == OCD_ARGERRCHK_USE) {		// 組合せ制限エラー検出ありの場合
			// 命令実行とオペランドアクセスの重複チェック
			// RevNo110316-002 Modify Start
			// E1/E20 V.1.02では仕様を変えないため、Class2の場合状態遷移のときのみチェックとしているが、
			// 本来はClass3の累積ANDもチェック不要
			// RevRxNo121026-001, RevRxNo130411-001 Modify Line
			if ( ((pFwCtrl->eOcdCls == RX_OCD_CLS2) && (dwCombi == COMB_BRK_STATE)) // Class2の状態遷移の場合
				|| pFwCtrl->eOcdCls == RX_OCD_CLS3) {		// Class3の状態遷移/累積ANDの場合
				if (((evBrk->dwBrkpe & 0x0000000F) & evBrk->dwBrkde) != 0x00000000) {
					return FFWERR_FFW_REPEAT_ADDR_OPC;
				}
			}
			// RevNo110316-002 Modify End

			// ブレークイベントで使用しているイベントを他のイベントと併用していないかチェック
			// 実行前PCブレークイベントとチェック すでにチェック済
			// トレース開始イベントとチェック
			if ((evBrk->dwBrkpe & s_EvCombTrc_RX.dwStartpe) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_ADDR;
			}

			// トレース終了実行PCイベントとチェック
			if ((evBrk->dwBrkpe & s_EvCombTrc_RX.dwEndpe) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_ADDR;
			}

			// パフォーマンス開始実行PCイベントとチェック
			for(j=0;j< (pMcuInfo->wPPC);j++){
				if ((evBrk->dwBrkpe & s_EvCombPerf_RX.dwStartpe[j]) != 0x00000000) {
					return FFWERR_FFW_REPEAT_COMB_ADDR;
				}
			}
			// パフォーマンス終了実行PCイベントとチェック
			for(j=0;j< (pMcuInfo->wPPC);j++){
				if ((evBrk->dwBrkpe & s_EvCombPerf_RX.dwEndpe[j]) != 0x00000000) {
					return FFWERR_FFW_REPEAT_COMB_ADDR;
				}
			}

			// トレース開始オペランドイベントとチェック
			if ((evBrk->dwBrkde & s_EvCombTrc_RX.dwStartde) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_OPC;
			}

			// トレース終了オペランドイベントとチェック
			if ((evBrk->dwBrkde & s_EvCombTrc_RX.dwEndde) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_OPC;
			}
			// トレースイベント抽出オペランドイベントとチェック
			if ((evBrk->dwBrkde & s_EvCombTrc_RX.dwPickupde) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_OPC;
			}

			// オペランドパフォーマンス開始イベントとチェック
			for(j=0;j< (pMcuInfo->wPPC);j++){
				if ((evBrk->dwBrkde & s_EvCombPerf_RX.dwStartde[j]) != 0x00000000) {
					return FFWERR_FFW_REPEAT_COMB_OPC;
				}
			}
			// オペランドパフォーマンス終了イベントとチェック
			for(j=0;j< (pMcuInfo->wPPC);j++){
				if ((evBrk->dwBrkde & s_EvCombPerf_RX.dwEndde[j]) != 0x00000000) {
					return FFWERR_FFW_REPEAT_COMB_OPC;
				}
			}
		}

		dwEvBuffData = evBrk->dwBrkpe | evBrk->dwBrkde;		// ブレークイベント設定をまとめる
		// RevNo110309-003 Modify Line
		dwShiftData = 0x00000001<<(DWORD)(pMcuInfo->wPCEvent);
		bStateEvSet = FALSE;

		if (dwCombi == COMB_BRK_STATE) {
			// 設定が中抜けしていないかチェック
			// EV0は設定がなくてもよいので、0x2まではチェックを行う
			for (i = 0; dwShiftData > 2; i++) {
				dwShiftData >>= 1;
				if ((dwEvBuffData & dwShiftData) == dwShiftData) {
					bStateEvSet = TRUE;
				} else {
					if (bStateEvSet == TRUE) {
						return FFWERR_FFW_EVNO_STATE;
					}
				}
			}
		}
	}

	return FFWERR_OK;
}

//==============================================================================
/**
 * トレースイベントの設定内容をチェックする。
 * @param  dwCombi         イベントの組合せを指定
 * @param  evTrc           トレースイベント情報を格納するFFWRX_COMB_TRC 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ChkEvTraceData(DWORD dwCombi, FFWRX_COMB_TRC* evTrc)
{
	DWORD					dwShiftData;		// 状態遷移時のイベント設定チェック用
	DWORD					dwEvBuffData;		// 状態遷移時のイベント設定チェック用
	BOOL					bStateEvSet;		// 状態遷移時のイベント設定チェック用
	int						i;
	WORD					j;
	FFWMCU_DBG_DATA_RX*		pDbgData;
	FFWMCU_MCUINFO_DATA_RX*	pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*		pFwCtrl;	// RevRxNo130411-001 Append Line

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pDbgData = GetDbgDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	//トレース開始実行通過PCイベントチェック
	if(evTrc->dwStartpe >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wPCEvent))){
		return FFWERR_FFW_ARG;
	}

	//トレース終了実行通過PCイベントチェック
	if(evTrc->dwEndpe >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wPCEvent))){
		return FFWERR_FFW_ARG;
	}

	// Rev110303-002 Modify Start
	// オペランドイベントはClass3とClass2でことなるためチェックを修正
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {

		//トレース開始オペランドイベントチェック
		if(evTrc->dwStartde >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent))){
			return FFWERR_FFW_ARG;
		}

		//トレース終了オペランドイベントチェック
		if(evTrc->dwEndde >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent))){
			return FFWERR_FFW_ARG;
		}

		//トレース抽出オペランドイベントチェック
		if(evTrc->dwPickupde >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent))){
			return FFWERR_FFW_ARG;
		}
	
	}
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS2) {
		//トレース開始オペランドイベントチェック
		if((evTrc->dwStartde >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent+1))) 
			|| ((evTrc->dwStartde & 0x00000001) == 0x00000001) ){
			return FFWERR_FFW_ARG;
		}

		//トレース終了オペランドイベントチェック
		if((evTrc->dwEndde >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent+1))) 
			|| ((evTrc->dwEndde & 0x00000001) == 0x00000001) ){
			return FFWERR_FFW_ARG;
		}

		//トレース抽出オペランドイベントチェック
		if((evTrc->dwPickupde >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent+1))) 
			|| ((evTrc->dwPickupde & 0x00000001) == 0x00000001) ){
			return FFWERR_FFW_ARG;
		}
	}
	// Rev110303-002 Modify End

	// 実行前PCブレークとの重複チェック(トレース開始)
	if (((evTrc->dwStartpe & s_EvCombBrk_RX.dwPreBrk) != 0x00000000)) {
		if (pDbgData->eOcdArgErrChk == OCD_ARGERRCHK_USE) {		// 組合せ制限エラー検出ありの場合
			return FFWERR_FFW_REPEAT_ADDR_ADDR;
		}
	}
	// 実行前PCブレークとの重複チェック(トレース終了)
	if (((evTrc->dwEndpe & s_EvCombBrk_RX.dwPreBrk) != 0x00000000)) {
		if (pDbgData->eOcdArgErrChk == OCD_ARGERRCHK_USE) {		// 組合せ制限エラー検出ありの場合
			return FFWERR_FFW_REPEAT_ADDR_ADDR;
		}
	}
	// 累積AND,状態遷移指定時のチェック
	if ((dwCombi == COMB_TRC_AND) || (dwCombi == COMB_TRC_STATE)) {
		if (pDbgData->eOcdArgErrChk == OCD_ARGERRCHK_USE) {		// 組合せ制限エラー検出ありの場合
			// 命令実行とオペランドアクセスの重複チェック
			// RevNo110316-002 Modify Start
			// E1/E20 V.1.02では仕様を変えないため、Class2の場合状態遷移のときのみチェックとしているが、
			// 本来はClass3の累積ANDもチェック不要
			// RevRxNo121026-001, RevRxNo130411-001 Modify Line
			if ( ((pFwCtrl->eOcdCls == RX_OCD_CLS2) && (dwCombi == COMB_TRC_STATE)) // Class2の状態遷移の場合
				|| (pFwCtrl->eOcdCls == RX_OCD_CLS3)) {		// Class3の状態遷移/累積ANDの場合
				if (((evTrc->dwStartpe & 0x0000000F) & evTrc->dwStartde) != 0x00000000) {
					return FFWERR_FFW_REPEAT_ADDR_OPC;
				}
			}
			// RevNo110316-002 Modify End

			// トレースイベント開始で使用しているイベントを他のイベントと併用していないかチェック
			// 実行前PCブレークイベントとチェック すでにチェック済
			// 実行後PCブレークイベントとチェック
			if ((evTrc->dwStartpe & s_EvCombBrk_RX.dwBrkpe) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_ADDR;
			}
	
			// トレース終了実行PCイベントとチェック
			if ((evTrc->dwStartpe & evTrc->dwEndpe) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_ADDR;
			}

			// パフォーマンス開始実行PCイベントとチェック
			for(j=0;j< (pMcuInfo->wPPC);j++){
				if ((evTrc->dwStartpe & s_EvCombPerf_RX.dwStartpe[j]) != 0x00000000) {
					return FFWERR_FFW_REPEAT_COMB_ADDR;
				}
			}
			// パフォーマンス終了実行PCイベントとチェック
			for(j=0;j< (pMcuInfo->wPPC);j++){
				if ((evTrc->dwStartpe & s_EvCombPerf_RX.dwEndpe[j]) != 0x00000000) {
					return FFWERR_FFW_REPEAT_COMB_ADDR;
				}
			}

			// 実行後オペランドブレークイベントとチェック
			if ((evTrc->dwStartde & s_EvCombBrk_RX.dwBrkde) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_OPC;
			}

			// トレース終了オペランドイベントとチェック
			if ((evTrc->dwStartde & evTrc->dwEndde) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_OPC;
			}
			// トレースイベント抽出オペランドイベントとチェック
			if ((evTrc->dwStartde & evTrc->dwPickupde) != 0x00000000) {
				return FFWERR_FFW_REPEAT_COMB_OPC;
			}

			// オペランドパフォーマンス開始イベントとチェック
			for(j=0;j< (pMcuInfo->wPPC);j++){
				if ((evTrc->dwStartde & s_EvCombPerf_RX.dwStartde[j]) != 0x00000000) {
					return FFWERR_FFW_REPEAT_COMB_OPC;
				}
			}
			// オペランドパフォーマンス終了イベントとチェック
			for(j=0;j< (pMcuInfo->wPPC);j++){
				if ((evTrc->dwStartde & s_EvCombPerf_RX.dwEndde[j]) != 0x00000000) {
					return FFWERR_FFW_REPEAT_COMB_OPC;
				}
			}
		}

		dwEvBuffData = evTrc->dwStartpe | evTrc->dwStartde;		// トレース開始イベント設定をまとめる
		// RevNo110309-003 Modify Line
		dwShiftData = 0x00000001<<(DWORD)(pMcuInfo->wPCEvent );
		bStateEvSet = FALSE;

		// 状態遷移引数チェック
		if (dwCombi == COMB_TRC_STATE) {
			// 設定が中抜けしていないかチェック
			for (i = 0; dwShiftData > 2; i++) {
				dwShiftData >>= 1;
				if ((dwEvBuffData & dwShiftData) == dwShiftData) {
					bStateEvSet = TRUE;
				} else {
					if (bStateEvSet == TRUE) {
						return FFWERR_FFW_EVNO_STATE;
					}
				}
			}
		}
	}

	return FFWERR_OK;
}
// V.1.02 No.10 Modify End

//==============================================================================
/**
 * パフォーマンスイベントの設定内容をチェックする。
 * @param  dwPerfNo        パフォーマンスCH番号
 * @param  dwCombi         イベントの組合せを指定
 * @param  evPerfTime      パフォーマンス計測イベント情報を格納するFFWRX_COMB_PERFTIME 構造体のアドレスを指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ChkEvPerfData(DWORD dwPerfNo, DWORD dwCombi, FFWRX_COMB_PERFTIME* evPerfTime)
{
	FFWMCU_DBG_DATA_RX*		pDbgData;
	FFWRX_PPCCTRL_DATA*		pPpcCtrl;
	FFWMCU_MCUINFO_DATA_RX*	pMcuInfo;		// MCU情報構造体のポインタ
	FFWMCU_FWCTRL_DATA*		pFwCtrl;	// RevRxNo130411-001 Append Line

	//ワーニング対策
	dwCombi;

	pMcuInfo = GetMcuInfoDataRX();	// MCU情報を取得
	pDbgData = GetDbgDataRX();
	pPpcCtrl = GetPpcCtrlData();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// ユーザプログラム側でパフォーマンス計測カウンタを操作する場合
	if (s_PpcUsrCtrl == FFWRX_EVCOMBI_PPC_USR) {
		return FFWERR_FFW_RERFSET_USE_USER;
	}

	//パフォーマンス開始実行通過PCイベントチェック
	if(evPerfTime->dwStartpe[dwPerfNo] >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wPCEvent))){
		return FFWERR_FFW_ARG;
	}

	//パフォーマンス終了実行通過PCイベントチェック
	if(evPerfTime->dwEndpe[dwPerfNo] >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wPCEvent))){
		return FFWERR_FFW_ARG;
	}

	// Rev110303-002 Modify Start
	// オペランドイベントはClass3とClass2でことなるためチェックを修正
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {

		//パフォーマンス開始オペランドイベントチェック
		if(evPerfTime->dwStartde[dwPerfNo] >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent))){
			return FFWERR_FFW_ARG;
		}
		//パフォーマンス終了オペランドイベントチェック
		if(evPerfTime->dwEndde[dwPerfNo] >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent))){
			return FFWERR_FFW_ARG;
		}
	// RevRxNo130411-001 Modify Line
	} else {
		//パフォーマンス開始オペランドイベントチェック
		if((evPerfTime->dwStartde[dwPerfNo] >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent+1))) 
			|| ((evPerfTime->dwStartde[dwPerfNo] & 0x00000001) == 0x00000001) ){
			return FFWERR_FFW_ARG;
		}

		//パフォーマンス終了オペランドイベントチェック
		if((evPerfTime->dwEndde[dwPerfNo] >= (DWORD)(0x00000001 << (DWORD)(pMcuInfo->wOAEvent+1))) 
			|| ((evPerfTime->dwEndde[dwPerfNo] & 0x00000001) == 0x00000001) ){
			return FFWERR_FFW_ARG;
		}
	}
	// Rev110303-002 Modify End

	// 実行前PCブレークとの重複チェック(パフォーマンスn開始)
	if (((evPerfTime->dwStartpe[dwPerfNo] & s_EvCombBrk_RX.dwPreBrk) != 0x00000000)) {
		if (pDbgData->eOcdArgErrChk == OCD_ARGERRCHK_USE) {		// 組合せ制限エラー検出ありの場合
			return FFWERR_FFW_REPEAT_ADDR_ADDR;
		}
	}
	// 実行前PCブレークとの重複チェック(パフォーマンスn終了)
	if (((evPerfTime->dwEndpe[dwPerfNo] & s_EvCombBrk_RX.dwPreBrk) != 0x00000000)) {
		if (pDbgData->eOcdArgErrChk == OCD_ARGERRCHK_USE) {		// 組合せ制限エラー検出ありの場合
			return FFWERR_FFW_REPEAT_ADDR_ADDR;
		}
	}

	return FFWERR_OK;
}
//==============================================================================
/**
 * イベントの実行前PCブレーク組合せを退避する。
 * @param byMode	0:退避 1:参照 その他:初期化
 * @param dwPreEvCombi	退避時の組合せ設定値 
 * @retval TRUE ブレーク組合せを退避している
 * @retval FALSE ブレーク組合せを退避していない
 */
//==============================================================================
BOOL EscEvPreBrkPoint(BYTE byMode, DWORD* dwPreEvCombi)
{
	static BOOL bEscape = FALSE;

	if (byMode == 0x00) {		// 退避 
		s_dwPreEvCombi = *dwPreEvCombi;
		bEscape = TRUE;
		return TRUE;
	}else if (byMode == 0x01) {		//参照
		*dwPreEvCombi = s_dwPreEvCombi;
		if (bEscape == TRUE) {
			return TRUE;
		}else {
			return FALSE;
		}
	}else {							// 初期化
		s_dwPreEvCombi = 0;
		bEscape = FALSE;
		return FALSE;
	}
}
//==============================================================================
/**
 * 実行PCブレークの設定情報を退避/復帰する。
 * @param  bEscape TRUE:退避, FALSE:復帰
 * @retval なし
 */
//==============================================================================
void EscExecPcBreakEv(BOOL bEscape)
{
	int i;
	// V.1.02 No.10 Class2 イベント対応 Append Start
	FFWRX_MCUINFO_DATA* pMcuInfo;

	//イベント情報取得
	pMcuInfo = GetMcuInfoDataRX();
	// V.1.02 No.10 Class2 イベント対応 Append End

	if (bEscape == TRUE) {	// 退避
		// 命令実行イベントの設定
		// V.1.02 No.10 Class2 イベント対応 Modify Line
		for (i=0;i< pMcuInfo->wPCEvent; i++){
			s_EscEvAddr[i] = s_EvAddr[i];
		}
		// ブレークイベントの設定
		s_EscEvCombBrk_RX.dwPreBrk = s_EvCombBrk_RX.dwPreBrk;
		s_EscEvCombBrk_RX.dwBrkpe = s_EvCombBrk_RX.dwBrkpe;
		s_EscEvCombBrk_RX.dwBrkde = s_EvCombBrk_RX.dwBrkde;
	} else {				// 復帰
		// 命令実行イベントの設定
		// V.1.02 No.10 Class2 イベント対応 Modify Line
		for (i=0;i< pMcuInfo->wPCEvent; i++){
			s_EvAddr[i] = s_EscEvAddr[i];
		}
		// ブレークイベントの設定
		s_EvCombBrk_RX.dwPreBrk = s_EscEvCombBrk_RX.dwPreBrk;
		s_EvCombBrk_RX.dwBrkpe = s_EscEvCombBrk_RX.dwBrkpe;
		s_EvCombBrk_RX.dwBrkde = s_EscEvCombBrk_RX.dwBrkde;
	}
	return;
}

//==============================================================================
/**
 * トレースイベントの設定情報を退避/復帰する。
 * @param  bEscape TRUE:退避, FALSE:復帰
 * @retval なし
 */
//==============================================================================
void EscExecTrcEv(BOOL bEscape)
{
	if (bEscape == TRUE) {	// 退避
		// トレースイベントの設定
		s_EscEvCombTrc_RX.dwStartpe = s_EvCombTrc_RX.dwStartpe;
		s_EscEvCombTrc_RX.dwEndpe   = s_EvCombTrc_RX.dwEndpe;
		s_EscEvCombTrc_RX.dwStartde = s_EvCombTrc_RX.dwStartde;
		s_EscEvCombTrc_RX.dwEndde   = s_EvCombTrc_RX.dwEndde;
		s_EscEvCombTrc_RX.dwPickupde = s_EvCombTrc_RX.dwPickupde;
	} else {				// 復帰
		// トレースイベントの設定
		s_EvCombTrc_RX.dwStartpe = s_EscEvCombTrc_RX.dwStartpe;
		s_EvCombTrc_RX.dwEndpe   = s_EscEvCombTrc_RX.dwEndpe;
		s_EvCombTrc_RX.dwStartde = s_EscEvCombTrc_RX.dwStartde;
		s_EvCombTrc_RX.dwEndde   = s_EscEvCombTrc_RX.dwEndde;
		s_EvCombTrc_RX.dwPickupde = s_EscEvCombTrc_RX.dwPickupde;
	}
	return;
}

//==============================================================================
/**
 * パフォーマンスイベントの設定情報を退避/復帰する。
 * @param  bEscape TRUE:退避, FALSE:復帰
 * @retval なし
 */
//==============================================================================
void EscExecPerfEv(BOOL bEscape)
{

	if (bEscape == TRUE) {	// 退避
		// パフォーマンスイベントの設定
		s_EscEvCombPerf_RX.dwStartpe[0] = s_EvCombPerf_RX.dwStartpe[0];
		s_EscEvCombPerf_RX.dwEndpe[0]   = s_EvCombPerf_RX.dwEndpe[0];
		s_EscEvCombPerf_RX.dwStartpe[1] = s_EvCombPerf_RX.dwStartpe[1];
		s_EscEvCombPerf_RX.dwEndpe[1]   = s_EvCombPerf_RX.dwEndpe[1];
		s_EscEvCombPerf_RX.dwStartde[0] = s_EvCombPerf_RX.dwStartde[0];
		s_EscEvCombPerf_RX.dwEndde[0]   = s_EvCombPerf_RX.dwEndde[0];
		s_EscEvCombPerf_RX.dwStartde[1] = s_EvCombPerf_RX.dwStartde[1];
		s_EscEvCombPerf_RX.dwEndde[1]   = s_EvCombPerf_RX.dwEndde[1];
	} else {				// 復帰
		// パフォーマンスイベントの設定
		s_EvCombPerf_RX.dwStartpe[0] = s_EscEvCombPerf_RX.dwStartpe[0];
		s_EvCombPerf_RX.dwEndpe[0]   = s_EscEvCombPerf_RX.dwEndpe[0];
		s_EvCombPerf_RX.dwStartpe[1] = s_EscEvCombPerf_RX.dwStartpe[1];
		s_EvCombPerf_RX.dwEndpe[1]   = s_EscEvCombPerf_RX.dwEndpe[1];
		s_EvCombPerf_RX.dwStartde[0] = s_EscEvCombPerf_RX.dwStartde[0];
		s_EvCombPerf_RX.dwEndde[0]   = s_EscEvCombPerf_RX.dwEndde[0];
		s_EvCombPerf_RX.dwStartde[1] = s_EscEvCombPerf_RX.dwStartde[1];
		s_EvCombPerf_RX.dwEndde[1]   = s_EscEvCombPerf_RX.dwEndde[1];
	}
	return;
}
//==============================================================================
/**
 * イベントの設定情報を退避/復帰する。
 * @param  byEvKind        変更対象のイベントを指定
 * @param  evBrk           ブレークイベント情報を格納するFFWRX_COMB_BRK 構造体のアドレスを指定する。
 * @param  evTrc           トレースイベント情報を格納するFFWRX_COMB_TRC 構造体のアドレスを指定する。
 * @param  evPerfTime      パフォーマンス計測イベント情報を格納するFFWRX_COMB_PERFTIME 構造体のアドレスを指定する。
 * @param  ePpcUsrCtrl     パフォーマンス計測資源に対するアクセスコントロールを設定する。
 * @param  bEscape TRUE:退避, FALSE:復帰
 * @retval なし
 */
//==============================================================================
void EscExecEv(DWORD byEvKind, FFWRX_COMB_BRK* evBrk, FFWRX_COMB_TRC* evTrc,
										FFWRX_COMB_PERFTIME* evPerfTime, BOOL bEscape)
{
	// RevRxNo121126-002 Modify Start
	// s_EvCombBrk_RX,s_EvCombTrc_RX,s_EvCombPerf_RXの退避は、先にs_EscEvComb～への退避後
	// 引数をs_EvComb～に渡さないと、s_EvComb自体が書き換わってしまう。
	if((byEvKind & 0x02) == 0x02) {		//ブレークイベント退避/復帰
		EscExecPcBreakEv(bEscape);
	}
	if((byEvKind & 0x04) == 0x04) {		//トレースイベント退避/復帰
		EscExecTrcEv(bEscape);
	}
	if((byEvKind & 0x18) != 0x0){		//パフォーマンス退避/復帰
		EscExecPerfEv(bEscape);
	}

	if(bEscape == TRUE) {
		if((byEvKind & 0x02) == 0x02) {
			// ブレークイベントを一時的に変更値へ変更
			s_EvCombBrk_RX.dwPreBrk = evBrk->dwPreBrk;
			s_EvCombBrk_RX.dwBrkpe = evBrk->dwBrkpe;
			s_EvCombBrk_RX.dwBrkde = evBrk->dwBrkde;
		}
		if((byEvKind & 0x04) == 0x04) {
			// トレースイベントを一時的に変更値へ変更
			s_EvCombTrc_RX.dwStartpe = evTrc->dwStartpe;
			s_EvCombTrc_RX.dwEndpe = evTrc->dwEndpe;
			s_EvCombTrc_RX.dwStartde = evTrc->dwStartde;
			s_EvCombTrc_RX.dwEndde = evTrc->dwEndde;
			s_EvCombTrc_RX.dwPickupde = evTrc->dwPickupde;
		}
		if((byEvKind & 0x08) == 0x08) {
			// パフォーマンスイベントを一時的に変更値へ変更
			s_EvCombPerf_RX.dwStartpe[0] = evPerfTime->dwStartpe[0];
			s_EvCombPerf_RX.dwEndpe[0] = evPerfTime->dwEndpe[0];
			s_EvCombPerf_RX.dwStartde[0] = evPerfTime->dwStartde[0];
			s_EvCombPerf_RX.dwEndde[0] = evPerfTime->dwEndde[0];
		}
		if((byEvKind & 0x10) == 0x10) {
			// パフォーマンスイベントを一時的に変更値へ変更
			s_EvCombPerf_RX.dwStartpe[1] = evPerfTime->dwStartpe[1];
			s_EvCombPerf_RX.dwEndpe[1] = evPerfTime->dwEndpe[1];
			s_EvCombPerf_RX.dwStartde[1] = evPerfTime->dwStartde[1];
			s_EvCombPerf_RX.dwEndde[1] = evPerfTime->dwEndde[1];
		}
	}
	// RevRxNo121126-002 Modify End

	return;
}
//==============================================================================
/**
 * 命令実行イベントの設定情報を取得する。
 * @param  dwEvNum イベント番号
 * @param  EvAddr  命令実行イベントの設定情報
 * @retval なし
 */
//==============================================================================
void GetEvAddrInfo(DWORD dwEvNum, FFWRX_EV_ADDR* EvAddr)
{
	memcpy(EvAddr, &s_EvAddr[dwEvNum], sizeof(FFWRX_EV_ADDR));
	return;
}

//==============================================================================
/**
 * イベントカウントの設定情報を取得する。
 * @param  なし
 * @retval イベント家運路の設定情報
 */
//==============================================================================
FFWRX_EVCNT_DATA* GetEvCntInfo(void)
{
	return &s_EvCount_RX;
}
// RevNo110825-01 Append Start
//==============================================================================
/**
 * 指定イベントに回数設定がされているか検索
 * (ただし1回設定は回数設定とみなさない)
 * @param  eEvNum 検索するイベント番号
 * @retval FFWERR
 */
//==============================================================================
BOOL SearchEvCnt(enum FFWRX_EV eEvNum){

	if((s_EvCount_RX.eEvNo == eEvNum) && (s_EvCount_RX.bEnable == TRUE) ){
	//指定イベントが設定有効の場合
		if(s_EvCount_RX.dwCount != 1){
		//回数設定1回以上の場合
			return TRUE;
		}
	}

	return FALSE;
}
// RevNo110825-01 Append End
//==============================================================================
/**
 * イベント組合せの設定情報を取得する。
 * @param  なし
 * @retval イベント組合せの設定情報
 */
//==============================================================================
DWORD GetCombiInfo(void)
{
	return s_dwCombi_RX;
}

//==============================================================================
/**
 * ブレークイベントの設定情報を取得する。
 * @param  なし
 * @retval ブレークイベントの設定情報
 */
//==============================================================================
FFWRX_COMB_BRK* GetEvCombBrkInfo(void)
{
	return &s_EvCombBrk_RX;
}
//==============================================================================
/**
 * ブレークイベントの設定情報を設定する。
 * @param  ブレークイベントの設定情報
 * @retval なし
 */
//==============================================================================
void SetEvCombBrkInfo(FFWRX_COMB_BRK* evBrk)
{
	memcpy(&s_EvCombBrk_RX, evBrk, sizeof(FFWRX_COMB_BRK));
	return;
}
//==============================================================================
/**
 * トレースイベントの設定情報を取得する。
 * @param  なし
 * @retval トレースイベントの設定情報
 */
//==============================================================================
FFWRX_COMB_TRC* GetEvCombTrcInfo(void)
{
	return &s_EvCombTrc_RX;
}
//==============================================================================
/**
 * トレースイベントの設定情報を設定する。
 * @param  トレースイベントの設定情報
 * @retval なし
 */
//==============================================================================
void SetEvCombTrcInfo(FFWRX_COMB_TRC* evTrc)
{
	memcpy(&s_EvCombTrc_RX, evTrc, sizeof(FFWRX_COMB_TRC));
	return;
}
//==============================================================================
/**
 * パフォーマンスイベントの設定情報を取得する。
 * @param  なし
 * @retval パフォーマンスイベントの設定情報
 */
//==============================================================================
FFWRX_COMB_PERFTIME* GetEvCombPerfInfo(void)
{
	return &s_EvCombPerf_RX;
}
//==============================================================================
/**
 * パフォーマンスイベントの設定情報を取得する。
 * @param  パフォーマンスイベントの設定情報
 * @retval なし
 */
//==============================================================================
void SetEvCombPerfInfo(FFWRX_COMB_PERFTIME* evPerf)
{
	memcpy(&s_EvCombPerf_RX, evPerf, sizeof(FFWRX_COMB_PERFTIME));
	return ;
}
// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append Start
//==============================================================================
/**
 * イベントトリガの設定情報を取得する。
 * @param  なし
 * @retval イベントトリガの設定情報
 */
//==============================================================================
FFWRX_TRG_DATA* GetEvTrgInfo(void){
	return &s_EvTrgData_RX;
}
//==============================================================================
/**
 * パフォーマンス測定ユーザ使用フラグを取得。
 * @retval enum FFWRX_EVCOMBI_PPC_USR_CTRL 0:USER 1:EMEL
 */
//==============================================================================
enum FFWRX_EVCOMBI_PPC_USR_CTRL GetPerfUsrCtrl(void)
{
	return s_PpcUsrCtrl;
}
//==============================================================================
/**
 * パフォーマンス測定ユーザ使用フラグを設定。
 * @retval enum FFWRX_EVCOMBI_PPC_USR_CTRL 0:USER 1:EMEL
 */
//==============================================================================
void SetPerfUsrCtrl(enum FFWRX_EVCOMBI_PPC_USR_CTRL PpcUsrCtrl)
{
	s_PpcUsrCtrl = PpcUsrCtrl;
	return;
}
//==============================================================================
/**
 * イベント動作許可レジスタを設定する。
 * @param  dwPreBrk     実行前PCブレークイベント動作許可レジスタへの設定値
 * @param  dwExecPcSet  実行PC通過イベント動作許可レジスタへの設定値
 * @param  dwOpcSet     オペランドアクセスイベント動作許可レジスタへの設定値
 * @retval なし
 */
//==============================================================================
void GetRegEvEnable(DWORD* pdwPreBrk, DWORD* pdwExecPcSet, DWORD* pdwOpcSet)
{
	//RevRxNo120713-001 Append Start 
	DWORD				dwPpcEvNum,dwPpcPcEv,dwPpcOpcEv;
	DWORD				dwExecPcSet,dwOpcSet;

	dwPpcEvNum = GetPpcCntEvNum();
	// 実行PC/オペランドアクセスイベント許可の設定
	dwPpcPcEv = dwPpcEvNum & 0x000000FF;
	dwPpcOpcEv = (dwPpcEvNum & 0x000F0000) >> 16;
	dwExecPcSet = s_EvCombBrk_RX.dwBrkpe |
					s_EvCombTrc_RX.dwStartpe | s_EvCombTrc_RX.dwEndpe |
					s_EvCombPerf_RX.dwStartpe[0] | s_EvCombPerf_RX.dwEndpe[0] |
					s_EvCombPerf_RX.dwStartpe[1] | s_EvCombPerf_RX.dwEndpe[1] |
					dwPpcPcEv;
	dwOpcSet = s_EvCombBrk_RX.dwBrkde |
				s_EvCombTrc_RX.dwStartde | s_EvCombTrc_RX.dwEndde | s_EvCombTrc_RX.dwPickupde |
				s_EvCombPerf_RX.dwStartde[0] | s_EvCombPerf_RX.dwEndde[0] |
				s_EvCombPerf_RX.dwStartde[1] | s_EvCombPerf_RX.dwEndde[1] |
				dwPpcOpcEv;
	//RevRxNo120713-001 Append End

	(*pdwPreBrk) = s_EvCombBrk_RX.dwPreBrk;
	//RevRxNo120713-001 Modify End
	(*pdwExecPcSet) = dwExecPcSet;
	(*pdwOpcSet) = dwOpcSet;
	//RevRxNo120713-001 Modify End
	return;
}
//RevRxNo120713-001 Delete SetEveEXECPCSET(),SetEveOPCSET()
// V.1.02 No.10 Class2 トレース対応 Append Start
//==============================================================================
/**
 * オペランドイベント情報取得。
 * @param  dwEvNum イベント番号
 * @retval オペランドイベント情報格納ポインタ
 */
//==============================================================================
FFWRX_EV_OPC* GetEvOpc(DWORD dwEvNum){

	BYTE	byEvNum;

	switch(dwEvNum){
	case DE0:
		byEvNum = 0;
		break;
	case DE1:
		byEvNum = 1;
		break;
	case DE2:
		byEvNum = 2;
		break;
	case DE3:
		byEvNum = 3;
		break;
	default:
		byEvNum = (BYTE)dwEvNum;
		break;
	}

	return &s_EvOpc[byEvNum];
}
// V.1.02 No.10 Class2 トレース対応 Append End

//=============================================================================
/**
 * イベント関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfRxData_Ev(void)
{

	int		i;

	// 命令実行イベントの管理変数
	for (i = 0; i < EV_ADDR_MAX_RX; i++) {
		s_EvAddr[i].dwAddr = 0;
	}

	// オペランドアクセスイベントの管理変数
	for (i = 0; i < EV_OPC_MAX_RX; i++) {
		s_EvOpc[i].byKindSelect = EV_MODE_ADDR;
		s_EvOpc[i].dwAddrStart = 0;
		s_EvOpc[i].dwAddrEnd = 0;
		s_EvOpc[i].dwDataValue = 0;
		s_EvOpc[i].dwDataMask = 0;
		//RevNo100715-023 Modify Start
		s_EvOpc[i].byDataSize = EV_DATASIZE_BYTE;
		s_EvOpc[i].byDataCmp = EV_CMPMASK_AGREE;
		s_EvOpc[i].byAddrCmp = EV_CMPMASK_AGREE;
		s_EvOpc[i].byAccess = EV_ACCESS_W;
		// RevRxNo120910-001 Append Line
		s_EvOpc[i].dwBusSel = EV_BUSSEL_CPU_BUS;
		//RevNo100715-023 Modify End
	}

	// イベント成立回数の管理変数
	s_EvCount_RX.bEnable = FALSE;
	s_EvCount_RX.eEvNo = FFWRX_PE0;
	s_EvCount_RX.dwCount = 1;

	// イベントトリガの設定管理変数
	s_EvTrgData_RX.byEvKind = 0;
	s_EvTrgData_RX.dwTrcTrg = 0;
	s_EvTrgData_RX.dwPerfTrg = 0;

	// イベント組合せの設定管理変数
	s_dwCombi_RX = COMB_OR;

	// ブレークイベントの設定管理変数
	s_EvCombBrk_RX.dwPreBrk = 0;
	s_EvCombBrk_RX.dwBrkpe = 0;
	s_EvCombBrk_RX.dwBrkde = 0;

	// トレースイベントの設定管理変数
	s_EvCombTrc_RX.dwStartpe = 0;
	s_EvCombTrc_RX.dwEndpe = 0;
	s_EvCombTrc_RX.dwStartde = 0;
	s_EvCombTrc_RX.dwEndde = 0;
	s_EvCombTrc_RX.dwPickupde = 0;

	// パフォーマンス計測イベントの設定管理変数
	for (i = 0; i < 2; i++) {
		s_EvCombPerf_RX.dwStartpe[i] = 0;
		s_EvCombPerf_RX.dwEndpe[i] = 0;
		s_EvCombPerf_RX.dwStartde[i] = 0;
		s_EvCombPerf_RX.dwEndde[i] = 0;
	}

	// パフォーマンス計測アクセスコントロールの管理変数
	s_PpcUsrCtrl = FFWRX_EVCOMBI_PPC_USR;

	// 退避/復帰用の命令実行イベント管理変数
	for (i = 0; i < EV_ADDR_MAX_RX; i++) {
		s_EscEvAddr[i].dwAddr = 0;
	}

	// 退避/復帰用のブレークイベント設定管理変数
	s_EscEvCombBrk_RX.dwPreBrk = 0;
	s_EscEvCombBrk_RX.dwBrkpe = 0;
	s_EscEvCombBrk_RX.dwBrkde = 0;

	// RevRxNo120713-001 Delete s_dwExecPcSet,s_dwExecOpcSet
	// RevNo110224-003 Append Start
	s_dwPreEvCombi = 0;						// 退避用実行前PCブレーク組み合わせ管理変数
	// RevNo110224-003 Append End
	return;

}
