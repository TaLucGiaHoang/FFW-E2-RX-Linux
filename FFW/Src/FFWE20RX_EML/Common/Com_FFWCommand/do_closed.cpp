///////////////////////////////////////////////////////////////////////////////
/**
 * @file do_closed.cpp
 * @brief 非公開コマンドの実装ファイル
 * @author RSD Y.Minami, H.Hashiguchi, H.Akashi, S.Ueda
 * @author Copyright (C) 2009(2010-2015) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2015/01/21
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120910-002	2012/11/12 明石
　　MONPALLコマンド ダウンロードデータサイズの奇数バイト対応
・RevRxE2LNo141104-001 2015/01/21 上田
	E2 Lite対応
	E1/E20 レベル0 旧バージョン(MP版以外)用関数削除。
・RevRxNo170710-001 2017/07/10 佐々木(広)
	E2拡張対応
*/


#include "do_closed.h"
#include "ffw_closed.h"	// RevRxE2LNo141104-001 Append Line
#include "ffw_sys.h"
#include "do_sys.h"
#include "prot_cpu.h"
#include "prot_common.h"
#include "prot_sys.h"
#include "hw_fpga.h"
#include "fpga_com.h"
#include "emudef.h"	// RevRxE2LNo141104-001 Append Line
#include <stdio.h>	//拡張機能対応
// ファイル内static変数の宣言
static DWORD	s_dwTotalLength;		// FFWCmd_xxxx_SENDで送信するデータの総バイト数 (MONPALLで使用)
static DWORD	s_dwSendLength;			// FFWCmd_xxxx_SENDで送信したデータの総バイト数 (MONPALLで使用)
static WORD s_wTMCTRL;

// ファイル内static関数の宣言
// RevRxE2LNo141104-001 Append Start
static FFWERR monpAllClose_E2(void);
static FFWERR monpAllSendExec(DWORD dwSendStart, DWORD dwSendLength, BYTE* pbyBfwCodeBuf, 
							  enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd, DWORD dwTotalSendCount, DWORD* pdwSendGageCount);
// RevRxE2LNo141104-001 Append End


///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * モニタCPU 空間のリード
 * @param eadrStartAddr モニタCPUアドレス
 * @param eAccessSize アクセスサイズ
 * @param dwReadCount 繰り返し回数
 * @param pbyReadBuff リードデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_CPUR(EADDR eadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwReadCount, 
			   BYTE *const pbyReadBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	DWORD	dwAddAddr;
	DWORD	dwAreaNum;
	DWORD	dwCount;
	BYTE*	pbyBuff;
	DWORD	dwTotalLength;
	EADDR	eadrEndAddr;

	// RevRxNo170710-001 Append Start
	//拡張機能対応
	FFWE20_EINF_DATA	einfData;
	BYTE	byBuf0;
	BYTE	byBuf1;
	BYTE	byBuf2;
	BYTE	byBuf3;
	DWORD	dwLoopCount;

#ifdef	_DEBUG
	char buffer[256];
	BOOL e2Fpgaflg = FALSE;
#endif
	// RevRxNo170710-001 Append End

	ProtInit();

	switch (eAccessSize) {
	case EBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case EWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case ELWORD_ACCESS:
		dwAddAddr = 4;
		break;
	default:
		dwAddAddr = 4;		//ワーニング対策
		break;
	}

	dwAreaNum = 1;
	pbyBuff = pbyReadBuff;

	eadrEndAddr = eadrStartAddr + (dwReadCount * dwAddAddr) - 1;
	dwTotalLength = eadrEndAddr - eadrStartAddr + 1;

	dwReadCount = dwTotalLength / dwAddAddr;
	while (dwReadCount) {
		if (dwReadCount >= PROT_CPU_R_LENGTH_MAX) {
			dwCount = PROT_CPU_R_LENGTH_MAX;
		} else {
			dwCount = dwReadCount;
		}
		// V.1.02 No.14,15 USB高速化対応 Append & Modify Start
		// RevRxE2LNo141104-001 Modify Line
		// エミュレータ種別、BFW動作モードに対応したCPU_Rコマンドを発行(エミュレータ種別はFFW内部変数を参照)
		ferr = ProtCpuRead(eAccessSize, dwAreaNum, &eadrStartAddr, &dwCount, pbyReadBuff);
		// V.1.02 No.14,15 USB高速化対応 Append & Modify End
		if (ferr != FFWERR_OK) {
			break;
		}
		// RevRxNo170710-001 Append Start
		//拡張機能対応

		getEinfData(&einfData);

		/* とりあえずE2のみ */
		if (einfData.wEmuStatus == EML_E2) {

			// FPGAアドレスの場合(エンディアンが逆)
			if ((eadrStartAddr >= 0x06000000) && (eadrStartAddr < 0x08000000)) {
#ifdef	_DEBUG
				e2Fpgaflg = TRUE;
#endif
				// 4バイトアクセスの前提
				for (dwLoopCount = 0; dwLoopCount<dwCount; dwLoopCount++) {
					// データのエンディアンを入れ替える
					byBuf3 = *pbyBuff;
					pbyBuff++;
					byBuf2 = *pbyBuff;
					pbyBuff++;
					byBuf1 = *pbyBuff;
					pbyBuff++;
					byBuf0 = *pbyBuff;
					pbyBuff++;
					pbyBuff -= dwAddAddr;
					*pbyBuff = byBuf0;
					pbyBuff++;
					*pbyBuff = byBuf1;
					pbyBuff++;
					*pbyBuff = byBuf2;
					pbyBuff++;
					*pbyBuff = byBuf3;
					pbyBuff++;
				}

				// 進んだポインタを一旦元に戻す
				pbyBuff -= (dwCount * dwAddAddr);
#ifdef	_DEBUG
				if (e2Fpgaflg) {
					sprintf_s(buffer, sizeof(buffer), "E2 FPGA Read   Addr:%08x, data:%08x \n", eadrStartAddr, (*((DWORD*)pbyBuff)));
					OutputDebugString(buffer);
					e2Fpgaflg = FALSE;
				}
#endif
			}
		}
		// RevRxNo170710-001 Append Start

		eadrStartAddr += (dwCount * dwAddAddr);
		pbyBuff += (dwCount * dwAddAddr);
		dwReadCount -= dwCount;	
	}


	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}


