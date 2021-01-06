///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_srm.h
 * @brief 指定ルーチン実行関連コマンドのヘッダファイル
 * @author RSO H.Hashiguchi, Y.Miyake, SDS T.Iwata, Y.Kawakami, PA K.Tsumu
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-006	2012/11/21 三宅(2012/12/04 SDS 岩田 マージ)
　・以下の定義追加。
　　WORKPROGRAMSTARTADDR、SRM_WORK_PROGRAM_LENGTH、
　　WORKRAMREGADDR、WORKRAMSTOPFLAGADDR、
    SRM_START_FUNC_BIT、SRM_STOP_FUNC_BIT、
　　SRM_BREAK_FUNC_BIT、SRM_INTERRUPT_FUNC_BIT、
	SRM_WORKRAM_LENGTH。
  ・GetSrmWorkProgramCode()のExtern関数宣言追加。
  ・GetSrmData()のExtern関数宣言追加。
・RevRxNo130301-001	2013/04/16 植盛
	RX64M対応
・RevRxNo130730-004 2013/11/19 川上
	Start/Stop高速化対応
・RevRxE2No171004-001 2017/10/04 PA 紡車
	E2拡張機能対応
*/
#ifndef	__FFWMCU_SRM_H__
#define	__FFWMCU_SRM_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義
#define INIT_SRM_MODE			0				// 動作モード
#define	INIT_BRUN_FUNCADDR		0x000000		// ユーザプログラム実行開始前に実行する指定ルーチン先頭アドレス
#define	INIT_ABREAK_FUNCADDR	0x000000		// ユーザプログラムブレーク後に実行する指定ルーチン先頭アドレス
#define	INIT_BREAK_FUNCADDR		0x000000		// ユーザプログラムブレーク中に繰り返し実行する指定ルーチン先頭アドレス
#define	INIT_INT_IPL			0				// ユーザプログラムブレーク中の割り込み優先レベル
#define	INIT_WORKROM_ADDR		0x000000		// 指定ルーチン呼び出しのためのプログラムコードを配置する領域
#define	INIT_WORKRAM_ADDR		0x000000		// 指定ルーチン実行で使用するスタック領域の開始アドレス

// RevRxNo120910-006 Append Start
#define	WORKPROGRAMSTARTADDR		0x00000084	// 指定ルーチン用ワーク・プログラム領域の相対先頭アドレス
#define	SRM_WORK_PROGRAM_LENGTH		0x000001AC	// 指定ルーチン用ワーク・プログラム領域のバイト数
#define	WORKRAMREGADDR				0x00000004	// 指定ルーチン用ワークRAMでのレジスタを格納する相対先頭アドレス
#define	WORKRAMSTOPFLAGADDR			0x0000022C	// 指定ルーチン用ワークRAMでのStopルーチン完了フラグの相対先頭アドレス
#define	SRM_START_FUNC_BIT			0x00000001	// "ユーザプログラム実行開始前に指定ルーチンを1回実行"許可ビット
#define	SRM_STOP_FUNC_BIT			0x00000002	// "ユーザプログラムブレーク後に指定ルーチンを1回実行"許可ビット
#define	SRM_BREAK_FUNC_BIT			0x00000004	// "ブレーク中に指定ルーチンを繰り返し実行"許可ビット
#define	SRM_INTERRUPT_FUNC_BIT		0x00000008	// "ブレーク中にユーザ割り込みルーチン実行を受け付ける"許可ビット
// RevRxNo130730-004 Append Start
// 指定ルーチン実行用ワークプログラム(s_bySrmWorkProgram[])の要素番号定義
#define SRM_START_PROG2_NUM		0x000000A4		// STARTルーチン用プログラム(2)の先頭要素番号
#define SRM_STOP_PROG2_NUM		0x00000172		// STOPルーチン用プログラム(2)の先頭要素番号
// RevRxNo130730-004 Append End

