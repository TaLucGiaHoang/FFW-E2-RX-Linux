///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_RX.h
 * @brief
 * @author REL M.Yamamoto
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/8/23
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴

*/
#ifndef	__RFWASP_RX_H__
#define	__RFWASP_RX_H__

#include <math.h>			/* VS2015移行時には削除予定 */
#include "rfw_typedef.h"
#include "mcudef.h"


#include "doasp_sys_family.h"


// define定義
// -RFWASP_GetAspFunc-
//#define RF_ASPFUNC_RH_SWTRC		0x00000001
//#define RF_ASPFUNC_RH_EVTOZ		0x00000002

#define NOSET 0

// グローバル関数の宣言


// cppファイルで使用する関数のextern



extern DWORD CmdExecutCheck_ASP(BYTE byCommandID);
extern DWORD EndOfCmdExecut_ASP(BYTE byCommandID);


// Cファイルで定義しているdefineの再定義


#endif	// __RFWASP_RX_H__
