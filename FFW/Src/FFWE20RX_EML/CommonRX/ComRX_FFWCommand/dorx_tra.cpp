////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_tra.cpp
 * @brief RX共通トレース関連コマンドのヘッダファイル
 * @author RSD H.Hashiguchi, H.Akashi, S.Ueda, SDS T.Iwata, K.Okita, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/11/01
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RX220 WS対応 RX210でRX220対応 2012/07/11 橋口
・RevRxNo120130-002 タイムスタンプ解析異常修正 2012/07/11 橋口
・RevRxNo120626-001 2012/07/12 橋口
　RX63TH トレース駆動能力対応
・RevRxNo120606-005 2012/07/12 橋口
  RX Class2トレース オペランドトレースイベント番号がずれる不具合対応
・RevNo120410-001 2012/07/12 橋口
  RX Class3内蔵トレース BTR3サイクルのタイムスタンプ値異常対応
・RevRxNo120629-001 2012/07/12 橋口
　外部トレース 分岐元+分岐先サイクル出力時の分岐先アドレス解析異常 修正RevNoのみ追記
・RevNo120727-001 2012/07/27 橋口
	内蔵ROM DUMP 高速化
・RevNo121017-004	2012/11/16 明石、上田
　VS2008対応エラーコード　WARNING C4554対策
・RevRxNo121022-001	2012/10/22 SDS 岩田
　　DO_SetRXRM(), setTraceModeData(), SetTrace(), ClrTrcReg(), GetTrcInramMode(), StopE20Trc()関数に
　　EZ-CUBE用処理を追加
・RevRxNo121022-001	2012/10/31 SDS 岩田
　　clrRXCls3RD()関数のE20上のトレースメモリクリア処理を、E20のみで実施に変更
・RevRxNo121022-001	2012/11/1 SDS 岩田
　　DO_SetRXRM(), setTraceModeData(), SetTrace(), ClrTrcReg(), GetTrcInramMode(), StopE20Trc()関数の
　　エミュレータ種別 EZ-CUBE判定処理の定義名を変更
・RevNo121017-004 2012/11/15 橋口
　　RX200トレースでトレースサイクル数が半分ぐらいになる不具合修正
　　VS2008対応エラーコード　WARNING C4554対策を実施したときに、以下の不具合を埋め込んだため修正した。
・RevRxNo121227-001	2012/12/27 上田
　　FREEトレース中のRCY時、トレースメモリフルフラグのクリア処理追加。
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001	2013/09/06 上田
	RX64M対応
・RevRxNo130730-006	2013/11/18 上田
	E20トレースクロック遅延設定タイミング変更
・RevRxNo130730-008 2013/11/15 大喜多
	エラーで返る前にメモリ解放処理を追加
・RevRxNo130408-001	2014/04/22 上田
	外部トレースデータ8ビット幅対応
・RevRxE2LNo141104-001 2014/12/24 上田
	E2 Lite対応
・RevRxNo161003-001	2016/11/01 PA 辻
	ROMデータトレースアクセスタイプ出力対応
*/
#include "dorx_tra.h"
#include "ffwrx_tra.h"
#include "protrx_tra.h"
#include "domcu_prog.h"
#include "prot_common.h"
#include "do_sys.h"
#include "domcu_mcu.h"
#include "ffwrx_ev.h"
#include "hwrx_fpga_tra.h"
#include "mcurx_tra.h"
#include "mcurx_ev.h"
#include "ocdspec.h"
#include "fpga_tra.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line
#include "domcu_rst.h"	// RevRxNo130301-001 Append Line
#include "mcu_rst.h"	// RevRxNo130301-001 Append Line

///// static変数の宣言
// トレースサイクル取得関連
static FFWRX_RCY_DATA s_rcyData;				// トレースサイクル数の内部データ
static BOOL  s_bTracSetRCY;						// トレースサイクルの取得状態を示すフラグ

// トレースデータ取得関連
static DWORD s_dwTraceInfo;						// トレース出力情報の設定状態

// 外部トレース関連
static RD_INDEX s_dwRdIndex[RD_TRCMEM_INDEX];	// トレースインデックス値
static DWORD s_dwRdIndexSBlkPos;				// トレースインデックスの開始ブロック位置
static DWORD s_dwRdIndexEBlkPos;				// トレースインデックスの終了ブロック位置
static int   s_nNonCycleCnt;					// トレース無効サイクル数
static int	 s_nPreBrAddr;						// 直前のアドレス情報(分岐命令用)
static int	 s_nPreDtAddr;						// 直前のアドレス情報(データアクセス用)
static int   s_nPreTimeStamp;					// 直前のタイムスタンプ情報
static BOOL	 s_bLostBrAddr;						// トレース分岐アドレスLost状態示すフラグ
static BOOL	 s_bLostDtAddr;						// トレースオペランドアドレスLost状態示すフラグ
static BOOL	 s_bGetBrAddr;						// トレースフルアドレス取得したフラグ(分岐用)
static BOOL	 s_bGetDtAddr;						// トレースフルアドレス取得したフラグ(データアクセス用)
static BOOL	 s_bGetTimeStamp;					// トレース32bitタイムスタンプ取得したフラグ
static DWORD s_dwTrcCmdCnt;						// トレースメモリ内のパケット数
// V.1.02 No.36 タイムスタンプ解析修正 Append Start
static WORD s_wTmwaEnd;							//メモリ終了ブロック位置
static BOOL s_bE20TrcMemOvr;					//E20トレースメモリオーバーフロー発生フラグ
// V.1.02 No.36 タイムスタンプ解析修正 Append End
static BOOL	 s_bDummyTrcPkt;					// ダミートレースパケットが残っているよフラグ
static BOOL  s_bSetDummyPkt;					// ダミートレースパケット設定したよフラグ
static BOOL  s_bSetDummyPktBtr1;				// BTR1出力用 ダミートレースパケット設定したよフラグ
static float s_fTrcClkClc;
static WORD  s_wUsedMemSize;
// RevNo110309-005 Append Line
static WORD	 s_bUsedTrace;			// Clr後は必ず128Mトレースメモリをクリアする
// RevRxNo130408-001 Append Start
static WORD	s_wSetE20TrcMemSize;				// E20トレースメモリの使用サイズ
static WORD	s_wTrDataCmdInfStnby;				// トレースメモリデータのトレース情報種別定義値(無効データ)
static WORD	s_wTrDataCmdInfIdnt1;				// トレースメモリデータのトレース情報種別定義値(識別子1)
static WORD	s_wTrDataCmdInfIdnt2;				// トレースメモリデータのトレース情報種別定義値(識別子2)
static WORD	s_wTrDataCmdInfIdnt3_1;				// トレースメモリデータのトレース情報種別定義値(識別子3-1)
static WORD	s_wTrDataCmdInfIdnt3_2;				// トレースメモリデータのトレース情報種別定義値(識別子3-2)
static WORD	s_wTrDataCmdInfData;				// トレースメモリデータのトレース情報種別定義値(データ)
// RevRxNo130408-001 Append End

// 内蔵トレース関連
static DWORD *s_pdwTrcDataInram;
static BOOL s_bGetTrcDataInram;
static DWORD s_bOff;



///// static関数の宣言
//トレースモード設定関連
static FFWERR setTraceModeData(DWORD dwMode, DWORD* dwData);
static FFWERR setCls2TraceModeData(DWORD dwMode, DWORD* dwData);
static BOOL checkTrcModeInramToExtram(void);	// RevRxNo130301-001 Append Line

//トレースモード設定関連 (外部トレース用)
static void setTrcDataWidthCtrlInfo(void);	// RevRxNo130408-001 Append Line

//トレースサイクル取得関連
// V.1.02 No.31 Class2 トレース対応 Append Start
static FFWERR getRXCls3RCY(FFWRX_RCY_DATA *pRcy);
static FFWERR getRXCls2RCY(FFWRX_RCY_DATA *pRcy);
// V.1.02 No.31 Class2 トレース対応 Append End

//トレースサイクル取得関連 (外部トレース用)
static DWORD getTraceCycleNumMax(void);	// RevRxNo130408-001 Append Line
static FFWERR getTraceCycle(FFWRX_RCY_DATA *pRcy);
static int cntTraceBuffCycle(WORD* pwRd, DWORD dwRdSize);
static FFWERR getTraceAllBlockCycleDiv(DWORD* pdwTrcCmdCntBuf);
static FFWERR getTraceNonCycle(DWORD dwBlock, DWORD* pdwRetCnt, DWORD* pdwRetNonCnt);
static void analysisNonTrcData(WORD* wRd, DWORD dwRdSize, DWORD* pdwRetCnt);
static void analysisNonAddrTrcData(WORD* wRd, DWORD dwRdSize, DWORD* pdwRetCnt);

//トレースサイクル取得関連 (内部トレース Class3用)
static FFWERR getTraceCycleTrbf(FFWRX_RCY_DATA *pRcy);
static void analysisTrcCycleTrbf(DWORD* pdwRd, DWORD* dwCycle);

//トレースサイクル取得関連 (内部トレース Class2用)
static FFWERR getTraceCycleTrbfCls2(FFWRX_RCY_DATA *pRcy);

//トレースデータ解析関連
// V.1.02 No.31 Class2 トレース対応 Append Start
static FFWERR getRXCls3RD(DWORD dwStartCyc, DWORD dwEndCyc, FFWRX_RD_DATA* pRd);
static FFWERR getRXCls2RD(DWORD dwStartCyc, DWORD dwEndCyc, FFWRX_RD_DATA* pRd);
// V.1.02 No.31 Class2 トレース対応 Append End
static void setTraceType(FFWRX_RD_DATA* pRd, WORD wCmdInf);
static void getTraceInfo(void);

//トレースデータ解析関連 (外部トレース用)
static FFWERR getTrcData(FFWRX_RD_DATA* pRd, DWORD dwStartBlock, DWORD dwEndBlock, int nStartOffset, int nEndOffset);
static FFWERR getTrcIndex(DWORD dwFindCyc, DWORD dwSrchStartBlock, DWORD dwSrchEndBlock, DWORD* p_dwGetBlock);
static DWORD getBlockEnd(void);	// RevRxNo130408-001 Modify Line
static DWORD getTrcMemByteSize(void);	// RevRxNo130408-001 Append Line
static FFWERR analysisTrcData(FFWRX_RD_DATA* pRd, WORD* wRd, DWORD dwRdSize, DWORD dwSBlockNo, int nStartOffset, int nEndOffset);
static BOOL searchFullAddr(WORD wKind, WORD* wRd, DWORD dwStartPos, DWORD dwEndPos);
static FFWERR searchFwFullAddr(DWORD dwSBlockNo, DWORD dwEBlockNo, BOOL* pbRet);
static void setTraceDataBTR1(FFWRX_RD_DATA* pRd, WORD* wRd, int nTrcCmd3, int* nShiftCnt, int nDataCnt);
static void setTraceDataBTR2(FFWRX_RD_DATA* pRd, WORD* wRd, int nTrcCmd3, DWORD* dwTmShiftCnt, int* nShiftCnt, int nDataCnt, DWORD* dwAddrBuff);
static void setTraceDataBTR3(FFWRX_RD_DATA* pRd, WORD* wRd, int* nShiftCnt, int nDataCnt);
static void setTraceDataDTRW(FFWRX_RD_DATA* pRd, WORD* wRd, WORD wTrcCmdKind, int nTrcCmd3, DWORD* dwTmShiftCnt,  DWORD* dwAddrShiftCnt, int* nShiftCnt, int nDataCnt);
static void setTraceDataHWDBG(FFWRX_RD_DATA* pRd, WORD* wRd, int nTrcCmd3, DWORD* dwDestAddrCnt, int* nDestShiftCnt, DWORD* dwSrcAddrCnt, int* nSrcShiftCnt, int nDataCnt, DWORD dwBrType);
static DWORD getBitCompressInfo(BYTE byCompSize);
static void setTimeStampComplement(BYTE byCompSize, DWORD* pdwTimeStamp);
static BOOL setAddrInfoComplement(DWORD dwCompSize, DWORD* pdwAddrInfo, DWORD dwCmdKind, BOOL bSetAddr);
static FFWERR getFullAddr(WORD wKind, DWORD dwSBlockNo, DWORD dwEBlockNo);
static void setDataAccessInfo(WORD wCmdKind, DWORD* pdwAccess, DWORD* pdwAccessSize);		// RevRxNo161003-001 Append Line

//トレースデータ解析関連 (内部トレース Class3用)
static FFWERR getTrcDataTrbf(FFWRX_RD_DATA* pRd,int nStartOffset,int nEndOffset);
static void analysisTrcDataTrbf(FFWRX_RD_DATA* pRd, DWORD* dwRd, DWORD dwRdSize, int nStartOffset);

//トレースデータ解析関連 (内部トレース Class2用)
static void analysisTrcDataTrbfCls2(FFWRX_RD_DATA* pRd, DWORD* pdwRd, DWORD dwStartCyc, DWORD dwRdSize);
static void setTrcRdOprCls2(FFWRX_RD_DATA* pRd, WORD wTrcType, DWORD dwHead, DWORD dwData,DWORD dwDataTs );

// トレースデータクリア関連
static void clrTrcDataInfo(void);	// RevRxE2LNo141104-001 Append Line
static FFWERR clrRXCls3RD(void);
static FFWERR clrRXCls2RD(void);

// トレース停止/再開関連
static FFWERR setTrStp(enum FFWENM_TRSTP eTrstp);
static FFWERR setTrRst(void);
static FFWERR setTrcDummyDataBTR1(void);


//==============================================================================
/**
 * トレースモードを設定する。Class3用
 * @param pTraceMode トレースモードを格納するFFWRX_RM_DATA 構造体のアドレス
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWERR DO_SetRXRM(const FFWRX_RM_DATA* pTraceMode)
{
	FFWERR						ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	BOOL						bRet;					// プログラム実行状態
	FFWE20_EINF_DATA			einfData;
	DWORD						dwMode;
	DWORD						dwSetMode,dwSetInitial;
	DWORD						dwModeData;
	DWORD						dwBuff = 0;
	DWORD						dwTrcfrq = 0;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;

	ProtInit();

	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// Run中かの判定
	//RevNo120727-001 Modify Line
	bRet = GetMcuRunState();

	dwMode = pTraceMode->dwSetMode >> 24;		// トレースモード取得

	// トレースモードとピン数の関連チェック
	getEinfData(&einfData);						// エミュレータ情報取得

	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続以外の場合
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
		if ((dwMode == MODE2) || (dwMode == MODE3) || (dwMode == MODE6)) {
			ferrEnd = ProtEnd();
			return FFWERR_MODE_AND_PIN;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// トレースモード設定チェック
	if ((dwMode >= MODE1) && (dwMode <= MODE10)) {
		// 固定値を設定
		ferr = setTraceModeData(dwMode, &dwModeData);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		dwSetMode = ((dwMode << 24) | dwModeData);
	} else {
		dwSetMode = pTraceMode->dwSetMode;
	}

	// 内部変数にセット
	SetRmModeData(dwSetMode);

	// V.1.02 No.31 Class2 トレース対応 Modify Start
	// トレースクロックのチェック
	pMcuInfoData = GetMcuInfoDataRX();
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eTrcClkCtrl == RX_TRCLK_CTRL_RX630) {
	// RX630系のクロック系の場合
		if(pMcuInfoData->eExTAL == TRC_EXTAL_DIS){
			// ExTALを使用しない場合は　ExTAL*1を選択
			dwBuff = (pTraceMode->dwSetInitial & ~RM_SETINITIAL_TRFS);
			dwSetInitial = dwBuff;
		} else {
			// ExTALを使用する場合は　ExTAL*1/2を選択
			dwBuff = (pTraceMode->dwSetInitial & ~RM_SETINITIAL_TRFS);
			dwSetInitial = dwBuff | 0x00000010;
		}
	// RevRxNo130411-001 Modify Line
	} else if (pFwCtrl->eTrcClkCtrl == RX_TRCLK_CTRL_RX610){
	// RX610系のクロック系の場合
		dwBuff = (pTraceMode->dwSetInitial & RM_SETINITIAL_TRFS) >> 4;
		ferr = CheckTrcClkFrq(dwBuff, &dwTrcfrq);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		dwBuff = (pTraceMode->dwSetInitial & ~RM_SETINITIAL_TRFS);
		dwSetInitial = dwBuff | (dwTrcfrq << 4);
	} else {
	// RX210系のクロック系の場合
		dwBuff = (pTraceMode->dwSetInitial & ~RM_SETINITIAL_TRFS);
		dwSetInitial = dwBuff;
	}
	
	// SetInitialの設定入れなおし
	SetRmInitialData(dwSetInitial);

	// トレースデータ出力端子幅に依存する情報の設定
	setTrcDataWidthCtrlInfo();	// RevRxNo130408-001 Append Line

	// RevNo110331-001 Append Start
	// ホットプラグ動作中は内部変数に設定値を渡すところで正常終了させる
	if ((bHotPlugState == TRUE) && (bIdCodeResultState == TRUE)) {
		// 次回FPGAレジスタ設定時、ホットプラグ動作中に設定されたトレース情報のFPGAレジスタ設定が
		// 必要であることをフラグに設定
		SetNeedTrcE20FpgaHotplug(TRUE);	// RevRxNo130730-006 Append Line

		ferrEnd = ProtEnd();
		return FFWERR_OK;
	}
	// RevNo110331-001 Append End

	//RevNo1000805-001 Modify Start
	//E20 トレースFPGAの設定を入力
	if(GetTrcInramMode() == FALSE){
		//外部トレース時のみ実行
		ferr = SetTrcE20FpgaData();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	//RX OCD レジスタの設定
	if(bRet == FALSE){
		ferr = SetTrcOcdData();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		// RevRxNo130301-001 Append Start
		// 外部トレース切り替え時にデバッグモジュールリセットが必要なMCUの場合
		// デバッグモジュールリセット後、OCDレジスタを再設定する
		if (pFwCtrl->bResetDebugModuleExtTrc == TRUE) {
			if (checkTrcModeInramToExtram() == TRUE) {
				// トレースモードを内部トレースから外部トレースへ切り替えた場合

				// OCDのデバッグモジュールリセット
				ferr = OcdDbgModuleReset();
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}

				// OCDレジスタ再設定
				ferr = SysReSet();
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
		}
		// RevRxNo130301-001 Append End
	}
	//RevNo1000805-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	
	return ferr;

}

// V.1.02 No.31 Class2 トレース対応 Append Start
//==============================================================================
/**
 * トレースモードを設定する。Class2用
 * @param pTraceMode トレースモードを格納するFFWRX_RM_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetRXCls2RM(const FFWRX_RM_DATA* pTraceMode)
{
	FFWERR						ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	DWORD						dwMode;
	DWORD						dwSetMode = 0;
	DWORD						dwModeData;

	ProtInit();

	dwMode = pTraceMode->dwSetMode >> 24;		// トレースモード取得

	// トレースモード設定チェック
	if (dwMode >= MODE1 && dwMode <= MODE10) {
		// 固定値を設定
		ferr = setCls2TraceModeData(dwMode, &dwModeData);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		dwSetMode = ((dwMode << 24) | dwModeData);
	}

	// 内部変数にセット
	SetRmModeData(dwSetMode);

	//RX OCD レジスタの設定
	ferr = SetTrcCls2OcdData();
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	return ferr;

}
// V.1.02 No.31 Class2 トレース対応 Append End



// V.1.02 No.31 Class2 トレース対応 Modify Start
//==============================================================================
/**
 * トレースメモリの内容を取得する。
 * @param dwStartCyc 開始サイクルの指定
 * @param dwEndCyc   終了サイクルの指定
 * @param pdwSetInfo トレース出力情報の設定状態を取得
 * @param rd         取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_GetRXRD(DWORD dwStartCyc, DWORD dwEndCyc, DWORD* pdwSetInfo, FFWRX_RD_DATA* pRd)
{
	FFWERR				ferr;
	FFWERR				ferrEnd;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	ProtInit();

	getTraceInfo();

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		//OCD Class3の場合
		ferr = getRXCls3RD(dwStartCyc, dwEndCyc, pRd);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	} else {
		//OCD Class2の場合
		ferr = getRXCls2RD(dwStartCyc, dwEndCyc, pRd);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	
	(*pdwSetInfo) = s_dwTraceInfo;

	ferrEnd = ProtEnd();

	return ferrEnd;

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
FFWERR DO_GetRD2_SPL(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* rd)
{

	FFWERR				ferr;
	FFWERR				ferrEnd;

	ProtInit();


	/* トレースデータを取得 */
	ferr = GetTrcFpgaMemBlock(dwBlockStart, dwBlockEnd, rd);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();

	return ferrEnd;

}

// V.1.02 No.31 Class2 トレース対応 Modify Start
//==============================================================================
/**
 * RCLコマンド発行時のトレースデータクリア処理
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_ClrRD(void)
{
	// RevRxE2LNo141104-001 Modify Start
	// RCLコマンド処理では、FFW内部変数のみ初期化するよう変更。
	// H/Wメモリを含めたトレースデータクリア時はClrTrcMemData()を使用する。
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ProtInit();

	clrTrcDataInfo();	// トレースサイクス情報、トレースデータ関連FFW内部変数を初期化

	ferrEnd = ProtEnd();

	return ferr;
	// RevRxE2LNo141104-001 Modify End
}

//==============================================================================
/**
 * トレースメモリに格納されたデータの開始サイクルと終了サイクルの情報を取得する。
 * @param pRcy トレースメモリに格納されたトレースデータの開始サイクル、
 *             終了サイクルを格納するFFW_RCY_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_GetRXRCY(FFWRX_RCY_DATA *pRcy)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	ProtInit();

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		// Class 3の場合
		ferr = getRXCls3RCY(pRcy);
		if(ferr != FFWERR_OK){
			ferrEnd = ProtEnd();
			return ferr;
		}
	} else {
		// Class 2の場合
		ferr = getRXCls2RCY(pRcy);
		if(ferr != FFWERR_OK){
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;

}

// 2008.10.30 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * トレース入力を停止する。
 * @param  eTrstp         トレース入力の停止/再開を指定
 * @param  bSetDmmyBTR1   BTR1吐き出し処理のTRUE:実施 FALSE:未実施
 * @retval FFWエラーコード
 */
