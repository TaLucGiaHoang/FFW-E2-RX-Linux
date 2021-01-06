///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_sfr.cpp
 * @brief MCU SFR制御関数
 * @author RSO Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, K.Uemori, S.Ueda, SDS T.Iwata, Y.Kawakami
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/12/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120615-001	2012/07/11 橋口
  ライタモード時のHOCO切り替え処理改善(HocoChange()削除)
・V.1.03 RevRxNo120112-001 FFW SFRアクセス見直し対応 2012/07/11 橋口
  →RevRxNo120615-001対応でHocoChange()を削除したため、修正なし。
・RevRxNo120606-003 2012/07/12 橋口
  ・SYSCR0レジスタがないMCU対応
・RevRxNo120910-001	2012/09/13 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・GetStatData()で、引数のbyStatKindをdwStatKindに変更。
・RevRxNo121026-001 2012/11/01 植盛
  RX100量産対応
・RevRxNo121022-001	2012/11/28 SDS 岩田
	EZ-CUBE PROT_MCU_DUMP()分割処理対応
・RevRxNo121206-001 2012/12/07 植盛
  ブートスワップ時のキャッシュ処理対応
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001 2013/06/10 植盛
	RX64M対応
・RevRxNo130828-001 2013/11/11 植盛
	スタートアッププログラム保護機能によるスワップ時のキャッシュ更新処理改善
・RevRxNo130411-001 2014/01/21 植盛
	enum FFWRXENM_MDE_TYPE型のメンバ名変更(RX_MDE_OSM→RX_MDE_FLASH)
・RevRxNo130730-001 2014/06/20 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140515-005 2014/07/18 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo140515-006 2014/08/28 川上
	RX231対応
・RevRxE2LNo141104-001 2015/01/20 上田
	E2 Lite対応
・RevRxNo150827-002 2015/08/28 PA 紡車
	RX651オプション設定メモリアドレス、配置変更対応
・RevRxNo161003-001 2016/12/01 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
*/
#ifdef _WIN32
#include <windows.h>
#elif __linux__
#endif

#include "ffw_sys.h"
#include "mcu_sfr.h"
#include "ffwmcu_brk.h"
#include "ffwmcu_mem.h"
#include "ffwmcu_dwn.h"
#include "domcu_mcu.h"
#include "domcu_prog.h"
#include "protmcu_mem.h"
#include "domcu_rst.h"

#include "protmcu_mcu.h"
#include "mcu_extflash.h"
#include "mcu_flash.h"
#include "mcu_brk.h"
#include "mcuspec.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line

#include "do_sys.h"		// RevRxNo121022-001 Append Line
#include "mcu_mem.h"	// RevRxNo121022-001 Append Line

// static変数
static WORD		s_wRamEnableData;			// RAM有効/無効レジスタデータ退避変数
// V.1.02 覚え書き32 RAM停止時の処理対応 Append Start
static DWORD	s_dwMstpcrcData;			// モジュールストップコントロールレジスタC値退避変数
// V.1.02 覚え書き32 RAM停止時の処理対応 Append End
// V.1.02 新デバイス対応 Append Line
static WORD		s_wPrcrData;				// プロテクトレジスタデータ退避変数

static WORD s_wEscPrcr;			//PRCRの設定値を格納

// RevRxNo121206-001 Append Line
static DWORD		s_dwBootSwapSize;			// ブートスワップのサイズ値

// static関数の宣言

