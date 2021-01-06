///////////////////////////////////////////////////////////////////////////////
/**
 * @file comctrl.cpp
 * @brief 通信管理/例外処理
 *			Communi.DLLの関数呼び出しは、すべて本ファイル内で行う。
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#include "comctrl.h"
// RevRxE2No171004-001 Append Line
#include "do_sys.h"		// USB転送サイズ取得用

// ファイル内static変数の宣言
static int	(*s_COM_SendBlock)(char*, long) = NULL;
static int	(*s_COM_ReceiveBlock)(char*, long) = NULL;
static int	(*s_COM_ReceiveBlockH)(char*, long*) = NULL;
static int	(*s_COM_ReceivePolling)(long*) = NULL;
static int	(*s_COM_SetTimeOut)(long) = NULL;
static int	(*s_COM_Lock)(void) = NULL;
static int	(*s_COM_UnLock)(void) = NULL;
static int	(*s_COM_SetTransferSize)(long) = NULL;

static BYTE		s_bySendData[COM_SEND_BUFFER_SIZE];
static DWORD	s_dwSendDataSize;
static BYTE		s_byRcvData[COM_RCV_BUFFER_SIZE];
static DWORD	s_dwRcvDataSize;
static DWORD	s_dwRcvDataPtr;

static BOOL		s_bCOMErrFlg;	// 通信エラー発生有無フラグ(TRUE:通信エラー発生/FALSE:通信エラー発生していない)

// static関数の宣言
static int flashProtData(void);
//拡張機能対応
static BOOL _s_COM_ReceiveBlockN(char* rbuf, long rpacksiz);
static int(*s_COM_ReceiveBlockN)(char*, long) = _s_COM_ReceiveBlockN;
//=============================================================================
/**
 * Communi.DLLの各関数アドレスを設定する。
 * @param pSend Communi.DLLのCOM_SendBlock関数のアドレス
 * @param pRecv Communi.DLLのCOM_ReceiveBlock関数のアドレス
 * @param pRecvH Communi.DLLのCOM_ReceiveBlockH関数のアドレス
 * @param pRecvPolling Communi.DLLのCOM_ReceivePolling関数のアドレス
 * @param pSetTimeOut Communi.DLLのCOM_SetTimeOut関数のアドレス
 * @param pLock Communi.DLLのCOM_Lock関数のアドレス
 * @param pUnlock Communi.DLLのCOM_UnLock関数のアドレス
 * @param pTransSize Communi.DLLのCOM_SetTransferSize関数のアドレス
 * @retval なし
 */
//=============================================================================
void SetCOMFuncAddr(void* pSend, void* pRecv, void* pRecvH, void* pRecvPolling, void* pSetTimeOut, 
					void* pLock, void* pUnlock, void* pTransSize)
{
	s_COM_SendBlock = (int (*)(char*, long))pSend;
	s_COM_ReceiveBlock = (int (*)(char*, long))pRecv;
	s_COM_ReceiveBlockH = (int (*)(char*, long*))pRecvH;
	s_COM_ReceivePolling = (int (*)(long*))pRecvPolling;
	s_COM_SetTimeOut = (int (*)(long))pSetTimeOut;
	s_COM_Lock = (int (*)(void))pLock;
	s_COM_UnLock = (int (*)(void))pUnlock;
	s_COM_SetTransferSize = (int (*)(long))pTransSize;
}


