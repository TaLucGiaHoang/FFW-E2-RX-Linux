////////////////////////////////////////////////////////////////////////////////
/**
 * @file ocpspec.h
 * @brief OCD関連のレジスタヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda, Y.Miyake, K.Okita, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2016/10/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RX220 WS対応 RX210でRX220対応 2012/07/11 橋口
・RevNo120131-001　2012/07/13 橋口
  GPB実行中の実行前、イベントブレーク不具合対応　COMB内のPACレジスタ設定値をdefine定義
・RevRxNo130308-001	2013/03/25 三宅
　カバレッジ開発対応
・RevRxNo130301-001 2013/09/10 上田
	RX64M対応
・RevRxNo131101-006	2013/11/11 上田
	GPB実行中のパフォーマンス計測不具合修正
・RevRxNo140515-005 2014/06/19 大喜多
	RX71M対応
・RevRxNo161003-001	2016/10/05 PA 辻
	ROMデータトレースアクセスタイプ出力対応
*/
#ifndef	__OCDSPEC_H__
#define	__OCDSPEC_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義
// イベント関連レジスタ//
// 実行PCイベント関連レジスタ//
#define OCD_REG_EV_EVEPCPREE				0x00084C00		// 実行前PCブレークイベント動作許可//
	#define OCD_REG_EV_EVEPCPREE_ALL_DIS	0x00000000		// 実行前PCブレークイベント動作許可 全無効//
#define OCD_REG_EV_EVEPCPE					0x00084C08		// 実行PC通過イベント動作許可//
#define OCD_REG_EV_EVEPCPFLG				0x00084C14		// 実行PC通過イベント成立結果//
#define OCD_REG_EV_EVEPCA					0x00084A00		// 実行PCイベントiアドレス（i=0～7）//

// オペランドアクセス設定レジスタ//
#define OCD_REG_EV_EVEOPE					0x00084C20		// オペランドアクセスイベント動作許可//
#define OCD_REG_EV_EVEOPFLG					0x00084C34		// オペランドアクセスイベント成立結果//
#define OCD_REG_EV_EVEOPA					0x00084E00		// オペランドアクセスイベントjアドレス（j=0～3）//
#define OCD_REG_EV_EVEOPAM					0x00084E40		// オペランドアクセスイベントjアドレスマスク（j=0～3）//
#define OCD_REG_EV_EVEOPD					0x00084E80		// オペランドアクセスイベントjデータ（j=0～3）//
#define OCD_REG_EV_EVEOPDM					0x00084EC0		// オペランドアクセスイベントjデータマスク（j=0～3）//
#define OCD_REG_EV_EVEOPCD					0x00084F00		// オペランドアクセスイベントj検出条件指定（j=0～3）//
// RevRxNo130301-001 Append Line
#define OCD_REG_EV_EVEOPSB					0x00084F80		// オペランドアクセスイベントjバス選択（j=0～3）//

// イベント成立回数設定レジスタ//
#define OCD_REG_EV_EVECNT					0x00084C40		// イベント成立回数カウントii（ii=0）//
#define OCD_REG_EV_EVECNTSEL				0x00084C80		// 成立回数指定イベント選択ii（ii=0）//
	#define OCD_REG_EV_EVECNTSEL_SEL			0x0000001F		
	#define OCD_REG_EV_EVECNTSEL_CNTK_EXEC		0x00000100		
	#define OCD_REG_EV_EVECNTSEL_CNTK_OPC		0x00000200		

// イベントトリガ設定レジスタ
#define OCD_REG_EV_EVETTRG					0x00084D80		// トレース用イベントトリガ出力制御//
#define OCD_REG_EV_EVEPTRG					0x00084D84		// パフォーマンス計測用イベントトリガ出力制御//

// イベント組合せ設定レジスタ//
#define OCD_REG_EV_EVEBRKTSC				0x00084CC0		// イベントブレーク/トレース開始イベント組合せ指定//
	#define OCD_REG_EV_EVEBRKTSC_BTCTC			0x00000100	// イベントブレーク/トレース開始イベント組合せ指定 初期化ビット//
