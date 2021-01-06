///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_extram.cpp
 * @brief MCU 外部RAM制御関数
 * @author RSD K.Okita, H.Hashiguchi, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/27
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121022-001	2012/11/21 SDS 岩田
	EZ-CUBE PROT_MCU_WRITE()分割処理対応
・RevRxE2LNo141104-001 2014/11/27 上田
	E2 Lite対応
*/
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif
#include "ffw_sys.h"
#include "mcu_extram.h"
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
#include "mcu_mem.h"
#include "mcu_sfr.h"
#include "mcuspec.h"

#include "do_sys.h"		// RevRxNo121022-001 Append Line

// static変数
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned Start
EXTRAM_DWNP_DATA	*ExtramDwnpTop;
static DWORD		s_dwExtramStructCount;
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned End
static FFW_ERROR_DETAIL_INFO	s_ExtramVrfyErrInfo;	// RevRxE2LNo141104-001 Append Line

// static関数の宣言
static void setExtRamVerifyErrorDetailInfo(FFW_VERIFYERR_DATA* pVerifyErr);	// RevRxE2LNo141104-001 Append Line

//=============================================================================
/**
 * 外部SDRAM CS領域のエンディアン取得
 * @param pwEndianState	:ENDIAN_LITTLE/ENDIAN_BIG
 * @retval FFWエラーコード
*/
//=============================================================================
FFWERR GetMcuExtSdCsEndian(WORD* pwEndianState)
{	
	FFWERR	ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE						byReadData[1];		// 参照データ格納領域

	*pwEndianState = GetEndianState();		// リセット時CPUエンディアン状態を取得
	eAccessSize = MBYTE_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(byReadData);
	ferr = GetMcuSfrReg(MCU_REG_BSC_SDCMOD, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if( (byReadData[0] & MCU_REG_BSC_SDCMOD_EMODE) ==  MCU_REG_BSC_SDCMOD_EMODE){				// SDCMOD.bit1が1の場合(CPUエンディアンと違う)
		if (*pwEndianState == FFWRX_ENDIAN_BIG){		// CPUがBIGエンディアンの場合
			(*pwEndianState) = FFWRX_ENDIAN_LITTLE;
		}else{					// CPUがLITTLEエンディアンの場合
			(*pwEndianState) = FFWRX_ENDIAN_BIG;
		}
	}
	return ferr;
}

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned Start
//=============================================================================
/**
 * 外部RAM用キャッシュメモリ構造体領域開放
 * @param なし
 * @retval なし
 */
//=============================================================================
void DeleteExtramCacheMem(void)
{

	// キャッシュメモリ管理構造体領域解放(既にキャッシュメモリ構造体領域が確保されている場合)
	EXTRAM_DWNP_DATA	*next, *p;
	if ( ExtramDwnpTop != NULL ) {
		p = ExtramDwnpTop;
		while (p) {
			next = p->next;
			delete [] p->pbyBuff;
			delete [] p;
			p = next;
		}
		ExtramDwnpTop = NULL;
	}

}

//=============================================================================
/**
 * 外部RAMダウンロード処理
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ExtWriteRamData(void)
{
	FFWERR ferr = FFWERR_OK;
	FFW_VERIFYERR_DATA	VerifyErr;
	EXTRAM_DWNP_DATA	*extram_ptr;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BOOL	bSameWriteData;
	DWORD	dwGageTotal = 0;
	DWORD	dwGageCnt = 0;
	// V.1.02 RevNo110323-002 Modify&Append Start
	DWORD	dwDwnpGageSize = 0;
	float	fCount;
	// V.1.02 RevNo110323-002 Modify&Append End

	// V.1.02 RevNo110517-001 Append Line
	DWORD	dwAccSize;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
// RevRxNo121022-001 Append Start

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	if( ExtramDwnpTop == NULL ){	// 外部RAMダウンロード用構造体が作ってない場合
		return ferr;		// ダウンロードデータなし
	}else{
		// 進捗ゲージトータルカウンタ計算
		extram_ptr = ExtramDwnpTop;
		// V.1.02 RevNo110517-001 Modify Start				
		while( extram_ptr != NULL ){
			switch (extram_ptr->eAccessSize) {
			case MBYTE_ACCESS:
				dwAccSize = 1;	// バイト数計算用乗算値
				break;
			case MWORD_ACCESS:
				dwAccSize = 2;	// バイト数計算用乗算値
				break;
			case MLWORD_ACCESS:
				dwAccSize = 4;	// バイト数計算用乗算値
				break;
			default:
				dwAccSize = 1;	// バイト数計算用乗算値(ここには来ないはずだが不定値だとおかしくなるので)
			}
			dwDwnpGageSize += (extram_ptr->dwCount * dwAccSize);
			extram_ptr = extram_ptr->next;
		}
		// V.1.02 RevNo110517-001 Modify End				
		// ExtFlashModule_007 Append Line
		SetGageCount(0);	// 進捗ゲージカウンタ用変数初期化
		SetMsgGageCount("External RAM");

		dwAreaNum = 1;
		bSameAccessSize = FALSE;
		bSameAccessCount = FALSE;
		bSameWriteData = FALSE;

		extram_ptr = ExtramDwnpTop;
		while( extram_ptr != NULL ){
// RevRxNo121022-001 Append Start
			if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
				ferr = McuWriteDivide(extram_ptr->eVerify, dwAreaNum, extram_ptr->madrStartAddr, bSameAccessSize, extram_ptr->eAccessSize,
								bSameAccessCount, extram_ptr->dwCount, bSameWriteData, extram_ptr->pbyBuff, &VerifyErr, extram_ptr->byEndian, PROT_EXT_WRITE_LENGTH_MAX_EZ);
			} else{
				ferr = PROT_MCU_WRITE(extram_ptr->eVerify, dwAreaNum, &extram_ptr->madrStartAddr, bSameAccessSize, &extram_ptr->eAccessSize,
								bSameAccessCount, &extram_ptr->dwCount, bSameWriteData, extram_ptr->pbyBuff, &VerifyErr, extram_ptr->byEndian);
			}
// RevRxNo121022-001 Append End

			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ベリファイチェック
			if (extram_ptr->eVerify == VERIFY_ON) {
				ferr = ChkVerifyWriteData(extram_ptr->madrStartAddr, extram_ptr->eAccessSize, extram_ptr->dwCount, extram_ptr->pbyBuff, 
																									&VerifyErr, extram_ptr->byEndian);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				if (VerifyErr.eErrorFlag == VERIFY_ERR) {			// ベリファイエラー発生時、処理を終了する。
					// RevRxE2LNo141104-001 Append Start
					// E2/E2 Liteの場合、FFWERR_EXTRAM_VERIFYを返送して詳細エラー情報を設定する。
					// E1/E20/EZ-CUBEは、デバッガ(CS+)がエラーコードに対応する時期に合わせて返送するようFFWを改定する。
					if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
						ferr = FFWERR_EXTRAM_VERIFY;	// 外部RAMのベリファイエラー
						setExtRamVerifyErrorDetailInfo(&VerifyErr);
					}
					// RevRxE2LNo141104-001 Append End
					return ferr;
				}
			}

			// 進捗ゲージ更新
			// V.1.02 RevNo110323-002 Modify Start
			// V.1.02 RevNo110517-001 Append Start				
			switch (extram_ptr->eAccessSize) {
			case MBYTE_ACCESS:
				dwAccSize = 1;	// バイト数計算用乗算値
				break;
			case MWORD_ACCESS:
				dwAccSize = 2;	// バイト数計算用乗算値
				break;
			case MLWORD_ACCESS:
				dwAccSize = 4;	// バイト数計算用乗算値
				break;
			default:
				dwAccSize = 1;	// バイト数計算用乗算値(ここには来ないはずだが不定値だとおかしくなるので)
			}
			// V.1.02 RevNo110517-001 Append End				
			// V.1.02 RevNo110517-001 Modify Line				
			dwGageCnt += (extram_ptr->dwCount * dwAccSize);
			fCount = ((float)dwGageCnt / dwDwnpGageSize) * GAGE_COUNT_MAX;
			dwGageTotal = (DWORD)fCount;
			SetGageCount(dwGageTotal);
			SetMsgGageCount("External RAM");
			// V.1.02 RevNo110323-002 Modify End
			// V.1.02 RevNo110326-002 Modify Line 進捗ゲージ計算の前にNextを入れていたのでおかしくなっていた
			extram_ptr = extram_ptr->next;
		}
	}
	return ferr;
}

//=============================================================================
/**
 * 外部RAMダウンロード用構造体登録数取得
 * @param なし
 * @retval DWORD s_dwExtramStructCount;
 */
//=============================================================================
DWORD GetExtramDwnpNum(void)
{

	return s_dwExtramStructCount;
}

//=============================================================================
/**
 * 外部RAMダウンロード用構造体登録数設定
 * @param DWORD *dwCount;		// 登録数
 * @retval なし
 */
//=============================================================================
void SetExtramDwnpNum(DWORD dwCount)
{

	s_dwExtramStructCount = dwCount;
	return;
}

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * 外部RAMベリファイエラーの詳細情報取得
 * @param なし
 * @retval 外部RAMベリファイエラーの詳細情報格納変数のポインタ
 */
//=============================================================================
FFW_ERROR_DETAIL_INFO* GetExtRamVerifyErrorDetailInfo(void)
{
	return &s_ExtramVrfyErrInfo;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * ターゲットMCU 外部RAM制御関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData_Extram(void)
{
	DWORD	i;		// RevRxE2LNo141104-001 Append Line
	errno_t ernerr;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	s_ExtramVrfyErrInfo.dwErrorCode = FFWERR_OK;
	for (i = 0; i < ERROR_MESSAGE_NUM_MAX; i++) {
		ernerr = strcpy_s(s_ExtramVrfyErrInfo.szErrorMessage[i], ERROR_MESSAGE_LENGTH_MAX, "No Message");
	}
	// RevRxE2LNo141104-001 Append End

	return;
}

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * 外部RAMベリファイエラーの詳細情報設定
 * @param pVerifyErr ベリファイ結果格納変数のポインタ
 * @retval なし
 */
//=============================================================================
static void setExtRamVerifyErrorDetailInfo(FFW_VERIFYERR_DATA* pVerifyErr)
{
	int		ierr;
	BYTE	byAcc;

	// エラーコード
	s_ExtramVrfyErrInfo.dwErrorCode = FFWERR_EXTRAM_VERIFY;

	// アクセスサイズ
	if (pVerifyErr->eAccessSize == MBYTE_ACCESS) {
		byAcc = 1;
	} else if (pVerifyErr->eAccessSize == MWORD_ACCESS) {
		byAcc = 2;
	} else {
		byAcc = 4;
	}
	ierr = sprintf_s(s_ExtramVrfyErrInfo.szErrorMessage[EXTRAMVRFY_ERRD_ACC_NO], ERROR_MESSAGE_LENGTH_MAX, "%01X", byAcc);

	// アドレス
	ierr = sprintf_s(s_ExtramVrfyErrInfo.szErrorMessage[EXTRAMVRFY_ERRD_ADDR_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%08X", pVerifyErr->dwErrorAddr);

	// ライトデータ、リードデータ
	if (byAcc == 1) {
		ierr = sprintf_s(s_ExtramVrfyErrInfo.szErrorMessage[EXTRAMVRFY_ERRD_WDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%02X", (pVerifyErr->dwErrorWriteData & 0x000000ff));
		ierr = sprintf_s(s_ExtramVrfyErrInfo.szErrorMessage[EXTRAMVRFY_ERRD_RDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%02X", (pVerifyErr->dwErrorReadData & 0x000000ff));
	} else if (byAcc == 2) {
		ierr = sprintf_s(s_ExtramVrfyErrInfo.szErrorMessage[EXTRAMVRFY_ERRD_WDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%04X", (pVerifyErr->dwErrorWriteData & 0x0000ffff));
		ierr = sprintf_s(s_ExtramVrfyErrInfo.szErrorMessage[EXTRAMVRFY_ERRD_RDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%04X", (pVerifyErr->dwErrorReadData & 0x0000ffff));
	} else {
		ierr = sprintf_s(s_ExtramVrfyErrInfo.szErrorMessage[EXTRAMVRFY_ERRD_WDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%08X", pVerifyErr->dwErrorWriteData);
		ierr = sprintf_s(s_ExtramVrfyErrInfo.szErrorMessage[EXTRAMVRFY_ERRD_RDATA_NO], ERROR_MESSAGE_LENGTH_MAX, "0x%08X", pVerifyErr->dwErrorReadData);
	}

	return;
}
// RevRxE2LNo141104-001 Append End
