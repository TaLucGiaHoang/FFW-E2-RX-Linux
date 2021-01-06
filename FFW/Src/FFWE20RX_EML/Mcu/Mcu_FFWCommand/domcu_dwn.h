///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_dwn.h
 * @brief ダウンロード関連コマンドのヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/12/21
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121221-001	2012/12/21 上田
　　WTRダウンロード処理改善
*/
#ifndef	__DOMCU_DWN_H__
#define	__DOMCU_DWN_H__

#include "ffw_typedef.h"
#include "mcudef.h"


#define WRT_VER_MEM_LENGTH			0x10		// WTRプログラムのVerを格納するメモリバイトサイズ
#define WRT_NOT_LOAD_STRING_SIZE	0x11		// WTRプログラム未ロード時の文字サイズ

// グローバル関数の宣言
extern FFWERR DO_DWNP_OPEN(void);	///< ユーザプログラムのダウンロード開始
extern FFWERR DO_DWNP_CLOSE(FFW_DWNP_CHKSUM_DATA* chksum);	///< ユーザプログラムのダウンロード終了
extern FFWERR DO_WTRLOAD(enum FFWRXENM_WTRTYPE eWtrType, DWORD dwDataSize, const BYTE* pbyDataBuff);	///< Flash書き換みプログラムのロード
extern FFWERR DO_WTRVER(enum FFWRXENM_WTRTYPE eWtrType, char *const psWtrVer);	///< Flash書き換みプログラムバージョン参照
extern void DO_SetFCLR(void);


extern BOOL GetFlashDataClearFlg(void);		///< フラッシュROMブロックデータ初期化情報を取得する
extern void SetFlashDataClearFlg(void);		///< フラッシュROMブロックデータ初期化フラグを初期化済みに設定する
extern void ClrFlashDataClearFlg(void);		///< フラッシュROMブロックデータ初期化フラグを未初期化に設定する

extern void InitFfwCmdMcuData_Dwn(void);	///< プログラムダウンロードコマンド用変数初期化

extern void SetDownloadWtrLoad(BOOL bDownloadWtrLoad);	///< 	ダウンロード用WTRプログラム格納済みフラグを設定する
extern BOOL GetDownloadWtrLoad(void);						///< 	ダウンロード用WTRプログラム格納済みフラグを参照する
extern DWORD GetDownloadWtrDataSize(void);	///< ダウンロード用WTRプログラム格納メモリのサイズ取得	// RevRxNo121221-001 Append Line
extern BYTE* GetDownloadWtrDataAddr();		///<	ダウンロード用WTRプログラム格納メモリのポインタ取得
extern BYTE* GetDownloadWtrVerAddr();		///<	ダウンロード用WTRプログラム格納メモリのポインタ取得
extern void DeleteWtrLoadMem(enum FFWRXENM_WTRTYPE eWtrType);			///<	WTRプログラムロード用メモリの開放処理

#endif	// __DOMCU_DWN_H__
