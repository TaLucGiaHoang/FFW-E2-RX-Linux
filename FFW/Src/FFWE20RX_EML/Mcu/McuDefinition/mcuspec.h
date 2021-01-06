///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcuspec.h
 * @brief RXレジスタ情報定義のヘッダファイル
 * @author RSO Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, K.Uemori, S.Ueda, Y.Kawakami, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/02/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・V.1.03 RevNo111121-008 OFS1ライトデータチェック 2012/07/11 橋口
・V.1.03 RevRxNo120112-001 FFW SFRアクセス見直し対応 2012/07/11 橋口
・RevRxNo120606-003 2012/06/08 橋口
  ・SYSCR0レジスタがないMCU対応
・RevRxNo120606-004 2012/07/12 橋口
  ・MPU領域リード/ライト対応
・RevRxNo120910-004 2012/11/01 三宅
  ・MCU_OSM_OFS1_STUPLVD1REN_MASK_DATA、MCU_OSM_OFS1_STUPLVD1REN_DIS_DATAの定義追加。
・RevRxNo120910-003 2012/11/01 三宅
  ・以下の定義を削除。
　　MCU_OSM_OFS1_VDSEL_MASK_DATA、MCU_OSM_OFS1_VDSEL11、
　　MCU_OSM_OFS1_VDSEL10、MCU_OSM_OFS1_VDSEL01、MCU_OSM_OFS1_VDSEL00、
　　MCU_OSM_OFS1_VDSEL11_UVCC_RX200、MCU_OSM_OFS1_VDSEL10_UVCC_RX200、
　　MCU_OSM_OFS1_VDSEL01_UVCC_RX200、MCU_OSM_OFS1_VDSEL00_UVCC_RX200、
　　MCU_OSM_OFS1_VDSEL11_UVCC_RX630。
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121206-001 2012/12/07 植盛
  ブートスワップ時のキャッシュ処理対応
・RevRxNo130301-001 2013/08/23 上田、植盛
	RX64M対応
・RevRxNo130828-001 2013/11/11 植盛
	スタートアッププログラム保護機能によるスワップ時のキャッシュ更新処理改善
・RevRxNo130730-004 2013/11/25 川上
	Start/Stop高速化対応
・RevRxNo140109-001 2014/01/17 植盛、上田
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo130730-001 2014/06/18 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140617-001	2014/06/17 大喜多
	TrustedMemory機能対応
・RevRxNo140515-005 2014/07/18 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo140515-006 2014/08/28 川上、2014/12/18 大喜多
	RX231対応
・RevRxNo150827-005 2015/09/11 PA 紡車
	RX651対応
・RevRxNo150827-003 2015/12/01 PA 紡車
	RV40F Phase2対応
・RevRxNo160525-001 2016/06/27 PA 紡車
	RX24T HOCO 64MHz対応
・RevRxNo161003-001 2016/12/01 PA 紡車、PA 辻
　　RX651-2MB 起動時のバンクモード、起動バンク対応
	データフラッシュメモリアクセス周波数設定レジスタ対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
*/
#ifndef	__MCUSPEC_H__
#define	__MCUSPEC_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義

// RevRxNo130730-004 Append Start
// 命令コード
#define MCU_INST_CODE_NOP							0x03			//NOPコード
#define	MCU_INST_CODE_BRA_B							0x2E			//BRA.Bコード
// RevRxNo130730-004 Append End

// MCU CS空間アドレス
#define MCU_CS0_START_ADDR							0xFF000000		//CS0開始アドレス
#define MCU_CS0_END_ADDR							0xFFFFFFFF		//CS0終了アドレス
#define MCU_CS1_START_ADDR							0x07000000		//CS1開始アドレス
#define MCU_CS1_END_ADDR							0x07FFFFFF		//CS1終了アドレス
#define MCU_CS2_START_ADDR							0x06000000		//CS2開始アドレス
#define MCU_CS2_END_ADDR							0x06FFFFFF		//CS2終了アドレス
#define MCU_CS3_START_ADDR							0x05000000		//CS3開始アドレス
#define MCU_CS3_END_ADDR							0x05FFFFFF		//CS3終了アドレス
#define MCU_CS4_START_ADDR							0x04000000		//CS4開始アドレス
#define MCU_CS4_END_ADDR							0x04FFFFFF		//CS4終了アドレス
#define MCU_CS5_START_ADDR							0x03000000		//CS5開始アドレス
#define MCU_CS5_END_ADDR							0x03FFFFFF		//CS5終了アドレス
#define MCU_CS6_START_ADDR							0x02000000		//CS6開始アドレス
#define MCU_CS6_END_ADDR							0x02FFFFFF		//CS6終了アドレス
#define MCU_CS7_START_ADDR							0x01000000		//CS7開始アドレス
#define MCU_CS7_END_ADDR							0x01FFFFFF		//CS7終了アドレス

#define MCU_SDCS_START_ADDR							0x08000000		//SDRAM 開始アドレス
#define MCU_SDCS_END_ADDR							0x0FFFFFFF		//SDRAM 終了アドレス

//RevRxNo120606-004	Append Start
// MPU領域
#define MCU_MPU_ADDR_START							0x00086400		// MPUエリア開始アドレス 
#define MCU_MPU_ADDR_END							0x00086FFF		// MPUエリア終了アドレス
//RevRxNo120606-004	Append End

// 動作モードレジスタ//
#define MCU_REG_SYSTEM_MDMONR						0x00080000		// モードモニタレジスタ//
	#define MCU_REG_SYSTEM_MDMONR_MDE					0x0080		// モードモニタレジスタ MDE
	#define MCU_REG_SYSTEM_MDMONR_MDE_BIG				0x0080		// モードモニタレジスタ MDE BIGエンディアン
#define MCU_REG_SYSTEM_MDSR							0x00080002		// モードステータスレジスタ//
	#define	MCU_REG_SYSTEM_MDSR_IROM					0x0001		//モードステータスレジスタIROM 起動時の内蔵ROM状態
	#define	MCU_REG_SYSTEM_MDSR_EXB						0x0002		//モードステータスレジスタEXB 起動時の外部バス状態
	#define	MCU_REG_SYSTEM_MDSR_BOTS					0x0010		//モードステータスレジスタBOTS SCIブート起動
