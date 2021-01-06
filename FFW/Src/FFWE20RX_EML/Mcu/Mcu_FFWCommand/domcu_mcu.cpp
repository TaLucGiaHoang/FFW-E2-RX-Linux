///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_mcu.cpp
 * @brief MCU依存コマンドの実装ファイル
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, H.Akashi, K.Uemori, S.Ueda, Y.Kawakami
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/05/11
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo120621-003 2012/07/02
  ・GetEndianType2() CS0CRレジスタDUMP時のアドレス指定変更
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・V.1.03 RevNo111121-008 2012/07/11 橋口 OFS1 VDSELチェック対応
・V.1.03 RevNo111121-006 2012/07/11 橋口 P/Eモード中のSTAT処理
・RevNo111121-005	2012/07/11 橋口
  ・IsMcuRomAddr(), IsNotMcuRomArea(), ChkBlockArea()
    ・ユーザブート領域の判定処理を行っている個所について、
      ユーザブート領域の開始／終了アドレスの指定が共に0 の場合、
      判定処理を行わないように変更。
・RevRxNo111121-002		2012/07/11 橋口
  ・DO_GetRXSTAT()
    ・ROM レスの場合、STATのエンディアン指定状況はPMOD で指定されたエンディアン固定と
	  するように変更。
・RevRxNo111128-001		2012/07/11 橋口
  ・DO_GetRXSTAT()
    ・ROM レスの場合、STATの動作モードを以下の内容で差し替えるように変更。
	    PMODで指定された"端子設定によるMCU動作モード"
・RevNo111128-001	2012/07/11 橋口
  ・DO_GetRXSTAT()
    ・内蔵ROM 無効拡張モードの場合、STATのエンディアン指定状況は
	  前回STAT実行時に取得したMDE レジスタ情報とするように変更。
・RevRxNo120606-004 2012/07/12 橋口
  ・MPU領域リード/ライト対応
・RevRxNo120606-009 2012/07/13 橋口
　・HotPlug起動でRX630のBIGエンディアン起動時のSTATエンディアン情報対応
・RevNo120727-001 2012/07/27 橋口
  内蔵ROM DUMP 高速化 
・RevRxNo120910-001	2012/10/15 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・DO_GetRXSTAT()での、引数のbyStatKindをdwStatKindに変更。
    DO_SetRXPMOD()での、GetStatData()関数呼び出し時の引数のbyStatKindをdwStatKindに変更。
  ・以下のstatic変数を削除。
　　s_dwRomNum、s_madrRomStart[MCU_AREANUM_MAX_RX]、s_madrRomEnd[MCU_AREANUM_MAX_RX]
　　InitFfwCmdMcuData_Mcu() で、s_dwRomNumの設定を削除。
  ・メンバ名 dwFcuFilmLen を dwFcuFirmLen に全て変更。
・RevRxNo120720-001	2012/10/09	橋口
  ・DO_GetRXSTAT()
    ・ROM レスの場合、STATの動作モードをPMOD指定値に差し替える処理をSTAT_KIND_SFR指定の
	  場合のみ実施するよう移動した。
・RevNo121017-004	2012/10/18 明石
  VS2008対応エラーコード　WARNING C4554対策
・RevRxNo121029-001	2012/10/31 明石
　Flashダウンロード時のWorkRamサイズを動的変化させる対応
・RevRxNo120910-005	2012/10/31 明石
　ユーザブートモード起動時のUSBブート領域非消去指定対応
・RevRxNo120910-010	2012/10/31 明石
　SDRAM領域判定処理改善
・RevRxNo120910-011	2012/10/31 明石
　FCUファームウェアがないMCUへの対応
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121206-001 2012/12/06 植盛
  ブートスワップ時のキャッシュ処理対応
・RevRxNo130411-001	2014/01/16 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130308-001 2013/08/21 上田 (2013/03/14 三宅担当分マージ)
　カバレッジ開発対応
・RevRxNo130301-001 2013/08/28 上田、植盛
	RX64M対応
・RevRxNo130830-001 2013/08/30 三宅
　　STATで、dwStatKindで有効でないパラメータへの設定があるのを改修
・RevRxNo130301-002 2013/11/18 上田
	RX64M MP対応
・RevRxNo130730-007 2013/11/18 大喜多
	GetStatData()の戻り値がFFWERR_OKの場合のみpeStatMcuを参照するようにする
・RevRxNo131101-005 2013/11/21 大喜多
    ChkBlockAreaの予約領域→有効領域完の判定処理修正
・RevRxNo140109-001 2014/01/17 上田
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo130411-001 2014/01/21 植盛
	enum FFWRXENM_MDE_TYPE型のメンバ名変更(RX_MDE_OSM→RX_MDE_FLASH)
・RevRxNo140616-001 2014/07/18 植盛、大喜多
	MCUコマンド個別制御領域に対する内部処理実装
・RevRxNo130730-001 2014/07/23 植盛、大喜多
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140515-006 2014/06/24 植盛
	RX231対応
・RevRxNo140617-001	2014/07/14 大喜多
	TrustedMemory機能対応
・RevRxNo140515-004 2014/07/08 川上
	FINE 切断処理失敗後の処理改善
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
・RevRxNo150827-002 2015/08/28 PA 紡車
	RX651オプション設定メモリアドレス、配置変更対応
・RevRxNo161003-001 2016/12/01 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxNo170511-001 2017/05/11 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応不具合修正
*/

#include "domcu_mcu.h"
#include "prot_common.h"
#include "protmcu_mcu.h"
#include "domcu_rst.h"
#include "domcu_mem.h"
#include "mcu_flash.h"
#include "mcu_extflash.h"
#include "domcu_prog.h"
#include "ffwmcu_dwn.h"
#include "domcu_dwn.h"
#include "dorx_tra.h"
#include "prot_sys.h"

#include "mcu_rst.h"
#include "mcuspec.h"
#include "instcode.h"
#include "mcu_sfr.h"
#include "mcu_extflash.h"
#include "mcu_extram.h"
#include "domcu_reg.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line
// RevRxNo130308-001 Append Line
#include "ffwrx_cv.h"
#include "ffwmcu_mcu.h"		// RevRxNo161003-001 Append Line

// 制御レジスタのアドレス範囲
static FFWRX_CTRLREG_DATA	s_dwRegAddr = {0x00084000, 0x00085FFF};

// V.1.02 RevNo110509-002 Append Line
static BOOL		s_bPmodFlag;					// PMODコマンド発行確認用( TRUE:発行済み、FALSE:未発行 )

// V.1.02 RevNo110407-001 Append Line
static BOOL		s_bHponOff;				// Hpon off中かどうかを示す( TRUE:Hpon off処理中、 FALSE:Hpon off処理中ではない )

// RevRxNo140617-001 Append Start
static BOOL s_bTMEnable;				// TrustedMemory機能が有効かどうかを示す(TRUE:有効、FALSE:無効)
static DWORD s_dwTMINFData;				// TrustedMemory識別データ情報格納変数(TMINFの値を格納)
// RevRxNo140617-001 Append End

// RevRxNo130730-001 Delete

// RevRxNo161003-001 Append start
static BOOL s_bTMEFCheck;				// TM機能有無チェック実施済みフラグ(TRUE：TM機能有無チェック実施済み、FALSE：TM機能有無チェック未実施)
static BOOL s_bTMEnableDual;			// FFEE0000h～FFEEFFFFhのTrustedMemory機能が有効かどうかを示す(TRUE:有効、FALSE:無効)
// RevRxNo161003-001 Append End


///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * MCU情報を設定する。
 * @param pMcuArea MCU空間情報格納構造体のアドレス
 * @param pMcuInfo エミュレータ制御に必要なMCU情報格納構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetRXMCU(const FFWRX_MCUAREA_DATA* pMcuArea, const FFWRX_MCUINFO_DATA* pMcuInfo)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	ProtInit();

	pDbgData = GetDbgDataRX();

	ferr = PROT_MCU_SetRXMCU(pMcuArea, pMcuInfo);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	ferr = ProtEnd();

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
	//キャッシュメモリ構造体の領域確保チェック/ 領域開放
// V.1.02 RevNo110506-001 Delete Line
// MCUコマンドでキャッシュメモリの解放を行うと、HEWデバッガの場合２回目のPMOD後にもMCUコマンドが
// 発行されるため、PMODで格納した最終ブロック情報が消えてしまう(メモリが解放されるため不定値になる)。
// キャッシュメモリの解放は、Hpon off時とDLLMainのdetach時に行うようにする。
// V.1.02 RevNo1105009-001 Append Line
// デバッガ起動処理の途中でエラーが出た後再起動する際、HEWデバッガの場合はHPON発行前であればDLLMainのDetachを
// 呼び出し、HPON発行後であればHpon offを実行するため、キャッシュメモリは必ず解放されるようになっている。
// しかし、3rdパーティの場合はどういう処理になっているか不明のため、再起動時にMCUファイル指定を変更(大きなMCU
// 領域のものに変更)した場合、キャッシュメモリは小さいサイズで確保されている状態のため、そのまま処理を進めて
// しまうとメモリが無いところにデータを書いてしまい予測できない問題が発生する可能性があるためキャッシュメモリの
// 解放処理を復活させることにした。
	DeleteFlashRomCacheMem();
	//キャッシュメモリ構造体領域確保
	NewFlashRomCacheMem(pMcuArea);

	//キャッシュメモリ管理フラグを初期化
	InitFlashRomCacheMemFlg();
	
	// RevRxNo121029-001	Append Line
	//	8.	Flashダウンロード時のWorkRam退避用キャッシュ領域解放
	DeleteEscWorkRamCacheMem();

	//Flashダウンロード時のWorkRamサイズを動的変化させる対応 Append Line
	NewEscWorkRamCacheMem(pMcuInfo->dwsizWorkRam);

// V.1.02 RevNo1105009-001 Append Start
// 最終ブロック情報をキャッシュメモリに格納しておく( PMOD発行済みの場合のみ )
// RX610/RX62Nの場合は以下の処理不要であるが、取得しておいても問題ないので取得しておくことにする。
	// V.1.02 RevNo110513-002 Append Start
	// ROMレス品の場合、以降の処理を実行すると暴走する(キャッシュ取得してないのにライトするから)ので
	// 以降の処理を行わないようにする。
	if( pMcuArea->dwFlashRomPatternNum == 0 ){	// 内蔵FlashROM領域パターン数が0の場合はROMレス品と判断
		return ferr;
	}
	// V.1.02 RevNo110513-002 Append End
	
	if( GetPmodInfo() ){		// 既にPMOD発行済み
		// リセットベクタを再取得するため、最終ブロックだけは起動時に実メモリを取得して入れておく必要有り
		enum FFWRXENM_PMODE			ePmode;				// 起動時に指定された端子設定起動モード
		ePmode = GetPmodeDataRX();
		ferr = GetFlashCacheRestVect(ePmode);
	}
// V.1.02 RevNo1105009-001 Append Start
	
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

	return ferr;
}

