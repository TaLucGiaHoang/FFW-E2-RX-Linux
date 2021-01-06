///////////////////////////////////////////////////////////////////////////////
/**
* @file asp_setting_family.cpp
* @brief
* @author M.Yamamoto
* @author Copyright (C) 2016 Renesas Electronics Corporation and
* @author Renesas System Design Co., Ltd. All rights reserved.
* @date 2016/12/02
*/
///////////////////////////////////////////////////////////////////////////////

/*
■改定履歴
新規作成
*/

#include "asp_setting_family.h"
#include "rfwasprl_monev.h"
#include "doasprl_monev.h"
extern "C" {
	int e2_CPU_R(ULONG addr, ULONG len, UCHAR *data);
	int e2_CPU_W(ULONG addr, ULONG len, UCHAR *data);
}
#include "e2_asp_fpga.h"
#include "e2_asprl_fpga.h"

void Start_ASP_Family(void)
{
	if (IsMonitorEventEna()) {
		/* モニタイベント有効 */
		FPGA_SET_PAT(MONEV_MEOEN, MONEV_MEOEN_BIT);

	}

}