// V.1.02 RevNo110414-002 Appned Line
	#define MCU_REG_SYSTEM_MDSR_UBTS					0x0020		// モードステータスレジスタbit5確認定義
#define MCU_REG_SYSTEM_SYSCR0						0x00080006		// システムコントロールレジスタ0//
	#define MCU_REG_SYSTEM_SYSCR0_ROME					0x0001		// システムコントロールレジスタ0 ROMEビット
	#define MCU_REG_SYSTEM_SYSCR0_ROMD					0x0000		// システムコントロールレジスタ0 ROMD		// RevRxNo120606-003 Append Line
	#define MCU_REG_SYSTEM_SYSCR0_EXBE					0x0002		// システムコントロールレジスタ0 EXBEビット
#define MCU_REG_SYSTEM_SYSCR1						0x00080008		// システムコントロールレジスタ1//
	#define MCU_REG_SYSTEM_SYSCR1_RAME					0x0001		// システムコントロールレジスタ1 RAMEビット
	#define MCU_REG_SYSTEM_SYSCR1_RAME_ENA				0x0001		// システムコントロールレジスタ1 RAM有効設定データ	// RevRxNo130301-001 Append Line
#define MCU_REG_SYSTEM_MSTPCRC						0x00080018		// モジュールストップコントロールレジスタC//
// RevRxNo130301-001 Modify Start
	#define MCU_REG_SYSTEM_MSTPCRC_RAM0				0x00000001;		// RAM0ビット
	#define MCU_REG_SYSTEM_MSTPCRC_RAM0_ENA			0x00000000;		// RAM0動作設定データ
	#define MCU_REG_SYSTEM_MSTPCRC_RAM01			0x00000003;		// RAM0/RAM1ビット
	#define MCU_REG_SYSTEM_MSTPCRC_RAM01_ENA		0x00000000;		// RAM0/RAM1動作設定データ
// RevRxNo130301-001 Modify Start

//クロック関係レジスタ
// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append Start
#define MCU_REG_SYSTEM_SCKCR						0x00080020		// システムクロック制御レジスタ
	#define	MCU_REG_SYSTEM_SCKCR_ICK					0x0F000000	// システムクロック制御レジスタ ICKビット
	// RevRxNo140515-005 Append Start
	#define	MCU_REG_SYSTEM_SCKCR_ICK_1					0x00000000	// 1分周
	#define	MCU_REG_SYSTEM_SCKCR_ICK_2					0x01000000	// 2分周
	#define	MCU_REG_SYSTEM_SCKCR_ICK_4					0x02000000	// 4分周
	#define	MCU_REG_SYSTEM_SCKCR_ICK_8					0x03000000	// 8分周
	#define	MCU_REG_SYSTEM_SCKCR_ICK_16					0x04000000	// 16分周
	#define	MCU_REG_SYSTEM_SCKCR_ICK_32					0x05000000	// 32分周
	#define	MCU_REG_SYSTEM_SCKCR_ICK_64					0x06000000	// 64分周
	// RevRxNo140515-005 Append End
	// RevRxNo161003-001 Append Start +8
	#define	MCU_REG_SYSTEM_SCKCR_FCK					0xF0000000	// FlashIFクロック制御レジスタ FCKビット
	#define	MCU_REG_SYSTEM_SCKCR_FCK_1					0x00000000	// 1分周
	#define	MCU_REG_SYSTEM_SCKCR_FCK_2					0x10000000	// 2分周
	#define	MCU_REG_SYSTEM_SCKCR_FCK_4					0x20000000	// 4分周
	#define	MCU_REG_SYSTEM_SCKCR_FCK_8					0x30000000	// 8分周
	#define	MCU_REG_SYSTEM_SCKCR_FCK_16					0x40000000	// 16分周
	#define	MCU_REG_SYSTEM_SCKCR_FCK_32					0x50000000	// 32分周
	#define	MCU_REG_SYSTEM_SCKCR_FCK_64					0x60000000	// 64分周
	// RevRxNo161003-001 Append End
#define MCU_REG_SYSTEM_SCKCR2						0x00080024		// システムクロック制御レジスタ2
#define MCU_REG_SYSTEM_SCKCR3						0x00080026		// システムクロック制御レジスタ3
	// RevRxNo140515-005 Append Start
	#define MCU_REG_SYSTEM_SCKCR3_CLKSEL				0x0700		// CLKSELビット
	#define MCU_REG_SYSTEM_SCKCR3_LOCO					0x0000		// LOCO
	#define MCU_REG_SYSTEM_SCKCR3_HOCO					0x0100		// HOCO
	#define MCU_REG_SYSTEM_SCKCR3_EXTAL					0x0200		// EXTAL
	#define MCU_REG_SYSTEM_SCKCR3_SUB					0x0300		// SUB
	#define MCU_REG_SYSTEM_SCKCR3_PLL					0x0400		// PLL
	// RevRxNo140515-005 Append End
// RevRxNo140515-005 Append Start
#define MCU_REG_SYSTEM_PLLCR						0x00080028		// PLL制御レジスタ
	#define MCU_REG_SYSTEM_PLLCR_STC_MASK			0x0000003F		// PLLCR 8bit右シフト後のSTCビットマスク値
	// RevRxNo160525-001 Append Start
	#define MCU_REG_SYSTEM_PLLCR_PLLSRCSEL_RX24T		0x0004		// PLLSRCSELビット(RX24T)
	#define MCU_REG_SYSTEM_PLLCR_EXTAL_RX24T			0x0000		// PLLソースがEXTAL(RX24T)
	#define MCU_REG_SYSTEM_PLLCR_HOCO_RX24T				0x0004		// PLLソースがHOCO(RX24T)
	// RevRxNo160525-001 Append End
	#define MCU_REG_SYSTEM_PLLCR_PLLSRCSEL				0x0010		// PLLSRCSELビット
	#define MCU_REG_SYSTEM_PLLCR_EXTAL					0x0000		// PLLソースがEXTAL
	#define MCU_REG_SYSTEM_PLLCR_HOCO					0x0010		// PLLソースがHOCO
	#define MCU_REG_SYSTEM_PLLCR_DIV					0x0003		// PLLDIVビット
	#define MCU_REG_SYSTEM_PLLCR_1DIV					0x0000		// PLL 1分周
	#define MCU_REG_SYSTEM_PLLCR_2DIV					0x0001		// PLL 2分周
	#define MCU_REG_SYSTEM_PLLCR_3DIV					0x0002		// PLL 3分周(RX71M)
	#define MCU_REG_SYSTEM_PLLCR_4DIV					0x0002		// PLL 4分周(RX231)
