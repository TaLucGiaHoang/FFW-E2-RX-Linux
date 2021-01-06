///////////////////////////////////////////////////////////////////////////////
/**
 * @file protasp_cpu.h
 * @brief BFWコマンド プロトコル生成関数(制御CPU空間アクセス関連)ヘッダファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/09/09 
*/
#ifndef	__PROTASP_CPU_H__
#define	__PROTASP_CPU_H__

#include "rfw_typedef.h"
#include "mcudef.h"

/////////// define定義 ///////////


/////////// 関数の宣言 ///////////
///< BFWCmd_ASP_INコマンドの発行
extern DWORD ProtAspIn(DWORD dwReadByte, DWORD *const pbyReadData);
///< BFWCmd_ASP_OUTコマンドの発行
extern DWORD ProtAspOut(DWORD dwWriteByte, const DWORD* pbyWriteData);
///< BFWCmd_ASP_SETPORTコマンドの発行
extern DWORD ProtAspSetPort(BYTE bUsbRdyN, BYTE bBfwRdyN);

///< BFWCmd_ASP_INコマンドの発行
extern DWORD PROT_AspIn(DWORD dwReadByte, DWORD *const pbyReadData);
///< BFWCmd_ASP_OUTコマンドの発行
extern DWORD PROT_AspOut(DWORD dwWriteByte, const DWORD* pbyWriteData);
///< BFWCmd_ASP_SETPORTコマンドの発行
extern DWORD PROT_AspSetPort(BYTE bUsbRdyN, BYTE bBfwRdyN);
#endif	// __PROTASP_CPU_H__