//=============================================================================
/**
 * Communi.DLLの受信タイムアウトを設定する。
 * @param wFfwTimeOut 受信タイムアウト値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetCOMTimeOut(WORD wFfwTimeOut)
{
	long lTimeOut;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FFWERR_COM;
	}

	lTimeOut = static_cast<long>(wFfwTimeOut);

	if (s_COM_SetTimeOut(lTimeOut) != TRUE) {
		s_bCOMErrFlg = TRUE;
		return FFWERR_COM;
	}

	return FFWERR_OK;
}

//=============================================================================
/**
 * Communi.DLLのUSB転送サイズを設定する。
 * @param dwTransSize USB転送サイズ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetCOMTransferSize(DWORD dwTransSize)
{
	long lTransSize;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FFWERR_COM;
	}

	lTransSize = static_cast<long>(dwTransSize);

	if (s_COM_SetTransferSize(lTransSize) != TRUE) {
		s_bCOMErrFlg = TRUE;
		return FFWERR_COM;
	}

	return FFWERR_OK;
}


//=============================================================================
/**
 * BFWコマンドコード(2バイト)送信
 *   コマンドコードを上位、下位の順番に送信する。
 * @param wCmdCode コマンドコード
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int PutCmd(WORD wCmdCode)
{
	if (s_dwSendDataSize > (COM_SEND_BUFFER_SIZE - 2)) {
		if (flashProtData() != TRUE) {
			return FALSE;
		}
	}
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>((wCmdCode >> 8) & 0xff);
	s_dwSendDataSize++;
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>(wCmdCode & 0xff);
	s_dwSendDataSize++;

	return TRUE;
}


//=============================================================================
/**
 * 1バイトデータ送信
 * @param byData 送信データ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int PutData1(BYTE byData)
{
	if (s_dwSendDataSize > (COM_SEND_BUFFER_SIZE - 1)) {
		if (flashProtData() != TRUE) {
			return FALSE;
		}
	}
	s_bySendData[s_dwSendDataSize] = byData;
	s_dwSendDataSize++;

	return TRUE;
}


//=============================================================================
/**
 * 2バイトデータ送信
 *   wDataを 上位(bit15-8)、下位(bit7-0)の順番に送信する。
 * @param wData 送信データ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int PutData2(WORD wData)
{
	if (s_dwSendDataSize > (COM_SEND_BUFFER_SIZE - 2)) {
		if (flashProtData() != TRUE) {
			return FALSE;
		}
	}
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>((wData >> 8) & 0xff);
	s_dwSendDataSize++;
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>(wData & 0xff);
	s_dwSendDataSize++;

	return TRUE;
}


//=============================================================================
/**
 * 3バイトデータ送信
 *   dwDataのbit23-0の値を、上位(bit23-16)、下位(bit15-8)、最下位(bit7-0)の
 *   順番に送信する。
 * @param dwData 送信データ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int PutData3(DWORD dwData)
{
	if (s_dwSendDataSize > (COM_SEND_BUFFER_SIZE - 3)) {
		if (flashProtData() != TRUE) {
			return FALSE;
		}
	}
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>((dwData >> 16) & 0xff);
	s_dwSendDataSize++;
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>((dwData >> 8) & 0xff);
	s_dwSendDataSize++;
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>(dwData & 0xff);
	s_dwSendDataSize++;

	return TRUE;
}


//=============================================================================
/**
 * 4バイトデータ送信
 *   dwDataを最上位(bit31-24)、上位(bit23-16)、下位(bit15-8)、最下位(bit7-0)の
 *   順番に送信する。
 * @param dwData 送信データ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int PutData4(DWORD dwData)
{
	if (s_dwSendDataSize > (COM_SEND_BUFFER_SIZE - 4)) {
		if (flashProtData() != TRUE) {
			return FALSE;
		}
	}
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>((dwData >> 24) & 0xff);
	s_dwSendDataSize++;
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>((dwData >> 16) & 0xff);
	s_dwSendDataSize++;
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>((dwData >> 8) & 0xff);
	s_dwSendDataSize++;
	s_bySendData[s_dwSendDataSize] = static_cast<BYTE>(dwData & 0xff);
	s_dwSendDataSize++;

	return TRUE;
}


//=============================================================================
/**
 * nバイトデータ送信
 * @param dwSize 送信データのバイト数
 * @param pbyData 送信データが格納されているバッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int PutDataN(DWORD dwSize, const BYTE* pbyData)
{
	DWORD	dwTmpSize;
	DWORD	dwCnt;

	while (dwSize) {
		if (dwSize >= COM_SEND_BUFFER_SIZE) {
			dwTmpSize = COM_SEND_BUFFER_SIZE;
		} else {
			dwTmpSize = dwSize;
		}

		if (s_dwSendDataSize > (COM_SEND_BUFFER_SIZE - dwTmpSize)) {
			if (flashProtData() != TRUE) {
				return FALSE;
			}
		}
		for (dwCnt=0; dwCnt < dwTmpSize; dwCnt++) {
			s_bySendData[s_dwSendDataSize] = *pbyData;
			s_dwSendDataSize++;
			pbyData++;
		}

		dwSize -= dwTmpSize;
	}

	return TRUE;
}

//COM_ReceiveBlockH()
#ifdef COM_RECEIVE_H
//=============================================================================
/**
 * 1バイトデータ受信
 * @param pbyData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetData1(BYTE *const pbyData)
{
	DWORD	dwSize;

	dwSize = 1;
	if (GetDataN(dwSize, pbyData) != TRUE) {
		return FALSE;
	}

	return TRUE;
}

//=============================================================================
/**
 * 2バイトデータ受信
 *   pwDataを上位(bit15-8)、下位(bit7-0)の順番に受信する。
 * @param pwData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetData2(WORD *const pwData)
{
	BYTE	byData[2];
	WORD	wData;
	DWORD	dwSize;

	dwSize = 2;
	if (GetDataN(dwSize, &byData[0]) != TRUE) {
		return FALSE;
	}

	wData = (WORD)((static_cast<WORD>(byData[0]) << 8) & 0xff00);
	wData |= (static_cast<WORD>(byData[1]) & 0x00ff);
	*pwData = wData;

	return TRUE;
}


//=============================================================================
/**
 * 3バイトデータ受信
 *   pdwData bit23-0のデータを、上位(bit23-16)、下位(bit15-8)、最下位(bit7-0)の
 *   順番に受信する。
 * @param pdwData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetData3(DWORD *const pdwData)
{
	BYTE	byData[3];
	DWORD	dwData;
	DWORD	dwSize;

	dwSize = 3;
	if (GetDataN(dwSize, &byData[0]) != TRUE) {
		return FALSE;
	}

	dwData = (static_cast<DWORD>(byData[0]) << 16) & 0x00ff0000;
	dwData |= (static_cast<DWORD>(byData[1]) << 8) & 0x0000ff00;
	dwData |= (static_cast<DWORD>(byData[2]) & 0x000000ff);
	*pdwData = dwData;

	return TRUE;
}


//=============================================================================
/**
 * 4バイトデータ受信
 *   pdwDataを最上位(bit31-24)、上位(bit23-16)、下位(bit15-8)、最下位(bit7-0)の
 *   順番に受信する。
 * @param pdwData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetData4(DWORD *const pdwData)
{
	BYTE	byData[4];
	DWORD	dwData;
	DWORD	dwSize;

	dwSize = 4;
	if (GetDataN(dwSize, &byData[0]) != TRUE) {
		return FALSE;
	}

	dwData = (static_cast<DWORD>(byData[0]) << 24) & 0xff000000;
	dwData |= (static_cast<DWORD>(byData[1]) << 16) & 0x00ff0000;
	dwData |= (static_cast<DWORD>(byData[2]) << 8) & 0x0000ff00;
	dwData |= (static_cast<DWORD>(byData[3]) & 0x000000ff);
	*pdwData = dwData;

	return TRUE;
}


//=============================================================================
/**
 * Nバイトデータ受信
 *   pbyDataにdwSizeで指定されたバイト数分の受信データを格納する。
 * @param dwSize 受信データのバイト数 (1～0xFFFFFFFF)
 * @param pbyData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetDataN(DWORD dwSize, BYTE *const pbyData)
{
	DWORD	dwTmpSize;
	char*	pcRcvData;
	DWORD	dwCnt;
	long	lRcvSize;
	BYTE*	pbyBuff;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FALSE;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}

	if (s_COM_ReceiveBlockH) {

		pbyBuff = const_cast<BYTE*>(pbyData);
		while (dwSize) {
			if (s_dwRcvDataSize == 0) {	// 受信バッファにデータがない場合
				if (dwSize > COM_RCV_BUFFER_SIZE) {
					dwTmpSize = COM_RCV_BUFFER_SIZE;
				} else {
					dwTmpSize = dwSize;
				}

				pcRcvData = reinterpret_cast<char*>(&s_byRcvData);

				if (s_COM_ReceiveBlockH(pcRcvData, &lRcvSize) != TRUE) {
					s_bCOMErrFlg = TRUE;
					return FALSE;
				}
				s_dwRcvDataPtr = 0;
				s_dwRcvDataSize = static_cast<DWORD>(lRcvSize);

			} else {	// 受信バッファにデータがある場合
				dwTmpSize = dwSize;
			}

			if (s_dwRcvDataSize < dwTmpSize) {	// 受信バッファにあるデータサイズが要求サイズよりも小さい場合
				dwTmpSize = s_dwRcvDataSize;
			}
			for (dwCnt = 0; dwCnt < dwTmpSize; dwCnt++) {
				*pbyBuff = s_byRcvData[s_dwRcvDataPtr];
				s_dwRcvDataPtr++;
				pbyBuff++;
			}
			s_dwRcvDataSize -= dwTmpSize;
			dwSize -= dwTmpSize;
		}
	}

	return TRUE;
}

//=============================================================================
/**
 * 受信データの有無確認
 * @param pbArrive 受信データ有無フラグ格納バッファへのポインタ
 * @retval TRUE 受信データあり
 * @retval FALSE 受信データなし
 */