// 指定ルーチン実行用ワークプログラム(s_bySrmWorkProgram[])のサイズ定義
#define	SRM_WORKRAM_LENGTH			0x00000230	// 指定ルーチン用ワークRAMのサイズ(230h)
// RevRxNo120910-006 Append End
// RevRxNo130730-004 Append Start
#define SRM_COMMON_PROG1_LENGTH		0x0000006E	// Startルーチンプログラム(1)～FINTV復帰のサイズ
#define	SRM_COMMON_PROG2_LENGTH		0x00000094	// Startルーチンプログラム(2)～FINTV退避のサイズ
#define	SRM_COMMON_PROG3_LENGTH		0x0000003A	// Stopルーチンプログラム(2)のサイズ
#define SRM_RET_FPU_LENGTH			0x00000006	// FPU復帰プログラムサイズ
#define SRM_RET_CPEN_LENGTH			0x00000006	// CPEN復帰プログラムサイズ
#define SRM_RET_ACC_LENGTH			0x0000000C	// ACC復帰プログラムサイズ
#define SRM_SAVE_FPU_LENGTH			0x00000006	// FPU退避プログラムサイズ
#define SRM_SAVE_CPEN_LENGTH		0x00000006	// CPEN退避プログラムサイズ
#define SRM_SAVE_ACC_LENGTH			0x0000000E	// ACC退避プログラムサイズ

// RXV2アーキテクチャ用プログラム(s_bySrmWorkProgramRxv2[])のサイズ定義
#define SRM_WORK_PROGRAM_RXV2		0x00000054	// RXV2アーキテクチャ用ワークプログラム領域のバイト数
#define SRM_RET_ACC_RXV2_LENGTH		0x00000024	// RXV2アーキテクチャ用ACC0,1復帰プログラムサイズ
#define SRM_RET_EXTB_RXV2_LENGTH	0x00000006	// RXV2アーキテクチャ用EXTB復帰プログラムサイズ
#define SRM_SAVE_ACC_RXV2_LENGTH	0x00000024	// RXV2アーキテクチャ用ACC0,1退避プログラムサイズ
#define SRM_SAVE_EXTB_RXV2_LENGTH	0x00000006	// RXV2アーキテクチャ用EXTB退避プログラムサイズ
// RevRxNo130730-004 Append End

// RevRxNo130730-004 Delete Start
// RevRxNo130730-004 Delete End

// RevRxE2No171004-001 Append Start
// 通過ポイント用モニタプログラム関連定義
#define	SRM_MON_POINT_FUNC_BIT		0x00000010	// "通過ポイント用モニタプログラム実行"許可ビット
#define	MON_POINT_PROGRAM_LENGTH	0x000000A0	// 通過ポイント用モニタプログラムコード領域サイズ
#define	MON_PROGR_START_ADDR		0x00000060	// 通過ポイント用モニタプログラム領域の相対先頭アドレス
#define	MON_POINT_WORKRAM_LENGTH	0x00000060	// 通過ポイントモニタプログラム用ワーク領域サイズ
#define MON_PROGRAM_STOP_ADDR		0x00000055	// 強制ブレーク判断フラグ相対アドレス
#define COMP_FLG_ADDR_SET			0x18		// ワークRAM先頭+55h番地(強制ブレーク判断フラグ格納アドレス)設定位置1
#define WORKRAM_TOP_ADDR_SET		0x31		// ワークRAM先頭番地設定位置
// RevRxE2No171004-001 Append Endt


// グローバル関数の宣言
extern void InitFfwIfMcuData_Srm(void);	///< 指定ルーチン実行関連コマンド用FFW内部変数の初期化
// RevRxNo120910-006 Append Start
extern BYTE* GetSrmWorkProgramCode(const FFW_SRM_DATA* pSrMode);	///< 指定ルーチン用ワーク・プログラムコードの配列のポインタ取得
extern FFW_SRM_DATA* GetSrmData(void);	///< 指定ルーチン実行情報格納構造体変数のポインタ取得
// RevRxNo120910-006 Append End

// RevRxE2No171004-001 Append Line
extern BYTE* GetMonitorPointProgramCode(const FFW_SRM_DATA* pSrMode);	///< 通過ポイント用モニタプログラムコードの配列のポインタ取得

#endif	// __FFWMCU_SRM_H__
