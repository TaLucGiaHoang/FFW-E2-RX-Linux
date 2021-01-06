///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_brk.h
 * @brief ターゲットMCU ブレーク関係制御関数のヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
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
#ifndef	__MCU_BRK_H__
#define	__MCU_BRK_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義


// グローバル関数の宣言

extern FFWERR WriteBrkCode(MADDR dwmadrAddr, BYTE* pbyUsrCode);	///< ユーザ命令コードを退避させ、BRK命令コードを書込む
extern FFWERR WriteUserCode(MADDR dwmadrAddr);	///< 退避させていたユーザ命令コードを復帰する

extern FFWERR EscCmdData(MADDR madrStartAddr, BYTE* pbyUsrCode);	///< 命令コードを退避する
extern FFWERR RetCmdData(MADDR madrStartAddr);						///< 命令コードを復帰する
extern FFWERR FillCmdData(MADDR madrStartAddr);						///< 命令コードを埋め込む

extern FFWERR ReplaceCmdData(void);						///< S/Wブレーク設定アドレスの命令コード置換

extern FFWERR GetPbSetCmdData(MADDR madrStartAddr, MADDR madrEndAddr, BYTE* pbyBuff);	///< S/Wブレーク設定アドレスのデータ取得処理

extern BOOL GetCmdRetFillFlg(void);
extern void SetCmdRetFillFlg(BOOL bFlg);

extern void InitMcuData_Brk(void);						///< ターゲットMCUブレーク制御関数用変数初期化

#endif	// __MCU_BRK_H__