//==============================================================================
// RevNo110322-001	Modifiy Line
FFWERR DO_SetRXTRSTP(enum FFWENM_TRSTP eTrstp, BOOL bSetDmmyBTR1)
{
	FFWERR						ferr;
	FFWERR						ferrEnd;
	DWORD dwBuff = 0;
	BOOL bRet = FALSE;

	ProtInit();

	// 内蔵トレース時はトレースストップ機能使用不可
	if (GetTrcInramMode() == TRUE) {
		ferrEnd = ProtEnd();
		return FFWERR_RTTMODE_UNSUPRT;
	}

	//RevNo1000805-001 Modify Start
	dwBuff = eTrstp;
	if (eTrstp == TRSTP_RESTART){
		// トレースリスタート処理
		// トレースデータクリア
		ferr = ClrTrcMemData();	// RevRxE2LNo141104-001 Modify Line
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// トレースデータクリア後トレースを有効にする。
		ferr = setTrRst();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		ClrTraceDataClrFlg();	// RCLコマンドによるトレースデータクリア未実行

	} else if( (eTrstp == TRSTP_STOP_KEEP) || (eTrstp == TRSTP_STOP_CLR) ){
		// トレース停止処理
		// タイムスタンプOFFのブレーク後にBTR1を吐き出さない対策 RUN中は実行できない
		// 処理高速化のため、IsMcuRun->GetMcuRunStateに変更
		bRet = GetMcuRunState();
		// RevNo110322-001	Modify Line
		if ((bRet == FALSE) && (bSetDmmyBTR1 == TRUE)) {
			// タイムスタンプを出力しない場合
			if ( (s_dwTraceInfo & RM_SETINFO_TSE) != RM_SETINFO_TSE) {
				ferr = setTrcDummyDataBTR1();
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
		}
	//RevNo1000805-001 Modify End
		ferr = setTrStp(eTrstp);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	} else {
		return FFWERR_FFW_ARG;
	}

	ferrEnd = ProtEnd();

	return ferrEnd;

}
// 2008.10.30 INSERT_END_E20RX600 }

// RevRxE2LNo141104-001 Modify Start
// DO_ClrRD()をFFW内部処理のトレースデータクリア関数に変更
//==============================================================================
/**
 * トレースデータをクリア(H/Wメモリ、FFW変数のクリア)
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcMemData(void)
{
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	clrTrcDataInfo();	// トレースサイクス情報、トレースデータ関連FFW内部変数を初期化

	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
		//OCD Class3の場合
		ferr = clrRXCls3RD();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	} else {
		//OCD Class2の場合
		ferr = clrRXCls2RD();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	return ferr;
}
// RevRxE2LNo141104-001 Modify End

// RevRxNo130301-001 Append Start
//==============================================================================
/**
 * トレースモードを内部から外部へ切り替えたかを確認する
 * @param なし
 * @retval TRUE トレースモードを内部から外部へ切り替えた
 * @retval FALSE トレースモードを内部から外部へ切り替えていない
 */
//==============================================================================
static BOOL checkTrcModeInramToExtram(void)
{
	BOOL	bInramToExtram = FALSE;
	FFWRX_RM_OLDMODE_DATA*	pRmOldMode;

	pRmOldMode = GetRmOldMode();

	if (pRmOldMode->bSetMode == TRUE) {	// 前回のトレースモードを設定済みの場合
		// 現在のトレースモードが外部トレースの場合
		if (GetTrcInramMode() == FALSE) {
			// 前回のトレースモードが内部トレースの場合
				// ※GetTrcInramMode()がFALSEのとき、必ずE20+38pinであるため、
				// ここではエミュレータ種別、接続ケーブルの確認はしない。
			if ((pRmOldMode->dwMode >= MODE7) && (pRmOldMode->dwMode <= MODE9)) {
				bInramToExtram = TRUE;
			}
		}
	}

	return bInramToExtram;
}
// RevRxNo130301-001 Append End

// RevRxNo130408-001 Append Start
//==============================================================================
/**
 * トレースデータ出力端子幅依存制御情報の設定
 * @param なし
 * @retval なし
 */
//==============================================================================
static void setTrcDataWidthCtrlInfo(void)
{
	FFWRX_RM_DATA*	pRm;
	FFWMCU_DBG_DATA_RX*	pDbg;
	WORD	wData;

	pRm = GetRmDataInfo();
 	pDbg = GetDbgDataRX();

	// E20トレースメモリ使用サイズの設定
	if ((pDbg->eTrcFuncMode == EML_TRCFUNC_TRC) && ((pRm->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D8S2)) {
				// トレース機能有効で、DATA 8ビット、SYNC 2ビットの場合
		wData = static_cast<WORD>(pRm->eEmlTrcMemSize + 1);
		if (wData > E20_TMSIZE_1MB) {
			wData = E20_TMSIZE_1MB;
		}
		s_wSetE20TrcMemSize = wData;

	} else {	// カバレッジ機能有効、またはDATA 4ビット、SYNC 1ビットの場合
		s_wSetE20TrcMemSize = static_cast<WORD>(pRm->eEmlTrcMemSize);
	}

	// トレース情報種別定義値の設定
	if ((pRm->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D4S1) {	// DATA 4ビット、SYNC 1ビットの場合
		s_wTrDataCmdInfStnby = TRCDATA_CMD_INF_STNBY_D4S1;		// 無効データ
		s_wTrDataCmdInfIdnt1 = TRCDATA_CMD_INF_IDNT1_D4S1;		// 識別子1
		s_wTrDataCmdInfIdnt2 = TRCDATA_CMD_INF_IDNT2_D4S1;		// 識別子2
		s_wTrDataCmdInfIdnt3_1 = TRCDATA_CMD_INF_IDNT3_1_D4S1;	// 識別子3-1
		s_wTrDataCmdInfIdnt3_2 = TRCDATA_CMD_INF_IDNT3_2_D4S1;	// 識別子3-2
		s_wTrDataCmdInfData = TRCDATA_CMD_INF_DATA_D4S1;		// データ

	} else {	// DATA 8ビット、SYNC 2ビットの場合
		s_wTrDataCmdInfStnby = TRCDATA_CMD_INF_STNBY_D8S2;		// 無効データ
		s_wTrDataCmdInfIdnt1 = TRCDATA_CMD_INF_IDNT1_D8S2;		// 識別子1
		s_wTrDataCmdInfIdnt2 = TRCDATA_CMD_INF_IDNT2_D8S2;		// 識別子2
		s_wTrDataCmdInfIdnt3_1 = TRCDATA_CMD_INF_IDNT3_1_D8S2;	// 識別子3-1
		s_wTrDataCmdInfIdnt3_2 = TRCDATA_CMD_INF_IDNT3_2_D8S2;	// 識別子3-2
		s_wTrDataCmdInfData = TRCDATA_CMD_INF_DATA_D8S2;		// データ
	}

	return;
}
// RevRxNo130408-001 Append End


// 2008.10.8 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * トレースインデックスを作成しながら、サイクルを探す。
 * @param dwFindCyc インデックスを見つけるサイクル
 * @param dwSrchEndBlock トレースメモリ最後のブロック
 * @param p_dwGetBlock dwFindCycサイクルが存在するインデックス番号。
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getTrcIndex(DWORD dwFindCyc, DWORD dwSrchStartBlock, DWORD dwSrchEndBlock, DWORD* p_dwGetBlock)
{
	DWORD dwBaseEndCyc = 0;
	DWORD i;
	DWORD dwBlockEnd = 0;
	DWORD dwGetBlock = 0;

	dwBlockEnd = getBlockEnd();	// RevRxNo130408-001 Modify Line

	// インデックス設定の基準値をセット(開始ブロックの最終サイクル)
	dwBaseEndCyc = s_dwRdIndex[s_dwRdIndexSBlkPos].dwEndCycle;

	// 開始～最後のブロック検索
	for (i = dwSrchStartBlock; i <= dwSrchEndBlock; i++) {
		if (s_dwRdIndex[i].dwStartCycle == -1 && s_dwRdIndex[i].dwEndCycle == -1) {
			// インデックス設定
			// 既に取得済みのため何もしない
		} else {
			//dwBaseEndCyc 更新
			dwBaseEndCyc = s_dwRdIndex[i].dwEndCycle;
		}
		if (dwFindCyc >= s_dwRdIndex[i].dwStartCycle && dwFindCyc <= s_dwRdIndex[i].dwEndCycle) {
			dwGetBlock = i;
			break;
		}
	}
	if (i == dwSrchEndBlock+1) {
		// 最終ブロックまで該当サイクルがなかった場合、先頭ブロックから検索 終わりブロックまで検索
		for (i = 0; i <= s_dwRdIndexEBlkPos; i++) {
			if (s_dwRdIndex[i].dwStartCycle == -1 && s_dwRdIndex[i].dwEndCycle == -1) {
				// インデックス設定
				// 既に取得済みのため何もしない
			} else {
				//dwBaseEndCyc 更新
				dwBaseEndCyc = s_dwRdIndex[i].dwEndCycle;
			}
			if (dwFindCyc >= s_dwRdIndex[i].dwStartCycle && dwFindCyc <= s_dwRdIndex[i].dwEndCycle) {
				dwGetBlock = i;
				break;
			}
		}
		//トレースメモリに見つからないときはトレースバッファ内を探す
		if (i == s_dwRdIndexEBlkPos + 1) {
			if ( (dwFindCyc >= s_dwRdIndex[dwBlockEnd+1].dwStartCycle) && (dwFindCyc <= s_dwRdIndex[dwBlockEnd+1].dwEndCycle) ) {
				dwGetBlock = s_dwRdIndexEBlkPos;
				//最後のブロックのエンドサイクルをバッファの最後にする
				s_dwRdIndex[s_dwRdIndexEBlkPos].dwEndCycle = s_dwRdIndex[dwBlockEnd+1].dwEndCycle; 
			} else {
				//トレースバッファ内にないときはエラーで返す。
				return FFWERR_FFW_ARG;
			}
		}
	}
	
	*p_dwGetBlock = dwGetBlock;

	return FFWERR_OK;
}

//==============================================================================
/**
 * トレースメモリの内容を取得する。 OCD Class3用
 * @param dwStartCyc 開始サイクルの指定
 * @param dwEndCyc   終了サイクルの指定
 * @param pRd         取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getRXCls3RD(DWORD dwStartCyc, DWORD dwEndCyc, FFWRX_RD_DATA* pRd)
{
	FFWERR				ferr;
	DWORD				dwStartBlock;
	DWORD				dwEndBlock;
	int					nStartOffset, nEndOffset;
	DWORD				dwBlockEnd;
	DWORD				dwBaseEndCyc;
	DWORD				dwSrchEndBlock = 0;
	DWORD				dwStrBlockEnd;

	dwBlockEnd = getBlockEnd();	// RevRxNo130408-001 Modify Line

	// トレースモードとピン数の関連チェック
	if( GetTrcInramMode() ) {
		// 内蔵トレース
		ferr = getTrcDataTrbf(pRd,dwStartCyc,dwEndCyc);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	} else {
		// 外部トレース
		// インデックス設定の基準値をセット(開始ブロックの最終サイクル)
		dwBaseEndCyc = s_dwRdIndex[s_dwRdIndexSBlkPos].dwEndCycle;

		// 開始ブロック検索終了ブロックは、ラップラウンドしていない場合は、終了ブロックまで探す
		if(s_dwRdIndexSBlkPos <= s_dwRdIndexEBlkPos) {
			dwStrBlockEnd = s_dwRdIndexEBlkPos;
		} else {
			dwStrBlockEnd = dwBlockEnd;
		}
		//開始ブロック検索
		ferr = getTrcIndex(dwStartCyc, s_dwRdIndexSBlkPos ,dwStrBlockEnd,&dwStartBlock);
		if (ferr != FFWERR_OK) {
				return ferr;
		}
		// 検索終了位置を設定　通常最後のメモリブロック(dwBlockEnd)でよいが、メモリラップアラウンドして、開始サイクルブロックがs_dwRdIndexSBlkPosより前にきた場合は終了ブロックを渡す
		if ( dwStartBlock >= s_dwRdIndexSBlkPos) {
			dwSrchEndBlock = dwBlockEnd;
		} else {
			dwSrchEndBlock = s_dwRdIndexEBlkPos;
		}
		//終了ブロック検索
		ferr = getTrcIndex(dwEndCyc, dwStartBlock, dwSrchEndBlock, &dwEndBlock);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// ブロックの最後のサイクルと解析最後のサイクルが同じ場合、データが足りない場合があるので、次のブロックまでデータを取っておく。ただし最後のサイクルのときと最後のメモリブロックのときは必要なし
		if ( (dwEndCyc == s_dwRdIndex[dwEndBlock].dwEndCycle ) && (s_dwRdIndexEBlkPos != dwEndBlock) && (s_rcyData.dwEndCyc != dwEndCyc)){
			if( ( dwEndBlock == dwBlockEnd ) ){
				dwEndBlock = 0;
			} else {
				dwEndBlock++;
			}
		}

		// オフセット値をセット
		if (s_dwRdIndex[dwStartBlock].dwStartCycle == 0) {
			nStartOffset = dwStartCyc - s_dwRdIndex[dwStartBlock].dwStartCycle + s_nNonCycleCnt;
		} else {
			nStartOffset = dwStartCyc - s_dwRdIndex[dwStartBlock].dwStartCycle;
		}
		nEndOffset = dwEndCyc - dwStartCyc;

		// データ取得＆解析
		ferr = getTrcData(pRd, dwStartBlock, dwEndBlock, nStartOffset, nEndOffset);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	return ferr;

}

//==============================================================================
/**
 * トレースメモリの内容を取得する。 OCD Class2用
 * @param dwStartCyc 開始サイクルの指定
 * @param dwEndCyc   終了サイクルの指定
 * @param pdwSetInfo トレース出力情報の設定状態を取得
 * @param pRd         取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getRXCls2RD(DWORD dwStartCyc, DWORD dwEndCyc, FFWRX_RD_DATA* pRd)
{
	FFWERR				ferr = FFWERR_OK;
	DWORD				dwRdCount;
	DWORD				dwReadCyc;
	//RX220WS対応 Appned Line
	DWORD				dwBuffLength;

	// 内蔵トレースデータを保持しているか確認
	if(s_bGetTrcDataInram == FALSE){
		// まだ内蔵トレースデータを取得していない
		// FFWメモリ確保
		//RX220WS対応 Modify Start
		dwBuffLength = GetTrcCycMaxCls2()*8;
		dwRdCount = dwBuffLength/4;
		s_pdwTrcDataInram = new DWORD [dwRdCount];
		memset(s_pdwTrcDataInram,0,dwBuffLength);
		//RX220WS対応 Modify End
		// トレースメモリ取得
		ferr = GetAllTrcBuffDataTrbf(s_pdwTrcDataInram,dwRdCount);
		if(ferr != FFWERR_OK){
			delete [] s_pdwTrcDataInram;
			return ferr;
		}
		s_bGetTrcDataInram = TRUE;
	}

	// 解析サイクルを求める
	dwReadCyc = dwEndCyc - dwStartCyc +1;

	// 解析処理
	analysisTrcDataTrbfCls2(pRd,s_pdwTrcDataInram,dwStartCyc,dwReadCyc);

	return ferr;

}
// V.1.02 No.31 Class2 トレース対応 Modify End


// RevRxE2LNo141104-001 Append Start
//==============================================================================
/**
 * トレースサイクル/データ情報初期化
 * @param なし
 * @retval なし
 */
//==============================================================================
static void clrTrcDataInfo(void)
{
	// トレースサイクル数を初期化
	s_rcyData.dwStartCyc = RCY_CYCLE_NON;
	s_rcyData.dwEndCyc = RCY_CYCLE_NON;
	s_bTracSetRCY = FALSE;
	s_bOff = FALSE;

	// FFWトレースメモリクリア
	if(s_pdwTrcDataInram != NULL){
		if(s_bGetTrcDataInram == TRUE){
			delete [] s_pdwTrcDataInram;
		}
	}
	s_bGetTrcDataInram = FALSE;

	// 32bitトレースアドレス対策 フラグクリア
	s_bDummyTrcPkt = FALSE;
	s_bSetDummyPkt = FALSE;
	s_bSetDummyPktBtr1 = FALSE;
	s_dwTrcCmdCnt = RCY_CYCLE_NON;

	return;
}
// RevRxE2LNo141104-001 Append End

//==============================================================================
/**
 * トレースデータをクリアする。OCD Class3処理
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR clrRXCls3RD(void)
{

	FFWERR						ferr;
	FFWE20_EINF_DATA			Einf;
	// RevNo110309-005 Append Line
	WORD						wUsedMemSize = 0;
	// RevNo110510-001 Append Line
	DWORD						dwTrstp;

	// RevRxE2LNo141104-001 Delete: FFW内部変数初期化処理削除(呼び出し元へ移動)。

	//RevNo100715-014 Modify Line
	//エミュレータ情報取得
	getEinfData(&Einf);

	// RevNo110510-001 Modify Start
	// TRARがトレーススタート(0)の場合は、一時停止をさせる
	ferr = GetTrcOcdMemInputInfo(&dwTrstp);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	
	if(dwTrstp == TRSTP_RESTART){
		// E20の入力止めた後、バッファとメモリのクリア
		ferr = setTrStp(TRSTP_STOP_KEEP);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevNo110510-001 Modify End

	// RevRxNo121022-001 Modify Start
	// E20の場合、E20上のトレースメモリクリア処理を実施
	// E1/EZ-CUBEの場合は、E20上のトレースメモリクリア処理は実施しない
	if(Einf.wEmuStatus == EML_E20) {
	// RevRxNo121022-001 Modify End
		// E20トレースバッファクリア
		ferr = ClrTrcFpgaBuf();
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// E20トレースメモリクリア
		// RevNo110309-005 Modify Start
		if(s_bUsedTrace == TRUE){
			wUsedMemSize = s_wUsedMemSize;
		} else {
			wUsedMemSize = EML_TRCMEM_64M;
			s_bUsedTrace = TRUE;
		}
		ferr = ClrTrcFpgaMem(wUsedMemSize);
		// RevNo110309-005 Modify End
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	//OCDクリア
	// オーバーフローフラグクリアはTBSRリード時点でクリアされるため不要
	// モニタCPUのオーバーフローフラグクリア

	// トレースOCDのクリア処理
	ferr = ClrTrcOcdMemCls3();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevNo110510-001 Modify Start
	// TRARがトレーススタート(0)の場合は、一時停止をさせたので、元に戻しておく
	if(dwTrstp == TRSTP_RESTART){
	// RevNo110506-002 Append Start
	// 停止したので、再度有効にしておく
		ferr = setTrRst();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	// RevNo110506-002 Append End
	}
	// RevNo110510-001 Modify End

	return ferr;

}

//==============================================================================
/**
 * トレースデータをクリアする。OCD Class2処理
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR clrRXCls2RD(void)
{

	FFWERR						ferr;

	// RevRxE2LNo141104-001 Delete: FFW内部変数初期化処理削除(呼び出し元へ移動)。

	//OCDクリア
	ferr = ClrTrcOcdMemCls2();
	if(ferr != FFWERR_OK){
		return ferr;
	}

	return ferr;

}
// V.1.02 No.31 Class2 トレース対応 Modify End

// V.1.02 No.31 Class2 トレース対応 Modify Start
//==============================================================================
/**
 * トレースメモリに格納されたデータの開始サイクルと終了サイクルの情報を取得する。
 * RX Class3用
 * @param pRcy トレースメモリに格納されたトレースデータの開始サイクル、
 *             終了サイクルを格納するFFW_RCY_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getRXCls3RCY(FFWRX_RCY_DATA *pRcy)
{
	FFWERR	ferr = FFWERR_OK;

	// サイクル数未取得 StartとEnd -1 フラグFALSE
	if ( s_bTracSetRCY == FALSE ) {
		if(GetTrcInramMode()) {
			// 内蔵トレースサイクル取得
			ferr = getTraceCycleTrbf(pRcy);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		} else {
			// 外部トレースサイクル取得
			ferr = getTraceCycle(pRcy);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

		s_rcyData.dwStartCyc = pRcy->dwStartCyc;
		s_rcyData.dwEndCyc = pRcy->dwEndCyc;
	} else {
		pRcy->dwStartCyc = s_rcyData.dwStartCyc;
		pRcy->dwEndCyc = s_rcyData.dwEndCyc;
	}

	return ferr;

}

//==============================================================================
/**
 * トレースメモリに格納されたデータの開始サイクルと終了サイクルの情報を取得する。
 * RX Class2用
 * @param pRcy トレースメモリに格納されたトレースデータの開始サイクル、
 *             終了サイクルを格納するFFW_RCY_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getRXCls2RCY(FFWRX_RCY_DATA *pRcy)
{
	FFWERR	ferr = FFWERR_OK;

	// 内蔵トレースサイクル取得
	if(s_bTracSetRCY == FALSE){
		ferr = getTraceCycleTrbfCls2(pRcy);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		s_rcyData.dwStartCyc = pRcy->dwStartCyc;
		s_rcyData.dwEndCyc = pRcy->dwEndCyc;
	} else {
		pRcy->dwStartCyc = s_rcyData.dwStartCyc;
		pRcy->dwEndCyc = s_rcyData.dwEndCyc;
	}

	return ferr;

}
// V.1.02 No.31 Class2 トレース対応 Modify End


// RevRxNo130408-001 Append Start
//==============================================================================
/**
 * 外部トレースサイクル数最大値取得
 * @param なし
 * @retval トレースサイクル数最大値
 */
//==============================================================================
static DWORD getTraceCycleNumMax(void)
{
	FFWRX_RM_DATA*	pRm;
	DWORD	dwCycNumMax;

	pRm = GetRmDataInfo();

	if ((pRm->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D4S1) {	// DATA 4ビット、SYNC 1ビットの場合
		dwCycNumMax = RCY_CYCLE_MAX_D4S1;

	} else {	// DATA 8ビット、SYNC 2ビットの場合
		dwCycNumMax = RCY_CYCLE_MAX_D8S2;
	}

	return dwCycNumMax;

}
// RevRxNo130408-001 Append End

//==============================================================================
/**
 * 取得したトレースバッファデータから識別子1をカウントする。
 * @param  pwRd      カウントするトレースデータ
 * @param  dwRdSize  カウントするトレースデータのサイズ(ワード)
 * @retval nRetCnt   識別子1の数(-1の場合、識別子1は存在していない)
 */
//==============================================================================
static int cntTraceBuffCycle(WORD* pwRd, DWORD dwRdSize)
{
	DWORD	i;
	WORD	wCmdInf=0;
	int		nRetCnt = -1;

	for (i = 0; i < dwRdSize; i++) {
		// トレース情報種別を判定
		wCmdInf = (WORD)(pwRd[i] & TRCDATA_CMD_INF);

		// 識別子1の場合
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {	// RevRxNo130408-001 Modify Line
			nRetCnt++;
		}
	}

	return nRetCnt;

}
//==============================================================================
/**
 * トレースメモリに格納された無効な識別子1(サイクル数)取得
 * @param  dwBlock      ブロックNo
 * @param  pdwRetCnt    取得したサイクル数
 * @param  pdwRetNonCnt 無効なサイクル数
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getTraceNonCycle(DWORD dwBlock, DWORD* pdwRetCnt, DWORD* pdwRetNonCnt)
{
	FFWERR	ferr;
	WORD*	wRd;
	DWORD*	pdwNonCnt;
	DWORD	i;
	WORD	wCmdInf;
	int		nRetCnt = 0;
	int		nRdCnt = 0;
	DWORD	dwBuff = 0;
	DWORD	dwNonCntDmyBef = 0;
	BOOL	bSetDmyDtw1 = FALSE;
	BOOL	bSetDmyBtr2 = FALSE;

	WORD	wTmwa = 0;
	WORD	wTmwaEnd = 0;
	WORD	wTmwba = 0;
	WORD	wRdEnd = 0;
	DWORD	dwBlockEnd;
	DWORD	dwRdBlockSize;	// RevRxNo130408-001 Append Line
	DWORD	dwRdTrcBuffSize;	// RevRxNo130408-001 Append Line
	DWORD	dwRdNextBlockSize;	// RevRxNo130408-001 Append Line

	dwBlockEnd = getBlockEnd();	// RevRxNo130408-001 Modify Line

	pdwNonCnt = pdwRetNonCnt;

	// RevRxNo130408-001 Modify Start
	// トレースメモリブロック 2つ分のバッファ確保
	dwRdBlockSize = GetRdBufSizeTraceBlock();
	wRd = new WORD[(dwRdBlockSize+1)*2];
	memset(wRd, 0, ((dwRdBlockSize+1)*2)*2);
	// RevRxNo130408-001 Modify End

	// トレースメモリのデータ取得
	// トレースメモリ最終位置を取得
	ferr = GetTrcE20MemTmwa(&wTmwa);
	if (ferr != FFWERR_OK) {
		// RevRxNo130730-008 Append Line
		delete [] wRd;
		return ferr;
	}

	// ブロック終了位置取得
	if(wTmwa==0){
		wTmwaEnd = (WORD)(dwBlockEnd & 0x0000FFFF);
	}else{
		wTmwaEnd = (WORD)(wTmwa-1);
	}

	// RevRxNo130408-001 Modify Start
	// ブロックdwBlockと、次のブロック/E20トレースバッファデータを取得する
	if (dwBlock == (DWORD)wTmwaEnd) {	// 取得ブロックがトレースメモリデータ格納最終ブロックの場合
		// ブロックdwBlockのデータを wRd[0]～wRd[dwRdBlockSize-1]に取得
		ferr = GetTrcFpgaMemBlock(dwBlock, dwBlock, &wRd[0]);
		if (ferr != FFWERR_OK) {
			delete [] wRd;
			return ferr;
		}

		// E20トレースバッファのポインタ位置を取得
		ferr = GetTrcFpgaTmwba(&wTmwba);
		if (ferr != FFWERR_OK) {
			// RevRxNo130730-008 Append Line
			delete [] wRd;
			return ferr;
		}

		// E20トレースバッファのトレースメモリデータ格納バッファサイズ取得
		dwRdTrcBuffSize = GetRdBufSizeTraceBuffer(wTmwba);

		//E20トレースバッファデータを wRd[dwRdBlockSize]～wRd[dwRdBlockSize+dwRdTrcBuffSize-1]に取得
		ferr = GetTrcFpgaBuffData(wTmwba, wRd, dwRdBlockSize);
		if (ferr != FFWERR_OK) {
			// RevRxNo130730-008 Append Line
			delete [] wRd;
			return ferr;
		}

		// ブロックdwBlockの、次ブロックのデータ数設定
		dwRdNextBlockSize = dwRdTrcBuffSize;

	} else {
		// 次にブロックデータをくっつける
		if (dwBlock == dwBlockEnd) {	// 取得ブロックがトレースメモリ最終ブロックの場合
			// ブロックdwBlockのデータを wRd[0]～wRd[dwRdBlockSize-1]に取得
			ferr = GetTrcFpgaMemBlock(dwBlock, dwBlock, &wRd[0]);
			if (ferr != FFWERR_OK) {
				delete [] wRd;
				return ferr;
			}

			// ブロック0のデータを wRd[dwRdBlockSize]～wRd[dwRdBlockSize*2-1]に取得
			ferr = GetTrcFpgaMemBlock(0, 0, &wRd[dwRdBlockSize]);
			if (ferr != FFWERR_OK) {
				delete [] wRd;
				return ferr;
			}

		} else {	// 取得ブロックがトレースメモリ最終ブロックでない場合
			// ブロックdwBlock, dwBlock+1のデータを wRd[0]～wRd[dwRdBlockSize*2-1]に取得
			ferr = GetTrcFpgaMemBlock(dwBlock, dwBlock+1, &wRd[0]);
			if (ferr != FFWERR_OK) {
				delete [] wRd;
				return ferr;
			}
		}

		// ブロックdwBlockの、次ブロックのデータ数設定
		dwRdNextBlockSize = dwRdBlockSize;
	}

	//32bitトレースアドレス対策 
	bSetDmyDtw1 = FALSE;
	bSetDmyBtr2 = FALSE;

	// トレースメモリブロックdwBlock内の"識別子1"個数をカウント
	for (i = 0; i < dwRdBlockSize; i++) {
		// トレース情報種別を判定
		wCmdInf = (WORD)(wRd[i] & TRCDATA_CMD_INF);

		// 識別子1の場合
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {
			nRetCnt++;
			//32bitトレースアドレス対策 
			// ダミーデータがある場合、最初のBTR2とDTW1はダミーデータのため、その二つが出てきた後ろが本当のデータ
			if( ((wRd[i] & TRCDATA_TRDATA03) == BTR2) && (bSetDmyBtr2 == FALSE)){
				bSetDmyBtr2 = TRUE;
			} else if( ((wRd[i] & TRCDATA_TRDATA03) == DTW1) && (bSetDmyDtw1 == FALSE) && (bSetDmyBtr2 == TRUE)){
				bSetDmyDtw1 = TRUE;
				nRdCnt = i+1;
				dwNonCntDmyBef = nRetCnt - 2;
			}
		}
	}
	(*pdwRetCnt) = nRetCnt;


	// 次ブロックにある最初の"識別子1"位置を検索
	for (i = dwRdBlockSize; i < (dwRdBlockSize + dwRdNextBlockSize); i++) {
		// トレース情報種別を判定
		wCmdInf = (WORD)(wRd[i] & TRCDATA_CMD_INF);
		// 識別子1の場合
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {
			break;
		}
	}
	wRdEnd = (WORD)i;


	// 無効サイクル数取得
	if ( (s_bDummyTrcPkt == TRUE) && (dwBlock == 0) ) {
		//32bitトレースアドレス対策 ダミートレースが残っている　かつ ブロック0の場合
		//無効なデータは ダミーデータ+アドレスがないパケット
		analysisNonAddrTrcData(&wRd[nRdCnt], (wRdEnd - nRdCnt), pdwNonCnt);
		dwBuff = *pdwNonCnt;
		*pdwNonCnt = dwBuff + dwNonCntDmyBef + 2;
	} else if (s_bDummyTrcPkt == TRUE) {
		//32bitトレースアドレス対策 ブロック0以降を検索　無効なデータはアドレスがないパケット
		analysisNonAddrTrcData(&wRd[0], wRdEnd, pdwNonCnt);
	}
	else {
		// 無効なトレースデータ解析
		analysisNonTrcData(&wRd[0], wRdEnd, pdwNonCnt);
	}
	// RevRxNo130408-001 Modify End

	delete [] wRd;

	return ferr;
}

//==============================================================================
/**
 * 無効なトレースデータ解析
 * @param  wRd        取得したトレースデータ
 * @param  dwRdSize   取得したトレースサイズ
 * @param  pdwRetCnt  無効なサイクル数
 * @retval なし
 */
//==============================================================================
static void analysisNonTrcData(WORD* wRd, DWORD dwRdSize, DWORD* pdwRetCnt)
{
	FFWRX_RD_DATA rd;
	DWORD	i;
	WORD	wTrcCmdKind = 0;
	WORD	wCmdInf;
	DWORD	dwTmShiftCnt = 0;
	DWORD	dwAddrBuff = 0;
	DWORD	dwAddrShiftCnt = 0;
	int		nCycCnt = 0;
	int		nTrcCmd2 = -1;
	int		nTrcCmd3 = -1;
	int		nShiftCnt = 0;
	DWORD	dwSrcAddrCnt = 0;
	DWORD	dwDestAddrCnt = 0;
	int		nSrcShiftCnt = 0;
	int		nDestShiftCnt = 0;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo161003-001 Append Line

	pFwCtrl = GetFwCtrlData();		// RevRxNo161003-001 Append Line

	memset(&rd, 0, sizeof(FFWRX_RD_DATA));
	//Lostフラグ初期化
	s_bLostBrAddr = FALSE;
	s_bLostDtAddr = FALSE;

	// 取得データの解析
	for (i = 0; i < dwRdSize; i++) {
		// トレース情報種別を判定
		wCmdInf = (WORD)(wRd[i] & TRCDATA_CMD_INF);
		// RevRxNo130408-001 Modify Start
//		以下は処理がないため、不要な条件判定をコメント化している。
//		if (wCmdInf == s_wTrDataCmdInfStnby) {		// 無効データ
//			// 処理なし
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {		// 識別子１
			// フルアドレス取得時
			// トレース結果が分岐のみ、もしくはオペランドのみの場合があるのでどちらかがフルアドレス出たところから解析スタート
			if (s_bGetBrAddr == TRUE || s_bGetDtAddr == TRUE) {
				(*pdwRetCnt) = nCycCnt - 1;		// 1サイクルは有効サイクルの為、-1する
				return;
			}
			// 識別子2,3のデータ、データシフトカウントを初期化
			nTrcCmd2 = -1;
			nTrcCmd3 = -1;
			nShiftCnt = 0;
			dwTmShiftCnt = 0;
			dwAddrShiftCnt = 0;
			dwAddrBuff = 0;
			dwSrcAddrCnt = 0;
			dwDestAddrCnt = 0;
			nSrcShiftCnt = 0;
			nDestShiftCnt = 0;

			// 識別子1の内容セット
			wTrcCmdKind = (WORD)(wRd[i] & TRCDATA_TRDATA03);
			// サイクル数をインクリメント
			nCycCnt++;

			// STDBYの場合
			if (wTrcCmdKind == STDBY) {
			}
			// LOSTの場合
			else if (wTrcCmdKind == LOST) {
				// LOST発生時、LOST前のパケットとの差分を取るためLOST状態にしなくてよい
				s_bLostBrAddr = FALSE;
				s_bLostDtAddr = FALSE;
			}
			// RevRxNo161003-001 Modify Start +25
			// DTR1/DTR2/DTR4/DTW1/DTW2/DTW4の場合
			else if ((wTrcCmdKind == DTR1) || (wTrcCmdKind == DTR2) || (wTrcCmdKind == DTR4) ||
					 (wTrcCmdKind == DTW1) || (wTrcCmdKind == DTW2) || (wTrcCmdKind == DTW4)) {
				if (pFwCtrl->bRomDataTrcAccessTypeExist && ((s_dwTraceInfo & RM_SETINFO_RODTE) == RM_SETINFO_RODTE)) {
										// ROMデータトレース アクセスタイプ出力ビットあり && ROMデータトレース アクセスタイプ出力あり
					// 次のデータも識別子1の場合
					if ( ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfIdnt1) || ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfStnby) ){
						// ROMアクセスでデータなし(アドレス＆タイムスタンプ未出力)かどうかチェック
						if (((s_dwTraceInfo & RM_SETINFO_RODE) != RM_SETINFO_RODE) &&
							((s_dwTraceInfo & RM_SETINFO_TDA) != RM_SETINFO_TDA) && ((s_dwTraceInfo & RM_SETINFO_TSE) != RM_SETINFO_TSE)) {
							// アクセス属性、アクセスサイズ設定
							setDataAccessInfo(wTrcCmdKind, &rd.opr.dwAccess, &rd.opr.dwAccessSize);
							rd.opr.bComplete = TRUE;
							s_bGetDtAddr = TRUE;
						}
					}
				} else {
					// DTR1の場合
					if (wTrcCmdKind == DTR1) {
						// 次のデータも識別子1の場合
						if ( ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfIdnt1) || ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfStnby) ){
							// ROMアクセスでデータなし(アドレス＆タイムスタンプ未出力)かどうかチェック
							if ((((s_dwTraceInfo >> 10) & 0x00000001) == FALSE) &&
								(((s_dwTraceInfo >> 21) & 0x00000001) == FALSE) && (((s_dwTraceInfo >> 22) & 0x00000001) == FALSE)) {
								// アクセス属性
								rd.opr.dwAccess = TRC_OPR_ACS_R;
								// アクセスサイズ
								rd.opr.dwAccessSize = TRC_OPR_ACSSIZE_B;
								rd.opr.bComplete = TRUE;
								s_bGetDtAddr = TRUE;
							}
						}
					}
				}
			}
			// RevRxNo161003-001 Modify End
			// 上記以外の識別子1の場合
			else {
			}

			// トレース情報種別をセット
			setTraceType(&rd, wTrcCmdKind);

		} else if (wCmdInf == s_wTrDataCmdInfIdnt2) {		// 識別子2
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != 0) {
				// MODEの場合
				if (wTrcCmdKind == MODE) {
					// スタックポインタ情報出力可否チェック
					if ((s_dwTraceInfo & RM_SETINFO_TMO) == RM_SETINFO_TMO) {
						rd.stac.dwStac = wRd[i] & TRCDATA_TRDATA03;
					}
				}
				// DTR1,DTR2,DTR4,DTW1,DTW2,DTW4の場合
				else if (wTrcCmdKind == DTR1 || wTrcCmdKind == DTR2 || wTrcCmdKind == DTR4 || 
							wTrcCmdKind == DTW1 || wTrcCmdKind == DTW2 || wTrcCmdKind == DTW4) {
					// イベント番号情報出力可否チェック
					if ((s_dwTraceInfo & RM_SETINFO_TRI) == RM_SETINFO_TRI) {
						// イベント番号(識別子2)を取得
						rd.opr.dwEvNum = wRd[i] & TRCDATA_TRDATA03;
					}
				}
				// 識別子2の内容セット
				nTrcCmd2 = wRd[i] & TRCDATA_TRDATA03;
			}

		} else if (wCmdInf == s_wTrDataCmdInfIdnt3_1) {		// 識別子3-1
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != 0) {
				// 識別子3の内容セット
				nTrcCmd3 = wRd[i] & TRCDATA_TRDATA03;
			}

		} else if (wCmdInf == s_wTrDataCmdInfIdnt3_2) {		// 識別子3-2
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != 0) {
				// BTR1でS/Wデバッグモードの場合
				if (wTrcCmdKind == BTR1 && ((s_dwTraceInfo & RM_SETINFO_BSS) != RM_SETINFO_BSS)) {
					// タイムスタンプ情報出力可否チェック
					if ((((s_dwTraceInfo >> 22) & 0x00000001) == TRUE)) {
						// 分岐成立可否情報を取得(※+2はスタートビット付加)
						rd.bcnd.dwBcnd = ((wRd[i] & 0x0008) >> 3) + 2;
					}
				}
				// 識別子3の内容セット
				nTrcCmd3 = wRd[i] & TRCDATA_TRDATA03;
			}

		} else if (wCmdInf == s_wTrDataCmdInfData) {		// データ
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != 0) {
				if (wTrcCmdKind == BTR1) {
					if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS) {		// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&rd, wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_NORMAL);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR1(&rd, wRd, nTrcCmd3, &nShiftCnt, i);
					}
				} else if (wTrcCmdKind == BTR2) {
					if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS) {		// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&rd, wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_SUB);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR2(&rd, wRd, nTrcCmd3, &dwTmShiftCnt, &nShiftCnt, i, &dwAddrBuff);
					}
				} else if (wTrcCmdKind == BTR3) {
					if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS) {		// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&rd, wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_EXCEPT);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR3(&rd, wRd, &nShiftCnt, i);
					}
				} else if (wTrcCmdKind == DTR1 || wTrcCmdKind == DTR2 || wTrcCmdKind == DTR4 ||
							wTrcCmdKind == DTW1 || wTrcCmdKind == DTW2 || wTrcCmdKind == DTW4) {
					setTraceDataDTRW(&rd, wRd, wTrcCmdKind, nTrcCmd3, &dwTmShiftCnt, &dwAddrShiftCnt, &nShiftCnt, i);
				}
			}
		}
		// RevRxNo130408-001 Modify End
	}

	if (s_bGetBrAddr == TRUE || s_bGetDtAddr == TRUE) {
		(*pdwRetCnt) = nCycCnt - 1;		// 1サイクルは有効サイクルの為、-1する
	} else {
		(*pdwRetCnt) = nCycCnt;
	}

	return;

}