// RevRxNo140515-005 Append End
#define MCU_REG_SYSTEM_HOCOCR						0x00080036		// 高速クロック発振器コントロールレジスタ
// RevRxNo140515-005 Append Start
#define MCU_REG_SYSTEM_HOCOCR2						0x00080037		// 高速クロック発振器コントロールレジスタ2
	#define MCU_REG_SYSTEM_HOCOCR2_HCFRQ				0x03		// HCFRGビット
	#define MCU_REG_SYSTEM_HOCOCR2_16_RX700				0x00		// 16MHz
	#define MCU_REG_SYSTEM_HOCOCR2_18_RX700				0x01		// 18MHz
	#define MCU_REG_SYSTEM_HOCOCR2_20_RX700				0x02		// 20MHz
	#define MCU_REG_SYSTEM_HOCOCR2_32_RX231				0x00		// 32MHz
	#define MCU_REG_SYSTEM_HOCOCR2_54_RX231				0x03		// 54MHz(RX231)、64MHz(RX24T)	// RevRxNo160525-001 Modify Line
// RevRxNo140515-005 Append End

#define MCU_REG_SYSTEM_OPCCR						0x000800A0		// 動作電力コントロールレジスタ
// RevRxNo140515-005 Append Start
	#define MCU_REG_SYSTEM_OPCCR_OPCM					0x07		// OPCMビット
	#define MCU_REG_SYSTEM_OPCCR_HIGH_RX700				0x00		// 高速動作モード
	#define MCU_REG_SYSTEM_OPCCR_LOW1_RX700				0x06		// 低速動作モード1
	#define MCU_REG_SYSTEM_OPCCR_LOW2_RX700				0x07		// 低速動作モード2
	#define MCU_REG_SYSTEM_OPCCR_HIGH_RX231				0x00		// 高速動作モード
	#define MCU_REG_SYSTEM_OPCCR_MID_RX231				0x02		// 中速動作モード
#define MCU_REG_SYSTEM_SOPCCR						0x000800A4		// サブ動作電力コントロールレジスタ
	#define MCU_REG_SYSTEM_SOPCCR_HIGHMID				0x00		// 高速動作モードまたは中速動作モード
	#define MCU_REG_SYSTEM_SOPCCR_LOW					0x01		// 低速動作モード
// RevRxNo140515-005 Append End
#define MCU_REG_SYSTEM_HOCOPCR						0x0008C294		// 高速オンチップオシレータ電源コントロールレジスタ
	#define	MCU_REG_SYSTEM_HOCOPCR_HOCOPCNT_ON			0x00		// HOCO設定データ( HOCO電源ON / HOCO動作 )
//V.1.03 RevRXNo120112-001 Append Start
#define MCU_REG_SYSTEM_MACJCR						0x00080200		// 電流ジャンプ対策制御レジスタ
	#define MCU_REG_SYSTEM_MACJCR_DATA					0x00		// PACC制御無効
//V.1.03 RevRXNo120112-001 Append End
// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append End

//プロテクト関係レジスタ
// V.1.02 新デバイス対応(ExtFlashModule_014) Append Start
#define MCU_REG_SYSTEM_PRCR							0x000803FE		// プロテクトレジスタ( RX630/RX210 )
	#define	MCU_REG_SYSTEM_PRCR_PRC0					0x0001		// PRC0ビットマスク値
	#define	MCU_REG_SYSTEM_PRCR_PRC1					0x0002		// PRC1ビットマスク値
//V.1.03 RevRXNo120112-001 Append Line
	#define	MCU_REG_SYSTEM_PRCR_PRC2					0x0400		// PRC2ビットマスク値
	#define	MCU_REG_SYSTEM_PRCR_PRC3					0x0008		// PRC3ビットマスク値
	#define	MCU_REG_SYSTEM_PRCR_ALL_PROTECT_SET			0x0000		// PRC全ビット0ライト
	#define	MCU_REG_SYSTEM_PRCR_PRKEY					0xA500		// PRCキーコード(PRCRレジスタ書き込み許可キー)
// V.1.02 新デバイス対応(ExtFlashModule_014) Append End

//バスステートコントローラ
#define MCU_REG_BSC_CS0CR							0x00083802		//CS0制御レジスタ
#define MCU_REG_BSC_CS1CR							0x00083812		//CS1制御レジスタ
#define MCU_REG_BSC_CS2CR							0x00083822		//CS2制御レジスタ
#define MCU_REG_BSC_CS3CR							0x00083832		//CS3制御レジスタ
#define MCU_REG_BSC_CS4CR							0x00083842		//CS4制御レジスタ
#define MCU_REG_BSC_CS5CR							0x00083852		//CS5制御レジスタ
#define MCU_REG_BSC_CS6CR							0x00083862		//CS6制御レジスタ
#define MCU_REG_BSC_CS7CR							0x00083872		//CS7制御レジスタ
	#define MCU_REG_BSC_CSCR_EMODE						0x0100		//CSn制御レジスタ EMODEbit

#define MCU_REG_BSC_SDCMOD							0x00083C01		//SDRAM SDCモードレジスタ
	#define MCU_REG_BSC_SDCMOD_EMODE					0x01		//SDRAM SDCモードレジスタ EMODEbit

// フラッシュアクセス関連レジスタ//
#define MCU_REG_FLASH_FMODR							0x007FC402		// フラッシュモードレジスタ//
#define MCU_REG_FLASH_FASTAT						0x007FC410		// フラッシュアクセスステータスレジスタ//
#define MCU_REG_FLASH_FAEINT						0x007FC411		// フラッシュアクセスエラー割り込み許可レジスタ//
#define MCU_REG_FLASH_FRDYIE						0x007FC412		// フラッシュレディ割り込み許可レジスタ//
#define MCU_REG_FLASH_DFLRE							0x007FC440		// データFlash読み出し許可レジスタ//
	#define	MCU_REG_FLASH_DFLRE_DATA					0x2D0f		// データFlash読み出し許可レジスタの書き込みデータ
	#define	MCU_REG_FLASH_DFLRE_DMASK					0x2D00		// データFlash読み出し許可レジスタのデータマスク
