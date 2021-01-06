///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_brk.cpp
 * @brief ターゲットMCU ブレーク関係制御制御関数
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, H.Akashi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/11/15
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo121017-001	2012/10/18 明石
  VS2008対応エラーコード ERROR C4430対策
・RevRxNo130730-009 2013/11/15 大喜多
　　mallocでメモリが確保できなかった場合にエラーを返す処理を追加
・RevRxNo130730-008 2013/11/15 大喜多
	エラーで返る前にメモリ解放処理を追加
*/
#include "mcu_brk.h"
#include "ffwmcu_brk.h"
#include "ffwmcu_mcu.h"
#include "domcu_mcu.h"
#include "domcu_mem.h"
#include <malloc.h>

// static変数
static BOOL		s_bSetAllPBpointFlg;		// PBポイント設定フラグ

// RevNo121017-001	Modify Line
static BOOL	s_bCmdRetFillFlg;					// 命令コード復帰/埋め込み時フラグ


// static関数の宣言


///////////////////////////////////////////////////////////////////////////////
// ブレーク関連関数
///////////////////////////////////////////////////////////////////////////////
//==============================================================================
/**
 * ユーザプログラム実行中のPBポイントの設定
 * @param  madrPbAddr PB設定アドレス
 * @param  pbyUsrCode 退避させるユーザ命令コードを格納する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR WriteBrkCode(MADDR madrPbAddr, BYTE* pbyUsrCode)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	pDbgData = GetDbgDataRX();

	// 命令コード退避
	ferr = EscCmdData(madrPbAddr, pbyUsrCode);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 命令コード埋め込み
	ferr = FillCmdData(madrPbAddr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	s_bSetAllPBpointFlg = TRUE;

	return ferr;
}

//==============================================================================
/**
 * ユーザプログラム実行中のPBポイントの解除
 * @param madrPbAddr ユーザ命令コードを復帰するアドレス
 * @param byUsrCode 復帰するユーザ命令コード 本関数内でRetCmdData()を使用して復帰コードを検索するため削除
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR WriteUserCode(MADDR madrPbAddr)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_PB_DATA* pPb;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	pDbgData = GetDbgDataRX();

	// 命令コード復帰
	ferr = RetCmdData(madrPbAddr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	pPb = GetPbData();
	if (pPb->dwSetNum == 1) {
		s_bSetAllPBpointFlg = FALSE;
	}

	return ferr;
}

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
 * 命令コード復帰/埋め込み時フラグの状態を取得する。
 * @param なし
 * @retval TRUE フラッシュメモリ書き込み開始通知済み (BFWMCUCmd_FWRITESTARTを送信済み)
 * @retval FALSE フラッシュメモリ書き込み開始を未通知 (BFWMCUCmd_FWRITESTARTを未送信)
 */
//=============================================================================
BOOL GetCmdRetFillFlg(void)
{
	return s_bCmdRetFillFlg;
}

//=============================================================================
/**
 * 命令コード復帰/埋め込み時フラグを設定する。
 * @param  bFlg (TRUE：命令コード復帰/埋め込み時、FALSE：それ以外)
 * @retval なし
 */
//=============================================================================
void SetCmdRetFillFlg(BOOL bFlg)
{
	s_bCmdRetFillFlg = bFlg;
}

//==============================================================================
/**
 * 命令コードを退避する。
 * @param  madrStartAddr 開始アドレス
 * @param  pbyUsrCode    命令コード格納領域
 * @retval FFWERR_OK     正常終了
 * @retval FFWERR_COM    通信異常発生
 */
//==============================================================================
FFWERR EscCmdData(MADDR madrStartAddr, BYTE* pbyUsrCode)
{
	FFWERR			ferr = FFWERR_OK;
	BYTE			byReadData[12];		// 4Byte*3ブロック分の領域確保

	// 命令コード取得
	ferr = DO_CDUMP(madrStartAddr, madrStartAddr, &byReadData[0]);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	memcpy(pbyUsrCode, &byReadData[0], sizeof(BYTE));

	return ferr;
}

//==============================================================================
/**
 * 命令コードを復帰する。
 * @param  madrStartAddr 開始アドレス
 * @retval FFWERR_OK     正常終了
 * @retval FFWERR_COM    通信異常発生
 */
