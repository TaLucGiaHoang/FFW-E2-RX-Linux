////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_runset.cpp
 * @brief プログラム実行中の設定変更コマンドのソースファイル
 * @author RSO Y.Miyake
 * @author Copyright (C) 2012 Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/10/17
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo120910-001	2012/10/17 三宅
  FFW I/F仕様変更に伴うFFWソース変更による新規ファイル作成。
　・FFWMCUCmd_SetRSST()、FFWMCUCmd_SetRSEN() の空の関数定義追加。
*/
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#elif __linux__
#endif

#include "ffwrx_runset.h"


//==============================================================================
/**
 * ユーザプログラム実行中のイベントブレーク設定、トレース設定の開始を通知する。
 * @param なし
 * @retval FFWERR_OK 正常終了
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_SetRSST(void)
{
	return FFWERR_OK;
}


//==============================================================================
/**
 * ユーザプログラム実行中のイベントブレーク設定、トレース設定の終了を通知する。
 * @param なし
 * @retval FFWERR_OK 正常終了
 */
//==============================================================================
FFWE100_API DWORD FFWMCUCmd_SetRSEN(void)
{
	return FFWERR_OK;
}


