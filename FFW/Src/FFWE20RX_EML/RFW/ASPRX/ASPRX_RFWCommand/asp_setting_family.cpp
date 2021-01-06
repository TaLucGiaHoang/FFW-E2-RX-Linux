///////////////////////////////////////////////////////////////////////////////
/**
* @file asp_setting_family.cpp
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

#include "asp_setting_family.h"
#include "rfwasprx_monev.h"
#include "doasprx_monev.h"
#ifdef __cplusplus
extern	int e2_CPU_R(ULONG addr, ULONG len, UCHAR *data);
extern	int e2_CPU_W(ULONG addr, ULONG len, UCHAR *data);
#else
extern "C" {
	int e2_CPU_R(ULONG addr, ULONG len, UCHAR *data);
	int e2_CPU_W(ULONG addr, ULONG len, UCHAR *data);
}
#endif
#include "e2_asp_fpga.h"
#include "e2_asprx_fpga.h"

void Start_ASP_Family(void)
{
	// 処理なし
}
//現状Communi.dllとのI/Fが無いためダミー関数とする
int COM_VendorControlIn(
	BYTE req,
	WORD val,
	WORD index,
	BYTE *pbyIn,
	WORD len)
{
	req;	// Warning回避(RX用暫定処置)
	val;	// Warning回避(RX用暫定処置)
	index;	// Warning回避(RX用暫定処置)
	pbyIn;	// Warning回避(RX用暫定処置)
	len;	// Warning回避(RX用暫定処置)

	return TRUE;
}
//現状Communi.dllとのI/Fが無いためダミー関数とする
int COM_VendorControlOut(
	BYTE req,
	WORD val,
	WORD index,
	const BYTE *pbyOut,
	WORD len)
{
	req;	// Warning回避(RX用暫定処置)
	val;	// Warning回避(RX用暫定処置)
	index;	// Warning回避(RX用暫定処置)
	pbyOut;	// Warning回避(RX用暫定処置)
	len;	// Warning回避(RX用暫定処置)

	return TRUE;
}

