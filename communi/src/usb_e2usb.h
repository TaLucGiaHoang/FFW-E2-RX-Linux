///////////////////////////////////////////////////////////////////////////////
/**
 * @file  usb_e2usb.h
 * @brief Communi.dll Header File for "E2usb.sys" USB Driver
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
#ifndef	__USB_E2USB_H__
#define	__USB_E2USB_H__

#include	"if_obj.h"
#if (0)	// Windows
#include	<afxmt.h>		// for CSemaphore & CSingleLock
#include	<winioctl.h>
#else // Linux
	#include <libusb-1.0/libusb.h> // LINUX
#endif
#include	"communi.h"

#define	GET_SIZE		(0x400)	// ReceiveBlock用の受信サイズ(ReceiveBlockは未使用なので、未使用)
//#define	GET_SIZE_MAX	(0x20000) Delete From E1usb.sys

//-----------------------------------------------------------------------------
/// Communi.DLL E2usb.sys Class
//-----------------------------------------------------------------------------
class CUSB_E2USB : public CIF_OBJ
{
public:
			CUSB_E2USB();		// Constructor
	virtual	~CUSB_E2USB();		// Virtual Destructor

	// Member Method (Public)
	int		Open(long type, char *cparam, long lparam);	// ドライバオープン
	int		Close(void);								// ドライバクローズ 
	int		SendBlock(char *buf, long size);			// データ送信
	int		ReceiveBlock(char *buf, long size);			// データ受信(未使用)
	int		ReceiveBlockH(char *buf, long *size);		// データ受信(大容量)
	int		ReceiveBlockM(char *buf, long size);		// データ受信(Masopi用)
	int		ReceivePolling(long *arrive);				// 受信データの有無確認(非サポート)
	int		SetTimeOut(long timeout);					// 通信タイムアウトの設定
	int		SetTransferSize(long size);					// 転送バイト数の設定
	int		Lock(void);
	int 	UnLock(void);

	int		VendorControlIn(char req, unsigned short val, unsigned short index, char *in, int len);
	int		VendorControlOut(char req, unsigned short val, unsigned short index, const char *out, int len);

private:
	// Member Method (Private)
	UINT	Info_DriverOpen(void);	// F/Wにドライバをオープンしたことを通知する(未使用)	
	UINT	Info_DriverClose(void);	// F/Wにドライバをクローズしたことを通知する(未使用)	
	void	wait_1s(void);			// 1sec wait
	void	ComCancel(void);		// デバイスドライバへ、通信キャンセルを通知(E2usb追加)

	// Member Variable (Private)
#if (0)	// Windows
	HANDLE hDevice;					// ドライバ本体のハンドル
	HANDLE hUsbSend;				// ドライバの送信パイプ用のハンドル(E2usb追加)
	HANDLE hUsbRecv;				// ドライバの受信パイプ用のハンドル(E2usb追加)
	HANDLE hUsbSendDone;			// 送信完了イベントハンドル取得(非同期用、E2usb追加)
	HANDLE hUsbRecvDone;			// 受信完了イベントハンドル取得(非同期用、E2usb追加)
	OVERLAPPED hUsbSendAsync;		// 送信用OVERLAPPED構造体 (非同期用、E2usb追加)
	OVERLAPPED hUsbRecvAsync;		// 受信用OVERLAPPED構造体 (非同期用E2usb追加)
#endif
	int		open_close;				// ドライバのOpen/CloseをF/Wへ通知するかの管理フラグ(未使用)
	int		recv_size;				// 受信データサイズ
	int		recv_point;				// 受信データのオフセット
	char	recv_data[GET_SIZE];	// 受信データバッファ
	int		read_size;				// ReceiveBlockH専用の受信データサイズ
	long	transfer_size;			// Transfer size
	unsigned int e2usb_timeout;			// Transfer timeout
#if (0)	// Windows
	CSemaphore	*pSemaphore;		// Open/Close排他制御用セマフォ
	CSingleLock	*pLock;				// Open/Close排他制御用シングルロック
#else // Linux
	libusb_device_handle *e2usb_devh;
	libusb_context *e2usb_ctx;
#endif
	BOOL m_bFirstcom;				// 接続(Configuration)有無を示すフラグ
};

#endif	//__USB_E2USB_H__