//=============================================================================
int ReceiveCheck(BOOL* pbArrive)
{
	BOOL	bFlg = FALSE;
	long	lArrive;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FALSE;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}

	if (s_dwRcvDataSize == 0) {

		if (s_COM_ReceivePolling) {
			if (s_COM_ReceivePolling(&lArrive) != TRUE) {
				s_bCOMErrFlg = TRUE;
				return FALSE;
			}
			if (lArrive == COM_EXIST) {
				bFlg = TRUE;
			} else {
				bFlg = FALSE;
			}
		}

	} else {
		bFlg = TRUE;
	}

	*pbArrive = bFlg;

	return TRUE;
}


// COM_ReceiveBlockH()

// COM_ReceiveBlock()
#else
//=============================================================================
/**
 * 1バイトデータ受信
 * @param pbyData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetData1(BYTE *const pbyData)
{
	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FFWERR_COM;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}
	if (s_COM_ReceiveBlock) {
		if (s_COM_ReceiveBlock((reinterpret_cast<char*>(pbyData)), 1) != TRUE) {
			s_bCOMErrFlg = TRUE;
			return FALSE;
		}
	}

	return TRUE;
}

//=============================================================================
/**
 * 2バイトデータ受信
 *   pwDataを上位(bit15-8)、下位(bit7-0)の順番に受信する。
 * @param pwData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetData2(WORD *const pwData)
{
	char cData[2];
	int	i;
	WORD wData;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FFWERR_COM;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}
	if (s_COM_ReceiveBlock) {
		for (i = 0; i < 2; i++) {
			if (s_COM_ReceiveBlock(&cData[i], 1) != TRUE) {
				s_bCOMErrFlg = TRUE;
				return FALSE;
			}
		}

		wData = (static_cast<WORD>(cData[0]) << 8) & 0xff00;
		wData |= (static_cast<WORD>(cData[1]) & 0x00ff);
		*pwData = wData;
	}

	return TRUE;
}


//=============================================================================
/**
 * 3バイトデータ受信
 *   pdwData bit23-0のデータを、上位(bit23-16)、下位(bit15-8)、最下位(bit7-0)の
 *   順番に受信する。
 * @param pdwData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetData3(DWORD *const pdwData)
{
	char cData[3];
	int	i;
	DWORD	dwData;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FFWERR_COM;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}
	if (s_COM_ReceiveBlock) {
		for (i = 0; i < 3; i++) {
			if (s_COM_ReceiveBlock(&cData[i], 1) != TRUE) {
				s_bCOMErrFlg = TRUE;
				return FALSE;
			}
		}

		dwData = (static_cast<DWORD>(cData[0]) << 16) & 0x00ff0000;
		dwData |= (static_cast<DWORD>(cData[1]) << 8) & 0x0000ff00;
		dwData |= (static_cast<DWORD>(cData[2]) & 0x000000ff);
		*pdwData = dwData;
	}

	return TRUE;
}


//=============================================================================
/**
 * 4バイトデータ受信
 *   pdwDataを最上位(bit31-24)、上位(bit23-16)、下位(bit15-8)、最下位(bit7-0)の
 *   順番に受信する。
 * @param pdwData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetData4(DWORD *const pdwData)
{
	char cData[4];
	int	i;
	DWORD dwData;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FFWERR_COM;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}
	if (s_COM_ReceiveBlock) {
		for (i = 0; i < 4; i++) {
			if (s_COM_ReceiveBlock(&cData[i], 1) != TRUE) {
				s_bCOMErrFlg = TRUE;
				return FALSE;
			}
		}

		dwData = (static_cast<DWORD>(cData[0]) << 24) & 0xff000000;
		dwData |= (static_cast<DWORD>(cData[1]) << 16) & 0x00ff0000;
		dwData |= (static_cast<DWORD>(cData[2]) << 8) & 0x0000ff00;
		dwData |= (static_cast<DWORD>(cData[3]) & 0x000000ff);
		*pdwData = dwData;
	}

	return TRUE;
}


//=============================================================================
/**
 * Nバイトデータ受信
 *   pbyDataにdwSizeで指定されたバイト数分の受信データを格納する。
 * @param dwSize 受信データのバイト数 (1～0xFFFFFFFF)
 * @param pbyData 受信データ格納バッファへのポインタ
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int GetDataN(DWORD dwSize, BYTE *const pbyData)
{
	DWORD	dwTmpSize;
	BYTE*	pRcvData;
	char cData;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FFWERR_COM;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}
	if (s_COM_ReceiveBlock) {

		pRcvData = pbyData;
		for (dwTmpSize = 0; dwTmpSize < dwSize; dwTmpSize++) {
//			if (s_COM_ReceiveBlock((reinterpret_cast<char*>(pRcvData)), 1) != TRUE) {
//				s_bCOMErrFlg = TRUE;
//				return FALSE;
//			}
			if (s_COM_ReceiveBlock(&cData, 1) != TRUE) {
				s_bCOMErrFlg = TRUE;
				return FALSE;
			}
			*pRcvData = static_cast<BYTE>(cData);
			pRcvData++;
		}	
	}

	return TRUE;
}
//=============================================================================
/**
 * 受信データの有無確認
 * @param pbArrive 受信データ有無フラグ格納バッファへのポインタ
 * @retval TRUE 受信データあり
 * @retval FALSE 受信データなし
 */
