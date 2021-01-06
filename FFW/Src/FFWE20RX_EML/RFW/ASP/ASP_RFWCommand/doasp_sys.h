///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_sys.h
 * @brief E2拡張機能システムコマンドのヘッダファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/10/20 新規作成 S.Nagai
*/
#ifndef	__DOASP_SYS_H__
#define	__DOASP_SYS_H__

#include "rfw_typedef.h"
#include "mcudef.h"

typedef struct {
	DWORD	dwFactSw;		// E2トリガの設定状況。設定済み(1)、未設定(0)
	DWORD	dwFactType;		// E2トリガのタイプ。パルス型(1)、レベル型(0)
	UINT64	u64LinkAct;		// リンクされているアクション情報
} RFW_ASPE2TRG_INFO;

typedef struct {
	DWORD	dwActSw;		// E2アクションの設定状況。設定済み(1)、未設定(0)
	DWORD	dwActType;		// E2アクションのタイプ。パルス型(0)、レベル型(1)、どちらも可(2)
	UINT64	u64LinkFact;	// リンクされている要因情報
} RFW_ASPE2ACT_INFO;


// define定義
#define RFW_ASPFUNC_SET			(0)
#define RFW_ASPFUNC_CLR			(1)

#define RFW_EVENTLINK_SWOFF		(0)	// E2トリガアクションOFF
#define RFW_EVENTLINK_SWON		(1)	// E2トリガアクションON

#define RFW_EVENTLINK_NONSEL	(0)	// 未設定
#define RFW_EVENTLINK_PULSE		(1)	// パルス型
#define RFW_EVENTLINK_LEVEL		(2)	// レベル型
#define RFW_EVENTLINK_BOTH		(3)	// パルス型とレベル型どちらも可

#define RF_ASPFUNC_PC_FULLSTOP	(1<<4)	// PCストレージのフルストップ
#define RF_ASPFUNC_PC_RING		(1<<3)	// PCストレージのリングモード
#define RF_ASPFUNC_E2_FULLBRK	(1<<2)	// E2ストレージのフルブレーク
#define RF_ASPFUNC_E2_FULLSTOP	(1<<1)	// E2ストレージのフルストップ
#define RF_ASPFUNC_E2_RING		(1<<0)	// E2ストレージのリングモード

// -AspConf-
#define RF_ASPCONF0_MASK		(0x0000007F)
#define RF_ASPCONF1_MASK		(0x0000000F)
#define RF_ASPCONF2_MASK		(0x00001F80)
#define RF_ASPCONF3_MASK		(0x000FF800)

// E2 ACT Ch
#define RF_ASPE2ACTCH_PERCH0_START	0x00		// Pulse
#define RF_ASPE2ACTCH_PERCH0_END	0x01		// Pulse
#define RF_ASPE2ACTCH_PERCH0_SECT	0x02		// Level
#define RF_ASPE2ACTCH_PERCH1_START	0x03		// Pulse
#define RF_ASPE2ACTCH_PERCH1_END	0x04		// Pulse
#define RF_ASPE2ACTCH_PERCH1_SECT	0x05		// Level
#define RF_ASPE2ACTCH_EXTRGINCH0	0x06		// Level/Pulse
#define RF_ASPE2ACTCH_EXTRGINCH1	0x07		// Level/Pulse
#define RF_ASPE2ACTCH_PWRMON		0x08		// Level
#define RF_ASPE2ACTCH_PC			0x09		// -
#define RF_ASPE2ACTCH_BRKEV			0x0A		// Level/Pulse
#define RF_ASPE2ACTCH_TRCEV			0x0B		// Level/Pulse