//==============================================================================
/**
 * トレースメモリ・バッファに格納された識別子1(サイクル数)取得
 * @param  pRcy       トレースメモリに格納されたトレースデータの開始サイクル、
 *                    終了サイクルを格納するFFW_RCY_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getTraceCycle(FFWRX_RCY_DATA *pRcy)
{
	FFWERR	ferr;
	WORD*	wRd;
	DWORD	dwRdBuffSize = 0;
	DWORD	dwBlockStart = 0x00000000;
	DWORD	dwBlockEnd;
	DWORD	i;
	DWORD	dwCnt = 0;
	WORD	wTmwba = 0;
	WORD	wTmwa = 0;
	BOOL	bTmFull;
	BOOL	bTmFull2;
	WORD	wFuncs = 0;
	BOOL	bTren;
	int		nTrcBuffCmdCnt = -1;
	DWORD	dwBlkTrcCmdCnt = 0;
	DWORD	dwNonTrcCmdCnt = 0;
	DWORD	dwTrcCmdCnt = 0;
	BOOL	bNonTrcCycSearch = FALSE;

	WORD	wTmwaEnd = 0;
	DWORD	dwNonTrcCmdAllCnt = 0;
	DWORD	dwEndCycBuf = 0;

	BOOL	bOverFlow = FALSE;
	// RevNo110322-01 Append Line
	BOOL	bSetDmmyBTR1 = TRUE;
	DWORD	dwCycNumMax;	// RevRxNo130408-001 Append Line

	dwBlockEnd = getBlockEnd();	// RevRxNo130408-001 Modify Line
	dwCycNumMax = getTraceCycleNumMax();	// RevRxNo130408-001 Append Line

	// static変数の初期化
	s_nPreTimeStamp = -1;
	s_nPreBrAddr = -1;
	s_nPreDtAddr = -1;
	s_dwTraceInfo = 0;
	s_nNonCycleCnt = 0;
	s_bGetBrAddr = FALSE;
	s_bGetDtAddr = FALSE;

	s_bDummyTrcPkt = FALSE;
	s_bSetDummyPktBtr1 = FALSE;
	s_dwRdIndexSBlkPos = 0xFFFFFFFF;
	s_dwRdIndexEBlkPos = 0xFFFFFFFF;
	s_dwTrcCmdCnt = RCY_CYCLE_NON;

	// V.1.02 No.36 タイムスタンプ解析修正 Append Start
	s_bE20TrcMemOvr = FALSE;
	s_wTmwaEnd = 0;
	// V.1.02 No.36 タイムスタンプ解析修正 Append End

	// インデックス初期化
	for (i = 0; i < RD_TRCMEM_INDEX; i++) {
		s_dwRdIndex[i].dwStartCycle = RCY_CYCLE_NON;
		s_dwRdIndex[i].dwEndCycle = RCY_CYCLE_NON;
	}

	// トレース出力情報の設定状態を取得
	getTraceInfo();

	//トレースFPGAの状態取得
	ferr = GetTrcFpgaStatInfo(&bTren,&wFuncs,&bTmFull,&bTmFull2);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAがまだ入力有効の場合は閉じてBTR1強制出力処理を実施
	// RevNo110322-01 Modify Start
	if(bTren == TRUE){
		bSetDmmyBTR1 = TRUE;
	} else {
		bSetDmmyBTR1 = FALSE;
	}

	// デバイスからの出力とE20の入力を止める
	ferr = DO_SetRXTRSTP(TRSTP_STOP_KEEP,bSetDmmyBTR1);
	// RevNo110322-01 Modify End
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo121227-001 Append Start
	// フリートレースモード時、TMFUL,TM2FULフラグをクリアする。
	// TMFUL,TM2FULフラグ以外の成立しているフラグもクリアするが、
	// RCY結果を一度取得した後は、FFW内の情報を返送するため問題ない。
	if ((wFuncs == 0) || (bTmFull == FALSE)) {
		// E20トレースFPGAのトレース状態クリア
		ferr = ClrTrcFpgaStatus();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo121227-001 Append End

	// トレースFPGAのトレースメモリ書込み用アドレスレジスタ
	//このアドレスは次に書き込む位置を示す
	ferr = GetTrcE20MemTmwa(&wTmwa);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// トレースFPGAのトレースメモリ書込み用バッファアドレスレジスタ
	// (b9:TMWBS (0:バッファ #0、1:バッファ #1))
	ferr = GetTrcFpgaTmwba(&wTmwba);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// オーバーフローフラグ設定
	// オーバーフローはFULLフラグが立ったときではない。
	// TMWA=0のとき TMFUL=1かつTM2FUL=1のときにオーバーフロー
	// TMWA=0x1FFのとき TMFUL=1かつTM2FUL=0かつのときにオーバーフロー
	if(bTmFull2 == TRUE){
		bOverFlow = TRUE;
	} else if((bTmFull == TRUE) && ( wTmwa == dwBlockEnd) && (wTmwba<TRC_FPGA_BUFF_FULL) ){
		bOverFlow = TRUE;
	} else if((bTmFull == TRUE) && (( wTmwa > 0 ) && (wTmwa < dwBlockEnd))){
		bOverFlow = TRUE;
	}

	// 32bitアドレス対策 フリートレースでトレースフル状態以外はダミーデータが残っている
	if (s_bSetDummyPkt == TRUE) {
		if ((wFuncs == 0) && (bOverFlow == TRUE)) {
			s_bDummyTrcPkt = FALSE;
		} else {
			s_bDummyTrcPkt = TRUE;
		}
	}

	//読み込みスタート位置はwTmwa-1になる
	//オーバーフローが起こった時のアドレスが0の時は最後のブロックアドレスを示す
	if ( (bOverFlow == TRUE) && (wTmwa == 0)) {
		wTmwaEnd = (WORD)dwBlockEnd;
	} else if ((bTmFull == TRUE) &&(bOverFlow == FALSE) && (wTmwa == 0)) {
		//オーバーフローはしていないが、メモリラップラウンド発生
		wTmwaEnd = (WORD)dwBlockEnd;
	} else if ((bTmFull == FALSE) &&(bOverFlow == FALSE) && (wTmwa == 0)) {
		//メモリブロック0までデータが入っていない
		wTmwaEnd = wTmwa;
	} else if ( wTmwa == 0 ) {
		wTmwaEnd = (WORD)dwBlockEnd;
	} else {
		wTmwaEnd = (WORD)(wTmwa - 1);
	}
	// V.1.02 No.36 タイムスタンプ解析修正 Append Start
	s_bE20TrcMemOvr = bOverFlow;
	s_wTmwaEnd = wTmwaEnd; 
	// V.1.02 No.36 タイムスタンプ解析修正 Append End

	// 対象のトレースバッファデータ格納用メモリ確保

	dwRdBuffSize = GetRdBufSizeTraceBuffer(wTmwba) + 1;	// RevRxNo130408-001 Modify Line
	wRd = new WORD[dwRdBuffSize];
	memset(wRd, 0, (dwRdBuffSize*2));

	//トレースバッファにしかデータがない場合はバッファデータをメモリに書き込む
	if ((wTmwa == 0) && (wTmwba != 0 ) && (bTmFull == FALSE)) {
		//トレースバッファにはデータが存在しない。
		//トレースバッファからトレースメモリへデータを押し出す処理
		ferr = SetTrcFpgaBuf2Mem(&wTmwa,&wTmwba);
		if (ferr != FFWERR_OK) {
			delete [] wRd;
			return ferr;
		}
	} else {
		//トレースバッファにデータが存在する
		//E20トレースバッファデータ取得
		ferr = GetTrcFpgaBuffData(wTmwba,wRd, 0);
		if (ferr != FFWERR_OK) {
			delete [] wRd;
			return ferr;
		}
	}
	// トレースバッファ内の識別子1の数をカウント
	nTrcBuffCmdCnt = cntTraceBuffCycle(&wRd[0], dwRdBuffSize);

	//トレースサイクル数仮取得
	dwTrcCmdCnt = GetTrcFpgaAllBlockCycle();
	dwTrcCmdCnt	= nTrcBuffCmdCnt + dwTrcCmdCnt;
	//サイクルが多すぎる(トレースメモリ+トレースバッファの最大値を超える)　もしくは　サイクルが無い(0xFFFFFFFF)
	if (dwTrcCmdCnt > dwCycNumMax) {	// RevRxNo130408-001 Modify Line
		dwTrcCmdCnt = RCY_CYCLE_ALL_FF;
	}

	// 取得サイクルあり
	// トレースフル状態チェック
	if ((bOverFlow ==  TRUE) && (dwTrcCmdCnt != RCY_CYCLE_NON)) {	// オーバーフロー発生の場合
		// 先頭サイクル～最終ブロック
		for (dwCnt = wTmwa; dwCnt <= dwBlockEnd; dwCnt++) {
			// 無効な識別子1を検索
			if (bNonTrcCycSearch == FALSE) {
				ferr = getTraceNonCycle(dwCnt, &dwBlkTrcCmdCnt, &dwNonTrcCmdCnt);
				if (ferr != FFWERR_OK) {
					delete [] wRd;
					return ferr;
				}
				//無効サイクルのトータルをカウント
				dwNonTrcCmdAllCnt = dwNonTrcCmdAllCnt + dwNonTrcCmdCnt;
				// 全データが無効でない場合
				if (dwBlkTrcCmdCnt != dwNonTrcCmdCnt) {
					s_nNonCycleCnt = dwNonTrcCmdCnt;
					bNonTrcCycSearch = TRUE;	// 検索完了
					// インデックス設定
					s_dwRdIndex[dwCnt].dwStartCycle = 0;
					s_dwRdIndex[dwCnt].dwEndCycle = dwBlkTrcCmdCnt - dwNonTrcCmdCnt - 1;	// サイクルNoは0スタートなので-1する
					s_dwRdIndexSBlkPos = dwCnt;												//スタートブロック
					if ( dwCnt == 0) {														//エンドブロック　スタートが0の時は最後のブロックになる
						s_dwRdIndexEBlkPos = dwBlockEnd;
					} else {
						s_dwRdIndexEBlkPos = (DWORD)wTmwaEnd;
					}
				}
			}
		}
		// 先頭ブロック～最終書き込みブロック
		for (dwCnt = dwBlockStart; dwCnt <= (DWORD)wTmwaEnd; dwCnt++) {
			// 無効な識別子1を検索
			if (bNonTrcCycSearch == FALSE) {
				ferr = getTraceNonCycle(dwCnt, &dwBlkTrcCmdCnt, &dwNonTrcCmdCnt);
				if (ferr != FFWERR_OK) {
					delete [] wRd;
					return ferr;
				}
				//無効サイクルのトータルをカウント
				dwNonTrcCmdAllCnt = dwNonTrcCmdAllCnt + dwNonTrcCmdCnt;
				// 全データが無効でない場合
				if (dwBlkTrcCmdCnt != dwNonTrcCmdCnt) {
					s_nNonCycleCnt = dwNonTrcCmdCnt;
					bNonTrcCycSearch = TRUE;	// 検索完了
					// インデックス設定
					s_dwRdIndex[dwCnt].dwStartCycle = 0;
					s_dwRdIndex[dwCnt].dwEndCycle = dwBlkTrcCmdCnt - dwNonTrcCmdCnt - 1;	// サイクルNoは0スタートなので-1する
					s_dwRdIndexSBlkPos = dwCnt;												
					s_dwRdIndexEBlkPos =  (DWORD)wTmwaEnd;
				}
			}
		}
	} else if(dwTrcCmdCnt != RCY_CYCLE_ALL_FF) {										// トレース未フルの場合
		// 先頭ブロック～最終書き込みブロック
		for (dwCnt = dwBlockStart; dwCnt <= (DWORD)wTmwaEnd; dwCnt++) {
			// 無効な識別子1を検索
			if (bNonTrcCycSearch == FALSE) {
				ferr = getTraceNonCycle(dwCnt, &dwBlkTrcCmdCnt, &dwNonTrcCmdCnt);
				if (ferr != FFWERR_OK) {
					delete [] wRd;
					return ferr;
				}
				//無効サイクルのトータルをカウント
				dwNonTrcCmdAllCnt = dwNonTrcCmdAllCnt + dwNonTrcCmdCnt;
				// 全データが無効でない場合
				if (dwBlkTrcCmdCnt != dwNonTrcCmdCnt) {
					s_nNonCycleCnt = dwNonTrcCmdCnt;
					bNonTrcCycSearch = TRUE;	// 検索完了
					// インデックス設定
					s_dwRdIndex[dwCnt].dwStartCycle = 0;
					if (dwBlkTrcCmdCnt == 1) {													// 1サイクルだけのとき
						s_dwRdIndex[dwCnt].dwEndCycle = dwBlkTrcCmdCnt;
					} else {
						s_dwRdIndex[dwCnt].dwEndCycle = dwBlkTrcCmdCnt - dwNonTrcCmdCnt - 1;	// サイクルNoは0スタートなので-1する
					}
					s_dwRdIndexSBlkPos = dwCnt;
					s_dwRdIndexEBlkPos = (DWORD)wTmwaEnd;									//終了ブロック位置
				}
			}
		}
	}

	// トレースメモリ領域がすべて無効サイクルで、トレースバッファ内にパケットが存在するとき
	if((bNonTrcCycSearch == FALSE) && (nTrcBuffCmdCnt >= 0) ){
		dwTrcCmdCnt = 0;				//トレースメモリ内は0
		//FPGAバッファ内を探す
		if(s_bDummyTrcPkt != TRUE) {
			analysisNonTrcData(&wRd[0], dwRdBuffSize, &dwNonTrcCmdCnt);
		} else {
			analysisNonAddrTrcData(&wRd[0], dwRdBuffSize, &dwNonTrcCmdCnt);
		}
		nTrcBuffCmdCnt = nTrcBuffCmdCnt - dwNonTrcCmdCnt;
		if(nTrcBuffCmdCnt != RCY_CYCLE_ALL_FF){
			s_nNonCycleCnt = dwNonTrcCmdCnt;
		}
	}
	bNonTrcCycSearch = FALSE;	// フラグ初期

	//FPGAバッファデータ格納メモリをクリア
	delete [] wRd;


	if(( s_dwRdIndexSBlkPos != -1) && (s_dwRdIndexEBlkPos != -1) ) {
	// トレースメモリ内にデータがあった場合、全サイクル数取得しながら、インデックス作成
		ferr = getTraceAllBlockCycleDiv(&dwTrcCmdCnt);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// 無効サイクルを全サイクルから引く
		dwTrcCmdCnt = dwTrcCmdCnt - dwNonTrcCmdAllCnt;
	} else {
		// トレースメモリ内にデータがなかった場合、
		dwTrcCmdCnt = 0;
	}
	s_dwTrcCmdCnt = dwTrcCmdCnt;

	// トレースサイクル数計算
	dwEndCycBuf = dwTrcCmdCnt + nTrcBuffCmdCnt;
	// ブレーク後BTR1掃きだし対策 対策実施していた場合は、最後BTR2はダミー
	if ((s_bSetDummyPktBtr1 == TRUE) && (dwEndCycBuf != RCY_CYCLE_ALL_FF) ){
		dwEndCycBuf = dwEndCycBuf-1;
	}

	//サイクルが多すぎる(トレースメモリ+トレースバッファの最大値を超える)　もしくは　サイクルが無い(0xFFFFFFFF)
	if(dwEndCycBuf > dwCycNumMax) {	// RevRxNo130408-001 Modify Line
		// 取得サイクルなし
		pRcy->dwStartCyc = 0;
		pRcy->dwEndCyc = RCY_CYCLE_ALL_FF;
		// RCY完了
		s_bTracSetRCY = TRUE;
	} else {
		if (dwTrcCmdCnt == 0) {
			//トレースメモリ内にデータがない場合
			s_dwRdIndexSBlkPos = dwBlockEnd + 1;
			s_dwRdIndexEBlkPos = dwBlockEnd + 1;	
			for(i=0;i<=dwBlockEnd;i++){
				s_dwRdIndex[wTmwaEnd].dwStartCycle = RCY_CYCLE_ALL_FF;
				s_dwRdIndex[wTmwaEnd].dwEndCycle = RCY_CYCLE_ALL_FF;
			}
			// バッファのインデックス情報は、トレースメモリの最後のブロックの後ろにつけておく
			s_dwRdIndex[dwBlockEnd + 1].dwStartCycle = 0;
			s_dwRdIndex[dwBlockEnd + 1].dwEndCycle = dwEndCycBuf;
		} else {
			// バッファにデータがある場合はバッファのインデックス情報は、トレースメモリの最後のブロックの後ろにつけておく
			if(nTrcBuffCmdCnt != RCY_CYCLE_ALL_FF) {
				s_dwRdIndex[dwBlockEnd + 1].dwStartCycle = dwTrcCmdCnt - 1;
				s_dwRdIndex[dwBlockEnd + 1].dwEndCycle = dwEndCycBuf;
			}
		}
		// 取得サイクルセット
		pRcy->dwStartCyc = 0;
		pRcy->dwEndCyc = dwEndCycBuf;
		// RCY完了
		s_bTracSetRCY = TRUE;
	}

	return FFWERR_OK;

}

//==============================================================================
/**
 * トレース情報種別を設定する。
 * @param  pRd         取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wCmdInf     識別子1の値を指定
 * @retval なし
 */
//==============================================================================
static void setTraceType(FFWRX_RD_DATA* pRd, WORD wCmdInf)
{
	// トレース識別子1情報をセット
	pRd->byType1 = (BYTE)wCmdInf;

	switch (wCmdInf) {
	case STDBY:
		pRd->stdby.dwStdby = TRUE;
		pRd->dwType = RM_STDBY;
		break;
	case LOST:
		pRd->lost.dwLost = TRUE;
		pRd->dwType = RM_LOST;
		break;
	case MODE:
		pRd->dwType = RM_STACK;
		break;
	case BTR1:
		if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS){	// ハードウェアデバッグモードの場合
			pRd->dwType = RM_HWBR;
		} else {												// ソフトウェアデバッグモードの場合
			pRd->dwType = RM_BRCND;
		}
		break;
	case BTR2:
		if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS){	// ハードウェアデバッグモードの場合
			pRd->dwType = RM_HWBR;
		} else {												// ソフトウェアデバッグモードの場合
			pRd->dwType = RM_DEST;
		}
		break;
	case BTR3:
		if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS){	// ハードウェアデバッグモードの場合
			pRd->dwType = RM_HWBR;
		} else {												// ソフトウェアデバッグモードの場合
			pRd->dwType = RM_SRC;
		}
		break;
	case DTR1:
		pRd->dwType = RM_OPR;
		break;
	case DTR2:
		pRd->dwType = RM_OPR;
		break;
	case DTR4:
		pRd->dwType = RM_OPR;
		break;
	case DTW1:
		pRd->dwType = RM_OPR;
		break;
	case DTW2:
		pRd->dwType = RM_OPR;
		break;
	case DTW4:
		pRd->dwType = RM_OPR;
		break;
	default:
		break;
	}
	
	return;

}

// RevRxNo161003-001 Append Start +33
//==============================================================================
/**
 * コマンド種別に応じたアクセス属性とアクセスサイズを設定する。
 * @param  wCmdKind			コマンド種別を指定
 * @param  pdwAccess		アクセス属性を設定
 * @param  pdwAccessSize	アクセスサイズを設定
 * @retval なし
 */
//==============================================================================
static void setDataAccessInfo(WORD wCmdKind, DWORD* pdwAccess, DWORD* pdwAccessSize)
{
	switch (wCmdKind) {
	case DTR1:
		*pdwAccess = TRC_OPR_ACS_R;				// アクセス属性　：リード
		*pdwAccessSize = TRC_OPR_ACSSIZE_B;		// アクセスサイズ：バイト
		break;
	case DTR2:
		*pdwAccess = TRC_OPR_ACS_R;				// アクセス属性　：リード
		*pdwAccessSize = TRC_OPR_ACSSIZE_W;		// アクセスサイズ：ワード
		break;
	case DTR4:
		*pdwAccess = TRC_OPR_ACS_R;				// アクセス属性　：リード
		*pdwAccessSize = TRC_OPR_ACSSIZE_L;		// アクセスサイズ：ロングワード
		break;
	case DTW1:
		*pdwAccess = TRC_OPR_ACS_W;				// アクセス属性　：ライト
		*pdwAccessSize = TRC_OPR_ACSSIZE_B;		// アクセスサイズ：バイト
		break;
	case DTW2:
		*pdwAccess = TRC_OPR_ACS_W;				// アクセス属性　：ライト
		*pdwAccessSize = TRC_OPR_ACSSIZE_W;		// アクセスサイズ：ワード
		break;
	case DTW4:
		*pdwAccess = TRC_OPR_ACS_W;				// アクセス属性　：ライト
		*pdwAccessSize = TRC_OPR_ACSSIZE_L;		// アクセスサイズ：ロングワード
		break;
	default:		// フェールセーフ
		*pdwAccess = TRC_OPR_ACS_W;				// アクセス属性　：ライト
		*pdwAccessSize = TRC_OPR_ACSSIZE_L;		// アクセスサイズ：ロングワード
		break;
	}

	return;
}
// RevRxNo161003-001 Append End

//==============================================================================
/**
 * フルアドレス検索
 * @param  wKind      コマンド種別
 * @param  wRd        取得したトレースデータ
 * @param  dwStartPos 検索開始位置
 * @param  dwEndPos   検索終了位置
 * @retval フルアドレスの検索結果 TRUE:見つかった FALSE:見つからなかった
 */
//==============================================================================
static BOOL searchFullAddr(WORD wKind, WORD* wRd, DWORD dwStartPos, DWORD dwEndPos)
{
	FFWRX_RD_DATA rd;
	DWORD	i;
	WORD	wTrcCmdKind = 0;
	WORD	wCmdInf;
	DWORD	dwTmShiftCnt = 0;
	DWORD	dwAddrBuff = 0;
	DWORD	dwAddrShiftCnt = 0;
	int		nCycCnt = -1;
	int		nTrcCmd2 = -1;
	int		nTrcCmd3 = -1;
	int		nShiftCnt = 0;
	DWORD	dwSrcAddrCnt = 0;
	DWORD	dwDestAddrCnt = 0;
	int		nSrcShiftCnt = 0;
	int		nDestShiftCnt = 0;
	BOOL	bRet = FALSE;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo161003-001 Append Line

	pFwCtrl = GetFwCtrlData();		// RevRxNo161003-001 Append Line

	memset(&rd, 0, sizeof(FFWRX_RD_DATA));

	for (i = dwStartPos; i < dwEndPos; i++) {
		// トレース情報種別を判定
		wCmdInf = (WORD)(wRd[i] & TRCDATA_CMD_INF);
		// RevRxNo130408-001 Modify Start
//		以下は処理がないため、不要な条件判定をコメント化している。
//		if (wCmdInf == s_wTrDataCmdInfStnby) {		// 無効データ
//			// 処理なし
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {		// 識別子１
			// 識別子2,3のデータ、データシフトカウントを初期化
			nTrcCmd2 = -1;
			nTrcCmd3 = -1;
			nShiftCnt = 0;
			dwTmShiftCnt = 0;
			dwAddrShiftCnt = 0;
			dwAddrBuff = 0;
			dwSrcAddrCnt = 0;
			dwDestAddrCnt = 0;
			nSrcShiftCnt = 0;
			nDestShiftCnt = 0;

			// 識別子1の内容セット
			wTrcCmdKind = (WORD)(wRd[i] & TRCDATA_TRDATA03);

			// サイクル数をインクリメント
			nCycCnt++;

			// STDBY,LOSTの場合
			if (wTrcCmdKind == STDBY || wTrcCmdKind == LOST) {
			}
			// RevRxNo161003-001 Modify Start +21
			// DTR1/DTR2/DTR4/DTW1/DTW2/DTW4の場合
			else if ((wTrcCmdKind == DTR1) || (wTrcCmdKind == DTR2) || (wTrcCmdKind == DTR4) ||
					 (wTrcCmdKind == DTW1) || (wTrcCmdKind == DTW2) || (wTrcCmdKind == DTW4)) {
				if (pFwCtrl->bRomDataTrcAccessTypeExist && ((s_dwTraceInfo & RM_SETINFO_RODTE) == RM_SETINFO_RODTE)) {
										// ROMデータトレース アクセスタイプ出力ビットあり && ROMデータトレース アクセスタイプ出力あり
					// 次のデータも識別子1の場合
					if ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfIdnt1) {
						// ROMアクセスでデータなし(アドレス＆タイムスタンプ未出力)かどうかチェック
						if (((s_dwTraceInfo & RM_SETINFO_RODE) != RM_SETINFO_RODE) &&
							((s_dwTraceInfo & RM_SETINFO_TDA) != RM_SETINFO_TDA) && ((s_dwTraceInfo & RM_SETINFO_TSE) != RM_SETINFO_TSE)) {
							// アクセス属性、アクセスサイズ設定
							setDataAccessInfo(wTrcCmdKind, &rd.opr.dwAccess, &rd.opr.dwAccessSize);
						}
					}
				} else {
					// DTR1の場合
					if (wTrcCmdKind == DTR1) {
						// 次のデータも識別子1の場合
						if ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfIdnt1) {
							// ROMアクセスでデータなし(アドレス＆タイムスタンプ未出力)かどうかチェック
							if (((s_dwTraceInfo & RM_SETINFO_RODE) != RM_SETINFO_RODE) &&
								((s_dwTraceInfo & RM_SETINFO_TDA) != RM_SETINFO_TDA) && ((s_dwTraceInfo & RM_SETINFO_TSE) != RM_SETINFO_TSE)) {
								// アクセス属性
								rd.opr.dwAccess = TRC_OPR_ACS_R;
								// アクセスサイズ
								rd.opr.dwAccessSize = TRC_OPR_ACSSIZE_B;
							}
						}
					}
				}
			}
			// RevRxNo161003-001 Modify End
			// 上記以外の識別子1の場合
			else {
			}

			// トレース情報種別をセット
			setTraceType(&rd, wTrcCmdKind);

		} else if (wCmdInf == s_wTrDataCmdInfIdnt2) {		// 識別子2
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != -1) {
				// MODEの場合
				if (wTrcCmdKind == MODE) {
					// スタックポインタ情報出力可否チェック
					if ((s_dwTraceInfo & RM_SETINFO_TMO) == RM_SETINFO_TMO) {
						rd.stac.dwStac = wRd[i] & TRCDATA_TRDATA03;
					}
				}
				// DTR1,DTR2,DTR4,DTW1,DTW2,DTW4の場合
				else if (wTrcCmdKind == DTR1 || wTrcCmdKind == DTR2 || wTrcCmdKind == DTR4 || 
							wTrcCmdKind == DTW1 || wTrcCmdKind == DTW2 || wTrcCmdKind == DTW4) {
					// イベント番号情報出力可否チェック
					if ((s_dwTraceInfo & RM_SETINFO_TRI) == RM_SETINFO_TRI) {
						// イベント番号(識別子2)を取得
						rd.opr.dwEvNum = wRd[i] & TRCDATA_TRDATA03;
					}
				}
				// 識別子2の内容セット
				nTrcCmd2 = wRd[i] & TRCDATA_TRDATA03;
			}

		} else if (wCmdInf == s_wTrDataCmdInfIdnt3_1) {		// 識別子3-1
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != -1) {
				// 識別子3の内容セット
				nTrcCmd3 = wRd[i] & TRCDATA_TRDATA03;
			}

		} else if (wCmdInf == s_wTrDataCmdInfIdnt3_2) {		// 識別子3-2
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != -1) {
				// BTR1でS/Wデバッグモードの場合
				if (wTrcCmdKind == BTR1 && ((s_dwTraceInfo & RM_SETINFO_BSS) != RM_SETINFO_BSS)) {
					// タイムスタンプ情報出力可否チェック
					if ((s_dwTraceInfo & RM_SETINFO_TSE) == RM_SETINFO_TSE) {
						// 分岐成立可否情報を取得(※+2はスタートビット付加)
						rd.bcnd.dwBcnd = ((wRd[i] & 0x0008) >> 3) + 2;
					}
				}
				// 識別子3の内容セット
				nTrcCmd3 = wRd[i] & TRCDATA_TRDATA03;
			}

		} else if (wCmdInf == s_wTrDataCmdInfData) {		// データ
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != -1) {
				if (wTrcCmdKind == BTR1) {
					if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS){	// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&rd, wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_NORMAL);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR1(&rd, wRd, nTrcCmd3, &nShiftCnt, i);
					}
				} else if (wTrcCmdKind == BTR2) {
					if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS){	// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&rd, wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_SUB);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR2(&rd, wRd, nTrcCmd3, &dwTmShiftCnt, &nShiftCnt, i, &dwAddrBuff);
					}
				} else if (wTrcCmdKind == BTR3) {
					if ((s_dwTraceInfo & RM_SETINFO_BSS) == RM_SETINFO_BSS){	// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&rd, wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_EXCEPT);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR3(&rd, wRd, &nShiftCnt, i);
					}
				} else if (wTrcCmdKind == DTR1 || wTrcCmdKind == DTR2 || wTrcCmdKind == DTR4 ||
							wTrcCmdKind == DTW1 || wTrcCmdKind == DTW2 || wTrcCmdKind == DTW4) {
					setTraceDataDTRW(&rd, wRd, wTrcCmdKind, nTrcCmd3, &dwTmShiftCnt, &dwAddrShiftCnt, &nShiftCnt, i);
				}
			}
		}
		// RevRxNo130408-001 Modify End
	}

	if (wKind == TRC_CMD_BRANCH && s_bGetBrAddr == TRUE) {
		return TRUE;
	} else if (wKind == TRC_CMD_ACCESS && s_bGetDtAddr == TRUE) {
		return TRUE;
	} else if (wKind == TRC_CMD_TIMESTAMP && s_bGetTimeStamp == TRUE) {
		return TRUE;
	}

	return bRet;

}

