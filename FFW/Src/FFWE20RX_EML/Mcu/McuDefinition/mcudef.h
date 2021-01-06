///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcudef.h
 * @brief RX対応 MCU依存情報の定義ファイル
 * @author RSO Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, S.Ueda, K.Uemori, Y.Miyake, Y.Kawakami, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/11/22
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
・RevRxE2LNo141104-001 2014/11/21 上田
	E2 Lite対応
・RevRxNo150827-005 2015/09/11 PA 紡車
	RX651対応
・RevRxNo150827-003 2015/12/01 PA 紡車
	RV40F Phase2対応
・RevRxNo160527-001 2016/06/10 PA 辻
	RX651 正式対応
・RevRxNo161003-001 2016/10/03 PA 紡車、PA 辻
　　RX651-2MB 起動時のバンクモード、起動バンク対応
	データフラッシュメモリアクセス周波数設定レジスタ対応
	ROMデータトレースアクセスタイプ出力対応
・RevRxNo170823-001 2017/08/23 佐々木(広)
	E2拡張対応
・RevRxNo170920-001 2017/09/20 大賀
	RFWARM.hのincludeをコメントに
	RFW_mini.hのincludeを追加
*/
#ifndef	__MCUDEF_H__
#define	__MCUDEF_H__

#include "ffw_typedef.h"
#include "FFWE20ERR.h"
#include "FFWE20RX600.h"
#include "FFWE2.h"	// RevRxE2LNo141104-001 Append Line

// RevRxNo170823-001 Append Start
//拡張機能対応
#include "rfw_typedef.h"
#define RFWE2_EXPORTS
#include "RFW_mini.h"
//#include "RFWARM.h"
#include "RFWERR.h"
#include "RFWASP.h"
#include "RFWASPRX.h"
// RevRxNo170823-001 Append End

/////////// define定義 ///////////
// RevRxNo130411-001 Append Start
// データフラッシュリード許可レジスタの1ビットに対応するブロック数(1ビットあたりで許可/不許可を操作できるブロック数)
#define MCU_DF_ENA_BLKNUM_RX610		0x00000001	// 1ブロック
#define MCU_DF_ENA_BLKNUM_RX630		0x00000040	// 64ブロック
#define MCU_DF_ENA_BLKNUM_RX200		0x00000010	// 16ブロック
#define MCU_DF_ENA_BLKNUM_RX230		0x00000008	// 8ブロック	RevRxNo140515-006 Append Line
#define MCU_DF_ENA_BLKNUM_RX100		0x00000008	// 8ブロック
#define MCU_DF_ENA_BLKNUM_RX640		0x00000000	// 0ブロック(レジスタなし)
// RevRxNo130411-001 Append End

// RevRxNo140109-001 Append Start
// コンフィギュレーション設定領域が定義されているMCUコマンドの内部RAMブロック番号
#define MCU_CONFIGAREA_RAM_NO_NON	0	// 未使用
#define MCU_CONFIGAREA_RAM_NO_RX640	3	// RX640用
#define MCU_CONFIGAREA_RAM_NO_RX650	3	// RX65x用	// RevRxNo150827-005 Append Line
// RevRxNo140109-001 Append End


