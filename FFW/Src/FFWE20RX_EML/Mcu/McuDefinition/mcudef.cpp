///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcudef.cpp
 * @brief MCU依存のサブルーチンファイル
 * @author RSO Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, S.Ueda, Y.Miyake, K.Uemori, Y.Kawakami, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/02/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130411-001	2014/01/16 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130308-001	2013/05/20 三宅
　カバレッジ開発対応
・RevRxNo130301-001 2013/09/05 上田
	RX64M対応
・RevRxNo140109-001 2014/01/17 植盛、上田
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo130411-001 2014/01/21 植盛
	enum FFWRXENM_MDE_TYPE型のメンバ名変更(RX_MDE_OSM→RX_MDE_FLASH)
・RevRxNo140515-006 2014/09/09 川上
	RX231対応
・RevRxNo150827-005 2015/10/14 PA 紡車
	RX651対応
・RevRxNo151028-001 2015/10/28 南
	RX130 電圧監視リセット0対応
・RevRxNo150827-003 2015/12/01 PA 紡車
	RV40F Phase2対応
・RevRxNo160527-001 2016/06/10 PA 辻
	RX651 正式対応
・RevRxNo161003-001 2016/10/13 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
*/
#include "mcudef.h"
#include "mcuspec.h"
#include "ffwmcu_mcu.h"

// ファイル内static変数の宣言

// MCU固有情報管理構造体定義(初期化)
static FFWMCU_MCUDEF_DATA	s_McuDefData = {
	0xffffffff,					// MCUアドレス空間 - MCUアドレス最大値
	// RevRxNo130301-001 Append Line
	MCU_REG_FLASH_FENTRYR,		// フラッシュP/Eモードエントリレジスタアドレス
	// RevRxNo130411-001 Append Line
	MCU_DF_ENA_BLKNUM_RX610,	// データフラッシュリード許可レジスタの1ビットに対応するブロック数
	// RevRxNo130301-001 Append Line
	MCU_REG_FLASH_FCURAME,		// FCU RAMイネーブルレジスタ
	0x00000000,					// コンフィギュレーション設定領域ダウンロード対象開始アドレス	// RevRxNo140109-001 Append Line
	0x00000000,					// コンフィギュレーション設定領域ダウンロード対象終了アドレス	// RevRxNo140109-001 Append Line
	MCU_CONFIGAREA_RAM_NO_NON,	// コンフィギュレーション設定領域が定義されている内部RAMブロック番号
	MCU_OSM_OFS1_LVD_DIS_NON,	// 電圧検出リセット無効設定値				// RevRxNo140109-001 Append Line
	MCU_OSM_OFS1_LVD_MASK_NON,	// 電圧検出リセット有効/無効ビットマスク値	// RevRxNo140109-001 Append Line
	// RevRxNo150827-005 Append Start
	0x00000000,					// ExtraA領域キャッシュ開始アドレス
	0x00000000,					// ExtraA領域開始アドレスのオフセット値
	0x00000000,					// ExtraA領域サイズ
	0x00000000,					// MDEレジスタ開始アドレス
	0x00000000,					// OFS1レジスタ開始アドレス
	0x00000000,					// TMEFレジスタ開始アドレス
	0x00000000,					// TMINFレジスタ開始アドレス
	0x00000000,					// ExtraA領域ダウンロード不可開始アドレスオフセット値
	0x00000000,					// ExtraA領域ダウンロード不可終了アドレスオフセット値
	0x00000000,					// MDEレジスタ開始アドレスのオフセット値
	0x00000000,					// OFS1レジスタ開始アドレスのオフセット値
	0x00000000,					// OSISレジスタ(IDコード)開始アドレスのオフセット値
	0x00000000,					// OSISレジスタ(IDコード)終了アドレスのオフセット値
	0x00000000,					// TMINFレジスタ開始アドレスのオフセット値
	// RevRxNo161003-001 Append Start
	0x00000000,					// FAWレジスタ開始アドレスのオフセット値
	0x00000000,					// バンク選択レジスタ開始アドレスのオフセット値
	0x00000000					// デュアルモード時起動バンク側先頭アドレス
	// RevRxNo161003-001 Append End
	// RevRxNo150827-005 Append End
};


// RevRxNo130411-001 Append Start
// MC個別制御情報管理構造体定義(初期化)
static FFWMCU_FWCTRL_DATA	s_FwCtrlData = {
// MCU仕様
	RX_MDE_PIN,				// MDE設定種別
	FALSE,					// UBコード有無
	RX_OFS_NON,				// OFSレジスタ設定種別
	FALSE,					// オプション設定メモリ有無
	RX_IDCODE_FLASH,		// IDコード設定種別
	RX_FLASH_RC03F,			// フラッシュの種類
	FALSE,					// プロテクトレジスタ有無

// OCD仕様
	RX_OCD_CLS3,			// OCDデバッグI/Fのクラス
	FALSE,					// オペランドアクセスイベントのバス選択有無
	FALSE,					// トレースソース選択有無
	FALSE,					// トレースタイムスタンプカウンタ分周設定有無
	RX_TRCLK_CTRL_RX610,	// トレースクロック設定制御種別

// F/W制御仕様
	RX_LVD_NON_CHK,			// 電圧監視リセット有効設定エラーチェック種別
	RX_LVDERR_CODE_NON,		// エラーを返さない		// RevRxNo140109-001 Append Line
	FALSE,					// ユーザブートモードでのリセット時のデバッガ認証切れ発生有無
	FALSE,					// ユーザブートモードでのリセット時、FPGA, OCDレジスタ設定の実施有無
	// RevRxNo130301-001 Append Line
	FALSE,					// 外部トレース切り替え時のデバッグモジュールリセット実施有無
	// RevRxNo130308-001 Append Line
	FALSE,					// トレースの利用機能に"カバレッジ用に利用"を設定可能/不可
	FALSE,					// FAWレジスタ有無		// RevRxNo160527-001 Append Line
	// RevRxNo161003-001 Append Start
	FALSE,					// データフラッシュメモリアクセス周波数設定レジスタ有無
	FALSE,					// バンクモード選択ビット有無
	FALSE,					// バンク選択レジスタ有無
	FALSE					// ROMデータトレースアクセスタイプ出力ビット有無
	// RevRxNo161003-001 Append End
};
// RevRxNo130411-001 Append End

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
// RevRxNo130411-001 Append Start
//=============================================================================
/**
 * MCU種別に対応したMCU固有情報を設定する。
 * @param なし
 * @retval なし
 */