//==============================================================================
/**
 * 指定ブロックの32ビットアドレス/タイムスタンプ検索
 * (解析対象の1ブロック前の32ビットアドレス/タイムスタンプ検索時に使用)
 * @param  dwSBlockNo 検索開始ブロック
 * @param  dwEBlockNo 検索終了ブロック
 * @param pbRet フルアドレスの検索結果 TRUE:見つかった FALSE:見つからなかった
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR searchFwFullAddr(DWORD dwSBlockNo, DWORD dwEBlockNo, BOOL* pbRet)
{
	FFWERR	ferr;
	DWORD	i,k;
	int		j;
	WORD	wCmdInf;
	WORD*	wTmpRd;
	DWORD*	pdwTmpCmd1Pos;
	DWORD	dwSBlkPosCnt = 0;
	DWORD	dwEBlkPosCnt = 0;
	BOOL	bRet = FALSE;
	BOOL	bBrRet = FALSE;
	BOOL	bOpcRet = FALSE;
	BOOL	bGetDtAddr;
	BOOL	bTimRet = FALSE;
	BOOL	bGetTimeStamp = FALSE;
	DWORD	dwRdBlockSize;	// RevRxNo130408-001 Append Line


	dwRdBlockSize = GetRdBufSizeTraceBlock();	// RevRxNo130408-001 Append Line

	// RevRxNo130408-001 Modify Start
	// トレースメモリブロック 2つ分のデータ格納バッファ確保
	wTmpRd = new WORD[dwRdBlockSize*2];
	memset(wTmpRd, 0, ((dwRdBlockSize*2)*2));

	// トレースメモリブロック 2つ分の"識別子1"位置格納バッファ確保
	pdwTmpCmd1Pos = new DWORD[dwRdBlockSize*2];
	memset(pdwTmpCmd1Pos, 0, ((dwRdBlockSize*2)*4));
	// RevRxNo130408-001 Modify End


	// 検索開始ブロックのトレースメモリデータを取得
	ferr = GetTrcFpgaMemBlock(dwSBlockNo, dwSBlockNo, wTmpRd);
	if (ferr != FFWERR_OK) {
		delete [] wTmpRd;
		delete [] pdwTmpCmd1Pos;
		*pbRet = bRet;
		return ferr;
	}

	// 検索開始ブロックの"識別子1"位置を記憶
	for (i = 0; i < dwRdBlockSize; i++) {	// RevRxNo130408-001 Modify Line
		// トレース情報種別を判定
		wCmdInf = (WORD)(wTmpRd[i] & TRCDATA_CMD_INF);
		// 識別子１の場合
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {	// RevRxNo130408-001 Modify Line
			pdwTmpCmd1Pos[dwSBlkPosCnt] = i;
			dwSBlkPosCnt++;
		}
	}

	wTmpRd += dwRdBlockSize;	// トレースメモリデータ格納位置を1ブロック分アドレスを進める	// RevRxNo130408-001 Modify Line
	dwEBlkPosCnt = dwSBlkPosCnt;

	// 検索終了ブロックのトレースメモリデータを取得
	ferr = GetTrcFpgaMemBlock(dwEBlockNo, dwEBlockNo, wTmpRd);
	if (ferr != FFWERR_OK) {
		delete [] wTmpRd;
		delete [] pdwTmpCmd1Pos;
		*pbRet = bRet;
		return ferr;
	}

	// 検索終了ブロックの"識別子1"位置を記憶
	for (i = 0; i < dwRdBlockSize; i++) {	// RevRxNo130408-001 Modify Line
		// トレース情報種別を判定
		wCmdInf = (WORD)(wTmpRd[i] & TRCDATA_CMD_INF);
		// 識別子１の場合
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {	// RevRxNo130408-001 Modify Line
			pdwTmpCmd1Pos[dwEBlkPosCnt] = dwRdBlockSize + i;	// RevRxNo130408-001 Modify Line
			dwEBlkPosCnt++;
		}
	}

	wTmpRd -= dwRdBlockSize;	// トレースメモリデータ格納位置を戻す	// RevRxNo130408-001 Modify Line

	// フルアドレス検索(分岐アドレス)
	//分岐アドレス取得時にオペランドアドレスを取ってしまうと、オペランドアドレス検索ルーチンを通らなくなる。
	bGetDtAddr = s_bGetDtAddr;
	bGetTimeStamp = s_bGetTimeStamp;
	if ( (s_bGetBrAddr == FALSE) && ((s_dwTraceInfo & RM_SETINFO_TBE) == RM_SETINFO_TBE) ) {
		for (j = dwSBlkPosCnt - 1; j >= 0; j--) {
		// 32bitトレースアドレス対策 0ブロック目の0番目のパケットまできたらダミーパケット
			if( (s_bDummyTrcPkt == TRUE) && (dwSBlockNo == 0) && (j == 0) ) {
				s_nPreBrAddr = DUMMY_BTR_DSTADDR;
				s_bGetBrAddr = TRUE;
				for (k = j + 1; k <= dwSBlkPosCnt - 1; k++) {
					searchFullAddr(TRC_CMD_BRANCH, &wTmpRd[0], pdwTmpCmd1Pos[k], pdwTmpCmd1Pos[k+1]);
				}
				bBrRet = TRUE;
				break;
			} else {
				bBrRet = searchFullAddr(TRC_CMD_BRANCH, &wTmpRd[0], pdwTmpCmd1Pos[j], pdwTmpCmd1Pos[j+1]);
				if (bBrRet == TRUE) {
					for (k = j + 1; k <= dwSBlkPosCnt - 1; k++) {
						searchFullAddr(TRC_CMD_BRANCH, &wTmpRd[0], pdwTmpCmd1Pos[k], pdwTmpCmd1Pos[k+1]);
					}
					break;
				}
			}
		}
	} else {
		bBrRet = TRUE;
	}
	// フルアドレス検索(オペランドアドレス)
	if ( (bGetDtAddr == FALSE) && ((s_dwTraceInfo & RM_SETINFO_TDE) == RM_SETINFO_TDE)) {
		s_bGetDtAddr = FALSE;
		for (j = dwSBlkPosCnt - 1; j >= 0; j--) {
		// 32bitトレースアドレス対策 0ブロック目の0番目のパケットまできたらダミーパケット
			if( (s_bDummyTrcPkt == TRUE) && (dwSBlockNo == 0) && (j == 0) ) {
				s_nPreDtAddr = DUMMY_DTW_ADDRESS;
				s_bGetDtAddr = TRUE;
				for (k = j + 1; k <= dwSBlkPosCnt - 1; k++) {
					searchFullAddr(TRC_CMD_ACCESS, &wTmpRd[0], pdwTmpCmd1Pos[k], pdwTmpCmd1Pos[k+1]);
				}
				bOpcRet = TRUE;
				break;
			} else {
				bOpcRet = searchFullAddr(TRC_CMD_ACCESS, &wTmpRd[0], pdwTmpCmd1Pos[j], pdwTmpCmd1Pos[j+1]);
				if (bOpcRet == TRUE) {
					for (k = j + 1; k <= dwSBlkPosCnt - 1; k++) {
						searchFullAddr(TRC_CMD_ACCESS, &wTmpRd[0], pdwTmpCmd1Pos[k], pdwTmpCmd1Pos[k+1]);
					}
					break;
				}
			}
		}
	} else {
		bOpcRet = TRUE;
	}

	// フルタイムスタンプ検索
	if ( (bGetTimeStamp == FALSE) && ((s_dwTraceInfo & RM_SETINFO_TSE) == RM_SETINFO_TSE)) {
		s_bGetTimeStamp = FALSE;
		for (j = dwSBlkPosCnt - 1; j >= 0; j--) {
		// 32bitトレースアドレス対策 0ブロック目の0番目のパケットまできたらダミーパケット
			if( (s_bDummyTrcPkt == TRUE) && (dwSBlockNo == 0) && (j == 0) ) {
				s_nPreTimeStamp = 0x00000000;
				s_bGetTimeStamp = TRUE;
				for (k = j + 1; k <= dwSBlkPosCnt - 1; k++) {
					searchFullAddr(TRC_CMD_TIMESTAMP, &wTmpRd[0], pdwTmpCmd1Pos[k], pdwTmpCmd1Pos[k+1]);
				}
				bTimRet = TRUE;
				break;
			} else {
				bTimRet = searchFullAddr(TRC_CMD_TIMESTAMP, &wTmpRd[0], pdwTmpCmd1Pos[j], pdwTmpCmd1Pos[j+1]);
				if (bTimRet == TRUE) {
					for (k = j + 1; k <= dwSBlkPosCnt - 1; k++) {
						searchFullAddr(TRC_CMD_TIMESTAMP, &wTmpRd[0], pdwTmpCmd1Pos[k], pdwTmpCmd1Pos[k+1]);
					}
					break;
				}
			}
		}
	} else {
		bTimRet = TRUE;
	}

	// 32ビットの分岐アドレス、オペランドアドレス、タイムスタンプがすべて見つかった場合
	// V.1.02 No.36 タイムスタンプ解析修正 Modify Line
	if ((bBrRet == TRUE) && (bOpcRet == TRUE) && (bTimRet == TRUE)) {
		bRet = TRUE;
	}

	delete [] pdwTmpCmd1Pos;
	delete [] wTmpRd;

	*pbRet = bRet;
	return ferr;

}

//==============================================================================
/**
 * 取得したトレースデータを解析する。
 * @param  pRd           取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wRd           取得したトレースデータ
 * @param  dwRdSize      取得したトレースサイズ
 * @param  dwSBlockNo    開始ブロックNo
 * @param  nStartOffset  開始サイクルのオフセット値
 * @param  nEndOffset    終了サイクルのオフセット値
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR analysisTrcData(FFWRX_RD_DATA* pRd, WORD* wRd, DWORD dwRdSize, DWORD dwSBlockNo, int nStartOffset, int nEndOffset)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	i;
	int		j,k;
	WORD	wTrcCmdKind = 0;
	WORD	wCmdInf;
	DWORD	dwTmShiftCnt = 0;
	DWORD	dwAddrBuff = 0;
	DWORD	dwAddrShiftCnt = 0;
	int		nCycCnt = -1;
	int		nTrcCmd2 = -1;
	int		nTrcCmd3 = -1;
	int		nShiftCnt = 0;
	DWORD	dwSrcAddrCnt = 0;
	DWORD	dwDestAddrCnt = 0;
	int		nSrcShiftCnt = 0;
	int		nDestShiftCnt = 0;
	DWORD*	pdwCmd1Pos;
	DWORD	dwPosCnt = 0;
	DWORD	dwLoopStartCnt;
	BOOL	bBrRet = FALSE;
	BOOL	bOpcRet = FALSE;
	BOOL	bTimRet = FALSE;
	BOOL	bBrOpcRet = FALSE;
	DWORD	dwTmpSBlock;
	DWORD	dwTmpEBlock;
	DWORD	dwBlockEnd;
	int		nPreBrAddr = -1;
	int		nPreDtAddr = -1;
	int		nPreTimeStamp = -1;
	DWORD	bGetBTR3Bkt = FALSE;
	DWORD	dwTraceInfoMask = 0;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo161003-001 Append Line

	pFwCtrl = GetFwCtrlData();		// RevRxNo161003-001 Append Line

	dwBlockEnd = getBlockEnd();	// RevRxNo130408-001 Modify Line

	// 取得データの識別子1の位置を記憶
	pdwCmd1Pos = new DWORD[dwRdSize + 1];
	memset(pdwCmd1Pos, 0, ((dwRdSize + 1)*4));
	for (i = 0; i < dwRdSize; i++) {
		// トレース情報種別を判定
		wCmdInf = (WORD)(wRd[i] & TRCDATA_CMD_INF);
		// 識別子１の場合
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {	// RevRxNo130408-001 Modify Line
			pdwCmd1Pos[dwPosCnt] = i;
			dwPosCnt++;
		}
	}
	//最後の識別子1の終わりの位置を、pdwCmd1Pos[dwRdSize]に格納
	pdwCmd1Pos[dwPosCnt] = dwRdSize;
	// 先頭サイクルが含まれるブロックでオフセットなしの場合
	dwTraceInfoMask = RM_SETINFO_TSE | RM_SETINFO_TDE | RM_SETINFO_TBE;
	if (s_dwRdIndex[dwSBlockNo].dwStartCycle == 0 && nStartOffset == 0 && ((s_dwTraceInfo & dwTraceInfoMask) == dwTraceInfoMask)) {
		// フルアドレス検索を実施しないが0ブロック目かつ、ダミーサイクルがある場合は、先頭を補正する
		if ((s_bDummyTrcPkt == TRUE) && (dwSBlockNo == 0)) { 
			nStartOffset = pdwCmd1Pos[1]+2;
		}
	} else {
		//分岐情報出力
		if ((s_dwTraceInfo & RM_SETINFO_TBE) == RM_SETINFO_TBE) {
			// フルアドレス検索(分岐アドレス)
			s_nPreBrAddr = -1;
			s_bGetBrAddr = FALSE;
			for (j = nStartOffset; j >= 0; j--) {
		// 32bitトレースアドレス対策 0ブロック目の0番目のパケットまできたらダミーパケット
				if( (s_bDummyTrcPkt == TRUE) && (dwSBlockNo == 0) && (j == 0) ) {
					s_nPreBrAddr = DUMMY_BTR_DSTADDR;
					s_bGetBrAddr = TRUE;
					for (k = j + 1; k <= nStartOffset; k++) {
						searchFullAddr(TRC_CMD_BRANCH, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
					}
					nPreBrAddr = s_nPreBrAddr;
					bBrRet = TRUE;
					break;
				} else if((s_bDummyTrcPkt == TRUE) && (dwSBlockNo == dwBlockEnd+1) && (j == 0) && (s_dwTrcCmdCnt == 0)){
					s_nPreBrAddr = DUMMY_BTR_DSTADDR;
					s_bGetBrAddr = TRUE;
					searchFullAddr(TRC_CMD_BRANCH, &wRd[0], pdwCmd1Pos[0], pdwCmd1Pos[1]);
					for (k = j + 1; k <= nStartOffset; k++) {
						searchFullAddr(TRC_CMD_BRANCH, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
					}
					nPreBrAddr = s_nPreBrAddr;
					bBrRet = TRUE;
					break;
				} else 
				{ 
					bBrRet = searchFullAddr(TRC_CMD_BRANCH, &wRd[0], pdwCmd1Pos[j], pdwCmd1Pos[j+1]);
					if (bBrRet == TRUE) {
						for (k = j + 1; k <= nStartOffset; k++) {
							searchFullAddr(TRC_CMD_BRANCH, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
						}
						nPreBrAddr = s_nPreBrAddr;
						break;
					}
				}
			}
		} else {
			s_bGetBrAddr = TRUE;
			bBrRet = TRUE;
		}
		//オペランド情報出力
		if ((s_dwTraceInfo & RM_SETINFO_TDE) == RM_SETINFO_TDE) {
			// フルアドレス検索(オペランドアドレス)
			s_nPreDtAddr = -1;
			s_bGetDtAddr = FALSE;
			for (j = nStartOffset; j >= 0; j--) {
			// 32bitトレースアドレス対策 0ブロック目の0番目のパケットまできたらダミーパケット
				if( (s_bDummyTrcPkt == TRUE) && (dwSBlockNo == 0) && (j == 1) ) {
					s_nPreDtAddr = DUMMY_DTW_ADDRESS;
					s_bGetDtAddr = TRUE;
					for (k = j + 1; k <= nStartOffset; k++) {
						searchFullAddr(TRC_CMD_ACCESS, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
					}
					nPreDtAddr = s_nPreDtAddr;
					bOpcRet = TRUE;
					break;
				} else if((s_bDummyTrcPkt == TRUE) && (dwSBlockNo == dwBlockEnd+1) && (j == 0) && (s_dwTrcCmdCnt == 0)){
					s_nPreDtAddr = DUMMY_DTW_ADDRESS;
					s_bGetDtAddr = TRUE;
					for (k = j + 1; k <= nStartOffset; k++) {
						searchFullAddr(TRC_CMD_ACCESS, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
					}
					// RevRxNo120130-002 Modify Start
					nPreDtAddr = s_nPreDtAddr;
					bOpcRet = TRUE;
					// RevRxNo120130-002 Modify End
					break;
				} else 
				{
					bOpcRet = searchFullAddr(TRC_CMD_ACCESS, &wRd[0], pdwCmd1Pos[j], pdwCmd1Pos[j+1]);
					if (bOpcRet == TRUE) {
						for (k = j + 1; k <= nStartOffset; k++) {
							searchFullAddr(TRC_CMD_ACCESS, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
						}
						nPreDtAddr = s_nPreDtAddr;
						break;
					}
				}
			}
		}else {
			s_bGetDtAddr = TRUE;
			bOpcRet = TRUE;
		}
		//タイムスタンプ情報出力
		if ((s_dwTraceInfo & RM_SETINFO_TSE) == RM_SETINFO_TSE) {
			// 32bitデータ検索(タイムスタンプ)
			s_nPreTimeStamp = -1;
			s_bGetTimeStamp = FALSE;
			for (j = nStartOffset; j >= 0; j--) {
			// 32bitトレースアドレス対策 0ブロック目の0番目のパケットまできたらダミーパケット
				if( (s_bDummyTrcPkt == TRUE) && (dwSBlockNo == 0) && (j == 1) ) {
					s_nPreTimeStamp = 0x00000000;
					s_bGetTimeStamp = TRUE;
					for (k = j + 1; k <= nStartOffset; k++) {
						searchFullAddr(TRC_CMD_TIMESTAMP, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
					}
					nPreTimeStamp = s_nPreTimeStamp;	// RevRxNo120130-002 Modify Line
					bTimRet = TRUE;
					break;
				// RevRxNo120130-002 Delete トレースメモリ内に解析データがない場合の処理を入れると、タイムスタンプは
				// 正常に解析できない場合があるため削除。
				// (ダミータイムスタンプからメモリ位置までデータが飛んでしまうと、上位側を伸張できない場合がある)
				} else 
				{
					bTimRet = searchFullAddr(TRC_CMD_TIMESTAMP, &wRd[0], pdwCmd1Pos[j], pdwCmd1Pos[j+1]);
					if (bTimRet == TRUE) {
						for (k = j + 1; k <= nStartOffset; k++) {
							searchFullAddr(TRC_CMD_TIMESTAMP, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
						}
						nPreTimeStamp = s_nPreTimeStamp;
						break;
					}
				}
			}
		}else {
			s_bGetTimeStamp = TRUE;
			bTimRet = TRUE;
		}
		// 分岐、オペランド、タイムスタンプともに32bitデータが見つかった場合
		// V.1.02 No.36 タイムスタンプ解析修正 Append Start
		if ((bBrRet == TRUE) && (bOpcRet == TRUE) && (bTimRet == TRUE)) {
			bBrOpcRet = TRUE;
		}

		// フルアドレスが見つからなかった場合、先頭サイクルが含まれるブロックまで検索
		dwTmpSBlock = dwSBlockNo;
		dwTmpEBlock = dwSBlockNo + 1;
		i = 0;
		while (bBrOpcRet == FALSE ){
			// V.1.02 No.36 タイムスタンプ解析修正 Append Start
			// オーバーフローしていない場合かつ、開始ブロックが先頭ブロックの場合
			if((s_bE20TrcMemOvr!=TRUE)&&(dwTmpSBlock == 0)){
				bBrOpcRet = TRUE;
			}
			// V.1.02 No.36 タイムスタンプ解析修正 Append End
			// 開始ブロックが先頭ブロックの場合
			else if (dwTmpSBlock == 0) {
				dwTmpSBlock = dwBlockEnd;
				dwTmpEBlock = 0;
			}
			// 前回の終了ブロックが先頭ブロックの場合
			else if (dwTmpEBlock == 0) {
				dwTmpSBlock--;
				dwTmpEBlock = dwBlockEnd;
			}
			// 検索開始ブロックがE20トレースバッファの場合
			// V.1.02 No.36 タイムスタンプ解析修正 Append Start
			else if(dwTmpSBlock == dwBlockEnd+1){
				dwTmpSBlock = (DWORD)s_wTmwaEnd;
				dwTmpEBlock = (DWORD)s_wTmwaEnd+1;
			}
			// V.1.02 No.36 タイムスタンプ解析修正 Append End
			// 上記以外
			else {
				dwTmpSBlock--;
				dwTmpEBlock--;
			}
			//カウントアップ
			i++;
			// V.1.02 No.36 タイムスタンプ解析修正 Modify Start
			if(bBrOpcRet == FALSE){
				ferr = searchFwFullAddr(dwTmpSBlock, dwTmpEBlock,&bBrOpcRet);
				if(ferr != FFWERR_OK){
					delete [] pdwCmd1Pos;
					return ferr;
				}
			}
			// V.1.02 No.36 タイムスタンプ解析修正 Append End
			//フルアドレスが見つかった後、2ブロック以上さかのぼっていた場合、
			//searchFwFullAddr()ではアドレスを求められない アドレス求めなおし
			if( (bBrOpcRet == TRUE) && (i > 1) ){
				for ( j = i; j > 1 ; j--) {
					// 開始ブロックが先頭ブロックの場合
					if (dwTmpSBlock == dwBlockEnd) {
						dwTmpSBlock = 0;
						dwTmpEBlock = 1;
					}
					// 上記以外
					else {
						dwTmpSBlock++;
						dwTmpEBlock++;
					}
					// RevNo110412-002 Modify Start
					// サイクルが存在しない場合は、検索しない
					if(s_dwRdIndex[dwTmpSBlock].dwStartCycle !=  s_dwRdIndex[dwTmpSBlock].dwEndCycle){
						//分岐フルアドレスが求められていなかった
						if (bBrRet == FALSE) {
							getFullAddr(TRC_CMD_BRANCH, dwTmpSBlock, dwTmpEBlock);
						}
						//オペランドフルアドレスが求められていなかった
						if (bOpcRet == FALSE) {
							getFullAddr(TRC_CMD_ACCESS, dwTmpSBlock, dwTmpEBlock);
						}
						//タイムスタンプが求められていなかった
						if (bTimRet == FALSE) {
							getFullAddr(TRC_CMD_TIMESTAMP, dwTmpSBlock, dwTmpEBlock);
						}
					}
					// RevNo110412-002 Modify End
					i--;
				}
			}
			//最後の1ブロックはオフセット分だけ進めて、フルアドレスを求める
			if ( (bBrOpcRet == TRUE) && (i == 1)) {
				if (bBrRet == FALSE) {
					for (k = 0; k <= nStartOffset; k++) {
						searchFullAddr(TRC_CMD_BRANCH, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
					}
				}
				if (bOpcRet == FALSE) {
					for (k = 0; k <= nStartOffset; k++) {
						searchFullAddr(TRC_CMD_ACCESS, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
					}
				}
				if (bTimRet == FALSE) {
					for (k = 0; k <= nStartOffset; k++) {
						searchFullAddr(TRC_CMD_TIMESTAMP, &wRd[0], pdwCmd1Pos[k], pdwCmd1Pos[k+1]);
					}
				}
			}

			// メモリを一周検索をしても見つからなかったら、フルアドレスがなかったとして、終了
			if (i == (dwBlockEnd)) {
				break;
			}
		}
		//事前にアドレス取得していたならば、アドレス差し戻し
		//分岐アドレス差し戻し
		if(bBrRet == TRUE) {
			s_nPreBrAddr = nPreBrAddr;
		}
		if(bOpcRet == TRUE) {
			s_nPreDtAddr = nPreDtAddr;
		}
		if(bTimRet == TRUE) {
			s_nPreTimeStamp = nPreTimeStamp;
		}
	}

	// 解析開始サイクル位置を取得
	dwLoopStartCnt = pdwCmd1Pos[nStartOffset];
	delete [] pdwCmd1Pos;

	// 取得データの解析
	// << 取得トレースデータの配置 >>
	//   <--------------------wRd[0]---------------------->
	//  15      12          9   8  7                      0
	//  ┌────┬──┬─┬─┬────────────┐
	//  │        │    │  │  │                        │
	//  └─┬──┴┬─┴┬┴┬┴┬───────────┘
	//      │      │    │  │  └ [7:0]   TRDATA  : コマンド/データ
	//      │      │    │  └── [8]     TRSYNC1 : コマンド/データ識別信号１
	//      │      │    └──── [9]     TRSYNC2 : コマンド/データ識別信号２
	//      │      └─────── [11:10] Reserved
	//      └─────────── [15:12] CMD_INF: トレース情報種別
	//                                                4'h0 : 無効データ
	//                                                4'h1 : 識別子１
	//                                                4'h2 : 識別子２
	//                                                4'h4 : 識別子３－１
	//                                                       ・前の値が識別子２
	//                                                4'h6 : 識別子３－２
	//                                                       ・前の値が識別子１
	//                                                4'h8 : データ

	for (i = dwLoopStartCnt; i < dwRdSize; i++) {
		// トレース情報種別を判定
		wCmdInf = (WORD)(wRd[i] & TRCDATA_CMD_INF);
		// RevRxNo130408-001 Modify Start
//		以下は処理がないため、不要な条件判定をコメント化している。
//		if (wCmdInf == s_wTrDataCmdInfStnby) {		// 無効データ
//			// 処理なし
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {		// 識別子１
			// サイクル数とタイムスタンプ情報を初期化(フルアドレス取得まで)
			if (s_bGetBrAddr == FALSE && s_bGetDtAddr == FALSE) {
				nCycCnt = -1;
				s_nPreTimeStamp = -1;
				//LostフラグをLostじゃない状態にしておく
				s_bLostBrAddr = FALSE;
				s_bLostDtAddr = FALSE;
			}
			// 識別子2,3のデータ、データシフトカウントを初期化
			nTrcCmd2 = -1;
			nTrcCmd3 = -1;
			nShiftCnt = 0;
			dwTmShiftCnt = 0;
			dwAddrShiftCnt = 0;
			dwAddrBuff = 0;
			dwSrcAddrCnt = 0;
			dwDestAddrCnt = 0;
			nSrcShiftCnt = 0;
			nDestShiftCnt = 0;

			// 識別子1の内容セット
			wTrcCmdKind = (WORD)(wRd[i] & TRCDATA_TRDATA03);

			// サイクル数をインクリメント
			nCycCnt++;
			// サイクル数が終了サイクルを超えた場合、終了
			if (nEndOffset < nCycCnt) {
				return FFWERR_OK;
			}
			// トレースデータ格納領域を初期化
			memset(&pRd[nCycCnt], 0, sizeof(FFWRX_RD_DATA));			

			// STDBYの場合
			if (wTrcCmdKind == STDBY) {
			}
			// LOSTの場合
			else if (wTrcCmdKind == LOST) {
				// LOST発生時、LOST前のパケットとの差分を取るためLOST状態にしなくてよい
				s_bLostBrAddr = FALSE;
				s_bLostDtAddr = FALSE;
			}
			// RevRxNo161003-001 Modify Start +23
			// DTR1/DTR2/DTR4/DTW1/DTW2/DTW4の場合
			else if ((wTrcCmdKind == DTR1) || (wTrcCmdKind == DTR2) || (wTrcCmdKind == DTR4) ||
					 (wTrcCmdKind == DTW1) || (wTrcCmdKind == DTW2) || (wTrcCmdKind == DTW4)) {
				if (pFwCtrl->bRomDataTrcAccessTypeExist && ((s_dwTraceInfo & RM_SETINFO_RODTE) == RM_SETINFO_RODTE)) {
										// ROMデータトレース アクセスタイプ出力ビットあり && ROMデータトレース アクセスタイプ出力あり
					// 次のデータも識別子1か0(終わり)の場合
					if (((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfIdnt1) || ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfStnby) ) {
						// ROMアクセスでデータなし(アドレス＆タイムスタンプ未出力)かどうかチェック
						if (((s_dwTraceInfo & RM_SETINFO_RODE) != RM_SETINFO_RODE) &&
							((s_dwTraceInfo & RM_SETINFO_TDA) != RM_SETINFO_TDA) && ((s_dwTraceInfo & RM_SETINFO_TSE) != RM_SETINFO_TSE)) {
							// アクセス属性、アクセスサイズ設定
							setDataAccessInfo(wTrcCmdKind, &pRd[nCycCnt].opr.dwAccess, &pRd[nCycCnt].opr.dwAccessSize);
							pRd[nCycCnt].opr.bComplete = TRUE;
						}
					}
				} else {
					// DTR1の場合
					if (wTrcCmdKind == DTR1) {
						// 次のデータも識別子1か0(終わり)の場合
						if (((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfIdnt1) || ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfStnby) ) {
							// ROMアクセスでデータなし(アドレス＆タイムスタンプ未出力)かどうかチェック
							if (((s_dwTraceInfo & RM_SETINFO_RODE) != RM_SETINFO_RODE) &&
								((s_dwTraceInfo & RM_SETINFO_TDA) != RM_SETINFO_TDA) && ((s_dwTraceInfo & RM_SETINFO_TSE) != RM_SETINFO_TSE)) {
								// アクセス属性
								pRd[nCycCnt].opr.dwAccess = TRC_OPR_ACS_R;
								// アクセスサイズ
								pRd[nCycCnt].opr.dwAccessSize = TRC_OPR_ACSSIZE_B;
								pRd[nCycCnt].opr.bComplete = TRUE;
							}
						}
					}
				}
			}
			// RevRxNo161003-001 Modify End
			// 上記以外の識別子1の場合
			else {
			}

			// トレース情報種別をセット
			setTraceType(&pRd[nCycCnt], wTrcCmdKind);

		} else if (wCmdInf == s_wTrDataCmdInfIdnt2) {		// 識別子2
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != -1) {
				// MODEの場合
				if (wTrcCmdKind == MODE) {
					// スタックポインタ情報出力可否チェック
					if ((s_dwTraceInfo & RM_SETINFO_TMO) == RM_SETINFO_TMO) {
						pRd[nCycCnt].stac.dwStac = wRd[i] & TRCDATA_TRDATA03;
					}
				}
				// DTR1,DTR2,DTR4,DTW1,DTW2,DTW4の場合
				else if (wTrcCmdKind == DTR1 || wTrcCmdKind == DTR2 || wTrcCmdKind == DTR4 || 
							wTrcCmdKind == DTW1 || wTrcCmdKind == DTW2 || wTrcCmdKind == DTW4) {
					// イベント番号情報出力可否チェック
					if ((s_dwTraceInfo & RM_SETINFO_TRI) == RM_SETINFO_TRI) {
						// イベント番号(識別子2)を取得
						pRd[nCycCnt].opr.dwEvNum = wRd[i] & TRCDATA_TRDATA03;
					}
				}
				// 識別子2の内容セット
				nTrcCmd2 = wRd[i] & TRCDATA_TRDATA03;
			}

		} else if (wCmdInf == s_wTrDataCmdInfIdnt3_1) {		// 識別子3-1
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != -1) {
				// 識別子3の内容セット
				nTrcCmd3 = wRd[i] & TRCDATA_TRDATA03;
			}
			// 次のデータが識別子1か0(終わり)の場合
			if (((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfIdnt1) || ((wRd[i+2] & TRCDATA_CMD_INF) == s_wTrDataCmdInfStnby) ) {
				// ROMアクセスでデータなし(アドレス＆タイムスタンプ未出力)かどうかチェック
				if (((s_dwTraceInfo & RM_SETINFO_RODE) != RM_SETINFO_RODE) &&
					((s_dwTraceInfo & RM_SETINFO_TDA) != RM_SETINFO_TDA) && ((s_dwTraceInfo & RM_SETINFO_TSE) != RM_SETINFO_TSE)) {
					// RevRxNo161003-001 Modify Start +6
					if (pFwCtrl->bRomDataTrcAccessTypeExist && ((s_dwTraceInfo & RM_SETINFO_RODTE) == RM_SETINFO_RODTE)) {
										// ROMデータトレース アクセスタイプ出力ビットあり && ROMデータトレース アクセスタイプ出力あり
						// アクセス属性、アクセスサイズ設定
						setDataAccessInfo(wTrcCmdKind, &pRd[nCycCnt].opr.dwAccess, &pRd[nCycCnt].opr.dwAccessSize);
					} else {
						// アクセス属性
						pRd[nCycCnt].opr.dwAccess = TRC_OPR_ACS_R;
						// アクセスサイズ
						pRd[nCycCnt].opr.dwAccessSize = TRC_OPR_ACSSIZE_B;
					}
					// RevRxNo161003-001 Modify End
					pRd[nCycCnt].opr.bComplete = TRUE;
				}
			}

		} else if (wCmdInf == s_wTrDataCmdInfIdnt3_2) {		// 識別子3-2
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != -1) {
				// BTR1でS/Wデバッグモードの場合
				if (wTrcCmdKind == BTR1 && ((s_dwTraceInfo & RM_SETINFO_BSS) != RM_SETINFO_BSS)) {
					// タイムスタンプ情報出力可否チェック
					if ((s_dwTraceInfo & RM_SETINFO_TSE) == RM_SETINFO_TSE) {
						// 分岐成立可否情報を取得(※+2はスタートビット付加)
						pRd[nCycCnt].bcnd.dwBcnd = ((wRd[i] & 0x0008) >> 3) + 2;
					}
				}
				// 識別子3の内容セット
				nTrcCmd3 = wRd[i] & TRCDATA_TRDATA03;
			}

		} else if (wCmdInf == s_wTrDataCmdInfData) {		// データ
			// 識別子1を取得済みの場合のみ
			if (nCycCnt != -1) {
				if (wTrcCmdKind == BTR1) {
					if ((s_dwTraceInfo  & RM_SETINFO_BSS) == RM_SETINFO_BSS) {		// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&pRd[nCycCnt], wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_NORMAL);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR1(&pRd[nCycCnt], wRd, nTrcCmd3, &nShiftCnt, i);
					}
				} else if (wTrcCmdKind == BTR2) {
					if ((s_dwTraceInfo  & RM_SETINFO_BSS) == RM_SETINFO_BSS) {		// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&pRd[nCycCnt], wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_SUB);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR2(&pRd[nCycCnt], wRd, nTrcCmd3, &dwTmShiftCnt, &nShiftCnt, i, &dwAddrBuff);
						//ひとつ前がBTR3だった場合はBTR3パケットにタイムスタンプをつける
						if(	(bGetBTR3Bkt == TRUE) && (pRd[nCycCnt].dest.bComplete == TRUE)){
							pRd[nCycCnt-1].src.dwTime = pRd[nCycCnt].dest.dwTime;
							bGetBTR3Bkt = FALSE;
						}
					}
				} else if (wTrcCmdKind == BTR3) {
					if ((s_dwTraceInfo  & RM_SETINFO_BSS) == RM_SETINFO_BSS) {		// ハードウェアデバッグモードの場合
						setTraceDataHWDBG(&pRd[nCycCnt], wRd, nTrcCmd3, &dwDestAddrCnt, &nDestShiftCnt, &dwSrcAddrCnt, &nSrcShiftCnt, i, TRC_HW_EXCEPT);
					} else {												// ソフトウェアデバッグモードの場合
						setTraceDataBTR3(&pRd[nCycCnt], wRd, &nShiftCnt, i);
						bGetBTR3Bkt = TRUE;
					}
				} else if (wTrcCmdKind == DTR1 || wTrcCmdKind == DTR2 || wTrcCmdKind == DTR4 ||
							wTrcCmdKind == DTW1 || wTrcCmdKind == DTW2 || wTrcCmdKind == DTW4) {
					setTraceDataDTRW(&pRd[nCycCnt], wRd, wTrcCmdKind, nTrcCmd3, &dwTmShiftCnt, &dwAddrShiftCnt, &nShiftCnt, i);
				}
			}
		}
		// RevRxNo130408-001 Modify End
	}

	return ferr;

}

//==============================================================================
/**
 * トレースメモリに格納されたデータの解析
 * @param  pRd          取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  dwStartBlock 取得開始ブロックNo
 * @param  dwEndBlock   取得終了ブロックNo
 * @param  nStartOffset 開始サイクルのオフセット値
 * @param  nEndOffset   終了サイクルのオフセット値
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getTrcData(FFWRX_RD_DATA* pRd, DWORD dwStartBlock, DWORD dwEndBlock, int nStartOffset, int nEndOffset)
{
	FFWERR	ferr;
	WORD*	wRd;
	DWORD	dwRdSize = 0;
	DWORD	dwRdTrcBuffSize = 0;	// RevRxNo130408-001 Modify Line
	DWORD	dwBlockStart = 0x00000000;
	DWORD	dwBlockEnd;
	WORD	wTmwba = 0;
	DWORD	dwRdBlockSize;	// RevRxNo130408-001 Append Line
	DWORD	dwDataBufSize;	// RevRxNo130408-001 Append Line
	DWORD	dwRdTmpSize;	// RevRxNo130408-001 Append Line

	// static変数の初期化
	s_nPreTimeStamp = -1;
	s_nPreBrAddr = -1;
	s_nPreDtAddr = -1;
	s_dwTraceInfo = 0;
	s_bGetBrAddr = FALSE;
	s_bGetDtAddr = FALSE;
	s_bGetTimeStamp = FALSE;

	dwBlockEnd = getBlockEnd();	// RevRxNo130408-001 Modify Line

	// トレース出力情報の設定状態を取得
	getTraceInfo();

	// トレースFPGAのトレースメモリ書込み用バッファアドレスレジスタ
	// (b9:TMWBS (0:バッファ #0、1:バッファ #1))
	ferr = GetTrcFpgaTmwba(&wTmwba);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

// RevRxNo130408-001 Modify Start
	// E20トレースバッファのトレースメモリデータ格納バッファサイズ取得
	dwRdTrcBuffSize = GetRdBufSizeTraceBuffer(wTmwba);

	// 1トレースメモリブロック分のトレースメモリデータ格納バッファサイズ取得
	dwRdBlockSize = GetRdBufSizeTraceBlock();


	// トレースメモリデータ格納バッファ確保と、トレースメモリデータ取得
	if (dwStartBlock == (dwBlockEnd + 1)) {
			// 取得ブロックがdwBlockEndよりも1大きい場合(E20トレースバッファからデータを取得)
		dwRdSize = dwRdBlockSize;	// トレースメモリデータ格納バッファサイズ設定(1ブロック分)
		dwDataBufSize = dwRdSize + dwRdTrcBuffSize + 1;	// wRd[]確保サイズ
		wRd = new WORD[dwDataBufSize];
		memset(wRd, 0, (dwDataBufSize*2));

	} else if (dwStartBlock > dwEndBlock) {
			// 取得ブロックがラップアラウンドしている場合
		dwRdSize = ((dwBlockEnd - dwStartBlock + 1) + (dwEndBlock + 1)) * dwRdBlockSize;	// トレースメモリデータ格納バッファサイズ設定
		dwDataBufSize = dwRdSize + dwRdTrcBuffSize + 1;	// wRd[]確保サイズ
		wRd = new WORD[dwDataBufSize];
		memset(wRd, 0, (dwDataBufSize*2));

		// トレースメモリデータを取得
		// 取得開始ブロック～トレースメモリ最終ブロック
		ferr = GetTrcFpgaMemBlock(dwStartBlock, dwBlockEnd, &wRd[0]);
		if (ferr != FFWERR_OK) {
			delete [] wRd;
			return ferr;
		}

		// ブロック0～終了ブロック
		dwRdTmpSize = (dwBlockEnd - dwStartBlock + 1) * dwRdBlockSize;
		ferr = GetTrcFpgaMemBlock(dwBlockStart, dwEndBlock, &wRd[dwRdTmpSize]);
		if (ferr != FFWERR_OK) {
			delete [] wRd;
			return ferr;
		}

	} else {	// 取得ブロックがラップアラウンドしていない場合
		// (終了ブロック - 開始ブロック + 1) * 1ブロックサイズ
		dwRdSize = (dwEndBlock - dwStartBlock + 1) * dwRdBlockSize;	// トレースメモリデータ格納バッファサイズ設定
		dwDataBufSize = dwRdSize + dwRdTrcBuffSize + 1;	// wRd[]確保サイズ
		wRd = new WORD[dwDataBufSize];
		memset(wRd, 0, (dwDataBufSize*2));

		// トレースメモリデータを取得
		// 取得開始ブロック～終了ブロック
		ferr = GetTrcFpgaMemBlock(dwStartBlock, dwEndBlock, &wRd[0]);
		if (ferr != FFWERR_OK) {
			delete [] wRd;
			return ferr;
		}
	}
// RevRxNo130408-001 Modify End

	// E20トレースバッファデータ取得(wRd[dwRdSize]からデータ格納)
	ferr = GetTrcFpgaBuffData(wTmwba, wRd, dwRdSize);
	if (ferr != FFWERR_OK) {
		delete [] wRd;
		return ferr;
	}

	// 対象データの解析処理
	ferr = analysisTrcData(pRd, &wRd[0], dwDataBufSize, dwStartBlock, nStartOffset, nEndOffset);	// RevRxNo130408-001 Modify Line
	delete [] wRd;

	return ferr;

}

//==============================================================================
/**
 * 取得したトレースデータを解析する。(分岐成立可否情報)
 * @param  pRd            取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wRd            取得したトレースデータ
 * @param  nTrcCmd3       取得した識別子3
 * @param  nShiftCnt      現在のシフト数
 * @param  nDataCnt       現在のデータ位置
 * @retval なし
 */
