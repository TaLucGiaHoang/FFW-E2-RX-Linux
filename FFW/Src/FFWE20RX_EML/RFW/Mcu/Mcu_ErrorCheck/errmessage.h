///////////////////////////////////////////////////////////////////////////////
/**
 * @file errmessage.h
 * @brief エラーメッセージ定義ヘッダ
 * @author RSD H.Hatahara
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2014/04/13 初版 H.Hatahara
*/
#ifndef	__ERRMESSAGE_h__
#define	__ERRMESSAGE_h__

#include "rfw_typedef.h"
#include "mcudef.h"

// 型定義
typedef struct {
	DWORD	dwRfwErrNo;
	WCHAR	*pswzErrMsgJp;
	WCHAR	*pswzErrMsgEn;
} ERRMSG_DATA;


// 外部参照の定義
extern const ERRMSG_DATA* GetErrMsgPtr(void);

#endif	// __ERRMESSAGE_h__
