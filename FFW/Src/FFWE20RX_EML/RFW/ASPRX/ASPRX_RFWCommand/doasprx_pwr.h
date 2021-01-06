///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasprx_pwr.h
 * @brief パワーモニタコマンドのヘッダファイル(RX固有部)
 * @author REL H.Ohga
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/20
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2017/10/20 新規作成 H.Ohga
*/
#ifndef	__DOASPRX_PWR_H__
#define	__DOASPRX_PWR_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義

// グローバル関数の宣言
#ifdef	__cplusplus
extern	"C" {
#endif
extern DWORD DO_GetPwrMonDataRange(DWORD *pdwUAmin, DWORD *pdwUAmax);
#ifdef	__cplusplus
};
#endif

#endif	// __DOASPRX_PWR_H__
