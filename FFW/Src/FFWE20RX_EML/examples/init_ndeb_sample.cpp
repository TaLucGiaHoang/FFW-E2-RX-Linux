#include <vector>
#include <communi.h>

#include "ffw_sys.h"
#include "ffw_closed.h"

#define Print(x) printf("%s\n", x )
#define PrintErr_E20RX(x) printf("[%s] Err: 0x%.8x , line %d in file %s\n", __func__, x, __LINE__, __FILE__);
#define PrintErr(x) printf("Error: 0x%08x", x );
// [NDEB Source:]
//////////////////////////
// prot_core.h Start
// [ バッファサイズ ]
#define	BUFF_SIZE	(1024)
// prot_core.h End
//////////////////////////

///////////////////////////////////////////////////////////////
// ;===================================================================
// ; Transition to EML mode
// ;===================================================================
// trans EML
int	DO_TRANS(char *Lv)
{
	int	ret;
	_strupr(Lv);
	printf("[%s] Lv %s\n", __func__, Lv);
	if( Strcmp( Lv,"0" ) == 0 ){
		if((ret = FFWE20Cmd_TRANS_LV0()) != FFWERR_OK){
			PrintErr_E20RX(ret);
			return FALSE;
		}
	}
	else if( Strcmp( Lv,"EML" ) == 0 ){
		if((ret = FFWE20Cmd_TRANS_EML()) != FFWERR_OK){
			PrintErr_E20RX(ret);
			printf("[%s] Returned FALSE\n", __func__);
			printf("BFW EML mode already set\n");
			return FALSE;
		}
	}
	else if( Strcmp( Lv,"FDT" ) == 0 ){
		if((ret = FFWE20Cmd_TRANS_FDT()) != FFWERR_OK){
			PrintErr_E20RX(ret);
			return FALSE;
		}
	}
	else{
		Print( "BFWの遷移先の指定が誤っています。" );
		return FALSE;
	}
	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}
//////////////////////////////////////////////////////////////////////
// cmd_file.cpp start
static int		s_FPGA_READY = FALSE;
static int		s_FpgaType = -1;
unsigned char	hwcfData[0x300000];
static int load_hwcf_rx600(FILE *fp);
static int do_dcnf_rx600(char *filename);

static	BOOL	s_bDCNF_MaskOnOff = FALSE;
//////////////////////////////////////////////////////////////////////
// ;===================================================================
// ;Issue the following when supplying power from the emulator
// ;===================================================================
// ;extpwr ena,3.3v
static int	DO_EXTPWR(char *ena, char* val)
{
	enum FFWENM_EXPWR_ENABLE eExPwrEnable;
	enum FFWENM_EXPWR_VAL eExPwrVal;
	int	ret;

	_strupr(ena);
	printf("[%s] ena %s\n", __func__, ena);
	if ((Strcmp(ena, "ENA") == 0) || (Strcmp(ena, "ENABLE") == 0) || (Strcmp(ena, "ON") == 0)) {
		eExPwrEnable = EXPWR_ENABLE;
		if (*val == '\0') {
			Print("Invalid volt value");
			return FALSE;
		}
		_strupr(val);
		printf("[%s] val %s\n", __func__, val);
		if ((Strcmp(val, "1.8") == 0) || (Strcmp(val, "1.8V") == 0)) {
			eExPwrVal = EXPWR_V180;
		}else if ((Strcmp(val, "3.3") == 0) || (Strcmp(val, "3.3V") == 0)) {
			eExPwrVal = EXPWR_V330;
		}else if ((Strcmp(val, "5.0") == 0) || (Strcmp(val, "5.0V") == 0) || (Strcmp(val, "5") == 0)) {
			eExPwrVal = EXPWR_V500;
		}else{
			Print("Invalid volt value");
			return FALSE;
		}
	} else if ((Strcmp(ena, "DIS") == 0) || (Strcmp(ena, "DIS") == 0) || (Strcmp(ena, "OFF") == 0)) {
		eExPwrEnable = EXPWR_DISABLE;
		eExPwrVal = EXPWR_NON;
	} else {
		Print("Invalid enable value");
		return FALSE;
	}
	if((ret = FFWE20Cmd_EXTPWR(eExPwrEnable, eExPwrVal)) != FFWERR_OK){
		PrintErr_E20RX(ret);
		if(ret == FFWERR_BTARGET_POWERSUPPLY)
		{
			printf("[%s] Power is already supplied\n", __func__);
		}
		return FALSE;
	}
	
	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}
//////////////////////////////////////////////////////////////////////
static int DO_DCNF(char *filename) {

	// if ((PROT_EMU_TYPE() == EMU_E20) || (PROT_EMU_TYPE() == EMU_E2)) {
		return do_dcnf_rx600(filename);
	// } else {
	// 	printf("Error: This command is for the E20 emulator.");
	// 	return(FALSE);
	// }
	// return(FALSE);
}


static int do_dcnf_rx600(char *filename)
{
	char	tmp[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT];
	int		ret;

	// : omission (No source confirmation required)

	char	str[BUFF_SIZE + 1];
	FILE	*fp;

	// strcpy(filename, GetInPath(filename));
	if((fp = fopen(filename, "rb")) == NULL){
		sprintf(str, "Error: %s can not be opened.", filename);
		Print(str);
		Print("       Preparation for download is valid.");
		return(FALSE);
	}

	if(s_bDCNF_MaskOnOff == FALSE) {
		sprintf(str, "%s Now loading.", filename);
	} else {
		char szDrive[_MAX_DRIVE];
		char szDir[_MAX_DIR];
		char szFilename[_MAX_FNAME];
		char szExt[_MAX_EXT];
		_splitpath(filename, szDrive, szDir, szFilename, szExt);
		strcat(szFilename, szExt);			
		sprintf(str, ";%s Now loading.", szFilename);
	}
	Print(str);

	ret = load_hwcf_rx600(fp);
	fclose(fp);

	if(ret != TRUE){
		return FALSE;
	}

	// Download completion notification
	if((ret = FFWE20Cmd_DCNF_CLOSE()) != FFWERR_OK) {
		PrintErr_E20RX(ret);
		if((ret = FFWE20Cmd_DCNF_CLOSE()) != FFWERR_OK) {
			PrintErr_E20RX(ret);
		}
		return(FALSE);
	}

	// Flag initialization after downloading is complete
	s_FPGA_READY = FALSE;
	s_FpgaType = -1;

	if(s_bDCNF_MaskOnOff == FALSE) {
		sprintf(str, "%s Load end.", filename);
	} else {
		char szDrive[_MAX_DRIVE];
		char szDir[_MAX_DIR];
		char szFilename[_MAX_FNAME];
		char szExt[_MAX_EXT];
		_splitpath(filename, szDrive, szDir, szFilename, szExt);
		strcat(szFilename, szExt);			
		sprintf(str, ";%s Load end.", szFilename);
	}
	Print(str);

	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}

static int load_hwcf_rx600(FILE *fp)
{
	long	filelen, l;
	int		ret;

	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	for(l = 0L; l < filelen; l++){
		hwcfData[l] = (unsigned char)fgetc(fp);
	}

	// Notify download start and pass the total size.
	if((ret = FFWE20Cmd_DCNF_OPEN(l, s_FpgaType)) != FFWERR_OK) {
		PrintErr_E20RX(ret);
		if((ret = FFWE20Cmd_DCNF_CLOSE()) != FFWERR_OK) {
			PrintErr_E20RX(ret);
		}
		return(FALSE);
	}

	// data transfer
	if((ret = FFWE20Cmd_DCNF_SEND(l, hwcfData) ) != FFWERR_OK) {
		PrintErr_E20RX(ret);
		if((ret = FFWE20Cmd_DCNF_CLOSE()) != FFWERR_OK) {
			PrintErr_E20RX(ret);
		}
		return(FALSE);
	}

	if(fgetc(fp) != EOF){
		printf("Error: Format Error\n");
		return(TRUE);
	}
	return(TRUE);
}

// cmd_file.cpp end
//////////////////////////

///////////////////////////////////////////////////////////////
// 10.	FFWRXCmd_SetMCU	Sets per-MCU information
// ;===================================================================
// ; MCU file setting (Set a fixed value this time)
// ;===================================================================
// ;mcu MCUFiles\RXGr\R5F52318.MCU,1
// mcu IF_TYPE 1
static void set_McuArea(FFWRX_MCUAREA_DATA *pMcuArea) {
	pMcuArea->dwSfrAreaNum = 0x00000008;
	// // DWORD dwmadrSfrStartAddr[256];
	pMcuArea->dwmadrSfrStartAddr[0] = 0x00080000;
	pMcuArea->dwmadrSfrStartAddr[1] = 0x00086000;
	pMcuArea->dwmadrSfrStartAddr[2] = 0x00088000;
	pMcuArea->dwmadrSfrStartAddr[3] = 0x00090000;
	pMcuArea->dwmadrSfrStartAddr[4] = 0x000a0000;
	pMcuArea->dwmadrSfrStartAddr[5] = 0x000c0000;
	pMcuArea->dwmadrSfrStartAddr[6] = 0x007fc000;
	pMcuArea->dwmadrSfrStartAddr[7] = 0x007ffc00;
	// DWORD dwmadrSfrEndAddr[256];
	pMcuArea->dwmadrSfrEndAddr[0] = 0x00083fff;
	pMcuArea->dwmadrSfrEndAddr[1] = 0x00087fff;
	pMcuArea->dwmadrSfrEndAddr[2] = 0x0008dfff;
	pMcuArea->dwmadrSfrEndAddr[3] = 0x0009ffff;
	pMcuArea->dwmadrSfrEndAddr[4] = 0x000bffff;
	pMcuArea->dwmadrSfrEndAddr[5] = 0x000fffff;
	pMcuArea->dwmadrSfrEndAddr[6] = 0x007fc4ff;
	pMcuArea->dwmadrSfrEndAddr[7] = 0x007fffff;
	// BYTE  bySfrEndian[256];
	pMcuArea->bySfrEndian[0] = 0x00;
	pMcuArea->bySfrEndian[1] = 0x00;
	pMcuArea->bySfrEndian[2] = 0x01;
	pMcuArea->bySfrEndian[3] = 0x01;
	pMcuArea->bySfrEndian[4] = 0x00;
	pMcuArea->bySfrEndian[5] = 0x01;
	pMcuArea->bySfrEndian[6] = 0x01;
	pMcuArea->bySfrEndian[7] = 0x01;
	pMcuArea->dwRamAreaNum = 0x00000001;
	// DWORD dwmadrRamStartAddr[256];
	pMcuArea->dwmadrRamStartAddr[0] = 0x00000000;
	// DWORD dwmadrRamEndAddr[256];
	pMcuArea->dwmadrRamEndAddr[0] = 0x0000ffff;
	pMcuArea->dwmadrFlashRomWriteStart = 0x00000000;
	pMcuArea->dwmadrFlashRomReadStart = 0xfff80000;
	pMcuArea->dwFlashRomPatternNum = 0x00000001;
	// DWORD dwmadrFlashRomStart[32];
	pMcuArea->dwmadrFlashRomStart[0] = 0xfff80000;
	// DWORD dwFlashRomBlockSize[32];
	pMcuArea->dwFlashRomBlockSize[0] = 0x00000800;
	// DWORD dwFlashRomBlockNum[32];
	pMcuArea->dwFlashRomBlockNum[0] = 0x00000100;
	pMcuArea->dwDataFlashRomPatternNum = 0x00000001;
	// DWORD dwmadrDataFlashRomStart[32];
	pMcuArea->dwmadrDataFlashRomStart[0] = 0x00100000;
	// DWORD dwDataFlashRomBlockSize[32];
	pMcuArea->dwDataFlashRomBlockSize[0] = 0x00000400;
	// DWORD dwDataFlashRomBlockNum[32];
	pMcuArea->dwDataFlashRomBlockNum[0] = 0x00000008;
	pMcuArea->dwExtMemBlockNum = 0x00000003;
	// DWORD dwmadrExtMemBlockStart[256];
	pMcuArea->dwmadrExtMemBlockStart[0] = 0x05000000;
	pMcuArea->dwmadrExtMemBlockStart[1] = 0x06000000;
	pMcuArea->dwmadrExtMemBlockStart[2] = 0x07000000;
	pMcuArea->dwmadrExtMemBlockStart[3] = 0x00000000;
	// DWORD dwmadrExtMemBlockEnd[256];
	pMcuArea->dwmadrExtMemBlockEnd[0] = 0x05ffffff;
	pMcuArea->dwmadrExtMemBlockEnd[1] = 0x06ffffff;
	pMcuArea->dwmadrExtMemBlockEnd[2] = 0x07ffffff;
	pMcuArea->dwmadrExtMemBlockEnd[3] = 0x00000000;
	// enum FFWENM_MACCESS_SIZE eExtMemCondAccess[255];
	pMcuArea->eExtMemCondAccess[0] = MBYTE_ACCESS; // (0);
	pMcuArea->eExtMemCondAccess[1] = MBYTE_ACCESS; // (0);
	pMcuArea->eExtMemCondAccess[2] = MBYTE_ACCESS; // (0);
	pMcuArea->eExtMemCondAccess[3] = MBYTE_ACCESS; // (0);
	// BYTE  byExtMemEndian[256];
	pMcuArea->byExtMemEndian[0] = 0x00;
	pMcuArea->byExtMemEndian[1] = 0x00;
	pMcuArea->byExtMemEndian[2] = 0x00;
	pMcuArea->byExtMemEndian[3] = 0x00;
	// BYTE  byExtMemType[256];
	pMcuArea->byExtMemType[0] = 0x00;
	pMcuArea->byExtMemType[1] = 0x00;
	pMcuArea->byExtMemType[2] = 0x00;
	pMcuArea->byExtMemType[3] = 0x00;
	pMcuArea->dwAdrFcuFirmStart = 0x00000000;
	// pMcuArea->dwFcuFilmLen = 0x00000000; // error: ‘struct FFWRX_MCUAREA_DATA’ has no member named ‘dwFcuFilmLen’
	pMcuArea->dwAdrFcuRamStart = 0x00000000;
	pMcuArea->dwmadrUserBootStart = 0x00000000;
	pMcuArea->dwmadrUserBootEnd = 0x00000000;
};

