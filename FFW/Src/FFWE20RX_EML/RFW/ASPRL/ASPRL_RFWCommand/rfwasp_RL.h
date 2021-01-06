///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_RL.h
 * @brief
 * @author M.Yamamoto
 * @author Copyright (C) 2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/10/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴

*/
#ifndef	__RFWASP_RL_H__
#define	__RFWASP_RL_H__

#include <math.h>			/* VS2015移行時には削除予定 */
#include "rfw_typedef.h"
#include "mcudef.h"

//////////////////////////////////
// #include "ex_inc.h"
// #include "ex_com.h" 
// #include "ex_strct.h" 
// #include "k0ocd.h"
//////////////////////////////////
#include "doasp_sys_family.h"


// define定義
// -RFWASP_GetAspFunc-
//#define RF_ASPFUNC_RH_SWTRC		0x00000001
//#define RF_ASPFUNC_RH_EVTOZ		0x00000002

#define NOSET 0

// グローバル関数の宣言


// cppファイルで使用する関数のextern
#ifdef __cplusplus
extern "C" {
#endif
	extern ULONG	WINAPI	getexestatruntrc(int);
	extern int e2_GetVout(ULONG *pulVout);
	extern USHORT GetExtPwrMode(void);
	extern void SetExtPwrMode(USHORT uSetData);
	extern ULONG GetExtPwrFunc(void);
	extern int	WINAPI	CmdExecutCheck(UCHAR);
	extern int	WINAPI	EndOfCmdExecut(UCHAR, int);
	extern int	WINAPI	Memory_Read(EXMEM *);
	extern int	WINAPI	Memory_Write(EXMEM *);
#ifdef __cplusplus
};
#endif


extern DWORD CmdExecutCheck_ASP(BYTE byCommandID);
extern DWORD EndOfCmdExecut_ASP(BYTE byCommandID);


// Cファイルで定義しているdefineの再定義


#endif	// __RFWAP_RL_H__