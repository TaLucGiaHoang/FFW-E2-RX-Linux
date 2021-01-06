///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasprh_swtrc.h
 * @brief 外部ソフトトレースイベントのヘッダファイル
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
#ifndef	__DOASPRH_SWTRC_H__
#define	__DOASPRH_SWTRC_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義
#define RF_ASPSWTRC_ON	1
#define RF_ASPSWTRC_OFF	0

// グローバル関数の宣言
extern DWORD DO_SetSwtrcEv(BYTE byChNo, const RFW_SWTRC_DATA *pSwTrcData);
extern DWORD DO_GetSwtrcEv(BYTE byChNo, RFW_SWTRC_DATA *const pSwTrcData);
extern DWORD DO_ClrSwtrcEv(BYTE byChNo);
extern void InitAspSwTrcEv(BYTE byChNo);
extern BOOL IsAspSwTrcEvEna(void);
extern BYTE GetAspSwTrcEvEna(BYTE byChNo);
extern void GetSwTrcData(BYTE byChNo, RFW_SWTRC_DATA *pSwTrcData);

#ifdef __cplusplus
extern "C" {
#endif
extern BOOL IsAspSwTrcEna(void);
#ifdef __cplusplus
}
#endif

#endif	// __DOASPRH_SWTRC_H__
