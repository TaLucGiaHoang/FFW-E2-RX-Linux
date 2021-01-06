///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_inram.cpp
 * @brief MCU 内部RAMダウンロード制御関数
 * @author RSD S.Ueda, PA K.Tsumu
 * @author Copyright (C) 2014(2015) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2015/05/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/27 上田
	新規作成
・RevRxNo150528-001 2015/05/28 PA 紡車
    メモリウェイト仕様追加対応
*/
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif
#include "mcu_inram.h"
#include "mcu_mem.h"
#include "protmcu_mem.h"
#include "ffwmcu_mcu.h"
#include "do_sys.h"
#include "mcu_sfr.h"


// static変数
static BOOL		s_bCacheDwnpInRamData;				///< 内部RAMダウンロードデータのキャッシュ有無フラグ(TRUE:キャッシュあり/FALSE:キャッシュなし)
static INRAM_DWNP_INFO*	s_pInRamDwnpInfoTop;		///< 内部RAMダウンロード情報構造体変数のポインタ
static INRAM_DATA_BUFF*	s_pInRamDataBuff;			///< 内部RAMダウンロードデータ格納バッファ管理構造体変数のポインタ
static FFW_ERROR_DETAIL_INFO	s_InramVrfyErrInfo;	///< ベリファイエラーの詳細情報

// static関数宣言
static void	newInRamCacheMem(void);				///< 内部RAMダウンロードデータ格納バッファ確保
static void getRamAreaInfo(DWORD* pdwAreaNum, MADDR* pdwAreaStart, MADDR* pdwAreaEnd);	///< 内部RAM領域情報取得
static BOOL	chkDwnpInfoSameLastArea(INRAM_DWNP_INFO* pInfo, enum FFWENM_VERIFY_SET eVerify, MADDR madrStartAddr, 
									enum FFWENM_MACCESS_SIZE eAccessSize, BYTE byEndian);
												///< 指定された内部RAMダウンロード情報と同じ領域に登録できるかの確認
static void setInRamVerifyErrorDetailInfo(FFW_VERIFYERR_DATA* pVerifyErr);	///< MCU内蔵RAMベリファイエラーの詳細情報設定


//=============================================================================
/**
 * 内部RAMダウンロード情報格納
 * @param eVerify ベリファイ有無
 * @param madrStartAddr 開始アドレス
 * @param eAccessSize アクセスサイズ
 * @param dwAccessCount アクセス回数
 * @param byEndian エンディアン
 * @retval なし
 */
//=============================================================================
void SetInRamDwnpInfoPtr(enum FFWENM_VERIFY_SET eVerify, MADDR dwStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, BYTE byEndian)
{
	INRAM_DWNP_INFO*	pInfoLast;
	INRAM_DWNP_INFO*	pInfoAdd;
	BOOL	bSameArea;

	if (s_bCacheDwnpInRamData == FALSE) {	// 内部RAMダウンロードデータのキャッシュがない場合
		newInRamCacheMem();	// ダウンロードデータ格納バッファ確保
		s_pInRamDwnpInfoTop = (INRAM_DWNP_INFO*)new(INRAM_DWNP_INFO);	// ダウンロード情報構造体変数領域確保

		// ダウンロード情報格納
		s_pInRamDwnpInfoTop->eVerify = eVerify;
		s_pInRamDwnpInfoTop->dwStartAddr = dwStartAddr;
		s_pInRamDwnpInfoTop->eAccessSize = eAccessSize;
		s_pInRamDwnpInfoTop->dwCount = dwAccessCount;
		s_pInRamDwnpInfoTop->byEndian = byEndian;
		s_pInRamDwnpInfoTop->pNext = NULL;

		s_bCacheDwnpInRamData = TRUE;	// 内部RAMダウンロードデータバッファ確保済み

	} else {	// 内部RAMダウンロードデータのキャッシュがある場合
		// 最終情報のポインタを取得する
		pInfoLast = s_pInRamDwnpInfoTop;
		while (pInfoLast->pNext != NULL) {
			pInfoLast = pInfoLast->pNext;
		}

		// 最後に登録されている内部RAMダウンロード情報と同じ領域に登録できるかを確認
		bSameArea = chkDwnpInfoSameLastArea(pInfoLast, eVerify, dwStartAddr, eAccessSize, byEndian);
		if (bSameArea) {	//最終情報に同じ領域として登録可能な場合
			// アクセス回数を加算し、ダウンロード領域を1領域として登録する。
			pInfoLast->dwCount += dwAccessCount;

		} else {	// 既に登録されている領域と異なる場合
			pInfoAdd = (INRAM_DWNP_INFO*)new(INRAM_DWNP_INFO);	// ダウンロード情報構造体変数領域確保

			// ダウンロード情報格納
			pInfoAdd->eVerify = eVerify;
			pInfoAdd->dwStartAddr = dwStartAddr;
			pInfoAdd->eAccessSize = eAccessSize;
			pInfoAdd->dwCount = dwAccessCount;
			pInfoAdd->byEndian = byEndian;
			pInfoAdd->pNext = NULL;

			pInfoLast->pNext = pInfoAdd;	// ダウンロード情報格納構造体変数を最後に追加する
		}
	}
	return;
}