//=============================================================================
/**
 * BFWに設定されているMCU情報を取得する。
 * @param pMcuArea MCU空間情報格納構造体のアドレス
 * @param pMcuInfo エミュレータ制御に必要なMCU情報格納構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetRXMCU(FFWRX_MCUAREA_DATA* pMcuArea, FFWRX_MCUINFO_DATA* pMcuInfo)
{
	FFWERR	ferr = FFWERR_OK;

	ProtInit();

	ferr = PROT_MCU_GetRXMCU(pMcuArea,pMcuInfo);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtEnd();

	return ferr;
}
// 2008.9.2 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * デバッグ情報を設定する。
 * @param pDbg デバッグ情報格納構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_SetRXDBG(const FFWMCU_DBG_DATA_RX* pDbg)
{
	FFWERR	ferr;

	ProtInit();

	ferr = PROT_MCU_SetRXDBG(pDbg);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtEnd();

	return ferr;
}

//=============================================================================
/**
 * デバッグ情報を設定する。
 * @param pDbg デバッグ情報格納構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetRXDBG(FFWMCU_DBG_DATA_RX* pDbg)
{
	FFWERR	ferr;

	ProtInit();

	ferr = PROT_MCU_GetRXDBG(pDbg);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtEnd();

	return ferr;
}

// 2008.9.2 INSERT_END_E20RX600 }

//=============================================================================
/**
 * ターゲットMCU のステータス情報を取得する。
 * @param dwStatKind 取得対象の情報(SFR/JTAG/EML)を指定する
 * @param peStatMcu  ターゲットMCUステータス情報を格納する変数のアドレス
 * @param pStatSFR   SFR情報を格納する
 * @param pStatJTAG  JTAG情報を格納する
 * @param pStatFINE  FINE情報を格納する
 * @param pStatEML   EML情報を格納する
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
// RevRxNo120910-001 Modify Line
FFWERR DO_GetRXSTAT(DWORD dwStatKind, enum FFWRXENM_STAT_MCU* peStatMcu, FFWRX_STAT_SFR_DATA* pStatSFR, 
					FFWRX_STAT_JTAG_DATA* pStatJTAG, FFWRX_STAT_FINE_DATA* pStatFINE, FFWRX_STAT_EML_DATA* pStatEML)
{
	FFWERR	ferr;
	FFWERR			ferrEnd;
	// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append Line
	BOOL	bStopTrc;
// RevRxNo111121-002 Append Start
	FFWMCU_MCUAREA_DATA_RX*		pMcuArea;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
// RevRxNo111121-002 Append End

	enum FFWRXENM_PMODE			ePmode;		// RevRxNo111128-001 Append Line

	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	ProtInit();

	// RevRxNo120910-001 Modify Line
	ferr = GetStatData(dwStatKind, peStatMcu, pStatSFR, pStatJTAG, pStatFINE, pStatEML);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

// RevRxNo111121-002 Append Start
	pMcuArea = GetMcuAreaDataRX();
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// RevRxNo120910-001 Modify Line
	if ((dwStatKind & STAT_KIND_SFR) == STAT_KIND_SFR) {
			// SFR 情報を取得する場合

		// RevRxNo121026-001, RevRxNo130411-001, RevRxNo130301-001 Modify Line
		if ((pFwCtrl->eMdeType == RX_MDE_FLASH) || (pFwCtrl->eMdeType == RX_MDE_EXTRA)) {
				// MDEをフラッシュROM, Extra領域で設定するMCUの場合
			if (pMcuArea->dwFlashRomPatternNum == 0) {
					// プログラムROM 領域のパターン数が0 の場合

				// ROM レスの場合、PMOD で指定されたエンディアン固定とする
				pStatSFR->eEndian = (enum FFWRX_STAT_ENDIAN)GetEndianDataRX();
// RevNo111128-001 Append Start
			// RevRxNo120606-009 Appned Start
			} else if ((getHotPlugState() == TRUE) && (getIdCodeResultState() == TRUE)) {
				// ホットプラグ状態の場合、PMOD で指定されたエンディアン固定とする
				pStatSFR->eEndian = (enum FFWRX_STAT_ENDIAN)GetEndianDataRX();
			// RevRxNo120606-009 Appned End
			} else {
					// プログラムROM 領域のパターン数が0 でない場合

				if (pStatSFR->byStatProcMode == STAT_PROC_MODE_ROMD) {
						// 内蔵ROM 無効拡張モードの場合

					// 前回のMDE 値の内容でエンディアン情報とする
					pStatSFR->eEndian = (enum FFWRX_STAT_ENDIAN)GetMdeRegData();
				} else {
						// 内蔵ROM 無効拡張モードでない場合

					// 取得したMDE 値を退避する
					SetMdeRegData(pStatSFR->eEndian);
				}
			}
// RevNo111128-001 Append End
		}
		// RevRxNo120720-001 Append Start この処理をSTAT_KIND_SFR指定の場合のみ実施するように移動
		// RevRxNo111128-001 Append Start
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eMdeType == RX_MDE_FLASH) {		// MDEをフラッシュROMで設定するMCUの場合
			if (pMcuArea->dwFlashRomPatternNum == 0) {
					// プログラムROM 領域のパターン数が0 の場合

				// ROM レスの場合、PMOD で指定された「端子設定によるMCU動作モード」を
				// 設定する
				ePmode = GetPmodeDataRX();
				if (ePmode == RX_PMODE_SNG) {
						// シングルチップモードの場合
					pStatSFR->byStatMdPin = STAT_MD_PIN_SNG;
				} else if (ePmode == RX_PMODE_USRBOOT) {
					// ユーザブートモードの場合
					pStatSFR->byStatMdPin = STAT_MD_PIN_USRBOOT;
				}
			}
		}
	// RevRxNo111128-001 Append End
	// RevRxNo120720-001 Append End
	}
// RevRxNo111121-002 Append End

	// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append Start
	//dDMA情報とEML情報を取得する場合 トレース停止チェックを行う
	// RevRxNo120910-001 Modify Line
	// RevNo121017-004	Modify Line
	if((dwStatKind & (STAT_KIND_SFR|STAT_KIND_EML)) ==  (STAT_KIND_SFR|STAT_KIND_EML)){
		ferr=StopE20Trc(pStatEML,&bStopTrc);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		//トレースを停止した場合はトレース出力を停止にする。
		if(bStopTrc == TRUE){
			pStatEML->dwTraceInfo =  pStatEML->dwTraceInfo & ~STAT_EML_TRCINFO_TRCRUN;
		}
	}
	// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append End

	// RevRxNo130308-001 Append start
 	pDbgData = GetDbgDataRX();
	// RevRxNo130830-001 Modify Start
	if ((dwStatKind & STAT_KIND_EML) ==  STAT_KIND_EML) {	// 取得指定がEML情報を含む場合	// RevRxNo130308-001-036 Modify Line	// 位置を移動。
		if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
			if ((pStatEML->dwCoverageInfo & RX_STAT_CV_LOST) == RX_STAT_CV_LOST) {	// "ロスト発生"の場合
				SetCoverageLostFlag(TRUE);	// "カバレッジのLOSTが発生したことを示すフラグ"に"TRUE"設定。
			}
			if ((pStatEML->dwCoverageInfo & RX_STAT_CV_RESET) == RX_STAT_CV_RESET) {	// "リセット発生によるカバレッジ計測異常発生の可能性あり"の場合
				SetCoverageResetFlag(TRUE);	// "ユーザリセットが発生したことを示すフラグ"に"TRUE"設定。
			}
			if (GetCoverageLostFlag() == TRUE) {	// "カバレッジのLOSTが発生したことを示すフラグ"が立っている場合
				pStatEML->dwCoverageInfo = pStatEML->dwCoverageInfo | RX_STAT_CV_LOST;	// ビット1に、"1"をORする。
			}
			if (GetCoverageResetFlag() == TRUE) {	// "ユーザリセットが発生したことを示すフラグ"が立っている場合
				pStatEML->dwCoverageInfo = pStatEML->dwCoverageInfo | RX_STAT_CV_RESET;	// ビット2に、"1"をORする。
			}
		} else {										// "カバレッジ用に利用"以外の場合
			pStatEML->dwCoverageInfo = 0x00000000;
		}
	}
	// RevRxNo130830-001 Modify End
	// RevRxNo130308-001 Append end

	ferr = ProtEnd();

	return ferr;
}

// 2008.9.10 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * ホットプラグ機能の設定/解除
 * @param dwPlug ターゲットシステムとの接続/切断を指定
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_RXHPON(DWORD dwPlug)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr_tmp = FFWERR_OK;	//RevRxNo140515-004 Append Line
	FFWERR	ferr2 = FFWERR_OK;
	FFWERR	ferr3 = FFWERR_OK;	////RevRxNo140515-004 Append Line
	FFWERR	ferrEnd;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modidy Start
	FFWENM_ENDIAN				eMcuEndian;

	// RevNo110224-001 Append Start
	enum FFWRXENM_PMODE	ePmode;				// 起動時に指定された端子設定起動モード
	ePmode = GetPmodeDataRX();
	// RevNo110224-001 Append End
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	ProtInit();

	pMcuInfoData = GetMcuInfoDataRX();
	if (dwPlug == HPON_CON_DBG) {
		setFirstConnectInfo(TRUE);	// 接続完了を設定
		setIdCodeResultState(TRUE);	// ID認証結果設定状態を設定に変更
		ferrEnd = ProtEnd();
		return ferr;
	}

	if (dwPlug >= HPON_NCON_RESETKEEP) {
		//切断処理

		// RevNo110224-001 Modify Line
		// V.1.02 RevNo110323-006 Modify Line
		if( pMcuInfoData->byInterfaceType == IF_TYPE_1 ){	// FINEの場合(デバッグ継続モードで起動していてもいなくてもデバッグ継続解除する)

			// RevNo110224-001 Append Start
			ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
			//RevRxNo140515-004 Delete Line
			if (GetPmodeInRomDisFlg()) {	
				// 内蔵ROM無効の場合、OFSを書き換えるため、リセットをして、内蔵ROM無効状態を解除
				ferr_tmp = McuRest();	//RevRxNo140515-004 Modify Line
				//RevRxNo140515-004 Append Start
				if (ferr_tmp != FFWERR_OK) {
					ferr = ferr_tmp;
				}
				//RevRxNo140515-004 Append End
			//RevRxNo140515-004 Delete Line
			}
			// RevNo110224-001 Append End
			
			// デバッグ継続モードを解除処理

			// 設定されている動作エンディアンを取得
			eMcuEndian = GetEndianDataRX();

			// 対象ブロック情報取得
			// デバッグ継続モード解除設定(シングルチップモードに設定)
			// RevRxNo130301-001 Modify Start
			if (pFwCtrl->eOfsType == RX_OFS_FLASH) {	// OFSレジスタをフラッシュROMで設定するMCUの場合
				ClrFlashCacheOfsDbg2Sng(eMcuEndian);
			} else if (pFwCtrl->eOfsType == RX_OFS_EXTRA) {	// OFSレジスタをExtra領域で設定するMCUの場合
				ClrExtraCacheOfsDbg2Sng(eMcuEndian);
			}
			// RevRxNo130301-001 Modify End

			// V.1.02 RevNo110318-007 Append Start			
			// クロック切り替え許可不許可設定を許可にする
			// 切断処理のため、「許可」にした後、設定を元に戻さない。
			ferr_tmp = SetDbgClkChg(RX_CLKCHANGE_ENA);		//RevRxNo140515-004 Modify Line
			//RevRxNo140515-004 Append Start
			if (ferr_tmp != FFWERR_OK) {
				ferr = ferr_tmp;
			}
			//RevRxNo140515-004 Append End
			//RevRxNo140515-004 Delete Line
			// V.1.02 RevNo110318-007 Append End

			// V.1.02 RevNo110407-001 Append Line
			SetHponOffInfo(TRUE);		// HPON OFF処理中に設定

			if(ferr == FFWERR_OK){		//エラーが未発生の場合 RevRxNo140515-004 Append Line 
			// イレーズ・ライト処理実施
				ferr2 = WriteFlashExec(VERIFY_OFF);
				if (ferr2 != FFWERR_OK) {
					SetDwnpOpenData(FALSE);
				}
			}

			// V.1.02 RevNo110407-001 Append Line
			SetHponOffInfo(FALSE);		// HPON OFF処理中ではないに設定

			// ここでリセットを発行すると後のPROT_RXHPON発行でHPON処理がエラーとなるのでRESTコマンドは発行しない
			// BFW側HPON切断処理でリセットかけるのでここでは必要なし

		}
		setFirstConnectInfo(FALSE);		// 未接続状態にしておく( FINEでSetMCUが発行されるとSTATコマンドを発行してしまいぶっ飛ぶので )
		setIdCodeResultState(FALSE);	// ID認証結果設定状態を未設定に変更

		// キャッシュメモリ構造体の領域 確保状態確認/開放(ユーザー/データ/ユーザーブート)
		DeleteFlashRomCacheMem();
		
		// V.1.02 No.22 書き込みプログラムダウンロード化対応 Modidy Line
		// WTRプログラムロード用メモリの開放
		DeleteWtrLoadMem(WTRTYPE_0);

		// RevRxNo121029-001	Append Line
		//Flashダウンロード時のWorkRamサイズを動的変化させる対応 Append Line
		DeleteEscWorkRamCacheMem();

		// V.1.02 RevNo110609-002 Append Line
		// 内蔵トレース取得メモリをクリアしておく
		DeleteTrcDataInram();

		// RevNo110405-002 Append Line
		// プログラム停止状態にしておく
		SetMcuRunState(FALSE);

	}
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modidy End

	// V.1.02 No.4 ユーザブートモード起動対応 Append & Modify Start
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
		ePmode = RX_PMODE_SNG;				// PMODからの呼び出し以外はシングルチップモードの必要あり
	// RevRxNo130411-001 Modify Line
	} else {	// UBコードがないMCUの場合
		ePmode = GetPmodeDataRX();			// 起動時指定の起動モードでHPON処理実行する
	}
	// BFWのHPON処理実施
	ferr3 = PROT_RXHPON(dwPlug, ePmode);	// RevRxNo140515-004 Modify Line
	//RevRxNo140515-004 Append Start
	if (ferr3 != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr3;
	}
	//RevRxNo140515-004 Append End
	// V.1.02 No.4 ユーザブートモード起動対応 Append & Modify End
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	//Flashライトでエラーがあった場合はここでエラーを返す
	if (ferr2 != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr2;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
// 2008.9.10 INSERT_END_E20RX600 }


//=============================================================================
/**
 * 認証IDコードの設定
 * @param dwPlug IDコード(16バイトを指定)
 * @param pbyResult IDコード(16バイトを指定)
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
FFWERR DO_RXIDCODE(BYTE* pbyID, BYTE* pbyResult)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	BOOL	bHotPlugState = FALSE;

	ProtInit();

	ferr = PROT_RXIDCODE(pbyID, pbyResult);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	if (*pbyResult == IDCODE_RESULT_AUTH) {
		//認証OK
		setFirstConnectInfo(TRUE);	// 接続完了を設定
		// エンディアン状態を記憶(未リセット時に対応)
		ferr = SetEndianState();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}

		setIdCodeResultState(TRUE);	// ID認証結果設定状態を設定に変更

		//HotPlug接続時はFFW実行管理変数を実行中にする
		bHotPlugState = getHotPlugState();
		if(bHotPlugState == TRUE){
			SetMcuRunState(TRUE);
		}
	} else if(*pbyResult == IDCODE_RESULT_DEVICEID_ILLEGAL) {
		ferrEnd = ProtEnd();
		return FFWERR_ILLEGAL_DEVICEID;
	}else{
		ferrEnd = ProtEnd();
		return FFWERR_ILLEGAL_IDCODE;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;

}


//=============================================================================
/**
 * ホットプラグ接続時の接続前H/W処理
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_E20SetHotPlug(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	ferr = PROT_E20SetHotPlug();
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}


	return ferr;
}

// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append Start
//=============================================================================
/**
 * プロセッサモードの設定
 * @param ePmode プロセッサモード
 * @param eRmode メモリ空間拡張モード
 * @param eMcuEndian 起動エンディアン
 * @param pUbcode UBコード格納ポインタ
 * @param pUbAreaChange	:フラッシュROM上ユーザブート領域の書き換え情報へのポインタ
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
// RevRxNo120910-005	Modify Line
FFWERR DO_SetRXPMOD(enum FFWRXENM_PMODE ePmode, enum FFWRXENM_RMODE eRmode, enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA *pUbcode,
                    const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange )
{
	FFWERR						ferr = FFWERR_OK;
	FFWERR						ferrEnd;
	// V.1.02 MDE,UBコード書き換え対応 Append Line
	FFWERR						ferrWarning = FFWERR_OK;
	BOOL						bEndianChange;		// エンディアンの変更がいるかどうかを示す
	// V.1.02 新デバイス対応 Append Start
	BOOL						bUbcodeChange;		// UBコードの変更がいるかどうかを示す
	BOOL						bDebugCont;		// デバッグ継続モード設定必要かどうかを示す
	BOOL						bLvdChange;		// 電圧監視リセット無効設定が必要かどうかを示す	// RevRxNo130730-001 Append Line
	BOOL						bHotPlugState = FALSE;
	BOOL						bPmodCmd;			// PMODコマンドから実行するかを示す	// RevRxNo130730-001 Append Line
	// V.1.02 新デバイス対応 Append End
	// RevRxNo161003-001 Append Start
	BOOL						bBankmdChange;		// バンクモードの変更がいるかどうかを示す
	BOOL						bBankselChange;		// 起動バンクの変更がいるかどうかを示す
	enum FFWRXENM_BANKSEL		eBankSel;
	// RevRxNo161003-001 Append End
	// RevRxNo130301-001 Append Line
	BOOL						bExtraChange;		// Extra領域の変更が必要かを示す
	BOOL						bIdCodeResultState = FALSE;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	// V.1.02 WS2 ROMレス品対応 Append Line
	FFWMCU_MCUAREA_DATA_RX*		pMcuArea;			// ROMレス品判断のためMCU領域情報が必要
	FFWMCU_DBG_DATA_RX			*pDbgData;
	enum FFWRXENM_CLKCHANGE_ENA	eClkChangeEna;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line

	// RevNo110422-002 Append Start
	FFWMCU_REG_DATA_RX*	pRegDataRX;
	FFWMCU_REG_DATA_RX	RegData;
	// RevNo110422-002 Append End
	BOOL	bUbts;
		
	// V.1.03 RevNo111121-006 Append Start
	// RevRxNo120910-001 Modify Line
	DWORD						dwStatKind;
	enum FFWRXENM_STAT_MCU		eStatMcu;
	FFWRX_STAT_SFR_DATA			pStatSFR;
	FFWRX_STAT_JTAG_DATA		pStatJTAG;
	FFWRX_STAT_FINE_DATA		pStatFINE;
	FFWRX_STAT_EML_DATA			pStatEML;
	// V.1.03 RevNo111121-006 Append End

	// RevRxNo140617-001 Append Start
	MADDR madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域
	s_bTMEnable = FALSE;							// TM機能無効にしておく
	s_dwTMINFData = 0x00000000;						// TM識別データ値を0x0000000で初期化しておく
	// RevRxNo140617-001 Append End

	ProtInit();
	pMcuInfo = GetMcuInfoDataRX();
	pDbgData = GetDbgDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	// RevNo110422-002 Append Start
	pRegDataRX = &RegData;
	memset(pRegDataRX, 0, sizeof(FFWMCU_REG_DATA_RX));
	// RevNo110422-002 Append End

	// フラッシュ書き換え判断フラグ初期化
	bEndianChange = FALSE;				// エンディアン書き換えしない
	// RevRxNo161003-001 Append Start
	bBankmdChange = FALSE;				// バンクモード書き換えしない
	bBankselChange = FALSE;				// 起動バンク書き換えしない
	// RevRxNo161003-001 Append End
	bUbcodeChange = FALSE;				// UBコード書き換えしない
	bDebugCont = FALSE;					// デバッグ継続モード設定しない
	bLvdChange = FALSE;					// 電圧監視リセットを無効設定しない	// RevRxNo130730-001 Append Line
	bExtraChange = FALSE;				// Extra領域書き換えしない	// RevRxNo130301-001 Append Line
	bPmodCmd = TRUE;					// PMODコマンド発行			// RevRxNo130301-001 Append Line

	// V.1.02 RevNo110519-001 Modify Start
	// ID未認証時のみユーザ指定情報を保存していたが、RX610/RX62xの場合は1回目のPMODエンディアン設定値と
	// 2回目のPMODエンディアン設定値が違う場合があるため、FFWRXCmd_GetPMOD()を発行すると2回目に設定した
	// 本当のエンディアン情報と違う結果が返ってくることになる。
	// 動作上RX610/RX62xで1回目と2回目のエンディアンが違っていてもRX610/RX62xの場合PMODのエンディアン情報は
	// 使用しないので問題なし。
	// → PMOD発行の度に設定を保存するよう変更する。基本的に1回目と2回目のPMODは同じ値を設定すること
	//    (RX610/RX62xのエンディアン値以外)。1回目と2回目で設定値が違った場合は2回目の設定値でRX620/RX210は
	//    起動処理実行する。
	SetPmodeDataRX(ePmode);
	// V.1.02 No.4 ユーザブート/USBブート対応 Append Line
	SetRmodeDataRX(eRmode);
	SetEndianDataRX(eMcuEndian);
	SetUBCodeDataRX(pUbcode);
	// RevRxNo120910-005	Append Line
	SetUbAreaChangeRX(pUbAreaChange);		// FFWRX_UBAREA_CHANGEINFO_DATA*型のデータを管理変数に保管する。
	// V.1.02 RevNo110519-001 Modify End
	// RevRxNo161003-001 Append Start
	if (eRmode > RX_RMODE_ROMD) {			// eRmodeがRX_RMODE_ROMDより大きい場合はデュアルモードでの起動である。
		SetBankModeDataRX(RX_DUAL);
	} else {
		SetBankModeDataRX(RX_LINEAR);
	}
	// RevRxNo161003-001 Append End

	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得
	if( bIdCodeResultState == FALSE ){	// ID未認証
		ferrEnd = ProtEnd();
		// V.1.02 RevNo110509-001 Appned Line
		SetPmodInfo(TRUE);
		return FFWERR_OK;
	}

// RevNo111128-001 Append Start
	// MDE レジスタ値をPMOD で指定するエンディアンで初期化
	SetMdeRegData((enum FFWRX_STAT_ENDIAN)eMcuEndian);
// RevNo111128-001 Append End

	// RevRxNo161003-001 Append Start
	s_bTMEFCheck = FALSE;
	s_bTMEnableDual = FALSE;
	// RevRxNo161003-001 Append End

	// V.1.02 WS2 ROMレス品対応 Append Start
	// ROMレス品の場合、内蔵ROMがないのでエンディアン確認もフラッシュ書き換えもできないため
	// 何もせず終了するようにしないといけない。I/FはJTAGしかありえない。
	pMcuArea = GetMcuAreaDataRX();
	if( pMcuArea->dwFlashRomPatternNum == 0 ){	// 内蔵FlashROM領域パターン数が0の場合はROMレス品と判断
		// BFW STAT変数初期化のため、起動時に一度STATを実行しておく
		// V.1.03 RevNo111121-006 Append Start
		// RevRxNo120910-001 Modify Line
		dwStatKind = STAT_KIND_SFR;
		// RevRxNo120910-001 Modify Line
		ferr = GetStatData(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
		// V.1.03 RevNo111121-006 Append End
		ferrEnd = ProtEnd();
		return ferr;
	}
	// V.1.02 WS2 ROMレス品対応 Append End

	// V.1.02 新デバイス対応 Append Line
	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得

	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->bOsmExist == TRUE) {	// オプション設定メモリがあるMCUの場合
		// RevNo110406-001 Append Start
		// ホットプラグ起動かつROM無効モードの場合は、内蔵ROMのMDE,UBコードなどのチェックをしない
		// FINEの場合の仕様がよくわからないので、とりあえずJTAGだけ
		if((eRmode == RX_RMODE_ROMD) && (bHotPlugState == TRUE) && (pMcuInfo->byInterfaceType == IF_TYPE_0) ){
			ferrEnd = ProtEnd();
			return FFWERR_OK;
		}
		// RevNo110406-001 Append End


		// RevRxNo130301-001 Modify Start
		// オプション設定メモリの書き換え情報設定
		if (pFwCtrl->eFlashType != RX_FLASH_RV40F) {	// フラッシュがRV40F以外の場合
			// RevRxNo130730-001 Modify Line
			ferr = CheckOsm_RC03F_MF3(ePmode, eMcuEndian, pUbcode, pUbAreaChange, &bEndianChange, &bUbcodeChange, &bDebugCont, &bLvdChange, &ferrWarning, bPmodCmd);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

		} else {	// フラッシュがRV40Fの場合
			// RevRxNo130730-001 Modify Line	// RevRxNo161003-001 Modify Line
			ferr = CheckOsm_RV40F(ePmode, eMcuEndian, pUbcode, pUbAreaChange, &bEndianChange, &bBankmdChange, &bBankselChange, &bUbcodeChange, &bDebugCont, &bExtraChange, &bLvdChange, &ferrWarning, bPmodCmd);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
		// RevRxNo130301-001 Modify End

		// RevRxNo130301-001 Append Line
		ClrMdeUbcodeChaneFlg();		// ダウンロード時にMDE/UBコード書き換えが有ったかどうかを格納する変数を初期化
		// RevRxNo161003-001 Append Line
		ClrBankmdBankselChaneFlg();	// ダウンロード時にBANKMDビット、BANKSWPビット書き換えが有ったかどうかを格納する変数を初期化

		// RevRxNo130301-001 Modify Line	// RevRxNo161003-001 Modify Line
		if ((bEndianChange == TRUE) || (bBankmdChange == TRUE) || (bBankselChange == TRUE) || (bUbcodeChange == TRUE) || (bDebugCont == TRUE) || (bExtraChange == TRUE)) {
			// フラッシュ書き換えする場合

			// RevRxNo130301-001 Append Start
			if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
				ferr = GetFlashCacheRestVect(ePmode);
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
			}
			// RevRxNo130301-001 Append End

			// イレーズ・ライト処理実施
			//今のクロック切り替え情報保持
			eClkChangeEna = pDbgData->eClkChangeEna;
			// クロック切り替え許可不許可設定を許可にする
			ferr = SetDbgClkChg(RX_CLKCHANGE_ENA);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}

			ferr = WriteFlashExec(VERIFY_OFF);
			if (ferr != FFWERR_OK) {
				SetDwnpOpenData(FALSE);
				ferrEnd = ProtEnd();
				return ferr;
			}

			ClrFlashDataClearFlg();	// フラッシュROMブロック初期化情報を"未初期化"に設定

			// クロック切り替え許可不許可設定を元に戻す
			ferr = SetDbgClkChg(eClkChangeEna);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}

		// RevNo110401-002 Modfy Line
		if(bHotPlugState){
			//ホットプラグ時はリセットやHPON処理をしない
			;
		} else	{
			//リセットを入れる
			ferr = McuRest();	// コールドスタート&ユーザブートモードでHPON(接続)
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			
			//レジスタ値を初期化 リセットベクタを更新する
			ferr = ClrFwRegData(REGCLRCMD_PMOD);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if( ePmode == RX_PMODE_USRBOOT ){		// ユーザブート起動の場合
			// V.1.02 RevNo110414-002 Appned Start
			// ユーザブート起動 & JTAG の場合、MD端子設定が間違っているとシングルチップ起動してしまい
			// デバッグが正常にできなくなるため、ここで端子設定がユーザブート起動になっていることを確認する。
			// ユーザブート起動になっていない場合は認証切れエラーを返す。
				if( pMcuInfo->byInterfaceType == IF_TYPE_0 ){	// JTAGの場合のみ
					ferr = GetMcuUsrBootMode(&bUbts);
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}

					if(bUbts == FALSE){
						ferrEnd = ProtEnd();
						//ユーザーブート起動になっていない場合はエラーで返す
						return FFWERR_EL3_AMCU_AUTH_DIS;
					}
				}
			// V.1.02 RevNo110414-002 Appned End

			// V.1.02 RevNo110520-003 Apped Start
			// RX630/RX210のブートプログラムがFINEでユーザブート起動の場合だけプロテクトを解除しっぱなしにする不具合の対策
				if( pMcuInfo->byInterfaceType == IF_TYPE_1 ){	// FINEの場合のみ

					//プロテクトを設定
					ferr = SetMcuPrcrProtect(MCU_REG_SYSTEM_PRCR_ALL_PROTECT_SET);
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
				}
				// V.1.02 RevNo110520-003 Apped End

			} 
		}

		// V.1.02 RevNo110309-001 Append Start
		// リセットベクタを再取得するため、最終ブロックだけは起動時に実メモリを取得して入れておく必要有り
		// RevRxNo170511-001 Modify Start
		if (pFwCtrl->bBankmdExist) {	// BANKMDが存在するMCUの場合
			ferr = GetFlashCacheRestVectDual(ePmode);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		} else {
			ferr = GetFlashCacheRestVect(ePmode);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
		}
		// RevRxNo170511-001 Modify End
		// V.1.02 RevNo110309-001 Append End
	}


	// BFW STAT変数初期化のため、起動時に一度STATを実行しておく
	// V.1.03 RevNo111121-006 Append Start
	// RevRxNo120910-001 Modify Line
	dwStatKind = STAT_KIND_SFR;
	// RevRxNo120910-001 Modify Line
	ferr = GetStatData(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
	// V.1.03 RevNo111121-006 Append End
	// RevRxNo130730-007 Append Start
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxNo130730-007 Append End

	// RevRxNo121206-001 Append Start
	// RevRxNo130411-001 Modify Line, RevRxNo140515-006 Modify Line
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
		// フラッシュがMF3またはSC32用MF3の場合

		ferr = ChkBootSwapSizeInfo();			// ブートスワップサイズ情報を取得する。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxNo121206-001 Append End

	// RevRxNo140617-001 Append Start
	// TM機能有効無効確認
	if ((pMcuInfo->dwSpc[0] & SPC_TM_EXIST) == SPC_TM_EXIST) {		// TrustedMemory搭載品種
		// RevRxNo161003-001 Modify Start
		madrStartAddr = pMcuDef->madrTmefStartAddr;				// TMEFをリード	// RevRxNo150827-002 Modify Line
		eAccessSize = MLWORD_ACCESS;
		pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
		ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevRxNo161003-001 Modify End
		switch (dwReadData[0] & MCU_OSM_TMEF_MASK) {
			case MCU_OSM_TMEF_ENA1:
			case MCU_OSM_TMEF_ENA2:
			case MCU_OSM_TMEF_ENA3:
			case MCU_OSM_TMEF_ENA4:
				// RevRxNo161003-001 Append Start
				if (pFwCtrl->bBankmdExist) {				// BANKMDビットが存在するMCUの場合
					// レジスタ情報定義内容更新
					if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {	// リニアモード指定の場合
						pMcuInfo->dwMcuRegInfoBlkNum = pMcuInfo->dwMcuRegInfoBlkNum - 2;
					} else {								// デュアルモード指定の場合
						eBankSel = GetBankSelDataRX();		// 起動バンク情報取得
						switch (dwReadData[0] & MCU_OSM_TMEFDB_MASK) {
							case MCU_OSM_TMEFDB_BANK1TM_0:
							case MCU_OSM_TMEFDB_BANK1TM_1:
							case MCU_OSM_TMEFDB_BANK1TM_2:
							case MCU_OSM_TMEFDB_BANK1TM_4:	// FFEE0000h～FFEEFFFFhのTM機能有効
								break;					// 変更不要
							case MCU_OSM_TMEFDB_BANK1TM_3:
							case MCU_OSM_TMEFDB_BANK1TM_5:
							case MCU_OSM_TMEFDB_BANK1TM_6:
							case MCU_OSM_TMEFDB_BANK1TM_7:	// FFEE0000h～FFEEFFFFhのTM機能無効
								pMcuInfo->dwMcuRegInfoBlkNum = pMcuInfo->dwMcuRegInfoBlkNum - 2;
								break;
							default:	// ここには来ない
								;		// 何もしない
						}
					}
				}
				// RevRxNo161003-001 Append End
				s_bTMEnable = TRUE;							// TM機能有効に設定
				// TM識別データ値取得
				madrStartAddr = pMcuDef->madrTminfStartAddr;					// TMINFをリード	// RevRxNo150827-002 Modify Line
				eAccessSize = MLWORD_ACCESS;
				pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
				ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_dwTMINFData = dwReadData[0];				// TM識別データ値を退避しておく
				break;
			default:
				s_bTMEnable = FALSE;						// TM機能無効に設定
		}
		s_bTMEFCheck = TRUE;		// RevRxNo161003-001 Append Line
	}
	// RevRxNo140617-001 Append End

	if (ferr == FFWERR_OK) {
		ferr = ferrWarning;					// エラーが出ていない場合はワーニングを出す。
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	
	return ferr;
}
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * RC03F, MF3用オプション設定メモリ書き換え情報設定処理
 * @param ePmode PMODで指定された起動モード
 * @param eMcuEndian PMODで指定されたエンディアン
 * @param pUbcode PMODで指定されたUBコード格納ポインタ
 * @param pUbAreaChange	PMODで指定されたフラッシュROM上ユーザブート領域の書き換え情報へのポインタ
 * @param pbEndianChange MDEレジスタ書き換え要否格納変数のアドレス
 * @param pbUbcodeChange UBコード領域書き換え要否格納変数のアドレス
 * @param pbDebugCont デバッグ継続ビット書き換え要否格納変数のアドレス
 * @param pbLvdChange 電圧監視リセット書き換え要否格納変数のアドレス
 * @param pferrWarning Warning情報格納変数のアドレス
 * @param bPmodCmd PMODコマンドフラグ
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
// RevRxNo130730-001 Modify Line
FFWERR CheckOsm_RC03F_MF3(enum FFWRXENM_PMODE ePmode, enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA *pUbcode, 
						  const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange, BOOL* pbEndianChange, BOOL* pbUbcodeChange, 
						  BOOL* pbDebugCont, BOOL* pbLvdChange, FFWERR* pferrWarning, BOOL bPmodCmd)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrWarning = FFWERR_OK;
	BOOL	bEndianChange = FALSE;
	BOOL	bUbcodeChange = FALSE;
	BOOL	bDebugCont = FALSE;
	BOOL	bHotPlugState = FALSE;
	BOOL	bLvdChange = FALSE;				// RevRxNo130730-001 Append Line
	BYTE	byData;							// RevRxNo130730-001 Append Line
	DWORD	dwOfs1Val;						// RevRxNo130730-001 Append Line
	enum FFWENM_ENDIAN	eGetMcuEndian;		// 現在のエンディアン情報格納
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;

	pMcuInfo = GetMcuInfoDataRX();
	pFwCtrl = GetFwCtrlData();

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得

	// RevRxNo130411-001 Append Line
	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eMdeType == RX_MDE_FLASH) {	// MDEをフラッシュROMで設定するMCUの場合
		ferr = GetMcuEndian(&eGetMcuEndian);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// V.1.02 新デバイス対応 Modify Start
		if( eGetMcuEndian == ENDIAN_BIG ){		// bit0-2が000だったらビッグエンディアン
			if( eMcuEndian != eGetMcuEndian ){		// 指定エンディアンと実際のエンディアンが違う場合
				if( bHotPlugState ){				// ホットプラグ起動の場合はフラッシュ書き換えできないのでエラーとする
					return FFWERR_HOTPLUG_MDE_NOTMATCH;
				}
				// ワーニングを出すことを覚えておく
				bEndianChange = TRUE;
			}
		}else if( eGetMcuEndian == ENDIAN_LITTLE ){		// bit0-2が000だったらビッグエンディアン
			if( eMcuEndian != eGetMcuEndian ){		// 指定エンディアンと実際のエンディアンが違う場合
				if( bHotPlugState ){				// ホットプラグ起動の場合はフラッシュ書き換えできないのでエラーとする
					return FFWERR_HOTPLUG_MDE_NOTMATCH;
				}
				bEndianChange = TRUE;
			}
		}else{		// bit0-2が000でも111でもない場合はエンディアン書き換え必要
			bEndianChange = TRUE;
		}
		// V.1.02 新デバイス対応 Modify End
	}

	// V.1.02 新デバイス対応 Append Start
	if( ePmode == RX_PMODE_USRBOOT ){		// ユーザブート起動の場合
		// RevRxNo130730-001 Modify Start
		if (bPmodCmd == TRUE) {				// PMODコマンド発行時
			// RevRxNo130411-001 Append Line
			if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
				// 指定UBコードと実際のUBコードが違うかどうか確認 
				// RevRxNo120910-005	Modify Line
				ferr = CheckMemUbcode(pUbcode,&bUbcodeChange,pUbAreaChange);
				if (ferr != FFWERR_OK) {
					return ferr;
				}

				if((bUbcodeChange == TRUE) && (bHotPlugState == TRUE) ){				
					// UBコードが違っていても、ホットプラグ起動の場合はフラッシュ書き換えできないのでエラーとする
					return FFWERR_HOTPLUG_UBCODE_NOTMATCH;
				}
			}
		}
		// RevRxNo130730-001 Modify End
	}else{		// ユーザブート起動ではない(シングルチップ起動)場合
		if( pMcuInfo->byInterfaceType == IF_TYPE_1 ){	// FINEの場合のみ
			if( ePmode == RX_PMODE_USRBOOT ){			// ユーザブート起動の場合
				// RevRxNo130411-001 Append Line
				if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
					if( bHotPlugState ){					// ホットプラグ起動の場合はフラッシュ書き換えできないのでエラーとする
						return FFWERR_HOTPLUG_USERBOOT;
					}
				}
			}
			if( !bHotPlugState ){						// ホットプラグ起動以外の場合のみデバッグ継続モードを設定
				// RevRxNo130730-001 Append Start
				ferr = GetMcuOfs1(&dwOfs1Val);			// 現在のOFS1レジスタ設定値を取得
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				if ((dwOfs1Val & MCU_OSM_OFS1_OCD_MASK_FULL) != MCU_OSM_OFS1_OCDMOD_DEBUG_FULL) {
					// 現在のOFS1 bit25-24が10bではない場合(デバッグ継続モードではない)

					bDebugCont = TRUE;
				}
				// RevRxNo130730-001 Append Start
			}
		}
	}
	// V.1.02 新デバイス対応 Append End

	// RevRxNo130730-001 Append Start
	if (bPmodCmd == FALSE) {	// PMODコマンド発行時以外の場合
		if (pFwCtrl->eLvdErrType != RX_LVD_NON_CHK) {	// LVDxのエラーチェックを実施する場合
			ferr = GetMcuOfs1(&dwOfs1Val);			// 現在のOFS1レジスタ設定値を取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// 取得したOFS1レジスタ値の下位8bitを格納
			byData = static_cast<BYTE>(dwOfs1Val & MCU_OSM_OFS1_LVD_MASK_FULL);

			// 電圧監視リセット有効かを判定する
			ferr = CheckOfs1LvdDis(&bLvdChange, byData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// RevRxNo130730-001 Append End


	// V.1.02 新デバイス対応 Modify Start
	// フラッシュ書き換え準備			
	if( bEndianChange ){		// エンディアン書き換え必要な場合
		//内蔵Flashキャッシュのエンディアン情報を更新
		ferr = SetFlashCacheMdeEndian(ePmode,eMcuEndian);
		if(ferr != FFWERR_OK){
			return ferr;
		}
		// V.1.02 MDE,UBコード書き換え対応 Append Line
		// V.1.02 RevNo110228-003 Modify Line
		ferrWarning = FFWERR_ROM_MDE_CHANGE;		// MDEレジスタ書き換えワーニング格納
	}
	if( bUbcodeChange ){		// UBコード書き換え必要な場合		
		//内蔵FlashキャッシュのUBコード情報を更新
		ferr = SetFlashCacheUbcode(eMcuEndian,pUbcode);
		if(ferr != FFWERR_OK){
			return ferr;
		}
		// V.1.02 MDE,UBコード書き換え対応 Append Start
		// V.1.02 RevNo110228-003 Modify Start
		if( ferrWarning != FFWERR_ROM_MDE_CHANGE ){
			ferrWarning = FFWERR_ROM_UBCODE_CHANGE;		// UBコードレジスタ書き換えワーニング格納
		}else{
			ferrWarning = FFWERR_ROM_MDE_UBCODE_CHANGE;	// MDE/UBコードレジスタ書き換えワーニング格納
		}
		// V.1.02 RevNo110228-003 Modify End
		// V.1.02 MDE,UBコード書き換え対応 Append End
	}
	if( bDebugCont ){			// デバッグ継続モード設定が必要な場合
		//内蔵FlashキャッシュのOFSデバッグ継続モード情報を更新
		ferr = SetFlashCacheOfsDbg(eMcuEndian);
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}
	// V.1.02 新デバイス対応 Modify End

	// RevRxNo130730-001 Append Start
	if (bLvdChange == TRUE) {			// LVDxの設定が必要な場合
		ferr = SetFlashCacheOfsLvd(eMcuEndian);		// キャッシュメモリの更新
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		if (ferrWarning == FFWERR_ROM_MDE_CHANGE) {
			ferrWarning = FFWERR_ROM_MDE_OFS1_CHANGE;	// MDE/OFS1レジスタ書き換えワーニング格納
		} else {
			ferrWarning = FFWERR_ROM_OFS1_CHANGE;		// OFS1レジスタ書き換えワーニング格納
		}
	}

	// RevRxNo130730-001 Append End


	// MDE, UBコード、デバッグ継続モード、電圧監視リセットの変更要否フラグを設定
	*pbEndianChange = bEndianChange;
	*pbUbcodeChange = bUbcodeChange;
	*pbDebugCont = bDebugCont;
	*pferrWarning = ferrWarning;
	*pbLvdChange = bLvdChange;				// RevRxNo130730-001 Append Line

	return ferr;
}
// RevRxNo130301-001 Append End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * RV40F用オプション設定メモリ書き換え情報設定処理
 * @param ePmode PMODで指定された起動モード
 * @param eMcuEndian PMODで指定されたエンディアン
 * @param pUbcode PMODで指定されたUBコード格納ポインタ
 * @param pUbAreaChange	PMODで指定されたフラッシュROM上ユーザブート領域の書き換え情報へのポインタ
 * @param pbEndianChange MDEレジスタ書き換え要否格納変数のアドレス
 * @param pbBankmdChange MDEレジスタBANKMDビット書き換え要否格納変数のアドレス
 * @param pbBankselChange BANKSELレジスタ書き換え要否格納変数のアドレス
 * @param pbUbcodeChange UBコード領域書き換え要否格納変数のアドレス
 * @param pbDebugCont デバッグ継続ビット書き換え要否格納変数のアドレス
 * @param pbExtraChange Extra領域書き換え要否格納変数のアドレス
 * @param pbLvdChange 電圧監視リセット書き換え要否格納変数のアドレス
 * @param pferrWarning Warning情報格納変数のアドレス
 * @param bPmodCmd PMODコマンドフラグ
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
// RevRxNo130730-001 Modify Line	// RevRxNo161003-001 Modify Line
FFWERR CheckOsm_RV40F(enum FFWRXENM_PMODE ePmode, enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA *pUbcode, 
					  const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange, BOOL* pbEndianChange, BOOL* pbBankmdChange, BOOL* pbBankselChange, BOOL* pbUbcodeChange, 
					  BOOL* pbDebugCont, BOOL* pExtraChange, BOOL* pbLvdChange, FFWERR* pferrWarning, BOOL bPmodCmd)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrWarning = FFWERR_OK;
	BOOL	bEndianChange = FALSE;
	// RevRxNo161003-001 Append Start
	BOOL	bBankmdChange = FALSE;
	BOOL	bBankselChange = FALSE;
	BOOL	bBankselNotChange = FALSE;
	// RevRxNo161003-001 Append End
	BOOL	bUbcodeChange = FALSE;
	BOOL	bDebugCont = FALSE;
	BOOL	bExtraChange = FALSE;
	BOOL	bFmcdMdeChange = FALSE;	// RevRxNo130301-002 Append Line
	BOOL	bHotPlugState = FALSE;
	BOOL	bChange;
	BOOL	bLvdChange = FALSE;				// RevRxNo130730-001 Append Line
	BOOL	bDebugContinue = FALSE;			// RevRxNo130730-001 Append Line
	BYTE*	pbyExtraCacheMem;				// RevRxNo130730-001 Append Line
	BYTE	byData;							// RevRxNo130730-001 Append Line
	DWORD	dwCacheCnt;						// RevRxNo130730-001 Append Line
	FFWRX_FMCD_DATA*	pFmcd;
	DWORD	dwCnt;
	DWORD	dwSetData;
	DWORD	dwChkData;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	// RevRxNo161003-001 Append Start
	MADDR	madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*	pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD	dwReadData[1];		// 参照データ格納領域
	enum FFWRXENM_RMODE		eRmode;
	enum FFWRXENM_BANKSEL	eBankSel;
	BOOL	bFmcdBankmdChange = FALSE;
	BOOL	bFmcdBankselChange = FALSE;
	// RevRxNo161003-001 Append End

	pMcuInfo = GetMcuInfoDataRX();
	pFwCtrl = GetFwCtrlData();
	pFmcd = GetFmcdData();
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得

	// RevRxNo140109-001 Delete: InitExtraCacheMemFlg()

	// Extra領域の内容をキャッシュに設定
	ferr = SetExtraCache(eMcuEndian);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	eRmode = GetRmodeDataRX();		// RevRxNo161003-001 Append Line

	// RevRxNo161003-001 Append Start
	// TM機能有効チェック
	if ((pMcuInfo->dwSpc[0] & SPC_TM_EXIST) == SPC_TM_EXIST) {	// TrustedMemory搭載品種
		if (s_bTMEFCheck == FALSE) {							// TM機能有無チェック未実施の場合
			madrStartAddr = pMcuDef->madrTmefStartAddr;			// TMEFをリード
			eAccessSize = MLWORD_ACCESS;
			pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
			ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			switch (dwReadData[0] & MCU_OSM_TMEF_MASK) {
				case MCU_OSM_TMEF_ENA1:
				case MCU_OSM_TMEF_ENA2:
				case MCU_OSM_TMEF_ENA3:
				case MCU_OSM_TMEF_ENA4:
					s_bTMEnable = TRUE;							// TM機能有効に設定
					if (pFwCtrl->bBankselExist) {				// BANKSELレジスタが存在するMCUの場合
						switch (dwReadData[0] & MCU_OSM_TMEFDB_MASK) {
							case MCU_OSM_TMEFDB_BANK1TM_0:
							case MCU_OSM_TMEFDB_BANK1TM_1:
							case MCU_OSM_TMEFDB_BANK1TM_2:
							case MCU_OSM_TMEFDB_BANK1TM_4:	// FFEE0000h～FFEEFFFFhのTM機能有効
								s_bTMEnableDual = TRUE;	// FFEE0000h～FFEEFFFFhのTM機能有効に設定
								break;
							case MCU_OSM_TMEFDB_BANK1TM_3:
							case MCU_OSM_TMEFDB_BANK1TM_5:
							case MCU_OSM_TMEFDB_BANK1TM_6:
							case MCU_OSM_TMEFDB_BANK1TM_7:	// FFEE0000h～FFEEFFFFhのTM機能無効
								s_bTMEnableDual = FALSE;	// FFEE0000h～FFEEFFFFhのTM機能無効に設定
								break;
							default:	// ここには来ない
								;		// 何もしない
						}
					} else {
						s_bTMEnableDual = FALSE;	// FFEE0000h～FFEEFFFFhのTM機能無効に設定しておく
					}
					break;
				default:
					s_bTMEnable = FALSE;						// TM機能無効に設定
			}
			s_bTMEFCheck = TRUE;
		}
	}
	// RevRxNo161003-001 Append End

	// RevRxNo130301-002 Modify Start
	// キャッシュ内容のエンディアン情報確認、およびキャッシュ内容更新
	bEndianChange = CheckExtraCacheMdeEndian(eMcuEndian);

	if (bEndianChange == TRUE) {	// ホットプラグ起動の場合はフラッシュ書き換えできないのでエラーとする
		if (bHotPlugState == TRUE) {
			return FFWERR_HOTPLUG_MDE_NOTMATCH;
		}
	}

	// RevRxNo161003-001 Append Start
	// キャッシュ内容のバンクモード情報確認、およびキャッシュ内容更新
	ferr = CheckExtraCacheMdeBankmd(&bBankmdChange, TRUE);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (bBankmdChange) {	// バンクモード変更必要
		if (bHotPlugState) {
			return FFWERR_HOTPLUG_BANKMD_NOTMATCH;
		}
	}

	if (bPmodCmd == TRUE) {	// PMODコマンド発行時の場合
		if (eRmode >= RX_RMODE_SNG_DUAL_BANK0) {	// 起動バンク指定付モードの場合
			switch (eRmode) {
				case RX_RMODE_SNG_DUAL_BANK0:
				case RX_RMODE_ROME_DUAL_BANK0:
				case RX_RMODE_ROMD_DUAL_BANK0:
					eBankSel = RX_BANKSEL_BANK0;
					break;
				case RX_RMODE_SNG_DUAL_BANK1:
				case RX_RMODE_ROME_DUAL_BANK1:
				case RX_RMODE_ROMD_DUAL_BANK1:
					eBankSel = RX_BANKSEL_BANK1;
					break;
				case RX_RMODE_SNG_DUAL:
				case RX_RMODE_ROME_DUAL:
				case RX_RMODE_ROMD_DUAL:
					eBankSel = RX_BANKSEL_BANKCONT;
					break;
				default:	// ここには来ない
					eBankSel = RX_BANKSEL_BANK0;
			}
			// キャッシュ内容の起動バンク情報確認、およびキャッシュ内容更新
			ferr = CheckExtraCacheBanksel(eBankSel, &bBankselChange, &bBankselNotChange, TRUE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			if (bBankselChange) {	// 起動バンク変更必要
				if (bHotPlugState) {
					return FFWERR_HOTPLUG_BANKSWP_NOTMATCH;
				}
			}
			// s_eBankSelに起動バンク情報を設定するために、起動バンク継続の場合はキャッシュの起動バンク情報を取得しておく。
			if ((eRmode == RX_RMODE_SNG_DUAL) || (eRmode == RX_RMODE_ROME_DUAL) || (eRmode == RX_RMODE_ROMD_DUAL)) {
				dwCacheCnt = pMcuDef->dwBankselStartOffset - pMcuDef->dwExtraAStartOffset;	//キャッシュメモリ位置算出
				if (eMcuEndian == ENDIAN_BIG) {		// ビッグエンディアンの場合
					dwCacheCnt += 3;
				}
				pbyExtraCacheMem = GetExtraCacheAddr(dwCacheCnt);	// Extra領域用キャッシュメモリポインタを取得
				dwChkData = *pbyExtraCacheMem;	// キャッシュの起動バンク情報を設定
				switch (dwChkData & MCU_OSM_BANKSWP_MASK_DATA) {
					case MCU_OSM_BANKSWP_BANK1_DATA:
					case MCU_OSM_BANKSWP_CONT1_DATA:
					case MCU_OSM_BANKSWP_CONT2_DATA:
					case MCU_OSM_BANKSWP_CONT4_DATA:
						eBankSel = RX_BANKSEL_BANK1;
						break;
					case MCU_OSM_BANKSWP_BANK0_DATA:
					case MCU_OSM_BANKSWP_CONT3_DATA:
					case MCU_OSM_BANKSWP_CONT5_DATA:
					case MCU_OSM_BANKSWP_CONT6_DATA:
						eBankSel = RX_BANKSEL_BANK0;
						break;
					default:	// ここには来ない
						eBankSel = RX_BANKSEL_BANK0;
				}
			}				
			SetBankSelDataRX(eBankSel);		// 現在の起動バンク情報を設定
		}
	} else {	// PMODコマンド発行以外(DO_REST()/DO_SREST()からの発行)
		eBankSel = GetBankSelDataRX();		// 現在の起動バンク情報を取得
		// キャッシュ内容の起動バンク情報確認
		ferr = CheckExtraCacheBanksel(eBankSel, &bBankselChange, &bBankselNotChange, FALSE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo161003-001 Append End

	// FMCDコマンド設定データとキャッシュ内容の一致確認、およびキャッシュ内容更新
	bChange = FALSE;

	// RevRxNo130730-001 Append Start
	if (bPmodCmd == TRUE) {	// PMODコマンド発行時の場合
		// FMCDコマンド設定データチェック
		for (dwCnt = 0; dwCnt < pFmcd->dwSetNum; dwCnt++) {
			dwSetData = pFmcd->dwSetData[dwCnt];

			// 設定先がMDEレジスタの場合
			if (pFmcd->dwSetNo[dwCnt] == pMcuDef->dwMdeStartOffset) {	// RevRxNo150827-002 Modify Line
				// 設定データがPMODのエンディアンと一致しているか確認
				if (eMcuEndian == ENDIAN_LITTLE) {
					dwChkData = MCU_OSM_MDE_LITTLE_DATA;
				} else {
					dwChkData = MCU_OSM_MDE_BIG_DATA;
				}
				if ((dwSetData & MCU_OSM_MDE_MASK_DATA) != dwChkData) {
					bFmcdMdeChange = TRUE;	// FMCDで指定されたMDE値を差し替え
					dwSetData = (dwSetData & ~MCU_OSM_MDE_MASK_DATA) | dwChkData;	// 設定データのエンディアン情報を変更
				}
				// RevRxNo161003-001 Append Start
				if (pFwCtrl->bBankmdExist) {	// BANKMDビットありのMCU
					// 設定データがPMODのバンクモードと一致しているか確認
					if (eRmode >= RX_RMODE_SNG_DUAL_BANK0) {	// デュアルモードの場合
						dwChkData = MCU_OSM_BANKMD_DUAL_DATA;
					} else {
						dwChkData = MCU_OSM_BANKMD_LINEAR_DATA;
					}
					if ((dwSetData & MCU_OSM_BANKMD_MASK_DATA) != dwChkData) {
						bFmcdBankmdChange = TRUE;	// FMCDで指定されたBANKMD値を差し替え
						dwSetData = (dwSetData & ~MCU_OSM_BANKMD_MASK_DATA) | dwChkData;	// 設定データのバンクモード情報を変更
					}
				}
				// RevRxNo161003-001 Append End
			}

			// RevRxNo161003-001 Append Start
			// 設定先がBANKSELレジスタの場合
			if (pFmcd->dwSetNo[dwCnt] == pMcuDef->dwBankselStartOffset) {
				if (pFwCtrl->bBankselExist) {	// BANKSELレジスタありのMCU
					if (eRmode >= RX_RMODE_SNG_DUAL_BANK0) {	// 起動バンク指定ありの場合
						// 設定データがPMOD指定の起動バンクと一致しているか確認
						switch (eRmode) {
							case RX_RMODE_SNG_DUAL_BANK0:
							case RX_RMODE_ROME_DUAL_BANK0:
							case RX_RMODE_ROMD_DUAL_BANK0:
								dwChkData = MCU_OSM_BANKSWP_BANK0_DATA;
								break;
							case RX_RMODE_SNG_DUAL_BANK1:
							case RX_RMODE_ROME_DUAL_BANK1:
							case RX_RMODE_ROMD_DUAL_BANK1:
								dwChkData = MCU_OSM_BANKSWP_BANK1_DATA;
								break;
							case RX_RMODE_SNG_DUAL:
							case RX_RMODE_ROME_DUAL:
							case RX_RMODE_ROMD_DUAL:
								dwCacheCnt = pMcuDef->dwBankselStartOffset - pMcuDef->dwExtraAStartOffset;	//キャッシュメモリ位置算出
								if (eMcuEndian == ENDIAN_BIG) {		// ビッグエンディアンの場合
									dwCacheCnt += 3;
								}
								// Extra領域用キャッシュメモリポインタを取得
								pbyExtraCacheMem = GetExtraCacheAddr(dwCacheCnt);
								dwChkData = *pbyExtraCacheMem;	// キャッシュの起動バンク情報を設定
								break;
							default:
								dwChkData = MCU_OSM_BANKSWP_BANK0_DATA;
						}
						if ((dwSetData & MCU_OSM_BANKSWP_MASK_DATA) != dwChkData) {
							bFmcdBankselChange = TRUE;	// FMCDで指定されたBANKSEL値を差し替え
							dwSetData = (dwSetData & ~MCU_OSM_BANKSWP_MASK_DATA) | dwChkData;	// 設定データの起動バンク情報を変更
						}
					}
				}
			}
			// RevRxNo161003-001 Append End

			// キャッシュ(現在のExtraA領域データ)とFMCDコマンド設定データが一致しているかを確認
			bChange = CheckExtraCacheData(eMcuEndian, pFmcd->dwSetNo[dwCnt], dwSetData);
			if (bChange == TRUE) {
				bExtraChange = TRUE;
			}
		}
		// RevRxNo130301-002 Modify End
	}
	// RevRxNo130730-001 Append End

	// UBコード領域の書き換え要否確認
	if (ePmode == RX_PMODE_USRBOOT) {		// ユーザブート起動の場合
		// RevRxNo130730-001 Modify Start
		if (bPmodCmd == TRUE) {				// PMODコマンド発行時
			// 指定UBコードと実際のUBコードが違うかどうか確認 
			ferr = CheckMemUbcode(pUbcode, &bUbcodeChange, pUbAreaChange);
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			if (bUbcodeChange == TRUE) {	// UBコード書き換えが必要な場合
				if (bHotPlugState == TRUE) {
					// UBコードが違っていても、ホットプラグ起動の場合はフラッシュ書き換えできないのでエラーとする
					return FFWERR_HOTPLUG_UBCODE_NOTMATCH;

				} else {
					//内蔵FlashキャッシュのUBコード情報を更新
					ferr = SetFlashCacheUbcode(eMcuEndian, pUbcode);
					if(ferr != FFWERR_OK){
						return ferr;
					}
				}
			}
		}
		// RevRxNo130730-001 Modify End
	// FINE時のデバッグ継続ビット設定要否確認
	} else {		// ユーザブート起動ではない(シングルチップ起動)場合
		if (pMcuInfo->byInterfaceType == IF_TYPE_1) {	// FINEの場合のみ
			if (ePmode == RX_PMODE_USRBOOT) {			// ユーザブート起動の場合
				if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
					if (bHotPlugState == TRUE) {			// ホットプラグ起動の場合はフラッシュ書き換えできないのでエラーとする
						return FFWERR_HOTPLUG_USERBOOT;
					}
				}
			}
			if (bHotPlugState == FALSE) {				// ホットプラグ起動以外の場合のみデバッグ継続モードを設定
				// RevRxNo130730-001 Append Start
				// キャッシュのOFS1値を確認し、デバッグ継続モードが無効であれば、有効に設定
				bDebugContinue = CheckExtraCacheOfsDbg(eMcuEndian);		// RevRxNo130730-001 Modify Line
				if (bDebugContinue == TRUE) {
					bDebugCont = TRUE;
				}
				// RevRxNo130730-001 Append End
			}
		}
	}

	// RevRxNo130730-001 Append Start
	if (bPmodCmd == FALSE) {	// PMODコマンド発行時以外の場合
		if (pFwCtrl->eLvdErrType != RX_LVD_NON_CHK) {	// LVDxのエラーチェックを実施する場合
			// OFS1レジスタの下位8bitデータに相当するキャッシュデータ位置の算出
			if (eMcuEndian == ENDIAN_LITTLE) {	// リトルエンディアン時
				dwCacheCnt = pMcuDef->dwOfs1StartOffset - pMcuDef->dwExtraAStartOffset;	// RevRxNo150827-002 Modify Line
			} else {	// ビッグエンディアン時
				dwCacheCnt = pMcuDef->dwOfs1StartOffset - pMcuDef->dwExtraAStartOffset + 3;	// RevRxNo150827-002 Modify Line
			}

			// Extra領域用キャッシュメモリポインタを取得
			pbyExtraCacheMem = GetExtraCacheAddr(dwCacheCnt);

			// OFS1レジスタの下位8bitデータ取得
			byData = *pbyExtraCacheMem;

			// 電圧監視リセット有効かを判定する
			ferr = CheckOfs1LvdDis(&bLvdChange, byData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			if (bLvdChange == TRUE) {			// LVDxの設定が必要な場合
				SetExtraCacheOfsLvd(eMcuEndian);		// キャッシュメモリの更新
			}
		}
	}
	// RevRxNo130730-001 Append End

	// Warning情報設定
	// RevRxNo161003-001 Modify Start
	if (bEndianChange && (bBankmdChange == FALSE) && (bBankselChange == FALSE)) {			// エンディアン書き換えのみ必要な場合
		ferrWarning = FFWERR_ROM_MDE_CHANGE;						// MDEビット書き換えワーニング格納
	} else if((bEndianChange == FALSE) && bBankmdChange && (bBankselChange == FALSE)) {		// バンクモード書き換えのみ必要な場合
		ferrWarning = FFWERR_ROM_BANKMD_CHANGE;						// BANKMDビット書き換えワーニング格納
	} else if((bEndianChange == FALSE) && (bBankmdChange == FALSE) && bBankselChange) {		// 起動バンク書き換えのみ必要な場合
		if (bPmodCmd == FALSE) {	// PMODコマンド発行時以外(リセット/システムリセットからの発行)の場合
			// 何もしない(BANKSWP書き換えワーニングは出さないため)
		} else {	
			ferrWarning = FFWERR_ROM_BANKSWP_CHANGE;				// BANKSWPビット書き換えワーニング格納
		}
	} else if(bEndianChange && bBankmdChange && (bBankselChange == FALSE)) {				// エンディアン、バンクモード書き換えのみ必要な場合
		ferrWarning = FFWERR_ROM_MDE_BANKMD_CHANGE;					// MDEビット、BANKMDビット書き換えワーニング格納
	} else if(bEndianChange && (bBankmdChange == FALSE) && bBankselChange) {				// エンディアン、起動バンク書き換えのみ必要な場合
		if (bPmodCmd == FALSE) {	// PMODコマンド発行時以外(リセット/システムリセットからの発行)の場合
			ferrWarning = FFWERR_ROM_MDE_CHANGE;					// MDEビット書き換えワーニング格納
		} else {	
			ferrWarning = FFWERR_ROM_MDE_BANKSWP_CHANGE;			// MDEビット、BANKSWPビット書き換えワーニング格納
		}
	} else if((bEndianChange == FALSE) && bBankmdChange && bBankselChange) {				// バンクモード、起動バンク書き換えのみ必要な場合
		if (bPmodCmd == FALSE) {	// PMODコマンド発行時以外(リセット/システムリセットからの発行)の場合
			ferrWarning = FFWERR_ROM_BANKMD_CHANGE;					// BANKMDビット書き換えワーニング格納
		} else {	
			ferrWarning = FFWERR_ROM_BANKMD_BANKSWP_CHANGE;			// BANKMDビット、BANKSWPビット書き換えワーニング格納
		}
	} else if(bEndianChange && bBankmdChange && bBankselChange) {							// エンディアン、バンクモード、起動バンク書き換え必要な場合
		if (bPmodCmd == FALSE) {	// PMODコマンド発行時以外(リセット/システムリセットからの発行)の場合
			ferrWarning = FFWERR_ROM_MDE_BANKMD_CHANGE;				// MDEビット、BANKMDビット書き換えワーニング格納
		} else {	
			ferrWarning = FFWERR_ROM_MDE_BANKMD_BANKSWP_CHANGE;		// MDEビット、BANKMDビット、BANKSWPビット書き換えワーニング格納
		}
	}
	// RevRxNo161003-001 Modify End

	if (bUbcodeChange == TRUE) {		// UBコード書き換え必要な場合
		if (ferrWarning != FFWERR_ROM_MDE_CHANGE) {
			ferrWarning = FFWERR_ROM_UBCODE_CHANGE;		// UBコードレジスタ書き換えワーニング格納
		} else {
			ferrWarning = FFWERR_ROM_MDE_UBCODE_CHANGE;	// MDE/UBコードレジスタ書き換えワーニング格納
		}
	}

	// RevRxNo130301-002 Append Start
	if (bHotPlugState == TRUE) {
		if (bExtraChange == TRUE) {
			// ホットプラグ起動の場合はフラッシュ書き換えできないため、書き換えずにWarningを返送する
			ferrWarning = FFWERR_HOTPLUG_OSM_NOT_CHANGE;
			bExtraChange = FALSE;
		}
	}

	if (ferrWarning == FFWERR_OK) {
		// RevRxNo161003-001 Modify Start
		// FMCDコマンドWarning情報設定
		if (bFmcdMdeChange || bFmcdBankmdChange || bFmcdBankselChange) {
			ferrWarning = FFWERR_WRITE_MDE_BANKSEL_CHANGE;
		}
		// RevRxNo161003-001 Modify End
	}
	// RevRxNo130301-002 Append End

	// RevRxNo130730-001 Append Start
	if (bLvdChange == TRUE) {			// LVDxの設定が必要な場合
		// RevRxNo161003-001 Modify Start
		if (ferrWarning == FFWERR_ROM_MDE_CHANGE) {
			ferrWarning = FFWERR_ROM_MDE_OFS1_CHANGE;	// MDE/OFS1レジスタ書き換えワーニング格納
		} else if (ferrWarning == FFWERR_ROM_BANKMD_CHANGE) {
			ferrWarning = FFWERR_ROM_BANKMD_OFS1_CHANGE;	// BANKMD/OFS1レジスタ書き換えワーニング格納
		} else if (ferrWarning == FFWERR_ROM_MDE_BANKMD_CHANGE) {
			ferrWarning = FFWERR_ROM_MDE_BANKMD_OFS1_CHANGE;	// MDE/BANKMD/OFS1レジスタ書き換えワーニング格納
		} else {
			ferrWarning = FFWERR_ROM_OFS1_CHANGE;		// OFS1レジスタ書き換えワーニング格納
		}
		// RevRxNo161003-001 Modify End
	}
	// RevRxNo130730-001 Append End

	// MDE, UBコード、デバッグ継続モードの変更要否フラグを設定
	*pbEndianChange = bEndianChange;
	*pbBankmdChange = bBankmdChange;		// RevRxNo161003-001 Append Line
	*pbBankselChange = bBankselChange;		// RevRxNo161003-001 Append Line
	*pbUbcodeChange = bUbcodeChange;
	*pbDebugCont = bDebugCont;
	*pExtraChange = bExtraChange;
	*pferrWarning = ferrWarning;
	*pbLvdChange = bLvdChange;				// RevRxNo130730-001 Append Line

	return ferr;
}
// RevRxNo130301-001 Append End


//=============================================================================
/**
 * 命令コード解析結果の参照
 * @param  dwPC        解析する命令のPC値を格納
 * @param  pInstResult 命令解析結果を格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_GetRXINSTCODE(DWORD dwPC, FFWRX_INST_DATA* pInstResult)
{
	FFWERR	ferr;
	FFWERR			ferrEnd;
	FFWRX_INST_MST* tbl;	// 命令解析マスタテーブル
	DWORD	cmd;							// 命令コード
	DWORD	pc = dwPC;						// PCアドレス
	int		pcdsp;							// PC相対アドレスの相対値
	int		nTmp;
	BYTE	byTmpH, byTmpMH, byTmpML, byTmpL;
	BYTE	byReadData[12];					// 4Byte*3ブロック分の領域確保

	ProtInit();

	tbl = GetINSTCMDPtr();	// 命令解析マスタテーブル

	// 命令コード取得 アドレス補正が必要なのでDO_CDUMPで取得する
	ferr = DO_CDUMP(dwPC, (dwPC + 3), &byReadData[0]);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	cmd = ((byReadData[0] << 24) | (byReadData[1] << 16) | (byReadData[2] << 8) | byReadData[3]);

	if(!pInstResult) {
		ferr = ProtEnd();
		// pInstResult領域が確保されていないときはOKで終了する
		return FFWERR_OK;
	}
	memset(pInstResult, 0, sizeof(FFWRX_INST_DATA));	//構造体初期化

	while(tbl->cmd_bitptn != 0xffffffff){	//End of data まで検索
		if((cmd & tbl->cmd_bitptn_mask) == tbl->cmd_bitptn){
			pInstResult->byCmdLen = (BYTE)(tbl->cmd_length);		//命令バイト長
			//分岐命令
			if(tbl->cmd_class.data & TYPE_IS_BRANCH){
				pInstResult->isBranch = TRUE;
				if(tbl->cmd_class.bit.BRANCH_NOCOND){
					pInstResult->branch_type = BRANCH_NOCOND;
				}
				if(tbl->cmd_class.bit.BRANCH_COND){
					pInstResult->branch_type = BRANCH_COND;
				}
				if(tbl->cmd_class.bit.BRANCH_SUB1){
					pInstResult->branch_type = BRANCH_SUB1;
					pInstResult->isSubroutineJump = TRUE;
				}
				if(tbl->cmd_class.bit.BRANCH_SUB2){
					pInstResult->branch_type = BRANCH_SUB2;
				}
				if(tbl->cmd_class.bit.BRANCH_BRK){
					pInstResult->branch_type = BRANCH_BRK;
				}
			}else{
				pInstResult->branch_type = NOT_BRANCH;
			}
			//レジスタ更新命令
			if(tbl->cmd_class.bit.REGISTER_UPDATE){
				pInstResult->isRegUpdate = TRUE;
			}
			//メモリ更新命令
			if(tbl->cmd_class.bit.MEMORY_UPDATE){
				pInstResult->isMemUpdate = TRUE;
			}
			//特殊命令
			if(tbl->cmd_class.data & TYPE_IS_SPECIAL){
				if(tbl->cmd_class.bit.SPECIAL_PSW_DIRECT){
					pInstResult->isPswUpdate = TRUE;
				}
				if(tbl->cmd_class.bit.SPECIAL_WAIT){
					pInstResult->isWait = TRUE;
				}
			}
			//その他命令
			if(tbl->cmd_class.bit.MISC){
				pInstResult->isMisc = TRUE;
			}
			//分岐先アドレスの計算
			if(tbl->branch_mask!=0){
				pcdsp = (int)(cmd & tbl->branch_mask);		//マスクして値を取り出し
				switch (pInstResult->byCmdLen) {
				case 3:		// 命令バイト長が3Byte
					// 以下のように並び替え
					// (例)「0x11, 0x22, 0x33, 0x44」⇒「0x44, 0x33, 0x22, 0x11」
					byTmpH = (BYTE)(pcdsp >> 0);
					byTmpMH = (BYTE)(pcdsp >> 8);
					byTmpML = (BYTE)(pcdsp >> 16);
					byTmpL = (BYTE)(pcdsp >> 24);
					pcdsp = (byTmpH << 24) | (byTmpMH << 16) | (byTmpML << 8) | byTmpL;
					break;
				case 4:		// 命令バイト長が4Byte
					// 以下のように並び替え
					// (例)「0x11, 0x22, 0x33, 0x44」⇒「0x11, 0x44, 0x33, 0x22」
					byTmpH = (BYTE)(pcdsp >> 24);
					byTmpMH = (BYTE)(pcdsp >> 0);
					byTmpML = (BYTE)(pcdsp >> 8);
					byTmpL = (BYTE)(pcdsp >> 16);
					pcdsp = (byTmpH << 24) | (byTmpMH << 16) | (byTmpML << 8) | byTmpL;
					break;
				default:	// 上記以外は何もしない
					break;
				}

				if((sizeof(tbl->branch_mask) * 8 - (tbl->branch_mask_rs + tbl->branch_mask_ls))==3){
					//[符号なし] pcdsp:3
					//右シフトして相対値を取得
					nTmp = (int)(pcdsp >> tbl->branch_mask_rs);
					if (0x3 <= nTmp) {
						pInstResult->dwBranchAddr = pc + nTmp;	//3～7
					}else{
						pInstResult->dwBranchAddr = pc + nTmp + 8;	//8～10
					}

				}else{
					//[符号あり] 上記以外
					//最上位ビット判別
					if(0x80000000 & (pcdsp << tbl->branch_mask_ls)){
						//負
						pcdsp = (~pcdsp) & tbl->branch_mask;		//該当ビット反転
						pcdsp = (pcdsp >> tbl->branch_mask_rs) + 1;	//右シフトして1加算
						pcdsp *= (-1);	//マイナス符号付加
					}else{
						//正
						pcdsp = (pcdsp >> tbl->branch_mask_rs);	//右シフトするだけ
					}
					pInstResult->dwBranchAddr = pc + pcdsp;
				}
				pInstResult->isValidAddr = TRUE;		//上記、有効なアドレスが入っている
			}
			pInstResult->wUpdateRegLlist = 0;		   					//更新レジスタリスト（予約）

			return FFWERR_OK;	//FOUND：解析結果が見つかった
		}
		tbl++;
	}

	ferr = ProtEnd();
	return FFWERR_INSTCODE_FAIL;	//NOT FOUND：解析結果なし
}
//=============================================================================
/**
 * 指定開始アドレスが含まれるブロックのエンディアンを取得する。
 *  MCUファイルでのみ判定をする
 * @param madrStartAddr 開始アドレス
 * @param pbyEndian     エンディアン
 * @retval なし
 */
