///////////////////////////////////////////////////////////////////////////////
/**
 * @file do_closed.h
 * @brief 非公開コマンドのヘッダファイル
 * @author RSD Y.Minami, H.Hashiguchi, H.Akashi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/13
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120910-002	2012/10/30 明石
　　MONPALLコマンド ダウンロードデータサイズの奇数バイト対応
・RevRxE2LNo141104-001 2014/11/13 上田
	E2 Lite対応
	E1/E20 レベル0 旧バージョン(MP版以外)用関数削除。
*/
#ifndef	__DO_CLOSED_H__
#define	__DO_CLOSED_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義


// グローバル関数の宣言
extern FFWERR DO_CPUR(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwReadCount, BYTE *const pbyReadBuff);
extern FFWERR DO_CPUW(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwWriteCount, BYTE* pbyWriteBuff);
extern FFWERR DO_CPUF(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwWriteCount, const BYTE* pbyWriteBuff);
extern FFWERR DO_CPURF(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, DWORD dwWriteCount, DWORD dwWriteData, DWORD dwMaskData);	// RevRxE2LNo141104-001 Append Line
extern FFWERR DO_MONPALL_OPEN(DWORD dwTotalLength);
extern FFWERR DO_MONPALL_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);
extern FFWERR DO_MONPALL_CLOSE(void);

extern FFWERR DO_SetEMLMECLK(enum FFWENM_EMLMESCLK_SEL eTclk);
extern FFWERR DO_GetEMLMED(DWORD* pdwEmlTime, BYTE* pbyEmlOverflow, UINT64* pu64PcTime);
extern FFWERR DO_ClrEMLMED(void);

extern FFWERR DO_E20GetD0FIFO( BYTE* );

extern void InitFfwCmdData_Closed(void);	///< 非公開コマンド用変数初期化

// グローバル変数の宣言
// RevRxE2LNo141104-001 Delete Line MONPコマンド用BFWコード格納バッファの宣言削除

#endif	// __DO_CLOSED_H__