//RevNo010804-001	2010/08/06 Append Start
#define MCU_REG_FLASH_DFLRE0						0x007FC440		// データFlash読み出し許可レジスタ0 [RX602系]//
#define MCU_REG_FLASH_DFLRE1						0x007FC442		// データFlash読み出し許可レジスタ1 [RX602系]//
//RevNo010804-001 Append Start
	#define	MCU_REG_FLASH_DFLRE0_DATA					0x2Dff		// データFlash読み出し許可レジスタ0の書き込みデータ [RX602系]
	#define	MCU_REG_FLASH_DFLRE1_DATA					0xD2ff		// データFlash読み出し許可レジスタ1の書き込みデータ [RX602系]
//RevNo010804-001 Append End
//RevNo010804-001 Append Start
	#define	MCU_REG_FLASH_DFLRE0_KEYCODE				0x2D00		// データFlash読み出し許可レジスタ0のデータマスク [RX602系]
	#define	MCU_REG_FLASH_DFLRE1_KEYCODE				0xD200		// データFlash読み出し許可レジスタ1のデータマスク [RX602系]

//RevNo010804-001	2010/08/06 Append Start
#define MCU_REG_FLASH_DFLWE							0x007FC450		// データFlash書込み/消去許可レジスタ//
	#define	MCU_REG_FLASH_DFLWE_DATA					0x1E0F		// データFlash書込み/消去許可レジスタの書き込みデータ
	#define	MCU_REG_FLASH_DFLWE_DMASK					0x1E00		// データFlash書込み/消去許可レジスタのデータマスク
//RevNo010804-001	2010/08/06 Append End
#define MCU_REG_FLASH_DFLWE0						0x007FC450		// データFlash書込み/消去許可レジスタ0 [RX602系]//
#define MCU_REG_FLASH_DFLWE1						0x007FC452		// データFlash書込み/消去許可レジスタ1 [RX602系]//
//RevNo010804-001 Append Start
	#define	MCU_REG_FLASH_DFLWE0_KEYCODE				0x1E00		// データFlash書込み/消去許可レジスタ0のデータマスク
	#define	MCU_REG_FLASH_DFLWE1_KEYCODE				0xE100		// データFlash書込み/消去許可レジスタ1のデータマスク
//RevNo010804-001 Append End

//RevNo010804-001	2010/08/06 Append End
#define MCU_REG_FLASH_FCURAME						0x007FC454		// FCU RAMイネーブルレジスタ//
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Line
	#define	MCU_REG_FLASH_FCURAME_KEYCODE				0xC400		// FCU RAMイネーブルレジスタのデータマスク
	#define	MCU_REG_FLASH_FCURAME_DATA					0xC401		// FCU RAMイネーブルレジスタの書き込みデータ
	#define	MCU_REG_FLASH_FCURAME_DMASK					0xC400		// FCU RAMイネーブルレジスタのデータマスク

#define MCU_REG_FLASH_FSTATR0						0x007FFFB0		// フラッシュステータスレジスタ0//
#define MCU_REG_FLASH_FSTATR1						0x007FFFB1		// フラッシュステータスレジスタ1//
#define MCU_REG_FLASH_FENTRYR						0x007FFFB2		// フラッシュP/Eモードエントリレジスタ//
	#define	MCU_REG_FLASH_FENTRYR_DATA					0xAA83		// フラッシュP/Eモードエントリレジスタの書き込みデータ
	#define	MCU_REG_FLASH_FENTRYR_DMASK					0xAA00		// フラッシュP/Eモードエントリレジスタのデータマスク
// V.1.02 RevNo110629-001 Appned Start
	#define MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE		0x0080		// FENTRYレジスタ　データフラッシュP/Eモード設定ビットマスク値
	#define MCU_REG_FLASH_FENTRYR_USERMAT_PEMODE		0x000F		// FENTRYレジスタ　ROM P/Eモード設定ビットマスク値
	#define MCU_REG_FLASH_FENTRYR_NO_PEMODE				0x0000		// FENTRYレジスタ　P/Eモードではない場合の値
// V.1.02 RevNo110629-001 Appned End

#define MCU_REG_FLASH_FPROTR						0x007FFFB4		// フラッシュプロテクトレジスタ//
// V.1.02 新デバイス対応 Append&Modify Start
	#define	MCU_REG_FLASH_FPROTR_DATA					0x5501		// フラッシュプロテクトレジスタの書き込みデータ
	#define	MCU_REG_FLASH_FPROTR_DMASK					0x5500		// フラッシュプロテクトレジスタのデータマスク
#define MCU_REG_FLASH_FRESETR						0x007FFFB6		// フラッシュリセットレジスタ//
	#define	MCU_REG_FLASH_FRESETR_DMASK					0xCC00		// フラッシュP/Eモードエントリレジスタのデータマスク
#define MCU_REG_FLASH_FCMDR							0x007FFFBA		// FCUコマンドレジスタ//
#define MCU_REG_FLASH_FCPSR							0x007FFFC8		// FCU処理切り替えレジスタ//
#define MCU_REG_FLASH_DFLBCCNT						0x007FFFCA		// データフラッシュブランクチェック制御レジスタ//
#define MCU_REG_FLASH_FPESTAT						0x007FFFCC		// フラッシュP/Eステータスレジスタ//
#define MCU_REG_FLASH_DFLBCSTAT						0x007FFFCE		// データフラッシュブランクチェックステータスレジスタ//
#define MCU_REG_FLASH_PCKAR							0x007FFFE8		// 周辺クロック通知レジスタ//
#define MCU_REG_FLASH_FWEPROR						0x0008C289		// フラッシュライトイレースプロテクトレジスタ//

// RevRxNo121026-001 Append Start
	#define MCU_REG_FLASH_FENTRYR_KEY_RX100				0xAA00		// RX100 FENTRYRキーコード
	#define MCU_REG_FLASH_FENTRYR_FENTRYI_DATA_RX100	0x0001		// RX100 FENTRYI設定値
