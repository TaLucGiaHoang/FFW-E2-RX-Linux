///////////////////////////////////////////////////////////////////////////////
/**
 * @file prote2_sys.h
 * @brief BFWコマンド プロトコル生成関数(システム関連)ヘッダファイル
 * @author PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/02/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
*/
#ifndef	__PROTE2_SYS_H__
#define	__PROTE2_SYS_H__

#include "ffw_typedef.h"
#include "mcudef.h"


/////////// 関数の宣言 ///////////
extern FFWERR PROT_EXPWR2(enum FFWENM_EXPWR2_MODE eExPwr2Mode, WORD wExPwr2Val, enum FFWENM_EXPWR2_SUPPLY eExPwr2Sply);	///< ターゲットシステムへの電源供給(リニア指定)(E2用)
extern FFWERR PROT_CHGPWR(WORD wChgPwrVal, DWORD dwChgPwrWait);		///< ターゲットシステムへの電源供給電圧の変更(リニア指定)(E2用)
extern FFWERR PROT_GetPwrSrc(BYTE* pbySupply);						///< 電源供給ソース情報取得(E2用)

#endif	// __PROTE2_SYS_H__