//=============================================================================
void GetEndianType(MADDR madrStartAddr, BYTE* pbyEndian)
{
	FFWRX_MCUAREA_DATA* pMcuArea;
	DWORD	dwCnt;
	WORD	wEndianState;

	pMcuArea = GetMcuAreaDataRX();

	// SFR領域の場合
	for (dwCnt = 0; dwCnt < pMcuArea->dwSfrAreaNum; dwCnt++) {
		if (pMcuArea->dwmadrSfrStartAddr[dwCnt] <= madrStartAddr && madrStartAddr <= pMcuArea->dwmadrSfrEndAddr[dwCnt]) {
			(*pbyEndian) = pMcuArea->bySfrEndian[dwCnt];
			return;
		}
	}

	// OCD領域の場合
	if (s_dwRegAddr.dwStartAddr <= madrStartAddr && madrStartAddr <= s_dwRegAddr.dwEndAddr) {
		(*pbyEndian) = FFWRX_ENDIAN_LITTLE;
		return;
	}

	// 外部メモリ領域の場合
	for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
		if (pMcuArea->dwmadrExtMemBlockStart[dwCnt] <= madrStartAddr && madrStartAddr <= pMcuArea->dwmadrExtMemBlockEnd[dwCnt]) {
			(*pbyEndian) = pMcuArea->byExtMemEndian[dwCnt];
			return;
		}
	}

	// 上記以外の場合
	wEndianState = GetEndianState();	// リセット時のエンディアン状態を取得
	(*pbyEndian) = (BYTE)wEndianState;

	return;
}