#define MCU_REG_FLASH_DFLCTL_RX100					0x007FC090		// RX100 データ・フラッシュ・コントロール
// RevRxNo121206-001 Append Start
#define MCU_REG_FLASH_FSCMR_RX100					0x007FC0B0		// RX100 フラッシュ・セキュリティフラグ・モニター・レジスタ
	#define	MCU_REG_FLASH_FSCMR_MASK					0x0100		
	#define	MCU_REG_FLASH_FSCMR_NONSWAP_DATA			0x0100		
	#define	MCU_REG_FLASH_FSCMR_SWAP_DATA				0x0000		// RevRxNo130828-001 Append Line
// RevRxNo121206-001 Append End
#define MCU_REG_FLASH_FISR_RX100					0x007FC0B6		// RX100 フラッシュ・シーケンサ・初期設定レジスタ
// RevRxNo121206-001 Append Line
	#define	MCU_REG_FLASH_FISR_MASK					0xC0		
	#define	MCU_REG_FLASH_FISR_NONSWAP_DATA			0x80		// RevRxNo130828-001 Append Line
	#define	MCU_REG_FLASH_FISR_SWAP_DATA				0xC0		
#define MCU_REG_FLASH_FEXCR_RX100					0x007FC0B7		// RX100 フラッシュ・Extra用シーケンサ制御レジスタ
#define MCU_REG_FLASH_FPR_RX100						0x007FC0C0		// RX100 フラッシュ・プロテクト・コマンド・レジスタ
	#define MCU_REG_FLASH_FPR_DATA_RX100				0xA5		
#define MCU_REG_FLASH_FPMCR_RX100					0x007FFF80		// RX100 フラッシュ・プログラミング・モード・コントロール・レジスタ
	#define MCU_REG_FLASH_FPMCR_PESET_DATA_RX100		0x12		
	#define MCU_REG_FLASH_FPMCR_PESET_NOTDATA_RX100		0xED		
#define MCU_REG_FLASH_FASR_RX100					0x007FFF81		// RX100 フラッシュ・領域選択レジスタ
#define MCU_REG_FLASH_FSARL_RX100					0x007FFF82		// RX100 フラッシュ・アドレス・ポインタL
#define MCU_REG_FLASH_FSARH_RX100					0x007FFF84		// RX100 フラッシュ・アドレス・ポインタH
#define MCU_REG_FLASH_FCR_RX100						0x007FFF85		// RX100 フラッシュ・シーケンサ制御レジスタ
#define MCU_REG_FLASH_FEARL_RX100					0x007FFF86		// RX100 フラッシュ・エンド・アドレス・ポインタL
#define MCU_REG_FLASH_FEARH_RX100					0x007FFF88		// RX100 フラッシュ・エンド・アドレス・ポインタH
#define MCU_REG_FLASH_FRESETR_RX100					0x007FFF89		// RX100 フラッシュ・レジスタ・初期化レジスタ
#define MCU_REG_FLASH_FWBL_RX100					0x007FFF8C		// RX100 フラッシュ・ライト・バッファ・レジスタL
#define MCU_REG_FLASH_FWBH_RX100					0x007FFF8E		// RX100 フラッシュ・ライト・バッファ・レジスタH
#define MCU_REG_FLASH_SET_REGNUM_RX100				22				//RX100 フラッシュ書き換え後ユーザ値によって復帰時、ライトするレジスタ数
// RevRxNo121026-001 Append End
// RevRxNo121206-001 Append Start
#define	MCU_REG_FLASH_PORGA_RX100					0x007FFFB6		// RX100 製品情報キャプチャレジスタ
	#define	MCU_REG_FLASH_PORGA_MASK					0xC000		
	#define MCU_REG_FLASH_PORGA_SWAP_4KB				0x0000		// ブートスワップサイズ4KB
	#define MCU_REG_FLASH_PORGA_SWAP_8KB				0x4000		// ブートスワップサイズ8KB
	#define MCU_REG_FLASH_PORGA_SWAP_16KB_1				0x8000		// ブートスワップサイズ16KB
	#define MCU_REG_FLASH_PORGA_SWAP_16KB_2				0xC000		// ブートスワップサイズ16KB
// RevRxNo121206-001 Append End
// RevRxNo140515-006 Append Start
#define MCU_REG_FLASH_DFLCTL_RX230					0x007FC090
// RevRxNo140515-006 Modify Line
#define MCU_REG_FLASH_FSCMR_RX230					0x007FC1C0		// RX230 フラッシュ・セキュリティフラグ・モニター・レジスタ
#define MCU_REG_FLASH_FISR_RX230					0x007FC1D8		// RX230 フラッシュ・シーケンサ・初期設定レジスタ
#define MCU_REG_FLASH_FEXCR_RX230					0x007FC1DC		// RX230 フラッシュ・Extra用シーケンサ制御レジスタ
#define MCU_REG_FLASH_FPR_RX230						0x007FC180		// RX230 フラッシュ・プロテクト・コマンド・レジスタ
#define MCU_REG_FLASH_FPMCR_RX230					0x007FC100		// RX230 フラッシュ・プログラミング・モード・コントロール・レジスタ
#define MCU_REG_FLASH_FASR_RX230					0x007FC104		// RX230 フラッシュ・領域選択レジスタ
#define MCU_REG_FLASH_FSARL_RX230					0x007FC108		// RX230 フラッシュ・アドレス・ポインタL
#define MCU_REG_FLASH_FSARH_RX230					0x007FC110		// RX230 フラッシュ・アドレス・ポインタH
#define MCU_REG_FLASH_FCR_RX230						0x007FC114		// RX230 フラッシュ・シーケンサ制御レジスタ
#define MCU_REG_FLASH_FEARL_RX230					0x007FC118		// RX230 フラッシュ・エンド・アドレス・ポインタL
#define MCU_REG_FLASH_FEARH_RX230					0x007FC120		// RX230 フラッシュ・エンド・アドレス・ポインタH
#define MCU_REG_FLASH_FRESETR_RX230					0x007FC124		// RX230 フラッシュ・レジスタ・初期化レジスタ
#define MCU_REG_FLASH_FWB0_RX230					0x007FC130		// RX230 フラッシュ・ライト・バッファ・レジスタ0
#define MCU_REG_FLASH_FWB1_RX230					0x007FC138		// RX230 フラッシュ・ライト・バッファ・レジスタ1
#define MCU_REG_FLASH_FWB2_RX230					0x007FC140		// RX230 フラッシュ・ライト・バッファ・レジスタ2
#define MCU_REG_FLASH_FWB3_RX230					0x007FC144		// RX230 フラッシュ・ライト・バッファ・レジスタ3
#define MCU_REG_FLASH_SET_REGNUM_RX230				24				// RX230 フラッシュ書き換え後ユーザ値によって復帰時、ライトするレジスタ数