//==============================================================================
FFWERR RetCmdData(MADDR madrStartAddr)
{
	DWORD					i;
	FFWERR					ferr = FFWERR_OK;
	FFW_PB_DATA*			pPb;
	BYTE					byCmdData;
	enum FFWENM_VERIFY_SET	eVerify;			// ベリファイON/OFF
	FFW_VERIFYERR_DATA		VerifyErr;			// ベリファイ結果格納構造体
	FFW_PBCLR_DATA*			pPbClr;
	BOOL					bRetFlg = FALSE;
	BYTE					byDbtCmd = 0x01;	// DBT

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	pPb = GetPbData();

	for (i = 0; i < pPb->dwSetNum; i++) {
		if (pPb->dwmadrAddr[i] == madrStartAddr) {
			// S/Wブレーク設定アドレスに現在書き込まれている内容を取得
			ferr = GetPbSetCmdData(madrStartAddr, madrStartAddr, &byCmdData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// DBT命令の場合だけ退避命令コードで置換
			if (byCmdData == byDbtCmd) {
			memcpy(&byCmdData, &pPb->byCmdData[i], sizeof(BYTE));
			}
			bRetFlg = TRUE;
			break;
		}
	}

	if (bRetFlg == FALSE) {
		pPbClr = GetPbClrData();					// S/Wブレーク埋め戻しデータ取得
		for (i = 0; i < pPbClr->dwSetNum; i++) {
			if (pPbClr->dwmadrAddr[i] == madrStartAddr) {
				// S/Wブレーク設定埋め戻しアドレスに現在書き込まれている内容を取得
				ferr = GetPbSetCmdData(madrStartAddr, madrStartAddr, &byCmdData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				// DBT命令の場合だけ退避命令コードで置換
				if (byCmdData == byDbtCmd) {
				memcpy(&byCmdData, &pPbClr->byCmdData[i], sizeof(BYTE));
				}
				break;
			}
		}
	}

	eVerify = VERIFY_OFF;

	SetCmdRetFillFlg(TRUE);		// 命令コード復帰状態をセット

	// 命令コード復帰
	ferr = DO_CWRITE(madrStartAddr, madrStartAddr, eVerify, &byCmdData, &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	SetCmdRetFillFlg(FALSE);	// 命令コード復帰状態をクリア

	return ferr;
}

//==============================================================================
/**
 * 命令コードを埋め込む。
 * @param  madrStartAddr 開始アドレス
 * @retval FFWERR_OK     正常終了
 * @retval FFWERR_COM    通信異常発生
 */
//==============================================================================
FFWERR FillCmdData(MADDR madrStartAddr)
{
	FFWERR					ferr = FFWERR_OK;
	enum FFWENM_VERIFY_SET	eVerify;									// ベリファイON/OFF
	FFW_VERIFYERR_DATA		VerifyErr;									// ベリファイ結果格納構造体
	BYTE	byFillData[] = {0x01};	// DBT

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	eVerify = VERIFY_OFF;

	SetCmdRetFillFlg(TRUE);		// 命令コード復帰状態をセット

	// 命令コード埋め込み
	ferr = DO_CWRITE(madrStartAddr, madrStartAddr, eVerify, &byFillData[0], &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	SetCmdRetFillFlg(FALSE);	// 命令コード復帰状態をクリア

	return ferr;
}

//=============================================================================
/**
 * S/Wブレーク設定アドレスの命令コード置換
 * @param  なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ReplaceCmdData(void)
{
	FFWERR			ferr = FFWERR_OK;
	BYTE			byReadData[12];		// 4Byte*3ブロック分の領域確保
	DWORD			i;
	FFW_PB_DATA*	pPb;
	FFW_PBCLR_DATA*	pPbClr;

	pPb = GetPbData();

	for (i = 0; i < pPb->dwSetNum; i++) {
		if (pPb->eFillState[i] != PB_FILL_NON) {
			ferr = DO_CDUMP(pPb->dwmadrAddr[i], pPb->dwmadrAddr[i], &byReadData[0]);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			memcpy(&pPb->byCmdData[i], &byReadData[0], sizeof(BYTE));	// DBT命令は1BYTE命令
		}
	}

	pPbClr = GetPbClrData();	// S/Wブレーク埋め戻しデータ取得
	for (i = 0; i < pPbClr->dwSetNum; i++) {
		ferr = DO_CDUMP(pPbClr->dwmadrAddr[i], pPbClr->dwmadrAddr[i], &byReadData[0]);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		memcpy(&pPbClr->byCmdData[i], &byReadData[0], sizeof(BYTE));	// DBT命令は1BYTE命令
	}
	
	return ferr;
}

//=============================================================================
/**
 * S/Wブレーク設定アドレスのデータ取得処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param pbyBuff DUMPデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetPbSetCmdData(MADDR madrStartAddr, MADDR madrEndAddr, BYTE* pbyBuff)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwLength;			// 取得エリアのバイトサイズ
	BYTE*	pbyReadBuf;
	BYTE*	pbyReadBufEsc;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	DWORD	dwTotalLength;
	// V.1.02 RevNo110304-001 Deleate & Append Line		Delete:BOOL	bFlashRomArea = FALSE;
	enum FFWRXENM_MAREA_TYPE	eAreaType;
	BOOL	bEnableArea = FALSE;

// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	dwTotalLength = madrEndAddr - madrStartAddr + 1;

	// データ格納領域を初期化
	// 確保ポインタと操作ポインタは別
	// アドレス補正により前後+0x3Byte、dwOffsetCntが最大3つシフトするため、+0xAメモリを確保しておく
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	pbyReadBufEsc = (BYTE*)malloc(dwTotalLength + 0xA);
	// RevRxNo130730-009 Append Start
	if( pbyReadBufEsc == NULL ){
		return FFWERR_MEMORY_NOT_SECURED;
	}
	// RevRxNo130730-009 Append End

	pbyReadBuf = pbyReadBufEsc;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	memset(pbyReadBuf, 0, dwTotalLength + 0xA);

//	// 取得アドレス範囲補正
	// V.1.02 RevNo110304-001 Modify Line
	ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 0, &bEnableArea);
	if (ferr != FFWERR_OK) {
		free(pbyReadBufEsc);	// RevRxNo130730-008 Append Line
		return ferr;
	}

	// アクセス可能領域の場合だけデータ取得
	if (dwLength > 0) {
		// S/Wブレーク設定アドレスをDUMP
		ferr = DO_CDUMP(madrStartAddr, madrStartAddr, pbyReadBuf);
		if (ferr != FFWERR_OK) {
			free(pbyReadBufEsc);
			return ferr;
		}
		memcpy(&pbyBuff[0], &pbyReadBuf[0], sizeof(BYTE));	// DBT命令は1BYTE命令
	} else {
		memset(&pbyBuff[0], 0x00, sizeof(BYTE));			// アクセス不可領域なので0x00を入れておく
	}

	free(pbyReadBufEsc);

	return ferr;
}

//=============================================================================
/**
 * ターゲットMCU制御関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData_Brk(void)
{

	s_bSetAllPBpointFlg = FALSE;

	s_bCmdRetFillFlg = FALSE;

	return;
}
