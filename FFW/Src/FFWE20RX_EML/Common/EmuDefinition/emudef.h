///////////////////////////////////////////////////////////////////////////////
/**
 * @file emudef.h
 * @brief エミュレータ依存情報の定義ファイル
 * @author RSD S.Ueda, PA K.Tsumu
 * @author Copyright (C) 201(2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/05/15
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/10 上田
	新規作成
・RevRxNo170515-001 2017/05/15 PA 紡車
　　E2 OB-mini対応
*/
#ifndef	__EMUDEF_H__
#define	__EMUDEF_H__

#include "ffw_typedef.h"

/////////// define定義 ///////////
#define MONPPROG_FILENAME_NUM_MAX	32	// BFW書き換え制御プログラムファイル名の最大文字数
// RevRxNo170515-001 Append Start
#define E2_OB_MINI_MAGIC_ADDR		0xFFFE8006		// E2 OB-miniマジックコード格納アドレス
#define E2_OB_MINI_MAGIC_CODE		0x32			// E2 OB-miniマジックコード
// RevRxNo170515-001 Append End

/////////// enum定義 ///////////


/////////// 構造体定義 ///////////
// FFW_EMUDEF_DATA
	// エミュレータ固有情報管理構造体定義
	// ソース中で固定データとして使用する(コマンドのパラメータとして受信しない)
	// 制御CPUアドレス、データ情報を定義する。
typedef	struct {
	DWORD dwBfwAreaStartAdr;		// BFWコード領域開始アドレス
	DWORD dwBfwAreaSize;			// BFWコード領域サイズ
	DWORD dwBfwLv0StartAdr;			// BFWレベル0領域開始アドレス
	DWORD dwBfwLv0Size;				// BFWレベル0領域サイズ
	DWORD dwBfwEmlStartAdr;			 // BFW EML領域開始アドレス
	DWORD dwBfwEmlSize; 			// BFW EML領域サイズ
	DWORD dwBfwPrgStartAdr;			// BFW FDT/PRG領域開始アドレス
	DWORD dwBfwPrgSize;				// BFW FDT/PRG領域サイズ
	DWORD dwBfwComStartAdr;			// BFW COM領域開始アドレス
	DWORD dwBfwComSize;				// BFW COM領域サイズ
	DWORD dwSelIdStartAdr;			// 製品シリアル番号設定領域開始アドレス
	DWORD dwSelIdSize;				// 製品シリアル番号設定領域サイズ
	DWORD dwBfwRewriteEmuCode;		// BFW書き換え時のエミュレータ識別コード
	char szMonpprogFileName[MAX_PATH];	// BFW書き換え制御プログラムのファイル名	// RevRxNo170517-001 Modify Line
} FFW_EMUDEF_DATA;


/////////// グローバル関数の宣言 ///////////
// RevRxNo170515-001 Modify Line
extern FFWERR SetEmuDefData(void);				///< エミュレータ固有情報管理構造体の設定
extern FFW_EMUDEF_DATA* GetEmuDefData(void);	///< エミュレータ固有情報管理構造体のポインタ取得

#endif	// __EMUDEF_H__