//=============================================================================
/**
 * モニタCPU 空間のライト
 * @param dweadrStartAddr モニタCPUアドレス
 * @param eAccessSize アクセスサイズ
 * @param dwReadCount 繰り返し回数
 * @param pbyWriteBuff ライトデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_CPUW(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwWriteCount, 
			   BYTE* pbyWriteBuff)
{//pbyWriteBuffのconst宣言を暫定で外す
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	DWORD	dwAddAddr;
	DWORD	dwAreaNum;
	DWORD	dwCount;

	// RevRxNo170710-001 Append Start
	//拡張機能対応
	BYTE	byBuf0;
	BYTE	byBuf1;
	BYTE	byBuf2;
	BYTE	byBuf3;
	DWORD	dwLoopCount;
	FFWE20_EINF_DATA	einfData;
#ifdef	_DEBUG
	char buffer[256];
	BOOL e2Fpgaflg = FALSE;
	DWORD dwWriteData = *((DWORD*)pbyWriteBuff);
#endif
	// RevRxNo170710-001 Append End

	ProtInit();

	switch (eAccessSize) {
	case EBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case EWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case ELWORD_ACCESS:
		dwAddAddr = 4;
		break;
	default:
		dwAddAddr = 4;		//ワーニング対策
		break;
	}

	dwAreaNum = 1;

	while (dwWriteCount) {
		if (dwWriteCount >= PROT_CPU_W_LENGTH_MAX) {
			dwCount = PROT_CPU_W_LENGTH_MAX;
		} else {
			dwCount = dwWriteCount;
		}

		// RevRxNo170710-001 Append Start
		//拡張機能対応
		getEinfData(&einfData);

		/* とりあえずE2のみ */
		if (einfData.wEmuStatus == EML_E2) {
			// FPGAアドレスの場合(エンディアンが逆)
			if ((dweadrStartAddr >= 0x06000000) && (dweadrStartAddr < 0x08000000)) {
				// 4バイトアクセスの前提
				for (dwLoopCount = 0; dwLoopCount<dwCount; dwLoopCount++) {
					// データのエンディアンを入れ替える
					byBuf3 = *pbyWriteBuff;
					pbyWriteBuff++;
					byBuf2 = *pbyWriteBuff;
					pbyWriteBuff++;
					byBuf1 = *pbyWriteBuff;
					pbyWriteBuff++;
					byBuf0 = *pbyWriteBuff;
					pbyWriteBuff++;
					pbyWriteBuff -= dwAddAddr;
					*pbyWriteBuff = byBuf0;
					pbyWriteBuff++;
					*pbyWriteBuff = byBuf1;
					pbyWriteBuff++;
					*pbyWriteBuff = byBuf2;
					pbyWriteBuff++;
					*pbyWriteBuff = byBuf3;
					pbyWriteBuff++;
				}
				// 進んだポインタを一旦元に戻す
				pbyWriteBuff -= (dwCount * dwAddAddr);

#ifdef	_DEBUG
				e2Fpgaflg = TRUE;
#endif
			}
		}
		// RevRxNo170710-001 Append End

		// V.1.02 No.14,15 USB高速化対応 Append & Modify Start
		// RevRxE2LNo141104-001 Modify Line
		// エミュレータ種別、BFW動作モードに対応したCPU_Rコマンドを発行(エミュレータ種別はFFW内部変数を参照)
		ferr = ProtCpuWrite(eAccessSize, dwAreaNum, &dweadrStartAddr, &dwCount, pbyWriteBuff);
		// V.1.02 No.14,15 USB高速化対応 Append & Modify E
		if (ferr != FFWERR_OK) {
			break;
		}