//=============================================================================
/**
 * 指定開始アドレスが含まれるブロックのエンディアンを取得する。
 *		外部領域判定可能
 * @param madrStartAddr 開始アドレス
 * @param pbyEndian     エンディアン
 * @retval なし
 */
//=============================================================================
FFWERR GetEndianType2(MADDR madrStartAddr, BYTE* pbyEndian)
{
	FFWRX_MCUAREA_DATA* pMcuArea;
	DWORD	dwCnt;
	WORD	wEndianState;

	FFWERR	ferr = FFWERR_OK;
	MADDR	madrExtStart[INIT_EXTROM_NUM] = {MCU_CS0_START_ADDR,MCU_CS1_START_ADDR, MCU_CS2_START_ADDR, MCU_CS3_START_ADDR, MCU_CS4_START_ADDR, MCU_CS5_START_ADDR, MCU_CS6_START_ADDR, MCU_CS7_START_ADDR};
	MADDR	madrExtEnd[INIT_EXTROM_NUM] = {MCU_CS0_END_ADDR, MCU_CS1_END_ADDR, MCU_CS2_END_ADDR, MCU_CS3_END_ADDR, MCU_CS4_END_ADDR, MCU_CS5_END_ADDR, MCU_CS6_END_ADDR, MCU_CS7_END_ADDR};
	// RevNo010804-002 Append Line
	pMcuArea = GetMcuAreaDataRX();

	// SFR領域の場合
	for (dwCnt = 0; dwCnt < pMcuArea->dwSfrAreaNum; dwCnt++) {
		if (pMcuArea->dwmadrSfrStartAddr[dwCnt] <= madrStartAddr && madrStartAddr <= pMcuArea->dwmadrSfrEndAddr[dwCnt]) {
			(*pbyEndian) = pMcuArea->bySfrEndian[dwCnt];
			return ferr;
		}
	}

// V.1.01 Append Start
	// OCD領域の場合
	if (s_dwRegAddr.dwStartAddr <= madrStartAddr && madrStartAddr <= s_dwRegAddr.dwEndAddr) {
		(*pbyEndian) = FFWRX_ENDIAN_LITTLE;
		return ferr;
	}
// V.1.01 Append End

// V.1.01 No.17 RAMへのダウンロード高速化 Modify Start
	for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
		if (pMcuArea->dwmadrExtMemBlockStart[dwCnt] <= madrStartAddr && madrStartAddr <= pMcuArea->dwmadrExtMemBlockEnd[dwCnt]){
			// MCUファイルにCS0空間の情報が登録されている(ROM無効指定)ので、指定アドレスがHITした場合
			// 実際にはROM有効モードで、指定アドレスがCS0空間内の場合はCPUエンディアン情報を返す必要あり。
			if((!GetPmodeInRomDisFlg()) && ((madrExtStart[0] <= madrStartAddr) && (madrStartAddr <= madrExtEnd[0]))){
				wEndianState = GetEndianState();	// リセット時のエンディアン状態を取得
				(*pbyEndian) = (BYTE)wEndianState;
				return ferr;
			}else{
				(*pbyEndian) = pMcuArea->byExtMemEndian[dwCnt];
				return ferr;
			}
		}
	}
	// 指定アドレスがMCUファイルで指定された領域にHITしなかった場合
	// → 起動時にROM有効モードが指定されているが、実際にはROM無効モードの場合
	// 実際にはROM無効モードで、指定アドレスがCS0空間内の場合は、レジスタ値をリードしてエンディアンを判断する必要有り
	if((GetPmodeInRomDisFlg()) && ((madrExtStart[0] <= madrStartAddr) && (madrStartAddr <= madrExtEnd[0]))){
		ferr = GetMcuExtCsEndian(0,&wEndianState);// リセット時CPUエンディアン状態を取得		// RevRxNo120621-003 Modify Line
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		*pbyEndian = (BYTE)wEndianState;
		return ferr;
	}

