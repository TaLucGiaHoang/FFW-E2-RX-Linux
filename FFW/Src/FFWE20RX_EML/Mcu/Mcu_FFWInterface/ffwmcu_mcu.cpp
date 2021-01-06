///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_mcu.cpp
 * @brief MCU依存コマンドの実装ファイル
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, H.Akashi, K.Uemori, S.Ueda, Y.Kawakami, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/02/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120206-001 　2012/07/11 橋口
　・UBコードコピー間違い修正( &pUBCode → pUBCode )
・RevNo111128-001	2012/07/11 橋口
  ・以下関数を新規追加。
      GetMdeRegData()
      SetMdeRegData()
・RevNo111121-007	2012/07/11 橋口
  ・FFWRXCmd_INSTCODE()
    ・以下のWarning を検出する為、DO_GetRXINSTCODE() 実行後にWarningChk_MemAccessErr() を
      実行するように変更。
	    FFWERR_READ_ROM_FLASHDBG_MCURUN
・RevRxNo120910-001	2012/10/18 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・FFWRXCmd_SetMCU、FFWRXCmd_GetMCU の引数dwSizeOfMcuAreaStruct、dwSizeOfMcuInfoStruct追加。
    FFWRXCmd_SetMCU、FFWRXCmd_GetMCU で、
  　V.2.00.00(本仕様の初期バージョン)時点では、"設定する構造体サイズ"が構造体サイズより小さい場合、
  　引数エラーとする。
  　"設定する構造体サイズ"が構造体サイズより大きい場合、エラー「FFWに設定可能な
  　パラメータサイズを超えていた(Warning)」を返すことを追加。
  ・FFWRXCmd_SetDBG、FFWRXCmd_GetDBG の引数dwSizeOfDbgStruct追加。
    FFWRXCmd_SetDBG、FFWRXCmd_GetDBG で、
  　V.2.00.00(本仕様の初期バージョン)時点では、"デバッグ情報を格納する構造体サイズ"が
  　構造体サイズより小さい場合、引数エラーとする。
  　"デバッグ情報を格納する構造体サイズ"が構造体サイズより大きい場合、エラー「FFWに設定可能な
  　パラメータサイズを超えていた(Warning)」を返すことを追加。
  ・FFWRXCmd_SetDBG で、使用するプログラムの引数エラーチェック変更。
  ・FFWRXCmd_GetSTAT の引数のbyStatKindをdwStatKindに変更。。
    FFWRXCmd_GetSTAT のdwStatKind以外のパラメータを、FFWRX_STAT_DATA構造体ポインタ変数に変更。
    FFWRXCmd_GetSTAT の引数dwSizeOfStatStruct追加。
  ・FFWRXCmd_GetSTAT で、
  　V.2.00.00(本仕様の初期バージョン)時点では、"ターゲットMCU状態の情報を格納する構造体サイズ"が
  　構造体サイズより小さい場合、引数エラーとする。
  　"ターゲットMCU状態の情報を格納する構造体サイズ"が構造体サイズより大きい場合、エラー
  　「FFWに設定可能なパラメータサイズを超えていた(Warning)」を返すことを追加。
  　pStat->dwSizeOfSfrStruct、 FFWRX_STAT_SFR_DATA  についても同様。
  　pStat->dwSizeOfJtagStruct、FFWRX_STAT_JTAG_DATA についても同様。
  　pStat->dwSizeOfFineStruct、FFWRX_STAT_FINE_DATA についても同様。
  　pStat->dwSizeOfEmlStruct、 FFWRX_STAT_EML_DATA  についても同様。
  ・InitFfwIfMcuData_Mcu()で、wOcdTrcCycMax の初期設定追加。
  ・FFWRXCmd_SetMCU()で、dwVdet0LevelNum の引数エラーチェック追加。
    InitFfwIfMcuData_Mcu()で、dwVdet0LevelNum の初期設定追加。
  ・FFWRXCmd_SetMCU()で、dwMcuJtagIdcodeNum、dwMcuFineDeviceCodeNum の引数エラーチェック追加。
    InitFfwIfMcuData_Mcu()で、dwMcuJtagIdcodeNum、dwMcuFineDeviceCodeNum の初期設定追加、
    かつ dwMcuJtagIdcode を dwMcuJtagIdcode[0] に変更。
  ・FFWRXCmd_SetPMOD()、FFWRXCmd_GetPMOD()で、引数 pUbAreaChange 追加。
  ・FFWRXCmd_SetMCU()で、dwEmuAccCtrlAreaNum、dwmadrEmuAccCtrlStart[]、dwmadrEmuAccCtrlEnd[] 
    の引数エラーチェック追加。
  ・FFWRXCmd_SetMCU()で、byExtMemType の引数エラーチェック追加。
    InitFfwIfMcuData_Mcu()で、byExtMemType[dwCnt] の初期設定追加。
  ・FFWRXCmd_SetDBGで、pDbg->eDbgBootSwapの引数エラーチェック追加。
    InitFfwIfMcuData_Mcu()で、s_DbgData_RX.eDbgBootSwap の初期設定追加。
  ・メンバ名 dwFcuFilmLen を dwFcuFirmLen に全て変更。
　・FFWMCUCmd_SetMCUD()、FFWMCUCmd_GetMCUD() の空の関数定義追加。
・RevNo121101-001	2012/11/05 上田
　・FFWRXCmd_SetMCU()
 　　Flash書き込みプログラム格納アドレスが4の倍数でない場合の引数エラーチェック追加。
・RevRxNo120910-005	2012/11/12 明石
  ユーザブートモード起動時のUSBブート領域非消去指定対応
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo120910-001	2012/11/12 三宅
　・DWORD FFWRXCmd_SetMCU() で、
　  pMcuArea->dwFcuFirmLen が 0x0000_0000でないとき、dwAdrFcuFirmStart, dwAdrFcuRamStartの
　  引数エラーチェックをするように変更。
・RevRxNo121106-001	2012/11/15 上田
　・FFWRXCmd_SetPMOD()にSetPmodeInRomReg2Flg()発行を追加。
・RevNo121213-001	2012/11/15 上田
　・FFWRXCmd_SetMCU()の引数エラーチェック修正。
・RevRxNo130411-001	2013/04/16 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130809-001 2013/08/21 上田
	MCU, DBG, STATコマンドのパラメータ可変長対応処理改善
・RevRxNo130308-001	2013/08/21 上田 (2013/05/20 三宅担当分マージ)
　カバレッジ開発対応
・RevRxNo130301-001 2013/09/24 上田、植盛
	RX64M対応
・RevRxNo130411-001 2014/01/21 植盛
	enum FFWRXENM_MDE_TYPE型のメンバ名変更(RX_MDE_OSM→RX_MDE_FLASH)
・RevRxNo130730-001 2014/06/20 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140515-006 2014/06/24 植盛
	RX231対応
・RevRxNo140515-005 2014/07/29 大喜多
	RX71M対応
・RevRxNo140515-012 2014/06/25 川上
	ホットプラグ起動中のカバレッジ機能エラー処理修正
・RevRxE2LNo141104-001 2014/11/20 上田
	E2 Lite対応
・RevRxNo150827-002 2015/08/28 PA 紡車
	RX651オプション設定メモリアドレス、配置変更対応
・RevRxNo161003-001 2016/11/28 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
*/
#include "ffw_sys.h"
#include "do_sys.h"
#include "ffwmcu_mcu.h"
#include "domcu_mcu.h"
#include "domcu_mem.h"
#include "domcu_prog.h"
#include "errchk.h"
#include "ffw_sys.h"
#include "domcu_rst.h"
#include "domcu_dwn.h"
#include "mcu_flash.h"					// RevNo111121-007 Append Line
#include "mcudef.h"		// RevRxNo130411-001 Append Line
#include "mcuspec.h"	// RevRxNo130301-001 Append Line
#include "ocdspec.h"	// RevRxNo140515-005 Append Line


// ファイル内static変数の宣言
//static FFW_MCUD_DATA		s_McudData;		// ターゲットMCU 空間に設定するデータ格納構造体変数
static FFWMCU_MCUAREA_DATA_RX	s_McuAreaData_RX;	// MCU空間情報格納構造体変数(RX用)
static FFWMCU_MCUINFO_DATA_RX	s_McuInfoData_RX;	// MCU情報格納構造体変数(RX用)
static FFWMCU_DBG_DATA_RX		s_DbgData_RX;		// デバッグ情報格納構造体変数(RX用)
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append Start
static enum FFWRXENM_PMODE		s_ePmode;			// ユーザ指定端子設定MCU動作モード情報格納変数
static enum FFWRXENM_RMODE		s_eRmode;			// ユーザ指定レジスタ設定MCU動作モード情報格納変数
static enum FFWENM_ENDIAN		s_eMcuEndian;		// ユーザ指定エンディアン情報格納変数
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append End
// V.1.02 新デバイス対応 Append Line
static FFWMCU_UBCODE_DATA_RX	s_UBCodeData_RX;		// UBコード情報格納構造体変数(RX用)

static enum FFWRX_STAT_ENDIAN	s_eMdeRegData;		// MDE レジスタに設定されているエンディアン情報  RevNo111128-001 Append Line

static BOOL	s_bHotPlugState;			// ホットプラグ設定状態を保持
//RevRxNo140515-012 Append Line
static BOOL s_bHotPlgStartFlg;			// ホットプラグ起動フラグ
static BOOL	s_bIdCodeResult;			// ID認証結果設定状態を保持

// V.1.02 No.4 ユーザブートモード起動対応 Apped Line
static BYTE	s_byUserSetIdCode[16];		// ユーザ設定IDコード格納用
// RevRxNo120910-005	Append Line
static FFWRX_UBAREA_CHANGEINFO_DATA	s_UBCodeDataAreaChange_RX;	// フラッシュROM上ユーザブート領域の書き換え情報

// RevRxNo130301-001 Append Line
static FFWRX_FMCD_DATA			s_Fmcd;	// フラッシュメモリコンフィギュレーションデータ情報

// RevRxNo161003-001 Append Start
static enum FFWRXENM_BANKMD		s_eBankMode;	// バンクモード情報格納変数(RX_LINEAR：リニアモード、RX_DUAL：デュアルモード)
static enum FFWRXENM_BANKSEL	s_eBankSel;		// 起動バンク情報格納変数(RX_BANKSEL_BANK0：起動バンク0、RX_BANKSEL_BANK1：起動バンク1、RX_BANKCONT：起動バンク継続)
// RevRxNo161003-001 Append End

// 関数宣言
// RevRxNo120910-005	Append Start
// RevRxNo130730-001 Delete
void SetUbAreaChangeRX(const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange);
// RevRxNo120910-005	Append End
static void setMcuRegInfoData(FFWRX_MCUINFO_DATA* pMcuInfo);	// RevRxNo130301-001 Append Line

///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * MCU情報を設定する。
 * @param dwSizeOfMcuAreaStruct 設定するFFWRX_MCUAREA_DATA構造体のサイズ	// RevRxNo120910-001 Append Line
 * @param pMcuArea MCU空間情報格納構造体のアドレス
 * @param dwSizeOfMcuInfoStruct 設定するFFWRX_MCUINFO_DATA構造体のサイズ	// RevRxNo120910-001 Append Line
 * @param pMcuInfo エミュレータ制御に必要なMCU情報格納構造体のアドレス
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
// RevRxNo120910-001 Modify Start
FFWE100_API	DWORD FFWRXCmd_SetMCU(DWORD dwSizeOfMcuAreaStruct, const FFWRX_MCUAREA_DATA* pMcuArea, 
								  DWORD dwSizeOfMcuInfoStruct, const FFWRX_MCUINFO_DATA* pMcuInfo)