// RevRxNo170710-001 Append Start
#ifdef	_DEBUG
		if (e2Fpgaflg) {
			sprintf_s(buffer, sizeof(buffer), "E2 FPGA Write  Addr:%08x, data:%08x \n", dweadrStartAddr, dwWriteData);
			OutputDebugString(buffer);
			e2Fpgaflg = FALSE;
		}
#endif
// RevRxNo170710-001 Append End
		dweadrStartAddr += (dwCount * dwAddAddr);
		pbyWriteBuff += (dwCount * dwAddAddr);
		dwWriteCount -= dwCount;	
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * モニタCPU 空間の単一データライト(FILL)
 * @param dweadrStartAddr モニタCPUアドレス
 * @param eAccessSize アクセスサイズ
 * @param dwReadCount 繰り返し回数
 * @param pbyWriteBuff ライトデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_CPUF(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwWriteCount, 
			   const BYTE* pbyWriteBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	DWORD	dwCount;
	DWORD	dwAddAddr;

	ProtInit();

	switch (eAccessSize) {
	case EBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case EWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case ELWORD_ACCESS:
		dwAddAddr = 4;
		break;
	default:
		dwAddAddr = 4;		//ワーニング対策
		break;
	}

	while (dwWriteCount) {
		if (dwWriteCount >= PROT_CPU_FILL_LENGTH_MAX) {	// RevRxE2LNo141104-001 Modify Line
			dwCount = PROT_CPU_FILL_LENGTH_MAX;
		} else {
			dwCount = dwWriteCount;
		}

		// RevRxE2LNo141104-001 Modify Line
		// エミュレータ種別、BFW動作モードに対応したCPU_Rコマンドを発行(エミュレータ種別はFFW内部変数を参照)
		ferr = ProtCpuFill(eAccessSize, dweadrStartAddr, dwCount, pbyWriteBuff);
		if (ferr != FFWERR_OK) {
			break;
		}
		dweadrStartAddr += (dwCount * dwAddAddr);
		dwWriteCount -= dwCount;	
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * モニタCPU 空間のリードモディファイライト
 * @param dweadrStartAddr モニタCPU空間の開始アドレス
 * @param eAccessSize アクセスサイズ
 * @param dwWriteCount 繰り返し回数
 * @param dwWriteData ライトデータ
 * @param dwMaskData マスクデータ
 * @return FFWエラーコード
 */
//=============================================================================
FFWERR DO_CPURF(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwWriteCount, 
			   DWORD dwWriteData, DWORD dwMaskData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	DWORD	dwCount;
	DWORD	dwAddAddr;

	ProtInit();

	switch (eAccessSize) {
	case EBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case EWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case ELWORD_ACCESS:
		dwAddAddr = 4;
		break;
	default:
		dwAddAddr = 4;		//ワーニング対策
		break;
	}

	while (dwWriteCount) {
		if (dwWriteCount >= PROT_CPU_RMWFILL_LENGTH_MAX) {
			dwCount = PROT_CPU_RMWFILL_LENGTH_MAX;
		} else {
			dwCount = dwWriteCount;
		}

		ferr = PROT_CPU_RMWFILL(eAccessSize, dweadrStartAddr, dwCount, dwMaskData, dwWriteData);
		if (ferr != FFWERR_OK) {
			break;
		}
		dweadrStartAddr += (dwCount * dwAddAddr);
		dwWriteCount -= dwCount;	
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムデータの送信開始を通知
 * @param dwTotalLength FFWCmd_MONPALL_SENDで送信するBFWデータの総バイト数
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPALL_OPEN(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;

	// ワーニング対策
	dwTotalLength;

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	// MONPALLコマンドを指定して、MONP OPEN処理実行
	eBfwLoadCmd = BFWLOAD_CMD_MONPALL;
	ferr = MonpOpen(eBfwLoadCmd);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムデータの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPALL_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;

	// RevRxE2LNo141104-001 Modify Start
	ProtInit();

	// MONP SEND処理実行(MONP, MONPFDT, MONPCOM, MONPALL共通)
	ferr = MonpSend(dweadrAddr, dwLength, pbyBfwBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	// RevRxE2LNo141104-001 Modify End

	return ferr;
}

//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムデータの送信終了を通知
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MONPALL_CLOSE(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	FFWE20_EINF_DATA einfData;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;

	ProtInit();

	// RevRxE2LNo141104-001 Modify Start
	getEinfData(&einfData);

	if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {
		// MONPALLコマンドを指定して、MONP CLOSE処理実行
		eBfwLoadCmd = BFWLOAD_CMD_MONPALL;
		ferr = MonpClose(eBfwLoadCmd);

	} else {
		ferr = monpAllClose_E2();
	}
	// RevRxE2LNo141104-001 Modify End

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//==============================================================================
/**
 * 全実行時間計測クロック設定を行う。
 * @param  eTclk 設定クロック値
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_SetEMLMECLK(enum FFWENM_EMLMESCLK_SEL eTclk)
{
	WORD wBuff = 0;
	WORD wTMCTRL = 0;
	FFWERR		ferr;

	wBuff = (BYTE)eTclk;

	// TMCTRLレジスタに使用クロックをセット
	ferr = ReadFpgaReg(REG_TIM_TMCTRL, &wTMCTRL);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wTMCTRL = (WORD)((wTMCTRL & 0xF0FF) | (wBuff<<8));
	ferr = WriteFpgaReg(REG_TIM_TMCTRL, s_wTMCTRL);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;

}

//==============================================================================
/**
 * 全実行時間計測データ取得。
 * @param  pdwEmlTime SCI FPGA 計測時間
 * @param  pbyEmlOverflow SCI FPGA オーバーフロー結果
 * @param  pu64PcTime PC資源による計測時間
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_GetEMLMED(DWORD* pdwEmlTime, BYTE* pbyEmlOverflow, UINT64* pu64PcTime)
{
	WORD wBuff = 0;
	WORD wTMCTRL = 0;
	WORD wTMCNTL = 0;
	WORD wTMCNTH = 0;
	FFWERR		ferr;
	
	// PC時間取得
	*pu64PcTime = GetTickCount();

	// TMCTRLレジスタ値取得
	ferr = ReadFpgaReg(REG_TIM_TMCTRL, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	wTMCTRL = wBuff;
	// オーバーフロービットセット
	*pbyEmlOverflow = (BYTE)((wTMCTRL & 0x0002) >> 1);

	// TMCNTHレジスタ値取得
	ferr = ReadFpgaReg(REG_TIM_TMCNTH, &wTMCNTH);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// TMCNTLレジスタ値取得
	ferr = ReadFpgaReg(REG_TIM_TMCNTL, &wTMCNTL);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// エミュレータ時間取得
	*pdwEmlTime = (DWORD)(wTMCNTH << 16) | wTMCNTL;

	return FFWERR_OK;

}

//==============================================================================
/**
 * 全実行時間計測クロックのクリア。
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR DO_ClrEMLMED(void)
{
	WORD wBuff = 0;
	WORD wTMCTRL = 0;
	FFWERR		ferr;
	

	// TMCTRLレジスタ値取得
	ferr = ReadFpgaReg(REG_TIM_TMCTRL, &wBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	wTMCTRL = (WORD)((wBuff & 0xFFF9) | 0x0002);
	ferr = WriteFpgaReg(REG_TIM_TMCTRL, wTMCTRL);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return FFWERR_OK;

}

//=============================================================================
/**
 * E20 USB FIFOデータの取得（出荷検査用コマンド）
 * @param pFifo  FIFOバッファ取得データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_E20GetD0FIFO( BYTE* pFifo )
{
	FFWERR	ferr = FFWERR_OK;
	
	ferr = PROT_E20GetD0FIFO( pFifo );
	return	ferr;
}

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムデータの送信終了を通知(E2/E2 Lite用)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR monpAllClose_E2(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr2;
	enum FFWENM_BFWLOAD_CMD_NO	eBfwLoadCmd;	// コマンドコード
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwSendCount;						// 送信済みデータ数
	DWORD	dwTotalSendCount;					// 総送信データ数
	BYTE*	pbyBfwCodeBuf;						// 書き込み用データバッファポインタ
	BYTE	byMonAreaFlg;
	DWORD	dwAreaStart;
	DWORD	dwAreaLength;

	pEmuDef = GetEmuDefData();

	eBfwLoadCmd = BFWLOAD_CMD_MONPALL;

	byMonAreaFlg = GetMonpAreaFlg();	// MONP書き込み領域フラグ値取得

	// 総データ送信カウント数設定
	dwTotalSendCount = 0;
	if ((byMonAreaFlg & BFW_MONP_AREA_EML) == BFW_MONP_AREA_EML) {
		dwTotalSendCount += pEmuDef->dwBfwEmlSize;
	}
	if ((byMonAreaFlg & BFW_MONP_AREA_FDT) == BFW_MONP_AREA_FDT) {
		dwTotalSendCount += pEmuDef->dwBfwPrgSize;
	}
	if ((byMonAreaFlg & BFW_MONP_AREA_COM) == BFW_MONP_AREA_COM) {
		dwTotalSendCount += pEmuDef->dwBfwComSize;
	}
	if ((byMonAreaFlg & BFW_MONP_AREA_LV0) == BFW_MONP_AREA_LV0) {
		dwTotalSendCount += pEmuDef->dwBfwLv0Size;
	}

	dwSendCount = 0;

	// EML領域の書き込み
	if ((byMonAreaFlg & BFW_MONP_AREA_EML) == BFW_MONP_AREA_EML) {
		dwAreaStart = pEmuDef->dwBfwEmlStartAdr;
		dwAreaLength = pEmuDef->dwBfwEmlSize;
		pbyBfwCodeBuf = GetBfwCodeBuffPtr(dwAreaStart);

		ferr = monpAllSendExec(dwAreaStart, dwAreaLength, pbyBfwCodeBuf, eBfwLoadCmd, dwTotalSendCount, &dwSendCount);
	}
	// PRG領域の書き込み
	if (ferr == FFWERR_OK) {
		if ((byMonAreaFlg & BFW_MONP_AREA_FDT) == BFW_MONP_AREA_FDT) {
			dwAreaStart = pEmuDef->dwBfwPrgStartAdr;
			dwAreaLength = pEmuDef->dwBfwPrgSize;
			pbyBfwCodeBuf = GetBfwCodeBuffPtr(dwAreaStart);

			ferr = monpAllSendExec(dwAreaStart, dwAreaLength, pbyBfwCodeBuf, eBfwLoadCmd, dwTotalSendCount, &dwSendCount);
		}
	}
	// COM領域の書き込み
	if (ferr == FFWERR_OK) {
		if ((byMonAreaFlg & BFW_MONP_AREA_COM) == BFW_MONP_AREA_COM) {
			dwAreaStart = pEmuDef->dwBfwComStartAdr;
			dwAreaLength = pEmuDef->dwBfwComSize;
			pbyBfwCodeBuf = GetBfwCodeBuffPtr(dwAreaStart);

			ferr = monpAllSendExec(dwAreaStart, dwAreaLength, pbyBfwCodeBuf, eBfwLoadCmd, dwTotalSendCount, &dwSendCount);
		}
	}
	// レベル0領域の書き込み (レベル0領域の書き込みは最後に実施する)
	if (ferr == FFWERR_OK) {
		if ((byMonAreaFlg & BFW_MONP_AREA_LV0) == BFW_MONP_AREA_LV0) {
			dwAreaStart = pEmuDef->dwBfwLv0StartAdr;
			dwAreaLength = pEmuDef->dwBfwLv0Size;
			pbyBfwCodeBuf = GetBfwCodeBuffPtr(dwAreaStart);

			ferr = monpAllSendExec(dwAreaStart, dwAreaLength, pbyBfwCodeBuf, eBfwLoadCmd, dwTotalSendCount, &dwSendCount);
		}
	}

	// BFW書き換え終了通知(エラー発生時も必ず実行する)
	ferr2 = PROT_MONP_CLOSE(eBfwLoadCmd);
	if (ferr == FFWERR_OK) {
		ferr = ferr2;
	}

	// BFWコード格納用バッファの解放
	DeleteBfwDataBuff();

	return ferr;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * モニタプログラムデータの送信
 * @param dwSendStart BFWコード送信開始アドレス
 * @param dwSendLength BFWコード送信バイト数
 * @param pbyBfwCodeBuf BFWコードデータ格納バッファポインタ
 * @param eBfwLoadCmd BFW書き換えコマンド種別
 * @param dwTotalSendCount BFWコード全送信バイト数(進捗ゲージカウント用)
 * @param pdwSendGageCount BFWコード送信済みバイト数格納変数ポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR monpAllSendExec(DWORD dwSendStart, DWORD dwSendLength, BYTE* pbyBfwCodeBuf, enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd, DWORD dwTotalSendCount, DWORD* pdwSendGageCount)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTmpLength;
	DWORD	dwCount;
	float	fCount;
	DWORD	dwSendAddr;
	DWORD	dwLength;
	BYTE*	pbyBuf;
	DWORD	dwSendCount;

	dwSendAddr = dwSendStart;
	dwLength = dwSendLength;
	pbyBuf = pbyBfwCodeBuf;
	dwSendCount = *pdwSendGageCount;

	while (dwLength > 0) {
		if (dwLength > FFW_SEND_UNITCNT) {
			dwTmpLength = FFW_SEND_UNITCNT;
		} else {
			dwTmpLength = dwLength;
		}
		ferr = PROT_MONP_SEND(dwSendAddr, dwTmpLength, pbyBuf, eBfwLoadCmd);
		if (ferr != FFWERR_OK) {
			break;
		}

		dwSendAddr += dwTmpLength;
		dwLength -= dwTmpLength;
		pbyBuf += dwTmpLength;
		dwSendCount += dwTmpLength;

		//	(e)	プログレスバー更新用データ(dwCount) 更新
		if (dwSendCount == 0) {
			dwCount = 0;
		} else {
			fCount = static_cast<float>(dwTotalSendCount) / static_cast<float>(dwSendCount);
			if (0 < fCount) {
				fCount = static_cast<float>(GAGE_COUNT_MAX) / fCount;
				dwCount = static_cast<DWORD>(fCount);
			} else {
				dwCount = 0;
			}
		}
		//	(f)	プログレスバー表示更新
		SetGageCount(dwCount);
	}

	*pdwSendGageCount = dwSendCount;

	return ferr;
}
// RevRxE2LNo141104-001 Append End

///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 非公開コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdData_Closed(void)
{
	s_wTMCTRL   = 0;
	return;
}


// RevRxNo170710-001 Append Start
//=============================================================================
/**
* 制御CPU空間のリード
* @note アドレスと読み出しサイズは4バイトアラインとすること
* @param addr 読み出し先頭アドレス
* @param len 読み出しサイズ(バイト)
* @param data 読み出しデータ
* @retval EX_NOERROR  参照成功
* @retval EX_FE_USBFAIL 参照失敗
*/
//=============================================================================
int e2_CPU_R(ULONG addr, ULONG len, UCHAR *data)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	*pbyReadBuf;
	DWORD	dweadrStartAddr;
	DWORD	dwReadCount;
	enum	FFWENM_EACCESS_SIZE eAccessSize;

	eAccessSize = ELWORD_ACCESS;
	dweadrStartAddr = addr;
	dwReadCount = len / 4;
	pbyReadBuf = data;

	// CPU_Rコマンド呼び出し
	ferr = DO_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuf);
	if (ferr != FFWERR_OK) {
		return FFWERR_COM;
	}

	return FFWERR_OK;
}

//=============================================================================
/**
* 制御CPU空間のライト
* @note アドレスと書き込みサイズは4バイトアラインとすること
* @param addr 書き込み先頭アドレス
* @param len 書き込みサイズ(バイト)
* @param data 書き込みデータ
* @retval EX_NOERROR  参照成功
* @retval EX_FE_USBFAIL 参照失敗
*/
//=============================================================================
int e2_CPU_W(ULONG addr, ULONG len, UCHAR *data)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	*pbyWriteBuf;
	DWORD	dweadrStartAddr;
	DWORD	dwWriteCount;
	enum	FFWENM_EACCESS_SIZE eAccessSize;

	eAccessSize = ELWORD_ACCESS;
	dweadrStartAddr = addr;
	dwWriteCount = len / 4;
	pbyWriteBuf = data;

	// CPU_Wコマンド呼び出し
	ferr = DO_CPUW(dweadrStartAddr, eAccessSize, dwWriteCount, pbyWriteBuf);
	if (ferr != FFWERR_OK) {
		return FFWERR_COM;
	}

	return FFWERR_OK;
}
// RevRxNo170710-001 Append End