//=============================================================================
void SetMcuDefData(void)
{
	FFWMCU_MCUINFO_DATA_RX*	pMcuInfo;

	pMcuInfo = GetMcuInfoDataRX();

	switch (pMcuInfo->byMcuType) {
	case MCU_TYPE_0:	// RX610, RX62xの場合
		s_McuDefData.madrMaxAddr = 0xffffffff;			// MCUアドレス空間 - MCUアドレス最大値
		s_McuDefData.madrFentryr = MCU_REG_FLASH_FENTRYR;	// フラッシュP/Eモードエントリレジスタアドレス
		s_McuDefData.dwDataFlashEnaBlkNum = MCU_DF_ENA_BLKNUM_RX610;	// データフラッシュリード許可レジスタの1ビットに対応するブロック数
		s_McuDefData.madrFcurame = MCU_REG_FLASH_FCURAME;	// FCU RAMイネーブルレジスタ	RevRxNo130301-001 Append Line
		s_McuDefData.madrDwnpConfigStart = 0x00000000;		// コンフィギュレーション設定領域ダウンロード対象開始アドレス	// RevRxNo140109-001 Append Line
		s_McuDefData.madrDwnpConfigEnd = 0x00000000;		// コンフィギュレーション設定領域ダウンロード対象終了アドレス	// RevRxNo140109-001 Append Line
		s_McuDefData.dwConfigAreaRamNo = MCU_CONFIGAREA_RAM_NO_NON;	// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号	// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdDis = MCU_OSM_OFS1_LVD_DIS_NON;		// 電圧検出リセット無効設定値				// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdMask = MCU_OSM_OFS1_LVD_MASK_NON;		// 電圧検出リセット有効/無効ビットマスク値	// RevRxNo140109-001 Append Line
		// RevRxNo150827-005 Append Start
		s_McuDefData.madrExtraACacheStart = 0x00000000;		// ExtraA領域キャッシュ開始アドレス
		s_McuDefData.dwExtraAStartOffset = 0x00000000;		// ExtraA領域開始アドレスのオフセット値
		s_McuDefData.dwExtraASize = 0x00000000;				// ExtraA領域サイズ
		s_McuDefData.madrMdeStartAddr = 0x00000000;			// MDEレジスタ開始アドレス
		s_McuDefData.madrOfs1StartAddr = 0x00000000;		// OFS1レジスタ開始アドレス
		s_McuDefData.madrTmefStartAddr = 0x00000000;		// TMEFレジスタ開始アドレス
		s_McuDefData.madrTminfStartAddr = 0x00000000;		// TMINFレジスタ開始アドレス
		s_McuDefData.dwBanDwnpStartOffset = 0x00000000;		// ExtraA領域ダウンロード不可開始アドレスオフセット値
		s_McuDefData.dwBanDwnpEndOffset = 0x00000000;		// ExtraA領域ダウンロード不可終了アドレスオフセット値
		s_McuDefData.dwMdeStartOffset = 0x00000000;			// MDEレジスタ開始アドレスのオフセット値
		s_McuDefData.dwOfs1StartOffset = 0x00000000;		// OFS1レジスタ開始アドレスのオフセット値
		s_McuDefData.dwOsisStartOffset = 0x00000000;		// OSISレジスタ(IDコード)開始アドレスのオフセット値
		s_McuDefData.dwOsisEndOffset = 0x00000000;			// OSISレジスタ(IDコード)終了アドレスのオフセット値
		s_McuDefData.dwTminfStartOffset = 0x00000000;		// TMINFレジスタ開始アドレスのオフセット値
		s_McuDefData.dwFawStartOffset = 0x00000000;			// FAWレジスタ開始アドレスのオフセット値	// RevRxNo150827-003 Append Line
		// RevRxNo150827-005 Append End
		s_McuDefData.dwBankselStartOffset = 0x00000000;		// バンク選択レジスタ開始アドレスのオフセット値	// RevRxNo161003-001 Append Line
		s_McuDefData.dwDualStartupBankStartAddr = 0x00000000;	// デュアルモード時起動バンク側先頭アドレス	// RevRxNo161003-001 Append Line
		break;

	case MCU_TYPE_1:	// RX63xの場合
		s_McuDefData.madrMaxAddr = 0xffffffff;			// MCUアドレス空間 - MCUアドレス最大値
		s_McuDefData.madrFentryr = MCU_REG_FLASH_FENTRYR;	// フラッシュP/Eモードエントリレジスタアドレス
		s_McuDefData.dwDataFlashEnaBlkNum = MCU_DF_ENA_BLKNUM_RX630;	// データフラッシュリード許可レジスタの1ビットに対応するブロック数
		s_McuDefData.madrFcurame = MCU_REG_FLASH_FCURAME;	// FCU RAMイネーブルレジスタ	RevRxNo130301-001 Append Line
		s_McuDefData.madrDwnpConfigStart = 0x00000000;		// コンフィギュレーション設定領域ダウンロード対象開始アドレス	// RevRxNo140109-001 Append Line
		s_McuDefData.madrDwnpConfigEnd = 0x00000000;		// コンフィギュレーション設定領域ダウンロード対象終了アドレス	// RevRxNo140109-001 Append Line
		s_McuDefData.dwConfigAreaRamNo = MCU_CONFIGAREA_RAM_NO_NON;	// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号	// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdDis = MCU_OSM_OFS1_LVDAS_DIS_DATA;	// 電圧検出リセット無効設定値				// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdMask = MCU_OSM_OFS1_LVDAS_MASK_DATA;	// 電圧検出リセット有効/無効ビットマスク値	// RevRxNo140109-001 Append Line
		// RevRxNo150827-005 Append Start
		s_McuDefData.madrExtraACacheStart = 0x00000000;		// ExtraA領域キャッシュ開始アドレス
		s_McuDefData.dwExtraAStartOffset = 0x00000000;		// ExtraA領域開始アドレスのオフセット値
		s_McuDefData.dwExtraASize = 0x00000000;				// ExtraA領域サイズ
		s_McuDefData.madrMdeStartAddr = 0x00000000;			// MDEレジスタ開始アドレス
		s_McuDefData.madrOfs1StartAddr = 0x00000000;		// OFS1レジスタ開始アドレス
		s_McuDefData.madrTmefStartAddr = 0x00000000;		// TMEFレジスタ開始アドレス
		s_McuDefData.madrTminfStartAddr = 0x00000000;		// TMINFレジスタ開始アドレス
		s_McuDefData.dwBanDwnpStartOffset = 0x00000000;		// ExtraA領域ダウンロード不可開始アドレスオフセット値
		s_McuDefData.dwBanDwnpEndOffset = 0x00000000;		// ExtraA領域ダウンロード不可終了アドレスオフセット値
		s_McuDefData.dwMdeStartOffset = 0x00000000;			// MDEレジスタ開始アドレスのオフセット値
		s_McuDefData.dwOfs1StartOffset = 0x00000000;		// OFS1レジスタ開始アドレスのオフセット値
		s_McuDefData.dwOsisStartOffset = 0x00000000;		// OSISレジスタ(IDコード)開始アドレスのオフセット値
		s_McuDefData.dwOsisEndOffset = 0x00000000;			// OSISレジスタ(IDコード)終了アドレスのオフセット値
		s_McuDefData.dwTminfStartOffset = 0x00000000;		// TMINFレジスタ開始アドレスのオフセット値
		s_McuDefData.dwFawStartOffset = 0x00000000;			// FAWレジスタ開始アドレスのオフセット値	// RevRxNo150827-003 Append Line
		// RevRxNo150827-005 Append End
		s_McuDefData.dwBankselStartOffset = 0x00000000;		// バンク選択レジスタ開始アドレスのオフセット値	// RevRxNo161003-001 Append Line
		s_McuDefData.dwDualStartupBankStartAddr = 0x00000000;	// デュアルモード時起動バンク側先頭アドレス	// RevRxNo161003-001 Append Line
		break;

	case MCU_TYPE_2:	// RX200の場合
		s_McuDefData.madrMaxAddr = 0xffffffff;			// MCUアドレス空間 - MCUアドレス最大値
		s_McuDefData.madrFentryr = MCU_REG_FLASH_FENTRYR;	// フラッシュP/Eモードエントリレジスタアドレス
		// RevRxNo140515-006 Append Start
		if ((pMcuInfo->dwSpc[0] & SPC_FLASH_SC32_MF3) == SPC_FLASH_SC32_MF3) {	// データフラッシュリード許可レジスタの1ビットに対応するブロック数
			s_McuDefData.dwDataFlashEnaBlkNum = MCU_DF_ENA_BLKNUM_RX230;	// SC32_MF3用
		} else {
			// RevRxNo140515-006 Append End
			s_McuDefData.dwDataFlashEnaBlkNum = MCU_DF_ENA_BLKNUM_RX200;	// RC03F用
		}
		s_McuDefData.madrFcurame = MCU_REG_FLASH_FCURAME;	// FCU RAMイネーブルレジスタ	RevRxNo130301-001 Append Line
		s_McuDefData.madrDwnpConfigStart = 0x00000000;		// コンフィギュレーション設定領域ダウンロード対象開始アドレス	// RevRxNo140109-001 Append Line
		s_McuDefData.madrDwnpConfigEnd = 0x00000000;		// コンフィギュレーション設定領域ダウンロード対象終了アドレス	// RevRxNo140109-001 Append Line
		s_McuDefData.dwConfigAreaRamNo = MCU_CONFIGAREA_RAM_NO_NON;	// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号	// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdDis = MCU_OSM_OFS1_LVDAS_DIS_DATA;	// 電圧検出リセット無効設定値				// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdMask = MCU_OSM_OFS1_LVDAS_MASK_DATA;	// 電圧検出リセット有効/無効ビットマスク値	// RevRxNo140109-001 Append Line
		// RevRxNo150827-005 Append Start
		s_McuDefData.madrExtraACacheStart = 0x00000000;		// ExtraA領域キャッシュ開始アドレス
		s_McuDefData.dwExtraAStartOffset = 0x00000000;		// ExtraA領域開始アドレスのオフセット値
		s_McuDefData.dwExtraASize = 0x00000000;				// ExtraA領域サイズ
		s_McuDefData.madrMdeStartAddr = 0x00000000;			// MDEレジスタ開始アドレス
		s_McuDefData.madrOfs1StartAddr = 0x00000000;		// OFS1レジスタ開始アドレス
		s_McuDefData.madrTmefStartAddr = 0x00000000;		// TMEFレジスタ開始アドレス
		s_McuDefData.madrTminfStartAddr = 0x00000000;		// TMINFレジスタ開始アドレス
		s_McuDefData.dwBanDwnpStartOffset = 0x00000000;		// ExtraA領域ダウンロード不可開始アドレスオフセット値
		s_McuDefData.dwBanDwnpEndOffset = 0x00000000;		// ExtraA領域ダウンロード不可終了アドレスオフセット値
		s_McuDefData.dwMdeStartOffset = 0x00000000;			// MDEレジスタ開始アドレスのオフセット値
		s_McuDefData.dwOfs1StartOffset = 0x00000000;		// OFS1レジスタ開始アドレスのオフセット値
		s_McuDefData.dwOsisStartOffset = 0x00000000;		// OSISレジスタ(IDコード)開始アドレスのオフセット値
		s_McuDefData.dwOsisEndOffset = 0x00000000;			// OSISレジスタ(IDコード)終了アドレスのオフセット値
		s_McuDefData.dwTminfStartOffset = 0x00000000;		// TMINFレジスタ開始アドレスのオフセット値
		s_McuDefData.dwFawStartOffset = 0x00000000;			// FAWレジスタ開始アドレスのオフセット値	// RevRxNo150827-003 Append Line
		// RevRxNo150827-005 Append End
		s_McuDefData.dwBankselStartOffset = 0x00000000;		// バンク選択レジスタ開始アドレスのオフセット値	// RevRxNo161003-001 Append Line
		s_McuDefData.dwDualStartupBankStartAddr = 0x00000000;	// デュアルモード時起動バンク側先頭アドレス	// RevRxNo161003-001 Append Line
		break;

	case MCU_TYPE_3:	// RX100の場合
		s_McuDefData.madrMaxAddr = 0xffffffff;			// MCUアドレス空間 - MCUアドレス最大値
		s_McuDefData.madrFentryr = MCU_REG_FLASH_FENTRYR;	// フラッシュP/Eモードエントリレジスタアドレス
		s_McuDefData.dwDataFlashEnaBlkNum = MCU_DF_ENA_BLKNUM_RX100;	// データフラッシュリード許可レジスタの1ビットに対応するブロック数
		s_McuDefData.madrFcurame = MCU_REG_FLASH_FCURAME;	// FCU RAMイネーブルレジスタ	RevRxNo130301-001 Append Line
		s_McuDefData.madrDwnpConfigStart = 0x00000000;		// コンフィギュレーション設定領域ダウンロード対象開始アドレス	// RevRxNo140109-001 Append Line
		s_McuDefData.madrDwnpConfigEnd = 0x00000000;		// コンフィギュレーション設定領域ダウンロード対象終了アドレス	// RevRxNo140109-001 Append Line
		s_McuDefData.dwConfigAreaRamNo = MCU_CONFIGAREA_RAM_NO_NON;	// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号	// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdDis = MCU_OSM_OFS1_STUPLVD1REN_MASK_DATA;	// 電圧検出リセット無効設定値				// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdMask = MCU_OSM_OFS1_STUPLVD1REN_DIS_DATA;	// 電圧検出リセット有効/無効ビットマスク値	// RevRxNo140109-001 Append Line
		// RevRxNo150827-005 Append Start
		s_McuDefData.madrExtraACacheStart = 0x00000000;		// ExtraA領域キャッシュ開始アドレス
		s_McuDefData.dwExtraAStartOffset = 0x00000000;		// ExtraA領域開始アドレスのオフセット値
		s_McuDefData.dwExtraASize = 0x00000000;				// ExtraA領域サイズ
		s_McuDefData.madrMdeStartAddr = 0x00000000;			// MDEレジスタ開始アドレス
		s_McuDefData.madrOfs1StartAddr = 0x00000000;		// OFS1レジスタ開始アドレス
		s_McuDefData.madrTmefStartAddr = 0x00000000;		// TMEFレジスタ開始アドレス
		s_McuDefData.madrTminfStartAddr = 0x00000000;		// TMINFレジスタ開始アドレス
		s_McuDefData.dwBanDwnpStartOffset = 0x00000000;		// ExtraA領域ダウンロード不可開始アドレスオフセット値
		s_McuDefData.dwBanDwnpEndOffset = 0x00000000;		// ExtraA領域ダウンロード不可終了アドレスオフセット値
		s_McuDefData.dwMdeStartOffset = 0x00000000;			// MDEレジスタ開始アドレスのオフセット値
		s_McuDefData.dwOfs1StartOffset = 0x00000000;		// OFS1レジスタ開始アドレスのオフセット値
		s_McuDefData.dwOsisStartOffset = 0x00000000;		// OSISレジスタ(IDコード)開始アドレスのオフセット値
		s_McuDefData.dwOsisEndOffset = 0x00000000;			// OSISレジスタ(IDコード)終了アドレスのオフセット値
		s_McuDefData.dwTminfStartOffset = 0x00000000;		// TMINFレジスタ開始アドレスのオフセット値
		s_McuDefData.dwFawStartOffset = 0x00000000;			// FAWレジスタ開始アドレスのオフセット値	// RevRxNo150827-003 Append Line
		// RevRxNo150827-005 Append End
		s_McuDefData.dwBankselStartOffset = 0x00000000;		// バンク選択レジスタ開始アドレスのオフセット値	// RevRxNo161003-001 Append Line
		s_McuDefData.dwDualStartupBankStartAddr = 0x00000000;	// デュアルモード時起動バンク側先頭アドレス	// RevRxNo161003-001 Append Line
		break;

	case MCU_TYPE_4:	// RX64xの場合
		s_McuDefData.madrMaxAddr = 0xffffffff;							// MCUアドレス空間 - MCUアドレス最大値
		s_McuDefData.madrFentryr = MCU_REG_FLASH_FENTRYR_RX640;			// フラッシュP/Eモードエントリレジスタアドレス
		s_McuDefData.dwDataFlashEnaBlkNum = MCU_DF_ENA_BLKNUM_RX640;	// データフラッシュリード許可レジスタの1ビットに対応するブロック数(なし)
		// RevRxNo150827-005 Modify Start
		if ((pMcuInfo->dwSpc[0] & SPC_FLASH_TYPE_MASK) == SPC_FLASH_RV40F2) {	// RX65x(RV40F Phase2)の場合
			s_McuDefData.madrFcurame = 0x00000000;							// FCU RAMイネーブルレジスタ
			s_McuDefData.madrDwnpConfigStart = MCU_EXTRA_A_START_RX650;		// コンフィギュレーション設定領域ダウンロード対象開始アドレス
			s_McuDefData.madrDwnpConfigEnd = MCU_EXTRA_A_END_RX650;			// コンフィギュレーション設定領域ダウンロード対象終了アドレス
			s_McuDefData.dwConfigAreaRamNo = MCU_CONFIGAREA_RAM_NO_RX650;	// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号
			s_McuDefData.madrExtraACacheStart = MCU_EXTRA_A_CACHE_START_RX650;	// ExtraA領域キャッシュ開始アドレス
			s_McuDefData.dwExtraAStartOffset = MCU_EXTRA_A_AREA_START_RX650;	// ExtraA領域開始アドレスのオフセット値
			s_McuDefData.dwExtraASize = MCU_EXTRA_A_AREA_SIZE_RX650;			// ExtraA領域サイズ
			s_McuDefData.madrMdeStartAddr = MCU_OSM_MDE_START_RX650;		// MDEレジスタ開始アドレス
			s_McuDefData.madrOfs1StartAddr = MCU_OSM_OFS1_START_RX650;		// OFS1レジスタ開始アドレス
			s_McuDefData.madrTmefStartAddr = MCU_OSM_TMEF_START_RX650;		// TMEFレジスタ開始アドレス
			s_McuDefData.madrTminfStartAddr = MCU_OSM_TMINF_START_RX650;	// TMINFレジスタ開始アドレス
			s_McuDefData.dwBanDwnpEndOffset = MCU_EXTRA_A_BAN_END_RX650;		// ExtraA領域ダウンロード不可終了アドレスオフセット値
			s_McuDefData.dwMdeStartOffset = MCU_EXTRA_A_MDE_START_RX650;	// MDEレジスタ開始アドレスのオフセット値
			s_McuDefData.dwOfs1StartOffset = MCU_EXTRA_A_OFS1_START_RX650;	// OFS1レジスタ開始アドレスのオフセット値
			s_McuDefData.dwTminfStartOffset = MCU_EXTRA_A_TMINF_START_RX650;	// TMINFレジスタ開始アドレスのオフセット値
			s_McuDefData.dwFawStartOffset = MCU_EXTRA_A_FAW_START_RX650;	// FAWレジスタ開始アドレスのオフセット値	// RevRxNo150827-003 Append Line
		} else {															// RX65x以外(RV40F)の場合
			s_McuDefData.madrFcurame = MCU_REG_FLASH_FCURAME_RX640;			// FCU RAMイネーブルレジスタ	RevRxNo130301-001 Append Line
			s_McuDefData.madrDwnpConfigStart = MCU_OSM_OSIS_START_RX640;	// コンフィギュレーション設定領域ダウンロード対象開始アドレス	// RevRxNo140109-001 Append Line
			// RevRxNo180228-001 Modify Start
			if ((pMcuInfo->dwSpc[2] & SPC_ROMCODE_PROTECT) == SPC_ROMCODE_PROTECT) {	// ROMコードプロテクト配置
				// ROMコードプロテクトレジスタがオプション設定メモリ上にある
				s_McuDefData.madrDwnpConfigEnd = MCU_OSM_OFS1_END_RX66T;		// コンフィギュレーション設定領域ダウンロード対象終了アドレス
			} else {
				// ROMコードプロテクトレジスタがプログラムROM上、または存在しない
				s_McuDefData.madrDwnpConfigEnd = MCU_OSM_OFS1_END_RX640;		// コンフィギュレーション設定領域ダウンロード対象終了アドレス	// RevRxNo140109-001 Append Line
			}
			// RevRxNo180228-001 Modify End
			s_McuDefData.dwConfigAreaRamNo = MCU_CONFIGAREA_RAM_NO_RX640;	// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号	// RevRxNo140109-001 Append Line
			s_McuDefData.madrExtraACacheStart = MCU_EXTRA_A_CACHE_START_RX640;	// ExtraA領域キャッシュ開始アドレス
			s_McuDefData.dwExtraAStartOffset = MCU_EXTRA_A_AREA_START_RX640;	// ExtraA領域開始アドレスのオフセット値
			s_McuDefData.dwExtraASize = MCU_EXTRA_A_AREA_SIZE_RX640;			// ExtraA領域サイズ
			s_McuDefData.madrMdeStartAddr = MCU_OSM_MDE_START_RX640;		// MDEレジスタ開始アドレス
			s_McuDefData.madrOfs1StartAddr = MCU_OSM_OFS1_START_RX640;		// OFS1レジスタ開始アドレス
			s_McuDefData.madrTmefStartAddr = MCU_OSM_TMEF_START_RX640;		// TMEFレジスタ開始アドレス
			s_McuDefData.madrTminfStartAddr = MCU_OSM_TMINF_START_RX640;	// TMINFレジスタ開始アドレス
			s_McuDefData.dwBanDwnpEndOffset = 0x00000000;					// ExtraA領域ダウンロード不可終了アドレスオフセット値
			s_McuDefData.dwMdeStartOffset = MCU_EXTRA_A_MDE_START_RX640;	// MDEレジスタ開始アドレスのオフセット値
			s_McuDefData.dwOfs1StartOffset = MCU_EXTRA_A_OFS1_START_RX640;	// OFS1レジスタ開始アドレスのオフセット値
			s_McuDefData.dwTminfStartOffset = MCU_EXTRA_A_TMINF_START_RX640;	// TMINFレジスタ開始アドレスのオフセット値
			s_McuDefData.dwFawStartOffset = 0x00000000;						// FAWレジスタ開始アドレスのオフセット値	// RevRxNo150827-003 Append Line
		}
		// RevRxNo150827-005 Modify End
		s_McuDefData.dwOsisStartOffset = MCU_EXTRA_A_IDCODE_START;		// OSISレジスタ(IDコード)開始アドレスのオフセット値
		s_McuDefData.dwOsisEndOffset = MCU_EXTRA_A_IDCODE_END;			// OSISレジスタ(IDコード)終了アドレスのオフセット値
		s_McuDefData.dwOfs1LvdDis = MCU_OSM_OFS1_LVDAS_DIS_DATA;		// 電圧検出リセット無効設定値				// RevRxNo140109-001 Append Line
		s_McuDefData.dwOfs1LvdMask = MCU_OSM_OFS1_LVDAS_MASK_DATA;		// 電圧検出リセット有効/無効ビットマスク値	// RevRxNo140109-001 Append Line
		// RevRxNo161003-001 Append Start
		if ((pMcuInfo->dwSpc[0] & SPC_BANKSEL_EXIST) == SPC_BANKSEL_EXIST) {	// BANKSELレジスタがあるMCUの場合
			s_McuDefData.dwBanDwnpStartOffset = MCU_EXTRA_A_BAN_START_RX650_2MB;	// ダウンロード禁止領域開始アドレスのオフセット値
			s_McuDefData.dwBankselStartOffset = MCU_EXTRA_A_BANKSEL_START;			// バンク選択レジスタ開始アドレスのオフセット値
			s_McuDefData.dwDualStartupBankStartAddr = MCU_DUAL_STARTUP_BANK_START;	// デュアルモード時起動バンク側先頭アドレス
		} else {
			if ((pMcuInfo->dwSpc[0] & SPC_FLASH_TYPE_MASK) == SPC_FLASH_RV40F2) {	// RX65x(RV40F Phase2)の場合
				s_McuDefData.dwBanDwnpStartOffset = MCU_EXTRA_A_BAN_START_RX650;	// ダウンロード禁止領域開始アドレスのオフセット値
			} else {
				s_McuDefData.dwBanDwnpStartOffset = 0x00000000;						// ダウンロード禁止領域開始アドレスのオフセット値
			}
			s_McuDefData.dwBankselStartOffset = 0x00000000;							// バンク選択レジスタ開始アドレスのオフセット値
			s_McuDefData.dwDualStartupBankStartAddr = 0x00000000;					// デュアルモード時起動バンク側先頭アドレス
		}
		// RevRxNo161003-001 Append End
		break;

	default:	// ここは通らない
		break;
	}

	return;
}
// RevRxNo130411-001 Append End