/////////// enum定義 ///////////
// RevRxNo130411-001 Append Start
enum FFWRXENM_MDE_TYPE {		// MDE設定種別
	RX_MDE_PIN = 0,					// MDE端子で設定
	RX_MDE_FLASH,					// フラッシュROMで設定		// RevRxNo130411-001 Modify Line
	RX_MDE_EXTRA					// Extra領域で設定
};
enum FFWRXENM_OFS_TYPE {		// OFSレジスタ設定種別
	RX_OFS_NON = 0,					// OFSレジスタなし
	RX_OFS_FLASH,					// OFSレジスタはフラッシュROMで設定
	RX_OFS_EXTRA					// OFSレジスタはExtra領域で設定
};
enum FFWRXENM_IDCODE_TYPE {		// IDコード設定種別
	RX_IDCODE_FLASH = 0,			// IDコードはフラッシュROMで設定
	RX_IDCODE_EXTRA					// IDコードはExtra領域で設定
};
enum FFWRXENM_FLASH_TYPE {		// フラッシュの種類
	RX_FLASH_RC03F = 0,				// RC03F
	RX_FLASH_MF3,					// MF3
	RX_FLASH_SC32_MF3,				// SC32用MF3	RevRxNo140515-006 Append Line
	RX_FLASH_RV40F,					// RV40F
	RX_FLASH_RV40F2					// RV40F Phase2		RevRxNo150827-005 Append Line
};
enum FFWRXENM_OCD_CLASS {		// OCDデバッグI/Fのクラス
	RX_OCD_CLS3 = 0,				// Class 3
	RX_OCD_CLS2						// Class 2
};
enum FFWRXENM_TCLK_CTRL {		// トレースクロック設定制御種別
	RX_TRCLK_CTRL_RX610 = 0,		// RX610,RX62x用トレースクロック設定処理
	RX_TRCLK_CTRL_RX630,			// RX63x,RX64x用トレースクロック設定処理
	RX_TRCLK_CTRL_RX200				// RX200,RX100用トレースクロック設定処理
};
enum FFWRXENM_LVDERR_TYPE {		// 電圧監視リセット有効設定エラーチェック種別
	RX_LVD_NON_CHK = 0,				// LVDxのエラーチェックを実施しない
	RX_LVD_LEVEL_CHK,				// LVDx有効時の電圧レベルチェックを実施する
	RX_LVD_ENA_CHK					// LVDx有効設定チェックを実施する
};
// RevRxNo130411-001 Append End
// RevRxNo140109-001 Append Start
enum FFWRXENM_LVDERR_CODETYPE {	// 電圧監視リセット有効設定エラーコード種別
	RX_LVDERR_CODE_NON = 0,			// エラーを返さない
	RX_LVDERR_CODE_LVDAS,			// FFWERR_WRITE_OFS1_LVDAS_CHANGEを返す
	RX_LVDERR_CODE_LVD1REN			// FFWERR_WRITE_OFS1_LVD1REN_CHANGEを返す
};
// RevRxNo140109-001 Append End



/////////// 構造体定義 ///////////
// FFWRX_MCUDEF_DATA
	// MCU固有情報管理構造体定義
	// ソース中で固定データとして使用する(コマンドのパラメータとして受信しない)
	// MCUのアドレス、設定データを定義する。
typedef	struct {
	MADDR	madrMaxAddr;			// MCUアドレス空間 - MCUアドレス最大値
	// RevRxNo130301-001 Append Line
	MADDR	madrFentryr;			// フラッシュP/Eモードエントリレジスタアドレス
	// RevRxNo130411-001 Append Line
	DWORD	dwDataFlashEnaBlkNum;	// データフラッシュリード許可レジスタの1ビットに対応するブロック数
	// RevRxNo130301-001 Append Line
	MADDR	madrFcurame;			// FCU RAMイネーブルレジスタアドレス
	MADDR	madrDwnpConfigStart;	// コンフィギュレーション設定領域ダウンロード対象開始アドレス	// RevRxNo140109-001 Append Line
	MADDR	madrDwnpConfigEnd;		// コンフィギュレーション設定領域ダウンロード対象終了アドレス	// RevRxNo140109-001 Append Line
	DWORD	dwConfigAreaRamNo;		// コンフィギュレーション設定領域が定義されている内部RAMブロック番号	// RevRxNo140109-001 Append Line
	DWORD	dwOfs1LvdDis;			// 電圧検出リセット無効設定値				// RevRxNo140109-001 Append Line
	DWORD	dwOfs1LvdMask;			// 電圧検出リセット有効/無効ビットマスク値	// RevRxNo140109-001 Append Line
	// RevRxNo150827-005 Append Start
	MADDR	madrExtraACacheStart;	// ExtraA領域キャッシュ開始アドレス
	DWORD	dwExtraAStartOffset;	// ExtraA領域開始アドレスのオフセット値
	DWORD	dwExtraASize;			// ExtraA領域サイズ
	MADDR	madrMdeStartAddr;		// MDEレジスタ開始アドレス
	MADDR	madrOfs1StartAddr;		// OFS1レジスタ開始アドレス
	MADDR	madrTmefStartAddr;		// TMEFレジスタ開始アドレス
	MADDR	madrTminfStartAddr;		// TMINFレジスタ開始アドレス
	DWORD	dwBanDwnpStartOffset;	// ExtraA領域ダウンロード不可開始アドレスオフセット値
	DWORD	dwBanDwnpEndOffset;		// ExtraA領域ダウンロード不可終了アドレスオフセット値
	DWORD	dwMdeStartOffset;		// MDEレジスタ開始アドレスのオフセット値
	DWORD	dwOfs1StartOffset;		// OFS1レジスタ開始アドレスのオフセット値
	DWORD	dwOsisStartOffset;		// OSISレジスタ(IDコード)開始アドレスのオフセット値
	DWORD	dwOsisEndOffset;		// OSISレジスタ(IDコード)終了アドレスのオフセット値
	DWORD	dwTminfStartOffset;		// TMINFレジスタ開始アドレスのオフセット値
	// RevRxNo150827-005 Append End
	DWORD	dwFawStartOffset;		// FAWレジスタ開始アドレスのオフセット値	// RevRxNo150827-003 Append Line
	DWORD	dwBankselStartOffset;	// バンク選択レジスタ開始アドレスのオフセット値	// RevRxNo161003-001 Append Line
	DWORD	dwDualStartupBankStartAddr;	// デュアルモード時起動バンク側先頭アドレス	// RevRxNo161003-001 Append Line
} FFWRX_MCUDEF_DATA;


