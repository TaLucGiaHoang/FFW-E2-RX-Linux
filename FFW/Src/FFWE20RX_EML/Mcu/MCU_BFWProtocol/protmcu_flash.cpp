///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_flash.cpp
 * @brief BFWコマンド プロトコル生成関数(内蔵Flash操作関連)
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/12/18
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121026-001	2012/10/30 上田
  RX100対応
  ・PROT_MCU_FWRITESTART()に起動時のエンディアン情報送信処理追加。
・RevRxEzNo130117-001 2013/01/17 橋口
   EZ-CUBE ブロック数が0x197ブロックを超える場合にダウンロードができない不具合改修
・RevRxNo130301-001 2013/04/16 上田
	RX64M対応
・RevRxNo130730-009 2013/11/15 大喜多
　　mallocでメモリが確保できなかった場合にエラーを返す処理を追加
・RevRxNo130730-009 2013/12/18 大喜多
　　PROT_MCU_FWRITE()のmalloc処理をnewに変更
*/
#include "protmcu_flash.h"
#include "comctrl.h"
#include "prot.h"
#include "errchk.h"
#include "prot_common.h"
#include "ffw_sys.h"
#include <malloc.h>

#include "ffwmcu_mcu.h"
#include "domcu_mcu.h"
#include "do_sys.h"	// RevRxNo130301-001 Append Line

