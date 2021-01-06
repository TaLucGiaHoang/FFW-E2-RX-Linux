/**
 * @file RFW_min.h
 * @brief E2エミュレータRFW I/F(共通)ヘッダファイル
 * @brief ※E2拡張機能で必要なもののみ
 * @author S.Nagai
 * @author Copyright (C) 2015-2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/10/05
 */
/*
■改定履歴
・2017/10/05 RFW.hからE2拡張機能で必要なものを抜粋 S.Nagai
*/
#ifndef	__RFW_MIN_H__
#define	__RFW_MIN_H__

#ifdef __GNUC__
#include <inttypes.h>
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

//-----------------------------------------------------------------------
// DEFINE定義
//-----------------------------------------------------------------------
#define CORE_MAX				(16)		// サポートコア最大数

//コア番号の定義
#define RF_PE1					(0x00000000)
#define RF_PE2					(0x00000001)
#define RF_PE3					(0x00000002)
#define RF_PE4					(0x00000003)
#define RF_PE5					(0x00000004)
#define RF_PE6					(0x00000005)
#define RF_PE7					(0x00000006)
#define RF_PE8					(0x00000007)
#define RF_PE9					(0x00000008)
#define RF_PE10					(0x00000009)
#define RF_PE11					(0x0000000A)
#define RF_PE12					(0x0000000B)
#define RF_PE13					(0x0000000C)
#define RF_PE14					(0x0000000D)
#define RF_PE15					(0x0000000E)
#define RF_PE16					(0x0000000F)

// --- DEFINE定義 システムコマンド --------
////// RFW_GetErrInfo
#define	ERRMSGINFO_ERRMSG_MAX	(1024)
#define ERRMSGINFO_MSGSIZE_MAX	(4096)
// byMsgLanguage
#define RF_MSG_ENG				(0x00)		// 英語メッセージ
#define RF_MSG_JPN				(0x01)		// 日本メッセージ
// pbyErrType
#define RF_ERR_COMMON			(0x00)		// 共通エラー
#define RF_ERR_CORE				(0x01)		// 個別エラー


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
	DWORD	dwCoreInfo;
	DWORD	dwRfwErrNo;
	WCHAR	swzRfwErrMsg[ERRMSGINFO_ERRMSG_MAX + 1];
} RFW_ERRMSGINFO_DATA;



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

#if defined(_MSC_VER)
#if defined(RFWE2_EXPORTS)
//----- RFW用宣言 -----//
#if defined(DEFFILE_EXPORTS)
/* 
 * dllexportとWINAPI(= __stdcall)を使ってexportするとC++関数名はmangleされてしまう。
 * そのためC++の関数をcの名前かつWINAPI(__stdcall)でEXPORTするには
 * defファイルを使う
 */
#define DECLARE(fnc) DWORD WINAPI fnc
#else
#define DECLARE(fnc)	__declspec(dllexport) DWORD fnc
#endif
#else
//----- Debugger, NDEB用 -----//
#define DECLARE(fnc)	__EXTERN__ DWORD (*fnc)
#endif
#else
//----- Debugger用 -----//
#define DECLARE(fnc)	__EXTERN__ DWORD (*fnc)
#endif

// システムコマンド
DECLARE(RFW_GetErrInfo)(BYTE byMsgLanguage, DWORD *const pdwErrType, RFW_ERRMSGINFO_DATA *const pErrMsgInfo);
// 非公開コマンド2(デバッグ用コマンド)
DECLARE(RFW_VendorControlIn)(BYTE req, WORD val, WORD index, BYTE *pbyIn, WORD len);
DECLARE(RFW_VendorControlOut)(BYTE req, WORD val, WORD index);

#ifdef	__cplusplus
}
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

#endif	// __RFW_MIN_H__