// V.1.01 No.17 RAMへのダウンロード高速化 Modify End

	// 上記以外の場合
	wEndianState = GetEndianState();	// リセット時のエンディアン状態を取得
	(*pbyEndian) = (BYTE)wEndianState;

	return ferr;
}


// V.1.01 No.17 RAMへのダウンロード高速化 Append Start
//=============================================================================
/**
 * 指定開始アドレスが含まれるブロックのエンディアンを取得する。
 *		外部領域判定可能( 外部フラッシュダウンロード専用 )
 * @param madrStartAddr 開始アドレス
 * @param pbyEndian     エンディアン
 * @retval なし
 */
//=============================================================================
FFWERR GetEndianTypeExtRom(MADDR madrStartAddr, BYTE* pbyEndian)
{
	FFWRX_MCUAREA_DATA* pMcuArea;
	DWORD	dwCnt;
	DWORD	dwSetCsNum;
	WORD	wEndianState;

	FFWERR	ferr = FFWERR_OK;
	MADDR	madrExtStart[INIT_EXTROM_NUM] = {MCU_CS1_START_ADDR, MCU_CS2_START_ADDR, MCU_CS3_START_ADDR, MCU_CS4_START_ADDR, MCU_CS5_START_ADDR, MCU_CS6_START_ADDR, MCU_CS7_START_ADDR,MCU_CS0_START_ADDR};
	MADDR	madrExtEnd[INIT_EXTROM_NUM] = {MCU_CS1_END_ADDR, MCU_CS2_END_ADDR, MCU_CS3_END_ADDR, MCU_CS4_END_ADDR, MCU_CS5_END_ADDR, MCU_CS6_END_ADDR, MCU_CS7_END_ADDR,MCU_CS0_END_ADDR};
	DWORD	dwMaxBlock;
	// RevNo120727-001 Delete bRomEna,bExtBus;

	// RevNo010804-002 Append Line
	pMcuArea = GetMcuAreaDataRX();


	// SFR領域の場合
	for (dwCnt = 0; dwCnt < pMcuArea->dwSfrAreaNum; dwCnt++) {
		if (pMcuArea->dwmadrSfrStartAddr[dwCnt] <= madrStartAddr && madrStartAddr <= pMcuArea->dwmadrSfrEndAddr[dwCnt]) {
			(*pbyEndian) = pMcuArea->bySfrEndian[dwCnt];
			return ferr;
		}
	}

	// OCD領域の場合
	if (s_dwRegAddr.dwStartAddr <= madrStartAddr && madrStartAddr <= s_dwRegAddr.dwEndAddr) {
		(*pbyEndian) = FFWRX_ENDIAN_LITTLE;
		return ferr;
	}

	// 外部メモリ領域の場合
	// RevNo120727-001 Modify Line 内蔵Flash有効/無効確認を内部変数に変更
	if( GetPmodeInRomDisFlg() == FALSE) {					// ROM有効の場合
		dwMaxBlock = INIT_EXTROM_NUM - 1;
	}else{						// ROM無効時
		dwMaxBlock = INIT_EXTROM_NUM;
	}
	for (dwCnt = 0; dwCnt < dwMaxBlock; dwCnt++) {
		if (madrExtStart[dwCnt] <= madrStartAddr && madrStartAddr <= madrExtEnd[dwCnt]) {
			// 外部領域アドレスの場合
			// CS番号を算出
			if(dwCnt == 7){
				// CS0の場合
				dwSetCsNum = 0;
			} else {
				// CS1～7の場合
				dwSetCsNum = dwCnt+1;
			}
			ferr = GetMcuExtCsEndian(dwSetCsNum,&wEndianState);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			*pbyEndian = (BYTE)wEndianState;
			return ferr;
		}
	}

	// RevRxNo120910-010	Delete Line

	// RevRxNo120910-010	Append Start
	//	<1>	ｘ = 0～pMcuArea->dwExtMemNum回以下を繰り返す。（MCU情報から、MCUの外部メモリエリア総数 dwExtMemBlockNumを取得）
	for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
		//	(Ⅰ)	指定アドレスが、dwmadrExtMemBlockStart[x]～、dwmadrExtMemBlockEnd[x]の範囲か確認	
		if ((pMcuArea->dwmadrExtMemBlockStart[dwCnt] <= madrStartAddr ) && ( madrStartAddr <= pMcuArea->dwmadrExtMemBlockEnd[dwCnt] )){
			//	(ⅰ）	範囲であれば
			if(pMcuArea->byExtMemType[dwCnt] == MCU_EXT_MEM_SDRAM) {
				ferr = GetMcuExtSdCsEndian(&wEndianState);
				//	<1>	CPUエンディアン取得(wEndianState = GetEndianState())	
				//	((*pbyEndian) = (BYTE)wEndianState)	
				//	<2>	SDCMOD値を取得	
				//	(DO_DUMP(madrSDCMOD[0], madrSDCMOD[0],MBYTE_ACCESS, bReadBuff))	
				//	<3>	CPUエンディアンと異なるエンディアンの場合(SDCMOD.EMODE(0bit目)が1 if( bReadBuff[0] & 0x1 ))	
				//		あ．CPUがビッグエンディアンの場合(if(*pbyEndian))
				//		・エンディアンをビッグに設定((*pbyEndian) = FFWRX_ENDIAN_BIG)
				//		い．CPUがリトルエンディアンの場合(else)
				//		・エンディアン定義をリトルに設定
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				*pbyEndian = (BYTE)wEndianState;
				return ferr;
			}
		}
	}
	// RevNo010804-002 Append End
	// RevRxNo120910-010	Append End

	// 上記以外の場合
	wEndianState = GetEndianState();	// リセット時のエンディアン状態を取得
	(*pbyEndian) = (BYTE)wEndianState;

	return ferr;
}
// V.1.01 No.17 RAMへのダウンロード高速化 Append End

