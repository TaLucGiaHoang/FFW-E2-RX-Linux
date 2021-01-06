///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_extram.h
 * @brief ターゲットMCU 外部RAM制御関数のヘッダファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/27
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/11/27 上田
	E2 Lite対応
*/
#ifndef	__MCU_EXTRAM_H__
#define	__MCU_EXTRAM_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義
// RevRxE2LNo141104-001 Append Start
// 外部RAMベリファイエラーの詳細情報
#define EXTRAMVRFY_ERRD_NUM			4	// ベリファイエラーの詳細情報格納数
#define EXTRAMVRFY_ERRD_ACC_NO		0	// アクセスサイズ格納番号
#define EXTRAMVRFY_ERRD_ADDR_NO		1	// アドレス格納番号
#define EXTRAMVRFY_ERRD_WDATA_NO	2	// ライトデータ格納番号
#define EXTRAMVRFY_ERRD_RDATA_NO	3	// リードデータ格納番号
// RevRxE2LNo141104-001 Append End


// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned Start
// 外部RAMダウンロード用データ格納構造体
typedef	struct extram_dwnp_data_tag{
	struct	extram_dwnp_data_tag	*next;			// 次の構造体へのポインタ
	enum	FFWENM_VERIFY_SET		eVerify;		// ベリファイチェック指定
	MADDR	madrStartAddr;							// 開始アドレス
	enum	FFWENM_MACCESS_SIZE		eAccessSize;	// アクセスサイズ
	DWORD	dwCount;								// ライト回数
	BYTE	byEndian;								// エンディアン
	BYTE	*pbyBuff;								// DWNPデータ格納バッファ(ポインタだけ定義)
} EXTRAM_DWNP_DATA;
extern EXTRAM_DWNP_DATA	*ExtramDwnpTop;
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned End


// グローバル関数の宣言

extern FFWERR GetMcuExtSdCsEndian(WORD* pwEndianState);	//SDRAMエンディアン取得

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned Start
extern void DeleteExtramCacheMem(void);	// 外部RAM用キャッシュ領域解放
extern FFWERR ExtWriteRamData(void);	// 外部RAMダウンロード処理
extern DWORD GetExtramDwnpNum(void);	// 外部RAMダウンロード用構造体数取得
extern void SetExtramDwnpNum(DWORD);	// 外部RAMダウンロード用構造体数設定
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned End
extern FFW_ERROR_DETAIL_INFO* GetExtRamVerifyErrorDetailInfo(void);	///< 外部RAMベリファイエラーの詳細情報取得	// RevRxE2LNo141104-001 Append Line
extern void InitMcuData_Extram(void);						///< ターゲットMCU 外部RAM制御関数用変数初期化


#endif	// __MCU_EXTRAM_H__