// RevRxNo140515-006 Append End
// RevRxNo130301-001 Append Start
#define MCU_REG_FLASH_FWEPROR_RX640					0x0008C296		// フラッシュライトイレースプロテクトレジスタ//
#define	MCU_REG_FLASH_FCURAME_RX640					0x007FE054		// RX640 FCURAMイネーブルレジスタ
	#define	MCU_REG_FLASH_FCURAME_RX640_KEYCODE			0xC400		// FCURAMレジスタのキーコード
#define	MCU_REG_FLASH_FPCKAR_RX640					0x007FE0E4		// RX640 フラッシュシーケンサ処理クロック通知レジスタ
	#define	MCU_REG_FLASH_FPCKAR_RX640_KEYCODE			0x1E00		// FPCKARレジスタのキーコード
#define	MCU_REG_FLASH_FSUINITR_RX640				0x007FE08C		// RX640 フラッシュシーケンサ設定初期化レジスタ
	#define	MCU_REG_FLASH_FSUINITR_RX640_KEYCODE		0x2D00		// FSUINITRレジスタのキーコード
#define	MCU_REG_FLASH_FENTRYR_RX640					0x007FE084		// RX640 フラッシュP/Eモードエントリレジスタ
	#define	MCU_REG_FLASH_FENTRYR_RX640_KEYCODE			0xAA00		// FENTRYRレジスタのキーコード
#define	MCU_REG_FLASH_FMATSELC_RX640				0x007FE020		// RX640 コードフラッシュマット選択レジスタ
	#define	MCU_REG_FLASH_FMATSELC_RX640_KEYCODE		0x3B00		// FMATSELCレジスタのキーコード
#define	MCU_REG_FLASH_FPROTR_RX640					0x007FE088		// RX640 フラッシュプロテクトレジスタ
	#define	MCU_REG_FLASH_FPROTR_RX640_KEYCODE			0x5500		// FPROTRレジスタのキーコード
#define	MCU_REG_FLASH_FSADDR_RX640					0x007FE030		// RX640 FACIコマンド処理開始アドレスレジスタ
#define MCU_REG_FLASH_SET_REGNUM_BYTE_RX640			1				// RX640 フラッシュ書き換え後ユーザ値によって復帰時、ライトするバイトのレジスタ数
#define MCU_REG_FLASH_SET_REGNUM_WORD_RX640			6				// RX640 フラッシュ書き換え後ユーザ値によって復帰時、ライトするワードのレジスタ数
#define MCU_REG_FLASH_SET_REGNUM_LWORD_RX640		1				// RX640 フラッシュ書き換え後ユーザ値によって復帰時、ライトするロングのレジスタ数
// RevRxNo130301-001 Append End

// RevRxNo161003-001 Append Line
#define MCU_REG_EEPFCLK								0x007FC040		// データフラッシュメモリアクセス周波数設定レジスタ(EEPFCLK)

//オプション設定メモリ(内蔵Flash上のSFR)
#define MCU_IDCODE_ADDR_START						0xFFFFFFA0		// IDコード設定領域開始アドレス
#define MCU_IDCODE_ADDR_END							0xFFFFFFAF	// IDコード設定領域終了アドレス

// V.1.02 No.3 起動時エンディアン&デバッグ継続モード処理 Append Start
#define	MCU_OSM_MDES_START							0xFFFFFF80		// エンディアン選択レジスタ開始アドレス(ユーザマット用)
#define	MCU_OSM_MDES_END							0xFFFFFF83		// エンディアン選択レジスタ終了アドレス(ユーザマット用)
#define	MCU_OSM_MDEB_START							0xFF7FFFF8		// エンディアン選択レジスタ開始アドレス(ユーザブート、USBブート用)
#define	MCU_OSM_MDEB_END							0xFF7FFFFb		// エンディアン選択レジスタ終了アドレス(ユーザブート、USBブート用)
	#define MCU_OSM_MDE_BIG								0xF8		// エンディアン用MDEビット値(BIG)
	#define MCU_OSM_MDE_LITTLE							0x07		// エンディアン用MDEビット値(LITTLE)
	#define MCU_OSM_MDE_MASK_DATA						0x00000007	// MDEビットマスク値
	#define MCU_OSM_MDE_LITTLE_DATA						0x00000007	// エンディアン用MDEビット値(LITTLE)
	#define MCU_OSM_MDE_BIG_DATA						0x00000000	// エンディアン用MDEビット値(BIG)
// RevRxNo161003-001 Append Start
	#define	MCU_OSM_BANKMD_MASK_DATA					0x00000070	// BANKMDビットマスク値
	#define	MCU_OSM_BANKMD_DUAL_DATA					0x00000000	// バンクモード用BANKMDビット値(デュアルモード)
	#define	MCU_OSM_BANKMD_LINEAR_DATA					0x00000070	// バンクモード用BANKMDビット値(リニアモード)
#define MCU_OSM_BANKSEL_START						0xFE7F5D20		// BANKSELレジスタ開始アドレス
	#define	MCU_OSM_BANKSWP_MASK_DATA					0x00000007	// BANKSWPビットマスク値
	#define	MCU_OSM_BANKSWP_BANK0_DATA					0x00000007	// 起動バンク用BANKSWPビット値(起動バンク0)
	#define	MCU_OSM_BANKSWP_BANK1_DATA					0x00000000	// 起動バンク用BANKSWPビット値(起動バンク1)
	#define	MCU_OSM_BANKSWP_CONT1_DATA					0x00000001	// 起動バンク用BANKSWPビット値(起動バンク継続：起動バンク1)
	#define	MCU_OSM_BANKSWP_CONT2_DATA					0x00000002	// 起動バンク用BANKSWPビット値(起動バンク継続：起動バンク1)
	#define	MCU_OSM_BANKSWP_CONT3_DATA					0x00000003	// 起動バンク用BANKSWPビット値(起動バンク継続：起動バンク0)
	#define	MCU_OSM_BANKSWP_CONT4_DATA					0x00000004	// 起動バンク用BANKSWPビット値(起動バンク継続：起動バンク1)
	#define	MCU_OSM_BANKSWP_CONT5_DATA					0x00000005	// 起動バンク用BANKSWPビット値(起動バンク継続：起動バンク0)
	#define	MCU_OSM_BANKSWP_CONT6_DATA					0x00000006	// 起動バンク用BANKSWPビット値(起動バンク継続：起動バンク0)
