///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_RL.cpp
 * @brief
 * @author M.Yamamoto
 * @author Copyright (C) 2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/10/28
 */
///////////////////////////////////////////////////////////////////////////////

/*
■改定履歴
	新規作成
*/

#include <windows.h>
#include <stdio.h>

#include "ex_inc2.h"

#include "rfwasp_RL.h"
#include "getstat.h"
#include "socunit.h"
#include "k0ocd.h"
#include "ereg_acc.h"
#include "ex_com.h"
#include "extpower.h"
#include "uqbe1e20.h"

// マクロ定義

// static変数

// static関数





DWORD CmdExecutCheck_ASP(BYTE byCommandID)
{
	WORD wRet;

	if ( (wRet = (WORD)CmdExecutCheck(byCommandID)) != EX_NOERROR) {
		if (rsucheck() != 1) {
			return RFWERR_N_BMCU_NONCONNECT;
		}
	}

	return (DWORD)wRet;
}
DWORD EndOfCmdExecut_ASP(BYTE byCommandID)
{
	return EndOfCmdExecut(byCommandID, RFWERR_OK);
}



