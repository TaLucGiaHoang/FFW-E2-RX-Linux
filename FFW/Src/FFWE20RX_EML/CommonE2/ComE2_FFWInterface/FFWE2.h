///////////////////////////////////////////////////////////////////////////////
/**
 * @file FFWE2.h//
 * @brief E2 LiteエミュレータFFW API関数のヘッダファイル//
 * @author RSD S.Ueda, PA M.Tsuji//
 * @author Copyright (C) 2014(2017) Renesas Electronics Corporation and//
 * @author Renesas System Design Co., Ltd. All rights reserved.//
 * @date 2017/02/01//
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴//
・RevRxE2LNo141104-001 2014/11/07 上田//
	新規作成//
・RevRxE2No170201-001 2017/02/01 PA 辻//
	E2 対応//
*/
#ifndef	__FFWE2_H__
#define	__FFWE2_H__


#ifdef _WIN32
#ifndef	FFWE100_API
#ifdef FFWE20_EXPORTS
	#define FFWE100_API __declspec(dllexport)
#else
	#define FFWE100_API __declspec(dllimport)
	#ifdef FFWE100_INSTANCE
		#define	__EXTERN__
	#else
		#define	__EXTERN__	extern
	#endif
#endif
#endif
#else // __linux__
#define FFWE20_EXPORTS
#define FFWE100_API 
#define __EXTERN__	extern
#endif

#ifdef	__cplusplus
	extern	"C" {
#endif


/////////// enum定義 ///////////
//---------------------------
// システムコマンド//
//---------------------------
// RevRxE2No170201-001 Append Start
// EXPWR2
enum FFWENM_EXPWR2_MODE {		// ユーザ基板への電源供給モードを設定//
	EXPWR2_DISABLE = 0,				// 電源供給しない//
	EXPWR2_USER,					// 20pinコネクタから電源供給する//
	EXPWR2_ASP,						// ASP I/Fから電源供給する//
	EXPWR2_TARGET_ASPIN,			// 外部電源を20pinコネクタから取り込み、ASP回路に供給してASPを使用可能にする//
	EXPWR2_USER_ASP,				// 20pinコネクタとASP I/Fの両方から電源供給する//
};
// RevRxE2No170201-001 Append End

//////// FFW I/F関数の宣言 ////////
//----- FFW用宣言 -----//
#ifdef FFWE20_EXPORTS

// システムコマンド//
// RevRxE2No170201 - 001 Append Line
extern FFWE100_API	DWORD FFWE2Cmd_EXTPWR2(enum FFWENM_EXPWR2_MODE eExPwr2Mode,	BYTE byExPwr2Val);	///< ユーザ基板への電源供給(リニア指定)を設定//

// 非公開コマンド(デバッグ用)//
// 以下のコマンドは、F/W、H/Wデバッグ用コマンドである。NDEBとの組み合わせでのみ使用し、デバッガでは使用しない。//
extern FFWE100_API	DWORD FFWE2Cmd_MONPCOM_OPEN(DWORD dwTotalLength);				///< モニタプログラム(BFW) COMモジュールのダウンロード開始//
extern FFWE100_API	DWORD FFWE2Cmd_MONPCOM_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW) COMモジュールの送信//
extern FFWE100_API	DWORD FFWE2Cmd_MONPCOM_CLOSE(void);								///< モニタプログラム(BFW) COMモジュールのダウンロード終了//
extern FFWE100_API	DWORD FFWE2Cmd_SetACTLED(DWORD dwActLedControl);				///< ACT LED制御//

// 非公開コマンド(特殊用途用)//
// 以下のコマンドは、デバッガでは使用しない。//
extern FFWE100_API	DWORD FFWE2Cmd_SetLID(DWORD dwOffsetAddr, DWORD dwLength, const BYTE* pbyData);	///< ライセンス情報の設定//
extern FFWE100_API	DWORD FFWE2Cmd_GetLID(DWORD dwOffsetAddr, DWORD dwLength, BYTE *const pbyData);	///< ライセンス情報の参照//


//----- デバッガ、NDEB用 -----//
#else
// システムコマンド//
// RevRxE2No170201-001 Append Line
__EXTERN__	DWORD(*FFWE2Cmd_EXTPWR2)(enum FFWENM_EXPWR2_MODE eExPwr2Mode, BYTE byExPwr2Val);	///< ユーザ基板への電源供給(リニア指定)を設定//

// 非公開コマンド(デバッグ用)//
// 以下のコマンドは、F/W、H/Wデバッグ用コマンドである。NDEBとの組み合わせでのみ使用し、デバッガでは使用しない。//
__EXTERN__	DWORD (*FFWE2Cmd_MONPCOM_OPEN)(DWORD dwTotalLength);					///< モニタプログラム(BFW) COMモジュールのダウンロード開始//
__EXTERN__	DWORD (*FFWE2Cmd_MONPCOM_SEND)(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW) COMモジュールの送信//
__EXTERN__	DWORD (*FFWE2Cmd_MONPCOM_CLOSE)(void);									///< モニタプログラム(BFW) COMモジュールのダウンロード終了//
__EXTERN__	DWORD (*FFWE2Cmd_SetACTLED)(DWORD dwActLedControl);						///< ACT LED制御//

// 非公開コマンド(特殊用途用)//
// 以下のコマンドは、デバッガでは使用しない。//
__EXTERN__	DWORD (*FFWE2Cmd_SetLID)(DWORD dwOffsetAddr, DWORD dwLength, const BYTE* pbyData);	///< ライセンス情報の設定//
__EXTERN__	DWORD (*FFWE2Cmd_GetLID)(DWORD dwOffsetAddr, DWORD dwLength, BYTE *const pbyData);	///< ライセンス情報の参照//

#endif

#ifdef	__cplusplus
	};
#endif

#endif	// __FFWE2_H__
