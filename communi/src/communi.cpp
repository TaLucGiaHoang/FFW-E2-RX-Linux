////////////////////////////////////////////////////////////////////////////////
/**
 * @file communi.cpp.cpp
 * @brief Communi.DLLトップモジュール処理ソースコード
 * @author RSD M.Yamamoto
 * @author Copyright (C) 2015 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2015/01/15
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴 (V.3.00.00.000から記載)
・RevRxE2LNo141104-001 2015/01/15 山本
	新規作成
*/

#include "libusb.h"
#include "communi.h"

#include	"Communierr.h"

// #include	"serial.h"
// #include	"usb_e1usb.h"
#include	"usb_e2usb.h"
// #include	"usb_vcom_ezcube.h"

static	CIF_OBJ	*pIF = (CIF_OBJ*)NULL;
static	int		error = 0;
static	int		TimeOut = 4;
//==============================================================================
/**
 * 通信インターフェースのオープン
 * @param  type	オープンする通信インターフェース
 * @param  cparam	オープンする通信インターフェースの個別識別情報(COM番号orシリアル番号)
 * @param  lparam	オープンする通信インターフェースの設定
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_Open(long type, char *cparam, long lparam)
{
	ULONGLONG condition = 0;

	if(pIF != (CIF_OBJ*)NULL){
		pIF->SetErr(ER_COM_EXIST);
		return FALSE;
	}

	switch(type){
		case COM_TYPE_USB_E2USB:
		pIF = (CIF_OBJ*) new (CUSB_E2USB);
		break;
		default:
		error = ER_COM_PARA;
		return FALSE;
	}
	
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_OPEN;
		return FALSE;
	}
	
	if(pIF->SetTimeOut(TimeOut) != TRUE){
		error = pIF->GetLastError();
		delete pIF;
		pIF = (CIF_OBJ*)NULL;
		return FALSE;
	}
	
	if(pIF->Open(type, cparam, lparam) != TRUE){
		error = pIF->GetLastError();
		delete pIF;
		pIF = (CIF_OBJ*)NULL;
		return FALSE;
	}

	return TRUE;
}



//==============================================================================
/**
 * 通信インターフェースのクローズ
 * @param  なし(void)
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_Close(void)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}

	long ret = pIF->Close();
	error = pIF->GetLastError();
	delete pIF;
	pIF = (CIF_OBJ*)NULL;
	return ret;
}



//==============================================================================
/**
 * 受信タイムアウト設定
 * @param timeout タイムアウト時間(秒単位)
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_SetTimeOut(long timeout)
{
	int	ret = TRUE;

	if(timeout >= 4){
		TimeOut = timeout;
		if(pIF != (CIF_OBJ*)NULL){
			ret = pIF->SetTimeOut(timeout);
		}
	}
	return ret;
}

//==============================================================================
/**
 * ブロックデータの送信(コマンドの送信)
 * @param buf 受信データを格納する領域へのポインタ
 * @param long 送信するデータのバイト数
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_SendBlock(char *buf, long size)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}

	return pIF->SendBlock(buf, size);
}



//==============================================================================
/**
 * 受信データの取得(モニタシリアル専用)
 * @param *buf 受信データを格納する領域へのポインタ
 * @param size 受信バッファのサイズ(バイト数)
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_ReceiveBlock(char *buf, long size)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}
	int ret = pIF->ReceiveBlock(buf, size);

	return ret;
}



//==============================================================================
/**
 * 受信データの取得_大量データ取得用(E1usb.sys,E2usb.sys専用)
 * @param *buf 受信データを格納する領域へのポインタ
 * @param *size 受信したデータ数を格納するポインタ
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_ReceiveBlockH(char *buf, long *size)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}
	int ret = pIF->ReceiveBlockH(buf, size);

	return ret;
}

//==============================================================================
/**
 * 受信データの取得(応答の受信)Masopi用
 * @param *buf 受信データを格納する領域へのポインタ
 * @param size 受信バッファのサイズ(バイト数)
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_ReceiveBlockM(char *buf, long size)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}
	int ret = pIF->ReceiveBlockM(buf, size);

	return ret;
}



//==============================================================================
/**
 * 受信データの有無を確認(E1usb.sys専用)
 * @param arrive 受信データの有無を格納。COM_EXIST:受信データあり、COM_NONE:受信データなし
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_ReceivePolling(long *arrive)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}
	return pIF->ReceivePolling(arrive);
}



//==============================================================================
/**
 * 受信データ数を確認
 * @param arrive データ数を格納する領域へのポインタ
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_Return_data_size(int *size)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}
	return pIF->Return_data_size(size);
}



//==============================================================================
/**
 * エラー番号の取得
 * @param なし
 * @retval 最後に発生したエラーのエラー番号
 */
//==============================================================================
int COM_GetLastError(void)
{
	if(pIF == (CIF_OBJ*)NULL){
		return error;
	}
	return pIF->GetLastError();
}



//==============================================================================
/**
 * 通信状況を取得
 * @param mode オープンされているモード(COM_Open()関数のtypeパラメタの値が格納)
 * @retval TRUE:Openされている。 FALSE:Openされていない
 */
//==============================================================================
int COM_Is_Open(int *mode)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}
	if(mode != NULL){
		*mode = pIF->GetComType();
	}
	return TRUE;
}




//==============================================================================
/**
 * 通信インターフェースのロック(予約関数)
 */
//==============================================================================
int COM_Lock(void)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}
	return pIF->Lock();
}



//==============================================================================
/**
 * 通信インターフェースのアンロック(予約関数)
 */
//==============================================================================
int COM_UnLock(void)
{
	if(pIF == (CIF_OBJ*)NULL){
		error = ER_COM_NONE;
		return FALSE;
	}
	return pIF->UnLock();
}



//==============================================================================
/**
 * USB通信転送サイズ設定
 * @param  long 転送サイズを指定(byte単位)
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_SetTransferSize(long size)
{
	int	ret = TRUE;

	if(size >= 1){		// 設定値は1以上
		if(pIF != (CIF_OBJ*)NULL){
			ret = pIF->SetTransferSize(size);
		}
	}else{
		ret = FALSE;
	}
	return ret;
}

//==============================================================================
/**
 * USBベンダーコントロールイン
 * @param  char コントロール転送のbRequest
 * @param  unsigned short  コントロール転送のwVal
 * @param  unsigned short  コントロール転送のwIndex
 * @param  char* 入力領域
 * @param  int  入力領域サイズ
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_VendorControlIn(char req, unsigned short val, unsigned short index, char *in, int len)
{
	int	ret = FALSE;

	if (pIF != (CIF_OBJ*)NULL) {
		ret = pIF->VendorControlIn(req, val, index, in, len);
	}
	return ret;
}

//==============================================================================
/**
 * USBベンダーコントロールアウト
 * @param  char コントロール転送のbRequest
 * @param  unsigned short  コントロール転送のwVal
 * @param  unsigned short  コントロール転送のwIndex
 * @param  char* 出力領域
 * @param  int  出力領域サイズ
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int COM_VendorControlOut(char req, unsigned short val, unsigned short index, char *out, int len)
{
	int	ret = FALSE;

	if (pIF != (CIF_OBJ*)NULL) {
		ret = pIF->VendorControlOut(req, val, index, out, len);
	}
	return ret;
}
