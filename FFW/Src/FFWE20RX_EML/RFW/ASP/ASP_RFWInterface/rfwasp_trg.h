///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_trg.h
 * @brief 外部トリガコマンドのヘッダファイル
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
#ifndef	__RFWASP_TRG_H__
#define	__RFWASP_TRG_H__

#include "rfw_typedef.h"
#include "mcudef.h"


// define定義

/* 外部トリガ出力パルス幅 */
#define RF_ASPTRGPLS_MIN		(1)			// 最小パルス幅
#define RF_ASPTRGPLS_MAX		(0xFFFF)	// 最大パルス幅

// グローバル関数の宣言

#endif	// __RFWASP_TRG_H__
