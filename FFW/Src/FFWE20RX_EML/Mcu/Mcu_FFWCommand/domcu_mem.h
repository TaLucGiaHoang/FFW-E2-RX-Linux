///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_mem.h
 * @brief メモリ操作コマンドのヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/11
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo111121-003   2012/07/10 橋口
  ・OffsetStartAddr() 
    ・domcu_mem.cpp 以外でも使用する為、extern 宣言を新規追加。
*/
#ifndef	__DOMCU_MEM_H__
#define	__DOMCU_MEM_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義

// グローバル関数の宣言
extern FFWERR DO_DUMP(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE *const pbyReadBuff);
														///< ターゲットメモリ内容を取得する
extern FFWERR DO_FILL(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
					  enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
					  FFW_VERIFYERR_DATA* pVerifyErr);	///< ターゲットメモリに単一データを書き込む
extern FFWERR DO_WRITE(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
					   enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);	
														///< ターゲットメモリ内容を変更する
extern FFWERR DO_MOVE(MADDR madrSrcStartAddr, MADDR madrSrcEndAddr, MADDR madrDisStartAddr, MADDR madrDisEndAddr, 
			   enum FFWENM_VERIFY_SET eVerify, enum FFWENM_MACCESS_SIZE eAccessSize, FFW_VERIFYERR_DATA* pVerifyErr);
														///< ターゲットメモリ内容をコピーする
extern FFWERR DO_MCMP(MADDR madrSrcStartAddr, MADDR madrSrcEndAddr, MADDR madrDisStartAddr, 
			   enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwCompDataSize, 
			   FFW_COMPERR_DATA* pCompError);
														///< ターゲットメモリ内容を比較する
extern FFWERR DO_MSER(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MSEARCH_EQU eSearchEqual, 
			 enum FFWENM_MSEARCH_DIR eSerchForward, enum FFWENM_MSEARCH_PATERN eSearchPatern,
			 enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwSearchDataSize,
			 const BYTE* pbySearchDataBuff, const BYTE* pbySearchMaskBuff,
			 FFW_SEARCH_DATA* pSearchAns);
														///< ターベットメモリから指定データを検索する

extern FFWERR DO_CDUMP(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, const BYTE* pbyReadBuff);
														///< ターゲットメモリ内容を取得する(命令コード順に並び替え)
extern FFWERR DO_CWRITE(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
						const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);	
														///< ターゲットメモリ内容を変更する(命令コード順に並び替え)
extern void OffsetStartAddr(enum FFWENM_MACCESS_SIZE eAccessSize, MADDR madrStartAddr, DWORD* pdwStartAddr);		// RevNo111121-003 Append Line

extern void InitFfwCmdMcuData_Mem(void);	///< メモリ操作コマンド用変数初期化

#endif	// __DOMCU_MEM_H__
