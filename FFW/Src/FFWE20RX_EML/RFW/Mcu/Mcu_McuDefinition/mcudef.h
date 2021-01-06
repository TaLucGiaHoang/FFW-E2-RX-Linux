///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcudef.h
 * @brief RX対応 MCU依存情報の定義ファイル
 * @author RSD H.Hatahara
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2015/12/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴

*/
#ifndef	__MCUDEF_H__
#define	__MCUDEF_H__

#include "rfw_typedef.h"
#define RFWE2_EXPORTS
#include "RFW_mini.h"	//E2拡張機能用にRFW.hから必要なものだけ絞ったヘッダ
//#include "RFWARM.h"	//E2拡張機能では不要なためコメントアウト
#include "RFWERR.h"
#include "RFWASP.h"
#ifdef ASP_RH
#include "RFWASPRH.h"
#endif
#ifdef ASP_RL
#include "RFWASPRL.h"
#endif


#endif	// __MCUDEF_H__
