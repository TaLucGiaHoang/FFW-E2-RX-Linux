////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_brk.h
 * @brief ブレーク関連コマンドのヘッダファイル
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2016/07/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRx6z4FpgaNo150525-001 2016/07/05 PA紡車
	RX6Z4マルチコア対応
*/
#ifndef	__FFWMCU_BRK_H__
#define	__FFWMCU_BRK_H__


#include "ffw_typedef.h"
#include "mcudef.h"


// 定数定義
#define BMODE_SWB_BIT		0x00000001		// ブレークモードのS/W ブレーク対象ビット(b0)
#define BMODE_EB_BIT		0x00000002		// ブレークモードのイベントブレーク対象ビット(b1)
#define BMODE_OCB_BIT		0x00000004		// ブレークモードのオンチップブレーク対象ビット(b2)
#define BMODE_PRE_BIT		0x00000004		// ブレークモードの実行前ブレーク対象ビット(b2)
#define	BFACT_OR_BIT		0x00000001		// 論理組み合わせOR 選択ビット(b0)
#define	BFACT_AND_BIT		0x00000002		// 論理組み合わせ累積AND 選択ビット(b1)
#define	BFACT_SAND_BIT		0x00000004		// 論理組み合わせ同時AND 選択ビット(b2)
#define	BFACT_SEQ_BIT		0x00000008		// シーケンシャル組み合わせ 選択ビット(b3)
#define	BFACT_SAND_SUB_BIT	0x00000010		// 論理組み合わせ同時ANDサブルーチン 選択ビット(b4)

// 初期値用定義
#define INIT_PB_NUM			0				// S/W ブレークポイントの数
#define INIT_BM_MODE		0x00000000		// ブレークモードの設定
#define INIT_BM_FACTOR		0x00000000		// ブレーク要因の設定

// ソフトウェアブレークポイント埋め戻し用構造体//  <- FFWE20.Hから移動
typedef struct {
	DWORD	dwSetNum;										// 埋め戻し対象点数//
	DWORD	dwmadrAddr[PB_SETNUM_MAX];						// 埋め戻し対象アドレス//
	BYTE	byCmdData[PB_SETNUM_MAX];						// 埋め戻す命令コード//
} FFW_PBCLR_DATA;
// RevRx6z4FpgaNo150525-001 Append End

// プロトタイプ宣言（構造体変数定義、アクセス関数）
extern void UpdatePbData(MADDR madrStartAddr, BYTE bySet);
extern FFW_PB_DATA* GetPbData(void);
extern FFW_PBCLR_DATA* GetPbClrData(void);
extern BYTE* GetUsrCodeData(void);
extern void GetBmMode(DWORD* pdwBMode, DWORD* pdwBFactor);
extern void SetBmData(void);

// プロトタイプ宣言（プログラム実行中の連続コマンド対応における一時領域へのアクセス関数）

// プロトタイプ宣言（その他の関数）
extern void InitFfwIfMcuData_Brk(void);	///< ブレーク関連コマンド用FFW内部変数の初期化

extern void SetPbClrTbl(DWORD dwmadrAddr, BYTE byCmdData);	// 埋め戻し対象のS/Wブレークポイントをテーブルに登録
extern void ClrPbClrTbl(void);								// 埋め戻し対象のS/Wブレークポイントをテーブルからクリア

#endif	// __FFWMCU_BRK_H__