//=============================================================================
int ReceiveCheck(BOOL* pbArrive)
{
	BOOL	bFlg = FALSE;
	long	lArrive;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FALSE;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}

	if (s_dwRcvDataSize == 0) {

		if (s_COM_ReceivePolling) {
			if (s_COM_ReceivePolling(&lArrive) != TRUE) {
				s_bCOMErrFlg = TRUE;
				return FALSE;
			}
			if (lArrive == COM_EXIST) {
				bFlg = TRUE;
			} else {
				bFlg = FALSE;
			}
		}

	} else {
		bFlg = TRUE;
	}

	*pbArrive = bFlg;

	return TRUE;
}


#endif
// COM_ReceiveBlock()



//=============================================================================
/**
 * 送信データ掃き出し(BFWCmd_SetUSBT送信時に使用する)
 * @param なし
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
int FlashProtDataForced(void)
{
	int	nRet;

	nRet = flashProtData();

	return nRet;
}

//=============================================================================
/**
 * INITコマンド発行時の通信管理/例外処理用内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitComctrlData(void)
{
	s_dwSendDataSize = 0;
	s_dwRcvDataSize = 0;
	s_dwRcvDataPtr = 0;
	s_bCOMErrFlg = FALSE;
}

//拡張機能対応
//=============================================================================
/**
* Nバイトデータ受信(ReceiveBlockH)
*	 pbyDataにdwSizeで指定されたバイト数分の受信データを格納する。
* @param dwSize 受信データのバイト数 (1～0xFFFFFFFF)
* @param pbyData 受信データ格納バッファへのポインタ
* @retval TRUE 正常終了
* @retval FALSE 送信失敗
* @attention dwSizeはUSB_TRANS_UNIT以下であること
*/
//=============================================================================
int GetDataN_H(DWORD dwSize, BYTE *const pbyData)
{
	long size;
	int	 rc = TRUE;
	DWORD dwTransSize;		// RevRxE2No171004-001 Append Line

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg) {
		return FALSE;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}

	/*
	* COM_ReceiveBlockHでReadFileの呼び出しを一回にするため
	* 一時的にread_sizeを変更する目的でSetCOMTransferSize()を呼び出す。
	* USBドライバのMaximumTransferSizeは初回のSetCOMTransferSize()でのみ
	* 設定されるのでここでは変更されない
	*/
	if (SetCOMTransferSize(dwSize) != FFWERR_OK) {
		return FALSE;
	}

	if (s_COM_ReceiveBlockH((char*)pbyData, &size) != TRUE) {
		s_bCOMErrFlg = TRUE;
		rc = FALSE;
	}

	dwTransSize = GetUsbTransferSize();		// RevRxE2No171004-001 Append Line

	if (SetCOMTransferSize(dwTransSize) != FFWERR_OK) {		// RevRxE2No171004-001 Modify Line
		//	if (SetCOMTransferSize(USB_TRANS_UNIT) != TRUE) {
		return FALSE;
	}

	return rc;
}