//=============================================================================
/**
 * 外部メモリのアクセスサイズを取得する。
 * @param madrStartAddr 開始アドレス
 * @param eAccessSize   アクセスサイズ
 * @param pbExtArea     外部領域かどうか
 * @retval なし
 */
//=============================================================================
// V.1.02 RevNo110510-003 Modify Line
void GetExtAccessSize(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE* eAccessSize, BOOL* pbExtArea)
{
	FFWRX_MCUAREA_DATA* pMcuArea;
	DWORD	dwCnt;

	// V.1.02 RevNo110510-003 Append Start
	MADDR	madrAreaEnd;
	BOOL	bExtFlashRomArea;
	// V.1.02 RevNo110510-003 Append Start

	pMcuArea = GetMcuAreaDataRX();

	// 外部メモリ領域の場合
	for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
		if (pMcuArea->dwmadrExtMemBlockStart[dwCnt] <= madrStartAddr && madrStartAddr <= pMcuArea->dwmadrExtMemBlockEnd[dwCnt]) {
			// V.1.02 RevNo110510-003 Append Start
			// 外部フラッシュ領域の確認
			CheckExtFlashRomArea(madrStartAddr, madrEndAddr, &madrAreaEnd, &bExtFlashRomArea);
			if (bExtFlashRomArea == TRUE) {			// 外部フラッシュROM領域の場合
				(*pbExtArea) = FALSE;
			}else{		// 外部フラッシュROM領域ではない場合
			// V.1.02 RevNo110510-003 Append End
				(*eAccessSize) = pMcuArea->eExtMemCondAccess[dwCnt];
				(*pbExtArea) = TRUE;
			// V.1.02 RevNo110510-003 Append Line
			}
			return;
		}
	}

	(*pbExtArea) = FALSE;

	return;
}
//=============================================================================
/**
 * 指定開始アドレスを補正する。(CodeWRITE用)
 * @param madrStartAddr 補正前の開始アドレス
 * @param pdwStartAddr  補正後の開始アドレス
 * @retval なし
 */
//=============================================================================
void OffsetCWriteAddr(BYTE byEndian, MADDR madrStartAddr, DWORD* pdwStartAddr)
{
	DWORD dwStart;

	dwStart = madrStartAddr & 0x0000000F;

	if (byEndian == FFWRX_ENDIAN_BIG) {
		if (dwStart == 0x0 || dwStart == 0x4 || dwStart == 0x8 || dwStart == 0xC) {
			(*pdwStartAddr) = madrStartAddr + 3;
		} else if (dwStart == 0x1 || dwStart == 0x5 || dwStart == 0x9 || dwStart == 0xD) {
			(*pdwStartAddr) = madrStartAddr + 1;
		} else if (dwStart == 0x2 || dwStart == 0x6 || dwStart == 0xA || dwStart == 0xE) {
			(*pdwStartAddr) = madrStartAddr - 1;
		} else if (dwStart == 0x3 || dwStart == 0x7 || dwStart == 0xB || dwStart == 0xF) {
			(*pdwStartAddr) = madrStartAddr - 3;
		}
	} else if (byEndian == FFWRX_ENDIAN_LITTLE) {
		(*pdwStartAddr) = madrStartAddr;		// リトルエンディアンの場合、補正しない
	}

	return;
}
//=============================================================================
/**
 * 取得データをエンディアンに合わせる。
 * @param pbyBuff     対象データ
 * @param eAccessSize アクセスサイズ
 * @param dwLen       対象データサイズ
 * @retval なし
 */
//=============================================================================
void ReplaceEndian(BYTE pbyBuff[], FFWENM_MACCESS_SIZE eAccessSize, DWORD dwLen)
{
	BYTE byTmp;
	DWORD i;

	if(eAccessSize == MWORD_ACCESS) {
		for(i=0; i < dwLen; i+=2) {
			byTmp			= pbyBuff[i];
			pbyBuff[i]		= pbyBuff[i+1];
			pbyBuff[i+1]	= byTmp;
		}
	}else if(eAccessSize == MLWORD_ACCESS) {
		for(i=0; i < dwLen; i+=4) {
			byTmp			= pbyBuff[i];
			pbyBuff[i]		= pbyBuff[i+3];
			pbyBuff[i+3]	= byTmp;
			byTmp			= pbyBuff[i+1];
			pbyBuff[i+1]	= pbyBuff[i+2];
			pbyBuff[i+2]	= byTmp;
		}
	}
}
// V.1.03 RevNo111121-008 Append Start
//=============================================================================
/**
 * LWordデータをエンディアンに合わせてバイト並びに並びかえる。
 * @param pdwData   変換前データ格納ポインタ
 * @param pbyData	変換後データ格納ポインタ
 * @param eEndian	エンディアン
 * @param dwCount	変換するLWordデータ数
 * @return なし
 */
//=============================================================================
void ReplaceEndianLWord2Byte(DWORD* pdwData,BYTE* pbyData,FFWENM_ENDIAN eEndian,DWORD dwCount)
{
	DWORD i;

	for(i = 0 ; i < dwCount ; i++){
		if (eEndian == ENDIAN_LITTLE) {
			pbyData[i+0] = static_cast<BYTE>(pdwData[i] & 0x000000ff);
			pbyData[i+1] = static_cast<BYTE>((pdwData[i] >> 8) & 0x000000ff);
			pbyData[i+2] = static_cast<BYTE>((pdwData[i] >> 16) & 0x000000ff);
			pbyData[i+3] = static_cast<BYTE>((pdwData[i] >> 24) & 0x000000ff);
		} else {
			pbyData[i+0] = static_cast<BYTE>((pdwData[i] >> 24) & 0x000000ff);
			pbyData[i+1] = static_cast<BYTE>((pdwData[i] >> 16) & 0x000000ff);
			pbyData[i+2] = static_cast<BYTE>((pdwData[i] >> 8) & 0x000000ff);
			pbyData[i+3] = static_cast<BYTE>(pdwData[i] & 0x000000ff);
		}
	}
}
// V.1.03 RevNo111121-008 Append End

//=============================================================================
/**
 * 指定アドレスがMCU内蔵RAM領域であるかを判定する。
 * @param madr MCUアドレス
 * @retval TRUE アドレスはMCU内蔵RAM領域である
 * @retval FALSE アドレスはMCU内蔵RAM領域でない
 */
//=============================================================================
BOOL IsMcuRamAddr(MADDR madr)
{
	DWORD	i;
	BOOL	bRet = FALSE;

	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;
	pMcuArea = GetMcuAreaDataRX();

	for (i = 0; i < pMcuArea->dwRamAreaNum; i++) {
		if ((pMcuArea->dwmadrRamStartAddr[i] <= madr)
			&& (madr <= pMcuArea->dwmadrRamEndAddr[i])) {
			bRet = TRUE;
		}
	}

	return	bRet;
}

//=============================================================================
/**
 * 指定アドレスがMCU内蔵ROM領域であるかを判定する。
 * @param madr MCUアドレス
 * @retval TRUE アドレスはMCU内蔵ROM領域である
 * @retval FALSE アドレスはMCU内蔵ROM領域でない
 */
