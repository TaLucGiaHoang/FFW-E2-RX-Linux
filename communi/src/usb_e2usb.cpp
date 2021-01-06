////////////////////////////////////////////////////////////////////////////////
/**
 * @file usb_e2usb.cpp
 * @brief Communi.DLL E2usb.sysインターフェース通信処理
 * @author RSD M.Yamamoto
 * @author Copyright (C) 2015 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2015/01/15
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴 (V.3.00.00.000から記載)
・RevRxE2LNo141104-001 2015/02/03 山本
	新規作成
*/

#define __USB_ATTACH__		// setapi.dllからの関数ポインタ取得処理の2重定義を抑制。

#include <string.h>
#include "usb_e2usb.h"
#include "Communierr.h"

#if (0) // Windows
#include "usb_api.h"
#include "usbio_i.h"
#else // Linux
#include <libusb-1.0/libusb.h>
#define VID 0x045b
#define PID 0x82a1

#define TIMEOUT (4);
#endif

// 1度の送信サイズ上限(64KB)
#define	PACKET_SIZE_OF_ONCE	(1024 * 64)	
#if (0) // Windows
E2usb.sys用のGUID対応定義・宣言
#define E2usbDevice_CLASS_GUID \
{ 0x4E95F166, 0x78F1, 0x4c09, 0xA9, 0x3C, 0x0E, 0xF5, 0xAC, 0xB4, 0x5B, 0x4A}
static GUID ClassGuid = USBIO_IID;
#else // Linux
#endif
//==============================================================================
/**
 * CUSB_E2USBクラスのコンストラクタ関数
 * @param  なし
 * @retval なし
 */
//==============================================================================
CUSB_E2USB::CUSB_E2USB()		
	: CIF_OBJ()
{
	transfer_size = PACKET_SIZE_OF_ONCE;	// 64KB (0x10000)
	TimeOut = 4;   // 使用時はmsec単位;
	e2usb_devh = NULL;
	e2usb_ctx = NULL;
	m_bFirstcom = FALSE;

	recv_size = 0;
	recv_point = 0;
	read_size = 0x400;		// 1KB
	e2usb_timeout = 1000;	// fixed transfer size
}


//==============================================================================
/**
 * CUSB_E2USBクラスのデストラクタ関数(仮想)
 * @param  なし
 * @retval なし
 */
//==============================================================================
CUSB_E2USB::~CUSB_E2USB()
{
}