static void set_McuInfo(FFWRX_MCUINFO_DATA *pMcuInfo) {
	pMcuInfo->dwadrIspOnReset = 0x00000000;
	pMcuInfo->dwadrWorkRam = 0x00001000;
	pMcuInfo->dwsizWorkRam = 0x00000500;
	pMcuInfo->eEdge = TRC_EDGE_DOUBLE; // (1);
	pMcuInfo->eRatio = TRC_RATIO_2_1; // (1);
	pMcuInfo->eExTAL = TRC_EXTAL_ENA; // (7;
	pMcuInfo->eAccSet = ACC_SET_ON; // (1);
	pMcuInfo->eCoProSet = COPRO_SET_OFF; // (0);
	// DWORD	dwSpc[MCU_SPC_MAX];
	pMcuInfo->dwSpc[0] = 0x00200002;
	pMcuInfo->dwSpc[1] = 0x0000a420;
	pMcuInfo->fClkExtal = 22.000000;
	pMcuInfo->dwmadrIdcodeStart;	0xffffffa0;
	pMcuInfo->byIdcodeSize = 0x10;
	pMcuInfo->dwMcuJtagIdcodeNum = 0x00000000;
	// DWORD	dwMcuJtagIdcode[MCU_OCD_ID_MAX]; *Data is not important; Don’t mind data.
	pMcuInfo->dwMcuDeviceCode = 0x38613032;
	pMcuInfo->wMcuFineMakerCode = 0x0223;
	pMcuInfo->wMcuFineFamilyCode = 0x0000;
	pMcuInfo->dwMcuFineDeviceCodeNum = 0x00000001;
	// WORD	wMcuFineDeviceCode[MCU_OCD_ID_MAX];
	pMcuInfo->wMcuFineDeviceCode[0] = 0xd027;
	pMcuInfo->wMcuFineVersionCode = 0x0000;
	pMcuInfo->byBaudRateSelectNum = 0x00;
	// DWORD	dwBaudRateVal[MCU_BRTABLE_NUM_MAX]; *Data is not important; Don’t mind data.
	pMcuInfo->byModeEntryPtNum = 0x09;
	// WORD	wModeEntryPt[MCU_MDENR_PTNNUM_MAX];
	pMcuInfo->wModeEntryPt[0] = 0x4008;
	pMcuInfo->wModeEntryPt[1] = 0x4003;
	pMcuInfo->wModeEntryPt[2] = 0x4007;
	pMcuInfo->wModeEntryPt[3] = 0x4005;
	pMcuInfo->wModeEntryPt[4] = 0x3810;
	pMcuInfo->wModeEntryPt[5] = 0x0008;
	pMcuInfo->wModeEntryPt[6] = 0x38a0;
	pMcuInfo->wModeEntryPt[7] = 0x0005;
	pMcuInfo->wModeEntryPt[8] = 0x38a0;
	pMcuInfo->wSendInterValTimeME = 0x0000;
	pMcuInfo->wSendIntervalTimeNORMAL = 0x0000;
	pMcuInfo->wRcvToSendIntervalTime = 0x0000;
	pMcuInfo->byMcuType = 0x02;
	pMcuInfo->byInterfaceType = 0x01;//0x00;
	// if( pMcuInfo->byInterfaceType == 0x00 ){
	// 	printf("Interface Type              = JTAG\n");
	// } else if( pMcuInfo->byInterfaceType == 0x01 ){
	// 	printf("Interface Type              = FINE\n");
	// }
	pMcuInfo->wPCEvent = 0x0004;
	pMcuInfo->wOAEvent = 0x0002;
	pMcuInfo->wCombination = 0x0006;
	pMcuInfo->wPPC = 0x0001;
	pMcuInfo->wOcdTrcCycMax = 0x0040;
	pMcuInfo->fTgtPwrVal = 0.000000;
	pMcuInfo->dwMcuRegInfoBlkNum = 0x00000006;
	// DWORD	dwMcuRegInfoNum[MCU_REGBLKNUM_MAX];
	pMcuInfo->dwMcuRegInfoNum[0] = 0x00000001;
	pMcuInfo->dwMcuRegInfoNum[1] = 0x00000001;
	pMcuInfo->dwMcuRegInfoNum[2] = 0x00000000;
	pMcuInfo->dwMcuRegInfoNum[3] = 0x00000001;
	pMcuInfo->dwMcuRegInfoNum[4] = 0x00000001;
	pMcuInfo->dwMcuRegInfoNum[5] = 0x00000001;
	// DWORD	dwmadrMcuRegInfoAddr[MCU_REGBLKNUM_MAX][MCU_REGADDRNUM_MAX];
	pMcuInfo->dwmadrMcuRegInfoAddr[0][0] = 0x007fc090;
	pMcuInfo->dwmadrMcuRegInfoAddr[1][0] = 0x007fc090;
	pMcuInfo->dwmadrMcuRegInfoAddr[2][0] = 0x00000000;
	pMcuInfo->dwmadrMcuRegInfoAddr[3][0] = 0x00080008;
	pMcuInfo->dwmadrMcuRegInfoAddr[4][0] = 0x00080018;
	pMcuInfo->dwmadrMcuRegInfoAddr[5][0] = 0x00080031;
	// enum FFWENM_MACCESS_SIZE eMcuRegInfoAccessSize[MCU_REGBLKNUM_MAX][MCU_REGADDRNUM_MAX];
	pMcuInfo->eMcuRegInfoAccessSize[0][0] = MBYTE_ACCESS; // (0);
	pMcuInfo->eMcuRegInfoAccessSize[1][0] = MBYTE_ACCESS; // (0);
	pMcuInfo->eMcuRegInfoAccessSize[2][0] = MBYTE_ACCESS; // (0);
	pMcuInfo->eMcuRegInfoAccessSize[3][0] = MWORD_ACCESS; // (1);
	pMcuInfo->eMcuRegInfoAccessSize[4][0] = MLWORD_ACCESS; // (2);
	pMcuInfo->eMcuRegInfoAccessSize[5][0] = MBYTE_ACCESS; // (0);
	// DWORD	dwMcuRegInfoMask[MCU_REGBLKNUM_MAX][MCU_REGADDRNUM_MAX];
	pMcuInfo->dwMcuRegInfoMask[0][0] = 0x00000001;
	pMcuInfo->dwMcuRegInfoMask[1][0] = 0x00000001;
	pMcuInfo->dwMcuRegInfoMask[2][0] = 0x00000000;
	pMcuInfo->dwMcuRegInfoMask[3][0] = 0x00000001;
	pMcuInfo->dwMcuRegInfoMask[4][0] = 0x00000003;
	pMcuInfo->dwMcuRegInfoMask[5][0] = 0x00000001;
	// DWORD	dwMcuRegInfoData[MCU_REGBLKNUM_MAX][MCU_REGADDRNUM_MAX];
	pMcuInfo->dwMcuRegInfoData[0][0] = 0x00000001;
	pMcuInfo->dwMcuRegInfoData[1][0] = 0x00000000;
	pMcuInfo->dwMcuRegInfoData[2][0] = 0x00000000;
	pMcuInfo->dwMcuRegInfoData[3][0] = 0x00000001;
	pMcuInfo->dwMcuRegInfoData[4][0] = 0x00000000;
	pMcuInfo->dwMcuRegInfoData[5][0] = 0x00000001;
	pMcuInfo->dwVdet0LevelNum = 	0x00000004;
	pMcuInfo->dwVdet0LevelSelMask = 0x00000003;
	// DWORD	dwVdet0LevelSelData[MCU_VDET0LEVELNUM_MAX];
	pMcuInfo->dwVdet0LevelSelData[0] = 0x00000000;
	pMcuInfo->dwVdet0LevelSelData[1] = 0x00000001;
	pMcuInfo->dwVdet0LevelSelData[2] = 0x00000002;
	pMcuInfo->dwVdet0LevelSelData[3] = 0x00000003;
	// DWORD	dwVdet0Level[MCU_VDET0LEVELNUM_MAX];
	pMcuInfo->dwVdet0Level[0] = 0x00000f82;
	pMcuInfo->dwVdet0Level[1] = 0x00000bb8;
	pMcuInfo->dwVdet0Level[2] = 0x00000a6e;
	pMcuInfo->dwVdet0Level[3] = 0x000007c6;
	pMcuInfo->dwEmuAccCtrlAreaNum = 0x00000001;
	// DWORD	dwmadrEmuAccCtrlStart[MCU_ACCCTRL_AREANUM_MAX];
	pMcuInfo->dwmadrEmuAccCtrlStart[0] = 0x00086400;
	// DWORD	dwmadrEmuAccCtrlEnd[MCU_ACCCTRL_AREANUM_MAX];
	pMcuInfo->dwmadrEmuAccCtrlEnd[0] = 0x00086fff;
	// enum FFWRXENM_EMU_ACCESS_TYPE	eEmuAccType[MCU_ACCCTRL_AREANUM_MAX];
	pMcuInfo->eEmuAccType[0] = RX_EMU_ACCTYPE_RW; // (0);
	// enum FFWRXENM_EMU_ACCESS_MEANS	eEmuAccMeans[MCU_ACCCTRL_AREANUM_MAX];
	pMcuInfo->eEmuAccMeans[0] = RX_EMU_ACCMEANS_CPU; //(1)
	pMcuInfo->eCpuIsa = RX_ISA_RXV2; // (1)
	pMcuInfo->eFpuSet = RX_FPU_ON; // (1)
	pMcuInfo->fClkIclk = 0.00000000;
};

static void setIOExtAreaData(FFWRX_MCUAREA_DATA* pMcuArea);
static int do_mcu_rx600(void)
{
	int ret;
	DWORD dwMcuAreaStructSize = 0x00002230;
	DWORD dwMcuInfoStructSize = 0x00001634;
	FFWRX_MCUAREA_DATA McuArea;
	FFWRX_MCUINFO_DATA McuInfo;
	set_McuArea(&McuArea);
	set_McuInfo(&McuInfo);
	if((ret = FFWRXCmd_SetMCU(dwMcuAreaStructSize, &McuArea, dwMcuInfoStructSize, &McuInfo)) != FFWERR_OK) {
		PrintErr_E20RX(ret);
		printf("[%s] Returned 0x%.8x\n", __func__, ret);
		if (ret != FFWERR_FFWPARA_OVER) {
			return FALSE;
		}
	}

	setIOExtAreaData(&McuArea);

	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}
///////////////////////////////////////////////////////////////
// 11.	FFWRXCmd_SetDBG	Sets debug information
// ;===================================================================
// ; Setting debug information
// ;===================================================================
// dbg use, nouse, trace, nouse, normal, 0, 0, 0, 0, debug, off, off, on, on, dump, non, ena
// static void set_Dbg(FFWRX_DBG_DATA* pDbg)
// {
// 	pDbg->eOcdArgErrChk = OCD_ARGERRCHK_USE;	// OCD_ARGERRCHK_USE (1)
// 	pDbg->eMcuMultiCore = EML_MCU_MULTI_NON; //  (0)
// 	pDbg->eTrcFuncMode = EML_TRCFUNC_TRC; // (0)
// 	pDbg->eRomCorrection = EML_ROM_CORRECT_NON; // (0)
// 	pDbg->eWriter = EML_WRITERMODE_NON; // (0)
// 	pDbg->dwJtagByPass = 0x00000000;
// 	pDbg->eUseProgram = EML_HEW; // (0)
// 	pDbg->eDbgFlashWrite = EML_DBG_FLASHWRITE_NON; // (0)
// 	pDbg->eDbgDataFlashWrite = EML_DBG_FLASHWRITE_NON; // (0)
// 	pDbg->eBfrRunRegSet = EML_RUNREGSET_USE; // (1)
// 	pDbg->eAftRunRegSet = EML_RUNREGSET_USE; // (1)
// 	pDbg->eOcdRegAcsMode = OCD_ACSMODE_DUMP; // (3)
// 	pDbg->eSciIF = SCI_NON; // (0)
// 	pDbg->eClkChangeEna = RX_CLKCHANGE_ENA; // (1)
// 	pDbg->eDbgBootSwap = RX_DBG_BOOTSWAP_NON; // (0)
// 	pDbg->dwDbgRegInfoBlkNum = 0x00000000;
// 	// DWORD	dwDbgRegInfoNum[32];		* Data does not matter
// 	// DWORD	dwmadrDbgRegInfoAddr[32][8];	* Data does not matter	
// 	// enum FFWENM_MACCESS_SIZE		eDbgRegInfoAccessSize[32][8];	* Data does not matter
// 	// DWORD	dwDbgRegInfoMask[32][8];		* Data does not matter
// 	// DWORD	dwDbgRegInfoData[32][8];		* Data does not matter
// 	// DWORD	dwDbgAreaBlkNum;		* Data does not matter
// 	// DWORD	dwDbgAreaNum[32];		* Data does not matter
// 	// DWORD	dwmadrDbgAreaStartAddr[32][8];	* Data does not matter
// 	// DWORD	dwmadrDbgAreaEndAddr[32][8];	* Data does not matter
// };

