#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <communi.h>
#include <Communierr.h>
#include <usb_e2usb.h>

#include "ffw_sys.h"
#include "FFWE2.h"
#include "ffw_closed.h"
// #include "addition.h"
static void printEinf(FFWE20_EINF_DATA *Einf)
{
	printf("Emulator Information:\n");
	printf("	wEmuStatus              = %d\n", Einf->wEmuStatus);
	printf("	wBoardRev               = %d\n", Einf->wBoardRev);
	printf("	eEmlPwrSupply           = %d\n", Einf->eEmlPwrSupply);
	printf("	wVbusVal                = %d\n", Einf->wVbusVal);
	printf("	flNowVbus               = %f\n", Einf->flNowVbus);
	printf("	wTagetCable             = %d\n", Einf->wTagetCable);
	printf("	eStatUIF                = %d\n", Einf->eStatUIF);
	printf("	eStatUCON               = %d\n", Einf->eStatUCON);
	printf("	wUvccVal                = %d\n", Einf->wUvccVal);
	printf("	flNowUVCC               = %f\n", Einf->flNowUVCC);
	printf("	eStatUVCC               = %d\n", Einf->eStatUVCC);
	printf("	byBfwMode               = %d\n", Einf->byBfwMode);
	printf("	dwBfwLV0Ver             = %d\n", Einf->dwBfwLV0Ver);
	printf("	dwBfwEMLVer             = %d\n", Einf->dwBfwEMLVer);
	printf("	wEmlTargetCpu           = %d\n", Einf->wEmlTargetCpu);
	printf("	wEmlTargetCpuSub        = %d\n", Einf->wEmlTargetCpuSub);
	printf("	cEmlTgtMucId            = %s\n", Einf->cEmlTgtMucId);
	printf("	dwEmlTargetLv0Ver       = %d\n", Einf->dwEmlTargetLv0Ver);
	printf("	dwBfwFdtVer             = %d\n", Einf->dwBfwFdtVer);
	printf("	cFdtTgtMucId            = %s\n", Einf->cFdtTgtMucId);
	printf("	dwFdtTargetLv0Ver       = %d\n", Einf->dwFdtTargetLv0Ver);
	printf("	byFpga1Status           = %d\n", Einf->byFpga1Status);
	printf("	byFpga1DbgIf            = %d\n", Einf->byFpga1DbgIf);
	printf("	eDebugIF                = %d\n", Einf->eDebugIF);
	printf("	byFpga1Rev              = %d\n", Einf->byFpga1Rev);
	printf("	byFpga2DbgIf            = %d\n", Einf->byFpga2DbgIf);
	printf("	eMCUType                = %d\n", Einf->eMCUType);
	printf("	byFpga2Rev              = %d\n", Einf->byFpga2Rev);
	printf("	byFpgaVer               = %d\n", Einf->byFpgaVer);
	printf("	byTraceFlg              = %d\n", Einf->byTraceFlg);
	printf(" \n");
}

void TC_01(void)
{
	DWORD pdwGageCount = 0;
	FFW_WINVER_DATA pWinVer;

	FFWE20Cmd_INIT((void*)COM_SendBlock, (void*)COM_ReceiveBlock, (void*)COM_ReceiveBlockH, (void*)COM_ReceivePolling, (void*)COM_SetTimeOut, (void*)COM_Lock, (void*)COM_UnLock, (void*)COM_SetTransferSize, &pdwGageCount, &pWinVer, NULL, NULL);

	FFWE20_EINF_DATA Einf;
	FFWE20Cmd_GetEINF(&Einf);
	printf("\n");
	
	printEinf(&Einf);
}

void TC_02(void)
{
{
	DWORD dweadrStartAddr = 0x00003000;
	enum FFWENM_EACCESS_SIZE eAccessSize = EBYTE_ACCESS;
	BYTE pbyWriteBuff[] = "12345678";
	DWORD dwWriteCount = 8;
	DWORD dwReadCount = dwWriteCount;
	BYTE pbyReadBuff[100];

	BYTE dump[8+8+8+16];
	memset(dump, 'f', sizeof(dump));
	printf("Clear CPU: [0x%.8x:0x%.8x]\n" , dweadrStartAddr, dweadrStartAddr+(8+8+8+16));
	FFWE20Cmd_CPUW(dweadrStartAddr, eAccessSize, (8+8+8+16), dump);
	printf("\n");

	memset(pbyReadBuff, 0, sizeof(pbyReadBuff));
	FFWE20Cmd_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);
	printf("Read %d old bytes from CPU: [0x%.8x] ==> \"%s\"\n", dwReadCount, dweadrStartAddr, pbyReadBuff);
	printf("\n");

	printf("Write %d new bytes to CPU: [0x%.8x] <== \"%s\"\n" , dwWriteCount, dweadrStartAddr, pbyWriteBuff);
	FFWE20Cmd_CPUW(dweadrStartAddr, eAccessSize, dwWriteCount, pbyWriteBuff);
	printf("\n");

	memset(pbyReadBuff, 0, sizeof(pbyReadBuff));
	FFWE20Cmd_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);
	printf("Read %d new bytes from CPU: [0x%.8x] ==> \"%s\"\n", dwReadCount, dweadrStartAddr, pbyReadBuff);
	printf("\n");
}

