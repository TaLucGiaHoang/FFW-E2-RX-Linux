///////////////////////////////////////////////////////////////////////////////
/**
 * @file RFWASPRL.h
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
#ifndef	__RFWASPRL_H__
#define	__RFWASPRL_H__

#ifdef __GNUC__
#include <inttypes.h>
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif


//-----------------------------------------------------------------------
// DEFINE定義
//-----------------------------------------------------------------------

// --- DEFINE定義 拡張機能コマンド --------


// --- DEFINE定義 拡張機能コマンドID ------


//-----------------------------------------------------------------------
// 変数の型定義
//-----------------------------------------------------------------------
#ifdef __GNUC__
typedef int						BOOL;
typedef uint8_t					BYTE;
typedef uint16_t				WORD;
typedef uint32_t				DWORD;
typedef uint64_t				UINT64;
typedef wchar_t					WCHAR;
#endif

//-----------------------------------------------------------------------
// 構造体定義
//-----------------------------------------------------------------------

/* RFWASP_SetMonitorEvent() */
typedef struct {
	UINT64	u64EventAddr; 
	DWORD	dwFuncMode;
	BYTE	byDetectType;
} RFW_ASPMONEVCOND_DATA;




//-----------------------------------------------------------------------
// I/F関数の宣言
//-----------------------------------------------------------------------
#ifdef RFWE2_INSTANCE
#define    __EXTERN__
#else
#define    __EXTERN__	 extern
#endif

#ifdef	__cplusplus
extern	"C" {
#endif
	
//-----------------------------------------------------------------------
// DEFINE定義
//-----------------------------------------------------------------------

// --- DEFINE定義 拡張機能コマンド --------
// RFWASP_SetMonitorEvent
/// byEventNo
#define RF_ASPMONEV_EVNO_MIN		(0xE0)
#define RF_ASPMONEV_EVNO_MAX		(0xFF)

/// dwFuncMode
#define	RF_ASPMONEV_RECORD			(0)
#define	RF_ASPMONEV_MESSAGE			(1)
#define	RF_ASPMONEV_TIMER0_STAR		(1 << 2)
#define	RF_ASPMONEV_TIMER0_STOP		(1 << 3)
#define	RF_ASPMONEV_TIMER1_START	(1 << 4)
#define	RF_ASPMONEV_TIMER1_STOP		(1 << 5)

/// byDetectType
#define	RF_ASPMONEV_TYPE_HWBRK		(0x00)
#define	RF_ASPMONEV_TYPE_SWBRK		(0x01)

// RFWASP_SetMonitorEvent byChno
#define MONEV_TRG_CH0	(0x00)
#define MONEV_TRG_CH1	(0x01)
#define MONEV_TRG_CH2	(0x02)
#define MONEV_TRG_CH3	(0x03)
#define MONEV_TRG_CH4	(0x04)
#define MONEV_TRG_CH5	(0x05)
#define MONEV_TRG_CH6	(0x06)
#define MONEV_TRG_MAX	(0x07)

// RFWASP_SetMonitorEvent byData(ch6)
#define MONEV_CH6_OFF	(0x00)
#define MONEV_CH6_ON	(0x01)


// RFWASP_GetMonitorEventData byDataType
#define RF_MONEVDATA_PASSCOUNT	(0x00)

// RFWASP_SetMonitorEventFactor
// E2拡張機能コマンド
DECLARE(RFWASP_SetMonitorEvent)(BYTE byEventNo, const RFW_ASPMONEVCOND_DATA* pAspMonitorEvent);
DECLARE(RFWASP_GetMonitorEvent)(BYTE byEventNo, RFW_ASPMONEVCOND_DATA *const pAspMonitorEvent);
DECLARE(RFWASP_ClrMonitorEvent)(BYTE byEventNo);
DECLARE(RFWASP_SetMonitorEventFactor)(BYTE byChNo, BYTE byData,	BYTE byMask);
DECLARE(RFWASP_GetMonitorEventFactor)(BYTE byChNo, BYTE *const pbyData, BYTE *const pbyMask);
DECLARE(RFWASP_ClrMonitorEventFactor)(BYTE byChNo);
DECLARE(RFWASP_GetMonitorEventData)(BYTE byDataType, BYTE byEventNo, UINT64 *const pu64Data1, UINT64 *const pu64Data2);

#ifdef	__cplusplus
};
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif


#endif	// __RFWASPRL_H__