//=============================================================================
/**
 * 内部RAM用ダウンロードデータ格納バッファ解放
 * @param なし
 * @retval なし
 */
//=============================================================================
void DeleteInRamCacheMem(void)
{
	DWORD	dwCnt;
	INRAM_DWNP_INFO* pNext;
	INRAM_DWNP_INFO* p;

	if (s_bCacheDwnpInRamData) {

		// 内部RAMダウンロードデータ格納バッファ管理構造体変数のメモリ解放
		for (dwCnt = 0; dwCnt < s_pInRamDataBuff->dwAreaNum; dwCnt++) {
			delete [] s_pInRamDataBuff->pbyBuff[dwCnt];	// ダウンロードデータ格納バッファ解放
		}
		delete s_pInRamDataBuff;

		// 内部RAMダウンロード情報構造体変数用メモリ解放
		p = s_pInRamDwnpInfoTop;
		while (p) {
			pNext = p->pNext;
			delete [] p;
			p = pNext;
		}
		s_pInRamDwnpInfoTop = NULL;

		s_bCacheDwnpInRamData = FALSE;	// 内部RAMダウンロードデータバッファ未確保
	}

	return;
}

//=============================================================================
/**
 * 内部RAMダウンロードデータキャッシュ有無フラグの参照
 * @param なし
 * @retval TRUE 内部RAMダウンロードデータキャッシュあり
 * @retval FALSE 内部RAMダウンロードデータキャッシュなし
 */
//=============================================================================
BOOL GetInRamDwnpCacheFlg(void)
{
	return s_bCacheDwnpInRamData;
}

//=============================================================================
/**
 * 内部RAMダウンロードデータバッファのポインタ取得
 * @param dwRamAddr 内部RAMアドレス
 * @retval dwRamAddrの内部RAMダウンロードデータ格納バッファアドレス
 */
//=============================================================================
BYTE* GetInRamBuffPtr(MADDR dwRamAddr)
{
	DWORD	dwCnt;
	BYTE*	pbyBuf;

	pbyBuf = s_pInRamDataBuff->pbyBuff[0];
	for (dwCnt = 0; dwCnt < s_pInRamDataBuff->dwAreaNum; dwCnt++) {
		if ((s_pInRamDataBuff->dwRamStartAddr[dwCnt] <= dwRamAddr) && (dwRamAddr <= s_pInRamDataBuff->dwRamEndAddr[dwCnt])) {
			pbyBuf = s_pInRamDataBuff->pbyBuff[dwCnt] + (dwRamAddr - s_pInRamDataBuff->dwRamStartAddr[dwCnt]);
			break;
		}
	}

	return pbyBuf;
}