//=============================================================================
/**
 * 内蔵ROM無効拡張モードの状態の取得
 * @param bRomEna : TRUE MCU内蔵ROM有効/FALSE MCU内蔵ROM無効
 * @param bExtBus : TRUE MCU外部バス有効/FALSE MCU外部バス無効
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetMcuRomExtBusStatInfo(BOOL* bRomEna,BOOL* bExtBus)
{

	// V.1.02 覚書 No.25　SetPmodeInRomReg2Flg()処理修正 Modify Start
	FFWERR	ferr = FFWERR_OK;
	MADDR madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;

	//RevNo110316-001 Modify Start
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// 参照データ格納領域

	//RevRxNo120606-003 Append Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	enum FFWRXENM_RMODE			eRmode;

	pMcuInfoData = GetMcuInfoDataRX();
	//RevRxNo120606-003 Append Ene

	//RevRxNo120606-003 Modify Start
	if((pMcuInfoData->dwSpc[0] & SPC_SYSCR0_READ) == SPC_SYSCR0_READ_OK) {
		//SYSCR0があるMCUの場合
		madrStartAddr = MCU_REG_SYSTEM_SYSCR0;
		eAccessSize = MWORD_ACCESS;
		pbyReadData = reinterpret_cast<BYTE*>(wReadData);
		ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT,pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	} else {
		//SYSCR0がないMCUの場合
		eRmode = GetRmodeDataRX();
		wReadData[0] = MCU_REG_SYSTEM_SYSCR0_ROME;
		// RevRxNo161003-001 Modify Start
		if((eRmode == RX_RMODE_SNG) || (eRmode == RX_RMODE_SNG_DUAL_BANK0) || (eRmode == RX_RMODE_SNG_DUAL_BANK1) || (eRmode == RX_RMODE_SNG_DUAL)){
			wReadData[0] = MCU_REG_SYSTEM_SYSCR0_ROME;
		} else if((eRmode == RX_RMODE_ROME) || (eRmode == RX_RMODE_ROME_DUAL_BANK0) || (eRmode == RX_RMODE_ROME_DUAL_BANK1) || (eRmode == RX_RMODE_ROME_DUAL)){
			wReadData[0] = MCU_REG_SYSTEM_SYSCR0_ROME | MCU_REG_SYSTEM_SYSCR0_EXBE;
		} else if((eRmode == RX_RMODE_ROMD) || (eRmode == RX_RMODE_ROMD_DUAL_BANK0) || (eRmode == RX_RMODE_ROMD_DUAL_BANK1) || (eRmode == RX_RMODE_ROMD_DUAL)){
			wReadData[0] = MCU_REG_SYSTEM_SYSCR0_ROMD | MCU_REG_SYSTEM_SYSCR0_EXBE;
		}
		// RevRxNo161003-001 Modify Start

	}
	//RevRxNo120606-003 Modify End

	//ROM有効無効状態取得
	if((wReadData[0] & MCU_REG_SYSTEM_SYSCR0_ROME) == MCU_REG_SYSTEM_SYSCR0_ROME){
		//ROM有効
		*bRomEna = TRUE;
	} else {
		//ROM無効
		*bRomEna = FALSE;
	}

	//外部バス有効無効状態取得
	if((wReadData[0] & MCU_REG_SYSTEM_SYSCR0_EXBE) == MCU_REG_SYSTEM_SYSCR0_EXBE){
		//外部バス有効
		*bExtBus = TRUE;
	} else {
		//外部バス無効
		*bExtBus = FALSE;
	}

	
	return ferr;
}

//=============================================================================
/**
 * MCU ICKの設定値取得
 * @param pdwIck	SCKCRに設定されているICK値格納ポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetMcuSckcrIckVal(DWORD* pdwIck)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域

	madrStartAddr = MCU_REG_SYSTEM_SCKCR;
	eAccessSize = MLWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
	ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	*pdwIck = (dwReadData[0] & MCU_REG_SYSTEM_SCKCR_ICK) >> 24;

	return ferr;
}


//=============================================================================
/**
 * MCUの起動モードがユーザブートかどうかの取得 (MDSRのUBTSビット状態取得)
 * @param pbUbts	TRUE:ユーザブート起動 FALSE:ユーザブート起動ではない
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetMcuUsrBootMode(BOOL* pbUbts)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// 参照データ格納領域

	madrStartAddr = MCU_REG_SYSTEM_MDSR;
	eAccessSize = MWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//起動時のユーザブートモード
	if((wReadData[0] & MCU_REG_SYSTEM_MDSR_UBTS) == MCU_REG_SYSTEM_MDSR_UBTS){
		//ユーザブート起動
		*pbUbts = TRUE;
	} else {
		//ユーザブート起動ではない
		*pbUbts = FALSE; 
	}
	return ferr;

}

//=============================================================================
/**
 * MCUで認識しているエンディアン取得取得
 * @param eMcuEndian ENDIAN_LITTLE/ENDIAN_BIG/OFS_ENDIAN_OTH(0x3)
 *                   OFS_ENDIAN_OTHはRX63x/RX200でMDEレジスタがLITTLE(b'111),BIG(b'000)以外の場合に返る。
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetMcuEndian(enum FFWENM_ENDIAN* eMcuEndian)
{	
	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrReadAddr;
	DWORD						dwAccessCount;		// DUMP時のアクセスサイズによるアクセス回数
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// 参照データ格納領域
	DWORD						dwReadData[1];		// 参照データ格納領域
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BOOL						bBootSwap = FALSE;	// RevRxNo130730-001 Append Line

	enum FFWRXENM_PMODE			ePmode;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*			pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWMCU_MCUDEF_DATA*			pMcuDef;	// RevRxNo150827-002 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	pMcuDef = GetMcuDefData();	// RevRxNo150827-002 Append Line

	// RevRxNo130411-001 Modify Line
	if (pFwCtrl->eMdeType == RX_MDE_PIN) {		// MDEをMDE端子で設定するMCUの場合
		madrReadAddr = MCU_REG_SYSTEM_MDMONR;
		eAccessSize = MWORD_ACCESS;
		pbyReadData = reinterpret_cast<BYTE*>(wReadData);

		ferr = GetMcuSfrReg(madrReadAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if((wReadData[0] & MCU_REG_SYSTEM_MDMONR_MDE) == MCU_REG_SYSTEM_MDMONR_MDE_BIG){
			//ビッグエンディアンの場合
			*eMcuEndian = ENDIAN_BIG;
		} else {
			//リトルエンディアンの場合
			*eMcuEndian = ENDIAN_LITTLE;
		}

	// RevRxNo130411-001 Modify Start
	} else {	// MDEをフラッシュROMまたはExtra領域で設定するMCUの場合
		if (pFwCtrl->eMdeType == RX_MDE_FLASH) {	// MDEをフラッシュROMで設定するMCUの場合	// RevRxNo130411-001 Modify Line
			ePmode = GetPmodeDataRX();
			if( ePmode == RX_PMODE_SNG ){
				// RevRxNo130730-001 Modify Start
				if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
					// フラッシュがMF3またはSC32用MF3の場合

					ferr = ChkBootSwap(&bBootSwap);			// IOレジスタからブートスワップの設定状態を確認する。
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					if(bBootSwap == TRUE) {
						// MCUのリセットによりブートスワップする→しないになった場合
						// リセットにより代替え領域からデフォルト領域に入れ替わるため、
						// 代替え領域に相当するMDEレジスタアドレスを算出する
						madrReadAddr = MCU_OSM_MDES_START - s_dwBootSwapSize;
					} else {
						// リセットしてもスワップしないため、デフォルト領域のMDEレジスタアドレスを使用
						madrReadAddr = MCU_OSM_MDES_START;
					}
				} else {		// フラッシュがMF3またはSC32用MF3以外の場合
					madrReadAddr = MCU_OSM_MDES_START;
				}
				// RevRxNo130730-001 Modify End
			}else if( ePmode == RX_PMODE_USRBOOT ){
				madrReadAddr = MCU_OSM_MDEB_START;
			}
		// RevRxNo130301-001 Append Start
		} else {	// MDEをExtra領域で設定するMCUの場合
			madrReadAddr = pMcuDef->madrMdeStartAddr;		// RevRxNo150827-002 Modify Line
		// RevRxNo130301-001 Append End
		}
		eAccessSize = MLWORD_ACCESS;
		pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
		dwAccessCount = 1;
		dwAreaNum = 1;
		bSameAccessSize = FALSE;
		bSameAccessCount = FALSE;
		memset(pbyReadData, 0, sizeof(WORD));
		ferr = PROT_MCU_DDUMP(dwAreaNum, &madrReadAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if( (dwReadData[0] & MCU_OSM_MDE_MASK_DATA) == MCU_OSM_MDE_BIG_DATA){		// bit0-2が000だったらビッグエンディアン
			*eMcuEndian = ENDIAN_BIG;
		} else if( (dwReadData[0] & MCU_OSM_MDE_MASK_DATA) == MCU_OSM_MDE_LITTLE_DATA ){
			*eMcuEndian = ENDIAN_LITTLE;
		} else {
			*eMcuEndian = (enum FFWENM_ENDIAN)OFS_ENDIAN_OTH;
		}
	}
	// RevRxNo130411-001 Modify End
	return ferr;
}


// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * OFS1レジスタ値取得
 * @param pdwOfs1Val 現在のOFS1レジスタ値格納ポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetMcuOfs1(DWORD* pdwOfs1Val)
{	
	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrReadAddr;
	DWORD						dwAccessCount;		// DUMP時のアクセスサイズによるアクセス回数
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BOOL						bBootSwap = FALSE;

	FFWMCU_FWCTRL_DATA*			pFwCtrl;
	FFWMCU_MCUDEF_DATA* 		pMcuDef;	// RevRxNo150827-002 Append Line

	pFwCtrl = GetFwCtrlData();
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	if (pFwCtrl->eOfsType == RX_OFS_FLASH) {	// OFSレジスタはフラッシュROMで設定するMCUの場合
		if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
			// フラッシュがMF3またはSC32用MF3の場合

			ferr = ChkBootSwap(&bBootSwap);			// IOレジスタからブートスワップの設定状態を確認する。
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			if(bBootSwap == TRUE) {
				// MCUのリセットによりブートスワップする→しないになった場合
				// リセットにより代替え領域からデフォルト領域に入れ替わるため、
				// 代替え領域に相当するOFS1レジスタアドレスを算出する
				madrReadAddr = MCU_OSM_OFS1_START - s_dwBootSwapSize;
			} else {
				// リセットしてもスワップしないため、デフォルト領域のOFS1レジスタアドレスを使用
				madrReadAddr = MCU_OSM_OFS1_START;
			}
		} else {		// フラッシュがMF3またはSC32用MF3以外の場合
			madrReadAddr = MCU_OSM_OFS1_START;
		}
	} else if (pFwCtrl->eOfsType == RX_OFS_EXTRA) {	// OFSレジスタはExtra領域で設定するMCUの場合
		madrReadAddr = pMcuDef->madrOfs1StartAddr;	// RevRxNo150827-002 Modify Line
	}

	eAccessSize = MLWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
	dwAccessCount = 1;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	memset(pbyReadData, 0, sizeof(WORD));
	ferr = PROT_MCU_DDUMP(dwAreaNum, &madrReadAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	*pdwOfs1Val = dwReadData[0];

	return ferr;
}
// RevRxNo130730-001 Append End


//=============================================================================
/**
 * MCU PRCRに設定
 * @param wPrcr	Prcrレジスタに設定する値 bit0:PRCR0 1(許可)/0(プロテクト) bit1:PRCR1 1(許可)/0(プロテクト) bit3:PRCR3 1(許可)/0(プロテクト)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetMcuPrcrProtect(WORD wPrcr)
{
	FFWERR						ferr = FFWERR_OK;
	MADDR						madrWriteAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyData;		// 参照データを格納する領域のアドレス
	WORD						wWriteData[1];		// 参照データ格納領域

	madrWriteAddr = MCU_REG_SYSTEM_PRCR;
	eAccessSize = MWORD_ACCESS;

	pbyData = reinterpret_cast<BYTE*>(wWriteData);
	wWriteData[0] =  (WORD)(MCU_REG_SYSTEM_PRCR_PRKEY | wPrcr);
	ferr = SetMcuSfrReg(madrWriteAddr, eAccessSize, pbyData);
	return ferr;
}
//=============================================================================
/**
 * MCU PRCRに設定
 * @param wPrcr	Prcrレジスタに設定する値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetMcuPrcrProtect(WORD* pwPrcr)
{

	FFWERR	ferr = FFWERR_OK;
	MADDR madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// 参照データ格納領域

	madrStartAddr = MCU_REG_SYSTEM_PRCR;
	eAccessSize = MWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	*pwPrcr = wReadData[0];

	return ferr;
}
//=============================================================================
/**
 * MCU PRCRプロテクトの退避/復帰
 * @param bEsc:TRUE(退避)/FALSE(復帰)
 * @param wPrcr: Prcrレジスタに設定する値 bit0:PRCR0 1(許可)/0(プロテクト) bit1:PRCR1 1(許可)/0(プロテクト) bit3:PRCR3 1(許可)/0(プロテクト)
 * @param pbProtectFree: 退避結果を格納 TRUE(退避しました)/FALSE(退避しませんでした)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR EscMcuPrcrProtect(BOOL bEsc,WORD wPrcr,BOOL* pbProtectFree)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wBuff;

	*pbProtectFree = FALSE;
	if(bEsc == TRUE){
		ferr = GetMcuPrcrProtect(&wBuff);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if( (wBuff & wPrcr) != wPrcr){		// プロテクトがかかっている場合
			*pbProtectFree = TRUE;
			s_wEscPrcr = (WORD)(wBuff | wPrcr);
			ferr = SetMcuPrcrProtect(s_wEscPrcr);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}else {
		wBuff = (WORD)(s_wEscPrcr & ~wPrcr);
		ferr = SetMcuPrcrProtect(wBuff);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

	return ferr;

}

//=============================================================================
/**
 * 内蔵RAM有効/無効設定と内蔵RAM動作/停止設定を行う
 * @param  bEnable   TRUE  : 内蔵RAM有効化
 *                   FALSE : 元の設定に戻す
 * @param  pbRamBlockEna RAM領域重複有無情報格納配列ポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo130301-001 Modify Line
FFWERR SetRegRamEnable(BOOL bEnable, BOOL* pbRamBlockEna)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRWaddr;			// 参照/設定するレジスタのアドレス
	WORD						wReadData[1];		// 参照データ格納領域
	WORD						wWriteData[1];		// 設定データ格納領域
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	WORD						wData;				// 実際に書き込むデータ

	// RevRxNo130301-001 Append Start
	DWORD	i;
	MADDR						madrSyscr1;			// SYSCR1アドレス値
	enum FFWENM_MACCESS_SIZE	eSyscr1Size;		// SYSCR1アクセスサイズ
	WORD						wSyscr1Data;		// SYSCR1設定値
	WORD						wSyscr1Mask;		// SYSCR1マスク値
	MADDR						madrMstpcrc;		// MSTPCRCアドレス値
	enum FFWENM_MACCESS_SIZE	eMstpcrcSize;		// MSTPCRCアクセスサイズ
	DWORD						dwMstpcrcData;		// MSTPCRC設定値
	DWORD						dwMstpcrcMask;		// MSTPCRCマスク値
	// RevRxNo130301-001 Append End

	// V.1.02 覚え書き32 RAM停止時の処理対応 Append Start
	DWORD						dwReadData[1];		// 参照データ格納領域
	DWORD						dwWriteData[1];		// 設定データ格納領域
	DWORD						dwData;				// 実際に書き込むデータ
	// V.1.02 覚え書き32 RAM停止時の処理対応 Append End
	// V.1.02 覚え書き26 RX630/RX210 SYSCR1プロテクト設定処理追加 Append Start
	BOOL						bProtectFree;		// プロテクト解除記録用 
	BOOL						bEsc;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*			pFwCtrl;	// RevRxNo130411-001 Append Line
	// V.1.02 覚え書き26 RX630/RX210 SYSCR1プロテクト設定処理追加 Append End
	// RevRxNo130301-001 Append Start
	FFWRX_MCUINFO_DATA* pMcuInfo;
	FFWRX_MCUAREA_DATA* pMcuArea;
	// RevRxNo130301-001 Append End

	// V.1.02 覚え書き26 RX630/RX210 SYSCR1プロテクト設定処理追加 Append Line
	bProtectFree = FALSE;

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();		// RevRxNo130411-001 Append Line
	pMcuInfo = GetMcuInfoDataRX();	// RevRxNo130301-001 Append Line
	pMcuArea = GetMcuAreaDataRX();	// RevRxNo130301-001 Append Line

	// RevRxNo130301-001 Append Start
	// SYSCR1/MSTPCRC設定で使用する内部変数を初期化
	// 二次元配列に格納されているアドレスとサイズ情報は、どの配列でも同じ値である。
	// そのため、[3][0]/[4][0]の値を変数に設定する。
	madrSyscr1 = pMcuInfo->dwmadrMcuRegInfoAddr[3][0];
	eSyscr1Size = pMcuInfo->eMcuRegInfoAccessSize[3][0];
	wSyscr1Data = 0x0000;
	wSyscr1Mask = 0x0000;
	madrMstpcrc = pMcuInfo->dwmadrMcuRegInfoAddr[4][0];
	eMstpcrcSize = pMcuInfo->eMcuRegInfoAccessSize[4][0];
	dwMstpcrcData = 0x00000000;
	dwMstpcrcMask = 0x00000000;
	// RevRxNo130301-001 Append End

	// RevRxNo130301-001 Modify Start
	// 内蔵RAM有効化の場合
	if (bEnable == TRUE) {

		for (i = 0; i < pMcuArea->dwRamAreaNum; i++) {
			if (pbRamBlockEna[i] == TRUE) {		// i番目のRAM領域と重複している場合

				// i番目のRAM領域に対応したSYSCR1/MSTPCRC値の設定値を格納
				// SYSCR1関連
				wSyscr1Data |= static_cast<WORD>(pMcuInfo->dwMcuRegInfoData[3][i]);	// SYSCR1設定値(RAM領域毎に値が異なるため、論理和)
				wSyscr1Mask |= static_cast<WORD>(pMcuInfo->dwMcuRegInfoMask[3][i]);	// SYSCR1マスク値(RAM領域毎に値が異なるため、論理和)

				// MSTPCRC関連
				dwMstpcrcData |= pMcuInfo->dwMcuRegInfoData[4][i];			// MSTPCRC設定値(RAM領域毎に値が異なるため、論理和)
				dwMstpcrcMask |= pMcuInfo->dwMcuRegInfoMask[4][i];			// MSTPCRCマスク値(RAM領域毎に値が異なるため、論理和)
			}
		}

		// SYSCR1レジスタの読み出し
		madrRWaddr = madrSyscr1;
		eAccessSize = eSyscr1Size;
		pbyReadData = reinterpret_cast<BYTE*>(wReadData);
		ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		memcpy(&s_wRamEnableData, pbyReadData, sizeof(WORD));		// ユーザ値を記憶
		// SYSCR1レジスタ書き込みデータをセット
		wData = (s_wRamEnableData & ~(wSyscr1Mask)) | wSyscr1Data;

		// V.1.02 覚え書き32 RAM停止時の処理対応 Append Start
		// モジュールストップコントロールレジスタCの読み出し
		madrRWaddr = madrMstpcrc;
		eAccessSize = eMstpcrcSize;
		pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
		// V.1.02 RevNo110304-003 Modify Line
		ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		memcpy(&s_dwMstpcrcData, pbyReadData, sizeof(DWORD));		// 設定内容を記憶
		// モジュールストップコントロールレジスタC書き込みデータをセット
		dwData = (s_dwMstpcrcData & ~(dwMstpcrcMask)) | dwMstpcrcData;
		// V.1.02 覚え書き32 RAM停止時の処理対応 Append End

	} else {	// 元の設定に戻す場合

		// レジスタ書き込みデータをセット
		wData = s_wRamEnableData;
		// V.1.02 覚え書き32 RAM停止時の処理対応 Append Line
		dwData = s_dwMstpcrcData;
	}
	// RevRxNo130301-001 Modify End
	
	// V.1.02 覚え書き26 RX630/RX210 SYSCR1プロテクト解除処理追加 Append Start
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
		bEsc = TRUE;
		ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// V.1.02 覚え書き26 RX630/RX210 SYSCR1プロテクト解除処理追加 Append End

	// SYSCR1レジスタに書き込み
	pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);
	memcpy(pbyWriteData, &wData, sizeof(WORD));
	// RevRxNo130301-001 Modify Start
	madrRWaddr = madrSyscr1;
	// V.1.02 RevNo110304-003 Appned Line
	eAccessSize = eSyscr1Size;
	// RevRxNo130301-001 Modify End
	ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.02 覚え書き32 RAM停止時の処理対応 Append Start
	// モジュールストップコントロールレジスタCに書き込み
	pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
	memcpy(pbyWriteData, &dwData, sizeof(DWORD));
	// RevRxNo130301-001 Modify Start
	madrRWaddr = madrMstpcrc;
	// V.1.02 RevNo110304-003 Appned Line
	eAccessSize = eMstpcrcSize;
	// RevRxNo130301-001 Modify End
	// V.1.02 RevNo110304-003 Modify Line
	ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.02 覚え書き32 RAM停止時の処理対応 Append End

	// V.1.02 覚え書き26 RX630/RX210 SYSCR1プロテクト設定処理追加 Append Start
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
		if(bProtectFree){
			bEsc = FALSE;
			ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// V.1.02 覚え書き26 RX630/RX210 SYSCR1プロテクト設定処理追加 Append End

	return FFWERR_OK;
}

//=============================================================================
/**
 * ターゲットMCUのステータス情報を取得する。
 * @param dwStatKind 取得対象の情報(SFR/JTAG/EML)を指定する
 * @param peStatMcu  ターゲットMCUステータス情報を格納する変数のアドレス
 * @param pStatSFR   SFR情報を格納する
 * @param pStatJTAG  JTAG情報を格納する
 * @param pStatFINE  FINE情報を格納する		// RevRxNo120910-001 Append Line
 * @param pStatEML   EML情報を格納する
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo120910-001 Modify Line
FFWERR GetStatData(DWORD dwStatKind, enum FFWRXENM_STAT_MCU* peStatMcu, FFWRX_STAT_SFR_DATA* pStatSFR, 
				 FFWRX_STAT_JTAG_DATA* pStatJTAG, FFWRX_STAT_FINE_DATA* pStatFINE, FFWRX_STAT_EML_DATA* pStatEML)
{
	FFWERR	ferr = FFWERR_OK;

	// RevRxNo120910-001 Modify Line
	ferr = PROT_MCU_GetRXSTAT(dwStatKind, peStatMcu, pStatSFR, pStatJTAG, pStatFINE, pStatEML);

	// RevRxE2LNo141104-001 Append Start
	// SFR情報を取得した場合、内蔵ROM無効拡張モードフラグを更新する。
	if ((dwStatKind & STAT_KIND_SFR) == STAT_KIND_SFR) {
		// peStatMcu->byStatProcModeには、GetMcuRomExtBusStatInfo()と同等の処理(SYSCR0のあり/なしを判定して値を格納)を
		// BFWで実施した結果が格納される。

		// 内蔵ROM無効拡張モードの場合、内蔵ROM無効拡張モードフラグをセット
		if ((pStatSFR->byStatProcMode & MCU_REG_SYSTEM_SYSCR0_ROME) != MCU_REG_SYSTEM_SYSCR0_ROME) {
			SetPmodeInRomDisFlg(TRUE);

		// 内蔵ROM無効拡張モード以外の場合、内蔵ROM無効拡張モードフラグをクリア
		} else {
			SetPmodeInRomDisFlg(FALSE);
		}

		if (GetMcuRunState() == FALSE) {
			// プログラム停止中の場合、内蔵ROM無効拡張モード状態参照レジスタリードフラグをクリア
			SetReadPmodeInRomDisRegFlg(FALSE);
		}
	}
	// RevRxE2LNo141104-001 Append End

	return ferr;
}


//==============================================================================
/**
 * SFRへの書き込み
 * @param madrWriteAddr 書き込み対象アドレス
 * @param dwData        書き込みデータ
 * @param eAccessSize   アクセスサイズ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetMcuSfrReg(MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyData)
{
	FFWERR						ferr;
	enum FFWENM_VERIFY_SET		eVerify;				// ベリファイON/OFF
	FFW_VERIFYERR_DATA			VerifyErr;				// ベリファイ結果格納構造体
	BYTE						byWriteData[4];			// 設定データを格納する領域のアドレス
	DWORD						dwWriteDataSize;
	DWORD						dwAccessCount;
	BYTE						byEndian;

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	//エンディアン取得
	GetEndianType(madrWriteAddr,&byEndian);
	eVerify = VERIFY_OFF;

	// レジスタ設定
	dwAccessCount = 1;			//1回アクセス
	switch(eAccessSize){
	case MBYTE_ACCESS:
		dwWriteDataSize = MDATASIZE_1BYTE;
		break;
	case MWORD_ACCESS:
		dwWriteDataSize = MDATASIZE_2BYTE;
		break;
	case MLWORD_ACCESS:
		dwWriteDataSize = MDATASIZE_4BYTE;
		break;
	default:
		dwWriteDataSize = MDATASIZE_4BYTE;
		break;
	}

	memcpy(&byWriteData, pbyData, dwWriteDataSize);
	//ビッグエンディアンの場合はスワップさせる
	if(byEndian==FFWRX_ENDIAN_BIG){
		ReplaceEndian(byWriteData, eAccessSize, dwAccessCount);
	}

	ferr = PROT_MCU_FILL(eVerify, madrWriteAddr, eAccessSize, dwAccessCount, dwWriteDataSize, byWriteData, &VerifyErr,byEndian);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}

//==============================================================================
/**
 * SFRからの読み込み
 * @param eadrReadAddr 読み込み対象アドレス
 * @param eAccessSize      アクセスサイズ
 * @param dwAccessCount      アクセス回数
 * @param pdwData      読み込みデータ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetMcuSfrReg(MADDR madrReadAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, BYTE* pbyData)
{
	FFWERR						ferr;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwAreaNum;
	DWORD						dwSize;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BYTE						byEndian;
	DWORD						dwReadDataLength;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
// RevRxNo121022-001 Append Start

	// レジスタ設定
	dwAreaNum = 1;
	bSameAccessSize = TRUE;
	bSameAccessCount = TRUE;
	//エンディアン取得
	GetEndianType(madrReadAddr,&byEndian);

	switch(eAccessSize){
	case MBYTE_ACCESS:
		dwSize = MDATASIZE_1BYTE;
		break;
	case MWORD_ACCESS:
		dwSize = MDATASIZE_2BYTE;
		break;
	case MLWORD_ACCESS:
		dwSize = MDATASIZE_4BYTE;
		break;
	default:
		dwSize = MDATASIZE_4BYTE;
		break;
	}
	dwReadDataLength = dwSize*dwAccessCount;
	pbyReadData = new BYTE [dwReadDataLength];

	memset(pbyReadData, 0, dwReadDataLength);

// RevRxNo121022-001 Append Start
	if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
		ferr = McuDumpDivide(dwAreaNum, madrReadAddr, bSameAccessSize, eAccessSize, bSameAccessCount, dwAccessCount, pbyReadData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
	} else{
		ferr = PROT_MCU_DUMP(dwAreaNum, &madrReadAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadData, byEndian);
	}
// RevRxNo121022-001 Append End

	if (ferr != FFWERR_OK) {
		delete [] pbyReadData;
		return ferr;
	}

	//ビッグエンディアンの場合はスワップさせる
	if(byEndian == FFWRX_ENDIAN_BIG){
		ReplaceEndian(pbyReadData, eAccessSize, dwAccessCount);
	}
	memcpy(pbyData, pbyReadData, dwReadDataLength);		// 設定内容を記憶

	delete [] pbyReadData;

	return ferr;
}

// RevRxNo140515-005 Append Start
//==============================================================================
/**
 * SFRへの書き込み(CPUアクセスレジスタ用)
 * @param madrWriteAddr 書き込み対象アドレス
 * @param dwData        書き込みデータ
 * @param eAccessSize   アクセスサイズ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetMcuSfrReg_CPU(MADDR madrWriteAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyData)
{
	FFWERR						ferr;
	enum FFWENM_VERIFY_SET		eVerify;				// ベリファイON/OFF
	FFW_VERIFYERR_DATA			VerifyErr;				// ベリファイ結果格納構造体
	BYTE						byWriteData[4];			// 設定データを格納する領域のアドレス
	DWORD						dwWriteDataSize;
	DWORD						dwAccessCount;
	BYTE						byEndian;

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	//エンディアン取得
	GetEndianType(madrWriteAddr,&byEndian);
	eVerify = VERIFY_OFF;

	// レジスタ設定
	dwAccessCount = 1;			//1回アクセス
	switch(eAccessSize){
	case MBYTE_ACCESS:
		dwWriteDataSize = MDATASIZE_1BYTE;
		break;
	case MWORD_ACCESS:
		dwWriteDataSize = MDATASIZE_2BYTE;
		break;
	case MLWORD_ACCESS:
		dwWriteDataSize = MDATASIZE_4BYTE;
		break;
	default:
		dwWriteDataSize = MDATASIZE_4BYTE;
		break;
	}

	memcpy(&byWriteData, pbyData, dwWriteDataSize);
	//ビッグエンディアンの場合はスワップさせる
	if(byEndian==FFWRX_ENDIAN_BIG){
		ReplaceEndian(byWriteData, eAccessSize, dwAccessCount);
	}

	ferr =  PROT_MCU_CPUFILL(eVerify, madrWriteAddr, eAccessSize, dwAccessCount, dwWriteDataSize, byWriteData, &VerifyErr,byEndian);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}

//==============================================================================
/**
 * SFRからの読み込み(CPUアクセスレジスタ用)
 * @param eadrReadAddr 読み込み対象アドレス
 * @param eAccessSize      アクセスサイズ
 * @param dwAccessCount      アクセス回数
 * @param pdwData      読み込みデータ
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR GetMcuSfrReg_CPU(MADDR madrReadAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, BYTE* pbyData)
{
	FFWERR						ferr;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwAreaNum;
	DWORD						dwSize;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BYTE						byEndian;
	DWORD						dwReadDataLength;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
// RevRxNo121022-001 Append Start

	// レジスタ設定
	dwAreaNum = 1;
	bSameAccessSize = TRUE;
	bSameAccessCount = TRUE;
	//エンディアン取得
	GetEndianType(madrReadAddr,&byEndian);

	switch(eAccessSize){
	case MBYTE_ACCESS:
		dwSize = MDATASIZE_1BYTE;
		break;
	case MWORD_ACCESS:
		dwSize = MDATASIZE_2BYTE;
		break;
	case MLWORD_ACCESS:
		dwSize = MDATASIZE_4BYTE;
		break;
	default:
		dwSize = MDATASIZE_4BYTE;
		break;
	}
	dwReadDataLength = dwSize*dwAccessCount;
	pbyReadData = new BYTE [dwReadDataLength];

	memset(pbyReadData, 0, dwReadDataLength);

// RevRxNo121022-001 Append Start
	if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
		ferr = McuCpuDumpDivide(dwAreaNum, madrReadAddr, bSameAccessSize, eAccessSize, bSameAccessCount, dwAccessCount, pbyReadData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
	} else{
		ferr = PROT_MCU_CPUDUMP(dwAreaNum, &madrReadAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadData, byEndian);
	}
// RevRxNo121022-001 Append End

	if (ferr != FFWERR_OK) {
		delete [] pbyReadData;
		return ferr;
	}

	//ビッグエンディアンの場合はスワップさせる
	if(byEndian == FFWRX_ENDIAN_BIG){
		ReplaceEndian(pbyReadData, eAccessSize, dwAccessCount);
	}
	memcpy(pbyData, pbyReadData, dwReadDataLength);		// 設定内容を記憶

	delete [] pbyReadData;

	return ferr;
}
// RevRxNo140515-005 Append End

//=============================================================================
/**
 * ターゲットMCU制御関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData_Sfr(void)
{

	s_wRamEnableData = 0x01;	// デフォルト有効(0x01)

	return;
}

// RevRxNo121206-001 Append Start
//=============================================================================
/**
 * ブートスワップサイズ情報を取得する
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ChkBootSwapSizeInfo(void)
{
	FFWERR 						ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRWaddr;			// 設定/参照するレジスタのアドレス
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// ワード参照データ格納領域(バイトポインタ指定のため、配列定義)
	WORD						wSwapSizeVal;		// ブートスワップクラスタサイズ値

	// ワード単位レジスタ設定
	eAccessSize = MWORD_ACCESS;		// ワードアクセス

	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	memset(pbyReadData, 0, sizeof(WORD));

	// PORGAレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_PORGA_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	wSwapSizeVal = wReadData[0] & MCU_REG_FLASH_PORGA_MASK;

	switch (wSwapSizeVal) {
	case MCU_REG_FLASH_PORGA_SWAP_4KB:
		s_dwBootSwapSize = SWAP_SIZE_4KB;
		break;
	case MCU_REG_FLASH_PORGA_SWAP_8KB:
		s_dwBootSwapSize = SWAP_SIZE_8KB;
		break;
	case MCU_REG_FLASH_PORGA_SWAP_16KB_1:
	case MCU_REG_FLASH_PORGA_SWAP_16KB_2:
		s_dwBootSwapSize = SWAP_SIZE_16KB;
		break;
	default:
		s_dwBootSwapSize = SWAP_SIZE_8KB;
		break;
	}

	return ferr;
}
// RevRxNo121206-001 Append End

// RevRxNo121206-001 Append Start
//=============================================================================
/**
 * ブートスワップサイズ値を取得
 * @param  なし
 * @retval s_dwBootSwapSize ブートスワップサイズ値
 */
