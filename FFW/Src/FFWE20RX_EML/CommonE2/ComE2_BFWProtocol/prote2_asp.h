///////////////////////////////////////////////////////////////////////////////
/**
 * @file prote2_asp.h
 * @brief BFWコマンド プロトコル生成関数(E2 拡張機能関連)ヘッダファイル
 * @author PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2No171004-001 2017/10/04 PA 辻
	新規作成
	E2拡張機能対応
*/
#ifndef	__PROTE2_ASP_H__
#define	__PROTE2_ASP_H__

#include "ffw_typedef.h"


/////////// 関数の宣言 ///////////
extern FFWERR PROT_SET_ASPCONF(BYTE byBfwSendData);		///< BFWへのE2拡張機能の有効状態の通知(E2用)
extern FFWERR PROT_SET_MONITORPOINTADDR(void);			///< BFWへの通過ポイントの設定条件の通知(E2用)

#endif	// __PROTE2_ASP_H__
