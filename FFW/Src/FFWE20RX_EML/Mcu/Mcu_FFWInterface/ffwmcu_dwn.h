///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_dwn.h
 * @brief ダウンロード関連コマンドのヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/
#ifndef	__FFWMCU_DWN_H__
#define	__FFWMCU_DWN_H__

#include "ffw_typedef.h"
#include "mcudef.h"

#define INIT_FCLR_RX_NUM	0	// フラッシュ初期化ブロック数初期値

#define FCLR_UM_BLKNUM_MAX	1024	// ユーザーマット初期化ブロック数最大値
#define FCLR_DM_BLKNUM_MAX	2048	// データマット初期化ブロック数最大値

#define WTR_HEADER_SIZE		0x10	//WTRのヘッダ部のバイト数

#include "FFWE20RX600.h"
#include "domcu_mcu.h"
// グローバル関数の宣言
extern BOOL GetDwnpOpenData(void);		///< DWNPコマンド実行中フラグの参照
extern void SetDwnpOpenData(BOOL);		///< DWNPコマンド実行中フラグの設定
extern void InitFfwIfMcuData_Dwn(void);	///< プログラムダウンロードコマンド用変数初期化

// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Start
extern FFWRX_FOVR_DATA* GetRxFovrData(void);	///< ダウンロード時のフラッシュ上書きブロック情報の参照
// V.1.02 No.8 フラッシュROMブロック細分化対応 Append End


extern void SetAllBlockFclrData(const FFWRX_FOVR_DATA* pFovr,enum FFWRXENM_MAREA_TYPE);	///< 初期化ブロック情報の設定

extern BOOL GetUserBootMatFclr(void);						///< ユーザーブート領域初期化フラグの参照
extern void SetUserBootMatFclr(BOOL bUserBootFclrFlg);		///< ユーザーブート領域初期化フラグの設定



// ユーザーマット初期化ブロック格納構造体
typedef struct{
	DWORD	dwNum;									// ユーザーマットブロック数
	DWORD	dwmadrBlkStart[FCLR_UM_BLKNUM_MAX];		// ユーザーマットブロック先頭アドレス
	DWORD	dwmadrBlkEnd[FCLR_UM_BLKNUM_MAX];		// ユーザーマットブロック終了アドレス
} FFW_UM_FCLR_DATA;

// データマット初期化ブロック格納構造体
typedef struct{
	DWORD	dwNum;									// データマットブロック数
	DWORD	dwmadrBlkStart[FCLR_DM_BLKNUM_MAX];		// データマットブロック先頭アドレス
	DWORD	dwmadrBlkEnd[FCLR_DM_BLKNUM_MAX];		// データマットブロック終了アドレス
} FFW_DM_FCLR_DATA;

extern FFW_UM_FCLR_DATA* GetUserFclrData(void);	///< ダウンロード時のフラッシュ上書きブロック情報の参照
extern FFW_DM_FCLR_DATA* GetDataFclrData(void);	///< ダウンロード時のフラッシュ上書きブロック情報の参照

#endif	// __FFWMCU_DWN_H__
