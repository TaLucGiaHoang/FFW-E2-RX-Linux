///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffw_sys.h
 * @brief システム関連コマンドのヘッダファイル
 * @author RSD Y.Minami, H.Hashiguchi, H.Akashi, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/02/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo121017-003	2012/10/30 明石
　VS2008 warning C4996対策
・RevNo120910-001	2012/11/05 明石
　USB VBUS電圧判定処理追加
・RevRxE2LNo141104-001 2014/11/12 上田
	E2 Lite対応
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
*/
#ifndef	__FFW_SYS_H__
#define	__FFW_SYS_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義
#define GAGE_COUNT_MAX	1000		// ゲージカウント値最大値
// RevNo121017-003	Append Line
#define	GAGE_COUNT_MSG_MAX	256		// 進捗ゲージ用メッセージ最大文字数
#define USBTS_SIZE_MIN	0x00200		// USB転送サイズ最小値
//#define USBTS_SIZE_MAX	0x10000		// USB転送サイズ最大値
#define USBTS_SIZE_MAX	0x2000		// USB転送サイズ最大値
#define MONP_LENGTH_MAX	0x00200000	// FFWCmd_MONP_SENDの送信データバイト数最大値
#define INIT_FFW_TMOUT	30			// FFW用受信タイムアウト初期値
#define INIT_BFW_TMOUT	5			// BFW用受信タイムアウト初期値

#define		CPU_W_SIZE		2048
// RevNo120910-001	Append Line
#define		USB_VBUS_THRESHHOLD		4.35f	// 浮動小数点数 4.35V
// RevRxE2No170201-001 Modify Line
#define		USB_VBUS_THRESHHOLD_4_00V	4.00f	// 浮動小数点数 4.00V(E2用)


// enum定義
enum FFWENM_HALT_ENABLE {	// HALT受付許可/禁止
	HALT_DIS=0,					// 禁止
	HALT_ENA					// 許可
};

typedef struct FFW_ERROR_DETAIL {					// GetLastError関数で返すエラー情報用構造体
	DWORD dwErrorCode;							// FFWエラーコード
	char  szErrorMessage[ERROR_MESSAGE_NUM_MAX][ERROR_MESSAGE_LENGTH_MAX];	// メッセージ[16][32]
} FFW_ERROR_DETAIL_INFO;

// グローバル関数の宣言

extern FFW_WINVER_DATA* GetWinVersion(void);					///< OSバージョン情報の参照
extern void SetGageCount(DWORD dwCount);						///< 進捗ゲージカウンタ値の設定
extern WORD GetFfwTimeOut(void);								///< FFW用受信タイムアウト値の参照
extern WORD GetBfwTimeOut(void);								///< BFW用タイムアウト値の参照	// RevRxE2LNo141104-001 Append Line
extern void GetMonpprogFilename(char* pFilename, DWORD dwBufSize);	///< 制御CPUフラッシュ書き換えプログラム(MONPPROG)のファイル名(フルパス)取得	// RevRxE2LNo141104-001 Append Line
extern FFWERR Load_wtr_e20(char* filename);						///< 制御CPUフラッシュ書き換えプログラム(MONPPROG)ファイルのダウンロード	// RevRxE2LNo141104-001 Append Line
extern void InitFfwIfData_Sys(void);							///< システムコマンド用FFW内部変数の初期化

extern void	(*CallbackFromFfw2Target)(FFWENM_CALLBACKFUNC_FWRITE_STATUS);

extern void SetMsgGageCount(char* pchMsgGageCount);				///< 進捗ゲージカウンタに表示するメッセージ文字列の設定

extern void InitErrorDetailInfo(void);
extern FFW_ERROR_DETAIL_INFO* GetErrorDetailInfo(void);
extern void SetErrorDetailInfo(FFW_ERROR_DETAIL_INFO* pErrDetailInfo, BYTE byMessageNum);

// V.1.02 No.14,15 USB高速化対応 Append Line
extern BOOL GetLevelEML(void);
extern void SetLevelEML(BOOL bLvEml);

extern void ChkE2IndividualCurrentData(void);
#endif	// __FFW_SYS_H__