//==============================================================================
/**
 * CUSB_E2USBクラスの通信インターフェースオープン関数
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::Open(long type, char *cparam, long lparam)
{
	////////////////////////////////////
	//
	// cparam, lparam are unused for E2USB
	//
	libusb_hotplug_callback_handle hp[2];
	int product_id, vendor_id, class_id;
	int rc;

	vendor_id  = VID;
	product_id = PID;
	class_id   = LIBUSB_HOTPLUG_MATCH_ANY;

	rc = libusb_init(&e2usb_ctx);
	if (rc < 0)
	{
		printf("Failed to initialize libusb: %s\n", libusb_error_name(rc));
		return FALSE;
	}
	printf("Initialized libusb\n");

	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	int cnt = libusb_get_device_list(e2usb_ctx, &devs); //get the list of devices
	if(cnt < 0) {
		printf("Get Device Error\n");
		return FALSE;
	}
	printf("%d Devices in list.\n", cnt);

	printf("Opening device %04X:%04X...\n", vendor_id, product_id);
	e2usb_devh = libusb_open_device_with_vid_pid(e2usb_ctx, VID, PID); //vendor_id, product_id);
	if (e2usb_devh == NULL) {
		fprintf (stderr, "Error opening device\n");
		return FALSE;
	}


	libusb_device *dev;
	uint8_t bus, port_path[8];

	dev = libusb_get_device(e2usb_devh);

	bus = libusb_get_bus_number(dev);
	rc = libusb_get_port_numbers(dev, port_path, sizeof(port_path));
	if (rc > 0) {
		printf("\nDevice properties:\n");
		printf("        bus number: %d\n", bus);
		printf("         port path: %d", port_path[0]);
		for (int i=1; i<rc; i++) {
			printf("->%d", port_path[i]);
		}
		printf(" (from root hub)\n");
	}

	struct libusb_device_descriptor dev_desc;

	printf("\nReading device descriptor:\n");
	libusb_get_device_descriptor(dev, &dev_desc);
	printf("            length: %d\n", dev_desc.bLength);
	printf("      device class: %d\n", dev_desc.bDeviceClass);
	printf("               S/N: %d\n", dev_desc.iSerialNumber);
	printf("           VID:PID: %04X:%04X\n", dev_desc.idVendor, dev_desc.idProduct);
	printf("         bcdDevice: %04X\n", dev_desc.bcdDevice);
	printf("   iMan:iProd:iSer: %d:%d:%d\n", dev_desc.iManufacturer, dev_desc.iProduct, dev_desc.iSerialNumber);
	printf("          nb confs: %d\n", dev_desc.bNumConfigurations);


	struct libusb_config_descriptor *conf_desc;
	int iface, nb_ifaces;
	printf("\nReading first configuration descriptor:\n");
	libusb_get_config_descriptor(dev, 0, &conf_desc);
	nb_ifaces = conf_desc->bNumInterfaces;
	printf("     nb interfaces: %d\n", nb_ifaces);
	// ...
	libusb_free_config_descriptor(conf_desc);


	/* Claim interfaces */
	libusb_set_auto_detach_kernel_driver(e2usb_devh, 1);
	for (iface = 0; iface < nb_ifaces; iface++)
	{
		printf("\nClaiming interface %d...\n", iface);
		rc = libusb_claim_interface(e2usb_devh, iface);
		// rc = libusb_claim_interface(e2usb_devh, 0); // Claim the first interface
		if (rc != LIBUSB_SUCCESS) {
			libusb_close(e2usb_devh);
			e2usb_devh = NULL;
			fprintf (stderr, "libusb_claim_interface failed: %s\n", libusb_error_name(rc));
		}
	}
	printf("Claimed interface\n");

	// タイムアウト値の設定に失敗した場合(あり得ない)
	if(SetTimeOut(TimeOut) != TRUE){	
		goto Communi_Error;
	}
	
	// 
	com_type = type;
	recv_size = 0;
	recv_point = 0;
	read_size = 0x400;		// デフォルト1Kバイト(Win2K+Full-Speedの場合0x1000以下でないと通信エラーとなるため)

	return TRUE;

Communi_Error:
	Close();			//　Close関数に全クリア機能があるので、それを呼ぶ。
	SetErr(ER_COM_OPEN);
	return FALSE;

}

