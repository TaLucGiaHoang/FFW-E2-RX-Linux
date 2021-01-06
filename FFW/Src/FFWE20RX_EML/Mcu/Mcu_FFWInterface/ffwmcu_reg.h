////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_reg.h
 * @brief レジスタ操作コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, Y.Miyake, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/09/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/09/14 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・REG_NUM_RX_ACC0G, REG_NUM_RX_ACC0H, REG_NUM_RX_ACC0L, 
    REG_NUM_RX_ACC1G, REG_NUM_RX_ACC1H, REG_NUM_RX_ACC1L, 
    REG_NUM_RX_EXTB の定義追加。
    REG_NO_USE_VALUE の定義追加。
・RevRxNo130301-001	2013/05/16 植盛、上田
  RX64M対応
・RevRxNo180625-001,PB18047-CD01-001 2018/06/25 PA 辻
	RX66T-H/RX72T対応
*/
#ifndef	__FFWMCU_REG_H__
#define	__FFWMCU_REG_H__


#include "ffw_typedef.h"
#include "mcudef.h"


// 定数定義

// プロトタイプ宣言（構造体変数定義、アクセス関数）

// プロトタイプ宣言（コマンド処理部）



// 2008.9.9 INSERT_BEGIN_E20RX600(+NN) {
// 最大値
#define REG_MAX_GENERAL			0xFFFFFFFF		// 汎用レジスタの最大サイズ
#define REG_MAX_CONTROL			0xFFFFFFFF		// 制御レジスタの最大サイズ

//レジスタ番号
#define REG_NUM_RX_R0		0x00			// R0レジスタ番号
#define REG_NUM_RX_R1		0x01			// R1レジスタ番号
#define REG_NUM_RX_R2		0x02			// R2レジスタ番号
#define REG_NUM_RX_R3		0x03			// R3レジスタ番号
#define REG_NUM_RX_R4		0x04			// R4レジスタ番号
#define REG_NUM_RX_R5		0x05			// R5レジスタ番号
#define REG_NUM_RX_R6		0x06			// R6レジスタ番号
#define REG_NUM_RX_R7		0x07			// R7レジスタ番号
#define REG_NUM_RX_R8		0x08			// R8レジスタ番号
#define REG_NUM_RX_R9		0x09			// R9レジスタ番号
#define REG_NUM_RX_R10		0x0A			// R10レジスタ番号
#define REG_NUM_RX_R11		0x0B			// R11レジスタ番号
#define REG_NUM_RX_R12		0x0C			// R12レジスタ番号
#define REG_NUM_RX_R13		0x0D			// R13レジスタ番号
#define REG_NUM_RX_R14		0x0E			// R14レジスタ番号
#define REG_NUM_RX_R15		0x0F			// R15レジスタ番号
#define REG_NUM_RX_ISP		0x10			// ISPレジスタ番号
#define REG_NUM_RX_USP		0x11			// USPレジスタ番号
#define REG_NUM_RX_INTB		0x12			// INTBレジスタ番号
#define REG_NUM_RX_PC		0x13			// PCレジスタ番号
#define REG_NUM_RX_PSW		0x14			// PSWレジスタ番号
#define REG_NUM_RX_BPC		0x15			// BPCレジスタ番号
#define REG_NUM_RX_BPSW		0x16			// BPSWレジスタ番号
#define REG_NUM_RX_FINTV	0x17			// FINTVレジスタ番号
#define REG_NUM_RX_FPSW		0x18			// FPSWレジスタ番号
#define REG_RX_FPSW_INIT_VAL 0x00000100		// FPSWリセット後の初期値
#define REG_NUM_RX_CPEN		0x19			// CPENレジスタ番号
#define REG_NUM_RX_ACCH		0x1A			// ACCHレジスタ番号
#define REG_NUM_RX_ACCL		0x1B			// ACCLレジスタ番号
// RevRxNo120910-001 Modify Start
#define REG_NUM_RX_ACC0G	0x1C			// ACC0Gレジスタ番号
#define REG_NUM_RX_ACC0H	0x1D			// ACC0Hレジスタ番号
#define REG_NUM_RX_ACC0L	0x1E			// ACC0Lレジスタ番号
#define REG_NUM_RX_ACC1G	0x1F			// ACC1Gレジスタ番号
#define REG_NUM_RX_ACC1H	0x20			// ACC1Hレジスタ番号
#define REG_NUM_RX_ACC1L	0x21			// ACC1Lレジスタ番号
#define REG_NUM_RX_EXTB		0x22			// EXTBレジスタ番号
// RevRxNo120910-001 Modify End
#define REG_NUM_RX_ALL		0xFF			// 全MCUレジスタ指定