// RevRxNo130411-001 Append Start
// FFWRX_FWCTRL_DATA
	// MCU個別制御情報管理構造体
	// ファームウェア内部の制御情報を定義する。
typedef	struct {
// MCU仕様
	enum FFWRXENM_MDE_TYPE		eMdeType;		// MDE設定種別
	BOOL						bUbcodeExist;	// UBコード有無
	enum FFWRXENM_OFS_TYPE		eOfsType;		// OFSレジスタ設定種別
	BOOL						bOsmExist;		// オプション設定メモリ有無
	enum FFWRXENM_IDCODE_TYPE	eIdcodeType;	// IDコード設定種別
	enum FFWRXENM_FLASH_TYPE	eFlashType;		// フラッシュの種類
	BOOL						bProtRegExist;	// プロテクトレジスタ有無

// OCD仕様
	enum FFWRXENM_OCD_CLASS		eOcdCls;		// OCDデバッグI/Fのクラス
	BOOL						bEvOaBusSelEna;	// オペランドアクセスイベントのバス選択有無
	BOOL						bTraSrcSelEna;	// トレースソース選択有無
	BOOL						bTraTimeDivEna;	// トレースタイムスタンプカウンタ分周設定有無
	enum FFWRXENM_TCLK_CTRL		eTrcClkCtrl;	// トレースクロック設定制御種別

// F/W制御仕様
	enum FFWRXENM_LVDERR_TYPE	eLvdErrType;				// 電圧監視リセット有効設定エラーチェック種別
	enum FFWRXENM_LVDERR_CODETYPE	eLvdErrCodeType;			// 電圧監視リセット有効設定エラーコード種別		// RevRxNo140109-001 Append Line
	BOOL						bAuthClrUsrBootReset;		// ユーザブートモードでのリセット時のデバッガ認証切れ発生有無
	BOOL						bSetFpgaOcdUsrBootReset;	// ユーザブートモードでのリセット時、FPGA, OCDレジスタ設定の実施有無
	// RevRxNo130301-001 Append Line
	BOOL						bResetDebugModuleExtTrc;	// 外部トレース切り替え時のデバッグモジュールリセット実施有無
	// RevRxNo130308-001 Append Line
	BOOL						bCoverageFunc;	// トレースの利用機能に"カバレッジ用に利用"を設定可能/不可
	BOOL						bFawExist;		// FAWレジスタ有無		// RevRxNo160527-001 Append Line
	// RevRxNo161003-001 Append Start
	BOOL						bEepfclkExist;				// データフラッシュメモリアクセス周波数設定レジスタ有無
	BOOL						bBankmdExist;				// バンクモード選択ビット有無
	BOOL						bBankselExist;				// バンク選択レジスタ有無
	BOOL						bRomDataTrcAccessTypeExist;	// ROMデータトレースアクセスタイプ出力ビット有無
	// RevRxNo161003-001 Append End
} FFWRX_FWCTRL_DATA;
// RevRxNo130411-001 Append End



/////////// 構造体型のtypedef ///////////
typedef FFWRX_MCUDEF_DATA FFWMCU_MCUDEF_DATA;
typedef FFWRX_FWCTRL_DATA FFWMCU_FWCTRL_DATA;	// RevRxNo130411-001 Append Line


/////////// グローバル関数の宣言 ///////////
// RevRxNo130411-001 Append Line
extern void SetMcuDefData(void);					///< MCU固有データ管理構造体の設定
extern FFWMCU_MCUDEF_DATA* GetMcuDefData(void);		///< MCU固有データ管理構造体のポインタ取得
// RevRxNo130411-001 Append Start
extern void SetFwCtrlData(void);					///< MCU個別制御情報管理構造体の設定
extern FFWMCU_FWCTRL_DATA* GetFwCtrlData(void);		///< MCU個別制御情報管理構造体のポインタ取得
// RevRxNo130411-001 Append End


#endif	// __MCUDEF_H__
