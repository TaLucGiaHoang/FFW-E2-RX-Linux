#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <communi.h>
#include <Communierr.h>
#include <usb_e2usb.h>
int main(void)
{
	COM_Open(COM_TYPE_USB_E2USB, NULL, 0);
	char data[0x10000 + 100];
	memset(data, 'u', sizeof(data));
	COM_SendBlock(data, sizeof(data));
	printf("COM_SendBlock sent %ld\n", sizeof(data));
	// COM_Close();


	COM_Open(COM_TYPE_USB_E2USB, NULL, 0);
	char recv_data[1024+152];
	long recv_sz = sizeof(recv_data);
	COM_ReceiveBlockH(recv_data, &recv_sz);
	printf("COM_ReceiveBlockH received %ld\n", recv_sz);
	// COM_Close();

	// COM_Open(COM_TYPE_USB_E2USB, NULL, 0);
	char recv_data2[2];
	long recv_sz2 = sizeof(recv_data2);
	COM_ReceiveBlock(recv_data2, recv_sz2);
	printf("COM_ReceiveBlock received %ld\n", recv_sz2);
	COM_Close();
	return 0;
}

/* int main(void)
{
	COM_Open(0, NULL, 0);
	COM_Close();
	COM_SetTimeOut(0);
	COM_SendBlock(NULL, 0);
	COM_ReceiveBlock(NULL, 0);
	COM_ReceiveBlockH(NULL, NULL);
	COM_ReceiveBlockM(NULL, 0);
	COM_ReceivePolling(NULL);
	COM_Return_data_size(NULL);
	COM_GetLastError();
	COM_Is_Open(NULL);
	COM_Lock();
	COM_UnLock();
	COM_SetTransferSize(0);
	COM_VendorControlIn(0 , 0 , 0 , NULL, 0 );
	COM_VendorControlOut(0 , 0 , 0 , NULL, 0 );
	return 0;
} */