//==============================================================================
/**
 * CUSB_E2USBクラスの通信インターフェースクローズ関数
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::Close(void)
{
	int rc;

	printf ("Device detached\n");

	if(e2usb_devh)
	{
		rc = libusb_release_interface(e2usb_devh, 0); //release the claimed interface
		if(rc != 0) {
			fprintf(stderr, "Cannot Release Interface\n");
			return FALSE;
		}
		printf("Released Interface\n");
	}
	
	if(e2usb_devh) {
		printf("Close usb device\n");
		libusb_close(e2usb_devh);
		e2usb_devh = NULL;
	}

	printf("Exit libusb\n");
	libusb_exit(e2usb_ctx);
	
	return TRUE;
}



//==============================================================================
/**
 * CUSB_E2USBクラスの通信の送信関数
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::SendBlock(char *buf, long size)
{
	unsigned char endpoint_out = LIBUSB_ENDPOINT_OUT | 0x02;
	unsigned int timeout = e2usb_timeout;
	
	int transfer_size_max = transfer_size;	// PACKET_SIZE_OF_ONCE 64KB (0x10000)

	int ReturnedLength = 0;
	long DataCnt = size;
	int	SendSize;

	ULONG	result;

	DataCnt = size;
	while(DataCnt > 0){
		if(DataCnt > (ULONG)PACKET_SIZE_OF_ONCE){
			SendSize = PACKET_SIZE_OF_ONCE;
		}else{
			SendSize = DataCnt;
		}
		
		if(e2usb_devh == NULL) {
			// perror("Error device handle is NULL\n");
		}
			

		result = libusb_bulk_transfer(e2usb_devh, endpoint_out, (unsigned char*)buf, SendSize, &ReturnedLength, e2usb_timeout);
		if (result == 0 && ReturnedLength == SendSize) {
			;
		} else {
			// perror("Error libusb_bulk_transfer");
			return FALSE;
		}
		// libusb_bulk_transferに成功した(0が返った)場合はそのまま比較処理してよい。
		// 実際に送信したサイズを確認
		if ( ReturnedLength > SendSize ){
			// 通信キャンセル(終わっている想定だが、念のため)
			// ComCancel();
			SetErr(ER_COM_SEND);
			// perror("Error libusb_bulk_transfer");
			return FALSE;
		}
	
		DataCnt -= SendSize;
		buf += SendSize;
	}
	recv_size = 0;
	recv_point = 0;
	m_bFirstcom = TRUE;

	return TRUE;
}

//==============================================================================
/**
 * CUSB_E2USBクラスの通信インターフェースの受信関数
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::ReceiveBlock(char *buf, long size)
{
	int	ReturnedLength = 0;
	int	i;
	int	result;
	unsigned char endpoint_in = LIBUSB_ENDPOINT_IN | 0x81;
	unsigned int timeout = e2usb_timeout;

	if(!buf)
	{
		// perror("Error null buffer input");
		return FALSE;
	}
retry:
	// データ受信処理(非同期の場合、即座に制御を返すため、0を返す)
	if(recv_size == 0){
		result = libusb_bulk_transfer(e2usb_devh, endpoint_in, (unsigned char*)recv_data, GET_SIZE, &ReturnedLength, timeout);
		if (result == 0){
			;
		} else {
			// 通信キャンセル
			// ComCancel();
			SetErr(ER_COM_RECV);
			// perror("Error libusb_bulk_transfer");
			return FALSE;
		}
		
		// libusb_bulk_transferに成功した(0が返った)場合はそのまま比較処理してよい。
		if((recv_size = (int)ReturnedLength) == 0){
			// 0バイトパケットを受信した場合、リトライする。エラーにはしない
			// リトライしてよいのは0バイトパケットを受信した場合のみである。
			goto retry;
		}
		recv_point = 0;
	}

	for(i = 0; i < size && 0 < recv_size; i++){
		buf[i] = recv_data[recv_point++];
		recv_size--;
	}
	p_size = i;
	if(p_size != size){
		SetErr(ER_COM_DIFFERENT_SIZE);
		return FALSE;
	}
	else{
		
		m_bFirstcom = TRUE;
		return TRUE;
	}
}

//==============================================================================
/**
 * CUSB_E2USBクラスの通信インターフェースの受信関数(大容量転送版)
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::ReceiveBlockH(char *buf, long *size)
{
	int	ReturnedLength = 0;
	int	result;
	unsigned char endpoint_in = LIBUSB_ENDPOINT_IN | 0x81;
	unsigned int timeout = e2usb_timeout;
	
	if(!buf)
	{
		// perror("Error null buffer input");
		return FALSE;
	}

retry:
	// データ受信処理(非同期の場合、即座に制御を返すため、0(失敗)を返す)
	result = libusb_bulk_transfer(e2usb_devh, endpoint_in, (unsigned char*)buf, read_size, &ReturnedLength, timeout);
	// ReturnedLength = *size;result=1;	// test without libusb_bulk_transfer
	if (result == 0) {
		;
	} else {
		// 通信キャンセル
		// ComCancel();
		SetErr(ER_COM_RECV);
		// perror("Error libusb_bulk_transfer");
		return FALSE;
	}
	// libusb_bulk_transferに成功した(0が返った)場合はそのまま比較処理してよい。
	if((*size = (int)ReturnedLength) == 0){
		// 0バイトパケットを受信した場合、リトライする。エラーにはしない
		// リトライしてよいのは0バイトパケットを受信した場合のみである。
		goto retry;
	}

	m_bFirstcom = TRUE;

	return TRUE;
}


//==============================================================================
/**
 * CUSB_E2USBクラスの通信インターフェースの受信関数(Masopi版)
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::ReceiveBlockM(char *buf, long size)
{
	// Unsed for E2USB
	return TRUE;
}


//==============================================================================
/**
 * CUSB_E2USBクラスの通信インターフェースの受信データ有無確認(未サポート)
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::ReceivePolling(long *arrive)
{
	// E2usb.sysは未サポートの関数。TRUEを返す。
	*arrive;

	// Unsed for E2USB
	return TRUE;
}

//==============================================================================
/**
 * CUSB_E2USBクラスのタイムアウト受信処理
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::SetTimeOut(long timeout)
{
	////////////////////////
	timeout = TIMEOUT;	// Set fixed value
	////////////////////////
	// 非同期なので、INFINITE(0xFFFFFFFF)は禁止
	if (timeout == INFINITE) {
		return FALSE;
	} else {
		TimeOut = timeout; // 設定はsec、使用はmsec */
	}

	return TRUE;
}

