///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_mcu.cpp
 * @brief BFWコマンド プロトコル生成関数(MCU依存コマンド)
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/21
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo111121-002		2012/07/11 橋口
  ・PROT_MCU_GetRXSTAT()
    ・「ROMレス等でのエンディアン情報差し替えは行わない」旨のコメントを追加
・RevRxNo120615-001 2012/07/12 橋口
  ライタモード時のHOCO切り替え処理改善
・RevRxNo120910-001	2012/10/17 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・PROT_MCU_GetRXSTAT()で、引数のbyStatKindをdwStatKindに変更。
  ・PROT_MCU_SetRXMCU(), PROT_MCU_GetRXMCU()で、JTAG IDコード、FINE IDコードを
    JTAG IDコード、FINE IDコード設定数分送信するように変更。
  ・メンバ名 dwFcuFilmLen を dwFcuFirmLen に全て変更。
・RevRxNo121022-001	2012/10/22 SDS 岩田
　　do_sys.hのインクルード処理を追加
　　PROT_MCU_SetRXMCU(), PROT_MCU_GetRXMCU()関数に、EZ-CUBE使用時のターゲット電圧処理を追加
・RevRxNo120910-001	2012/10/26 上田
  FFW I/F仕様変更に伴うFFWソース変更。
  ・PROT_MCU_GetRXSTAT()で、ユーザシステム電圧値受信処理を追加。
・RevNo121017-003	2012/10/30 上田
　VS2008 warning C4996対策
・RevRxNo121022-001	2012/11/1 SDS 岩田
　　PROT_MCU_SetRXMCU(), PROT_MCU_GetRXMCU()関数のエミュレータ種別 EZ-CUBE判定処理の定義名を変更
・RevRxNo130301-001 2013/04/16 上田
	RX64M対応
・RevRxNo130308-001 2013/08/21 上田 (2013/04/17 三宅担当分マージ)
　カバレッジ開発対応
・RevRxE2LNo141104-001 2014/11/21 上田
	E2 Lite対応
*/
#include <stdio.h>
#include <stdlib.h>

#include "protmcu_mcu.h"
#include "protmcu_mem.h"
#include "protmcu_prog.h"
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "ffwmcu_mcu.h"
#include "dorx_tra.h"
#include "domcu_prog.h"
#include "domcu_mem.h"

// RevRxNo121022-001 Append Line
#include "do_sys.h"

