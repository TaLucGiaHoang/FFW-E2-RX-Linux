///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_flash.h
 * @brief BFWコマンド プロトコル生成関数(内蔵Flash操作関連)ヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, S.Ueda, SDS T,Iwata
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/04/15
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121026-001	2012/10/30 上田
  RX100対応
  ・PROT_MCU_FWRITESTART()プロトタイプ宣言変更。
・RevRxNo121022-001	2012/11/27 SDS 岩田
   EZ-CUBE PROT_MCU_FWRITE()分割処理対応
・RevRxEzNo130117-001 2013/01/18 橋口
   EZ-CUBE ブロック数が0x197ブロックを超える場合にダウンロードができない不具合改修
・RevRxNo130301-001 2013/04/15 上田
	RX64M対応
*/
#ifndef	__PROTMCU_FLASH_H__
#define	__PROTMCU_FLASH_H__

#include "ffw_typedef.h"
#include "mcudef.h"
#include "mcu_flash.h"
#include "comctrl.h"							// RevRxEzNo130117-001 Append Line

/////////// define ///////////
// BFWMCUCmd_FWRITESTARTのフラッシュ書き換えタイミング情報
#define BFW_FWRITESTART_TIMING_FWRITE	0x00	// FWRITEコマンド実行時
#define BFW_FWRITESTART_TIMING_RUN		0x01	// 次回ユーザプログラム実行開始時
#define BFW_FWRITESTART_FCLR_OFF		0x00	// フラッシュメモリ初期化しない
#define BFW_FWRITESTART_FCLR_ON			0x01	// フラッシュメモリ初期化する

// RevRxNo121022-001 Append Line
#define PROT_FWRITE_LENGTH_MAX_EZ		0x200	// BFWMCUCmd_FWRITEコマンドのライトバイト数最大値(EZ-CUBE用)

// RevRxNo130301-001 Append Start
// BFWMCUCmd_FWRITEの書き換え領域指定
#define BFW_FWRITE_AREA_FLASH			0x00	// フラッシュメモリ
#define BFW_FWRITE_AREA_EXTRA_A			0x01	// ExtraA領域
// RevRxNo130301-001 Append End

#define	FWRITESTART_FCLRBLK_MAX_EZ		(((COM_SEND_BUFFER_SIZE-10)/5)-1)	// RevRxEzNo130117-001

/////////// 関数の宣言 ///////////
//RevNo010804-001 Modify Line
// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Line
//RevRxNo121026-001 Modify Line
extern FFWERR PROT_MCU_FWRITESTART(DWORD dwFlashWriteTiming, DWORD dwFlashClrInfo, FFW_FCLR_DATA_RX* pFclrData, enum FFWRX_WTR_NO eSetWtrNo, enum FFWENM_ENDIAN eMcuEndian);
																					///< BFWMCUCmd_FWRITESTARTコマンドの発行
// RevRxEzNo130117-001 Append Line
extern FFWERR PROT_MCU_FWRITESTART_EZ(DWORD dwFlashWriteTiming, DWORD dwFlashClrInfo, FFW_FCLR_DATA_RX* pFclrData, enum FFWRX_WTR_NO eSetWtrNo, enum FFWENM_ENDIAN eMcuEndian);
																					///< EZ用 BFWMCUCmd_FWRITESTARTプロトコルの実施
// RevRxNo130301-001 Modify Line
extern FFWERR PROT_MCU_FWRITE(BYTE byWriteArea, enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, const MADDR madrWriteAddr[], 
							  BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize[], 
							  BOOL bSameLength, const DWORD dwLength[], 
							  BOOL bSameWriteData, const BYTE* pbyWriteData, 
							  FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian);	///< BFWMCUCmd_FWRITEコマンドの発行
extern FFWERR PROT_MCU_FWRITEEND(void);												///< BFWMCUCmd_FWRITEENDコマンドの発行
extern void InitProtMcuData_Flash(void);	///< 内蔵Flash操作コマンド用変数初期化

#endif	// __PROTMCU_FLASH_H__