#define OCD_REG_EV_EVEBRKPCP				0x00084CC4		// イベントブレーク用実行PC通過イベント選択//
	#define OCD_REG_EV_EVEBRKPCP_ALL_DIS		0x00000000	// イベントブレーク用実行PC通過イベント選択 全無効//
#define OCD_REG_EV_EVEBRKOP					0x00084CC8		// イベントブレーク用オペランドアクセスイベント選択//
	#define OCD_REG_EV_EVEBRKOP_ALL_DIS			0x00000000	// イベントブレーク用オペランドアクセスイベント選択 全無効//
	#define OCD_REG_EV_EVBRK_ALL_DIS			0x00000000	// イベントブレーク用実行PC通過/オペランドアクセスイベント選択 全無効//
#define OCD_REG_EV_EVETSPCP					0x00084CD0		// トレース開始用実行PC通過イベント選択//
#define OCD_REG_EV_EVETSOP					0x00084CD4		// トレース開始用オペランドアクセスイベント選択//
#define OCD_REG_EV_EVETEPCP					0x00084CD8		// トレース終了用実行PC通過イベント選択//
#define OCD_REG_EV_EVETEOP					0x00084CDC		// トレース終了用オペランドアクセスイベント選択//
#define OCD_REG_EV_EVETXOP					0x00084CE0		// トレース抽出用オペランドアクセスイベント選択//
#define OCD_REG_EV_EVEPSPCP0				0x00084D00		// パフォーマンス計測0開始用実行PC通過イベント選択//
#define OCD_REG_EV_EVEPSPCP1				0x00084D04		// パフォーマンス計測1開始用実行PC通過イベント選択//
#define OCD_REG_EV_EVEPSOP0					0x00084D20		// パフォーマンス計測0開始用オペランドアクセスイベント選択//
#define OCD_REG_EV_EVEPSOP1					0x00084D24		// パフォーマンス計測1開始用オペランドアクセスイベント選択//
#define OCD_REG_EV_EVEPEPCP0				0x00084D40		// パフォーマンス計測0終了用実行PC通過イベント選択//
#define OCD_REG_EV_EVEPEPCP1				0x00084D44		// パフォーマンス計測1終了用実行PC通過イベント選択//
#define OCD_REG_EV_EVEPEOP0					0x00084D60		// パフォーマンス計測0終了用オペランドアクセスイベント選択//
#define OCD_REG_EV_EVEPEOP1					0x00084D64		// パフォーマンス計測1終了用オペランドアクセスイベント選択//

// トレース設定関連レジスタ//
#define OCD_REG_RM_TRCR						0x00084800		// トレースコントロールレジスタ//
	#define OCD_REG_RM_TRCR_TRV					0x80000000
	#define OCD_REG_RM_TRCR_TBW					0x0F000000
	#define OCD_REG_RM_TRCR_TRC					0x000E0000
	#define OCD_REG_RM_TRCR_TRO					0x00010000
	#define OCD_REG_RM_TRCR_TPE					0x00001800
	#define OCD_REG_RM_TRCR_TSE					0x00002000
	#define OCD_REG_RM_TRCR_TSD					0x0000C000	// RevRxNo130301-001 Append Line
	#define OCD_REG_RM_TRCR_TDA					0x00000400
	#define OCD_REG_RM_TRCR_TMO					0x00000200
	#define OCD_REG_RM_TRCR_TRI					0x00000100
	#define OCD_REG_RM_TRCR_TOI					0x00000020
	#define OCD_REG_RM_TRCR_TDE					0x00000010
	#define OCD_REG_RM_TRCR_TBE					0x00000008
	#define OCD_REG_RM_TRCR_TRM					0x00000002
	#define OCD_REG_RM_TRCR_TRE					0x00000001
	// RevRxNo140515-005 Append Start
	#define OCD_REG_RM_TRCR_TRC_60MHz			60.0		// 60MHz(トレースクロック分周比を1:1の上限)
	#define OCD_REG_RM_TRCR_TRC_120MHz			120.0		// 120MHz(トレースクロック分周比を2:1の上限)
	#define OCD_REG_RM_TRCR_TRC_1_1				0x00000000	// トレースクロック分周比1:1
	#define OCD_REG_RM_TRCR_TRC_2_1				0x00020000	// トレースクロック分周比2:1
	#define OCD_REG_RM_TRCR_TRC_4_1				0x00040000	// トレースクロック分周比4:1
	// RevRxNo140515-005 Append End
