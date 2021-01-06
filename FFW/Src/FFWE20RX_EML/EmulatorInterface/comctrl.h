///////////////////////////////////////////////////////////////////////////////
/**
 * @file comctrl.h
 * @brief 通信管理/例外処理のヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
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
#ifndef	__COMCTRL_H__
#define	__COMCTRL_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義
#define COM_SEND_BUFFER_SIZE	(2048)		// 送信バッファサイズ
#define COM_RCV_BUFFER_SIZE		(0x20000)	// 受信バッファサイズ

#define COM_NONE	0	// 受信データなし
#define COM_EXIST	1	// 受信データ取得可能

#define TIME_RCVCHK_INTERVAL	(200)		// ReceiveCheck()発行間隔 (200ms)


// グローバル関数の宣言
extern void SetCOMFuncAddr(void* pSend, void* pRecv, void* pRecvH, void* pRecvPolling, void* pSetTimeOut,
						   void* pLock, void* pUnlock, void* pTransSize);
extern FFWERR SetCOMTimeOut(WORD wFfwTimeOut);
extern FFWERR SetCOMTransferSize(DWORD dwTransSize);

extern int PutCmd(WORD wCmdCode);						///< BFWコマンドコード(2バイト)を送信バッファに格納
extern int PutData1(BYTE byData);						///< 1バイトデータを送信バッファに格納
extern int PutData2(WORD wData);						///< 2バイトデータを送信バッファに格納
extern int PutData3(DWORD dwData);						///< 3バイトデータを送信バッファに格納
extern int PutData4(DWORD dwData);						///< 4バイトデータを送信バッファに格納
extern int PutDataN(DWORD dwSize, const BYTE* pbyData);	///< nバイトデータを送信バッファに格納
extern int GetData1(BYTE *const pbyData);				///< 1バイトデータを受信バッファから取得
extern int GetData2(WORD *const pwData);				///< 2バイトデータを受信バッファから取得
extern int GetData3(DWORD *const pdwData);				///< 3バイトデータを受信バッファから取得
extern int GetData4(DWORD *const pdwData);				///< 4バイトデータを受信バッファから取得
extern int GetDataN(DWORD dwSize, BYTE *const pbyData);	///< nバイトデータを受信バッファから取得
extern int ReceiveCheck(BOOL* pbArrive);				///< 受信データの有無確認
extern int FlashProtDataForced(void);					///< BFWCmd_SetUSBT送信時の送信バッファ掃き出し
extern void InitComctrlData(void);		///< INITコマンド発行時の通信管理/例外処理用内部変数の初期化

#endif	// __COMCTRL_H__