//=============================================================================
/**
 * BFWRX600Cmd_SetMCUコマンドの発行
 *  !!!!EZ-CUBEの場合 FFW→BFWDLL間で1回のデータ送信にCOM_SEND_BUFFER_SIZE(2048)バイト制限があるため、1回のPROT_SetMCUコマンドで超えないように注意すること!!!!
 * @param pMcuArea FFWRX_MCUAREA_DATA構造体のアドレス
 * @param pMcuInfo FFWRX_MCUINFO_DATA構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_SetRXMCU(const FFWRX_MCUAREA_DATA* pMcuArea, const FFWRX_MCUINFO_DATA* pMcuInfo)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wCmdCode;
	BYTE	byData;
	WORD	wBuf;
	DWORD	dwCnt, dwCnt2;
	DWORD	dwData;
	WORD	wDataH,wDataL;
	float	fTmp;
	// RevRxNo120910-001 Append Start
	BYTE	byMcuJtagIdcodeNum;
	BYTE	byMcuFineDeviceCodeNum;
	// RevRxNo120910-001 Append End

	// RevRxNo121022-001 Append Line
	FFWE20_EINF_DATA	einfData;

	wCmdCode = BFWCMD_SET_MCU;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo120910-001 Append Start
	// MCU内部FlashROM領域パターン数送信
	if (PutData4(pMcuArea->dwFlashRomPatternNum) != TRUE) {
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < pMcuArea->dwFlashRomPatternNum; dwCnt++) {
		// MCU内部FlashROM領域パターン開始アドレス送信
		if (PutData4(pMcuArea->dwmadrFlashRomStart[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		// MCU内部FlashROMブロックサイズ送信
		if (PutData4(pMcuArea->dwFlashRomBlockSize[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		// MCU内部FlashROMブロック数送信
		if (PutData4(pMcuArea->dwFlashRomBlockNum[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}
	// MCU内部Data(E2)FlashROM領域パターン数送信
	if (PutData4(pMcuArea->dwDataFlashRomPatternNum) != TRUE) {
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < pMcuArea->dwDataFlashRomPatternNum; dwCnt++) {
		// MCU内部Data(E2)FlashROM領域パターン開始アドレス送信
		if (PutData4(pMcuArea->dwmadrDataFlashRomStart[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		// MCU内部Data(E2)FlashROMブロックサイズ送信
		if (PutData4(pMcuArea->dwDataFlashRomBlockSize[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		// MCU内部Data(E2)FlashROMブロック数送信
		if (PutData4(pMcuArea->dwDataFlashRomBlockNum[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}
	// RevRxNo120910-001 Append End

	// FCUファームウェア格納アドレス送信
	if (PutData4(pMcuArea->dwAdrFcuFirmStart) != TRUE) {
		return FFWERR_COM;
	}

	// FCUファームウェアサイズ送信
	// RevRxNo120910-001 Modify Line
	if (PutData4(pMcuArea->dwFcuFirmLen) != TRUE) {
		return FFWERR_COM;
	}

	// FCU-RAMアドレス送信
	if (PutData4(pMcuArea->dwAdrFcuRamStart) != TRUE) {
		return FFWERR_COM;
	}

	// リセット時のISPレジスタ初期値送信
	if (PutData4(pMcuInfo->dwadrIspOnReset) != TRUE) {
		return FFWERR_COM;
	}

	// フラッシュ書き込みプログラム格納アドレス送信
	if (PutData4(pMcuInfo->dwadrWorkRam) != TRUE) {
		return FFWERR_COM;
	}

	// フラッシュ書き込みプログラム格納サイズ送信
	if (PutData4(pMcuInfo->dwsizWorkRam) != TRUE) {
		return FFWERR_COM;
	}

	// アキュムレータ、コプロセッサ有無送信
	BYTE byDataAcc;
	BYTE byDataCpen;
	byDataAcc =  static_cast<BYTE>(pMcuInfo->eAccSet);
	byDataCpen = static_cast<BYTE>(pMcuInfo->eCoProSet); 
	byData = (BYTE)(((byDataCpen & 0x1) < 1) | (byDataAcc & 0x1));
	if (PutData4(byData) != TRUE) {
		return FFWERR_COM;
	}

	// モードエントリパターン数送信
	if (PutData1(pMcuInfo->byModeEntryPtNum) != TRUE) {
		return FFWERR_COM;
	}
	// モードエントリパターン送信
	for (dwCnt = 0; dwCnt < pMcuInfo->byModeEntryPtNum; dwCnt++) {
		if (PutData2(pMcuInfo->wModeEntryPt[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}

	// モードエントリ時送信間隔送信
	if (PutData2(pMcuInfo->wSendInterValTimeME) != TRUE) {
		return FFWERR_COM;
	}
	// 通常デバッグ時送信間隔送信
	if (PutData2(pMcuInfo->wSendIntervalTimeNORMAL) != TRUE) {
		return FFWERR_COM;
	}
	// 受信完了->送信間隔送信
	if (PutData2(pMcuInfo->wRcvToSendIntervalTime) != TRUE) {
		return FFWERR_COM;
	}

	// MCU種別送信
	if (PutData1(pMcuInfo->byMcuType) != TRUE) {
		return FFWERR_COM;
	}
	// I/F種別送信
	if (PutData1(pMcuInfo->byInterfaceType) != TRUE) {
		return FFWERR_COM;
	}

	// RevRxNo120910-001 Append Start
	// PPCチャンネル数送信
	if (PutData2(pMcuInfo->wPPC) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxNo120910-001 Append End

	// SPC変数送信
	for (dwCnt = 0; dwCnt < MCU_SPC_MAX; dwCnt++) {
		if (PutData4(pMcuInfo->dwSpc[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}

	// 入力クロック値をDWORD型に置き換え
	// 上位2バイトを整数部、下位2バイトを小数部とする
	//  12.5MHzの場合は、0x00125000を格納する
	wDataH = (WORD)pMcuInfo->fClkExtal;
	fTmp = (pMcuInfo->fClkExtal - (float)wDataH) * 10000;
	wDataL = (WORD)fTmp;
	dwData = wDataH;
	dwData = (dwData << 16) | wDataL;
	if (PutData4(dwData) != TRUE) {	// 入力クロック値をDWORD型に置き換え
		return FFWERR_COM;
	}

	//IDコード格納開始アドレス
	if (PutData4(pMcuInfo->dwmadrIdcodeStart) != TRUE) {
		return FFWERR_COM;
	}
	if (PutData1(pMcuInfo->byIdcodeSize) != TRUE) {
		return FFWERR_COM;
	}

	// RevRxNo120910-001 Append Start
	// 対象デバイスJTAG IDコード設定数送信
	byMcuJtagIdcodeNum = static_cast<BYTE>(pMcuInfo->dwMcuJtagIdcodeNum);
	if (PutData1(byMcuJtagIdcodeNum) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxNo120910-001 Append End

	// 対象デバイス JTAG IDコード
	// RevRxNo120910-001 Modify Start
	for (dwCnt = 0; dwCnt < byMcuJtagIdcodeNum; dwCnt++) {
		if (PutData4(pMcuInfo->dwMcuJtagIdcode[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}
	// RevRxNo120910-001 Modify End
	// 対象デバイス デバイスコード
	if (PutData4(pMcuInfo->dwMcuDeviceCode) != TRUE) {
		return FFWERR_COM;
	}

	// 対象デバイス FINEメーカコード
	if (PutData2(pMcuInfo->wMcuFineMakerCode) != TRUE) {
		return FFWERR_COM;
	}
	// 対象デバイス FINEファミリコード
	if (PutData2(pMcuInfo->wMcuFineFamilyCode) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxNo120910-001 Append Start
	// 対象デバイスFINE デバイスコード設定数送信
	byMcuFineDeviceCodeNum = static_cast<BYTE>(pMcuInfo->dwMcuFineDeviceCodeNum);
	if (PutData1(byMcuFineDeviceCodeNum) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxNo120910-001 Append End
	// 対象デバイス FINEデバイスコード
	// RevRxNo120910-001 Modify Start
	for (dwCnt = 0; dwCnt < byMcuFineDeviceCodeNum; dwCnt++) {
		if (PutData2(pMcuInfo->wMcuFineDeviceCode[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}
	// RevRxNo120910-001 Modify End
	// 対象デバイス FINEバージョンコード
	if (PutData2(pMcuInfo->wMcuFineVersionCode) != TRUE) {
		return FFWERR_COM;
	}

	// V.1.02 V.1.01.90.206 Append Start
	// ユーザ指定MCU動作モード
	// V.1.02 V.1.01.00.215 Delete( BFWRXCmd_HPONで渡すことになったため )

	// MCUレジスタ情報
	// MCUレジスタ情報定義ブロック数
	if (PutData4(pMcuInfo->dwMcuRegInfoBlkNum) != TRUE) {
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < pMcuInfo->dwMcuRegInfoBlkNum; dwCnt++) {
		if (PutData4(pMcuInfo->dwMcuRegInfoNum[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		for (dwCnt2 = 0; dwCnt2 < pMcuInfo->dwMcuRegInfoNum[dwCnt]; dwCnt2++) {
			if (PutData4(pMcuInfo->dwmadrMcuRegInfoAddr[dwCnt][dwCnt2]) != TRUE) {
				return FFWERR_COM;
			}
			if (PutData4(pMcuInfo->eMcuRegInfoAccessSize[dwCnt][dwCnt2]) != TRUE) {
				return FFWERR_COM;
			}
			if (PutData4(pMcuInfo->dwMcuRegInfoMask[dwCnt][dwCnt2]) != TRUE) {
				return FFWERR_COM;
			}
			if (PutData4(pMcuInfo->dwMcuRegInfoData[dwCnt][dwCnt2]) != TRUE) {
				return FFWERR_COM;
			}
		}
	}
	// ROMレス情報
	if( pMcuArea->dwFlashRomPatternNum ){
		if (PutData1(0x00) != TRUE) {		// ROMあり
			return FFWERR_COM;
		}
	}else{
		if (PutData1(0x01) != TRUE) {		// ROMなし
			return FFWERR_COM;
		}
	}
	// V.1.02 V.1.01.90.206 Append End

	getEinfData(&einfData);						// エミュレータ情報取得

	// RevRxNo130301-001 Append Start
	if (einfData.wEmuStatus != EML_EZCUBE) {	// EZ-CUBEでない場合
		// 命令セットアーキテクチャ情報
		byData = static_cast<BYTE>(pMcuInfo->eCpuIsa);
		if (PutData1(byData) != TRUE) {
			return FFWERR_COM;
		}

		// FPU有無情報
		byData = static_cast<BYTE>(pMcuInfo->eFpuSet);
		if (PutData1(byData) != TRUE) {
			return FFWERR_COM;
		}
	}
	// RevRxNo130301-001 Append End

	// RevRxNo121022-001 Append Start
	// ターゲット電圧情報
	// EZ-CUBE使用時の場合 ターゲット電圧設定処理を実施。
	if (einfData.wEmuStatus == EML_EZCUBE){
		// ターゲット電圧値をDWORD型に置き換え
		// 上位2バイトを整数部、下位2バイトを小数部とする
		//  3.3Vの場合は、0x00033000を格納する
		wDataH = (WORD)pMcuInfo->fTgtPwrVal;
		fTmp = (pMcuInfo->fTgtPwrVal - (float)wDataH) * 10000;
		wDataL = (WORD)fTmp;
		dwData = wDataH;
		dwData = (dwData << 16) | wDataL;
		if (PutData4(dwData) != TRUE) {		// ターゲット電圧値をDWORD型に置き換え
			return FFWERR_COM;
		}
	}
	// RevRxNo121022-001 Append End

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWRX600Cmd_GetMCUコマンドの発行
 *  !!!!EZ-CUBEの場合 BFWDLL→FFW間で1回のデータ送信に2048バイト制限があるため、1回のPROT_GetMCUコマンドで超えないように注意すること!!!!
 * @param pMcuArea FFWRX_MCUAREA_DATA構造体のアドレス
 * @param pMcuInfo FFWRX_MCUINFO_DATA構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_GetRXMCU(FFWRX_MCUAREA_DATA* pMcuArea, FFWRX_MCUINFO_DATA* pMcuInfo)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wCmdCode;
	BYTE	byData;
	WORD	wBuf;
	DWORD	dwCnt, dwCnt2;
	DWORD	dwData;
	WORD	wDataH,wDataL;
	DWORD dwAcc;
	DWORD dwCpen;
	// RevRxNo120910-001 Append Start
	BYTE	byMcuJtagIdcodeNum;
	BYTE	byMcuFineDeviceCodeNum;
	// RevRxNo120910-001 Append End

	// RevRxNo121022-001 Append Line
	FFWE20_EINF_DATA			einfData;

	wCmdCode = BFWCMD_GET_MCU;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);		// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo120910-001 Append Start
	// MCU内部FlashROM領域パターン数
	if (GetData4(&pMcuArea->dwFlashRomPatternNum) != TRUE) {
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < pMcuArea->dwFlashRomPatternNum; dwCnt++) {
		// MCU内部FlashROM領域パターン開始アドレス
		if (GetData4(&pMcuArea->dwmadrFlashRomStart[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		// MCU内部FlashROMブロックサイズ
		if (GetData4(&pMcuArea->dwFlashRomBlockSize[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		// MCU内部FlashROMブロック数
		if (GetData4(&pMcuArea->dwFlashRomBlockNum[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}
	// MCU内部Data(E2)FlashROM領域パターン数
	if (GetData4(&pMcuArea->dwDataFlashRomPatternNum) != TRUE) {
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < pMcuArea->dwDataFlashRomPatternNum; dwCnt++) {
		// MCU内部Data(E2)FlashROM領域パターン開始アドレス
		if (GetData4(&pMcuArea->dwmadrDataFlashRomStart[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		// MCU内部Data(E2)FlashROMブロックサイズ
		if (GetData4(&pMcuArea->dwDataFlashRomBlockSize[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		// MCU内部Data(E2)FlashROMブロック数
		if (GetData4(&pMcuArea->dwDataFlashRomBlockNum[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}
	// RevRxNo120910-001 Append End

	// FCUファームウェア格納アドレス送信
	if (GetData4(&pMcuArea->dwAdrFcuFirmStart) != TRUE) {
		return FFWERR_COM;
	}

	// FCUファームウェアサイズ送信
	// RevRxNo120910-001 Modify Line
	if (GetData4(&pMcuArea->dwFcuFirmLen) != TRUE) {
		return FFWERR_COM;
	}

	// FCU-RAMアドレス送信
	if (GetData4(&pMcuArea->dwAdrFcuRamStart) != TRUE) {
		return FFWERR_COM;
	}

	// リセット時のISPレジスタ初期値送信
	if (GetData4(&pMcuInfo->dwadrIspOnReset) != TRUE) {
		return FFWERR_COM;
	}

	// フラッシュ書き込みプログラム格納アドレス送信
	if (GetData4(&pMcuInfo->dwadrWorkRam) != TRUE) {
		return FFWERR_COM;
	}

	// フラッシュ書き込みプログラム格納サイズ送信
	if (GetData4(&pMcuInfo->dwsizWorkRam) != TRUE) {
		return FFWERR_COM;
	}

	// アキュムレータ、コプロセッサ有無送信
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	dwAcc = dwData & 0x01;
	dwCpen = (dwData & 0x02) >> 1;
	pMcuInfo->eAccSet = static_cast<FFWENM_ACC_SET>(dwAcc);
	pMcuInfo->eCoProSet = static_cast<FFWENM_COPRO_SET>(dwCpen);

	// モードエントリパターン数送信
	if (GetData1(&pMcuInfo->byModeEntryPtNum) != TRUE) {
		return FFWERR_COM;
	}
	// モードエントリパターン送信
	for (dwCnt = 0; dwCnt < pMcuInfo->byModeEntryPtNum; dwCnt++) {
		if (GetData2(&pMcuInfo->wModeEntryPt[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}

	// モードエントリ時送信間隔送信
	if (GetData2(&pMcuInfo->wSendInterValTimeME) != TRUE) {
		return FFWERR_COM;
	}
	// 通常デバッグ時送信間隔送信
	if (GetData2(&pMcuInfo->wSendIntervalTimeNORMAL) != TRUE) {
		return FFWERR_COM;
	}
	// 受信完了->送信間隔送信
	if (GetData2(&pMcuInfo->wRcvToSendIntervalTime) != TRUE) {
		return FFWERR_COM;
	}

	// MCU種別送信
	if (GetData1(&pMcuInfo->byMcuType) != TRUE) {
		return FFWERR_COM;
	}
	// I/F種別送信
	if (GetData1(&pMcuInfo->byInterfaceType) != TRUE) {
		return FFWERR_COM;
	}

	// RevRxNo120910-001 Append Start
	// PPCチャンネル数
	if (GetData2(&pMcuInfo->wPPC) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxNo120910-001 Append End

	// SPC変数送信
	for (dwCnt = 0; dwCnt < MCU_SPC_MAX; dwCnt++) {
		if (GetData4(&pMcuInfo->dwSpc[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}

	if (GetData4(&dwData) != TRUE) {	// 入力クロック値
		return FFWERR_COM;
	}
	// 入力クロック値 DWORD型→floatに置き換え
	// Setでは上位2バイトを整数部、下位2バイトを小数部とする
	//  12.5MHzの場合は、0x00125000を格納しているので、これを逆変換
	wDataL = (WORD)dwData;
	wDataH = (WORD)(dwData >> 16);
	pMcuInfo->fClkExtal = (float)wDataH;
	pMcuInfo->fClkExtal += ((float)wDataL)/10000;

	//IDコード格納開始アドレス
	if (GetData4(&pMcuInfo->dwmadrIdcodeStart) != TRUE) {
		return FFWERR_COM;
	}
	if (GetData1(&pMcuInfo->byIdcodeSize) != TRUE) {
		return FFWERR_COM;
	}

	// RevRxNo120910-001 Append Start
	// 対象デバイスJTAG IDコード設定数
	if (GetData1(&byMcuJtagIdcodeNum) != TRUE) {
		return FFWERR_COM;
	}
	pMcuInfo->dwMcuJtagIdcodeNum = (static_cast<DWORD>(byMcuJtagIdcodeNum)) & 0x000000FF;
	// RevRxNo120910-001 Append End

	// 対象デバイス JTAG IDコード
	// RevRxNo120910-001 Modify Start
	for (dwCnt = 0; dwCnt < byMcuJtagIdcodeNum; dwCnt++) {
		if (GetData4(&pMcuInfo->dwMcuJtagIdcode[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}
	// RevRxNo120910-001 Modify End
	// 対象デバイス デバイスコード
	if (GetData4(&pMcuInfo->dwMcuDeviceCode) != TRUE) {
		return FFWERR_COM;
	}

	// 対象デバイス FINEメーカコード
	if (GetData2(&pMcuInfo->wMcuFineMakerCode) != TRUE) {
		return FFWERR_COM;
	}
	// 対象デバイス FINEファミリコード
	if (GetData2(&pMcuInfo->wMcuFineFamilyCode) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxNo120910-001 Append Start
	// 対象デバイスFINE デバイスコード設定数
	if (GetData1(&byMcuFineDeviceCodeNum) != TRUE) {
		return FFWERR_COM;
	}
	pMcuInfo->dwMcuFineDeviceCodeNum = static_cast<DWORD>(byMcuFineDeviceCodeNum) & 0x000000FF;
	// RevRxNo120910-001 Append End
	// 対象デバイス FINEデバイスコード
	// RevRxNo120910-001 Modify Start
	for (dwCnt = 0; dwCnt < byMcuFineDeviceCodeNum; dwCnt++) {
		if (GetData2(&pMcuInfo->wMcuFineDeviceCode[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
	}
	// RevRxNo120910-001 Modify End
	// 対象デバイス FINEバージョンコード
	if (GetData2(&pMcuInfo->wMcuFineVersionCode) != TRUE) {
		return FFWERR_COM;
	}

	// V.1.02 V.1.01.90.206 Append Start
	// ユーザ指定MCU動作モード
	// V.1.02 V.1.01.00.215 Delete( BFWRXCmd_HPONで渡すことになったため )

	// MCUレジスタ情報
	// MCUレジスタ情報定義ブロック数
	if (GetData4(&pMcuInfo->dwMcuRegInfoBlkNum) != TRUE) {
		return FFWERR_COM;
	}
	for (dwCnt = 0; dwCnt < pMcuInfo->dwMcuRegInfoBlkNum; dwCnt++) {
		if (GetData4(&pMcuInfo->dwMcuRegInfoNum[dwCnt]) != TRUE) {
			return FFWERR_COM;
		}
		for (dwCnt2 = 0; dwCnt2 < pMcuInfo->dwMcuRegInfoNum[dwCnt]; dwCnt2++) {
			if (GetData4(&pMcuInfo->dwmadrMcuRegInfoAddr[dwCnt][dwCnt2]) != TRUE) {
				return FFWERR_COM;
			}
			if (GetData4(&dwData) != TRUE) {
				return FFWERR_COM;
			}
			pMcuInfo->eMcuRegInfoAccessSize[dwCnt][dwCnt2] = static_cast<FFWENM_MACCESS_SIZE>(dwData);
			if (GetData4(&pMcuInfo->dwMcuRegInfoMask[dwCnt][dwCnt2]) != TRUE) {
				return FFWERR_COM;
			}
			if (GetData4(&pMcuInfo->dwMcuRegInfoData[dwCnt][dwCnt2]) != TRUE) {
				return FFWERR_COM;
			}
		}
	}
	// ROMレス情報
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	// ROMレス情報が1(ROMレス)の場合だけ、0で返す。
	if(byData == 1){
		pMcuArea->dwFlashRomPatternNum = 0;
	}

	getEinfData(&einfData);						// エミュレータ情報取得

	// RevRxNo130301-001 Append Start
	if (einfData.wEmuStatus != EML_EZCUBE) {	// EZ-CUBEでない場合
		// 命令セットアーキテクチャ情報
		if (GetData1(&byData) != TRUE) {
			return FFWERR_COM;
		}
		pMcuInfo->eCpuIsa = static_cast<enum FFWRXENM_CPU_ISA>(byData);

		// FPU有無情報
		if (GetData1(&byData) != TRUE) {
			return FFWERR_COM;
		}
		pMcuInfo->eFpuSet = static_cast<enum FFWRXENM_FPU_SET>(byData);
	}
	// RevRxNo130301-001 Append End

	// RevRxNo121022-001 Append Start
	// ターゲット電圧情報
	// EZ-CUBE使用時の場合 ターゲット電圧取得処理を実施。
	if (einfData.wEmuStatus == EML_EZCUBE){
		if (GetData4(&dwData) != TRUE) {	// ターゲット電圧値
			return FFWERR_COM;
		}
		// ターゲット電圧値 DWORD型→floatに置き換え
		// Setでは上位2バイトを整数部、下位2バイトを小数部とする
		//  3.3Vの場合は、0x00033000を格納しているので、これを逆変換
		wDataL = (WORD)dwData;
		wDataH = (WORD)(dwData >> 16);
		pMcuInfo->fTgtPwrVal = (float)wDataH;
		pMcuInfo->fTgtPwrVal += ((float)wDataL)/10000;
	}
	// RevRxNo121022-001 Append End

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWRXCmd_SetDBGコマンドの発行
 * @param pDbg FFWMCU_DBG_DATA_RX構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_SetRXDBG(const FFWMCU_DBG_DATA_RX* pDbg)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_SET_DBG;
	ferr = ProtSendCmd(wCmdCode);					// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)pDbg->eClkChangeEna) != TRUE) {	// クロック切り替え
		return FFWERR_COM;
	}
	// V.1.02 V.1.01.90.206 Append Start
	if (PutData1((BYTE)pDbg->eDbgFlashWrite) != TRUE) {	// Flash書き換えデバッグ指定
		return FFWERR_COM;
	}
	if (PutData1((BYTE)pDbg->eDbgDataFlashWrite) != TRUE) {	// Data(E2)Flash書き換えデバッグ指定
		return FFWERR_COM;
	}
	// V.1.02 V.1.01.90.206 Append End
	// RevRxNo120615-001 Append Start
	if (PutData1((BYTE)pDbg->eWriter) != TRUE) {	// ライターモード情報
		return FFWERR_COM;
	}
	// RevRxNo120615-001 Append End
	ferr = ProtGetStatus(wCmdCode);					// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);					// 処理中断コード受信


	return ferr;
}
//=============================================================================
/**
 * BFWRXCmd_SetDBGコマンドの発行
 * @param pDbg FFWMCU_DBG_DATA_RX構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_GetRXDBG(FFWMCU_DBG_DATA_RX* pDbg)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	BYTE	byBuf;

	wCmdCode = BFWCMD_GET_DBG;
	ferr = ProtSendCmd(wCmdCode);					// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);					// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData1(&byBuf) != TRUE) {	// クロック切り替え
		return FFWERR_COM;
	}
	pDbg->eClkChangeEna = static_cast<FFWRXENM_CLKCHANGE_ENA>(byBuf);

	if (GetData1(&byBuf) != TRUE) {	// Flash書き換えデバッグ指定
		return FFWERR_COM;
	}
	pDbg->eDbgFlashWrite = static_cast<FFWENM_FLASHWRITE>(byBuf);

	if (GetData1(&byBuf) != TRUE) {	// Data(E2)Flash書き換えデバッグ指定
		return FFWERR_COM;
	}
	pDbg->eDbgDataFlashWrite = static_cast<FFWENM_FLASHWRITE>(byBuf);
	// RevRxNo120615-001 Append Start
	if (GetData1(&byBuf) != TRUE) {	// ライターモード情報
		return FFWERR_COM;
	}
	pDbg->eWriter = static_cast<FFWENM_WRITER>(byBuf);
	// RevRxNo120615-001 Append End

	ferr = ProtRcvHaltCode(&wBuf);					// 処理中断コード受信


	return ferr;
}
//=============================================================================
/**
 * BFWRXCmd_GetSTATコマンドの発行
 * @param dwStatKind 取得対象の情報(SFR/JTAG/EML)を指定する
 * @param peStatMcu  ターゲットMCUステータス情報を格納する変数のアドレス
 * @param pStatSFR   SFR情報を格納する
 * @param pStatJTAG  JTAG情報を格納する
 * @param pStatFINE  FINE情報を格納する
 * @param pStatEML   EML情報を格納する
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo120910-001 Modify Line
FFWERR PROT_MCU_GetRXSTAT(DWORD dwStatKind, enum FFWRXENM_STAT_MCU* peStatMcu, FFWRX_STAT_SFR_DATA* pStatSFR, 
						  FFWRX_STAT_JTAG_DATA* pStatJTAG, FFWRX_STAT_FINE_DATA* pStatFINE, FFWRX_STAT_EML_DATA* pStatEML)
{
// RevRxNo111121-002 Append Start
/*
	注)
	  FFW 内で使用するエンディアン情報は、FFW 内の取得関数を使用している。
	  DO_STAT() で実行されるROM レス、及び内蔵ROM 無効拡張モードでの
	  エンディアン情報差し替えは、STAT コマンド用の処理として現状のままとする。
	  →PROT_MCU_GetRXSTAT() では、上記エンディアン情報差し替えは行わない。
*/
// RevRxNo111121-002 Append End


	FFWERR	ferr = FFWERR_OK;
	BYTE	byBuf;
	WORD	wBuf;
	DWORD	dwBuf;
	WORD	wCmdCode;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	FFWE20_EINF_DATA	einfData;	// RevRxNo130308-001-005 Append Line

	pDbgData = GetDbgDataRX();

	getEinfData(&einfData);	// RevRxNo130308-001-005 Append Line

	wCmdCode = BFWCMD_GET_STAT;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo120910-001 Modify Line
	if (PutData4(dwStatKind) != TRUE) {		// 取得対象情報ビット送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData1(&byBuf) != TRUE) {			// MCU状態状況を受信
		return FFWERR_COM;
	}
	*peStatMcu = (FFWRXENM_STAT_MCU)byBuf;

	// RevRxNo120910-001 Modify Line
	if ((dwStatKind & STAT_KIND_EML) == STAT_KIND_EML) {		// EML情報の受信
		if (GetData1(&byBuf) != TRUE) {			// ユーザシステム電源状態を受信
			return FFWERR_COM;
		}
		if (pStatEML != NULL) {
			if (byBuf == 1) {
				pStatEML->eStatUVCC = UVCC_ON;
			} else {
				pStatEML->eStatUVCC = UVCC_OFF;
			}
		}

		// RevRxNo120910-001 Append Start
		if (GetData2(&wBuf) != TRUE) {			// ユーザシステム電圧値を受信
			return FFWERR_COM;
		}
		if (pStatEML != NULL) {
			// ユーザシステム電圧(A/D変換値→電圧値への変換)
			// RevRxE2LNo141104-001 Modify Line
			pStatEML->fNowUVCC = ConvUvccVal(wBuf, einfData.wEmuStatus);
		}
		// RevRxNo120910-001 Append End

		if (GetData1(&byBuf) != TRUE) {			// ユーザシステム接続状態を受信
			return FFWERR_COM;
		}
		if (pStatEML != NULL) {
			if (byBuf == 0) {
				pStatEML->eStatUCON = TRGT_EML_NCON;
			} else {
				pStatEML->eStatUCON = TRGT_EML_CON;
			}
		}

		if (GetData4(&dwBuf) != TRUE) {			// トレースクロック実測値を受信
			return FFWERR_COM;
		}
		if (pStatEML != NULL) {
			pStatEML->flTrcClkData = GetTRCLKCalc(dwBuf);
		}

		if (GetData4(&dwBuf) != TRUE) {			// トレース計測動作を受信
			return FFWERR_COM;
		}
		if (pStatEML != NULL) {
			if(GetTrcInramMode() == TRUE) {
				//内蔵トレースが設定されている
				//RevNo100715-038 Modify Start
				//FFW内部変数が実行中のときは、実行中とする
				if(GetMcuRunState() == TRUE){
					pStatEML->dwTraceInfo = STAT_EML_TRCINFO_TRCRUN;
				} else {
					pStatEML->dwTraceInfo = STAT_EML_TRCINFO_TRCSTOP;
				}
				// RevNo100715-038 Modify End
			} else {
				if( (pStatEML->flTrcClkData == 0) && (*peStatMcu == RX_MCUSTAT_NORMAL)) {
					//トレースの設定がされていないのでトレース実行中ではないとする
					pStatEML->dwTraceInfo = STAT_EML_TRCINFO_TRCSTOP;
				} else { 
					//外部トレース設定
					pStatEML->dwTraceInfo = dwBuf;
				}
			}
		}

		// RevRxNo130308-001-005 Append Start
		if (einfData.wEmuStatus != EML_EZCUBE) {	// EZ-CUBEでない場合
			if (GetData4(&dwBuf) != TRUE) {			// カバレッジ計測動作を受信
				return FFWERR_COM;
			}
			if (pStatEML != NULL) {
				pStatEML->dwCoverageInfo = dwBuf;
			}
		}
		// RevRxNo130308-001-005 Append End
	}

	// RevRxNo120910-001 Modify Line
	if ((dwStatKind & STAT_KIND_JTAG) == STAT_KIND_JTAG) {		// JTAG情報の受信
		if (GetData1(&byBuf) != TRUE) {			// デバッガ接続可否情報を受信
			return FFWERR_COM;
		}
		if (pStatJTAG != NULL) {
			if (byBuf == 1) {
				pStatJTAG->eStatBan = FFWRX_BAN_DISABLE;
			} else {
				pStatJTAG->eStatBan = FFWRX_BAN_ENABLE;
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// デバッガ認証状態を受信
			return FFWERR_COM;
		}
		if (pStatJTAG != NULL) {
			if (byBuf == 1) {
				pStatJTAG->eStatAuth = FFWRX_AUTH_ALREADY;
			} else {
				pStatJTAG->eStatAuth = FFWRX_AUTH_BEFORE;
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// デバッグDMAデータ転送状態を受信
			return FFWERR_COM;
		}
		if (pStatJTAG != NULL) {
			if (byBuf == 1) {
				pStatJTAG->eStatdDmaBusy = FFWRX_DDMA_BUSY;
			} else {
				pStatJTAG->eStatdDmaBusy = FFWRX_DDMA_NORMAL;
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// ターゲットプログラム実行状態を受信
			return FFWERR_COM;
		}
		if (pStatJTAG != NULL) {
			if (byBuf == 1) {
				pStatJTAG->eStatExec = FFWRX_STAT_EXEC_RUN;
				//RevNo100715-038 Modify Start
				//EML情報取得かつ、内蔵トレース取得時は強制的に実行中に差し替え
				// RevRxNo120910-001 Modify Line
				if ((dwStatKind & STAT_KIND_EML) == STAT_KIND_EML){
					if (pStatEML != NULL) {
						if(GetTrcInramMode() == TRUE) {
							pStatEML->dwTraceInfo = STAT_EML_TRCINFO_TRCRUN;
						}
					}
				}
				//RevNo100715-038 Modify End
			} else {
				pStatJTAG->eStatExec = FFWRX_STAT_EXEC_STOP;
				//RevNo100715-038 Modify Start
				//EML情報取得かつ、内蔵トレース取得時は強制的に停止中に差し替え
				// RevRxNo120910-001 Modify Line
				if ((dwStatKind & STAT_KIND_EML) == STAT_KIND_EML){
					if (pStatEML != NULL) {
						if(GetTrcInramMode() == TRUE) {
							pStatEML->dwTraceInfo = STAT_EML_TRCINFO_TRCSTOP;
						}
					}
				}
				//RevNo100715-038 Modify End
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// デバッガ認証の要否を受信
			return FFWERR_COM;
		}
		if (pStatJTAG != NULL) {
			if (byBuf == 1) {
				pStatJTAG->eStatAuthSet = FFWRX_STAT_AUTH_SET;
			} else {
				pStatJTAG->eStatAuthSet = FFWRX_STAT_AUTH_NONSET;
			}
		}

		if (GetData4(&dwBuf) != TRUE) {			// JTAGのIDコードを受信
			return FFWERR_COM;
		}
		if (pStatJTAG != NULL) {
			pStatJTAG->dwJtagIdCode = dwBuf;
		}
		if (GetData2(&wBuf) != TRUE) {			// FINEメーカコードを受信(ダミーデータ)
			return FFWERR_COM;
		}
		if (GetData2(&wBuf) != TRUE) {			// FINEファミリコードを受信(ダミーデータ)
			return FFWERR_COM;
		}
		if (GetData2(&wBuf) != TRUE) {			// FINEデバイスコードを受信(ダミーデータ)
			return FFWERR_COM;
		}

	// RevRxNo120910-001 Modify Line
	} else if ((dwStatKind & STAT_KIND_FINE) == STAT_KIND_FINE) {		// FINE情報の受信
		if (GetData1(&byBuf) != TRUE) {			// デバッガ接続可否情報を受信
			return FFWERR_COM;
		}
		if (pStatFINE != NULL) {
			if (byBuf == 1) {
				pStatFINE->eStatBan = FFWRX_BAN_DISABLE;
			} else {
				pStatFINE->eStatBan = FFWRX_BAN_ENABLE;
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// デバッガ認証状態を受信
			return FFWERR_COM;
		}
		if (pStatFINE != NULL) {
			if (byBuf == 1) {
				pStatFINE->eStatAuth = FFWRX_AUTH_ALREADY;
			} else {
				pStatFINE->eStatAuth = FFWRX_AUTH_BEFORE;
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// デバッグDMAデータ転送状態を受信
			return FFWERR_COM;
		}
		if (pStatFINE != NULL) {
			if (byBuf == 1) {
				pStatFINE->eStatdDmaBusy = FFWRX_DDMA_BUSY;
			} else {
				pStatFINE->eStatdDmaBusy = FFWRX_DDMA_NORMAL;
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// ターゲットプログラム実行状態を受信
			return FFWERR_COM;
		}
		if (pStatFINE != NULL) {
			if (byBuf == 1) {
				pStatFINE->eStatExec = FFWRX_STAT_EXEC_RUN;
				//RevNo100715-038 Modify Start
				//EML情報取得かつ、内蔵トレース取得時は強制的に実行中に差し替え
				// RevRxNo120910-001 Modify Line
				if ((dwStatKind & STAT_KIND_EML) == STAT_KIND_EML){
					if (pStatEML != NULL) {
						if(GetTrcInramMode() == TRUE) {
							pStatEML->dwTraceInfo = STAT_EML_TRCINFO_TRCRUN;
						}
					}
				}
				//RevNo100715-038 Modify End
			} else {
				pStatFINE->eStatExec = FFWRX_STAT_EXEC_STOP;
				//RevNo100715-038 Modify Start
				//EML情報取得かつ、内蔵トレース取得時は強制的に停止中に差し替え
				// RevRxNo120910-001 Modify Line
				if ((dwStatKind & STAT_KIND_EML) == STAT_KIND_EML){
					if (pStatEML != NULL) {
						if(GetTrcInramMode() == TRUE) {
							pStatEML->dwTraceInfo = STAT_EML_TRCINFO_TRCSTOP;
						}
					}
				}
			//RevNo100715-038 Modify End
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// デバッガ認証の要否を受信
			return FFWERR_COM;
		}
		if (pStatFINE != NULL) {
			if (byBuf == 1) {
				pStatJTAG->eStatAuthSet = FFWRX_STAT_AUTH_SET;
			} else {
				pStatJTAG->eStatAuthSet = FFWRX_STAT_AUTH_NONSET;
			}
		}

		if (GetData4(&dwBuf) != TRUE) {			// JTAG IDコードを受信(ダミーデータ)
			return FFWERR_COM;
		}

		if (GetData2(&wBuf) != TRUE) {			// FINEメーカコードを受信
			return FFWERR_COM;
		}
		if (pStatFINE != NULL) {
			pStatFINE->dwFineMakerCode = (DWORD)wBuf;
		}
		if (GetData2(&wBuf) != TRUE) {			// FINEファミリコードを受信
			return FFWERR_COM;
		}
		if (pStatFINE != NULL) {
			pStatFINE->dwFineFamilyCode = (DWORD)wBuf;
		}
		if (GetData2(&wBuf) != TRUE) {			// FINEデバイスコードを受信
			return FFWERR_COM;
		}
		if (pStatFINE != NULL) {
			pStatFINE->dwFineDeviceCode = (DWORD)wBuf;
		}
	}

	// RevRxNo120910-001 Modify Line
	if ((dwStatKind & STAT_KIND_SFR) == STAT_KIND_SFR) {		// SFR情報の受信
		if (GetData1(&byBuf) != TRUE) {			// エンディアン指定状況を受信
			return FFWERR_COM;
		}
		if (pStatSFR != NULL) {
			if (byBuf == 1) {
				pStatSFR->eEndian = FFWRX_ENDIAN_BIG;
			} else {
				pStatSFR->eEndian = FFWRX_ENDIAN_LITTLE;
			}
		}

		if (GetData1(&byBuf) != TRUE) {			// MODE端子状態を受信
			return FFWERR_COM;
		}
		if (pStatSFR != NULL) {
			pStatSFR->byStatModePin = byBuf;
		}
		// V.1.02 RevNo110304-002 Append Start
		if (GetData1(&byBuf) != TRUE) {			// MD端子状態を受信
			return FFWERR_COM;
		}
		if (pStatSFR != NULL) {
			pStatSFR->byStatMdPin = byBuf;
		}
		// V.1.02 RevNo110304-002 Append Start

		if (GetData1(&byBuf) != TRUE) {			// プロセッサモード状態を受信
			return FFWERR_COM;
		}
		if (pStatSFR != NULL) {
			pStatSFR->byStatProcMode = byBuf;
		}

		if (GetData4(&dwBuf) != TRUE) {			// パフォーマンス計測動作情報受信
			return FFWERR_COM;
		}
		if (pStatSFR != NULL) {
			pStatSFR->dwPerfInfo = dwBuf;
		}

		if (GetData4(&dwBuf) != TRUE) {			// OR/累積AND/状態遷移成立状態情報受信
			return FFWERR_COM;
		}
		if (pStatSFR != NULL) {
			pStatSFR->dwAndStateInfo = dwBuf;
		}
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_RXHPONコマンドの発行
 * @param dwPlug		// 接続種別
 * @param ePmode		// 起動モード
 * @retval FFWエラーコード
 */
//=============================================================================
// V.1.02 No.4 ユーザブートモード起動対応 Modify Lilne
FFWERR PROT_RXHPON(DWORD dwPlug, enum FFWRXENM_PMODE ePmode)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	// V.1.02 No.47 ROM無効モード時の処理対応 Append Line
	enum FFWRXENM_RMODE	eRmode;

	FFWMCU_DBG_DATA_RX*	pDbgData;
	pDbgData = GetDbgDataRX();
	WORD	wBuf;

	wCmdCode = BFWCMD_HPON;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)dwPlug) != TRUE) {		// 接続種別送信
		return FFWERR_COM;
	}

	// V.1.02 No.4 ユーザブートモード起動対応 Modify Lilne
	if (PutData1((BYTE)ePmode) != TRUE) {				// 起動モード送信
		return FFWERR_COM;
	}
	// V.1.02 No.47 ROM無効モード時の処理対応 Append Start
	// レジスタ設定動作モード
	eRmode = GetRmodeDataRX();			// レジスタ設定の起動モードを取得
	if (PutData1((BYTE)eRmode) != TRUE) {
		return FFWERR_COM;
	}
	// V.1.02 No.47 ROM無効モード時の処理対応 Append End

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_SetHotPlugコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20SetHotPlug(void)
{
	FFWERR	ferr;
	WORD	wCmdCode;

	FFWMCU_DBG_DATA_RX*	pDbgData;
	pDbgData = GetDbgDataRX();
	WORD	wBuf;

	wCmdCode = BFWCMD_SET_HOTPLUG;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_RXIDCODEコマンドの発行
 * @param pbyID IDコード格納ポインタ
 * @param pbyResult 認証結果格納ポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_RXIDCODE(BYTE* pbyID, BYTE* pbyResult)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	int		i;

	WORD	wBuf;

	wCmdCode = BFWCMD_IDCODE;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	for(i = 0; i < 16; i++) {
		if (PutData1(*(pbyID+i)) != TRUE) {		// IDコード送信
			return FFWERR_COM;
		}
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr == FFWERR_OK) {
		if (GetData1(pbyResult) != TRUE) {		// 認証結果受信
			return FFWERR_COM;
		}
		ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信
	}
 

	return ferr;
}

//=============================================================================
/**
 * MCU依存コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Mcu(void)
{
	return;
}
