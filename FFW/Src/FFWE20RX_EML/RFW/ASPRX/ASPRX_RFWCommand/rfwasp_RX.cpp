///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_RX.cpp
 * @brief
 * @author REL M.Yamamoto
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/8/23
 */
///////////////////////////////////////////////////////////////////////////////

/*
■改定履歴
	新規作成
*/

#include <windows.h>
#include <stdio.h>


#include "rfwasp_RX.h"
// RevRxE2No171004-001 Append Line
#include "ffwmcu_mcu.h"		// getIdCodeResultState()呼び出し用

// マクロ定義

// static変数

// static関数



//DWORD CmdExecutCheck_ASP(BYTE byCommandID)
//{
//	byCommandID;	// Warning回避(RX用暫定処置)
//	return RFWERR_OK;
//}
DWORD CmdExecutCheck_ASP(BYTE wCommandID)
{
	wCommandID;	// Warning回避(RX用暫定処置)

	// RevRxE2No171004-001 Append Start
	// ID認証確認
	if (getIdCodeResultState() == FALSE) {	// 起動時(ID認証結果が未設定状態)
		return RFWERR_N_BMCU_NONCONNECT;
	}

	return RFWERR_OK;
	// RevRxE2No171004-001 Append End
}
//DWORD EndOfCmdExecut_ASP(BYTE byCommandID)
//{
//	byCommandID;	// Warning回避(RX用暫定処置)
//	return RFWERR_OK;
//}
DWORD EndOfCmdExecut_ASP(BYTE wCommandID)
{
	wCommandID;	// Warning回避(RX用暫定処置)
	return RFWERR_OK;
}