//=============================================================================
/**
 * BFWMCUCmd_FWRITESTARTコマンドの発行
 * @param dwFlashWriteTiming フラッシュ書き換えタイミング
 * @param dwFlashClrInfo フラッシュメモリ初期化情報
 * @param pFclrData フラッシュメモリ初期化ブロック情報格納構造体変数へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
//RevNo010804-001 Modify Line
//RevRxNo121026-001 Modify Line
FFWERR PROT_MCU_FWRITESTART(DWORD dwFlashWriteTiming, DWORD dwFlashClrInfo, FFW_FCLR_DATA_RX* pFclrData, enum FFWRX_WTR_NO eSetWtrNo, enum FFWENM_ENDIAN eMcuEndian)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;
	DWORD	dwCnt;
	BYTE	byData;
	WORD	wData;

	//ワーニング対策
	dwFlashWriteTiming;

	// BFWMCUCmd_FWRITESTART送信
	wCmdCode = BFWCMD_FWRITESTART;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.02 RevNo110323-004 Append Start
	wData = static_cast<WORD>(dwFlashClrInfo);
	if (PutData2(wData) != TRUE) {		// FCLR指定送信
		return FFWERR_COM;
	}
	// V.1.02 RevNo110323-004 Append End

	if (dwFlashClrInfo == BFW_FWRITESTART_FCLR_ON) {	// フラッシュメモリを初期化する場合

		if (PutData4(pFclrData->dwNum) != TRUE) {	// 初期化ブロック数送信
			return FFWERR_COM;
		}

		for (dwCnt = 0; dwCnt < pFclrData->dwNum; dwCnt++) {
			// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Line
			if (PutData4(pFclrData->dwmadrBlkStart[dwCnt]) != TRUE) {	// 初期化ブロック開始アドレス送信
				return FFWERR_COM;
			}
			// V.1.02 No.11 フラッシュROMブロック細分化対応 Append Start
			if (PutData1(pFclrData->byFlashType[dwCnt]) != TRUE) {		// 初期化対象領域送信
				return FFWERR_COM;
			}
			// V.1.02 No.11 フラッシュROMブロック細分化対応 Append End
		}
	}

	//RevNo010804-001 Append Start
	// WTR書き込みプログラムNo設定
	byData = static_cast<BYTE>(eSetWtrNo);
	if (PutData1(byData) != TRUE) {
		return FFWERR_COM;
	}
	//RevNo010804-001 Append End

	// RevRxNo121026-001 Append Start
	byData = static_cast<BYTE>(eMcuEndian);
	if (PutData1(byData) != TRUE) {
		return FFWERR_COM;
	}
	// RevRxNo121026-001 Append End

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}
// RevRxEzNo130117-001 Append Start
//=============================================================================
/**
 * EZ-CUBE用 BFWMCUCmd_FWRITESTARTコマンドの発行
 *  pFclrDataをBFWDLLに渡す時に、pFclrDataのサイズがCOM_SEND_BUFFER_SIZEを超える場合があるので分割して渡す
 *  !!!!!!!!!!!!!!!!!!!注意!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *  PROT_MCU_FWRITESTART()のプロトコルが変わるときは注意すること
 *  !!!!!!!!!!!!!!!!!!!注意!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * @param dwFlashWriteTiming フラッシュ書き換えタイミング
 * @param dwFlashClrInfo フラッシュメモリ初期化情報
 * @param pFclrData フラッシュメモリ初期化ブロック情報格納構造体変数へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_FWRITESTART_EZ(DWORD dwFlashWriteTiming, DWORD dwFlashClrInfo, FFW_FCLR_DATA_RX* pFclrData, enum FFWRX_WTR_NO eSetWtrNo, enum FFWENM_ENDIAN eMcuEndian)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wBuf;
	WORD	wCmdCode;
	DWORD	dwFclrCnt = 0;			// BFWDLLに送ったフラッシュメモリ初期化ブロックカウント
	DWORD	dwBuffCnt = 0;			// FFW→BFWDLL間バッファカウンタ
	BYTE	byData;
	WORD	wData;
	DWORD	bFclrLoop = TRUE;		// BFWCMD_FWRITESTARTループフラグ

	//ワーニング対策
	dwFlashWriteTiming;
	
	// pFclrDataをBFWDLLに渡す時に、pFclrDataのサイズがCOM_SEND_BUFFER_SIZEを超える場合があるので分割して渡す
	while(bFclrLoop == TRUE){

		// BFWMCUCmd_FWRITESTART送信
		wCmdCode = BFWCMD_FWRITESTART;
		ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// V.1.02 RevNo110323-004 Append Start
		wData = static_cast<WORD>(dwFlashClrInfo);
		if (PutData2(wData) != TRUE) {		// FCLR指定送信
			return FFWERR_COM;
		}
		// V.1.02 RevNo110323-004 Append End

		if (dwFlashClrInfo == BFW_FWRITESTART_FCLR_ON) {	// フラッシュメモリを初期化する場合

			if (PutData4(pFclrData->dwNum) != TRUE) {	// 初期化ブロック数送信	
				return FFWERR_COM;
			}

			for (dwBuffCnt = 0; dwFclrCnt < pFclrData->dwNum; dwFclrCnt++,dwBuffCnt++) {
				// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Line
				if (PutData4(pFclrData->dwmadrBlkStart[dwFclrCnt]) != TRUE) {	// 初期化ブロック開始アドレス送信
					return FFWERR_COM;
				}
				// V.1.02 No.11 フラッシュROMブロック細分化対応 Append Start
				if (PutData1(pFclrData->byFlashType[dwFclrCnt]) != TRUE) {		// 初期化対象領域送信
					return FFWERR_COM;
				}
				// V.1.02 No.11 フラッシュROMブロック細分化対応 Append End
				// COM_SEND_BUFFER_SIZE分データがたまったら、一度BFWDLLのFWRITESTARTを完了させるのでfor文を抜ける
				// コマンドヘッダ情報+フラッシュメモリ初期化情報+フラッシュメモリ初期化ブロック数 8BYTE
				// フラッシュメモリ初期化ブロック先頭アドレス+フラッシュメモリ初期化対象領域 5BYTE * 取得回数
				// 書き込みプログラムのバージョン情報 +起動時のエンディアン指定取得 2BYTE のため
				// FWRITESTART_FCLRBLK_MAX_EZ((COM_SEND_BUFFER_SIZE-10)/5)-1)回以上の情報はCOM_SEND_BUFFER_SIZEがオーバーフローするので
				// BFWMCUCmd_FWRITESTART再発行
				if( dwBuffCnt >= FWRITESTART_FCLRBLK_MAX_EZ ){
					//dwFclrCntを+1しておかないと、次のFWRITESTARTが前の最後と同じ値になる
					dwFclrCnt++;
					break;
				}
			}
			if(dwFclrCnt == pFclrData->dwNum){
				// pFclrDataのブロック数分送信完了したらループフラグはFALSEにする
				bFclrLoop = FALSE;
			}
		} else {
			// フラッシュ未初期化の場合はループフラグをFALSEにして、1回だけFWRITESTARTを実施
			bFclrLoop = FALSE;
		}

		//RevNo010804-001 Append Start
		// WTR書き込みプログラムNo設定
		byData = static_cast<BYTE>(eSetWtrNo);
		if (PutData1(byData) != TRUE) {
			return FFWERR_COM;
		}
		//RevNo010804-001 Append End

		// RevRxNo121026-001 Append Start
		byData = static_cast<BYTE>(eMcuEndian);
		if (PutData1(byData) != TRUE) {
			return FFWERR_COM;
		}
		// RevRxNo121026-001 Append End

		ferr = ProtGetStatus(wCmdCode);	// ステータス受信
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信
	}

	return ferr;
}
// RevRxEzNo130117-001 Append End
//=============================================================================
/**
 * BFWMCUCmd_FWRITEコマンドの発行
 *   pbyWriteData[]には、madrWriteAddr[], eAccessSize[], dwLength[]の
 *   配列要素番号0の領域に対応するライトデータから順に格納する。
 *   bSameAccessSizeがTRUEの場合、全領域eAccessSize[0]のアクセスサイズでアクセスする。
 *   bSameLengthがTRUEの場合、全領域dwLength[0]のアクセスサイズでアクセスする。
 *   bSameWriteDataがTRUEの場合、全領域pbyWriteDataの先頭ライトデータをライトする。
 * @param byWriteArea 書き換え領域指定(BFW_FWRITE_AREA_FLASH/BFW_FWRITE_AREA_EXTRA_A)
 * @param eVerify べリファイ指定(VERIFY_OFF/VERIFY_ON)
 * @param dwAreaNum ライト領域数
 * @param madrWriteAddr[] ライト開始アドレス
 * @param bSameAccessSize 同一アクセスサイズ指定フラグ
 * @param eAccessSize[] アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameLength 同一ライトアクセス回数指定フラグ
 * @param dwLength[] 1領域のライトアクセス回数(1～0x10000)
 * @param bSameWriteData 同一ライトデータ指定フラグ
 * @param pbyWriteData ライトデータ格納用バッファへのポインタ
 * @param pVerifyErrorInfo ベリファイ結果格納用バッファアドレス
 * @param byEndian エンディアン指定(0:リトル、1:ビッグ)
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo130301-001 Modify Line
FFWERR PROT_MCU_FWRITE(BYTE byWriteArea, enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, const MADDR madrWriteAddr[], 
					   BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize[], 
					   BOOL bSameLength, const DWORD dwLength[], 
					   BOOL bSameWriteData, const BYTE* pbyWriteData, 
					   FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian)
{
	FFWERR	ferr;
	BYTE	byData;
	BYTE	byBuf;
	WORD	wBuf;
	WORD	wData;
	DWORD	dwBuf;
	DWORD	dwCnt;
	BYTE*	pbyData;
	enum FFWENM_MACCESS_SIZE eSendAcc;
	DWORD	dwSendLength;
	WORD	wCmdCode;
	FFWE20_EINF_DATA	einfData;	// RevRxNo130301-001 Append Line

	// RevRxNo130301-001 Append Line
	getEinfData(&einfData);			// エミュレータ情報取得

	// BFWMCUCmd_FWRITE送信
	wCmdCode = BFWCMD_FWRITE;
	ferr = ProtSendCmd(wCmdCode);			// コマンドコード送信
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130301-001 Modify Start
	if (einfData.wEmuStatus != EML_EZCUBE) {	// EZ-CUBEでない場合
		if (PutData1(byWriteArea) != TRUE) {		// 書き換え領域指定
			return FFWERR_COM;
		}
	} else {
		if (PutData1(BFWDATA_RESERV) != TRUE) {
			return FFWERR_COM;
		}
	}
	// RevRxNo130301-001 Modify End

	byData = static_cast<BYTE>(eVerify);
	if (PutData1(byData) != TRUE) {	// ベリファイ指定送信
		return FFWERR_COM;
	}
	if (PutData2(BFWDATA_RESERV) != TRUE) {
		return FFWERR_COM;
	}
	wData = static_cast<WORD>(dwAreaNum);
	if (PutData2(wData) != TRUE) {	// ブロック数送信
		return FFWERR_COM;
	}

	pbyData = const_cast<BYTE*>(pbyWriteData);

	for (dwCnt = 0; dwCnt < dwAreaNum; dwCnt++) {
		if (bSameAccessSize == TRUE) {
			eSendAcc = eAccessSize[0];
		} else {
			eSendAcc = eAccessSize[dwCnt];
		}
		byData = static_cast<BYTE>(eSendAcc);
		if (PutData1(byData) != TRUE) {		// アクセスサイズ送信
			return FFWERR_COM;
		}

		if (bSameLength == TRUE) {
			dwSendLength = dwLength[0];
		} else {
			dwSendLength = dwLength[dwCnt];
		}
		if (PutData3(dwSendLength) != TRUE) {	// レングス送信
			return FFWERR_COM;
		}

		if (PutData4(madrWriteAddr[dwCnt]) != TRUE) {	// アドレス送信
			return FFWERR_COM;
		}

		if (eSendAcc == MWORD_ACCESS) {
			dwSendLength = dwSendLength * 2;
		} else if (eSendAcc == MLWORD_ACCESS) {
			dwSendLength = dwSendLength * 4;
		}

		// ------------------------------------------------------------------------------
		// 以下の並び替え判定処理は
		// BFW側でのデータの扱いがWRITEとFWRITEで異なっている為、
		// FFWでもWRITEとFWRITEで異なっている
		// ※WRITE：エンディアンに依存せず、ワード/ロングワードなら並び替える
		// ※FWRITE：ビッグエンディアンの場合はワード/ロングワードであっても並び替えない
		// ------------------------------------------------------------------------------

		// ビッグエンディアン時
		if (byEndian == FFWRX_ENDIAN_BIG) {
			if (PutDataN(dwSendLength, pbyData) != TRUE) {
				return FFWERR_COM;
			}
		}
		// リトルエンディアン時
		else {
			if (eAccessSize[dwCnt] == MBYTE_ACCESS) {	// バイトアクセス時
				if (PutDataN(dwSendLength, pbyData) != TRUE) {
					return FFWERR_COM;
				}
			}
			else {										// ワード、ロングワードアクセス時
				BYTE*	arbyBuff;
				// RevRxNo130730-009 Modify Line(USB通信処理の途中で終わると以降通信異常となるためエラー処理を削除)
				arbyBuff = (BYTE *)new(BYTE[dwSendLength]);		// malloc→new：newだと確保できたかの確認不要
				memcpy(arbyBuff, pbyData, dwSendLength);

				// データ並び替え
				ReplaceEndian(arbyBuff, eSendAcc, dwSendLength);
				memmove(pbyData, arbyBuff, dwSendLength); 

				if (PutDataN(dwSendLength, pbyData) != TRUE) {
					delete [] arbyBuff;		// RevRxNo130730-009 Append Line
					return FFWERR_COM;
				}

				// 並び替えたデータを元に戻す(ベリファイチェックで使用する為)
				memcpy(arbyBuff, pbyData, dwSendLength);
				ReplaceEndian(arbyBuff, eSendAcc, dwSendLength);
				memmove(pbyData, arbyBuff, dwSendLength); 
				delete [] arbyBuff;		// RevRxNo130730-009 Modify Line
			}
		}
		if (bSameWriteData == FALSE) {
			pbyData += dwSendLength;	// ライトデータ格納バッファアドレス更新
		}
	}

	ferr = ProtGetStatus(wCmdCode);	// ステータス受信
	if (ferr != FFWERR_OK) {
		return ferr;
	}


	if (GetData1(&byBuf) != TRUE) {	// ベリファイエラー発生有無受信
		return FFWERR_COM;
	}
	pVerifyErrorInfo->eErrorFlag = static_cast<FFWENM_VERIFY_RESULT>(byBuf);

	if (GetData1(&byBuf) != TRUE) {	// ベリファイエラー発生時のアクセスサイズ受信
		return FFWERR_COM;
	}
	pVerifyErrorInfo->eAccessSize = static_cast<FFWENM_MACCESS_SIZE>(byBuf);

	if (GetData4(&dwBuf) != TRUE) {	// ベリファイエラー発生時のアドレス受信
		return FFWERR_COM;
	}
	pVerifyErrorInfo->dwErrorAddr = dwBuf;

	if (GetData4(&dwBuf) != TRUE) {	// ベリファイエラー発生時のライトデータ受信
		return FFWERR_COM;
	}
	pVerifyErrorInfo->dwErrorWriteData = dwBuf;

	if (GetData4(&dwBuf) != TRUE) {	// ベリファイエラー発生時のリードデータ受信
		return FFWERR_COM;
	}
	pVerifyErrorInfo->dwErrorReadData = dwBuf;

	ferr = ProtRcvHaltCode(&wBuf);	// 処理中断コード受信

	return ferr;
}

//=============================================================================
/**
 * BFWMCUCmd_FWRITEENDコマンドの発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR PROT_MCU_FWRITEEND(void)
{
	FFWERR	ferr;
	WORD	wBuf;
	WORD	wCmdCode;

	// BFWMCUCmd_FWRITEEND送信
	wCmdCode = BFWCMD_FWRITEEND;
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
 * 内蔵Flash操作コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitProtMcuData_Flash(void)
{
	return;
}