{
	DWORD dweadrStartAddr = 0x00003000+8;
	enum FFWENM_EACCESS_SIZE eAccessSize = EWORD_ACCESS;
	BYTE pbyWriteBuff[] = "12345678";
	DWORD dwWriteCount = 4;
	DWORD dwReadCount = dwWriteCount;
	BYTE pbyReadBuff[100];
	
	memset(pbyReadBuff, 0, sizeof(pbyReadBuff));
	FFWE20Cmd_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);
	printf("Read %d old Words from CPU: [0x%.8x] ==> \"%s\"\n", dwReadCount, dweadrStartAddr, pbyReadBuff);
	printf("\n");

	printf("Write %d new Words to CPU: [0x%.8x] <== \"%s\"\n" , dwWriteCount, dweadrStartAddr, pbyWriteBuff);
	FFWE20Cmd_CPUW(dweadrStartAddr, eAccessSize, dwWriteCount, pbyWriteBuff);
	printf("\n");

	memset(pbyReadBuff, 0, sizeof(pbyReadBuff));
	FFWE20Cmd_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);
	printf("Read %d new Words from CPU: [0x%.8x] ==> \"%s\"\n", dwReadCount, dweadrStartAddr, pbyReadBuff);
	printf("\n");
}

{
	DWORD dweadrStartAddr = 0x00003000+16;
	enum FFWENM_EACCESS_SIZE eAccessSize = ELWORD_ACCESS;
	BYTE pbyWriteBuff[] = "12345678";
	DWORD dwWriteCount = 2;
	DWORD dwReadCount = dwWriteCount;
	BYTE pbyReadBuff[100];
	
	memset(pbyReadBuff, 0, sizeof(pbyReadBuff));
	FFWE20Cmd_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);
	printf("Read %d old Long Words from CPU: [0x%.8x] ==> \"%s\"\n", dwReadCount, dweadrStartAddr, pbyReadBuff);
	printf("\n");

	printf("Write %d new Long Words to CPU: [0x%.8x] <== \"%s\"\n" , dwWriteCount, dweadrStartAddr, pbyWriteBuff);
	FFWE20Cmd_CPUW(dweadrStartAddr, eAccessSize, dwWriteCount, pbyWriteBuff);
	printf("\n");

	memset(pbyReadBuff, 0, sizeof(pbyReadBuff));
	FFWE20Cmd_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);
	printf("Read %d new Long Words from CPU: [0x%.8x] ==> \"%s\"\n", dwReadCount, dweadrStartAddr, pbyReadBuff);
	printf("\n");
}

{
	DWORD dweadrStartAddr = 0x00003000+24;
	enum FFWENM_EACCESS_SIZE eAccessSize = EBYTE_ACCESS;
	BYTE pbyWriteBuff[] = "0123456789ABCDEF";//"Hello World !!!";
	DWORD dwWriteCount = 16;//sizeof(pbyWriteBuff);
	DWORD dwReadCount = dwWriteCount;
	BYTE pbyReadBuff[100];

	memset(pbyReadBuff, 0, sizeof(pbyReadBuff));
	FFWE20Cmd_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);
	printf("Read %d old bytes from CPU: [0x%.8x] ==> \"%s\"\n", dwReadCount, dweadrStartAddr, pbyReadBuff);
	printf("\n");

	printf("Write %d new bytes to CPU: [0x%.8x] <== \"%s\"\n" , dwWriteCount, dweadrStartAddr, pbyWriteBuff);
	FFWE20Cmd_CPUW(dweadrStartAddr, eAccessSize, dwWriteCount, pbyWriteBuff);
	printf("\n");

	memset(pbyReadBuff, 0, sizeof(pbyReadBuff));
	FFWE20Cmd_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);
	printf("Read %d new bytes from CPU: [0x%.8x] ==> \"%s\"\n", dwReadCount, dweadrStartAddr, pbyReadBuff);
	printf("\n");
}
}

void TC_03(void);
void TC_04(void);
void TC_05(void);


int main(void)
{
	COM_Open(COM_TYPE_USB_E2USB, NULL, 0);
	COM_ReceiveBlock(NULL, 0);

	TC_01();
	TC_02();


	COM_Close();
}

