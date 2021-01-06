///////////////////////////////////////////////////////////////////////////////
/**
 * @file prot_common.h
 * @brief BFWコマンド プロトコル生成関数(共通部)ヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#ifndef	__PROT_COMMON_H__
#define	__PROT_COMMON_H__

#include "ffw_typedef.h"
#include "mcudef.h"


/////////// 関数の宣言 ///////////
extern void	ProtInit(void);							///< BFWコマンド送信処理の初期化
extern FFWERR ProtEnd(void);						///< BFWコマンド送信処理の終了
extern FFWERR ProtSendCmd(WORD wCmdCode);			///< BFWコマンドコードの送信
extern FFWERR ProtGetStatus(WORD wCmdCode);			///< BFWコマンドのエラーコード受信
extern FFWERR ProtGetStatus_1st(WORD wCmdCode);		///< FFW起動後、最初のBFWコマンドのエラーコード受信
extern FFWERR ProtGetStatus_SetUSBT(WORD wCmdCode);	///< BFWCmd_SetUSBTコマンドのエラーコード受信
extern FFWERR ProtRcvHaltCode(WORD *const pwBuf);	///< 処理中断コードの受信
extern void InitProtData_Common(void);				///< BFWプロトコル生成部共通処理用変数初期化



#endif	// __PROT_COMMON_H__