// E2 TRG Ch
#define RF_ASPE2TRGCH_PERCH0				0x00		// Pulse
#define RF_ASPE2TRGCH_PERCH1				0x01		// Pulse
#define RF_ASPE2TRGCH_EXTRGOUTCH0			0x02		// Level
#define RF_ASPE2TRGCH_EXTRGOUTCH1			0x03		// Pulse
#define RF_ASPE2TRGCH_RESETEV				0x04		// Pulse
#define RF_ASPE2TRGCH_EVTOZ					0x05		// -
#define RF_ASPE2TRGCH_COMUMONCH0_MATCH		0x06		// Pulse
#define RF_ASPE2TRGCH_COMUMONCH1_MATCH		0x08		// Pulse
#define RF_ASPE2TRGCH_PWRMON_LEVEL			0x0A		// Level
#define RF_ASPE2TRGCH_PWRMON_PULSE			0x0B		// Pulse
#define RF_ASPE2TRGCH_PWRMON_BRKEV			0x0C		// Pulse/Level
#define RF_ASPE2TRGCH_MONEVCH0				0x0D		// Pulse
#define RF_ASPE2TRGCH_MONEVCH1				0x0E		// Pulse
#define RF_ASPE2TRGCH_MONEVCH2				0x0F		// Pulse
#define RF_ASPE2TRGCH_MONEVCH3				0x10		// Pulse
#define RF_ASPE2TRGCH_MONEVCH4				0x11		// Pulse
#define RF_ASPE2TRGCH_MONEVCH5				0x12		// Pulse
#define RF_ASPE2TRGCH_MONEVCH6				0x13		// Level
#define RF_ASPE2TRGCH_SWTRCCH0				0x14		// Pulse
#define RF_ASPE2TRGCH_SWTRCCH1				0x15		// Pulse
#define RF_ASPE2TRGCH_SWTRCCH2				0x16		// Pulse
#define RF_ASPE2TRGCH_SWTRCCH3				0x17		// Pulse


// グローバル関数の宣言
#ifdef __cplusplus
extern "C" {
#endif
extern DWORD DO_SetAspSw(BYTE byAspSwitch);
extern DWORD DO_SetAspConfiguration(const RFW_ASPCONF_DATA *pAspConfiguration);
#ifdef __cplusplus
}
#endif
extern DWORD DO_GetAspSw(BYTE *pbyAspSwitch);
extern DWORD DO_GetAspFunc(DWORD *pdwAspCommonFunction, DWORD *pdwAspSeriesFunction, DWORD *pdwAspStorage, DWORD *pdwAspOption);
extern DWORD DO_GetAspConfiguration(RFW_ASPCONF_DATA const *pAspConfiguration);
extern void SaveSingleEvent(DWORD dwFactChNo, UINT64 u64ActCh);
extern void ClearSingleEvent(UINT64 u64ActCh);
extern void GetE2TrgTable(DWORD dwChNo, RFW_ASPE2TRG_INFO *pAspE2TrgInfo);
extern void SetE2TrgTable(DWORD dwChNo, DWORD dwFactType);
extern void ClrE2TrgTable(DWORD dwChNo);
extern void GetE2ActTable(DWORD dwChNo, RFW_ASPE2ACT_INFO *pAspE2actInfo);
extern void SetE2ActTable(DWORD dwChNo, DWORD dwType, UINT64 u64FactEvent);
extern void ClrE2ActTable(DWORD dwChNo);
extern DWORD CheckSingleEvent(DWORD dwFunc, UINT64 u64SingleEvent, DWORD dwActType, DWORD dwActChNo);
extern void GetAspConfData(RFW_ASPCONF_DATA const *pAspConfiguration);
extern DWORD ChkInvalidAspFunction(const RFW_ASPCONF_DATA *pAspConfiguration);
extern DWORD ChkSupportAspFunction(const RFW_ASPCONF_DATA *pAspConfiguration);


// E2トリガCH番号
#define RF_E2ASP_TRG_CH_BRKEV	(0x0C)

// s_AspE2actTable 
#define RF_E2TRG_TABLE_MAX	(0x40)
#define RF_E2ACT_TABLE_MAX	(0x40)

#ifdef	__cplusplus
extern	"C" {
#endif
extern void InitAsp(void);
extern void InitAspPinConf(void);
extern BOOL IsAspOn(void);
#ifdef	__cplusplus
};
#endif

#endif	// __DOASP_SYS_H__
