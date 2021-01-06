///////////////////////////////////////////////////////////////////////////////
/**
 * @file protmcu_closed.h
 * @brief BFWコマンド プロトコル生成関数(非公開コマンド関連)ヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
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
#ifndef	__PROTMCU_CLOSED_H__
#define	__PROTMCU_CLOSED_H__

#include "ffw_typedef.h"
#include "mcudef.h"

/////////// 関数の宣言 ///////////
extern FFWERR PROT_MCU_RXIR(DWORD dwLength, BYTE* pbyData);					///< BFWCmd_GetRXIRの発行
extern FFWERR PROT_MCU_RXDR(DWORD dwLength, BYTE* pbyData, DWORD dwPause);	///< BFWCmd_GetRXDRの発行
extern FFWERR PROT_MCU_RXGetDR(DWORD dwLength, BYTE* pbyData, DWORD dwPause);	///< BFWCmd_GetRXDRの発行
extern FFWERR PROT_MCU_RXMAST(void);										///< 供給命令コードの実行
extern FFWERR PROT_MCU_RXSEMC(BYTE byLength, DWORD* pdwData);
extern FFWERR PROT_MCU_RXGEMD(DWORD *pdwData);								///< 供給命令コード実行による結果(C2E_0)内容の取得
extern FFWERR PROT_MCU_RXSEMP(DWORD dwData);								///< 命令供給コードへのパラメータ渡し(E2C_0)に設定

extern FFWERR PROT_MCU_SetRXSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteData);		///< BFWCmd_SetTMOUTコマンドの発行
extern FFWERR PROT_MCU_GetRXSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD *pdwReadData);	///< BFWCmd_GetTMOUTコマンドの発行

extern void InitProtMcuData_Closed(void);	///< システムコマンド用変数初期化


#endif	// __PROTMCU_CLOSED_H__