#define OCD_REG_RM_TACR						0x00084804		// トレース動作中コントロールレジスタ//
#define OCD_REG_RM_TRCS						0x00084808		// トレースデータ圧縮方式設定//
	#define OCD_REG_RM_TRCS_BSS					0x01000000 
	#define OCD_REG_RM_TRCS_BTA					0x00100000 
	#define OCD_REG_RM_TRCS_BCE					0x00010000 
	#define OCD_REG_RM_TRCS_RODTE				0x00000020	// ROMデータトレース アクセスタイプ出力設定//	// RevRxNo161003-001 Append Line
	#define OCD_REG_RM_TRCS_DAE					0x00000010 
	#define OCD_REG_RM_TRCS_DISE				0x00000008 
	#define OCD_REG_RM_TRCS_TSCE				0x00000004 
	#define OCD_REG_RM_TRCS_RODE				0x00000002 
	#define OCD_REG_RM_TRCS_DCE					0x00000001 
#define OCD_REG_RM_TISR						0x00084820		// トレースデータ命令選別レジスタ//
	#define OCD_REG_RM_TISR_DEXS				0x00000080
	#define OCD_REG_RM_TISR_DFPS				0x00000040
	#define OCD_REG_RM_TISR_DBTS				0x00000020
	#define OCD_REG_RM_TISR_DLOS				0x00000010
	#define OCD_REG_RM_TISR_DARS				0x00000008
	#define OCD_REG_RM_TISR_DSRS				0x00000004
	#define OCD_REG_RM_TISR_DSTS				0x00000002
	#define OCD_REG_RM_TISR_DTRS				0x00000001
#define OCD_REG_RM_RSAR						0x00084840		// ROM空間先頭アドレス設定レジスタ//
#define OCD_REG_RM_REAR						0x00084844		// ROM空間終了アドレス設定レジスタ//
#define OCD_REG_RM_WSAR						0x00084848		// ウィンドウトレース用先頭アドレス設定レジスタ//
#define OCD_REG_RM_WEAR						0x0008484C		// ウィンドウトレース用終了アドレス設定レジスタ//
#define OCD_REG_RM_TSSR						0x00084818		// トレーストレース選択レジスタ//	// RevRxNo130301-001 Append Line
#define OCD_REG_RM_TBSR						0x00084810		// トレースバッファ設定レジスタ//
	#define OCD_REG_RM_TBSR_TRFS				0x000F0000
	#define OCD_REG_RM_TBSR_RWE					0x00001000
	#define OCD_REG_RM_TBSR_TMWM				0x00000100
	// RevRxNo130308-001 Append Line
	#define OCD_REG_RM_TBSR_LOF					0x00000040
	#define OCD_REG_RM_TBSR_OFF					0x00000020
	#define OCD_REG_RM_TBSR_TDOS				0x0000000C
	#define OCD_REG_RM_TBSR_TRPE				0x00000001
#define OCD_REG_RM_TBPA						0x00084870		// トレースバッファ出力ポインタレジスタ//
#define OCD_REG_RM_TBIP						0x00084874		// トレースバッファ入力ポインタレジスタ//
#define OCD_REG_RM_TSCR						0x00084824		// タイムスタンプカウントレジスタ//

// トレースバッファアドレス//
#define OCD_BUF_RM_TRC0_H					0x00085000
#define OCD_BUF_RM_TRC0_D1					0x00085004
#define OCD_BUF_RM_TRC0_D2					0x00085008
#define OCD_BUF_RM_TRC1_H					0x00085010
#define OCD_BUF_RM_TRC1_D1					0x00085014
#define OCD_BUF_RM_TRC1_D2					0x00085018
#define OCD_BUF_RM_TRC2_H					0x00085020
#define OCD_BUF_RM_TRC240_H					0x00085F00	// RevRxNo130301-001 Append Line
#define OCD_BUF_RM_TRC255_H					0x00085FF0
//RX220WS対応 Appned Start
#define OCD_BUF_RM_TRC32_H					0x000850F0