//==============================================================================
static void setTraceDataBTR1(FFWRX_RD_DATA* pRd, WORD* wRd, int nTrcCmd3, int* nShiftCnt, int nDataCnt)
{
	int		nBuff;
	BYTE	byTimeStamp;
	DWORD	dwDataCnt;

	// 識別子3がない場合
	if (nTrcCmd3 == -1) {
		// 分岐成立可否情報を取得
		nBuff = (wRd[nDataCnt] & TRCDATA_TRDATA03);
		nBuff <<= (*nShiftCnt) * 4;
		pRd->bcnd.dwBcnd |= nBuff;
		(*nShiftCnt)++;
	} else {
		// タイムスタンプ情報出力可否チェック
		if ((s_dwTraceInfo & RM_SETINFO_TSE) == RM_SETINFO_TSE) {
			// タイムスタンプ圧縮情報を取得
			byTimeStamp = (BYTE)(nTrcCmd3 & 0x00000003);
			dwDataCnt = getBitCompressInfo(byTimeStamp);
			if ((DWORD)(*nShiftCnt) != dwDataCnt) {
				nBuff = (wRd[nDataCnt] & TRCDATA_TRDATA03);
				nBuff <<= (*nShiftCnt) * 4;
				pRd->bcnd.dwTime |= nBuff;
				(*nShiftCnt)++;
				// 次のデータが識別子1の場合
				if ( ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfIdnt1) || ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfStnby) ){	// RevRxNo130408-001 Modify Line
						//(次が識別子1)もしくは(最後のデータ)
					setTimeStampComplement(byTimeStamp, &pRd->bcnd.dwTime);
				}
			}
		}
	}

	return;

}

//==============================================================================
/**
 * 取得したトレースデータを解析する。(分岐先アドレス情報)
 * @param  pRd            取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wRd            取得したトレースデータ
 * @param  nTrcCmd3       取得した識別子3
 * @param  dwTmShiftCnt   現在のタイムスタンプのシフト数
 * @param  nShiftCnt      現在のシフト数
 * @param  nDataCnt       現在のデータ位置
 * @param  dwAddrBuff     現在のアドレスデータ
 * @retval なし
 */
//==============================================================================
static void setTraceDataBTR2(FFWRX_RD_DATA* pRd, WORD* wRd, int nTrcCmd3, DWORD* dwTmShiftCnt, int* nShiftCnt, int nDataCnt, DWORD* dwAddrBuff)
{
	DWORD	dwDataCnt;
	int		dwBuff;
	BOOL	bComplete = FALSE;

	// 識別子3がある場合
	if (nTrcCmd3 != -1) {
		// タイムスタンプ圧縮情報を取得
		dwDataCnt = getBitCompressInfo((BYTE)nTrcCmd3);
		if ((*dwTmShiftCnt) != dwDataCnt) {
			dwBuff = (wRd[nDataCnt] & TRCDATA_TRDATA03);
			dwBuff <<= (*dwTmShiftCnt) * 4;
			pRd->dest.dwTime |= dwBuff;
			(*dwTmShiftCnt)++;
			return;
		}
		setTimeStampComplement((BYTE)nTrcCmd3, &pRd->dest.dwTime);
	}
	// 分岐アドレス情報出力可否チェック
	if ((s_dwTraceInfo & RM_SETINFO_TBE) == RM_SETINFO_TBE) {
		dwBuff = (wRd[nDataCnt] & TRCDATA_TRDATA03);
		dwBuff <<= (*nShiftCnt) * 4;
		(*dwAddrBuff) |= dwBuff;
		(*nShiftCnt)++;
		// 次のデータが識別子1の場合
		if ( ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfIdnt1) || ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfStnby) ){	// RevRxNo130408-001 Modify Line
				//(次が識別子1)もしくは(最後のデータ)
			// アドレスが圧縮されている場合
			if ((*nShiftCnt) < 8) {
				bComplete = setAddrInfoComplement((*nShiftCnt), dwAddrBuff, TRC_CMD_BRANCH, TRUE);
			} else {
				bComplete = setAddrInfoComplement((*nShiftCnt), dwAddrBuff, TRC_CMD_BRANCH, TRUE);
			}
			pRd->dest.dwAddr = (*dwAddrBuff);
			pRd->dest.bComplete = bComplete;
		}
	}

	return;
}

//==============================================================================
/**
 * 取得したトレースデータを解析する。(分岐元/先アドレス情報)
 * @param  pRd            取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wRd            取得したトレースデータ
 * @param  nShiftCnt      現在のシフト数
 * @param  nDataCnt       現在のデータ位置
 * @retval なし
 */
//==============================================================================
static void setTraceDataBTR3(FFWRX_RD_DATA* pRd, WORD* wRd, int* nShiftCnt, int nDataCnt)
{
	int		dwBuff;
	BOOL	bComplete = FALSE;

	// 分岐アドレス情報出力可否チェック
	if ((s_dwTraceInfo & RM_SETINFO_TBE) == RM_SETINFO_TBE) {
		dwBuff = (wRd[nDataCnt] & TRCDATA_TRDATA03);
		dwBuff <<= (*nShiftCnt) * 4;
		pRd->src.dwAddr |= dwBuff;
		(*nShiftCnt)++;
		// 次のデータが識別子1の場合
		if ( ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfIdnt1) || ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfStnby) ){	// RevRxNo130408-001 Modify Line
				//(次が識別子1)もしくは(最後のデータ)
			// アドレスが圧縮されている場合
			bComplete = setAddrInfoComplement((*nShiftCnt), &pRd->src.dwAddr, TRC_CMD_BRANCH,FALSE);
			pRd->src.bComplete = bComplete;
		}
	}

	return;

}

//==============================================================================
/**
 * 取得したトレースデータを解析する。(分岐情報:ハードウェアデバッグモード専用)
 * @param  pRd            取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wRd            取得したトレースデータ
 * @param  nTrcCmd3       取得した識別子3
 * @param  dwDestAddrCnt  現在の分岐先アドレス数
 * @param  nDestShiftCnt  現在の分岐先アドレスのシフト数
 * @param  dwSrcAddrCnt   現在の分岐元アドレス数
 * @param  nSrcShiftCnt   現在の分岐元アドレスのシフト数
 * @param  nDataCnt       現在のデータ位置
 * @param  dwBrType       分岐情報の種類(0:一般,1:サブルーチン,2:例外)
 * @retval なし
 */
//==============================================================================
static void setTraceDataHWDBG(FFWRX_RD_DATA* pRd, WORD* wRd, int nTrcCmd3, DWORD* dwDestAddrCnt, int* nDestShiftCnt, DWORD* dwSrcAddrCnt,
						 int* nSrcShiftCnt, int nDataCnt, DWORD dwBrType)
{
	DWORD	dwSrcDataCnt = 0;
	DWORD	dwDestDataCnt = 0;
	DWORD	dwBuff;

	// 識別子3がある場合
	if (nTrcCmd3 != -1) {
		// 分岐先アドレス圧縮情報を取得
		dwBuff = nTrcCmd3 & 0x0000000C;
		dwBuff >>= 2;
		dwDestDataCnt = getBitCompressInfo((BYTE)dwBuff);
		// 分岐元アドレス圧縮情報を取得
		dwBuff = nTrcCmd3 & 0x00000003;
		dwSrcDataCnt = getBitCompressInfo((BYTE)dwBuff);
	}

	// 分岐アドレス情報出力可否チェック
	if ((s_dwTraceInfo & RM_SETINFO_TBE) == RM_SETINFO_TBE) {
		// 分岐先アドレス設定
		if ((*dwDestAddrCnt) != dwDestDataCnt) {
			dwBuff = wRd[nDataCnt] & TRCDATA_TRDATA03;
			dwBuff <<= (*nDestShiftCnt) * 4;
			pRd->hwbr.dwAddrDest |= dwBuff;
			(*nDestShiftCnt)++;
			(*dwDestAddrCnt)++;
			return;
		}
		// 分岐元アドレス設定(分岐先アドレスの後ろに付加されている)
		if (((*dwSrcAddrCnt) != dwSrcDataCnt) && ((*dwDestAddrCnt) == dwDestDataCnt)) {
			dwBuff = wRd[nDataCnt] & TRCDATA_TRDATA03;
			dwBuff <<= (*nSrcShiftCnt) * 4;
			pRd->hwbr.dwAddrSrc |= dwBuff;
			(*nSrcShiftCnt)++;
			(*dwSrcAddrCnt)++;
			// 次のデータが識別子1の場合
			if ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfIdnt1){	// RevRxNo130408-001 Modify Line
					//(次が識別子1)
				// 分岐アドレスが出力される場合
				if ((*dwDestAddrCnt) > 0 && (*dwSrcAddrCnt) > 0) {
					// 分岐先アドレス補完処理
					pRd->hwbr.bDestComplete = setAddrInfoComplement((*dwDestAddrCnt), &pRd->hwbr.dwAddrDest, TRC_CMD_BRANCH ,FALSE);
					// 分岐元アドレス補完処理
					// ※分岐元アドレスはアドレス比較の基準にならない為、s_nPreBrAddrに記憶しない。
					pRd->hwbr.bSrcComplete = setAddrInfoComplement((*dwSrcAddrCnt), &pRd->hwbr.dwAddrSrc, TRC_CMD_HWDBG ,FALSE);
				}

				// 分岐情報の種類
				pRd->hwbr.dwType = dwBrType;
			}
		}
	}

	return;

}

//==============================================================================
/**
 * 取得したトレースデータを解析する。(オペランドアクセス情報(ライト/リード))
 * @param  pRd            取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wRd            取得したトレースデータ
 * @param  wTrcCmdKind    取得した識別子1
 * @param  nTrcCmd3       取得した識別子3
 * @param  dwTmShiftCnt   現在のタイムスタンプのシフト数
 * @param  dwAddrShiftCnt 現在のアドレスデータのシフト数
 * @param  nShiftCnt      現在のシフト数
 * @param  nDataCnt       現在のデータ位置
 * @retval なし
 */
//==============================================================================
static void setTraceDataDTRW(FFWRX_RD_DATA* pRd, WORD* wRd, WORD wTrcCmdKind, int nTrcCmd3, DWORD* dwTmShiftCnt,  DWORD* dwAddrShiftCnt,
						int* nShiftCnt, int nDataCnt)
{
	BOOL	bComplete = FALSE;
	DWORD	dwDataCnt = 0;
	DWORD	dwAddrCnt = 0;
	BYTE	byTimeStamp = 0;
	BYTE	byOprAddr = 0;
	int		dwBuff;
	DWORD	dwSizeDiff = 0;
	DWORD	dwOffsetB = 0x000000FF;
	DWORD	dwOffsetW = 0x0000FFFF;
	DWORD	dwOffsetL = 0xFFFFFFFF;
	DWORD	dwAccess;
	DWORD	dwAccessSize;
	FFWRX_RM_DATA*		pRm;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo161003-001 Append Line

	pFwCtrl = GetFwCtrlData();		// RevRxNo161003-001 Append Line

	pRm = GetRmDataInfo();

	// アクセス属性、アクセスサイズの設定
	switch (wTrcCmdKind) {
	case DTR1:
		dwAccess = TRC_OPR_ACS_R;
		dwAccessSize = TRC_OPR_ACSSIZE_B;
		break;
	case DTR2:
		dwAccess = TRC_OPR_ACS_R;
		dwAccessSize = TRC_OPR_ACSSIZE_W;
		break;
	case DTR4:
		dwAccess = TRC_OPR_ACS_R;
		dwAccessSize = TRC_OPR_ACSSIZE_L;
		break;
	case DTW1:
		dwAccess = TRC_OPR_ACS_W;
		dwAccessSize = TRC_OPR_ACSSIZE_B;
		break;
	case DTW2:
		dwAccess = TRC_OPR_ACS_W;
		dwAccessSize = TRC_OPR_ACSSIZE_W;
		break;
	case DTW4:
		dwAccess = TRC_OPR_ACS_W;
		dwAccessSize = TRC_OPR_ACSSIZE_L;
		break;
	default:
		dwAccess = TRC_OPR_ACS_W;
		dwAccessSize = TRC_OPR_ACSSIZE_L;
		break;
	}

	// RevRxNo161003-001 Append Start +2
	pRd->opr.dwAccess = dwAccess;				// アクセス属性
	pRd->opr.dwAccessSize = dwAccessSize;		// アクセスサイズ
	// RevRxNo161003-001 Append End

	// 識別子3がある場合
	if (nTrcCmd3 != -1) {
		// タイムスタンプ情報出力可否チェック(S/Wデバッグモードのみ対象)
		if (((s_dwTraceInfo & RM_SETINFO_TSE) == RM_SETINFO_TSE) && ((s_dwTraceInfo & RM_SETINFO_BSS) != RM_SETINFO_BSS)) {
			// タイムスタンプ圧縮情報を取得
			byTimeStamp = (BYTE)(nTrcCmd3 & 0x00000003);
			dwDataCnt = getBitCompressInfo(byTimeStamp);
			if ((*dwTmShiftCnt) != dwDataCnt) {
				dwBuff = (wRd[nDataCnt] & TRCDATA_TRDATA03);
				dwBuff <<= (*dwTmShiftCnt) * 4;
				pRd->opr.dwTime |= dwBuff;
				(*dwTmShiftCnt)++;
				//DTR1(ROMアクセス)、識別子1(+識別子2)+識別子3+タイムスタンプの場合
				if ( ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfIdnt1) || ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfStnby) ){	// RevRxNo130408-001 Modify Line
						//(次が識別子1)もしくは(最後のデータ)
					setTimeStampComplement(byTimeStamp, &pRd->opr.dwTime);
					pRd->opr.bComplete = TRUE;
				}
				return;
			}
			setTimeStampComplement(byTimeStamp, &pRd->opr.dwTime);
		}
		// オペランドアクセス時のアドレス情報出力可否チェック
		if ((s_dwTraceInfo & RM_SETINFO_TDA) == RM_SETINFO_TDA) {
			byOprAddr = (BYTE)((nTrcCmd3 & 0x0000000C) >> 2);
			dwAddrCnt = getBitCompressInfo(byOprAddr);
			if ((*dwAddrShiftCnt) != dwAddrCnt) {
				dwBuff = (wRd[nDataCnt] & TRCDATA_TRDATA03);
				dwBuff <<= (*dwAddrShiftCnt) * 4;
				pRd->opr.dwAddr |= dwBuff;
				(*dwAddrShiftCnt)++;
				if ((*dwAddrShiftCnt) == dwAddrCnt) {
					// アドレスが圧縮されている場合
					if ((*dwAddrShiftCnt) < 8) {
						bComplete = setAddrInfoComplement((*dwAddrShiftCnt), &pRd->opr.dwAddr, TRC_CMD_ACCESS,TRUE);
					} else {
						bComplete = setAddrInfoComplement((*dwAddrShiftCnt), &pRd->opr.dwAddr, TRC_CMD_ACCESS,TRUE);
					}
					pRd->opr.bComplete = bComplete;
				}
				return;
			}
		} else {				//オペランドアドレスがない場合は0とする
			s_nPreDtAddr = 0;
			s_bGetDtAddr = TRUE;
			pRd->opr.bComplete = TRUE;
		}
	}
	// RevRxNo161003-001 Modify Start +5
	if (pFwCtrl->bRomDataTrcAccessTypeExist && ((s_dwTraceInfo & RM_SETINFO_RODE) != RM_SETINFO_RODE)
		&& ((pRd->opr.dwAddr >= pRm->dwRomStartAddr) && (pRd->opr.dwAddr <= pRm->dwRomEndAddr))) {
		// ROMデータトレース アクセスタイプ出力ビットあり、かつ、ROMデータトレース出力なしの場合、データ出力しない。
		// アドレスがROM空間先頭アドレスからROM空間最終アドレスで指定した範囲内に入っている場合に限り！
	} else if (pFwCtrl->bRomDataTrcAccessTypeExist == FALSE && (wTrcCmdKind == DTR1)
		&& ((s_dwTraceInfo & RM_SETINFO_RODE) != RM_SETINFO_RODE )
		&& ((pRd->opr.dwAddr >= pRm->dwRomStartAddr) && (pRd->opr.dwAddr <= pRm->dwRomEndAddr) ) ) {
		// ROMデータトレース アクセスタイプ出力ビットなし、かつ、コマンド種別がDTR1、かつ、
		// ROMデータトレース出力なしの場合、データ出力しない。
		// アドレスがROM空間先頭アドレスからROM空間最終アドレスで指定した範囲内に入っている場合に限り！
	// RevRxNo161003-001 Modify End
	} else {
		dwBuff = (wRd[nDataCnt] & TRCDATA_TRDATA03);
		dwBuff <<= (*nShiftCnt) * 4;
		pRd->opr.dwData |= dwBuff;
		(*nShiftCnt)++;
		// 次のデータが識別子1の場合と次が無効データの場合
		if ( ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfIdnt1) || ((wRd[nDataCnt+2] & TRCDATA_CMD_INF ) == s_wTrDataCmdInfStnby) ){	// RevRxNo130408-001 Modify Line
				//(次が識別子1)もしくは(最後のデータ)
			// データが圧縮されている場合 かつ データLost状態じゃない場合
			if ((*nShiftCnt) < 8) {
				// 差分bit数 = (アクセスサイズ * 8bit) - (有効データ数 * 4bit)
				dwSizeDiff = (dwAccessSize * 8) - ((*nShiftCnt) * 4);
				// データ補完処理
				if (dwAccessSize == TRC_OPR_ACSSIZE_B) {
					pRd->opr.dwData &= (dwOffsetB >> dwSizeDiff);
				} else if (dwAccessSize == TRC_OPR_ACSSIZE_W) {
					pRd->opr.dwData &= (dwOffsetW >> dwSizeDiff);
				} else if (dwAccessSize == TRC_OPR_ACSSIZE_L) {
					pRd->opr.dwData &= (dwOffsetL >> dwSizeDiff);
				}
			}

			if ((s_dwTraceInfo & RM_SETINFO_TDA) != RM_SETINFO_TDA) {	//アドレスがない場合
				pRd->opr.bComplete = TRUE;						//データに依存
				s_bGetDtAddr = TRUE;
			}
		}
	}

	// RevRxNo161003-001 Delete -2
	// ROMデータトレースでROMデータを出力しない場合でも、アクセス属性、アクセスサイズはトレース種別に合わせて設定するため、
	// アクセス属性、アクセスサイズの設定は、上部のアクセス属性、アクセスサイズの設定のswitch文の直後に移動。
	
	return;

}