static DWORD	s_dwSizeOfDbgStruct = sizeof(FFWRX_DBG_DATA);
// [E20]
static int do_dbg_rx600(char *para[])
{
	FFWRX_DBG_DATA	dbgdata;
	char	buffer[BUFF_SIZE + 1];
	char	tmp[BUFF_SIZE + 1];
	int		ret;
	char	chJtagByPassH;
	char	chJtagByPassMH;
	char	chJtagByPassML;
	char	chJtagByPassL;
	ULONG	ulData;
	DWORD	i,j;
	DWORD	dwRegNum,dwRegBlkNum;
	DWORD	dwAreaNum,dwAreaBlkNum;
	DWORD	dwStructSize;
	BOOL	bSetParaSize;

	// パラメータサイズの参照/設定処理
	sprintf(buffer, "DBG parameter size = %08X", s_dwSizeOfDbgStruct);
	Print(buffer);

	// FFWRX_DBG_DATA構造体のサイズを取得
	dwStructSize = s_dwSizeOfDbgStruct;

	// ゴミを表示しないように初期化しておく
	memset(&dbgdata,0,sizeof(FFWRX_DBG_DATA));

	// 参照
	if (para[0] == NULL) {
	}
	// ファイル読込
	else if(para[1] == NULL) {
	}
	// コマンド設定
	else {
		// OCD関連の組合せ制限エラー検出有無
		if (para[0] == NULL) {
			Print("Error: OCD関連の組合せ制限エラー検出有無が指定されていません。");
			return FALSE;
		}

		_strupr(para[0]);
		if (Strcmp(para[0], "NOUSE") == 0) {
			dbgdata.eOcdArgErrChk = OCD_ARGERRCHK_NOUSE;
		} else if (Strcmp(para[0], "USE") == 0) {
			dbgdata.eOcdArgErrChk = OCD_ARGERRCHK_USE;
		} else {
			Print("Error para[0] dbgdata.eOcdArgErrChk");
			dbgdata.eOcdArgErrChk = OCD_ARGERRCHK_USE;
		}

		// コア指定有無
		_strupr(para[1]);
		if (Strcmp(para[1], "NOUSE") == 0) {
			dbgdata.eMcuMultiCore = EML_MCU_MULTI_NON;
		} else if (Strcmp(para[1], "USE") == 0) {
			dbgdata.eMcuMultiCore = EML_MCU_MULTI_USE;
		} else {
			Print("Error para[1] dbgdata.eMcuMultiCore");
			dbgdata.eMcuMultiCore = EML_MCU_MULTI_NON;
		}

		// トレース利用機能

		_strupr(para[2]);
		if (Strcmp(para[2], "TRACE") == 0) {
			dbgdata.eTrcFuncMode = EML_TRCFUNC_TRC;
		} else if ((Strcmp(para[2], "COVARAGE") == 0) || (Strcmp(para[2], "CV") == 0)) {	// // RevRxNo130308-001 Modify Line
			dbgdata.eTrcFuncMode = EML_TRCFUNC_CV;
		} else {
			Print("Error para[2] dbgdata.eTrcFuncMode");
			dbgdata.eTrcFuncMode = EML_TRCFUNC_TRC;
		}

		// ROMコレクション利用
		_strupr(para[3]);
		if (Strcmp(para[3], "NOUSE") == 0) {
			dbgdata.eRomCorrection = EML_ROM_CORRECT_NON;
		} else if (Strcmp(para[3], "USE") == 0) {
			dbgdata.eRomCorrection = EML_ROM_CORRECT_USE;
		} else {
			Print("Error para[3] dbgdata.eRomCorrection");
			dbgdata.eRomCorrection = EML_ROM_CORRECT_NON;
		}

		// 通常モード/ライタモード指定
		_strupr(para[4]);
		if (Strcmp(para[4], "NORMAL") == 0) {
			dbgdata.eWriter = EML_WRITERMODE_NON;
		} else if (Strcmp(para[4], "WRITER") == 0) {
			dbgdata.eWriter = EML_WRITERMODE_USE;
		} else {
			Print("Error para[4] dbgdata.eWriter");
			dbgdata.eWriter = EML_WRITERMODE_NON;	
		}

		// JTAGカスケード接続時のバイパス情報
		// para[5]
		// para[6]
		// para[7]
		// para[8]
		ulData = 0x00;
		dbgdata.dwJtagByPass = ((ulData << 24) & 0xFF000000);	// TDIデバイス数
		dbgdata.dwJtagByPass |= ((ulData << 16) & 0x00FF0000);	// TDOデバイス数
		dbgdata.dwJtagByPass |= ((ulData << 8) & 0x0000FF00);	// TDIビット総和
		dbgdata.dwJtagByPass |= ((ulData << 0) & 0x000000FF);	// TDOビット総和

		// 使用するプログラム
		_strupr(para[9]);
		if (Strcmp(para[9], "DEBUG") == 0) {
			dbgdata.eUseProgram = EML_HEW;
		}
		else if (Strcmp(para[9], "FDT") == 0) {
			dbgdata.eUseProgram = EML_FDT;
		} else if (Strcmp(para[9], "STICK") == 0) {
			dbgdata.eUseProgram = EML_RXSTICK;
		} else if (Strcmp(para[9], "MONITOR") == 0) {
			dbgdata.eUseProgram = EML_MONITOR;
		}
		else if (Strcmp(para[9], "EZ") == 0) {
			dbgdata.eUseProgram = EML_EZ_CUBE;
		}
		else {
			Print("Error para[9] dbgdata.eUseProgram");
			dbgdata.eUseProgram = EML_HEW;
		}

		// コードFlash書き換えデバッグ
		_strupr(para[10]);
		if (Strcmp(para[10], "OFF") == 0) {
			dbgdata.eDbgFlashWrite = EML_DBG_FLASHWRITE_NON;
		} else if (Strcmp(para[10], "ON") == 0) {
			dbgdata.eDbgFlashWrite = EML_DBG_FLASHWRITE_USE;
		} else {
			Print("Error para[10] dbgdata.eDbgFlashWrite");
			dbgdata.eDbgFlashWrite = EML_DBG_FLASHWRITE_NON;
		}

		// データFlash書き換えデバッグ
		_strupr(para[11]);
		if (Strcmp(para[11], "OFF") == 0) {
			dbgdata.eDbgDataFlashWrite = EML_DBG_FLASHWRITE_NON;
		} else if (Strcmp(para[11], "ON") == 0) {
			dbgdata.eDbgDataFlashWrite = EML_DBG_FLASHWRITE_USE;
		} else {
			Print("Error para[11] dbgdata.eDbgDataFlashWrite");
			dbgdata.eDbgDataFlashWrite = EML_DBG_FLASHWRITE_NON;
		}

		// プログラム実行前設定の有無
		_strupr(para[12]);
		if (Strcmp(para[12], "OFF") == 0) {
			dbgdata.eBfrRunRegSet = EML_RUNREGSET_NON;
		} else if (Strcmp(para[12], "ON") == 0) {
			dbgdata.eBfrRunRegSet = EML_RUNREGSET_USE;
		} else {
			Print("Error para[12] dbgdata.eBfrRunRegSet");
			dbgdata.eBfrRunRegSet = EML_RUNREGSET_USE;			
		}

		// プログラム実行後設定の有無
		_strupr(para[13]);
		if (Strcmp(para[13], "OFF") == 0) {
			dbgdata.eAftRunRegSet = EML_RUNREGSET_NON;
		} else if (Strcmp(para[13], "ON") == 0) {
			dbgdata.eAftRunRegSet = EML_RUNREGSET_USE;
		} else {
			Print("Error para[13] dbgdata.eBfrRunRegSet");
			dbgdata.eAftRunRegSet = EML_RUNREGSET_USE;
		}

		// 制御レジスタへのアクセスモード指定
		_strupr(para[14]);
		if (Strcmp(para[14], "OFF") == 0) {
			dbgdata.eOcdRegAcsMode = OCD_ACSMODE_NONDUMPFILL;
		} else if (Strcmp(para[14], "FILL") == 0) {
			dbgdata.eOcdRegAcsMode = OCD_ACSMODE_FILL;
		} else if (Strcmp(para[14], "ON") == 0) {
			dbgdata.eOcdRegAcsMode = OCD_ACSMODE_DUMPFILL;
		} else if (Strcmp(para[14], "DUMP") == 0) {
			dbgdata.eOcdRegAcsMode = OCD_ACSMODE_DUMP;
		} else {
			Print("Error para[14] dbgdata.eOcdRegAcsMode");
			dbgdata.eOcdRegAcsMode = OCD_ACSMODE_DUMP;
		}

		// 同期/非同期通信指定
		_strupr(para[15]);
		if (Strcmp(para[15], "NON") == 0) {
			dbgdata.eSciIF = SCI_NON;
		} else if (Strcmp(para[15], "SCI") == 0) {
			dbgdata.eSciIF = SCI_SCI;
		} else if (Strcmp(para[15], "UART") == 0) {
			dbgdata.eSciIF = SCI_UART;
		} else {
			Print("Error para[15] dbgdata.eOcdRegAcsMode");
			dbgdata.eSciIF = SCI_NON;
		}

		// クロック切り替え指定
		_strupr(para[16]);
		if (Strcmp(para[16], "DIS") == 0) {
			dbgdata.eClkChangeEna = RX_CLKCHANGE_DIS;
		} else if (Strcmp(para[16], "ENA") == 0) {
			dbgdata.eClkChangeEna = RX_CLKCHANGE_ENA;
		} else {
			Print("Error para[16] dbgdata.eClkChangeEna");
			dbgdata.eClkChangeEna = RX_CLKCHANGE_ENA;
		}

	// ブートスワップ機能のパラメータ省略対応
		if( para[17] == NULL ){
			// 省略時はOFF
			dbgdata.eDbgBootSwap = RX_DBG_BOOTSWAP_NON;
		}
		else{
		// NDEB新I/F対応
		// ブートスワップ機能を使用するプログラムのデバッグ指定
			_strupr(para[17]);
			if (Strcmp(para[17], "OFF") == 0) {
				dbgdata.eDbgBootSwap = RX_DBG_BOOTSWAP_NON;
			} else if (Strcmp(para[17], "ON") == 0) {
				dbgdata.eDbgBootSwap = RX_DBG_BOOTSWAP_USE;
			} else {
				Print("Error para[17] dbgdata.eDbgBootSwap");
				dbgdata.eDbgBootSwap = RX_DBG_BOOTSWAP_NON;
			}
		}

	// NDEB新I/F対応
	// FFWRXCmd_SetDBGの第1引数に、第2引数の構造体のサイズを渡す
		if ((ret = FFWRXCmd_SetDBG( dwStructSize,&dbgdata )) != FFWERR_OK) {
			PrintErr_E20RX(ret);
			if (ret != FFWERR_FFWPARA_OVER) {
				printf("[%s] returned FALSE\n", __func__);
				return FALSE;
			}
		}
	}
	printf("[%s] returned TRUE\n", __func__);
	return TRUE;
}

static int	DO_DBG(char *b_vec, char *b_inst, char *b_w, char *p1, char *p2, char *p3, char *p4, char *p5, 
		   char *p6, char *p7, char *p8, char *p9, char *p10, char *p11, char *p12, char *p13,char *p14,char *p15)
{

	char *para[18];
	// [E20]
	// if ((PROT_EMU_TYPE() == EMU_E20) || (PROT_EMU_TYPE() == EMU_E2)) {
		para[0] = b_vec;
		para[1] = b_inst;
		para[2] = b_w;
		para[3] = p1;
		para[4] = p2;
		para[5] = p3;
		para[6] = p4;
		para[7] = p5;
		para[8] = p6;
		para[9] = p7;
		para[10] = p8;
		para[11] = p9;
		para[12] = p10;
		para[13] = p11;
		para[14] = p12;
		para[15] = p13;
		para[16] = p14;
		para[17] = p15;

		return do_dbg_rx600(para);
	// }

	// return(FALSE);
}


///////////////////////////////////////////////////////////////
/////  No.12  Supplementary information
// Loads internal flash write program.
// 
// [NDEB Command]
// >wtr type0, wtrrx230.bin
// 
// [NDEB Source:]
// cmd_file.cpp
#define	BUFFER_MAX	(0x20000)
#define	FILE_LENGTH_MAX	(0x800)		/* CPE2対応のため追加 */

#define	LENGTH_MAX	(0x10000)	/* DWNHFD用追加 */
static char	buffer[BUFFER_MAX + 1];

static int DO_WTR(char *type, char *filename)
{
	int	ret;
	FILE	*fp;
	char	str[BUFF_SIZE + 1];
	long filelen = 0;
	FFWRXENM_WTRTYPE eWtrType;
	DWORD dwDataSize;
	BYTE* pbyDataBuff;
	ULONG	ulData;
	char chWtrVer[17];

	char chV1[2];
	char chV2[3];
	char chV3[3];
	char chV4[4];
	char chName[9];
	int i;

	// ファイルタイプの選別
	if (type == NULL) {
		printf("Error: WTR File type is not defined."); // 
		return FALSE;
	}

	_strupr(type);
	printf("[%s] type %s\n", __func__, type);
	if (Strcmp(type, "TYPE0") == 0) {
		eWtrType = WTRTYPE_0;
	} else if (Strcmp(type, "TYPE1") == 0) {
		eWtrType = WTRTYPE_1;
	} else {
		eWtrType = WTRTYPE_0;
	}
	// Ver取得

	if(filename == NULL){
		printf("Error: filename is null\n");
	} else {
	// WTR ロード
		// ファイル名称に作業パスを付加
		// // strcpy( filename,GetInPath(filename) );
		// WTRファイルオープン
		if( (fp = fopen( filename,"rb") ) == NULL ){
			// ファイルが正常にオープン出来なかった場合はエラー処理
			sprintf( str,"Error: Can not open %s.", filename );
			printf("%s\n", str );
			return( FALSE );
		}
		printf("[%s] Open %s to read\n", __func__, filename);
		// ファイルサイズを調査 
		fseek(fp,0,SEEK_END); 
		filelen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		dwDataSize = (DWORD)filelen;

		// ファイル格納メモリの生成
		pbyDataBuff = new BYTE [dwDataSize];

		// ファイルデータの読み出し
		fread(pbyDataBuff,sizeof(char),dwDataSize,fp);

		if((ret = FFWRXCmd_WTRLOAD(eWtrType,dwDataSize,pbyDataBuff)) != FFWERR_OK) {
			PrintErr_E20RX(ret);
			delete [] pbyDataBuff;
			fclose(fp);
			return(FALSE);
		}
		fclose(fp);

		delete [] pbyDataBuff;
	}
	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}
///////////////////////////////////////////////////////////////
// 13. FFWE20Cmd_SetSCIBR Sets emulator-to-MCU serial communication baud rate	
// ;===================================================================
// ; Emulator<->MCU serial communication baud rate setting
// ;===================================================================
// scibr 2000000
static int	DO_SCIBR( char *pBaudId )
{
	int		ret;

	// 引数有りの場合は設定モード
	DWORD	dwBaudId;
	if( pBaudId == NULL ){
		printf("[%s] Invalid input\n", __func__);
		return FALSE;
	}

	_strupr( pBaudId );		// 引数を大文字変換
	dwBaudId = strtoul( pBaudId,NULL,10 );
	if( (ret = FFWE20Cmd_SetSCIBR( dwBaudId )) != FFWERR_OK ){
		// エラー表示処理
		PrintErr_E20RX( ret );
		return FALSE;
	}

	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}
///////////////////////////////////////////////////////////////
// 14.	FFWE20Cmd_SetSCICTRL	Sets emulator’s serial communication control information	
// ;===================================================================
// ; Emulator serial communication control information setting
// ;===================================================================
// scictrl 0404
static int	DO_SCICTRL( char *scictrl )
{

	int		ret;
	WORD	wSciCtrl;
	char	strMsg[BUFF_SIZE + 1];
	DWORD	dwData;

	if( scictrl == NULL ){
		printf("[%s] Invalid input\n", __func__);
		return FALSE;
	}

	dwData = strtoul( scictrl,NULL,16 );
	wSciCtrl = (WORD)dwData;
	printf("[%s] SciCtrl 0x%.4x\n", __func__, dwData);
	// シリアル通信設定コマンド送信
	if( (ret = FFWE20Cmd_SetSCICTRL( wSciCtrl )) != FFWERR_OK ){
		// エラー表示処理
		PrintErr_E20RX( ret );
		return FALSE;
	}

	printf("[%s] Returned TRUE\n", __func__);
	return	TRUE;
}
///////////////////////////////////////////////////////////////
// ;===================================================================
// ; Processor mode and endian setting (start mode specification)
// ;===================================================================
// #ifdef ENDIAN_BIG
// pmod sng,r_rome, big,55736572,426F6F74,FFFFFF07,0008C04C
// #endif
// #ifdef ENDIAN_LITTLE
// pmod sng,r_rome, little,55736572,426F6F74,FFFFFF07,0008C04C
// #endif

#define MCUADDR_INT_EXT		0xff000000	// 内蔵ROM無効拡張モードでの、内部/外部切り分けするアドレス
// #define	MCU_AREANUM_MAX		32	// MCUコマンドの内部I/O,RAM,データフラッシュROM領域数の最大値

static FFWENM_ENDIAN 	s_eMcuEndian;
static DWORD				s_dwIOExtNum;
static ULONG				s_madrIOExtStart[MCU_AREANUM_MAX_RX + MCU_AREANUM_MAX_RX];
static ULONG				s_madrIOExtEnd[MCU_AREANUM_MAX_RX + MCU_AREANUM_MAX_RX];
static enum FFWENM_ENDIAN	s_eIOExtEndian[MCU_AREANUM_MAX_RX + MCU_AREANUM_MAX_RX];