//=============================================================================
/**
* 送信データ掃き出し及びNバイトデータ受信
*  dwSizeで指定されたバイト数分のデータを受信する。(内部バッファに格納するのみ)
* @param dwSize 受信データのバイト数 (1～0xFFFFFFFF)
* @retval TRUE 正常終了
* @retval FALSE 送信失敗
*/
//=============================================================================
int flashProtDataRcvDataN(DWORD dwSize)
{
	DWORD	dwTmpSize;
	char*	pcRcvData;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FALSE;
	}

	if (flashProtData() != TRUE) {
		return FALSE;
	}

	if (s_COM_ReceiveBlockN) {

		if (s_dwRcvDataSize == 0) {	// 受信バッファにデータがない場合
			if (dwSize > COM_RCV_BUFFER_SIZE) {
				dwTmpSize = COM_RCV_BUFFER_SIZE;
			}
			else {
				dwTmpSize = dwSize;
			}

			pcRcvData = (char*)(&s_byRcvData);

			if (s_COM_ReceiveBlockN(pcRcvData, dwTmpSize) != TRUE) {
				s_bCOMErrFlg = TRUE;
				return FALSE;
			}
			s_dwRcvDataPtr = 0;
			s_dwRcvDataSize = (DWORD)(dwTmpSize);
		}
	}

	return TRUE;
}

