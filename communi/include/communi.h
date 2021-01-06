///////////////////////////////////////////////////////////////////////////////
/**
 * @file  Communi.h
 * @brief Communi.dll Header File
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
#ifndef __COMMUNI_H__
#define __COMMUNI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USELOADLIBRARY
int		COM_Open(long, char *, long);
int		COM_Close(void);
int		COM_SetTimeOut(long);
int		COM_SendBlock(char *, long);
int		COM_ReceiveBlock(char *, long);
int		COM_ReceiveBlockH(char *, long *);
int		COM_ReceiveBlockM(char *, long);
int		COM_ReceivePolling(long *);
int		COM_Return_data_size(int *);
int		COM_GetLastError(void);
int		COM_Is_Open(int *);
int		COM_Lock(void);
int		COM_UnLock(void);
int		COM_SetTransferSize(long);
int 	COM_VendorControlIn(char , unsigned short , unsigned short , char *, int);
int 	COM_VendorControlOut(char , unsigned short , unsigned short , char *, int);
#endif

#define BUFFER_SIZE		4096	/* 送・受信バッファサイズ */

enum _com_type{
	COM_TYPE_RS232C = 1,		// モニタシリアル

	COM_TYPE_LAN_UDP,			// 非サポート(V3.00削除)
	COM_TYPE_LAN_TCP,			// 非サポート(V3.00削除)
	COM_TYPE_PARA98,			// 非サポート(V3.00削除)
	COM_TYPE_PARAPC,			// 非サポート(V3.00削除)
	COM_TYPE_EPP,				// 非サポート(V3.00削除)
	COM_TYPE_ECP,				// 非サポート(V3.00削除)
	COM_TYPE_BYTE,				// 非サポート(V3.00削除)
	COM_TYPE_NIBBLE,			// 非サポート(V3.00削除)
	COM_TYPE_EPP_AUTO,			// 非サポート(V3.00削除)
	COM_TYPE_USB,				// 非サポート(V3.00削除)
	COM_TYPE_EPP_DC,			// 非サポート(V3.00削除)
	COM_TYPE_ECP_DC,			// 非サポート(V3.00削除)
	COM_TYPE_BYTE_DC,			// 非サポート(V3.00削除)
	COM_TYPE_NIBBLE_DC,			// 非サポート(V3.00削除)
	COM_TYPE_SIMXX,				// 非サポート(V3.00削除)
	COM_TYPE_EPP_D10V,			// 非サポート(V3.00削除)
	COM_TYPE_ECP_D10V,			// 非サポート(V3.00削除)
	COM_TYPE_BYTE_D10V,			// 非サポート(V3.00削除)
	COM_TYPE_NIBBLE_D10V,		// 非サポート(V3.00削除)
	COM_TYPE_EPP_AUTO_D10V,		// 非サポート(V3.00削除)
	COM_TYPE_USB_D10V,			// 非サポート(V3.00削除)

	COM_TYPE_USB_E1USB,			// E1usb.sys(E1/E20/E100/E30Aなど)
	COM_TYPE_USB_E2USB,			// E2usb.sys(E2/E2 Lite対応thesycon社製USBドライバ(USBIO))
	COM_TYPE_USB_VCOM_EZCUBE,	// EZ-CUBE専用仮想COM(MQ2SALL.sys, MQB2SVCP.sys)

};

#define	COM_PORT_DC_0	((char *)0)	// 未使用(V3.00～)
#define	COM_PORT_DC_1	((char *)1)	// 未使用(V3.00～)
#define	COM_PORT_DC_2	((char *)2)	// 未使用(V3.00～)
#define	COM_PORT_DC_3	((char *)3)	// 未使用(V3.00～)

// ボーレート
#define		COM_B115200	11520
#define		COM_B57600	5760
#define 	COM_B38400	3840
#define 	COM_B19200	1920
#define 	COM_B9600	960
#define 	COM_B4800	480
#define 	COM_B2400	240
#define 	COM_B1200	120

#define		COM_NONE	0	/* not arrived */
#define		COM_EXIST	1	/* arrived */

#ifdef __cplusplus
}				/* extern "C" */

#endif

#endif	// __COMMUNI_H__