static int	do_pmod_rx600(char *p1, char *p2, char *p3, char *p4, char *p5, char *p6, char *p7)//, char *p8, char *p9, char *p10)
{
	FFWRXENM_PMODE	ePmode;
	FFWRXENM_RMODE	eRmode;
	FFWENM_ENDIAN	eMcuEndian;
	FFWRX_UBCODE_DATA	pUbcode;
	DWORD ret, dwdata, dwdata1;
	char	buffer[128];
	FFWRX_UBAREA_CHANGEINFO_DATA pUbAreaChange;	

	// 設定
	if( p1 == NULL ) {
		Print("Error: プロセッサモード指定がありません。");
		return FALSE;
	}
	_strupr(p1);
	printf("[%s] p1 %s\n", __func__, p1);
	if(Strcmp("SNG", p1) == 0){
		ePmode = RX_PMODE_SNG;
	}else if(Strcmp("ROME", p1) == 0){
		ePmode = RX_PMODE_ROME;
	}else if(Strcmp("ROMD16", p1) == 0){
		ePmode = RX_PMODE_ROMD16;
	}else if(Strcmp("ROMD8", p1) == 0){
		ePmode = RX_PMODE_ROMD8;
	}else if(Strcmp("USRBOOT", p1) == 0){
		ePmode = RX_PMODE_USRBOOT;
	}else if(Strcmp("USBBOOT", p1) == 0){
		ePmode = RX_PMODE_USRBOOT;
	}else{
		Print("Error: Invalid p1");
		ePmode = RX_PMODE_SNG;
	}


	// 設定
	if( p2 == NULL ) {
		Print("Error: レジスタ設定プロセッサモード指定がありません。");
		return FALSE;
	}
	_strupr(p2);
	printf("[%s] p2 %s\n", __func__, p2);
	if(Strcmp("R_SNG", p2) == 0){
		eRmode = RX_RMODE_SNG;
	}else if(Strcmp("R_ROME", p2) == 0){
		eRmode = RX_RMODE_ROME;
	}else if(Strcmp("R_ROMD", p2) == 0){
		eRmode = RX_RMODE_ROMD;
	}else if(Strcmp("R_SNG_D_0", p2) == 0){
		eRmode = RX_RMODE_SNG_DUAL_BANK0;
	}else if(Strcmp("R_SNG_D_1", p2) == 0){
		eRmode = RX_RMODE_SNG_DUAL_BANK1;
	}else if(Strcmp("R_SNG_D", p2) == 0){
		eRmode = RX_RMODE_SNG_DUAL;
	}else if(Strcmp("R_ROME_D_0", p2) == 0){
		eRmode = RX_RMODE_ROME_DUAL_BANK0;
	}else if(Strcmp("R_ROME_D_1", p2) == 0){
		eRmode = RX_RMODE_ROME_DUAL_BANK1;
	}else if(Strcmp("R_ROME_D", p2) == 0){
		eRmode = RX_RMODE_ROME_DUAL;
	}else if(Strcmp("R_ROMD_D_0", p2) == 0){
		eRmode = RX_RMODE_ROMD_DUAL_BANK0;
	}else if(Strcmp("R_ROMD_D_1", p2) == 0){
		eRmode = RX_RMODE_ROMD_DUAL_BANK1;
	}else if(Strcmp("R_ROMD_D", p2) == 0){
		eRmode = RX_RMODE_ROMD_DUAL;
	// RevRxNo161003-001 Append End
	}else{
		Print("Error: Invalid p2");
		eRmode = RX_RMODE_ROME;
	}


	if( p3 == NULL ) {
		Print("Error: エンディアンの指定がありません。");
		return FALSE;
	}
	_strupr(p3);
	printf("[%s] p3 %s\n", __func__, p3);
	if(Strcmp("LITTLE", p3) == 0){
		eMcuEndian = ENDIAN_LITTLE;
	}else if(Strcmp("BIG", p3) == 0){
		eMcuEndian = ENDIAN_BIG;
	}else{
		Print("Error: Invalid p3");
		eMcuEndian = ENDIAN_LITTLE;
	}

	if( p4 == NULL ) {
		// 引数4番目以降がない場合は正常終了。4番目はないが5番目以降がかかれている場合はエラー
		if((p5 != NULL) | (p6 != NULL) | (p7 != NULL)) {
			Print("Error: UBコードA1の指定がありません。");
		}
		return TRUE;
	}

	dwdata = strtoul( p4,NULL,16 );
	printf("[%s] p4 0x%08X\n", __func__, dwdata);
	dwdata = 0x55736572;
	pUbcode.byUBCodeA[0] = (BYTE)((dwdata >> 24) & 0xFF);
	pUbcode.byUBCodeA[1] = (BYTE)((dwdata >> 16) & 0xFF);
	pUbcode.byUBCodeA[2] = (BYTE)((dwdata >> 8) & 0xFF);
	pUbcode.byUBCodeA[3] = (BYTE)((dwdata >> 0) & 0xFF);
	if( p5 == NULL ) {
		Print("Error: UBコードA2の指定がありません。");
		return FALSE;
	}

	dwdata = strtoul( p5,NULL,16 );
	printf("[%s] p5 0x%08X\n", __func__, dwdata);
	// dwdata = 0x426F6F74;
	pUbcode.byUBCodeA[4] = (BYTE)((dwdata >> 24) & 0xFF);
	pUbcode.byUBCodeA[5] = (BYTE)((dwdata >> 16) & 0xFF);
	pUbcode.byUBCodeA[6] = (BYTE)((dwdata >> 8) & 0xFF);
	pUbcode.byUBCodeA[7] = (BYTE)((dwdata >> 0) & 0xFF);

	if( p6 == NULL ) {
		Print("Error: UBコードB1の指定がありません。");
		return FALSE;
	}

	dwdata = strtoul( p6,NULL,16 );
	printf("[%s] p6 0x%08X\n", __func__, dwdata);
	pUbcode.byUBCodeB[0] = (BYTE)((dwdata >> 24) & 0xFF);
	pUbcode.byUBCodeB[1] = (BYTE)((dwdata >> 16) & 0xFF);
	pUbcode.byUBCodeB[2] = (BYTE)((dwdata >> 8) & 0xFF);
	pUbcode.byUBCodeB[3] = (BYTE)((dwdata >> 0) & 0xFF);
	if( p7 == NULL ) {
		Print("Error: UBコードB2の指定がありません。");
		return FALSE;
	}

	dwdata = strtoul( p7,NULL,16 );
	printf("[%s] p7 0x%08X\n", __func__, dwdata);
	// dwdata = 0x0008C04C;
	pUbcode.byUBCodeB[4] = (BYTE)((dwdata >> 24) & 0xFF);
	pUbcode.byUBCodeB[5] = (BYTE)((dwdata >> 16) & 0xFF);
	pUbcode.byUBCodeB[6] = (BYTE)((dwdata >> 8) & 0xFF);
	pUbcode.byUBCodeB[7] = (BYTE)((dwdata >> 0) & 0xFF);


	ret = FFWRXCmd_SetPMOD(ePmode, eRmode, eMcuEndian, &pUbcode, &pUbAreaChange);
	s_eMcuEndian = eMcuEndian;
	if(ret != FFWERR_OK ) {		
		PrintErr_E20RX(ret);
		return(FALSE);
	}

	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}

static int DO_PMOD(char *p1, char *p2, char *p3, char *p4, char *p5, char *p6, char *p7)
{
	int ret = 0;
	ret = do_pmod_rx600(p1, p2, p3, p4, p5, p6, p7);//, p8, p9, p10);
	return ret;
	
}
///////////////////////////////////////////////////////////////
// 17.
// ;===================================================================
// ; Communication established
// ;===================================================================
// hpon

static int do_hpon_rx600(char *plug, char *condition)
{
	int		ret, Plug = 0;
	ULONG	ulData;

	if (plug != NULL && condition != NULL) {
		_strupr(plug);
		_strupr(condition);

		if ((Strcmp("ON", plug) == 0) && (Strcmp("COLD", condition) == 0)) {
			Plug = 0;
		} else if ((Strcmp("ON", plug) == 0) && (Strcmp("HOT", condition) == 0)) {
			Plug = 1;
		} else if ((Strcmp("OFF", plug) == 0) && (Strcmp("KEEPLOW", condition) == 0)) {
			Plug = 2;
		} else if ((Strcmp("OFF", plug) == 0) && (Strcmp("KEEPOFF", condition) == 0)) {
			Plug = 3;
		} else if ((Strcmp("OFF", plug) == 0) && (Strcmp("OFF", condition) == 0)) {
			Plug = 4;
		} else if ((Strcmp("ON", plug) == 0) && (Strcmp("ALREADYCONNECT", condition) == 0)) {
			Plug = 0xFF;
		} else {
			if(Plug != 0) {
				Print("Error: HPON設定が異常です。");
				return(FALSE);
			}
		}
	}

printf("[%s] Plug %d\n", __func__ , Plug);
	if((ret = FFWRXCmd_HPON(Plug)) != FFWERR_OK){
		PrintErr_E20RX(ret);
		return FALSE;
	}
	if(Plug == 0){
		Print("Hotplug Cold ON");
	}else if(Plug == 1){
		Print("Hotplug Hot ON");
	}else if(Plug == 2){
		Print("Hotplug OFF (ResetKeep,EMLE_LOW)");
	}else if(Plug == 3){
		Print("Hotplug OFF (ResetKeep,Off)");
	}else if(Plug == 4){
		Print("Hotplug OFF (ResetOff,Off)");
	}else if(Plug == 0xFF){
		Print("Hotplug ON (Already Connect)");
	}

	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}

static int	DO_HPON(char *plug, char *condition)
{
	// if ((PROT_EMU_TYPE() == EMU_E20) || (PROT_EMU_TYPE() == EMU_E2)) {
		return(do_hpon_rx600(plug, condition));
	// }

	// return(FALSE);
}
///////////////////////////////////////////////////////////////
// 18.	FFWRXCmd_IDCODE	Sets ID code	Required only when internal flash ROM is enabled
// ;===================================================================
// ; ID code setting
// ;===================================================================
// idcode FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
static int	do_idcode_rx600(char *idcode)
{
	int				ret;
	unsigned char	code[20];
	unsigned int	nLen;
	unsigned int	ui, uj;
	char			buff[20];

	if(idcode == NULL){
		Print("Error: IDコードが指定されていません。");
		return(FALSE);
	}else{
		while (isspace(*idcode) != 0) {
			idcode++;
		}
		if(*idcode == '"') {
			//文字列指定
			idcode++;
			nLen = strlen(idcode);
			if(nLen < 1) {
				Print("Error: 文字列長が不足しています。");
				return FALSE;
			}
			if(idcode[nLen-1] !='\"') {
				Print("Error: 文字列が'\"'で終了していません。");
				return FALSE;
			}
			memset(code, 0xFF, sizeof(code));
			memcpy(code, idcode, nLen - 1);
		} else {
			//データ指定
			nLen = strlen(idcode);
			if((nLen % 2) != 0) {
				Print("Error: データ文字が奇数個指定されています。");
				return FALSE;
			}
			memset(code, 0xFF, sizeof(code));
			for(ui = 0, uj = 0; ui < nLen; ui+=2, uj++) {
				strncpy(buff, &idcode[ui], 2);
				buff[2] = '\0';
				code[uj] = (BYTE)strtol(buff, '\0', 16);
				if(uj == 15) {
					break;
				}
			}
		}

// memset(code, 'F', sizeof(code));
for(int i = 0; i < nLen; i++)
{
	printf("[%d]0x%X ",i, code[i]);
}
printf("\n");

// printf("[%s] code '%x'\n", __func__, code);
		if((ret = FFWRXCmd_IDCODE(code)) != FFWERR_OK){
			PrintErr_E20RX(ret);
			return FALSE;
		}
	}
	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}

static int	DO_IDCODE(char *idcode)
{
	// if ((PROT_EMU_TYPE() == EMU_E20) || (PROT_EMU_TYPE() == EMU_E2)) {
		return do_idcode_rx600(idcode);
	// } else {
	// 	Print("このコマンドは使用できません。");
	// }
	// return FALSE;
}
///////////////////////////////////////////////////////////////
// 19.	FFWRXCmd_SetPMOD	Sets processor mode and endian (to check actual settings)	
// ; ===================================================================
// ; Processor mode and endian setting (actual setting check)
// ; ===================================================================
// #ifdef ENDIAN_BIG
// pmod sng,r_rome, big,55736572,426F6F74,FFFFFF07,0008C04C
// #endif
// #ifdef ENDIAN_LITTLE
// pmod sng,r_rome, little,55736572,426F6F74,FFFFFF07,0008C04C
// #endif

///////////////////////////////////////////////////////////////
// 25.	FFWMCUCmd_CLR	Initializes emulator’s internal settings
// ; ===================================================================
// ; Initialization of emulator internal settings
// ; ===================================================================
// clr
static int DO_CLR(void)
{
	DWORD	ret;

	if((ret = FFWMCUCmd_CLR()) != FFWERR_OK){
		PrintErr_E20RX(ret);
		printf("[%s] Returned FALSE\n", __func__);
		return FALSE;
	}
	
	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}

///////////////////////////////////////////////////////////////

