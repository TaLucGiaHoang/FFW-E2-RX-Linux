///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_closed.h
 * @brief 非公開コマンドのヘッダファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/09/09
*/
#ifndef	__DOASP_CLOSED_H__
#define	__DOASP_CLOSED_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義


// グローバル関数の宣言
extern DWORD DO_AspOut(DWORD dwWriteByte, const DWORD* pbyWriteBuff);

#endif	// __DOASP_CLOSED_H__