//=============================================================================
/**
 * FFW内で定義しているMCU固有情報を構造体のポインタとして返送する。
 * @param なし
 * @retval MCU固有情報管理構造体へのポインタ
 */
//=============================================================================
FFWMCU_MCUDEF_DATA* GetMcuDefData(void)
{
	return	&s_McuDefData;
}


// RevRxNo130411-001 Append Start
//=============================================================================
/**
 * MCU種別に対応したMCU個別制御情報を設定する。
 * @param なし
 * @return なし
 */
//=============================================================================
void SetFwCtrlData(void)
{
	FFWMCU_MCUINFO_DATA_RX*	pMcuInfo;

	pMcuInfo = GetMcuInfoDataRX();

	switch (pMcuInfo->byMcuType) {
	case MCU_TYPE_0:	// RX610, RX62xの場合
		s_FwCtrlData.eMdeType = RX_MDE_PIN;				// MDE端子設定
		s_FwCtrlData.bUbcodeExist = FALSE;				// UBコードなし
		s_FwCtrlData.eOfsType = RX_OFS_NON;				// OFSレジスタなし
		s_FwCtrlData.bOsmExist = FALSE;					// オプション設定メモリなし
		s_FwCtrlData.eIdcodeType = RX_IDCODE_FLASH;		// IDコードはフラッシュROMで設定
		s_FwCtrlData.eFlashType = RX_FLASH_RC03F;		// RC03F
		s_FwCtrlData.bProtRegExist = FALSE;				// プロテクトレジスタなし
		s_FwCtrlData.eOcdCls = RX_OCD_CLS3;				// Class 3
		s_FwCtrlData.bEvOaBusSelEna = FALSE;			// オペランドアクセスイベントのバス選択なし
		s_FwCtrlData.bTraSrcSelEna = FALSE;				// トレースソース選択なし
		s_FwCtrlData.bTraTimeDivEna = FALSE;			// トレースタイムスタンプカウンタ分周設定なし
		s_FwCtrlData.eTrcClkCtrl = RX_TRCLK_CTRL_RX610;	// RX610,RX62x用トレースクロック設定処理
		s_FwCtrlData.eLvdErrType = RX_LVD_NON_CHK;		// LVDxのエラーチェックを実施しない
		s_FwCtrlData.eLvdErrCodeType = RX_LVDERR_CODE_NON;	// エラーを返さない		// RevRxNo140109-001 Append Line
		s_FwCtrlData.bAuthClrUsrBootReset = FALSE;		// ユーザブートモードでのリセット時にデバッガ認証が切れない
		s_FwCtrlData.bSetFpgaOcdUsrBootReset = FALSE;	// ユーザブートモードでのリセット時にFPGA, OCDレジスタを設定しない
		s_FwCtrlData.bResetDebugModuleExtTrc = FALSE;	// 外部トレース切り替え時のデバッグモジュールリセット実施有無	// RevRxNo130301-001 Append Line
		// RevRxNo130308-001 Append Line
		s_FwCtrlData.bCoverageFunc = FALSE;				// トレースの利用機能に"カバレッジ用に利用"を設定不可
		s_FwCtrlData.bFawExist = FALSE;					// FAWレジスタなし	// RevRxNo160527-001 Append Line
		// RevRxNo161003-001 Append Start
		s_FwCtrlData.bEepfclkExist = FALSE;					// データフラッシュメモリアクセス周波数設定レジスタ(EEPFCLK)なし
		s_FwCtrlData.bBankmdExist = FALSE;					// バンクモード選択ビット(MDE.BANKMD)なし
		s_FwCtrlData.bBankselExist = FALSE;					// バンク選択レジスタ(BANKSEL)なし
		s_FwCtrlData.bRomDataTrcAccessTypeExist = FALSE;	// ROMデータトレースアクセスタイプ出力ビット(TRCS.RODTE)なし
		// RevRxNo161003-001 Append End
		break;

	case MCU_TYPE_1:	// RX63xの場合
		s_FwCtrlData.eMdeType = RX_MDE_FLASH;			// MDEはフラッシュROMで設定		// RevRxNo130411-001 Modify Line
		s_FwCtrlData.bUbcodeExist = TRUE;				// UBコードあり
		s_FwCtrlData.eOfsType = RX_OFS_FLASH;			// OFSレジスタはフラッシュROMで設定
		s_FwCtrlData.bOsmExist = TRUE;					// オプション設定メモリあり
		s_FwCtrlData.eIdcodeType = RX_IDCODE_FLASH;		// IDコードはフラッシュROMで設定
		s_FwCtrlData.eFlashType = RX_FLASH_RC03F;		// RC03F
		s_FwCtrlData.bProtRegExist = TRUE;				// プロテクトレジスタあり
		s_FwCtrlData.eOcdCls = RX_OCD_CLS3;				// Class 3
		s_FwCtrlData.bEvOaBusSelEna = FALSE;			// オペランドアクセスイベントのバス選択なし
		s_FwCtrlData.bTraSrcSelEna = FALSE;				// トレースソース選択なし
		s_FwCtrlData.bTraTimeDivEna = FALSE;			// トレースタイムスタンプカウンタ分周設定なし
		s_FwCtrlData.eTrcClkCtrl = RX_TRCLK_CTRL_RX630;	// RX63x,RX64x用トレースクロック設定処理
		s_FwCtrlData.eLvdErrType = RX_LVD_LEVEL_CHK;	// LVDx有効時の電圧レベルチェックを実施する
		s_FwCtrlData.eLvdErrCodeType = RX_LVDERR_CODE_NON;	// エラーを返さない		// RevRxNo140109-001 Append Line
		s_FwCtrlData.bAuthClrUsrBootReset = TRUE;		// ユーザブートモードでのリセット時にデバッガ認証が切れる
		s_FwCtrlData.bSetFpgaOcdUsrBootReset = TRUE;	// ユーザブートモードでのリセット時にFPGA, OCDレジスタを設定する
		s_FwCtrlData.bResetDebugModuleExtTrc = FALSE;	// 外部トレース切り替え時のデバッグモジュールリセット実施有無	// RevRxNo130301-001 Append Line
		// RevRxNo130308-001 Append Line
		s_FwCtrlData.bCoverageFunc = FALSE;				// トレースの利用機能に"カバレッジ用に利用"を設定不可
		s_FwCtrlData.bFawExist = FALSE;					// FAWレジスタなし	// RevRxNo160527-001 Append Line
		// RevRxNo161003-001 Append Start
		s_FwCtrlData.bEepfclkExist = FALSE;					// データフラッシュメモリアクセス周波数設定レジスタ(EEPFCLK)なし
		s_FwCtrlData.bBankmdExist = FALSE;					// バンクモード選択ビット(MDE.BANKMD)なし
		s_FwCtrlData.bBankselExist = FALSE;					// バンク選択レジスタ(BANKSEL)なし
		s_FwCtrlData.bRomDataTrcAccessTypeExist = FALSE;	// ROMデータトレースアクセスタイプ出力ビット(TRCS.RODTE)なし
		// RevRxNo161003-001 Append End
		break;

	case MCU_TYPE_2:	// RX200の場合
		s_FwCtrlData.eMdeType = RX_MDE_FLASH;			// MDEはフラッシュROMで設定		// RevRxNo130411-001 Modify Line
		s_FwCtrlData.bUbcodeExist = TRUE;				// UBコードあり
		s_FwCtrlData.eOfsType = RX_OFS_FLASH;			// OFSレジスタはフラッシュROMで設定
		s_FwCtrlData.bOsmExist = TRUE;					// オプション設定メモリあり
		s_FwCtrlData.eIdcodeType = RX_IDCODE_FLASH;		// IDコードはフラッシュROMで設定
		// RevRxNo140515-006 Append Start
		if ((pMcuInfo->dwSpc[0] & SPC_FLASH_SC32_MF3) == SPC_FLASH_SC32_MF3) {
			s_FwCtrlData.eFlashType = RX_FLASH_SC32_MF3;	// SC32用MF3
		} else {
			s_FwCtrlData.eFlashType = RX_FLASH_RC03F;		// RC03F
		}
		// RevRxNo140515-006 Append End
		s_FwCtrlData.bProtRegExist = TRUE;				// プロテクトレジスタあり
		s_FwCtrlData.eOcdCls = RX_OCD_CLS2;				// Class 2
		s_FwCtrlData.bEvOaBusSelEna = FALSE;			// オペランドアクセスイベントのバス選択なし
		s_FwCtrlData.bTraSrcSelEna = FALSE;				// トレースソース選択なし
		s_FwCtrlData.bTraTimeDivEna = FALSE;			// トレースタイムスタンプカウンタ分周設定なし
		s_FwCtrlData.eTrcClkCtrl = RX_TRCLK_CTRL_RX200;	// RX200,RX100用トレースクロック設定処理
		s_FwCtrlData.eLvdErrType = RX_LVD_LEVEL_CHK;	// LVDx有効時の電圧レベルチェックを実施する
		s_FwCtrlData.eLvdErrCodeType = RX_LVDERR_CODE_NON;	// エラーを返さない		// RevRxNo140109-001 Append Line
		s_FwCtrlData.bAuthClrUsrBootReset = TRUE;		// ユーザブートモードでのリセット時にデバッガ認証が切れる
		s_FwCtrlData.bSetFpgaOcdUsrBootReset = TRUE;	// ユーザブートモードでのリセット時にFPGA, OCDレジスタを設定する
		s_FwCtrlData.bResetDebugModuleExtTrc = FALSE;	// 外部トレース切り替え時のデバッグモジュールリセット実施有無	// RevRxNo130301-001 Append Line
		// RevRxNo130308-001 Append Line
		s_FwCtrlData.bCoverageFunc = FALSE;				// トレースの利用機能に"カバレッジ用に利用"を設定不可
		s_FwCtrlData.bFawExist = FALSE;					// FAWレジスタなし	// RevRxNo160527-001 Append Line
		// RevRxNo161003-001 Append Start
		s_FwCtrlData.bEepfclkExist = FALSE;					// データフラッシュメモリアクセス周波数設定レジスタ(EEPFCLK)なし
		s_FwCtrlData.bBankmdExist = FALSE;					// バンクモード選択ビット(MDE.BANKMD)なし
		s_FwCtrlData.bBankselExist = FALSE;					// バンク選択レジスタ(BANKSEL)なし
		s_FwCtrlData.bRomDataTrcAccessTypeExist = FALSE;	// ROMデータトレースアクセスタイプ出力ビット(TRCS.RODTE)なし
		// RevRxNo161003-001 Append End
		break;

	case MCU_TYPE_3:	// RX100の場合
		s_FwCtrlData.eMdeType = RX_MDE_FLASH;			// MDEはフラッシュROMで設定		// RevRxNo130411-001 Modify Line
		s_FwCtrlData.bUbcodeExist = FALSE;				// UBコードなし
		s_FwCtrlData.eOfsType = RX_OFS_FLASH;			// OFSレジスタはフラッシュROMで設定
		s_FwCtrlData.bOsmExist = TRUE;					// オプション設定メモリあり
		s_FwCtrlData.eIdcodeType = RX_IDCODE_FLASH;		// IDコードはフラッシュROMで設定
		s_FwCtrlData.eFlashType = RX_FLASH_MF3;			// MF3
		s_FwCtrlData.bProtRegExist = TRUE;				// プロテクトレジスタあり
		s_FwCtrlData.eOcdCls = RX_OCD_CLS2;				// Class 2
		s_FwCtrlData.bEvOaBusSelEna = FALSE;			// オペランドアクセスイベントのバス選択なし
		s_FwCtrlData.bTraSrcSelEna = FALSE;				// トレースソース選択なし
		s_FwCtrlData.bTraTimeDivEna = FALSE;			// トレースタイムスタンプカウンタ分周設定なし
		s_FwCtrlData.eTrcClkCtrl = RX_TRCLK_CTRL_RX200;	// RX200,RX100用トレースクロック設定処理
		// RevRxNo151028-001 Append Start
		//  RX130 電圧監視リセット0対応
		if ((pMcuInfo->dwSpc[1] & SPC_RX100_LVD_LEVEL_CHK) == SPC_RX100_LVD_LEVEL_CHK) {
			// 現状、このif文内処理はRX130のみ
			s_FwCtrlData.eLvdErrType = RX_LVD_LEVEL_CHK;		// LVDx有効時の電圧レベルチェックを実施する
			s_FwCtrlData.eLvdErrCodeType = RX_LVDERR_CODE_NON;	// エラーを返さない
		} else {
			//RX100シリーズでRX130以外
			s_FwCtrlData.eLvdErrType = RX_LVD_ENA_CHK;		// LVDx有効設定チェックを実施する
			s_FwCtrlData.eLvdErrCodeType = RX_LVDERR_CODE_LVD1REN;	// FFWERR_WRITE_OFS1_LVD1REN_CHANGEを返す		// RevRxNo140109-001 Append Line
		}
		// RevRxNo151028-001 Append End
		s_FwCtrlData.bAuthClrUsrBootReset = FALSE;		// ユーザブートモードでのリセット時にデバッガ認証が切れない
		s_FwCtrlData.bSetFpgaOcdUsrBootReset = FALSE;	// ユーザブートモードでのリセット時にFPGA, OCDレジスタを設定しない
		s_FwCtrlData.bResetDebugModuleExtTrc = FALSE;	// 外部トレース切り替え時のデバッグモジュールリセット実施有無	// RevRxNo130301-001 Append Line
		// RevRxNo130308-001 Append Line
		s_FwCtrlData.bCoverageFunc = FALSE;				// トレースの利用機能に"カバレッジ用に利用"を設定不可
		s_FwCtrlData.bFawExist = FALSE;					// FAWレジスタなし	// RevRxNo160527-001 Append Line
		// RevRxNo161003-001 Append Start
		s_FwCtrlData.bEepfclkExist = FALSE;					// データフラッシュメモリアクセス周波数設定レジスタ(EEPFCLK)なし
		s_FwCtrlData.bBankmdExist = FALSE;					// バンクモード選択ビット(MDE.BANKMD)なし
		s_FwCtrlData.bBankselExist = FALSE;					// バンク選択レジスタ(BANKSEL)なし
		s_FwCtrlData.bRomDataTrcAccessTypeExist = FALSE;	// ROMデータトレースアクセスタイプ出力ビット(TRCS.RODTE)なし
		// RevRxNo161003-001 Append End
		break;

	case MCU_TYPE_4:	// RX64xの場合
		s_FwCtrlData.eMdeType = RX_MDE_EXTRA;			// MDEはExtra領域で設定
		// RevRxNo150827-005 Modify Start
		if ((pMcuInfo->dwSpc[0] & SPC_FLASH_TYPE_MASK) == SPC_FLASH_RV40F2) {	// RX65x(RV40F Phase2)の場合
			s_FwCtrlData.bUbcodeExist = FALSE;				// UBコードなし
			s_FwCtrlData.eFlashType = RX_FLASH_RV40F;		// RV40F(FFWではRV40FとRV40F Phase2で判断を変更する必要なし)
			s_FwCtrlData.bFawExist = TRUE;					// FAWレジスタあり	// RevRxNo160527-001 Append Line
		} else {															// RX65x以外(RV40F)の場合
			s_FwCtrlData.bUbcodeExist = TRUE;				// UBコードあり
			s_FwCtrlData.eFlashType = RX_FLASH_RV40F;		// RV40F
			s_FwCtrlData.bFawExist = FALSE;					// FAWレジスタなし	// RevRxNo160527-001 Append Line
		}
		// RevRxNo150827-005 Modify End
		s_FwCtrlData.eOfsType = RX_OFS_EXTRA;			// OFSレジスタはExtra領域で設定
		s_FwCtrlData.bOsmExist = TRUE;					// オプション設定メモリあり
		s_FwCtrlData.eIdcodeType = RX_IDCODE_EXTRA;		// IDコードはExtra領域で設定
		s_FwCtrlData.bProtRegExist = TRUE;				// プロテクトレジスタあり
		s_FwCtrlData.eOcdCls = RX_OCD_CLS3;				// Class 3
		s_FwCtrlData.bEvOaBusSelEna = TRUE;				// オペランドアクセスイベントのバス選択あり
		s_FwCtrlData.bTraSrcSelEna = TRUE;				// トレースソース選択あり
		s_FwCtrlData.bTraTimeDivEna = TRUE;				// トレースタイムスタンプカウンタ分周設定あり
		s_FwCtrlData.eTrcClkCtrl = RX_TRCLK_CTRL_RX630;	// RX63x,RX64x用トレースクロック設定処理
		// RevRxNo180228-001 Modify Start
		//  RX66x 電圧監視リセット0対応
		if ((pMcuInfo->dwSpc[1] & SPC_RX100_LVD_LEVEL_CHK) == SPC_RX100_LVD_LEVEL_CHK) {
			s_FwCtrlData.eLvdErrType = RX_LVD_LEVEL_CHK;		// LVDx有効時の電圧レベルチェックを実施する
			s_FwCtrlData.eLvdErrCodeType = RX_LVDERR_CODE_NON;	// エラーを返さない
		} else {
			//RX66x以外
			s_FwCtrlData.eLvdErrType = RX_LVD_ENA_CHK;		// LVDx有効設定チェックを実施する
			s_FwCtrlData.eLvdErrCodeType = RX_LVDERR_CODE_LVDAS;	// FFWERR_WRITE_OFS1_LVDAS_CHANGEを返す		// RevRxNo140109-001 Append Line
		}
		// RevRxNo180228-001 Modify End
		s_FwCtrlData.bAuthClrUsrBootReset = TRUE;		// ユーザブートモードでのリセット時にデバッガ認証が切れる
		s_FwCtrlData.bSetFpgaOcdUsrBootReset = TRUE;	// ユーザブートモードでのリセット時にFPGA, OCDレジスタを設定する
		s_FwCtrlData.bResetDebugModuleExtTrc = TRUE;	// 外部トレース切り替え時のデバッグモジュールリセット実施有無	// RevRxNo130301-001 Append Line
		// RevRxNo130308-001 Append Line
		s_FwCtrlData.bCoverageFunc = TRUE;				// トレースの利用機能に"カバレッジ用に利用"を設定可能
		// RevRxNo161003-001 Append Start
		if ((pMcuInfo->dwSpc[0] & SPC_EEPFCLK_EXIST) == SPC_EEPFCLK_EXIST) {	// EEPFCLKレジスタが存在するMCUの場合
			s_FwCtrlData.bEepfclkExist = TRUE;					// データフラッシュメモリアクセス周波数設定レジスタ(EEPFCLK)あり
		} else {
			s_FwCtrlData.bEepfclkExist = FALSE;					// データフラッシュメモリアクセス周波数設定レジスタ(EEPFCLK)なし
		}
		if ((pMcuInfo->dwSpc[0] & SPC_BANKMD_EXIST) == SPC_BANKMD_EXIST) {		// MDE.BANKMDビットが存在するMCUの場合
			s_FwCtrlData.bBankmdExist = TRUE;					// バンクモード選択ビット(MDE.BANKMD)あり
		} else {
			s_FwCtrlData.bBankmdExist = FALSE;					// バンクモード選択ビット(MDE.BANKMD)なし
		}
		if ((pMcuInfo->dwSpc[0] & SPC_BANKSEL_EXIST) == SPC_BANKSEL_EXIST) {	// BANKSELレジスタが存在するMCUの場合
			s_FwCtrlData.bBankselExist = TRUE;					// バンク選択レジスタ(BANKSEL)あり
		} else {
			s_FwCtrlData.bBankselExist = FALSE;					// バンク選択レジスタ(BANKSEL)なし
		}
		if ((pMcuInfo->dwSpc[1] & SPC_ROM_DATA_TRC_ACCESS_TYPE_EXIST) == SPC_ROM_DATA_TRC_ACCESS_TYPE_EXIST) {	// TRCS.RODTEビットが存在するMCUの場合
			s_FwCtrlData.bRomDataTrcAccessTypeExist = TRUE;		// ROMデータトレースアクセスタイプ出力ビット(TRCS.RODTE)あり
		} else {
			s_FwCtrlData.bRomDataTrcAccessTypeExist = FALSE;	// ROMデータトレースアクセスタイプ出力ビット(TRCS.RODTE)なし
		}
		// RevRxNo161003-001 Append End
		break;

	default:	// ここは通らない
		break;
	}

	return;
}
// RevRxNo130411-001 Append End


// RevRxNo130411-001 Append Start
//=============================================================================
/**
 * MCU個別制御情報の管理構造体のポインタとして返送する。
 * @param なし
 * @return MCU個別制御情報管理構造体へのポインタ
 */
//=============================================================================
FFWMCU_FWCTRL_DATA* GetFwCtrlData(void)
{
	return	&s_FwCtrlData;
}
// RevRxNo130411-001 Append End

