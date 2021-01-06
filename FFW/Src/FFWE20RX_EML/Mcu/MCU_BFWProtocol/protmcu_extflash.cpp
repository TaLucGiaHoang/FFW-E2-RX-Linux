///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_extflash.cpp
 * @brief BFWコマンド プロトコル生成関数(外部フラッシュダウンロード関連)
 * @author RSO K.Okita, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#include "protmcu_extflash.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "prot_common.h"
#include "mcu_extflash.h"

// 外部フラッシュダウンロード用処理
//=============================================================================
/**
 * BFWMCUCmd_EFWRITESTARTコマンドの発行
 * @param byEndian CPUエンディアン
 * @param byConnect 外部フラッシュ接続形態
 * @param eAccessSize ワークRAMへのアクセスサイズ
 * @param madrProgStartAddr ワークRAMスタートアドレス
 * @param madrBuffStartAddr 外部フラッシュライトバッファ開始アドレス
 * @param dwBuffSize 外部フラッシュライトバッファサイズ
 * @param madrSectAddr イレーズ/ライトベースアドレス
 * @param dwCom8_1 コマンド設定アドレス8bit(1st)
 * @param dwCom8_2 コマンド設定アドレス8bit(2nd)
 * @param dwCom16_1 コマンド設定アドレス16bit(1st)
 * @param dwCom16_2 コマンド設定アドレス16bit(2nd)
 * @param dwCom32_1 コマンド設定アドレス32bit(1st)
 * @param dwCom32_2 コマンド設定アドレス32bit(1st)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_EFWRITESTART(BYTE byEndian, BYTE byConnect, enum FFWENM_MACCESS_SIZE eAccessSize, 
							 MADDR madrProgStartAddr, MADDR madrBuffStartAddr, DWORD dwBuffSize, MADDR madrSectAddr,
							 WORD Com8_1, WORD Com8_2, WORD Com16_1, WORD Com16_2, WORD Com32_1, WORD Com32_2 )
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	// BFWMCUCmd_EFWRITESTART送信
	wCmdCode = BFWCMD_EFWRITESTART;
	ferr = ProtSendCmd(wCmdCode);				// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (PutData1(byConnect) != TRUE) {			// 接続形態
		return FFWERR_COM;
	}
	if (PutData1((BYTE)eAccessSize) != TRUE) {		// ワークRAMアクセスサイズ
		return FFWERR_COM;
	}
	if (PutData1(byEndian) != TRUE) {			// エンディアン
		return FFWERR_COM;
	}
	if (PutData4(madrProgStartAddr) != TRUE) {	// ワークRAM先頭アドレス
		return FFWERR_COM;
	}

	if (PutData4(madrBuffStartAddr) != TRUE) {	// バッファ開始ベースアドレス
		return FFWERR_COM;
	}

	if (PutData4(dwBuffSize) != TRUE) {			// バッファサイズ
		return FFWERR_COM;
	}

	if (PutData4(madrSectAddr) != TRUE) {		// イレーズ/ライトベースアドレス
		return FFWERR_COM;
	}

	if (PutData2(Com8_1) != TRUE) {				// JEDECコマンド設定アドレス8bit(1st)
		return FFWERR_COM;
	}
	if (PutData2(Com8_2) != TRUE) {				// JEDECコマンド設定アドレス8bit(2nd)
		return FFWERR_COM;
	}
	if (PutData2(Com16_1) != TRUE) {			// JEDECコマンド設定アドレス16bit(1st)
		return FFWERR_COM;
	}
	if (PutData2(Com16_2) != TRUE) {			// JEDECコマンド設定アドレス16bit(2nd)
		return FFWERR_COM;
	}
	if (PutData2(Com32_1) != TRUE) {			// JEDECコマンド設定アドレス32bit(1st)
		return FFWERR_COM;
	}
	if (PutData2(Com32_2) != TRUE) {			// JEDECコマンド設定アドレス32bit(2nd)
		return FFWERR_COM;
	}

	ferr = ProtGetStatus(wCmdCode);				// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);				// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWMCUCmd_EFWRITEコマンドの発行
 * @param eAccessSize ワークRAMアクセスサイズ
 * @param byMode 実行モード
 * @param dwMakerID			メーカID
 * @param wDeviceID		デバイスID
 * @param madrStartAddr イレーズ/ライト開始アドレス
 * @param pbyWriteData ライトデータ格納用バッファへのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数にpwEraseStatusを追加 )
FFWERR PROT_MCU_EFWRITE(BYTE byMode, WORD wSectEraseTimeout,
						DWORD dwMakerID, DWORD dwDeviceID, MADDR madrStartAddr, const BYTE* pbyWriteData, WORD* pwEraseStatus)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;
	BYTE*	pbyData;

	// BFWMCUCmd_EFWRITE送信
	wCmdCode = BFWCMD_EFWRITE;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (PutData1(byMode) != TRUE) {			// 実行モード
		return FFWERR_COM;
	}
	if( byMode != EFLASHROM_CMP_ID ){					// メーカID/デバイスID比較でない場合
		if (PutData4(madrStartAddr) != TRUE) {
			return FFWERR_COM;
		}
		if( byMode == EFLASHROM_WRITE ){		// ライトモードの場合
			if (PutData2(wSectEraseTimeout) != TRUE) {
				return FFWERR_COM;
			}
			pbyData = const_cast<BYTE*>(pbyWriteData);
			if (PutDataN(PROT_EFWRITE_LENGTH, pbyData) != TRUE) {					// ライトデータ送信
				return FFWERR_COM;
			}
		}else if( byMode == EFLASHROM_STATUS_CHK ){
			if (PutData2(wSectEraseTimeout) != TRUE) {
				return FFWERR_COM;
			}
			if( wSectEraseTimeout ){		// イレーズタイムアウト時間指定有りの場合
				pbyData = const_cast<BYTE*>(pbyWriteData);
				if (PutDataN(PROT_EFWRITE_LENGTH, pbyData) != TRUE) {					// ライトデータ送信
					return FFWERR_COM;
				}
			}
		}
	}else{
		if (PutData4(dwMakerID) != TRUE) {			// メーカID
			return FFWERR_COM;
		}

		if (PutData4(dwDeviceID) != TRUE) {			// デバイスID
			return FFWERR_COM;
		}
	}
 	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// ExtFlashModule_005 Modify Start ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
	if (GetData2(&wBuf) != TRUE) {	// イレーズ後ステータスチェック結果受信
		return FFWERR_COM;
	}
	*pwEraseStatus = wBuf;
	// ExtFlashModule_005 Modify End

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}


//=============================================================================
/**
 * BFWMCUCmd_EFWRITEENDコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_EFWRITEEND(void)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	// BFWMCUCmd_EFWRITEEND送信
	wCmdCode = BFWCMD_EFWRITEEND;
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
 * 外部Flashダウンロードコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Extflash(void)
{
	return;
}