// RevRxNo120910-001 Modify End
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwCnt;
	// V.1.02 RevNo110330-001 Append Line
	DWORD	dwCnt2;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	DWORD	dwFcuSetLen;
	// RevRxNo130301-001 Delete
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Line
	MADDR	madrEnd;

	// RevRxNo130809-001 Append Start
	FFWRX_MCUAREA_DATA	mcuAreaTmp;				// MCU空間情報の一時格納用変数
	FFWRX_MCUINFO_DATA	mcuInfoTmp;				// エミュレータ制御に必要なMCU情報の一時格納用変数
	BOOL	bOverStructSize;					// 構造体サイズが超えているかを示すフラグ
	DWORD	dwSetSize;
	// RevRxNo130809-001 Append End
	// RevRxNo130301-001 Append Start
	MADDR	madrRamWorkStart;					// ワークRAM先頭アドレス
	MADDR	madrRamWorkEnd;						// ワークRAM終了アドレス
	MADDR	madrCmpStart[MCU_AREANUM_MAX_RX];	// 比較用先頭アドレス
	MADDR	madrCmpEnd[MCU_AREANUM_MAX_RX];		// 比較用終了アドレス
	DWORD	dwCmpCnt;
	BOOL	bWorkRamInRam;						// ワークRAM領域包含フラグ
	// RevRxNo130301-001 Append End
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuDef = GetMcuDefData();
	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}


	// RevRxNo130809-001 Modify Start
	bOverStructSize = FALSE;	// 構造体サイズが超えているかを示すフラグの初期化

	// RevRxNo120910-001 Append Start
	// "設定するFFWRX_MCUAREA_DATA構造体サイズ"の引数エラーチェック
	if (dwSizeOfMcuAreaStruct < sizeof(FFWRX_MCUAREA_DATA)) {
		return FFWERR_FFW_ARG;
	}
	if (dwSizeOfMcuAreaStruct > sizeof(FFWRX_MCUAREA_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}
	
	// "設定するFFWRX_MCUAREA_DATA構造体サイズ"の引数エラーチェック
	// RevRxE2LNo141104-001 Append Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		if (dwSizeOfMcuInfoStruct < FFWRX_MCUINFO_DATA_SIZE_V20300) {
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxE2LNo141104-001 Append End
	if (dwSizeOfMcuInfoStruct < sizeof(FFWRX_MCUINFO_DATA)) {
		// 旧バージョンの構造体サイズであるかをチェック
		// RevRxNo140515-005 Modify Line
		if ((dwSizeOfMcuInfoStruct != FFWRX_MCUINFO_DATA_SIZE_V20000) && 
				(dwSizeOfMcuInfoStruct != FFWRX_MCUINFO_DATA_SIZE_V20200)) {	// V.2.00.00の構造体サイズでも、V.2.02.00の構造体サイズでもない場合
			return FFWERR_FFW_ARG;
		}
	}
	if (dwSizeOfMcuInfoStruct == FFWRX_MCUINFO_DATA_SIZE_V20000) {
		// V.2.00.00の構造体サイズを指定された場合、MCU_TYPE_0, 1, 2, 3以外はエラー
		switch (pMcuInfo->byMcuType) {
		case MCU_TYPE_0:	// no break
		case MCU_TYPE_1:	// no break
		case MCU_TYPE_2:	// no break
		case MCU_TYPE_3:
			break;
		default:			// MCU_TYPE_0,1,2,3 以外
			ferr =  FFWERR_FFW_ARG;
		}
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	if (dwSizeOfMcuInfoStruct > sizeof(FFWRX_MCUINFO_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}
	// RevRxNo120910-001 Append End
	// RevRxNo130809-001 Modify End


	// SFR領域の引数エラーチェック
	if (pMcuArea->dwSfrAreaNum > MCU_AREANUM_MAX_RX) {
		return FFWERR_FFW_ARG;
	}
	for (dwCnt = 0; dwCnt < pMcuArea->dwSfrAreaNum; dwCnt++) {
		if (pMcuArea->dwmadrSfrStartAddr[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		if (pMcuArea->dwmadrSfrEndAddr[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		if ((pMcuArea->bySfrEndian[dwCnt] != FFWRX_ENDIAN_LITTLE) && (pMcuArea->bySfrEndian[dwCnt] != FFWRX_ENDIAN_BIG)) {
			return FFWERR_FFW_ARG;
		}
		if (pMcuArea->dwmadrSfrStartAddr[dwCnt] > pMcuArea->dwmadrSfrEndAddr[dwCnt]) {
			return FFWERR_FFW_ARG;
		}
	}

	// MCU内部RAM領域の引数エラーチェック
	if (pMcuArea->dwRamAreaNum > MCU_AREANUM_MAX_RX) {
		return FFWERR_FFW_ARG;
	}
	for (dwCnt = 0; dwCnt < pMcuArea->dwRamAreaNum; dwCnt++) {
		if (pMcuArea->dwmadrRamStartAddr[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		if (pMcuArea->dwmadrRamEndAddr[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		if (pMcuArea->dwmadrRamStartAddr[dwCnt] > pMcuArea->dwmadrRamEndAddr[dwCnt]) {
			return FFWERR_FFW_ARG;
		}
	}

	// MCU内部FlashROM領域の引数エラーチェック
	if (pMcuArea->dwmadrFlashRomWriteStart > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (pMcuArea->dwmadrFlashRomReadStart > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Line
	if (pMcuArea->dwFlashRomPatternNum > MCU_AREAPTNUM_MAX) {
		return FFWERR_FFW_ARG;
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Start
	for (dwCnt = 0; dwCnt < pMcuArea->dwFlashRomPatternNum; dwCnt++ ) {
		if (pMcuArea->dwmadrFlashRomStart[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		madrEnd = pMcuArea->dwmadrFlashRomStart[dwCnt] + (pMcuArea->dwFlashRomBlockSize[dwCnt] * pMcuArea->dwFlashRomBlockNum[dwCnt]) -1;
		if (madrEnd > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify End

	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Start
	if (pMcuArea->dwDataFlashRomPatternNum > MCU_AREAPTNUM_MAX) {
		return FFWERR_FFW_ARG;
	}
	for (dwCnt = 0; dwCnt < pMcuArea->dwDataFlashRomPatternNum; dwCnt++ ) {
		if (pMcuArea->dwmadrDataFlashRomStart[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		madrEnd = pMcuArea->dwmadrDataFlashRomStart[dwCnt] + (pMcuArea->dwDataFlashRomBlockSize[dwCnt] * pMcuArea->dwDataFlashRomBlockNum[dwCnt]) -1;
		if (madrEnd > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Append End

	// MCUの外部メモリ領域の引数エラーチェック
	if (pMcuArea->dwExtMemBlockNum > MCU_AREANUM_MAX_RX) {
		return FFWERR_FFW_ARG;
	}
	for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
		if (pMcuArea->dwmadrExtMemBlockStart[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		if (pMcuArea->dwmadrExtMemBlockEnd[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		if ((pMcuArea->byExtMemEndian[dwCnt] != FFWRX_ENDIAN_LITTLE) && (pMcuArea->byExtMemEndian[dwCnt] != FFWRX_ENDIAN_BIG)) {
			return FFWERR_FFW_ARG;
		}
		// RevRxNo120910-001 Append Start
		if ((pMcuArea->byExtMemType[dwCnt] != MCU_EXT_MEM_CS) && (pMcuArea->byExtMemType[dwCnt] != MCU_EXT_MEM_SDRAM)) {
			return FFWERR_FFW_ARG;
		}
		// RevRxNo120910-001 Append End
		if (pMcuArea->dwmadrExtMemBlockStart[dwCnt] > pMcuArea->dwmadrExtMemBlockEnd[dwCnt]) {
			return FFWERR_FFW_ARG;
		}
	}

	// FCUファームウェアサイズのチェック
	// RevRxNo120910-001 Modify Line
	if (pMcuArea->dwFcuFirmLen > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}

	// RevRxNo120910-001 Modify Start
	if (pMcuArea->dwFcuFirmLen != 0x00000000) {		// pMcuArea->dwFcuFirmLen が 0x0000_0000のとき、dwAdrFcuFirmStart, dwAdrFcuRamStartの内容は無効。
		// FCUファームウェア格納アドレスのチェック
		if (pMcuArea->dwAdrFcuFirmStart > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}

		// FCUファームウェア格納可否チェック
		dwFcuSetLen = pMcuDef->madrMaxAddr - pMcuArea->dwAdrFcuFirmStart;
		// RevRxNo120910-001 Modify Line
		if (pMcuArea->dwFcuFirmLen > dwFcuSetLen) {
			return FFWERR_FFW_ARG;
		}

		// FCU-RAMアドレスのチェック
		if (pMcuArea->dwAdrFcuRamStart > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxNo120910-001 Modify End

// V.1.02 No.5 ユーザブート/USBブート対応 Append Start
	// ユーザブート領域先頭アドレスのチェック
	if (pMcuArea->dwmadrUserBootStart > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}

	// ユーザブート領域終了アドレスのチェック
	if (pMcuArea->dwmadrUserBootEnd > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
// V.1.02 No.5 ユーザブート/USBブート対応 Append End

	// リセット時ISPレジスタ初期値のチェック
	if (pMcuInfo->dwadrIspOnReset > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}

	// Flash書き込みプログラム格納アドレスのチェック
	if (pMcuInfo->dwadrWorkRam > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	// RevNo121101-001 Modify Line
	if (pMcuInfo->dwadrWorkRam % 4) {	// 4の倍数以外を指定時
		return FFWERR_FFW_ARG;
	}

	// RevRxNo130301-001 Modify Start
	// ワークRAM領域指定範囲のチェック
	// MCUコマンドで指定された内蔵RAMの先頭/終了アドレスを比較用配列に格納
	for (dwCnt = 0; dwCnt < pMcuArea->dwRamAreaNum; dwCnt++) {
		madrCmpStart[dwCnt] = pMcuArea->dwmadrRamStartAddr[dwCnt];
		madrCmpEnd[dwCnt] = pMcuArea->dwmadrRamEndAddr[dwCnt];
	}
	// MCUコマンドで指定された内蔵RAMブロック数を確保
	dwCmpCnt = pMcuArea->dwRamAreaNum;

	// ワークRAM先頭/終了アドレスを格納
	madrRamWorkStart = pMcuInfo->dwadrWorkRam;
	madrRamWorkEnd = pMcuInfo->dwadrWorkRam + pMcuInfo->dwsizWorkRam - 1;

	// ワークRAM領域がRAM領域の範囲内であるかを確認
	bWorkRamInRam = ChkIncludeArea(madrRamWorkStart, madrRamWorkEnd, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);

	if (bWorkRamInRam == FALSE) {	// RAM領域範囲外の場合、引数エラーでreturnする
		return FFWERR_FFW_ARG;
	}
	// RevRxNo130301-001 Modify End

	// トレースデータ端子出力エッジのチェック
	if ((pMcuInfo->eEdge < TRC_EDGE_SINGLE) || (pMcuInfo->eEdge > TRC_EDGE_DOUBLE)) {
		return FFWERR_FFW_ARG;
	}

	// トレースクロック比のチェック
	if ((pMcuInfo->eRatio < TRC_RATIO_1_1) ||( pMcuInfo->eRatio > TRC_RATIO_4_1)) {
		return FFWERR_FFW_ARG;
	}

	// トレースクロック動作周波数のチェック
	// V.1.02 No.32 PLL使用有無に依存したトレースクロックの設定 Modify Line
	if ((pMcuInfo->eExTAL < TRC_EXTAL_X4) || (pMcuInfo->eExTAL > TRC_EXTAL_ENA)) {
		return FFWERR_FFW_ARG;
	}

	// アキュムレータ有無のチェック
	if ((pMcuInfo->eAccSet < ACC_SET_OFF) || (pMcuInfo->eAccSet > ACC_SET_ON)) {
		return FFWERR_FFW_ARG;
	}

	// コプロセッサ有無のチェック
	if ((pMcuInfo->eCoProSet < COPRO_SET_OFF) || (pMcuInfo->eCoProSet > COPRO_SET_ON)) {
		return FFWERR_FFW_ARG;
	}

	// SPC変数のチェック
	// RevRxNo130301-001 Delete

	//入力クロックの無指定チェック Extal未使用のときは周波数チェックをしない
	//pMcuInfo->fClkExtal = 12.5;
	// V.1.02 No.32 PLL使用有無に依存したトレースクロックの設定 Modify Line
	if(pMcuInfo->eExTAL != TRC_EXTAL_DIS){
		if (pMcuInfo->fClkExtal == 0.0) {
			return FFWERR_FFW_ARG;
		}
	}
	//IDコード格納領域のアドレスチェック
	if (pMcuInfo->dwmadrIdcodeStart > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if ((pMcuInfo->dwmadrIdcodeStart + pMcuInfo->byIdcodeSize) > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	// RevRxNo120910-001 Append Start
	//対象デバイスJTAG IDコード設定数チェック
	if (pMcuInfo->dwMcuJtagIdcodeNum > MCU_OCD_ID_MAX) {
		return FFWERR_FFW_ARG;
	}
	//対象デバイスFINE デバイスコード設定数チェック
	if (pMcuInfo->dwMcuFineDeviceCodeNum > MCU_OCD_ID_MAX) {
		return FFWERR_FFW_ARG;
	}
	// RevRxNo120910-001 Append End

	// MCU種別のチェック
	// RevRxNo121026-001, RevRxNo130301-001 Modify Line
	if( pMcuInfo->byMcuType > MCU_TYPE_4 ){
		return FFWERR_FFW_ARG;
	}

	// I/F種別のチェック
	if( pMcuInfo->byInterfaceType > IF_TYPE_1 ){
		return FFWERR_FFW_ARG;
	}

	// MCUレジスタ情報定義ブロック数のチェック
	if( pMcuInfo->dwMcuRegInfoBlkNum > MCU_REGBLKNUM_MAX ){	// RevNo121213-001 Modify Line
		return FFWERR_FFW_ARG;
	}

	for (dwCnt = 0; dwCnt < pMcuInfo->dwMcuRegInfoBlkNum; dwCnt++) {
		// V.1.02 RevNo110330-001 Modify Start
		// RevRxNo130301-001 Modify Start
		// MCUレジスタ情報定義レジスタ数のチェック
		if( pMcuInfo->dwMcuRegInfoNum[dwCnt] > MCU_REGADDRNUM_MAX ){	// RevNo121213-001 Modify Line
			return FFWERR_FFW_ARG;
		}
		for( dwCnt2 = 0; dwCnt2 < pMcuInfo->dwMcuRegInfoNum[dwCnt]; dwCnt2++ ){
			// MCUレジスタ情報定義アクセスサイズのチェック
			if( pMcuInfo->eMcuRegInfoAccessSize[dwCnt][dwCnt2] > MLWORD_ACCESS ){
				return FFWERR_FFW_ARG;
			}
		}
		// RevRxNo130301-001 Modify End
		// V.1.02 RevNo110330-001 Modify End
	}

	// RevRxNo120910-001 Append Start
	// 電圧検出0レベル設定数のチェック
	if( pMcuInfo->dwVdet0LevelNum > MCU_VDET0LEVELNUM_MAX ){
		return FFWERR_FFW_ARG;
	}
	// RevRxNo120910-001 Append End

	// RevRxNo120910-001 Append Start
	// 個別アクセス制御領域数のチェック
	if( pMcuInfo->dwEmuAccCtrlAreaNum > MCU_ACCCTRL_AREANUM_MAX ){
		return FFWERR_FFW_ARG;
	}
	// 個別アクセス制御領域 開始アドレス、終了アドレスのチェック
	for (dwCnt = 0; dwCnt < pMcuInfo->dwEmuAccCtrlAreaNum; dwCnt++) {
		if (pMcuInfo->dwmadrEmuAccCtrlStart[dwCnt] > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
		if (pMcuInfo->dwmadrEmuAccCtrlEnd[dwCnt]   > pMcuDef->madrMaxAddr) {
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxNo120910-001 Append End

	// RevRxNo130301-001 Append Start
	// V.2.02.00以降で有効な引数のエラーチェック
	if (dwSizeOfMcuInfoStruct >= FFWRX_MCUINFO_DATA_SIZE_V20200) {
		// 命令セットアーキテクチャの引数エラーチェック
		// RevRxNo180228-001 Modify Line
		if ((pMcuInfo->eCpuIsa != RX_ISA_RXV1) && (pMcuInfo->eCpuIsa != RX_ISA_RXV2) && (pMcuInfo->eCpuIsa != RX_ISA_RXV3)) {
			return FFWERR_FFW_ARG;
		}
		// FPU有無の引数エラーチェック
		if ((pMcuInfo->eFpuSet != RX_FPU_OFF) && (pMcuInfo->eFpuSet != RX_FPU_ON)) {
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxNo130301-001 Append Start

	// RevRxNo130809-001 Append Start
	// FFW I/Fで設定されたMCU別情報を一時変数に格納
	if (dwSizeOfMcuAreaStruct < sizeof(FFWRX_MCUAREA_DATA)) {
		dwSetSize = dwSizeOfMcuAreaStruct;
	} else {
		dwSetSize = sizeof(FFWRX_MCUAREA_DATA);
	}
	memcpy((void*)&mcuAreaTmp, (const void*)pMcuArea, dwSetSize);

	if (dwSizeOfMcuInfoStruct < sizeof(FFWRX_MCUINFO_DATA)) {
		dwSetSize = dwSizeOfMcuInfoStruct;
	} else {
		dwSetSize = sizeof(FFWRX_MCUINFO_DATA);
	}
	memcpy((void*)&mcuInfoTmp, (const void*)pMcuInfo, dwSetSize);

	// RevRxNo140515-005 Append Start
	if (dwSizeOfMcuInfoStruct >= FFWRX_MCUINFO_DATA_SIZE_V20300) {
		if (mcuInfoTmp.fClkIclk == 0.0) {		// ICLK周波数が0.0の場合、MCUタイプに応じて初期値を入れる
			if ((pMcuInfo->byMcuType == MCU_TYPE_0) || (pMcuInfo->byMcuType == MCU_TYPE_1)) {
				mcuInfoTmp.fClkIclk = MCU_FCLKICLK_100MHz;
			} else if (pMcuInfo->byMcuType == MCU_TYPE_2) {
				mcuInfoTmp.fClkIclk = MCU_FCLKICLK_50MHz;
			} else if (pMcuInfo->byMcuType == MCU_TYPE_3) {
				mcuInfoTmp.fClkIclk = MCU_FCLKICLK_32MHz;
			} else if (pMcuInfo->byMcuType == MCU_TYPE_4) {
				mcuInfoTmp.fClkIclk = MCU_FCLKICLK_120MHz;
			}
		}
	}
	// RevRxNo140515-005 Append End

	// 省略可能なパラメータの設定
	if (dwSizeOfMcuInfoStruct < FFWRX_MCUINFO_DATA_SIZE_V20200) {
		// 命令セットアーキテクチャ
		mcuInfoTmp.eCpuIsa = RX_ISA_RXV1;

		// FPU有無
		if ((pMcuInfo->byMcuType == MCU_TYPE_0) || (pMcuInfo->byMcuType == MCU_TYPE_1)) {
			mcuInfoTmp.eFpuSet = RX_FPU_ON;
		} else {
			mcuInfoTmp.eFpuSet = RX_FPU_OFF;
		}
	}
	// RevRxNo130809-001 Append End

	// RevRxNo140515-005 Append Start
	if (dwSizeOfMcuInfoStruct < FFWRX_MCUINFO_DATA_SIZE_V20300) {
		if( (pMcuInfo->byMcuType == MCU_TYPE_0) || (pMcuInfo->byMcuType == MCU_TYPE_1) ){
			mcuInfoTmp.fClkIclk = MCU_FCLKICLK_100MHz;
		}else if( pMcuInfo->byMcuType == MCU_TYPE_2 ){
			mcuInfoTmp.fClkIclk = MCU_FCLKICLK_50MHz;
		}else if( pMcuInfo->byMcuType == MCU_TYPE_3 ){
			mcuInfoTmp.fClkIclk = MCU_FCLKICLK_32MHz;
		}else if( pMcuInfo->byMcuType == MCU_TYPE_4 ){
			mcuInfoTmp.fClkIclk = MCU_FCLKICLK_120MHz;
		}
	}
	// RevRxNo140515-005 Append End

	// 指定されていないMCUレジスタ情報の設定
	setMcuRegInfoData(&mcuInfoTmp);	// RevRxNo130301-001 Append Line


	ferr = DO_SetRXMCU(&mcuAreaTmp, &mcuInfoTmp);	// RevRxNo130809-001 Modify Line

	if (ferr == FFWERR_OK) {
		// FFW内部管理構造体変数の更新
		// RevRxNo130809-001 Modify Start
		memcpy( (void*)&s_McuAreaData_RX, (const void*)&mcuAreaTmp, sizeof(FFWRX_MCUAREA_DATA) );
		memcpy( (void*)&s_McuInfoData_RX, (const void*)&mcuInfoTmp, sizeof(FFWRX_MCUINFO_DATA) );
		// RevRxNo130809-001 Modify End

		// RevRxNo130411-001 Append Start
		SetFwCtrlData();	// MCU個別制御情報の設定
		SetMcuDefData();	// MCU固有情報の設定
		// RevRxNo130411-001 Append End
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	// RevRxNo120910-001 Append Start
	if(ferr == FFWERR_OK){
		if (bOverStructSize == TRUE) {	// RevRxNo130809-001 Modify Line
			return FFWERR_FFWPARA_OVER;		// FFWに設定可能なパラメータサイズを超えていた(Warning)//
		}
	}
	// RevRxNo120910-001 Append End

	return ferr;
}

//=============================================================================
/**
 * MCU情報を参照する。
 * @param pMcuArea MCU空間情報格納構造体のアドレス
 * @param pMcuInfo エミュレータ制御に必要なMCU情報格納構造体のアドレス
 * @retval	FFWエラーコード 
 */
//=============================================================================
// RevRxNo120910-001 Modify Start
FFWE100_API	DWORD FFWRXCmd_GetMCU(DWORD dwSizeOfMcuAreaStruct, FFWRX_MCUAREA_DATA* pMcuArea, 
								  DWORD dwSizeOfMcuInfoStruct, FFWRX_MCUINFO_DATA* pMcuInfo)
// RevRxNo120910-001 Modify End
{
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo130809-001 Append Start
	FFWRX_MCUAREA_DATA	mcuAreaTmp;				// MCU空間情報の一時格納用変数
	FFWRX_MCUINFO_DATA	mcuInfoTmp;				// エミュレータ制御に必要なMCU情報の一時格納用変数
	BOOL	bOverStructSize;					// 構造体サイズが超えているかを示すフラグ
	DWORD	dwSetSize;
	// RevRxNo130809-001 Append End
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxNo130809-001 Modify Start
	bOverStructSize = FALSE;	// 構造体サイズが超えているかを示すフラグの初期化

	// RevRxNo120910-001 Append Start
	// "参照するFFWRX_MCUAREA_DATA構造体サイズ"の引数エラーチェック
	if (dwSizeOfMcuAreaStruct < sizeof(FFWRX_MCUAREA_DATA)) {
		return FFWERR_FFW_ARG;
	}
	if (dwSizeOfMcuAreaStruct > sizeof(FFWRX_MCUAREA_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}

	// "参照するFFWRX_MCUINFO_DATA構造体サイズ"の引数エラーチェック
	// RevRxE2LNo141104-001 Append Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		if (dwSizeOfMcuInfoStruct < FFWRX_MCUINFO_DATA_SIZE_V20300) {
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxE2LNo141104-001 Append End
	if (dwSizeOfMcuInfoStruct < sizeof(FFWRX_MCUINFO_DATA)) {
		// 旧バージョンの構造体サイズであるかをチェック
		// RevRxNo140515-005 Modify Line
		if ((dwSizeOfMcuInfoStruct != FFWRX_MCUINFO_DATA_SIZE_V20000) && 
			(dwSizeOfMcuInfoStruct != FFWRX_MCUINFO_DATA_SIZE_V20200)) {	// V.2.00.00の構造体サイズでも、V.2.02.00の構造体サイズでもない場合
			return FFWERR_FFW_ARG;
		}
	
	}
	if (dwSizeOfMcuInfoStruct == FFWRX_MCUINFO_DATA_SIZE_V20000) {
		// V.2.00.00の構造体サイズを指定された場合、MCU_TYPE_0, 1, 2, 3以外はエラー
		switch (s_McuInfoData_RX.byMcuType) {
			// 参照時は、FFW内部変数で判断する。
		case MCU_TYPE_0:	// no break
		case MCU_TYPE_1:	// no break
		case MCU_TYPE_2:	// no break
		case MCU_TYPE_3:
			break;
		default:			// MCU_TYPE_0,1,2,3 以外
			ferr =  FFWERR_FFW_ARG;
		}
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	if (dwSizeOfMcuInfoStruct > sizeof(FFWRX_MCUINFO_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}
	// RevRxNo120910-001 Append End
	// RevRxNo130809-001 Modify End

	// RevRxNo130809-001 Modify Start
	memcpy( (void*)&mcuAreaTmp, (const void*)&s_McuAreaData_RX, sizeof(FFWRX_MCUAREA_DATA) );
	memcpy( (void*)&mcuInfoTmp, (const void*)&s_McuInfoData_RX, sizeof(FFWRX_MCUINFO_DATA) );
	// RevRxNo130809-001 Modify End

// RevRxNo161003-001 Delete Line
// DO_SetRXPMOD()のTM機能有無チェックで、デュアルモードが存在するMCUの場合、MCUレジスタ定義情報を
// TM機能状態に合わせて更新する。そのため、MCUファイルで指定した情報を保持しているBFWからMCUファイル
// 情報を取得すると、DO_SetRXPMOD()で更新したMCUレジスタ定義情報が元に戻ってしまう。したがってBFWから
// 情報を取得しないようにする。
//	ferr = DO_GetRXMCU(&mcuAreaTmp, &mcuInfoTmp);	// RevRxNo130809-001 Modify Line

	if(ferr == FFWERR_OK){
		// RevRxNo130809-001 Append Start
		// FFW I/Fで設定された変数に一時変数の内容を格納
		if (dwSizeOfMcuAreaStruct < sizeof(FFWRX_MCUAREA_DATA)) {
			dwSetSize = dwSizeOfMcuAreaStruct;
		} else {
			dwSetSize = sizeof(FFWRX_MCUAREA_DATA);
		}
		memcpy((void*)pMcuArea, (const void*)&mcuAreaTmp, dwSetSize);

		if (dwSizeOfMcuInfoStruct < sizeof(FFWRX_MCUINFO_DATA)) {
			dwSetSize = dwSizeOfMcuInfoStruct;
		} else {
			dwSetSize = sizeof(FFWRX_MCUINFO_DATA);
		}
		memcpy((void*)pMcuInfo, (const void*)&mcuInfoTmp, dwSetSize);
		// RevRxNo130809-001 Append End

		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	// RevRxNo120910-001 Append Start
	if(ferr == FFWERR_OK){
		if (bOverStructSize == TRUE) {	// RevRxNo130809-001 Modify Line
			return FFWERR_FFWPARA_OVER;		// FFWに設定可能なパラメータサイズを超えていた(Warning)
		}
	}
	// RevRxNo120910-001 Append End

	return ferr;
}

//=============================================================================
/**
 * デバッグ情報を設定する。
 * @param pDbg デバッグ情報格納構造体のアドレス
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
// RevRxNo120910-001 Modify Line
FFWE100_API	DWORD FFWRXCmd_SetDBG(DWORD dwSizeOfDbgStruct, const FFWRX_DBG_DATA* pDbg)
{
	FFWERR	ferr = FFWERR_OK;
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	// RevRxNo130809-001 Append Start
	FFWRX_DBG_DATA	dbgDataTmp;			// デバッグ情報の一時格納用変数
	BOOL	bOverStructSize;			// 構造体サイズが超えているかを示すフラグ
	DWORD	dwSetSize;
	// RevRxNo130809-001 Append End
	// RevRxNo130308-001 Append Line
	FFWE20_EINF_DATA			einfData;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// RevRxNo130308-001 Append Line
	getEinfData(&einfData);				// エミュレータ情報取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}

	// 引数エラーチェック

	// RevRxNo130809-001 Modify Start
	bOverStructSize = FALSE;	// 構造体サイズが超えているかを示すフラグの初期化

	// RevRxNo120910-001 Append Start
	// "デバッグ情報を格納するFFWRX_DBG_DATA構造体のサイズ"の引数エラーチェック（未満時）
	if (dwSizeOfDbgStruct < sizeof(FFWRX_DBG_DATA)) {
		return FFWERR_FFW_ARG;
	}
	// RevRxNo120910-001 Append End
	if (dwSizeOfDbgStruct > sizeof(FFWRX_DBG_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}
	// RevRxNo130809-001 Modify End

	// OCD関連の組合せ制限エラー検出有無
	if ((pDbg->eOcdArgErrChk != OCD_ARGERRCHK_NOUSE) && (pDbg->eOcdArgErrChk != OCD_ARGERRCHK_USE)) {
		return FFWERR_FFW_ARG;
	}

	// コア指定有無
	if ((pDbg->eMcuMultiCore != EML_MCU_MULTI_NON) && (pDbg->eMcuMultiCore != EML_MCU_MULTI_USE)) {
		return FFWERR_FFW_ARG;
	}

	// トレース利用機能
	//
	if ((pDbg->eTrcFuncMode != EML_TRCFUNC_TRC) && (pDbg->eTrcFuncMode != EML_TRCFUNC_CV)) 
	{
		return FFWERR_FFW_ARG;
	}

	// ROMコレクション利用
	if ((pDbg->eRomCorrection != EML_ROM_CORRECT_NON) && (pDbg->eRomCorrection != EML_ROM_CORRECT_USE)) {
		return FFWERR_FFW_ARG;
	}

	// 通常モード/ライタモード指定
	if ((pDbg->eWriter != EML_WRITERMODE_NON) && (pDbg->eWriter != EML_WRITERMODE_USE)) {
		return FFWERR_FFW_ARG;
	}

	// 使用するプログラム
	if ((pDbg->eUseProgram != EML_HEW) && (pDbg->eUseProgram != EML_FDT) &&
		(pDbg->eUseProgram != EML_RXSTICK) && (pDbg->eUseProgram != EML_MONITOR) && (pDbg->eUseProgram != EML_EZ_CUBE)) {	// RevRxNo120910-001 Modify Line
		return FFWERR_FFW_ARG;
	}

	// Flash書き換えデバッグ
	if ((pDbg->eDbgFlashWrite != EML_DBG_FLASHWRITE_NON) && (pDbg->eDbgFlashWrite != EML_DBG_FLASHWRITE_USE)) {
		return FFWERR_FFW_ARG;
	}

	// V.1.02 No.26 データフラッシュCPU書き換えデバッグ対象化 Append Start
	// DataFlash書き換えデバッグ
	if ((pDbg->eDbgDataFlashWrite != EML_DBG_DATA_FLASHWRITE_NON) && (pDbg->eDbgDataFlashWrite != EML_DBG_DATA_FLASHWRITE_USE)) {
		return FFWERR_FFW_ARG;
	}
	// V.1.02 No.26 データフラッシュCPU書き換えデバッグ対象化 Append End

	// プログラム実行前設定の有無
	if ((pDbg->eBfrRunRegSet != EML_RUNREGSET_NON) && (pDbg->eBfrRunRegSet != EML_RUNREGSET_USE)) {
		return FFWERR_FFW_ARG;
	}

	// プログラム実行後設定の有無
	if ((pDbg->eAftRunRegSet != EML_RUNREGSET_NON) && (pDbg->eAftRunRegSet != EML_RUNREGSET_USE)) {
		return FFWERR_FFW_ARG;
	}
	
	// V.1.02 I/F仕様書変更対応(OCD_ACSMODE_DUMP指定追加) Modify Line
	// 制御レジスタへのアクセスモード指定
	if ((pDbg->eOcdRegAcsMode != OCD_ACSMODE_NONDUMPFILL) && (pDbg->eOcdRegAcsMode != OCD_ACSMODE_FILL) &&
		(pDbg->eOcdRegAcsMode != OCD_ACSMODE_DUMPFILL) && (pDbg->eOcdRegAcsMode != OCD_ACSMODE_DUMP)) {
		return FFWERR_FFW_ARG;
	}

	// 同期/非同期通信指定
	if ((pDbg->eSciIF != SCI_NON) && (pDbg->eSciIF != SCI_SCI) &&
		(pDbg->eSciIF != SCI_UART)) {
		return FFWERR_FFW_ARG;
	}

	// V.1.02 No.13 ダウンロード時のクロック切り替え Append Start
	// クロック切り替え指定
	if ((pDbg->eClkChangeEna != RX_CLKCHANGE_DIS) && (pDbg->eClkChangeEna != RX_CLKCHANGE_ENA) ) {
		return FFWERR_FFW_ARG;
	}
	// V.1.02 No.13 ダウンロード時のクロック切り替え Append End

	// RevRxNo120910-001 Append Start
	// ブートスワッププログラムのデバッグ指定
	// RevRxNo130411-001 Modify Line, RevRxNo140515-006 Modify Line
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
		// フラッシュがMF3またはSC32用MF3の場合

		if ((pDbg->eDbgBootSwap != RX_DBG_BOOTSWAP_NON) && (pDbg->eDbgBootSwap != RX_DBG_BOOTSWAP_USE) ) {
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxNo120910-001 Append End

	// RevRxNo130308-001 Append Start
	if (s_bHotPlgStartFlg == TRUE) {			// ホットプラグ起動状態中の場合		RevRxNo140515-012 Modify Line
		if (pDbg->eTrcFuncMode == EML_TRCFUNC_CV) {		// "カバレッジ用に利用"の場合
			return FFWERR_CV_UNSUPPORT;		// エラー"カバレッジ機能はサポートしていない。"
		}
	}
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN) && (pFwCtrl->bCoverageFunc == TRUE))) {
		// "E20エミュレータ、38ピンケーブル接続時、かつトレースの利用機能に"カバレッジ用に利用"を設定可能"以外の場合
		if (pDbg->eTrcFuncMode == EML_TRCFUNC_CV) {		// "カバレッジ用に利用"の場合
			return FFWERR_CV_UNSUPPORT;		// エラー"カバレッジ機能はサポートしていない。"
		}
	}
	// RevRxNo130308-001 Append End
	
	// RevRxNo130809-001 Append Start
	// FFW I/Fで設定されたMCU別情報を一時変数に格納
	if (dwSizeOfDbgStruct < sizeof(FFWRX_DBG_DATA)) {
		dwSetSize = dwSizeOfDbgStruct;
	} else {
		dwSetSize = sizeof(FFWRX_DBG_DATA);
	}
	memcpy((void*)&dbgDataTmp, (const void*)pDbg, dwSetSize);

	// 省略可能なパラメータの設定
	// 　現状なし

	// RevRxNo130809-001 Append End

	ferr = DO_SetRXDBG(&dbgDataTmp);	// RevRxNo130809-001 Modify Line
	if (ferr == FFWERR_OK) {
		// FFW内部管理構造体変数の更新
		memcpy(&s_DbgData_RX, &dbgDataTmp, sizeof(FFWMCU_DBG_DATA_RX));	// RevRxNo130809-001 Modify Line
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	// RevRxNo120910-001 Append Start
	// "デバッグ情報を格納するFFWRX_DBG_DATA構造体のサイズ"の引数エラーチェック（越える場合）について
	// dwSizeOfDbgStruct が構造体FFWRX_DBG_DATAのサイズを越える場合、
	// 通常動作をして、かつ以下のエラー(Warning)を返す。
	//     FFWERR_FFWPARA_OVER：FFWに設定可能なパラメータサイズを超えていた(Warning)
	if(ferr == FFWERR_OK){
		if (bOverStructSize == TRUE) {	// RevRxNo130809-001 Modify Line
			return FFWERR_FFWPARA_OVER;		// FFWに設定可能なパラメータサイズを超えていた(Warning)
		}
	}
	// RevRxNo120910-001 Append End

	return ferr;
}

//=============================================================================
/**
 * デバッグ情報を参照する。
 * @param pDbg デバッグ情報格納構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo120910-001 Modify Line
FFWE100_API	DWORD FFWRXCmd_GetDBG(DWORD dwSizeOfDbgStruct, FFWRX_DBG_DATA* pDbg)
{
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo130809-001 Append Start
	FFWRX_DBG_DATA	dbgDataTmp;			// デバッグ情報の一時格納用変数
	BOOL	bOverStructSize;			// 構造体サイズが超えているかを示すフラグ
	DWORD	dwSetSize;
	// RevRxNo130809-001 Append End

	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130809-001 Modify Start
	bOverStructSize = FALSE;	// 構造体サイズが超えているかを示すフラグの初期化

	// RevRxNo120910-001 Append Start
	// "デバッグ情報を格納するFFWRX_DBG_DATA構造体のサイズ"の引数エラーチェック
	if (dwSizeOfDbgStruct < sizeof(FFWRX_DBG_DATA)) {
		return FFWERR_FFW_ARG;
	}
	// RevRxNo120910-001 Append End
	if (dwSizeOfDbgStruct > sizeof(FFWRX_DBG_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}
	// RevRxNo130809-001 Modify End

	// FFW内部管理構造体変数の更新
	memcpy(&dbgDataTmp, &s_DbgData_RX, sizeof(FFWRX_DBG_DATA));	// RevRxNo130809-001 Modify Line

	ferr = DO_GetRXDBG(&dbgDataTmp);	// RevRxNo130809-001 Modify Line

	if(ferr == FFWERR_OK){
		// RevRxNo130809-001 Append Start
		// FFW I/Fで設定された変数に一時変数の内容を格納
		if (dwSizeOfDbgStruct < sizeof(FFWRX_DBG_DATA)) {
			dwSetSize = dwSizeOfDbgStruct;
		} else {
			dwSetSize = sizeof(FFWRX_DBG_DATA);
		}
		memcpy((void*)pDbg, (const void*)&dbgDataTmp, dwSetSize);
		// RevRxNo130809-001 Append End

		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	// RevRxNo120910-001 Append Start
	// "デバッグ情報を格納するFFWRX_DBG_DATA構造体のサイズ"の引数エラーチェック（越える場合）について
	// dwSizeOfDbgStruct が構造体FFWRX_DBG_DATAのサイズを越える場合、
	// 通常動作をして、かつ以下のエラー(Warning)を返す。
	//     FFWERR_FFWPARA_OVER：FFWに設定可能なパラメータサイズを超えていた(Warning)
	if(ferr == FFWERR_OK){
		if (bOverStructSize == TRUE) {	// RevRxNo130809-001 Modify Line
			return FFWERR_FFWPARA_OVER;		// FFWに設定可能なパラメータサイズを超えていた(Warning)
		}
	}
	// RevRxNo120910-001 Append End

	return ferr;
}

//=============================================================================
/**
 * ターゲットMCUのステータス情報を取得する。
 * @param byStatKind 取得対象の情報(SFR/JTAG/EML)を指定する
 * @param peStatMcu  ターゲットMCUステータス情報を格納する変数のアドレス
 * @param pStatSFR   SFR情報を格納する
 * @param pStatJTAG  JTAG情報を格納する
 * @param pStatFINE  FINE情報を格納する
 * @param pStatEML   EML情報を格納する
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo120910-001 Modify Start
FFWE100_API	DWORD FFWRXCmd_GetSTAT(DWORD dwStatKind, DWORD dwSizeOfStatStruct, 
								   FFWRX_STAT_DATA* pStat)
// RevRxNo120910-001 Modify End
{
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo130809-001 Append Start
	enum FFWRXENM_STAT_MCU	eStatMcuTmp;	// ターゲットMCUステータス情報の一時格納用変数
	FFWRX_STAT_SFR_DATA		statSfrTmp;		// SFR情報の一時格納用変数
	FFWRX_STAT_JTAG_DATA	statJtagTmp;	// JTAG情報の一時格納用変数
	FFWRX_STAT_FINE_DATA	statFineTmp;	// FINE情報の一時格納用変数
	FFWRX_STAT_EML_DATA		statEmlTmp;		// エミュレータ情報の一時格納用変数
	BOOL	bOverStructSize;				// 構造体サイズが超えているかを示すフラグ
	DWORD	dwSetSize;
	// RevRxNo130809-001 Append End
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// 引数チェック

	// RevRxNo130809-001 Modify Start
	bOverStructSize = FALSE;	// 構造体サイズが超えているかを示すフラグの初期化

	// RevRxNo120910-001 Append Start
	// "取得するターゲットMCU状態の情報を格納するFFWRX_STAT_DATA構造体のサイズ"の引数エラーチェック
	if (dwSizeOfStatStruct < sizeof(FFWRX_STAT_DATA)) {
		return FFWERR_FFW_ARG;
	}
	// RevRxNo120910-001 Append End
	if (dwSizeOfStatStruct > sizeof(FFWRX_STAT_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}

	// pStat->dwSizeOfSfrStruct、FFWRX_STAT_SFR_DATA についても同様。
	if (pStat->dwSizeOfSfrStruct < sizeof(FFWRX_STAT_SFR_DATA)) {
		return FFWERR_FFW_ARG;
	}
	if (pStat->dwSizeOfSfrStruct > sizeof(FFWRX_STAT_SFR_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}

	// pStat->dwSizeOfJtagStruct、FFWRX_STAT_JTAG_DATA についても同様。
	if (pStat->dwSizeOfJtagStruct < sizeof(FFWRX_STAT_JTAG_DATA)) {
		return FFWERR_FFW_ARG;
	}
	if (pStat->dwSizeOfJtagStruct > sizeof(FFWRX_STAT_JTAG_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}

	// pStat->dwSizeOfFineStruct、FFWRX_STAT_FINE_DATA についても同様。
	if (pStat->dwSizeOfFineStruct < sizeof(FFWRX_STAT_FINE_DATA)) {
		return FFWERR_FFW_ARG;
	}
	if (pStat->dwSizeOfFineStruct > sizeof(FFWRX_STAT_FINE_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}

	// pStat->dwSizeOfEmlStruct、FFWRX_STAT_EML_DATA についても同様。
	// RevRxE2LNo141104-001 Append Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		if (pStat->dwSizeOfEmlStruct < FFWRX_STAT_EML_DATA_SIZE_V20200) {
			return FFWERR_FFW_ARG;
		}
	}
	// RevRxE2LNo141104-001 Append End
	if (pStat->dwSizeOfEmlStruct < sizeof(FFWRX_STAT_EML_DATA)) {
		// 旧バージョンの構造体サイズであるかをチェック
		if (pStat->dwSizeOfEmlStruct != FFWRX_STAT_EML_DATA_SIZE_V20000) {	// V.2.00.00の構造体サイズでない場合
			return FFWERR_FFW_ARG;
		}
	}
	if (pStat->dwSizeOfEmlStruct > sizeof(FFWRX_STAT_EML_DATA)) {
		bOverStructSize = TRUE;		// コマンド処理終了後Warningを返送するためにフラグをTRUEに設定する
	}
	// RevRxNo120910-001 Append End
	// RevRxNo130809-001 Modify End

	// RevRxNo120910-001 Modify Line
	if ((dwStatKind < STAT_KIND_NON) || (dwStatKind > STAT_KIND_ALL)) {
		return FFWERR_FFW_ARG;
	}
	// V.1.02 新デバイス対応 Append Start
	// RevRxNo120910-001 Modify Line
	if ((dwStatKind & STAT_KIND_JTAG) && (dwStatKind & STAT_KIND_FINE)) {	// JTAGとFINE両方指定されている
		return FFWERR_FFW_ARG;
	}
	// V.1.02 新デバイス対応 Append End

	// RevRxNo130809-001 Modify Start
	// 一時変数に初期値セット
	memset(&statSfrTmp, 0, sizeof(FFWRX_STAT_SFR_DATA));
	memset(&statJtagTmp, 0, sizeof(FFWRX_STAT_JTAG_DATA));
	memset(&statFineTmp, 0, sizeof(FFWRX_STAT_FINE_DATA));
	memset(&statEmlTmp, 0, sizeof(FFWRX_STAT_EML_DATA));
	// RevRxNo130809-001 Modify End

	// V.1.02 新デバイス対応 Modify Line
	// RevRxNo130809-001 Modify Line
	ferr = DO_GetRXSTAT(dwStatKind, &eStatMcuTmp, &statSfrTmp, &statJtagTmp, &statFineTmp, &statEmlTmp);

	// RevRxNo120910-001 Modify End

	// V.1.02 RevNo110613-001 Append Start
	if(ferr == FFWERR_OK){
		// RevRxNo130809-001 Append Start
		// FFW I/Fで設定された変数に一時変数の内容を格納
		*(pStat->peStatMcu) = eStatMcuTmp;
		if (pStat->pStatSFR != NULL) {
			if (pStat->dwSizeOfSfrStruct < sizeof(FFWRX_STAT_SFR_DATA)) {
				dwSetSize = pStat->dwSizeOfSfrStruct;
			} else {
				dwSetSize = sizeof(FFWRX_STAT_SFR_DATA);
			}
			memcpy((void*)pStat->pStatSFR, (const void*)&statSfrTmp, dwSetSize);
		}
		if (pStat->pStatJTAG != NULL) {
			if (pStat->dwSizeOfJtagStruct < sizeof(FFWRX_STAT_JTAG_DATA)) {
				dwSetSize = pStat->dwSizeOfJtagStruct;
			} else {
				dwSetSize = sizeof(FFWRX_STAT_JTAG_DATA);
			}
			memcpy((void*)pStat->pStatJTAG, (const void*)&statJtagTmp, dwSetSize);
		}
		if (pStat->pStatFINE != NULL) {
			if (pStat->dwSizeOfFineStruct < sizeof(FFWRX_STAT_FINE_DATA)) {
				dwSetSize = pStat->dwSizeOfFineStruct;
			} else {
				dwSetSize = sizeof(FFWRX_STAT_FINE_DATA);
			}
			memcpy((void*)pStat->pStatFINE, (const void*)&statFineTmp, dwSetSize);
		}
		if (pStat->pStatEML != NULL) {
			if (pStat->dwSizeOfEmlStruct < sizeof(FFWRX_STAT_EML_DATA)) {
				dwSetSize = pStat->dwSizeOfEmlStruct;
			} else {
				dwSetSize = sizeof(FFWRX_STAT_EML_DATA);
			}
			memcpy((void*)pStat->pStatEML, (const void*)&statEmlTmp, dwSetSize);
		}
		// RevRxNo130809-001 Append End

		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	// RevRxNo120910-001 Append Start
	// 取得する情報を格納する各構造体のサイズが、FFWの各構造体サイズを越える場合、
	// 通常動作をして、Warningを返す。
	if(ferr == FFWERR_OK){
		// RevRxNo130809-001 Modify Start
		if (bOverStructSize == TRUE) {
			return FFWERR_FFWPARA_OVER;		// FFWに設定可能なパラメータサイズを超えていた(Warning)
		}
		// RevRxNo130809-001 Modify End
	}
	// RevRxNo120910-001 Append End

	return ferr;
}

// 2008.9.9 INSERT_BEGIN_E20RX600(+9) {
//=============================================================================
/**
 * ホットプラグ機能の設定/解除
 * @param dwPlug ターゲットシステムとの接続/切断を指定
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_HPON(DWORD dwPlug)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	// V.1.02 RevNo110506-004 Appned Line
	FFWERR	ferr2 = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevNo110405-003 Append Line
	// HPONを呼んだ時点でホットプラグのフラグは無効状態にする
	setHotPlugState(FALSE);
	// RevRxNo140515-012 Append Line
	setHotPlgStartFlg(FALSE);		// ホットプラグ起動フラグはFALSEに設定
	// 引数エラーチェック
	if (dwPlug < HPON_COLD_CON || dwPlug > HPON_NCON_RESETOFF) {
		if (dwPlug != HPON_CON_DBG) {
			// V.1.02 RevNo110509-001 Appned Line
			SetPmodInfo(FALSE);		// PMOD未発行
			return FFWERR_FFW_ARG;
		}
	}

	ferr = DO_RXHPON(dwPlug);
	// V.1.02 RevNo110509-002 Append Start
	// HPON処理でエラーが発生したら強制的にエミュレータF/Wのレベルをレベル0に戻しておくようにする
	if( ferr != FFWERR_OK ){
		ferr2 = E20TransLV0();
		// V.1.02 RevNo110509-001 Appned Line
		SetPmodInfo(FALSE);		// PMOD未発行
		return ferr;
	}
	// V.1.02 RevNo110509-002 Append End

	// ホットスタート時
	if (dwPlug == HPON_HOT_CON) {
		setHotPlugState(TRUE);	// ホットプラグ設定状態を設定に変更
	// V.1.02 No.14,15 USB高速化対応 Append Start
	}else if( dwPlug >= HPON_NCON_RESETKEEP ){	// 切断時
		ferr2 = E20TransLV0();
		// V.1.02 RevNo110506-004 Modify Line
		// V.1.02 RevNo110509-001 Appned Line
		SetPmodInfo(FALSE);		// PMOD未発行
	// V.1.02 No.14,15 USB高速化対応 Append End
	}
	// V.1.02 RevNo110506-004 Modify Start
	if( ferr != FFWERR_OK ){	// DO_RXHPON()でエラーが発生していたらそのエラーを返す
		return ferr;
	}
	if( ferr2 != FFWERR_OK ){	// DO_E20Trans()でエラーが発生したらそのエラーを返す(USB通信がおかしくなったときだけエラー発生)
		return ferr2;
	}

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;			// DO_RXHPON()でもDO_E20Trans()でもエラーが発生していない場合はOKを返す
	// V.1.02 RevNo110613-001 Modify End
	// V.1.02 RevNo110506-004 Modify End

}
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * 認証用のIDコードを設定
 * @param pbyID E2Cを使用して、IDコードをID認証プログラムに渡す
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_IDCODE(BYTE* pbyID)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	byResult = 0;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	FFWMCU_MCUINFO_DATA_RX*	pMcuInfo;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

 	pDbgData = GetDbgDataRX();
	pMcuInfo = GetMcuInfoDataRX();

	// V.1.02 No.4 ユーザブートモード起動対応 Append Line
	// IDコードを内部管理変数へ保存
	SetIdCode(pbyID);

	// IDコード処理実施
	ferr = DO_RXIDCODE(pbyID, &byResult);
	if(ferr != FFWERR_OK) {
		return ferr;
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}
//=============================================================================
/**
 * ホットプラグ機能ハードセット
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetHotPlug(void)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	ferr = DO_E20SetHotPlug();

	// V.1.02 RevNo110613-001 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Append End
	s_bHotPlgStartFlg = TRUE;					// ホットプラグ起動フラグを設定 RevRxNo140515-012 Append Line

	return ferr;
}
// 2008.9.9 INSERT_END_E20RX600 }
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append Start
//=============================================================================
/**
 * 起動モード、起動エンディアン情報を設定する。
 * @param ePmode		起動モード
 * @param eMcuEndian	起動エンディアン
 * @param pUbcode		UBコード
 * @param pUbAreaChange	フラッシュROM上ユーザブート領域の書き換え情報へのポインタ
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
// RevRxNo120910-001 Modify Start
FFWE100_API	DWORD FFWRXCmd_SetPMOD(enum FFWRXENM_PMODE ePmode, enum FFWRXENM_RMODE eRmode, 
                                   enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA* pUbcode, 
                                   const FFWRX_UBAREA_CHANGEINFO_DATA* pUbAreaChange)
// RevRxNo120910-001 Modify End
{
	FFWERR	ferr = FFWERR_OK;
	// V.1.02 MDE,UBコード書き換え対応 Append Line
	FFWERR	ferr2= FFWERR_OK;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	// V.1.02 RevNo110411-001 Appned Start
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	// V.1.02 RevNo110411-001 Appned End

	// RevRxNo120910-005	Delete Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// V.1.02 RevNo110411-001 Appned Start
	bHotPlugState = getHotPlugState();				// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得
	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}
	// V.1.02 RevNo110411-001 Appned End

	// 引数エラーチェック
	// 端子設定起動モード
	// 端子設定起動モード( V.1.02ではシングルチップモードとユーザブートモードのみ指定可 )
	if ((ePmode != RX_PMODE_SNG) && (ePmode != RX_PMODE_USRBOOT)) {
		return FFWERR_FFW_ARG;
	}
	// レジスタ設定起動モード
	if (eRmode < RX_RMODE_SNG || eRmode > RX_RMODE_ROMD_DUAL) {		// RegRxNo161003-001 Modify Line
		return FFWERR_FFW_ARG;
	}

	// エンディアン
	if ((eMcuEndian != ENDIAN_LITTLE) && (eMcuEndian != ENDIAN_BIG)) {
		return FFWERR_FFW_ARG;
	}

	// ダウンロード用WTRプログラムがロード済みかの確認
	if(GetDownloadWtrLoad() == FALSE){
		return FFWERR_WTR_NOTLOAD;
	}

	// RevRxNo121106-001 Append Start
	// メモリアクセス、内蔵フラッシュの書き換えが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	// PMODは、IDCODEコマンド発行前にも発行されるため、ID認証済みの場合のみ実施する。
	if (bIdCodeResultState == TRUE) {
		ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo121106-001 Append End

	// RevRxNo120910-005	Modify Line	引数追加：pUbAreaChange	フラッシュROM上ユーザブート領域の書き換え情報へのポインタ
	ferr2 = DO_SetRXPMOD(ePmode, eRmode, eMcuEndian, pUbcode, pUbAreaChange);

	// RevRxNo121026-001, RevRxNo130411-001, RevRxNo130301-001 Modify Line
	if ((pFwCtrl->eMdeType == RX_MDE_FLASH) || (pFwCtrl->eMdeType == RX_MDE_EXTRA)) {
			// MDEを、フラッシュROMまたはEXTRA領域で設定するMCUの場合
		// エンディアン状態を記憶(未リセット時に対応)
		ferr = SetEndianState();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr2 == FFWERR_OK){
		ferr2 = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr2;
}


//=============================================================================
/**
 * 起動モード、起動エンディアン情報を参照する。
 * @param ePmode		起動モード
 * @param eMcuEndian	起動エンディアン
 * @param pUbcode		UBコード
 * @param pUbAreaChange	フラッシュROM上ユーザブート領域の書き換え情報へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo120910-001 Modify Start
FFWE100_API	DWORD FFWRXCmd_GetPMOD(enum FFWRXENM_PMODE *const pePmode, enum FFWRXENM_RMODE *const peRmode, 
                                   enum FFWENM_ENDIAN *const peMcuEndian, FFWRX_UBCODE_DATA* pUbcode, 
                                   FFWRX_UBAREA_CHANGEINFO_DATA *const pUbAreaChange)
// RevRxNo120910-001 Modify End
{
	FFWERR	ferr = FFWERR_OK;
	FFWRX_UBCODE_DATA* pUBCode;
	// RevRxNo120910-005	Append Line
	FFWRX_UBAREA_CHANGEINFO_DATA	*pGetUbAreaChange;

	// RevRxNo120910-005	Delete Line

	*pePmode = GetPmodeDataRX();
	// V.1.02 No.5 ユーザブート/USBブート対応 Append Line
	*peRmode = GetRmodeDataRX();
	*peMcuEndian = GetEndianDataRX();
	pUBCode = GetUBCodeDataRX();
	// RevRxNo120910-005	Append Line
	pGetUbAreaChange = GetUBCodeDataAreaChangeRX();
	
	// RevNo120206-001 UBコードコピー間違い修正 &pUBCode → pUBCode  Modify Line
	memcpy( pUbcode, pUBCode, sizeof(FFWRX_UBCODE_DATA) );
	// RevRxNo120910-005	Append Line	
	memcpy( pUbAreaChange, pGetUbAreaChange, sizeof(FFWRX_UBAREA_CHANGEINFO_DATA) );

	return ferr;
}
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append End



//=============================================================================
/**
 * ターゲットMCU端子のマスク情報の設定(未サポート)
 * @param pMask ターゲットMCU端子のマスク情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_SetMSTSM(const FFWRX_MSTSM_DATA* pMask)
{
	FFWERR	ferr = FFWERR_OK;
	
	pMask;

	return ferr;
}

//=============================================================================
/**
 * ターゲットMCU端子のマスク情報の参照(未サポート)
 * @param pMask ターゲットMCU端子のマスク情報を格納する構造体のアドレス
 * @retval FFWERR_OK		正常終了
 * @retval FFWERR_COM		致命的な通信異常が発生した
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetMSTSM(FFWRX_MSTSM_DATA *const pMask)
{
	FFWERR	ferr = FFWERR_OK;

	pMask;

	return ferr;
}

//=============================================================================
/**
 * 指定したPC位置から始まる命令の命令コードを解析
 * @param dwPC        解析する命令のPC値を格納
 * @param pInstResult 命令解析結果を格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_INSTCODE(DWORD dwPC, FFWRX_INST_DATA* pInstResult)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrWarning;	// RevNo111121-007 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// メモリアクセスが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
// RevNo111121-007 Append Start
	// FFWERR_READ_DTF_MCURUN/FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング記録変数初期化
	ClrMemAccWarningFlg();
// RevNo111121-007 Append End

	ferr = DO_GetRXINSTCODE(dwPC, pInstResult);

// RevNo111121-007 Append Start
	if((ferr == FFWERR_OK) || (ferr == FFWERR_INSTCODE_FAIL)){
		ferrWarning = WarningChk_MemAccessErr();		// メモリアクセス時のWarning確認
		if (ferrWarning != FFWERR_OK) {
			ferr = ferrWarning;
		}
	}
// RevNo111121-007 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}
// 2008.11.18 INSERT_END_E20RX600 }

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * フラッシュメモリコンフィギュレーションデータの設定
 * @param pFmcd	フラッシュメモリのコンフィギュレーションデータ情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_SetFMCD(const FFWRX_FMCD_DATA* pFmcd)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwCnt;
	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}

	// 設定データ数の引数エラーチェック
	if (pFmcd->dwSetNum > FMCD_NUM_MAX) {
		return FFWERR_FFW_ARG;
	}

	// 設定先情報の引数エラーチェック
	for (dwCnt = 0; dwCnt < pFmcd->dwSetNum; dwCnt++) {
		if ((pFmcd->dwSetNo[dwCnt] < FMCD_SETNO_MIN) || (pFmcd->dwSetNo[dwCnt] > FMCD_SETNO_MAX)) {
			return FFWERR_FFW_ARG;
		}
		if (pFmcd->dwSetNo[dwCnt] % 4) {
			return FFWERR_FFW_ARG;
		}
	}

	// 設定データのエラーチェック
	for (dwCnt = 0; dwCnt < pFmcd->dwSetNum; dwCnt++) {
		if (pFmcd->dwSetNo[dwCnt] == pMcuDef->dwOfs1StartOffset) {	// RevRxNo150827-002 Modify Line
			if ((pFmcd->dwSetData[dwCnt] & MCU_OSM_OFS1_LVDAS_MASK_DATA) != MCU_OSM_OFS1_LVDAS_DIS_DATA) {
				// OFS1レジスタのLVDASビットが"リセット後、電圧監視0リセット無効"でない場合
				return FFWERR_WRITE_OFS1_LVDAS_ENA;
			}
		}
	}

	memcpy((void*)&s_Fmcd, (const void*)pFmcd, sizeof(FFWRX_FMCD_DATA));

	return ferr;
}
// RevRxNo130301-001 Append End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * フラッシュメモリコンフィギュレーションデータの参照
 * @param pFmcd	フラッシュメモリのコンフィギュレーションデータ情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWRXCmd_GetFMCD(FFWRX_FMCD_DATA *const pFmcd)
{
	FFWERR	ferr = FFWERR_OK;

	memcpy(pFmcd, &s_Fmcd, sizeof(FFWRX_FMCD_DATA));

	return ferr;
}
// RevRxNo130301-001 Append End

// RevRxNo120910-001 Append Start
//=============================================================================
/**
 * ターゲットMCU空間のデータ設定(未対応)
 * @param pMcud        ターゲットMCU空間に設定するデータを格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_SetMCUD(const FFW_MCUD_DATA* pMcud)
{
	pMcud;		// コンパイル時のWarning対策
	return FFWERR_OK;
}

//=============================================================================
/**
 * ターゲットMCU空間のデータ参照(未対応)
 * @param pMcud        ターゲットMCU空間に設定するデータを格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_GetMCUD(FFW_MCUD_DATA* pMcud)
{
	pMcud;		// コンパイル時のWarning対策

	return FFWERR_OK;
}
// RevRxNo120910-001 Append End

///////////////////////////////////////////////////////////////////////////////
// FFW内部関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * MCU空間情報を構造体のポインタとして返送する。
 * @param なし
 * @retval MCU空間情報管理構造体へのポインタ
 */
//=============================================================================
FFWMCU_MCUAREA_DATA_RX* GetMcuAreaDataRX(void)
{
	return	&s_McuAreaData_RX;
}

//=============================================================================
/**
 * MCU情報を構造体のポインタとして返送する。
 * @param なし
 * @retval MCU情報管理構造体へのポインタ
 */
//=============================================================================
FFWMCU_MCUINFO_DATA_RX* GetMcuInfoDataRX(void)
{
	return	&s_McuInfoData_RX;
}

//=============================================================================
/**
 * RXのデバッグ情報を構造体のポインタとして返送する。
 * @param なし
 * @retval デバッグ情報管理構造体へのポインタ
 */
//=============================================================================
FFWMCU_DBG_DATA_RX* GetDbgDataRX(void)
{
	return	&s_DbgData_RX;
}

//=============================================================================
/**
 * 管理変数に設定されているユーザ指定端子設定MCU動作モードを返送する。
 * @param なし
 * @retval MCU動作モード
 */
//=============================================================================
enum FFWRXENM_PMODE GetPmodeDataRX(void)
{
	return	s_ePmode;
}

// RevRxNo120910-005	Append Start
//=============================================================================
/**
 * ユーザブート領域の書き換え情報の管理構造体ポインタとして返送する。
 * @param なし
 * @retval ユーザブート領域の書き換え情報の管理構造体ポインタ
 */
//=============================================================================
FFWRX_UBAREA_CHANGEINFO_DATA*	GetUBCodeDataAreaChangeRX(void)
{
	return	&s_UBCodeDataAreaChange_RX;
}
//=============================================================================
/**
 * ユーザブート領域の書き換え情報の管理テーブルへのコピー
 * @param *pUbAreaChange	:フラッシュROM上ユーザブート領域の書き換え情報へのポインタ
 * @retval なし
 */
//=============================================================================
void SetUbAreaChangeRX(const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange)
{
	DWORD i;
	
	// ユーザブート領域の書き換え許可情報のセット
	s_UBCodeDataAreaChange_RX.eChangeUbAreaEna = pUbAreaChange->eChangeUbAreaEna;

	// UBコードA情報のセット
	for( i = 0; i < UBCODE_MAX; i++ ){
		s_UBCodeDataAreaChange_RX.byChangeInfoUBCodeA[i] = pUbAreaChange->byChangeInfoUBCodeA[i];
	}
	return;
}
// RevRxNo120910-005	Append End
//=============================================================================
/**
 * ユーザ指定端子設定MCU動作モードを管理変数に設定する。
 * @param FFWRXENM_PMODE情報
 * @retval なし
 */
//=============================================================================
void SetPmodeDataRX(FFWRXENM_PMODE ePmode)
{
	s_ePmode = ePmode;
	return;
}

//=============================================================================
/**
 * 管理変数に設定されているユーザ指定レジスタ設定MCU動作モードを返送する。
 * @param なし
 * @retval MCU動作モード
 */
//=============================================================================
enum FFWRXENM_RMODE GetRmodeDataRX(void)
{
	return	s_eRmode;
}

//=============================================================================
/**
 * ユーザ指定レジスタ設定MCU動作モードを管理変数に設定する。
 * @param FFWRXENM_PMODE情報
 * @retval なし
 */
//=============================================================================
void SetRmodeDataRX(FFWRXENM_RMODE eRmode)
{
	s_eRmode = eRmode;
	return;
}

//=============================================================================
/**
 * 管理変数に設定されているUBコード情報を返送する。
 * @param なし
 * @retval UBコード
 */
//=============================================================================
FFWRX_UBCODE_DATA* GetUBCodeDataRX(void)
{
	return	&s_UBCodeData_RX;
}

//=============================================================================
/**
 * UBコードを管理変数に設定する。
 * @param pUbcode UBコード格納ポインタ
 * @retval なし
 */
//=============================================================================
void SetUBCodeDataRX(const FFWRX_UBCODE_DATA *pUbcode)
{
	DWORD i;
	for( i = 0; i < 8; i++ ){
		s_UBCodeData_RX.byUBCodeA[i] = pUbcode->byUBCodeA[i];
		s_UBCodeData_RX.byUBCodeB[i] = pUbcode->byUBCodeB[i];
	}
	return;
}


//=============================================================================
/**
 * フラッシュメモリのコンフィギュレーションデータ情報を構造体のポインタとして返送する。
 * @param なし
 * @retval フラッシュメモリのコンフィギュレーションデータ情報管理構造体へのポインタ
 */
//=============================================================================
FFWRX_FMCD_DATA* GetFmcdData(void)
{
	return	&s_Fmcd;
}


//=============================================================================
/**
 * 管理変数に設定されているユーザ指定エンディアン情報を返送する。
 * @param なし
 * @retval エンディアン情報
 */
//=============================================================================
enum FFWENM_ENDIAN GetEndianDataRX(void)
{
	return	s_eMcuEndian;
}

//=============================================================================
/**
 * ユーザ指定エンディアン情報を管理変数に設定する。
 * @param eMcuEndian エンディアン
 * @retval なし
 */
//=============================================================================
void SetEndianDataRX(FFWENM_ENDIAN eMcuEndian)
{
	s_eMcuEndian = eMcuEndian;
	return;
}

//=============================================================================
/**
 * ホットプラグ設定状態を取得
 * @param  なし
 * @retval 現在の設定状態(TRUE:ホットプラグ設定状態、FALSE：未設定状態)
 */
//=============================================================================
BOOL getHotPlugState(void)
{
	return s_bHotPlugState;
}

//=============================================================================
/**
 * ホットプラグ起動中かどうかを設定
 * @param  現在の起動状態(TRUE:ホットプラグ起動中、FALSE：ホットプラグ起動中ではない)
 * @retval なし
 */
//=============================================================================
void setHotPlugState(BOOL bSet)
{
	s_bHotPlugState = bSet;

	return;
}

//RevRxNo140515-012 Append Start
//=============================================================================
/**
 * ホットプラグ起動フラグを設定(ホットプラグ起動設定されているかどうかを示す)
 * @param  ホットプラグ起動フラグ設定値(TRUE:ホットプラグ起動、FALSE：ホットプラグ起動ではない)
 * @retval なし
 */
//=============================================================================
void setHotPlgStartFlg(BOOL bSet)
{
	s_bHotPlgStartFlg = bSet;

	return;
}
//RevRxNo140515-012 Append End

//=============================================================================
/**
 * ID認証結果状態を取得
 * @param  なし
 * @retval 現在の設定状態(TRUE:ID認証結果設定状態、FALSE：未設定状態)
 */
//=============================================================================
BOOL getIdCodeResultState(void)
{
	return s_bIdCodeResult;
}

//=============================================================================
/**
 * ID認証結果状態を設定
 * @param  現在の設定状態(TRUE:ID認証結果設定状態、FALSE：未設定状態)
 * @retval なし
 */
//=============================================================================
void setIdCodeResultState(BOOL bSet)
{
	s_bIdCodeResult = bSet;

	return;
}

// V.1.02 No.4 ユーザブートモード起動対応 Append Start
//=============================================================================
/**
 * ユーザが設定したIDコードを保存しておく
 * @param BYTE* pbyID		IDコード(最大16バイト)
 * @retval なし
 */
//=============================================================================
void SetIdCode(BYTE* pbyID)
{
	BYTE i;
	for( i = 0; i < 16; i++ ){
		s_byUserSetIdCode[i] = *(pbyID+i);
	}
}

//=============================================================================
/**
 * 保存しておいたユーザ設定IDコードデータ取得
 * @param BYTE* pbyIDBuff		IDコード(最大16バイト)格納用バッファへのポインタ
 * @retval なし
 */
//=============================================================================
void GetIdCode(BYTE *pbyIDBuff)
{
	BYTE i;
	for( i = 0; i < 16; i++ ){
		*(pbyIDBuff+i) = s_byUserSetIdCode[i];
	}
}
// V.1.02 No.4 ユーザブートモード起動対応 Append End
// RevNo111128-001 Append Start
//=============================================================================
/**
 * MDE レジスタに設定されているエンディアン情報を取得する。
 * @param なし
 * @return MDE レジスタ情報
 */
//=============================================================================
enum FFWRX_STAT_ENDIAN GetMdeRegData()
{
	return	s_eMdeRegData;
}

//=============================================================================
/**
 * MDE レジスタに設定されているエンディアン情報を設定する。
 * @param eEndian MDE レジスタ情報
 * @return なし
 */
//=============================================================================
void SetMdeRegData(enum FFWRX_STAT_ENDIAN eEndian)
{
	s_eMdeRegData = eEndian;
	return;
}
// RevNo111128-001 Append End

// RevRxNo161003-001 Append Start
//=============================================================================
/**
 * バンクモード情報管理変数に設定する。
 * @param FFWRXENM_BANKMD情報
 * @retval なし
 */
//=============================================================================
void SetBankModeDataRX(FFWRXENM_BANKMD eBankMode)
{
	s_eBankMode = eBankMode;
	return;
}

//=============================================================================
/**
 * 管理変数に設定されているバンクモード情報を返送する。
 * @param なし
 * @retval FFWRXENM_BANKMD情報
 */
//=============================================================================
enum FFWRXENM_BANKMD GetBankModeDataRX(void)
{
	return	s_eBankMode;
}

//=============================================================================
/**
 * 起動バンク情報管理変数に設定する。
 * @param FFWRXENM_BANKSEL情報
 * @retval なし
 */
//=============================================================================
void SetBankSelDataRX(FFWRXENM_BANKSEL eBankSel)
{
	s_eBankSel = eBankSel;
	return;
}

//=============================================================================
/**
 * 管理変数に設定されている起動バンク情報を返送する。
 * @param なし
 * @retval FFWRXENM_BANKSEL情報
 */
//=============================================================================
enum FFWRXENM_BANKSEL GetBankSelDataRX(void)
{
	return	s_eBankSel;
}
// RevRxNo161003-001 Append Start


// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * FFWRXCmd_SetMCUで指定されないMCUレジスタ情報の設定
 * @param pMcuInfo エミュレータ制御に必要なMCU情報格納構造体のアドレス
 * @retval	なし
 */
//=============================================================================
static void setMcuRegInfoData(FFWRX_MCUINFO_DATA* pMcuInfo)
{
	if (pMcuInfo->dwMcuRegInfoBlkNum < 4) {
		// システムコントロールレジスタ1情報設定
		switch (pMcuInfo->byMcuType) {
		case MCU_TYPE_0:	// no break
		case MCU_TYPE_1:	// no break
		case MCU_TYPE_2:	// no break
		case MCU_TYPE_3:
			pMcuInfo->dwMcuRegInfoNum[3] = 1;
			pMcuInfo->dwmadrMcuRegInfoAddr[3][0] = MCU_REG_SYSTEM_SYSCR1;
			pMcuInfo->eMcuRegInfoAccessSize[3][0] = MWORD_ACCESS;
			pMcuInfo->dwMcuRegInfoMask[3][0] = MCU_REG_SYSTEM_SYSCR1_RAME;
			pMcuInfo->dwMcuRegInfoData[3][0] = MCU_REG_SYSTEM_SYSCR1_RAME_ENA;
			break;
		default:	// MCU_TYPE_0～3以外では必ず指定されるため、FFWによる固定値設定はしない
			break;
		}
	}

	if (pMcuInfo->dwMcuRegInfoBlkNum < 5) {
		// モジュールストップコントロールレジスタC情報設定
		switch (pMcuInfo->byMcuType) {
		case MCU_TYPE_0:	// no break
		case MCU_TYPE_1:	// no break
		case MCU_TYPE_2:
			pMcuInfo->dwMcuRegInfoNum[4] = 1;
			pMcuInfo->dwmadrMcuRegInfoAddr[4][0] = MCU_REG_SYSTEM_MSTPCRC;
			pMcuInfo->eMcuRegInfoAccessSize[4][0] = MLWORD_ACCESS;
			pMcuInfo->dwMcuRegInfoMask[4][0] = MCU_REG_SYSTEM_MSTPCRC_RAM01;
			pMcuInfo->dwMcuRegInfoData[4][0] = MCU_REG_SYSTEM_MSTPCRC_RAM01_ENA;
			break;
		case MCU_TYPE_3:
			pMcuInfo->dwMcuRegInfoNum[4] = 1;
			pMcuInfo->dwmadrMcuRegInfoAddr[4][0] = MCU_REG_SYSTEM_MSTPCRC;
			pMcuInfo->eMcuRegInfoAccessSize[4][0] = MLWORD_ACCESS;
			pMcuInfo->dwMcuRegInfoMask[4][0] = MCU_REG_SYSTEM_MSTPCRC_RAM0;
			pMcuInfo->dwMcuRegInfoData[4][0] = MCU_REG_SYSTEM_MSTPCRC_RAM0_ENA;
			break;
		default:	// MCU_TYPE_0～3以外では必ず指定されるため、FFWによる固定値設定はしない
			break;
		}
	}

	return;
}
// RevRxNo130301-001 Append End

//=============================================================================
/**
 * MCU依存コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Mcu(void)
{
	DWORD	dwCnt;
	MADDR	madrRamStart[INIT_RAMAREA_NUM] = {0x00000000};
	MADDR	madrRamEnd[INIT_RAMAREA_NUM] = {0x0001FFFF};
	MADDR	madrSfrStart[INIT_SFRAREA_NUM] = {0x00080000};
	MADDR	madrSfrEnd[INIT_SFRAREA_NUM] = {0x000FFFFF};
	MADDR	madrFlashStart[INIT_FLASHBLOCK_NUM] = {0x00000000};
	MADDR	madrExtStart[INIT_EXTROM_NUM] = {0x01000000, 0x02000000, 0x03000000, 0x04000000, 0x05000000, 0x06000000, 0x07000000, 0xFF000000};
	MADDR	madrExtEnd[INIT_EXTROM_NUM] = {0x01FFFFFF, 0x02FFFFFF, 0x03FFFFFF, 0x04FFFFFF, 0x05FFFFFF, 0x06FFFFFF, 0x07FFFFFF, 0xFFFFFFFF};
	// RevRxNo120910-001 Append Line
	float	f33v = 3.3f;

	// MCU空間情報格納構造体変数の初期化
	s_McuAreaData_RX.dwSfrAreaNum = INIT_SFRAREA_NUM;
	for (dwCnt = 0; dwCnt < INIT_SFRAREA_NUM; dwCnt++) {
		s_McuAreaData_RX.dwmadrSfrStartAddr[dwCnt] = madrSfrStart[dwCnt];
		s_McuAreaData_RX.dwmadrSfrEndAddr[dwCnt] = madrSfrEnd[dwCnt];
		s_McuAreaData_RX.bySfrEndian[dwCnt] = FFWRX_ENDIAN_LITTLE;
	}
	s_McuAreaData_RX.dwRamAreaNum = INIT_RAMAREA_NUM;
	for (dwCnt = 0; dwCnt < INIT_RAMAREA_NUM; dwCnt++) {
		s_McuAreaData_RX.dwmadrRamStartAddr[dwCnt] = madrRamStart[dwCnt];
		s_McuAreaData_RX.dwmadrRamEndAddr[dwCnt] = madrRamEnd[dwCnt];
	}
	s_McuAreaData_RX.dwmadrFlashRomWriteStart = 0x00E00000;
	s_McuAreaData_RX.dwmadrFlashRomReadStart = 0xFFE00000;
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Start
	s_McuAreaData_RX.dwFlashRomPatternNum = INIT_FLASHBLOCK_NUM;
	for (dwCnt = 0; dwCnt < INIT_FLASHBLOCK_NUM; dwCnt++) {
		s_McuAreaData_RX.dwmadrFlashRomStart[dwCnt] = madrFlashStart[dwCnt];
		s_McuAreaData_RX.dwFlashRomBlockSize[dwCnt] = INIT_FLASHBLOCK_SIZE;
		s_McuAreaData_RX.dwFlashRomBlockNum[dwCnt] = INIT_FLASHBLOCK_NUM;
	}
	s_McuAreaData_RX.dwDataFlashRomPatternNum = INIT_FLASHBLOCK_NUM;
	for (dwCnt = 0; dwCnt < INIT_FLASHBLOCK_NUM; dwCnt++) {
		s_McuAreaData_RX.dwmadrDataFlashRomStart[dwCnt] = madrFlashStart[dwCnt];
		s_McuAreaData_RX.dwDataFlashRomBlockSize[dwCnt] = INIT_FLASHBLOCK_SIZE;
		s_McuAreaData_RX.dwDataFlashRomBlockNum[dwCnt] = INIT_FLASHBLOCK_NUM;
	}
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify End
	s_McuAreaData_RX.dwExtMemBlockNum = INIT_EXTROM_NUM;
	for (dwCnt = 0; dwCnt < INIT_EXTROM_NUM; dwCnt++) {
		s_McuAreaData_RX.dwmadrExtMemBlockStart[dwCnt] = madrExtStart[dwCnt];
		s_McuAreaData_RX.dwmadrExtMemBlockEnd[dwCnt] = madrExtEnd[dwCnt];
		s_McuAreaData_RX.eExtMemCondAccess[dwCnt] = MBYTE_ACCESS;
		s_McuAreaData_RX.byExtMemEndian[dwCnt] = FFWRX_ENDIAN_LITTLE;
		// RevRxNo120910-001 Append Line
		s_McuAreaData_RX.byExtMemType[dwCnt] = MCU_EXT_MEM_CS;
	}

	s_McuAreaData_RX.dwAdrFcuFirmStart = 0xFEFFE000;
	// RevRxNo120910-001 Modify Line
	s_McuAreaData_RX.dwFcuFirmLen = 0x2001;
	s_McuAreaData_RX.dwAdrFcuRamStart = 0x007F8000;

	// エミュレータ制御に必要なMCU情報格納構造体変数の初期化
	s_McuInfoData_RX.dwadrIspOnReset = 0;
	s_McuInfoData_RX.dwadrWorkRam = 0;
	s_McuInfoData_RX.dwsizWorkRam = 0;
	s_McuInfoData_RX.eEdge = TRC_EDGE_DOUBLE;
	s_McuInfoData_RX.eRatio = TRC_RATIO_2_1;
	s_McuInfoData_RX.eExTAL = TRC_EXTAL_X4;
	s_McuInfoData_RX.eAccSet = ACC_SET_OFF;
	s_McuInfoData_RX.eCoProSet = COPRO_SET_OFF;
	for (dwCnt = 0; dwCnt < MCU_SPC_MAX; dwCnt++) {
		s_McuInfoData_RX.dwSpc[dwCnt] = 0;
	}
	s_McuInfoData_RX.fClkExtal = 0.0;
	s_McuInfoData_RX.dwmadrIdcodeStart = 0;	
	s_McuInfoData_RX.byIdcodeSize = 0;	
	// RevRxNo120910-001 Append Line
	s_McuInfoData_RX.dwMcuJtagIdcodeNum = 1;
	// RevRxNo120910-001 Modify Line
	s_McuInfoData_RX.dwMcuJtagIdcode[0] = 0;
	s_McuInfoData_RX.dwMcuDeviceCode = 0;
	// RevRxNo120910-001 Append Line
	s_McuInfoData_RX.dwMcuFineDeviceCodeNum = 0;
	// RevRxNo120910-001 Append Line
	s_McuInfoData_RX.wOcdTrcCycMax = 0;
	// RevRxNo120910-001 Append Start
	s_McuInfoData_RX.fTgtPwrVal = f33v;	// 初期値 3.3[V]
	s_McuInfoData_RX.dwVdet0LevelNum = 0;
	s_McuInfoData_RX.dwEmuAccCtrlAreaNum = 0;
	// RevRxNo120910-001 Append End

	// RevRxNo130301-001 Append Start
	s_McuInfoData_RX.eCpuIsa = RX_ISA_RXV1;
	s_McuInfoData_RX.eFpuSet = RX_FPU_OFF;
	// RevRxNo130301-001 Append End

	// デバッグ情報格納構造体変数の初期化
	// ターゲットMCU空間に設定するデータ格納構造体変数の初期化
//	s_McudData.dwSetDataNum = INIT_MCUD_NUM;

	// プロセッサモード情報格納構造体変数の初期化
	s_ePmode = INIT_PMODE;

	// デバッグ情報格納構造体変数の初期化
	s_DbgData_RX.eOcdArgErrChk = OCD_ARGERRCHK_USE;
	s_DbgData_RX.eMcuMultiCore = EML_MCU_MULTI_NON;
	s_DbgData_RX.eTrcFuncMode = EML_TRCFUNC_TRC;
	s_DbgData_RX.eRomCorrection = EML_ROM_CORRECT_NON;
	s_DbgData_RX.eWriter = EML_WRITERMODE_NON;
	s_DbgData_RX.dwJtagByPass = 0;
	s_DbgData_RX.eUseProgram = EML_HEW;
	s_DbgData_RX.eDbgFlashWrite = EML_DBG_FLASHWRITE_NON;
	s_DbgData_RX.eBfrRunRegSet = EML_RUNREGSET_USE;
	s_DbgData_RX.eAftRunRegSet = EML_RUNREGSET_USE;
	s_DbgData_RX.eOcdRegAcsMode = OCD_ACSMODE_DUMPFILL;
	s_DbgData_RX.eSciIF = SCI_NON;
	// RevRxNo120910-001 Append Line
	s_DbgData_RX.eDbgBootSwap = RX_DBG_BOOTSWAP_NON;

	s_bHotPlugState = FALSE;			// ホットプラグ設定状態
	//RevRxNo140515-012 Append Line
	s_bHotPlgStartFlg = FALSE;			// ホットプラグ起動無効に設定
	s_bIdCodeResult = FALSE;			// ID認証結果設定状態


	// RevRxNo130301-001 Append Start
	// フラッシュメモリコンフィギュレーションデータ情報初期化
	s_Fmcd.dwSetNum = 0;
	// RevRxNo130301-001 Append End

	return;
}
