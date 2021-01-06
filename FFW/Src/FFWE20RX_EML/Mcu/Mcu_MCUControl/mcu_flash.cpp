///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_flash.cpp
 * @brief ターゲットMCU(内蔵Flash)制御関数
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, H.Akashi, S.Ueda, K.Uemori, SDS T.Iwata, Y.Kawakami, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/05/11
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・Flashダウンロード時のWorkRamサイズを動的変化させる対応	2011/09/27 橋口
・RevNo120120-001	2012/07/11 橋口
　・initCacheDataSetFlg()でキャッシュがない場合はmemsetを実行しないよう修正。
　・ユーザブート領域がない場合のライタモードチェックサム計算処理(setResultCheckSum())修正。
　・ユーザブート領域がない場合のユーザブート領域情報計算処理()修正。
・RevNo120110-001	2012/07/11 橋口
  ・警告レベル4でビルド時のWarning除去対応
・RevNo111121-007	2012/07/11 橋口
  ・IsMcuFcuFirmArea() を新規追加。
・RevNo111121-004	2012/07/11 橋口
　Flash書き換えデバッグ　バスエラー対応 → コメントのみ追加
・V.1.03 RevRxNo111215-001 2012/07/11 橋口
　FINE I/Fのホットプラグ起動処理無効化
・RevNo120314-001 2012/07/13 橋口 
  ・RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正
・RevNo120621-004 2012/10/26 橋口 
  ・データFlashの書き込みサイズ単位ブロック数を算出処理の0以下の場合1に補正する処理削除
・RevNo120622-001 2012/07/13 橋口
  ・RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正抜けの修正
・RevRxNo120606-008 2012/07/13 橋口
  ・USBブートコードが書かれている場合のユーザブート起動処理対応
・RevRxNo120910-001	2012/10/15 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・WriteFlashStart(), EraseFlashRomBlock()で、GetStatData()関数呼び出し時の
    引数のbyStatKindをdwStatKindに変更。
  ・メンバ名 dwFcuFilmLen を dwFcuFirmLen に全て変更。
・RevNo121017-005	2012/10/18 明石
  VS2008対応エラーコード　WARNING C4702対策
・RevRxNo121026-001	2012/10/30 上田
  RX100対応
  ・PROT_MCU_FWRITESTART()の引数に起動時のエンディアン情報を追加。
・RevRxNo120910-013	2012/11/05 上田
　・SetRegDataFlashReadEnable(), escFlashAccessSfrAreaWord()
　　データフラッシュ読み出し許可レジスタ設定データ生成時、マスク用変数を
　　参照していたのをデータ用変数を参照するよう修正。
・RevNo121105-001	2012/11/05 上田
　・escFlashAccessSfrAreaWord()
　　データフラッシュ読み出し許可レジスタ、書き込み・消去許可レジスタアクセス時、
　　MCU_REGINFO_DFLWE_NOまでループするよう変更。
・RevRxNo120910-008 2012/11/01 三宅
　WriteFlashData()で、ユーザマットライトデータのUSB転送サイズを256バイト単位から
　最大64KBに変更。
・RevRxNo120910-004	2012/11/01 三宅
　・static変数 s_bOfs1Lvd1renChangeFlg を追加。
　・maskIdCodeArea()で、RX1xxの場合、OFS1レジスタの電圧監視1リセットが
　　有効設定のとき(1)、無効設定にすることを追加。同時に、(1)の場合、
　　”ワーニングのフラグs_bOfs1Lvd1renChangeFlg”を立てることを追加。
　・GetOfs1Lvd1renChangeFlgPtr() の関数定義追加。
・RevRxNo120910-005	2012/11/12 明石
  ユーザブートモード起動時のUSBブート領域非消去指定対応
・RevRxNo120910-011	2012/10/31 明石
  FCUファームウェアがないMCUへの対応
・RevRxNo121029-001 2012/10/31 明石
  ダウンロード時のワークRAM用FFW内バッファの可変長対応
・RevRxNo121026-001 2012/10/29 植盛
  RX100量産対応
・RevRxEzNo121120-001 2012/11/20 橋口
　デバッグ時のFlashROMワークデータ戻し忘れ修正
・RevRxNo121022-001	2012/11/21 SDS 岩田
   EZ-CUBE PROT_MCU_DUMP()分割処理対応
   EZ-CUBE PROT_MCU_WRITE()分割処理対応
・RevRxNo121122-002	2012/11/27 上田
　ライタモードチェックサム個数不正修正。
・RevRxNo121022-001	2012/11/27 SDS 岩田
   EZ-CUBE PROT_MCU_FWRITE()分割処理対応
・RevRxNo121206-001 2012/12/07 植盛
  ブートスワップ時のキャッシュ処理対応
 ・RevRxEzNo121213-002 2012/12/18 橋口
  EZ-CUBE 性能向上対応
 ・RevRxNo121219-001 2012/12/20 植盛
  上書き指定時のデータROM書き込み処理修正
 ・RevRxNo121221-001	2012/12/21 上田
　　WTRダウンロード処理改善
 ・RevRxNo121221-002	2012/12/21 上田
　　データフラッシュ領域がない場合のダウンロード処理改善
・RevRxEzNo130117-001 2013/01/17 橋口
   EZ-CUBE ブロック数が0x197ブロックを超える場合にダウンロードができない不具合改修
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001 2013/08/22 上田、植盛
	RX64M対応
・RevRxNo130301-002 2013/11/18 上田
	RX64M MP対応
・RevRxNo130730-007 2013/11/18 大喜多
	GetStatData()の戻り値がFFWERR_OKの場合のみpeStatMcuを参照するようにする
・RevRxNo140109-001 2014/01/24 植盛、上田
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo130411-001 2014/01/21 植盛
	enum FFWRXENM_MDE_TYPE型のメンバ名変更(RX_MDE_OSM→RX_MDE_FLASH)
・RevRxNo140129-001 2014/01/29 上田
	FBERコマンドのWTRロード処理修正
・RevRxNo140515-011 2014/07/24 川上
	RX64M IDコード ALL F以外のユーザブート起動対応
・RevRxNo130730-001 2014/07/23 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140617-001	2014/06/17 大喜多
	TrustedMemory機能対応
・RevRxNo140515-005 2014/08/01大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo140616-001 2014/07/22 植盛
	MCUコマンド個別制御領域に対する内部処理実装
・RevRxNo140515-006 2014/09/09 川上
	RX231対応
・RevRxE2LNo141104-001 2015/01/07 上田
	E2 Lite対応
・RevRxNo150827-002 2015/10/30 PA 紡車
	RX651オプション設定メモリアドレス、配置変更対応
・RevRxNo150827-003 2015/12/01 PA 紡車
	RV40F Phase2対応
・RevRxNo160527-001 2016/06/10 PA 辻
	RX651 正式対応
・RevRxNo161003-001 2017/04/21 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxNo170511-001 2017/05/11 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応不具合修正
*/
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif

#include "mcu_flash.h"

#include "ffw_sys.h"
#include "mcu_sfr.h"
#include "mcu_mem.h"
#include "ffwmcu_dwn.h"
#include "domcu_mcu.h"
#include "domcu_mem.h"
#include "protmcu_mem.h"
#include "protmcu_flash.h"
#include "domcu_dwn.h"
#include "protmcu_mcu.h"
#include "mcuspec.h"
#include "do_sys.h"		// RevRxNo121022-001 Append Line
#include "mcudef.h"	// RevRxNo130411-001 Append Line
#include "errchk.h"		// RevRxNo130730-001 Append Line
#include "domcu_rst.h"	// RevRxNo130730-001 Append Line
#include "domcu_prog.h"	// RevRxNo140515-005 Append Line

// static変数

static BOOL		s_bDataFlashBlockCache[FILLDATA_CACHEBLOCK_MAX];
static BOOL		s_bSendFlashWriteStartFlg;	// BFWへのフラッシュメモリ書き込み開始通知情報

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
// ユーザーマットキャッシュメモリ格納変数
static	BOOL	s_bUmNewCacheMemFlag;			// ユーザーマットキャッシュメモリ確保フラグ、TRUE：確保済み、FALSE：未確保
static	MADDR	s_madrUmCacheMemStartAddr;		// ユーザーマット先頭アドレス
static	MADDR	s_madrUmCacheMemEndAddr;		// ユーザーマット終了アドレス
static	DWORD	s_dwUmCacheMemSize;				// ユーザーマットメモリサイズ
static	DWORD	s_dwUmCacheBlockNum;			// ユーザーマットブロック数
static	DWORD	s_dwUmProgSize;					// ユーザーマット書き込みサイズ
static	DWORD	s_bUserFlashInitFlag;			// ユーザーマット初期化実行フラグ

// データマットキャッシュメモリ格納変数
static	BOOL	s_bDmNewCacheMemFlag;			// データマットキャッシュメモリ確保フラグ、TRUE：確保済み、FALSE：未確保
static	MADDR	s_madrDmCacheMemStartAddr;		// データマット先頭アドレス
static	MADDR	s_madrDmCacheMemEndAddr;		// データマット終了アドレス
static	DWORD	s_dwDmCacheMemSize;				// データマットメモリサイズ
static	DWORD	s_dwDmCacheBlockNum;			// データマットブロック数
static	DWORD	s_dwDmProgSize;					// データマット書き込みサイズ

// ユーザーブートマットキャッシュメモリ格納変数
static	BOOL	s_bUbmNewCacheMemFlag;			// ユーザーブートマットキャッシュメモリ確保フラグ、TRUE：確保済み、FALSE：未確保
static	MADDR	s_madrUbmCacheMemStartAddr;		// ユーザーブートマット先頭アドレス
static	MADDR	s_madrUbmCacheMemEndAddr;		// ユーザーブートマット終了アドレス
static	DWORD	s_dwUbmCacheMemSize;			// ユーザーブートマットメモリサイズ
static	DWORD	s_dwUbmCacheBlockNum;			// ユーザーブートマットブロック数
static	DWORD	s_dwUbmProgSize;				// ユーザーブートマット書き込みサイズ
static	DWORD	s_bUserBootFlashInitFlag;		// ユーザーブートマット初期化実行フラグ

static	BOOL	s_bUbmCacheSetFlag;				// ユーザーブートマットキャッシュメモリ データ格納状態、格納済み：TRUE、未格納：FALSE
static	BOOL	s_bUbmBlockWriteFlag;			// ユーザーブートマットブロック書き換え有無、書き換え有り：TRUE、書き換えなし：FALSE
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// RevRxNo130301-001 Append Start
// Extra領域書き換え用変数
static BYTE	s_byExtraCacheMem[MCU_EXTRA_A_AREA_SIZE_RX650];	// Extra領域用キャッシュメモリ	// RevRxNo150827-002 Modify Line(大きい方(RX65x)のサイズに変更)
static BOOL	s_bExtraCacheSetFlag;						// Extra領域用キャッシュメモリデータ設定状態フラグ	// RevRxNo140109-001 Append Line
static BOOL	s_bExtraWriteFlag[EXTRA_A_WRITE_FLG_NUM];	// Extra領域 16バイト単位での書き換えフラグ
static BOOL	s_bExtraAreaWriteFlag;						// Extra領域書き換えフラグ
// RevRxNo130301-001 Append End

// V.1.02 RevNo110318-003 Append Start
static BOOL		s_bDwnpWorkRamGetFlg;			// 内蔵RAMダウンロード処理でワークRAMデータを退避したかを示す
static BOOL		s_bDwnpFcuRamGetFlg;			// 内蔵RAMダウンロード処理でFCURAMデータを退避したかを示す
static BOOL		s_bDwnpSfrGetFlg;				// 内蔵RAMダウンロード処理でSFRデータを退避したかを示す
// V.1.02 RevNo110318-003 Append End
// RevRxNo130301-001 Append Line
static BOOL		s_bDwnpRamEnableFlg;			// 内蔵RAMダウンロード処理でRAMを有効/動作設定したかを示す
// RevRxNo140515-005 Append Line
static BOOL		s_bMemWaitInsertFlg;			// メモリウェイトを挿入したかどうかを示す(TRUE:挿入した、FALSE:挿入していない)

static USER_ROMCACHE_RX			s_UserMatCacheData;			// ユーザーマットキャッシュメモリ構造体情報
static DATA_ROMCACHE_RX			s_DataMatCacheData;			// データブートマットキャッシュメモリ構造体情報
static USERBOOT_ROMCACHE_RX		s_UserBootMatCacheData;		// ユーザーブートマットキャッシュメモリ構造体情報


// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
USER_ROMCACHE_RX			*g_UserRomCacheTop = NULL;
DATA_ROMCACHE_RX			*g_DataRomCacheTop = NULL;
USERBOOT_ROMCACHE_RX		*g_UserBootRomCacheTop = NULL;

static	BOOL	s_bUmCacheDataInitFlg;			// ユーザーマットキャッシュメモリ初期化実行フラグ
static	BOOL	s_bUbmCacheDataInitFlg;			// ユーザーブートマットキャッシュメモリ初期化実行フラグ
static	BOOL	s_bDataFlashInitFlag;			// データマット初期化実行フラグ
static	BOOL	s_bDataFlashEraseFlag;			// データマットのみの初期化を実行するかどうかのフラグ
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

static FFW_DWNP_CHKSUM_DATA	s_ChecksumData;	// チェックサムデータ

// V.1.02 新デバイス対応 Append Start
static	BOOL	s_bEndianChange;				// ダウンロード時にエンディアンMDEビットを書き換えたかどうかを示すフラグ
static	BOOL	s_bUbcodeChange;				// ダウンロード時にUBコードを書き換えたかどうかを示すフラグ
// V.1.02 新デバイス対応 Append End
static	BOOL	s_bIdcodeChange;				// IDコードを書き換えたかどうかを示すフラグ	 RevRxNo140515-011 Append Line
// RevRxNo161003-001 Append Start
static	BOOL	s_bBankModeChange;				// ダウンロード時にエンディアンBANKMDビットを書き換えたかどうかを示すフラグ
static	BOOL	s_bBankSelChange;				// ダウンロード時にBANKSELレジスタを書き換えたかどうかを示すフラグ
static	BOOL	s_bBankSelNotChange;			// ダウンロード時にTM機能有効かつFFEE0000h～FFEEFFFFhのTM機能無効のためBANKSELレジスタが書き換えられなかったかどうかを示すフラグ
// RevRxNo161003-001 Append End

// V.1.02 新デバイス対応 Append&Modify Start
static WORD	s_wDataFlashEnableData[2];				// データFlash読み出し許可レジスタデータ退避変数
static WORD	s_wDflreKeycode[] = {MCU_REG_FLASH_DFLRE0_KEYCODE, MCU_REG_FLASH_DFLRE1_KEYCODE};		// データFlash読み出し許可レジスタキーコード 
static WORD	s_wDflweKeycode[] = {MCU_REG_FLASH_DFLWE0_KEYCODE, MCU_REG_FLASH_DFLWE1_KEYCODE};		// データFlash書き込み・消去許可レジスタキーコード 
// V.1.02 新デバイス対応 Append&Modify End


static BYTE		s_byFlashAccessSfr[MAX_FLASHACCESS_SFR_BYTE];	// フラッシュアクセス時に書き換わるSFRレジスタ格納データ(バイトアクセス)
static WORD		s_wFlashAccessSfr[MAX_FLASHACCESS_SFR_WORD];	// フラッシュアクセス時に書き換わるSFRレジスタ格納データ(ワードアクセス)
static DWORD	s_dwFlashAccessSfr[MAX_FLASHACCESS_SFR_LWORD];	// フラッシュアクセス時に書き換わるSFRレジスタ格納データ(ロングワードアクセス)

// V.1.02 RevNo110308-002 Append Start
static	BOOL	s_bDFBanBlockRead;				// FFWERR_READ_DTF_MCURUNワーニング発生確認用
static	BOOL	s_bFlashChangeRead;			// FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング発生確認用

// ユーザーRAM退避/復帰
static DWORD	s_dwmadrStartAddr;			// ワークRAM先頭アドレス
static DWORD	s_dwmadrEndAddr;			// ワークRAM終了アドレス
//static BYTE	s_byReadData[0x400+0x10];	// DUMP データ格納領域
static BYTE*	s_pbyReadData;		// DUMP データ格納バッファへのポインタ
//Flashダウンロード時のWorkRamサイズを動的変化させる対応 Append Line
static BOOL		s_bEscWorkRam;				//ワークRAM用メモリ取得フラグ

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
// FCU-RAM退避/復帰
static DWORD	s_dwmadrFcuRamStartAddr;		// FCU-RAM先頭アドレス	
static DWORD	s_dwmadrFcuRamEndAddr;			// FCU-RAM終了アドレス
static BYTE	s_byFcuRamReadData[0x2000];	// DUMP データ格納領域
static BYTE*	s_pbyFcuRamReadData;			// DUMP データ格納バッファへのポインタ
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// RevRxNo121026-001 Append Start
static BYTE		s_byDflctl_RX100;					// RX100 データ・フラッシュ・コントロールデータ退避変数
static BYTE		s_byFpmcr_RX100;					// RX100 フラッシュ・プログラミング・モード・コントロール・レジスタデータ退避変数
static BYTE		s_byFasr_RX100;					// RX100 フラッシュ・領域選択レジスタデータ退避変数
static BYTE		s_byFsarh_RX100;					// RX100 フラッシュ・アドレス・ポインタHデータ退避変数
static BYTE		s_byFcr_RX100;						// RX100 フラッシュ・シーケンサ制御レジスタデータ退避変数
static BYTE		s_byFearh_RX100;					// RX100 フラッシュ・エンド・アドレス・ポインタHデータ退避変数
static BYTE		s_byFisr_RX100;					// RX100 フラッシュ・シーケンサ・初期設定レジスタデータ退避変数
static BYTE		s_byFexcr_RX100;						// RX100 フラッシュ・Extra用シーケンサ制御レジスタデータ退避変数
static BYTE		s_byFresetr_RX100;					// RX100 フラッシュ・レジスタ・初期化レジスタデータ退避変数
static WORD		s_wFentryr_RX100;					// RX100 フラッシュP/Eモードエントリレジスタデータ退避変数
static WORD		s_wFsarl_RX100;						// RX100 フラッシュ・アドレス・ポインタLデータ退避変数
static WORD		s_wFearl_RX100;					// RX100 フラッシュ・エンド・アドレス・ポインタLデータ退避変数
static WORD		s_wfwbh_RX100;						// RX100 フラッシュ・ライト・バッファ・レジスタHデータ退避変数
static WORD		s_wfwbl_RX100;						// RX100 フラッシュ・ライト・バッファ・レジスタLデータ退避変数
static BYTE		s_byDataFlashEnableData_RX100;		// RX100データFlash読み出し許可レジスタデータ退避変数
// RevRxNo121026-001 Append End

// RevRxNo140515-006 Append Start
static BYTE		s_byDflctl_RX230;					// RX230 データ・フラッシュ・コントロールデータ退避変数
static BYTE		s_byFpmcr_RX230;					// RX230 フラッシュ・プログラミング・モード・コントロール・レジスタデータ退避変数
static BYTE		s_byFasr_RX230;						// RX230 フラッシュ・領域選択レジスタデータ退避変数
static BYTE		s_byFcr_RX230;						// RX230 フラッシュ・シーケンサ制御レジスタデータ退避変数
static BYTE		s_byFisr_RX230;						// RX230 フラッシュ・シーケンサ・初期設定レジスタデータ退避変数
static BYTE		s_byFexcr_RX230;					// RX230 フラッシュ・Extra用シーケンサ制御レジスタデータ退避変数
static BYTE		s_byFresetr_RX230;					// RX230 フラッシュリセットレジスタデータ退避変数
static WORD		s_wFentryr_RX230;					// RX230 フラッシュP/Eモードエントリレジスタデータ退避変数
static WORD		s_wFsarh_RX230;						// RX230 フラッシュ処理開始アドレスレジスタHデータ退避変数
static WORD		s_wFsarl_RX230;						// RX230 フラッシュ処理開始アドレスレジスタLデータ退避変数
static WORD		s_wFearh_RX230;						// RX230 フラッシュ処理終了アドレスレジスタHデータ退避変数
static WORD		s_wFearl_RX230;						// RX230 フラッシュ処理終了アドレスレジスタLデータ退避変数
static WORD		s_wfwb0_RX230;						// RX230 フラッシュ・ライト・バッファ・レジスタ0データ退避変数
static WORD		s_wfwb1_RX230;						// RX230 フラッシュ・ライト・バッファ・レジスタ1データ退避変数
static WORD		s_wfwb2_RX230;						// RX230 フラッシュ・ライト・バッファ・レジスタ2データ退避変数
static WORD		s_wfwb3_RX230;						// RX230 フラッシュ・ライト・バッファ・レジスタ3データ退避変数
static BYTE		s_byDataFlashEnableData_RX230;		// RX230データFlash読み出し許可レジスタデータ退避変数
// RevRxNo140515-006 Append End

// RevRxNo130301-001 Append Start
static BYTE		s_byFwepror_RX640;					// RX640 フラッシュライトイレースプロテクトレジスタ
static WORD		s_wFcurame_RX640;					// RX640 FCURAMイネーブルレジスタ
static WORD		s_wFpckar_RX640;					// RX640 フラッシュシーケンサ処理クロック通知レジスタ
static WORD		s_wFsuinitr_RX640;					// RX640 フラッシュシーケンサ設定初期化レジスタ
static WORD		s_wFentryr_RX640;					// RX640 フラッシュP/Eモードエントリレジスタ
static WORD		s_wFmatselc_RX640;					// RX640 コードフラッシュマット選択レジスタ
static WORD		s_wFprotr_RX640;					// RX640 フラッシュプロテクトレジスタ
static DWORD	s_dwFsaddr_RX640;					// RX640 FACIコマンド処理開始アドレスレジスタ
// RevRxNo130301-001 Append End

// RevRxNo120910-004 Append Line
static BOOL		s_bOfs1Lvd1renChangeFlg = FALSE;	// RX100のOFS1レジスタの電圧監視1リセットを無効設定に変更したフラグ(TRUE:変更した、FALSE:変更していない)

// RevRxNo140617-001 Append Line
static BOOL s_bDwnpDataIncludeTMArea;				// TM領域へのダウンロードデータがあったかどうかを示すフラグ(TRUE:あった、FALSE:なかった)

// RevRxNo160527-001 Append Line
static BOOL	s_bFawFsprChangeFlg = FALSE;			// FAWレジスタのFSPRビットを'1'に変更したフラグ(TRUE:変更した、FALSE:変更していない)

// static関数の宣言

static BOOL getSendFlashWriteStartFlg(void);
static void setSendFlashWriteStartFlg(void);
static void clrSendFlashWriteStartFlg(void);

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
static BOOL chkFlashRomInitBlock(enum FFWRXENM_MAREA_TYPE eAreaType, MADDR madrBlkStartAddr, MADDR madrBlkEndAddr); // フラッシュROM初期化ブロックであるかの確認 
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

static void maskIdCodeArea(void);
// RevRxNo140109-001 Append Start
static BOOL checkFlashCacheOfsDbg(enum FFWENM_ENDIAN eMcuEndian);
static BOOL checkFlashCacheOfsLvd(enum FFWENM_ENDIAN eMcuEndian);
static BOOL checkFlashCacheIdcode(void);
// RevRxNo130730-001 Delete
static BOOL checkExtraCacheOfsLvd(enum FFWENM_ENDIAN eMcuEndian);
static BOOL checkExtraCacheIdcode(void);
static BOOL checkFlashCacheMdeEndian(enum FFWENM_ENDIAN eMcuEndian);
static BOOL checkFlashCacheUbcode(enum FFWENM_ENDIAN eMcuEndian);
// RevRxNo140109-001 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
static FFWERR setFcuRam(void);
static FFWERR getFcuRam(void);
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

static FFWERR escFlashAccessSfrAreaByte(BOOL bEscape);
static FFWERR escFlashAccessSfrAreaWord(BOOL bEscape);
static FFWERR escFlashAccessSfrAreaLWord(BOOL bEscape);

// RevRxNo121026-001 Append Start
static FFWERR escFlashAccessSfrArea_RX100(BOOL bEscape);
static FFWERR escFlashReg_RX100(void);		// RevRxNo130301-001 Modify Line
static FFWERR retFlashReg_RX100(void);		// RevRxNo130301-001 Modify Line
static FFWERR setRegDataFlashReadEnable(BOOL bEnable);
static FFWERR setRegDataFlashReadEnable_RX100(BOOL bEnable);
static FFWERR getRegDataFlashReadBlockEnable(DWORD dwBlockNo,BOOL *bResult);
static FFWERR getRegDataFlashReadBlockEnable_RX100(DWORD dwBlockNo,BOOL *bResult);
// RevRxNo121026-001 Append End

// RevRxNo140515-006 Append Start
static FFWERR escFlashAccessSfrArea_RX230(BOOL bEscape);
static FFWERR escFlashReg_RX230(void);
static FFWERR retFlashReg_RX230(void);
// RevRxNo140515-006 Append End

// RevRxNo130301-001 Append Start
static FFWERR escFlashAccessSfrArea_RX640(BOOL bEscape);
static FFWERR escFlashReg_RX640(void);
static FFWERR retFlashReg_RX640(void);
// RevRxNo130301-001 Append End

// RevRxNo120606-008 Appned Line
// USBブートのUBcodeA DDUMPしたデータと比較できるようにスワップしてある 
static BYTE s_byUsbBootCode[] = {0x42,0x62,0x73,0x55,0xFF,0x74,0x6F,0x6F};

// RevRxNo140617-001 Append Line
static void setExtraCacheTmData(enum FFWENM_ENDIAN eMcuEndian);


static void maskBanDwnpExtraAArea(void);	// RevRxNo150827-002 Append Line
//=============================================================================
/**
 * フラッシュROM領域へのWRITE処理開始
 * BFWにフラッシュ書き込み開始を通知する。
 * @param eVerify         べリファイ指定(VERIFY_OFF/VERIFY_ON)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR WriteFlashStart(enum FFWENM_VERIFY_SET eVerify)
{
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modidy Start
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwFlashWriteTiming;
	DWORD	dwFlashClrInfo;
	// V.1.02 RevNo110228-002 Delete Line	FFW_FCLR_DATA_RX	BaseFclrData;
	FFW_FCLR_DATA_RX	AddFclrData;
	BOOL	bRet;
	FFW_VERIFYERR_DATA VerifyErr;
	DWORD	dwmadrProgStartAddr;
	DWORD	dwmadrProgEndAddr;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	FFWMCU_MCUAREA_DATA_RX*		pMcuAreaData;		//MCU情報
	enum FFWRX_WTR_NO eSetWtrNo;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	// V.1.02 RevNo110318-002 Append Line
	BOOL	bProtectFree = FALSE;	// プロテクト解除記録用 
	BOOL	bEsc;

	// RevRxNo130301-001 Append Start
	MADDR	madrStartAddr;
	MADDR	madrEndAddr;
	BOOL	bRamEnable = FALSE;
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];
	// RevRxNo130301-001 Append End

	BYTE	byMemWaitInsert;		// RevRxNo140515-005 Append Line

	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Append Start
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BYTE						byEndian;
	DWORD						dwAccessCount;
	BOOL						bSameWriteData;
	bSameWriteData = FALSE;
	eAccessSize = MBYTE_ACCESS;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Append End
	// RevRxNo121026-001 Append Line
	enum FFWENM_ENDIAN			eMcuEndian;

	BYTE	*pbyWtrDownloadData;
	DWORD	dwWtrDownloadDataSize;	// RevRxNo121221-001 Append Line
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	enum FFWRXENM_STAT_MCU peStatMcu;
	// RevRxNo120910-001 Modify Start
	DWORD dwStatKind;
	FFWRX_STAT_SFR_DATA pStatSFR;
	FFWRX_STAT_JTAG_DATA pStatJTAG;
	FFWRX_STAT_FINE_DATA pStatFINE;
	FFWRX_STAT_EML_DATA pStatEML;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
// RevRxNo121022-001 Append Start

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

//  ここにこの関数を持ってくると、HPON OFF時にもここを通るので、デバッグ継続を解除したくてもこの関数内で
//  補正されてしまうためデバッグ継続が解除できない。HPON OFFから呼び出されているかどうかを判断できれば
//  ここにこの処理を入れてもよい。
	// V.1.02 RevNo110407-001 Append Start
	// RevRxNo130730-001 Modify Line
	if ((GetDataFlashEraseFlag() == FALSE) && (GetHponOffInfo() == FALSE) && (GetSkipMdeMskResetOsmFwrite() == FALSE)) {		
		// データフラッシュの初期化中のイレーズ・ライト でも Hpon off時のイレーズ・ライトでもOSM書き戻し時のマスク省略でもない場合
		// V.1.02 RevNo110418-002 Modify Line
		MaskMdeUbCodeArea();	// MDE/UBコード領域のマスク設定
	}
	// V.1.02 RevNo110407-001 Append End
	maskBanDwnpExtraAArea();	// RevRxNo150827-002 Append Line

	// V.1.02 RevNo110318-003 Append Start
	s_bDwnpWorkRamGetFlg = FALSE;		// ワークRAMデータ未取得
	s_bDwnpFcuRamGetFlg = FALSE;		// FCURAMデータ未取得
	s_bDwnpSfrGetFlg = FALSE;			// SFRデータ未取得
	// V.1.02 RevNo110318-003 Append End
	// RevRxNo130301-001 Append Line
	s_bDwnpRamEnableFlg = FALSE;		// RAM有効/動作は未設定

	// RevRxNo140515-005 Appned Line
	s_bMemWaitInsertFlg = FALSE;		// メモリウェイトは挿入していない

	pDbgData = GetDbgDataRX();		// デバッグ情報取得

	pMcuInfoData = GetMcuInfoDataRX();
	pMcuAreaData = GetMcuAreaDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	clrSendFlashWriteStartFlg();			// BFWへのフラッシュメモリ書き込み開始通知情報を未通知に設定

	pbyWtrDownloadData = GetDownloadWtrDataAddr();		// 書き込みプログラム格納メモリのポインタ取得
	dwWtrDownloadDataSize = GetDownloadWtrDataSize();	// 書き込みプログラム格納メモリサイズ取得 RevRxNo121221-001 Append Line

	// RevRxNo120910-001 Modify Start
	dwStatKind = STAT_KIND_NON;		// ステータス情報以外は不要なので0x00
	// RevRxNo120910-001 Modify Line
	ferr = GetStatData(dwStatKind, &peStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
	// RevRxNo130730-007 Append Start
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130730-007 Append End
	if( peStatMcu != RX_MCUSTAT_NORMAL) {
		switch (peStatMcu) {
		case RX_MCUSTAT_RESET:
				  return FFWERR_BMCU_RESET;
		case RX_MCUSTAT_SLEEP:
				  return FFWERR_BMCU_SLEEP;
		case RX_MCUSTAT_SOFT_STDBY:
				  return FFWERR_BMCU_STANBY;
		case RX_MCUSTAT_DEEP_STDBY:
				  return FFWERR_BMCU_DEEPSTANBY;
		}
	}
	
	// フラッシュROM領域へのWRITE処理開始通知
	if (getSendFlashWriteStartFlg() == FALSE) {	// BFWMCUCmd_FWRITESTARTを送信していない場合
		bRet = GetFlashRomInitInfo(&AddFclrData);
		if (bRet == TRUE) {
			dwFlashClrInfo = BFW_FWRITESTART_FCLR_ON;
		} else {
			dwFlashClrInfo = BFW_FWRITESTART_FCLR_OFF;
		}

		if (GetDwnpOpenData() == TRUE) {		// DWNPコマンド実行中の場合
			dwFlashWriteTiming = BFW_FWRITESTART_TIMING_FWRITE;
		} else {								// DWNPコマンド実行中でない場合
			dwFlashWriteTiming = BFW_FWRITESTART_TIMING_RUN;
		}	

		// フラッシュROMアクセス前にSFR領域設定を退避
		// V.1.02 新デバイス対応 Append Start( プロテクトレジスタ値退避 & プロテクト解除 )
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
			// プロテクトレジスタ値も退避する必要あり
			bEsc = TRUE;
			ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// V.1.02 新デバイス対応 Append End( プロテクトレジスタ値退避 & プロテクト解除 )
		// RevRxNo140515-006 Append Start
		if (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3) {
			ferr = escFlashAccessSfrArea_RX230(TRUE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo140515-006 Append End
		// RevRxNo121026-001 Modify Start
		// RevRxNo130411-001 Modify Line
		} else if (pFwCtrl->eFlashType == RX_FLASH_MF3) {			// フラッシュがMF3の場合
			ferr = escFlashAccessSfrArea_RX100(TRUE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo130301-001 Append Start
		} else if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
			ferr = escFlashAccessSfrArea_RX640(TRUE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo130301-001 Append End
		} else {											// フラッシュがRC03Fの場合
			ferr = escFlashAccessSfrAreaByte(TRUE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = escFlashAccessSfrAreaWord(TRUE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = escFlashAccessSfrAreaLWord(TRUE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// RevRxNo121026-001 Modify End

		// V.1.02 RevNo110318-003 Append Line
		s_bDwnpSfrGetFlg = TRUE;		// SFRデータ取得済み

		// ソースコードDR指摘事項 No.4/5対応( この時点でプロテクトを戻しておく )
		// V.1.02 新デバイス対応 Append Start( プロテクトレジスタ値復帰 & プロテクト設定 )
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
			// プロテクトレジスタ値も退避する必要あり
			// V.1.02 RevNo110318-002 Modify Start
			if( bProtectFree ){		// プロテクト解除した場合(プロテクト設定する必要あり)
				// プロテクトレジスタ値も退避する必要あり
				bEsc = FALSE;
				ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
			// V.1.02 RevNo110318-002 Modify End
		}
		// V.1.02 新デバイス対応 Append End( プロテクトレジスタ値退避 & プロテクト解除 )

		// RevRxNo130301-001 Append Start
		// ワークRAM領域と重複するRAM領域を有効/動作設定する。
		// 本処理はRC03F以外で実施する。RC03FはSFRの退避復帰関数内で実施しているため、非該当。
		if (pFwCtrl->eFlashType != RX_FLASH_RC03F) {	// フラッシュがRC03F以外の場合

			// MCUコマンドで指定されたワークRAMアドレスを取得
			madrStartAddr = pMcuInfoData->dwadrWorkRam;
			madrEndAddr = pMcuInfoData->dwadrWorkRam + pMcuInfoData->dwsizWorkRam - 1;

			// ワークRAMがRAM領域と重複しているかを確認する
			bRamEnable = ChkRamArea(madrStartAddr, madrEndAddr, &bRamBlockEna[0]);

			if (bRamEnable == TRUE) {		// ワークRAMがRAM領域と重複している場合
				ferr = SetRegRamEnable(TRUE, &bRamBlockEna[0]);	// 重複しているRAM領域を有効/動作設定する
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}

			s_bDwnpRamEnableFlg = TRUE;		// RAM有効/動作を設定
		}
		// RevRxNo130301-001 Append End

		//ユーザRAMを退避
		// RX610では、先頭から256バイトをFlash書き込みデータ、その後xxxバイトにイレーズ/ライトプログラムを格納
		s_dwmadrStartAddr =  pMcuInfoData->dwadrWorkRam;
		s_dwmadrEndAddr = pMcuInfoData->dwadrWorkRam + pMcuInfoData->dwsizWorkRam - 1;
		// Flashダウンロード時のWorkRamサイズを動的変化させる対応 Delete Line
		// Rev.1.02までは0x400+0x10バイトRAMを退避していたが、RAM退避サイズはpMcuInfoData->dwsizWorkRam分退避する
		// DO_SetRXMCU()でメモリ確保 DO_RXHPON()で開放
		//	s_pbyReadData = s_byReadData;

		// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify Start
		// RevRxEzNo121213-002 Modify Start
		eAccessSize = MLWORD_ACCESS;
		dwAccessCount = (s_dwmadrEndAddr - s_dwmadrStartAddr + 1)/4;
		// RevRxEzNo121213-002 Modify End
		ferr = GetEndianType2(s_dwmadrStartAddr,&byEndian);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// RevRxNo140515-005 Append Start
		byMemWaitInsert = NO_MEM_WAIT_CHG;			// メモリウェイト設定変更不要
		if (GetStepContinueStartFlg() == FALSE) {	// 連続ステップ中でない場合
			ferr = ChkMemWaitInsert(s_dwmadrStartAddr, s_dwmadrEndAddr, &byMemWaitInsert);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
			SetMemWaitChange(FALSE);
			SetOPCModeChange(FALSE);
			ferr = SetMemWait(TRUE);				// メモリウェイト挿入する
			if( ferr != FFWERR_OK ){
				return ferr;
			}
			s_bMemWaitInsertFlg = TRUE;				// メモリウェイトを挿入したことを覚えておく
		}
		// RevRxNo140515-005 Append End

// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr = McuDumpDivide(dwAreaNum, s_dwmadrStartAddr, bSameAccessSize, eAccessSize, bSameAccessCount, dwAccessCount, s_pbyReadData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
		} else {
			ferr = PROT_MCU_DUMP(dwAreaNum, &s_dwmadrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, s_pbyReadData, byEndian);
		}
// RevRxNo121022-001 Append End

		// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify End
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// V.1.02 RevNo110318-003 Append Line
		s_bDwnpWorkRamGetFlg = TRUE;			// ワークRAMデータ取得済み
		//Flash書き込みプログラムを書き込み
		// + 0x110している理由は、書き込みプログラムがワークとして使用する領域のサイズが0x110のため。実際の書き込みプログラム
		// 開始アドレスはワーク領域の次となるため。書き込みプログラムが使用するワーク領域サイズが変わった場合は、この値を変更
		// する必要がある。
		dwmadrProgStartAddr = pMcuInfoData->dwadrWorkRam + 0x110;
		// RevRxNo121221-001 Modify Line
		dwmadrProgEndAddr = dwmadrProgStartAddr + dwWtrDownloadDataSize - 1;		// 書き込みプログラム格納メモリのサイズ分書き込む
		
		//Flash書き込みプログラムを書き込み
		// V.1.02 RevNo110318-003 Append&Modify Line
		eAccessSize = MLWORD_ACCESS;
		dwAccessCount = (dwmadrProgEndAddr - dwmadrProgStartAddr + 1)/4;
		//コードの書き込みなのでLITTLEエンディアンでライト
		byEndian = FFWRX_ENDIAN_LITTLE;

// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr = McuWriteDivide(eVerify, dwAreaNum, dwmadrProgStartAddr, bSameAccessSize, eAccessSize,
									bSameAccessCount, dwAccessCount, bSameWriteData, pbyWtrDownloadData, &VerifyErr, byEndian, PROT_WRITE_LENGTH_MAX_EZ);
		} else{
			ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, &dwmadrProgStartAddr, bSameAccessSize, &eAccessSize,
									bSameAccessCount, &dwAccessCount, bSameWriteData, pbyWtrDownloadData, &VerifyErr, byEndian);
		}
// RevRxNo121022-001 Append End

		// ベリファイチェック
		if (eVerify == VERIFY_ON) {
			ferr = ChkVerifyWriteData(dwmadrProgStartAddr, eAccessSize, dwAccessCount, pbyWtrDownloadData, &VerifyErr, FFWRX_ENDIAN_LITTLE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			if (VerifyErr.eErrorFlag == VERIFY_ERR) {			// ベリファイエラー発生時、処理を終了する。
				return ferr;
			}
		}
		// V.1.02 RevNo110318-003 Append&Modify End
		if (ferr != FFWERR_OK) {		  
			return ferr;
		}

		// ユーザーマット書き換えデバッグ有効 または、データマット書き換えデバッグ有効の場合
		if(pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE || pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE){
			//FCU-RAMを退避
			ferr = getFcuRam();
			// V.1.02 RevNo110318-003 Modify&Append Start
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_bDwnpFcuRamGetFlg = TRUE;			// FCURAMデータ取得済み
			// V.1.02 RevNo110318-003 Modify&Append End
		}

		//Flash書き込みプログラム番号設定(未使用)
		eSetWtrNo = WTR_NOTUSE;

		// RevRxNo121026-001 Append Line
		eMcuEndian = GetEndianDataRX();	// エンディアン情報取得

		// V.1.02 RevNo110228-002 Delete Line		memset(&BaseFclrData, 0, sizeof(FFW_FCLR_DATA_RX));  BaseFclrData未使用のため

		//E/W開始
		// V.1.02 RevNo110329 -001 Append Line
		if(GetDataFlashEraseFlag() == FALSE){		// データフラッシュの初期化でない通常のイレーズ・ライト時
			CallbackFromFfw2Target(FWRITE_START);
		// V.1.02 RevNo110329 -001 Append Line
		}
		// BFWMCUCmd_FWRITESTART送信
		setSendFlashWriteStartFlg();
		// RevRxEzNo130117-001 Modify Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr = PROT_MCU_FWRITESTART_EZ(dwFlashWriteTiming, dwFlashClrInfo, &AddFclrData, eSetWtrNo, eMcuEndian);
		} else {
			// RevRxNo121026-001 Modify Line
			ferr = PROT_MCU_FWRITESTART(dwFlashWriteTiming, dwFlashClrInfo, &AddFclrData, eSetWtrNo, eMcuEndian);
		}
		// RevRxEzNo130117-001 Modify End
		if (ferr != FFWERR_OK) {
			// V.1.02 RevNo110329 -001 Append Line
			if(GetDataFlashEraseFlag() == FALSE){		// データフラッシュの初期化でない通常のイレーズ・ライト時
				CallbackFromFfw2Target(FWRITE_ERROR);
			// V.1.02 RevNo110329 -001 Append Line
			}
			return ferr;
		}
	}
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modidy End

	return ferr;
}

//=============================================================================
/**
 * フラッシュROM領域へのWRITE処理終了
 * BFWにフラッシュ書き込み終了を通知する。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR WriteFlashEnd(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr2 = FFWERR_OK;
	enum FFWENM_VERIFY_SET eVerify;
	FFW_VERIFYERR_DATA VerifyErr;
	// V.1.02 新デバイス対応 Append Start( プロテクトレジスタ値退避 & プロテクト解除 )
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	// V.1.02 新デバイス対応 Append End( プロテクトレジスタ値退避 & プロテクト解除 )
	// V.1.02 RevNo110318-002 Append Line
	BOOL						bProtectFree = FALSE;	// プロテクト解除記録用 
	BOOL						bEsc;
	// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Append Start
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BOOL						bSameWriteData;
	BYTE						byEndian;
	DWORD						dwAccessCount;
	// RevRxNo130301-001 Append Line
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];
	// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Append End
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;
// RevRxNo121022-001 Append Line
	FFWE20_EINF_DATA	einfData;

	// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Append Start
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	bSameWriteData = FALSE;
	// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Append End
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Line
	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

// RevRxNo121022-001 Append Line
	getEinfData(&einfData);			// エミュレータ情報取得

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	eVerify = VERIFY_OFF;

	// フラッシュROM領域へのWRITE処理開始済みの場合
	if (getSendFlashWriteStartFlg() == TRUE) {	// BFWMCUCmd_FWRITESTARTを送信済みの場合

		eVerify = GetVerifySet();

		// BFWMCUCmd_FWRITEEND送信
		clrSendFlashWriteStartFlg();			// BFWへのフラッシュメモリ書き込み開始通知情報を未通知に設定
		ferr = PROT_MCU_FWRITEEND();
		if (ferr != FFWERR_OK) {
			// V.1.02 RevNo110329 -001 Append Line
			if(GetDataFlashEraseFlag() == FALSE){		// データフラッシュの初期化でない通常のイレーズ・ライト時
				CallbackFromFfw2Target(FWRITE_ERROR);
			// V.1.02 RevNo110329 -001 Append Line
			}
			//退避していたユーザRAMを復帰
			// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Modofy Start
			// RevRxEzNo121213-002 Modify Start
			eAccessSize = MLWORD_ACCESS;
			dwAccessCount = (s_dwmadrEndAddr - s_dwmadrStartAddr + 1)/4;
			// RevRxEzNo121213-002 Modify End
			ferr2 = GetEndianType2(s_dwmadrStartAddr,&byEndian);
			if (ferr2 != FFWERR_OK) {		  
				return ferr2;
			}

// RevRxNo121022-001 Append Start
			if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
				ferr2 = McuWriteDivide(eVerify, dwAreaNum, s_dwmadrStartAddr, bSameAccessSize, eAccessSize,
										bSameAccessCount, dwAccessCount, bSameWriteData, s_pbyReadData, &VerifyErr, byEndian, PROT_WRITE_LENGTH_MAX_EZ);
			} else{
				// RevRxEzNo121120-001 Modify Start
				ferr2 = PROT_MCU_WRITE(eVerify, dwAreaNum, &s_dwmadrStartAddr, bSameAccessSize, &eAccessSize,
										bSameAccessCount, &dwAccessCount, bSameWriteData, s_pbyReadData, &VerifyErr, byEndian);
				// RevRxEzNo121120-001 Modify End
			}
// RevRxNo121022-001 Append End

			// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Modofy End
			// V.1.02 RevNo110228-002 Append Start
			if (ferr2 != FFWERR_OK) {		  
				return ferr2;
			}
			// V.1.02 RevNo110228-002 Append Start
			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
			// ユーザーマット書き換えデバッグ有効 または、データマット書き換えデバッグ有効の場合
			if(pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE || pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE){
				//退避していたFCU-RAMを復帰
				ferr2 = setFcuRam();
			}
			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
			if (ferr2 != FFWERR_OK) {		  
				return ferr2;
			}
			// SFRの復帰はPROT_MCU_FWRITEEND()が正常終了した場合のみ実施する。SFRの復帰でフラッシュリセットレジスタを復帰すると
			// エラー状況が消えてしまうため。また、END処理でエラーが出た場合はここでエラーを返すようにしないといけない。
			// V.1.02 RevNo110228-002 Append Line
			return ferr;
		} 
	}

	// E/W終了
	// V.1.02 RevNo110329 -001 Append Line
	if(GetDataFlashEraseFlag() == FALSE){		// データフラッシュの初期化でない通常のイレーズ・ライト時
		CallbackFromFfw2Target(FWRITE_END);
	// V.1.02 RevNo110329 -001 Append Line
	}
	// V.1.02 RevNo110318-003 Append Line
	if( s_bDwnpWorkRamGetFlg ){			// ワークRAMデータ取得済みの場合(復帰要)
		//退避していたユーザRAMを復帰
		// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Modify Start
		// RevRxEzNo121213-002 Modify Start
		eAccessSize = MLWORD_ACCESS;
		dwAccessCount = (s_dwmadrEndAddr - s_dwmadrStartAddr + 1)/4;
		// RevRxEzNo121213-002 Modify End
		ferr2 = GetEndianType2(s_dwmadrStartAddr,&byEndian);
		if (ferr2 != FFWERR_OK) {
			return ferr;
		}
		eVerify = GetVerifySet();	// RevNo120110-001 Append Line

// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr2 = McuWriteDivide(eVerify, dwAreaNum, s_dwmadrStartAddr, bSameAccessSize, eAccessSize,
									bSameAccessCount, dwAccessCount, bSameWriteData, s_pbyReadData, &VerifyErr, byEndian, PROT_WRITE_LENGTH_MAX_EZ);
		} else{
			// RevRxEzNo121120-001 Modify Start
			ferr2 = PROT_MCU_WRITE(eVerify, dwAreaNum, &s_dwmadrStartAddr, bSameAccessSize, &eAccessSize,
									bSameAccessCount, &dwAccessCount, bSameWriteData, s_pbyReadData, &VerifyErr, byEndian);
			// RevRxEzNo121120-001 Modify End
		}
// RevRxNo121022-001 Append End

		// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Modify End
	// V.1.02 RevNo110318-003 Append Line
	}

	// V.1.02 RevNo110318-003 Append Line
	if( s_bDwnpFcuRamGetFlg ){			// FCURAMデータ取得済みの場合(復帰要)
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
		// ユーザーマット書き換えデバッグ有効 または、データマット書き換えデバッグ有効の場合
		if(pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE || pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE){
			//退避していたFCU-RAMを復帰
			ferr2 = setFcuRam();
		}
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
		if (ferr2 != FFWERR_OK) {		  
			return ferr2;
		}
	// V.1.02 RevNo110318-003 Append Line
	}

	// ソースコードDR指摘事項 No.4/5対応( この時点で再度プロテクトを解除する )
	// V.1.02 新デバイス対応 Append Start( プロテクトレジスタ値退避 & プロテクト解除 )
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
		// プロテクトレジスタ値も退避する必要あり
		bEsc = TRUE;
		ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// V.1.02 新デバイス対応 Append End( プロテクトレジスタ値退避 & プロテクト解除 )
	
	// V.1.02 RevNo110318-003 Append Line
	if( s_bDwnpSfrGetFlg ){			// Sfrデータ取得済みの場合(復帰要)
		// フラッシュROMアクセス前に退避していたSFR領域設定を復帰
		// RevRxNo140515-006 Append Start
		if (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3) {			// フラッシュがSC32_MF3の場合
			ferr = escFlashAccessSfrArea_RX230(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo140515-006 Append End
		// RevRxNo121026-001 Modify Start
		// RevRxNo130411-001 Modify Line
		} else if (pFwCtrl->eFlashType == RX_FLASH_MF3) {			// フラッシュがMF3の場合
			ferr = escFlashAccessSfrArea_RX100(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo130301-001 Append Start
		} else if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
			ferr = escFlashAccessSfrArea_RX640(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo130301-001 Append End
		} else {											// フラッシュがRC03Fの場合
			ferr = escFlashAccessSfrAreaByte(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = escFlashAccessSfrAreaWord(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = escFlashAccessSfrAreaLWord(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// RevRxNo121026-001 Modify End
	// V.1.02 RevNo110318-003 Append Line
	}

	// V.1.02 新デバイス対応 Append Start( プロテクトレジスタ値復帰 & プロテクト設定 )
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
		// プロテクトレジスタ値も退避する必要あり
		// V.1.02 RevNo110318-002 Modify Start
		if( bProtectFree ){		// プロテクト解除した場合(プロテクト設定する必要あり)
			// V.1.02 RevNo031123-001 Modify Start
			bEsc = FALSE;
			ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// V.1.02 RevNo031123-001 Modify End
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// V.1.02 RevNo110318-002 Modify End
	}
	// V.1.02 新デバイス対応 Append End( プロテクトレジスタ値退避 & プロテクト解除 )

	// RevRxNo130301-001 Append Start
	// ワークRAM領域と重複するRAM領域を有効/動作した場合、ユーザ値に復帰する。
	// 本処理はRC03F以外で実施する。RC03FはSFRの退避復帰関数内で実施しているため、非該当。
	if (pFwCtrl->eFlashType != RX_FLASH_RC03F) {	// フラッシュがRC03F以外の場合
		if (s_bDwnpRamEnableFlg == TRUE) {		// RAM有効/動作を設定済みの場合
			ferr = SetRegRamEnable(FALSE, &bRamBlockEna[0]);	// RAM有効/動作設定をユーザ値に復帰
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// RevRxNo130301-001 Append End

	// RevRxNo140515-005 Append Start
	if (s_bMemWaitInsertFlg == TRUE) {		// メモリウェイト挿入している
		ferr = SetMemWait(FALSE);		// ユーザ設定値に戻す
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-005 Append End

	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Modify Line
	if (GetDwnpOpenData() == TRUE && GetDataFlashEraseFlag() == FALSE){		// DWNPコマンド実行中の場合
		ferr = SetResultCheckSum();		// ベリファイ＆チェックサム処理
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}

// V.1.02 RevNo110415-001 Delete Start
// ここにClearAllBlockWriteFlg()処理があると、WriteFlashStart/WriteFlashDataでエラーが発生した場合でも
// WriteFlasgEnd内の処理が正常終了した場合には、フラッシュ書き換えフラグをクリアしてしまうことになるため、
// 実行時のフラッシュ書き換え失敗後の次回実行時に再度フラッシュ書き換えを行う必要があるのにフラッシュ書き換え
// できなくなるためWriteFlashExec()で何もエラーが発生しなかった場合だけClearAllBlockWriteFlg()を実行するよう変更。
// データフラッシュ初期化のためにWriteFkashStart/WriteFlashEndが発行された場合はClearAllBlockWriteFlg()を実行
// しないようになっていたので、以下の処理を削除しても問題なし。
#if 0
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
	if(GetDataFlashEraseFlag() == FALSE){		// データフラッシュ初期化対象ブロックイレーズ中ではない
		if (ferr == FFWERR_OK) {
			ClearAllBlockWriteFlg();	// 全ブロック書き換えフラグ/ブロック領域書き換えフラグをクリア
		}
	}
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
#endif
// V.1.02 RevNo110415-001 Delete End

	return ferr;
}

//=============================================================================
/**
 * フラッシュROM領域へのWRITE処理
 *  フラッシュ書き込みを実施する。
 * @param eAccessSize     アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param eVerify         べリファイ指定(VERIFY_OFF/VERIFY_ON)
 * @param pVerifyErr      ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR WriteFlashData(enum FFWENM_MACCESS_SIZE eAccessSize, enum FFWENM_VERIFY_SET eVerify, FFW_VERIFYERR_DATA* pVerifyErr)
{

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BOOL	bSameWriteData;
	MADDR	madrTmpSAddr;
	DWORD	dwGageTotal = 0;
	DWORD	dwGageCnt = 0;
	// V.1.02 RevNo110323-002 Append Start
	DWORD	dwDwnpGageSize = 0;
	float	fCount;
	// V.1.02 RevNo110323-002 Append End
	DWORD	dwUmProgCnt = 0;
	DWORD	dwDmProgCnt = 0;
	DWORD	dwUbmProgCnt = 0;
	DWORD	dwExtraProgCnt = 0;	// RevRxNo130301-001 Append Line
	DWORD	dwAccessCount;
	BYTE	byEndian;
	BYTE	byWriteArea;	// RevRxNo130301-001 Append Line

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	bSameWriteData = FALSE;

	DWORD	dwProgSize;					//書き込み時のアクセスサイズ
	DWORD	dwBlkAreaCnt;
	DWORD	dwBlkCnt;
	DWORD	dwBlockNum;
	DWORD	dwRestBlkNum;

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーブートマットキャッシュメモリ構造体情報取得

	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	DWORD	dwBlkNo;
	DWORD	dwBlkLength;						// 1ブロック分のバイトサイズ
	MADDR	madrCacheStart;
	DWORD	dwStartWriteSizeBlkNo;
	DWORD	dwProgSizeWriteAccessCount;			// 1ブロック分の書き込み回数
	DWORD	dwProgSizeCnt;						// 書き込み回数カウンタ変数
	DWORD	dwBlkAreaNum;
	BOOL	bWriteFlg;
	DWORD	dwDataChkCnt;
	DWORD	dwAreaBlkNum;

	// RevRxNo120910-008 Append Start
	DWORD	madrTmpEAddr;					// 暫定エンドアドレス(ユーザマットフラッシュ書き込み処理で使用)
	BOOL	bStartFixFlg;					// スタートアドレス確定フラグ(TRUE:確定、FALSE:未確定)
	DWORD	dwGageAddSize;					// 進捗ゲージへの追加サイズ（バイト数）
	DWORD	dwUnitSize;						// 区切りサイズ（バイト数）
	DWORD	dwProgressCnt;					// 進行サイズ（バイト数）
	BOOL	bRemainderFlg;					// ブロックサイズを区切りサイズで割った余りありフラグ(TRUE:余りあり、FALSE:余りなし)
											// "進捗ゲージへの追加サイズ"の更新条件に使用する。
	// RevRxNo120910-008 Append End
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
	dwAccessCount = 0;				// ワーニング対策
// RevRxNo121022-001 Append End

	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	SetVerifySet(eVerify);

	// ユーザーマット領域 進捗ゲージ用サイズ取得
	dwProgSize = GetCacheProgSize(MAREA_USER);	// 書き込みサイズ取得
	dwBlockNum = GetCacheBlockNum(MAREA_USER);	// ブロック総数取得

	// ブロック領域書き換えフラグ数を算出
	dwBlkAreaNum = dwBlockNum/CACHE_BLOCK_AREA_NUM_RX;
	dwRestBlkNum = dwBlockNum%CACHE_BLOCK_AREA_NUM_RX;
	if(dwRestBlkNum > 0){
		dwBlkAreaNum++;
	}

	// ブロック領域書き換えフラグを確認
	for(dwBlkAreaCnt=0; dwBlkAreaCnt<dwBlkAreaNum; dwBlkAreaCnt++){
		// 検索ブロック数を32ブロックに設定
		dwAreaBlkNum = CACHE_BLOCK_AREA_NUM_RX;
		// 最終ブロック領域の場合
		if(dwBlkAreaCnt==(dwBlkAreaNum-1)){
			// 端数ブロックがある場合、検索ブロック数をブロック領域の端数ブロック数に設定
			if(dwRestBlkNum > 0){
				dwAreaBlkNum = dwRestBlkNum;
			}
		}
		if(um_ptr->pbyBlockAreaWriteFlag[dwBlkAreaCnt] == TRUE){		// 書き換えブロックの場合
			// ブロック書き換えフラグを確認
			for(dwBlkCnt=0; dwBlkCnt<dwAreaBlkNum; dwBlkCnt++){
				dwBlkNo = (dwBlkAreaCnt * CACHE_BLOCK_AREA_NUM_RX) + dwBlkCnt;
				if(um_ptr->pbyBlockWriteFlag[dwBlkNo] == TRUE){		// 書き換えブロックの場合
					// ブロック先頭アドレス取得
					GetFlashRomBlockNoInf(MAREA_USER, dwBlkNo, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
					// 書き込みサイズ単位ブロック数を算出
					dwProgSizeWriteAccessCount = dwBlkLength/dwProgSize;
					if(dwProgSizeWriteAccessCount < 1){
						dwProgSizeWriteAccessCount = 1;
					}
					for(dwProgSizeCnt=0; dwProgSizeCnt<dwProgSizeWriteAccessCount; dwProgSizeCnt++){
						// 書き込みサイズ単位の先頭アドレスを算出
						madrTmpSAddr = madrBlkStartAddr + (dwProgSizeCnt*dwProgSize);
						// キャッシュメモリの開始位置を取得
						madrCacheStart = madrTmpSAddr - GetCacheStartAddr(MAREA_USER);
						for(dwDataChkCnt=0; dwDataChkCnt<dwProgSize; dwDataChkCnt++){
							// ライトデータが0xFFかチェック
							if (um_ptr->pbyCacheMem[madrCacheStart + dwDataChkCnt] != 0xFF) {
								// V.1.02 RevNo110323-002 Append Line
								// RevRxNo120910-008 modify Line
								dwDwnpGageSize += dwProgSize;
								dwUmProgCnt++;
								break;
							}
						}
					}
				}
			}
		}
	}


				
	// データマット領域 進捗ゲージ用サイズ取得
	dwProgSize = GetCacheProgSize(MAREA_DATA);	// 書き込みサイズ取得
	dwBlockNum = GetCacheBlockNum(MAREA_DATA);	// ブロック総数取得

	// ブロック領域書き換えフラグ数を算出
	dwBlkAreaNum = dwBlockNum/CACHE_BLOCK_AREA_NUM_RX;
	dwRestBlkNum = dwBlockNum%CACHE_BLOCK_AREA_NUM_RX;
	if(dwRestBlkNum > 0){
		dwBlkAreaNum++;
	}

	// ブロック領域書き換えフラグを確認
	for(dwBlkAreaCnt=0; dwBlkAreaCnt<dwBlkAreaNum; dwBlkAreaCnt++){
		// 検索ブロック数を32ブロックに設定
		dwAreaBlkNum = CACHE_BLOCK_AREA_NUM_RX;
		// 最終ブロック領域の場合
		if(dwBlkAreaCnt==(dwBlkAreaNum-1)){
			// 端数ブロックがある場合、検索ブロック数をブロック領域の端数ブロック数に設定
			if(dwRestBlkNum > 0){
				dwAreaBlkNum = dwRestBlkNum;
			}
		}
		if(dm_ptr->pbyBlockAreaWriteFlag[dwBlkAreaCnt] == TRUE){		// 書き換えブロックの場合
			// ブロック書き換えフラグを確認
			for(dwBlkCnt=0; dwBlkCnt<dwAreaBlkNum; dwBlkCnt++){
				dwBlkNo = (dwBlkAreaCnt * CACHE_BLOCK_AREA_NUM_RX) + dwBlkCnt;
				if(dm_ptr->pbyBlockWriteFlag[dwBlkNo] == TRUE){		// 書き換えブロックの場合
					// ブロック先頭アドレス取得
					GetFlashRomBlockNoInf(MAREA_DATA, dwBlkNo, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
					// 書き込みサイズ単位書き込みブロック番号を取得
					GetDmProgSizeWriteFlagInf(madrBlkStartAddr,&dwStartWriteSizeBlkNo);
					// 書き込みサイズ単位ブロック数を算出
					dwProgSizeWriteAccessCount = dwBlkLength/dwProgSize;
					// RevNo120621-004 Delete 1以下の場合の切り上げ処理削除
					for(dwProgSizeCnt=0; dwProgSizeCnt<dwProgSizeWriteAccessCount; dwProgSizeCnt++){
						if (dm_ptr->pbyProgSizeWriteFlag[dwStartWriteSizeBlkNo + dwProgSizeCnt ] == TRUE) {
							// V.1.02 RevNo110323-002 Append Line
							// RevRxNo120910-008 modify Line
							dwDwnpGageSize += dwProgSize;
							dwDmProgCnt++;
						}
					}
				}
			}
		}
	}
			

	// ユーザーブートマット領域 進捗ゲージ用サイズ取得
	dwProgSize = GetCacheProgSize(MAREA_USERBOOT);	// 書き込みサイズ取得

	// ブロック書き換えフラグを確認
	if(GetUbmBlockWrite() == TRUE){		// 書き換えブロックの場合
		// ブロック先頭アドレス取得
		GetFlashRomBlockNoInf(MAREA_USERBOOT, 0, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
		// 書き込みサイズ単位ブロック数を算出
		dwProgSizeWriteAccessCount = dwBlkLength/dwProgSize;
		if(dwProgSizeWriteAccessCount < 1){
			dwProgSizeWriteAccessCount = 1;
		}
		for(dwProgSizeCnt=0; dwProgSizeCnt<dwProgSizeWriteAccessCount; dwProgSizeCnt++){
			// 書き込みサイズ単位の先頭アドレスを算出
			madrTmpSAddr = madrBlkStartAddr + (dwProgSizeCnt*dwProgSize);
			// キャッシュメモリの開始位置を取得
			madrCacheStart = madrTmpSAddr - GetCacheStartAddr(MAREA_USERBOOT);
			for(dwDataChkCnt=0; dwDataChkCnt<dwProgSize; dwDataChkCnt++){
				// ライトデータが0xFFかチェック
				// V.1.02 RevNo110311-002 Modify Line
				if (ubm_ptr->pbyCacheMem[madrCacheStart + dwDataChkCnt] != 0xFF) {
					// V.1.02 RevNo110323-002 Append Line
					// RevRxNo120910-008 modify Line
					dwDwnpGageSize += dwProgSize;
					dwUbmProgCnt++;
					break;
				}
			}
		}
	}

	// RevRxNo130301-001 Append Start
	// Extra領域 進捗ゲージ用サイズ取得
	if (s_bExtraAreaWriteFlag == TRUE) {	// Extra領域書き換えありの場合
		dwProgSize = EXTRA_A_PROG_SIZE;	// 書き込みサイズ取得

		for (dwProgSizeCnt = 0; dwProgSizeCnt < EXTRA_A_WRITE_FLG_NUM; dwProgSizeCnt++) {
			if (s_bExtraWriteFlag[dwProgSizeCnt] == TRUE) {	// 書き換えありの場合
				dwDwnpGageSize += dwProgSize;
				dwExtraProgCnt++;
			}
		}
	}
	// RevRxNo130301-001 Append End

	// ユーザーマット領域 フラッシュ書き込み処理
	if(dwUmProgCnt>0){
		dwProgSize = GetCacheProgSize(MAREA_USER);	// 書き込みサイズ取得
		dwBlockNum = GetCacheBlockNum(MAREA_USER);	// ブロック総数取得

		// ブロック領域書き換えフラグ数を算出
		dwBlkAreaNum = dwBlockNum/CACHE_BLOCK_AREA_NUM_RX;
		dwRestBlkNum = dwBlockNum%CACHE_BLOCK_AREA_NUM_RX;
		if(dwRestBlkNum > 0){
			dwBlkAreaNum++;
		}

		// RevRxNo120910-008 Delete
		// 書込みデータサイズのセットは、サイズが確定していないため、PROG直前に移動。
			
		// ブロック領域書き換えフラグを確認
		for(dwBlkAreaCnt=0; dwBlkAreaCnt<dwBlkAreaNum; dwBlkAreaCnt++){
			// 検索ブロック数を32ブロックに設定
			dwAreaBlkNum = CACHE_BLOCK_AREA_NUM_RX;
			// 最終ブロック領域の場合
			if(dwBlkAreaCnt==(dwBlkAreaNum-1)){
				// 端数ブロックがある場合、検索ブロック数をブロック領域の端数ブロック数に設定
				if(dwRestBlkNum > 0){
					dwAreaBlkNum = dwRestBlkNum;
				}
			}
			if(um_ptr->pbyBlockAreaWriteFlag[dwBlkAreaCnt] == TRUE){		// 書き換えブロックの場合
				// ブロック書き換えフラグを確認
				for(dwBlkCnt=0; dwBlkCnt<dwAreaBlkNum; dwBlkCnt++){
					dwBlkNo = (dwBlkAreaCnt * CACHE_BLOCK_AREA_NUM_RX) + dwBlkCnt;
					if(um_ptr->pbyBlockWriteFlag[dwBlkNo] == TRUE){		// 書き換えブロックの場合
						// ブロック先頭アドレス取得
						GetFlashRomBlockNoInf(MAREA_USER, dwBlkNo, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
						ferr = GetEndianType2(madrBlkStartAddr, &byEndian);			// エンディアン判定＆取得
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						// RevRxNo120910-008 Delete
						// 書き込みサイズ単位ブロック数dwProgSizeWriteAccessCountの算出を削除
						bWriteFlg = FALSE;
						// RevRxNo120910-008 Modify Start
						madrTmpSAddr = madrBlkStartAddr;	// 暫定スタートアドレス初期値設定
						bStartFixFlg = FALSE;				// スタートアドレス確定フラグ初期値設定("未確定")
						dwGageAddSize = 0;					// 進捗ゲージへの追加サイズ初期値設定
						dwUnitSize = dwProgSize;			// 区切りサイズ設定
						dwProgressCnt = 0;					// 進行サイズ初期値設定
						bRemainderFlg = FALSE;				// 余りありフラグに初期値設定("余りなし")
						while (dwProgressCnt < dwBlkLength) {	// 進行サイズがブロックサイズ未満の場合
							if (dwBlkLength < (dwProgressCnt + dwUnitSize)) {	// 次の区切りサイズを足したらブロックサイズを超える場合
								dwUnitSize = dwBlkLength - dwProgressCnt;		// 区切りサイズを、最終区切りのバイト数に置き換え
								bRemainderFlg = TRUE;		// 余りありフラグに"余りあり"設定。
							}
							bWriteFlg = FALSE;				// "ライト処理実行フラグ"を初期化
							// キャッシュメモリの開始位置を取得
							madrCacheStart = (madrBlkStartAddr + dwProgressCnt) - GetCacheStartAddr(MAREA_USER);
							for(dwDataChkCnt=0; dwDataChkCnt<dwUnitSize; dwDataChkCnt++){
								// ライトデータが0xFFかチェック
								if (um_ptr->pbyCacheMem[madrCacheStart + dwDataChkCnt] != 0xFF) {
									bWriteFlg = TRUE;		// "ライト処理実行フラグ"を立てる。
									if (bRemainderFlg == FALSE) {		// "余りありフラグ"が寝ている場合(注：通常は寝ている)
										dwGageAddSize += dwUnitSize;	// "進捗ゲージへの追加サイズ"に"区切りサイズ"を足す
									}
									bRemainderFlg = FALSE;	// "余りありフラグ"に初期値設定(不要だが安全のため。)
									break;
								}
							}
							if (bStartFixFlg == FALSE) {	// "スタートアドレス確定フラグ"が"未確定"の場合
								if (bWriteFlg == TRUE) {	// "ライト処理実行フラグ"が立っている場合
									madrTmpSAddr = madrBlkStartAddr + dwProgressCnt;	// 暫定スタートアドレスに、ブロックスタートアドレス + "進行サイズ"を設定
									bStartFixFlg = TRUE;				// "スタートアドレス確定フラグ"に"確定"を設定
									dwProgressCnt += dwUnitSize;		// "進行サイズ"に、"区切りサイズ"を足す
									if (dwProgressCnt >= dwBlkLength) {		// "進行サイズ"がブロックサイズと等しいまたは越える場合(注：上で補正しているので超えることはないが)
										madrTmpEAddr = madrBlkStartAddr + dwProgressCnt - 1;	// 暫定エンドアドレス設定
										// 書込みデータサイズをセット
										switch (eAccessSize) {
										case MBYTE_ACCESS:
											dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 1;
											break;
										case MWORD_ACCESS:
											dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 2;
											break;
										case MLWORD_ACCESS:
											dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 4;
											break;
										}
										madrCacheStart = madrTmpSAddr - GetCacheStartAddr(MAREA_USER);	// キャッシュメモリの開始位置を取得
										// フラッシュROMへの書込み
							// RevRxNo121022-001 Modify Start
										byWriteArea = BFW_FWRITE_AREA_FLASH;	// フラッシュメモリ指定	// RevRxNo130301-001 Append Line
										if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
											// RevRxNo130301-001 Modify Line
											ferr = McuFlashWriteDivide(byWriteArea, eVerify, dwAreaNum, madrTmpSAddr, bSameAccessSize, eAccessSize,
													bSameAccessCount, dwAccessCount, bSameWriteData, &um_ptr->pbyCacheMem[madrCacheStart], pVerifyErr, byEndian, PROT_FWRITE_LENGTH_MAX_EZ);
										} else{
											// RevRxNo130301-001 Modify Line
											ferr = PROT_MCU_FWRITE(byWriteArea, eVerify, dwAreaNum, &madrTmpSAddr, bSameAccessSize, &eAccessSize,
													bSameAccessCount, &dwAccessCount, bSameWriteData, &um_ptr->pbyCacheMem[madrCacheStart], pVerifyErr, byEndian);
										}
							// RevRxNo121022-001 Modify End

										bWriteFlg = FALSE;				// "ライト処理実行フラグ"を寝かせる
										if (ferr != FFWERR_OK) {
											CallbackFromFfw2Target(FWRITE_ERROR);
											return ferr;
										}
										if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
											CallbackFromFfw2Target(FWRITE_ERROR);
											return ferr;
										}
										// 進捗ゲージ更新
										dwGageCnt += dwGageAddSize;		// 進捗ゲージのカウンタに、"進捗ゲージへの追加サイズ"を足す。
										fCount = ((float)dwGageCnt / dwDwnpGageSize) * GAGE_COUNT_MAX;
										dwGageTotal = (DWORD)fCount;
										SetGageCount(dwGageTotal);
										SetMsgGageCount("Internal Flash");
										dwGageAddSize = 0;				// 進捗ゲージへの追加サイズ初期値設定
										// "スタートアドレス確定フラグ"に"未確定"を設定（注：while文を抜けるので不要だが、安全のため入れておく)
										bStartFixFlg = FALSE;
									}
								} else {					// "ライト処理実行フラグ"が寝ている場合
									dwProgressCnt += dwUnitSize;		// "進行サイズ"に、"区切りサイズ"を足す
								}
							} else {						// "スタートアドレス確定フラグ"が"確定"の場合
								if (bWriteFlg == TRUE) {	// "ライト処理実行フラグ"が立っている場合
									dwProgressCnt += dwUnitSize;		// "進行サイズ"に、"区切りサイズ"を足す
									madrTmpEAddr = madrBlkStartAddr + dwProgressCnt - 1;	// 暫定エンドアドレス設定
									if (((madrTmpEAddr - madrTmpSAddr + 1) >= MAX_FLASH_TRANS_SIZE) || 	// まとまりが64K以上の場合、または
																	(dwProgressCnt >= dwBlkLength)  ) {	// "進行サイズ"がブロックサイズと等しいまたは越える場合
										// 書込みデータサイズをセット
										switch (eAccessSize) {
										case MBYTE_ACCESS:
											dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 1;
											break;
										case MWORD_ACCESS:
											dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 2;
											break;
										case MLWORD_ACCESS:
											dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 4;
											break;
										}
										madrCacheStart = madrTmpSAddr - GetCacheStartAddr(MAREA_USER);	// キャッシュメモリの開始位置を取得
										// フラッシュROMへの書込み
							// RevRxNo121022-001 Modify Start
										byWriteArea = BFW_FWRITE_AREA_FLASH;	// フラッシュメモリ指定	// RevRxNo130301-001 Append Line
										if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
											// RevRxNo130301-001 Modify Line
											ferr = McuFlashWriteDivide(byWriteArea, eVerify, dwAreaNum, madrTmpSAddr, bSameAccessSize, eAccessSize,
													bSameAccessCount, dwAccessCount, bSameWriteData, &um_ptr->pbyCacheMem[madrCacheStart], pVerifyErr, byEndian, PROT_FWRITE_LENGTH_MAX_EZ);
										} else{
											// RevRxNo130301-001 Modify Line
											ferr = PROT_MCU_FWRITE(byWriteArea, eVerify, dwAreaNum, &madrTmpSAddr, bSameAccessSize, &eAccessSize,
													bSameAccessCount, &dwAccessCount, bSameWriteData, &um_ptr->pbyCacheMem[madrCacheStart], pVerifyErr, byEndian);
										}
							// RevRxNo121022-001 Modify End

										bWriteFlg = FALSE;				// "ライト処理実行フラグ"を寝かせる
										if (ferr != FFWERR_OK) {
											CallbackFromFfw2Target(FWRITE_ERROR);
											return ferr;
										}
										if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
											CallbackFromFfw2Target(FWRITE_ERROR);
											return ferr;
										}
										// 進捗ゲージ更新
										dwGageCnt += dwGageAddSize;		// 進捗ゲージのカウンタに、"進捗ゲージへの追加サイズ"を足す。
										fCount = ((float)dwGageCnt / dwDwnpGageSize) * GAGE_COUNT_MAX;
										dwGageTotal = (DWORD)fCount;
										SetGageCount(dwGageTotal);
										SetMsgGageCount("Internal Flash");
										dwGageAddSize = 0;				// 進捗ゲージへの追加サイズを初期値へ戻す
										// "スタートアドレス確定フラグ"に"未確定"を設定
										bStartFixFlg = FALSE;
									}
								} else {					// "ライト処理実行フラグ"が寝ている場合
									madrTmpEAddr = madrBlkStartAddr + dwProgressCnt - 1;	// 暫定エンドアドレス設定（区切りの１つ前の最終アドレス）
									// 書込みデータサイズをセット
									switch (eAccessSize) {
									case MBYTE_ACCESS:
										dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 1;
										break;
									case MWORD_ACCESS:
										dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 2;
										break;
									case MLWORD_ACCESS:
										dwAccessCount = (madrTmpEAddr - madrTmpSAddr + 1) / 4;
										break;
									}
									madrCacheStart = madrTmpSAddr - GetCacheStartAddr(MAREA_USER);	// キャッシュメモリの開始位置を取得
									// フラッシュROMへの書込み
									byWriteArea = BFW_FWRITE_AREA_FLASH;	// フラッシュメモリ指定	// RevRxNo130301-001 Append Line
						// RevRxNo121022-001 Modify Start
									if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
										// RevRxNo130301-001 Modify Line
										ferr = McuFlashWriteDivide(byWriteArea, eVerify, dwAreaNum, madrTmpSAddr, bSameAccessSize, eAccessSize,
													bSameAccessCount, dwAccessCount, bSameWriteData, &um_ptr->pbyCacheMem[madrCacheStart], pVerifyErr, byEndian, PROT_FWRITE_LENGTH_MAX_EZ);
									} else{
										// RevRxNo130301-001 Modify Line
										ferr = PROT_MCU_FWRITE(byWriteArea, eVerify, dwAreaNum, &madrTmpSAddr, bSameAccessSize, &eAccessSize,
													bSameAccessCount, &dwAccessCount, bSameWriteData, &um_ptr->pbyCacheMem[madrCacheStart], pVerifyErr, byEndian);
									}
						// RevRxNo121022-001 Modify End

									bWriteFlg = FALSE;				// "ライト処理実行フラグ"を寝かせる
									if (ferr != FFWERR_OK) {
										CallbackFromFfw2Target(FWRITE_ERROR);
										return ferr;
									}
									if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
										CallbackFromFfw2Target(FWRITE_ERROR);
										return ferr;
									}
									// 進捗ゲージ更新
									dwGageCnt += dwGageAddSize;		// 進捗ゲージのカウンタに、"進捗ゲージへの追加サイズ"を足す。
									fCount = ((float)dwGageCnt / dwDwnpGageSize) * GAGE_COUNT_MAX;
									dwGageTotal = (DWORD)fCount;
									SetGageCount(dwGageTotal);
									SetMsgGageCount("Internal Flash");
									dwGageAddSize = 0;				// 進捗ゲージへの追加サイズを初期値へ戻す
									// "スタートアドレス確定フラグ"に"未確定"を設定
									bStartFixFlg = FALSE;
									// 最後に"進行サイズ"更新。
									dwProgressCnt += dwUnitSize;		// "進行サイズ"に、"区切りサイズ"を足す
								}
							}
						}
						// RevRxNo120910-008 Modify End
					}
				}
			}
		}
	}

	// データマット領域 フラッシュ書き込み処理
	if(dwDmProgCnt>0){
		dwProgSize = GetCacheProgSize(MAREA_DATA);	// 書き込みサイズ取得
		dwBlockNum = GetCacheBlockNum(MAREA_DATA);	// ブロック総数取得

		// ブロック領域書き換えフラグ数を算出
		dwBlkAreaNum = dwBlockNum/CACHE_BLOCK_AREA_NUM_RX;
		dwRestBlkNum = dwBlockNum%CACHE_BLOCK_AREA_NUM_RX;
		if(dwRestBlkNum > 0){
			dwBlkAreaNum++;
		}

		// 書込みデータサイズをセット
		switch (eAccessSize) {
		case MBYTE_ACCESS:
			dwAccessCount = dwProgSize / 1;
			break;
		case MWORD_ACCESS:
			dwAccessCount = dwProgSize / 2;
			break;
		case MLWORD_ACCESS:
			dwAccessCount = dwProgSize / 4;
			break;
		}
			
		// ブロック領域書き換えフラグを確認
		for(dwBlkAreaCnt=0; dwBlkAreaCnt<dwBlkAreaNum; dwBlkAreaCnt++){
			// 検索ブロック数を32ブロックに設定
			dwAreaBlkNum = CACHE_BLOCK_AREA_NUM_RX;
			// 最終ブロック領域の場合
			if(dwBlkAreaCnt==(dwBlkAreaNum-1)){
				// 端数ブロックがある場合、検索ブロック数をブロック領域の端数ブロック数に設定
				if(dwRestBlkNum > 0){
					dwAreaBlkNum = dwRestBlkNum;
				}
			}
			if(dm_ptr->pbyBlockAreaWriteFlag[dwBlkAreaCnt] == TRUE){		// 書き換えブロックの場合
				// ブロック書き換えフラグを確認
				for(dwBlkCnt=0; dwBlkCnt<dwAreaBlkNum; dwBlkCnt++){
					dwBlkNo = (dwBlkAreaCnt * CACHE_BLOCK_AREA_NUM_RX) + dwBlkCnt;
					if(dm_ptr->pbyBlockWriteFlag[dwBlkNo] == TRUE){		// 書き換えブロックの場合
						// ブロック先頭アドレス取得
						GetFlashRomBlockNoInf(MAREA_DATA, dwBlkNo, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
						// 書き込みサイズ単位書き込みブロック番号を取得
						GetDmProgSizeWriteFlagInf(madrBlkStartAddr,&dwStartWriteSizeBlkNo);
						ferr = GetEndianType2(madrBlkStartAddr, &byEndian);			// エンディアン判定＆取得
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						// 書き込みサイズ単位ブロック数を算出
						dwProgSizeWriteAccessCount = dwBlkLength/dwProgSize;
						// RevNo120621-004 Delete 1以下の場合の切り上げ処理削除
						bWriteFlg = FALSE;
						for(dwProgSizeCnt=0; dwProgSizeCnt<dwProgSizeWriteAccessCount; dwProgSizeCnt++){
							// 書き込みサイズ単位の先頭アドレスを算出
							madrTmpSAddr = madrBlkStartAddr + (dwProgSizeCnt*dwProgSize);
							// キャッシュメモリの開始位置を取得
							madrCacheStart = madrTmpSAddr - GetCacheStartAddr(MAREA_DATA);
							if (dm_ptr->pbyProgSizeWriteFlag[dwStartWriteSizeBlkNo + dwProgSizeCnt ] == TRUE) {
								bWriteFlg = TRUE;
							}						
							if(bWriteFlg == TRUE){
								// フラッシュROMへの書込み
								byWriteArea = BFW_FWRITE_AREA_FLASH;	// フラッシュメモリ指定	// RevRxNo130301-001 Append Line
								// RevRxNo130301-001 Modify Line
								ferr = PROT_MCU_FWRITE(byWriteArea, eVerify, dwAreaNum, &madrTmpSAddr, bSameAccessSize, &eAccessSize,
											bSameAccessCount, &dwAccessCount, bSameWriteData, &dm_ptr->pbyCacheMem[madrCacheStart], pVerifyErr, byEndian);
								bWriteFlg = FALSE;
								if (ferr != FFWERR_OK) {
									CallbackFromFfw2Target(FWRITE_ERROR);
									return ferr;
								}
								if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
									CallbackFromFfw2Target(FWRITE_ERROR);
									return ferr;
								}
								// 進捗ゲージ更新
								// V.1.02 RevNo110323-002 Modify Start
								// RevRxNo120910-008 modify Line
								dwGageCnt += dwProgSize;
								fCount = ((float)dwGageCnt / dwDwnpGageSize) * GAGE_COUNT_MAX;
								dwGageTotal = (DWORD)fCount;
								SetGageCount(dwGageTotal);
								SetMsgGageCount("Internal Flash");
								// V.1.02 RevNo110323-002 Modify End
							}
						}
					}
				}
			}
		}
	}

	// ユーザーブートマット領域 フラッシュ書き込み処理
	if(dwUbmProgCnt>0){
		dwProgSize = GetCacheProgSize(MAREA_USERBOOT);	// 書き込みサイズ取得
		// 書込みデータサイズをセット
		switch (eAccessSize) {
		case MBYTE_ACCESS:
			dwAccessCount = dwProgSize / 1;
			break;
		case MWORD_ACCESS:
			dwAccessCount = dwProgSize / 2;
			break;
		case MLWORD_ACCESS:
			dwAccessCount = dwProgSize / 4;
			break;
		}

		// ブロック書き換えフラグを確認
		if(GetUbmBlockWrite() == TRUE){		// 書き換えブロックの場合
			// ブロック先頭アドレス取得
			GetFlashRomBlockNoInf(MAREA_USERBOOT, 0, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
			ferr = GetEndianType2(madrBlkStartAddr, &byEndian);			// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// 書き込みサイズ単位ブロック数を算出
			dwProgSizeWriteAccessCount = dwBlkLength/dwProgSize;
			if(dwProgSizeWriteAccessCount < 1){
				dwProgSizeWriteAccessCount = 1;
			}
			for(dwProgSizeCnt=0; dwProgSizeCnt<dwProgSizeWriteAccessCount; dwProgSizeCnt++){
				// 書き込みサイズ単位の先頭アドレスを算出
				madrTmpSAddr = madrBlkStartAddr + (dwProgSizeCnt*dwProgSize);
				// キャッシュメモリの開始位置を取得
				madrCacheStart = madrTmpSAddr - GetCacheStartAddr(MAREA_USERBOOT);
				bWriteFlg = FALSE;
				for(dwDataChkCnt=0; dwDataChkCnt<dwProgSize; dwDataChkCnt++){
					// ライトデータが0xFFかチェック
					if (ubm_ptr->pbyCacheMem[madrCacheStart + dwDataChkCnt] != 0xFF) {
						bWriteFlg = TRUE;
						break;
					}
				}
				if(bWriteFlg == TRUE){
					// フラッシュROMへの書込み
					byWriteArea = BFW_FWRITE_AREA_FLASH;	// フラッシュメモリ指定	// RevRxNo130301-001 Append Line
					// RevRxNo130301-001 Modify Line
					ferr = PROT_MCU_FWRITE(byWriteArea, eVerify, dwAreaNum, &madrTmpSAddr, bSameAccessSize, &eAccessSize,
								bSameAccessCount, &dwAccessCount, bSameWriteData, &ubm_ptr->pbyCacheMem[madrCacheStart], pVerifyErr, byEndian);
					bWriteFlg = FALSE;
					if (ferr != FFWERR_OK) {
						CallbackFromFfw2Target(FWRITE_ERROR);
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
						CallbackFromFfw2Target(FWRITE_ERROR);
						return ferr;
					}
					// 進捗ゲージ更新
					// V.1.02 RevNo110323-002 Modify Start
					// RevRxNo120910-008 modify Line
					dwGageCnt += dwProgSize;
					fCount = ((float)dwGageCnt / dwDwnpGageSize) * GAGE_COUNT_MAX;
					dwGageTotal = (DWORD)fCount;
					SetGageCount(dwGageTotal);
					SetMsgGageCount("Internal Flash");
					// V.1.02 RevNo110323-002 Modify End
				}
			}
		}
	}
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End


	// RevRxNo130301-001 Append Start
	// Extra領域 フラッシュ書き込み処理
	if (dwExtraProgCnt > 0) {
		dwProgSize = EXTRA_A_PROG_SIZE;	// 書き込みサイズ取得
		// 書込みデータサイズをセット
		switch (eAccessSize) {
		case MBYTE_ACCESS:
			dwAccessCount = dwProgSize / 1;
			break;
		case MWORD_ACCESS:
			dwAccessCount = dwProgSize / 2;
			break;
		case MLWORD_ACCESS:
			dwAccessCount = dwProgSize / 4;
			break;
		}

		byEndian = static_cast<BYTE>(GetEndianDataRX());	// エンディアン設定

		for (dwProgSizeCnt = 0; dwProgSizeCnt < EXTRA_A_WRITE_FLG_NUM; dwProgSizeCnt++) {
			if (s_bExtraWriteFlag[dwProgSizeCnt] == TRUE) {	// 書き換えありの場合
				// 書き込みサイズ単位の先頭アドレスを算出
				madrTmpSAddr = pMcuDef->dwExtraAStartOffset + (dwProgSizeCnt * dwProgSize);	// RevRxNo150827-002 Modify Line
				// キャッシュメモリの開始位置を取得
				madrCacheStart = madrTmpSAddr - pMcuDef->dwExtraAStartOffset;	// RevRxNo150827-002 Modify Line

				// フラッシュROMへの書き込み
				byWriteArea = BFW_FWRITE_AREA_EXTRA_A;	// ExtraA領域指定
				ferr = PROT_MCU_FWRITE(byWriteArea, eVerify, dwAreaNum, &madrTmpSAddr, bSameAccessSize, &eAccessSize,
							bSameAccessCount, &dwAccessCount, bSameWriteData, &s_byExtraCacheMem[madrCacheStart], pVerifyErr, byEndian);
				if (ferr != FFWERR_OK) {
					CallbackFromFfw2Target(FWRITE_ERROR);
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
					CallbackFromFfw2Target(FWRITE_ERROR);
					return ferr;
				}
				// 進捗ゲージ更新
				dwGageCnt += dwProgSize;
				fCount = ((float)dwGageCnt / dwDwnpGageSize) * GAGE_COUNT_MAX;
				dwGageTotal = (DWORD)fCount;
				SetGageCount(dwGageTotal);
				SetMsgGageCount("Internal Flash");
			}
		}
	}
	// RevRxNo130301-001 Append End

	return ferr;

}

//=============================================================================
/**
 * フラッシュROM領域へのWRITE処理実行(開始～終了)
 * BFWにフラッシュ書き込み開始～終了を通知する。
 * @param eVerify  べリファイ指定(VERIFY_OFF/VERIFY_ON)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR WriteFlashExec(enum FFWENM_VERIFY_SET eVerify)
{
	// V.1.02 RevNo110309-002 Modify Line
	FFWERR	ferr, ferr2;
	FFW_VERIFYERR_DATA	VerifyErr;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// デバッグ情報取得
	pDbgData = GetDbgDataRX();
	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	// フラッシュROMへのライト開始処理
	ferr = WriteFlashStart(eVerify);
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	if (ferr == FFWERR_OK) {
		// フラッシュROMへのライト処理
		ferr = WriteFlashData(MLWORD_ACCESS, eVerify, &VerifyErr);
	}

	// フラッシュROMへのライト終了処理
	// V.1.02 RevNo110309-002 Modify Line
	ferr2 = WriteFlashEnd();
	if (ferr != FFWERR_OK) {	// WriteFlashStart/Data()でエラーが出ていた場合
		return ferr;			// WriteFlashStart/Data()のエラーを返す
	}
	// V.1.02 RevNo110309-002 Append Start
	if (ferr2 != FFWERR_OK) {	// WriteFlashStart()でエラーが出ていない場合でWriteFlashEnd()でエラーの場合
		return ferr2;			// WriteFlashEnd()のエラーを返す
	}
	// V.1.02 RevNo110309-002 Append End
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

	// V.1.02 RevNo110415-001 Append Line
	ClearAllBlockWriteFlg();	// 全ブロック書き換えフラグ/ブロック領域書き換えフラグをクリア

	// RevRxNo140109-001 Append Line
	SetExtraCacheSet(FALSE);	// キャッシュメモリデータ未設定に設定
		// ※Extra領域はダウンロードのみに対応しているため、Extra領域に書き込み後"データ未設定"にする。

	return FFWERR_OK;
}

//=============================================================================
/**
 * フラッシュROMブロックをクリア(FCLRで登録したブロックのみ)
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR EraseFlashRomBlock(void)
{

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr2 = FFWERR_OK;
	FFW_FCLR_DATA_RX	pFclrData;

	// V.1.02 RevNo110318-003 Append Line
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	enum FFWENM_VERIFY_SET	eVerify = VERIFY_OFF;	//イレースのためベリファイはOFFにしておく
	FFW_VERIFYERR_DATA		VerifyErr;
	DWORD	dwmadrProgStartAddr;
	DWORD	dwmadrProgEndAddr;
	DWORD	dwAddCnt = 0;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報
	FFWMCU_MCUAREA_DATA_RX*		pMcuAreaData;		//MCU情報
	enum FFWRX_WTR_NO eSetWtrNo;
	// V.1.02 RevNo110318-002 Append Line
	BOOL	bProtectFree = FALSE;	// プロテクト解除記録用 
	BOOL	bEsc;
	
	// RevRxNo130301-001 Append Start
	MADDR	madrStartAddr;
	MADDR	madrEndAddr;
	BOOL	bRamEnable = FALSE;
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];
	// RevRxNo130301-001 Append End

	// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Append Start
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BOOL						bSameWriteData;
	BYTE						byEndian;
	DWORD						dwAccessCount;
	// RevRxNo121026-001 Append Line
	enum FFWENM_ENDIAN			eMcuEndian;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	enum FFWRXENM_STAT_MCU peStatMcu;
	// RevRxNo120910-001 Modify Line
	DWORD dwStatKind;
	FFWRX_STAT_SFR_DATA pStatSFR;
	FFWRX_STAT_JTAG_DATA pStatJTAG;
	FFWRX_STAT_FINE_DATA pStatFINE;
	FFWRX_STAT_EML_DATA pStatEML;
	BYTE *pbyWtrDownloadData;
	DWORD	dwWtrDownloadDataSize;	// RevRxNo140129-001 Append Line
	FFW_UM_FCLR_DATA *pUserFclrData;	// ユーザーマットFCLR構造体ポインタ
	FFW_DM_FCLR_DATA *pDataFclrData;	// データマットFCLR構造体ポインタ
	BOOL	bFclrEna = FALSE;			// 初期化チェック結果
	DWORD	dwFclrCnt;
	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	// RevRxNo140616-001 Append Start
	MADDR madrBlkStartAddrFclr;
	MADDR madrBlkEndAddrFclr;
	DWORD dwBlkNoFclr;
	DWORD dwBlkSizeFclr;
	// RevRxNo140616-001 Append End
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	BOOL	bEnableArea;
	enum FFWRXENM_MAREA_TYPE eAreaType;
	DWORD	dwLength;			// RevRxNo140616-001 Append Line
	BYTE	byAccMeans;			// RevRxNo140616-001 Append Line
	BYTE	byAccType;			// RevRxNo140616-001 Append Line
	BOOL	bDwnpEna;			// RevRxNo140616-001 Append Line
// RevRxNo121022-001 Append Line
	FFWE20_EINF_DATA	einfData;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ	// RevRxNo140616-001 Append Line

	BYTE	byMemWaitInsert;		// RevRxNo140515-005 Append Line

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	bSameWriteData = FALSE;
	eAccessSize = MBYTE_ACCESS;
	// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Append End

	// V.1.02 RevNo110318-003 Append Start
	s_bDwnpWorkRamGetFlg = FALSE;		// ワークRAMデータ未取得
	s_bDwnpFcuRamGetFlg = FALSE;		// FCURAMデータ未取得
	s_bDwnpSfrGetFlg = FALSE;			// SFRデータ未取得
	// V.1.02 RevNo110318-003 Append End
	// RevRxNo130301-001 Append Line
	s_bDwnpRamEnableFlg = FALSE;		// RAM有効/動作は未設定

	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	pMcuInfoData = GetMcuInfoDataRX();
	pMcuAreaData = GetMcuAreaDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	pbyWtrDownloadData = GetDownloadWtrDataAddr();
	dwWtrDownloadDataSize = GetDownloadWtrDataSize();	// 書き込みプログラム格納メモリサイズ取得 RevRxNo140129-001 Append Line

	pUserFclrData = GetUserFclrData();		// ユーザーマットFCLR構造体情報取得
	pDataFclrData = GetDataFclrData();		// データマットFCLR構造体情報取得

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	
// RevRxNo121022-001 Append Line
	getEinfData(&einfData);			// エミュレータ情報取得

	clrSendFlashWriteStartFlg();			// BFWへのフラッシュメモリ書き込み開始通知情報を未通知に設定

	memset(&pFclrData, 0, sizeof(FFW_FCLR_DATA_RX));

	for (dwFclrCnt = 0; dwFclrCnt < pUserFclrData->dwNum; dwFclrCnt++) {
		// RevRxNo140616-001 Append Start
		GetFlashRomBlockInf(MAREA_USER, pUserFclrData->dwmadrBlkStart[dwFclrCnt], &madrBlkStartAddrFclr, &madrBlkEndAddrFclr, &dwBlkNoFclr, &dwBlkSizeFclr);
		// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
		// そのため、以後の処理に不都合が生じることはない。
		bDwnpEna = FALSE;
		ChkAreaAccessMethod(madrBlkStartAddrFclr, madrBlkEndAddrFclr, &dwLength, &byAccMeans, &byAccType);
		if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
			// アクセス属性がリード/ライト可またはライトのみ可の場合

			bDwnpEna = TRUE;
		}
		// RevRxNo140616-001 Append End

		// RevRxNo140617-001 Modify Start
		if (GetTMEnable() == TRUE) {		// TM機能有効の場合
			if (ChkTmArea(pUserFclrData->dwmadrBlkStart[dwFclrCnt]) == FALSE) {		// ブロック先頭アドレスがTM領域内にない場合
				// RevRxNo140616-001 Modify Start
				if (bDwnpEna == TRUE) {						// アクセス属性がリード/ライト可またはライトのみ可の場合
					pFclrData.dwmadrBlkStart[dwAddCnt] = pUserFclrData->dwmadrBlkStart[dwFclrCnt];
					pFclrData.byFlashType[dwAddCnt] = INIT_UM;
					pFclrData.dwNum++;
					dwAddCnt++;
					bFclrEna = TRUE;
				}
				// RevRxNo140616-001 Modify End
			}
		} else {						// TM機能無効の場合
			// RevRxNo140616-001 Modify Start
			if (bDwnpEna == TRUE) {						// アクセス属性がリード/ライト可またはライトのみ可の場合
				pFclrData.dwmadrBlkStart[dwAddCnt] = pUserFclrData->dwmadrBlkStart[dwFclrCnt];
				pFclrData.byFlashType[dwAddCnt] = INIT_UM;
				pFclrData.dwNum++;
				dwAddCnt++;
				bFclrEna = TRUE;
			}
			// RevRxNo140616-001 Modify End
		}
		// RevRxNo140617-001 Modify End

	}
	for (dwFclrCnt = 0; dwFclrCnt < pDataFclrData->dwNum; dwFclrCnt++) {
		// RevRxNo140616-001 Append Start
		GetFlashRomBlockInf(MAREA_DATA, pDataFclrData->dwmadrBlkStart[dwFclrCnt], &madrBlkStartAddrFclr, &madrBlkEndAddrFclr, &dwBlkNoFclr, &dwBlkSizeFclr);
		// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
		// そのため、以後の処理に不都合が生じることはない。
		bDwnpEna = FALSE;
		ChkAreaAccessMethod(madrBlkStartAddrFclr, madrBlkEndAddrFclr, &dwLength, &byAccMeans, &byAccType);
		if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
			// アクセス属性がリード/ライト可またはライトのみ可の場合

			bDwnpEna = TRUE;
		}
		// RevRxNo140616-001 Append End

		// RevRxNo140616-001 Modify Start
		if (bDwnpEna == TRUE) {						// アクセス属性がリード/ライト可またはライトのみ可の場合
			pFclrData.dwmadrBlkStart[dwAddCnt] = pDataFclrData->dwmadrBlkStart[dwFclrCnt];
			pFclrData.byFlashType[dwAddCnt] = INIT_DM_UBM;
			pFclrData.dwNum++;
			dwAddCnt++;
			bFclrEna = TRUE;
		}
		// RevRxNo140616-001 Modify End
	}
	if(GetUserBootMatFclr() == TRUE){
		// RevRxNo140616-001 Append Start
		ubm_ptr = GetUserBootMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
		madrBlkStartAddr = GetCacheStartAddr(MAREA_USERBOOT);
		madrBlkEndAddr = GetCacheEndAddr(MAREA_USERBOOT);
		// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
		// そのため、以後の処理に不都合が生じることはない。
		bDwnpEna = FALSE;
		ChkAreaAccessMethod(madrBlkStartAddr, madrBlkEndAddr, &dwLength, &byAccMeans, &byAccType);
		if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
			// アクセス属性がリード/ライト可またはライトのみ可の場合

			bDwnpEna = TRUE;
		}
		// RevRxNo140616-001 Append End

		// RevRxNo140616-001 Modify Start
		if (bDwnpEna == TRUE) {						// アクセス属性がリード/ライト可またはライトのみ可の場合
			pFclrData.dwmadrBlkStart[dwAddCnt] = pMcuAreaData->dwmadrUserBootStart;
			pFclrData.byFlashType[dwAddCnt] = INIT_DM_UBM;
			pFclrData.dwNum++;
			dwAddCnt++;
			bFclrEna = TRUE;
		}
		// RevRxNo140616-001 Modify End
	}	

	if(bFclrEna == FALSE){
		return ferr;
	}

	// RevRxNo120910-001 Modify Line
	dwStatKind = STAT_KIND_NON;		// ステータス情報以外は不要なので0x00
	// RevRxNo120910-001 Modify Line
	ferr = GetStatData(dwStatKind, &peStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
	// RevRxNo130730-007 Append Start
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo130730-007 Append End
	if( peStatMcu != RX_MCUSTAT_NORMAL) {
		switch (peStatMcu) {
		case RX_MCUSTAT_RESET:
				  return FFWERR_BMCU_RESET;
		case RX_MCUSTAT_SLEEP:
				  return FFWERR_BMCU_SLEEP;
		case RX_MCUSTAT_SOFT_STDBY:
				  return FFWERR_BMCU_STANBY;
		case RX_MCUSTAT_DEEP_STDBY:
				  return FFWERR_BMCU_DEEPSTANBY;
		}
	}
	
	// RevRxNo130301-001 Append Start
	// ワークRAM領域と重複するRAM領域を有効/動作設定する。
	// 本処理はRC03F以外で実施する。RC03FはSFRの退避復帰関数内で実施しているため、非該当。
	if (pFwCtrl->eFlashType != RX_FLASH_RC03F) {	// フラッシュがRC03F以外の場合

		// MCUコマンドで指定されたワークRAMアドレスを取得
		madrStartAddr = pMcuInfoData->dwadrWorkRam;
		madrEndAddr = pMcuInfoData->dwadrWorkRam + pMcuInfoData->dwsizWorkRam - 1;

		// ワークRAMがRAM領域と重複しているかを確認する
		bRamEnable = ChkRamArea(madrStartAddr, madrEndAddr, &bRamBlockEna[0]);

		if (bRamEnable == TRUE) {		// ワークRAMがRAM領域と重複している場合
			ferr = SetRegRamEnable(TRUE, &bRamBlockEna[0]);	// 重複しているRAM領域を有効/動作設定する
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

		s_bDwnpRamEnableFlg = TRUE;		// RAM有効/動作を設定
	}
	// RevRxNo130301-001 Append End

	// フラッシュROM領域へのWRITE処理開始通知
	// フラッシュROMアクセス前にSFR領域設定を退避
	// V.1.02 新デバイス対応 Append Start( プロテクトレジスタ値退避 & プロテクト解除 )
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
		// プロテクトレジスタ値も退避する必要あり
		bEsc = TRUE;
		ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// V.1.02 新デバイス対応 Append End( プロテクトレジスタ値退避 & プロテクト解除 )

	// RevRxNo140515-006 Append Start
	if (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3) {			// フラッシュがSC32_MF3の場合
		ferr = escFlashAccessSfrArea_RX230(TRUE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	// RevRxNo140515-006 Append End
	// RevRxNo121026-001 Modify Start
	// RevRxNo130411-001 Modify Line
	} else if (pFwCtrl->eFlashType == RX_FLASH_MF3) {			// フラッシュがMF3の場合
		ferr = escFlashAccessSfrArea_RX100(TRUE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	// RevRxNo130301-001 Append Start
	} else if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
		ferr = escFlashAccessSfrArea_RX640(TRUE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	// RevRxNo130301-001 Append End
	} else {											// フラッシュがRC03Fの場合
		ferr = escFlashAccessSfrAreaByte(TRUE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		ferr = escFlashAccessSfrAreaWord(TRUE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		ferr = escFlashAccessSfrAreaLWord(TRUE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo121026-001 Modify End

	// V.1.02 RevNo110318-003 Append Line
	s_bDwnpSfrGetFlg = TRUE;		// SFRデータ取得済み

	//ユーザRAMを退避
	// RX610では、先頭から256バイトをFlash書き込みデータ、その後xxxバイトにイレーズ/ライトプログラムを格納
	s_dwmadrStartAddr =  pMcuInfoData->dwadrWorkRam;
	s_dwmadrEndAddr = pMcuInfoData->dwadrWorkRam + pMcuInfoData->dwsizWorkRam - 1;
	// Rev.1.02までは0x400+0x10バイトRAMを退避していたが、RAM退避サイズはpMcuInfoData->dwsizWorkRam分退避する
	// DO_SetRXMCU()でメモリ確保 DO_RXHPON()で開放
	//	s_pbyReadData = s_byReadData;

	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify Start
	eAccessSize = MBYTE_ACCESS;
	dwAccessCount = s_dwmadrEndAddr - s_dwmadrStartAddr + 1;
	ferr = GetEndianType2(s_dwmadrStartAddr,&byEndian);
	if (ferr != FFWERR_OK) {		  
		return ferr;
	}

	// RevRxNo140515-005 Append Start
	SetMemWaitChange(FALSE);
	SetOPCModeChange(FALSE);

	ferr = ChkMemWaitInsert(s_dwmadrStartAddr, s_dwmadrEndAddr, &byMemWaitInsert);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
		ferr = SetMemWait(TRUE);				// メモリウェイト挿入する
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		s_bMemWaitInsertFlg = TRUE;				// メモリウェイトを挿入したことを覚えておく
	}
	// RevRxNo140515-005 Append End

// RevRxNo121022-001 Append Start
	if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
		ferr = McuDumpDivide(dwAreaNum, s_dwmadrStartAddr, bSameAccessSize, eAccessSize, bSameAccessCount, dwAccessCount, s_pbyReadData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
	} else{
		ferr = PROT_MCU_DUMP(dwAreaNum, &s_dwmadrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, s_pbyReadData, byEndian);
	}
// RevRxNo121022-001 Append End

	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify End
	if (ferr != FFWERR_OK) {		  
		return ferr;
	}
	// V.1.02 RevNo110318-003 Append Line
	s_bDwnpWorkRamGetFlg = TRUE;			// ワークRAMデータ取得済み
	// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
	//Flash書き込みプログラムを書き込み
	dwmadrProgStartAddr = pMcuInfoData->dwadrWorkRam + 0x110;
	// RevRxNo140129-001 Modify Line
	dwmadrProgEndAddr = dwmadrProgStartAddr + dwWtrDownloadDataSize - 1;		// 書き込みプログラム格納メモリのサイズ分書き込む

	//Flash書き込みプログラムを書き込み
	// V.1.02 RevNo110318-003 Append&Modify Line
	eAccessSize = MLWORD_ACCESS;
	dwAccessCount = (dwmadrProgEndAddr - dwmadrProgStartAddr + 1)/4;
	//書き込みプログラムなので、LITTLEエンディアンでライト
	byEndian = FFWRX_ENDIAN_LITTLE;

// RevRxNo121022-001 Append Start
	if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
		ferr = McuWriteDivide(eVerify, dwAreaNum, dwmadrProgStartAddr, bSameAccessSize, eAccessSize,
								bSameAccessCount, dwAccessCount, bSameWriteData, pbyWtrDownloadData, &VerifyErr,byEndian, PROT_WRITE_LENGTH_MAX_EZ);
	} else{
		ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, &dwmadrProgStartAddr, bSameAccessSize, &eAccessSize,
								bSameAccessCount, &dwAccessCount, bSameWriteData, pbyWtrDownloadData, &VerifyErr,byEndian);
	}
// RevRxNo121022-001 Append End

	// ベリファイチェック
	if (eVerify == VERIFY_ON) {
		ferr = ChkVerifyWriteData(dwmadrProgStartAddr, eAccessSize, dwAccessCount, pbyWtrDownloadData, &VerifyErr, FFWRX_ENDIAN_LITTLE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (VerifyErr.eErrorFlag == VERIFY_ERR) {			// ベリファイエラー発生時、処理を終了する。
			return ferr;
		}
	}
	// V.1.02 RevNo110318-003 Append&Modify End
	if (ferr != FFWERR_OK) {		  
		return ferr;
	}

	// ユーザーマット書き換えデバッグ有効 または、データマット書き換えデバッグ有効の場合
	if(pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE || pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE){
		//FCU-RAMを退避
		ferr = getFcuRam();
		// V.1.02 RevNo110318-003 Modify&Append Start
		if (ferr != FFWERR_OK) {		  
			return ferr;
		}
		s_bDwnpFcuRamGetFlg = TRUE;			// FCURAMデータ取得済み
		// V.1.02 RevNo110318-003 Modify&Append End
	}

	//Flash書き込みプログラム番号設定(未使用)
	eSetWtrNo = WTR_NOTUSE;
	// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

	// RevRxNo121026-001 Append Line
	eMcuEndian = GetEndianDataRX();	// エンディアン情報取得

	//E/W開始
	CallbackFromFfw2Target(FWRITE_START);

	// BFWMCUCmd_FWRITESTART送信
	setSendFlashWriteStartFlg();
	// FLASH_ROM_FCU_300だけWTRのオフセット位置が異なるので、何をロードしたかの情報をBFWに渡す。
	// RevRxEzNo130117-001 Modify Start
	if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
		ferr = PROT_MCU_FWRITESTART_EZ(BFW_FWRITESTART_TIMING_FWRITE, BFW_FWRITESTART_FCLR_ON, &pFclrData, eSetWtrNo, eMcuEndian);
	} else {
		// RevRxNo121026-001 Modify Line
		ferr = PROT_MCU_FWRITESTART(BFW_FWRITESTART_TIMING_FWRITE, BFW_FWRITESTART_FCLR_ON, &pFclrData, eSetWtrNo, eMcuEndian);
	}
	// RevRxEzNo130117-001 Modify End
	if (ferr != FFWERR_OK) {
		CallbackFromFfw2Target(FWRITE_ERROR);
	}

	// 以降、終了処理
	// フラッシュROM領域へのWRITE処理開始済みの場合
	if (getSendFlashWriteStartFlg() == TRUE) {	// BFWMCUCmd_FWRITESTARTを送信済みの場合
		// BFWMCUCmd_FWRITEEND送信
		clrSendFlashWriteStartFlg();			// BFWへのフラッシュメモリ書き込み開始通知情報を未通知に設定
		ferr = PROT_MCU_FWRITEEND();
		if (ferr != FFWERR_OK) {
			CallbackFromFfw2Target(FWRITE_ERROR);
			//退避していたユーザRAMを復帰
			eAccessSize = MBYTE_ACCESS;
			dwAccessCount = s_dwmadrEndAddr - s_dwmadrStartAddr + 1;
			ferr2 = GetEndianType2(s_dwmadrStartAddr,&byEndian);
			if (ferr2 != FFWERR_OK) {		  
				return ferr2;
			}

// RevRxNo121022-001 Append Start
			if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
				ferr2 = McuWriteDivide(eVerify, dwAreaNum, s_dwmadrStartAddr, bSameAccessSize, eAccessSize,
										bSameAccessCount, dwAccessCount, bSameWriteData, s_pbyReadData, &VerifyErr, byEndian, PROT_WRITE_LENGTH_MAX_EZ);
			} else{
				ferr2 = PROT_MCU_WRITE(eVerify, dwAreaNum, &s_dwmadrStartAddr, bSameAccessSize, &eAccessSize,
										bSameAccessCount, &dwAccessCount, bSameWriteData, s_pbyReadData, &VerifyErr, byEndian);
			}
// RevRxNo121022-001 Append End

			if (ferr2 != FFWERR_OK) {		  
				return ferr2;
			}
			// ユーザーマット書き換えデバッグ有効 または、データマット書き換えデバッグ有効の場合
			if(pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE || pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE){
				//退避していたFCU-RAMを復帰
				ferr2 = setFcuRam();
				if (ferr2 != FFWERR_OK) {		  
					return ferr2;
				}
			}
			return ferr;
		}
	}
	// E/W終了
	CallbackFromFfw2Target(FWRITE_END);
	// V.1.02 RevNo110318-003 Modify Start
	if( s_bDwnpWorkRamGetFlg ){			// ワークRAMデータ取得済みの場合(復帰要)
		//退避していたユーザRAMを復帰
		eAccessSize = MBYTE_ACCESS;
		dwAccessCount = s_dwmadrEndAddr - s_dwmadrStartAddr + 1;
		ferr2 = GetEndianType2(s_dwmadrStartAddr,&byEndian);
		if (ferr2 != FFWERR_OK) {		  
			return ferr2;
		}

// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr2 = McuWriteDivide(eVerify, dwAreaNum, s_dwmadrStartAddr, bSameAccessSize, eAccessSize,
									bSameAccessCount, dwAccessCount, bSameWriteData, s_pbyReadData, &VerifyErr, byEndian, PROT_WRITE_LENGTH_MAX_EZ);
		} else{
			ferr2 = PROT_MCU_WRITE(eVerify, dwAreaNum, &s_dwmadrStartAddr, bSameAccessSize, &eAccessSize,
									bSameAccessCount, &dwAccessCount, bSameWriteData, s_pbyReadData, &VerifyErr, byEndian);
		}
// RevRxNo121022-001 Append End

	}
	if( s_bDwnpFcuRamGetFlg ){			// FCURAMデータ取得済みの場合(復帰要)
		// ユーザーマット書き換えデバッグ有効 または、データマット書き換えデバッグ有効の場合
		if(pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE || pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE){
			//退避していたFCU-RAMを復帰
			ferr2 = setFcuRam();
			if (ferr2 != FFWERR_OK) {		  
				return ferr2;
			}
		}
	}
	if( s_bDwnpSfrGetFlg ){			// SFRデータ取得済みの場合(復帰要)
		// フラッシュROMアクセス前に退避していたSFR領域設定を復帰
		// RevRxNo140515-006 Append Start
		if (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3) {			// フラッシュがSC32_MF3の場合
			ferr = escFlashAccessSfrArea_RX230(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo140515-006 Append End
		// RevRxNo121026-001 Modify Start
		// RevRxNo130411-001 Modify Line
		} else if (pFwCtrl->eFlashType == RX_FLASH_MF3) {			// フラッシュがMF3の場合
			ferr = escFlashAccessSfrArea_RX100(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo130301-001 Append Start
		} else if (pFwCtrl->eFlashType == RX_FLASH_RV40F) {	// フラッシュがRV40Fの場合
			ferr = escFlashAccessSfrArea_RX640(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		// RevRxNo130301-001 Append End
		} else {											// フラッシュがRC03Fの場合
			ferr = escFlashAccessSfrAreaByte(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = escFlashAccessSfrAreaWord(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = escFlashAccessSfrAreaLWord(FALSE);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// RevRxNo121026-001 Modify End
	}
	// V.1.02 RevNo110318-003 Modify End

	// V.1.02 新デバイス対応 Append Start( プロテクトレジスタ値復帰 & プロテクト設定 )
	// RevRxNo121026-001, RevRxNo130411-001 Modify Line
	if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
		// プロテクトレジスタ値も退避する必要あり
		if( bProtectFree == TRUE ){
			bEsc = FALSE;
			ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// V.1.02 新デバイス対応 Append End( プロテクトレジスタ値退避 & プロテクト解除 )

	// RevRxNo130301-001 Append Start
	// ワークRAM領域と重複するRAM領域を有効/動作した場合、ユーザ値に復帰する。
	// 本処理はRC03F以外で実施する。RC03FはSFRの退避復帰関数内で実施しているため、非該当。
	if (pFwCtrl->eFlashType != RX_FLASH_RC03F) {	// フラッシュがRC03F以外の場合
		if (s_bDwnpRamEnableFlg == TRUE) {		// RAM有効/動作を設定済みの場合
			ferr = SetRegRamEnable(FALSE, &bRamBlockEna[0]);	// RAM有効/動作設定をユーザ値に復帰
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// RevRxNo130301-001 Append End

	// RevRxNo140515-005 Append Start
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
		ferr = SetMemWait(FALSE);				// ユーザ設定値に戻す
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-005 Append End

	for(dwFclrCnt = 0;dwFclrCnt<pFclrData.dwNum;dwFclrCnt++){
		ferr = ChkBlockArea(pFclrData.dwmadrBlkStart[dwFclrCnt], pFclrData.dwmadrBlkStart[dwFclrCnt], &dwAccessCount, &eAreaType, 1, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if(eAreaType == MAREA_USER){
			// FCLR対象ブロックのブロック番号を取得
			GetFlashRomBlockInf(MAREA_USER, pFclrData.dwmadrBlkStart[dwFclrCnt], &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
			um_ptr->pbyCacheSetFlag[dwBlkNo] = FALSE;		// RevRxNo130301-001 Modify Line
		}
		else if(eAreaType == MAREA_DATA){
			// FCLR対象ブロックのブロック番号を取得
			GetFlashRomBlockInf(MAREA_DATA, pFclrData.dwmadrBlkStart[dwFclrCnt], &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
			dm_ptr->pbyCacheSetFlag[dwBlkNo] = FALSE;		// RevRxNo130301-001 Modify Line
		}
		else{
			SetUbmCacheSet(FALSE);
		}
	}
	ClearAllBlockWriteFlg();	// 全ブロック書き換えフラグ/ブロック領域書き換えフラグをクリア
	return ferr;
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

}

// V.1.02 RevNo110308-002 Append Start
//=============================================================================
/**
 * FFWERR_READ_DTF_MCURUN/FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング発生記録用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrMemAccWarningFlg(void)
{
	s_bDFBanBlockRead = FALSE;		// FFWERR_READ_DTF_MCURUNワーニング発生していない
	s_bFlashChangeRead = FALSE;		// FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング発生していない
}

//=============================================================================
/**
 * FFWERR_READ_DTF_MCURUNワーニング発生記録用変数の参照
 * @param なし
 * @retval BOOL TRUE:ワーニングあり　FALSE:ワーニングなし
 */
//=============================================================================
BOOL GetDtfMcurunFlg(void)
{
	return s_bDFBanBlockRead;
}
//=============================================================================
/**
 * FFWERR_READ_DTF_MCURUNワーニング発生記録用変数の設定
 * @param bDFBanBlockRead TRUE:ワーニングあり　FALSE:ワーニングなし
 * @retval なし
 */
//=============================================================================
void SetDtfMcurunFlg(BOOL bDFBanBlockRead)
{
	s_bDFBanBlockRead = bDFBanBlockRead;
	return;
}

//=============================================================================
/**
 * FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング発生記録用変数の参照
 * @param なし
 * @retval TRUE ワーニング発生　FALSE ワーニングなし
 */
//=============================================================================
BOOL GetFlashDbgMcurunFlg(void)
{
	return s_bFlashChangeRead;
}
//=============================================================================
/**
 * FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング発生記録用変数の参照
 * @param TRUE ワーニング発生　FALSE ワーニングなし
 * @retval なし
 */
//=============================================================================
void SetFlashDbgMcurunFlg(BOOL bFlashChangeRead)
{
	s_bFlashChangeRead = bFlashChangeRead;
	return;
}
// V.1.02 RevNo110308-002 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//==============================================================================
/**
 * 指定アドレスのブロック情報取得
 * @param eAreaType			指定Flashタイプ(MAREA_USER/MAREA_DATA/MAREA_USERBOOT)
 * @param madrStartAddr		指定アドレス
 * @param pdwBlkStartAddr	ブロック先頭アドレス格納ポインタ
 * @param pdwBlkEndAddr		ブロック終了アドレス格納ポインタ
 * @param pdwBlkNo			ブロック番号格納ポインタ
 * @param pdwBlkLength		ブロックバイトサイズ格納ポインタ
 * @retval TRUE:指定アドレスが検索メモリタイプであった 
 * @retval FALSE:指定アドレスが検索メモリタイプではない
 */
//==============================================================================
BOOL GetFlashRomBlockInf( enum FFWRXENM_MAREA_TYPE eAreaType, MADDR madrStartAddr, MADDR *pdwBlkStartAddr, MADDR *pdwBlkEndAddr, DWORD *pdwBlkNo, DWORD *pdwBlkLength)
{
	DWORD	dwMcuRomPtCnt;
	DWORD	dwMcuRomBlkCnt;
	MADDR	dwmadrAreaStart;
	MADDR	dwmadrAreaEnd;
	MADDR	dwmadrBlockStart;
	MADDR	dwmadrBlockEnd;

	DWORD	dwBlockLength;
	DWORD	dwBlockNum;

	DWORD	dwBlkNum;

	FFWMCU_MCUAREA_DATA_RX	*pMcuArea;

	pMcuArea = GetMcuAreaDataRX();

	dwBlkNum = 0;

	// 指定アドレスのブロック情報取得(ユーザーマット領域)
	if( eAreaType == MAREA_USER){
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwFlashRomPatternNum; dwMcuRomPtCnt++) {
			// 領域パターン先頭アドレス算出
			dwmadrAreaStart = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt];	
			// ブロック数を取得
			dwBlockNum = pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt];
			// ブロックサイズを取得
			dwBlockLength = pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt];
			// 領域パターン終了アドレス算出
			dwmadrAreaEnd = dwmadrAreaStart + (dwBlockNum*dwBlockLength) - 1;

			// 指定アドレスが検索領域パターンの範囲内であるか確認				
			if (dwmadrAreaStart <= madrStartAddr && madrStartAddr <= dwmadrAreaEnd) {
				for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < dwBlockNum; dwMcuRomBlkCnt++) {
					// ブロック先頭アドレスを取得
					dwmadrBlockStart = dwmadrAreaStart + (dwMcuRomBlkCnt * dwBlockLength);
					// ブロック終了アドレスを取得
					dwmadrBlockEnd = dwmadrAreaStart + ((dwMcuRomBlkCnt + 1)*dwBlockLength) - 1;
					// 指定アドレスが検索ブロックの範囲内であるか確認				
					if (dwmadrBlockStart <= madrStartAddr && madrStartAddr <= dwmadrBlockEnd) {
						(*pdwBlkStartAddr) = dwmadrBlockStart;	// ブロック先頭アドレスを設定
						(*pdwBlkEndAddr) = dwmadrBlockEnd;		// ブロック終了アドレスを設定
						(*pdwBlkNo) = dwBlkNum;			// ブロック番号を設定
						(*pdwBlkLength) = dwBlockLength;			// ブロックサイズを設定
						return TRUE;
					}
					else{
						// 取得ブロック数を更新
						dwBlkNum++;
					}
				}
			}
			else{
				// 取得ブロック数を更新
				dwBlkNum += dwBlockNum;
			}
		}
	}

	// 指定アドレスのブロック情報取得(データマット領域)
	else if( eAreaType == MAREA_DATA){
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwDataFlashRomPatternNum; dwMcuRomPtCnt++) {
			// 領域パターン先頭アドレス算出
			dwmadrAreaStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt];	
			// ブロック数を取得
			dwBlockNum = pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt];
			// ブロックサイズを取得
			dwBlockLength = pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt];
			// 領域パターン終了アドレス算出
			dwmadrAreaEnd = dwmadrAreaStart + (dwBlockNum*dwBlockLength) - 1;

			// 指定アドレスが検索領域パターンの範囲内であるか確認				
			if (dwmadrAreaStart <= madrStartAddr && madrStartAddr <= dwmadrAreaEnd) {
				for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < dwBlockNum; dwMcuRomBlkCnt++) {
					// ブロック先頭アドレスを取得
					dwmadrBlockStart = dwmadrAreaStart + (dwMcuRomBlkCnt * dwBlockLength);
					// ブロック終了アドレスを取得
					dwmadrBlockEnd = dwmadrAreaStart + ((dwMcuRomBlkCnt + 1)*dwBlockLength) - 1;
					// 指定アドレスが検索ブロックの範囲内であるか確認				
					if (dwmadrBlockStart <= madrStartAddr && madrStartAddr <= dwmadrBlockEnd) {
						(*pdwBlkStartAddr) = dwmadrBlockStart;	// ブロック先頭アドレスを設定
						(*pdwBlkEndAddr) = dwmadrBlockEnd;		// ブロック終了アドレスを設定
						(*pdwBlkNo) = dwBlkNum;			// ブロック番号を設定
						(*pdwBlkLength) = dwBlockLength;			// ブロックサイズを設定
						return TRUE;
					}
					else{
						// 取得ブロック数を更新
						dwBlkNum++;
					}
				}
			}
			else{
				// 取得ブロック数を更新
				dwBlkNum += dwBlockNum;
			}
		}
	}

	// 指定アドレスのブロック情報取得(ユーザーブートマット領域)
	else{
		// RevNo120120-001 ユーザブート領域がない場合のライタモードチェックサム計算処理不具合改修　Modify Line
		if(GetNewCacheMem(MAREA_USERBOOT) == TRUE){
			// ブロック先頭アドレスを取得
			dwmadrBlockStart = pMcuArea->dwmadrUserBootStart;
			// ブロック終了アドレスを取得
			dwmadrBlockEnd = pMcuArea->dwmadrUserBootEnd;

			(*pdwBlkStartAddr) = dwmadrBlockStart;				// ブロック先頭アドレスを設定
			(*pdwBlkEndAddr) = dwmadrBlockEnd;					// ブロック終了アドレスを設定
			(*pdwBlkNo) = 0;									// ブロック番号を設定
			(*pdwBlkLength) = dwmadrBlockEnd - dwmadrBlockStart + 1;	// ブロックサイズを設定
		}else{
			(*pdwBlkStartAddr) = 0;				// ブロック先頭アドレスを設定
			(*pdwBlkEndAddr) = 0;				// ブロック終了アドレスを設定
			(*pdwBlkNo) = 0;					// ブロック番号を設定
			(*pdwBlkLength) = 0;				// ブロックサイズを設定
		}
		// RevNo120120-001 ユーザブート領域がない場合のライタモードチェックサム計算処理不具合改修　Modify End
		return TRUE;
	}
	// 指定アドレスのブロックが検索出来なかった場合
	(*pdwBlkStartAddr) = 0;				// ブロック先頭アドレスを設定
	(*pdwBlkEndAddr) = 0;				// ブロック終了アドレスを設定
	(*pdwBlkNo) = 0;					// ブロック番号を設定
	(*pdwBlkLength) = 0;					// ブロックサイズを設定
	return FALSE;
	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//==============================================================================
/**
 * データマットの書き込みサイズ書き換えフラグの領域情報取得
 * @param madrStartAddr		指定先頭アドレス
 * @param pdwStartBlkNo		指定アドレスのブロック番号格納ポインタ
 * @retval TRUE:指定アドレスがデータマット領域であった 
 * @retval FALSE:指定アドレスがデータマット領域ではない
 */
//==============================================================================
BOOL GetDmProgSizeWriteFlagInf(MADDR madrStartAddr, DWORD *pdwStartBlkNo)
{

	DWORD	dwMcuRomPtCnt;
	DWORD	dwMcuRomBlkCnt;
	MADDR	dwmadrAreaStart;
	MADDR	dwmadrAreaEnd;
	MADDR	dwmadrBlockStart;
	MADDR	dwmadrBlockEnd;

	DWORD	dwBlockLength;			//ブロックのバイトサイズ
	DWORD	dwBlockNum;

	DWORD	dwBlkCnt = 0;			//ブロック数

	FFWMCU_MCUAREA_DATA_RX	*pMcuArea;

	pMcuArea = GetMcuAreaDataRX();

	for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwDataFlashRomPatternNum; dwMcuRomPtCnt++) {
		// 領域パターン先頭アドレス算出
		dwmadrAreaStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt];	
		// ブロック数を取得
		dwBlockNum = pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt];
		// ブロックサイズを取得
		dwBlockLength = pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt];
		// 領域パターン終了アドレス算出
		dwmadrAreaEnd = dwmadrAreaStart + (dwBlockNum*dwBlockLength) - 1;

		// 指定アドレスが検索領域パターンの範囲内であるか確認				
		if (dwmadrAreaStart <= madrStartAddr && madrStartAddr <= dwmadrAreaEnd) {
			for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < dwBlockNum; dwMcuRomBlkCnt++) {
				// ブロック先頭アドレスを取得
				dwmadrBlockStart = dwmadrAreaStart + (dwMcuRomBlkCnt * dwBlockLength);
				// ブロック終了アドレスを取得
				dwmadrBlockEnd = dwmadrAreaStart + ((dwMcuRomBlkCnt + 1)*dwBlockLength) - 1;
				// 指定アドレスが検索ブロックの範囲内であるか確認				
				if (dwmadrBlockStart == madrStartAddr){
					(*pdwStartBlkNo) = dwBlkCnt;
					return TRUE;
				}
				else{
					dwBlkCnt += dwBlockLength / GetCacheProgSize(MAREA_DATA);
				}
			}
		}
		else{
			// 書き込みサイズ単位のブロック数取得
			dwBlkCnt += (dwBlockNum*dwBlockLength) / GetCacheProgSize(MAREA_DATA);
		}
	}
	(*pdwStartBlkNo) = 0;
	return FALSE;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//==============================================================================
/**
 * 指定ブロック情報取得
 * @param eAreaType			指定Flashタイプ(MAREA_USER/MAREA_DATA/MAREA_USERBOOT)
 * @param dwBlockNo			指定ブロック
 * @param pdwBlkStartAddr	ブロック先頭アドレス格納ポインタ
 * @param pdwBlkEndAddr		ブロック先頭アドレス格納ポインタ
 * @param pdwBlkSize		ブロックサイズ格納ポインタ
 * @retval TRUE:指定ブロックが指定Flash領域であった 
 * @retval FALSE:指定ブロックが指定Flash領域ではない
 */
//==============================================================================
BOOL GetFlashRomBlockNoInf(enum FFWRXENM_MAREA_TYPE eAreaType, DWORD dwBlockNo, MADDR *pdwBlkStartAddr, MADDR *pdwBlkEndAddr, DWORD *pdwBlkLength)
{
	DWORD	dwMcuRomPtCnt;
	DWORD	dwMcuRomBlkCnt;
	MADDR	dwmadrAreaStart;
	MADDR	dwmadrBlockStart;
	MADDR	dwmadrBlockEnd;

	DWORD	dwBlockLength;		//ブロックのバイトサイズ
	DWORD	dwBlockNum;
	DWORD	dwBlockCnt;
	DWORD	dwBlockStartNo;
	DWORD	dwBlockEndNo;

	FFWMCU_MCUAREA_DATA_RX	*pMcuArea;

	pMcuArea = GetMcuAreaDataRX();

	// 指定ブロック情報取得(ユーザーマット領域)
	if( eAreaType == MAREA_USER){
		dwBlockCnt = 0;
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwFlashRomPatternNum; dwMcuRomPtCnt++) {
			// 領域パターン先頭アドレス取得
			dwmadrAreaStart = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt];	
			// ブロック数を取得
			dwBlockNum = pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt];
			// ブロックサイズを取得
			dwBlockLength = pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt];
			// 領域パターン先頭ブロック番号取得
			dwBlockStartNo = dwBlockCnt;
			// 領域パターン終了ブロック番号取得
			dwBlockEndNo = dwBlockCnt+(dwBlockNum-1);

			// 指定ブロック番号が検索領域パターンの範囲内であるか確認				
			if (dwBlockStartNo <= dwBlockNo && dwBlockNo <= dwBlockEndNo) {
				for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < dwBlockNum; dwMcuRomBlkCnt++) {
					
					// 指定アドレスが検索ブロックの範囲内であるか確認
					if (dwBlockNo==dwBlockCnt) {
						// ブロック先頭アドレスを取得
						dwmadrBlockStart = dwmadrAreaStart + (dwMcuRomBlkCnt * dwBlockLength);
						// ブロック終了アドレスを取得
						dwmadrBlockEnd = dwmadrAreaStart + ((dwMcuRomBlkCnt + 1)*dwBlockLength) - 1;

						(*pdwBlkStartAddr) = dwmadrBlockStart;	// ブロック先頭アドレスを設定
						(*pdwBlkEndAddr) = dwmadrBlockEnd;		// ブロック終了アドレスを設定
						(*pdwBlkLength) = dwBlockLength;			// ブロックバイトサイズを設定
						return TRUE;
					}
					else{
						dwBlockCnt++;
					}
				}
			}
			else{
				dwBlockCnt += dwBlockNum;
			}
		}
	}
	
	// 指定ブロック情報取得(データマット領域)
	else if( eAreaType == MAREA_DATA){
		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwDataFlashRomPatternNum; dwMcuRomPtCnt++) {
			// 領域パターン先頭アドレス取得
			dwmadrAreaStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt];	
			// ブロック数を取得
			dwBlockNum = pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt];
			// ブロックサイズを取得
			dwBlockLength = pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt];

			// 領域パターン先頭ブロック番号取得
			dwBlockStartNo = dwMcuRomPtCnt*dwBlockNum;	
			// 領域パターン終了ブロック番号取得
			dwBlockEndNo = ((dwMcuRomPtCnt+1)*dwBlockNum)-1;

			// 指定ブロック番号が検索領域パターンの範囲内であるか確認				
			if (dwBlockStartNo <= dwBlockNo && dwBlockNo <= dwBlockEndNo) {
				for (dwMcuRomBlkCnt = 0; dwMcuRomBlkCnt < dwBlockNum; dwMcuRomBlkCnt++) {
					
					// 指定アドレスが検索ブロックの範囲内であるか確認				
					if (dwBlockNo==dwBlockStartNo+dwMcuRomBlkCnt) {
						// ブロック先頭アドレスを取得
						dwmadrBlockStart = dwmadrAreaStart + (dwMcuRomBlkCnt * dwBlockLength);
						// ブロック終了アドレスを取得
						dwmadrBlockEnd = dwmadrAreaStart + ((dwMcuRomBlkCnt + 1)*dwBlockLength) - 1;

						(*pdwBlkStartAddr) = dwmadrBlockStart;	// ブロック先頭アドレスを設定
						(*pdwBlkEndAddr) = dwmadrBlockEnd;		// ブロック終了アドレスを設定
						(*pdwBlkLength) = dwBlockLength;		// ブロックバイトサイズを設定
						return TRUE;
					}
				}
			}
		}
	}

	// 指定ブロック情報取得(ユーザーブートマット領域)
	else{
		// RevNo120120-001 ユーザブート領域がない場合のライタモードチェックサム計算処理不具合改修　Modify Start
		if(GetNewCacheMem(MAREA_USERBOOT) == TRUE){
			// ブロック先頭アドレスを取得
			dwmadrBlockStart = pMcuArea->dwmadrUserBootStart;
			// ブロック終了アドレスを取得
			dwmadrBlockEnd = pMcuArea->dwmadrUserBootEnd;

			(*pdwBlkStartAddr) = dwmadrBlockStart;				// ブロック先頭アドレスを設定
			(*pdwBlkEndAddr) = dwmadrBlockEnd;					// ブロック終了アドレスを設定
			(*pdwBlkLength) = dwmadrBlockEnd - dwmadrBlockStart + 1;	// ブロックサイズを設定
		}else{
			(*pdwBlkStartAddr) = 0;		// ブロック先頭アドレスを設定
			(*pdwBlkEndAddr) = 0;		// ブロック終了アドレスを設定
			(*pdwBlkLength) = 0;			// ブロックサイズを設定
		}
		// RevNo120120-001 ユーザブート領域がない場合のライタモードチェックサム計算処理不具合改修　Modify End
		return TRUE;
	}
	// 指定ブロックが検索出来なかった場合
	(*pdwBlkStartAddr) = 0;		// ブロック先頭アドレスを設定
	(*pdwBlkEndAddr) = 0;		// ブロック終了アドレスを設定
	(*pdwBlkLength) = 0;			// ブロックバイトサイズを設定
	return FALSE;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * データマットの初期化が必要かチェックする。
 * @param  なし
 * @retval TRUE          初期化あり
 * @retval FALSE         初期化なし
 */
//=============================================================================
BOOL ChkDataFlashRomFclr(void)
{
	
	FFW_DM_FCLR_DATA *pDataFclr;	

	pDataFclr = GetDataFclrData();

	// データマットのFCLR情報が登録されているか確認
	if(pDataFclr->dwNum == 0){
		return FALSE;
	}
	return TRUE;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End


// RevRxNo121026-001 Append Start
//=============================================================================
/**
 * データFlash読み出し許可の設定を行う
 * @param  bEnable   TRUE  : データFlash読み出し許可ON
 *                   FALSE : データFlash読み出し許可OFF
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
FFWERR SetRegDataFlashReadEnable(BOOL bEnable)
{
	FFWERR						ferr = FFWERR_OK;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Modify Line
	// RevRxNo130301-001 Modify Start, RevRxNo140515-006 Modify Line
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {			// フラッシュがMF3の場合
		ferr = setRegDataFlashReadEnable_RX100(bEnable);
	} else if (pFwCtrl->eFlashType == RX_FLASH_RC03F) {	// フラッシュがRC03Fの場合
		ferr = setRegDataFlashReadEnable(bEnable);
	} else {											// フラッシュがRV40Fの場合
		;
	}
	// RevRxNo130301-001 Modify End

	return ferr;
}
// RevRxNo121026-001 Append End

// RevRxNo121026-001 Append Start
//=============================================================================
/**
 * データFlash読み出し許可の設定を行う(RX1xx用)
 * @param  bEnable   TRUE  : データFlash読み出し許可ON
 *                   FALSE : データFlash読み出し許可OFF
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
FFWERR setRegDataFlashReadEnable_RX100(BOOL bEnable)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	enum FFWENM_VERIFY_SET		eVerify;			// ベリファイON/OFF
	EADDR						madrRWaddr;			// 参照/設定するレジスタのアドレス
	DWORD						dwWriteDataSize;	// 書き込みデータサイズ
	BYTE						byReadData;			// 参照データを格納する領域のアドレス
	BYTE						byWriteData;		// 設定データを格納する領域のアドレス


	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		// MCU情報構造体のポインタ

	pMcuInfoData = GetMcuInfoDataRX();	// MCU情報を取得

	dwWriteDataSize = 1;
	eVerify = VERIFY_OFF;
	
	if (pMcuInfoData->dwMcuRegInfoBlkNum != 0) {	// MCUコマンドで渡すレジスタ情報がある場合
		if (bEnable == TRUE) {			// 読み出し許可ONの場合
			madrRWaddr = pMcuInfoData->dwmadrMcuRegInfoAddr[0][0];
			eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[0][0];
			ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_byDataFlashEnableData_RX100 = byReadData;

			// レジスタ書き込みデータをセット
			byWriteData = (BYTE)(pMcuInfoData->dwMcuRegInfoData[0][0]);
			ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, &byWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}else{						// 読み出し許可OFFの場合
			// レジスタ書き込みデータをセット
			byWriteData = s_byDataFlashEnableData_RX100;
			s_byDataFlashEnableData_RX100 = 0;
			// エンディアン判定＆取得
			madrRWaddr = pMcuInfoData->dwmadrMcuRegInfoAddr[0][0];
			eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[0][0];
			// レジスタに元の値をセット
			ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, &byWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	return FFWERR_OK;
}
// RevRxNo121026-001 Append End

//=============================================================================
/**
 * データFlash読み出し許可の設定を行う
 * @param  bEnable   TRUE  : データFlash読み出し許可ON
 *                   FALSE : データFlash読み出し許可OFF
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo121026-001 Modify Line
FFWERR setRegDataFlashReadEnable(BOOL bEnable)
{
	// V.1.02 新デバイス対応：MCUコマンドで情報を渡すようにしたことに対して
	// データフラッシュ読み出し許可レジスタはワードアクセス以外ないはずなので、
	// アクセスサイズがBYTE/LWORDの場合は考慮していない。

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRWaddr;			// 参照/設定するレジスタのアドレス
	// V.1.02 RevNo110304-003 Append Line
	WORD						wReadData[1];		// 参照データ格納領域
	WORD						wWriteData[1];		// 設定データ格納領域
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	DWORD						dwCnt;


	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		// MCU情報構造体のポインタ

	pMcuInfoData = GetMcuInfoDataRX();	// MCU情報を取得

	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);
	
	if (pMcuInfoData->dwMcuRegInfoBlkNum != 0) {	// MCUコマンドで渡すレジスタ情報がある場合
		for( dwCnt = 0; dwCnt < pMcuInfoData->dwMcuRegInfoNum[0]; dwCnt++ ){	// データフラッシュ読み出し許可レジスタ数分繰り返す
			if (bEnable == TRUE) {			// 読み出し許可ONの場合
				madrRWaddr = pMcuInfoData->dwmadrMcuRegInfoAddr[0][dwCnt];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[0][dwCnt];
				// V.1.02 RevNo110304-003 Append & Modify Start
				ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT,pbyReadData);
				// V.1.02 RevNo110304-003 Append & Modify End
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_wDataFlashEnableData[dwCnt] = wReadData[0];
				// レジスタ書き込みデータをセット
				// RevRxNo120910-013 Modify Line
				wWriteData[0] = (WORD)((pMcuInfoData->dwMcuRegInfoData[0][dwCnt]) | s_wDflreKeycode[dwCnt]);
				// V.1.02 RevNo110304-003 Modify Line
				ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}else{						// 読み出し許可OFFの場合
				// レジスタ書き込みデータをセット
				// V.1.02 RevNo110308-001 Modify Line
				wWriteData[0] = (WORD)(s_wDataFlashEnableData[dwCnt] | s_wDflreKeycode[dwCnt]);
				s_wDataFlashEnableData[dwCnt] = 0;
				// エンディアン判定＆取得
				madrRWaddr = pMcuInfoData->dwmadrMcuRegInfoAddr[0][dwCnt];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[0][dwCnt];
				// レジスタに元の値をセット
				// V.1.02 RevNo110304-003 Append & Modify Start
				ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
				// V.1.02 RevNo110304-003 Append & Modify End
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}
	}
	return FFWERR_OK;
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End
}

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * フラッシュP/Eモードエントリレジスタ値取得
 * @param  FENTRYRレジスタ値格納変数のポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetFentryrRegData(WORD *pwRegData)
{
	
	FFWERR ferr = FFWERR_OK;
	
	enum FFWENM_MACCESS_SIZE eAccessSize;	// アクセスサイズ
	MADDR	eadrReadAddr;					// レジスタ先頭アドレス
	WORD	wReadData[1];					// レジスタデータ格納領域
	BYTE*	pbyReadData;					// 参照データを格納する領域のアドレス
	// RevRxNo130301-001 Append Line
	FFWMCU_MCUDEF_DATA* pMcuDef;

	// RevRxNo130301-001 Append Line
	pMcuDef = GetMcuDefData();

	eAccessSize = MWORD_ACCESS; // アクセスサイズをワードアクセスに設定

	pbyReadData = reinterpret_cast<BYTE*>(wReadData);

	// RevRxNo130301-001 Modify Line
	eadrReadAddr = pMcuDef->madrFentryr;

	ferr = GetMcuSfrReg(eadrReadAddr,eAccessSize,SFR_ACCESS_COUNT,pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
		
	// FENTRYRレジスタ値を取得
	(*pwRegData) = wReadData[0];

	return ferr;
}

// RevRxNo121026-001 Append Start
//=============================================================================
/**
 * データFlash読み出し許可の設定を行う
 * @param  dwBlockNo ブロック番号
 * @param  pbResult	 読み出し許可レジスタ確認結果格納変数のポインタ　TRUE ： 許可、FALSE：禁止
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
FFWERR GetRegDataFlashReadBlockEnable(DWORD dwBlockNo,BOOL *pbResult)
{
	FFWERR						ferr = FFWERR_OK;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Modify Line
	// RevRxNo130301-001 Modify Start, RevRxNo140515-006 Modify Line
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {			// フラッシュがMF3の場合
		ferr = getRegDataFlashReadBlockEnable_RX100(dwBlockNo,pbResult);
	} else if (pFwCtrl->eFlashType == RX_FLASH_RC03F) {	// フラッシュがRC03Fの場合
		ferr = getRegDataFlashReadBlockEnable(dwBlockNo,pbResult);
	} else {											// フラッシュがRV40Fの場合
		(*pbResult) = (BOOL)TRUE;	// レジスタが存在しないため、読み出し許可情報を設定する
	}
	// RevRxNo130301-001 Modify End

	return ferr;
}
// RevRxNo121026-001 Append End

// RevRxNo121026-001 Append Start
//=============================================================================
/**
 * 対象ブロックのデータフラッシュ許可レジスタ確認(RX1xx用)
 * @param  dwBlockNo ブロック番号
 * @param  pbResult	 読み出し許可レジスタ確認結果格納変数のポインタ　TRUE ： 許可、FALSE：禁止
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
FFWERR getRegDataFlashReadBlockEnable_RX100(DWORD dwBlockNo,BOOL *pbResult)
{
	FFWERR ferr = FFWERR_OK;
	
	enum FFWENM_MACCESS_SIZE eAccessSize;	// アクセスサイズ
	MADDR	eadrRWaddr;						// レジスタ先頭アドレス
	BYTE	byReadData;						// 参照データを格納する領域のアドレス
	BYTE	byEnaMask;						// レジスタデータマスクビット
	BYTE	byBitNo;				  	    // 有効ビット
	BYTE	byBitCnt = 0;					// リード許可確認対象ブロックビット
	DWORD	dwDataFlashBlockOrder;			// データフラッシュブロック番号格納用
	DWORD	dwCount;
	BOOL	bFound = FALSE;
	DWORD	dwDataFlashEnaBlkNum;			// データフラッシュリード許可ビット数

	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		// MCU情報構造体のポインタ
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo130411-001 Append Line

	pMcuInfoData = GetMcuInfoDataRX();	// MCU情報を取得
	pMcuDef = GetMcuDefData();	// RevRxNo130411-001 Append Line

	// データフラッシュ読み出し許可レジスタの確認ビット番号取得
	// RevRxNo130411-001 Modify Line
	dwDataFlashEnaBlkNum = pMcuDef->dwDataFlashEnaBlkNum;	// データフラッシュリード許可レジスタの1ビットに対応するブロック数
	dwDataFlashBlockOrder = dwBlockNo / dwDataFlashEnaBlkNum;

	// 確認するデータフラッシュ読み出し許可レジスタ番号、ビット位置を取得
	byBitNo = 0;
	for (dwCount = 0; dwCount < pMcuInfoData->dwMcuRegInfoNum[0]; dwCount++) {
		byEnaMask = (BYTE)pMcuInfoData->dwMcuRegInfoMask[0][dwCount];
		for (byBitCnt = 0; byBitCnt < 8; byBitCnt++) {
			if (byEnaMask & ((BYTE)0x01 << byBitCnt)) {	// 読み出し許可ビットがある場合
				if (byBitNo == dwDataFlashBlockOrder) {
					bFound = TRUE;
					break;
				}
				byBitNo++;
			} else {									// 読み出し許可ビットがない場合
				break;	// 次の読み出し許可レジスタを確認
			}
		}
		if (bFound == TRUE) {
			break;
		}
	}

	// データフラッシュ読み出し許可レジスタ値リード
	eadrRWaddr = pMcuInfoData->dwmadrMcuRegInfoAddr[0][dwCount];
	eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[0][dwCount];
	ferr = GetMcuSfrReg(eadrRWaddr, eAccessSize,SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if ((byReadData & (0x01 << byBitCnt)) != 0x00) {	// そのブロックが、"読み出し許可"の場合
		(*pbResult) = (BOOL)TRUE;
	} else {										// そのブロックが、"読み出し禁止"の場合
		(*pbResult) = (BOOL)FALSE;
	}

	return ferr;
}

// RevRxNo121026-001 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * 対象ブロックのデータフラッシュ許可レジスタ確認
 * @param  dwBlockNo ブロック番号
 * @param  pbResult	 読み出し許可レジスタ確認結果格納変数のポインタ　TRUE ： 許可、FALSE：禁止
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo121026-001 Modify Line
FFWERR getRegDataFlashReadBlockEnable(DWORD dwBlockNo,BOOL *pbResult)
{
	FFWERR ferr = FFWERR_OK;
	
	enum FFWENM_MACCESS_SIZE eAccessSize;	// アクセスサイズ
	MADDR	eadrRWaddr;						// レジスタ先頭アドレス
	WORD	wReadData[1];					// レジスタデータ格納領域
	BYTE*	pbyReadData;					// 参照データを格納する領域のアドレス
	WORD	wData;							// 有効データ
	// V.1.02 RevNo110603-001 Modify & Append Start
	WORD	wEnaMask;						// レジスタデータマスクビット
	WORD	wBitNo;					  	    // 有効ビット
	WORD	wBitCnt = 0;						// リード許可確認対象ブロックビット
	DWORD	dwDataFlashBlockOrder;			// データフラッシュブロック番号格納用
	DWORD	dwCount;
	BOOL	bFound = FALSE;
	DWORD	dwDataFlashEnaBlkNum;			// データフラッシュリード許可ビット数
	// V.1.02 RevNo110603-001 Modify End

	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		// MCU情報構造体のポインタ
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo130411-001 Append Line

	pMcuInfoData = GetMcuInfoDataRX();	// MCU情報を取得
	pMcuDef = GetMcuDefData();	// RevRxNo130411-001 Append Line

	// V.1.02 RevNo110603-001 Modify Start( 元処理を全削除してE100処理流用 )
	pbyReadData = reinterpret_cast<BYTE*>(wReadData);

	// データフラッシュ読み出し許可レジスタの確認ビット番号取得
	// RevRxNo130411-001 Modify Line
	dwDataFlashEnaBlkNum = pMcuDef->dwDataFlashEnaBlkNum;	// データフラッシュリード許可レジスタの1ビットに対応するブロック数
	dwDataFlashBlockOrder = dwBlockNo / dwDataFlashEnaBlkNum;

	// 確認するデータフラッシュ読み出し許可レジスタ番号、ビット位置を取得
	wBitNo = 0;
	for (dwCount = 0; dwCount < pMcuInfoData->dwMcuRegInfoNum[0]; dwCount++) {
		wEnaMask = (WORD)pMcuInfoData->dwMcuRegInfoMask[0][dwCount];
		for (wBitCnt = 0; wBitCnt < 16; wBitCnt++) {
			if (wEnaMask & ((WORD)0x0001 << wBitCnt)) {	// 読み出し許可ビットがある場合
				if (wBitNo == dwDataFlashBlockOrder) {
					bFound = TRUE;
					break;
				}
				wBitNo++;
			} else {									// 読み出し許可ビットがない場合
				break;	// 次の読み出し許可レジスタを確認
			}
		}
		if (bFound == TRUE) {
			break;
		}
	}

	// データフラッシュ読み出し許可レジスタ値リード
	eadrRWaddr = pMcuInfoData->dwmadrMcuRegInfoAddr[0][dwCount];
	eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[0][dwCount];
	ferr = GetMcuSfrReg(eadrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	wData = wReadData[0];
	if ((wData & (0x0001 << wBitCnt)) != 0x0000) {	// そのブロックが、"読み出し許可"の場合
		(*pbResult) = (BOOL)TRUE;
	} else {										// そのブロックが、"読み出し禁止"の場合
		(*pbResult) = (BOOL)FALSE;
	}
	// V.1.02 RevNo110603-001 Modify End( 元処理を全削除してE100処理流用 )

	return ferr;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

//==============================================================================
/**
 * チェックサム結果を格納する。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR SetResultCheckSum(void)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	i;
	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Modify Start
	DWORD	dwCacheChksum;					// チェックサム値(キャッシュメモリ加算)
	DWORD	dwFlashChksum;					// チェックサム値(実メモリリード結果加算)
	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Modify End
	BYTE*	bBlockData;
	FFWMCU_DBG_DATA_RX*	pDbgData;
	pDbgData = GetDbgDataRX();

	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append Start
	DWORD	dwBlockNum;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	MADDR	madrCacheStart;
	DWORD	dwBlkNo;
	DWORD	dwBlkLength;					// 1ブロックのバイトサイズ
	DWORD	dwBlkLengthTmp;					// RevNo120120-001 Append Line
	// RevNo120314-001 RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正 Append Start
	DWORD	dwProgSizeCnt;					// 1ブロック内書き換え256単位領域チェック用変数
	DWORD	dwProgSizeWriteNum;				// 1ブロックを256バイトで割った個数
	DWORD	dwProgSize;						// 1回のフラッシュ書き換え単位(バイト数) 
	// RevNo120314-001 RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正 Append End
	DWORD	dwAccessCount;
	USER_ROMCACHE_RX		*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;		// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;		// ユーザーブートマットキャッシュメモリ構造体ポインタ
	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();	// ユーザーマットキャッシュメモリ構造体情報取得

	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BYTE						byEndian = 0;
	eAccessSize = MLWORD_ACCESS;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append End

	// RevNo120622-001 RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正 Append Line
	DWORD	dwVerifyOffset;					// 1ブロック内書き換え256単位のベリファイチェックオフセット値

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
// RevRxNo121022-001 Append Start

	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Modify Start
	// ライタモード時
	if (pDbgData->eWriter == EML_WRITERMODE_USE) {
		// ① データフラッシュ領域処理
		dwCacheChksum = 0;
		dwFlashChksum = 0;
		dwBlockNum = GetCacheBlockNum(MAREA_DATA);	// データフラッシュ領域ブロック総数取得
		// データフラッシュ読み出し許可設定
		ferr = SetRegDataFlashReadEnable(TRUE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// RevNo120314-001 RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正 Append Line
		dwProgSize = GetCacheProgSize(MAREA_DATA);	// 書き込みサイズ取得
		for( dwBlkNo = 0; dwBlkNo < dwBlockNum; dwBlkNo++ ){			// ブロック数分繰り返す
			GetFlashRomBlockNoInf(MAREA_DATA, dwBlkNo, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
			// 指定ブロック番号に対応するキャッシュメモリの位置を算出
			madrCacheStart = madrBlkStartAddr - GetCacheStartAddr(MAREA_DATA);
			bBlockData = new BYTE[dwBlkLength];		// １ブロック分のデータ格納バッファ領域確保
			// 1ブロック分のデータリード
			if( dwBlkNo == 0 ){		// 最初の1回だけエンディアンチェックする(2回目以降は不要のため)
				ferr = GetEndianType2(madrBlkStartAddr,&byEndian);
				if (ferr != FFWERR_OK) {
					delete [] bBlockData;
					return ferr;
				}
			}
			dwAccessCount = dwBlkLength/4;

// RevRxNo121022-001 Append Start
			if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
				ferr = McuDumpDivide(dwAreaNum, madrBlkStartAddr, bSameAccessSize, eAccessSize, bSameAccessCount, dwAccessCount, bBlockData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
			} else{
				ferr = PROT_MCU_DUMP(dwAreaNum, &madrBlkStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, bBlockData, byEndian);
			}
// RevRxNo121022-001 Append End

			if (ferr != FFWERR_OK) {
				delete [] bBlockData;
				return ferr;
			}
			// チェックサム計算
			for (i = 0; i < dwBlkLength; i++) {
				dwCacheChksum += dm_ptr->pbyCacheMem[madrCacheStart+i];		// キャッシュメモリのデータを加算
				dwFlashChksum += bBlockData[i];								// 実メモリのデータを加算
			}
			// RevNo120314-001 RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正 Modify Start
			// 書き込みサイズ単位ブロック数を算出　割り切れないdwBlkSizeはこない前提で切り上げ処理削除
			dwProgSizeWriteNum = dwBlkLength/dwProgSize;
			// RevNo120621-004 Delete 1以下の場合の切り上げ処理削除
			if(dm_ptr->pbyBlockWriteFlag[dwBlkNo] == TRUE){		// 書き換えブロックの場合
				for( dwProgSizeCnt = 0; dwProgSizeCnt < dwProgSizeWriteNum; dwProgSizeCnt++ ){
					if (dm_ptr->pbyProgSizeWriteFlag[dwBlkNo*dwProgSizeWriteNum + dwProgSizeCnt ] == TRUE) {
						// RevNo120622-001 RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正 Append Line
						dwVerifyOffset = dwProgSize * dwProgSizeCnt;  //メモリ内の書き込みサイズ単位のベリファイチェックオフセット値計算
						// ベリファイチェック実行
						for (i = 0; i < dwProgSize; i++) {
							// RevNo120622-001 RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正 Modify Line
							if (dm_ptr->pbyCacheMem[madrCacheStart+dwVerifyOffset+i] != bBlockData[dwVerifyOffset+i]) {	// ベリファイチェック
								delete [] bBlockData;
								return FFWERR_WRITE_VERIFY;		// ベリファイエラー
							}
						}
					}
				}
				// RevNo120314-001 RX610/RX62T/RX62Nライタモードベリファイチェック処理不具合修正 Modify End
			}
			delete [] bBlockData;
		}
		s_ChecksumData.byMemType[s_ChecksumData.byCntMem] = 1;		// データフラッシュ
		s_ChecksumData.pdwCheckSum[s_ChecksumData.byCntMem] = dwFlashChksum;
		if (dwCacheChksum == dwFlashChksum) {		// キャッシュと実メモリのチェックサム値が同じ場合
			s_ChecksumData.eCheck[s_ChecksumData.byCntMem] = CHECKSUM_AGREE;
		} else {
			s_ChecksumData.eCheck[s_ChecksumData.byCntMem] = CHECKSUM_NON_AGREE;
		}
		s_ChecksumData.byCntMem += 1;
		// データフラッシュ読み出し許可設定解除
		ferr = SetRegDataFlashReadEnable(FALSE);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// ② プログラムROM領域処理
		dwCacheChksum = 0;
		dwFlashChksum = 0;
		dwBlockNum = GetCacheBlockNum(MAREA_USER);	// プログラムROM領域ブロック総数取得
		for( dwBlkNo = 0; dwBlkNo < dwBlockNum; dwBlkNo++ ){			// ブロック数分繰り返す
			GetFlashRomBlockNoInf(MAREA_USER, dwBlkNo, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
			// 指定ブロック番号に対応するキャッシュメモリの位置を算出
			madrCacheStart = madrBlkStartAddr - GetCacheStartAddr(MAREA_USER);
			bBlockData = new BYTE[dwBlkLength];		// １ブロック分のデータ格納バッファ領域確保
			// 1ブロック分のデータリード
			if( dwBlkNo == 0 ){		// 最初の1回だけエンディアンチェックする(2回目以降は不要のため)
				ferr = GetEndianType2(madrBlkStartAddr,&byEndian);
				if (ferr != FFWERR_OK) {
					delete [] bBlockData;
					return ferr;
				}
			}
			dwAccessCount = dwBlkLength/4;

// RevRxNo121022-001 Append Start
			if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
				ferr = McuDumpDivide(dwAreaNum, madrBlkStartAddr, bSameAccessSize, eAccessSize, bSameAccessCount, dwAccessCount, bBlockData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
			} else{
				ferr = PROT_MCU_DUMP(dwAreaNum, &madrBlkStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, bBlockData, byEndian);
			}
// RevRxNo121022-001 Append End

			if (ferr != FFWERR_OK) {
				delete [] bBlockData;
				return ferr;
			}
			// チェックサム計算
			for (i = 0; i < dwBlkLength; i++) {
				dwCacheChksum += um_ptr->pbyCacheMem[madrCacheStart+i];		// キャッシュメモリのデータを加算
				dwFlashChksum += bBlockData[i];								// 実メモリのデータを加算
			}
			if(um_ptr->pbyBlockWriteFlag[dwBlkNo] == TRUE){		// 書き換えブロックの場合
				// ベリファイチェック実行
				for (i = 0; i < dwBlkLength; i++) {
					if (um_ptr->pbyCacheMem[madrCacheStart+i] != bBlockData[i]) {	// ベリファイチェック
						delete [] bBlockData;
						return FFWERR_WRITE_VERIFY;		// ベリファイエラー
					}
				}
			}
			delete [] bBlockData;
		}
		s_ChecksumData.byMemType[s_ChecksumData.byCntMem] = 3;			// プログラムROM
		s_ChecksumData.pdwCheckSum[s_ChecksumData.byCntMem] = dwFlashChksum;
		if (dwCacheChksum == dwFlashChksum) {		// キャッシュと実メモリのチェックサム値が同じ場合
			s_ChecksumData.eCheck[s_ChecksumData.byCntMem] = CHECKSUM_AGREE;
		} else {
			s_ChecksumData.eCheck[s_ChecksumData.byCntMem] = CHECKSUM_NON_AGREE;
		}
		s_ChecksumData.byCntMem += 1;

		// ③ ユーザブート領域処理
		// RevNo120120-001 ユーザブート領域がない場合のライタモードチェックサム計算処理不具合改修　Modify Start
		if(GetNewCacheMem(MAREA_USERBOOT) == TRUE){
			dwCacheChksum = 0;
			dwFlashChksum = 0;
			dwBlockNum = GetCacheBlockNum(MAREA_USERBOOT);	// ユーザブート領域ブロック総数取得
			GetFlashRomBlockNoInf(MAREA_USERBOOT, 0, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkLength); //指定ブロック情報取得
			// 指定ブロック番号に対応するキャッシュメモリの位置を算出
			madrCacheStart = madrBlkStartAddr - GetCacheStartAddr(MAREA_USERBOOT);
			bBlockData = new BYTE[dwBlkLength];		// １ブロック分のデータ格納バッファ領域確保
			// 1ブロック分のデータリード
			ferr = GetEndianType2(madrBlkStartAddr,&byEndian);
			if (ferr != FFWERR_OK) {
				delete [] bBlockData;
				return ferr;
			}
			dwBlkLengthTmp = dwBlkLength/4;

// RevRxNo121022-001 Append Start
			if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
				ferr = McuDumpDivide(dwAreaNum, madrBlkStartAddr, bSameAccessSize, eAccessSize, bSameAccessCount, dwBlkLengthTmp, bBlockData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
			} else{
				ferr = PROT_MCU_DUMP(dwAreaNum, &madrBlkStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwBlkLengthTmp, bBlockData, byEndian);
			}
// RevRxNo121022-001 Append End

			if (ferr != FFWERR_OK) {
				delete [] bBlockData;
				return ferr;
			}
			// チェックサム計算
			for (i = 0; i < dwBlkLength; i++) {
				dwCacheChksum += ubm_ptr->pbyCacheMem[madrCacheStart+i];	// キャッシュメモリのデータを加算
				dwFlashChksum += bBlockData[i];								// 実メモリのデータを加算
			}
			if(GetUbmBlockWrite() == TRUE){			// 書き換えた場合
				// ベリファイチェック実行
				for (i = 0; i < dwBlkLength; i++) {
					if (ubm_ptr->pbyCacheMem[madrCacheStart+i] != bBlockData[i]) {	// ベリファイチェック
						delete [] bBlockData;
						return FFWERR_WRITE_VERIFY;		// ベリファイエラー
					}
				}
			}
			delete [] bBlockData;
			s_ChecksumData.byMemType[s_ChecksumData.byCntMem] = 2;			// ユーザブート
			s_ChecksumData.pdwCheckSum[s_ChecksumData.byCntMem] = dwFlashChksum;
			if (dwCacheChksum == dwFlashChksum) {		// キャッシュと実メモリのチェックサム値が同じ場合
				s_ChecksumData.eCheck[s_ChecksumData.byCntMem] = CHECKSUM_AGREE;
			} else {
				s_ChecksumData.eCheck[s_ChecksumData.byCntMem] = CHECKSUM_NON_AGREE;
			}
			s_ChecksumData.byCntMem += 1;
		}
		// RevNo120120-001 ユーザブート領域がない場合のライタモードチェックサム計算処理不具合改修　Modify End

		// RevRxNo121122-002 Delete Line

		ClearAllBlockWriteFlg();	// 全ブロック書き換えフラグ/ブロック領域書き換えフラグをクリア

	} else {	// ライタモードでない時
		// ここでは何もしない
	}
	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Modify End
		
	return ferr;
}

//==============================================================================
/**
 * チェックサム結果をクリアする。
 * @param  なし
 * @retval なし
 */
//==============================================================================
void ClrResultCheckSum(void)
{
	int	i;

	s_ChecksumData.byCntMem = 0;
	for (i = 0; i < DWNP_FLASHTYPE_MAX; i++) {
		s_ChecksumData.byMemType[i] = 0;
		s_ChecksumData.eCheck[i] = CHECKSUM_AGREE;
		s_ChecksumData.pdwCheckSum[i] = 0;
	}

	return;
}

//==============================================================================
/**
 * チェックサム結果を取得する。
 * @param  pChecksumData チェックサム結果を取得
 * @retval なし
 */
//==============================================================================
void GetCheckSum(FFW_DWNP_CHKSUM_DATA* pChecksumData)
{
	int	i;

	pChecksumData->byCntMem = s_ChecksumData.byCntMem;

	for (i = 0; i < DWNP_FLASHTYPE_MAX; i++) {
		pChecksumData->byMemType[i] = s_ChecksumData.byMemType[i];
		pChecksumData->eCheck[i] = s_ChecksumData.eCheck[i];
		pChecksumData->pdwCheckSum[i] = s_ChecksumData.pdwCheckSum[i];
	}

	return;
}

//==============================================================================
/**
 * チェックサム変数を取得する。
 * @param  pChecksumData チェックサム結果を取得
 * @retval なし
 */
//==============================================================================
FFW_DWNP_CHKSUM_DATA* GetChecksumDataPt(void)
{
	return &s_ChecksumData;
}

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * 1ブロック分のキャッシュメモリを更新
 * @param  eAreaType 領域指定
 * @param  dwBlkNum ブロック番号
 * @param  madrBlkStartAddr ブロック先頭アドレス
 * @param  madrBlkEndAddr ブロック終了アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR UpdateFlashRomCache(enum FFWRXENM_MAREA_TYPE eAreaType, DWORD dwBlkNo, MADDR madrBlkStartAddr, MADDR madrBlkEndAddr, DWORD dwBlkSize)
{

	FFWERR	ferr = FFWERR_OK;
	MADDR	madrDumpStartAddr;
	MADDR	madrCacheStart;
	DWORD	dwDumpLength;
	DWORD	dwDumpTotalLen;				//DUMPするバイトサイズ
	DWORD	dwDumpCnt;
	DWORD	dwAccessCount;				// アクセスサイズによるアクセス回数
	BYTE	byEndian;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	enum FFWENM_MACCESS_SIZE eTmpAccessSize;
	BYTE*	byBlockBuff;	// RevRxE2LNo141104-001 Append Line

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	DWORD	dwDumpLengthMax = 0;
	getEinfData(&einfData);			// エミュレータ情報取得
// RevRxNo121022-001 Append End

	// RevRxE2LNo141104-001 Modify Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {	// E2/E2 Liteの場合
		dwDumpLengthMax = PROT_DUMP_LENGTH_MAX_E2;
	} else if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// E1/E20の場合
		dwDumpLengthMax = PROT_DUMP_LENGTH_MAX;
	} else {	// EZ-CUBEの場合
		dwDumpLengthMax = PROT_DUMP_LENGTH_MAX_EZ;
	}
	// RevRxE2LNo141104-001 Modify End

	//ワーニング対策
	madrBlkEndAddr;

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;

	if(eAreaType == MAREA_USER){
		um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得

		// キャッシュメモリデータ設定フラグを確認
		if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
			// リードデータ格納メモリ設定
			// RevRxE2LNo141104-001 Modify Line
			byBlockBuff = new BYTE [dwDumpLengthMax];		// dwDumpLengthMax分のバッファ確保

			// ダンプ開始アドレスを設定
			madrDumpStartAddr = madrBlkStartAddr;	
			// ダンプバイト数を初期化
			dwDumpTotalLen = 0;
			// ダンプ回数を初期化
			dwDumpCnt = 0;
			while(dwDumpTotalLen<dwBlkSize) {
				// ダンプサイズを算出
// RevRxNo121022-001 Modify Line
				dwDumpLength = dwDumpLengthMax;
				if(( dwBlkSize - dwDumpTotalLen ) < dwDumpLength ){
					// 残りダンプサイズが512バイト以上の場合残りバイト数に設定
					dwDumpLength = ( dwBlkSize - dwDumpTotalLen );
				}
				dwAccessCount = (dwDumpLength/4);	// ダンプレングスを設定
				eTmpAccessSize = MLWORD_ACCESS;	// ダンプアクセスサイズを設定
				// 指定ブロック番号に対応するキャッシュメモリの位置を算出
				madrCacheStart = madrDumpStartAddr - GetCacheStartAddr(eAreaType);
				ferr = GetEndianType2(madrDumpStartAddr, &byEndian);			// エンディアン判定＆取得
				if (ferr != FFWERR_OK) {
					delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line
					return ferr;
				}
				ferr = PROT_MCU_DUMP(dwAreaNum, &madrDumpStartAddr, bSameAccessSize, &eTmpAccessSize, bSameAccessCount, &dwAccessCount, &byBlockBuff[0], byEndian);
				if (ferr != FFWERR_OK) {
					delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line
					return ferr;
				}
				// 指定ブロック番号に対応するキャッシュメモリの位置を算出
				madrCacheStart = madrDumpStartAddr - GetCacheStartAddr(eAreaType);
				// キャッシュデータ登録
				memcpy(&um_ptr->pbyCacheMem[madrCacheStart], &byBlockBuff[0], dwDumpLength);

				dwDumpTotalLen += dwDumpLength;
				madrDumpStartAddr += dwDumpLength;
			}
			delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line

			// キャッシュメモリデータ設定フラグを設定済みに設定
			um_ptr->pbyCacheSetFlag[dwBlkNo] = TRUE;
		}
	}


	else if(eAreaType == MAREA_DATA){
		dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得

		// キャッシュメモリデータ設定フラグを確認
		if(dm_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
			// リードデータ格納メモリ設定
			// RevRxE2LNo141104-001 Modify Line
			byBlockBuff = new BYTE [dwDumpLengthMax];		// dwDumpLengthMax分のバッファ確保

			// ダンプ開始アドレスを設定
			madrDumpStartAddr = madrBlkStartAddr;	
			// ダンプバイト数を初期化
			dwDumpTotalLen = 0;
			// ダンプ回数を初期化
			dwDumpCnt = 0;
			while(dwDumpTotalLen<dwBlkSize) {
				// ダンプサイズを算出
// RevRxNo121022-001 Modify Line
				dwDumpLength = dwDumpLengthMax;
				if(( dwBlkSize - dwDumpTotalLen ) < dwDumpLength ){
					// 残りダンプサイズが512バイト以上の場合残りバイト数に設定
					dwDumpLength = ( dwBlkSize - dwDumpTotalLen );
				}

				dwAccessCount = (dwDumpLength/4);	// ダンプレングスを設定
				eTmpAccessSize = MLWORD_ACCESS;	// ダンプアクセスサイズを設定
				// 指定ブロック番号に対応するキャッシュメモリの位置を算出
				madrCacheStart = madrDumpStartAddr - GetCacheStartAddr(eAreaType);
				ferr = GetEndianType2(madrDumpStartAddr, &byEndian);			// エンディアン判定＆取得
				if (ferr != FFWERR_OK) {
					delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line
					return ferr;
				}
				ferr = PROT_MCU_DUMP(dwAreaNum, &madrDumpStartAddr, bSameAccessSize, &eTmpAccessSize, bSameAccessCount, &dwAccessCount, &byBlockBuff[0], byEndian);
				if (ferr != FFWERR_OK) {
					delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line
					return ferr;
				}
			
				// 指定ブロック番号に対応するキャッシュメモリの位置を算出
				madrCacheStart = madrDumpStartAddr - GetCacheStartAddr(eAreaType);
				// キャッシュデータ登録
				memcpy(&dm_ptr->pbyCacheMem[madrCacheStart], &byBlockBuff[0], dwDumpLength);

				dwDumpTotalLen += dwDumpLength;
				madrDumpStartAddr += dwDumpLength;
			}
			delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line

			// キャッシュメモリデータ設定フラグを設定済みに設定
			dm_ptr->pbyCacheSetFlag[dwBlkNo] = TRUE;
		}
	}


	else{
		ubm_ptr = GetUserBootMatCacheData();		// ユーザーブートマットキャッシュメモリ構造体情報取得

		// キャッシュメモリデータ設定フラグを確認
		if(GetUbmCacheSet() == FALSE){
			// リードデータ格納メモリ設定
			// RevRxE2LNo141104-001 Modify Line
			byBlockBuff = new BYTE [dwDumpLengthMax];		// dwDumpLengthMax分のバッファ確保

			// ダンプ開始アドレスを設定
			madrDumpStartAddr = madrBlkStartAddr;	
			// ダンプバイト数を初期化
			dwDumpTotalLen = 0;
			// ダンプ回数を初期化
			dwDumpCnt = 0;
			while(dwDumpTotalLen<dwBlkSize) {
				// ダンプサイズを算出
// RevRxNo121022-001 Modify Line
				dwDumpLength = dwDumpLengthMax;
				if(( dwBlkSize - dwDumpTotalLen ) < dwDumpLength ){
					// 残りダンプサイズが512バイト以上の場合残りバイト数に設定
					dwDumpLength = ( dwBlkSize - dwDumpTotalLen );
				}
				dwAccessCount = (dwDumpLength/4);	// ダンプレングスを設定
				eTmpAccessSize = MLWORD_ACCESS;	// ダンプアクセスサイズを設定
				// 指定ブロック番号に対応するキャッシュメモリの位置を算出
				madrCacheStart = madrDumpStartAddr - GetCacheStartAddr(eAreaType);
				ferr = GetEndianType2(madrDumpStartAddr, &byEndian);			// エンディアン判定＆取得
				if (ferr != FFWERR_OK) {
					delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line
					return ferr;
				}
				ferr = PROT_MCU_DUMP(dwAreaNum, &madrDumpStartAddr, bSameAccessSize, &eTmpAccessSize, bSameAccessCount, &dwAccessCount, &byBlockBuff[0], byEndian);
				if (ferr != FFWERR_OK) {
					delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line
					return ferr;
				}
				// 指定ブロック番号に対応するキャッシュメモリの位置を算出
				madrCacheStart = madrDumpStartAddr - GetCacheStartAddr(eAreaType);
				// キャッシュデータ登録
				memcpy(&ubm_ptr->pbyCacheMem[madrCacheStart], &byBlockBuff[0], dwDumpLength);

				dwDumpTotalLen += dwDumpLength;
				madrDumpStartAddr += dwDumpLength;
			}
			delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line

			SetUbmCacheSet(TRUE);	// キャッシュメモリデータ設定フラグを設定済みに設定
		}
	}

	return ferr;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End


// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * Extra領域用キャッシュメモリを更新
 * @param  なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR UpdateExtraCache(void)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	madrDumpStartAddr;
	DWORD	dwTotalLength;
	DWORD	dwDumpLength;
	DWORD	dwDumpLengthMax;
	DWORD	dwCacheStart;
	DWORD	dwAccessCount;
	BYTE	byEndian;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	enum FFWENM_MACCESS_SIZE eAccessSize;
	FFWE20_EINF_DATA	einfData;
	BYTE*	byBlockBuff;	// RevRxE2LNo141104-001 Modify Line
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line

	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	getEinfData(&einfData);			// エミュレータ情報取得

	// 最大DUMPバイト数設定
	// RevRxE2LNo141104-001 Modify Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {	// E2/E2 Liteの場合
		dwDumpLengthMax = PROT_DUMP_LENGTH_MAX_E2;
	} else if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// E1/E20の場合
		dwDumpLengthMax = PROT_DUMP_LENGTH_MAX;
	} else {	// EZ-CUBEの場合
		dwDumpLengthMax = PROT_DUMP_LENGTH_MAX_EZ;
	}
	// RevRxE2LNo141104-001 Modify End

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;

	// キャッシュメモリにデータが未設定の場合
	if (GetExtraCacheSet() == FALSE) {
		// RevRxE2LNo141104-001 Append Line
		byBlockBuff = new BYTE [dwDumpLengthMax];		// dwDumpLengthMax分のバッファ確保

		madrDumpStartAddr = pMcuDef->madrExtraACacheStart;	// DUMP開始アドレス設定	// RevRxNo150827-002 Modify Line
		dwTotalLength = pMcuDef->dwExtraASize;				// DUMPバイト数設定		// RevRxNo150827-002 Modify Line
		eAccessSize = MLWORD_ACCESS;					// アクセスサイズ設定
		dwCacheStart = 0;								// キャッシュ位置設定

		ferr = GetEndianType2(madrDumpStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line
			return ferr;
		}

		while (dwTotalLength > 0) {
			if (dwTotalLength > dwDumpLengthMax) {
				dwDumpLength = dwDumpLengthMax;
			} else {
				dwDumpLength = dwTotalLength;
			}
			dwAccessCount = dwDumpLength / 4;	// アクセス回数設定

			// Extra領域をDUMP
			ferr = PROT_MCU_DUMP(dwAreaNum, &madrDumpStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, &byBlockBuff[0], byEndian);
			if (ferr != FFWERR_OK) {
				delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line
				return ferr;
			}

			// キャッシュにDUMPデータを設定
			memcpy(&s_byExtraCacheMem[dwCacheStart], &byBlockBuff[0], dwDumpLength);

			madrDumpStartAddr += dwDumpLength;	// DUMP開始アドレス更新
			dwTotalLength -= dwDumpLength;		// DUMPバイト数更新
			dwCacheStart += dwDumpLength;		// キャッシュ位置更新
		}
		delete [] byBlockBuff;	// RevRxE2LNo141104-001 Append Line

		SetExtraCacheSet(TRUE);	// キャッシュメモリデータ設定済みに設定
	}

	return ferr;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * Extra領域用キャッシュメモリデータ設定状態フラグの設定
 * @param  bCacheSet Extra領域用キャッシュメモリデータ設定状態フラグの設定値
 * @retval なし
 */
//=============================================================================
void SetExtraCacheSet(BOOL bCacheSet)
{
	s_bExtraCacheSetFlag = bCacheSet;

	return;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * Extra領域用キャッシュメモリデータ設定状態フラグの参照
 * @param  なし
 * @retval  Extra領域用キャッシュメモリデータ設定状態フラグ値
 */
//=============================================================================
BOOL GetExtraCacheSet(void)
{
	return s_bExtraCacheSetFlag;
}
// RevRxNo140109-001 Append End


// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * WRITE/FILL時の指定アドレスのキャッシュメモリ情報を更新
 * @param  eAreaType 領域指定
 * @param  madrStartAddr ブロック先頭アドレス
 * @param  dwAccessCount アクセスサイズによるアクセス回数
 * @param  eAccessSize アクセスサイズ
 * @param  pbyWriteBuff ライトデータ格納バッファ
 * @param  dwWriteDataSize 書き込みデータサイズ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetFlashRomCacheMem(enum FFWRXENM_MAREA_TYPE eAreaType, MADDR madrStartAddr, DWORD dwAccessCount, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, DWORD dwWriteDataSize)
{
	
	FFWERR	ferr = FFWERR_OK;
	// V.1.02 RevNo110309-002 Append Line
	FFWERR	ferr2 = FFWERR_OK;
	const BYTE*	pbyBuff;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameLength;
	// RevRxNo121219-001 Append Line
	BOOL	bDmBlkUpdate;		//ブロック更新フラグ
	DWORD	dwDataLength;		//データのバイト長
	DWORD	i;
	DWORD	dwBlkDataCnt = 0;
	DWORD	dwTmpStartAddr;
	DWORD	dwTmpEndAddr;
	// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Line
	DWORD	dwTmpWriteNum;

	MADDR madrBlkStartAddr;
	MADDR madrBlkEndAddr;
	DWORD dwBlkNo;
	DWORD dwBlkSize;
	// RevNo110216-002 Append Start
	MADDR madrBlkStartAddrFclr;
	MADDR madrBlkEndAddrFclr;
	DWORD dwBlkNoFclr;
	DWORD dwBlkSizeFclr;
	// RevNo110216-002 Append Start
	MADDR madrCacheStart;
	DWORD dwStartWriteSizeBlkNo;
	DWORD dwCnt;	// RevRxNo140109-001 Append Line

	DWORD dwFclrCnt;

	DWORD	dwLength;			// RevRxNo140616-001 Append Line
	BYTE	byAccMeans;			// RevRxNo140616-001 Append Line
	BYTE	byAccType;			// RevRxNo140616-001 Append Line
	BOOL	bDwnpEna;			// RevRxNo140616-001 Append Line

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

	FFW_UM_FCLR_DATA *pUserFclrData;	
	FFW_DM_FCLR_DATA *pDataFclrData;	

	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーマットキャッシュメモリ構造体情報取得

	pUserFclrData = GetUserFclrData();
	pDataFclrData = GetDataFclrData();

	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameLength = FALSE;
	pbyBuff = pbyWriteBuff;

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwDataLength = 1;
		break;
	case MWORD_ACCESS:
		dwDataLength = 2;
		break;
	case MLWORD_ACCESS:
		dwDataLength = 4;
		break;
	default:
		dwDataLength = 4;
		break;
	}


	// RevNo110210-001 Append Line
	// ここで以下を実行しないと、プログラム実行前のWriteFlashEnd()で実行している
	// GetDataFlashEraseFlag() == FALSE だったらClearAllBlockWriteFlg()を実行するの処理で、
	// GetDataFlashEraseFlag()がTRUEになっている可能性があり(初期化してないので)、
	// 次回以降のプログラム実行で不要なフラッシュ書き換えが発生してしまう危険性があるため。
	SetDataFlashEraseFlag(FALSE);				// データフラッシュ初期化対象ブロック初期化処理ではない


	// V.1.02 RevNo110324-002 Append Start
	// DWNP中の初期化ブロックキャッシュ初期化処理はダウンロードデータの領域にかかわらず実行しておかなければならない
	if (GetDwnpOpenData() == TRUE) {	// DWNPコマンド実行の場合
		// ユーザマット：FCLR対象領域のキャッシュメモリのデータを初期化をまだ実行していない場合	
		if(GetUmCacheDataInit()==FALSE){
			um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
			// FCLR対象ブロックを検索
			// RevNo110216-002 Modify Start
			for(dwFclrCnt=0;dwFclrCnt<pUserFclrData->dwNum; dwFclrCnt++){
				// 対象ブロック情報取得
				GetFlashRomBlockInf( MAREA_USER, pUserFclrData->dwmadrBlkStart[dwFclrCnt], &madrBlkStartAddrFclr, &madrBlkEndAddrFclr, &dwBlkNoFclr, &dwBlkSizeFclr);

				// RevRxNo140616-001 Append Start
				// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
				// そのため、以後の処理に不都合が生じることはない。
				bDwnpEna = FALSE;
				ChkAreaAccessMethod(madrBlkStartAddrFclr, madrBlkEndAddrFclr, &dwLength, &byAccMeans, &byAccType);
				if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
					// アクセス属性がリード/ライト可またはライトのみ可の場合

					bDwnpEna = TRUE;
				}
				// RevRxNo140616-001 Append End

				// RevRxNo140616-001 Modify Start
				if (bDwnpEna == TRUE) {
					// 指定ブロック番号に対応するキャッシュメモリの位置を算出
					madrCacheStart = madrBlkStartAddrFclr - GetCacheStartAddr(MAREA_USER);
					// キャッシュデータ登録
					memset(&um_ptr->pbyCacheMem[madrCacheStart], 0xFF, dwBlkSizeFclr);
					um_ptr->pbyCacheSetFlag[dwBlkNoFclr] = TRUE;	// キャッシュメモリデータ設定フラグを設定済みに設定
				}
				// RevRxNo140616-001 Modify End
			}
			// RevNo110216-002 Modify End
			SetUmCacheDataInit(TRUE);
		}
		// データマット：初期化未実行の場合
		if(GetDataFlashInitFlag() == FALSE){
			dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
			// FCLR対象ブロックにデータマットブロックが含まれる場合
			if(ChkDataFlashRomFclr() == TRUE){
				// データマットのみの初期化実行フラグをセット
				SetDataFlashEraseFlag(TRUE);
				// フラッシュROMへのライト開始処理
				ferr = WriteFlashStart(VERIFY_OFF);

				// フラッシュROMへのライト終了処理
				// V.1.02 RevNo110309-002 Modify Line
				ferr2 = WriteFlashEnd();
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				// V.1.02 RevNo110309-002 Append Start
				if (ferr2 != FFWERR_OK) {	// WriteFlashStart()でエラーが出ていない場合でWriteFlashEnd()でエラーの場合
					return ferr2;			// WriteFlashEnd()のエラーを返す
				}
				// V.1.02 RevNo110309-002 Append End
				SetDataFlashInitFlag(TRUE);	// データマット初期化フラグを実行済みに設定
				SetDataFlashEraseFlag(FALSE);	// データマットのみの初期化実行フラグをクリア
				// 初期化ブロックのみキャッシュメモリデータ設定フラグを未設定に設定する必要あり
				// FCLR対象ブロックか検索
				for(dwFclrCnt=0; dwFclrCnt<pDataFclrData->dwNum; dwFclrCnt++){
					// 対象ブロック情報取得
					// V.1.02 RevNo110601-001 Modify Line( MAREA_USER → MAREA_DATA )
					GetFlashRomBlockInf( MAREA_DATA, pDataFclrData->dwmadrBlkStart[dwFclrCnt], &madrBlkStartAddrFclr, &madrBlkEndAddrFclr, &dwBlkNoFclr, &dwBlkSizeFclr);

					// RevRxNo140616-001 Append Start
					// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
					// そのため、以後の処理に不都合が生じることはない。
					bDwnpEna = FALSE;
					ChkAreaAccessMethod(madrBlkStartAddrFclr, madrBlkEndAddrFclr, &dwLength, &byAccMeans, &byAccType);
					if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
						// アクセス属性がリード/ライト可またはライトのみ可の場合

						bDwnpEna = TRUE;
					}
					// RevRxNo140616-001 Append End

					// RevRxNo140616-001 Modify Start
					if (bDwnpEna == TRUE) {
						dm_ptr->pbyCacheSetFlag[dwBlkNoFclr] = FALSE;	// キャッシュメモリデータ設定フラグを未設定に設定
					}
					// RevRxNo140616-001 Modify End
				}
			}
		}
		// ユーザブートマット：FCLR対象領域のキャッシュメモリのデータを初期化をまだ実行していない場合	
		if(GetUbmCacheDataInit()==FALSE){
			ubm_ptr = GetUserBootMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
			madrCacheStart = 0;
			madrBlkStartAddr = GetCacheStartAddr(MAREA_USERBOOT);
			madrBlkEndAddr = GetCacheEndAddr(MAREA_USERBOOT);
			dwBlkSize = madrBlkEndAddr - madrBlkStartAddr + 1;

			// RevRxNo140616-001 Append Start
			// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
			// そのため、以後の処理に不都合が生じることはない。
			bDwnpEna = FALSE;
			ChkAreaAccessMethod(madrBlkStartAddr, madrBlkEndAddr, &dwLength, &byAccMeans, &byAccType);
			if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
				// アクセス属性がリード/ライト可またはライトのみ可の場合

					bDwnpEna = TRUE;
			}
			// RevRxNo140616-001 Append End

			// RevRxNo140616-001 Modify Start
			if (bDwnpEna == TRUE) {
				// FCLR対象ブロック
				if(chkFlashRomInitBlock(MAREA_USERBOOT, madrBlkStartAddr, madrBlkEndAddr) == TRUE){
					// キャッシュメモリを初期化
					memset(&ubm_ptr->pbyCacheMem[madrCacheStart], 0xFF, dwBlkSize);
					SetUbmCacheSet(TRUE);	// キャッシュメモリデータ設定フラグを設定済みに設定
					SetUbmCacheDataInit(TRUE);
				}
			}
			// RevRxNo140616-001 Modify End
		}
	}
	// V.1.02 RevNo110324-002 Append End

	// FILLでアクセスサイズと書込みデータサイズが異なる場合
	if ((dwWriteDataSize > 0) && (dwWriteDataSize != dwDataLength)) {
		dwTmpWriteNum = dwAccessCount * dwDataLength;	// いったんバイト単位のFILL回数に戻す
		dwDataLength = dwWriteDataSize;			// 書込みデータサイズをデータ長にする
		dwAccessCount = dwTmpWriteNum / dwDataLength;	// 書込みデータサイズに合わせたFILL回数にする
	}

	
	///// 以下、ライトデータのキャッシュメモリへのマージ処理 /////
	// ユーザーマット キャッシュメモリデータ設定処理
	if(eAreaType == MAREA_USER){
		um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
		
		// 対象ブロック情報取得
		GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

		if (GetDwnpOpenData() == FALSE) {		// DWNPコマンド実行中でない場合
			// 1ブロック分のキャッシュメモリを更新
			ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ライトデータマージ
			// V.1.02 RevNo110408-001 Append Start
			while (dwAccessCount) {
				if (madrBlkStartAddr <= madrStartAddr && madrStartAddr <= madrBlkEndAddr) {
					// 開始位置を取得
					madrCacheStart = madrStartAddr - GetCacheStartAddr(eAreaType);
					for (i=0; i < dwDataLength; i++){		// アクセスサイズ分キャッシュメモリ内容とライトデータ比較
						// キャッシュの内容とライトデータが異なる場合のみキャッシュへライトデータマージ
						if( um_ptr->pbyCacheMem[madrCacheStart+i] != *(pbyBuff+i) ){
							um_ptr->pbyCacheMem[madrCacheStart+i] = *(pbyBuff+i);
							um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;		// ブロック書き換えフラグを書き換えありに設定
							um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定
						}
					}
					dwAccessCount--;
					madrStartAddr += dwDataLength;
					// WRITE/CWRITEの場合
					if (dwWriteDataSize == 0) {
						pbyBuff += dwDataLength;	// アドレス進める
					}
				}else{
					break;	// whileループをBreak
				}
			}
			// V.1.02 RevNo110408-001 Append End
		}else{								// DWNPコマンド実行中の場合
			// V.1.02 RevNo110324-002 Delete
			// FCLR対象領域のキャッシュメモリデータの初期化処理	

			// 1ブロック分のキャッシュメモリを更新
			ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ライトデータマージ
			// V.1.02 RevNo110408-001 Move Start
			while (dwAccessCount) {
				if (madrBlkStartAddr <= madrStartAddr && madrStartAddr <= madrBlkEndAddr) {
					// 開始位置を取得
					madrCacheStart = madrStartAddr - GetCacheStartAddr(eAreaType);
					memcpy(&um_ptr->pbyCacheMem[madrCacheStart], pbyBuff, dwDataLength);
					um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;	// ブロック書き換えフラグを書き換えありに設定
					um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定
					dwAccessCount--;
					madrStartAddr += dwDataLength;
					// WRITE/CWRITEの場合
					if (dwWriteDataSize == 0) {
						pbyBuff += dwDataLength;	// アドレス進める
					}
				}else{
					break;	// whileループをBreak
				}
			}
			// V.1.02 RevNo110408-001 Move End
		}
	}

	// データマット キャッシュメモリデータ設定処理
	else if(eAreaType == MAREA_DATA){
		dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得

		// 対象ブロック情報取得
		GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

		// DWNPコマンド実行中でない場合
		if (GetDwnpOpenData() == FALSE) {
			// 1ブロック分のキャッシュメモリを更新
			ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// DWNPコマンド実行中の場合
		else{
			// V.1.02 RevNo110324-002 Delete
			// データマット初期化処理

			// 1ブロック分のキャッシュメモリを更新
			ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		
		// ライトデータマージ
		dwBlkDataCnt = dwBlkSize / GetCacheProgSize(eAreaType);		// 書き込みサイズ単位でのブロック数取得
		// 書き込みサイズ単位書き込みブロック番号を取得
		GetDmProgSizeWriteFlagInf(madrBlkStartAddr,&dwStartWriteSizeBlkNo);
		
		// RevRxNo121219-001 Append Line
		bDmBlkUpdate = FALSE;			// ブロック更新フラグを初期化する

		for (i = 0; i < dwBlkDataCnt; i++) {
			dwTmpStartAddr = madrBlkStartAddr + (GetCacheProgSize(eAreaType) * i);
			dwTmpEndAddr = (dwTmpStartAddr + GetCacheProgSize(eAreaType)) - 1;
			while (dwAccessCount) {
				if (dwTmpStartAddr <= madrStartAddr && madrStartAddr <= dwTmpEndAddr) {
					// 開始位置を取得
					madrCacheStart = madrStartAddr - GetCacheStartAddr(eAreaType);
					memcpy(&dm_ptr->pbyCacheMem[madrCacheStart], pbyBuff, dwDataLength);
					dm_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;	// ブロック書き換えフラグを書き換えありに設定
					dm_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定
					dm_ptr->pbyProgSizeWriteFlag[dwStartWriteSizeBlkNo+i] = TRUE;	// 書き込みサイズ単位書き込みフラグを書き込みありに設定

					// RevRxNo121219-001 Append Start
					if(chkFlashRomInitBlock(MAREA_DATA, madrBlkStartAddr, madrBlkEndAddr) == FALSE){
						// ブロック開始アドレスが上書き対象ブロックの場合

						bDmBlkUpdate = TRUE;			// ブロック更新フラグをTRUEにする
					}
					// RevRxNo121219-001 Append End

					dwAccessCount--;
					madrStartAddr += dwDataLength;

					// WRITE/CWRITEの場合
					if (dwWriteDataSize == 0) {
						pbyBuff += dwDataLength;	// アドレス進める
					}
				}
				else{
					break;	// whileループをBreak
				}
			}
		}

		// RevRxNo121219-001 Append Start
		if (bDmBlkUpdate == TRUE) {			// ブロック更新フラグがTRUEの場合
			for (i = 0; i < dwBlkDataCnt; i++) {
				dm_ptr->pbyProgSizeWriteFlag[dwStartWriteSizeBlkNo+i] = TRUE;	// 書き込みサイズ単位書き込みフラグを書き込みありに設定
			}
		}
		// RevRxNo121219-001 Append End
	}

	// ユーザーブートマット キャッシュメモリデータ設定処理
	// RevRxNo140109-001 Modify Line
	else if (eAreaType == MAREA_USERBOOT) {
		ubm_ptr = GetUserBootMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得

		// 対象ブロック情報取得
		GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

		if (GetDwnpOpenData() == FALSE) {	// DWNPコマンド実行中でない場合
			// 1ブロック分のキャッシュメモリを更新
			ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ライトデータマージ
			// V.1.02 RevNo110408-001 Append Start
			while (dwAccessCount) {
				if (madrBlkStartAddr <= madrStartAddr && madrStartAddr <= madrBlkEndAddr) {
					// 開始位置を取得
					madrCacheStart = madrStartAddr - GetCacheStartAddr(eAreaType);
					for( i=0; i<dwDataLength; i++ ){		// アクセスサイズ分キャッシュメモリ内容とライトデータ比較
						// キャッシュの内容とライトデータが異なる場合のみキャッシュへライトデータマージ
						if( ubm_ptr->pbyCacheMem[madrCacheStart+i] != *(pbyBuff+i) ){
							ubm_ptr->pbyCacheMem[madrCacheStart+i] = *(pbyBuff+i);
							SetUbmBlockWrite(TRUE);	// ブロック書き換えフラグを書き換えありに設定
						}
					}
					dwAccessCount--;
					madrStartAddr += dwDataLength;
					// WRITE/CWRITEの場合
					if (dwWriteDataSize == 0) {
						pbyBuff += dwDataLength;	// アドレス進める
					}
				}else{
					break;	// whileループをBreak
				}
			}
			// V.1.02 RevNo110408-001 Append End
		}else{								// DWNPコマンド実行中の場合
			// V.1.02 RevNo110324-002 Delete
			// FCLR対象領域のキャッシュメモリデータの初期化処理	

			// 1ブロック分のキャッシュメモリを更新
			ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ライトデータマージ
			// V.1.02 RevNo110408-001 Move Start
			while (dwAccessCount) {
				if (madrBlkStartAddr <= madrStartAddr && madrStartAddr <= madrBlkEndAddr) {
					// 開始位置を取得
					madrCacheStart = madrStartAddr - GetCacheStartAddr(eAreaType);
					memcpy(&ubm_ptr->pbyCacheMem[madrCacheStart], pbyBuff, dwDataLength);
					SetUbmBlockWrite(TRUE);	// ブロック書き換えフラグを書き換えありに設定
					dwAccessCount--;
					madrStartAddr += dwDataLength;
					// WRITE/CWRITEの場合
					if (dwWriteDataSize == 0) {
						pbyBuff += dwDataLength;	// アドレス進める
					}
				}else{
					break;	// whileループをBreak
				}
			}
			// V.1.02 RevNo110408-001 Move End
		}

	// RevRxNo140109-001 Append Start
	// コンフィギュレーション設定領域 キャッシュメモリデータ設定処理
	} else {
		// 対象ブロック情報取得
		madrBlkStartAddr = pMcuDef->madrExtraACacheStart;		// RevRxNo150827-002 Modify Line
		madrBlkEndAddr =pMcuDef->madrExtraACacheStart + pMcuDef->dwExtraASize - 1;	// RevRxNo150827-002 Modify Line

		// キャッシュメモリを更新
		ferr = UpdateExtraCache();
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// ライトデータマージ
		while (dwAccessCount) {
			if ((madrBlkStartAddr <= madrStartAddr) && (madrStartAddr <= madrBlkEndAddr)) {
				// 開始位置を取得
				madrCacheStart = madrStartAddr - pMcuDef->madrExtraACacheStart;		// RevRxNo150827-002 Modify Line
				memcpy(&s_byExtraCacheMem[madrCacheStart], pbyBuff, dwDataLength);

				dwCnt = madrCacheStart / EXTRA_A_PROG_SIZE;
				s_bExtraWriteFlag[dwCnt] = TRUE;	// Extra領域の16バイト単位での書き換えフラグをセット
				s_bExtraAreaWriteFlag = TRUE;		// Extra領域書き換えフラグをセット

				dwAccessCount--;
				madrStartAddr += dwDataLength;
				// WRITE/CWRITEの場合
				if (dwWriteDataSize == 0) {
					pbyBuff += dwDataLength;	// アドレス進める
				}
			}else{
				break;	// whileループをBreak
			}
		}
	// RevRxNo140109-001 Append End
	}


	// RevRxNo140109-001 Append Line(各領域のキャッシュメモリデータ設定処理内で実施していたのをここへ移動)
	maskIdCodeArea();	// IDコード領域のマスク設定

	return ferr;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End


//=============================================================================
/**
 * フラッシュROMキャッシュ変更状態を取得する。
 * @param  なし
 * @retval TRUE     キャッシュ変更あり
 * @retval FALSE    キャッシュ変更なし
 */
//=============================================================================
BOOL GetStateFlashRomCache(void)
{
	
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	DWORD	dwCnt;
	DWORD	dwBlkNum;
	DWORD	dwBlkAreaNum;
	DWORD	dwRestBlkNum;

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーブートマットキャッシュメモリ構造体情報取得

	// ユーザ領域キャッシュ変更状態確認
	dwBlkNum = GetCacheBlockNum(MAREA_USER);	// ブロック総数取得
	dwBlkAreaNum = dwBlkNum/CACHE_BLOCK_AREA_NUM_RX;
	dwRestBlkNum = dwBlkNum%CACHE_BLOCK_AREA_NUM_RX;
	if(dwRestBlkNum > 0){
		dwBlkAreaNum++;
	}
	for(dwCnt=0; dwCnt< dwBlkAreaNum; dwCnt++){
		if(um_ptr->pbyBlockAreaWriteFlag[dwCnt] == TRUE){
			return TRUE;
		}
	}

	// データ領域キャッシュ変更状態確認
	dwBlkNum = GetCacheBlockNum(MAREA_DATA);	// ブロック総数取得
	dwBlkAreaNum = dwBlkNum/CACHE_BLOCK_AREA_NUM_RX;
	dwRestBlkNum = dwBlkNum%CACHE_BLOCK_AREA_NUM_RX;
	if(dwRestBlkNum > 0){
		dwBlkAreaNum++;
	}
	for(dwCnt=0; dwCnt< dwBlkAreaNum; dwCnt++){
		if(dm_ptr->pbyBlockAreaWriteFlag[dwCnt] == TRUE){
			return TRUE;
		}
	}

	// ユーザブート領域キャッシュ変更状態確認
	if(GetUbmBlockWrite() == TRUE){
		return TRUE;
	}

	// RevRxNo140109-001 Append Start
	// Extra領域キャッシュ変更状態確認
	if (s_bExtraAreaWriteFlag == TRUE) {
		return TRUE;
	}
	// RevRxNo140109-001 Append End

	return FALSE;
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

}

//=============================================================================
/**
 * リセットベクタのキャッシュデータを取得
 * @param  ePmode MCU動作モード　　RX_PMODE_USRBOOT:ユーザーブート RX_PMODE_SNG:シングルチップモード
 * @retval FFWERR FFWエラー
 */
//=============================================================================
FFWERR GetFlashCacheRestVect(enum FFWRXENM_PMODE ePmode)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	MADDR	madrCacheStartAddr;
	USER_ROMCACHE_RX		*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;		// ユーザーブートマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();	// ユーザーマットキャッシュメモリ構造体情報取得

	// 最終ブロックだけは起動時に実メモリを取得して入れておく必要有り
	GetFlashRomBlockInf(MAREA_USER, MCU_OSM_MDES_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロック情報取得
	madrCacheStartAddr = GetCacheStartAddr(MAREA_USER);
	if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
		ferr =UpdateFlashRomCache(MAREA_USER, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}
	if( ePmode == RX_PMODE_USRBOOT ){		// ユーザブート起動の場合
		GetFlashRomBlockInf(MAREA_USERBOOT, MCU_OSM_MDEB_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロック情報取得
		madrCacheStartAddr = GetCacheStartAddr(MAREA_USERBOOT);
		if(GetUbmCacheSet() == FALSE){		//　ユーザブートマット領域のデータをキャッシュに未取得の場合 
			ferr = UpdateFlashRomCache(MAREA_USERBOOT, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if(ferr != FFWERR_OK){
				return ferr;
			}
		}
	}

	return ferr;
}

// RevRxNo170511-001 Append Start
//=============================================================================
/**
* デュアルモード時リセットベクタのキャッシュデータを取得
* @param  ePmode MCU動作モード　　RX_PMODE_USRBOOT:ユーザーブート RX_PMODE_SNG:シングルチップモード
* @retval FFWERR FFWエラー
*/
//=============================================================================
FFWERR GetFlashCacheRestVectDual(enum FFWRXENM_PMODE ePmode)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	MADDR	madrCacheStartAddr;
	USER_ROMCACHE_RX		*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;		// ユーザーブートマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();	// ユーザーマットキャッシュメモリ構造体情報取得

	// 最終ブロックだけは起動時に実メモリを取得して入れておく必要有り
	GetFlashRomBlockInf(MAREA_USER, MCU_OSM_MDES_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロック情報取得
	madrCacheStartAddr = GetCacheStartAddr(MAREA_USER);
	// 強制的に最終ブロックのキャッシュを取得する
	um_ptr->pbyCacheSetFlag[dwBlkNo] = FALSE;
	ferr = UpdateFlashRomCache(MAREA_USER, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if (ePmode == RX_PMODE_USRBOOT) {		// ユーザブート起動の場合
		// 現状デュアルモードが存在するMCUでユーザブート起動できるものはないので処理は入れない(仕様が不明のため)。
	}

	return ferr;
}
// RevRxNo170511-001 Append End

//=============================================================================
/**
 * キャッシュデータ内のOFS デバッグ継続モードをシングルに設定
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @retval なし
 */
//=============================================================================
FFWERR ClrFlashCacheOfsDbg2Sng( enum FFWENM_ENDIAN eMcuEndian )
{

	FFWERR	ferr = FFWERR_OK;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	MADDR	madrCacheStartAddr;
	USER_ROMCACHE_RX		*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得


	GetFlashRomBlockInf(MAREA_USER, MCU_OSM_OFS1_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
	madrCacheStartAddr = GetCacheStartAddr(MAREA_USER);

	if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
		ferr = UpdateFlashRomCache(MAREA_USER, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}

	// デバッグ継続モード解除設定(シングルチップモードに設定)
	if( eMcuEndian == ENDIAN_BIG ){		// ビッグエンディアンの場合
		um_ptr->pbyCacheMem[MCU_OSM_OFS1_START - madrCacheStartAddr + 0] = (BYTE)(um_ptr->pbyCacheMem[MCU_OSM_OFS1_START - madrCacheStartAddr + 0] | MCU_OSM_OFS1_OCDMOD_SINGLE);
	}else{
		um_ptr->pbyCacheMem[MCU_OSM_OFS1_START - madrCacheStartAddr + 3] = (BYTE)(um_ptr->pbyCacheMem[MCU_OSM_OFS1_START - madrCacheStartAddr + 3] | MCU_OSM_OFS1_OCDMOD_SINGLE);
	}
	um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;	// ブロック書き換えフラグを書き換えありに設定
	um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定

	return ferr;
}

//=============================================================================
/**
 * キャッシュデータ内のOFS デバッグ継続モードに更新
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @retval なし
 */
//=============================================================================
FFWERR SetFlashCacheOfsDbg(enum FFWENM_ENDIAN eMcuEndian )
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	MADDR	madrReadAddr;					// RevRxNo130730-001 Append Line
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	MADDR	madrCacheStartAddr;
	BOOL	bBootSwap = FALSE;				// RevRxNo130730-001 Append Line
	USER_ROMCACHE_RX		*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ
	FFWMCU_FWCTRL_DATA*		pFwCtrl;		// RevRxNo130730-001 Append Line

	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得
	pFwCtrl = GetFwCtrlData();				// RevRxNo130730-001 Append Line

	// RevRxNo130730-001 Append Start
	madrReadAddr = MCU_OSM_OFS1_START;
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
		// フラッシュがMF3またはSC32用MF3の場合

		ferr = ChkBootSwap(&bBootSwap);			// IOレジスタからブートスワップの設定状態を確認する。
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if(bBootSwap == TRUE) {
			// MCUのリセットによりブートスワップする→しないになった場合
			// リセットにより代替え領域からデフォルト領域に入れ替わるため、
			// 代替え領域に相当するMDEレジスタアドレスを算出する
			madrReadAddr = MCU_OSM_OFS1_START - GetBootSwapSize();
		}
	}
	// RevRxNo130730-001 Append End

	// RevRxNo130730-001 Modify Line
	GetFlashRomBlockInf(MAREA_USER, madrReadAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロック情報取得
	madrCacheStartAddr = GetCacheStartAddr(MAREA_USER);
	if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
		ferr = UpdateFlashRomCache(MAREA_USER, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}
	if( eMcuEndian == ENDIAN_BIG ){		// ビッグエンディアンの場合
		um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 0] &= ~MCU_OSM_OFS1_OCD_MASK;		// OCDMODビットを0クリア	// RevRxNo130730-001 Modify Line
		um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 0] |= MCU_OSM_OFS1_OCDMOD_DEBUG;	// デバッグ継続モード値設定	// RevRxNo130730-001 Modify Line
	}else{
		um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 3] &= ~MCU_OSM_OFS1_OCD_MASK;		// OCDMODビットを0クリア	// RevRxNo130730-001 Modify Line
		um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 3] |= MCU_OSM_OFS1_OCDMOD_DEBUG;	// デバッグ継続モード値設定	// RevRxNo130730-001 Modify Line
	}
	um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;	// ブロック書き換えフラグを書き換えありに設定
	um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定

	return ferr;
}

// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * キャッシュデータ内のOFS 電圧監視リセットを無効に更新
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @retval なし
 */
//=============================================================================
FFWERR SetFlashCacheOfsLvd(enum FFWENM_ENDIAN eMcuEndian)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	MADDR	madrReadAddr;
	BYTE	byOfs1LvdDis;
	BYTE	byOfs1LvdMask;
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	MADDR	madrCacheStartAddr;
	BOOL	bBootSwap = FALSE;
	USER_ROMCACHE_RX		*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ
	FFWMCU_FWCTRL_DATA*		pFwCtrl;
	FFWMCU_MCUDEF_DATA*		pMcuDef;

	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得
	pFwCtrl = GetFwCtrlData();
	pMcuDef = GetMcuDefData();

	// LVD無効の設定値およびマスク値を生成
	byOfs1LvdDis = static_cast<BYTE>(pMcuDef->dwOfs1LvdDis);
	byOfs1LvdMask = static_cast<BYTE>(pMcuDef->dwOfs1LvdMask);

	madrReadAddr = MCU_OSM_OFS1_START;
	if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
		// フラッシュがMF3またはSC32用MF3の場合

		ferr = ChkBootSwap(&bBootSwap);			// IOレジスタからブートスワップの設定状態を確認する。
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if(bBootSwap == TRUE) {
			// MCUのリセットによりブートスワップする→しないになった場合
			// リセットにより代替え領域からデフォルト領域に入れ替わるため、
			// 代替え領域に相当するMDEレジスタアドレスを算出する
			madrReadAddr = MCU_OSM_OFS1_START - GetBootSwapSize();
		}
	}

	GetFlashRomBlockInf(MAREA_USER, madrReadAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロック情報取得
	madrCacheStartAddr = GetCacheStartAddr(MAREA_USER);
	if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
		ferr = UpdateFlashRomCache(MAREA_USER, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}
	if( eMcuEndian == ENDIAN_BIG ){		// ビッグエンディアンの場合
		um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 3] &= ~byOfs1LvdMask;	// 電圧監視リセット有効ビットクリア
		um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 3] |= byOfs1LvdDis;		// 電圧監視リセット無効設定
	}else{
		um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 0] &= ~byOfs1LvdMask;	// 電圧監視リセット有効ビットクリア
		um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 0] |= byOfs1LvdDis;		// 電圧監視リセット無効設定
	}
	um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;	// ブロック書き換えフラグを書き換えありに設定
	um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定

	return ferr;
}
// RevRxNo130730-001 Append End


//=============================================================================
/**
 * 内蔵Flashキャッシュのエンディアン情報を更新
 * @param  ePmode MCU動作モード　　RX_PMODE_USRBOOT:ユーザーブート RX_PMODE_SNG:シングルチップモード
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @retval FFWERR FFWエラー
 */
//=============================================================================
FFWERR SetFlashCacheMdeEndian(enum FFWRXENM_PMODE ePmode,enum FFWENM_ENDIAN eMcuEndian)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	MADDR	madrReadAddr;					// RevRxNo130730-001 Append Line
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	MADDR	madrCacheStartAddr;
	BOOL	bBootSwap = FALSE;				// RevRxNo130730-001 Append Line
	USER_ROMCACHE_RX		*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;		// ユーザーブートマットキャッシュメモリ構造体ポインタ
	FFWMCU_FWCTRL_DATA*		pFwCtrl;		// RevRxNo130730-001 Append Line

	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();	// ユーザーマットキャッシュメモリ構造体情報取得
	pFwCtrl = GetFwCtrlData();				// RevRxNo130730-001 Append Line

	if( ePmode == RX_PMODE_SNG ){

		// RevRxNo130730-001 Append Start
		madrReadAddr = MCU_OSM_MDES_START;
		if ((pFwCtrl->eFlashType == RX_FLASH_MF3) || (pFwCtrl->eFlashType == RX_FLASH_SC32_MF3)) {
			// フラッシュがMF3またはSC32用MF3の場合

			ferr = ChkBootSwap(&bBootSwap);			// IOレジスタからブートスワップの設定状態を確認する。
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			if(bBootSwap == TRUE) {
				// MCUのリセットによりブートスワップする→しないになった場合
				// リセットにより代替え領域からデフォルト領域に入れ替わるため、
				// 代替え領域に相当するMDEレジスタアドレスを算出する
				madrReadAddr = MCU_OSM_MDES_START - GetBootSwapSize();
			}
		}
		// RevRxNo130730-001 Append End

		// RevRxNo130730-001 Modify Line
		GetFlashRomBlockInf(MAREA_USER, madrReadAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロック情報取得
		madrCacheStartAddr = GetCacheStartAddr(MAREA_USER);
		if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
			// この関数では動作予定のエンディアンでフラッシュをLWORDで読み出してキャッシュに格納している
			ferr = UpdateFlashRomCache(MAREA_USER, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if(ferr != FFWERR_OK){
				return ferr;
			}
		}
		if( eMcuEndian == ENDIAN_BIG ){		// ビッグエンディアンにする予定の場合
			um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 3] &= MCU_OSM_MDE_BIG;	// MDEビットがあるところ		// RevRxNo130730-001 Modify Line
		}else{								// リトルエンディアンにする予定の場合
			um_ptr->pbyCacheMem[madrReadAddr - madrCacheStartAddr + 0] |= MCU_OSM_MDE_LITTLE;	// MDEビットがあるところ	// RevRxNo130730-001 Modify Line
		}
		um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;	// ブロック書き換えフラグを書き換えありに設定
		um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定
	}else if( ePmode == RX_PMODE_USRBOOT ){
		GetFlashRomBlockInf(MAREA_USERBOOT, MCU_OSM_MDEB_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロック情報取得
		madrCacheStartAddr = GetCacheStartAddr(MAREA_USERBOOT);
		if(GetUbmCacheSet() == FALSE){		//　ユーザブートマット領域のデータをキャッシュに未取得の場合 
			ferr = UpdateFlashRomCache(MAREA_USERBOOT, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if(ferr != FFWERR_OK){
				return ferr;
			}
		}
		if( eMcuEndian == ENDIAN_BIG ){		// ビッグエンディアンの場合
			ubm_ptr->pbyCacheMem[MCU_OSM_MDEB_START - madrCacheStartAddr + 3] &= MCU_OSM_MDE_BIG;	// MDEビットがあるところ
		}else{
			ubm_ptr->pbyCacheMem[MCU_OSM_MDEB_START - madrCacheStartAddr + 0] |= MCU_OSM_MDE_LITTLE;	// MDEビットがあるところ
		}
		SetUbmCacheSet(TRUE);	// キャッシュメモリデータ設定フラグを設定済みに設定
		SetUbmBlockWrite(TRUE);	// ブロック書き換えフラグを書き換えありに設定
	}

	return ferr;
}

//=============================================================================
/**
 * 内蔵FlashキャッシュのUBコード情報を更新
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @param  pUbcode 　　セットするUBコード格納ポインタ
 * @retval FFWERR FFWエラー
 */
//=============================================================================
FFWERR SetFlashCacheUbcode(enum FFWENM_ENDIAN eMcuEndian,const FFWRX_UBCODE_DATA *pUbcode)
{

	FFWERR	ferr = FFWERR_OK;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	MADDR	madrCacheStartAddr;
	int		i,j;
	USERBOOT_ROMCACHE_RX	*ubm_ptr;		// ユーザーブートマットキャッシュメモリ構造体ポインタ

	ubm_ptr = GetUserBootMatCacheData();	// ユーザーマットキャッシュメモリ構造体情報取得

	// 対象ブロック情報取得
	GetFlashRomBlockInf(MAREA_USERBOOT, MCU_OSM_UBCODE_A_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
	madrCacheStartAddr = GetCacheStartAddr(MAREA_USERBOOT);
	if(GetUbmCacheSet() == FALSE){		//　ユーザブートマット領域のデータをキャッシュに未取得の場合 
		ferr = UpdateFlashRomCache(MAREA_USERBOOT, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
		if(ferr != FFWERR_OK){
			return ferr;
		}
	}
	if( eMcuEndian == ENDIAN_BIG ){		// ビッグエンディアンの場合
		for( i = 0; i < 8; i++ ){		// 8バイト分のUBコードを埋め込む
			ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_A_START - madrCacheStartAddr) + i] = pUbcode->byUBCodeA[i]; 
		}
		for( i = 0; i < 8; i++ ){		// 8バイト分のUBコードを埋め込む
			ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_B_START - madrCacheStartAddr) + i] = pUbcode->byUBCodeB[i]; 
		}
	}else{
		for( i = 0, j = 3; i < 4; i++, j-- ){		// 8バイト分のUBコードA(前半)を埋め込む
			ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_A_START - madrCacheStartAddr) + i] = pUbcode->byUBCodeA[j]; 
		}
		for( i = 4, j = 7; i < 8; i++, j-- ){		// 8バイト分のUBコードA(後半)を埋め込む
			ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_A_START - madrCacheStartAddr) + i] = pUbcode->byUBCodeA[j]; 
		}
		for( i = 0, j = 3; i < 4; i++, j-- ){		// 8バイト分のUBコードB(前半)を埋め込む
			ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_B_START - madrCacheStartAddr) + i] = pUbcode->byUBCodeB[j]; 
		}
		for( i = 4, j = 7; i < 8; i++, j-- ){		// 8バイト分のUBコードB(後半)を埋め込む
			ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_B_START - madrCacheStartAddr) + i] = pUbcode->byUBCodeB[j]; 
		}
	}
	SetUbmCacheSet(TRUE);	// キャッシュメモリデータ設定フラグを設定済みに設定
	SetUbmBlockWrite(TRUE);	// ブロック書き換えフラグを書き換えありに設定

	return ferr;
}


// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * Extra領域用キャッシュメモリ設定処理 (RV40F用)
 * @param eMcuEndian PMODで指定されたエンディアン
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetExtraCache(enum FFWENM_ENDIAN eMcuEndian)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	byReadData[4];
	DWORD	dwCnt;
	DWORD	dwCacheCnt;
	DWORD	dwLoopNum;
	DWORD	dwAreaNum;
	MADDR	madrReadAddr;
	BOOL	bSameAccessSize;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BOOL	bSameAccessCount;
	DWORD	dwAccessCount;
	BYTE*	pbyReadData;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line

	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	dwAreaNum = 1;
	bSameAccessSize = TRUE;
	eAccessSize = MLWORD_ACCESS;
	bSameAccessCount = TRUE;
	dwAccessCount = 1;
	pbyReadData = &byReadData[0];

	dwCacheCnt = 0;		// Extra領域用キャッシュメモリの設定位置初期化
	dwLoopNum = pMcuDef->dwExtraASize / 4;	// ロングワードアクセスでのループ回数設定	// RevRxNo150827-002 Modify Line
	madrReadAddr = pMcuDef->madrExtraACacheStart;	// ExtraA領域のリード開始アドレス設定(Extra領域用キャッシュのMCU開始アドレス)	// RevRxNo150827-002 Modify Line

	for (dwCnt = 0; dwCnt < dwLoopNum; dwCnt++) {
		// Extra領域の内容をリード
		ferr = PROT_MCU_DDUMP(dwAreaNum, &madrReadAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// Extra領域用キャッシュメモリにリードデータを設定
		if (eMcuEndian == ENDIAN_LITTLE) {	// リトルエンディアン時
			s_byExtraCacheMem[dwCacheCnt] = byReadData[0];
			s_byExtraCacheMem[dwCacheCnt + 1] = byReadData[1];
			s_byExtraCacheMem[dwCacheCnt + 2] = byReadData[2];
			s_byExtraCacheMem[dwCacheCnt + 3] = byReadData[3];

		} else {	// ビッグエンディアン時
			s_byExtraCacheMem[dwCacheCnt] = byReadData[3];
			s_byExtraCacheMem[dwCacheCnt + 1] = byReadData[2];
			s_byExtraCacheMem[dwCacheCnt + 2] = byReadData[1];
			s_byExtraCacheMem[dwCacheCnt + 3] = byReadData[0];
		}

		dwCacheCnt += 4;
		madrReadAddr += 4;
	}

	return ferr;
}
// RevRxNo130301-001 Append End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * 指定データとExtra領域用キャッシュメモリ内容一致確認
 * @param eMcuEndian PMODで指定されたエンディアン
 * @param dwSetNo Extra領域設定アドレス
 * @param dwSetData Extra領域設定データ
 * @retval TRUE 不一致のため書き換えあり
 * @retval FALSE 書き換えなし
 */
//=============================================================================
BOOL CheckExtraCacheData(enum FFWENM_ENDIAN eMcuEndian, DWORD dwSetNo, DWORD dwSetData)
{
	BOOL	bExtraChange = FALSE;
	DWORD	dwCacheCnt;
	DWORD	dwData;
	BYTE	byData;
	DWORD	dwCnt;
	int		i;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	dwCacheCnt = dwSetNo - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出	// RevRxNo150827-002 Modify Line

	dwData = dwSetData;

	if (eMcuEndian == ENDIAN_LITTLE) {	// リトルエンディアン時
		for (i = 0; i < 4; i++) {
			byData = static_cast<BYTE>(dwSetData & 0x000000ff);
			if (s_byExtraCacheMem[dwCacheCnt + i] != byData) {
				s_byExtraCacheMem[dwCacheCnt + i] = byData;
				bExtraChange = TRUE;
			}
			dwSetData = dwSetData >> 8;
		}
	} else  {							// ビッグエンディアン時
		for (i = 0; i < 4; i++) {
			byData = static_cast<BYTE>(dwSetData & 0x000000ff);
			if (s_byExtraCacheMem[dwCacheCnt + 3 - i] != byData) {
				s_byExtraCacheMem[dwCacheCnt + 3 - i] = byData;
				bExtraChange = TRUE;
			}
			dwSetData = dwSetData >> 8;
		}
	}

	if (bExtraChange == TRUE) {	// ExtraA領域データの変更が必要な場合
		dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
		s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
		s_bExtraAreaWriteFlag = TRUE;	// ExtraA領域書き換えフラグをセット
	}

	return	bExtraChange;
}
// RevRxNo130301-001 Append End

// RevRxNo130301-002 Append Start
//=============================================================================
/**
 * 指定エンディアンとExtra領域用キャッシュメモリ内容一致確認
 * @param eMcuEndian PMODで指定されたエンディアン
 * @retval TRUE MDEレジスタ書き換え必要
 * @retval FALSE MDEレジスタ書き換え不要
 */
//=============================================================================
BOOL CheckExtraCacheMdeEndian(enum FFWENM_ENDIAN eMcuEndian)
{
	BOOL	bMdeChange = FALSE;
	DWORD	dwCacheCnt;
	BYTE	byData;
	DWORD	dwCnt;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	dwCacheCnt = pMcuDef->dwMdeStartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出	// RevRxNo150827-002 Modify Line

	if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
		byData = MCU_OSM_MDE_LITTLE_DATA;
	} else {
		byData = MCU_OSM_MDE_BIG_DATA;
		dwCacheCnt += 3;
	}

	if ((s_byExtraCacheMem[dwCacheCnt] & MCU_OSM_MDE_MASK_DATA) != byData) {
		s_byExtraCacheMem[dwCacheCnt] = (s_byExtraCacheMem[dwCacheCnt] & ~MCU_OSM_MDE_MASK_DATA) | byData;
		bMdeChange = TRUE;
	}

	if (bMdeChange == TRUE) {	// MDEレジスタの書き換えが必要な場合
		dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
		s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
		s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット
	}

	return bMdeChange;
}
// RevRxNo130301-002 Append End

// RevRxNo161003-001 Apeend Start
//=============================================================================
/**
 * 指定バンクモードとExtra領域用キャッシュメモリ内容一致確認
 * @param pbBankmdChange バンクモード変更判定結果格納変数のアドレス
 * @param bPmodCmd PMODコマンド発行有無
 * @retval FFWエラーコード
 * @retval バンクモード変更判定結果をpbBankmdChangeへ格納
 *			TRUE バンクモード書き換え必要
 *			FALSE バンクモード書き換え不要
 */
//=============================================================================
FFWERR CheckExtraCacheMdeBankmd(BOOL* pbBankmdChange, BOOL bPmodCmd)
{
	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_ENDIAN	eMcuEndian;
	enum FFWRXENM_BANKMD	eBankMode;
	BOOL	bBankmdChange = FALSE;
	DWORD	dwCacheCnt;
	BYTE	byData;
	DWORD	dwCnt;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;
	FFWMCU_MCUDEF_DATA* pMcuDef;

	pFwCtrl = GetFwCtrlData();
	pMcuDef = GetMcuDefData();

	*pbBankmdChange = FALSE;		// 初期化

	if (pFwCtrl->bBankmdExist) {	// BANKMDレジスタが存在するMCUの場合
		// 設定されているバンクモード情報から設定データ作成
		eBankMode = GetBankModeDataRX();
		if (eBankMode == RX_DUAL) {			// デュアルモードの場合
			byData = MCU_OSM_BANKMD_DUAL_DATA;
		} else {							// リニアモードの場合
			byData = MCU_OSM_BANKMD_LINEAR_DATA;
		}

		// エンディアンに応じたキャッシュオフセット値算出
		eMcuEndian = GetEndianDataRX();		// エンディアン情報取得
		dwCacheCnt = pMcuDef->dwMdeStartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出
		if (eMcuEndian == ENDIAN_BIG) {		// ビッグエンディアンの場合
			dwCacheCnt += 3;
		}

		// キャッシュのBANKMD値 != バンクモード情報　の場合
		if ((s_byExtraCacheMem[dwCacheCnt] & MCU_OSM_BANKMD_MASK_DATA) != byData) {
			s_byExtraCacheMem[dwCacheCnt] = (s_byExtraCacheMem[dwCacheCnt] & ~MCU_OSM_BANKMD_MASK_DATA) | byData;
			bBankmdChange = TRUE;
		}

		if (bBankmdChange) {		// MDEレジスタの書き換えが必要な場合
			if (GetTMEnable()) {	// TM機能有効の場合
				if (bPmodCmd) {				// PMODコマンドから発行された場合
					ferr = FFWERR_ROM_BANKMD_NOT_CHANGE;
				}
			} else {						// TM機能無効の場合
				dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
				s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
				s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット
				*pbBankmdChange = TRUE;
			}
		}
	}

	return ferr;
}

//=============================================================================
/**
 * 起動バンクとExtra領域用キャッシュメモリ内容一致確認
 * @param eBankSel 起動バンク情報
 * @param pbBankselChange 起動バンク変更判定結果格納変数のアドレス
 * @param pbBankselNotChange TM機能有効かつFFEE0000h～FFEEFFFFhのTM機能無効時の起動バンク変更不可判定結果格納変数のアドレス
 * @param bPmodCmd PMODコマンド発行有無
 * @retval FFWエラーコード
 * @retval 起動バンク変更判定結果をpbBankselChangeに格納
 *			TRUE 起動バンク書き換え必要
 *			FALSE 起動バンク書き換え不要
 * @retval TM機能有効かつFFEE0000h～FFEEFFFFhのTM機能無効時の起動バンク変更不可判定結果をpbBankselNotChangeに格納
 *			TRUE 起動バンク書き換え不可
 *			FALSE 起動バンク書き換え可
 */
//=============================================================================
FFWERR CheckExtraCacheBanksel(enum FFWRXENM_BANKSEL eBankSel, BOOL* pbBankselChange, BOOL* pbBankselNotChange, BOOL bPmodCmd)
{
	FFWERR	ferr = FFWERR_OK;
	enum FFWENM_ENDIAN	eMcuEndian;
	enum FFWRXENM_BANKMD	eBankMode;
	BOOL	bBankselChange = FALSE;
	DWORD	dwCacheCnt;
	BYTE	byData;
	DWORD	dwCnt;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;
	FFWMCU_MCUDEF_DATA* pMcuDef;

	pFwCtrl = GetFwCtrlData();
	pMcuDef = GetMcuDefData();

	*pbBankselChange = FALSE;			// 初期化
	*pbBankselNotChange = FALSE;		// 初期化

	if (pFwCtrl->bBankmdExist) {		// BANKMDレジスタが存在するMCUの場合
		eBankMode = GetBankModeDataRX();	// 現在のバンクモード取得
		if (eBankMode == RX_LINEAR) {		// リニアモードの場合
			// PMODコマンド処理からはリニアモードでこの関数が発行されることはない。
			// リニアモードではBANKSELレジスタ書き換えエラーは出さないため何もせずリターンする。
			return ferr;
		}

		// エンディアンに応じたキャッシュオフセット値算出
		eMcuEndian = GetEndianDataRX();		// エンディアン情報取得
		dwCacheCnt = pMcuDef->dwBankselStartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出
		if (eMcuEndian == ENDIAN_BIG) {		// ビッグエンディアンの場合
			dwCacheCnt += 3;
		}

		// 起動バンク情報から設定データ作成
		if (eBankSel == RX_BANKSEL_BANK0) {			// 起動バンク0の場合
			byData = MCU_OSM_BANKSWP_BANK0_DATA;
		} else if (eBankSel == RX_BANKSEL_BANK1) {	// 起動バンク1の場合
			byData = MCU_OSM_BANKSWP_BANK1_DATA;
		} else {							// 起動バンク継続の場合
			return ferr;
		}

		// キャッシュのBANKSEL値 != 起動バンク情報　の場合
		if ((s_byExtraCacheMem[dwCacheCnt] & MCU_OSM_BANKSWP_MASK_DATA) != byData) {
			if (bPmodCmd) {				// PMODコマンドから発行された場合
				s_byExtraCacheMem[dwCacheCnt] = (s_byExtraCacheMem[dwCacheCnt] & ~MCU_OSM_BANKSWP_MASK_DATA) | byData;
			}
			bBankselChange = TRUE;
		}

		if (bBankselChange) {	// BANKSELレジスタの書き換えが必要な場合
			if (GetTMEnable() && (GetTMEnableDual() == FALSE)) {	// TM機能有効 && FFEE0000h～FFEEFFFFhのTM機能無効の場合
				if (bPmodCmd) {							// PMODコマンドから発行された場合
					ferr = FFWERR_ROM_BANKSWP_NOT_CHANGE;
				} else {								// ダウンロード処理、またはリセット/システムリセットから発行された場合
					// FFWERR_WRITE_BANKSEL_NOT_CHANGE(Warning)を返してBANKSELの書き換えは行う(実際には書き換わらない)。
					*pbBankselNotChange = TRUE;
					dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
					s_bExtraWriteFlag[dwCnt] = TRUE;	// ExtraA領域の16バイト単位での書き換えフラグをセット
					s_bExtraAreaWriteFlag = TRUE;		// ExtraA領域書き換えフラグをセット
				}
			} else {									// TM機能無効 or (TM機能有効 && FFEE0000h～FFEEFFFFhの機能有効)の場合
				*pbBankselChange = TRUE;
				dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
				s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
				s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット
			}
		}
	}

	return ferr;
}
// RevRxNo161003-001 Apeend End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * Extra領域のOFS1レジスタ デバッグ継続モード設定
 * @param eMcuEndian PMODで指定されたエンディアン
 * @retval なし
 */
//=============================================================================
void SetExtraCacheOfsDbg(enum FFWENM_ENDIAN eMcuEndian)
{
	DWORD	dwCacheCnt;
	DWORD	dwCnt;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	dwCacheCnt = pMcuDef->dwOfs1StartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出	// RevRxNo150827-002 Modify Line

	if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
		dwCacheCnt += 3;
	}
	s_byExtraCacheMem[dwCacheCnt] &= ~MCU_OSM_OFS1_OCD_MASK;		// OCDMODビットを0クリア
	s_byExtraCacheMem[dwCacheCnt] |= MCU_OSM_OFS1_OCDMOD_DEBUG;	// デバッグ継続モード値設定

	dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
	s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
	s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット

	return;
}
// RevRxNo130301-001 Append End

// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * Extra領域のOFS1レジスタ 電圧監視リセット無効設定
 * @param eMcuEndian PMODで指定されたエンディアン
 * @retval なし
 */
//=============================================================================
void SetExtraCacheOfsLvd(enum FFWENM_ENDIAN eMcuEndian)
{
	BYTE	byOfs1LvdDis;
	BYTE	byOfs1LvdMask;
	DWORD	dwCacheCnt;
	DWORD	dwCnt;
	FFWMCU_MCUDEF_DATA*		pMcuDef;

	pMcuDef = GetMcuDefData();

	// LVD無効の設定値およびマスク値を生成
	byOfs1LvdDis = static_cast<BYTE>(pMcuDef->dwOfs1LvdDis);
	byOfs1LvdMask = static_cast<BYTE>(pMcuDef->dwOfs1LvdMask);

	dwCacheCnt = pMcuDef->dwOfs1StartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出	// RevRxNo150827-002 Modify Line

	if (eMcuEndian == ENDIAN_BIG) {		// ビッグエンディアンの場合
		dwCacheCnt += 3;
	}
	s_byExtraCacheMem[dwCacheCnt] &= ~byOfs1LvdMask;	// 電圧監視リセット有効ビットクリア
	s_byExtraCacheMem[dwCacheCnt] |= byOfs1LvdDis;		// 電圧監視リセット無効設定

	dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
	s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
	s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット

	return;
}
// RevRxNo130730-001 Append Start

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * Extra領域のOFS1レジスタ デバッグ継続モードをシングルチップモードに設定
 * @param eMcuEndian PMODで指定されたエンディアン
 * @retval なし
 */
//=============================================================================
void ClrExtraCacheOfsDbg2Sng(enum FFWENM_ENDIAN eMcuEndian)
{
	DWORD	dwCacheCnt;
	DWORD	dwCnt;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	dwCacheCnt = pMcuDef->dwOfs1StartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出	// RevRxNo150827-002 Modify Line

	if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
		dwCacheCnt += 3;
	}
	s_byExtraCacheMem[dwCacheCnt] &= ~MCU_OSM_OFS1_OCD_MASK;		// OCDMODビットを0クリア
	s_byExtraCacheMem[dwCacheCnt] |= MCU_OSM_OFS1_OCDMOD_SINGLE;	// シングルチップモード値設定

	dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
	s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
	s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット

	return;
}

// RevRxNo130301-001 Append End

// RevRxNo150827-003 Append Start
//=============================================================================
/**
 * FAWレジスタのFSPRビットマスク処理
 * @param eMcuEndian PMODで指定されたエンディアン
 * @retval TRUE FAWレジスタ書き換え必要
 * @retval FALSE FAWレジスタ書き換え不要
 */
//=============================================================================
BOOL CheckExtraCacheFawFspr(enum FFWENM_ENDIAN eMcuEndian)
{
	BOOL	bFawChange = FALSE;
	DWORD	dwCacheCnt;
	DWORD	dwCnt;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	FFWMCU_MCUINFO_DATA_RX*	pMcuInfo;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;		// RevRxNo160527-001 Append Line

	pMcuDef = GetMcuDefData();
	pMcuInfo = GetMcuInfoDataRX();
	pFwCtrl = GetFwCtrlData();			// RevRxNo160527-001 Append Line

	// RevRxNo160527-001 Modify Start +3
	if (pFwCtrl->bFawExist == FALSE) {	// FAWレジスタが存在しない場合
		return bFawChange;					// 何もせずFALSEでリターン
	}		
	// RevRxNo160527-001 Modify End

	dwCacheCnt = pMcuDef->dwFawStartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出

	if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
		dwCacheCnt += 1;
	} else {
		dwCacheCnt += 2;
	}

	if ((s_byExtraCacheMem[dwCacheCnt] & MCU_OSM_FAW_FSPR_MASK) != MCU_OSM_FAW_FSPR_MASK) {		// FAW.FSPRビットが'1'でない場合
		s_byExtraCacheMem[dwCacheCnt] |= MCU_OSM_FAW_FSPR_MASK;
		bFawChange = TRUE;
		// RevRxNo160527-001 Append Line
		s_bFawFsprChangeFlg = TRUE;				// ワーニング表示用フラグセット
	}

	if (bFawChange == TRUE) {	// FAWレジスタの書き換えが必要な場合
		dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
		s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
		s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット
	}

	return bFawChange;
}
// RevRxNo150827-003 Append End


// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * ユーザーマットキャッシュメモリ初期化実行状態の参照
 * @param なし
 * @retval s_bUmCacheDataInitFlg ユーザーマットキャッシュメモリ初期化実行フラグ
 */
//=============================================================================
BOOL GetUmCacheDataInit(void)
{
	return s_bUmCacheDataInitFlg;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * ユーザーマットキャッシュメモリ初期化実行状態の設定
 * @param s_bUmCacheDataInitFlg ユーザーマットキャッシュメモリ初期化実行フラグ
 * @retval なし
 */
//=============================================================================
void SetUmCacheDataInit(BOOL bUmCacheDataInitFlg)
{
	s_bUmCacheDataInitFlg = bUmCacheDataInitFlg;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * ユーザーブートマットキャッシュメモリ初期化実行状態の参照
 * @param なし
 * @retval s_bUbmCacheDataInitFlg ユーザーブートマットキャッシュメモリ初期化実行フラグ
 */
//=============================================================================
BOOL GetUbmCacheDataInit(void)
{
	return s_bUbmCacheDataInitFlg;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * ユーザーブートマットキャッシュメモリ初期化実行状態の設定
 * @param s_bUbmCacheDataInitFlg ユーザーブートマットキャッシュメモリ初期化実行フラグ
 * @retval なし
 */
//=============================================================================
void SetUbmCacheDataInit(BOOL bUbmCacheDataInitFlg)
{
	s_bUbmCacheDataInitFlg = bUbmCacheDataInitFlg;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * データマット初期化実行状態の参照
 * @param なし
 * @retval s_bDataFlashInitFlag データマット初期化実行フラグ
 */
//=============================================================================
BOOL GetDataFlashInitFlag(void)
{
	return s_bDataFlashInitFlag;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * データマット初期化実行状態の設定
 * @param s_bDataFlashInitFlag データマット初期化実行フラグ
 * @retval なし
 */
//=============================================================================
void SetDataFlashInitFlag(BOOL bDataFlashInitFlag)
{
	s_bDataFlashInitFlag = bDataFlashInitFlag;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * データマットのみの初期化実行フラグの参照
 * @param なし
 * @retval s_bDataFlashEraseFlag データマットのみの初期化実行フラグ
 */
//=============================================================================
BOOL GetDataFlashEraseFlag(void)
{
	return s_bDataFlashEraseFlag;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * データマットのみの初期化実行フラグの設定
 * @param bDataFlashErase データマットのみの初期化実行フラグ
 * @retval なし
 */
//=============================================================================
void SetDataFlashEraseFlag(BOOL bDataFlashEraseFlag)
{
	s_bDataFlashEraseFlag = bDataFlashEraseFlag;	
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End


// V.1.02 新デバイス対応 Append Start
//=============================================================================
/**
 * ダウンロード時にMDE/UBコードの書き換えがあったかを格納する変数のクリア
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrMdeUbcodeChaneFlg(void)
{
	s_bEndianChange = FALSE;	// MDE書き換えていない
	s_bUbcodeChange = FALSE;	// UBコード書き換えていない
}

//=============================================================================
/**
 * ダウンロード時にエンディアンMDEビットを書き換えたかどうかを格納する変数の参照
 * @param なし
 * @retval なし
 */
//=============================================================================
BOOL GetEndianChangeFlg(void)
{
	return s_bEndianChange;
}

// RevRxNo161003-001 Append Start
//=============================================================================
/**
 * ダウンロード時にBANKMD/BANKSELの書き換えがあったかを格納する変数のクリア
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrBankmdBankselChaneFlg(void)
{
	s_bBankModeChange = FALSE;	// BANKMD書き換えていない
	s_bBankSelChange = FALSE;	// BANKSEL書き換えていない
	s_bBankSelNotChange = FALSE;	// BANKSEL書き換えた
}

//=============================================================================
/**
 * ダウンロード時にMDEレジスタBANKMDビットを書き換えたかどうかを格納する変数の参照
 * @param なし
 * @retval s_bBankModeChange MDEレジスタBANKMDビット書き換え有無
 */
//=============================================================================
BOOL GetBankModeChangeFlg(void)
{
	return s_bBankModeChange;
}

//=============================================================================
/**
 * ダウンロード時にBANKSELレジスタを書き換えたかどうかを格納する変数の参照
 * @param なし
 * @retval s_bBankSelChange　BANKSELレジスタ書き換え有無
 */
//=============================================================================
BOOL GetBankSelChangeFlg(void)
{
	return s_bBankSelChange;
}

//=============================================================================
/**
 * ダウンロード時にTM機能有効かつFFEE0000h～FFEEFFFFhのTM機能が無効のためBANKSEL
 * レジスタを書き換えられなかったかどうかを格納する変数の参照
 * @param なし
 * @retval s_bBankSelNotChange　BANKSELレジスタ書き換え不可判定結果
 */
//=============================================================================
BOOL GetBankSelNotChangeFlg(void)
{
	return s_bBankSelNotChange;
}
// RevRxNo161003-001 Append End

//=============================================================================
/**
 * ダウンロード時にUBコードを書き換えたかどうかを格納する変数の参照
 * @param なし
 * @retval なし
 */
//=============================================================================
BOOL GetUbcodeChangeFlg(void)
{
	return s_bUbcodeChange;
}
// V.1.02 新デバイス対応 Append End


// V.1.02 RevNo110418-002 Append Start
//=============================================================================
/**
 * MDE/UBコード領域をマスクする。
 * @param  なし
 * @retval なし
 */
//=============================================================================
void MaskMdeUbCodeArea(void)
{

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	FFWMCU_DBG_DATA_RX*	pDbgData;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWENM_ENDIAN				eMcuEndian;
	USER_ROMCACHE_RX			*um_ptr;	// ユーザマットキャッシュメモリ構造体ポインタ
	// V.1.02 新デバイス対応 Append Start
	enum FFWRXENM_PMODE			ePmode;				// 起動時に指定された端子設定起動モード
	// V.1.02 新デバイス対応 Append End
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	DWORD	dwBlkNo;
	DWORD	dwBlkSize;
	BOOL	bEndianChange = FALSE;		// RevRxNo140109-001 Append Line
	BOOL	bUbcodeChange = FALSE;		// RevRxNo140109-001 Append Line
	// RevRxNo161003-001 Append Start
	BOOL	bBankmdChange = FALSE;
	BOOL	bBankselChange = FALSE;
	BOOL	bBankselNotChange = FALSE;
	enum FFWRXENM_BANKSEL	eBankSel;
	// RevRxNo161003-001 Append End

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	eMcuEndian = GetEndianDataRX();	// エンディアン情報取得
	um_ptr = GetUserMatCacheData();			// ユーザマットキャッシュメモリ構造体情報取得
	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	// V.1.02 新デバイス対応 Append Start
	ePmode = GetPmodeDataRX();
	// V.1.02 新デバイス対応 Append End

	// 通常モード時のみマスクする
	if (pDbgData->eWriter == EML_WRITERMODE_NON) {
		// V.1.02 新デバイス対応 Append Start
		// RX630/RX210の場合、起動時に指定したエンディアン、UBコード情報とダウンロードデータ情報が違う場合は、
		// 起動時に指定した情報に入れ替えるようにしなければならない。置き換えた場合は、ダウンローぢ終了後に
		// Warningを返すようにしなければならない。

		// RevRxNo140109-001 Modify Start
		// RevRxNo130411-001 Modify Line
		// MDEレジスタ情報一致確認と設定を実施
		if (pFwCtrl->eMdeType == RX_MDE_FLASH) {			// MDEをフラッシュROMで設定するMCUの場合
			if (ePmode == RX_PMODE_SNG) {				// シングルチップ起動の場合
				// MDESレジスタがあるフラッシュROM領域のブロック番号を取得する
				GetFlashRomBlockInf(MAREA_USER, MCU_OSM_MDES_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

				if (um_ptr->pbyCacheSetFlag[dwBlkNo] == TRUE) {		// MDEレジスタがあるフラッシュROM領域のキャッシュがある場合
					// MDESレジスタエンディアン情報一致確認と設定
					bEndianChange = checkFlashCacheMdeEndian(eMcuEndian);
					if (bEndianChange == TRUE) {	// エンディアンを変更した場合
						s_bEndianChange = TRUE;		// エンディアンを書き換えたことを覚えておく
					}
				}
			} else if (ePmode == RX_PMODE_USRBOOT) {	// ユーザブート起動の場合	
				if (GetUbmCacheSet() == TRUE) {			//　ユーザブートマット領域データをキャッシュに取得済み
					// MDEBレジスタエンディアン情報一致確認と設定
					bEndianChange = checkFlashCacheMdeEndian(eMcuEndian);
					if (bEndianChange == TRUE) {	// エンディアンを変更した場合
						s_bEndianChange = TRUE;		// エンディアンを書き換えたことを覚えておく
					}
				}
			}
		} else if (pFwCtrl->eMdeType == RX_MDE_EXTRA) {	// Extra領域で設定するMCUの場合
			if (GetExtraCacheSet() == TRUE) {			//　Extra領域データをキャッシュに取得済み
				// MDEレジスタエンディアン情報一致確認と設定
				bEndianChange = CheckExtraCacheMdeEndian(eMcuEndian);
				if (bEndianChange == TRUE) {	// エンディアンを変更した場合
					s_bEndianChange = TRUE;		// エンディアンを書き換えたことを覚えておく
				}
				// RevRxNo161003-001 Append Start
				CheckExtraCacheMdeBankmd(&bBankmdChange, FALSE);
				if (bBankmdChange) {
					s_bBankModeChange = TRUE;
				}
				if (pFwCtrl->bBankselExist) {	// BANKSELレジスタが存在するMCUの場合
					eBankSel = GetBankSelDataRX();
					CheckExtraCacheBanksel(eBankSel, &bBankselChange, &bBankselNotChange, FALSE);
					if (bBankselChange) {
						s_bBankSelChange = TRUE;
					}
					if (bBankselNotChange) {
						s_bBankSelNotChange = TRUE;
					}
				}
				// RevRxNo161003-001 Append End

				// FAWレジスタFSPRビットマスク処理
				CheckExtraCacheFawFspr(eMcuEndian);		// RevRxNo150827-003 Append Line
			}
		}


		// UBコード情報一致確認と設定を実施
		// RevRxNo130411-001 Append Line
		if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
			// UBコードの確認(　ユーザブート起動の場合のみ )
			if(ePmode == RX_PMODE_USRBOOT) {			// ユーザブート起動の場合
				if (GetUbmCacheSet() == TRUE) {			//　ユーザブートマット領域データをキャッシュに取得済み
					// UBコード情報一致確認と設定
					bUbcodeChange = checkFlashCacheUbcode(eMcuEndian);
					if (bUbcodeChange == TRUE) {	// UBコードを変更した場合
						s_bUbcodeChange = TRUE;		// UBコードを書き換えたことを覚えておく
					}
				}
			}
		}
		// RevRxNo140109-001 Modify End
	}

	return;
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

}
// V.1.02 RevNo110418-002 Append End


// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * 指定エンディアンとフラッシュROM用キャッシュメモリ内容一致確認
 * @param eMcuEndian PMODで指定されたエンディアン
 * @retval TRUE MDEレジスタ書き換え必要
 * @retval FALSE MDEレジスタ書き換え不要
 */
//=============================================================================
static BOOL checkFlashCacheMdeEndian(enum FFWENM_ENDIAN eMcuEndian)
{
	BOOL					bMdeChange = FALSE;
	enum FFWRXENM_PMODE		ePmode;				// 起動時に指定された端子設定起動モード
	BYTE					byData;
	MADDR					madrBlkStartAddr;
	MADDR					madrBlkEndAddr;
	DWORD					dwBlkNo;
	DWORD					dwBlkSize;
	DWORD					dwCacheCnt;
	USER_ROMCACHE_RX		*um_ptr;			// ユーザマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;			// ユーザブートマットキャッシュメモリ構造体ポインタ

	ePmode = GetPmodeDataRX();
	um_ptr = GetUserMatCacheData();			// ユーザマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();				// ユーザブートマットキャッシュメモリ構造体情報取得

	if (ePmode == RX_PMODE_SNG) {				// シングルチップ起動の場合
		dwCacheCnt = MCU_OSM_MDES_START - GetCacheStartAddr(MAREA_USER);	// フラッシュROM用キャッシュメモリ位置算出

		if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
			byData = MCU_OSM_MDE_LITTLE_DATA;
		} else {
			byData = MCU_OSM_MDE_BIG_DATA;
			dwCacheCnt += 3;
		}

		if ((um_ptr->pbyCacheMem[dwCacheCnt] & MCU_OSM_MDE_MASK_DATA) != byData) {
			um_ptr->pbyCacheMem[dwCacheCnt] = (um_ptr->pbyCacheMem[dwCacheCnt] & ~MCU_OSM_MDE_MASK_DATA) | byData;
			bMdeChange = TRUE;
		}

		if (bMdeChange == TRUE) {	// MDEレジスタの書き換えが必要な場合
			GetFlashRomBlockInf(MAREA_USER, MCU_OSM_MDES_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
			um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;		// ブロック書き換えフラグを書き換えありに設定
			um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定
		}

	} else if (ePmode == RX_PMODE_USRBOOT) {	// ユーザブート起動の場合
		dwCacheCnt = MCU_OSM_MDEB_START - GetCacheStartAddr(MAREA_USERBOOT);	// フラッシュROM用キャッシュメモリ位置算出

		if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
			byData = MCU_OSM_MDE_LITTLE_DATA;
		} else {
			byData = MCU_OSM_MDE_BIG_DATA;
			dwCacheCnt += 3;
		}

		if ((ubm_ptr->pbyCacheMem[dwCacheCnt] & MCU_OSM_MDE_MASK_DATA) != byData) {
			ubm_ptr->pbyCacheMem[dwCacheCnt] = (ubm_ptr->pbyCacheMem[dwCacheCnt] & ~MCU_OSM_MDE_MASK_DATA) | byData;
			bMdeChange = TRUE;
		}

		if (bMdeChange == TRUE) {	// MDEレジスタの書き換えが必要な場合
			// V.1.02 RevNo110324-004 Modify Line  ユーザマットのフラグを設定していたをユーザブートマットに修正
			SetUbmBlockWrite(TRUE);	// ブロック書き換えフラグを書き換えありに設定
		}
	}

	return bMdeChange;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * 指定UBコードとフラッシュROM用キャッシュメモリ内容一致確認
 * @param eMcuEndian PMODで指定されたエンディアン
 * @retval TRUE UBコード書き換え必要
 * @retval FALSE UBコード書き換え不要
 */
//=============================================================================
static BOOL checkFlashCacheUbcode(enum FFWENM_ENDIAN eMcuEndian)
{
	BOOL					bUbcodeChange = FALSE;
	DWORD					i,j;
	MADDR					madrCacheStartAddr;
	USERBOOT_ROMCACHE_RX	*ubm_ptr;			// ユーザブートマットキャッシュメモリ構造体ポインタ
	FFWRX_UBCODE_DATA		 *pUBCode;

	ubm_ptr = GetUserBootMatCacheData();				// ユーザブートマットキャッシュメモリ構造体情報取得
	pUBCode = GetUBCodeDataRX();

	madrCacheStartAddr = GetCacheStartAddr(MAREA_USERBOOT);
	if( eMcuEndian == ENDIAN_BIG ){			// 起動時指定のエンディアンがビッグエンディアンの場合
		for( i = 0; i < 8; i++ ){
			if( pUBCode->byUBCodeA[i] != ubm_ptr->pbyCacheMem[MCU_OSM_UBCODE_A_START - madrCacheStartAddr + i] ){
				ubm_ptr->pbyCacheMem[MCU_OSM_UBCODE_A_START - madrCacheStartAddr + i] = pUBCode->byUBCodeA[i];
				bUbcodeChange = TRUE;
			}
		}
		for( i = 0; i < 8; i++ ){
			if( pUBCode->byUBCodeB[i] != ubm_ptr->pbyCacheMem[MCU_OSM_UBCODE_B_START - madrCacheStartAddr + i] ){
				ubm_ptr->pbyCacheMem[MCU_OSM_UBCODE_B_START - madrCacheStartAddr + i] = pUBCode->byUBCodeB[i];
				bUbcodeChange = TRUE;
			}
		}
	}else{
		for( i = 0, j = 3; i < 4; i++, j-- ){		// 8バイト分のUBコードA(前半)を埋め込む
			if( pUBCode->byUBCodeA[i] != ubm_ptr->pbyCacheMem[MCU_OSM_UBCODE_A_START - madrCacheStartAddr + j] ){
				// V.1.02 RevNo110324-004 Modify Line  UBコードのキャッシュメモリへの格納が間違っていた
				ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_A_START - madrCacheStartAddr) + j] = pUBCode->byUBCodeA[i];
				bUbcodeChange = TRUE;
			}
		}
		for( i = 4, j = 7; i < 8; i++, j-- ){		// 8バイト分のUBコードA(後半)を埋め込む
			if( pUBCode->byUBCodeA[i] != ubm_ptr->pbyCacheMem[MCU_OSM_UBCODE_A_START - madrCacheStartAddr + j] ){
				// V.1.02 RevNo110324-004 Modify Line  UBコードのキャッシュメモリへの格納が間違っていた
				ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_A_START - madrCacheStartAddr) + j] = pUBCode->byUBCodeA[i];
				bUbcodeChange = TRUE;
			}
		}
		for( i = 0, j = 3; i < 4; i++, j-- ){		// 8バイト分のUBコードB(前半)を埋め込む
			if( pUBCode->byUBCodeB[i] != ubm_ptr->pbyCacheMem[MCU_OSM_UBCODE_B_START - madrCacheStartAddr + j] ){
				// V.1.02 RevNo110324-004 Modify Line  UBコードのキャッシュメモリへの格納が間違っていた
				ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_B_START - madrCacheStartAddr) + j] = pUBCode->byUBCodeB[i];
				bUbcodeChange = TRUE;
			}
		}
		for( i = 4, j = 7; i < 8; i++, j-- ){		// 8バイト分のUBコードB(後半)を埋め込む
			if( pUBCode->byUBCodeB[i] != ubm_ptr->pbyCacheMem[MCU_OSM_UBCODE_B_START - madrCacheStartAddr + j] ){
				// V.1.02 RevNo110324-004 Modify Line  UBコードのキャッシュメモリへの格納が間違っていた
				ubm_ptr->pbyCacheMem[(MCU_OSM_UBCODE_B_START - madrCacheStartAddr) + j] = pUBCode->byUBCodeB[i];
				bUbcodeChange = TRUE;
			}
		}
	}

	if (bUbcodeChange == TRUE) {	// UBコードの書き換えが必要な場合
		// V.1.02 RevNo110324-004 Modify Line  ユーザマットのフラグを設定していたをユーザブートマットに修正
		SetUbmBlockWrite(TRUE);	// ブロック書き換えフラグを書き換えありに設定
	}

	return bUbcodeChange;
}
// RevRxNo140109-001 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * フラッシュROM初期化が必要かチェック/初期化情報登録
 * @param  なし
 * @retval TRUE          初期化あり
 * @retval FALSE         初期化なし
 */
//=============================================================================
BOOL GetFlashRomInitInfo(FFW_FCLR_DATA_RX* pAddFclrData)
{
	
	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ

	FFW_UM_FCLR_DATA *pUserFclrData;	// ユーザーマットFCLR構造体ポインタ
	FFW_DM_FCLR_DATA *pDataFclrData;	// データマットFCLR構造体ポインタ


	DWORD	dwBlkNum;			// ブロック総数
	DWORD	dwBlkCnt;			// ブロック検索用カウンタ
	MADDR	madrBlkStartAddr;	// ブロック先頭アドレス
	MADDR	madrBlkEndAddr;		// ブロック終了アドレス
	DWORD	dwBlkSize;			// ブロックサイズ
	DWORD	dwLength;			// RevRxNo140616-001 Append Line
	BYTE	byAccMeans;			// RevRxNo140616-001 Append Line
	BYTE	byAccType;			// RevRxNo140616-001 Append Line
	BOOL	bDwnpEna;			// RevRxNo140616-001 Append Line
	
	DWORD	dwFclrCnt;			// FCLRブロック情報検索用カウンタ
	DWORD	dwAddFclrCnt;		// 初期化ブロック設定用カウンタ
	BOOL	bFclrBlockFlg ;		// ブロックアドレス設定済み

	BOOL	bFclrEna = FALSE;	// 初期化チェック結果

	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();			// データマットキャッシュメモリ構造体情報取得

	pUserFclrData = GetUserFclrData();		// ユーザーマットFCLR構造体情報取得
	pDataFclrData = GetDataFclrData();		// データマットFCLR構造体情報取得
	
	memset(pAddFclrData, 0, sizeof(FFW_FCLR_DATA_RX));

	// 初期化ブロック数カウンタを初期化
	dwAddFclrCnt = 0;

	// ユーザーマットの初期化情報登録
	// ユーザーマットのみの初期化実行ではない場合のみ、本処理を行う。
	if(GetDataFlashEraseFlag() == FALSE){	
		// ブロック総数取得
		dwBlkNum = GetCacheBlockNum(MAREA_USER);
		// 初期化ブロックを検索
		for(dwBlkCnt=0; dwBlkCnt<dwBlkNum; dwBlkCnt++){
			// 初期化ブロックではないに設定
			bFclrBlockFlg = FALSE;
			// ブロック先頭アドレス取得
			GetFlashRomBlockNoInf(MAREA_USER, dwBlkCnt, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkSize); //指定ブロック情報取得

			// RevRxNo140616-001 Append Start
			// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
			// そのため、以後の処理に不都合が生じることはない。
			bDwnpEna = FALSE;
			ChkAreaAccessMethod(madrBlkStartAddr, madrBlkEndAddr, &dwLength, &byAccMeans, &byAccType);
			if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
				// アクセス属性がリード/ライト可またはライトのみ可の場合

				bDwnpEna = TRUE;
			}
			// RevRxNo140616-001 Append End

			// FCLR対象ブロックか検索
			// DWNP実行中の場合
			if(GetDwnpOpenData() == TRUE){
				// RevRxNo140617-001 Modify Start
				if (GetTMEnable() == TRUE) {		// TM機能有効の場合
					if (ChkTmArea(madrBlkStartAddr) == FALSE) {		// ブロック先頭アドレスがTM領域内にない場合
						// RevRxNo140616-001 Modify Start
						if (bDwnpEna == TRUE) {						// アクセス属性がリード/ライト可またはライトのみ可の場合
							// V.1.02 RevNo110309-001 Append Line
							if (GetUmCacheDataInit() == TRUE) {	 		// 初期化ブロックのキャッシュ内容を0xFFで埋めてある場合
								// FCLR対象ブロックか検索
								for (dwFclrCnt=0; dwFclrCnt<pUserFclrData->dwNum; dwFclrCnt++) {
									// FCLR対象ブロックの場合
									if (pUserFclrData->dwmadrBlkStart[dwFclrCnt] == madrBlkStartAddr) {
										// 初期化ブロックであるに設定
										bFclrBlockFlg = TRUE;
										break;
									}
								}
							// V.1.02 RevNo110309-001 Append Start
							} else {		// 初期化ブロックのキャッシュ内容を0xFFで埋めてある場合
								// FILL等であるブロックのみ書き換えた状態で、RAMへのダウンロードが発生した場合に
								// 初期化する必要のないブロックまで初期化してしまわないようにするため、ここでは
								// 初期化ブロックではないということにする。
							}
							// V.1.02 RevNo110309-001 Append End
						}
						// RevRxNo140616-001 Modify End
					}
				} else {					// TM機能無効の場合
					// RevRxNo140616-001 Modify Start
					if (bDwnpEna == TRUE) {						// アクセス属性がリード/ライト可またはライトのみ可の場合
						// V.1.02 RevNo110309-001 Append Line
						if (GetUmCacheDataInit()==TRUE ) {		// 初期化ブロックのキャッシュ内容を0xFFで埋めてある場合
							// FCLR対象ブロックか検索
							for (dwFclrCnt=0; dwFclrCnt<pUserFclrData->dwNum; dwFclrCnt++) {
								// FCLR対象ブロックの場合
								if (pUserFclrData->dwmadrBlkStart[dwFclrCnt] == madrBlkStartAddr) {
									// 初期化ブロックであるに設定
									bFclrBlockFlg = TRUE;
									break;
								}
							}
						// V.1.02 RevNo110309-001 Append Start
						} else {		// 初期化ブロックのキャッシュ内容を0xFFで埋めてある場合
							// FILL等であるブロックのみ書き換えた状態で、RAMへのダウンロードが発生した場合に
							// 初期化する必要のないブロックまで初期化してしまわないようにするため、ここでは
							// 初期化ブロックではないということにする。
						}
						// V.1.02 RevNo110309-001 Append End
					}
					// RevRxNo140616-001 Modify End
				}
				// RevRxNo140617-001 Modify End
			}
			// 初期化ブロックの場合
			if(bFclrBlockFlg==TRUE){
				// FCLRブロックアドレスを初期化アドレスに設定
				pAddFclrData->dwmadrBlkStart[dwAddFclrCnt] = madrBlkStartAddr;
				// 初期化領域を設定
				pAddFclrData->byFlashType[dwAddFclrCnt] = INIT_UM;
				// 初期化ブロック数を更新
				dwAddFclrCnt++;
				// 初期化ブロックありに設定
				bFclrEna = TRUE;
			}
			// 初期化ブロックではない場合
			else{
				// ブロック書き換えフラグが書き換えありの場合
				if(um_ptr->pbyBlockWriteFlag[dwBlkCnt] == TRUE){
					// ブロック先頭アドレスを初期化先頭アドレスに設定
					pAddFclrData->dwmadrBlkStart[dwAddFclrCnt] = madrBlkStartAddr;
					// 初期化領域を設定
					pAddFclrData->byFlashType[dwAddFclrCnt] = INIT_UM;
					// 初期化ブロック数を更新
					dwAddFclrCnt++;
					// 初期化ブロックありに設定
					bFclrEna = TRUE;
				}
			}
		}
	}


	// データマットの初期化情報登録

	// ブロック総数取得
	dwBlkNum = GetCacheBlockNum(MAREA_DATA);
	// 初期化ブロックを検索
	for(dwBlkCnt=0; dwBlkCnt<dwBlkNum; dwBlkCnt++){
		// 初期化ブロックではないに設定
		bFclrBlockFlg = FALSE;
		// ブロック先頭アドレス取得
		GetFlashRomBlockNoInf(MAREA_DATA, dwBlkCnt, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkSize); //指定ブロック情報取得

		// RevRxNo140616-001 Append Start
		// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
		// そのため、以後の処理に不都合が生じることはない。
		bDwnpEna = FALSE;
		ChkAreaAccessMethod(madrBlkStartAddr, madrBlkEndAddr, &dwLength, &byAccMeans, &byAccType);
		if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
			// アクセス属性がリード/ライト可またはライトのみ可の場合

			bDwnpEna = TRUE;
		}
		// RevRxNo140616-001 Append End

		// FCLR対象ブロックか検索
		// DWNP実行中の場合
		if(GetDwnpOpenData() == TRUE){
			// RevRxNo140616-001 Modify Start
			if (bDwnpEna == TRUE) {						// アクセス属性がリード/ライト可またはライトのみ可の場合
				// FCLR対象ブロックか検索
				for(dwFclrCnt=0; dwFclrCnt<pDataFclrData->dwNum; dwFclrCnt++){
					// FCLR対象ブロックの場合
					if(pDataFclrData->dwmadrBlkStart[dwFclrCnt] == madrBlkStartAddr){
						// 初期化ブロックであるに設定
						bFclrBlockFlg = TRUE;
						break;
					}
				}
			}
			// RevRxNo140616-001 Modify End
		}

		// 初期化ブロックの場合
		if(bFclrBlockFlg==TRUE){
			// データマット初期化未実行の場合
			if(GetDataFlashInitFlag() == FALSE){
				// FCLRブロックアドレスを初期化アドレスに設定
				pAddFclrData->dwmadrBlkStart[dwAddFclrCnt] = madrBlkStartAddr;
				// 初期化領域を設定
				pAddFclrData->byFlashType[dwAddFclrCnt] = INIT_DM_UBM;
				// 初期化ブロック数を更新
				dwAddFclrCnt++;
				// 初期化ブロックありに設定
				bFclrEna = TRUE;
				// キャッシュメモリデータ格納フラグを格納なしに設定
				dm_ptr->pbyCacheSetFlag[dwBlkCnt] = FALSE;
			}
		}
		// 初期化ブロックではない場合
		else{
			// ブロック書き換えフラグが書き換えありの場合
			if(dm_ptr->pbyBlockWriteFlag[dwBlkCnt] == TRUE){
				// ブロック先頭アドレスを初期化先頭アドレスに設定
				pAddFclrData->dwmadrBlkStart[dwAddFclrCnt] = madrBlkStartAddr;
				// 初期化領域を設定
				pAddFclrData->byFlashType[dwAddFclrCnt] = INIT_DM_UBM;
				// 初期化ブロック数を更新
				dwAddFclrCnt++;
				// 初期化ブロックありに設定
				bFclrEna = TRUE;
			}
		}
	}


	// ユーザーブートマットの初期化情報登録
	// ブロック先頭アドレス取得
	// データマットのみの初期化実行ではない場合のみ、本処理を行う。
	if(GetDataFlashEraseFlag() == FALSE){	
		// 初期化ブロックではないに設定
		bFclrBlockFlg = FALSE;
		// ブロック先頭アドレス取得
		GetFlashRomBlockNoInf(MAREA_USERBOOT, dwBlkCnt, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkSize); //指定ブロック情報取得

		// RevRxNo140616-001 Append Start
		// 個別制御指定がない領域の場合、ChkAreaAccessMethod()関数内でアクセス属性はリード/ライト可となる。
		// そのため、以後の処理に不都合が生じることはない。
		bDwnpEna = FALSE;
		ChkAreaAccessMethod(madrBlkStartAddr, madrBlkEndAddr, &dwLength, &byAccMeans, &byAccType);
		if ((byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
			// アクセス属性がリード/ライト可またはライトのみ可の場合

			bDwnpEna = TRUE;
		}
		// RevRxNo140616-001 Append End

		// FCLR対象ブロックか検索
		// DWNP実行中の場合
		if ( GetDwnpOpenData() == TRUE ){
			// RevRxNo140616-001 Modify Start
			if (bDwnpEna == TRUE) {						// アクセス属性がリード/ライト可またはライトのみ可の場合
				// FCLR対象ブロックの場合
				if(GetUserBootMatFclr() == TRUE){
					// 初期化ブロックであるに設定
					bFclrBlockFlg = TRUE;
				}
			}
			// RevRxNo140616-001 Modify End
		}
		// 初期化ブロックの場合
		if(bFclrBlockFlg==TRUE){
			// FCLRブロックアドレスを初期化アドレスに設定
			pAddFclrData->dwmadrBlkStart[dwAddFclrCnt] = madrBlkStartAddr;
			// 初期化領域を設定
			pAddFclrData->byFlashType[dwAddFclrCnt] = INIT_DM_UBM;
			// 初期化ブロック数を更新
			dwAddFclrCnt++;
			// 初期化ブロックありに設定
			bFclrEna = TRUE;
		}
		// 初期化ブロックではない場合
		else{
			// ブロック書き換えフラグが書き換えありの場合
			if(GetUbmBlockWrite() == TRUE){
				// ブロック先頭アドレスを初期化先頭アドレスに設定
				pAddFclrData->dwmadrBlkStart[dwAddFclrCnt] = madrBlkStartAddr;
				// 初期化領域を設定
				pAddFclrData->byFlashType[dwAddFclrCnt] = INIT_DM_UBM;
				// 初期化ブロック数を更新
				dwAddFclrCnt++;
				// 初期化ブロックありに設定
				bFclrEna = TRUE;
			}
		}
	}
	
	// 初期化ブロック数を設定
	pAddFclrData->dwNum = dwAddFclrCnt;

	return bFclrEna;	// 初期化ブロックあり/なしを返信

}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * フラッシュROMキャッシュからデータを取得する。
 * @param  eAreaType Flash領域タイプ
 * @param  madrStartAddr 開始アドレス
 * @param  dwAccessCount     DUMP回数
 * @param  eAccessSize   アクセスサイズ
 * @param  pbyReadBuff   読込データ格納ポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
void GetFlashRomCacheData(enum FFWRXENM_MAREA_TYPE eAreaType, MADDR madrStartAddr, DWORD dwAccessCount, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyReadBuff)
{

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

	BYTE*	pbyBuff;		// 参照データを格納する領域のアドレス
	DWORD	dwDataLen;
	MADDR	madrCacheStart;

	DWORD	dwTotalLength;		//取得するデータ全体のバイトサイズ

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーブートマットキャッシュメモリ構造体情報取得

	pbyBuff = pbyReadBuff;

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwDataLen = 1;
		break;
	case MWORD_ACCESS:
		dwDataLen = 2;
		break;
	case MLWORD_ACCESS:
		dwDataLen = 4;
		break;
	default:
		dwDataLen = 4;
		break;
	}

	dwTotalLength = dwDataLen * dwAccessCount;

	// ユーザマット領域
	if(eAreaType == MAREA_USER){
		um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得

		// 指定アドレスに対応するキャッシュメモリの位置を算出
		madrCacheStart = madrStartAddr - GetCacheStartAddr(eAreaType);
		memcpy(pbyBuff, &um_ptr->pbyCacheMem[madrCacheStart], dwTotalLength);
	}

	// データマット領域
	else if(eAreaType == MAREA_DATA){
		dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得

		// 指定アドレスに対応するキャッシュメモリの位置を算出
		madrCacheStart = madrStartAddr - GetCacheStartAddr(eAreaType);
		memcpy(pbyBuff, &dm_ptr->pbyCacheMem[madrCacheStart], dwTotalLength);
	}

	// ユーザーブートマット領域
	else{
		ubm_ptr = GetUserBootMatCacheData();				// ユーザーブートマットキャッシュメモリ構造体情報取得
		// 指定アドレスに対応するキャッシュメモリの位置を算出
		madrCacheStart = madrStartAddr - GetCacheStartAddr(eAreaType);
		memcpy(pbyBuff, &ubm_ptr->pbyCacheMem[madrCacheStart], dwTotalLength);
	}
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End


///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
// RevRxNo140109-001 Modify Start
// V.1.02 RevNo110418-002 Modify Start
//=============================================================================
/**
 * IDコード設定領域&OFS1レジスタをマスクする。
 * @param  なし
 * @retval なし
 */
//=============================================================================
static void maskIdCodeArea(void)
{
	BOOL						bDebugContinue = FALSE;
	BOOL						bOfs1LvdChange = FALSE;
	BOOL						bIdcodeChange_tmp = FALSE;
	//RevRxNo140515-011 Delete Line
	enum FFWRXENM_PMODE			ePmode;				// 起動時に指定された端子設定起動モード
	MADDR						madrBlkStartAddr;
	MADDR						madrBlkEndAddr;
	DWORD						dwBlkNo;
	DWORD						dwBlkSize;
	FFWENM_ENDIAN				eMcuEndian;
	USER_ROMCACHE_RX			*um_ptr;	// ユーザマットキャッシュメモリ構造体ポインタ
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	FFWMCU_DBG_DATA_RX*			pDbgData;
	FFWMCU_FWCTRL_DATA*			pFwCtrl;	// RevRxNo130411-001 Append Line

	ePmode = GetPmodeDataRX();
	eMcuEndian = GetEndianDataRX();	// エンディアン情報取得
	um_ptr = GetUserMatCacheData();		// ユーザマットキャッシュメモリ構造体情報取得
	pMcuInfo = GetMcuInfoDataRX();	// MCU情報取得
	pDbgData = GetDbgDataRX();		// デバッグ情報取得
	pFwCtrl = GetFwCtrlData();		// RevRxNo130411-001 Append Line

	// RevRxNo140617-001 Append Start
	if (GetTMEnable() == TRUE) {			// TM機能有効の場合
		setExtraCacheTmData(eMcuEndian);		// EXTRA用キャッシュメモリのTMINF領域データを、起動時に取得しておいた内容に差し替える
	}
	// RevRxNo140617-001 Append End

	// 通常モード時のみマスクする
	if (pDbgData->eWriter == EML_WRITERMODE_NON) {
		// V.1.02 RevNo110418-002 Delete　MDE,UBコード領域チェック
		// MDE、UBコード領域のチェックはFILL/WRITE/CWRITE時はFFWMCUCmd_xxxx関数先頭で実施するためSetFlashRomCacheMem()内でのチェックは不要
		// ダウンロード時のみチェックすればよいので、MaskMdeUbCodeArea()としてWriteFlashStart()で実施するようにする。

		// OFS1レジスタのデバッグ継続モード有効および電圧監視リセット無効情報一致確認と設定を実施
		if (pFwCtrl->eOfsType == RX_OFS_FLASH) {		// OFSレジスタをフラッシュROMで設定するMCUの場合
			// OFS1があるフラッシュROM領域のブロック番号を取得する
			GetFlashRomBlockInf(MAREA_USER, MCU_OSM_OFS1_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

			if (um_ptr->pbyCacheSetFlag[dwBlkNo] == TRUE) {		// OFS1があるフラッシュROM領域のキャッシュがある場合
				if ((pMcuInfo->byInterfaceType == IF_TYPE_1) && (ePmode == RX_PMODE_SNG)) {		// FINE＋シングルチップ起動の場合のみ(デバッグ継続モードにする)
					// デバッグ継続モード有効情報一致確認と設定
					// デバッグ継続モード有効設定時はワーニング表示を実施する必要がないため、
					// 戻り値チェックは実施しない。
					bDebugContinue = checkFlashCacheOfsDbg(eMcuEndian);
				}

				if (pFwCtrl->eLvdErrType == RX_LVD_ENA_CHK) {	// LVDx有効設定チェックを実施するMCUの場合
					// OFS1レジスタ電圧監視リセット無効情報一致確認と設定
					bOfs1LvdChange = checkFlashCacheOfsLvd(eMcuEndian);
					if (bOfs1LvdChange == TRUE) {		// 電圧監視リセットを無効に設定した場合
						s_bOfs1Lvd1renChangeFlg = TRUE;	// OFS1レジスタの電圧監視リセットを無効設定に変更したフラグに"変更した"を設定
					}
				}
			}
		} else if (pFwCtrl->eOfsType == RX_OFS_EXTRA) {	// OFSレジスタをExtra領域で設定するMCUの場合
			if (GetExtraCacheSet() == TRUE) {		// OFS1があるExtra領域のキャッシュがある場合
				if ((pMcuInfo->byInterfaceType == IF_TYPE_1) && (ePmode == RX_PMODE_SNG)) {		// FINE＋シングルチップ起動の場合のみ(デバッグ継続モードにする)
					// デバッグ継続モード有効情報一致確認と設定
					// デバッグ継続モード有効設定時はワーニング表示を実施する必要がないため、
					// 戻り値チェックは実施しない。
					bDebugContinue = CheckExtraCacheOfsDbg(eMcuEndian);		// RevRxNo130730-001 Modify Line
				}

				if (pFwCtrl->eLvdErrType == RX_LVD_ENA_CHK) {	// LVDx有効設定チェックを実施するMCUの場合
					// OFS1レジスタ電圧監視リセット無効情報一致確認と設定
					bOfs1LvdChange = checkExtraCacheOfsLvd(eMcuEndian);
					if (bOfs1LvdChange == TRUE) {		// 電圧監視リセットを無効に設定した場合
						s_bOfs1Lvd1renChangeFlg = TRUE;	// OFS1レジスタの電圧監視リセットを無効設定に変更したフラグに"変更した"を設定
					}
				}
			}
		}


		// IDコード情報一致確認と設定を実施
		if (pFwCtrl->eIdcodeType == RX_IDCODE_FLASH) {	// IDコードをフラッシュROMで設定するMCUの場合
			GetFlashRomBlockInf(MAREA_USER, MCU_IDCODE_ADDR_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

			if (um_ptr->pbyCacheSetFlag[dwBlkNo] == TRUE) {		// IDコードがあるフラッシュROM領域のキャッシュがある場合
				// IDコード更新
				// IDコード 0xFFの設定時はワーニング表示を実施する必要がないため、
				// 戻り値チェックは実施しない。
				bIdcodeChange_tmp = checkFlashCacheIdcode();		//RevRxNo140515-011 Append Line
				if (s_bIdcodeChange == FALSE) {						//RevRxNo140515-011 Append Line
					s_bIdcodeChange = bIdcodeChange_tmp;		//RevRxNo140515-011 Modify Line
				}
			}
		} else {	// IDコードをExtra領域で設定するMCUの場合
			if (GetExtraCacheSet() == TRUE) {		// IDコードがあるExtra領域のキャッシュがある場合
				// IDコード更新
				// IDコード 0xFFの設定時はワーニング表示を実施する必要がないため、
				// 戻り値チェックは実施しない。
				bIdcodeChange_tmp = checkExtraCacheIdcode();		//RevRxNo140515-011 Append Line
				if (s_bIdcodeChange == FALSE) {						//RevRxNo140515-011 Append Line
					s_bIdcodeChange = bIdcodeChange_tmp;		//RevRxNo140515-011 Modify Line
				}
			}
		}
	}

	return;
}


//RevRxNo140515-011 Append Start
//=============================================================================
/**
 * IDコード書き換えフラグ取得
 * @param なし
 * @retval s_bIdcodeChange IDコードフラグ
 */
//=============================================================================
BOOL GetIdcodeChange(void)
{
	return s_bIdcodeChange;
}

//RevRxNo140515-011 Append End

//RevRxNo140515-011 Append Start
//=============================================================================
/**
 * IDコード書き換えフラグ取得
 * @param bIdcodeChange	IDコード書き換えフラグ設定値
 * @retval なし
 */
//=============================================================================
void SetIdcodeChange(BOOL bIdcodeChange)
{
		s_bIdcodeChange = bIdcodeChange;	
}

//RevRxNo140515-011 Append End

// V.1.02 RevNo110418-002 Modify End
// RevRxNo140109-001 Modify End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * フラッシュROM上のデバッグ継続ビットをマスクする。
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @retval TRUE  : デバッグ継続モード有効設定必要
 * @retval FALSE : デバッグ継続モード有効設定不要
 */
//=============================================================================
static BOOL checkFlashCacheOfsDbg(enum FFWENM_ENDIAN eMcuEndian)
{
	BOOL				bDebugContinue = FALSE;
	MADDR				madrBlkStartAddr;
	MADDR				madrBlkEndAddr;
	DWORD				dwBlkNo;
	DWORD				dwBlkSize;
	DWORD				dwCacheCnt;
	USER_ROMCACHE_RX	*um_ptr;				// ユーザマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();		// ユーザマットキャッシュメモリ構造体情報取得

	dwCacheCnt = MCU_OSM_OFS1_START - GetCacheStartAddr(MAREA_USER);	// フラッシュROM用キャッシュメモリ位置算出

	if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
		dwCacheCnt += 3;
	}

	if ((um_ptr->pbyCacheMem[dwCacheCnt] & MCU_OSM_OFS1_OCD_MASK) != MCU_OSM_OFS1_OCDMOD_DEBUG) { // デバッグ継続モードではない場合
		// RevRxNo111215-001 Modify Start
		// FINE LIVEデバッグモードはサポートしていない
		um_ptr->pbyCacheMem[dwCacheCnt] &= ~MCU_OSM_OFS1_OCD_MASK;		// OCDMODビットを0クリア
		um_ptr->pbyCacheMem[dwCacheCnt] |= MCU_OSM_OFS1_OCDMOD_DEBUG;	// デバッグ継続モード値設定
		// RevRxNo111215-001 Modify End
		bDebugContinue = TRUE;
	}

	if(bDebugContinue){		// フラッシュ書き換えが必要な場合
		GetFlashRomBlockInf(MAREA_USER, MCU_OSM_OFS1_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
		um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;		// ブロック書き換えフラグを書き換えありに設定
		um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定
	}

	return bDebugContinue;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * フラッシュROM上のOFS1レジスタ電圧監視リセットをマスクする。
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @retval TRUE  : LVD機能無効設定必要
 * @retval FALSE : LVD機能無効設定不要
 */
//=============================================================================
static BOOL checkFlashCacheOfsLvd(enum FFWENM_ENDIAN eMcuEndian)
{
	BOOL					bOfs1LvdChange = FALSE;
	BYTE					byOfs1LvdDis;
	BYTE					byOfs1LvdMask;
	MADDR					madrBlkStartAddr;
	MADDR					madrBlkEndAddr;
	DWORD					dwBlkNo;
	DWORD					dwBlkSize;
	DWORD					dwCacheCnt;
	USER_ROMCACHE_RX		*um_ptr;				// ユーザマットキャッシュメモリ構造体ポインタ
	FFWMCU_MCUDEF_DATA*		pMcuDef;

	um_ptr = GetUserMatCacheData();		// ユーザマットキャッシュメモリ構造体情報取得
	pMcuDef = GetMcuDefData();

	// LVD無効の設定値およびマスク値を生成
	byOfs1LvdDis = static_cast<BYTE>(pMcuDef->dwOfs1LvdDis);
	byOfs1LvdMask = static_cast<BYTE>(pMcuDef->dwOfs1LvdMask);

	dwCacheCnt = MCU_OSM_OFS1_START - GetCacheStartAddr(MAREA_USER);	// フラッシュROM用キャッシュメモリ位置算出

	if (eMcuEndian == ENDIAN_BIG) {		// ビッグエンディアンの場合
		dwCacheCnt += 3;
	}

	if ((um_ptr->pbyCacheMem[dwCacheCnt] & byOfs1LvdMask) != byOfs1LvdDis) {
		// LVDが無効設定ではなかった場合

		// LVD無効設定
		um_ptr->pbyCacheMem[dwCacheCnt] &= ~byOfs1LvdMask;
		um_ptr->pbyCacheMem[dwCacheCnt] |= byOfs1LvdDis;
		bOfs1LvdChange = TRUE;	// OFS1レジスタ電圧監視リセットを無効設定に変更
	}

	if(bOfs1LvdChange){		// フラッシュ書き換えが必要な場合
		GetFlashRomBlockInf(MAREA_USER, MCU_OSM_OFS1_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
		um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;		// ブロック書き換えフラグを書き換えありに設定
		um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定
	}

	return bOfs1LvdChange;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * フラッシュROM上のIDコード値をマスクする。
 * @param  なし
 * @retval TRUE  : IDコード書き換え必要
 * @retval FALSE : IDコード書き換え不要
 */
//=============================================================================
static BOOL checkFlashCacheIdcode(void)
{
	BOOL				bIdcodeChange = FALSE;
	BYTE				byMaskData = 0xFF;
	MADDR				madrBlkStartAddr;
	MADDR				madrBlkEndAddr;
	DWORD				dwBlkNo;
	DWORD				dwBlkSize;
	DWORD				dwCacheCnt;
	DWORD				dwLoopAddr;
	USER_ROMCACHE_RX	*um_ptr;	// ユーザマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();		// ユーザマットキャッシュメモリ構造体情報取得

	// IDコードを0xFFhにマスク
	dwCacheCnt = MCU_IDCODE_ADDR_START - GetCacheStartAddr(MAREA_USER);	// キャッシュメモリ開始位置を取得
	for (dwLoopAddr = MCU_IDCODE_ADDR_START; dwLoopAddr <= MCU_IDCODE_ADDR_END; dwLoopAddr++) {
		if (um_ptr->pbyCacheMem[dwCacheCnt] != byMaskData) {		// キャッシュメモリの値が0xFFではなかった場合
			memcpy(&um_ptr->pbyCacheMem[dwCacheCnt], &byMaskData, sizeof(BYTE));
			bIdcodeChange = TRUE;	// IDコード値を0xFFに設定
		}
		dwCacheCnt++;				// キャッシュメモリ開始位置をインクリメント
	}

	if(bIdcodeChange){		// フラッシュ書き換えが必要な場合
		GetFlashRomBlockInf(MAREA_USER, MCU_IDCODE_ADDR_START, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
		um_ptr->pbyBlockWriteFlag[dwBlkNo] = TRUE;		// ブロック書き換えフラグを書き換えありに設定
		um_ptr->pbyBlockAreaWriteFlag[dwBlkNo/CACHE_BLOCK_AREA_NUM_RX] = TRUE;		// ブロック領域書き換えフラグを書き換えありに設定
	}

	return bIdcodeChange;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * Extra領域上のデバッグ継続ビットをマスクする。
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @retval TRUE  : デバッグ継続モード有効設定必要
 * @retval FALSE : デバッグ継続モード有効設定不要
 */
//=============================================================================
// RevRxNo130730-001 Modify Line
BOOL CheckExtraCacheOfsDbg(enum FFWENM_ENDIAN eMcuEndian)
{
	BOOL	bDebugContinue = FALSE;
	DWORD	dwCacheCnt;
	DWORD	dwCnt;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	dwCacheCnt = pMcuDef->dwOfs1StartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出	// RevRxNo150827-002 Modify Line

	if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
		dwCacheCnt += 3;
	}

	if ((s_byExtraCacheMem[dwCacheCnt] & MCU_OSM_OFS1_OCD_MASK) != MCU_OSM_OFS1_OCDMOD_DEBUG) { // デバッグ継続モードではない場合
		s_byExtraCacheMem[dwCacheCnt] &= ~MCU_OSM_OFS1_OCD_MASK;		// OCDMODビットを0クリア
		s_byExtraCacheMem[dwCacheCnt] |= MCU_OSM_OFS1_OCDMOD_DEBUG;	// デバッグ継続モード値設定
		bDebugContinue = TRUE;
	}

	if(bDebugContinue){		// フラッシュ書き換えが必要な場合
		dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
		s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
		s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット
	}

	return bDebugContinue;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * Extra領域上のOFS1レジスタ電圧監視リセットをマスクする。
 * @param  eMcuEndian エンディアン ENDIAN_BIG:ビッグ ENDIAN_LITTLE:リトル
 * @retval TRUE  : LVD機能無効設定必要
 * @retval FALSE : LVD機能無効設定不要
 */
//=============================================================================
static BOOL checkExtraCacheOfsLvd(enum FFWENM_ENDIAN eMcuEndian)
{
	BOOL					bOfs1LvdChange = FALSE;
	BYTE					byOfs1LvdDis;
	BYTE					byOfs1LvdMask;
	DWORD					dwCacheCnt;
	DWORD					dwCnt;
	FFWMCU_MCUDEF_DATA*		pMcuDef;

	pMcuDef = GetMcuDefData();

	// LVD無効の設定値およびマスク値を生成
	byOfs1LvdDis = static_cast<BYTE>(pMcuDef->dwOfs1LvdDis);
	byOfs1LvdMask = static_cast<BYTE>(pMcuDef->dwOfs1LvdMask);

	dwCacheCnt = pMcuDef->dwOfs1StartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出	// RevRxNo150827-002 Modify Line

	if (eMcuEndian == ENDIAN_BIG) {		// ビッグエンディアンの場合
		dwCacheCnt += 3;
	}

	if ((s_byExtraCacheMem[dwCacheCnt] & byOfs1LvdMask) != byOfs1LvdDis) {
		// LVDが無効設定ではなかった場合

		// LVD無効設定
		s_byExtraCacheMem[dwCacheCnt] &= ~byOfs1LvdMask;
		s_byExtraCacheMem[dwCacheCnt] |= byOfs1LvdDis;
		bOfs1LvdChange = TRUE;		// OFS1レジスタ電圧監視リセットを無効設定に変更
	}

	if(bOfs1LvdChange){		// フラッシュ書き換えが必要な場合
		dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
		s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
		s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット
	}

	return bOfs1LvdChange;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * Extra領域上のIDコード値をマスクする。
 * @param  なし
 * @retval TRUE  : IDコード書き換え必要
 * @retval FALSE : IDコード書き換え不要
 */
//=============================================================================
static BOOL checkExtraCacheIdcode(void)
{
	BOOL	bIdcodeChange = FALSE;
	BYTE	byMaskData = 0xFF;
	DWORD	dwCacheCnt;
	DWORD	dwLoopAddr;
	DWORD	dwCnt;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	// IDコードを0xFFhにマスク
	dwCacheCnt = pMcuDef->dwOsisStartOffset - pMcuDef->dwExtraAStartOffset;		// キャッシュメモリ開始位置を取得	// RevRxNo150827-002 Modify Line
	for (dwLoopAddr = pMcuDef->dwOsisStartOffset; dwLoopAddr <= pMcuDef->dwOsisEndOffset; dwLoopAddr++) {	// RevRxNo150827-002 Modify Line
		if (s_byExtraCacheMem[dwCacheCnt] != byMaskData) {		// キャッシュメモリの値が0xFFではなかった場合
			memcpy(&s_byExtraCacheMem[dwCacheCnt], &byMaskData, sizeof(BYTE));
			bIdcodeChange = TRUE;	// IDコード値を0xFFに設定
		}
		dwCacheCnt++;				// キャッシュメモリ開始位置をインクリメント
	}

	if(bIdcodeChange){		// フラッシュ書き換えが必要な場合
		dwCacheCnt = pMcuDef->dwOsisStartOffset - pMcuDef->dwExtraAStartOffset;		// キャッシュメモリ開始位置を取得	// RevRxNo150827-002 Modify Line
		dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
		s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
		s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット
	}

	return bIdcodeChange;
}
// RevRxNo140109-001 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * フラッシュROM初期化ブロックであるかの確認
 * @param  eAreaType 領域指定
 * @param  madrBlkStartAddr ブロック先頭アドレス
 * @param  madrBlkEndAddr ブロック終了アドレス
 * @retval TRUE          初期化あり
 * @retval FALSE         初期化なし
 */
//=============================================================================
static BOOL chkFlashRomInitBlock(enum FFWRXENM_MAREA_TYPE eAreaType, MADDR madrBlkStartAddr, MADDR madrBlkEndAddr)
{

	FFW_UM_FCLR_DATA *pUserFclrData;	
	FFW_DM_FCLR_DATA *pDataFclrData;	

	pUserFclrData = GetUserFclrData();
	pDataFclrData = GetDataFclrData();

	DWORD	dwFclrCnt;

	if(eAreaType == MAREA_USER){
		for(dwFclrCnt=0;dwFclrCnt<pUserFclrData->dwNum; dwFclrCnt++){
			if(pUserFclrData->dwmadrBlkStart[dwFclrCnt] == madrBlkStartAddr && pUserFclrData->dwmadrBlkEnd[dwFclrCnt] == madrBlkEndAddr){
				return TRUE;
			}
		}
	}
	else if(eAreaType == MAREA_DATA){
		for(dwFclrCnt=0;dwFclrCnt<pDataFclrData->dwNum; dwFclrCnt++){
			if(pDataFclrData->dwmadrBlkStart[dwFclrCnt] == madrBlkStartAddr && pDataFclrData->dwmadrBlkEnd[dwFclrCnt] == madrBlkEndAddr){
				return TRUE;
			}
		}
	}
	else{
		if(GetUserBootMatFclr() == TRUE){
				return TRUE;
		}
	}
	return FALSE;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
//=============================================================================
/**
 * ライタモード用キャッシュメモリ初期化
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR InitFlashRomCacheWriter(void)
{
	FFWERR	ferr = FFWERR_OK;
	// V.1.02 RevNo110309-002 Append Line
	FFWERR	ferr2 = FFWERR_OK;
	DWORD dwBlkCnt = 0;						// ブロック数
	DWORD i,j;
	MADDR madrBlkStartAddr;
	MADDR madrBlkEndAddr;
	MADDR madrCacheStart;
	DWORD dwBlkNo;
	DWORD dwBlkSize;
	DWORD dwFclrCnt;
	BYTE byFclrFlg;						
	enum FFWRXENM_MAREA_TYPE eAreaType;	// 領域タイプ

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ
	FFW_UM_FCLR_DATA *pUserFclrData;	// ユーザマット用FCLR情報管理構造体ポインタ	
	FFW_DM_FCLR_DATA *pDataFclrData;	// データマット用FCLR情報管理構造体ポインタ		
	FFWMCU_MCUAREA_DATA_RX*		pMcuAreaData;		// MCU情報構造体のポインタ

	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーマットキャッシュメモリ構造体情報取得
	pUserFclrData = GetUserFclrData();			// ユーザマット用FCLR情報管理構造体情報取得	
	pDataFclrData = GetDataFclrData();			// データマット用FCLR情報管理構造体情報取得	
	pMcuAreaData = GetMcuAreaDataRX();	// MCU情報を取得

	// V.1.02 RevNo110601-001 Append Start( 応技評価で問題となったライタモードでエラーとなる不具合改修 )
	MADDR madrBlkStartAddrFclr;
	MADDR madrBlkEndAddrFclr;
	DWORD dwBlkNoFclr;
	DWORD dwBlkSizeFclr;
	// V.1.02 RevNo110601-001 Append End

	// ユーザマット用キャッシュメモリ初期化
	// ブロック領域書き換えフラグ数を算出
	if(GetNewCacheMem(MAREA_USER) == TRUE){
		dwBlkCnt = 0;
		for( i = 0; i < pMcuAreaData->dwFlashRomPatternNum; i++ ){
			for( j = 0; j < pMcuAreaData->dwFlashRomBlockNum[i]; j++ ){
				// ブロック情報設定
				madrBlkStartAddr = pMcuAreaData->dwmadrFlashRomStart[i] + (pMcuAreaData->dwFlashRomBlockSize[i] * j);
				madrBlkEndAddr = madrBlkStartAddr + pMcuAreaData->dwFlashRomBlockSize[i] -1;
				eAreaType = MAREA_USER;		// ユーザマット
				dwBlkNo = dwBlkCnt;
				dwBlkSize = pMcuAreaData->dwFlashRomBlockSize[i];

				// 指定ブロックがFCLR対象ブロックかチェック
				byFclrFlg = FALSE;
				for(dwFclrCnt=0;dwFclrCnt<pUserFclrData->dwNum; dwFclrCnt++){
					if( pUserFclrData->dwmadrBlkStart[dwFclrCnt] <= madrBlkStartAddr && madrBlkStartAddr <= pUserFclrData->dwmadrBlkEnd[dwFclrCnt] ){
						byFclrFlg = TRUE;	// 初期化ブロックに開始アドレスが含まれる
						break;
					}
				}
				if( byFclrFlg ){		// 初期化ブロックの場合
					// 指定ブロック番号に対応するキャッシュメモリの位置を算出
					madrCacheStart = madrBlkStartAddr - GetCacheStartAddr(eAreaType);
					// キャッシュデータ登録
					memset(&um_ptr->pbyCacheMem[madrCacheStart],0xFF ,dwBlkSize);
					um_ptr->pbyCacheSetFlag[dwBlkNo] = TRUE;	// キャッシュメモリデータ設定フラグを設定済みに設定
				}else{		// 上書きブロックの場合
					// 1ブロック分のキャッシュメモリを更新
					ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
				dwBlkCnt++;
			}
		}
	}

	// データマット領域キャッシュメモリ管理フラグを初期化
	if(GetNewCacheMem(MAREA_DATA) == TRUE){
		ferr = SetRegDataFlashReadEnable(TRUE);			// データフラッシュ読み出し許可設定
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// FCLR対象ブロックにデータマットブロックが含まれる場合(イレーズしておいてからそのブロックのデータを取得する)
		if(ChkDataFlashRomFclr() == TRUE){
			// データマットのみの初期化実行フラグをセット
			SetDataFlashEraseFlag(TRUE);
			// フラッシュROMへのライト開始処理
			ferr = WriteFlashStart(VERIFY_OFF);

			// フラッシュROMへのライト終了処理
			// V.1.02 RevNo110309-002 Modify Line
			ferr2 = WriteFlashEnd();
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// V.1.02 RevNo110309-002 Append Start
			if (ferr2 != FFWERR_OK) {	// WriteFlashStart()でエラーが出ていない場合でWriteFlashEnd()でエラーの場合
				return ferr2;			// WriteFlashEnd()のエラーを返す
			}
			// V.1.02 RevNo110309-002 Append End
			SetDataFlashInitFlag(TRUE);	// データマット初期化フラグを実行済みに設定
			SetDataFlashEraseFlag(FALSE);	// データマットのみの初期化実行フラグをクリア
			// V.1.02 RevNo110601-001 Modify Start( 応技評価で問題となったライタモードでエラーとなる不具合改修 )
			// 初期化ブロックのみキャッシュメモリデータ設定フラグを未設定に設定する必要あり
			// FCLR対象ブロックか検索
			for(dwFclrCnt=0; dwFclrCnt<pDataFclrData->dwNum; dwFclrCnt++){
				// 対象ブロック情報取得
				GetFlashRomBlockInf( MAREA_DATA, pDataFclrData->dwmadrBlkStart[dwFclrCnt], &madrBlkStartAddrFclr, &madrBlkEndAddrFclr, &dwBlkNoFclr, &dwBlkSizeFclr);
				dm_ptr->pbyCacheSetFlag[dwBlkNoFclr] = FALSE;	// キャッシュメモリデータ設定フラグを未設定に設定
			}
			// V.1.02 RevNo110601-001 Modify End( 応技評価で問題となったライタモードでエラーとなる不具合改修 )
		}
		
		dwBlkCnt = 0;
		for( i = 0; i < pMcuAreaData->dwDataFlashRomPatternNum; i++ ) {
			for( j = 0; j < pMcuAreaData->dwDataFlashRomBlockNum[i]; j++ ){
				// ブロック情報設定
				madrBlkStartAddr = pMcuAreaData->dwmadrDataFlashRomStart[i] + (pMcuAreaData->dwDataFlashRomBlockSize[i] * j);
				madrBlkEndAddr = madrBlkStartAddr + pMcuAreaData->dwDataFlashRomBlockSize[i] -1;
				eAreaType = MAREA_DATA;		// データマット
				dwBlkNo = dwBlkCnt;
				dwBlkSize = pMcuAreaData->dwDataFlashRomBlockSize[i];
				// 1ブロック分のキャッシュメモリを更新
				ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				dwBlkCnt++;
			}
		}
		ferr = SetRegDataFlashReadEnable(FALSE);			// データフラッシュ読み出し許可にしたのを元に戻す
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	
	// ユーザーブートマット領域キャッシュメモリ管理フラグを初期化
	if(GetNewCacheMem(MAREA_USERBOOT) == TRUE){
		// ブロック情報設定
		madrBlkStartAddr = pMcuAreaData->dwmadrUserBootStart;
		madrBlkEndAddr = pMcuAreaData->dwmadrUserBootEnd;
		eAreaType = MAREA_USERBOOT;		// ユーザブートマット
// RevNo120110-001 Modify Start
		/*
			注)
				dwBlkNo はUpdateFlashRomCache() のパラメータとして使用されるが、
				eAreaType がMAREA_USERBOOT の場合、UpdateFlashRomCache() 内で
				dwBlkNoは未使用である。
		*/
		dwBlkNo = 0;
// RevNo120110-001 Modify End
		dwBlkSize = pMcuAreaData->dwmadrUserBootEnd - pMcuAreaData->dwmadrUserBootStart + 1;
		if(GetUserBootMatFclr() == TRUE){		// 初期化ブロック指定の場合
			// 指定ブロック番号に対応するキャッシュメモリの位置を算出
			madrCacheStart = madrBlkStartAddr - GetCacheStartAddr(eAreaType);
			// キャッシュデータ登録
			memset(&ubm_ptr->pbyCacheMem[madrCacheStart],0xFF, dwBlkSize);
		}else{
			// 1ブロック分のキャッシュメモリを更新
			ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}
	return ferr;
}
// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append End
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * キャッシュメモリ構造体の領域確保チェック/ 領域開放
 * @param なし
 * @retval なし
 */
//=============================================================================
void DeleteFlashRomCacheMem(void)
{

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーブートマットキャッシュメモリ構造体情報取得

	// キャッシュメモリ管理構造体領域解放(既にキャッシュメモリ構造体領域が確保されている場合)
	if(s_bUmNewCacheMemFlag == TRUE){
		delete um_ptr->pbyCacheMem;
		delete um_ptr->pbyCacheSetFlag;
		delete um_ptr->pbyBlockWriteFlag;
		delete um_ptr->pbyBlockAreaWriteFlag;
		s_bUmNewCacheMemFlag = FALSE ;	 // キャッシュメモリ確保フラグを未確保に設定
	}

	// データマット用キャッシュメモリ管理構造体
	if(s_bDmNewCacheMemFlag == TRUE){
		delete dm_ptr->pbyCacheMem;
		delete dm_ptr->pbyCacheSetFlag;
		delete dm_ptr->pbyBlockWriteFlag;
		delete dm_ptr->pbyBlockAreaWriteFlag;
		delete dm_ptr->pbyProgSizeWriteFlag;
		s_bDmNewCacheMemFlag = FALSE ;	 // キャッシュメモリ確保フラグを未確保に設定
	}

	// ユーザーブートマット用キャッシュメモリ管理構造体
	if(s_bUbmNewCacheMemFlag == TRUE){
		delete ubm_ptr->pbyCacheMem;
		s_bUbmNewCacheMemFlag = FALSE ;	 // キャッシュメモリ確保フラグを未確保に設定
	}
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * キャッシュメモリ構造体領域確保
 * @param *pMcuArea  MCU 別情報格納構造体のポインタ
 * @retval なし
 */
//=============================================================================
void NewFlashRomCacheMem(const FFWRX_MCUAREA_DATA* pMcuArea)
{

	MADDR	dwmadrStart;				// 先頭アドレス
	MADDR	dwmadrEnd;					// 終了アドレス
	DWORD	dwBlockNum;					// ブロック数
	DWORD	dwAreaSize;					// メモリサイズ
	DWORD	dwBlockSizeMin;				// 最小ブロックサイズ
	DWORD	dwProgSizeWriteNum;			// 書き込みサイズ単位書き込みフラグ数
	DWORD	dwBlockAreaWriteNum;		// ブロック領域書き込みフラグ数
	DWORD	dwRestBlkNum;				// ブロック領域端数ブロック数

	MADDR	dwmadrAreaStart;			// MCU領域パターン先頭アドレス
	MADDR	dwmadrAreaEnd;				// MCU領域パターン終了アドレス
	DWORD	dwAreaBlockNum;				// MCU領域パターンブロック数
	DWORD	dwAreaBlockSize;			// MCU領域パターンブロックサイズ
	DWORD	dwBlockSize;				// MCUブロックサイズ

	DWORD	dwMcuRomPtCnt;				// MCU領域パターンカウンタ

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

	// ユーザーマット領域キャッシュメモリ確保処理
	// ユーザーマット領域のMCU情報を取得
	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	if(pMcuArea->dwFlashRomPatternNum == 0){
		s_madrUmCacheMemStartAddr = 0;			// 先頭アドレスを初期化
		s_madrUmCacheMemEndAddr = 0;			// 終了アドレスを初期化
		s_dwUmCacheBlockNum = 0;				// ブロック数を初期化
		s_dwUmCacheMemSize = 0;					// メモリサイズを初期化
		s_dwUmProgSize = 0;						// 書き込みサイズを初期化
		s_bUmNewCacheMemFlag = FALSE;			// キャッシュメモリ確保フラグを初期化
	}
	else{
		dwmadrStart = 0xFFFFFFFF;		// 先頭アドレスを初期化
		dwmadrEnd = 0x00000000;			// 終了アドレスを初期化
		dwBlockNum = 0;					// ブロック数を初期化
		dwAreaSize = 0;					// サイズを初期化
		dwBlockSizeMin = 0xFFFFFFFF;	// 最小ブロックサイズを初期化

		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwFlashRomPatternNum; dwMcuRomPtCnt++) {
			// MCU領域パターン先頭アドレス取得
			dwmadrAreaStart = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt];
			// MCU領域パターン終了アドレス取得
			dwmadrAreaEnd = pMcuArea->dwmadrFlashRomStart[dwMcuRomPtCnt] + (pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt]) -1;
			// MCU領域パターンブロック数取得
			dwAreaBlockNum = pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt];
			// MCU領域パターンブロックサイズ取得			
			dwAreaBlockSize = pMcuArea->dwFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt];
			// MCUブロックサイズ取得			
			dwBlockSize = pMcuArea->dwFlashRomBlockSize[dwMcuRomPtCnt];
			// 先頭アドレスの更新
			if(dwmadrStart > dwmadrAreaStart){	
				dwmadrStart = dwmadrAreaStart;	
			}
			// 終了アドレスの更新
			if(dwmadrEnd < dwmadrAreaEnd){
				dwmadrEnd = dwmadrAreaEnd;
			}
			// ブロック数更新
			dwBlockNum += dwAreaBlockNum;
			// メモリサイズ更新
			dwAreaSize += dwAreaBlockSize;
			// 最小ブロックサイズ更新
			if(dwBlockSizeMin > dwBlockSize){
				dwBlockSizeMin = dwBlockSize;
			}
		}

		s_madrUmCacheMemStartAddr = dwmadrStart;	// 先頭アドレス設定
		s_madrUmCacheMemEndAddr = dwmadrEnd;		// 終了アドレス設定
		s_dwUmCacheBlockNum = dwBlockNum;			// ブロック数設定
//		s_dwUmCacheMemSize = dwAreaSize;			// メモリサイズ設定
		//従来は、ROMの有効資源が1範囲であったが、ROMの有効資源が2範囲になった場合、キャッシュメモリを確保するための
		//サイズ換算が正しく行えない。キャッシュメモリの確保ができない箇所に対してアクセスを行った場合、 HEWクラッシュなどが発生する。
		s_dwUmCacheMemSize = dwmadrEnd - dwmadrStart + 1;

		
		// 書き込みサイズを設定
		s_dwUmProgSize = dwBlockSizeMin;
		if(s_dwUmProgSize > 256){
			s_dwUmProgSize = 256;
		}
		// ブロック領域書き換えフラグ数を算出
		dwBlockAreaWriteNum = s_dwUmCacheBlockNum/CACHE_BLOCK_AREA_NUM_RX;
		dwRestBlkNum = s_dwUmCacheBlockNum%CACHE_BLOCK_AREA_NUM_RX;
		if(dwRestBlkNum > 0){
			dwBlockAreaWriteNum++;
		}
		// ユーザーマット領域のキャッシュメモリ構造体領域確保
		if(s_bUmNewCacheMemFlag == FALSE){
			um_ptr->pbyCacheMem = (BYTE *)new(BYTE[s_dwUmCacheMemSize]);
			um_ptr->pbyCacheSetFlag = (BYTE *)new(BYTE[s_dwUmCacheBlockNum]);
			um_ptr->pbyBlockWriteFlag = (BYTE *)new(BYTE[s_dwUmCacheBlockNum]);
			um_ptr->pbyBlockAreaWriteFlag = (BYTE *)new(BYTE[dwBlockAreaWriteNum]);
			s_bUmNewCacheMemFlag = TRUE;	 // キャッシュメモリ確保フラグを確保済み
		}
	}

	// データマット領域キャッシュメモリ確保処理
	// データマット領域のMCU情報を取得
	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	if(pMcuArea->dwDataFlashRomPatternNum == 0){
		s_madrDmCacheMemStartAddr = 0;			// 先頭アドレスを初期化
		s_madrDmCacheMemEndAddr = 0;			// 終了アドレスを初期化
		s_dwDmCacheBlockNum = 0;				// ブロック数を初期化
		s_dwDmCacheMemSize = 0;					// メモリサイズを初期化
		s_dwDmProgSize = 0;						// 書き込みサイズを初期化
		s_bDmNewCacheMemFlag = FALSE;			// キャッシュメモリ確保フラグを初期化
	}
	else{
		dwmadrStart = 0xFFFFFFFF;		// 先頭アドレスを初期化
		dwmadrEnd = 0x00000000;			// 終了アドレスを初期化
		dwBlockNum = 0;					// ブロック数を初期化
		dwAreaSize = 0;					// サイズを初期化
		dwBlockSizeMin = 0xFFFFFFFF;	// 最小ブロックサイズを初期化

		for (dwMcuRomPtCnt = 0; dwMcuRomPtCnt < pMcuArea->dwDataFlashRomPatternNum; dwMcuRomPtCnt++) {
			// MCU領域パターン先頭アドレス取得
			dwmadrAreaStart = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt];
			// MCU領域パターン終了アドレス取得
			dwmadrAreaEnd = pMcuArea->dwmadrDataFlashRomStart[dwMcuRomPtCnt] + (pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt]) -1;
			// MCU領域パターンブロック数取得
			dwAreaBlockNum = pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt];
			// MCU領域パターンブロックサイズ取得			
			dwAreaBlockSize = pMcuArea->dwDataFlashRomBlockNum[dwMcuRomPtCnt]*pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt];
			// MCUブロックサイズ取得			
			dwBlockSize = pMcuArea->dwDataFlashRomBlockSize[dwMcuRomPtCnt];
			// 先頭アドレスの更新
			if(dwmadrStart > dwmadrAreaStart){	
				dwmadrStart = dwmadrAreaStart;	
			}
			// 終了アドレスの更新
			if(dwmadrEnd < dwmadrAreaEnd){
				dwmadrEnd = dwmadrAreaEnd;
			}
			// ブロック数更新
			dwBlockNum += dwAreaBlockNum;
			// メモリサイズ更新
			dwAreaSize += dwAreaBlockSize;
			// 最小ブロックサイズ更新
			if(dwBlockSizeMin > dwBlockSize){
				dwBlockSizeMin = dwBlockSize;
			}
		}
		s_madrDmCacheMemStartAddr = dwmadrStart;	// 先頭アドレス設定
		s_madrDmCacheMemEndAddr = dwmadrEnd;		// 終了アドレス設定
		s_dwDmCacheBlockNum = dwBlockNum;			// ブロック数設定
		s_dwDmCacheMemSize = dwAreaSize;			// メモリサイズ設定

		// 書き込みサイズを設定
		s_dwDmProgSize = dwBlockSizeMin;
		if(s_dwDmProgSize > 256){
			s_dwDmProgSize = 256;
		}
		// ブロック領域書き換えフラグ数を算出
		dwBlockAreaWriteNum = s_dwDmCacheBlockNum/CACHE_BLOCK_AREA_NUM_RX;
		dwRestBlkNum = s_dwDmCacheBlockNum%CACHE_BLOCK_AREA_NUM_RX;
		if(dwRestBlkNum > 0){
			dwBlockAreaWriteNum++;
		}

		// 書き込みサイズ単位書き換えフラグ数を算出
		dwProgSizeWriteNum = s_dwDmCacheMemSize/s_dwDmProgSize;
		if(dwProgSizeWriteNum < 1){
			dwProgSizeWriteNum = 1;
		}
		
		// データマット領域のキャッシュメモリ構造体領域確保
		if(s_bDmNewCacheMemFlag == FALSE){
			dm_ptr->pbyCacheMem = (BYTE *)new(BYTE[s_dwDmCacheMemSize]);
			dm_ptr->pbyCacheSetFlag = (BYTE *)new(BYTE[s_dwDmCacheBlockNum]);
			dm_ptr->pbyBlockWriteFlag = (BYTE *)new(BYTE[s_dwDmCacheBlockNum]);
			dm_ptr->pbyBlockAreaWriteFlag = (BYTE *)new(BYTE[dwBlockAreaWriteNum]);
			dm_ptr->pbyProgSizeWriteFlag = (BYTE *)new(BYTE[dwProgSizeWriteNum]);
			s_bDmNewCacheMemFlag = TRUE;	 // キャッシュメモリ確保フラグを確保済み
		}
	}

	// ユーザーブートマット領域キャッシュメモリ確保処理
	// ユーザーマットブート領域のMCU情報を取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーブートマットキャッシュメモリ構造体情報取得
	if(pMcuArea->dwmadrUserBootStart != 0 && pMcuArea->dwmadrUserBootEnd != 0){
		s_madrUbmCacheMemStartAddr = pMcuArea->dwmadrUserBootStart;		// 先頭アドレス設定
		s_madrUbmCacheMemEndAddr = pMcuArea->dwmadrUserBootEnd;			// 終了アドレス設定
		s_dwUbmCacheBlockNum = 1;										// ブロック数設定
		s_dwUbmCacheMemSize = pMcuArea->dwmadrUserBootEnd - pMcuArea->dwmadrUserBootStart + 1;	// メモリサイズ設定

		// 書き込み単位を算出
		s_dwUbmProgSize = s_dwUbmCacheMemSize;
		if(s_dwUbmProgSize > 256){
			s_dwUbmProgSize = 256;
		}
		
		// ユーザーブートマット領域のキャッシュメモリ構造体領域確保
		if(s_bUbmNewCacheMemFlag == FALSE){
			ubm_ptr->pbyCacheMem = (BYTE *)new(BYTE[s_dwUbmCacheMemSize]);
			s_bUbmNewCacheMemFlag = TRUE;	 // キャッシュメモリ確保フラグを確保済み
		}
	}
	else{
		s_madrUbmCacheMemStartAddr = 0;		// 先頭アドレスを初期化
		s_madrUbmCacheMemEndAddr = 0;		// 終了アドレスを初期化
		s_dwUbmCacheBlockNum = 0;			// ブロック数を初期化
		s_dwUbmCacheMemSize = 0;			// メモリサイズを初期化
		s_dwUbmProgSize = 0;				// 書き込み単位を初期化
		s_bUbmNewCacheMemFlag = FALSE;		// キャッシュメモリ確保フラグを初期化
	}
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End


// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * ユーザーマットキャッシュメモリ構造体情報取得
 * @param なし
 * @retval ユーザーマットキャッシュメモリ構造体へのポインタ
 */
//=============================================================================
USER_ROMCACHE_RX* GetUserMatCacheData(void)
{
	return &s_UserMatCacheData;
}
//=============================================================================
/**
 * データマットキャッシュメモリ構造体情報取得
 * @param なし
 * @retval データマットマットキャッシュメモリ構造体へのポインタ
 */
//=============================================================================
DATA_ROMCACHE_RX* GetDataMatCacheData(void)
{
	return &s_DataMatCacheData;
}
//=============================================================================
/**
 * ユーザーブートマットキャッシュメモリ構造体情報取得
 * @param なし
 * @retval ユーザーブートマットキャッシュメモリ構造体へのポインタ
 */
//=============================================================================
USERBOOT_ROMCACHE_RX* GetUserBootMatCacheData(void)
{
	return &s_UserBootMatCacheData;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
// RevRxNo140515-011 Append Start
//=============================================================================
/**
 * Extra領域キャッシュメモリアドレス取得
 * @param Extra領域キャッシュメモリのオフセットアドレス
 * @retval Extra領域キャッシュメモリへのポインタ
 */
//=============================================================================
BYTE* GetExtraCacheAddr(DWORD dwCacheCnt)
{
	return &s_byExtraCacheMem[dwCacheCnt];
}
// RevRxNo140515-011 Append End

//=============================================================================
/**
 * キャッシュメモリ管理フラグを初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFlashRomCacheMemFlg(void)
{

	DWORD	dwProgSizeWriteNum;			// 書き込みサイズ単位書き込みフラグ数
	DWORD	dwBlockAreaWriteNum;		// ブロック領域書き込みフラグ数
	DWORD	dwRestBlkNum;				// ブロック領域端数ブロック数

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ

	DWORD	dwCnt;

	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得

	// ユーザーマット領域キャッシュメモリ管理フラグを初期化
	// ブロック領域書き換えフラグ数を算出
	if(GetNewCacheMem(MAREA_USER) == TRUE){

		dwBlockAreaWriteNum = GetCacheBlockNum(MAREA_USER)/CACHE_BLOCK_AREA_NUM_RX;
		dwRestBlkNum = GetCacheBlockNum(MAREA_USER)%CACHE_BLOCK_AREA_NUM_RX;
		if(dwRestBlkNum > 0){
			dwBlockAreaWriteNum++;
		}
		for( dwCnt = 0; dwCnt < GetCacheBlockNum(MAREA_USER); dwCnt++ ){
			um_ptr->pbyCacheSetFlag[dwCnt] = FALSE; 
			um_ptr->pbyBlockWriteFlag[dwCnt] = FALSE;
		}
		for( dwCnt = 0; dwCnt < dwBlockAreaWriteNum; dwCnt++ ){
			um_ptr->pbyBlockAreaWriteFlag[dwCnt] = FALSE;
		}
	}

	// データマット領域キャッシュメモリ管理フラグを初期化
	if(GetNewCacheMem(MAREA_DATA) == TRUE){

		// ブロック領域書き換えフラグ数を算出
		dwBlockAreaWriteNum = GetCacheBlockNum(MAREA_DATA)/CACHE_BLOCK_AREA_NUM_RX;
		dwRestBlkNum = GetCacheBlockNum(MAREA_DATA)%CACHE_BLOCK_AREA_NUM_RX;
		if(dwRestBlkNum > 0){
			dwBlockAreaWriteNum++;
		}
		// 書き込みサイズ単位書き換えフラグ数を算出
		dwProgSizeWriteNum = s_dwDmCacheMemSize/s_dwDmProgSize;
		if(dwProgSizeWriteNum < 1){
			dwProgSizeWriteNum = 1;
		}
		for( dwCnt = 0; dwCnt < GetCacheBlockNum(MAREA_DATA); dwCnt++ ){
			dm_ptr->pbyCacheSetFlag[dwCnt] = FALSE; 
			dm_ptr->pbyBlockWriteFlag[dwCnt] = FALSE;
		}
		for( dwCnt = 0; dwCnt < dwBlockAreaWriteNum; dwCnt++ ){
			dm_ptr->pbyBlockAreaWriteFlag[dwCnt] = FALSE;
		}
		for( dwCnt = 0; dwCnt < dwProgSizeWriteNum; dwCnt++ ){
			dm_ptr->pbyProgSizeWriteFlag[dwCnt] = FALSE;
		}
	}
	
	// ユーザーブートマット領域キャッシュメモリ管理フラグを初期化
	if(GetNewCacheMem(MAREA_USERBOOT) == TRUE){
		s_bUbmCacheSetFlag = FALSE;			// キャッシュメモリデータ格納フラグを初期化
		s_bUbmBlockWriteFlag = FALSE;		// ブロック書き換えフラグを初期化
	}

	// RevRxNo140109-001 Append Start
	// Extra領域キャッシュメモリ管理フラグを初期化
	s_bExtraCacheSetFlag = FALSE;
	s_bExtraAreaWriteFlag = FALSE;
	for (dwCnt = 0; dwCnt < EXTRA_A_WRITE_FLG_NUM; dwCnt++) {
		s_bExtraWriteFlag[dwCnt] = FALSE;
	}
	// RevRxNo140109-001 Append End
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

//=============================================================================
/**
 * キャッシュメモリデータ設定フラグをクリア
 * @param eAreaType 領域指定
 * @retval なし
 */
//=============================================================================
void InitCacheDataSetFlg(enum FFWRXENM_MAREA_TYPE eAreaType)
{

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ

	DWORD dwBlockNum;

	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得

	// ユーザーマット領域キャッシュメモリデータ設定フラグをクリア
	if(eAreaType == MAREA_USER){
		// ブロック領域書き換えフラグ数を算出
		dwBlockNum = GetCacheBlockNum(eAreaType);
		// RevNo120120-001 ユーザマット領域がない場合は以下処理を実行しないようにする Append Line
		// (キャッシュを確保していないにも関わらずキャッシュをリード/ライトするとIDEがクラッシュする危険性があるため)
		if( dwBlockNum != 0 ){		// ユーザマットがある場合
			// キャッシュメモリデータ設定フラグクリア
			memset(&um_ptr->pbyCacheSetFlag[0], 0, dwBlockNum);
		// RevNo120120-001 ユーザマット領域がない場合は以下処理を実行しないようにする Append Line
		}
	}

	// データマット領域キャッシュメモリデータ設定フラグをクリア
	else if(eAreaType == MAREA_DATA){
		// ブロック領域書き換えフラグ数を算出
		dwBlockNum = GetCacheBlockNum(eAreaType);
		// RevNo120120-001 データマット領域がない場合は以下処理を実行しないようにする Append Line
		// (キャッシュを確保していないにも関わらずキャッシュをリード/ライトするとIDEがクラッシュする危険性があるため)
		if( dwBlockNum != 0 ){		// データマットがある場合
			// キャッシュメモリデータ設定フラグクリア
			memset(&dm_ptr->pbyCacheSetFlag[0], 0, dwBlockNum);
		// RevNo120120-001 データマット領域がない場合は以下処理を実行しないようにする Append Line
		}
	}

	// ユーザーブートマット領域キャッシュメモリデータ設定フラグをクリア
	else{
		SetUbmCacheSet(FALSE);
		
	}
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

//=============================================================================
/**
 * キャッシュメモリ確保フラグ参照
 * @param eAreaType 領域指定
 * @retval NewCacheMemFlag 指定領域のキャッシュメモリ確保状態
 */
//=============================================================================
BOOL GetNewCacheMem(enum FFWRXENM_MAREA_TYPE eAreaType)
{
	// RevNo121017-005	Append Line
	BOOL	bMemFlag;
	
	// RevNo121017-005	Modify Start
	if(eAreaType == MAREA_USER){
		bMemFlag = s_bUmNewCacheMemFlag;
	}
	else if(eAreaType == MAREA_DATA){
		bMemFlag =	s_bDmNewCacheMemFlag;
	}
	else{
		bMemFlag =	s_bUbmNewCacheMemFlag;
	}
	return bMemFlag;
	// RevNo121017-005	Modify End
}

//=============================================================================
/**
 * キャッシュメモリ確保フラグ設定
 * @param eAreaType 領域指定
 * @param bNewCacheMemFlag 指定領域のキャッシュメモリ確保状態設定
 * @retval なし
 */
//=============================================================================
void SetNewCacheMem(enum FFWRXENM_MAREA_TYPE eAreaType,BOOL bNewCacheMem)
{
	if(eAreaType == MAREA_USER){
		s_bUmNewCacheMemFlag = bNewCacheMem;	
	}
	else if(eAreaType == MAREA_DATA){
		s_bDmNewCacheMemFlag = bNewCacheMem;
	}
	else{
		s_bUbmNewCacheMemFlag = bNewCacheMem;
	}
}


//=============================================================================
/**
 * キャッシュメモリMCU先頭アドレス取得
 * @param eAreaType 領域指定
 * @retval madrStartAddr 先頭アドレス
 */
//=============================================================================
MADDR GetCacheStartAddr(enum FFWRXENM_MAREA_TYPE eAreaType)
{
	
	DWORD	madrStartAddr = 0;

	if(eAreaType == MAREA_USER){
		madrStartAddr = s_madrUmCacheMemStartAddr;
	}
	else if(eAreaType == MAREA_DATA){
		madrStartAddr = s_madrDmCacheMemStartAddr;
	}
	else{
		madrStartAddr = s_madrUbmCacheMemStartAddr;
	}
	return madrStartAddr;
}

//=============================================================================
/**
 * キャッシュメモリMCU終了アドレス取得
 * @param eAreaType 領域指定
 * @retval madrEndAddr 終了アドレス
 */
//=============================================================================
MADDR GetCacheEndAddr(enum FFWRXENM_MAREA_TYPE eAreaType)
{
	MADDR madrEndAddr = 0; 
	
	if(eAreaType == MAREA_USER){
		madrEndAddr = s_madrUmCacheMemEndAddr;
	}
	else if(eAreaType == MAREA_DATA){
		madrEndAddr = s_madrDmCacheMemEndAddr;
	}
	else{
		madrEndAddr = s_madrUbmCacheMemEndAddr;
	}
	return madrEndAddr;
}

//=============================================================================
/**
 * キャッシュメモリMCUブロック数取得
 * @param eAreaType 領域指定
 * @param eAreaType 領域指定
 * @retval dwBlockNum ブロック数
 */
//=============================================================================
DWORD GetCacheBlockNum(enum FFWRXENM_MAREA_TYPE eAreaType)
{

	DWORD dwBlockNum = 0;
	
	if(eAreaType == MAREA_USER){
		dwBlockNum = s_dwUmCacheBlockNum;
	}
	else if(eAreaType == MAREA_DATA){
		dwBlockNum = s_dwDmCacheBlockNum;
	}
	else{
		dwBlockNum = s_dwUbmCacheBlockNum;
	}
	return dwBlockNum;
}
//=============================================================================
/**
 * キャッシュメモリMCU書き込みサイズ取得
 * @param eAreaType 領域指定
 * @retval dwProgSize 書き込みサイズ
 */
//=============================================================================
DWORD GetCacheProgSize(enum FFWRXENM_MAREA_TYPE eAreaType)
{

	DWORD	dwProgSize = 0;

	if(eAreaType == MAREA_USER){
		dwProgSize = s_dwUmProgSize;
	}
	else if(eAreaType == MAREA_DATA){
		dwProgSize = s_dwDmProgSize;
	}
	else{
		dwProgSize = s_dwUbmProgSize;
	}
	return dwProgSize;
}

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * ブロック書き換えフラグの全クリア
 * @param  なし
 * @retval なし
 */
//=============================================================================
void ClearAllBlockWriteFlg(void)
{

	DWORD	dwBlockNum;
	DWORD	dwAreaBlockNum;
	DWORD	dwRestBlkNum;
	// RevRxNo121219-001 　Append Line
	DWORD	dwProgSizeWriteNum;			// 書き込みサイズ単位書き込みフラグ数
	DWORD	dwCnt;	// RevRxNo140109-001 Append Line

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得


	// ユーザマット領域のブロック書き換えフラグクリア
	// ブロック総数を取得
	dwBlockNum = GetCacheBlockNum(MAREA_USER);

	// RevRxNo121221-002 Modify Start
	if (dwBlockNum != 0) {	// ブロック数が0でない場合
		// ブロック書き換えフラグクリア
		memset(&um_ptr->pbyBlockWriteFlag[0], 0, dwBlockNum);

		// ブロック領域書き換えフラグ数を算出
		dwAreaBlockNum = dwBlockNum/CACHE_BLOCK_AREA_NUM_RX;
		dwRestBlkNum = dwBlockNum%CACHE_BLOCK_AREA_NUM_RX;
		if(dwRestBlkNum > 0){
			dwAreaBlockNum++;
		}
		// ブロック領域書き換えフラグクリア
		memset(&um_ptr->pbyBlockAreaWriteFlag[0], 0, dwAreaBlockNum);
	}
	// RevRxNo121221-002 Modify End


	// データマット領域のブロック書き換えフラグクリア
	// ブロック総数を取得
	dwBlockNum = GetCacheBlockNum(MAREA_DATA);

	// RevRxNo121221-002 Modify Start
	if (dwBlockNum != 0) {	// ブロック数が0でない場合
		// ブロック書き換えフラグクリア
		memset(&dm_ptr->pbyBlockWriteFlag[0], 0, dwBlockNum);

		// ブロック領域書き換えフラグ数を算出
		dwAreaBlockNum = dwBlockNum/CACHE_BLOCK_AREA_NUM_RX;
		dwRestBlkNum = dwBlockNum%CACHE_BLOCK_AREA_NUM_RX;
		if(dwRestBlkNum > 0){
			dwAreaBlockNum++;
		}
		// ブロック領域書き換えフラグクリア
		memset(&dm_ptr->pbyBlockAreaWriteFlag[0], 0, dwAreaBlockNum);

		// RevRxNo121219-001 　Append Start
		// 書き込みサイズ単位書き換えフラグ数を算出
		dwProgSizeWriteNum = s_dwDmCacheMemSize/s_dwDmProgSize;
		if(dwProgSizeWriteNum < 1){
			dwProgSizeWriteNum = 1;
		}
		// 書き込みサイズ単位書き換えフラグクリア
		memset(&dm_ptr->pbyProgSizeWriteFlag[0], 0, dwProgSizeWriteNum);
		// RevRxNo121219-001 　Append End
	}
	// RevRxNo121221-002 Modify End

	// ユーザブートマット領域のブロック書き換えフラグクリア
	SetUbmBlockWrite(FALSE);


	// RevRxNo140109-001 Append Start
	// Extra領域のブロック書き換えフラグクリア
	for (dwCnt = 0; dwCnt < EXTRA_A_WRITE_FLG_NUM; dwCnt++) {
		s_bExtraWriteFlag[dwCnt] = FALSE;
	}
	s_bExtraAreaWriteFlag = FALSE;
	// RevRxNo140109-001 Append End


}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
//=============================================================================
/**
 * ユーザーブートマットキャッシュメモリデータ格納済みフラグ取得
 * @param なし
 * @retval s_bUbmCacheSetFlag ユーザーブートマットキャッシュメモリデータ格納済みフラグ
 */
//=============================================================================
BOOL GetUbmCacheSet(void)
{
	return s_bUbmCacheSetFlag;	// ユーザーブートマットキャッシュメモリデータ格納済みフラグ取得
}
//=============================================================================
/**
 * ユーザーブートマットキャッシュメモリデータ格納済みフラグ設定
 * @param bUbmCacheSet ユーザーブートマットキャッシュメモリデータ格納済みフラグ設定値
 * @retval なし
 */
//=============================================================================
void SetUbmCacheSet(BOOL bUbmCacheSet)
{
	s_bUbmCacheSetFlag = bUbmCacheSet ;	// ユーザーブートマットキャッシュメモリデータ格納済みフラグ設定
}
//=============================================================================
/**
 * ユーザーブートマットブロック書き換えフラグ取得
 * @param なし
 * @retval s_bUbmBlockWriteFlag ユーザーブートマットブロック書き換えフラグ
 */
//=============================================================================
BOOL GetUbmBlockWrite(void)
{
	return s_bUbmBlockWriteFlag;	//  ユーザーブートマットブロック書き換えフラグ取得
}
//=============================================================================
/**
 * ユーザーブートマットブロック書き換えフラグ設定
 * @param bUbmBlockWriteFlag ユーザーブートマットブロック書き換えフラグ設定値
 * @retval なし
 */
//=============================================================================
void SetUbmBlockWrite(BOOL bUbmBlockWriteFlag)
{
	s_bUbmBlockWriteFlag = bUbmBlockWriteFlag;	//  ユーザーブートマットブロック書き換えフラグ設定
}


// RevRxNo140109-001 Delete: InitExtraCacheMemFlg()

//=============================================================================
/**
 * 内蔵ROMに書かれているUBコードが指定されたUBコードと同じかどうかを判定する
 * @param pUbcode:比較するUBコード
 * @param byUbcodeChange:比較するUBコード
 * @param pUbAreaChange	:フラッシュROM上ユーザブート領域の書き換え情報へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo120910-005	Modify Line
FFWERR CheckMemUbcode(const FFWRX_UBCODE_DATA *pUbcode, BOOL* pbUbcodeChange, const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange)
{

	FFWERR						ferr = FFWERR_OK;
	DWORD						i,j,n,m;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrReadAddr;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域
	DWORD						dwLength;
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameLength;
	BYTE						byTmpBuff[4];		// UBコード比較用テンポラリバッファ
	// RevRxNo120910-005	Delete Line
	// RevRxNo120910-005	Append Line
	BYTE						byArgBoot = 0;		// UBコード比較用フラグ
	BYTE						byTmpUbCodeA[8];	// UBコードA格納テンポラリバッファ // RevRxNo120606-008 Append Line

	*pbUbcodeChange = FALSE;			// UBコード書き換えあり

	for( i = 0; i < 4; i++ ){
		if( i == 0 ){
			madrReadAddr = MCU_OSM_UBCODE_A_START;		// UBコードAアドレス(上位)
		}else if( i == 1 ){
			madrReadAddr = MCU_OSM_UBCODE_A_START+4;	// UBコードAアドレス(下位)
		}else if( i == 2 ){
			madrReadAddr = MCU_OSM_UBCODE_B_START;		// UBコードBアドレス(上位)
		}else{
			madrReadAddr = MCU_OSM_UBCODE_B_START+4;	// UBコードBアドレス(下位)
		}
				
		//UBコードを取得
		eAccessSize = MLWORD_ACCESS;
		pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
		dwLength = 1;
		dwAreaNum = 1;
		bSameAccessSize = FALSE;
		bSameLength = FALSE;
		memset(pbyReadData, 0, sizeof(WORD));
		ferr = PROT_MCU_DDUMP(dwAreaNum, &madrReadAddr, bSameAccessSize, &eAccessSize, bSameLength, &dwLength, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		for( j = 0; j < 4; j++ ){
			if( i == 0 ){		// UBコードA前半
				for( n = 0, m = 3; n < 4; n++, m-- ){
					byTmpBuff[m] = pUbcode->byUBCodeA[n];
				}
				byTmpUbCodeA[j] = pbyReadData[j];			// RevRxNo120606-008 Append Line
			}else if( i == 1 ){
				for( n = 0, m = 3; n < 4; n++, m-- ){
					byTmpBuff[m] = pUbcode->byUBCodeA[n+4];
				}
				byTmpUbCodeA[j+4] = pbyReadData[j];			// RevRxNo120606-008 Append Line
			}else if( i == 2 ){
				for( n = 0, m = 3; n < 4; n++, m-- ){
					byTmpBuff[m] = pUbcode->byUBCodeB[n];
				}
			}else{
				for( n = 0, m = 3; n < 4; n++, m-- ){
					byTmpBuff[m] = pUbcode->byUBCodeB[n+4];
				}
			}
			if( byTmpBuff[j] != pbyReadData[j] ){
				*pbUbcodeChange = TRUE;			// UBコード書き換えあり
			}
		}
	}
	// RevRxNo120606-008 Append Start
	// UBブート書き換え発生だった場合
	if(*pbUbcodeChange == TRUE){
		// RevRxNo120910-005	Append Start
		//	(A)	取得UBコードAが指定されたUBコードA<指定コード>(pUbAreaChange->byChangeInfoUBCodeA[i])と一致するか4バイト
		//	（LWORD)づつ8バイトチェックする
		for(i=0;i<4;i++) {
			if(pUbAreaChange->byChangeInfoUBCodeA[i] == byTmpUbCodeA[3-i]) {
				// <a>	取得UBコードが指定コードだった場合
				//	<A>	一致していたビット位置のbyArgBootを1立てる
				byArgBoot |= 0x01 << i;	
			}
		}
		for(i=4;i<8;i++) {					
			if(pUbAreaChange->byChangeInfoUBCodeA[i] == byTmpUbCodeA[7+4-i]) {
				//	<a>	取得UBコードが指定コードだった場合
				//	<A>	一致していたビット位置のbyArgBootを1立てる
				byArgBoot |= 0x01 << i;
			}
		}
		//	(B)	取得UBコードAが指定コードと全バイト一致するか確認
		if(byArgBoot == MATCH_ARGBOOT) {
			//	<1>	一致する場合
			//・	ユーザブート領域書き換え許可フラグ(pUbAreaChange->eChangeUbAreaEna)を確認する。
			if(pUbAreaChange->eChangeUbAreaEna == RX_UBAREA_CHANGE_DIS) {
				// (i) RX_UBAREA_CHANGE_DISである場合
				*pbUbcodeChange = FALSE;
				//	書き換え禁止を返して終了
				return FFWERR_ROM_UBCODE_USBBOOT;
			}
		}
		// RevRxNo120910-005	Append End
		// RevRxNo120910-005	Delete Line
	}
	// RevRxNo120606-008 Append End

	return ferr;

}


// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * OFS1レジスタの電圧監視リセット値を確認し、無効に設定する必要があるかを判定する
 * @param pbLvdChange:電圧監視リセットの無効設定結果格納ポインタ
 * @param byOfs1LvdData:電圧監視リセット判定用データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR CheckOfs1LvdDis(BOOL* pbLvdChange, BYTE byOfs1LvdData)
{
	FFWERR					ferr = FFWERR_OK;
	BOOL					bOverVdsel = TRUE;
	BYTE					byOfs1LvdDis;
	BYTE					byOfs1LvdMask;
	FFWMCU_FWCTRL_DATA*		pFwCtrl;
	FFWMCU_MCUDEF_DATA* 	pMcuDef;

	pFwCtrl = GetFwCtrlData();
	pMcuDef = GetMcuDefData();

	*pbLvdChange = FALSE;	// 電圧監視リセットの無効設定は不要で初期化

	// LVD無効の設定値およびマスク値を生成
	byOfs1LvdDis = static_cast<BYTE>(pMcuDef->dwOfs1LvdDis);
	byOfs1LvdMask = static_cast<BYTE>(pMcuDef->dwOfs1LvdMask);

	if ((byOfs1LvdData & byOfs1LvdMask) != byOfs1LvdDis) {
		// 電圧監視リセットが有効に設定されている場合

		if (pFwCtrl->eLvdErrType == RX_LVD_LEVEL_CHK) {	// LVDx有効時の電圧レベルチェックを実施するMCUの場合
			ferr = CmpOfs1Vdsel2TargetVcc(byOfs1LvdData, &bOverVdsel);
			if(ferr != FFWERR_OK){
				return ferr;
			}
			if (bOverVdsel == FALSE) {	// ターゲット電源がVDSEL設定以下である場合
				*pbLvdChange = TRUE;	// 電圧監視リセットの無効設定が必要
			}
		} else if (pFwCtrl->eLvdErrType == RX_LVD_ENA_CHK) {	// LVDx有効設定チェックを実施するMCUの場合
			*pbLvdChange = TRUE;	// 電圧監視リセットの無効設定が必要
		}
	}

	return ferr;
}
// RevRxNo130730-001 Append End


//=============================================================================
/**
 * Flashダウンロード時のWorkRam退避用キャッシュ領域確保
 * @param dwSize :メモリ確保サイズ
 * @retval なし
 */
//=============================================================================
void NewEscWorkRamCacheMem(DWORD dwSize)
{
	if(s_bEscWorkRam == FALSE){
		//WORKRAM退避メモリ確保されていない場合は、メモリを確保
		s_bEscWorkRam = TRUE;
	// RevRxNo121029-001	Modify Start
		s_pbyReadData = new BYTE [dwSize+WTR_HEADER_SIZE];
	}
	// RevRxNo121029-001	Modify End
	return;
}
//=============================================================================
/**
 * Flashダウンロード時のWorkRam退避用キャッシュ領域開放
 * @param dwSize :メモリ確保サイズ
 * @retval なし
 */
//=============================================================================
void DeleteEscWorkRamCacheMem(void)
{
	if(s_bEscWorkRam == TRUE){
		delete [] s_pbyReadData;
		s_bEscWorkRam = FALSE;
	}

	return;
}

//=============================================================================
/**
 * Flash書き換え時のクロック切り替え有効/無効設定
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetDbgClkChg(enum FFWRXENM_CLKCHANGE_ENA	eClkChangeEna){

	FFWERR						ferr;
	FFWMCU_DBG_DATA_RX			*pDbgData;
	FFWMCU_DBG_DATA_RX			DbgDataTmp;

	pDbgData = GetDbgDataRX();		//FFW設定値取得
	memcpy(&DbgDataTmp,pDbgData,sizeof(FFWMCU_DBG_DATA_RX));
	DbgDataTmp.eClkChangeEna = eClkChangeEna;	//クロック切り替え設定は、eClkChangeEnaに差し替え。

	ferr = PROT_MCU_SetRXDBG(&DbgDataTmp);

	return ferr;
}
//=============================================================================
/**
 * Flashアクセス時に書き換わるSFR領域の退避/復帰(バイトアクセス用)
 * @param  bEscape   TRUE  : 退避
 *                   FALSE : 復帰
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR escFlashAccessSfrAreaByte(BOOL bEscape)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRwAddr;			// 設定/参照するレジスタのアドレス
	BYTE						byReadData[1];		// 参照データ格納領域
	BYTE						byWriteData[1];		// 設定データ格納領域
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	DWORD						dwRegAddr[] = {MCU_REG_FLASH_FMODR, MCU_REG_FLASH_FRDYIE, MCU_REG_FLASH_FWEPROR};
	DWORD						i;

	// レジスタ設定
	eAccessSize = MBYTE_ACCESS;
	pbyReadData = byReadData;
	pbyWriteData = byWriteData;

	if (bEscape == TRUE) {		// 退避時
		for (i = 0; i < (sizeof(dwRegAddr) / sizeof(DWORD)); i++) {
			madrRwAddr = dwRegAddr[i];
			// レジスタの読み出し
			ferr = GetMcuSfrReg(madrRwAddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			memcpy(&s_byFlashAccessSfr[i], pbyReadData, sizeof(BYTE));
		}
	} else {					// 復帰時
		for (i = 0; i < (sizeof(dwRegAddr) / sizeof(DWORD)); i++) {
			madrRwAddr = dwRegAddr[i];
			memcpy(pbyWriteData, &s_byFlashAccessSfr[i], sizeof(BYTE));
			// レジスタへ書き込み
			ferr = SetMcuSfrReg(madrRwAddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}

	return FFWERR_OK;
}

//=============================================================================
/**
 * Flashアクセス時に書き換わるSFR領域の退避/復帰(ワードアクセス用)
 * @param  bEscape   TRUE  : 退避
 *                   FALSE : 復帰
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR escFlashAccessSfrAreaWord(BOOL bEscape)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRWaddr;			// 設定/参照するレジスタのアドレス
	WORD						wReadData[1];		// 参照データ格納領域
	WORD						wWriteData[1];		// 設定データ格納領域
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	WORD						wData;
	// V.1.02 新デバイス対応 Modify Start
	// (DFLREとDFLWEレジスタはチップによって個数もアドレスも違うので、MCU情報から取得するようにする) 
	DWORD						dwRegAddr[] = {MCU_REG_SYSTEM_SYSCR1, MCU_REG_FLASH_FCURAME, MCU_REG_FLASH_FENTRYR, 
													MCU_REG_FLASH_FPROTR, MCU_REG_FLASH_DFLBCCNT, MCU_REG_FLASH_PCKAR, MCU_REG_FLASH_FRESETR};
	// V.1.02 新デバイス対応 Modify End
	BOOL						bFillFlg = FALSE;
	DWORD						i, j, k;
	// RevNo010804-001 Append Start
	DWORD						dwDataFlashCnt;
	dwDataFlashCnt = GetDataFlashBlockNum();
	// RevNo010804-001 Append End

	// RevRxNo130301-001 Append Start
	WORD						wSyscr1Data;		// SYSCR1設定値
	WORD						wSyscr1Mask;		// SYSCR1マスク値
	// RevRxNo130301-001 Append End

	// V.1.02 新デバイス対応 Append Start 
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		// MCU情報構造体のポインタ
	pMcuInfoData = GetMcuInfoDataRX();				// MCU情報を取得
	// V.1.02 新デバイス対応 Append End 

	// レジスタ設定
	eAccessSize = MWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);

	// RevNo010804-001でデータフラッシュ読み出し許可レジスタ、書き込み・消去許可レジスタが増えたMCUに対応したが、
	// 今後どのようなチップが出てくるか分からないので、V.1.02で数が違ったり、アドレスが違ったりするレジスタを
	// MCUコマンドで送るようにしたので、以下の処理がごっそり変わった。

	// RevRxNo130301-001 Append Start
	// SYSCR1設定で使用する内部変数を初期化
	// 二次元配列に格納されているアドレスとサイズ情報は、どの配列でも同じ値である。
	// そのため、[3][0]の値を変数に設定する。
	wSyscr1Data = 0x0000;
	wSyscr1Mask = 0x0000;

	for (i = 0; i < pMcuInfoData->dwMcuRegInfoNum[3]; i++) {
		// i番目のRAM領域に対応したSYSCR1/MSTPCRC値の設定値を格納
		// SYSCR1関連
		wSyscr1Data |= static_cast<WORD>(pMcuInfoData->dwMcuRegInfoData[3][i]);	// SYSCR1設定値(RAM領域毎に値が異なるため、論理和)
		wSyscr1Mask |= static_cast<WORD>(pMcuInfoData->dwMcuRegInfoMask[3][i]);	// SYSCR1マスク値(RAM領域毎に値が異なるため、論理和)
	}
	// RevRxNo130301-001 Append End

// V.1.02 新デバイス対応 Modify Start
	// RevNo010804-001 Modify Start
	if (bEscape == TRUE) {		// 退避時
		k = 0;
		for (i = 0; i < (sizeof(dwRegAddr) / sizeof(DWORD)); i++) {
			madrRWaddr = dwRegAddr[i];
			// レジスタの読み出し
			ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_wFlashAccessSfr[k++] = wReadData[0];

			// 退避後の設定変更
			bFillFlg = TRUE;
			switch (madrRWaddr) {
			case MCU_REG_SYSTEM_SYSCR1:		// システムコントロールレジスタ1	// RevRxNo130301-001 Modify Line
				// 内蔵RAM有効に設定
				wData = (wReadData[0] & ~(wSyscr1Mask)) | wSyscr1Data;			// RevRxNo130301-001 Modify Line
				break;
			case MCU_REG_FLASH_FCURAME:		// FCU RAMイネーブルレジスタ
				// FCU RAMへのアクセス許可に設定
				wData = MCU_REG_FLASH_FCURAME_DATA;
				break;
			default:
				bFillFlg = FALSE;
				wData = 0;
				break;
			}
			// 設定変更ありのレジスタに書込み
			if (bFillFlg == TRUE) {
				memcpy(pbyWriteData, &wData, sizeof(WORD));
				ferr = SetMcuSfrReg(madrRWaddr,eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}
		// データフラッシュ読み出し許可、書き込み・消去許可レジスタのユーザ値退避とデータフラッシュ読み出し許可設定
		// RevNo121105-001 Modify Line
		for( i = 0; i <= MCU_REGINFO_DFLWE_NO; i++ ){		// MCU登録レジスタ情報参照( ここでは2個だけ使用 )
			for( j = 0; j < pMcuInfoData->dwMcuRegInfoNum[i]; j++ ){	// データフラッシュ読み出し許可レジスタ数分繰り返す
				madrRWaddr = pMcuInfoData->dwmadrMcuRegInfoAddr[i][j];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[i][j];
				ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_wFlashAccessSfr[k++] = wReadData[0];
				if( i == 0 ){		// データフラッシュ読み出し許可レジスタの場合読み出し許可にする
					// データフラッシュ読み出し許可レジスタはビッグエンディアン領域のため、以下のようにデータ格納。
					// RevRxNo120910-013 Modify Line
					wWriteData[0] = (WORD)((pMcuInfoData->dwMcuRegInfoData[i][j]) | s_wDflreKeycode[j]);
					ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}
		}
	} else {				// 復帰時
		k = 0;
		for (i = 0; i < (sizeof(dwRegAddr) / sizeof(DWORD)); i++) {
			madrRWaddr = dwRegAddr[i];
			// 書き戻し時にキーコードが必要なレジスタのデータ加工
			switch (madrRWaddr) {
				// すでにBFW or 書き込みプログラムでプロテクトがかけられている可能性あり。2011.2.13
				case MCU_REG_SYSTEM_SYSCR1:		// システムコントロールレジスタ1
					wData = s_wFlashAccessSfr[k++];
					break;
				case MCU_REG_FLASH_FCURAME:		// FCU RAMイネーブルレジスタ
					wData = (WORD)(s_wFlashAccessSfr[k++] | MCU_REG_FLASH_FCURAME_DMASK);
					break;
				case MCU_REG_FLASH_FENTRYR:		// フラッシュP/Eモードエントリレジスタ
					wData = (WORD)(s_wFlashAccessSfr[k++] | MCU_REG_FLASH_FENTRYR_DMASK);
					break;
				case MCU_REG_FLASH_FRESETR:		// フラッシュリセットレジスタ
					wData = (WORD)(s_wFlashAccessSfr[k++] | MCU_REG_FLASH_FRESETR_DMASK);
					break;
				// 2009.9.17 下記レジスタの設定は書込みプログラムで実施しているので不要 → 書き込みプログラムが変わったので復帰
				case MCU_REG_FLASH_FPROTR:		// フラッシュプロテクトレジスタ
					wData = (WORD)(s_wFlashAccessSfr[k++] | MCU_REG_FLASH_FPROTR_DMASK);
					break;
				default:
					wData = s_wFlashAccessSfr[k++];
					break;
			}
			wWriteData[0] =  wData;
			// レジスタへ書き込み
			ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// データフラッシュ読み出し許可、書き込み・消去許可レジスタのユーザ値復帰
		// RevNo121105-001 Modify Line
		for( i = 0; i <= MCU_REGINFO_DFLWE_NO; i++ ){		// MCU登録レジスタ情報参照( ここでは2個だけ使用 )
			for( j = 0; j < pMcuInfoData->dwMcuRegInfoNum[i]; j++ ){	// データフラッシュ読み出し許可レジスタ数分繰り返す
				madrRWaddr = pMcuInfoData->dwmadrMcuRegInfoAddr[i][j];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[i][j];
				ferr = GetMcuSfrReg(madrRWaddr,  eAccessSize, SFR_ACCESS_COUNT,pbyReadData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				if( i == 0 ){		// データフラッシュ読み出し許可レジスタの場合
					wWriteData[0] = (WORD)(s_wFlashAccessSfr[k++] | s_wDflreKeycode[j]);
				}else{				// データフラッシュ書き込み・消去許可レジスタの場合
					wWriteData[0] = (WORD)(s_wFlashAccessSfr[k++] | s_wDflweKeycode[j]);
				}
				ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}
	}
	// RevNo010804-001 Modify End
// V.1.02 新デバイス対応 Modify End

	return FFWERR_OK;
}

//=============================================================================
/**
 * Flashアクセス時に書き換わるSFR領域の退避/復帰(ロングワードアクセス用)
 * @param  bEscape   TRUE  : 退避
 *                   FALSE : 復帰
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR escFlashAccessSfrAreaLWord(BOOL bEscape)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRwAddr;			// 設定/参照するレジスタのアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域
	DWORD						dwWriteData[1];		// 設定データ格納領域
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	DWORD						dwRegAddr[] = {MCU_REG_SYSTEM_MSTPCRC};
	DWORD						dwData;
	DWORD						i;

	// RevRxNo130301-001 Append Start
	BOOL						bFillFlg = FALSE;
	DWORD						dwMstpcrcData;		// MSTPCRC設定値
	DWORD						dwMstpcrcMask;		// MSTPCRCマスク値
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		// MCU情報構造体のポインタ
	pMcuInfoData = GetMcuInfoDataRX();				// MCU情報を取得
	// RevRxNo130301-001 Append End

	// レジスタ設定
	eAccessSize = MLWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
	pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);

	// RevRxNo130301-001 Append Start
	// MSTPCRC設定で使用する内部変数を初期化
	// 二次元配列に格納されているアドレスとサイズ情報は、どの配列でも同じ値である。
	// そのため、[4][0]の値を変数に設定する。
	dwMstpcrcData = 0x00000000;
	dwMstpcrcMask = 0x00000000;

	for (i = 0; i < pMcuInfoData->dwMcuRegInfoNum[4]; i++) {
		// i番目のRAM領域に対応したSYSCR1/MSTPCRC値の設定値を格納
		// MSTPCRC関連
		dwMstpcrcData |= pMcuInfoData->dwMcuRegInfoData[4][i];		// MSTPCRC設定値(RAM領域毎に値が異なるため、論理和)
		dwMstpcrcMask |= pMcuInfoData->dwMcuRegInfoMask[4][i];		// MSTPCRCマスク値(RAM領域毎に値が異なるため、論理和)
	}
	// RevRxNo130301-001 Append End

	if (bEscape == TRUE) {		// 退避時
		for (i = 0; i < (sizeof(dwRegAddr) / sizeof(DWORD)); i++) {
			madrRwAddr = dwRegAddr[i];
			// レジスタの読み出し
			ferr = GetMcuSfrReg(madrRwAddr, eAccessSize, SFR_ACCESS_COUNT,pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			memcpy(&s_dwFlashAccessSfr[i], pbyReadData, sizeof(DWORD));

			// RevRxNo130301-001 Modify Start
			// 退避後の設定変更
			bFillFlg = TRUE;
			switch (madrRwAddr) {
			case MCU_REG_SYSTEM_MSTPCRC:		// モジュールストップコントロールレジスタC
				// 内蔵RAM動作に設定
				dwData = (s_dwFlashAccessSfr[i] & ~(dwMstpcrcMask)) | dwMstpcrcData;	// RevRxNo130301-001 Modify Line
				break;
			default:
				bFillFlg = FALSE;
				dwData = 0;
				break;
			}
			// 設定変更ありのレジスタに書込み
			if (bFillFlg == TRUE) {
				// 設定変更ありのレジスタに書込み
				memcpy(pbyWriteData, &dwData, sizeof(DWORD));
				ferr = SetMcuSfrReg(madrRwAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
			// RevRxNo130301-001 Modify End
		}
	} else {					// 復帰時
		for (i = 0; i < (sizeof(dwRegAddr) / sizeof(DWORD)); i++) {
			madrRwAddr = dwRegAddr[i];
			memcpy(pbyWriteData, &s_dwFlashAccessSfr[i], sizeof(DWORD));
			// レジスタへ書き込み
			ferr = SetMcuSfrReg(madrRwAddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}

	return FFWERR_OK;
}

// RevRxNo121026-001 Append Start
//=============================================================================
/**
 * RX100 Flashアクセス時に書き換わるSFR領域の退避/復帰
 * @param  bEscape   TRUE  : 退避
 *                   FALSE : 復帰
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR escFlashAccessSfrArea_RX100(BOOL bEscape)
{
	FFWERR 						ferr = FFWERR_OK;

	if (bEscape == TRUE) {		// 退避時
		ferr = escFlashReg_RX100();			// RevRxNo130301-001 Modify Line
	} else {					// 復帰時
		ferr = retFlashReg_RX100();			// RevRxNo130301-001 Modify Line
	}

	return ferr;
}
// RevRxNo121026-001 Append End

// RevRxNo121026-001 Append Start
//=============================================================================
/**
 * RX100　Flashアクセス時に書き換わるSFR領域の退避
 * @param  
 *         
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR escFlashReg_RX100(void)		// RevRxNo130301-001 Modify Line
{
	FFWERR 						ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRWaddr;			// 設定/参照するレジスタのアドレス
	BYTE						byReadData;			// バイト参照データ格納領域
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// ワード参照データ格納領域(バイトポインタ指定のため、配列定義)

	// バイト単位レジスタ設定
	eAccessSize = MBYTE_ACCESS;		// バイトアクセス

	// DFLCTLレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_DFLCTL_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byDflctl_RX100 = byReadData;

	// FPMCRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FPMCR_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFpmcr_RX100 = byReadData;

	// FASRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FASR_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFasr_RX100 = byReadData;

	// FSARHレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FSARH_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFsarh_RX100 = byReadData;

	// FCRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FCR_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFcr_RX100 = byReadData;

	// FEARHレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FEARH_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFearh_RX100 = byReadData;

	// FISRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FISR_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFisr_RX100 = byReadData;

	// FEXCRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FEXCR_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFexcr_RX100 = byReadData;

	// FRESETRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FRESETR_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFresetr_RX100 = byReadData;


	// ワード単位レジスタ設定
	eAccessSize = MWORD_ACCESS;		// ワードアクセス

	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	memset(pbyReadData, 0, sizeof(WORD));

	// FENTRYRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FENTRYR;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFentryr_RX100 = wReadData[0];

	// FSARLレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FSARL_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFsarl_RX100 = wReadData[0];

	// FEARLレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FEARL_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFearl_RX100 = wReadData[0];

	// FWBHレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FWBH_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wfwbh_RX100 = wReadData[0];

	// FWBLレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FWBL_RX100;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wfwbl_RX100 = wReadData[0];

	return FFWERR_OK;
}
// RevRxNo121026-001 Append End

// RevRxNo121026-001 Append Start
//=============================================================================
/**
 * RX100　Flashアクセス時に書き換わるSFR領域の復帰
 * @param  
 *         
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR retFlashReg_RX100(void)			// RevRxNo130301-001 Modify Line
{
	FFWERR						ferr = FFWERR_OK;
	DWORD						dwAddrCnt,dwDataCnt,dwAcsCnt,dwRegCnt;
	DWORD						madrStartAddr[MCU_REG_FLASH_SET_REGNUM_RX100];
	enum FFWENM_MACCESS_SIZE	eAccessSize[MCU_REG_FLASH_SET_REGNUM_RX100];
	BYTE						byWriteData[MCU_REG_FLASH_SET_REGNUM_RX100 *2];
	enum FFWENM_VERIFY_SET 		eVerify = VERIFY_OFF;
	WORD						wData;
	WORD						wDataTmp;
	BYTE						byData;
	DWORD						dwAreaNum,dwLength;
	BOOL						bSameAccessSize,bSameLength,bSameWriteData;
	FFW_VERIFYERR_DATA 			VerifyErr;
	BYTE						byEndian = ENDIAN_BIG;

	// 復帰データ格納用配列を初期化
	memset(&byWriteData, 0, MCU_REG_FLASH_SET_REGNUM_RX100 *2);

	// カウント値初期化
	dwAddrCnt = 0;
	dwDataCnt = 0;
	dwAcsCnt = 0;
	dwRegCnt = 0;

	// FENTRYRでP/Eモード遷移
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FENTRYR;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FENTRYR_KEY_RX100 | MCU_REG_FLASH_FENTRYR_FENTRYI_DATA_RX100;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// ***特定シーケンス実行***
	// FPRレジスタ設定
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPR_DATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ設定でプログラミングモード遷移(現時点で無効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPMCR_PESET_DATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ反転値設定でプログラミングモード遷移(現時点で無効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPMCR_PESET_NOTDATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ設定でプログラミングモード遷移(現時点で有効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPMCR_PESET_DATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;


	// ***ユーザ値復帰***
	// ワード単位レジスタ
	// escFlashReg()でGetMcuSfrReg()を使用してDUMPしているため、ワード単位
	// レジスタ値はMCU仕様と同じ並びになっている。
	// 本関数はPROT_MCU_WRITE()で一度にWRITEするためワード値は上下8bitを
	// 反転する必要がある。
	// FWBL値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FWBL_RX100;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wfwbl_RX100;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FWBH値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FWBH_RX100;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wfwbh_RX100;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FSARL値復帰
 	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FSARL_RX100;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wFsarl_RX100;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FEARL値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FEARL_RX100;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wFearl_RX100;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// バイト単位レジスタ
	// FSARH値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FSARH_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFsarh_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FEARH値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FEARH_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFearh_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FISR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FISR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFisr_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FASR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FASR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFasr_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FCR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FCR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFcr_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FEXCR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FEXCR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFexcr_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FRESETR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FRESETR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFresetr_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;


	// ***特定シーケンス実行***
	// FPRレジスタ設定
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPR_DATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ設定でユーザ値復帰(現時点で無効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFpmcr_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ反転値設定でユーザ値復帰(現時点で無効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = ~(s_byFpmcr_RX100);
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ設定でユーザ値復帰(現時点で有効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFpmcr_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// DFLCTL値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_DFLCTL_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byDflctl_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FENTRYR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FENTRYR;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FENTRYR_KEY_RX100 | s_wFentryr_RX100;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;


	dwAreaNum = dwRegCnt;			//設定イベント個数
	bSameAccessSize = FALSE;		//アクセスサイズ混在
	bSameLength = TRUE;
	dwLength = 1;
	bSameWriteData = FALSE;

// RevRxNo121022-001 Append Start
	// ヘッダを含めて0x800hバイトを超える、PROT_MCU_WRITE()関数が呼ばれることはないので、BFWMCUCmd_WRITEコマンドの
	// ライトバイト数最大値でアクセスバイト数を分割してPROT_MCU_WRITE()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
	ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, madrStartAddr, bSameAccessSize, eAccessSize,
					bSameLength, &dwLength, bSameWriteData, &byWriteData[0], &VerifyErr, byEndian);

	return ferr;

}
// RevRxNo121026-001 Append End

// RevRxNo140515-006 Append Start
//=============================================================================
/**
 * RX230 Flashアクセス時に書き換わるSFR領域の退避/復帰
 * @param  bEscape   TRUE  : 退避
 *                   FALSE : 復帰
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR escFlashAccessSfrArea_RX230(BOOL bEscape)
{
	FFWERR 						ferr = FFWERR_OK;

	if (bEscape == TRUE) {		// 退避時
		ferr = escFlashReg_RX230();	
	} else {					// 復帰時
		ferr = retFlashReg_RX230();	
	}

	return ferr;
}
// RevRxNo140515-006 Append End

// RevRxNo140515-006 Append Start
//=============================================================================
/**
 * RX230　Flashアクセス時に書き換わるSFR領域の退避
 * @param  
 *         
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR escFlashReg_RX230(void)
{
	FFWERR 						ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRWaddr;			// 設定/参照するレジスタのアドレス
	BYTE						byReadData;			// バイト参照データ格納領域
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// ワード参照データ格納領域(バイトポインタ指定のため、配列定義)

	// バイト単位レジスタ設定
	eAccessSize = MBYTE_ACCESS;		// バイトアクセス

	// DFLCTLレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_DFLCTL_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byDflctl_RX230 = byReadData;

	// FPMCRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FPMCR_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFpmcr_RX230 = byReadData;

	// FASRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FASR_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFasr_RX230 = byReadData;

	// FCRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FCR_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFcr_RX230 = byReadData;

	// FISRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FISR_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFisr_RX230 = byReadData;

	// FEXCRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FEXCR_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFexcr_RX230 = byReadData;

	// FRESETRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FRESETR_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFresetr_RX230 = byReadData;


	// ワード単位レジスタ設定
	eAccessSize = MWORD_ACCESS;		// ワードアクセス

	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	memset(pbyReadData, 0, sizeof(WORD));

	// FENTRYRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FENTRYR;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFentryr_RX230 = wReadData[0];

	// FSARHレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FSARH_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFsarh_RX230 = wReadData[0];

	// FSARLレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FSARL_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFsarl_RX230 = wReadData[0];

	// FEARHレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FEARH_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFearh_RX230 = wReadData[0];

	// FEARLレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FEARL_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFearl_RX230 = wReadData[0];

	// FWB0レジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FWB0_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wfwb0_RX230 = wReadData[0];

	// FWB1レジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FWB1_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wfwb1_RX230 = wReadData[0];

	// FWB2レジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FWB2_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wfwb2_RX230 = wReadData[0];

	// FWB3レジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FWB3_RX230;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wfwb3_RX230 = wReadData[0];

	return FFWERR_OK;
}
// RevRxNo140515-006 Append End

// RevRxNo140515-006 Append Start
//=============================================================================
/**
 * RX230　Flashアクセス時に書き換わるSFR領域の復帰
 * @param  
 *         
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR retFlashReg_RX230(void)
{
	FFWERR						ferr = FFWERR_OK;
	DWORD						dwAddrCnt,dwDataCnt,dwAcsCnt,dwRegCnt;
	DWORD						madrStartAddr[MCU_REG_FLASH_SET_REGNUM_RX230];
	enum FFWENM_MACCESS_SIZE	eAccessSize[MCU_REG_FLASH_SET_REGNUM_RX230];
	BYTE						byWriteData[MCU_REG_FLASH_SET_REGNUM_RX230 *2];
	enum FFWENM_VERIFY_SET 		eVerify = VERIFY_OFF;
	WORD						wData;
	WORD						wDataTmp;
	BYTE						byData;
	DWORD						dwAreaNum,dwLength;
	BOOL						bSameAccessSize,bSameLength,bSameWriteData;
	FFW_VERIFYERR_DATA 			VerifyErr;
	BYTE						byEndian = ENDIAN_BIG;

	// 復帰データ格納用配列を初期化
	memset(&byWriteData, 0, MCU_REG_FLASH_SET_REGNUM_RX230 *2);

	// カウント値初期化
	dwAddrCnt = 0;
	dwDataCnt = 0;
	dwAcsCnt = 0;
	dwRegCnt = 0;

	// FENTRYRでP/Eモード遷移
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FENTRYR;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FENTRYR_KEY_RX100 | MCU_REG_FLASH_FENTRYR_FENTRYI_DATA_RX100;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// ***特定シーケンス実行***
	// FPRレジスタ設定
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPR_DATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ設定でプログラミングモード遷移(現時点で無効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPMCR_PESET_DATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ反転値設定でプログラミングモード遷移(現時点で無効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPMCR_PESET_NOTDATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ設定でプログラミングモード遷移(現時点で有効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPMCR_PESET_DATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;


	// ***ユーザ値復帰***
	// ワード単位レジスタ
	// escFlashReg()でGetMcuSfrReg()を使用してDUMPしているため、ワード単位
	// レジスタ値はMCU仕様と同じ並びになっている。
	// 本関数はPROT_MCU_WRITE()で一度にWRITEするためワード値は上下8bitを
	// 反転する必要がある。
	// FWB3値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FWB3_RX230;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wfwb3_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FWB2値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FWB2_RX230;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wfwb2_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FWB1値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FWB1_RX230;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wfwb1_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FWB0値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FWB0_RX230;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wfwb0_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FSARL値復帰
 	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FSARL_RX230;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wFsarl_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FEARL値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FEARL_RX230;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wFearl_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FSARHL値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FSARH_RX230;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wFsarh_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FEARH値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FEARH_RX230;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = s_wFearh_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// バイト単位レジスタ
	// FISR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FISR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFisr_RX230;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FASR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FASR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFasr_RX230;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FCR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FCR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFcr_RX230;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FEXCR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FEXCR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFexcr_RX230;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FRESETR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FRESETR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFresetr_RX230;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;


	// ***特定シーケンス実行***
	// FPRレジスタ設定
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = MCU_REG_FLASH_FPR_DATA_RX100;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ設定でユーザ値復帰(現時点で無効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFpmcr_RX230;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ反転値設定でユーザ値復帰(現時点で無効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = ~(s_byFpmcr_RX230);
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FPMCRレジスタ設定でユーザ値復帰(現時点で有効)
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPMCR_RX230;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFpmcr_RX230;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// DFLCTL値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_DFLCTL_RX100;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byDflctl_RX230;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// FENTRYR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FENTRYR;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FENTRYR_KEY_RX100 | s_wFentryr_RX230;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;


	dwAreaNum = dwRegCnt;			//設定イベント個数
	bSameAccessSize = FALSE;		//アクセスサイズ混在
	bSameLength = TRUE;
	dwLength = 1;
	bSameWriteData = FALSE;


	// ヘッダを含めて0x800hバイトを超える、PROT_MCU_WRITE()関数が呼ばれることはないので、BFWMCUCmd_WRITEコマンドの
	// ライトバイト数最大値でアクセスバイト数を分割してPROT_MCU_WRITE()関数を呼び出す必要はない。
	ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, madrStartAddr, bSameAccessSize, eAccessSize,
					bSameLength, &dwLength, bSameWriteData, &byWriteData[0], &VerifyErr, byEndian);

	return ferr;

}
// RevRxNo140515-006 Append End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * RX640 Flashアクセス時に書き換わるSFR領域の退避/復帰
 * @param  bEscape   TRUE  : 退避
 *                   FALSE : 復帰
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR escFlashAccessSfrArea_RX640(BOOL bEscape)
{
	FFWERR 						ferr = FFWERR_OK;

	if (bEscape == TRUE) {		// 退避時
		ferr = escFlashReg_RX640();
	} else {					// 復帰時
		ferr = retFlashReg_RX640();
	}

	return ferr;
}
// RevRxNo130301-001 Append End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * RX640　Flashアクセス時に書き換わるSFR領域の退避
 * @param  
 *         
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR escFlashReg_RX640(void)
{
	FFWERR 						ferr = FFWERR_OK;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrRWaddr;			// 設定/参照するレジスタのアドレス
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE						byReadData;			// バイト参照データ格納領域
	WORD						wReadData[1];		// ワード参照データ格納領域(バイトポインタ指定のため、配列定義)
	DWORD						dwReadData[1];		// ロング参照データ格納領域(バイトポインタ指定のため、配列定義)

	// バイト単位レジスタ設定
	eAccessSize = MBYTE_ACCESS;		// バイトアクセス

	// FWEPRORレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FWEPROR_RX640;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, &byReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_byFwepror_RX640 = byReadData;

	// ワード単位レジスタ設定
	eAccessSize = MWORD_ACCESS;		// ワードアクセス

	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	memset(pbyReadData, 0, sizeof(WORD));

	// FCURAMEレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FCURAME_RX640;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFcurame_RX640 = wReadData[0];

	// FPCKARレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FPCKAR_RX640;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFpckar_RX640 = wReadData[0];

	// FSUINITRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FSUINITR_RX640;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFsuinitr_RX640 = wReadData[0];

	// FENTRYRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FENTRYR_RX640;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFentryr_RX640 = wReadData[0];

	// FMATSELCレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FMATSELC_RX640;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFmatselc_RX640 = wReadData[0];

	// FPROTRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FPROTR_RX640;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_wFprotr_RX640 = wReadData[0];


	// ロング単位レジスタ設定
	eAccessSize = MLWORD_ACCESS;		// ロングアクセス
	pbyReadData = reinterpret_cast<BYTE*>(dwReadData);

	// FSADDRレジスタの読み出し
	madrRWaddr = MCU_REG_FLASH_FSADDR_RX640;
	ferr = GetMcuSfrReg(madrRWaddr, eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	s_dwFsaddr_RX640 = dwReadData[0];

	return ferr;
}
// RevRxNo130301-001 Append End

// RevRxNo130301-001 Append Start
//=============================================================================
/**
 * RX640　Flashアクセス時に書き換わるSFR領域の復帰
 * @param  
 *         
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//=============================================================================
static FFWERR retFlashReg_RX640(void)
{
	FFWERR						ferr = FFWERR_OK;
	DWORD						dwAddrCnt,dwDataCnt,dwAcsCnt,dwRegCnt;
	DWORD						madrStartAddr[MCU_REG_FLASH_SET_REGNUM_BYTE_RX640 + MCU_REG_FLASH_SET_REGNUM_WORD_RX640 + MCU_REG_FLASH_SET_REGNUM_LWORD_RX640];
	enum FFWENM_MACCESS_SIZE	eAccessSize[MCU_REG_FLASH_SET_REGNUM_BYTE_RX640 + MCU_REG_FLASH_SET_REGNUM_WORD_RX640 + MCU_REG_FLASH_SET_REGNUM_LWORD_RX640];
	BYTE						byWriteData[MCU_REG_FLASH_SET_REGNUM_BYTE_RX640 + (MCU_REG_FLASH_SET_REGNUM_WORD_RX640 * 2) + (MCU_REG_FLASH_SET_REGNUM_LWORD_RX640 * 4)];
	enum FFWENM_VERIFY_SET 		eVerify = VERIFY_OFF;
	BYTE						byData;
	WORD						wData;
	WORD						wDataTmp;
	DWORD						dwAreaNum,dwLength;
	BOOL						bSameAccessSize,bSameLength,bSameWriteData;
	FFW_VERIFYERR_DATA 			VerifyErr;
	BYTE						byEndian = ENDIAN_BIG;
	DWORD						dwData;

	// 復帰データ格納用配列を初期化
	memset(&byWriteData, 0, (MCU_REG_FLASH_SET_REGNUM_BYTE_RX640 + (MCU_REG_FLASH_SET_REGNUM_WORD_RX640 * 2) + (MCU_REG_FLASH_SET_REGNUM_LWORD_RX640 * 4)));

	// カウント値初期化
	dwAddrCnt = 0;
	dwDataCnt = 0;
	dwAcsCnt = 0;
	dwRegCnt = 0;


	// ロング単位レジスタ
	// FSADDR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FSADDR_RX640;
	eAccessSize[dwAcsCnt] = MLWORD_ACCESS;
	dwData = s_dwFsaddr_RX640;
	dwData = ((dwData << 24) & 0xFF000000) | ((dwData << 8) & 0x00FF0000) | ((dwData >> 8) & 0x0000FF00) | ((dwData >> 24) & 0x000000FF);
	memcpy(&byWriteData[dwDataCnt], &dwData, sizeof(DWORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 4;

	// ワード単位レジスタ
	// escFlashReg()でGetMcuSfrReg()を使用してDUMPしているため、ワード単位
	// レジスタ値はMCU仕様と同じ並びになっている。
	// 本関数はPROT_MCU_WRITE()で一度にWRITEするためワード値は上下8bitを
	// 反転する必要がある。
	// FCURAME値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FCURAME_RX640;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FCURAME_RX640_KEYCODE | s_wFcurame_RX640;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FPCKAR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPCKAR_RX640;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FPCKAR_RX640_KEYCODE | s_wFpckar_RX640;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FENTRYR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FENTRYR_RX640;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FENTRYR_RX640_KEYCODE | s_wFentryr_RX640;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FMATSELC値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FMATSELC_RX640;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FMATSELC_RX640_KEYCODE | s_wFmatselc_RX640;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// FPROTR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FPROTR_RX640;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FPROTR_RX640_KEYCODE | s_wFprotr_RX640;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	// バイト単位レジスタ
	// FWEPROR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FWEPROR_RX640;
	eAccessSize[dwAcsCnt] = MBYTE_ACCESS;
	byData = s_byFwepror_RX640;
	memcpy(&byWriteData[dwDataCnt], &byData, sizeof(BYTE));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 1;

	// ワード単位レジスタ
	// FSUINITR値復帰
	madrStartAddr[dwAddrCnt] = MCU_REG_FLASH_FSUINITR_RX640;
	eAccessSize[dwAcsCnt] = MWORD_ACCESS;
	wData = MCU_REG_FLASH_FSUINITR_RX640_KEYCODE | s_wFsuinitr_RX640;
	wDataTmp = (wData >> 8) & 0x00FF;
	wData = ((wData << 8) & 0xFF00) | wDataTmp;
	memcpy(&byWriteData[dwDataCnt], &wData, sizeof(WORD));
	dwRegCnt++;
	dwAddrCnt++;
	dwAcsCnt++;
	dwDataCnt = dwDataCnt + 2;

	dwAreaNum = dwRegCnt;			//設定イベント個数
	bSameAccessSize = FALSE;		//アクセスサイズ混在
	bSameLength = TRUE;
	dwLength = 1;
	bSameWriteData = FALSE;

// RevRxNo121022-001 Append Start
	// ヘッダを含めて0x800hバイトを超える、PROT_MCU_WRITE()関数が呼ばれることはないので、BFWMCUCmd_WRITEコマンドの
	// ライトバイト数最大値でアクセスバイト数を分割してPROT_MCU_WRITE()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
	ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, madrStartAddr, bSameAccessSize, eAccessSize,
					bSameLength, &dwLength, bSameWriteData, &byWriteData[0], &VerifyErr, byEndian);

	return ferr;

}
// RevRxNo130301-001 Append End



// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * FCU-RAM領域データの復帰処理
 *  FCURAME 有効状態で呼ぶこと。本処理内でFCURAME操作はしない。// RevNo111121-004 コメントのみ追加
 * @param  なし
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR setFcuRam(void)
{
	FFWERR						ferr = FFWERR_OK;	// RevRxNo120910-011	Modyify	Line
													// FCU-RAMが存在しない時も戻り値をFFWERR_OKにしておく。
	enum FFWENM_VERIFY_SET		eVerify;			// ベリファイON/OFF
	FFW_VERIFYERR_DATA			VerifyErr;			// ベリファイ結果格納構造体
	// RevRxNo130301-001 Append Start
	WORD						wWriteData[1];
	MADDR						madrRWaddr;			// レジスタ先頭アドレス
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	FFWMCU_MCUDEF_DATA* pMcuDef;
	// RevRxNo130301-001 Append End

	FFWMCU_MCUAREA_DATA_RX*		pMcuAreaData;		// MCU情報構造体のポインタ

	// RevRxNo130301-001 Append Line
	pMcuDef = GetMcuDefData();
	pMcuAreaData = GetMcuAreaDataRX();	// MCU情報を取得

	// RevRxNo130301-001 Append Line
	pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);

	// RevRxNo120910-011	Modify Start
	if( pMcuAreaData->dwFcuFirmLen != 0 ) {		// FCU-RAMが存在する時のみ以下の処理を実施

		// RevRxNo130301-001 Append Start
		// FCUファーム復帰のため、FCURAMEを有効に設定する。
		// なお、FCURAMEはフラッシュ書き換え終了後、FFWが復帰関数(escFlashAccessSfrAreaByteなど)で
		// ユーザ値に復帰するため、本関数ではFCURAME有効設定のみ実施する。
		madrRWaddr = pMcuDef->madrFcurame;
		eAccessSize = MWORD_ACCESS;
		wWriteData[0] = MCU_REG_FLASH_FCURAME_DATA;

		ferr = SetMcuSfrReg(madrRWaddr,eAccessSize, pbyWriteData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevRxNo130301-001 Append End

		eVerify = VERIFY_OFF;
		VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	
		//退避していたFCU-RAMを復帰
		ferr = DO_WRITE(s_dwmadrFcuRamStartAddr, s_dwmadrFcuRamEndAddr, eVerify, MBYTE_ACCESS, s_pbyFcuRamReadData, &VerifyErr);
	}
	// RevRxNo120910-011	Modify End
	return ferr;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End


// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
//=============================================================================
/**
 * FCU-RAM領域データの退避処理
 *  FCURAME 有効状態で呼ぶこと。本処理内でFCURAME操作はしない。// RevNo111121-004 コメントのみ追加
 * @param  なし
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR getFcuRam(void)
{
	FFWERR						ferr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	enum FFWENM_VERIFY_SET		eVerify;			// ベリファイON/OFF
	FFW_VERIFYERR_DATA			VerifyErr;			// ベリファイ結果格納構造体
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameLength;
	DWORD						dwLength;
	BYTE						byEndian;
	// RevRxNo130301-001 Append Start
	WORD						wWriteData[1];
	MADDR						madrRWaddr;			// レジスタ先頭アドレス
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	FFWMCU_MCUDEF_DATA* pMcuDef;
	// RevRxNo130301-001 Append End


	FFWMCU_MCUAREA_DATA_RX*		pMcuAreaData;		// MCU情報構造体のポインタ

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData); // エミュレータ情報取得
// RevRxNo121022-001 Append End

	// RevRxNo130301-001 Append Line
	pMcuDef = GetMcuDefData();
	pMcuAreaData = GetMcuAreaDataRX();	// MCU情報を取得

	// RevRxNo130301-001 Append Line
	pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);

	// RevRxNo120910-011	Modify Start
	if( pMcuAreaData->dwFcuFirmLen != 0 ) {		// FCU-RAMが存在する時のみ以下の処理を実施

		// RevRxNo130301-001 Append Start
		// FCUファーム待避のため、FCURAMEを有効に設定する。
		// なお、FCURAMEはフラッシュ書き換え終了後、FFWが復帰関数(escFlashAccessSfrAreaByteなど)で
		// ユーザ値に復帰するため、本関数ではFCURAME有効設定のみ実施する。
		madrRWaddr = pMcuDef->madrFcurame;
		eAccessSize = MWORD_ACCESS;
		wWriteData[0] = MCU_REG_FLASH_FCURAME_DATA;

		ferr = SetMcuSfrReg(madrRWaddr,eAccessSize, pbyWriteData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevRxNo130301-001 Append End

		eVerify = VERIFY_OFF;
		dwAreaNum = 1;
		bSameAccessSize = FALSE;
		bSameLength = FALSE;
		VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	
		//FCU-RAM領域データの退避
		s_dwmadrFcuRamStartAddr =  pMcuAreaData->dwAdrFcuRamStart;
		// RevRxNo120910-001 Modify Line
		s_dwmadrFcuRamEndAddr = pMcuAreaData->dwAdrFcuRamStart + pMcuAreaData->dwFcuFirmLen - 1;
		s_pbyFcuRamReadData = s_byFcuRamReadData;
		// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify Start
		dwLength = s_dwmadrFcuRamEndAddr - s_dwmadrFcuRamStartAddr + 1;
		eAccessSize = MBYTE_ACCESS;
		ferr = GetEndianType2(s_dwmadrFcuRamStartAddr, &byEndian);	// RevRxE2LNo141104-001 Modify Line
		if (ferr != FFWERR_OK) {		  
			return ferr;
		}

// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr = McuDumpDivide(dwAreaNum, s_dwmadrFcuRamStartAddr, bSameAccessSize, eAccessSize, bSameLength, dwLength, s_pbyFcuRamReadData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
		} else{
			ferr = PROT_MCU_DUMP(dwAreaNum, &s_dwmadrFcuRamStartAddr, bSameAccessSize, &eAccessSize, bSameLength, &dwLength, s_pbyFcuRamReadData, byEndian);
		}
// RevRxNo121022-001 Append Ens

		if (ferr != FFWERR_OK) {		  
			return ferr;
		}
		// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify End
	}
	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify End
	// RevRxNo120910-011	Modify End
	return FFWERR_OK;
}
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
// RevNo111121-007 Append Start
//=============================================================================
/**
 * 指定アドレスがMCU内蔵FCU ファーム領域であるかを判定する。
 * @param madrAddr MCUアドレス
 * @retval TRUE アドレスはMCU内蔵FCU ファーム領域である
 * @retval FALSE アドレスはMCU内蔵FCU ファーム領域でない
 */
//=============================================================================
BOOL IsMcuFcuFirmArea(MADDR madrAddr)
{
	BOOL	bRet = FALSE;

	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;
	pMcuArea = GetMcuAreaDataRX();

	// 指定アドレスがFCU ファーム領域であるかをチェック
	// RevRxNo120910-001 Modify Line
	if (pMcuArea->dwFcuFirmLen != 0) {
		if ((pMcuArea->dwAdrFcuFirmStart <= madrAddr) && 
			// RevRxNo120910-001 Modify Line
			(madrAddr <= (pMcuArea->dwAdrFcuFirmStart + pMcuArea->dwFcuFirmLen - 1))) {
			bRet = TRUE;
		}
	}

	return	bRet;
}
// RevNo111121-007 Append End

//=============================================================================
/**
 * BFWへのフラッシュメモリ書き込み開始通知情報を取得する。
 * @param なし
 * @retval TRUE フラッシュメモリ書き込み開始通知済み (BFWMCUCmd_FWRITESTARTを送信済み)
 * @retval FALSE フラッシュメモリ書き込み開始を未通知 (BFWMCUCmd_FWRITESTARTを未送信)
 */
//=============================================================================
static BOOL getSendFlashWriteStartFlg(void)
{
	return s_bSendFlashWriteStartFlg;
}

//=============================================================================
/**
 * BFWへのフラッシュメモリ書き込み開始通知情報を通知済みに設定する。
 * @param なし
 * @retval なし
 */
//=============================================================================
static void setSendFlashWriteStartFlg(void)
{
	s_bSendFlashWriteStartFlg = TRUE;
}

//=============================================================================
/**
 * BFWへのフラッシュメモリ書き込み開始通知情報を未通知に設定する。
 * @param なし
 * @retval なし
 */
//=============================================================================
static void clrSendFlashWriteStartFlg(void)
{
	s_bSendFlashWriteStartFlg = FALSE;
}

//=============================================================================
/**
 * ターゲットMCU制御関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData_Flash(void)
{

	int	i;

	s_bSendFlashWriteStartFlg = FALSE;

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Line
	InitFlashRomCacheMemFlg();

	// RevRxNo140109-001 Delete: InitExtraCacheMemFlg();

	s_ChecksumData.byCntMem = 0;
	for (i = 0; i < DWNP_FLASHTYPE_MAX; i++) {
		s_ChecksumData.byMemType[i] = 0;
		s_ChecksumData.eCheck[i] = CHECKSUM_AGREE;
		s_ChecksumData.pdwCheckSum[i] = 0;
	}

	// V.1.02 新デバイス対応 Mpdify Start
	s_wDataFlashEnableData[0] = 0;
	s_wDataFlashEnableData[1] = 0;
	// V.1.02 新デバイス対応 Mpdify End

	for (i = 0; i < MAX_FLASHACCESS_SFR_BYTE; i++) {
		s_byFlashAccessSfr[i] = 0x00;
	}

	for (i = 0; i < MAX_FLASHACCESS_SFR_WORD; i++) {
		s_wFlashAccessSfr[i] = 0x0000;
	}

	for (i = 0; i < MAX_FLASHACCESS_SFR_LWORD; i++) {
		s_dwFlashAccessSfr[i] = 0x00000000;
	}

	//Flashダウンロード時のWorkRamサイズを動的変化させる対応
	//Flashダウンロード時のWorkRam退避用キャッシュ領域開放 Append
	if(s_pbyReadData != NULL){
		if(s_bEscWorkRam == TRUE){
			delete [] s_pbyReadData;
		}
	}
	s_bEscWorkRam = FALSE;

	// RevRxNo140109-001 Append Line
	s_bOfs1Lvd1renChangeFlg = FALSE;	// OFS1レジスタの電圧監視1リセットを無効設定に変更したフラグを初期化

	return;
}


// RevRxNo120910-004 Append Start
//==============================================================================
/**
 * "RX100のOFS1レジスタの電圧監視1リセットを無効設定に変更したフラグ"のアドレス
 * を取得する。
 * @param  なし
 * @retval "RX100のOFS1レジスタの電圧監視1リセットを無効設定に変更したフラグ"の
 *         アドレス
 */
//==============================================================================
BOOL* GetOfs1Lvd1renChangeFlgPtr(void)
{
	return &s_bOfs1Lvd1renChangeFlg;
}
// RevRxNo120910-004 Append End


// RevRxNo121022-001 Append Start
//=============================================================================
/**
 * BFWMCUCmd_FWRITEコマンドのライトバイト数最大値でアクセスバイト数を分割して
 * PROT_MCU_FWRITE()関数を呼び出す(EZ-CUBE用)
 *
 * @param byWriteArea 書き換え領域指定(BFW_FWRITE_AREA_FLASH/BFW_FWRITE_AREA_EXTRA_A)
 * @param eVerify			べリファイ指定(VERIFY_OFF/VERIFY_ON)
 * @param dwAreaNum			ライト領域数
 * @param madrWriteAddr		ライト開始アドレス
 * @param bSameAccessSize	同一アクセスサイズ指定フラグ
 * @param eAccessSize		アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameAccessCount	同一ライトアクセス回数指定フラグ
 * @param dwAccessCount		1領域のライトアクセス回数(1～0x10000)
 * @param bSameWriteData	同一ライトデータ指定フラグ
 * @param pbyWriteData		ライトデータ格納用バッファへのポインタ
 * @param pVerifyErrorInfo	ベリファイ結果格納用バッファアドレス
 * @param byEndian			エンディアン指定(0:リトル、1:ビッグ)
 *
 *   ※以下の引数は、必ず固定値で渡すこと。
 *   ・dwAreaNum = 1;
 *   ・bSameAccessSize = FALSE;
 *   ・bSameAccessCount = FALSE;
 *   ・bSameWriteData = FALSE;
 *
 * @retval FFWエラーコード
 */
//=============================================================================
// RevRxNo130301-001 Modify Line
FFWERR McuFlashWriteDivide(BYTE byWriteArea, enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, MADDR madrWriteAddr,
						   BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize,
						   BOOL bSameAccessCount, DWORD dwAccessCount, 
						   BOOL bSameWriteData, BYTE* pbyWriteData, 
						   FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian, DWORD dwWriteMaxLength)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAddAddr;			// アドレス加算値
	DWORD	dwTotalLength;		// トータルアクセスバイトサイズ
	DWORD	dwLength=0;			// アクセスバイトサイズ
	DWORD	dwWriteAccCount;	// アクセス回数
	MADDR	madrStartAddr;
	BYTE*	pbyBuff;			// WRITEデータ格納バッファのポインタ

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwAddAddr = 1;	// アドレス加算値算出
		break;
	case MWORD_ACCESS:
		dwAddAddr = 2;	// アドレス加算値算出
		break;
	case MLWORD_ACCESS:
		dwAddAddr = 4;	// アドレス加算値算出
		break;
	default:
		dwAddAddr = 4;	// アドレス加算値算出
		break;
	}

	// アクセス回数をアクセスバイトサイズに変換
	dwTotalLength = dwAccessCount * dwAddAddr;
	// ライト開始アドレスを取得
	madrStartAddr = madrWriteAddr;
	// WRITEデータ格納バッファのポインタを取得
	pbyBuff = pbyWriteData;

	while (dwTotalLength > 0) {
		if (dwTotalLength > dwWriteMaxLength) {
			dwLength = dwWriteMaxLength;
		}
		else{
			dwLength = dwTotalLength;
		}

		//アクセスバイトサイズをアクセス回数に変換
		dwWriteAccCount = dwLength/dwAddAddr;
		if (dwWriteAccCount < 1) {
			dwWriteAccCount = 1;
		}

		// RevRxNo130301-001 Modify Line
		ferr = PROT_MCU_FWRITE(byWriteArea, eVerify, dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
								bSameAccessCount, &dwWriteAccCount, bSameWriteData, pbyBuff, pVerifyErrorInfo, byEndian);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		dwTotalLength -= dwLength;			// 残りWRITEサイズ更新
		madrStartAddr += dwLength;			// WRITE開始アドレス更新

		if (dwTotalLength > 0) {
			pbyBuff += dwLength;			// WRITEデータ格納バッファアドレス更新
		}
	}

	return ferr;

}
// RevRxNo121022-001 Append End

// RevRxNo121206-001 Append Start
//=============================================================================
/**
 * MCUのブートスワップ設定初期化に伴うキャッシュの入れ替えを実施する
 * @param なし
 * @retval なし
 */
//=============================================================================
void SwapFlashRomCacheData(void)
{
	BYTE	byCacheMemTmp;				// 入れ替え用のテンポラリデータ
	DWORD	dwBootSwapSize;				// クラスタサイズ(4KB：0x1000, 8KB：0x2000, 16KB：0x4000)
	DWORD	i;
	MADDR	madrBlkStartAddr;
	MADDR	madrBlkEndAddr;
	MADDR	madrCacheCluster0;			// クラスタ0のオフセットアドレス
	MADDR	madrCacheCluster1;			// クラスタ1のオフセットアドレス
	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得

	if(GetNewCacheMem(MAREA_USER) == TRUE){		// ユーザマットのキャッシュを作成している場合
		dwBootSwapSize = GetBootSwapSize();		// クラスタサイズを取得する

		madrBlkStartAddr = GetCacheStartAddr(MAREA_USER);	// ROM領域の先頭アドレスを取得
		madrBlkEndAddr = GetCacheEndAddr(MAREA_USER);		// ROM領域の最終アドレスを取得

		// クラスタ0,1のオフセットアドレスを算出する
		madrCacheCluster0 = madrBlkEndAddr - (dwBootSwapSize - 1) - madrBlkStartAddr;
		madrCacheCluster1 = madrBlkEndAddr - ((dwBootSwapSize * 2) - 1) - madrBlkStartAddr;

		// クラスタ0データ→テンポラリ、クラスタ1データをクラスタ0、テンポラリ→クラスタ1の
		// 順番でコピーする処理をクラスタサイズ分、繰り返す
		for (i = 0; i < dwBootSwapSize; i++) {
			// クラスタ0データをテンポラリにコピー
			byCacheMemTmp = um_ptr->pbyCacheMem[madrCacheCluster0];

			// クラスタ1データをクラスタ0にコピー
			um_ptr->pbyCacheMem[madrCacheCluster0] = um_ptr->pbyCacheMem[madrCacheCluster1];

			// テンポラリデータをクラスタ1にコピー
			um_ptr->pbyCacheMem[madrCacheCluster1] = byCacheMemTmp;

			// オフセットアドレスをインクリメントする
			madrCacheCluster0 = madrCacheCluster0 + 1;
			madrCacheCluster1 = madrCacheCluster1 + 1;
		}
	}

	return;
}
// RevRxNo121206-001 Append End

// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * スタートアッププログラム保護機能スワップ領域のブロック書き換えフラグ入れ替え処理
 * @param  なし
 * @retval なし
 */
//=============================================================================
FFWERR SwapFlashWriteFlag(void)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	madrBlkStartAddrCluster0;
	MADDR	madrBlkStartAddrCluster1;
	MADDR	madrBlkEndAddrCluster0;
	MADDR	madrBlkEndAddrCluster1;
	MADDR	madrReadAddrCluster0;
	MADDR	madrReadAddrCluster1;
	DWORD	dwBlkNoCluster0;
	DWORD	dwBlkNoCluster1;
	DWORD	dwBlkSizeCluster0;
	DWORD	dwBlkSizeCluster1;
	USER_ROMCACHE_RX		*um_ptr;		// ユーザーマットキャッシュメモリ構造体ポインタ

	um_ptr = GetUserMatCacheData();			// ユーザーマットキャッシュメモリ構造体情報取得

	if(GetNewCacheMem(MAREA_USER) == TRUE){		// ユーザマットのキャッシュを作成している場合
		madrReadAddrCluster0 = MCU_OSM_OFS1_START;
		madrReadAddrCluster1 = MCU_OSM_OFS1_START - GetBootSwapSize();

		GetFlashRomBlockInf(MAREA_USER, madrReadAddrCluster0, &madrBlkStartAddrCluster0, &madrBlkEndAddrCluster0, &dwBlkNoCluster0, &dwBlkSizeCluster0);
		GetFlashRomBlockInf(MAREA_USER, madrReadAddrCluster1, &madrBlkStartAddrCluster1, &madrBlkEndAddrCluster1, &dwBlkNoCluster1, &dwBlkSizeCluster1);

		// クラスタ1のブロック書き換えフラグおよびブロック領域書き換えフラグを書き換えなしに設定
		um_ptr->pbyBlockWriteFlag[dwBlkNoCluster1] = FALSE;
		um_ptr->pbyBlockAreaWriteFlag[dwBlkNoCluster1/CACHE_BLOCK_AREA_NUM_RX] = FALSE;

		// クラスタ0のブロック書き換えフラグおよびブロック領域書き換えフラグを書き換えありに設定
		um_ptr->pbyBlockWriteFlag[dwBlkNoCluster0] = TRUE;
		um_ptr->pbyBlockAreaWriteFlag[dwBlkNoCluster0/CACHE_BLOCK_AREA_NUM_RX] = TRUE;
	}

	return ferr;
}
// RevRxNo130730-001 Append End

// RevRxNo140617-001 Append Start
//=============================================================================
/**
 * ダウンロードデータにTM領域が含まれていたことを示すフラグを設定する
 * @param  bInclude		TRUE:tM領域含まれていた、FALSE:TM領域含まれてなかった
 * @retval なし
 */
//=============================================================================
void SetDwnpDataIncludeTMArea(BOOL bInclude)
{
		s_bDwnpDataIncludeTMArea = bInclude;
		return;
}

//=============================================================================
/**
 * ダウンロードデータにTM領域が含まれていたことを示すフラグ値を取得する
 * @param  なし
 * @retval なし
 */
//=============================================================================
BOOL GetDwnpDataIncludeTMArea()
{
		return s_bDwnpDataIncludeTMArea;
}
// RevRxNo140617-001 Append End

// RevRxNo140617-001 Append Start
//=============================================================================
/**
 * オプション設定メモリのTMINF情報を起動時に取得した情報に差し替える
 * @param  なし
 * @retval なし
 */
//=============================================================================
void setExtraCacheTmData(enum FFWENM_ENDIAN eMcuEndian)
{
	DWORD	dwCacheCnt;
	DWORD	dwTminfData;
	DWORD	dwCnt;
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line

	dwCacheCnt = pMcuDef->dwTminfStartOffset - pMcuDef->dwExtraAStartOffset;	// Extra領域用キャッシュメモリ位置算出	// RevRxNo150827-002 Modify Line
	dwTminfData = GetTMINFData();
	if (eMcuEndian == ENDIAN_LITTLE) {		// リトルエンディアンの場合
		s_byExtraCacheMem[dwCacheCnt] = (BYTE)(dwTminfData & 0xFF);
		s_byExtraCacheMem[dwCacheCnt+1] = (BYTE)((dwTminfData >> 8) & 0xFF);
		s_byExtraCacheMem[dwCacheCnt+2] = (BYTE)((dwTminfData >> 16) & 0xFF);
		s_byExtraCacheMem[dwCacheCnt+3] = (BYTE)((dwTminfData >> 24) & 0xFF);
	}else{									// ビッグエンディアンの場合
		s_byExtraCacheMem[dwCacheCnt] = (BYTE)((dwTminfData >> 24) & 0xFF);
		s_byExtraCacheMem[dwCacheCnt+1] = (BYTE)((dwTminfData >> 16) & 0xFF);
		s_byExtraCacheMem[dwCacheCnt+2] = (BYTE)((dwTminfData >> 8) & 0xFF);
		s_byExtraCacheMem[dwCacheCnt+3] = (BYTE)(dwTminfData & 0xFF);
	}
	dwCnt = dwCacheCnt / EXTRA_A_PROG_SIZE;
	s_bExtraWriteFlag[dwCnt] = TRUE;		// ExtraA領域の16バイト単位での書き換えフラグをセット
	s_bExtraAreaWriteFlag = TRUE;			// ExtraA領域書き換えフラグをセット

	return;
}
// RevRxNo140617-001 Append End

// RevRxNo150828-002 Append Start
//=============================================================================
/**
 * オプション設定メモリダウンロード不可領域の書き換えフラグクリア
 * @param  なし
 * @retval なし
 */
//=============================================================================
static void maskBanDwnpExtraAArea(void)
{
	DWORD	dwCnt, dwStartCnt, dwEndCnt;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;

	pMcuDef = GetMcuDefData();
	pFwCtrl = GetFwCtrlData();

	if (pFwCtrl->eOfsType == RX_OFS_EXTRA) {	// OFSレジスタをExtra領域で設定するMCUの場合
		if ((pMcuDef->dwBanDwnpStartOffset == 0x00000000) && (pMcuDef->dwBanDwnpEndOffset == 0x00000000)) {
			// 開始、終了ともに0の場合は、オプション設定メモリの真ん中にダウンロード不可領域がないということで、処理を抜ける。
			return;
		}

		dwStartCnt = pMcuDef->dwBanDwnpStartOffset / EXTRA_A_PROG_SIZE;	
		dwEndCnt = (pMcuDef->dwBanDwnpEndOffset / EXTRA_A_PROG_SIZE) + 1;	
		for (dwCnt = dwStartCnt; dwCnt < dwEndCnt; dwCnt++) {
			s_bExtraWriteFlag[dwCnt] = FALSE;		// ExtraA領域の16バイト単位での書き換えフラグをFALSEにする
		}
	}
	return;
}
// RevRxNo150828-002 Append End

// RevRxNo160527-001 Append Start +4
//=============================================================================
/**
 * "FAW.FSPRビットを'1'に変更したフラグ"のアドレス
 * を取得する。
 * @param  なし
 * @retval "FAW.FSPRビットを'1'に変更したフラグ"のアドレス
 */
//==============================================================================
BOOL* GetFawFsprChangeFlgPtr(void)
{
	return &s_bFawFsprChangeFlg;
}

// RevRxNo160527-001 Append End