//==============================================================================
/**
 * ビット圧縮情報を取得する。
 * @param  byCompSize     圧縮サイズを指定
 * @retval retCycle       サイクル数(0の場合、エラー)
 */
//==============================================================================
static DWORD getBitCompressInfo(BYTE byCompSize)
{
	DWORD	retCycle = 0;

	switch (byCompSize) {
	case COMP_4BIT:
		retCycle = 1;
		break;
	case COMP_8BIT:
		retCycle = 2;
		break;
	case COMP_16BIT:
		retCycle = 4;
		break;
	case COMP_NON:
		retCycle = 8;
		break;
	default:	// 通らないはず
		retCycle = 0;
		break;
	}

	return retCycle;
}

//==============================================================================
/**
 * タイムスタンプ補完処理
 * @param  byCompSize     圧縮サイズを指定
 * @param  pdwTimeStamp   タイムスタンプ情報
 * @retval なし
 */
//==============================================================================
static void setTimeStampComplement(BYTE byCompSize, DWORD* pdwTimeStamp)
{
	DWORD*	pdwTimeBuff;

	pdwTimeBuff = pdwTimeStamp;

	switch (byCompSize) {
	case COMP_4BIT:
		if (s_nPreTimeStamp == -1) {
			s_nPreTimeStamp = (*pdwTimeBuff) & 0x0000000f;
		} else {
			(*pdwTimeBuff) &= 0x0000000f;
			s_nPreTimeStamp &= 0xfffffff0;
			(*pdwTimeBuff) |= s_nPreTimeStamp;
			s_nPreTimeStamp = (*pdwTimeBuff);
		}
		break;
	case COMP_8BIT:
		if (s_nPreTimeStamp == -1) {
			s_nPreTimeStamp = (*pdwTimeBuff) & 0x000000ff;
		} else {
			(*pdwTimeBuff) &= 0x000000ff;
			s_nPreTimeStamp &= 0xffffff00;
			(*pdwTimeBuff) |= s_nPreTimeStamp;
			s_nPreTimeStamp = (*pdwTimeBuff);
		}
		break;
	case COMP_16BIT:
		if (s_nPreTimeStamp == -1) {
			s_nPreTimeStamp = (*pdwTimeBuff) & 0x0000ffff;
		} else {
			(*pdwTimeBuff) &= 0x0000ffff;
			s_nPreTimeStamp &= 0xffff0000;
			(*pdwTimeBuff) |= s_nPreTimeStamp;
			s_nPreTimeStamp = (*pdwTimeBuff);
		}
		break;
	case COMP_NON:
		s_nPreTimeStamp = (*pdwTimeBuff) & 0xffffffff;
		s_bGetTimeStamp = TRUE;
		break;
	default:	// 通らないはず
		s_nPreTimeStamp = -1;
		break;
	}

	return;
}

//==============================================================================
/**
 * アドレス情報補完処理
 * @param  dwCompSize     圧縮サイズを指定
 * @param  pdwAddrInfo    アドレス情報を指定
 * @param  dwCmdKind      命令種別(0:分岐, 1:データアクセス, 2:ハードウェアデバッグ)
 * @param　bSetAddr		  32bitフルアドレス情報の更新必要/不要フラグ TRUE:必要 FALSE:不要
 * @retval TRUE           解析完了
 * @retval FALSE          解析未完了
 */
//==============================================================================
static BOOL setAddrInfoComplement(DWORD dwCompSize, DWORD* pdwAddrInfo, DWORD dwCmdKind, BOOL bSetAddr)
{
	DWORD*	pdwAddrBuff;
	BOOL	bComplete = FALSE;
	int		nPreAddr = -1;
	BOOL	bPreAddr = FALSE;

	pdwAddrBuff = pdwAddrInfo;

	//Lost発生時次に32bitデータが来るまで、アドレス解析しないようにする
	if ( (dwCmdKind == TRC_CMD_BRANCH) && (s_bLostBrAddr == FALSE) && (s_bGetBrAddr == TRUE)){
		nPreAddr = s_nPreBrAddr;
		bPreAddr = TRUE;
	} else if ( (dwCmdKind == TRC_CMD_ACCESS) && (s_bLostDtAddr == FALSE) && (s_bGetDtAddr == TRUE)){
		nPreAddr = s_nPreDtAddr;
		bPreAddr = TRUE;
	} else if ( (dwCmdKind == TRC_CMD_HWDBG) && (s_bLostBrAddr == FALSE) && (s_bGetBrAddr == TRUE)){
		nPreAddr = s_nPreBrAddr;
		bPreAddr = TRUE;
	}

	switch (dwCompSize) {
	case 1:
		if (bPreAddr == FALSE) {
			bComplete = FALSE;		// 基準値が無いので解析未完了
		} else {
			(*pdwAddrBuff) &= 0x0000000f;
			nPreAddr &= 0xfffffff0;
			(*pdwAddrBuff) |= nPreAddr;
			nPreAddr = (*pdwAddrBuff);
			bComplete = TRUE;		// 基準値が有るので解析完了
		}
		break;
	case 2:
		if (bPreAddr == FALSE) {
			bComplete = FALSE;		// 基準値が無いので解析未完了
		} else {
			(*pdwAddrBuff) &= 0x000000ff;
			nPreAddr &= 0xffffff00;
			(*pdwAddrBuff) |= nPreAddr;
			nPreAddr = (*pdwAddrBuff);
			bComplete = TRUE;		// 基準値が有るので解析完了
		}
		break;
	case 3:
		if (bPreAddr == FALSE) {
			bComplete = FALSE;		// 基準値が無いので解析未完了
		} else {
			(*pdwAddrBuff) &= 0x00000fff;
			nPreAddr &= 0xfffff000;
			(*pdwAddrBuff) |= nPreAddr;
			nPreAddr = (*pdwAddrBuff);
			bComplete = TRUE;		// 基準値が有るので解析完了
		}
		break;
	case 4:
		if (bPreAddr == FALSE) {
			bComplete = FALSE;		// 基準値が無いので解析未完了
		} else {
			(*pdwAddrBuff) &= 0x0000ffff;
			nPreAddr &= 0xffff0000;
			(*pdwAddrBuff) |= nPreAddr;
			nPreAddr = (*pdwAddrBuff);
			bComplete = TRUE;		// 基準値が有るので解析完了
		}
		break;
	case 5:
		if (bPreAddr == FALSE) {
			bComplete = FALSE;		// 基準値が無いので解析未完了
		} else {
			(*pdwAddrBuff) &= 0x000fffff;
			nPreAddr &= 0xfff00000;
			(*pdwAddrBuff) |= nPreAddr;
			nPreAddr = (*pdwAddrBuff);
			bComplete = TRUE;		// 基準値が有るので解析完了
		}
		break;
	case 6:
		if (bPreAddr == FALSE) {
			bComplete = FALSE;		// 基準値が無いので解析未完了
		} else {
			(*pdwAddrBuff) &= 0x00ffffff;
			nPreAddr &= 0xffffff00;
			(*pdwAddrBuff) |= nPreAddr;
			nPreAddr = (*pdwAddrBuff);
			bComplete = TRUE;		// 基準値が有るので解析完了
		}
		break;
	case 7:
		if (bPreAddr == FALSE) {
			bComplete = FALSE;		// 基準値が無いので解析未完了
		} else {
			(*pdwAddrBuff) &= 0x0fffffff;
			nPreAddr &= 0xf0000000;
			(*pdwAddrBuff) |= nPreAddr;
			nPreAddr = (*pdwAddrBuff);
			bComplete = TRUE;		// 基準値が有るので解析完了
		}
		break;
	case 8:
		nPreAddr = (*pdwAddrBuff) & 0xffffffff;
		bComplete = TRUE;			// 非圧縮なので解析不要
		// 32bitなのでLost状態から開放
		if ((dwCmdKind == TRC_CMD_BRANCH) && (bSetAddr == TRUE)){				// RevRxNo120629-001 Modify Line
			s_bLostBrAddr = FALSE;
			s_bGetBrAddr = TRUE;
		} else if ((dwCmdKind == TRC_CMD_ACCESS) && (bSetAddr == TRUE)){		// RevRxNo120629-001 Modify Line
			s_bLostDtAddr = FALSE;
			s_bGetDtAddr = TRUE;
		} else if (dwCmdKind == TRC_CMD_HWDBG) {
			s_bLostBrAddr = FALSE;
			s_bGetBrAddr = TRUE;
		}
		break;
	default:	// 通らないはず
		nPreAddr = -1;
		bComplete = FALSE;			// 基準値が無いので解析未完了
		break;
	}

	// Lost中は32bitデータが来るまでは、前データも更新しない
	if ((dwCmdKind == TRC_CMD_BRANCH) && (s_bLostBrAddr == FALSE) && (bSetAddr == TRUE)){
		s_nPreBrAddr = nPreAddr;
	} else if ((dwCmdKind == TRC_CMD_ACCESS) && (s_bLostDtAddr == FALSE) && (bSetAddr == TRUE)) {
		s_nPreDtAddr = nPreAddr;
	} else if ((dwCmdKind == TRC_CMD_HWDBG) && (s_bLostBrAddr == FALSE)){
		// 分岐元アドレスは基準値にしない
	}

	return bComplete;
}

//==============================================================================
/**
 * トレース出力情報の設定状態を取得する。
 * @param pdwSetInfo トレース出力情報の設定状態を格納
 * @retval なし
 */
//==============================================================================
static void getTraceInfo(void)
{
	FFWRX_RM_DATA*		rm;
	DWORD				dwBuff = 0;
	DWORD				dwSetInfo = 0;

	// トレース設定を取得
	rm = GetRmDataInfo();

	// トレース出力情報の設定をセット
	dwSetInfo = rm->dwSetInfo;

	// トレース情報取得モード(dwSetModeのb0)
	dwBuff = (rm->dwSetMode & RM_SETMODE_TRM);
	dwBuff <<= 16;
	dwSetInfo |= dwBuff;

	s_dwTraceInfo = dwSetInfo;

	return;
}
// 2008.10.8 INSERT_END_E20RX600 }
// 2008.10.24 INSERT_BEGIN_E20RX600(+NN) {
//==============================================================================
/**
 * トレースモードの固定値を設定する。
 * @param dwMode       指定されたトレースモード
 * @param dwData       トレースモードの各データを設定
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR setTraceModeData(DWORD dwMode, DWORD* dwData)
{
	FFWE20_EINF_DATA	einfData;
	FFWERR				ferr;
	DWORD			dwFuncs;
	DWORD			dwTdos;
	DWORD			dwTmwm;
	DWORD			dwTrm;
	BYTE			byTrcMode = 0;				// RevRxNo120626-001 Append Line

	getEinfData(&einfData);						// エミュレータ情報取得

	switch (dwMode) {
	case MODE1:
		// RevRxE2LNo141104-001 Modify Start
		// E20+38ピンケーブル接続以外の場合
		if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
			dwFuncs = FUNCS_FULLTRC;
			dwTdos = TDOS_IN_NOTBRK;
			dwTmwm = TMWM_NOTWRAP;
			dwTrm = TRM_REALTIME;
			byTrcMode = TRCBRK_OFF | IN_TRC;		// RevRxNo120626-001 Append Line
		// RevRxE2LNo141104-001 Modify End
		} else {
			dwFuncs = FUNCS_FULLTRC;
			dwTdos = TDOS_OUT_NOTBRK;
			dwTmwm = TMWM_NOTWRAP;
			dwTrm = TRM_REALTIME;
			byTrcMode = TRCBRK_OFF | EXT_TRC;		// RevRxNo120626-001 Append Line
		}
		break;
	case MODE2:
		// RevRxE2LNo141104-001 Modify Start
		// E20+38ピンケーブル接続以外の場合
		if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
			return FFWERR_RTTMODE_UNSUPRT;
		// RevRxE2LNo141104-001 Modify End
		} else {
			dwFuncs = FUNCS_FULLTRC;
			dwTdos = TDOS_OUT_NOTBRK;
			dwTmwm = TMWM_NOTWRAP;
			dwTrm = TRM_TRCFULL;
			byTrcMode = TRCBRK_OFF | EXT_TRC;		// RevRxNo120626-001 Append Line
			break;
		}
	case MODE3:
		// RevRxE2LNo141104-001 Modify Start
		// E20+38ピンケーブル接続以外の場合
		if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
			return FFWERR_RTTMODE_UNSUPRT;
		// RevRxE2LNo141104-001 Modify End
		} else {
			dwFuncs = FUNCS_FULLTRC;
			dwTdos = TDOS_OUT_NOTBRK;
			dwTmwm = TMWM_NOTWRAP;
			dwTrm = TRM_REALTIME;
			byTrcMode = TRCBRK_ON | EXT_TRC;		// RevRxNo120626-001 Append Line
			break;
		}
	case MODE4:
		// RevRxE2LNo141104-001 Modify Start
		// E20+38ピンケーブル接続以外の場合
		if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
			dwFuncs = FUNCS_FULLTRC;
			dwTdos = TDOS_IN_BRK;
			dwTmwm = TMWM_NOTWRAP;
			dwTrm = TRM_TRCFULL;
			byTrcMode = TRCBRK_ON | IN_TRC;		// RevRxNo120626-001 Append Line
		// RevRxE2LNo141104-001 Modify End
		} else {
			dwFuncs = FUNCS_FULLTRC;
			dwTdos = TDOS_OUT_NOTBRK;
			dwTmwm = TMWM_NOTWRAP;
			dwTrm = TRM_TRCFULL;
			byTrcMode = TRCBRK_ON | EXT_TRC;		// RevRxNo120626-001 Append Line
		}
		break;
	case MODE5:
		// RevRxE2LNo141104-001 Modify Start
		// E20+38ピンケーブル接続以外の場合
		if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
			dwFuncs = FUNCS_FREETRC;
			dwTdos = TDOS_IN_NOTBRK;
			dwTmwm = TMWM_WRAP;
			dwTrm = TRM_REALTIME;
			byTrcMode = TRCBRK_OFF | IN_TRC;		// RevRxNo120626-001 Append Line
		// RevRxE2LNo141104-001 Modify End
		} else {
			dwFuncs = FUNCS_FREETRC;
			dwTdos = TDOS_OUT_NOTBRK;
			dwTmwm = TMWM_WRAP;
			dwTrm = TRM_REALTIME;
			byTrcMode = TRCBRK_OFF | EXT_TRC;		// RevRxNo120626-001 Append Line
		}
		break;
	case MODE6:
		// RevRxE2LNo141104-001 Modify Start
		// E20+38ピンケーブル接続以外の場合
		if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
			return FFWERR_RTTMODE_UNSUPRT;
		// RevRxE2LNo141104-001 Modify End
		} else {
			dwFuncs = FUNCS_FREETRC;
			dwTdos = TDOS_OUT_NOTBRK;
			dwTmwm = TMWM_WRAP;
			dwTrm = TRM_TRCFULL;
			byTrcMode = TRCBRK_OFF | EXT_TRC;		// RevRxNo120626-001 Append Line
		}
		break;
	case MODE7:				// 内蔵トレース フルトレース　ブレークしない
		dwFuncs = FUNCS_FULLTRC;
		dwTdos = TDOS_IN_NOTBRK;
		dwTmwm = TMWM_NOTWRAP;
		dwTrm = TRM_REALTIME;
		byTrcMode = TRCBRK_OFF | IN_TRC;		// RevRxNo120626-001 Append Line
		break;
	case MODE8:				// 内蔵トレース フルトレース　ブレーク
		dwFuncs = FUNCS_FULLTRC;
		dwTdos = TDOS_IN_BRK;
		dwTmwm = TMWM_NOTWRAP;
		dwTrm = TRM_TRCFULL;
		byTrcMode = TRCBRK_ON | IN_TRC;		// RevRxNo120626-001 Append Line
		break;
	case MODE9:				// 内蔵トレース フリートレース
		dwFuncs = FUNCS_FREETRC;
		dwTdos = TDOS_IN_NOTBRK;
		dwTmwm = TMWM_WRAP;
		dwTrm = TRM_REALTIME;
		byTrcMode = TRCBRK_OFF | IN_TRC;		// RevRxNo120626-001 Append Line
		break;
	case MODE10:
		dwFuncs = FUNCS_FULLTRC;
		dwTdos = TDOS_OUT_NOTBRK;
		dwTmwm = TMWM_WRAP;
		dwTrm = TRM_TRCFULL;
		byTrcMode = TRCBRK_OFF | EXT_TRC;		// RevRxNo120626-001 Append Line
		break;
	default:
		return FFWERR_COM;
		break;
	}

	// データ連結
	*dwData = ((dwFuncs << 4) | (dwTdos << 2) | (dwTmwm << 1) | dwTrm);

	// RevRxNo120626-001 Append Start
	// BFWにトレース動作モードを通知
	ferr = SetTrcFpgaTrcMode(byTrcMode);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo120626-001 Append End

	return FFWERR_OK;
}
// 2008.10.24 INSERT_END_E20RX600 }
// V.1.02 No.31 Class2 トレース対応 Append Start
//==============================================================================
/**
 * トレースモードの固定値を設定する。
 * @param dwMode       指定されたトレースモード
 * @param dwData       トレースモードの各データを設定
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR setCls2TraceModeData(DWORD dwMode, DWORD* dwData)
{
	FFWERR			ferr = FFWERR_OK;				//RevRxNo120626-001 Append Line
	DWORD			dwTdos;
	DWORD			dwTmwm;
	// RevNo110401-001 Append Line
	DWORD			dwFuncs;
	//RX220WS対応 Append Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	pMcuInfoData = GetMcuInfoDataRX();
	//RX220WS対応 Append End
	BYTE			byTrcMode = IN_TRC;				// トレースモードは内蔵トレースで初期化 RevRxNo120626-001 Append Line

	switch (dwMode) {
	case MODE1:				// フルトレースモード
		// RevNo110401-001 Modify Start
		dwFuncs = FUNCS_FULLTRC;	
		dwTdos = TDOS_IN_NOTBRK;
		// RevNo110401-001 Modify End
		dwTmwm = TMWM_NOTWRAP;
		byTrcMode |= TRCBRK_OFF;		// トレースフルブレークしない RevRxNo120626-001 Append Line
		break;
	case MODE2:
		return FFWERR_RTTMODE_UNSUPRT;
	case MODE3:
		return FFWERR_RTTMODE_UNSUPRT;
	case MODE4:				// トレースフルブレーク
		//RX220WS対応 Modify Start
		if(((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_WS) == SPC_TRC_RX220_BUFF64_32CYC) || ((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_WS) == SPC_TRC_RX220_BUFF64_0CYC)){
			return FFWERR_RTTMODE_UNSUPRT;
		} else {
			// RevNo110401-001 Modify Start
			dwFuncs = FUNCS_FULLTRC;		//フルトレースモード
			dwTdos = TDOS_IN_BRK;
			// RevNo110401-001 Modify End
			dwTmwm = TMWM_NOTWRAP;
			byTrcMode |= TRCBRK_ON;		// トレースフルブレークしない RevRxNo120626-001 Append Line
		}
		//RX220WS対応 Modify End
		break;
	case MODE5:				// フリートレースモード
		// RevNo110401-001 Modify Start
		dwFuncs = FUNCS_FREETRC;
		dwTdos = TDOS_IN_NOTBRK;
		// RevNo110401-001 Modify End
		dwTmwm = TMWM_WRAP;
		byTrcMode |= TRCBRK_OFF;		// トレースフルブレークしない RevRxNo120626-001 Append Line
		break;
	case MODE6:
		return FFWERR_RTTMODE_UNSUPRT;
	case MODE7:				// 内蔵トレース フルトレース　ブレークしない
		// RevNo110401-001 Modify Start
		dwFuncs = FUNCS_FULLTRC;	
		dwTdos = TDOS_IN_NOTBRK;
		// RevNo110401-001 Modify End
		dwTmwm = TMWM_NOTWRAP;
		byTrcMode |= TRCBRK_OFF;		// トレースフルブレークしない RevRxNo120626-001 Append Line
		break;
	case MODE8:				// 内蔵トレース フルトレース　ブレーク
		//RX220WS対応 Modify Start
		if(((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_WS) == SPC_TRC_RX220_BUFF64_32CYC) || ((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_WS) == SPC_TRC_RX220_BUFF64_0CYC)){
			return FFWERR_RTTMODE_UNSUPRT;
		} else {
			// RevNo110401-001 Modify Start
			dwFuncs = FUNCS_FULLTRC;		//フルトレースモード
			dwTdos = TDOS_IN_BRK;
			// RevNo110401-001 Modify End
			dwTmwm = TMWM_NOTWRAP;
			byTrcMode |= TRCBRK_ON;		// トレースフルブレークしない RevRxNo120626-001 Append Line
		}
		//RX220WS対応 Modify End
		break;
	case MODE9:				// 内蔵トレース フリートレース
		// RevNo110401-001 Modify Start
		dwFuncs = FUNCS_FREETRC;
		dwTdos = TDOS_IN_NOTBRK;
		// RevNo110401-001 Modify End
		dwTmwm = TMWM_WRAP;
		byTrcMode |= TRCBRK_OFF;		// トレースフルブレークしない RevRxNo120626-001 Append Line
		break;
	case MODE10:
		return FFWERR_RTTMODE_UNSUPRT;
	default:
		return FFWERR_FFW_ARG;
		break;
	}

	// RevNo110401-001 Modify Line
	// データ連結
	*dwData = (dwFuncs << 4) | (dwTdos << 2) | (dwTmwm << 1);

	// RevRxNo120626-001 Append Start
	// BFWにトレース動作モードを通知
	ferr = SetTrcFpgaTrcMode(byTrcMode);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo120626-001 Append End

	return FFWERR_OK;
}
// V.1.02 No.31 Class2 トレース対応 Append End

// RevRxNo130408-001 Delete:getTraceBlockDiv()

// RevRxNo130408-001 Append Start
//==============================================================================
/**
 * E20トレースメモリ使用サイズ情報取得
 * @param  なし
 * @retval E20トレースメモリ使用サイズ
 */
//==============================================================================
WORD GetE20TrcMemSize(void)
{
	return s_wSetE20TrcMemSize;
}
// RevRxNo130408-001 Append End

//=============================================================================
/**
 * トレース　実行前の設定
 * @param  eTrcBrk トレースフルブレークの設定有効/無効
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetTrace(enum FFWENM_TRCBRK_MODE eTrcBrk)
{
	FFWERR						ferr = FFWERR_OK;
	DWORD						dwMode;
	DWORD						dwSetMode,dwSetInitial;
	DWORD						dwModeData;
	DWORD						dwBuff;
	DWORD						dwTrcfrq = 0;
	FFWE20_EINF_DATA			einfData;
	DWORD						dwModeOrg, dwModeDataOrg, dwSetModeOrg;	//トレースフルブレーク設定の時にフリーランでブレーク
																		//させないようにするための退避用
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	BOOL	bRet = FALSE;
	FFWRX_RM_DATA*		pRm;

	pRm = GetRmDataInfo();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// 処理高速化のため、実行中フラグにて確認
	bRet = GetMcuRunState();

	// エミュレータ情報取得
	getEinfData(&einfData);	

	// 現在のトレースモード取得
	dwMode = (pRm->dwSetMode & 0xFF000000) >> 24;

	//オリジナルのモードを保持
	dwModeOrg = dwMode;	

	// GOコマンドの場合に、トレースフルブレークをしない
	if(eTrcBrk == TRC_BRK_DISABLE){
		// RevRxE2LNo141104-001 Modify Start
		// E20+38ピンケーブル接続以外の場合
		if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
			if(dwMode == MODE3){
				// MCU実行優先フルトレースブレークの場合、MCU実行優先フルトレースに変更
				dwMode = MODE7;
			} else if(dwMode == MODE4){
				// トレース優先フルトレースブレークの場合、トレース優先フルトレースに変更
				dwMode = MODE7;
			} else if(dwMode == MODE8){
				// 内蔵フルトレースブレークの場合、内蔵フルトレースに変更
				dwMode = MODE7;
			}
		// RevRxE2LNo141104-001 Modify End
		}else{
			// E20で38ピンケーブル使用時の場合
			if(dwMode == MODE3){
				// MCU実行優先フルトレースブレークの場合、MCU実行優先フルトレースに変更
				dwMode = MODE1;
			} else if(dwMode == MODE4){
				// トレース優先フルトレースブレークの場合、トレース優先フルトレースに変更
				dwMode = MODE2;
			} else if(dwMode == MODE8){
				// 内蔵フルトレースブレークの場合、内蔵フルトレースに変更
				dwMode = MODE7;
			}
		}
	}

	// トレースモード設定チェック(オリジナルのモード用)
	// dwSetModeOrgに値を格納することが目的なため、
	// setTraceModeData()の呼び出しは無効になって構わない
	if (dwModeOrg >= MODE1 && dwModeOrg <= MODE10) {
		// 固定値を設定
		ferr = setTraceModeData(dwModeOrg, &dwModeDataOrg);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		dwSetModeOrg = ((dwModeOrg << 24) | dwModeDataOrg);
	} else {
		dwSetModeOrg = dwModeOrg;
	}
	// トレースモード設定チェック
	if (dwMode >= MODE1 && dwMode <= MODE10) {
		// 固定値を設定
		ferr = setTraceModeData(dwMode, &dwModeData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		dwSetMode = ((dwMode << 24) | dwModeData);
	} else {
		dwSetMode = dwMode;
	}


	// 内部変数の更新
	SetRmModeData(dwSetMode);

	// V.1.02 No.31 Class2 トレース対応 Modify Start
	// トレースクロックのチェック
	pMcuInfoData = GetMcuInfoDataRX();
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eTrcClkCtrl == RX_TRCLK_CTRL_RX630) {
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
	// RevRxNo130411-001 Modify Line
	} else if (pFwCtrl->eTrcClkCtrl == RX_TRCLK_CTRL_RX610){
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

	//RevNo100715-014 Modify Start
	//E20 トレースFPGAの設定を入力
	if(GetTrcInramMode() == FALSE){
		//外部トレース時のみ実行
		ferr = SetTrcE20FpgaData();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	//RX OCD レジスタの設定
	if(bRet == FALSE){
		// RevRxNo130411-001 Modify Line
		if (pFwCtrl->eOcdCls == RX_OCD_CLS3) {
			// OCD Class3の場合
			ferr = SetTrcOcdData();
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		} else {
			// OCD Class2の場合
			ferr = SetTrcCls2OcdData();
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// V.1.02 No.31 Class2 トレース対応 Modify End

	// 32bit アドレス不具合対応
	if( (GetTrcInramMode() == FALSE ) && (bRet == FALSE)) {
		//外部トレースかつブレーク中のみ実行
		ferr = PROT_TRC_SetDummyData();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// ダミーパケット設定したフラグ
		s_bSetDummyPkt = TRUE;
	} 
	//RevNo100715-014 Modify End

	// RevNo110125-01 (V.1.02 覚書 No.27) トレースメモリクリア不具合対応
	// トレースメモリクリアサイズは実行処理で受け渡しをする。

	if(eTrcBrk == TRC_BRK_DISABLE){
		// 内部変数の更新
		// 元々のトレースモードがトレースフルブレークで、且つフリーラン実行などトレースフルブレークで
		// ブレークさせないようにした場合、トレースモードを元に戻しておく必要がある。
		SetRmModeData(dwSetModeOrg);
	}

	return ferr;
}

//==============================================================================
/**
 * トレース関連レジスタを初期化する。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR ClrTrcReg(void)
{
	FFWERR	ferr;

	FFWE20_EINF_DATA	einfData;
	
	getEinfData(&einfData);						// エミュレータ情報取得

	//トレースOCDレジスタのクリア
	ferr = ClrTrcOCDReg();
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// E20の入力許可
	ferr = setTrRst();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxE2LNo141104-001 Modify Start
	// E20の38ピンケーブル使用時の場合
	if ((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN)) {
		//E20トレースFPGAレジスタのクリア
		ferr = ClrTrcFpgaReg();
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	return FFWERR_OK;
}

// RevRxNo130408-001 Append Start
//==============================================================================
/**
 * トレースメモリブロックのトレースメモリデータ格納バッファサイズ取得
 * @param  なし
 * @retval トレースメモリデータ格納バッファサイズ(WORD型配列個数)
 *
 * ※本関数は、トレース機能およびカバレッジ機能で使用する。
 */
