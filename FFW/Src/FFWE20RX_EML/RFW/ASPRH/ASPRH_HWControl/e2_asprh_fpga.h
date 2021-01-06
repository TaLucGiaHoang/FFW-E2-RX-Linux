///////////////////////////////////////////////////////////////////////////////
/**
* @file e2_asprh_fpga.h
* @brief FPGAのASPRH関連レジスタ定義
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

#ifndef __E2_ASPRH_FPGA_H__
#define __E2_ASPRH_FPGA_H__

// ソフトトレース
#define TRACE_CTRL			(0x07000600)
#define TRACE_CTRL_TRACEEN		(1<<0)
#define TRACE_CTRL_TRACEEVLEN0	(1<<12)
#define TRACE_CTRL_TRACEEVLEN1	(1<<13)
#define TRACE_CTRL_TRACEEVLEN2	(1<<14)
#define TRACE_CTRL_TRACEEVLEN3	(1<<15)
#define TRACE_CTRL_BREAKFLAG	(1<<31)
#define TRACE_BREAKPC		(0x07000608)

#define TRACE_EVLFACT0		(0x07000610)
#define TRACE_EVLFACT0_MATCHP	(8)
#define TRACE_EVLFACT1		(0x07000614)
#define TRACE_EVLFACT1_PC		(0)
#define TRACE_EVLFACT1_TAG		(1)
#define TRACE_EVLFACT1_REG		(2)

#define TRACE_EVLP0			(0x07000620)
#define TRACE_EVLP1			(0x07000624)
#define TRACE_EVLP2			(0x07000628)
#define TRACE_EVLP3			(0x0700062C)

#define TRACE_EVLPMSK0		(0x07000630)
#define TRACE_EVLPMSK1		(0x07000634)
#define TRACE_EVLPMSK2		(0x07000638)
#define TRACE_EVLPMSK3		(0x0700063C)


// コマンドボックス
#define COMBOX_CTRL			(0x07000700)
#define COMBOX_CTRL_CMDBOXEN		(1<<0)
#define COMBOX_CTRL_CMDRDATASW		(1<<16)
#define COMBOX_CTRL_CMDITRACEFLG	(1<<30)
#define COMBOX_CTRL_CMDBREAKFLG		(1<<31)
#define COMBOX_TRG			(0x07000704)
#define COMBOX_TRG_DISABLE		(0)
#define COMBOX_TRG_SWTRG		(1)
#define COMBOX_TRG_HWTRG		(2)
#define COMBOX_TRG_AUTORUN		(3)
#define COMBOX_TRG_MSK			(3)
#define COMBOX_TRG_BREAK_BIT	(0)
#define COMBOX_TRG_TRCSTP_BIT	(2)
#define COMBOX_TRG_STATUS_BIT	(4)
#define COMBOX_TRG_COMMAND1_BIT	(6)
#define COMBOX_TRG_COMMAND2_BIT	(8)
#define COMBOX_TRG_COMMAND3_BIT	(10)
#define COMBOX_TRG_CMDGETBREAK	(1<<26)
#define COMBOX_TRG_CMDGETITRACE	(1<<27)
#define COMBOX_TRG_CMDGETSTAT	(1<<28)
#define COMBOX_TRG_CMDGETALL1	(1<<29)
#define COMBOX_TRG_CMDGETALL2	(1<<30)
#define COMBOX_TRG_CMDGETALL3	(1<<31)


#define COM_REP				(0x07000710)
#define COM_INTERVAL		(0x07000714)
#define COM_INTERVAL_EN			(1<<31)

#define COM_STSCHK_CMPV		(0x07000720)
#define COM_STSCHK_CMPMSK	(0x07000724)

// コマンド格納アドレス
#define COMBOX_ADDR_BREAK		(0x07000BB0)
#define COMBOX_ADDR_STPREQ		(0x07000B80)
#define COMBOX_ADDR_TRCSTP		(0x07000B40)
#define COMBOX_ADDR_STATUS		(0x07000B00)
#define COMBOX_ADDR_COMMAND3	(0x07000AC0)
#define COMBOX_ADDR_COMMAND2	(0x07000A80)
#define COMBOX_ADDR_COMMAND1	(0x07000A00)

// コマンドのbit
#define COMBOX_CMD_END				(1<<31)
#define COMBOX_CMD_READ				(1<<30)
#define COMBOX_CMD_WRITE			(0<<30)
#define COMBOX_CMD_TRACEOFF			(1<<25)
#define COMBOX_CMD_REG_FSMCTRL		(2<<25)
#define COMBOX_CMD_REG_CBUFCMD		(3<<25)
#define COMBOX_CMD_REG_TBUFHDAT		(4<<25)
#define COMBOX_CMD_REG_TBUFLDAT		(5<<25)
#define COMBOX_CMD_TRACERESET		(0xC<<25)
#define COMBOX_CMD_RDATASW			(0xD<<25)
#define COMBOX_CMD_AUTORUN			(0xE<<25)
#define COMBOX_CMD_TYPE_PC			(0<<16)
#define COMBOX_CMD_TYPE_STATUS		(0xF<<16)

// 固有部リセット
#define RH850_RST_CTRL			(0x07000C00)

#define RH850RSTCTRL_RST_RH850ALL	(1<<31)
#define RH850RSTCTRL_RST_RH850REG	(1<<23)
#define RH850RSTCTRL_RST_HCBREG		(1<<17)
#define RH850RSTCTRL_RST_STREG		(1<<16)
#define RH850RSTCTRL_RST_ASPFUNC	(1<<15)
#define RH850RSTCTRL_RST_RH850		(1<<7)
#define RH850RSTCTRL_RST_HCB		(1<<1)
#define RH850RSTCTRL_RST_ST			(1<<0)

#endif