//=============================================================================
/**
 * 内部RAMダウンロード処理
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR WriteInRamData(void)
{
	FFWERR ferr = FFWERR_OK;
	FFW_VERIFYERR_DATA	VerifyErr;
	INRAM_DWNP_INFO	*pInramData;
	BYTE*	pbyDataBuf;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BOOL	bSameWriteData;
	FFWE20_EINF_DATA	einfData;
	MADDR	dwStartAddr;
	MADDR	dwEndAddr;
	DWORD	dwDataLength;
	BOOL	bRamEnable;
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];
	BYTE	byMemWaitInsert;
	DWORD	dwWriteLengthMax;
	DWORD	dwTotalLength;
	DWORD	dwLength;
	DWORD	dwAccCount;

	if (s_bCacheDwnpInRamData == FALSE) {	// 内部RAMダウンロードデータがない場合
		return ferr;		// 何もせず正常終了する
	}

	getEinfData(&einfData);			// エミュレータ情報取得

	VerifyErr.eErrorFlag = VERIFY_OK;

	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {	// E2/E2 Liteの場合
		dwWriteLengthMax = PROT_INRAM_WRITE_LENGTH_MAX_E2;
	} else if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// E1/E20の場合
		dwWriteLengthMax = PROT_INRAM_WRITE_LENGTH_MAX;
	} else {	// EZ-CUBEの場合
		dwWriteLengthMax = PROT_INRAM_WRITE_LENGTH_MAX_EZ;
	}

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	bSameWriteData = FALSE;

	pInramData = s_pInRamDwnpInfoTop;
	while (pInramData != NULL) {
		switch (pInramData->eAccessSize) {
		case MBYTE_ACCESS:
			dwDataLength = 1;
			break;
		case MWORD_ACCESS:
			dwDataLength = 2;
			break;
		case MLWORD_ACCESS:
			dwDataLength = 4;
			break;
		default:
			dwDataLength = 4;
			break;
		}
		dwStartAddr = pInramData->dwStartAddr;
		dwEndAddr = dwStartAddr + pInramData->dwCount * dwDataLength - 1;

		// 内部RAM領域の有効、動作設定
		bRamEnable = ChkRamArea(dwStartAddr, dwEndAddr, &bRamBlockEna[0]);
		if (bRamEnable == TRUE) {
			ferr = SetRegRamEnable(TRUE, &bRamBlockEna[0]);	// アクセスする内部RAM領域を有効/動作設定する
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// 内部RAM領域のメモリウェイト設定
		ferr = ChkMemWaitInsert(dwStartAddr, dwEndAddr, &byMemWaitInsert);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevRxNo150528-001 Modify Line
		if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
			ferr = SetMemWait(TRUE);	// メモリウェイト挿入
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

		pbyDataBuf = GetInRamBuffPtr(pInramData->dwStartAddr);	// ダウンロードデータ格納バッファポインタ取得

		// MCUへのライト処理実行
		dwTotalLength = dwEndAddr - dwStartAddr + 1;
		while (dwTotalLength > 0) {
			if (dwTotalLength > dwWriteLengthMax) {
				dwLength = dwWriteLengthMax;
			} else {
				dwLength = dwTotalLength;
			}
			dwAccCount = dwLength / dwDataLength;
			if (dwAccCount < 1) {
				dwAccCount = 1;
			}

			ferr = PROT_MCU_WRITE(pInramData->eVerify, dwAreaNum, &dwStartAddr, bSameAccessSize, &pInramData->eAccessSize,
									bSameAccessCount, &dwAccCount, bSameWriteData, pbyDataBuf, &VerifyErr, pInramData->byEndian);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ベリファイチェック
			if (pInramData->eVerify == VERIFY_ON) {
				ferr = ChkVerifyWriteData(dwStartAddr, pInramData->eAccessSize, dwAccCount, pbyDataBuf, &VerifyErr, pInramData->byEndian);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				if (VerifyErr.eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					break;
				}
			}
			dwStartAddr += dwLength;
			pbyDataBuf += dwLength;
			dwTotalLength -= dwLength;
		}

		// 内部RAM領域のメモリウェイト設定復帰
		// RevRxNo150528-001 Modify Line
		if (byMemWaitInsert != NO_MEM_WAIT_CHG) {
			ferr = SetMemWait(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// 内部RAM領域の有効、動作設定復帰
		if (bRamEnable == TRUE) {
			ferr = SetRegRamEnable(FALSE, &bRamBlockEna[0]);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

		if (VerifyErr.eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
			// E2/E2 Liteの場合、FFWERR_MCURAM_VERIFYを返送して詳細エラー情報を設定する。
			// E1/E20/EZ-CUBEは、デバッガ(CS+)がエラーコードに対応する時期に合わせて返送するようFFWを改定する。
			if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
				ferr = FFWERR_MCURAM_VERIFY;	// MCU内蔵RAMのベリファイエラー
				setInRamVerifyErrorDetailInfo(&VerifyErr);
			}
			break;
		}

		pInramData = pInramData->pNext;
	}
	return ferr;
}

//=============================================================================
/**
 * MCU内蔵RAMベリファイエラーの詳細情報取得
 * @param なし
 * @retval MCU内蔵RAMベリファイエラーの詳細情報格納変数のポインタ
 */