// RevRxNo161003-001 Append End
#define MCU_OSM_OFS1_START							0xFFFFFF88		// オプション機能選択レジスタ開始アドレス
#define MCU_OSM_OFS1_END							0xFFFFFF8b		// オプション機能選択レジスタ終了アドレス
	#define MCU_OSM_OFS1_OCD_MASK						0x03		// OFS1レジスタのOCDMODビットマスク値
	#define MCU_OSM_OFS1_OCDMOD_LIVE_DEBUG				0x01		// LiveDebugモード
	#define MCU_OSM_OFS1_OCDMOD_DEBUG					0x02		// デバッグ継続モード
	#define MCU_OSM_OFS1_OCDMOD_SINGLE					0x03		// シングルチップモード
	#define MCU_OSM_OFS1_OCDMOD_BAN_SINGLE				0x00		// 設定禁止(シングルチップモード)
	#define MCU_OSM_OFS1_HOCOEN							0x01		// リセット後HOCO有効/無効ビット( 0：有効、1：無効 )
	#define MCU_OSM_OFS1_OCD_MASK_FULL					0x03000000	// OFS1レジスタのOCDMODビットマスク値(LWORD)	// RevRxNo130730-001 Append Line
	#define MCU_OSM_OFS1_OCDMOD_DEBUG_FULL				0x02000000	// デバッグ継続モード(LWORD)					// RevRxNo130730-001 Append Line
	#define MCU_OSM_OFS1_LVD_MASK_FULL					0x000000FF	// OFS1レジスタのLVDビットマスク値(LWORD)		// RevRxNo130730-001 Append Line
#define MCU_OSM_UBCODE_A_START						0xFF7FFFE8		// UBコードAレジスタ開始アドレス
#define MCU_OSM_UBCODE_A_END						0xFF7FFFEF		// UBコードAレジスタ終了アドレス
#define MCU_OSM_UBCODE_B_START						0xFF7FFFF0		// UBコードBレジスタ開始アドレス
#define MCU_OSM_UBCODE_B_END						0xFF7FFFF7		// UBコードBレジスタ終了アドレス
// V.1.02 No.3 起動時エンディアン&デバッグ継続モード処理 Append End
// V.1.03 RevNo111121-008 Append Start
	#define MCU_OSM_OFS1_LVDAS_DIS_DATA					0x00000004		// OFS1 LVDAS 無効設定
	#define MCU_OSM_OFS1_LVDAS_MASK_DATA				0x00000004		// OFS1 LVDAS ビットマスク値
	// RevRxNo120910-003 Delete
// V.1.03 RevNo111121-008 Append End
// RevRxNo120910-004 Append Start
	#define MCU_OSM_OFS1_STUPLVD1REN_MASK_DATA			0x00000002		// OFS1 STUPLVD1REN ビットマスク値(RX100用)
	#define MCU_OSM_OFS1_STUPLVD1REN_DIS_DATA			0x00000002		// OFS1 STUPLVD1REN 無効設定(RX100用)
// RevRxNo120910-004 Append End
// RevRxNo140109-001 Append Start
	#define MCU_OSM_OFS1_LVD_DIS_NON					0x00000000		// OFS1のLVD機能なし
	#define MCU_OSM_OFS1_LVD_MASK_NON					0x00000000		// OFS1のLVD機能なし
// RevRxNo140109-001 Append End

// RevRxNo130301-001 Append Start
#define MCU_OSM_OSIS_START_RX640					0x00120050		// OCD/シリアルプログラマID設定レジスタ開始アドレス(RX64x用)	// RevRxNo140109-001 Append Line
#define MCU_OSM_MDE_START_RX640						0x00120064		// エンディアン選択レジスタ開始アドレス(RX64x用)
#define MCU_OSM_OFS1_START_RX640					0x0012006C		// オプション機能選択レジスタ1開始アドレス(RX64x用)
#define MCU_OSM_OFS1_END_RX640						0x0012006F		// オプション機能選択レジスタ1終了アドレス(RX64x用)	// RevRxNo140109-001 Append Line
#define MCU_EXTRA_A_CACHE_START_RX640				0x00120040		// ExtraA領域用キャッシュ開始アドレス(MCU_EXTRA_A_AREA_START_RX640のリードアドレスを定義する)	// RevRxNo150827-005 Modify Line
#define MCU_EXTRA_A_AREA_START_RX640				0x00000040		// ExtraA領域の開始アドレス(RX64x用)	// RevRxNo150827-005 Modify Line
#define MCU_EXTRA_A_AREA_SIZE_RX640					0x00000040		// ExtraA領域サイズ(RX64x用)			// RevRxNo150827-005 Modify Line
#define MCU_EXTRA_A_IDCODE_START					0x00000050		// ExtraA領域のIDコード開始アドレス		// RevRxNo140109-001 Append Line
#define MCU_EXTRA_A_IDCODE_END						0x0000005F		// ExtraA領域のIDコード終了アドレス		// RevRxNo140109-001 Append Line
#define MCU_EXTRA_A_MDE_START_RX640					0x00000064		// ExtraA領域のMDEレジスタアドレス(RX64x用)		// RevRxNo150827-005 Modify Line
#define MCU_EXTRA_A_OFS1_START_RX640				0x0000006C		// ExtraA領域のOFS1レジスタアドレス(RX64x用)	// RevRxNo150827-005 Modify Line
// RevRxNo130301-001 Append End
// RevRxNo140617-001 Append Start
#define MCU_OSM_TMEF_START_RX640					0x00120048		// Trusted Memory Enable Flag(TMEF)(RX64x用)
	#define MCU_OSM_TMEF_MASK							0x07000000		// TMEF.bit26-24マスク値
	#define MCU_OSM_TMEF_ENA1							0x00000000		// TM機能有効値1(000b)
	#define MCU_OSM_TMEF_ENA2							0x01000000		// TM機能有効値2(001b)
	#define MCU_OSM_TMEF_ENA3							0x02000000		// TM機能有効値3(010b)
	#define MCU_OSM_TMEF_ENA4							0x04000000		// TM機能有効値4(100b)
	#define MCU_OSM_TMEF_DIS1							0x03000000		// TM機能無効値1(011b)
	#define MCU_OSM_TMEF_DIS2							0x05000000		// TM機能無効値2(101b)
	#define MCU_OSM_TMEF_DIS3							0x06000000		// TM機能無効値3(110b)
	#define MCU_OSM_TMEF_DIS4							0x07000000		// TM機能無効値4(111b)
	// RevRxNo161003-001 Append Start
	#define MCU_OSM_TMEFDB_MASK							0x70000000		// TMEF.bit30-28マスク値
	#define MCU_OSM_TMEFDB_BANK1TM_0					0x00000000		// FFEE0000h～FFEEFFFFhのTM機能有効値(000b)
	#define MCU_OSM_TMEFDB_BANK1TM_1					0x10000000		// FFEE0000h～FFEEFFFFhのTM機能有効値(001b)
	#define MCU_OSM_TMEFDB_BANK1TM_2					0x20000000		// FFEE0000h～FFEEFFFFhのTM機能有効値(010b)
	#define MCU_OSM_TMEFDB_BANK1TM_3					0x30000000		// FFEE0000h～FFEEFFFFhのTM機能無効値(011b)
	#define MCU_OSM_TMEFDB_BANK1TM_4					0x40000000		// FFEE0000h～FFEEFFFFhのTM機能有効値(100b)
	#define MCU_OSM_TMEFDB_BANK1TM_5					0x50000000		// FFEE0000h～FFEEFFFFhのTM機能無効値(101b)
	#define MCU_OSM_TMEFDB_BANK1TM_6					0x60000000		// FFEE0000h～FFEEFFFFhのTM機能無効値(110b)
	#define MCU_OSM_TMEFDB_BANK1TM_7					0x70000000		// FFEE0000h～FFEEFFFFhのTM機能無効値(111b)
	// RevRxNo161003-001 Append End