//暫定で実装
static BOOL _s_COM_ReceiveBlockN(char* rbuf, long rpacksiz)
{
	BOOL	recv_complete = FALSE;
	ULONG	remaining = rpacksiz;
	ULONG	total_recvd = 0;
	ULONG	rsize;

	if (rpacksiz > 0) {
		while (recv_complete == FALSE) {
			// 受信データは64KB単位
			rsize = min(remaining, 0x1000);

			if (s_COM_ReceiveBlockH((char *)(&(rbuf[total_recvd])), (long *)&rsize) == FALSE) {
				//error = COM_GetLastError();  // エラーコード確認用
											 //IncrementUsbServerErrorCount();	// 突然のUSB切断はCheckTargetStatus()で確認する
				return FALSE;
			}
			total_recvd += rsize;
			remaining -= rsize;
			if (remaining == 0) {
				recv_complete = TRUE;
			}
		}
	}
	return TRUE;
}

//=============================================================================
// static関数
//=============================================================================
//=============================================================================
/**
 * 送信データ掃き出し
 * @param なし
 * @retval TRUE 正常終了
 * @retval FALSE 送信失敗
 */
//=============================================================================
static int flashProtData(void)
{
	int	nRet;

	// 通信エラーが発生している場合、INITコマンドが発行されるまではエミュレータと通信せず
	// 通信エラーを返送する。
	if (s_bCOMErrFlg == TRUE) {
		return FFWERR_COM;
	}

	if (s_dwSendDataSize != 0) {
		if (s_COM_SendBlock) {
			nRet = s_COM_SendBlock((char*)&s_bySendData, s_dwSendDataSize);
			s_dwSendDataSize = 0;
			if (nRet != TRUE) {
				s_bCOMErrFlg = TRUE;
			}
			return nRet;
		}
	}

	return TRUE;
}

