///////////////////////////////////////////////////////////////////////////////
/**
 * @file  communierr.h
 * @brief Communi.dll Error Header File
 * @author M.Yamamoto(EM2)
 * @author Copyright(C) 2015 Renesas Electronics Corporation
 * @author and Renesas System Design Corporation All rights reserved.
 * @date 2015/01/15
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴 (V.3.00.00.000から記載)
・RevRxE2LNo141104-001 2015/01/15 山本
	新規作成
*/
#ifndef COMMUNIERR_H
#define COMMUNIERR_H

#define	COM_ER_BASE				(16000)
#define	ER_COM_EXIST			(COM_ER_BASE+0)		// 既にターゲットに接続済みである
#define	ER_COM_FORK				(COM_ER_BASE+1)		// fork エラーが発生
#define	ER_COM_HOSTNAME			(COM_ER_BASE+2)		// 指定したホスト名が見つからない
#define	ER_COM_ILLEGAL_MODE		(COM_ER_BASE+3)		// ボーレートの指定に誤りがある
#define	ER_COM_NONE				(COM_ER_BASE+4)		// ターゲットと接続されていない
#define	ER_COM_OPEN				(COM_ER_BASE+5)		// ターゲットに接続できない
#define	ER_COM_OUTRANGE			(COM_ER_BASE+6)		// タイムアウト時間の設定が範囲外。
#define	ER_COM_TIMEOUT			(COM_ER_BASE+7)		// タイムアウトエラーが発生
#define	ER_COM_CLOSE			(COM_ER_BASE+8)		// ターゲットの通信切断に失敗
#define	ER_COM_DIFFERENT_SIZE	(COM_ER_BASE+9)		// 指定したデータサイズの転送が行われなかった
/***** for xdb38_LAN ********/
#define	ER_COM_PARA				(COM_ER_BASE+10)	// パラメータの設定に誤りがある。
#define	ER_COM_NAME				(COM_ER_BASE+11)	// ホスト名の指定に誤りがある。
#define	ER_COM_CUT				(COM_ER_BASE+12)	// 通信エラー。ターゲットから切断済み。
#define	ER_COM_SEND				(COM_ER_BASE+13)	// 通信エラー。ターゲットにデータを転送できない
#define	ER_COM_RECV				(COM_ER_BASE+14)	// 通信エラー。ターゲットよりデータを受信できない
#define	ER_COM_ALREADYUSED		(COM_ER_BASE+15)	// 既にターゲットは使用されている。
#define	ER_COM_UNSUPPORT_IF		(COM_ER_BASE+16)	// 非サポートのWindowsOSのため、サポートされていない。
#define	ER_COM_UNSUPPORT_LAN	(COM_ER_BASE+17)	// Windows3.1上では、LAN I/Fは使用できない
#define	ER_COM_UNSUPPORT_PARA	(COM_ER_BASE+18)	// Windows NTでは、パラレル通信I/Fを使用できない
#define	ER_COM_ILLEGAL_SET		(COM_ER_BASE+19)	// 通信I/Fの設定内容が不適切
#define	ER_COM_OVERRUN			(COM_ER_BASE+20)	// シリアル通信でオーバーランエラーが発生
// for E2usb.sys
#define	ER_COM_NOTFOUND_DEVICE	(COM_ER_BASE+21)	// デバイスが見つからない(E2usb.sys専用)


#endif
