///////////////////////////////////////////////////////////////////////////////
/**
 * @file prote2_closed.h
 * @brief BFWコマンド プロトコル生成関数(非公開関連)ヘッダファイル
 * @author RSD S.Ueda
 * @author Copyright (C) 2014 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/10 上田
	新規作成
*/
#ifndef	__PROTE2_CLOSED_H__
#define	__PROTE2_CLOSED_H__

#include "ffw_typedef.h"
#include "mcudef.h"

/////////// 関数の宣言 ///////////
extern FFWERR PROT_SetACTLED(DWORD dwActLedControl);	///< BFWCmd_SetActLedコマンドの発行
extern FFWERR PROT_SetLID(DWORD dwOffsetAddr, DWORD dwLength, const BYTE* pbyData);	///< BFWCmd_SetLIDコマンドの発行
extern FFWERR PROT_GetLID(DWORD dwOffsetAddr, DWORD dwLength, BYTE *const pbyData);	///< BFWCmd_GetLIDコマンドの発行
void InitProtE2Data_Closed(void);	///< 非公開コマンド用変数初期化

#endif	// __PROTE2_CLOSED_H__
