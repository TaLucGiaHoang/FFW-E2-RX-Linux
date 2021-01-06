///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_exe.h
 * @brief 実行系拡張機能イベントリンクコマンドのヘッダファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/10/20 新規作成 S.Nagai
*/
#ifndef	__DOASP_EXE_H__
#define	__DOASP_EXE_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義


// グローバル関数の宣言
extern DWORD DO_SetBreakEvent(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent);
extern DWORD DO_GetBreakEvent(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent);
extern DWORD DO_ClrBreakEvent(void);
extern DWORD DO_SetStopTraceEvent(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent);
extern DWORD DO_GetStopTraceEvent(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent);
extern DWORD DO_ClrStopTraceEvent(void);

extern void InitBreakEvent(void);
extern BOOL IsBreakEventActEna(void);
extern BOOL IsBreakEventTrgEna(void);
extern void InitStopTraceEvent(void);
extern BOOL IsStopTraceEventActEna(void);

#endif	// __DOASP_EXE_H__