//==============================================================================
/**
 * CUSB_E2USB
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::Lock(void)
{
	// Reserved
	return TRUE;
}

//==============================================================================
/**
 * CUSB_E2USB
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::UnLock(void)
{
	// Reserved
	return TRUE;
}

//==============================================================================
/**
 * CUSB_E2USBクラスのUSB転送サイズの設定処理
 * @param  なし
 * @retval なし
 */
//==============================================================================
int CUSB_E2USB::SetTransferSize(long size)
{
	transfer_size = PACKET_SIZE_OF_ONCE;	// Set fixed value

	if (m_bFirstcom) {
		read_size = size;		// リードサイズは保持する。
		return TRUE;			// 正常終了
	}

	read_size = size;

	return TRUE;
}


//==============================================================================
/**
 * CUSB_E2USBクラスのF/Wへのオープン通知処理(未サポート)
 * @param  なし
 * @retval TRUEのみ。
 */
//==============================================================================
UINT CUSB_E2USB::Info_DriverOpen( void )	// F/Wにドライバをオープンしたことを通知する	
{
	return TRUE;
}


//==============================================================================
/**
 * CUSB_E2USBクラスのF/Wへのクローズ通知処理(未サポート)
 * @param  なし
 * @retval TRUEのみ。
 */
//==============================================================================
UINT CUSB_E2USB::Info_DriverClose( void )	// F/Wにドライバをクローズしたことを通知する	
{
	return TRUE;
}


//==============================================================================
/**
 * CUSB_E2USBクラスの1msecソフトウエイト処理(未使用)
 * @param  なし
 * @retval TRUEのみ。
 */
//==============================================================================
void CUSB_E2USB::wait_1s(void)
{
	clock_t dwTime1, dwTime2;
	dwTime1 = clock();	// 現在時間取得 */
	while (1) {
		dwTime2 = clock();	// 現在時間取得 */
		if (dwTime2 < dwTime1) {	// カウンターが１周してしまった場合再設定 */
			dwTime1 = dwTime2;
		}
		if (dwTime2 >= dwTime1 + CLOCKS_PER_SEC) {
			break;
		}
	}
}


//==============================================================================
/**
 * CUSB_E2USBクラスの通信キャンセル処理
 * @param  なし
 * @retval TRUEのみ。
 */
//==============================================================================
void CUSB_E2USB::ComCancel(void)
{
    // DWORD dwReturn;

    // // まず送信をキャンセルする
	// DeviceIoControl(hUsbSend,
	// 				IOCTL_USBIO_ABORT_PIPE,
	// 				NULL,
	// 				0,
	// 				NULL,
	// 				0,
	// 				&dwReturn,
	// 				NULL);

    // // 次に受信をキャンセルする
	// DeviceIoControl(hUsbRecv,
	// 				IOCTL_USBIO_ABORT_PIPE,
	// 				NULL,
	// 				0,
	// 				NULL,
	// 				0,
	// 				&dwReturn,
	// 				NULL);

    return;
}

//==============================================================================
/**
 * CUSB_E2USBクラスのベンダーコントロールイン
 * @param  char コントロール転送のbRequest
 * @param  unsigned short  コントロール転送のwVal
 * @param  unsigned short  コントロール転送のwIndex
 * @param  char* 入力領域
 * @param  int  入力領域サイズ
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int CUSB_E2USB::VendorControlIn(char req, unsigned short val, unsigned short index, char *in, int len)
{
	// Not implemented
	return TRUE;
}

//==============================================================================
/**
 * CUSB_E2USBクラスのベンダーコントロールアウト
 * @param  char コントロール転送のbRequest
 * @param  unsigned short  コントロール転送のwVal
 * @param  unsigned short  コントロール転送のwIndex
 * @param  char* 出力領域
 * @param  int  出力領域サイズ
 * @retval TRUE:正常終了、FALSE:異常終了
 */
//==============================================================================
int CUSB_E2USB::VendorControlOut(char req, unsigned short val, unsigned short index, const char *out, int len)
{
	// Not implemented
	return TRUE;
}
