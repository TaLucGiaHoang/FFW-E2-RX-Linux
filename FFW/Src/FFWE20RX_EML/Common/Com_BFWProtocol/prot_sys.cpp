///////////////////////////////////////////////////////////////////////////////
/**
 * @file prot_sys.cpp
 * @brief BFWコマンド プロトコル生成関数(システムコマンド)
 * @author RSD Y.Minami, H.Hashiguchi, H.Akashi, S.Ueda, M.Yamamoto, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/02/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo121017-003	2012/10/30 明石
　VS2008 warning C4996対策
・RevRxE2LNo141104-001 2015/02/03 上田,山本
	E2 Lite対応
	- E1/E20 レベル0 旧バージョン(MP版以外)用関数削除(上田SC)。
    不具合修正
	- ProtGetStatus()でエラー発生時、処理中断コードを2重取得する処理を修正。
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
*/
#include <stdio.h>
#include <stdlib.h>

#include "prot_sys.h"
#include "prot_common.h"
#include "comctrl.h"
#include "prot.h"
#include "prot_cpu.h"
#include "errchk.h"
#include "ffw_closed.h"
#include "emudef.h"	// RevRxE2LNo141104-001 Append Line

//=============================================================================
/**
 * BFWCmd_SetTMOUTコマンドの発行
 * @param wBfwTimeOut タイムアウト時間
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_SetTMOUT(WORD wBfwTimeOut)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_SET_TMOUT;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (PutData2(wBfwTimeOut) != TRUE) {	// タイムアウト時間送信
		return FFWERR_COM;
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
 * BFWCmd_GetTMOUTコマンドの発行
 * @param pwBfwTimeOut タイムアウト時間格納アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetTMOUT(WORD* pwBfwTimeOut)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_GET_TMOUT;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);		// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData2(&wBuf) != TRUE) {		// タイムアウト時間受信
		return FFWERR_COM;
	}
	*pwBfwTimeOut = wBuf;


	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

// 2008.11.13 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * BFWCmd_GetEINFコマンドの発行(RX用)
 * @param pEinf エミュレータハードウェア情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetE20EINF(FFWE20_EINF_DATA* pEinf)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf, wData;
	DWORD	dwData;
	BYTE	byData;
	char	cBuf[EINF_BFW_NAME_NUM+1];	// RevRxE2LNo141104-001 Append Line
	int		i, ierr;	// RevRxE2LNo141104-001 Modify Line

	wCmdCode = BFWCMD_GET_EINF;
	
	ferr = ProtSendCmd(wCmdCode);		// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);		// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wData = 0;
	dwData = 0;

	//---------------------
	// (1) エミュレータ情報
	//---------------------
	// ①エミュレータ種別
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wEmuStatus = byData;

	// ②エミュレータ基板REV/製品REV
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wBoardRev = wData;

	// ③エミュレータへの電源(ACアダプタ)供給（E20 REV.Aのみ）
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	if (byData == 1) {
		pEinf->eEmlPwrSupply = EML_PWR_ON;			//電源供給状態
	} else {
		pEinf->eEmlPwrSupply = EML_PWR_OFF;			//電源未供給状態
	}

	// ④USBバスパワー電圧（E1/E20 REV.B以降）
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wVbusVal = wData;
	//---------------------
	// (2) ユーザシステム情報
	//---------------------
	// ①ユーザI/F接続形態
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxE2No170201-001 Modify Start
	if (pEinf->wEmuStatus == EML_E2) {
		pEinf->wTagetCable = CONNECT_IF_RESERVE;
	} else {
		pEinf->wTagetCable = wData;
	}
	// RevRxE2No170201-001 Modify End
	// ②ユーザシステム接続状態
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	if (byData == 0) {
		//未接続状態
		pEinf->eStatUCON = TRGT_EML_NCON;
	} else {
		//接続状態
		pEinf->eStatUCON = TRGT_EML_CON;
	}

	// ③ユーザシステム電圧
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wUvccVal = wData;
	//---------------------
	// (3) BFW情報
	//---------------------
	// ①BFW動作モード
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->byBfwMode = byData;

	// ①LV0バージョン
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->dwBfwLV0Ver = dwData;

	// ②EMLバージョン
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->dwBfwEMLVer = dwData;

	// ③EMLターゲットMCU情報
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wEmlTargetCpu = wData;

	// ④EMLターゲットMCUの種別(サブ)
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wEmlTargetCpuSub = wData;

	// ⑤EMLターゲットMCU ID
	// RevRxE2LNo141104-001 Modify Start
	if ((pEinf->wEmuStatus == EML_E1) || (pEinf->wEmuStatus == EML_E20) || (pEinf->wEmuStatus == EML_EZCUBE)) {
		// E1/E20/EZ-CUBEの場合
		for (i = 0; i < EINF_BFW_NAME_NUM; i++) {
			if (GetData1(&byData) != TRUE) {
				return FFWERR_COM;
			}
			pEinf->cEmlTgtMucId[i] = byData;
		}
	} else {
		// E2/E2 Liteの場合
		for (i = 0; i < EINF_BFW_NAME_NUM; i++) {
			if (GetData1(&byData) != TRUE) {
				return FFWERR_COM;
			}
			pEinf->cEmlTgtMucId[i] = EINF_MCUID_CODE_E2;	// 固定値空白文字を格納
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// ⑥EML対応レベル0バージョン
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->dwEmlTargetLv0Ver = dwData;

	// ⑦FDT/PRGバージョン
	GetData4(&dwData);
	pEinf->dwBfwFdtVer = dwData;

	// ⑧FDT/PRGターゲットMCU ID
	// RevRxE2LNo141104-001 Modify Start
	if ((pEinf->wEmuStatus == EML_E1) || (pEinf->wEmuStatus == EML_E20) || (pEinf->wEmuStatus == EML_EZCUBE)) {
		// E1/E20/EZ-CUBEの場合
		for (i = 0; i < EINF_BFW_NAME_NUM; i++) {
			if (GetData1(&byData) != TRUE) {
				return FFWERR_COM;
			}
			pEinf->cFdtTgtMucId[i] = byData;
		}
	} else {
		// E2/E2 Liteの場合
		// 2バイトの16進データを4バイトの文字に変換して格納
		if (GetData2(&wData) != TRUE) {
			return FFWERR_COM;
		}
		ierr = sprintf_s(cBuf, _countof(cBuf), "%04X", wData);
		memcpy(pEinf->cFdtTgtMucId, cBuf, 4);

		// リザーブデータ 2バイト受信
		if (GetData2(&wData) != TRUE) {
			return FFWERR_COM;
		}

		// リザーブデータ (16-4)バイト受信と、(16-4)バイトの空白文字格納
		for (i = 4; i < EINF_BFW_NAME_NUM; i++) {
			if (GetData1(&byData) != TRUE) {
				return FFWERR_COM;
			}
			pEinf->cFdtTgtMucId[i] = EINF_MCUID_CODE_E2;	// 空白文字
		}
	}
	// RevRxE2LNo141104-001 Modify End


	// ⑨FDT/PRG対応レベル0バージョン
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->dwFdtTargetLv0Ver = dwData;

	//---------------------
	// (4) FPGA情報
	//---------------------
	// ①通信制御FPGAコンフィグレーション状態
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->byFpga1Status = byData;

	// ②通信制御FPGA(バージョン＋デバッグインタフェース種別)
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxE2No170201-001 Modify Start
	if (pEinf->wEmuStatus == EML_E2) {
		pEinf->byFpga1DbgIf = SCI_DEBUG_RESERVE;
		pEinf->byFpga1Rev = 0x00;
	} else {
		pEinf->byFpga1DbgIf = (BYTE)((wData & 0xFF00) >> 8);
		pEinf->byFpga1Rev = (BYTE)(wData & 0x00FF);
	}
	// RevRxE2No170201-001 Modify End
	// ③トレース制御FPGA(バージョン＋対象MCU種別)
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxE2No170201-001 Modify Start
	if (pEinf->wEmuStatus == EML_E2) {
		pEinf->byFpga2DbgIf = 0x00;
		pEinf->byFpga2Rev = 0x00;
	} else {
		pEinf->byFpga2DbgIf = (BYTE)((wData & 0xFF00) >> 8);
		pEinf->byFpga2Rev = (BYTE)(wData & 0x00FF);
	}
	// RevRxE2No170201-001 Modify End

	// ④トレーズ有無＋FPGA全体バージョン
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->byFpgaVer = (BYTE)(wData & 0x00FF);		// FPGA全体バージョン
	if ((wData & 0x1000) == 0x1000) {	// bit12が"1"ならばトレース情報有り
		pEinf->byTraceFlg = TRUE;
	} else {	// bit12が"0"ならばトレース情報なし
		pEinf->byTraceFlg = FALSE;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

// V.1.02 No.14,15 USB高速化対応 Append Start
//=============================================================================
/**
 * BFWCmd_GetEINF_EMLコマンドの発行(RX用)
 * @param pEinf エミュレータハードウェア情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetE20EINF_EML(FFWE20_EINF_DATA* pEinf)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf, wData;
	DWORD	dwData;
	BYTE	byData;
	int		i;

	wCmdCode = BFWCMD_GET_EINF_EML;
	
	ferr = ProtSendCmd(wCmdCode);		// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);		// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wData = 0;
	dwData = 0;

	//---------------------
	// (1) エミュレータ情報
	//---------------------
	// ①エミュレータ種別
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wEmuStatus = byData;

	// ②エミュレータ基板REV
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wBoardRev = wData;

	// ③エミュレータへの電源(ACアダプタ)供給（E20 REV.Aのみ）
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	if (byData == 1) {
		pEinf->eEmlPwrSupply = EML_PWR_ON;			//電源供給状態
	} else {
		pEinf->eEmlPwrSupply = EML_PWR_OFF;			//電源未供給状態
	}

	// ④USBバスパワー電圧（E1/E20 REV.B以降）
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wVbusVal = wData;
	//---------------------
	// (2) ユーザシステム情報
	//---------------------
	// ①ユーザI/F接続形態
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wTagetCable = wData;
	// ②ユーザシステム接続状態
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	if (byData == 0) {
		//未接続状態
		pEinf->eStatUCON = TRGT_EML_NCON;
	} else {
		//接続状態
		pEinf->eStatUCON = TRGT_EML_CON;
	}

	// ③ユーザシステム電圧
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wUvccVal = wData;
	//---------------------
	// (3) BFW情報
	//---------------------
	// ①BFW動作モード
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->byBfwMode = byData;

	// ①LV0バージョン
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->dwBfwLV0Ver = dwData;

	// ②EMLバージョン
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->dwBfwEMLVer = dwData;

	// ③EMLターゲットMCU情報
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wEmlTargetCpu = wData;

	// ④EMLターゲットMCUの種別(サブ)
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->wEmlTargetCpuSub = wData;

	// ⑤EMLターゲットMCU ID
	for (i = 0; i < EINF_BFW_NAME_NUM; i++) {
		if (GetData1(&byData) != TRUE) {
			return FFWERR_COM;
		}
		pEinf->cEmlTgtMucId[i] = byData;
		
	}

	// ⑥EML対応レベル0バージョン
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->dwEmlTargetLv0Ver = dwData;

	// ⑦FDTバージョン
	GetData4(&dwData);
	pEinf->dwBfwFdtVer = dwData;

	// ⑧FDTターゲットMCU ID
	for (i = 0; i < EINF_BFW_NAME_NUM; i++) {
		if (GetData1(&byData) != TRUE) {
			return FFWERR_COM;
		}
		pEinf->cFdtTgtMucId[i] = byData;
		
	}

	// ⑨FDT対応レベル0バージョン
	if (GetData4(&dwData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->dwFdtTargetLv0Ver = dwData;

	//---------------------
	// (4) FPGA情報
	//---------------------
	// ①通信制御FPGAコンフィグレーション状態
	if (GetData1(&byData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->byFpga1Status = byData;

	// ②通信制御FPGA(バージョン＋デバッグインタフェース種別)
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->byFpga1DbgIf = (BYTE)((wData & 0xFF00) >> 8);
	pEinf->byFpga1Rev = (BYTE)(wData & 0x00FF);
	// ③トレース制御FPGA(バージョン＋対象MCU種別)
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->byFpga2DbgIf = (BYTE)((wData & 0xFF00) >> 8);
	pEinf->byFpga2Rev = (BYTE)(wData & 0x00FF);

	// ④トレーズ有無＋FPGA全体バージョン
	if (GetData2(&wData) != TRUE) {
		return FFWERR_COM;
	}
	pEinf->byFpgaVer = (BYTE)(wData & 0x00FF);		// FPGA全体バージョン
	if ((wData & 0x1000) == 0x1000) {	// bit12が"1"ならばトレース情報有り
		pEinf->byTraceFlg = TRUE;
	} else {	// bit12が"0"ならばトレース情報なし
		pEinf->byTraceFlg = FALSE;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}
// V.1.02 No.14,15 USB高速化対応 Append End


//=============================================================================
/**
 * BFWCmd_MONPXXXコマンドの発行開始(実行1, 実行2を実行)
 * @param eBfwLoadCmd BFW書き換えコマンド種別
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxE2LNo141104-001 Modify Line
FFWERR PROT_MONP_OPEN(enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;
	FFW_EMUDEF_DATA* pEmuDef;	// RevRxE2LNo141104-001 Append Line

	pEmuDef = GetEmuDefData();	// RevRxE2LNo141104-001 Append Line

	/* BFW MONPコマンド 形式1 */
	/* RAMに転送したE/Wプログラムへの移行まで */
	// RevRxE2LNo141104-001 Modify Start
	if (eBfwLoadCmd == BFWLOAD_CMD_MONPALL) {
		wCmdCode = BFWCMD_MONPALL;
	} else if (eBfwLoadCmd == BFWLOAD_CMD_MONP) {
		wCmdCode = BFWCMD_MONP;
	} else if (eBfwLoadCmd == BFWLOAD_CMD_MONPFDT) {
		wCmdCode = BFWCMD_MONPFDT;
	} else {
		wCmdCode = BFWCMD_MONPCOM;
	// RevRxE2LNo141104-001 Modify End
	}

	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData4(pEmuDef->dwBfwRewriteEmuCode) != TRUE) {		// EMU識別コード送信	// RevRxE2LNo141104-001  Line
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	/* BFW MONPコマンド 形式2 */
	/* RAMに転送したイレーズプログラムの実行 */
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (FlashProtDataForced() != TRUE) {
		return FALSE;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
	//	GetData2(&wBuf);			// 処理中断コード受信
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_MONPXXXコマンドの発行(実行3を実行)
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @param eBfwLoadCmd BFW書き換えコマンド種別
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MONP_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff, enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;
	WORD	wData;

	// RevRxE2LNo141104-001 Modify Start
	if (eBfwLoadCmd == BFWLOAD_CMD_MONPALL) {
		wCmdCode = BFWCMD_MONPALL;
	} else if (eBfwLoadCmd == BFWLOAD_CMD_MONP) {
		wCmdCode = BFWCMD_MONP;
	} else if (eBfwLoadCmd == BFWLOAD_CMD_MONPFDT) {
		wCmdCode = BFWCMD_MONPFDT;
	} else {
		wCmdCode = BFWCMD_MONPCOM;
	}
	// RevRxE2LNo141104-001 Modify End

	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData2(BFWCMD_MONP_SEND) != TRUE) {	// SEND, CLOSE識別パラメータ送信
		return FFWERR_COM;
	}

	if (FlashProtDataForced() != TRUE) {
		return FALSE;
	}
	
	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
//		GetData2(&wData);	//処理中断コード受信
		return ferr;
	}
	GetData2(&wData);	//処理中断コード受信

	if (PutData4(dweadrAddr) != TRUE) {	// 開始アドレス送信
		return FFWERR_COM;
	}
	if (PutData4(dwLength) != TRUE) {	// レングス送信
		return FFWERR_COM;
	}

	if (PutDataN(dwLength, pbyBfwBuff) != TRUE) {	// ライトデータ送信
		return FFWERR_COM;
	}

	if (FlashProtDataForced() != TRUE) {
		return FALSE;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
	//	GetData2(&wData);			// 処理中断コード受信
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_MONPXXXコマンド終了(実行4を実行)
 * @param eBfwLoadCmd BFW書き換えコマンド種別
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MONP_CLOSE(enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	// RevRxE2LNo141104-001 Modify Start
	if (eBfwLoadCmd == BFWLOAD_CMD_MONPALL) {
		wCmdCode = BFWCMD_MONPALL;
	} else if (eBfwLoadCmd == BFWLOAD_CMD_MONP) {
		wCmdCode = BFWCMD_MONP;
	} else if (eBfwLoadCmd == BFWLOAD_CMD_MONPFDT) {
		wCmdCode = BFWCMD_MONPFDT;
	} else {
		wCmdCode = BFWCMD_MONPCOM;
	}
	// RevRxE2LNo141104-001 Modify End

	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData2(BFWCMD_MONP_CLOSE) != TRUE) {	// SEND, CLOSE識別パラメータ送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

// 2008.11.13 INSERT_BEGIN_E20RX600(+23) {
//=============================================================================
/**
 * BFWCmd_DCNFコマンドの発行開始(RX用)
 *   BFWCmd_DCNFの実行1を実行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_DCNF_OPEN(DWORD dwType)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_DCNF;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData2(BFWCMD_DCNF_OPEN) != TRUE) {	// DCNF終了コード(実行1)送信
		return FFWERR_COM;
	}

	if (PutData2((WORD)dwType) != TRUE) {		// FPGAファイルの種類を送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}
// 2008.11.13 INSERT_END_E20RX600 }

//=============================================================================
/**
 * BFWCmd_DCNFコマンドの発行
 *   BFWCmd_DCNFの実行2を実行
 * @param dwLength DCSF論理データのバイト数
 * @param pbyDcsfBuff DCSF論理データの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_DCNF_SEND(DWORD dwLength, const BYTE* pbyDcsfBuff)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_DCNF;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (PutData4(dwLength) != TRUE) {	// レングス送信
		return FFWERR_COM;
	}

	for(unsigned long i = 0; i < dwLength; i++) {
		if (PutData1(*pbyDcsfBuff++) != TRUE) {	// DCSF論理データ送信
			return FFWERR_COM;
		}
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
 * BFWCmd_SetUSBTコマンドの発行
 * @param dwTransSize 転送サイズ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_SetUSBT(DWORD dwTransSize)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_SET_USBT;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (PutData4(dwTransSize) != TRUE) {	// 最大転送サイズ送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus_SetUSBT(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_GetUSBTコマンドの発行
 * @param pdwTransSize 転送サイズ格納アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetUSBT(DWORD* pdwTransSize)
{
	FFWERR	ferr;
	WORD	wBuf;
	DWORD	dwBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_GET_USBT;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	
	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData4(&dwBuf) != TRUE) {	// 転送サイズ受信
		return FFWERR_COM;
	}
	*pdwTransSize = dwBuf;

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_GetUSBSコマンドの発行
 * @param peUsbSpeedType USB転送種別格納アドレス
 * @param bInitCmd FFWCmd_INITからのコマンド発行(BFWへの最初のコマンド発行)を示すフラグ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_GetUSBS(enum FFWENM_USB_SPEED* peUsbSpeedType, BOOL bInit)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	wCmdCode = BFWCMD_GET_USBS;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (bInit == TRUE) {	// FFWCmd_INITからのコマンド発行(BFWへの最初のコマンド発行)の場合
		ferr = ProtGetStatus_1st(wCmdCode);	// ステータス受信
		if (ferr != FFWERR_OK) {
			if (CheckComError(ferr) == TRUE) {
				ferr = FFWERR_COM_1ST;
			}
			return ferr;
		}
		if (GetData2(&wBuf) != TRUE) {	// USB転送種別受信
			return FFWERR_COM_1ST;
		}

	} else {	// BFWへの最初のコマンド発行でない場合
		ferr = ProtGetStatus(wCmdCode);	// ステータス受信
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (GetData2(&wBuf) != TRUE) {	// USB転送種別受信
			return ferr;
		}
	}

	*peUsbSpeedType = static_cast<enum FFWENM_USB_SPEED>(wBuf);

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWMCUCmd_TRANSコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20TRANS(BYTE byBfwTransMode)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_TRANS;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (PutData1(byBfwTransMode) != TRUE) {		// 内部動作モード送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);				// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);				// 処理中断コード受信

	return ferr;
}

// V.1.02 No.14,15 USB高速化対応 Append Start
//=============================================================================
/**
 * BFWMCUCmd_TRANSコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20TRANS_EML(BYTE byBfwTransMode)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_TRANS_EML;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (PutData1(byBfwTransMode) != TRUE) {		// 内部動作モード送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);				// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);				// 処理中断コード受信

	return ferr;
}

// 2009.6.22 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * BFWMCUCmd_TRANS_LV1コマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20TRANS_LV1(void)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_TRANS_LV1;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
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
 * BFWMCUCmd_TRANS_LV0コマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20TRANS_LV0(void)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_TRANS_LV0;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
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
 * BFWMCUCmd_SUMCHECKコマンドの発行
 * @param pSum レベル0のサム値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20SUMCHECK(FFWE20_SUM_DATA* pSum)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	WORD	wCmdCode;
	WORD	wBuf;
	BYTE	byData;

	wCmdCode = BFWCMD_SUMCHECK;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (GetData2(&wBuf) != TRUE) {			// コマンドコード受信
		return FFWERR_COM;
	}
	if (wBuf != wCmdCode) {
		return FFWERR_COMDATA_CMDCODE;		// コマンドコードが異常である
	}
	// Level0 + 1、データフラッシュ領域のサム値取得
	if (GetData2(&wBuf) != TRUE) {
		return FFWERR_COM;
	}
	// サム値取得
	if( GetData1(&byData) != TRUE ){			// LV0サム値受信
		return FFWERR_COM;
	}
	pSum->bySumLevel0 = byData;

	if( GetData1(&byData) != TRUE ){			// EML(LV0+EML)サム値受信
		return FFWERR_COM;
	}
	pSum->bySumEml = byData;

	if( GetData1(&byData) != TRUE ){			// FDT(LV0+FDT)のサム値受信
		return FFWERR_COM;
	}
	pSum->bySumFdt = byData;

	if( GetData1(&byData) != TRUE ){			// SIDのサム値受信
		return FFWERR_COM;
	}
	pSum->bySumBlockA = byData;

	ferrEnd = ProtRcvHaltCode(&wBuf);		// 処理中断コード受信
	if (ferrEnd != FFWERR_OK) {
		return ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_SetSELIDコマンドの開始コマンド発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxE2LNo141104-001 Modify Line
FFWERR PROT_E20SetSELID_OPEN(void)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	FFW_EMUDEF_DATA* pEmuDef;	// RevRxE2LNo141104-001 Append Line

	pEmuDef = GetEmuDefData();	// RevRxE2LNo141104-001 Append Line

	/* BFW SELIDコマンド 形式1 */
	/* RAMに転送したE/Wプログラムへの移行まで */

	wCmdCode = BFWCMD_SET_SELID;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData4(pEmuDef->dwBfwRewriteEmuCode) != TRUE) {		// EMU識別コード送信	// RevRxE2LNo141104-001  Line
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	/* BFW SELIDコマンド 形式2 */
	/* RAMに転送したイレーズプログラムの実行 */
	
	wCmdCode = BFWCMD_SET_SELID;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (FlashProtDataForced() != TRUE) {
		return FALSE;
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
//		GetData2(&wBuf);			// 処理中断コード受信
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWCmd_SetSELIDコマンドのシリアル番号書込みコマンド発行
 * @param dwAddr ライトアドレス
 * @param dwLength シリアル番号のレングス
 * @param pbyNumber シリアル番号
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20SetSELID_SEND(DWORD dwAddr,DWORD dwLength,const BYTE* pbyNumber)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	WORD	wData;

	wCmdCode = BFWCMD_SET_SELID;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 実行処理コード設定（SetSELID_SENDコマンドである事をBFWに知らせるため）
	wBuf = BFWCMD_SETSELID_SEND;
	if (PutData2(wBuf) != TRUE) {			// 実行処理コード送信
		return FFWERR_COM;
	}

	if (FlashProtDataForced() != TRUE) {
		return FALSE;
	}
	
	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
//		GetData2(&wData);					// 処理中断コード受信
		return ferr;
	}
	GetData2(&wData);						// 処理中断コード受信

	if (PutData4(dwAddr) != TRUE) {			// ライトアドレス送信
		return FFWERR_COM;
	}

	if (PutData4(dwLength) != TRUE) {		// レングス送信
		return FFWERR_COM;
	}

	if (PutDataN(dwLength,pbyNumber) != TRUE) {		// シリアル番号送信
		return FFWERR_COM;
	}

	if (FlashProtDataForced() != TRUE) {
		return FALSE;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
//		GetData2(&wData);					// 処理中断コード受信
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);			// 処理中断コード受信

	return ferr;
}


//=============================================================================
/**
 * BFWCmd_SetSELIDコマンドの終了コマンド発行
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20SetSELID_CLOSE(void)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_SET_SELID;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 実行処理コード設定（SetSELID_SENDコマンドである事をBFWに知らせるため）
	wBuf = BFWCMD_SETSELID_CLOSE;
	if (PutData2(wBuf) != TRUE) {			// 実行処理コード送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);			// 処理中断コード受信

	return ferr;
}

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWCmd_SetSELIDコマンドのシリアル番号書込みコマンド発行(E2/E2 Lite用)
 * @param dwAddr ライトアドレス
 * @param dwLength シリアル番号のレングス
 * @param pbyNumber シリアル番号
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E2_SetSELID(DWORD dwLength, const BYTE* pbyNumber)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwAddr;

	pEmuDef = GetEmuDefData();

	wCmdCode = BFWCMD_SET_SELID;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData4(pEmuDef->dwBfwRewriteEmuCode) != TRUE) {		// EMU識別コード送信
		return FFWERR_COM;
	}

	dwAddr = BFWCMD_SETSELID_STARTADDR_E2;
	if (PutData4(dwAddr) != TRUE) {			// ライトアドレス送信
		return FFWERR_COM;
	}

	if (PutData4(dwLength) != TRUE) {		// レングス送信
		return FFWERR_COM;
	}

	if (PutDataN(dwLength, pbyNumber) != TRUE) {	// シリアル番号送信
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);			// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);			// 処理中断コード受信

	return ferr;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * BFWCmd_GetSELIDコマンドの発行
 * @param byLen シリアル番号を取得するレングス
 * @param pbyNumber シリアル番号データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20GetSELID(DWORD *pdwNum,char* pbyNumber)
{
	FFWERR	ferr;
	BYTE	byLen;
	WORD	wCmdCode;
	WORD	wBuf;
	
	wCmdCode = BFWCMD_GET_SELID;
	ferr = ProtSendCmd(wCmdCode);		// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtGetStatus(wCmdCode);		// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// シリアル番号のレングス取得
	if( GetData1( &byLen ) != TRUE ){
		return FFWERR_COM;
	}

	*pdwNum = byLen;

	// シリアル番号を取得
	if( GetDataN( byLen,(BYTE *)pbyNumber ) != TRUE ){
		return FFWERR_COM;
	}

	ferr = ProtRcvHaltCode(&wBuf);		// 処理中断コード受信

	return ferr;
}
// 2009.6.22 INSERT_END_E20RX600 }

//=============================================================================
/**
 * ターゲットシステムへの電源供給(固定値指定)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_EXPWR(enum FFWENM_EXPWR_ENABLE eExPwrEnable, enum FFWENM_EXPWR_VAL eExPwrVal)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_EXPWR;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)eExPwrEnable) != TRUE) {		// 電源供給有無
		return FFWERR_COM;
	}

	if (PutData1((BYTE)eExPwrVal) != TRUE) {		// 電源供給値
		return FFWERR_COM;
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
 * ターゲットシステムへの電源供給
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_EXPWR_EML(enum FFWENM_EXPWR_ENABLE eExPwrEnable, enum FFWENM_EXPWR_VAL eExPwrVal)
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;

	wCmdCode = BFWCMD_EXPWR_EML;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1((BYTE)eExPwrEnable) != TRUE) {		// 電源供給有無
		return FFWERR_COM;
	}

	if (PutData1((BYTE)eExPwrVal) != TRUE) {		// 電源供給値
		return FFWERR_COM;
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
 * E20 USB FIFOデータ取得コマンド（出荷検査用コマンド）
 * @param pFifo 取得するFIFOバッファのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_E20GetD0FIFO( BYTE* pFifo )
{
	FFWERR	ferr;
	WORD	wCmdCode;
	WORD	wBuf;
	BYTE*	pbyBuff;

	wCmdCode = BFWCMD_GET_D0FIFO;
	ferr = ProtSendCmd( wCmdCode );		// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// ステータス受信
	ferr = ProtGetStatus( wCmdCode );
	if( ferr != FFWERR_OK ){
		return ferr;
	}

	// FIFOデータ（2KB）信
	pbyBuff = const_cast<BYTE*>( pFifo );
	if( GetDataN( 2048,pbyBuff ) != TRUE ){
		return FFWERR_COM;
	}

	// 処理中断コード受信
	ferr = ProtRcvHaltCode(&wBuf);
	return ferr;
}

//=============================================================================
/**
 * システムコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtData_Sys(void)
{
	return;
}
