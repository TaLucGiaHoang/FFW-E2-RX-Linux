///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_inram.h
 * @brief MCU 内部RAMダウンロード制御関数のヘッダファイル
 * @author RSD S.Ueda
 * @author Copyright (C) 2014 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/27
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/27 上田
	新規作成
*/
#ifndef	__MCU_INRAM_H__
#define	__MCU_INRAM_H__

#include "ffw_typedef.h"
#include "mcudef.h"
#include "ffw_sys.h"


// define定義
// MCU内蔵RAMベリファイエラーの詳細情報
#define INRAMVRFY_ERRD_NUM		4	// ベリファイエラーの詳細情報格納数
#define INRAMVRFY_ERRD_ACC_NO	0	// アクセスサイズ格納番号
#define INRAMVRFY_ERRD_ADDR_NO	1	// アドレス格納番号
#define INRAMVRFY_ERRD_WDATA_NO	2	// ライトデータ格納番号
#define INRAMVRFY_ERRD_RDATA_NO	3	// リードデータ格納番号


// 構造体定義
// 内部RAMダウンロード情報構造体
typedef	struct inram_dwnp_info_tag{
	struct	inram_dwnp_info_tag*	pNext;			///< 次の構造体へのポインタ
	enum	FFWENM_VERIFY_SET		eVerify;		///< ベリファイチェック指定
	MADDR	dwStartAddr;							///< 開始アドレス
	enum	FFWENM_MACCESS_SIZE		eAccessSize;	///< アクセスサイズ
	DWORD	dwCount;								///< ライト回数
	BYTE	byEndian;								///< エンディアン
} INRAM_DWNP_INFO;

// 内部RAMダウンロードデータ格納バッファ管理構造体
typedef struct{
	DWORD	dwAreaNum;								///< 内部RAMダウンロードデータ格納バッファ領域数
	MADDR	dwRamStartAddr[MCU_AREANUM_MAX_RX];		///< 内部RAM領域開始アドレス
	MADDR	dwRamEndAddr[MCU_AREANUM_MAX_RX];		///< 内部RAM領域終了アドレス
	BYTE*	pbyBuff[MCU_AREANUM_MAX_RX];			///< 内部RAMダウンロードデータ格納バッファポインタ
} INRAM_DATA_BUFF;


// グローバル関数の宣言
extern void SetInRamDwnpInfoPtr(enum FFWENM_VERIFY_SET eVerify, MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, 
								DWORD dwAccessCount, BYTE byEndian);	///< 内部RAMダウンロード情報格納
extern void DeleteInRamCacheMem(void);				///< 内部RAMダウンロードデータ格納バッファ解放
extern BOOL GetInRamDwnpCacheFlg(void);				///< 内部RAMダウンロードデータ格納バッファ有無フラグの参照
extern BYTE* GetInRamBuffPtr(MADDR dwRamAddr);		///< 内部RAMダウンロードデータバッファのポインタ取得
extern FFWERR WriteInRamData(void);					///< 内部RAMダウンロード処理
extern FFW_ERROR_DETAIL_INFO* GetInRamVerifyErrorDetailInfo(void);	///< MCU内蔵RAMベリファイエラーの詳細情報取得
extern void InitMcuData_Inram(void);				///< ターゲットMCU 内部RAMダウンロード制御関数用変数初期化

#endif	// __MCU_INRAM_H__
