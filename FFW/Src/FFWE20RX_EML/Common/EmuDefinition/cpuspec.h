///////////////////////////////////////////////////////////////////////////////
/**
 * @file cpuspec.h
 * @brief 制御CPU情報定義のヘッダファイル
 * @author RSD S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2014(2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/07
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/07 上田
	新規作成
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
・RevRxE2No170920-003 2017/09/20 大賀
	E2エミュレータ対応(定義追加)
・RevRxE1E20No180921-001 2018/09/21 大賀
	E1/E20用BFWのROM容量不足対策(PB18047-DD04-001)(定義追加)
*/
#ifndef	__CPUSPEC_H__
#define	__CPUSPEC_H__


// define定義
// E1, E20用定義
#define	BFWAREA_START_E20	0x00000000	// BFWコード領域開始アドレス
#define	BFWAREA_SIZE_E20	0x00040000	// BFWコード領域サイズ
#define	BFWLV0_START_E20	0x00000000	// BFWレベル0領域開始アドレス
#define	BFWLV0_SIZE_E20		0x00010000	// BFWレベル0領域サイズ
#define	BFWEML_START_E20	0x00010000	// BFW EML領域開始アドレス
#define	BFWEML_SIZE_E20		0x00020000	// BFW EML領域サイズ
// RevRxE1E20No180921-001 Append Start
#define	BFWEML_ENDCODE_E20	0x0002FFF8	// BFW EML対応エンドコード
#define	BFWEML_LV0_E20		0x00010018	// BFW EML対応LV0バージョン
#define	BFWEML_VERSION_E20	0x00010000	// BFW EMLバージョン
#define	BFWEML_MCU_E20		0x00010004	// BFW EMLターゲットMCU情報
#define	BFWEML_MCUSUB_E20	0x00010006	// BFW EMLターゲットMCUサブ情報
#define	BFWEML_MCUID_E20	0x00010008	// BFW EMLターゲットMCU ID情報
// RevRxE1E20No180921-001 Append End
#define	BFWPRG_START_E20	0x00030000	// BFW FDT/PRG領域開始アドレス
#define	BFWPRG_SIZE_E20		0x00010000	// BFW FDT/PRG領域サイズ
// RevRxE1E20No180921-001 Append Start
#define	BFWPRG_ENDCODE_E20	0x0003FFF8	// BFW FDT/PRG領域エンドコード
#define	BFWPRG_LV0_E20		0x0003FF94	// BFW FDT/PRG領域LV0バージョン
#define	BFWPRG_VERSION_E20	0x0003FF80	// BFW FDT/PRG領域バージョン
#define	BFWPRG_MCUID_E20	0x0003FF84	// BFW FDT/PRG領域ターゲットMCU ID情報
// RevRxE1E20No180921-001 Append End
#define	SELIDAREA_START_E20	0x00F00000	// 製品シリアル番号設定領域開始アドレス
#define	SELIDAREA_SIZE_E20	0x00001000	// 製品シリアル番号設定領域サイズ

// E2 Lite用定義
#define	BFWAREA_START_E2L	0xFFF80000	// BFWコード領域開始アドレス
#define	BFWAREA_SIZE_E2L	0x00080000	// BFWコード領域サイズ
#define	BFWLV0_START_E2L	0xFFFE8000	// BFWレベル0領域開始アドレス
#define	BFWLV0_SIZE_E2L		0x00018000	// BFWレベル0領域サイズ
#define	BFWEML_START_E2L	0xFFFA0000	// BFW EML領域開始アドレス
#define	BFWEML_VERSION_E2L	0xFFFA0000	// BFW EMLバージョンアドレス
#define	BFWEML_TARGETMCU_E2L 0xFFFA0004	// BFW EMLターゲット格納領域
#define	BFWEML_SIZE_E2L		0x00048000	// BFW EML領域サイズ
#define	BFWPRG_START_E2L	0xFFF88000	// BFW FDT/PRG領域開始アドレス
#define	BFWPRG_SIZE_E2L		0x00018000	// BFW FDT/PRG領域サイズ
#define	BFWCOM_START_E2L	0xFFF80000	// BFW COM領域開始アドレス
#define	BFWCOM_SIZE_E2L		0x00008000	// BFW COM領域サイズ
#define	SELIDAREA_START_E2L	0x00100000	// 製品シリアル番号設定領域開始アドレス
#define	SELIDAREA_SIZE_E2L	0x00000040	// 製品シリアル番号設定領域サイズ

// RevRxE2No170201-001 Append Start
// E2用定義
#define	BFWAREA_START_E2	0xFFF68000	// BFWコード領域開始アドレス
#define	BFWAREA_SIZE_E2		0x00098000	// BFWコード領域サイズ
#define	BFWLV0_START_E2		0xFFFE8000	// BFWレベル0領域開始アドレス
#define	BFWLV0_SIZE_E2		0x00018000	// BFWレベル0領域サイズ
#define	BFWEML_START_E2		0xFFF88000	// BFW EML領域開始アドレス
#define	BFWEML_VERSION_E2	0xFFF88000	// BFW EMLバージョンアドレス
#define	BFWEML_TARGETMCU_E2	0xFFF88004	// BFW EMLターゲット格納領域
#define	BFWEML_SIZE_E2		0x00060000	// BFW EML領域サイズ
#define	BFWPRG_START_E2		0xFFF68000	// BFW FDT/PRG領域開始アドレス
#define	BFWPRG_SIZE_E2		0x00020000	// BFW FDT/PRG領域サイズ
#define	BFWCOM_START_E2		0x00000000	// BFW COM領域開始アドレス
#define	BFWCOM_SIZE_E2		0x00000000	// BFW COM領域サイズ
#define	SELIDAREA_START_E2	0x00100000	// 製品シリアル番号設定領域開始アドレス
#define	SELIDAREA_SIZE_E2	0x00000040	// 製品シリアル番号設定領域サイズ
#define	NIDAREA_START_E2	0x00101800	// ニックネーム設定領域開始アドレス
#define	NIDAREA_SIZE_E2		0x00000140	// ニックネーム設定領域サイズ
#define	SECIDAREA_START_E2	0x00102000	// セキュリティキー設定領域開始アドレス
#define	SECIDAREA_SIZE_E2	0x00000400	// セキュリティキー設定領域サイズ
// RevRxE2No170201-001 Append End

#endif	// __CPUSPEC_H__
