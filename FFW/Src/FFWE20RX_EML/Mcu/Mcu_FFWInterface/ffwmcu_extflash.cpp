///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_extflash.cpp
 * @brief 外部Flashダウンロードの実装ファイル
 * @author RSO K.Okita, H.Hashiguchi, H.Akashi, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/08/02
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo121017-003	2012/10/30 明石
　VS2008 warning C4996対策
・RevRxNo121022-001   2012/11/20 SDS Iwata
  ・FFWCmd_SetEXTF()
     EZ-CUBEの場合、何も処理を行わず、正常終了を返し、処理を終了に変更
・RevRxNo130301-001 2013/08/02 植盛
	RX64M対応
*/
#include "ffwmcu_extflash.h"
#include "domcu_extflash.h"
#include "errchk.h"

#include "do_sys.h"		// RevRxNo121022-001 Append Line
#include "ffwmcu_mcu.h"	// RevRxNo130301-001 Append Line
#include "domcu_mcu.h"	// RevRxNo130301-001 Append Line

// グローバル変数
FFW_EXTF_DATA e_ExtfInfo;		// GUIから渡された外部フラッシュ情報保持用
// ファイル内static変数の宣言

//=============================================================================
/**
 * 外部フラッシュダウンロード情報の設定
 * @param pExtfInfo 外部フラッシュダウンロード情報を格納する構造体
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWCmd_SetEXTF(FFW_EXTF_DATA pExtfInfo)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE i;
	WORD n;
	// RevNo121017-003	Append Line
	errno_t ernerr;

	// RevRxNo130301-001 Append Start
	MADDR	madrRamWorkStart;					// ワークRAM先頭アドレス
	MADDR	madrRamWorkEnd;						// ワークRAM終了アドレス
	MADDR	madrCmpStart[MCU_AREANUM_MAX_RX];	// 比較用先頭アドレス
	MADDR	madrCmpEnd[MCU_AREANUM_MAX_RX];		// 比較用終了アドレス
	DWORD	dwCnt;
	DWORD	dwCmpCnt;
	BOOL	bWorkRamInRam;						// ワークRAM領域包含フラグ
	FFWRX_MCUAREA_DATA* pMcuArea;
	// RevRxNo130301-001 Append End
// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData); // エミュレータ情報取得
// RevRxNo121022-001 Append End

	pMcuArea = GetMcuAreaDataRX();		// RevRxNo130301-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

// RevRxNo121022-001 Append Start
	// EZ-CUBEの場合、何も処理を行わず、正常終了を返した処理終了
	if (einfData.wEmuStatus == EML_EZCUBE){
		return FFWERR_OK;
	}
// RevRxNo121022-001 Append End

	// RevRxNo130301-001 Append Start
	for( i = 0; i < FFW_EXTF_DATA_NUM_MAX; i++ ){
		if (pExtfInfo[i].wSectorNum != 0x0000) {		// 総セクタ数が0ではない場合
			if (pExtfInfo[i].byWorkRamType == WORKRAM_INT) {
				// ワークRAM種別が内蔵RAMの場合

				// MCUコマンドで指定された内蔵RAMの先頭/終了アドレスを比較用配列に格納
				for (dwCnt = 0; dwCnt < pMcuArea->dwRamAreaNum; dwCnt++) {
					madrCmpStart[dwCnt] = pMcuArea->dwmadrRamStartAddr[dwCnt];
					madrCmpEnd[dwCnt] = pMcuArea->dwmadrRamEndAddr[dwCnt];
				}
				// MCUコマンドで指定された内蔵RAMブロック数を確保
				dwCmpCnt = pMcuArea->dwRamAreaNum;

			} else {	
				// ワークRAM種別が外部RAMの場合

				// MCUコマンドで指定された外部RAMの先頭/終了アドレスを比較用配列に格納
				for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
					madrCmpStart[dwCnt] = pMcuArea->dwmadrExtMemBlockStart[dwCnt];
					madrCmpEnd[dwCnt] = pMcuArea->dwmadrExtMemBlockEnd[dwCnt];
				}
				// MCUコマンドで指定された外部RAMブロック数を確保
				dwCmpCnt = pMcuArea->dwExtMemBlockNum;
			}

			// ワークRAM先頭/終了アドレスを格納
			madrRamWorkStart = pExtfInfo[i].dwWorkStart;
			madrRamWorkEnd = pExtfInfo[i].dwWorkStart + pExtfInfo[i].wWorkSize - 1;

			// ワークRAM領域がRAM領域の範囲内であるかを確認
			bWorkRamInRam = ChkIncludeArea(madrRamWorkStart, madrRamWorkEnd, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);

			if (bWorkRamInRam == FALSE) {	// RAM領域範囲外の場合、引数エラーでreturnする
				return FFWERR_FFW_ARG;
			}
		}
	}
	// RevRxNo130301-001 Append End

	for( i = 0; i < FFW_EXTF_DATA_NUM_MAX; i++ ){
		e_ExtfInfo[i].wSectorNum = pExtfInfo[i].wSectorNum;

		// RevRxNo130301-001 Append Line
		if (pExtfInfo[i].wSectorNum != 0x0000) {		// 総セクタ数が0ではない場合
			e_ExtfInfo[i].byChipErase = pExtfInfo[i].byChipErase;
			e_ExtfInfo[i].wChipEraseTimeout = pExtfInfo[i].wChipEraseTimeout;
			for( n = 0; n < EXTF_FROM_SECTOR_MAX; n++ ){
				e_ExtfInfo[i].dwSectAddr[n] = pExtfInfo[i].dwSectAddr[n];		
				e_ExtfInfo[i].dwSectSize[n] = pExtfInfo[i].dwSectSize[n];
				e_ExtfInfo[i].byEraseInfo[n] = pExtfInfo[i].byEraseInfo[n];
			}
			e_ExtfInfo[i].dwStartAddr = pExtfInfo[i].dwStartAddr;
			e_ExtfInfo[i].dwEndAddr = pExtfInfo[i].dwEndAddr;
			// RevNo121017-003	Modify Start
			ernerr = strcpy_s( (char *)e_ExtfInfo[i].byExtRomBfScript, MAX_PATH_SIZE, (char *)pExtfInfo[i].byExtRomBfScript );
			ernerr = strcpy_s( (char *)e_ExtfInfo[i].byExtRomAfScript, MAX_PATH_SIZE, (char *)pExtfInfo[i].byExtRomAfScript );
			// RevNo121017-003	Modify End
			e_ExtfInfo[i].dwMaxSectorSize = pExtfInfo[i].dwMaxSectorSize;
			e_ExtfInfo[i].dwWorkStart = pExtfInfo[i].dwWorkStart;
			e_ExtfInfo[i].wWorkSize = pExtfInfo[i].wWorkSize;
			e_ExtfInfo[i].byWorkAccessSize = pExtfInfo[i].byWorkAccessSize;
			e_ExtfInfo[i].byWorkRamType = pExtfInfo[i].byWorkRamType;
			// RevNo121017-003	Modify Start
			ernerr = strcpy_s( (char *)e_ExtfInfo[i].byExtRamBfScript, MAX_PATH_SIZE, (char *)pExtfInfo[i].byExtRamBfScript );
			ernerr = strcpy_s( (char *)e_ExtfInfo[i].byExtRamAfScript, MAX_PATH_SIZE, (char *)pExtfInfo[i].byExtRamAfScript );
			// RevNo121017-003	Modify End
			e_ExtfInfo[i].dwBuffStart = pExtfInfo[i].dwBuffStart;
			e_ExtfInfo[i].wBuffSize = pExtfInfo[i].wBuffSize;
			e_ExtfInfo[i].wMakerID = pExtfInfo[i].wMakerID;
			e_ExtfInfo[i].wDeviceID = pExtfInfo[i].wDeviceID;
			e_ExtfInfo[i].wSectEraseTimeout = pExtfInfo[i].wSectEraseTimeout;
			e_ExtfInfo[i].byFastWriteMode = pExtfInfo[i].byFastWriteMode;
			e_ExtfInfo[i].byUnlock = pExtfInfo[i].byUnlock;
			e_ExtfInfo[i].byConnect = pExtfInfo[i].byConnect;
			e_ExtfInfo[i].byWriteProgramType = pExtfInfo[i].byWriteProgramType;
			e_ExtfInfo[i].wCommandAddr8_1 = pExtfInfo[i].wCommandAddr8_1;
			e_ExtfInfo[i].wCommandAddr8_2 = pExtfInfo[i].wCommandAddr8_2;
			e_ExtfInfo[i].wCommandAddr16_1 = pExtfInfo[i].wCommandAddr16_1;
			e_ExtfInfo[i].wCommandAddr16_2 = pExtfInfo[i].wCommandAddr16_2;
			e_ExtfInfo[i].wCommandAddr32_1 = pExtfInfo[i].wCommandAddr32_1;
			e_ExtfInfo[i].wCommandAddr32_2 = pExtfInfo[i].wCommandAddr32_2;
			for( n = 0; n < EXTF_WRITE_PROGRAM_MAX; n++ ){
				e_ExtfInfo[i].byWriteProgram[n] = pExtfInfo[i].byWriteProgram[n];
			}
		}
	}

	// V.1.02 RevNo110613-001 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Append End

	return ferr;
}


///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 外部フラッシュROMダウンロードコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_ExtFlash(void)
{
	return;
}