// RevRxNo180625-001 Append Start
// レジスタ退避バンク用レジスタ番号
#define REGBANK_REG_NUM_R1		0x00		// R1レジスタ番号
#define REGBANK_REG_NUM_R2		0x01		// R2レジスタ番号
#define REGBANK_REG_NUM_R3		0x02		// R3レジスタ番号
#define REGBANK_REG_NUM_R4		0x03		// R4レジスタ番号
#define REGBANK_REG_NUM_R5		0x04		// R5レジスタ番号
#define REGBANK_REG_NUM_R6		0x05		// R6レジスタ番号
#define REGBANK_REG_NUM_R7		0x06		// R7レジスタ番号
#define REGBANK_REG_NUM_R8		0x07		// R8レジスタ番号
#define REGBANK_REG_NUM_R9		0x08		// R9レジスタ番号
#define REGBANK_REG_NUM_R10		0x09		// R10レジスタ番号
#define REGBANK_REG_NUM_R11		0x0A		// R11レジスタ番号
#define REGBANK_REG_NUM_R12		0x0B		// R12レジスタ番号
#define REGBANK_REG_NUM_R13		0x0C		// R13レジスタ番号
#define REGBANK_REG_NUM_R14		0x0D		// R14レジスタ番号
#define REGBANK_REG_NUM_R15		0x0E		// R15レジスタ番号
#define REGBANK_REG_NUM_USP		0x0F		// USPレジスタ番号
#define REGBANK_REG_NUM_FPSW	0x10		// FPSWレジスタ番号
#define REGBANK_REG_NUM_ACC0G	0x11		// ACC0Gレジスタ番号
#define REGBANK_REG_NUM_ACC0H	0x12		// ACC0Hレジスタ番号
#define REGBANK_REG_NUM_ACC0L	0x13		// ACC0Lレジスタ番号
#define REGBANK_REG_NUM_ACC1G	0x14		// ACC1Gレジスタ番号
#define REGBANK_REG_NUM_ACC1H	0x15		// ACC1Hレジスタ番号
#define REGBANK_REG_NUM_ACC1L	0x16		// ACC1Lレジスタ番号
// RevRxNo180625-001 Append End

// 初期化用定義
#define INIT_REG_GENERAL		0x00000000		// 汎用レジスタ
#define INIT_REG_CONTROL		0x00000000		// 制御レジスタ
// RevRxNo120910-001 Append Line
#define REG_NO_USE_VALUE		0x00000000		// レジスタ参照値として0x0000_0000を格納する場合に使用する。
#define REG_RX_EXTB_INIT_VAL 0xFFFFFF80		// EXTBリセット後の初期値	// RevRxNo130301-001 Append Line

// RevRxNo130301-001 Append Line
#define	MSK_REG_ACCG		0x000000FF		// ACC0G/ACC1Gの[31:8]マスク


#define SM_BIT					0x00020000		// スタックモード
typedef FFWRX_REG_DATA		FFWMCU_REG_DATA_RX;

// プロトタイプ宣言（構造体変数定義、アクセス関数）
extern void SetRegData_RX(FFWMCU_REG_DATA_RX* pReg);
extern FFWMCU_REG_DATA_RX* GetRegData_RX(void);

extern void InitFfwRegData(void);		// FFWレジスタ内部管理変数を初期化

// プロトタイプ宣言（コマンド処理部）
extern void InitFfwIfMcuData_Reg(void);		///< レジスタ操作コマンド用FFW内部変数の初期化(RX用)
// 2008.9.9 INSERT_END_E20RX600 }


#endif	// __FFWMCU_REG_H__