//==============================================================================
DWORD GetRdBufSizeTraceBlock(void)
{
	FFWRX_RM_DATA*		pRm;
	FFWMCU_DBG_DATA_RX*	pDbg;
	DWORD	dwBufSize;

	pRm = GetRmDataInfo();
	pDbg = GetDbgDataRX();

	// トレース機能有効で、DATA 8ビット、SYNC 2ビットの場合
	if ((pDbg->eTrcFuncMode == EML_TRCFUNC_TRC) && ((pRm->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D8S2)) {
		dwBufSize = RD_BUFSIZE_TRCBLOCK_D8S2;

	// カバレッジ機能有効、または、DATA 4ビット、SYNC 1ビットの場合
	} else {
		dwBufSize = RD_BUFSIZE_TRCBLOCK_D4S1;
	}

	return dwBufSize;
}
// RevRxNo130408-001 Append End

// RevRxNo130408-001 Append Start
//==============================================================================
/**
 * E20トレースバッファのトレースメモリデータ格納バッファサイズ取得
 * @param  wTmwba FPGA SDRAMメモリ書き込み用バッファアドレスレジスタ(TMWBA)値
 * @retval トレースメモリデータ格納バッファサイズ(WORD型配列個数)
 */
//==============================================================================
DWORD GetRdBufSizeTraceBuffer(WORD wTmwba)
{
	FFWRX_RM_DATA*	pRm;
	DWORD	dwBufSize;

	pRm = GetRmDataInfo();

	if ((pRm->dwSetInitial & RM_SETINITIAL_TBW) == RM_SETINITIAL_TBW_D4S1) {
		dwBufSize = (wTmwba & REG_RD_TMWBA_TMWBA) << 2;
	} else {
		dwBufSize = (wTmwba & REG_RD_TMWBA_TMWBA) << 3;
	}

	return dwBufSize;
}
// RevRxNo130408-001 Append End

// RevRxNo130408-001 Modify Start
//==============================================================================
/**
 * トレースメモリ最終ブロック番号を取得
 * @param  なし
 * @retval トレースメモリ最終ブロック番号
 */
//==============================================================================
static DWORD getBlockEnd(void)
{
	DWORD dwBlockEnd = 0;
	DWORD dwTrcMode = 0;
	FFWRX_RM_DATA* pRm;
	DWORD dwMemByteSize;

	dwMemByteSize = getTrcMemByteSize();
	dwBlockEnd = dwMemByteSize / RD_TRC_BLOCK - 1;

	pRm = GetRmDataInfo();
	dwTrcMode = (pRm->dwSetMode & RM_SETMODE_MODE) >> 24;
	if (dwTrcMode <= MODE4) {	// トレースフルモードの場合
		dwBlockEnd--;
	}

	return dwBlockEnd;
}
// RevRxNo130408-001 Modify End

// RevRxNo130408-001 Appned Start
//==============================================================================
/**
 * トレースメモリバイト数取得
 * @param  なし
 * @retval トレースメモリバイト数
 */
//==============================================================================
static DWORD getTrcMemByteSize(void)
{
	DWORD	dwMemByteSize;
	FFWRX_RM_DATA*	pRm;

	pRm = GetRmDataInfo();

	switch (s_wSetE20TrcMemSize) {
	case E20_TMSIZE_128MB:
		dwMemByteSize = RD_TRCMEM_SIZE;
		break;
	case E20_TMSIZE_64MB:
		dwMemByteSize = RD_TRCMEM_SIZE / 2;
		break;
	case E20_TMSIZE_32MB:
		dwMemByteSize = RD_TRCMEM_SIZE / 4;
		break;
	case E20_TMSIZE_16MB:
		dwMemByteSize = RD_TRCMEM_SIZE / 8;
		break;
	case E20_TMSIZE_8MB:
		dwMemByteSize = RD_TRCMEM_SIZE / 16;
		break;
	case E20_TMSIZE_4MB:
		dwMemByteSize = RD_TRCMEM_SIZE / 32;
		break;
	case E20_TMSIZE_2MB:
		dwMemByteSize = RD_TRCMEM_SIZE / 64;
		break;
	case E20_TMSIZE_1MB:
		dwMemByteSize = RD_TRCMEM_SIZE / 128;
		break;
	default:
		dwMemByteSize = RD_TRCMEM_SIZE;
		break;
	}

	return dwMemByteSize;
}
// RevRxNo130408-001 Appned End

//==============================================================================
/**
 * 指定ブロックのフルアドレス生成
 * @param  wKind 分岐/オペランド/タイムスタンプ
 * @param  dwSBlockNo 検索開始ブロック
 * @param  dwEBlockNo 検索終了ブロック
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getFullAddr(WORD wKind, DWORD dwSBlockNo, DWORD dwEBlockNo)
{
	FFWERR	ferr;
	DWORD	i,k;
	WORD	wCmdInf;
	WORD*	wTmpRd;
	DWORD*	pdwTmpCmd1Pos;
	DWORD	dwSBlkPosCnt = 0;
	DWORD	dwEBlkPosCnt = 0;
	DWORD	dwRdBlockSize;	// RevRxNo130408-001 Append Line

	dwRdBlockSize = GetRdBufSizeTraceBlock();	// RevRxNo130408-001 Append Line

	// RevRxNo130408-001 Modify Start
	// トレースメモリブロック 2つ分のデータ格納バッファ確保
	wTmpRd = new WORD[dwRdBlockSize*2];
	memset(wTmpRd, 0, ((dwRdBlockSize*2)*2));

	// トレースメモリブロック 2つ分の"識別子1"位置格納バッファ確保
	pdwTmpCmd1Pos = new DWORD[dwRdBlockSize*2];
	memset(pdwTmpCmd1Pos, 0, ((dwRdBlockSize*2)*4));
	// RevRxNo130408-001 Modify End

	// 検索開始ブロックのトレースメモリデータを取得
	ferr = GetTrcFpgaMemBlock(dwSBlockNo, dwSBlockNo, wTmpRd);
	if (ferr != FFWERR_OK) {
		delete [] wTmpRd;
		delete [] pdwTmpCmd1Pos;
		return ferr;
	}

	// 検索開始ブロックの"識別子1"位置を記憶
	for (i = 0; i < dwRdBlockSize; i++) {	// RevRxNo130408-001 Modify Line
		// トレース情報種別を判定
		wCmdInf = (WORD)(wTmpRd[i] & TRCDATA_CMD_INF);
		// 識別子１の場合
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {	// RevRxNo130408-001 Modify Line
			pdwTmpCmd1Pos[dwSBlkPosCnt] = i;
			dwSBlkPosCnt++;
		}
	}

	wTmpRd += dwRdBlockSize;	// トレースメモリデータ格納位置を1ブロック分アドレスを進める	// RevRxNo130408-001 Modify Line
	dwEBlkPosCnt = dwSBlkPosCnt;

	// 検索終了ブロックのトレースメモリデータを取得
	ferr = GetTrcFpgaMemBlock(dwEBlockNo, dwEBlockNo, wTmpRd);
	if (ferr != FFWERR_OK) {
		delete [] wTmpRd;
		delete [] pdwTmpCmd1Pos;
		return ferr;
	}

	// 検索終了ブロックの"識別子1"位置を記憶
	for (i = 0; i < dwRdBlockSize; i++) {	// RevRxNo130408-001 Modify Line
		// トレース情報種別を判定
		wCmdInf = (WORD)(wTmpRd[i] & TRCDATA_CMD_INF);
		// 識別子１の場合
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {	// RevRxNo130408-001 Modify Line
			pdwTmpCmd1Pos[dwEBlkPosCnt] = dwRdBlockSize + i;	// RevRxNo130408-001 Modify Line
			dwEBlkPosCnt++;
		}
	}

	wTmpRd -= dwRdBlockSize;	// トレースメモリデータ格納位置を戻す	// RevRxNo130408-001 Modify Line

	// フルアドレス検索
	//分岐アドレス取得時にオペランドアドレスを取ってしまうと、オペランドアドレス検索ルーチンを通らなくなる。
	// RevNo110412-002 Modify Start
	if(dwSBlkPosCnt != 0){
		for (k = 0; k <= dwSBlkPosCnt; k++) {
			searchFullAddr(wKind, &wTmpRd[0], pdwTmpCmd1Pos[k], pdwTmpCmd1Pos[k+1]);
		}
	}
	// RevNo110412-002 Modify End
	delete [] pdwTmpCmd1Pos;
	delete [] wTmpRd;

	return ferr;

}

//==============================================================================
/**
 * アドレス情報がのらないトレースデータ解析
 * @param  wRd        取得したトレースデータ
 * @param  dwRdSize   取得したトレースサイズ
 * @param  pdwRetCnt  無効なサイクル数
 * @retval なし
 */
//==============================================================================
static void analysisNonAddrTrcData(WORD* wRd, DWORD dwRdSize, DWORD* pdwRetCnt)
{
	DWORD	i;
	WORD	wTrcCmdKind = 0;
	WORD	wCmdInf;
	int		nCycCnt = 0;
	BOOL	bGetAddrPkt = FALSE;

	// 取得データの解析
	for (i = 0; i < dwRdSize; i++) {
		// トレース情報種別を判定
		wCmdInf = (WORD)(wRd[i] & TRCDATA_CMD_INF);
		// RevRxNo130408-001 Modify Start
//		以下は処理がないため、不要な条件判定をコメント化している。
//		if (wCmdInf == s_wTrDataCmdInfStnby) {		// 無効データ
//			// 処理なし
		if (wCmdInf == s_wTrDataCmdInfIdnt1) {		// 識別子１
			// 前のパケットがアドレス付きパケットなら終了
			if (bGetAddrPkt == TRUE) {
				(*pdwRetCnt) = nCycCnt - 1;		// 1サイクルは有効サイクルの為、-1する
				return;
			}

			nCycCnt++;
			// 識別子1の内容セット
			wTrcCmdKind = (WORD)(wRd[i] & TRCDATA_TRDATA03);

			if ( (wTrcCmdKind > BTR1) && (wTrcCmdKind < 0xF) ){
				bGetAddrPkt = TRUE;
			}

//		以下は処理がないため、不要な条件判定をコメント化している。
//		} else if (wCmdInf == s_wTrDataCmdInfIdnt2) {	// 識別子２
//			// 処理なし
//		} else if (wCmdInf == s_wTrDataCmdInfIdnt3_1) {	// 識別子３－１
//			// 処理なし
//		} else if (wCmdInf == s_wTrDataCmdInfIdnt3_2) {	// 識別子３－２
//			// 処理なし
//		} else if (wCmdInf == s_wTrDataCmdInfData) {	// データ
//			// 処理なし
		}
		// RevRxNo130408-001 Modify End
	}

	if (bGetAddrPkt == TRUE) {
		//1パケットだけのトレースメモリにあるとき 1サイクルは有効サイクルの為、-1する
		(*pdwRetCnt) = nCycCnt-1;
	} else {
		//パケットなし
		(*pdwRetCnt) = nCycCnt;
	}

	return;

}
//==============================================================================
/**
 * トレース入力を停止する。
 * @param  eTrstp         トレース入力の停止/再開を指定
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR setTrStp(enum FFWENM_TRSTP eTrstp)
{
	FFWERR						ferr;
	enum FFWENM_TRSTP			eTrstpTmp;

	ProtInit();

	eTrstpTmp = eTrstp;
	//RevNo1000805-001 Modify Start
	// 内蔵トレース時は再開/停止を実施して処理終了
	if (GetTrcInramMode() == TRUE) {
		// OCDトレースバッファへのデータ入力 再開
		if(eTrstp == TRSTP_RESTART){
			//OCDトレースバッファへのデータ入力
			ferr = SetTrcOcdRst();
		} else {
			//OCDトレースバッファへのデータ停止
			ferr = SetTrcOcdStp(eTrstpTmp);
		}
		return ferr;
	}

	// 再開時の処理
	if(eTrstp == TRSTP_RESTART){
		//トレースリスタート処理
		ferr = setTrRst();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	} else {

		// OCDトレースバッファへのデータ入力 停止
		ferr = SetTrcOcdStp(eTrstpTmp);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		//OCDのトレース出力停止を確認
		ferr = CheckTrcOcdStop();
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// E20のトレース出力停止を確認
		ferr = CheckTrcFpgaStop();
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		//トレース入力停止
		ferr = SetTrcFpgaEnable(FALSE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

	}
	//RevNo1000805-001 Modify End

	return ferr;

}

//==============================================================================
/**
 * トレース入力を再スタートさせる。
 * @param  eTrstp         トレース入力の停止/再開を指定
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR setTrRst(void)
{
	FFWERR						ferr;

	//トレースリスタート処理
	// RevRxE2LNo141104-001 Modify Start
	if (GetTrcInramMode() == FALSE) {
		//トレースFPGA入力再開
		ferr = SetTrcFpgaEnable(TRUE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// OCDトレースバッファへのデータ入力 再開
	ferr = SetTrcOcdRst();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;

}

//内蔵トレース用関数
//==============================================================================
/**
 * 内蔵トレースメモリに格納されたサイクル数取得
 * @param  pRcy       トレースメモリに格納されたトレースデータの開始サイクル、
 *                    終了サイクルを格納するFFW_RCY_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getTraceCycleTrbf(FFWRX_RCY_DATA *pRcy)
{
	FFWERR	ferr;

	DWORD	dwRdCount = 0;		// トレースデータリードサイズ
	DWORD	dwCycle;			// トレースサイクル解析結果格納
	enum FFWENM_TRSTP	eTrstp;
	BOOL	bOff;

	// static変数の初期化
	s_bOff = 0;		// オーバーフローフラグクリア

	// トレース出力情報の設定状態を取得
	getTraceInfo();

	// OCDトレースバッファへのデータ入力停止
	eTrstp = TRSTP_STOP_KEEP;
	ferr = SetTrcOcdStp(eTrstp);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// オーバーフローフラグ取得
	ferr = GetTrcOcdOffFlg(&bOff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_bOff = bOff;

	// 内蔵トレースデータを保持しているか確認
	if(s_bGetTrcDataInram == FALSE){
		// まだ内蔵トレースデータを取得していない
		// FFWメモリ確保
		dwRdCount = TRC_BUF_MAX/4;
		s_pdwTrcDataInram = new DWORD [dwRdCount];
		memset(s_pdwTrcDataInram,0,TRC_BUF_MAX);
		// トレースメモリ取得
		ferr = GetAllTrcBuffDataTrbf(s_pdwTrcDataInram,dwRdCount);
		if(ferr != FFWERR_OK){
			delete [] s_pdwTrcDataInram;
			return ferr;
		}
		s_bGetTrcDataInram = TRUE;
	}

	// トレースサイクル解析
	analysisTrcCycleTrbf(s_pdwTrcDataInram, &dwCycle);

	if(dwCycle == 0) {
		// 取得サイクルなし
		pRcy->dwStartCyc = 0;
		pRcy->dwEndCyc = RCY_CYCLE_ALL_FF;
	} else {
		// 取得サイクルセット
		pRcy->dwStartCyc = 0;
		pRcy->dwEndCyc = dwCycle -1;
		// RCY完了
		s_bTracSetRCY = TRUE;
	}

	return FFWERR_OK;

}


//==============================================================================
/**
 * 内蔵トレースバッファに格納された有効サイクル数の解析
 * @param  pdwRd  内蔵トレースバッファポインタ
 * @param  dwCycle  トレースサイクル数
 * @retval なし
 */
//==============================================================================
static void analysisTrcCycleTrbf(DWORD* pdwRd, DWORD* dwCycle)
{
	DWORD	i,j;
	int		nCycCnt = 0;		// ｻｲｸﾙ数格納変数

	DWORD	dwTdv = 0;			// Header.TDV格納変数
	DWORD	dwMode = 0;			// Header.MODE格納変数
	DWORD	dwModeBef = 0;		// ひとつ前のｻｲｸﾙのHeader.MODE格納変数
	DWORD	dwHead = 0;			// Header格納変数
	BOOL	bBtr3Pkt = FALSE;	// BTR3処理フラグ
	WORD	wCmdInf = 0;

	// 取得データの解析
	// << 取得トレースヘッダデータの配置 >>
	//   <--------------------　　　---------------------->
	//  31                11     7     3   2   1   0
	//  ┌────────┬──┬──┬─┬─┬─┬─┐
	//  │                │    │    │  │  │  │  │
	//  └─┬──────┴┬─┴┬─┴┬┴┬┴┬┴┬┘
	//      │              │    │    │  │  │  └ [0]    TDV  : トレースデータValid 
	//      │              │    │    │  │  │                  1:データ有効     0:データ無効
	//      │              │    │    │  │  └── [1]    LOST : コマンド/データ
	//      │              │    │    │  │                      1:Lost発生       0:Lostしていない
	//      │              │    │    │  └──── [2]    MODE : スタックポインタ情報
	//      │              │    │    │                          1:割り込みスタックポインタ(ISP)
	//      │              │    │    │                          0:ユーザスタックポインタ(USP)
	//      │              │    │    └────── [3]    BTR  : 分岐可否成立情報
	//      │              │    │                                1:条件分岐成立   0:条件分岐不成立
	//      │              │    └───────── [4:7]  CMD1 : 識別子1情報
	//      │              └──────────── [11:8] CMD2 :イベント番号
	//      │                                                      4'h0 : イベントマッチなし
	//      │                                                      4'h1 : OAイベント0がマッチ
	//      │                                                      4'h2 : OAイベント1がマッチ
	//      │                                                      4'h3 : OAイベント2がマッチ
	//      │                                                      4'h4 : OAイベント3がマッチ
	//      └───────────―――────── [31:12] TS: タイムスタンプ情報
	//
	// サイクル数解析
	// ・TDV = 0の情報を削除
	// ・MODE情報を1サイクルとしてカウント

	j = 0;
	dwHead = pdwRd[0];
	dwMode = (dwHead & TRCRM_HDR_MODE)>>2;
	for (i = 0; i < (TRC_BUF_MAX>>4); i++) {
		// トレース情報種別を判定
		dwHead = pdwRd[j];
		j = j + 4;
		dwTdv = (dwHead & TRCRM_HDR_TDV);
		if(dwTdv == 1) {
			nCycCnt++;
			dwModeBef = dwMode;
			dwMode = (dwHead & TRCRM_HDR_MODE)>>2;
			wCmdInf = (WORD)((dwHead & TRCRM_HDR_CMD1)>>4);
			// スタック情報出力しないときは、解析をしない
			if( (dwMode != dwModeBef) && ( (s_dwTraceInfo & RM_SETINFO_TMO) == RM_SETINFO_TMO ) ){
				j = j - 4;				//もう一度同じバッファからデータを取る
				i --;
			} else if( (wCmdInf == BTR3) && (bBtr3Pkt == FALSE) ) {
				j = j - 4;				//もう一度同じバッファからデータを取る
				i --;
				bBtr3Pkt = TRUE;
			} else if( (wCmdInf == BTR3) && (bBtr3Pkt == TRUE) ) {
				bBtr3Pkt = FALSE;
			}
		}
	}

	// サイクル数渡し
	*dwCycle = nCycCnt;

}

//==============================================================================
/**
 * 内蔵トレースメモリに格納されたデータ解析
 * @param  pRd       トレースメモリに格納されたトレースデータの開始サイクル、
 *                    終了サイクルを格納するFFW_RCY_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getTrcDataTrbf(FFWRX_RD_DATA* pRd,int nStartOffset,int nEndOffset)
{
	FFWERR	ferr = FFWERR_OK;

	DWORD	dwReadCyc = 0;	// トレースデータ取得サイクル
	FFWRX_RCY_DATA pRcy;

	// 内蔵トレースデータを保持しているか確認
	if(s_bGetTrcDataInram == FALSE){
		ferr = getTraceCycleTrbf(&pRcy);
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}

	// 取得サイクル数計算 0からなので、+1が必要
	dwReadCyc = nEndOffset - nStartOffset + 1;
	// 対象データの解析処理
	analysisTrcDataTrbf(pRd, s_pdwTrcDataInram, dwReadCyc, nStartOffset);

	return ferr;

}

//==============================================================================
/**
 * 取得したトレースデータを解析する。
 * @param  pRd           取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wRd           取得したトレースデータ
 * @param  dwRdSize      取得したトレースサイズ
 * @param  nStartOffset  開始サイクルのオフセット値
 * @retval なし
 */
//==============================================================================
static void analysisTrcDataTrbf(FFWRX_RD_DATA* pRd, DWORD* dwRd, DWORD dwRdSize, int nStartOffset)
{
	int	i,j;					// i トレースサイクル数カウント　j 内蔵トレースポインタ位置
	WORD	wCmdInf = 0;		// HEADER.CMD1格納バッファ
	int		nCycCnt = -1;		// 解析をしているサイクル位置
	int		nAnaCnt = 0;		// 解析終了トレースサイクル数

	DWORD	dwTdv = 0;			// HEADER.TDV bit格納変数
	DWORD	dwLost = 0;			// HEADER.LOST bit格納変数
	DWORD	dwMode = 0;			// HEADER.MODE bit格納変数
	DWORD	dwModeBef = 0;		// 一つ前のサイクルのHEADER.MODE bit情報格納変数
	DWORD	dwCmd2 = 0;			// HEADER.CMD2(トリガ情報) 格納変数
	DWORD	dwHead = 0;			// HEADER格納変数
	DWORD	dwData1 = 0;		// データA格納変数
	DWORD	dwData2 = 0;		// データB格納変数
	DWORD	dwTime = 0;			// タイムスタンプ情報格納変数
	BOOL	bBtr3Pkt = FALSE;	// BTR3処理フラグ

	// 取得データの解析
	// << 取得トレースヘッダデータの配置 >>
	//   <--------------------　　　---------------------->
	//  31                11     7     3   2   1   0
	//  ┌────────┬──┬──┬─┬─┬─┬─┐
	//  │                │    │    │  │  │  │  │
	//  └─┬──────┴┬─┴┬─┴┬┴┬┴┬┴┬┘
	//      │              │    │    │  │  │  └ [0]    TDV  : トレースデータValid 
	//      │              │    │    │  │  │                  1:データ有効     0:データ無効
	//      │              │    │    │  │  └── [1]    LOST : コマンド/データ
	//      │              │    │    │  │                      1:Lost発生       0:Lostしていない
	//      │              │    │    │  └──── [2]    MODE : スタックポインタ情報
	//      │              │    │    │                          1:割り込みスタックポインタ(ISP)
	//      │              │    │    │                          0:ユーザスタックポインタ(USP)
	//      │              │    │    └────── [3]    BTR  : 分岐可否成立情報
	//      │              │    │                                1:条件分岐成立   0:条件分岐不成立
	//      │              │    └───────── [4:7]  CMD1 : 識別子1情報
	//      │              └──────────── [11:8] CMD2 :イベント番号
	//      │                                                      4'h0 : イベントマッチなし
	//      │                                                      4'h1 : OAイベント0がマッチ
	//      │                                                      4'h2 : OAイベント1がマッチ
	//      │                                                      4'h3 : OAイベント2がマッチ
	//      │                                                      4'h4 : OAイベント3がマッチ
	//      └───────────―――────── [31:12] TS: タイムスタンプ情報

	j = 0;
	nAnaCnt = dwRdSize + nStartOffset;
	// 0サイクル目にモード変更はないので、最初のモード情報は0サイクル目のモード情報を持っておく
	dwHead = dwRd[0];
	dwMode = (dwHead & TRCRM_HDR_MODE)>>2;
	for (i = 0; i < nAnaCnt; i++) {
		// トレース情報種別を判定
		dwHead = dwRd[j];
		dwData1 = dwRd[j+1];
		dwData2 = dwRd[j+2];
		dwTime = (dwHead & TRCRM_HDR_TIME)>>12;		// RevNo120410-001 Append Line
		j = j + 4;
		dwTdv = (dwHead & TRCRM_HDR_TDV);
		if(dwTdv == 1) {
			if (i >= nStartOffset) {
				nCycCnt++;
			}
			dwLost = (dwHead & TRCRM_HDR_LOST)>>1;
			dwModeBef = dwMode;
			dwMode = (dwHead & TRCRM_HDR_MODE)>>2;
			wCmdInf = (WORD)((dwHead & TRCRM_HDR_CMD1)>>4);
			dwCmd2 = (dwHead & TRCRM_HDR_CMD2)>>8;
			if(dwLost) {
				setTraceType(&pRd[nCycCnt], LOST);
				pRd[nCycCnt].lost.dwLost = FALSE;
			} else if( (dwMode != dwModeBef) && ( (s_dwTraceInfo & RM_SETINFO_TMO) == RM_SETINFO_TMO ) ) {	// スタック情報出力しないときは、解析をしない
				j = j - 4;				//もう一度同じバッファからデータを取る
				if (i >= nStartOffset) {
					setTraceType(&pRd[nCycCnt], MODE);
					pRd[nCycCnt].stac.dwStac = dwMode;
				}
			} else if ((wCmdInf == BTR3) && (bBtr3Pkt == FALSE) ) {	//BTR3パケット
				j = j - 4;				//もう一度同じバッファからデータを取る
				bBtr3Pkt = TRUE;
				if (i >= nStartOffset) {
					setTraceType(&pRd[nCycCnt], BTR3);
					pRd[nCycCnt].src.dwAddr = dwData2;
					pRd[nCycCnt].src.dwTime = dwTime;
					pRd[nCycCnt].src.bComplete = TRUE;
				}
			} else if ((wCmdInf == BTR3) && (bBtr3Pkt == TRUE) ) {	// BTR3パケット、解析でBTR2を出す
				bBtr3Pkt = FALSE;
				if (i >= nStartOffset) {
						setTraceType(&pRd[nCycCnt], BTR2);
						pRd[nCycCnt].dest.dwAddr = dwData1;
						pRd[nCycCnt].dest.dwTime = dwTime;
						pRd[nCycCnt].dest.bComplete = TRUE;
						bBtr3Pkt = FALSE; 
				}
			} else {
				if (i >= nStartOffset) {
					// RevNo120410-001 Delete タイムスタンプ情報の更新はここではやらない
					setTraceType(&pRd[nCycCnt], wCmdInf);
					switch (wCmdInf) {
					case BTR1:		// BTR1
						pRd[nCycCnt].bcnd.dwBcnd = ( (dwHead & TRCRM_HDR_BTR) >> 3) | (0x00000002);
						pRd[nCycCnt].bcnd.dwTime = dwTime;
						break;
					case BTR2:		// BTR2
						pRd[nCycCnt].dest.dwAddr = dwData1;
						pRd[nCycCnt].dest.dwTime = dwTime;
						pRd[nCycCnt].dest.bComplete = TRUE;
						break;
					case BTR3:		// BTR3 別処理
						break;
					case DTR1:		// DTR1
						pRd[nCycCnt].opr.dwAccess = 0;
						pRd[nCycCnt].opr.dwAccessSize = 1;
						pRd[nCycCnt].opr.dwEvNum = dwCmd2;
						pRd[nCycCnt].opr.dwAddr = dwData1;
						pRd[nCycCnt].opr.dwData = dwData2 & 0x000000FF;
						pRd[nCycCnt].opr.dwTime = dwTime;
						pRd[nCycCnt].opr.bComplete = 1;
						break;
					case DTR2:		// DTR2
						pRd[nCycCnt].opr.dwAccess = 0;
						pRd[nCycCnt].opr.dwAccessSize = 2;
						pRd[nCycCnt].opr.dwEvNum = dwCmd2;
						pRd[nCycCnt].opr.dwAddr = dwData1;
						pRd[nCycCnt].opr.dwData = dwData2 & 0x0000FFFF;
						pRd[nCycCnt].opr.dwTime = dwTime;
						pRd[nCycCnt].opr.bComplete = 1;
						break;
					case DTR4:		// DTR4
						pRd[nCycCnt].opr.dwAccess = 0;
						pRd[nCycCnt].opr.dwAccessSize = 4;
						pRd[nCycCnt].opr.dwEvNum = dwCmd2;
						pRd[nCycCnt].opr.dwAddr = dwData1;
						pRd[nCycCnt].opr.dwData = dwData2 & 0xFFFFFFFF;
						pRd[nCycCnt].opr.dwTime = dwTime;
						pRd[nCycCnt].opr.bComplete = 1;
						break;
					case DTW1:		// DTW1
						pRd[nCycCnt].opr.dwAccess = 1;
						pRd[nCycCnt].opr.dwAccessSize = 1;
						pRd[nCycCnt].opr.dwEvNum = dwCmd2;
						pRd[nCycCnt].opr.dwAddr = dwData1;
						pRd[nCycCnt].opr.dwData = dwData2 & 0x000000FF;
						pRd[nCycCnt].opr.dwTime = dwTime;
						pRd[nCycCnt].opr.bComplete = 1;
						break;
					case DTW2:		// DTW2
						pRd[nCycCnt].opr.dwAccess = 1;
						pRd[nCycCnt].opr.dwAccessSize = 2;
						pRd[nCycCnt].opr.dwEvNum = dwCmd2;
						pRd[nCycCnt].opr.dwAddr = dwData1;
						pRd[nCycCnt].opr.dwData = dwData2 & 0x0000FFFF;
						pRd[nCycCnt].opr.dwTime = dwTime;
						pRd[nCycCnt].opr.bComplete = 1;
						break;
					case DTW4:		// DTW4
						pRd[nCycCnt].opr.dwAccess = 1;
						pRd[nCycCnt].opr.dwAccessSize = 4;
						pRd[nCycCnt].opr.dwEvNum = dwCmd2;
						pRd[nCycCnt].opr.dwAddr = dwData1;
						pRd[nCycCnt].opr.dwData = dwData2 & 0xFFFFFFFF;
						pRd[nCycCnt].opr.dwTime = dwTime;
						pRd[nCycCnt].opr.bComplete = 1;
						break;
					default :
						break;
					}
				}
			}
		}
	}

	return;

}



// V.1.02 No.31 Class2 トレース対応 Modify Start
//==============================================================================
/**
 * 内蔵トレースメモリに格納されたサイクル数取得
 * @param  pRcy       トレースメモリに格納されたトレースデータの開始サイクル、
 *                    終了サイクルを格納するFFW_RCY_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getTraceCycleTrbfCls2(FFWRX_RCY_DATA *pRcy)
{
	FFWERR	ferr;

	DWORD	dwTBIP;
	DWORD	dwEndCyc,dwCyc;				// トレースサイクル解析結果格納
	BOOL	bOff;
	FFWRX_RM_DATA*		pRm;
	//RX220WS対応 Append Start
	DWORD	dwEndCycTmp;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	pMcuInfoData = GetMcuInfoDataRX();
	//RX220WS対応 Append Start

	pRm = GetRmDataInfo();

	// static変数の初期化
	s_bOff = FALSE;		// オーバーフローフラグクリア

	// OCDトレースバッファへのデータ入力停止
	ferr = SetTrcOcdStp(TRSTP_STOP_KEEP);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// OCDオーバーフローフラグ取得
	ferr = GetTrcOcdOffFlg(&bOff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_bOff = bOff;

	//トレースOCDメモリ入力ポインタ値取得
	ferr = GetTrcOcdMemInptPntCls2(&dwTBIP);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//RX220WS対応 Appned Start
	// 最大サイクル数取得
	if((pRm->dwSetInfo & RM_SETINFO_TPE) == 0x00000000){
			//タイムスタンプなし
		dwEndCycTmp = GetTrcCycMaxCls2();
	} else {
			//タイムスタンプあり
		dwEndCycTmp = GetTrcCycMaxCls2()/2;
	}
	//RX220WS対応 Append End

	//オーバーフロー発生の有無
	if(s_bOff == TRUE){
		//オーバーフロー発生
		//RX220WS対応 Modify Line
		dwEndCyc = dwEndCycTmp-1;
	} else {
		//オーバーフローなし
		// サイクル数の計算
		dwCyc = dwTBIP - OCD_BUF_RM_TRC0_H;
		//RX220WS対応 Modify Start
		if(dwEndCycTmp == 0){
			//トレースバッファがない
			dwEndCyc = RCY_CYCLE_NON;
		} else if(dwCyc == 0){
		//RX220WS対応 Modify End
			//サイクルがない
			dwEndCyc = RCY_CYCLE_NON;
		} else if((pRm->dwSetInfo & RM_SETINFO_TPE) == 0x00000000) {
			//タイムスタンプなし
			// RevNo121017-004 Modify Line
			dwEndCyc = (dwCyc -4)>>3;
			//RX220WS対応 Appned Start
			if((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_WS) == SPC_TRC_RX220_BUFF64_32CYC){
				if(dwEndCyc >= dwEndCycTmp){
					dwEndCyc = dwEndCycTmp -1;
				}
			}
			//RX220WS対応 Appned End
		} else {
			//タイムスタンプあり
			// RevNo121017-004 Modify Line
			dwEndCyc = (dwCyc -4)>>4;
			//RX220WS対応 Appned Start
			if((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_WS) == SPC_TRC_RX220_BUFF64_32CYC){
				if(dwEndCyc >= dwEndCycTmp){
					dwEndCyc = dwEndCycTmp -1;
				}
			}
			//RX220WS対応 Appned End
		}
	}

	pRcy->dwStartCyc = 0;
	pRcy->dwEndCyc = dwEndCyc;
	s_bTracSetRCY = TRUE;

	return FFWERR_OK;

}

//==============================================================================
/**
 * 内蔵トレースのサイクル数の最大値を返す
 * @param  なし
 * @retval 内蔵トレースの最大値を返す
 */
 //==============================================================================
// RX220 WS対応 Modify Start
DWORD GetTrcCycMaxCls2(void){
	
	DWORD dwMaxCyc;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報

	pMcuInfoData = GetMcuInfoDataRX();

	if((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_BUFFSIZE) == SPC_TRC_RX220_32CYC){
		dwMaxCyc = TRC_CYC_CLS2_CYC32;
	} else 	if((pMcuInfoData->dwSpc[0] & SPC_TRC_RX220_BUFFSIZE) == SPC_TRC_RX220_0CYC){
		dwMaxCyc = TRC_CYC_CLS2_CYC0;
	} else {
		dwMaxCyc = TRC_CYC_MAX_CLS2;
	}
	
	return dwMaxCyc;
}
// RX220 WS対応 Modify End
//==============================================================================
/**
 * 取得したトレースデータを解析する。
 * @param  pRd           取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wRd           取得したトレースデータ
 * @param  dwRdSize      取得したトレースサイズ
 * @param  nStartOffset  開始サイクルのオフセット値
 * @param  nEndOffset    終了サイクルのオフセット値
 * @retval なし
 */
//==============================================================================
static void analysisTrcDataTrbfCls2(FFWRX_RD_DATA* pRd, DWORD* pdwRd, DWORD dwStartCyc, DWORD dwRdSize)
{

	DWORD	dwAnaStartPt,dwAnaEndPt;
	DWORD	dwCnt = 2;
	DWORD	i,j;
	DWORD	dwHead = 0;
	DWORD	dwData = 0;
	DWORD	dwDataTs = 0;
	FFWRX_RM_DATA*		pRm;

	pRm = GetRmDataInfo();

	// 解析開始/終了位置の計算
	// RevNo110329-002 Modify Line
	if( ((pRm->dwSetInfo & RM_SETINFO_TPE) == RM_SETINFO_CLS2_TMSTMP) || ((pRm->dwSetInfo & RM_SETINFO_TPE) == RM_SETINFO_TPE)){
		// タイムスタンプありの場合
		// 解析開始位置
		dwAnaStartPt = dwStartCyc * 4;
		// カウント値
		dwCnt = 4;
		// 解析終了位置
		dwAnaEndPt = (dwRdSize + dwStartCyc) * 4;
	} else {
		// タイムスタンプなしの場合
		// 解析開始位置
		dwAnaStartPt = dwStartCyc * 2;
		// カウント値
		dwCnt = 2;
		// 解析終了位置
		dwAnaEndPt = (dwRdSize + dwStartCyc) * 2;
	}
	
	// 取得データの解析
	// << 取得トレースヘッダデータの配置 >>
	//   <--------------------　　　---------------------->
	//  31                       5     3   2   1   0
	//  ┌───────────┬──┬─┬───┬─┐
	//  │                      │    │  │      │  │
	//  └───────────┴┬─┴─┴┬──┴┬┘
	//                            │      　│      └ [0]    RW   : リードライト情報 
	//                            │      　│                      1:ライト     0:リード
	//                            │      　│
	//                            │      　│
	//                            │      　└──── [1:2]  ATTR : 属性情報
	//                            │                                CMD!=0の場合
	//                            │                                 00:リザーブ 01:8bitアクセス 10:16bitアクセス 11:32bitアクセス
	//                            │                                CMD==0の場合
	//                            │                                 00:無効 01:分岐元アドレス 10:分岐先アドレス 11:時間計測カウンタ値
	//                            └───────── [4:5]  CMD  : トレース情報種別
	//                                                              00: 分岐トレース情報および時間計測カウンタ値
	//                                                              01: OAトレース1ch
	//                                                              10: OAトレース2ch
	//                                                              11: OAトレース1,2ch同時成立(F/Wは1chで返す)

	for(i=0,j=dwAnaStartPt; j<dwAnaEndPt;j=j+dwCnt,i++){
		// ヘッダ情報取得
		dwHead = pdwRd[j];
		dwData = pdwRd[j+1];
		// タイムスタンプ情報
		dwDataTs = 0;
		// RevNo110329-002 Modify Line
	 	if( ((pRm->dwSetInfo & RM_SETINFO_TPE) == RM_SETINFO_CLS2_TMSTMP) || ((pRm->dwSetInfo & RM_SETINFO_TPE) == RM_SETINFO_TPE)){
			// タイムスタンプありの場合
			if((pdwRd[j+2] & TRCRM_HDR_CLS2_ATTR) == TRCRM_HDR_CLS2_ATTR_TIME) {
				dwDataTs = pdwRd[j+3];
			}
		}

		// トレースデータ解析

		if(dwHead == TRCRM_HDR_CLS2_BTR2){
			// 分岐先情報の場合
			setTraceType(&pRd[i],BTR2);
			pRd[i].dest.dwAddr = dwData;
			pRd[i].dest.dwTime = dwDataTs;
			pRd[i].dest.bComplete = TRUE;
		} else if( dwHead == TRCRM_HDR_CLS2_BTR3){
			// 分岐元情報の場合
			setTraceType(&pRd[i],BTR3);
			pRd[i].src.dwAddr = dwData;
			pRd[i].src.dwTime = dwDataTs;
			pRd[i].src.bComplete = TRUE;
		} else if( (dwHead & (TRCRM_HDR_CLS2_RW|TRCRM_HDR_CLS2_ATTR)) == TRCRM_HDR_CLS2_DTR1 ){
			// 8bitリード情報の場合
			setTrcRdOprCls2(&pRd[i],DTR1,dwHead,dwData,dwDataTs);
		} else if( (dwHead & (TRCRM_HDR_CLS2_RW|TRCRM_HDR_CLS2_ATTR)) == TRCRM_HDR_CLS2_DTR2 ){
			// 16bitリード情報の場合
			setTrcRdOprCls2(&pRd[i],DTR2,dwHead,dwData,dwDataTs);
		} else if( (dwHead & (TRCRM_HDR_CLS2_RW|TRCRM_HDR_CLS2_ATTR)) == TRCRM_HDR_CLS2_DTR4 ){
			// 32bitリード情報の場合
			setTrcRdOprCls2(&pRd[i],DTR4,dwHead,dwData,dwDataTs);
		} else if( (dwHead & (TRCRM_HDR_CLS2_RW|TRCRM_HDR_CLS2_ATTR)) == TRCRM_HDR_CLS2_DTW1 ){
			// 8bitライト情報の場合
			setTrcRdOprCls2(&pRd[i],DTW1,dwHead,dwData,dwDataTs);
		} else if( (dwHead & (TRCRM_HDR_CLS2_RW|TRCRM_HDR_CLS2_ATTR)) == TRCRM_HDR_CLS2_DTW2 ){
			// 16bitライト情報の場合
			setTrcRdOprCls2(&pRd[i],DTW2,dwHead,dwData,dwDataTs);
		} else if( (dwHead & (TRCRM_HDR_CLS2_RW|TRCRM_HDR_CLS2_ATTR)) == TRCRM_HDR_CLS2_DTW4 ){
			// 32bitライト情報の場合
			setTrcRdOprCls2(&pRd[i],DTW4,dwHead,dwData,dwDataTs);
		} else {
			// それ以外
			setTraceType(&pRd[i],STDBY);
		}
	}

	return;
}

//==============================================================================
/**
 * オペランドアクセスデータの解析
 * @param  pRd           取得したトレースデータを格納するFFWRX_RD_DATA 構造体の配列
 * @param  wTrcType      トレースタイプ
 * @param  dwHead        ヘッダ情報
 * @param  dwData        データ
 * @param  dwDataTs      タイムスタンプデータ
 * @retval なし
 */
//==============================================================================
static void setTrcRdOprCls2(FFWRX_RD_DATA* pRd, WORD wTrcType, DWORD dwHead, DWORD dwData, DWORD dwDataTs)
{

	FFWRX_EV_OPC*	EvOpc;

	// トレースタイプ
	setTraceType(pRd,wTrcType);
	// アクセスサイズ
	if((wTrcType == DTR1)||(wTrcType == DTW1)){
		pRd->opr.dwAccessSize = TRC_OPR_ACSSIZE_B;					// RevRxNo120606-005 Modify Line
	} else if((wTrcType == DTR2)||(wTrcType == DTW2)){
		pRd->opr.dwAccessSize = TRC_OPR_ACSSIZE_W;					// RevRxNo120606-005 Modify Line
	} else if((wTrcType == DTR4)||(wTrcType == DTW4)){
		pRd->opr.dwAccessSize = TRC_OPR_ACSSIZE_L;					// RevRxNo120606-005 Modify Line
	}
	// アクセス属性
	if((wTrcType == DTR1)||(wTrcType == DTR2)||(wTrcType == DTR4)){
		pRd->opr.dwAccess = TRC_OPR_ACS_R;							// RevRxNo120606-005 Modify Line
	} else if((wTrcType == DTW1)||(wTrcType == DTW2)||(wTrcType == DTW4)){
		pRd->opr.dwAccess = TRC_OPR_ACS_W;							// RevRxNo120606-005 Modify Line
	}
	// アクセスデータ
	pRd->opr.dwData = dwData;
	
	// アクセスイベント依存設定
	if((dwHead & TRCRM_HDR_CLS2_CMD) == TRCRM_HDR_CLS2_CH1){
		// イベント1が成立
		EvOpc = GetEvOpc(DE1);
		pRd->opr.dwAddr = EvOpc->dwAddrStart;
		pRd->opr.dwEvNum = TRC_OPR_EVNUM_DE1;				// RevRxNo120606-005 Modify Line
	} else if((dwHead & TRCRM_HDR_CLS2_CMD) == TRCRM_HDR_CLS2_CH2){
		// イベント2が成立
		EvOpc = GetEvOpc(DE2);
		pRd->opr.dwAddr = EvOpc->dwAddrStart;
		pRd->opr.dwEvNum = TRC_OPR_EVNUM_DE2;				// RevRxNo120606-005 Modify Line
	} else if((dwHead & TRCRM_HDR_CLS2_CMD) == (TRCRM_HDR_CLS2_CH1|TRCRM_HDR_CLS2_CH2)){
		// イベント1,2が同時成立の場合
		EvOpc = GetEvOpc(DE1);
		pRd->opr.dwAddr = EvOpc->dwAddrStart;
		pRd->opr.dwEvNum = TRC_OPR_EVNUM_DE1;				// RevRxNo120606-005 Modify Line
	}

	// タイムスタンプ情報
	pRd->opr.dwTime = dwDataTs;

	// トレースデータ取得完了
	pRd->opr.bComplete = TRUE;

	return;

}
// V.1.02 No.31 Class2 トレース対応 Modify End
//==============================================================================
/**
 * 内蔵トレースモードかどうかを取得する
 * @param  なし
 * @retval TRUE  内蔵トレースモード
 * @retval FALSE 外部トレースモード
 */
//==============================================================================
BOOL GetTrcInramMode(void)
{

	DWORD				dwMode;
	FFWE20_EINF_DATA	einfData;
	FFWRX_RM_DATA*		pRm;
	BOOL				bInramMode;	// RevRxE2LNo141104-001 Append Line

	pRm = GetRmDataInfo();

	// 現在のトレースモード取得
	dwMode = (pRm->dwSetMode & RM_SETMODE_MODE) >> 24;

	// エミュレータ情報取得
	getEinfData(&einfData);

	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続以外の場合
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
		bInramMode = TRUE;
	} else if ((dwMode >= MODE7) && (dwMode <= MODE9))  {
		bInramMode = TRUE;
	} else {
		bInramMode = FALSE;
	}
	// RevRxE2LNo141104-001 Modify End

	return bInramMode;	// RevRxE2LNo141104-001 Modify Line

}
//RX220WS対応 Appned Start
//==============================================================================
/**
 * トレースモードがFullトレースかどうかを取得する
 * @param  なし
 * @retval TRUE  内蔵トレースモード
 * @retval FALSE 外部トレースモード
 */
