///////////////////////////////////////////////////////////////////////////////
/**
 * @file doe2_closed.h
 * @brief 非公開コマンドのヘッダファイル
 * @author RSD S.Ueda
 * @author Copyright (C) 2014 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/11
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/11 上田
	新規作成
*/
#ifndef	__DOE2_CLOSED_H__
#define	__DOE2_CLOSED_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// グローバル関数の宣言
extern FFWERR DO_MONPCOM_OPEN(DWORD dwTotalLength);	///< モニタプログラム(BFW EML)データの送信開始を通知
extern FFWERR DO_MONPCOM_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW EML)データの送信
extern FFWERR DO_MONPCOM_CLOSE(void);				///< モニタプログラム(BFW EML)データの送信終了を通知
extern FFWERR DO_SetACTLED(DWORD dwActLedControl);	///< エミュレータACT-LED制御
extern FFWERR DO_SetLID(DWORD dwOffsetAddr, DWORD dwLength, const BYTE* pbyData);	///< ライセンス情報の設定
extern FFWERR DO_GetLID(DWORD dwOffsetAddr, DWORD dwLength, BYTE *const pbyData);	///< ライセンス情報の参照
extern void InitFfwCmdE2Data_Closed(void);			///< 非公開コマンド用変数初期化

#endif	// __DOE2_CLOSED_H__