//=============================================================================
DWORD GetBootSwapSize(void)
{
	return s_dwBootSwapSize;
}
// RevRxNo121206-001 Append End

// RevRxNo121206-001 Append Start
//=============================================================================
/**
 * IOレジスタによるブートスワップ実施有無を確認する
 * @param  pbFlg ブートスワップ実施有無確認用フラグのポインタ
 *			TRUE ：リセット後、スワップ対象領域が入れ替わる
 * 			FALSE：リセット後、スワップ対象領域は入れ替わらない
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ChkBootSwap(BOOL* pbFlg)
{
	FFWERR 						ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRWaddr;			// 設定/参照するレジスタのアドレス
	BYTE						byReadData;			// バイト参照データ格納領域
	BYTE						byFisrVal;			// 不要なビットを0マスクしたFISRレジスタ値
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// ワード参照データ格納領域(バイトポインタ指定のため、配列定義)
	WORD						wFscmrVal;			// 不要なビットを0マスクしたFSCMRレジスタ値
	FFWMCU_FWCTRL_DATA*	pFwCtrl;					// RevRxNo140515-006 Append Line

	pFwCtrl = GetFwCtrlData();						// RevRxNo140515-006 Append Line

	*pbFlg = FALSE;		// フラグを初期化する

	// FISR、FSCMRレジスタを読み出して不要なビットを0マスクする
	// バイト単位レジスタ設定
	eAccessSize = MBYTE_ACCESS;		// バイトアクセス

	// FISRレジスタの読み出し
	// RevRxNo140515-006 Append Start
	if (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3) {
		madrRWaddr = MCU_REG_FLASH_FISR_RX230;
	} else {
		madrRWaddr = MCU_REG_FLASH_FISR_RX100;
	}
	// RevRxNo140515-006 Append End
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	byFisrVal = byReadData & MCU_REG_FLASH_FISR_MASK;


	// ワード単位レジスタ設定
	eAccessSize = MWORD_ACCESS;		// ワードアクセス

	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	memset(pbyReadData, 0, sizeof(WORD));

	// FSCMRレジスタの読み出し
	// RevRxNo140515-006 Append Start
	if (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3) {
		madrRWaddr = MCU_REG_FLASH_FSCMR_RX230;
	} else {
		madrRWaddr = MCU_REG_FLASH_FSCMR_RX100;
	}
	// RevRxNo140515-006 Append End
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	wFscmrVal = wReadData[0] & MCU_REG_FLASH_FSCMR_MASK;


	// RevRxNo130828-001 Modify Line
	// FISR値、FSCMR値からブートスワップ実施有無を判定し、フラグを操作する
	if (((byFisrVal == MCU_REG_FLASH_FISR_SWAP_DATA) && (wFscmrVal == MCU_REG_FLASH_FSCMR_NONSWAP_DATA))
		|| ((byFisrVal == MCU_REG_FLASH_FISR_NONSWAP_DATA) && (wFscmrVal == MCU_REG_FLASH_FSCMR_SWAP_DATA))) {

		*pbFlg = TRUE;		// フラグをTRUEにする
	}

	return ferr;
}
// RevRxNo121206-001 Append End