#define MCU_OSM_TMINF_START_RX640					0x00120060		// Trusted Memory識別データ(TMINF)(RX64x用)
#define MCU_EXTRA_A_TMINF_START_RX640				0x00000060		// ExtraA領域のMDEレジスタアドレス(RX64x用)	// RevRxNo150827-005 Modify Line
// RevRxNo140617-001 Append End

// RevRxNo150827-005 Append Start
#define MCU_EXTRA_A_START_RX650						0xFE7F5D00		// ExtraA領域開始アドレス(RX65x用)
#define MCU_EXTRA_A_END_RX650						0xFE7F5D7F		// ExtraA領域終了アドレス(RX65x用)
#define MCU_EXTRA_A_BAN_START_RX650					0x00000020		// ExtraA領域ダウンロード禁止開始アドレス(RX651-1MB用)
#define MCU_EXTRA_A_BAN_START_RX650_2MB				0x00000030		// ExtraA領域ダウンロード禁止開始アドレス(RX651-2MB用)	// RevRxNo161003-001 Append Line
#define MCU_EXTRA_A_BAN_END_RX650					0x0000004F		// ExtraA領域ダウンロード禁止終了アドレス(RX65x用)
#define MCU_EXTRA_A_CACHE_START_RX650				0xFE7F5D00		// ExtraA領域用キャッシュ開始アドレス(MCU_EXTRA_A_AREA_START_RX650のリードアドレスを定義する)
#define MCU_OSM_OSIS_START_RX650					0xFE7F5D50		// OCD/シリアルプログラマID設定レジスタ開始アドレス(RX65x用)
#define MCU_OSM_MDE_START_RX650						0xFE7F5D00		// エンディアン選択レジスタ開始アドレス(RX65x用)
#define MCU_OSM_OFS1_START_RX650					0xFE7F5D08		// オプション機能選択レジスタ1開始アドレス(RX65x用)
#define MCU_OSM_TMEF_START_RX650					0xFE7F5D48		// Trusted Memory Enable Flag(TMEF)(RX65x用)
#define MCU_OSM_TMINF_START_RX650					0xFE7F5D10		// Trusted Memory識別データ(TMINF)(RX65x用)
#define MCU_EXTRA_A_AREA_START_RX650				0x00000000		// ExtraA領域の開始アドレス(RX65x用)
#define MCU_EXTRA_A_AREA_SIZE_RX650					0x00000080		// ExtraA領域サイズ(RX65x用)
#define MCU_EXTRA_A_MDE_START_RX650					0x00000000		// ExtraA領域のMDEレジスタアドレス(RX65x用)
#define MCU_EXTRA_A_OFS1_START_RX650				0x00000008		// ExtraA領域のOFS1レジスタアドレス(RX65x用)
#define MCU_EXTRA_A_TMINF_START_RX650				0x00000010		// ExtraA領域のMDEレジスタアドレス(RX65x用)
#define MCU_EXTRA_A_FAW_START_RX650					0x00000064		// ExtraA領域のFAWレジスタアドレス(RX65x用)	// RevRxNo150827-003 Append Line
	#define MCU_OSM_FAW_FSPR_MASK					0x80			// FAW.FSPR(bit15)マスク値	// RevRxNo150827-003 Append Line
// RevRxNo150827-005 Append End

// RevRxNo180228-001 Append Line
#define MCU_OSM_OFS1_END_RX66T						0x0012007F		// コンフィギュレーション設定領域ダウンロード対象終了アドレス(RX66x用)

#define	MCU_EXTRA_A_BANKSEL_START					0x00000020		// BANKSELレジスタアドレス	// RevRxNo161003-001 Append Line
#define	MCU_DUAL_STARTUP_BANK_START					0xFFF00000		// 起動バンク側先頭アドレス	// RevRxNo161003-001 Append Line

#define MCU_SNG_MODE_REST_VECT_ADDR			0xFFFFFFFC		//シングルチップモードのリセットベクタアドレス


#endif	// __MCUSPEC_H__