#define OCD_BUF_RM_TRC32_H_CLS2				0x000850F8
#define OCD_BUF_RM_TRC33_H_CLS2				0x00085100
#define OCD_BUF_RM_TRC64_H_CLS2				0x000851F8
#define OCD_BUF_RM_TRC65_H_CLS2				0x00085200
//RX220WS対応 Appned End

// パフォーマンス関連レジスタ//
#define OCD_REG_PPC_PCCSR0					0x00084310		// パフォーマンス計測条件設定レジスタ０//
#define OCD_REG_PPC_PCCSR1					0x00084314		// パフォーマンス計測条件設定レジスタ１//
	#define OCD_REG_PPC_PCCSR_COF				0x00000200		//PPCSR オーバーフローフラグ
	#define OCD_REG_PPC_PCCSR_FOC				0x00000010
	#define OCD_REG_PPC_PCCSR_PRC				0x00000008
	#define OCD_REG_PPC_PCCSR_CCE				0x00000004
	#define OCD_REG_PPC_PCCSR_COS				0x00000002
	#define OCD_REG_PPC_PCCSR_PPCE				0x00000001		//PPCSR パフォーマンスイネーブルフラグ
	#define OCD_REG_PPC_PCCSR_CIN				0x07FF0000		//PPCSR パフォーマンスイネーブルフラグ
#define OCD_REG_PPC_PCR0					0x00084220		// パフォーマンス計測レジスタ０//
#define OCD_REG_PPC_PCR1					0x00084224		// パフォーマンス計測レジスタ１//
#define OCD_REG_PPC_PAC						0x00084208		// パフォーマンス計測レジスタアクセスコントロールレジスタ//
// RevNo120131-001 Append Start
	#define OCD_REG_PPC_PAC_ESA0_EML				0x40000000
	#define OCD_REG_PPC_PAC_UA_EML					0x00000003
	#define OCD_REG_PPC_PAC_ESA0_USR				0x00000000
	#define OCD_REG_PPC_PAC_UA_USR					0x00000000
// RevNo120131-001 Append End
#define OCD_REG_PPC_EVENSR					0x00084200		// カウントイベント設定レジスタ//
	#define OCD_REG_PPC_EVENSR_PCE					0x000000FF	// 実行PCイベント設定ビット	// RevRxNo131101-006 Append Line

enum FFWENM_PPCSR_CIN {		// 全実行時間測定のクロックソース//
	PPC_NON = 0,					// 計測しない//
	PPC_CYC_PASS = 0x0A1,			// 時間計測 経過サイクル //
	PPC_CYC_PASS_SP,				// 時間計測 経過サイクル　特権モード//
	PPC_CYC_EXC_INT,				// 時間計測 例外・割り込みサイクル//
	PPC_CYC_EXC,					// 時間計測 例外処理サイクル//
	PPC_CYC_INT,					// 時間計測 割り込み処理サイクル//
	PPC_CNT_VALINST = 0x0C1,		// 回数計測 有効命令発行回数 //
	PPC_CNT_EXC_INT,				// 回数計測 例外・割り込み回数//
	PPC_CNT_EXC,					// 回数計測 例外処理回数//
	PPC_CNT_INT,					// 回数計測 割り込み処理回数//
	PPC_CNT_EVT						// 回数計測 イベントマッチ回数//
};

// RevRxNo140515-005 Append Start
#define MCU_FCLKICLK_120MHz		120.0		// MCU_TYPE_4 デフォルトICLK周波数(120MHz)
#define MCU_FCLKICLK_100MHz		100.0		// MCU_TYPE_0/1 デフォルトICLK周波数(100MHz)
#define MCU_FCLKICLK_50MHz		50.0		// MCU_TYPE_2 デフォルトICLK周波数(50MHz)
#define MCU_FCLKICLK_32MHz		32.0		// MCU_TYPE_3 デフォルトICLK周波数(32MHz)
// RevRxNo140515-005 Append End

#endif	// __OCDSPEC_H__
