////////////////////////////////////////////////////////////////////////////////
/**
 * @file fpga_tra.h
 * @brief トレースFPGAのレジスタヘッダファイル
 * @author RSO H.Hashiguchi, Y.Miyake, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/04/10
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130308-001	2013/03/19 三宅
　カバレッジ開発対応
・RevRxNo130408-001	2014/04/10 上田
	外部トレースデータ8ビット幅対応
*/
#ifndef	__FPGA_TRA_H__
#define	__FPGA_TRA_H__


#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義
// トレース設定関連レジスタ//
#define REG_RM_TRCTL0				0x00400100		// トレースFPGAのトレース制御０レジスタ//
#define REG_RM_TRCTL1				0x00400102		// トレースFPGAのトレース制御１レジスタ//
	// RevRxNo130408-001 Append Start
	#define REG_RM_TRCTL1_TRDW			0x0007		// トレースデータバス幅ビット//
		#define REG_RM_TRCTL1_TRDW_4_DATA	0x0002		// トレースデータバス幅 4ビット設定値//
		#define REG_RM_TRCTL1_TRDW_8_DATA	0x0003		// トレースデータバス幅 8ビット設定値//
	#define REG_RM_TRCTL1_TRSW			0x0010		// TRSYNC信号幅ビット//
		#define REG_RM_TRCTL1_TRSW_1_DATA	0x0000		// TRSYNC信号幅 1ビット設定値//
		#define REG_RM_TRCTL1_TRSW_2_DATA	0x0010		// TRSYNC信号幅 2ビット設定値//
	// RevRxNo130408-001 Append End

#define REG_RM_TMBCTL				0x00400108		// トレースメモリバッファ制御//
#define REG_RM_IN_DYL				0x0040011A		// トレースFPGA Delay制御//
#define REG_RM_TMFILD				0x0040011C		// トレースメモリフィルデータ(検査用)//
#define REG_RM_TMSIZE				0x0040011E		// トレースメモリ制御//	// RevRxNo130408-001 Modify Line
#define REG_RM_TC_CTL				0x00400120		// トレースコマンド取得(上位側コマンド数、コマンドクリア)//
#define REG_RM_TC_QL				0x00400122		// トレースコマンド取得(下位側コマンド数)//
#define REG_RM_TRCLKM				0x00400124		// トレースクロックモニタ機能(クロック計測結果上位側)//
#define REG_RM_TRCLKVL				0x00400126		// トレースクロックモニタ機能(クロック計測結果下位側)//
// RevRxNo130308-001 Append Start
#define REG_RM_RSTVH				0x00400128		// リセット先頭アドレス上位側レジスタ//
#define REG_RM_RSTVL				0x0040012A		// リセット先頭アドレス上位側レジスタ//
// RevRxNo130308-001 Append End

// トレースデータ取得関連レジスタ//
#define REG_RD_TMWBA				0x0040010A		// トレースFPGAのトレースメモリ書込み用バッファアドレスレジスタ//
	#define REG_RD_TMWBA_TMWBS		0x0200
	#define REG_RD_TMWBA_TMWBA		0x01FF
#define REG_RD_TMWA					0x0040010C		// トレースFPGAのトレースメモリ書込み用アドレスレジスタ//
#define REG_RD_TMRA					0x0040010E		// トレースFPGAのトレースメモリ読込み用アドレスレジスタ//
#define REG_RD_TMBUFF0				0x00401000		// トレースFPGAのトレースバッファ０//
#define REG_RD_TMBUFF1				0x00401800		// トレースFPGAのトレースバッファ１//


#endif	// __FPGA_TRA_H__