int init_ndeb(void)
{
	int ret;
	
	// 1.	FFWE20Cmd_INIT	Initializes FFW module
	DWORD dwGageCount = 0;
	FFW_WINVER_DATA WinVer;
	FFWE20Cmd_INIT((void*)COM_SendBlock, (void*)COM_ReceiveBlock, (void*)COM_ReceiveBlockH, (void*)COM_ReceivePolling, (void*)COM_SetTimeOut, (void*)COM_Lock, (void*)COM_UnLock, (void*)COM_SetTransferSize, &dwGageCount, &WinVer, NULL, NULL);

	// // 2.	FFWCmd_SetUSBTS	Sets USB transfer size	
	// // 3.	FFWCmd_SetTMOUT	Sets timeout time of FFW and BFW
	// 4.	FFWE20Cmd_GetEINF	Obtains emulator information
	Print("\neinf");
	FFWE20_EINF_DATA Einf;
	FFWE20Cmd_GetEINF(&Einf);

	// // 5.	FFWE20Cmd_MONP_xxx 	Rewrites emulator’s internal BFW module
	// 6.	FFWE20Cmd_DCNF_xxx	Configures DCSF in emulator
	Print("\ndcnf 0\ndcnf E2_RXRL.bit");
	DO_DCNF((char*)"E2_RXRL.bit");

	// 7.	FFWE20Cmd_EXTPWR	Supplies power to the user board
	// char s_ena[] = "ena";
	// char ena[] = "EML";
	// char val[] = "3.3v";
	char pwr_param[][10] = {"ena", "3.3v"};
	Print("\nextpwr ena,3.3v");
	// DO_EXTPWR(pwr_param[0], pwr_param[1]);


	if((ret = FFWE20Cmd_EXTPWR(EXPWR_ENABLE, EXPWR_V330)) != FFWERR_OK){
		PrintErr_E20RX(ret);
		if(ret == FFWERR_BTARGET_POWERSUPPLY)
		{
			printf("[%s] Power is already supplied\n", __func__);
		}
		// return FALSE;
	}

	// 8.	FFWE20Cmd_TRANS_EML	Goes to BFW EML mode 
	char level[] = "EML";
	Print("\ntrans EML");
	DO_TRANS(level);
	

	// // 9.	FFWE20Cmd_SetHotPlug	Enables emulator to be hot-plugged	Required only when emulator is hot-plugged
	// 10.	FFWRXCmd_SetMCU	Sets per-MCU information
	Print("\nmcu IF_TYPE 1");
	// do_mcu_rx600();
	DWORD dwMcuAreaStructSize = 0x00002230;
	DWORD dwMcuInfoStructSize = 0x00001634;
	FFWRX_MCUAREA_DATA McuArea1;
	FFWRX_MCUINFO_DATA McuInfo1;
	set_McuArea(&McuArea1);
	set_McuInfo(&McuInfo1);
	
	ret = FFWRXCmd_SetMCU(dwMcuAreaStructSize, &McuArea1, dwMcuInfoStructSize, &McuInfo1);
		// PrintErr_E20RX(ret);
		printf("[%s] Returned 0x%.8x\n", __func__, ret);
		if (ret != FFWERR_FFWPARA_OVER) {
			printf("[%s] Returned FALSE\n", __func__);
		}
	setIOExtAreaData(&McuArea1);
	printf("[%s] Returned TRUE\n", __func__);


	// 11.	FFWRXCmd_SetDBG	Sets debug information
	char para[][100] = {"use", "nouse", "trace", "nouse", "normal", "0", "0", "0", "0", "debug", "off", "off", "on", "on", "dump", "non", "ena", "off"};
	Print("\ndbg use, nouse, trace, nouse, normal, 0, 0, 0, 0, debug, off, off, on, on, dump, non, ena");
	DO_DBG(para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7], para[8], para[9], para[10], para[11], para[12], para[13], para[14], para[15], para[16], para[17]);

	// 12.	FFWRXCmd_WTRLOAD	Loads internal flash write program
	char type[] = "type0";
	Print("\nwtr type0,WTRRX230.bin");
	DO_WTR( type, (char*)"WTRRX230.bin");

	// 13.	FFWE20Cmd_SetSCIBR	Sets emulator-to-MCU serial communication baud rate	
	char s_scibr[] = "2000000";
	Print("\nscibr 2000000");
	DO_SCIBR(s_scibr);

	// 14.	FFWE20Cmd_SetSCICTRL	Sets emulator’s serial communication control information	
	char s_scictrl[] = "0x0404";
	Print("\nscictrl 0404");
	DO_SCICTRL(s_scictrl);

	// 15.	FFWRXCmd_SetPMOD	Sets processor mode and endian (to specify startup mode)
// #define ENDIAN_BIG_CASE
#ifdef ENDIAN_BIG_CASE
	char pmod_para[][100] = {"sng", "r_rome", "big", "55736572", "426F6F74", "FFFFFF07", "0008C04C"};
	Print((char*)"\npmod sng, r_rome, big, 55736572, 426F6F74, FFFFFF07, 0008C04C");
#else // ENDIAN_LITTLE_CASE
	char pmod_para[][100] = {"sng", "r_rome", "little", "55736572", "426F6F74", "FFFFFF07", "0008C04C"}; 
	Print((char*)"\npmod sng, r_rome, little, 55736572, 426F6F74, FFFFFF07, 0008C04C");