//=============================================================================
BOOL IsMcuRomAddr(MADDR madr)
{
	DWORD	i;
	BOOL	bRet = FALSE;
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Line
	MADDR	madrEnd;

	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;
	pMcuArea = GetMcuAreaDataRX();

	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Start
	// データマット領域チェック
	for (i = 0; i < pMcuArea->dwDataFlashRomPatternNum; i++) {
		madrEnd = pMcuArea->dwmadrDataFlashRomStart[i] + (pMcuArea->dwDataFlashRomBlockSize[i] * pMcuArea->dwDataFlashRomBlockNum[i]) - 1;
		if ((pMcuArea->dwmadrDataFlashRomStart[i] <= madr)	&& (madr <= madrEnd)) {
			bRet = TRUE;
			return	bRet;
		}
	}
	// ユーザマット領域チェック
	for (i = 0; i < pMcuArea->dwFlashRomPatternNum; i++) {
		madrEnd = pMcuArea->dwmadrFlashRomStart[i] + (pMcuArea->dwFlashRomBlockSize[i] * pMcuArea->dwFlashRomBlockNum[i]) - 1;
		if ((pMcuArea->dwmadrFlashRomStart[i] <= madr)	&& (madr <= madrEnd)) {
			bRet = TRUE;
			return	bRet;
		}
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify End
	// V.1.02 覚え書き35対応 Append Start
	// ユーザブートマット領域チェック
// RevNo111121-005 Modify Start
	if ((pMcuArea->dwmadrUserBootStart != 0) && (pMcuArea->dwmadrUserBootEnd != 0)) {
		if ((pMcuArea->dwmadrUserBootStart <= madr)	&& (madr <= pMcuArea->dwmadrUserBootEnd)) {
			bRet = TRUE;
			return	bRet;
		}
	}
// RevNo111121-005 Modify End
	// V.1.02 覚え書き35対応 Append End

	return	bRet;
}

// V.1.02 No.26 データフラッシュCPU書き換えデバッグ対象化 Append Start
//=============================================================================
/**
 * 指定アドレスがMCU内蔵ROM(プログラムROM)領域であるかを判定する。
 * @param madr MCUアドレス
 * @retval TRUE アドレスはMCU内蔵ROM(プログラムROM)領域である
 * @retval FALSE アドレスはMCU内蔵ROM(プログラムROM)領域でない
 */
//=============================================================================
BOOL IsProgramRomAddr(MADDR madr)
{
	DWORD	i;
	BOOL	bRet = FALSE;
	MADDR	madrEnd;

	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;
	pMcuArea = GetMcuAreaDataRX();

	// ユーザマット領域チェック
	for (i = 0; i < pMcuArea->dwFlashRomPatternNum; i++) {
		madrEnd = pMcuArea->dwmadrFlashRomStart[i] + (pMcuArea->dwFlashRomBlockSize[i] * pMcuArea->dwFlashRomBlockNum[i]) - 1;
		if ((pMcuArea->dwmadrFlashRomStart[i] <= madr)	&& (madr <= madrEnd)) {
			bRet = TRUE;
			return	bRet;
		}
	}

	return	bRet;
}

//=============================================================================
/**
 * 指定アドレスがMCU内蔵ROM(データフラッシュ)領域であるかを判定する。
 * @param madr MCUアドレス
 * @retval TRUE アドレスはMCU内蔵ROM(データフラッシュ)領域である
 * @retval FALSE アドレスはMCU内蔵ROM(データフラッシュ)領域でない
 */
//=============================================================================
BOOL IsDataRomAddr(MADDR madr)
{
	DWORD	i;
	BOOL	bRet = FALSE;
	MADDR	madrEnd;

	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;
	pMcuArea = GetMcuAreaDataRX();

	// データマット領域チェック
	for (i = 0; i < pMcuArea->dwDataFlashRomPatternNum; i++) {
		madrEnd = pMcuArea->dwmadrDataFlashRomStart[i] + (pMcuArea->dwDataFlashRomBlockSize[i] * pMcuArea->dwDataFlashRomBlockNum[i]) - 1;
		if ((pMcuArea->dwmadrDataFlashRomStart[i] <= madr)	&& (madr <= madrEnd)) {
			bRet = TRUE;
			return	bRet;
		}
	}

	return	bRet;
}
// V.1.02 No.26 データフラッシュCPU書き換えデバッグ対象化 Append End


//=============================================================================
/**
 * 指定領域がMCU内蔵ROM領域を含まないかどうかを判定する。
 * @param madrStart 開始アドレス
 * @param madrEnd 終了アドレス
 * @retval TRUE  指定アドレス範囲(開始アドレス～終了アドレス)はMCU内蔵ROM領域を含まない
 * @retval FALSE 指定アドレス範囲は(開始アドレス～終了アドレス)MCU内蔵ROM領域を含む
 */
//=============================================================================
BOOL IsNotMcuRomArea(MADDR madrStart, MADDR madrEnd)
{
	DWORD	i;
	BOOL	bRet = TRUE;
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Line
	MADDR	madrTmpEnd;
	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;
	pMcuArea = GetMcuAreaDataRX();

	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Start
	// データマット領域チェック
	for (i = 0; i < pMcuArea->dwDataFlashRomPatternNum; i++) {
		madrTmpEnd = pMcuArea->dwmadrDataFlashRomStart[i] + (pMcuArea->dwDataFlashRomBlockSize[i] * pMcuArea->dwDataFlashRomBlockNum[i]) - 1;
		if ((pMcuArea->dwmadrDataFlashRomStart[i] <= madrStart) && (madrStart <= madrTmpEnd)) {	// 開始アドレスがブロック範囲内の場合
			bRet = FALSE;
			return	bRet;
		}
		if ((pMcuArea->dwmadrDataFlashRomStart[i] <= madrEnd) && (madrEnd <= madrTmpEnd)) {	// 終了アドレスがブロック範囲内の場合
			bRet = FALSE;
			return	bRet;
		}
		if ((madrStart < pMcuArea->dwmadrDataFlashRomStart[i]) && (madrTmpEnd < madrEnd)) {	// 開始、終了アドレスがブロック範囲を含む場合
			bRet = FALSE;
			return	bRet;
		}
	}
	// ユーザマット領域チェック
	for (i = 0; i < pMcuArea->dwFlashRomPatternNum; i++) {
		madrTmpEnd = pMcuArea->dwmadrFlashRomStart[i] + (pMcuArea->dwFlashRomBlockSize[i] * pMcuArea->dwFlashRomBlockNum[i]) - 1;
		if ((pMcuArea->dwmadrFlashRomStart[i] <= madrStart) && (madrStart <= madrTmpEnd)) {	// 開始アドレスがブロック範囲内の場合
			bRet = FALSE;
			return	bRet;
		}
		if ((pMcuArea->dwmadrFlashRomStart[i] <= madrEnd) && (madrEnd <= madrTmpEnd)) {	// 終了アドレスがブロック範囲内の場合
			bRet = FALSE;
			return	bRet;
		}
		if ((madrStart < pMcuArea->dwmadrFlashRomStart[i]) && (madrTmpEnd < madrEnd)) {	// 開始、終了アドレスがブロック範囲を含む場合
			bRet = FALSE;
			return	bRet;
		}
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify End
	// V.1.02 RevNo110311-003 Appned Start
	// ユーザブートマット領域チェック
// RevNo111121-005 Modify Start
	if ((pMcuArea->dwmadrUserBootStart != 0) && (pMcuArea->dwmadrUserBootEnd != 0)) {
		if ((pMcuArea->dwmadrUserBootStart <= madrStart) && (madrStart <= pMcuArea->dwmadrUserBootEnd)) {
			bRet = FALSE;
			return	bRet;
		}
		if ((pMcuArea->dwmadrUserBootStart <= madrEnd) && (madrEnd <= pMcuArea->dwmadrUserBootEnd)) {
			bRet = FALSE;
			return	bRet;
		}
		if ((madrStart < pMcuArea->dwmadrUserBootStart) && (pMcuArea->dwmadrUserBootEnd < madrEnd)) {	// 開始、終了アドレスがユーザブート領域を含む場合
			bRet = FALSE;
			return	bRet;
		}
	}
// RevNo111121-005 Modify End
	// V.1.02 RevNo110311-003 Appned End

	return	bRet;
}

//=============================================================================
/**
 * 指定アドレスを含むフラッシュROMブロックの終了アドレスを取得する。
 *  指定アドレスは、MCUコマンドで指定されているフラッシュROM領域内のアドレスでなければならない。
 *	(本関数ではチェックしない)
 * @param madr MCUアドレス
 * @retval madrを含むフラッシュROMブロックの終了アドレス
 */
//=============================================================================
MADDR	GetFlashRomBlockEndAddr(MADDR madr)
{
	DWORD	i;
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Line( 初期値入れておく )
	MADDR	madrEnd = 0xFFFFFFFF;
	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;
	
	pMcuArea = GetMcuAreaDataRX();

	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Start
	for (i = 0; i < pMcuArea->dwFlashRomPatternNum; i++) {
		madrEnd = pMcuArea->dwmadrFlashRomStart[i] + (pMcuArea->dwFlashRomBlockSize[i] * pMcuArea->dwFlashRomBlockNum[i]) - 1;
		if ((pMcuArea->dwmadrFlashRomStart[i] <= madr) && (madr <= madrEnd)) {
			break;
		}
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify End

	return madrEnd;
}

// RevNo010804-001 Append Start
//=============================================================================
/**
 * ターゲットMCUのデータFlashブロック数取得
 * @param なし
 * @retval データFlashブロック数
 */
//=============================================================================
DWORD GetDataFlashBlockNum(void){

	DWORD i;
	DWORD dwDataFlashCnt = 0;
	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;
	pMcuArea = GetMcuAreaDataRX();

	//データFlashのブロック数カウント
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Start
	for(i = 0; i < pMcuArea->dwDataFlashRomPatternNum; i++){
		dwDataFlashCnt += pMcuArea->dwDataFlashRomBlockNum[i];
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify End
	return dwDataFlashCnt;
}
// RevNo010804-001 Append End

//=============================================================================
/**
 * 指定開始アドレスがどの領域に属するかを判定する。
 * 指定開始アドレスが含まれるブロックの読込サイズを返送する。 　
 * 予約領域の場合、無効な読込サイズを返送する。
 * 本関数を呼ぶ前に必ずSetPmodeInRomReg2Flg()を呼んでおくこと //RevNo120727-001 Append Line
 * @param  madrStartAddr 開始アドレス
 * @param  madrEndAddr   終了アドレス
 * @param  pdwLen        サイズ
 * @param  pbRomArea     ROMエリア有無(TRUE：ROMエリア、FALSE：ROMエリア以外)
 * @param  byDumpFill    0:Dump 1:Fill/Write
 * @param  bEnableArea   TRUE:有効な領域、FALSE:無効な領域
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ChkBlockArea(MADDR madrStartAddr, MADDR madrEndAddr, DWORD* pdwLen, enum FFWRXENM_MAREA_TYPE *eAreaType, BYTE byDumpFill, BOOL* bEnableArea)
{
	FFWRX_MCUAREA_DATA* pMcuArea;
	DWORD	dwCnt, dwCnt2;
	DWORD	dwNonSize = 0;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	FFWERR	ferr = FFWERR_OK;
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Start
	MADDR	madrStart;
	MADDR	madrEnd;
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append End
	DWORD	dwAreaNo;			// RevRxNo140109-001 Append Line
	MADDR	madrAreaStart;		// RevRxNo140109-001 Append Line
	// RevRxNo131101-005 Append Start 
	MADDR	madrAreaEnd;
	BOOL	bExtFlashRomArea;
	// RevRxNo131101-005 Append End 
	// RevNo120727-001 Delete bRomEna,bExtBus
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo140109-001 Append Line
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo140109-001 Append Line
	BOOL	bConfigArea;			// RevRxNo140109-001 Append Line
	MADDR	madrDwnpStart;			// RevRxNo140109-001 Append Line
	MADDR	madrDwnpEnd;			// RevRxNo140109-001 Append Line
	BOOL	bTMInclude = FALSE;		// RevRxNo140617-001 Append Line 

	(*eAreaType) = MAREA_ETC;		// RevRxNo131101-005 Append Line

	pMcuArea = GetMcuAreaDataRX();
	pDbgData = GetDbgDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo140109-001 Append Line
	pMcuDef = GetMcuDefData();	// RevRxNo140109-001 Append Line

	// 制御レジスタ領域の場合
	if ((s_dwRegAddr.dwStartAddr <= madrStartAddr) && (madrStartAddr <= s_dwRegAddr.dwEndAddr)) {
		switch (pDbgData->eOcdRegAcsMode) {
		case OCD_ACSMODE_NONDUMPFILL:
			// 無効な領域として扱う為、ここでは何もしない
			break;
		case OCD_ACSMODE_FILL:
			if (byDumpFill == 1) {	// Fill/Write時のみ有効な領域として扱う
				if (madrEndAddr <= s_dwRegAddr.dwEndAddr) {
					(*pdwLen) = madrEndAddr - madrStartAddr + 1;
				} else {
					(*pdwLen) = s_dwRegAddr.dwEndAddr - madrStartAddr + 1;
				}
				(*eAreaType) = MAREA_ETC;
				(*bEnableArea) = TRUE;
				return ferr;
			}
			break;
		case OCD_ACSMODE_DUMPFILL:
			// 有効な領域として扱う
			if (madrEndAddr <= s_dwRegAddr.dwEndAddr) {
				(*pdwLen) = madrEndAddr - madrStartAddr + 1;
			} else {
				(*pdwLen) = s_dwRegAddr.dwEndAddr - madrStartAddr + 1;
			}
			(*eAreaType) = MAREA_ETC;
			(*bEnableArea) = TRUE;
			return ferr;
		case OCD_ACSMODE_DUMP:
			if (byDumpFill == 0) {	// Dump時のみ有効な領域として扱う
				// 有効な領域として扱う
				if (madrEndAddr <= s_dwRegAddr.dwEndAddr) {
					(*pdwLen) = madrEndAddr - madrStartAddr + 1;
				} else {
				(*pdwLen) = s_dwRegAddr.dwEndAddr - madrStartAddr + 1;
				}
				(*eAreaType) = MAREA_ETC;
				(*bEnableArea) = TRUE;
				return ferr;
			}		
			break;
		}
	}

	// SFR領域の場合
	for (dwCnt = 0; dwCnt < pMcuArea->dwSfrAreaNum; dwCnt++) {
		if ((pMcuArea->dwmadrSfrStartAddr[dwCnt] <= madrStartAddr) && (madrStartAddr <= pMcuArea->dwmadrSfrEndAddr[dwCnt])) {
			if (madrEndAddr <= pMcuArea->dwmadrSfrEndAddr[dwCnt]) {
				(*pdwLen) = madrEndAddr - madrStartAddr + 1;
			} else {
				(*pdwLen) = pMcuArea->dwmadrSfrEndAddr[dwCnt] - madrStartAddr + 1;
			}
			(*eAreaType) = MAREA_ETC;
			(*bEnableArea) = TRUE;
			return ferr;
		}
	}

	// RevRxNo140109-001 Append Start
	// コンフィギュレーション設定領域の場合
	// ※コンフィギュレーション設定領域を内部RAM領域として定義しているため、内部RAM領域判定より前に実施する。
	if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
		dwAreaNo = pMcuDef->dwConfigAreaRamNo;	// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号設定
		madrAreaStart = pMcuArea->dwmadrRamStartAddr[dwAreaNo];
		madrAreaEnd = pMcuArea->dwmadrRamEndAddr[dwAreaNo];

		// 開始アドレスがコンフィギュレーション設定領域内の場合
		if ((madrAreaStart <= madrStartAddr) && (madrStartAddr <= madrAreaEnd)) {

			// ダウンロード以外のFill/Write、またはROM無効の場合
			if (((byDumpFill == 1) && (GetDwnpOpenData() == FALSE)) 
					|| (GetPmodeInRomDisFlg() == TRUE)) {
				if (madrEndAddr <= madrAreaEnd) {
					(*pdwLen) = madrEndAddr - madrStartAddr + 1;
				} else {
					(*pdwLen) = madrAreaEnd - madrStartAddr + 1;
				}
				bConfigArea = FALSE;	// コンフィギュレーション設定領域でない

			// ROM有効時の、Dumpまたはダウンロード中のFill/Writeの場合
			} else {
				if (byDumpFill == 0) {	// Dump時
					if (madrEndAddr <= madrAreaEnd) {
						(*pdwLen) = madrEndAddr - madrStartAddr + 1;
					} else {
						(*pdwLen) = madrAreaEnd - madrStartAddr + 1;
					}
					bConfigArea = TRUE;	// コンフィギュレーション設定領域である

				} else {	// ダウンロード中のFill/Write時
					//ダウンロード対象領域の設定
					madrDwnpStart = pMcuDef->madrDwnpConfigStart;
					madrDwnpEnd = pMcuDef->madrDwnpConfigEnd;

					// 開始アドレスがダウンロード対象領域より小さい場合(予約領域判定)
					if (madrStartAddr < madrDwnpStart) {
						if (madrEndAddr < madrDwnpStart){	// madrStartAddr <= madrEndAddr < ダウンロード対象開始アドレス
							(*pdwLen) = madrEndAddr - madrStartAddr + 1;
						} else {							// madrStartAddr < ダウンロード対象開始アドレス <= madrEndAddr
							(*pdwLen) = (madrDwnpStart - 1) - madrStartAddr + 1;
						}
						bConfigArea = FALSE;	// コンフィギュレーション設定領域でない

					// 開始アドレスがダウンロード対象領域内の場合(コンフィギュレーション設定領域判定)
					} else if (madrStartAddr <= madrDwnpEnd) {
						if (madrEndAddr <= madrDwnpEnd) {	// ダウンロード対象開始アドレス <= madrStartAddr <= madrEndAddr <= ダウンロード対象終了アドレス
							(*pdwLen) = madrEndAddr - madrStartAddr + 1;
						} else {							// ダウンロード対象開始アドレス <= madrStartAddr <= ダウンロード対象終了アドレス < madrEndAddr
							(*pdwLen) = madrDwnpEnd - madrStartAddr + 1;
						}
						bConfigArea = TRUE;	// コンフィギュレーション設定領域である

					// 開始アドレスがダウンロード対象領域より大きい場合(予約領域判定)
					} else {
						if (madrEndAddr <= madrAreaEnd) {	// ダウンロード対象終了アドレス < madrStartAddr <= madrEndAddr <= コンフィギュレーション設定領域アドレス
							(*pdwLen) = madrEndAddr - madrStartAddr + 1;
						} else {
							(*pdwLen) = madrAreaEnd - madrStartAddr + 1;
						}
						bConfigArea = FALSE;	// コンフィギュレーション設定領域でない
					}
				}
			}

			if (bConfigArea == TRUE) {
				(*eAreaType) = MAREA_CONFIG;	// コンフィギュレーション設定領域
				(*bEnableArea) = TRUE;
			} else {
				(*eAreaType) = MAREA_ETC;		// 予約領域
				(*bEnableArea) = FALSE;
			}
			return ferr;
		}
	}
	// RevRxNo140109-001 Append End


	// 内部RAM領域の場合
	for (dwCnt = 0; dwCnt < pMcuArea->dwRamAreaNum; dwCnt++) {
		if ((pMcuArea->dwmadrRamStartAddr[dwCnt] <= madrStartAddr) && (madrStartAddr <= pMcuArea->dwmadrRamEndAddr[dwCnt])) {
			if (madrEndAddr <= pMcuArea->dwmadrRamEndAddr[dwCnt]) {
				(*pdwLen) = madrEndAddr - madrStartAddr + 1;
			} else {
				(*pdwLen) = pMcuArea->dwmadrRamEndAddr[dwCnt] - madrStartAddr + 1;
			}
			(*eAreaType) = MAREA_INRAM;	// RevRxE2LNo141104-001 Modify Line
			(*bEnableArea) = TRUE;
			return ferr;
		}
	}

	// FCU-RAM領域の場合
	// RevRxNo120910-011	Modify Start
	// (1)サイズdwFcuFirmLenが0でない場合、以下(a)の内容を実行する。	
	if( pMcuArea->dwFcuFirmLen != 0) {
		// RevRxNo120910-001 Modify Start
		if ((pMcuArea->dwAdrFcuRamStart <= madrStartAddr) && (madrStartAddr <= (pMcuArea->dwAdrFcuRamStart + pMcuArea->dwFcuFirmLen -1))) {
			if (madrEndAddr <= (pMcuArea->dwAdrFcuRamStart + pMcuArea->dwFcuFirmLen -1) ){
		// RevRxNo120910-001 Modify End
				(*pdwLen) = madrEndAddr - madrStartAddr + 1;
			}
			else{
				// V.1.02 RevNo110421-001 Modify Line
				// RevRxNo120910-001 Modify Line
				(*pdwLen) = (pMcuArea->dwAdrFcuRamStart + pMcuArea->dwFcuFirmLen -1) - madrStartAddr + 1;
			}
			(*eAreaType) = MAREA_ETC;
			(*bEnableArea) = TRUE;
			return ferr;
		}
	}
	// RevRxNo120910-011	Modify End

	// FCUファーム領域の場合
	// RevRxNo120910-011	Modify Start
	// (1)サイズdwFcuFirmLenが0でない場合、以下(a)の内容を実行する。	
	if( pMcuArea->dwFcuFirmLen != 0) {
	// RevRxNo120910-001 Modify Start
		if ((pMcuArea->dwAdrFcuFirmStart <= madrStartAddr) && (madrStartAddr <= (pMcuArea->dwAdrFcuFirmStart + pMcuArea->dwFcuFirmLen -1))) {
			if (madrEndAddr <= (pMcuArea->dwAdrFcuFirmStart + pMcuArea->dwFcuFirmLen -1) ){
			// RevRxNo120910-001 Modify End
				(*pdwLen) = madrEndAddr - madrStartAddr + 1;
			}
			else{
				// V.1.02 RevNo110421-001 Modify Line
				// RevRxNo120910-001 Modify Line
				(*pdwLen) = (pMcuArea->dwAdrFcuFirmStart + pMcuArea->dwFcuFirmLen -1) - madrStartAddr + 1;
			}
			(*eAreaType) = MAREA_ETC;
			(*bEnableArea) = TRUE;
			return ferr;
		}
	}
	// RevRxNo120910-011	Modify End

	// 内部FlashROM領域の場合(データマット)
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Start
	for (dwCnt = 0; dwCnt < pMcuArea->dwDataFlashRomPatternNum; dwCnt++) {
		madrStart = pMcuArea->dwmadrDataFlashRomStart[dwCnt];
		for (dwCnt2 = 0; dwCnt2 < pMcuArea->dwDataFlashRomBlockNum[dwCnt]; dwCnt2++) {
			madrEnd = madrStart + pMcuArea->dwDataFlashRomBlockSize[dwCnt] - 1;
			if ((madrStart <= madrStartAddr) && (madrStartAddr <= madrEnd)) {
				if (madrEndAddr <= madrEnd) {
					(*pdwLen) = madrEndAddr - madrStartAddr + 1;
				} else {
					(*pdwLen) = madrEnd - madrStartAddr + 1;
				}

				// RevNo120727-001 Modify Line 内蔵Flash有効/無効確認を内部変数に変更
				if( GetPmodeInRomDisFlg() == TRUE) {					// ROM無効の場合
					(*eAreaType) = MAREA_ETC;		// RevRxNo131101-005 Modify Line( MAREA_EXTRAM→MAREA_ETC )
					(*bEnableArea) = FALSE;			// RevRxNo131101-005 Modify Line
				} else {
					(*eAreaType) = MAREA_DATA;
					(*bEnableArea) = TRUE;
				}
				return ferr;
			}
			madrStart += pMcuArea->dwDataFlashRomBlockSize[dwCnt];
		}
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append End

	// 内部FlashROM領域の場合(ユーザマット)
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Start
	for (dwCnt = 0; dwCnt < pMcuArea->dwFlashRomPatternNum; dwCnt++) {
		madrStart = pMcuArea->dwmadrFlashRomStart[dwCnt];
		for (dwCnt2 = 0; dwCnt2 < pMcuArea->dwFlashRomBlockNum[dwCnt]; dwCnt2++) {
			madrEnd = madrStart + pMcuArea->dwFlashRomBlockSize[dwCnt] - 1;
			if ((madrStart <= madrStartAddr) && (madrStartAddr <= madrEnd)) {
				if (madrEndAddr <= madrEnd) {
					(*pdwLen) = madrEndAddr - madrStartAddr + 1;
				} else {
					(*pdwLen) = madrEnd - madrStartAddr + 1;
				}

				// RevNo120727-001 Modify Line 内蔵Flash有効/無効確認を内部変数に変更
				if( GetPmodeInRomDisFlg() == TRUE) {					// ROM無効の場合
					// RevRxNo131101-005 Mpdify Start(ROM無効の場合は、外部RAMなのか外部ROMなのかまで判定するようにする)
					CheckExtFlashRomArea(madrStartAddr, madrEndAddr, &madrAreaEnd, &bExtFlashRomArea);
					if (bExtFlashRomArea == TRUE) {					// 外部フラッシュROM領域の場合
						(*eAreaType) = MAREA_EXTROM;
					} else {										// 外部フラッシュROM領域以外の場合
						(*eAreaType) = MAREA_EXTRAM;
					}
					// RevRxNo131101-005 Modify End
				} else {
					(*eAreaType) = MAREA_USER;
				}

				// RevRxNo140617-001 Append Start
				if (GetTMEnable() == TRUE) {					// TM機能有効の場合
					if (ChkTmArea(madrStartAddr) == TRUE) {		// 開始アドレスがTM領域内の場合
						bTMInclude = TRUE;
					}
				}
				if ((*eAreaType == MAREA_USER) && bTMInclude) {		// ユーザマットでTM領域内の場合
					SetDwnpDataIncludeTMArea(TRUE);				// ダウンロードデータにTM領域データが含まれていたフラグにTRUEを設定
					(*bEnableArea) = FALSE;			// 無効領域にする
				} else {
				// RevRxNo140617-001 Append End
					(*bEnableArea) = TRUE;
				}	// RevRxNo140617-001 Append Line				
				return ferr;
			}
			madrStart += pMcuArea->dwFlashRomBlockSize[dwCnt];
		}
	}

	// 内部FlashROM領域の場合(ユーザーブートマット)
// RevNo111121-005 Modify Start
	if ((pMcuArea->dwmadrUserBootStart != 0) && (pMcuArea->dwmadrUserBootEnd != 0)) {
		if ((pMcuArea->dwmadrUserBootStart <=  madrStartAddr) && (madrStartAddr <= pMcuArea->dwmadrUserBootEnd)) {
			if(madrEndAddr <= pMcuArea->dwmadrUserBootEnd){
				(*pdwLen) = madrEndAddr - madrStartAddr + 1;
			}
			else{
				(*pdwLen) = pMcuArea->dwmadrUserBootEnd - madrStartAddr + 1;
			}
			// RevNo120727-001 Modify Line 内蔵Flash有効/無効確認を内部変数に変更
			if( GetPmodeInRomDisFlg() == TRUE) {					// ROM無効の場合
				// RevRxNo131101-005 Mpdify Start(ROM無効の場合は、外部RAMなのか外部ROMなのかまで判定するようにする)
				CheckExtFlashRomArea(madrStartAddr, madrEndAddr, &madrAreaEnd, &bExtFlashRomArea);
				if (bExtFlashRomArea == TRUE) {					// 外部フラッシュROM領域の場合
					(*eAreaType) = MAREA_EXTROM;
				} else {										// 外部フラッシュROM領域以外の場合
					(*eAreaType) = MAREA_EXTRAM;
				}
				// RevRxNo131101-005 Modify End
			} else {
				(*eAreaType) = MAREA_USERBOOT;
			}
			(*bEnableArea) = TRUE;
			return ferr;
		}
	}
// RevNo111121-005 Modify End

	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify End

	// 外部メモリ領域の場合
	for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
		if ((pMcuArea->dwmadrExtMemBlockStart[dwCnt] <= madrStartAddr) && (madrStartAddr <= pMcuArea->dwmadrExtMemBlockEnd[dwCnt])) {
			// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Append&Modify Start
			// RevRxNo131101-005 Delete(madrAreaEnd,bExtFlashRomAreaの宣言を関数の先頭に移動)
			if (madrEndAddr <= pMcuArea->dwmadrExtMemBlockEnd[dwCnt]) {
				(*pdwLen) = madrEndAddr - madrStartAddr + 1;
			} else {
				(*pdwLen) = pMcuArea->dwmadrExtMemBlockEnd[dwCnt] - madrStartAddr + 1;
			}
			CheckExtFlashRomArea(madrStartAddr, madrEndAddr, &madrAreaEnd, &bExtFlashRomArea);
			if (bExtFlashRomArea == TRUE) {					// 外部フラッシュROM領域の場合
				(*eAreaType) = MAREA_EXTROM;
				(*bEnableArea) = TRUE;
			} else {										// 外部フラッシュROM領域以外の場合
				(*eAreaType) = MAREA_EXTRAM;
				(*bEnableArea) = TRUE;
			}
			// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Append&Modify End
			return ferr;
		}
	}


	// 上記以外の場合、無効なサイズを算出(有効な領域に入った場合、途中で抜ける)
	while (madrStartAddr <= madrEndAddr) {
		// SFR領域の場合
		for (dwCnt = 0; dwCnt < pMcuArea->dwSfrAreaNum; dwCnt++) {
			if ((pMcuArea->dwmadrSfrStartAddr[dwCnt] <= madrStartAddr) && (madrStartAddr <= pMcuArea->dwmadrSfrEndAddr[dwCnt])) {
				(*pdwLen) = dwNonSize;
				(*bEnableArea) = FALSE;
				return ferr;
			}
		}

		// コンフィギュレーション設定領域の場合
		// ※コンフィギュレーション設定領域を内部RAM領域として定義しているため、内部RAM領域判定より前に実施する。
		if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
			dwAreaNo = pMcuDef->dwConfigAreaRamNo;	// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号設定
			madrAreaStart = pMcuArea->dwmadrRamStartAddr[dwAreaNo];
			madrAreaEnd = pMcuArea->dwmadrRamEndAddr[dwAreaNo];

			if ((madrAreaStart <= madrStartAddr) && (madrStartAddr <= madrAreaEnd)) {
				(*pdwLen) = dwNonSize;
				(*bEnableArea) = FALSE;
				return ferr;
			}
		}

		// 内部RAM領域の場合
		for (dwCnt = 0; dwCnt < pMcuArea->dwRamAreaNum; dwCnt++) {
			if ((pMcuArea->dwmadrRamStartAddr[dwCnt] <= madrStartAddr) && (madrStartAddr <= pMcuArea->dwmadrRamEndAddr[dwCnt])) {
				(*pdwLen) = dwNonSize;
				(*bEnableArea) = FALSE;
				return ferr;
			}
		}
		// RevRxNo131101-005 Delete(内部FlashROM領域の場合の処理)

		// FCU-RAM領域の場合
		// RevRxNo120910-011	Modify Start
		// <1> サイズdwFcuFirmLenが0でない場合、以下(a)の内容を実行する。
		if( pMcuArea->dwFcuFirmLen != 0) {
			// RevRxNo120910-001 Modify Start
			if ((pMcuArea->dwAdrFcuRamStart <= madrStartAddr) && (madrStartAddr <= (pMcuArea->dwAdrFcuRamStart + pMcuArea->dwFcuFirmLen -1))) {
				// RevRxNo131101-005 Delete Lline
				// RevRxNo120910-001 Modify End
				(*pdwLen) = dwNonSize;
				(*bEnableArea) = FALSE;
				return ferr;
				// RevRxNo131101-005 Delete Lline
			}
		}
		// RevRxNo120910-011	Modify End
		// FCUファーム領域の場合
		// RevRxNo120910-011	Modify Start
		// <1> サイズdwFcuFirmLenが0でない場合、以下(a)の内容を実行する。
		if( pMcuArea->dwFcuFirmLen != 0) {
			// RevRxNo120910-001 Modify Start
			if ((pMcuArea->dwAdrFcuFirmStart <= madrStartAddr) && (madrStartAddr <= (pMcuArea->dwAdrFcuFirmStart + pMcuArea->dwFcuFirmLen -1))) {
				// RevRxNo131101-005 Delete Lline
			// RevRxNo120910-001 Modify End
				(*pdwLen) = dwNonSize;
				(*bEnableArea) = FALSE;
				return ferr;
				// RevRxNo131101-005 Delete Lline
			}
		}	
		// RevRxNo120910-011	Modify End
		// 内部FlashROM領域の場合(データマット)
		for (dwCnt = 0; dwCnt < pMcuArea->dwDataFlashRomPatternNum; dwCnt++) {
			madrStart = pMcuArea->dwmadrDataFlashRomStart[dwCnt];
			for (dwCnt2 = 0; dwCnt2 < pMcuArea->dwDataFlashRomBlockNum[dwCnt]; dwCnt2++) {
				madrEnd = madrStart + pMcuArea->dwDataFlashRomBlockSize[dwCnt] - 1;
				if (madrStart <= madrStartAddr && madrStartAddr <= madrEnd) {		
					(*pdwLen) = dwNonSize;
					(*bEnableArea) = FALSE;
					return ferr;
				}
			}	
		}
		// 内部FlashROM領域の場合(ユーザマット)
		// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Start
		for (dwCnt = 0; dwCnt < pMcuArea->dwFlashRomPatternNum; dwCnt++) {
			madrStart = pMcuArea->dwmadrFlashRomStart[dwCnt];
			for (dwCnt2 = 0; dwCnt2 < pMcuArea->dwFlashRomBlockNum[dwCnt]; dwCnt2++) {
				madrEnd = madrStart + pMcuArea->dwFlashRomBlockSize[dwCnt] - 1;
				if (madrStart <= madrStartAddr && madrStartAddr <= madrEnd) {
					(*pdwLen) = dwNonSize;
					(*bEnableArea) = FALSE;
					return ferr;
				}
			}
		}
		// 内部FlashROM領域の場合(ユーザブートマット)
// RevNo111121-005 Modify Start
		if ((pMcuArea->dwmadrUserBootStart != 0) && (pMcuArea->dwmadrUserBootEnd != 0)) {
			if ((pMcuArea->dwmadrUserBootStart <=  madrStartAddr) && (madrStartAddr <= pMcuArea->dwmadrUserBootEnd)) {
				(*pdwLen) = dwNonSize;
				(*bEnableArea) = FALSE;
				return ferr;
			}
		}
// RevNo111121-005 Modify End
		// 外部メモリ領域の場合
		for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
			if ((pMcuArea->dwmadrExtMemBlockStart[dwCnt] <= madrStartAddr) && (madrStartAddr <= pMcuArea->dwmadrExtMemBlockEnd[dwCnt])) {
				(*pdwLen) = dwNonSize;
				(*bEnableArea) = FALSE;
				return ferr;
			}
		}
		// 制御レジスタ領域の場合
		if (s_dwRegAddr.dwStartAddr <= madrStartAddr && madrStartAddr <= s_dwRegAddr.dwEndAddr) {
			switch (pDbgData->eOcdRegAcsMode) {
			case OCD_ACSMODE_NONDUMPFILL:
				// 無効な領域として扱う為、ここでは何もしない
				break;
			case OCD_ACSMODE_FILL:
				if (byDumpFill == 1) {	// Fill/Write時のみ有効な領域として扱う
					(*pdwLen) = dwNonSize;
					(*bEnableArea) = FALSE;
					return ferr;
				}
				break;
			case OCD_ACSMODE_DUMPFILL:
				// 有効な領域として扱う
				(*pdwLen) = dwNonSize;
				(*bEnableArea) = FALSE;
				return ferr;
			case OCD_ACSMODE_DUMP:
				if (byDumpFill == 0) {	// Dump時のみ有効な領域として扱う
					(*pdwLen) = dwNonSize;
					(*bEnableArea) = FALSE;
					return ferr;
				}
				break;
			}
		}
		dwNonSize++;
		madrStartAddr++;
	}

	(*pdwLen) = dwNonSize;
	(*bEnableArea) = FALSE;
	// RevNo110216-003 Append Line
	(*eAreaType) = MAREA_ETC;

	return ferr;
}

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * 指定領域が内部ROM領域であるかを判定する。
 * @param eAreaType 領域タイプ
 * @retval TRUE 内部ROM領域である
 * @retval FALSE 内部ROM領域でない
 */
//=============================================================================
BOOL IsMcuInRomArea(enum FFWRXENM_MAREA_TYPE eAreaType)
{
	BOOL	bRomArea;

	switch (eAreaType) {
	case MAREA_USER:		// no break
	case MAREA_DATA:		// no break
	case MAREA_USERBOOT:	// no break
	case MAREA_CONFIG:
		bRomArea = TRUE;
		break;

	default:	// 上記以外
		bRomArea = FALSE;
		break;
	}

	return bRomArea;
}
// RevRxNo140109-001 Append End


// RevRxNo140616-001 Modify Start
// RevRxNo120606-004 Append Start
//=============================================================================
/**
 * 指定アドレスのアクセス方法(dDMAorCPU)を判定する。
 * @param madrStartAddr チェック開始アドレス
 * @param madrEndAddr チェック終了アドレス
 * @param pdwLen 返答アクセス方法領域の開始アドレスからのバイト長
 * @param pbyAccMeans アクセス方法(DDMA_ACCESS/CPU_ACCESS)
 * @param pbyAccType アクセス属性(RX_EMU_ACCTYPE_RW/RX_EMU_ACCTYPE_R/RX_EMU_ACCTYPE_W/RX_EMU_ACCTYPE_NON)
 * @retval なし
 */
//=============================================================================
void ChkAreaAccessMethod(MADDR madrStartAddr, MADDR madrEndAddr, DWORD* pdwLen, BYTE* pbyAccMeans, BYTE* pbyAccType)
{
	DWORD	i;
	FFWMCU_MCUINFO_DATA_RX*	pMcuInfo;		// MCU情報構造体のポインタ

	pMcuInfo = GetMcuInfoDataRX();

	// pMcuInfo->dwEmuAccCtrlAreaNumが0の場合に不定値となるため、初期値を入れておく
	*pdwLen = madrEndAddr - madrStartAddr +1;
	*pbyAccMeans = static_cast<BYTE>(RX_EMU_ACCMEANS_DDMA);			// dDMAアクセス	
	*pbyAccType = static_cast<BYTE>(RX_EMU_ACCTYPE_RW);				// R/Wアクセス可

	for (i = 0; i < pMcuInfo->dwEmuAccCtrlAreaNum; i++) {
		if ((pMcuInfo->dwmadrEmuAccCtrlStart[i] <= madrStartAddr) && (madrStartAddr <= pMcuInfo->dwmadrEmuAccCtrlEnd[i])) {
			// 個別制御領域内に開始アドレスがある場合
			if (madrEndAddr <= pMcuInfo->dwmadrEmuAccCtrlEnd[i]) {
				// 個別制御領域内に終了アドレスがある場合
				// maddrStartAddr～madrEndAddrのバイトサイズを返す
				*pdwLen = madrEndAddr - madrStartAddr + 1;
			} else {
				// 個別制御領域内に終了アドレスがない場合
				// maddrStartAddr～MPU_END_ADDRのバイトサイズを返す
				*pdwLen = pMcuInfo->dwmadrEmuAccCtrlEnd[i] - madrStartAddr + 1 ;
			}
			*pbyAccMeans = static_cast<BYTE>(pMcuInfo->eEmuAccMeans[i]);	// 個別制御領域のアクセス方法
			*pbyAccType = static_cast<BYTE>(pMcuInfo->eEmuAccType[i]);		// 個別制御領域のアクセス属性
			break;

		// RevRxNo120606-004 Append Start
		} else if ((madrStartAddr < pMcuInfo->dwmadrEmuAccCtrlStart[i]) && (pMcuInfo->dwmadrEmuAccCtrlStart[i] <= madrEndAddr)) {
			// 個別制御領域内の前に開始アドレスがあって、終了アドレスが個別制御領域内かそれ以上の場合
			*pdwLen = pMcuInfo->dwmadrEmuAccCtrlStart[i] - madrStartAddr;
			*pbyAccMeans = static_cast<BYTE>(RX_EMU_ACCMEANS_DDMA);			// dDMAアクセス	
			*pbyAccType = static_cast<BYTE>(RX_EMU_ACCTYPE_RW);				// R/Wアクセス可
		// RevRxNo120606-004 Append End
			break;
		}
	}

	return;
}
// RevRxNo120606-004 Append End
// RevRxNo140616-001 Modify End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * 指定領域のすべてが比較領域に含まれているかを判定する。
 * 指定領域のすべてが比較領域に含まれていたかを返送する。 　
 * 指定領域が連続した領域のブロックをまたがっていた場合も判定可能とする。
 * @param  madrStart		開始アドレス
 * @param  madrEnd			終了アドレス
 * @param  madrCmpStart[]	比較用開始アドレス配列ポインタ
 * @param  madrCmpEnd[]     比較用終了アドレス配列ポインタ
 * @param  dwCmpCnt			比較領域数
 * @retval bIncludeArea		指定領域の包含有無結果
 */
//=============================================================================
FFWERR ChkIncludeArea(MADDR madrStart, MADDR madrEnd, MADDR* madrCmpStart, MADDR* madrCmpEnd, DWORD dwCmpCnt)
{
	DWORD	dwCnt;				// 比較領域番号
	BOOL	bIncludeArea;		// 指定領域包含フラグ

	bIncludeArea = FALSE;	// 指定領域包含フラグを初期化

	// ワークRAMアドレス判定処理
	// for文の再初期化式はなしで、ループ処理内でdwCntのインクリメントを実施する。
	// ループ処理内でdwCntを0x0に初期化して、最初から判定をやり直す場合がある。
	// for文の再初期化式に"dwCnt++"があると、0x0に初期化してもインクリメントされてしまうため、
	// 最初からやり直すことができなくなる。
	for (dwCnt = 0; dwCnt < dwCmpCnt; ) {
		if ((madrCmpStart[dwCnt] <= madrStart) && (madrStart <= madrCmpEnd[dwCnt])) {
			// 指定領域開始アドレスが比較領域開始アドレス以上且つ比較領域終了アドレス以下の場合

			if (madrEnd <= madrCmpEnd[dwCnt]) {
				// 指定領域終了アドレスが比較領域終了アドレス以下の場合

				bIncludeArea = TRUE;	// 指定領域が比較領域範囲内のため、包含フラグをTRUEにする
				break;
			} else {
				// 指定領域終了アドレスが比較領域終了アドレスを超える場合(次の領域にまたがる)

				// 次の領域判定のため、指定領域先頭アドレスと判定用カウンタを再設定
				madrStart = madrCmpEnd[dwCnt] + 1;
				dwCnt = 0x0;
			}
		} else {
			// 指定領域開始アドレスが比較領域範囲外の場合

			dwCnt++;	// カウンタをインクリメント
		}
	}

	return bIncludeArea;
}
// RevRxNo130301-001 Append End

// V.1.02 RevNo110509-001 Append Start
//=============================================================================
/**
 * PMODコマンド発行済みかどうかの情報を取得
 * @param  なし
 * @retval 現在の状態(TRUE:発行済み、FALSE：未発行)
 */
//=============================================================================
BOOL GetPmodInfo(void)
{
	return s_bPmodFlag;
}

//=============================================================================
/**
 * PMODコマンド発行済みかどうかの情報を設定
 * @param  状態(TRUE:発行済み、FALSE：未発行)
 * @retval なし
 */
//=============================================================================
void SetPmodInfo(BOOL bSet)
{
	s_bPmodFlag = bSet;

	return;
}
// V.1.02 RevNo110509-001 Append End

// V.1.02 RevNo110407-001 Append Start
//=============================================================================
/**
 * HPON OFF処理中かどうかの情報を取得
 * @param  なし
 * @retval 現在の状態(TRUE:処理中、FALSE：処理中でない)
 */
//=============================================================================
BOOL GetHponOffInfo(void)
{
	return s_bHponOff;
}

//=============================================================================
/**
 * HPON OFF処理中かどうかの情報を設定
 * @param  状態(TRUE:処理中、FALSE：処理中でない)
 * @retval なし
 */
//=============================================================================
void SetHponOffInfo(BOOL bSet)
{
	s_bHponOff = bSet;

	return;
}
// V.1.02 RevNo110407-001 Append End

// RevRxNo140617-001 Append Start
//=============================================================================
/**
 * TM機能有効無効情報を返す
 * @param  なし
 * @retval TM機能(TRUE:TM機能有効、FALSE:TM機能無効)
 */
//=============================================================================
BOOL GetTMEnable(void)
{
	return s_bTMEnable;
}

// RevRxNo161003-001 Append Start
//=============================================================================
/**
 * FFEE0000h～FFEEFFFFhのTM機能有効無効情報を返す
 * @param  なし
 * @retval TM機能(TRUE:FFEE0000h～FFEEFFFFhのTM機能有効、FALSE:FFEE0000h～FFEEFFFFhのTM機能無効)
 */
//=============================================================================
BOOL GetTMEnableDual(void)
{
	return s_bTMEnableDual;
}
// RevRxNo161003-001 Append End

//=============================================================================
/**
 * 退避しておいたTM識別データを返す
 * @param  なし
 * @retval 退避しておいたTMINFデータ
 */
//=============================================================================
DWORD GetTMINFData(void)
{
	return s_dwTMINFData;
}

//=============================================================================
/**
 * 指定アドレスがTM領域内かどうかを確認する
 * @param  なし
 * @retval 結果(TRUE:TM領域内、FALSE:TM領域外)
 */
//=============================================================================
BOOL ChkTmArea(MADDR madrAddr)
{
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	BOOL	bTmArea = FALSE;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo161003-001 Append Line

	pFwCtrl = GetFwCtrlData();		// RevRxNo161003-001 Append Line
	pMcuInfoData = GetMcuInfoDataRX();
	if( madrAddr < pMcuInfoData->dwmadrMcuRegInfoAddr[8][0] ) {			// 指定アドレス < TM領域開始アドレス
		// デュアルモードがないMCUの場合は指定アドレスはTM領域内にない
		// デュアルモードがあるMCUの場合で、リニアモードで起動している場合は指定アドレスはTM領域内にない
		// デュアルモードがあるMCUの場合で、デュアルモードで起動している場合は指定アドレスがTM領域内にある可能性がある
		// RevRxNo161003-001 Append Start
		if (pMcuInfoData->dwMcuRegInfoBlkNum == 12) {					// TM領域がもう一つある場合
			// デュアルモードが存在するMCUの場合、DO_SetRXPMOD()内TM機能有効無効チェック処理でTM領域定義を更新するため、
			// この関数が発行される時点では、dwMcuRegInfoBlkNum == 12となっているのはデュアルでFFEE0000h～FFEEFFFFhのTM機能が
			// 有効となっている場合のみである。
			if (madrAddr < pMcuInfoData->dwmadrMcuRegInfoAddr[10][0]) {			// 指定アドレス < TM領域開始アドレス
				// 指定アドレスはTM領域内にない
			} else {
				if( madrAddr <= pMcuInfoData->dwmadrMcuRegInfoAddr[11][0] ){	// 指定アドレス <= TM領域終了アドレス
					bTmArea = TRUE;		// 指定アドレスはTM領域内
				}
			}
		}
		// RevRxNo161003-001 Append End
	}else{																// 指定アドレス >= TM領域開始アドレス
		if( madrAddr <= pMcuInfoData->dwmadrMcuRegInfoAddr[9][0] ){		// 指定アドレス <= TM領域終了アドレス
			bTmArea = TRUE;		// 指定アドレスはTM領域内
		}
	}
	return bTmArea;
}

//=============================================================================
/**
 * 指定領域が比較領域と重複するかどうかを判断する
 * @param  なし
 * @retval 結果(TRUE:比較領域と重複する、FALSE:比較領域と重複しない)
 */
//=============================================================================
extern BOOL ChkAreaOverlap(MADDR madrStart, MADDR madrEnd, MADDR* madrCmpStart, MADDR* madrCmpEnd, DWORD dwCmpCnt)
{
	DWORD	i;					// 比較領域番号
	BOOL	bAreaOverlap;		// 重複確認結果フラグ

	bAreaOverlap = FALSE;		// 重複確認結果フラグを初期化
	for (i = 0; i < dwCmpCnt; i++ ) {
		if (madrEnd < madrCmpStart[i]) {		// 指定領域終了アドレス < 比較領域開始アドレス
			// 次の比較領域判断へ
		} else {								// 指定領域終了アドレス >= 比較領域開始アドレス
			if (madrStart <= madrCmpEnd[i]) {	// 指定領域開始アドレス <= 比較領域終了アドレス
				bAreaOverlap = TRUE;			// 指定領域は重複している
				break;
			}
		}
	}
	return bAreaOverlap;
}
// RevRxNo140617-001 Append End



//=============================================================================
/**
 * MCU依存コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Mcu(void)
{
	// V.1.02 RevNo110509-001 Appned Line
	SetPmodInfo(FALSE);		// PMOD未発行

	// V.1.02 RevNo110407-001 Append Line
	s_bHponOff = FALSE;

	return;
}