//==============================================================================
BOOL GetTrcFreeMode(void)
{

	DWORD				dwMode;
	FFWRX_RM_DATA*		pRm;

	pRm = GetRmDataInfo();

	// 現在のトレースモード取得
	dwMode = (pRm->dwSetMode & RM_SETMODE_MODE) >> 24;

	if (( dwMode == MODE5) || ( dwMode == MODE6) ||  ( dwMode == MODE9) )  {
		return TRUE;
	}
	return FALSE;

}
//RX220WS対応 Appned End
//==============================================================================
/**
 * トレース ブレーク時BTR1出力対策
 * BTR1パケットがトレースバッファにたまっているかもしれないので、掃きだし
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR setTrcDummyDataBTR1(void)
{

	FFWERR	ferr = FFWERR_OK;
	WORD wFuncs;
	BOOL	bTmful;
	BOOL	bTm2ful;
	BOOL	bTren;
	FFWRX_RM_DATA*		pRm;	// RevRxNo130301-001 Append Line

	pRm = GetRmDataInfo();	// RevRxNo130301-001 Append Line

	//E20 トレース状態の取得
	ferr = GetTrcFpgaStatInfo(&bTren,&wFuncs,&bTmful,&bTm2ful);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130301-001 Modify Start
	// 以下の場合は処理不要
	// (a)フルトレースモードでトレースフル時
	// (b)トレースクロック停止時
	// (c)トレースクロックリソース無効時
	if (((wFuncs == TRC_FPGA_FUNCS_FULL) && (bTmful == TRUE)) || 
			((pRm->dwSetInitial & RM_SETINITIAL_TRE) == RM_SETINITIAL_TRE_DIS) || 
			((pRm->dwSetInitial & RM_SETINITIAL_TRV) == RM_SETINITIAL_TRV_DIS)) {
		s_bSetDummyPktBtr1 = FALSE;
		return FFWERR_OK;
	}
	// RevRxNo130301-001 Modify End

	//OCDにBTR1吐き出し処理を実施させる
	ferr = SetTrcOCDDummyDataBTR1();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	s_bSetDummyPktBtr1 = TRUE;

	return ferr;
}

//==============================================================================
/**
 * 全ブロックの識別子1を取得する。
 * @param  pdwTrcCmdCntBuf 全ブロックの識別子1の数
 * @retval FFWエラーコード
 */
//==============================================================================
static FFWERR getTraceAllBlockCycleDiv(DWORD* pdwTrcCmdCntBuf)
{
	DWORD	dwTrcCmdCntBuf = 0;
	DWORD*	p_dwIndexNum;
	DWORD dwBlockEnd = 0;
	DWORD i;
	int nIndexCnt = 0;
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwSrcBlockEnd = 0;

	p_dwIndexNum = new DWORD [RD_TRCMEM_INDEX];
	memset(p_dwIndexNum, 0, (RD_TRCMEM_INDEX*4));

	dwBlockEnd = getBlockEnd();	// RevRxNo130408-001 Modify Line

	//メモリラップラウンドしていない場合は、終了ブロックまで検索
	if (s_dwRdIndexSBlkPos <= s_dwRdIndexEBlkPos) {
		dwSrcBlockEnd = s_dwRdIndexEBlkPos;
	} else {
		dwSrcBlockEnd = dwBlockEnd;
	}

	nIndexCnt = 0;
	// 全ブロックの識別子1の数と全サイクル数を取り出す。
	ferr = GetTrcFpgaAllBlockCycleIndex(dwBlockEnd,&dwTrcCmdCntBuf,p_dwIndexNum);
	if (ferr != FFWERR_OK) {
		delete [] p_dwIndexNum;
		return ferr;
	}

	// スタートブロックから順に、インデックス作成
	// スタートブロック～メモリの最後まで
	nIndexCnt = 0;
	for (i = s_dwRdIndexSBlkPos ; i <= dwSrcBlockEnd; i++) {
		if ((s_dwRdIndex[i].dwStartCycle == -1) && (s_dwRdIndex[i].dwEndCycle == -1) && ( i!=0 )){
			 s_dwRdIndex[i].dwStartCycle = s_dwRdIndex[i-1].dwEndCycle+1;
			 s_dwRdIndex[i].dwEndCycle = s_dwRdIndex[i].dwStartCycle + p_dwIndexNum[i]-1;	// 0からだから-1
		}
	}
	// スタートブロック～メモリの最後まで スタートが0のときとやらない
	if ( (s_dwRdIndexSBlkPos != 0) && (s_dwRdIndexSBlkPos > s_dwRdIndexEBlkPos) ){
		for (i = 0 ; i <= s_dwRdIndexEBlkPos; i++) {
			if ((s_dwRdIndex[i].dwStartCycle == -1) && (s_dwRdIndex[i].dwEndCycle == -1) && ( i!=0 )){
				 s_dwRdIndex[i].dwStartCycle = s_dwRdIndex[i-1].dwEndCycle+1;
				 s_dwRdIndex[i].dwEndCycle = s_dwRdIndex[i].dwStartCycle + p_dwIndexNum[i]-1;	// 0からだから-1
			} else if ((s_dwRdIndex[i].dwStartCycle == -1) && (s_dwRdIndex[i].dwEndCycle == -1) && ( i ==0 )) {
				 s_dwRdIndex[0].dwStartCycle = s_dwRdIndex[dwBlockEnd].dwEndCycle+1;
				 s_dwRdIndex[0].dwEndCycle = s_dwRdIndex[0].dwStartCycle + p_dwIndexNum[0]-1;	// 0からだから-1
			}
		}
	}

	delete [] p_dwIndexNum;

	*pdwTrcCmdCntBuf = dwTrcCmdCntBuf;

	return ferr;

}
//==============================================================================
/**
 * トレースクロック計算値計算
 * @param STATで取得した計算前のトレースクロック値
 * @retval トレースクロック値
 */
//==============================================================================
float GetTRCLKCalc(DWORD dwTrclk)
{
	float	fTRCLK = 0;

	if(dwTrclk != 0){
		fTRCLK = (float)((1/((dwTrclk*7.58) /65)) * 1000);
	} else {
		fTRCLK = 0;
	}
	s_fTrcClkClc = fTRCLK;
	return fTRCLK;
}

// V.1.02 No.31 Class2 トレース対応 Append Start
//==============================================================================
/**
 * RCYサイクル数取得
 * @param  rcy RCY取得サイクル数
 * @retval TRUE:RCY取得済み　FALSE:RCY取得未
 */
//==============================================================================
BOOL GetSetRcyFlg(FFWRX_RCY_DATA* rcy){
	memcpy(rcy,&s_rcyData,sizeof(FFWRX_RCY_DATA));
	return s_bTracSetRCY;
}
// V.1.02 No.31 Class2 トレース対応 Append End
// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append Start
//==============================================================================
/**
 * E20トレース停止処理を実施
 * @param bStopTrc:TRUE  トレースを停止した 
 *                 FALSE トレースを停止しなかった  
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR StopE20Trc(FFWRX_STAT_EML_DATA* pStatEML,BOOL* pbStopTrc){
	FFWERR						ferr = FFWERR_OK;
	FFWRX_TRG_DATA*				evTrg;
	FFWRX_COMB_TRC*				evTrc;
	DWORD						dwData;

	*pbStopTrc = FALSE;

	// RevRxE2LNo141104-001 Delete: エミュレータ判定処理を削除(GetTrcInramMode()で実施)。

	// 内蔵トレースモードの場合は、何もしない。
	if (GetTrcInramMode() == TRUE) {
		return ferr;
	}

	//すでに停止中の場合は何もしない
	if((pStatEML->dwTraceInfo & STAT_EML_TRCINFO_RUN) != STAT_EML_TRCINFO_RUN){
		return ferr;
	}

	//トレーストリガ情報取得	
	evTrg = GetEvTrgInfo();
	// トレース終了イベントのみの設定ではない場合は何もしない 0で有効
	if((evTrg->dwTrcTrg & (EVETTRG_TRC_START | EVETTRG_TRC_END)) != (~EVETTRG_TRC_END & (EVETTRG_TRC_START | EVETTRG_TRC_END))){
		return ferr;
	}

	//トリガ情報取得
	evTrc = GetEvCombTrcInfo();
	//終了イベントに実行PCイベントが設定してある場合
	if(evTrc->dwEndpe != 0){
		//PCイベント成立状態を取得
		ferr = GetEvExecPcCompInfo(&dwData);		
		if(ferr != FFWERR_OK){
			return ferr;
		}

		if((evTrc->dwEndpe & dwData) != 0){
			// RevNo110322-001	Modifiy Line
			//終了イベントが成立している
			ferr = DO_SetRXTRSTP(TRSTP_STOP_KEEP,FALSE);
			if(ferr != FFWERR_OK){
				return ferr;
			}
			*pbStopTrc = TRUE;
			return ferr;
		}
	}

	//終了イベントにオペランドイベントが設定してある場合
	if(evTrc->dwEndde != 0){
		//オペランドイベント成立状態を取得
		ferr = GetEvOpcCompInfo(&dwData);		
		if(ferr != FFWERR_OK){
			return ferr;
		}

		if((evTrc->dwEndde & dwData) != 0){
			// RevNo110322-001	Modifiy Line
			//終了イベントが成立している
			ferr = DO_SetRXTRSTP(TRSTP_STOP_KEEP,FALSE);
			if(ferr != FFWERR_OK){
				return ferr;
			}
			*pbStopTrc = TRUE;
			return ferr;
		}
	}

	return ferr;
}
// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append End
// V.1.02 覚書 No.27 トレースメモリクリア不具合対応 Append Start
//==============================================================================
/**
 * E20トレース クリアするメモリサイズ情報を更新
 * @param  なし
 * @retval なし
 */
//==============================================================================
void SetClrTrcMemSize(void)
{

	s_wUsedMemSize = s_wSetE20TrcMemSize;	// RevRxNo130408-001 Modify Line

	return ;
}
// V.1.02 覚書 No.27 トレースメモリクリア不具合対応 Append End
// V.1.02 RevNo110609-002 Appned Start
//==============================================================================
/**
 * 内蔵トレース FFWに取得した内蔵トレースメモリクリア
 * @param  なし
 * @retval なし
 */
//==============================================================================
void DeleteTrcDataInram(void){

	if(s_pdwTrcDataInram != NULL){
		if(s_bGetTrcDataInram == TRUE){
			delete [] s_pdwTrcDataInram;
		}
	}
	s_bGetTrcDataInram = FALSE;

}
// V.1.02 RevNo110609-002 Appned End

//==============================================================================
/**
 * OCDのトレースメモリオーバーフローフラグ取得
 * @param  なし
 * @retval TRUE:OCDトレースメモリオーバーフローあり　FALSE:なし
 */
//==============================================================================
BOOL GetTrcOffSwFlg(void){
	return s_bOff;
}
//=============================================================================
/**
 * トレース関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdRxData_Tra(void)
{

	s_rcyData.dwStartCyc = RCY_CYCLE_NON;
	s_rcyData.dwEndCyc = RCY_CYCLE_NON;
	s_bTracSetRCY = FALSE;
	s_nNonCycleCnt = 0;

	for (int i = 0; i < RD_TRCMEM_INDEX; i++) {
		s_dwRdIndex[i].dwStartCycle = RCY_CYCLE_NON;
		s_dwRdIndex[i].dwEndCycle = RCY_CYCLE_NON;
	}

	s_nPreTimeStamp = -1;
	s_nPreBrAddr = -1;
	s_nPreDtAddr = -1;
	s_dwTraceInfo = 0;
	s_nNonCycleCnt = 0;
	s_dwRdIndexSBlkPos = 0;
	s_bGetBrAddr = FALSE;
	s_bGetDtAddr = FALSE;
	s_bGetTimeStamp = FALSE;

	s_bSetDummyPkt = FALSE;
	s_bSetDummyPktBtr1 = FALSE;
	s_fTrcClkClc = 0;
	// Rev110303-001, RevRxNo130408-001 Modify Line
	s_wUsedMemSize = E20_TMSIZE_32MB;
	// RevNo110309-005 Append Line
	s_bUsedTrace = FALSE;
	s_dwTrcCmdCnt = RCY_CYCLE_NON;

	// V.1.02 No.31 Class2 トレース対応 Append Start
	// FFW内蔵トレース用トレースメモリクリア
	if(s_pdwTrcDataInram != NULL){
		if(s_bGetTrcDataInram == TRUE){
			delete [] s_pdwTrcDataInram;
		}
	}
	s_bGetTrcDataInram = FALSE;
	// V.1.02 No.31 Class2 トレース対応 Append End

	// V.1.02 No.36 タイムスタンプ解析修正 Append Start
	s_bE20TrcMemOvr = FALSE;
	s_wTmwaEnd = 0;
	// V.1.02 No.36 タイムスタンプ解析修正 Append End

	// RevRxNo130408-001 Append Start
	s_wSetE20TrcMemSize = E20_TMSIZE_32MB;
	s_wTrDataCmdInfStnby = TRCDATA_CMD_INF_STNBY_D4S1;
	s_wTrDataCmdInfIdnt1 = TRCDATA_CMD_INF_IDNT1_D4S1;
	s_wTrDataCmdInfIdnt2 = TRCDATA_CMD_INF_IDNT2_D4S1;
	s_wTrDataCmdInfIdnt3_1 = TRCDATA_CMD_INF_IDNT3_1_D4S1;
	s_wTrDataCmdInfIdnt3_2 = TRCDATA_CMD_INF_IDNT3_2_D4S1;
	s_wTrDataCmdInfData = TRCDATA_CMD_INF_DATA_D4S1;
	// RevRxNo130408-001 Append End

	return;

}