#endif
	DO_PMOD(pmod_para[0], pmod_para[1], pmod_para[2], pmod_para[3], pmod_para[4], pmod_para[5], pmod_para[6]);

	// // 16.	FFWRXCmd_SetFMCD	Sets flash memory configuration data	Issuance not required *
	// 17.	FFWRXCmd_HPON	Connects with RX
	char s_plug[][10]      = {"ON"  , "ON" , "OFF"    , "OFF",     "OFF", "ON"};
	char s_condition[][20] = {"COLD", "HOT", "KEEPLOW", "KEEPOFF", "OFF", "ALREADYCONNECT"};
	Print("\nhpon");
	DO_HPON( NULL, NULL);		// Err: 0x01020005

	// Print("\nhpon on cold");
	// DO_HPON( s_plug[0], s_condition[0]);     // Plug = 0; Err: 0x01020005

	// Print("\nhpon on hot");
	// DO_HPON( s_plug[1], s_condition[1]);     // Plug = 1;
	
	// Print("\nhpon off keeplow");
	// DO_HPON( s_plug[2], s_condition[2]);     // Plug = 2;
		
	// Print("\nhpon off keepoff");
	// DO_HPON( s_plug[3], s_condition[3]);     // Plug = 3;
	
	// Print("\nhpon off off");
	// DO_HPON( s_plug[4], s_condition[4]);     // Plug = 4;
	
	// Print("\nhpon on alreadyconnect");
	// DO_HPON( s_plug[5], s_condition[5]);     // Plug = 0xff;

	// 18.	FFWRXCmd_IDCODE	Sets ID code	Required only when internal flash ROM is enabled
	Print("\nidcode FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
	DO_IDCODE((char*)"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

	// 19.	FFWRXCmd_SetPMOD	Sets processor mode and endian (to check actual settings)	
#ifdef ENDIAN_BIG_CASE
	Print((char*)"\npmod sng, r_rome, big, 55736572, 426F6F74, FFFFFF07, 0008C04C");
#else // ENDIAN_LITTLE_CASE
	Print((char*)"\npmod sng,r_rome, little,55736572,426F6F74,FFFFFF07,0008C04C");
#endif
	DO_PMOD(pmod_para[0], pmod_para[1], pmod_para[2], pmod_para[3], pmod_para[4], pmod_para[5], pmod_para[6]);

	// // 20.	FFWRXCmd_GetSTAT	Gets the target MCU status	
	// // 21.	FFWCmd_SetEXTF	Sets external ROM download information	
	// // 22.	FFWRXCmd_SetMCU	Sets per-MCU information	Issued by debugger (optional)
	// // 23.	FFWRXCmd_SetDBG	Sets debug information	Issued by debugger (optional)
	// // 24.	FFWRXCmd_SetFOVR	Specifies flash block to be overwritten during download	Changed in V.1.02 or later
	// 25.	FFWMCUCmd_CLR	Initializes emulator’s internal settings	
	Print("\nclr");
	DO_CLR();

	return 0;
}
///////////////////////////////////////////////////////////////
// - Example 1: Write a file of incremental data from 00 to the address 0 to ffff
// write 00000000_0000ffff_inc_00.mot




///////////////////////////////////////////////////////////////////////////
// Example 2: Write incremental data from 0 to F to addresses 0 to f in byte size
// write 0,f,b,0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f
static void setIOExtAreaData(FFWRX_MCUAREA_DATA* pMcuArea)
{
	DWORD	dwCnt;
	DWORD	dwCnt2;
	DWORD	dwSetCnt;
	DWORD	dwTempStart;
	DWORD	dwTempEnd;
	enum FFWENM_ENDIAN	eTempEndian;

	// I/O 領域ブロック情報の設定
	for (dwCnt = 0; dwCnt < pMcuArea->dwSfrAreaNum; dwCnt++) {
		s_madrIOExtStart[dwCnt] = pMcuArea->dwmadrSfrStartAddr[dwCnt];
		s_madrIOExtEnd[dwCnt] = pMcuArea->dwmadrSfrEndAddr[dwCnt];
		s_eIOExtEndian[dwCnt] = (enum FFWENM_ENDIAN)pMcuArea->bySfrEndian[dwCnt];
	}
	s_dwIOExtNum = pMcuArea->dwSfrAreaNum;

	// I/O 領域ブロック情報に外部領域ブロック情報を追加
	dwSetCnt = s_dwIOExtNum;
	for (dwCnt = 0; dwCnt < pMcuArea->dwExtMemBlockNum; dwCnt++) {
		if (pMcuArea->dwmadrExtMemBlockStart[dwCnt] < MCUADDR_INT_EXT) {	// CS0領域でない、すなわち、FF000000h番地以下の場合
			s_madrIOExtStart[dwSetCnt] = pMcuArea->dwmadrExtMemBlockStart[dwCnt];
			s_madrIOExtEnd[dwSetCnt] = pMcuArea->dwmadrExtMemBlockEnd[dwCnt];
			s_eIOExtEndian[dwSetCnt] = (enum FFWENM_ENDIAN)pMcuArea->byExtMemEndian[dwCnt];
			dwSetCnt++;
		}
	}
	s_dwIOExtNum = dwSetCnt;

	// 並べ替え
	if (s_dwIOExtNum >= 2) {													// １個のときソートしたくないため。０個も。
		for (dwCnt = 0; dwCnt < s_dwIOExtNum - 1; dwCnt++) {					// 範囲を狭めていく
			for (dwCnt2 = 0; dwCnt2 < s_dwIOExtNum - dwCnt - 1; dwCnt2++) {	// 先頭から順に処理
				if (s_madrIOExtStart[dwCnt2] > s_madrIOExtStart[dwCnt2 + 1]) {
				 	// 隣より大きいか？　そのときは、交換して後ろに行かせる。
					dwTempStart = s_madrIOExtStart[dwCnt2];
					dwTempEnd   = s_madrIOExtEnd[dwCnt2];
					eTempEndian = s_eIOExtEndian[dwCnt2];
					s_madrIOExtStart[dwCnt2] = s_madrIOExtStart[dwCnt2 + 1];
					s_madrIOExtEnd[dwCnt2]   = s_madrIOExtEnd[dwCnt2 + 1];
					s_eIOExtEndian[dwCnt2]   = s_eIOExtEndian[dwCnt2 + 1];
					s_madrIOExtStart[dwCnt2 + 1] = dwTempStart;
					s_madrIOExtEnd[dwCnt2 + 1]   = dwTempEnd;
					s_eIOExtEndian[dwCnt2 + 1]   = eTempEndian;
				}
			}
		}
	}

	return;
}

static void CheckEndian(ULONG start, ULONG end, ULONG *pend_endian, enum FFWENM_ENDIAN *peEndian)
{
	// start アドレスと同じエンディアンのブロック最終アドレスか end アドレスの小さい方の
	// アドレスを、*pend_endian に格納する。また、start アドレスのエンディアンを、
	// *peEndian に格納する。
	// 但し、CS0領域は、PMOD指定のエンディアンとする。

	DWORD	dwCnt;
	BOOL	bAreaChk;
	
	bAreaChk = FALSE;
	for (dwCnt = 0; dwCnt < s_dwIOExtNum; dwCnt++) {
		if (end < s_madrIOExtStart[dwCnt]) {	// madrStartAddr <= madrEndAddr < s_madrIOExtStart <= s_madrIOExtEnd
			*pend_endian = end;
			*peEndian = s_eMcuEndian;	// I/O 領域でも外部領域でない
			bAreaChk = TRUE;		// 領域判定済み
			break;
		}
		if (start < s_madrIOExtStart[dwCnt]) {	// madrStartAddr < s_madrIOExtStart <= madrEndAddr
			*pend_endian = s_madrIOExtStart[dwCnt] - 1;
			*peEndian = s_eMcuEndian;	// I/O 領域でも外部領域でない
			bAreaChk = TRUE;		// 領域判定済み
			break;
		}
		if (start <= s_madrIOExtEnd[dwCnt]) {
			if (end <= s_madrIOExtEnd[dwCnt]) {	// s_madrIOExtStart <= madrStartAddr <= madrEndAddr <= s_madrIOExtEnd
				*pend_endian = end;
				*peEndian = s_eIOExtEndian[dwCnt];	// I/O 領域または外部領域である
				bAreaChk = TRUE;			// 領域判定済み
				break;
			} else {		// s_madrIOExtStart <= madrStartAddr <= s_madrIOExtEnd < madrEndAddr
				*pend_endian = s_madrIOExtEnd[dwCnt];
				*peEndian = s_eIOExtEndian[dwCnt];	// I/O 領域または外部領域である
				bAreaChk = TRUE;			// 領域判定済み
				break;
			}
		}
	}
	if (bAreaChk != TRUE) {	// 領域判定済みでない場合(s_madrIOExtStart <= s_madrIOExtEnd < madrStartAddr <= madrEndAddr)
		*pend_endian = end;
		*peEndian = s_eMcuEndian;	// I/O 領域でも外部領域でない
	}

	return;
}

static UCHAR get_hex(char *p, long *value, short cnt)
{
	long	val;					/* 数値 */

	for(val = 0L; cnt > 0; cnt--){
		val *= 0x10L;				/* 桁上げ */
		if(isdigit(*p)){		/* 0x0 ～ 0x9 */
			val += *p - '0';
		}else if(isxdigit(*p)){	/* 0xA ～ 0xF */
			if(isupper(*p))
				val += (*p - 'A') + 0xA;
			else
				return(FALSE);		/* 小文字の場合 */
		}else{
			return(FALSE);			/* 16進数でない場合 */
		}
		p++;						/* 次の文字へ */
	}
	*value = val;					/* 変換した数値を格納 */
	return(TRUE);					/* 正常終了 */
}

static int read_data_from_file(FILE *fp, UCHAR dataCnt, char *data, UCHAR *checkSum)
{
	char	buf[0x10 * 2];		/* ファイルの入力バッファ */
	UCHAR	cnt;				/* 一度に処理するデータ数 */
	UCHAR	i;					/* 作業変数 */
	long	value;				/* 作業変数 */

	while(dataCnt > 0){
		/* 1度に処理するデータ数 */
		if(dataCnt > 0x10){
			cnt = 0x10;
		}else{
			cnt = dataCnt;
		}
		/* データの読み込み */
		if(fread(buf, 1, cnt * 2, fp) != (size_t)(cnt * 2)){
			return(FALSE);
		}
		/* データの格納 */
		for(i = 0; i < cnt; i++){
			/* データを数値変換、格納 */
			if(get_hex(&buf[i * 2], &value, 2) == FALSE){
				return(FALSE);
			}
			*data = (UCHAR)value;
			/* チェックサムに加算 */
			*checkSum += *data;
			data++;		/* 次のデータへ */
		}
		dataCnt -= cnt;		/* データ数を更新 */
	}
	return(TRUE);
}



// WRITE, CWRITE で指定するファイルの種類
#define WRITE_FILE_WRIT		0			// WRIT ファイル
#define WRITE_FILE_MOT		1			// mot ファイル

static char	cLoadMotBuf[BUFFER_MAX + 1];

class CSrecord {
// Construction
public:
	CSrecord() {
	};
	CSrecord(ULONG len, ULONG sendAddr, char *buffer) {
		m_length = len;
		m_startaddr = sendAddr; 
		m_databuffer = new BYTE[len];
		memcpy(m_databuffer, buffer, m_length);
	};
	~CSrecord() {
		// delete m_databuffer;
	};
	ULONG	m_startaddr;
	ULONG	m_length;
	BYTE	*m_databuffer;
};

static int write_change_data_rx600_big(ULONG start, ULONG end, int size, BYTE* pbyBuf, BYTE* pbyDataBuf)
{
	// ビッグエンディアン時は、pbyBuf を pbyDataBuf へ並べ替えずコピーする。
	ULONG	i;

	for (i = 0L; i <= (end - start); i++) {
		*(pbyDataBuf + i) = *(pbyBuf + i);
	}
	return TRUE;
}

static int write_change_data_rx600_little(ULONG start, ULONG end, int size, BYTE* pbyBuf, BYTE* pbyDataBuf)
{
	// リトルエンディアン時は、pbyBuf を pbyDataBuf へ並べ替えてコピーする。
	ULONG	i, j;
	ULONG	size_Data;
	ULONG	TmpSize;
	BYTE	DwnpDataTemp[4];
	
	if(size == MBYTE_ACCESS){
		size_Data = 1L;
	} else if(size == MWORD_ACCESS){
		size_Data = 2L;
	} else if(size == MLWORD_ACCESS){
		size_Data = 4L;
	}

	TmpSize = end - start + 1;
	for (i = 0L; i <= (end - start); ) {
		for (j = 0L; j < size_Data; j++) {
			DwnpDataTemp[j] = *(pbyBuf + i + j);						// ReadDataTemp[] へ一旦退避
		}
		for (j = 0L; j < size_Data; j++) {
			if (TmpSize >=  size_Data) {
				// アクセスサイズの倍数データの並び替え
				*(pbyDataBuf + i + (size_Data - 1L) - j) = DwnpDataTemp[j];
			} else {
				// アクセスサイズの倍数でないデータの並び替え
				*(pbyDataBuf + i + j) = DwnpDataTemp[j];
			}
		}
		i = i + size_Data;
		if (TmpSize >=  size_Data) {
			TmpSize -= size_Data;
		}
		if( i == 0x00000000 ){		// i が0x00000000になったら強制的に終了
			break;					// ここでBreakしないと、i が 0 になってしまい、
		}							// (end - start) と比較してfor文から抜けられなくなる。
	}
	return TRUE;
}

#define WRITE_FILE_READ_FGETS	TRUE
#define WRITE_FILE_EVAL			FALSE
#define WRITE_PARAMMAX			20		// RevNo110915_004 Append Line
#define WRITE_SET_DATA_MAX		16		// RevNo110915_004 Append Line

static int load_mot_e20(FILE *fp, std::vector<CSrecord> &m_CsRecord, int mode)
{
	UCHAR	recType;	/* [ レコードタイプ ] */
	UCHAR	dataCnt;	/* [ データ数 ]       */
	ULONG	offset;		/* [ 先頭アドレス ]   */
	UCHAR	checkSum;	/* [ チェックサム ]   */
	char	buf[128];	/* ファイルの入力バッファ */
	long	value;		/* 作業変数 */
	UCHAR	sum;		/* チェックサム */
	ULONG	addr;		/* 開始アドレス */
	ULONG	sendAddr;	/* [ 開始アドレス ] */
	UINT	sendLength;	/* [ レングス ]     */
	int		dataNum, i;

	if(mode == 1) {
		if(fgets((char *)buf, 128, fp) == NULL){
			Print("Error: Format error.");
			return(TRUE);
		}
	}

	/* Check S0 start record */
	if(fgets((char *)buf, 128, fp) == NULL){
		Print("Error: Format error.");
		return(TRUE);
	}
	printf("[%s] read 1st line \"%s\"\n", __func__, buf);
	/* 初期化処理 */
	sendAddr = (ULONG) -1L;		/* [ 開始アドレス ] */
	sendLength = 0;		/* [ レングス ]     */
	do{
		sum = 0;	/* 初期化 */

		/* スタートマーク、レコードタイプ、　*/
		/* データ数の読み込み		     */
		if(fread(buf, 1, 1 + 1 + 2 , fp) != 4){
			Print("Error: Format error.");
			printf("[%s] line %d\n", __func__, __LINE__);
			return(TRUE);
		}
		/* スタートマークのチェック */
		if(buf[0] != 'S'){
			Print("Error: Format error. 'S' is not star character");
			printf("[%s] line %d\n", __func__, __LINE__);
			return(TRUE);
		}
		/* レコードタイプ、データ数の取得 */
		if(get_hex(&buf[1], &value, 1 + 2) == FALSE){
			Print("Error: Format error.");
			printf("[%s] line %d\n", __func__, __LINE__);
			return(TRUE);
		}	
		recType = (UCHAR)(value >> 8);
		dataCnt = (UCHAR)(value >> 0);

		switch(recType){
			case 0x00:
				if(dataCnt != 0x03){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
		   		/* 先頭アドレス の読み込み */
		    	if(fread(buf, 1, 4, fp) != 4){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
		    	/* 先頭アドレスの取得 */
		    	if(get_hex(buf, &value, 4) == FALSE){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		offset  = (UINT)(value);
				if(offset != 0x0000){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
				dataNum = (int)((dataCnt - 3) * 2);
				if(fread(buf, 1, dataNum, fp) != (size_t)dataNum){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
				for(i = 0; i < dataNum; i += 2){
		    		if(get_hex(&buf[i], &value, 2) == FALSE){
						Print("Error: フォーマットエラー。");
						return(TRUE);
					}
					sum += (UCHAR)(value & 0xff);
				}
				break;
			case 0x01:
		   		/* 先頭アドレス の読み込み */
	    		if(fread(buf, 1, 4, fp) != 4){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		/* 先頭アドレスの取得 */
	    		if(get_hex(buf, &value, 4) == FALSE){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		offset  = (UINT)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 8);
	    		sum += (UCHAR)(offset >> 0);
				addr = offset;	/* 開始アドレス */
				/************** [ MONPコマンドの送信 ] **************/
				if(addr != (sendAddr + sendLength)){
					CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
					m_CsRecord.push_back(csrd);
					sendAddr = addr;
					sendLength = 0;
				}
				if((sendLength + (dataCnt-(2+1))) > BUFFER_MAX){
					CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
					m_CsRecord.push_back(csrd);
					sendAddr += sendLength;
					sendLength = 0;
				}
				/****************************************************/
				/* データの読み込み、格納 */
				if(read_data_from_file(fp, (dataCnt-(2+1)), &cLoadMotBuf[sendLength], &sum) != TRUE){
					return(FALSE);
				}
				/* バッファのデータ数の更新 */
				sendLength += (dataCnt-(2+1));
				break;
			case 0x02:
		   		/* 先頭アドレス の読み込み */
	    		if(fread(buf, 1, 6, fp) != 6){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		/* 先頭アドレスの取得 */
	    		if(get_hex(buf, &value, 6) == FALSE){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		offset  = (ULONG)value;
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 16);
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				addr = offset;	/* 開始アドレス */
				/************** [ MONPコマンドの送信 ] **************/
				if(addr != (sendAddr + sendLength)){
					CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
					m_CsRecord.push_back(csrd);
					sendAddr = addr;
					sendLength = 0;
				}
				if((sendLength + (dataCnt-(3+1))) > BUFFER_MAX){
					CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
					m_CsRecord.push_back(csrd);
					sendAddr += sendLength;
					sendLength = 0;
				}
				/****************************************************/
				/* データの読み込み、格納 */
				if(read_data_from_file(fp, (dataCnt-(3+1)), &cLoadMotBuf[sendLength], &sum) != TRUE){
					return(FALSE);
				}
				/* バッファのデータ数の更新 */
				sendLength += (dataCnt-(3+1));
				break;
			case 0x03:
	    		/* Load start address */
	    		if(fread(buf, 1, 8, fp) != 8){
					Print("Error: Format error. Load start address failed");
					printf("[%s] line %d\n", __func__, __LINE__);
					return(TRUE);
				}
				// printf("[%s] addr [0x%c%c%c%c%c%c%c%c]\n", __func__, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
		   		/* Get address */
	    		if(get_hex(buf, &value, 8) == FALSE){
					Print("Error: Format error. Get address failed");
					printf("[%s] line %d\n", __func__, __LINE__);
					return(TRUE);
				}
	    		offset  = (ULONG)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 24);
	    		sum += (UCHAR)(offset >> 16);
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				addr = offset;	/* 開始アドレス */

				if(addr != (sendAddr + sendLength)){
					CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
					m_CsRecord.push_back(csrd);
					sendAddr = addr;
					sendLength = 0;
				}
				if((sendLength + (dataCnt-(4+1))) > BUFFER_MAX){					
					CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
					m_CsRecord.push_back(csrd);
					sendAddr += sendLength;
					sendLength = 0;
				}
				/****************************************************/
				/* データの読み込み、格納 */
				if(read_data_from_file(fp, (dataCnt-(4+1)), &cLoadMotBuf[sendLength], &sum) != TRUE){
					printf("[%s] read data from file FALSE, line %d\n", __func__, __LINE__);	
					return(FALSE);
				}
				// printf("[%s] data \"%s\"\n", __func__, cLoadMotBuf);
				sendLength += (dataCnt-(4+1));
				break;
			case 0x07:
				if(dataCnt != 0x05){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		/* 先頭アドレス の読み込み */
	    		if(fread(buf, 1, 8, fp) != 8){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		/* 先頭アドレスの取得 */
	    		if(get_hex(buf, &value, 8) == FALSE){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		offset  = (ULONG)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 24);
	    		sum += (UCHAR)(offset >> 16);
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				{
				CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
				m_CsRecord.push_back(csrd);
				// printf("[%s] data \"%s\"\n", __func__, cLoadMotBuf);
				}
				break;
			case 0x08:
				if(dataCnt != 0x04){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		/* 先頭アドレス の読み込み */
	    		if(fread(buf, 1, 6, fp) != 6){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		/* 先頭アドレスの取得 */
	    		if(get_hex(buf, &value, 6) == FALSE){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		offset  = (ULONG)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 16);
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				{
				CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
				m_CsRecord.push_back(csrd);
				}
				break;
			case 0x09:
				if(dataCnt != 0x03){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
		   		if(fread(buf, 1, 4, fp) != 4){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		if(get_hex(buf, &value, 4) == FALSE){
					Print("Error: フォーマットエラー。");
					return(TRUE);
				}
	    		offset  = (UINT)(value);
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				{
				CSrecord csrd = CSrecord(sendLength, sendAddr, cLoadMotBuf);
				m_CsRecord.push_back(csrd);
				}
				break;
			default:
				printf("[%s] Invalid recType (0x%02x), line %d\n", __func__, recType, __LINE__);
				return(TRUE);
		}
		if(fread(buf, 1, 2, fp) != 2){
			Print("Error: Format error.");
			printf("[%s] line %d\n", __func__, __LINE__);
			return(TRUE);
		}
		if(get_hex(buf, &value, 2) == FALSE){
			Print("Error: Format error.");
			printf("[%s] line %d\n", __func__, __LINE__);
			return(TRUE);
		}
		checkSum = (UCHAR)value;
		/* チェックサムのチェック */
		sum = (UCHAR)(~sum);		/* 1の補数 */
		if(checkSum != sum){
			Print("Error: Format error.");
			printf("[%s] line %d\n", __func__, __LINE__);
			return(TRUE);
		}
		/* 改行文字の読み込みとチェック */
		// For Linux : CRLF , CR=13, LF=10='\n' 
		fgetc(fp); // get CR character
		if(fgetc(fp) != '\n'){ // LF character
			Print("Error: Format error. Not end by '\\n' character");
			return(TRUE);
		}
	}while((recType != 0x07) && (recType != 0x08) && (recType != 0x09));	/* エンドレコードまで */
	
	/* EOFのチェック */
	if(fread(buf, 1, 1, fp) == 1){
		Print("Error: Format error. Not EOF character");
		return(TRUE);
	}
	printf("[%s] Return TRUE, line %d\n", __func__, __LINE__);
	return TRUE;
}

static int do_write_rx600(char *pa[])
{
	FILE	*fp;
	DWORD	Saddr, Eaddr, total, Ret;
	int		i;
	int		j = 0;
	DWORD	dwData[16];
	char	buffer[BUFF_SIZE + 1];
	char	tmp[BUFF_SIZE];
	int		nLine;
	int		nDataCnt = 0;
// RevNo110915_005 Append Start
	int		nTmpLine;
	ULONG	end_endian;
	FFWENM_ENDIAN	eEndian;
	DWORD	dwHasu;
	DWORD	dwCnt;
	DWORD	dwShift;
// RevNo110915_005 Append End
	BOOL	bLittleEndian;
	FFWENM_MACCESS_SIZE size;
	FFWENM_VERIFY_SET vfy;
	FFW_VERIFYERR_DATA vfydata;
#if (WRITE_FILE_READ_FGETS==TRUE)
	BOOL	bExistInfo;
	DWORD	dwNum;
	int		nCnt;
	char	buff1[BUFF_SIZE + 1];
	char	buff2[BUFF_SIZE + 1];
#else
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	char	szFilename[_MAX_FNAME];
	char	szExt[_MAX_EXT];
	LONG	dsize;
	char	szFlag[8];
	char	work[BUFF_SIZE + 1];
	DWORD	cnt;
	char	szBuff[_MAX_FNAME];
	char	szTmp[_MAX_FNAME];
#endif
#if (WRITE_FILE_EVAL==TRUE)
	char	buff[16][64];
#endif
	UCHAR *Wdata = NULL;
	BOOL	bEaddrSet = FALSE;

	char	chbuff[16][64];
	int		nWdataLen;
	int		nVfyPos = 0;
// RevNo110915_004 Append Start
	int		nDataCntStart = 0;
	DWORD	dwStartTmp;
	int		nDataSize = 0;
// RevNo110915_004 Append End

// RevNo110915_007 Append Start
	char	cExtent[10];
	long	lLast;
	int		nFormat;
	int		nRet;
	DWORD	dwEndAddr;
	BYTE*	pbyWriteBuf;
	// CPtrArray	m_CsRecord;
	std::vector<CSrecord> m_CsRecord;
	CSrecord	*tmpCSrd;
	ULONG	start_temp, end_temp;
	ULONG	start_remainder;	// start アドレスをアクセスサイズで割った余り
	ULONG	Length_temp;
	BYTE*	pbyBuf;
	BYTE*	pbyDataBuf;
	
	
	
	vfy = VERIFY_OFF;
	for (i = 1; i < WRITE_PARAMMAX; i++) {
		if (pa[i] != NULL) {
			_strupr(pa[i]);
			if (Strcmp("VF_ON", pa[i]) == 0) {
				nVfyPos = i;
				vfy = VERIFY_ON;
				break;
			} else if(Strcmp("VF_OFF", pa[i]) == 0) {
				nVfyPos = i;
				vfy = VERIFY_OFF;
				break;
			} else {
				vfy = VERIFY_OFF;
			}
		}
	}
	
	// WRITE from file name
	if ((pa[2] == NULL) || (nVfyPos == 2)) {
		printf("Write from .MOT file\n");
		lLast = strlen(pa[0]);
		for(i = 1; i < lLast; i++){
			if((pa[0][lLast - i] == ' ') || (pa[0][lLast - i] == '\t')){
				pa[0][lLast - i] = '\0';
			}
		}

		lLast = strlen(pa[0]);
		strcpy(cExtent, &pa[0][lLast - 4]);
		_strupr(cExtent);
		nFormat = WRITE_FILE_WRIT;
		if(strcmp(cExtent, ".MOT") == 0){
			nFormat = WRITE_FILE_MOT;
		}

		// strcpy(pa[0], GetInPath(pa[0]));
		if((fp = fopen(pa[0], "rt")) == NULL){		// ファイルの存在チェック
			sprintf(buffer, "Error: %sがオープンできません。", pa[0]);
			Print(buffer);
			return FALSE;
		}

		// アクセスサイズ指定の確認
		if ((pa[1] == NULL) || (nVfyPos == 1)) {
			size = MBYTE_ACCESS;
		} else {
			_strupr(pa[1]);
			if(Strcmp("B", pa[1]) == 0){
				size = MBYTE_ACCESS;
			}else if(Strcmp("W", pa[1]) == 0){
				size = MWORD_ACCESS;
			}else if(Strcmp("L", pa[1]) == 0){
				size = MLWORD_ACCESS;
			}else{
				Print("Error: アクセスサイズが異常です。");
				fclose(fp);
				return FALSE;
			}
		}

		if (nFormat == WRITE_FILE_MOT) {
			nRet = load_mot_e20(fp, m_CsRecord, 0);
			printf("[%s] load_mot_e20 return 0x%08x, line %d\n", __func__, nRet, __LINE__);
			fclose(fp);

			for(i = 0; i<m_CsRecord.size(); i++) {
				tmpCSrd = &m_CsRecord.at(i);
				printf("[%s] m_CsRecord.at(%d).m_length %ld bytes\n", __func__, i, tmpCSrd->m_length);
				if(tmpCSrd->m_length > 0) {
					if ((tmpCSrd->m_length == 0L) || (tmpCSrd->m_length+4L < 4L)) {	// tmpCSrd->m_length が 0 または 4 足してオーバーフローする場合
						//VS2008へ移行すると0x7FFFFFFFを超える配列確保ができない。
						//サイズが0x7FFFFFFFを超える場合はエラーを返すように改定する。
						if(0x7FFFFFFF < tmpCSrd->m_length){
							Print("Error: 一度に0x7FFFFFFFを超える範囲は指定できません。");
							return(FALSE);
						}
						pbyDataBuf = new BYTE[0x7FFFFFFF];	//new BYTE[0x100000000];
					} else {
						pbyDataBuf = new BYTE[tmpCSrd->m_length+4];
					}

					dwEndAddr = tmpCSrd->m_startaddr + tmpCSrd->m_length - 1;

					pbyBuf = tmpCSrd->m_databuffer;
					
					if(size == MBYTE_ACCESS){			// バイト長
						start_remainder = 0;
					}else if(size == MWORD_ACCESS){		// ワード長
						start_remainder = tmpCSrd->m_startaddr % 2;
					}else if(size == MLWORD_ACCESS){		// ロングワード長
						start_remainder = tmpCSrd->m_startaddr % 4;
					}
					for (start_temp = tmpCSrd->m_startaddr; start_temp <= dwEndAddr; ) {
						CheckEndian(start_temp, dwEndAddr, &end_endian, &eEndian);	// start_temp のエンディアンをeEndian に格納、
																					// start_temp と同じエンディアンブロックで、
																					// dwEndAddr までで最後のアドレスをend_endian で返す。
						end_temp = end_endian + start_remainder;					// スタートアドレスの余りをend_endian に足す。
						if (end_temp < end_endian) {								// end_temp がオーバーフローした場合は
							end_temp = 0xFFFFFFFF;									// end_temp を 0xFFFFFFFF にする。
						}
						if (dwEndAddr < end_temp) {									// dwEndAddr の方が end_temp より小さい場合は
							end_temp = dwEndAddr;									// end_temp を dwEndAddr にする。
						}
						if (eEndian == ENDIAN_LITTLE) {
							write_change_data_rx600_little(start_temp, end_temp, size, 
														  pbyBuf + start_temp - tmpCSrd->m_startaddr, 
														  pbyDataBuf + start_temp - tmpCSrd->m_startaddr);
						} else {
							write_change_data_rx600_big(start_temp, end_temp, size,  
														pbyBuf + start_temp - tmpCSrd->m_startaddr, 
														pbyDataBuf + start_temp - tmpCSrd->m_startaddr);
						}
						
						Length_temp = end_temp - start_temp + 1L;
						start_temp += Length_temp;
						if( start_temp == 0x00000000 ){		// start_temp が0x00000000になったら強制的に終了
							break;							// ここでBreakしないと、start_temp が 0 になってしまい、
						}									// end と比較してfor文から抜けられなくなる。
					}
					
					pbyWriteBuf = pbyDataBuf;
					memset(&vfydata, 0, sizeof(FFW_VERIFYERR_DATA));	// 初期化
					printf("[%s] Write %ld bytes => [0x%08lx:0x%08x]\n", __func__, tmpCSrd->m_length, tmpCSrd->m_startaddr, dwEndAddr);
					
					Ret = FFWMCUCmd_WRITE(tmpCSrd->m_startaddr, dwEndAddr, vfy, size, pbyWriteBuf, &vfydata);
					delete [] pbyDataBuf;
					if(Ret != FFWERR_OK){
						PrintErr_E20RX(Ret);
						return FALSE;
					}
					if(vfydata.eErrorFlag == VERIFY_ERR){		// ベリファイエラー発生
						if (vfydata.eAccessSize == MBYTE_ACCESS) {
							sprintf(buffer, "Verify Error : Address = %08XH, Write = %02XH, Read = %02XH", 
								vfydata.dwErrorAddr, vfydata.dwErrorWriteData, vfydata.dwErrorReadData);
						} else if (vfydata.eAccessSize == MWORD_ACCESS) {
							sprintf(buffer, "Verify Error : Address = %08XH, Write = %04XH, Read = %04XH", 
								vfydata.dwErrorAddr, vfydata.dwErrorWriteData, vfydata.dwErrorReadData);
						} else if (vfydata.eAccessSize == MLWORD_ACCESS) {
							sprintf(buffer, "Verify Error : Address = %08XH, Write = %08XH, Read = %08XH", 
								vfydata.dwErrorAddr, vfydata.dwErrorWriteData, vfydata.dwErrorReadData);
						}
						Print(buffer);
						break;
					}
				}
			}

		} else {
			Print("Error: Write from .MOT file");
			return FALSE;
		}
// RevNo110915_007 Modify End

	} 

	else
	// WRITE individually
	{
		printf("WRITE individually\n");
		Saddr = strtol( pa[0], (char **)NULL, 16 );
		Eaddr = strtol( pa[1], (char **)NULL, 16 );
		printf("[%s] pa[0] 0x%s\n", __func__, pa[0]);
		printf("[%s] pa[1] 0x%s\n", __func__, pa[1]);
		if (Saddr > Eaddr) {
			Print("Error: The start address is larger than the end address.");
			return FALSE;
		}

		if (nVfyPos == 0) {
			nVfyPos = WRITE_PARAMMAX - 1;
		}

		_strupr(pa[2]);
		printf("[%s] pa[2] %s\n", __func__, pa[2]);
		if(Strcmp("B", pa[2]) == 0){
			size = MBYTE_ACCESS;
			nDataCntStart = 3;
			Wdata = new UCHAR[WRITE_SET_DATA_MAX];
		}else if(Strcmp("W", pa[2]) == 0){
			size = MWORD_ACCESS;
			nDataCntStart = 3;
			Wdata = new UCHAR[WRITE_SET_DATA_MAX * 2];
		}else if(Strcmp("L", pa[2]) == 0){
			size = MLWORD_ACCESS;
			nDataCntStart = 3;
			Wdata = new UCHAR[WRITE_SET_DATA_MAX * 4];
		}else{
			size = MBYTE_ACCESS;
			nDataCntStart = 2;
			Wdata = new UCHAR[WRITE_SET_DATA_MAX];
		}

		for (i = nDataCntStart; i < nVfyPos; i++) {
			if (pa[i] != NULL) {
				if (size == MBYTE_ACCESS) {
					nDataSize++;
				} else if (size == MWORD_ACCESS) {
					nDataSize += 2;
				} else {
					nDataSize += 4;
				}
				nDataCnt++;
			} else {
				break;
			}
		}

		if (size == MBYTE_ACCESS) {
			if ((UCHAR)(Eaddr - Saddr + 1) != nDataSize) {
				Print("Error: アドレス設定が設定データ数と合っていません。");
				delete [] Wdata;
				return FALSE;
			}
		} else if (size == MWORD_ACCESS) {
			if (((UCHAR)(Eaddr - Saddr + 1) != nDataSize) && ((UCHAR)(Eaddr - Saddr + 1) != (nDataSize - 1))) {
				Print("Error: アドレス設定が設定データ数と合っていません。");
				delete [] Wdata;
				return FALSE;
			}
		} else {
			if (((UCHAR)(Eaddr - Saddr + 1) != nDataSize) && ((UCHAR)(Eaddr - Saddr + 1) != (nDataSize - 1)) && 
				((UCHAR)(Eaddr - Saddr + 1) != (nDataSize - 2)) && ((UCHAR)(Eaddr - Saddr + 1) != (nDataSize - 3))) {
				Print("Error: アドレス設定が設定データ数と合っていません。");
				delete [] Wdata;
				return FALSE;
			}
		}

		dwStartTmp = Saddr;
		for (i = nDataCntStart, j = 0; j < nDataCnt; i++, j++) {
			dwData[j] = strtoul(pa[i], NULL, 16);
			printf("[%s] pa[%d] 0x%s\n", __func__, i, pa[i]);
			if(size == MBYTE_ACCESS){
				Wdata[j] = (UCHAR)dwData[j];
			}else if(size == MWORD_ACCESS){
				if ((Eaddr - dwStartTmp + 1) >= 2) {
					Wdata[(j * 2)] = (UCHAR)((dwData[j] & 0xFF00) >> 8);
					Wdata[(j * 2) + 1] = (UCHAR)(dwData[j] & 0xFF);
					dwStartTmp += 2;
				} else {
					Wdata[(j * 2)] = (UCHAR)(dwData[j] & 0xFF);
				}
			}else if(size == MLWORD_ACCESS){
				if ((Eaddr - dwStartTmp + 1) >= 4) {
					Wdata[(j * 4)]	 = (UCHAR)((dwData[j] & 0xFF000000) >> 24);
					Wdata[(j * 4) + 1] = (UCHAR)((dwData[j] & 0x00FF0000) >> 16);
					Wdata[(j * 4) + 2] = (UCHAR)((dwData[j] & 0x0000FF00) >> 8);
					Wdata[(j * 4) + 3] = (UCHAR)(dwData[j] & 0xFF);
					dwStartTmp += 4;
				} else {
					CheckEndian(dwStartTmp, dwStartTmp, &end_endian, &eEndian);
					dwHasu = Eaddr - dwStartTmp + 1;
					dwShift = 0x000000FF;
					if (eEndian == ENDIAN_LITTLE) {
						for (dwCnt = 0; dwCnt < dwHasu; dwCnt++) {
							Wdata[(j * 4) + dwCnt] = (UCHAR)((dwData[j] >> (dwCnt * 8)) & dwShift);
						}
					} else {
						for (dwCnt = 0; dwCnt < dwHasu; dwCnt++) {
							Wdata[(j * 4) + dwCnt] = 
								(UCHAR)((dwData[j] >> ((dwHasu - (dwCnt + 1)) * 8)) & dwShift);
						}
					}
				}
			}
		}

		memset(&vfydata, 0, sizeof(FFW_VERIFYERR_DATA));

		Ret = FFWMCUCmd_WRITE(Saddr, Eaddr, vfy, size, Wdata, &vfydata);
		if(Ret != FFWERR_OK){
			PrintErr_E20RX(Ret);
			if( (Ret == FFWERR_RAM_WAIT_TMP_CHANGE) || (Ret == FFWERR_ROM_WAIT_TMP_CHANGE) ){
			}else{
				delete [] Wdata;
				return FALSE;
			}
		}
		if(vfydata.eErrorFlag == VERIFY_ERR){		// ベリファイエラー発生
			if (vfydata.eAccessSize == MBYTE_ACCESS) {	// RevNo111128-001 Modify Line
				sprintf(buffer, "Verify Error : Address = %08XH, Write = %02XH, Read = %02XH", 
					vfydata.dwErrorAddr, vfydata.dwErrorWriteData, vfydata.dwErrorReadData);
			} else if (vfydata.eAccessSize == MWORD_ACCESS) {	// RevNo111128-001 Modify Line
				sprintf(buffer, "Verify Error : Address = %08XH, Write = %04XH, Read = %04XH", 
					vfydata.dwErrorAddr, vfydata.dwErrorWriteData, vfydata.dwErrorReadData);
			} else if (vfydata.eAccessSize == MLWORD_ACCESS) {	// RevNo111128-001 Modify Line
				sprintf(buffer, "Verify Error : Address = %08XH, Write = %08XH, Read = %08XH", 
					vfydata.dwErrorAddr, vfydata.dwErrorWriteData, vfydata.dwErrorReadData);
			}
			Print(buffer);
		}

		delete [] Wdata;
	}

	printf("[%s] Returned TRUE\n", __func__);
	return TRUE;
}

int DO_WRITE(char *p1, char *p2, char *p3, char *p4, char *p5, char *p6, char *p7, char *p8, char *p9, char *p10,
		char *p11, char *p12, char *p13, char *p14, char *p15, char *p16, char *p17, char *p18, char *p19, char *p20)
// int DO_WRITE(char *filename, char *Size, char *Verify, char *Endian)
{
	char* pa[20];

	// if ((PROT_EMU_TYPE() == EMU_E20) || (PROT_EMU_TYPE() == EMU_E2)) {
		pa[0] = p1;
		pa[1] = p2;
		pa[2] = p3;
		pa[3] = p4;
		pa[4] = p5;
		pa[5] = p6;
		pa[6] = p7;
		pa[7] = p8;
		pa[8] = p9;
		pa[9] = p10;
		pa[10] = p11;
		pa[11] = p12;
		pa[12] = p13;
		pa[13] = p14;
		pa[14] = p15;
		pa[15] = p16;
		pa[16] = p17;
		pa[17] = p18;
		pa[18] = p19;
		pa[19] = p20;
		return(do_write_rx600(pa));
	// } else {
		// Print("Error: WRITEコマンドはE100/E20専用です｡");
	// }

	// return FALSE;
}

///////////////////////////////////////////////////////////////
// 2. Target memory read (FFWMCUCmd_DUMP)
// [NDEB Command]
// - Example 1(Read data of address 0 to ff in byte size)
// >dump b, 0,ff
// - Example 2(Read data of address 0 to 3f in word size)
// >dump w,0,3f
// - Example 3(Read data of address 0 to 3f in lword size)
// >dump l,0,3f

// static ULONG McuMaxAddr(void)
// {
// 	return(pView->McuMaxAddr);
// }

ULONG	DUMP_SADDR = 0L;
static int dump_disp_rx600(ULONG start, ULONG end, int size, BYTE* ReadData, int skip)
{
//	ULONG	end, dumpAddr, dumpEnd;
	ULONG	dumpAddr, dumpEnd;
	int		i, bufCnt;
	UCHAR	Data;
	char	buffer[BUFF_SIZE + 1], tmp[BUFF_SIZE + 1], ascii[20];
	ULONG	pos = 0;
	ULONG	base;

	if(size == MBYTE_ACCESS){
		// バイト長
		Print("ADDRESS    +0 +1 +2 +3 +4 +5 +6 +7   +8 +9 +A +B +C +D +E +F   ASCII");
//		end = start + Length - 1L;
	}else if(size == MWORD_ACCESS){
		//ワード長
		Print("ADDRESS    +1+0 +3+2 +5+4 +7+6   +9+8 +B+A +D+C +F+E   ASCII");
//		end = start + Length * 2L - 1L;
	}else if(size == MLWORD_ACCESS){
		//ロングワード長
		Print("ADDRESS    +3+2+1+0 +7+6+5+4   +B+A+9+8 +F+E+D+C   ASCII");
//		end = start + Length * 4L - 1L;
	}

	if (skip == 1) {
		// 表示範囲の取得
		dumpAddr = start;
		dumpEnd  = end + 0xF;

		base = dumpAddr & 0xF;

		buffer[0] = '\0';
		bufCnt = 0;

		for( i = 0; dumpAddr <= dumpEnd; dumpAddr++){
			// 開始アドレスの表示
			if((dumpAddr & 0xF) == base){
				sprintf(tmp, "%08lX  ", dumpAddr);
				strcat(buffer, tmp);
			}
			// スペース( +7 ～ +8 番地の間 )の表示
			if((dumpAddr & 0xF) == ((base+8) & 0xF)){
				strcat(buffer, "  ");
			}

			// << メモリデータの読み込み/表示 >>
			if((dumpAddr >= start) && (dumpAddr <= end)){
				// メモリデータの表示
				Data = *(ReadData + bufCnt);
				bufCnt++;
				if(size == MBYTE_ACCESS){
					// バイト長
					sprintf(tmp, " %02hX", Data & 0xFF);
					strcat(buffer, tmp);
				}else if(size == MWORD_ACCESS){
					// ワード長
					if(((dumpAddr - start) % 2) == 0){
						// 上位バイト
						sprintf(tmp, " %02hX", Data & 0xFF);
						strcat(buffer, tmp);
					}else{
						// 下位バイト
						sprintf(tmp, "%02hX", Data & 0xFF);
						strcat(buffer, tmp);
					}
				}else if(size == MLWORD_ACCESS){
					// ロング長
					if(((dumpAddr - start) % 4) == 0){
						// 最上位バイト
						sprintf(tmp, " %02hX", Data & 0xFF);
						strcat(buffer, tmp);
					}else{
						// 最上位バイト以外
						sprintf(tmp, "%02hX", Data & 0xFF);
						strcat(buffer, tmp);
					}
				}
			}else{
				// 指定範囲外の領域を表示
				Data =  ' ';	// ASCII表示用
				if(size == MBYTE_ACCESS){
					// バイト長 
					strcat(buffer, "   ");
				}else if(size == MWORD_ACCESS){
					// ワード長
					if(((dumpAddr - start) % 2) == 0){
						// 上位バイト
						strcat(buffer, "   ");
					}else{
						// 下位バイト
						strcat(buffer, "  ");
					}
				}else if(size == MLWORD_ACCESS){
					// ワード長
					if(((dumpAddr - start) % 4) == 0){
						// 上位バイト
						strcat(buffer, "   ");
					}else{
						// 下位バイト
						strcat(buffer, "  ");
					}
				}
			}
			// << メモリデータのASCII表示 >>
			if(!isprint(Data)){					// 非表示文字の場合
				ascii[i++] = '.';
			}else{
				ascii[i++] = Data;
			}
			if((dumpAddr & 0xF) == ((base+0xF) & 0xF)){		// XXXXXXXF番地の場合
				ascii[i++] = '\0';
				sprintf(tmp, "   %s", ascii);		// ASCII表示
				strcat(buffer, tmp);
				Print(buffer);
				buffer[0] = '\0';
				i = 0;
			}
			if( dumpAddr == 0xFFFFFFFF ){		// ダンプアドレスが0xFFFFFFFFになったら強制的に終了
				break;							// ここでBreakしないと、dumpAddr++でdumpAddrが0になってしまい、
			}									// dumpEndと比較してfor文から抜けられなくなる。
		}
	} else {
		// 表示範囲の取得
		dumpAddr = start & 0xFFFFFFF0L;	// XXXXXXX0番地にマスク
		dumpEnd  = end  | 0x0000000FL;	// XXXXXXXF番地にマスク

		base = dumpAddr;

		buffer[0] = '\0';
		bufCnt = 0;

		for( i = 0; dumpAddr <= dumpEnd; dumpAddr++){
			// 開始アドレスの表示
			if((dumpAddr & 0xF) == 0){
				sprintf(tmp, "%08lX  ", dumpAddr);
				strcat(buffer, tmp);
			}
			// スペース( +7 ～ +8 番地の間 )の表示
			if((dumpAddr & 0xF) == 8){
				strcat(buffer, "  ");
			}

			// << メモリデータの読み込み/表示 >>
			if((dumpAddr >= start) && (dumpAddr <= end)){
				// メモリデータの表示
				Data = *(ReadData + bufCnt);
				bufCnt++;
				if(size == MBYTE_ACCESS){
					// バイト長
					sprintf(tmp, " %02hX", Data & 0xFF);
					strcat(buffer, tmp);
				}else if(size == MWORD_ACCESS){
					// ワード長
					if(((dumpAddr - base) % 2) == 0){
						// 上位バイト
						sprintf(tmp, " %02hX", Data & 0xFF);
						strcat(buffer, tmp);
					}else{
						// 下位バイト
						sprintf(tmp, "%02hX", Data & 0xFF);
						strcat(buffer, tmp);
					}
				}else if(size == MLWORD_ACCESS){
					// ロング長
					if(((dumpAddr - base) % 4) == 0){
						// 最上位バイト
						sprintf(tmp, " %02hX", Data & 0xFF);
						strcat(buffer, tmp);
					}else{
						// 最上位バイト以外
						sprintf(tmp, "%02hX", Data & 0xFF);
						strcat(buffer, tmp);
					}
				}
			}else{
				// 指定範囲外の領域を表示
				Data =  ' ';	// ASCII表示用
				if(size == MBYTE_ACCESS){
					// バイト長 
					strcat(buffer, "   ");
				}else if(size == MWORD_ACCESS){
					// ワード長
					if(((dumpAddr - base) % 2) == 0){
						// 上位バイト
						strcat(buffer, "   ");
					}else{
						// 下位バイト
						strcat(buffer, "  ");
					}
				}else if(size == MLWORD_ACCESS){
					// ワード長
					if(((dumpAddr - base) % 4) == 0){
						// 上位バイト
						strcat(buffer, "   ");
					}else{
						// 下位バイト
						strcat(buffer, "  ");
					}
				}
			}
			// << メモリデータのASCII表示 >>
			if(!isprint(Data)){					// 非表示文字の場合
				ascii[i++] = '.';
			}else{
				ascii[i++] = Data;
			}
			if((dumpAddr & 0xF) == 0xF){		// XXXXXXXF番地の場合
				ascii[i++] = '\0';
				sprintf(tmp, "   %s", ascii);		// ASCII表示
				strcat(buffer, tmp);
				Print(buffer);
				buffer[0] = '\0';
				i = 0;
			}
			if( dumpAddr == 0xFFFFFFFF ){		// ダンプアドレスが0xFFFFFFFFになったら強制的に終了
				break;							// ここでBreakしないと、dumpAddr++でdumpAddrが0になってしまい、
			}									// dumpEndと比較してfor文から抜けられなくなる。
		}
	}

	return TRUE;
}

static int do_dump_rx600(char *b_w, char *sa_addr, char *e_addr, char *skip)
{
	int		ret;

	// if((ret = PROT_Init()) != TRUE){
	// 	PrintErr(ret);
	// 	return(FALSE);
	// }

	ULONG	Saddr, Eaddr, Length, Ret;
	int		Mode;
	FFWENM_MACCESS_SIZE	Size;
	BYTE	*ReadData = NULL;	// RevNo110915_002 Append Line

	if(b_w == NULL){
		Print("Error: データサイズが指定されていません。");
		return(FALSE);
	}

	Saddr = DUMP_SADDR;
	_strupr(b_w);
	if(Strcmp("B", b_w) == 0){
		Size = MBYTE_ACCESS;
		Eaddr = Saddr;
	}else if(Strcmp("W", b_w) == 0){
		Size = MWORD_ACCESS;
		Eaddr = Saddr + 1L;
	}else if(Strcmp("L", b_w) == 0){
		Size = MLWORD_ACCESS;
		Eaddr = Saddr + 3L;
	}else{
		Print("Error: データサイズが異常です。");
		return(FALSE);
	}
	Mode = 3;
	if(sa_addr != NULL){
		// 開始アドレス指定あり
		// if(NDEB_Eval(sa_addr, EXP_DEFAULT, EXP_LABEL, &Saddr) != TRUE){
		// 	Print("EXPRESS_ERROR");
		// 	return(FALSE);
		// }
		Saddr = strtoul(sa_addr, NULL, 16);		
		printf("[%s] Saddr 0x%08lx\n", __func__, Saddr);
		Mode = 2;
		DUMP_SADDR = Saddr;
		if(e_addr != NULL){
			// 開始・終了アドレス指定あり
			// if(NDEB_Eval(e_addr, EXP_DEFAULT, EXP_LABEL, &Eaddr) != TRUE){
			// 	Print("EXPRESS_ERROR");
			// 	return(FALSE);
			// }
			Eaddr = strtoul(e_addr, NULL, 16);
			printf("[%s] Eaddr 0x%08lx\n", __func__, Eaddr);
			DUMP_SADDR = Saddr;
			Mode = 1;
		}else{
			if(DUMP_SADDR > Saddr){
				Saddr = DUMP_SADDR;
			}
			if(Size == MBYTE_ACCESS){
				Eaddr = Saddr;
			}else if(Size == MWORD_ACCESS){
				Eaddr = Saddr + 1;
			}else if(Size == MLWORD_ACCESS){
				Eaddr = Saddr + 3;
			}
		}
	}

// RevNo110915_002 Modify Start
	Length = Eaddr - DUMP_SADDR + 1L;
// RevNo110915_002 Modify End

// RevNo110915_002 Append Start
	// 動的メモリ確保
	if ((Length == 0L) || (DUMP_SADDR > Eaddr)) {
		//VS2008へ移行すると0x7FFFFFFFを超える配列確保ができない。
		//サイズが0x7FFFFFFFを超える場合はエラーを返すように改定する。
		if(0x7FFFFFFF < (Eaddr - Saddr)){
			Print("Error: 一度に0x7FFFFFFFを超える範囲は指定できません。");
			return(FALSE);
		}
		ReadData = (BYTE*) new BYTE[0x7FFFFFFF];	//new BYTE[0x100000000];
	} else {
		ReadData = (BYTE*) new BYTE[Length];
	}
// RevNo110915_002 Append End

	Ret = FFWMCUCmd_DUMP(Saddr, Eaddr, Size, &ReadData[0]);
	if(Ret == FFWERR_FFW_ARG){
		PrintErr_E20RX(Ret);
// RevNo110915_002 Append Start
		delete [] ReadData;
		ReadData = NULL;
// RevNo110915_002 Append End
		return(FALSE);
	}

	if(Ret != FFWERR_OK){
		PrintErr_E20RX(Ret);
//		return(FALSE);
	}

	if (skip == NULL) {
//		dump_disp_rx600(Saddr, Size, Length, &ReadData[0], 0);
		dump_disp_rx600(Saddr, Eaddr, Size, &ReadData[0], 0);
	} else {
		_strupr(skip);
		printf("[%s] skip %s\n", __func__, skip);
		if(Strcmp("H", skip) == 0){
//			dump_disp_rx600(Saddr, Size, Length, &ReadData[0], 1);
			dump_disp_rx600(Saddr, Eaddr, Size, &ReadData[0], 0);
		}
	}
// RevNo110915_002 Append Start
	delete [] ReadData;
	ReadData = NULL;
// RevNo110915_002 Append End

	switch(Mode){
	case 1:
		DUMP_SADDR = Eaddr + 1;
		break;
	case 2:
	case 3:
		DUMP_SADDR++;
		if(Size == MWORD_ACCESS){
			DUMP_SADDR++;
		}else if(Size == MLWORD_ACCESS){
			DUMP_SADDR += 3;
		}
		break;
	}

	// printf("[%s] Ignored DUMP_SADDR > McuMaxAddr()\n", __func__);
	// if(DUMP_SADDR > McuMaxAddr()){
	// 	DUMP_SADDR = 0L;
	// }

	printf("[%s] Returned TRUE\n", __func__);
	return(TRUE);
}

int DO_DUMP(char *b_w, char *sa_addr, char *e_addr, char *skip)
{
	// if ((PROT_EMU_TYPE() == EMU_E20) || (PROT_EMU_TYPE() == EMU_E2)) {
		return(do_dump_rx600(b_w, sa_addr, e_addr, skip));
	// }
	// return FALSE;
}
///////////////////////////////////////////////////////////////

void target_memory_access(void)
{
	Print("\nwrite 00000000_0000ffff_inc_00.mot");
	char pa1[] = "00000000_0000ffff_inc_00.mot";
	DO_WRITE(pa1, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL, (char*)NULL );

	Print("\nwrite 0,f,b,0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f");
	char pa[][10] = {"0","f","b","0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f"};
	DO_WRITE(pa[0], pa[1], pa[2], pa[3], pa[4], pa[5], pa[6], pa[7], pa[8], pa[9], pa[10], pa[11], pa[12], pa[13], pa[14], pa[15], pa[16], pa[17], pa[18], (char*)NULL );


	Print("\ndump b, 0,ff");
	char dump_cmd_1[][10] = {"b", "0", "ff"};
	DO_DUMP(dump_cmd_1[0], dump_cmd_1[1], dump_cmd_1[2], NULL);

	Print("\ndump w,0,3f");
	char dump_cmd_2[][10] = {"w", "0", "3f"};
	DO_DUMP(dump_cmd_2[0], dump_cmd_2[1], dump_cmd_2[2], NULL);
	
	Print("\ndump l,0,3f");
	char dump_cmd_3[][10] = {"l", "0", "3f"};
	DO_DUMP(dump_cmd_3[0], dump_cmd_3[1], dump_cmd_3[2], NULL);
}
///////////////////////////////////////////////////////////////

int main(void)
{
	COM_Open(COM_TYPE_USB_E2USB, NULL, 0);

	printf("\nInitialize NDEB\n");
	init_ndeb();
	target_memory_access();

	printf("\n");
	COM_Close();

	return 0;
}

