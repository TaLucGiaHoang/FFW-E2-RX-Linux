///////////////////////////////////////////////////////////////////////////////
/**
* @file asprh_setting.h
* @brief ASPRH FPGA setting
* @author RSD S.Nagai
* @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
* @author Renesas System Design Co., Ltd. All rights reserved.
* @date 2017/03/28
*/
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/11/29 新規作成 S.Nagai
*/

#ifndef _ASPRH_SETTING_H_
#define _ASPRH_SETTING_H_

#include "rfw_typedef.h"
#include "mcudef.h"


#ifdef __cplusplus
extern "C" {
#endif
extern void StopCMDBOX(void);
extern BOOL ReleaseBrkCmd(void);
#ifdef __cplusplus
}
#endif

extern void EnableSwTrcEv(void);
extern void SwTrcResetRelease(void);
extern void EnableBrkEv(void);
extern void EnableDetectBrk_CMDBOX(void);
extern void EnableStopTrcEv(void);
extern void AspRun_RH(void);

#endif
