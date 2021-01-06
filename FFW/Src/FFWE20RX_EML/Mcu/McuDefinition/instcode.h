///////////////////////////////////////////////////////////////////////////////
/**
 * @file instcode.h
 * @brief RX対応 命令コード解析処理のヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi
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
#ifndef	__INSTCODE_H__
#define	__INSTCODE_H__

#include "ffw_typedef.h"
#include "mcudef.h"

/////////// define定義 ///////////
//コマンド種別定数
#define	TYPE_BIT_BRANCH_NOCOND		0x00000001	//(1)無条件分岐:BRA,JMP
#define	TYPE_BIT_BRANCH_COND		0x00000002	//(2)条件分岐:BCnd
#define	TYPE_BIT_BRANCH_SUB1		0x00000004	//(3)サブルーチン分岐１:BSR,JSR
#define	TYPE_BIT_BRANCH_SUB2		0x00000008	//(4)サブルーチン分岐２:RTS,RTSD
#define	TYPE_BIT_BRANCH_BRK			0x00000010	//(5)割り込み/例外:DBT,INT,BRK,RTE,RTFI
#define	TYPE_BIT_reserved1			0x000000e0	//padding
#define	TYPE_BIT_REGISTER_UPDATE	0x00000100	//レジスタ更新命令
#define	TYPE_BIT_MEMORY_UPDATE		0x00000200	//メモリ更新命令
#define	TYPE_BIT_reserved2			0x00000c00	//padding
#define	TYPE_BIT_SPECIAL_PSW_DIRECT	0x00001000	//(1)PSWを直接更新する命令:CLRPSW,SETPSW
#define	TYPE_BIT_SPECIAL_WAIT		0x00002000	//(2)WAIT命令:WAIT
#define	TYPE_BIT_reserved3			0x00004000	//padding
#define	TYPE_BIT_MISC				0x00008000	//ニーモイック一覧の最後5行分の命令（不明だけどとりあえず）
#define	TYPE_BIT_reserved4			0xffff0000	//padding

#define TYPE_IS_BRANCH	(TYPE_BIT_BRANCH_NOCOND | TYPE_BIT_BRANCH_COND | TYPE_BIT_BRANCH_SUB1 | TYPE_BIT_BRANCH_SUB2 | TYPE_BIT_BRANCH_BRK)
#define TYPE_IS_SPECIAL	(TYPE_BIT_SPECIAL_PSW_DIRECT | TYPE_BIT_SPECIAL_WAIT)



/////////// 構造体定義 ///////////
//コマンド種別ビットフィールド
typedef struct {
	//size 4byte(32bit);
	//byte no 1
	/*** 1.分岐命令 */
	unsigned BRANCH_NOCOND 			:	1;	//(1)無条件分岐:BRA,JMP
	unsigned BRANCH_COND			:	1;	//(2)条件分岐:BCnd
	unsigned BRANCH_SUB1			:	1;	//(3)サブルーチン分岐１:BSR,JSR
	unsigned BRANCH_SUB2			:	1;	//(4)サブルーチン分岐２:RTS,RTSD
	unsigned BRANCH_BRK 			:	1;	//(5)割り込み/例外:DBT,INT,BRK,RTE,RTFI
	unsigned reserved1				:	3;	//padding
	
	//byte no 2
	/*** 2.レジスタ更新命令 */
	unsigned REGISTER_UPDATE		:	1;	//レジスタ更新命令
	/*** 3.メモリ更新命令 */
	unsigned MEMORY_UPDATE			:	1;	//メモリ更新命令
	unsigned reserved2				:	2;	//padding
	/*** 4.特殊命令 */
	unsigned SPECIAL_PSW_DIRECT 	:	1;	//(1)PSWを直接更新する命令:CLRPSW,SETPSW
	unsigned SPECIAL_WAIT			:	1;	//(2)WAIT命令:WAIT
	unsigned reserved3				:	1;	//padding
	/*** 5.その他命令 */
	unsigned MISC					:	1;	//ニーモイック一覧の最後5行分の命令（不明だけどとりあえず）
	
	//byte no 3,4
	unsigned reserved4				:	16; //padding
} FFWRX_INST_CLASS_BIT;

//コマンド種別
typedef union {
	DWORD					data;
	FFWRX_INST_CLASS_BIT	bit;
} FFWRX_INST_CLASS;

//命令解析マスタテーブル
typedef struct {
	DWORD				cmd_bitptn;			//命令ビットパターン
	DWORD				cmd_bitptn_mask;	//命令ビットパターンマスク
	DWORD				cmd_length; 		//命令バイト長
	FFWRX_INST_CLASS	cmd_class;			//命令種別
	DWORD				branch_mask; 		//分岐先アドレスを特定するためのマスク
	DWORD				branch_mask_ls;		//分岐先アドレスを計算するための左シフト数
	DWORD				branch_mask_rs;		//分岐先アドレスを計算するための右シフト数
	DWORD				rmask;		   		//更新先レジスタを特定する為のマスク（予約）
} FFWRX_INST_MST;


/////////// 構造体型のtypedef ///////////

/////////// グローバル関数の宣言 ///////////
extern FFWRX_INST_MST* GetINSTCMDPtr(void);		///< 命令解析テーブルのポインタ取得


#endif	// __INSTCODE_H__
