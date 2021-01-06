///////////////////////////////////////////////////////////////////////////////
/**
 * @file RFWASPRH.h
 * @brief E2エミュレータRFW I/F(拡張機能) RH固有部ヘッダファイル
 * @author S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/10/21 初版作成 S.Nagai
*/
#ifndef	__RFWASPRH_H__
#define	__RFWASPRH_H__

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
// RFWASP_SetSWTraceEvent
#define RF_ASPSWTRC_PC			(0)
#define RF_ASPSWTRC_TAG			(1)
#define RF_ASPSWTRC_REG			(2)


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
typedef struct {
	BYTE	byTraceType;
	BYTE	byRegister;
	DWORD	dwData;
	DWORD	dwMask;
} RFW_SWTRC_DATA;

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

// E2拡張機能コマンド
DECLARE(RFWASPRH_SetSWTraceEvent)(BYTE byChNo, const RFW_SWTRC_DATA* pSwTraceData);
DECLARE(RFWASPRH_GetSWTraceEvent)(BYTE byChNo, RFW_SWTRC_DATA *const pSwTraceData);
DECLARE(RFWASPRH_ClrSWTraceEvent)(BYTE byChNo);

#ifdef	__cplusplus
}
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

#endif	// __RFWASPRH_H__