//=============================================================================
FFW_ERROR_DETAIL_INFO* GetInRamVerifyErrorDetailInfo(void)
{
	return &s_InramVrfyErrInfo;
}

///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ターゲットMCU 内部RAMダウンロード制御関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData_Inram(void)
{
	DWORD	i;
	errno_t ernerr;

	s_bCacheDwnpInRamData = FALSE;
	s_pInRamDwnpInfoTop = NULL;
	s_pInRamDataBuff = NULL;

	s_InramVrfyErrInfo.dwErrorCode = FFWERR_OK;
	for (i = 0; i < ERROR_MESSAGE_NUM_MAX; i++) {
		ernerr = strcpy_s(s_InramVrfyErrInfo.szErrorMessage[i], ERROR_MESSAGE_LENGTH_MAX, "No Message");
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 内部RAMダウンロードデータ格納バッファ確保
 * @param なし
 * @retval なし
 */
//=============================================================================
static void newInRamCacheMem(void)
{
	DWORD	dwRamAreaNum;
	DWORD	dwRamAreaStart[MCU_AREANUM_MAX_RX];
	DWORD	dwRamAreaEnd[MCU_AREANUM_MAX_RX];
	DWORD	dwCnt;
	DWORD	dwLength;


	// MCUコマンドで指定された内部RAM領域を、連続するブロックは1領域として昇順に並べ替える
	dwRamAreaNum = 0;
	getRamAreaInfo(&dwRamAreaNum, &dwRamAreaStart[0], &dwRamAreaEnd[0]);

	// 内部RAMダウンロードデータ格納バッファ管理構造体変数のメモリ確保
	s_pInRamDataBuff = (INRAM_DATA_BUFF*)new(INRAM_DATA_BUFF);

	s_pInRamDataBuff->dwAreaNum = dwRamAreaNum;
	for (dwCnt = 0; dwCnt < dwRamAreaNum; dwCnt++) {
		s_pInRamDataBuff->dwRamStartAddr[dwCnt] = dwRamAreaStart[dwCnt];
		s_pInRamDataBuff->dwRamEndAddr[dwCnt] = dwRamAreaEnd[dwCnt];
		dwLength = dwRamAreaEnd[dwCnt] - dwRamAreaStart[dwCnt] + 1;
		s_pInRamDataBuff->pbyBuff[dwCnt] = (BYTE *)new(BYTE[dwLength]);	// 内部RAMダウンロードデータ格納バッファのメモリ確保
	}

	return;
}

//=============================================================================
/**
 * 内部RAM領域情報取得(連続する領域を1領域に、昇順で取得する)
 * @param pdwAreaNum 領域数
 * @param pdwAreaStart 開始アドレス格納配列へのポインタ
 * @param pdwAreaEnd 終了アドレス格納配列へのポインタ
 * @return なし
 */
//=============================================================================
static void getRamAreaInfo(DWORD* pdwAreaNum, MADDR* pdwAreaStart, MADDR* pdwAreaEnd)
{
	FFWMCU_MCUAREA_DATA_RX	*pMcuArea;
	DWORD	i, j, k;
	BOOL	bSetFlg;
	DWORD	dwAreaNum;
	MADDR*	pdwAreaStartOrg;
	MADDR*	pdwAreaEndOrg;
	MADDR	dwRamStartAddr;
	MADDR	dwRamEndAddr;
	DWORD	dwAreaEnd;
	DWORD	dwNextAreaStart;

	pMcuArea = GetMcuAreaDataRX();

	pdwAreaStartOrg = pdwAreaStart;
	pdwAreaEndOrg = pdwAreaEnd;

	// MCU内部RAM領域を昇順に並べ替える
	dwAreaNum = 0;
	for (i = 0; i < pMcuArea->dwRamAreaNum; i++) {
		bSetFlg = FALSE;
		dwRamStartAddr = pMcuArea->dwmadrRamStartAddr[i];
		dwRamEndAddr = pMcuArea->dwmadrRamEndAddr[i];

		for (j = 0; j < dwAreaNum; j++) {
			// dwRamStartAddr, dwRamEndAddr < pdwAreaStart[j], pdwAreaEnd[j] の場合
			if (dwRamStartAddr < *(pdwAreaStartOrg + j)) {
				for (k = dwAreaNum; k > j; k--) {
					*(pdwAreaStartOrg + k) = *(pdwAreaStartOrg + k - 1);
					*(pdwAreaEndOrg + k) = *(pdwAreaEndOrg + k - 1);
				}
				// pdwAreaStart[j], pdwAreaEnd[j]に、dwRamStartAddr, dwRamEndAddrを追加
				*(pdwAreaStartOrg + j) = dwRamStartAddr;
				*(pdwAreaEndOrg + j) = dwRamEndAddr;

				bSetFlg = TRUE;
				dwAreaNum++;
				break;
			}
		}
		if (bSetFlg == FALSE) {
			*(pdwAreaStartOrg + dwAreaNum) = dwRamStartAddr;
			*(pdwAreaEndOrg + dwAreaNum) = dwRamEndAddr;
			dwAreaNum++;
		}
	}

	// 連続する領域は、1領域として格納する
	i = 0;
	while (i < dwAreaNum - 1) {
		dwAreaEnd = *(pdwAreaEndOrg + i);
		dwNextAreaStart = *(pdwAreaStartOrg + i + 1);

		if ((dwAreaEnd + 1) == dwNextAreaStart) {	// 領域[i]と領域[i+1]が連続している場合
			*(pdwAreaEndOrg + i) = *(pdwAreaEndOrg + i + 1);

			for (j = i + 1; j < dwAreaNum - 1; j++) {
				*(pdwAreaStartOrg + j) = *(pdwAreaStartOrg + j + 1);
				*(pdwAreaEndOrg + j) = *(pdwAreaEndOrg + j + 1);
			}
			dwAreaNum--;
			continue;
		}
		i++;
	}

	*pdwAreaNum = dwAreaNum;
}

//=============================================================================
/**
 * 指定された内部RAMダウンロード情報と同じ領域に登録できるかの確認
 * @param pInfo 内部RAMダウンロード情報構造体変数のポインタ
 * @param eVerify ベリファイ有無
 * @param madrStartAddr 開始アドレス
 * @param eAccessSize アクセスサイズ
 * @param dwAccessCount アクセス回数
 * @param byEndian エンディアン
 * @retval なし
 */
//=============================================================================
static BOOL chkDwnpInfoSameLastArea(INRAM_DWNP_INFO* pInfo, enum FFWENM_VERIFY_SET eVerify, MADDR dwStartAddr, 
									enum FFWENM_MACCESS_SIZE eAccessSize, BYTE byEndian)
{
	BOOL	bSame;
	MADDR	dwEndAddr;
	DWORD	dwLen;

	switch (pInfo->eAccessSize) {
	case MBYTE_ACCESS:
		dwLen = 1;
		break;
	case MWORD_ACCESS:
		dwLen = 2;
		break;
	case MLWORD_ACCESS:
		dwLen = 4;
		break;
	default:
		dwLen = 4;
		break;
	}
	dwEndAddr = pInfo->dwStartAddr + pInfo->dwCount * dwLen - 1;	// pInfoの終了アドレス算出

	// 追加するアドレスmadrStartAddrがpInfoと連続(madrStartAddr == pInfoの終了アドレス + 1)していて
	// アクセス回数以外の情報も一致する場合に、同じ領域に登録可と判定する。
	if (dwStartAddr == (dwEndAddr + 1)) {
		if (pInfo->eVerify != eVerify) {
			bSame = FALSE;
		} else if (pInfo->eAccessSize != eAccessSize) {
			bSame = FALSE;
		} else if (pInfo->byEndian != byEndian) {
			bSame = FALSE;
		} else {
			bSame = TRUE;
		}

	} else {
		bSame = FALSE;
	}

	return bSame;
}

//=============================================================================
/**
 * MCU内蔵RAMベリファイエラーの詳細情報設定
 * @param pVerifyErr ベリファイ結果格納変数のポインタ
 * @retval なし
 */
//=============================================================================
static void setInRamVerifyErrorDetailInfo(FFW_VERIFYERR_DATA* pVerifyErr)
{
	int		ierr;
	BYTE	byAcc;

	// エラーコード
	s_InramVrfyErrInfo.dwErrorCode = FFWERR_MCURAM_VERIFY;

	// アクセスサイズ
	if (pVerifyErr->eAccessSize == MBYTE_ACCESS) {
		byAcc = 1;
	} else if (pVerifyErr->eAccessSize == MWORD_ACCESS) {
		byAcc = 2;
	} else {
		byAcc = 4;
	}
	ierr = sprintf_s(s_InramVrfyErrInfo.szErrorMessage[INRAMVRFY_ERRD_ACC_NO], ERROR_MESSAGE_LENGTH_MAX, "%01X", byAcc);

	// アドレス
	ierr = sprintf_s(s_InramVrfyErrInfo.szErrorMessage[INRAMVRFY_ERRD_ADDR_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%08X", pVerifyErr->dwErrorAddr);

	// ライトデータ、リードデータ
	if (byAcc == 1) {
		ierr = sprintf_s(s_InramVrfyErrInfo.szErrorMessage[INRAMVRFY_ERRD_WDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%02X", (pVerifyErr->dwErrorWriteData & 0x000000ff));
		ierr = sprintf_s(s_InramVrfyErrInfo.szErrorMessage[INRAMVRFY_ERRD_RDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%02X", (pVerifyErr->dwErrorReadData & 0x000000ff));
	} else if (byAcc == 2) {
		ierr = sprintf_s(s_InramVrfyErrInfo.szErrorMessage[INRAMVRFY_ERRD_WDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%04X", (pVerifyErr->dwErrorWriteData & 0x0000ffff));
		ierr = sprintf_s(s_InramVrfyErrInfo.szErrorMessage[INRAMVRFY_ERRD_RDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%04X", (pVerifyErr->dwErrorReadData & 0x0000ffff));
	} else {
		ierr = sprintf_s(s_InramVrfyErrInfo.szErrorMessage[INRAMVRFY_ERRD_WDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%08X", pVerifyErr->dwErrorWriteData);
		ierr = sprintf_s(s_InramVrfyErrInfo.szErrorMessage[INRAMVRFY_ERRD_RDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%08X", pVerifyErr->dwErrorReadData);
	}

	return;
}

