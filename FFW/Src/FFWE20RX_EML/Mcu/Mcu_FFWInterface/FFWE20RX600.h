///////////////////////////////////////////////////////////////////////////////
/**
 * @file FFWE20RX600.h//
 * @brief E1/E20/E2 LiteエミュレータFFW API関数のヘッダファイル//
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, S.Ueda, PA K.Tsumu, PA M.Takabayashi, PA M.Tsuji//
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.//
 * @date 2018/10/10//
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴//
・ソース構成改善	2012/07/10 橋口//
　　ベース:FFWE20RX600.dll V.1.02.00.015//
・RevRxNo120910-001 2012/09/28 三宅//
  FFW I/F仕様変更に伴うFFWソース変更。//
  ・以下の関数の引数を変更。//
    FFWRXCmd_SetMCU()、FFWRXCmd_GetMCU()、FFWRXCmd_SetDBG()、FFWRXCmd_GetDBG()、//
    FFWRXCmd_GetSTAT()、FFWRXCmd_GetPROG()、FFWRXCmd_SetPMOD()、FFWRXCmd_GetPMOD()。//
  ・以下の定義追加。//
    MCU_VDET0LEVELNUM_MAX、MCU_OCD_ID_MAX、MCU_ACCCTRL_AREANUM_MAX、//
    MCU_EXT_MEM_CS、MCU_EXT_MEM_SDRAM、MCU_TYPE_3、MCU_TYPE_4。//
  ・XREG_REGNUM_RX の値を 28 から 35 に変更。//
  ・MCU_AREANUM_MAX_RX の値を 255 から 256 に変更。//
  ・以下の enum の定義追加。//
    enum FFWRXENM_BOOTSWAP、enum FFWRXENM_UBAREA_CHANGE_ENA、//
    enum FFWRXENM_EMU_ACCESS_TYPE、enum FFWRXENM_EMU_ACCESS_MEANS。//
  ・enum FFWENM_USEPROGRAMに、変数 EML_EZ_CUBE 追加。//
  ・以下の構造体の定義追加。//
    FFWRX_STAT_DATA、FFWRX_UBAREA_CHANGEINFO_DATA。//
  ・構造体 FFWRX_DBG_DATA に、メンバ eDbgBootSwap を追加。//
  ・構造体 FFWRX_STAT_EML_DATA に、メンバ fNowUVCC を追加。//
  ・構造体 FFWRX_EV_OPC に、メンバ dwBusSel を追加。//
  ・構造体 FFWRX_RM_DATA に、メンバ dwTrcSrcSel を追加。//
  ・構造体 FFWRX_MCUAREA_DATA について、//
    ・メンバ byExtMemType[256] を追加。//
    ・メンバ名 dwFcuFilmLen を dwFcuFirmLen に修正。//
  ・構造体 FFWRX_MCUINFO_DATA について、//
    ・以下のメンバ追加。//
　　　dwVdet0LevelNum、dwVdet0LevelSelMask、dwVdet0LevelSelData[16]、//
　　　dwVdet0Level[16]、dwMcuJtagIdcodeNum、dwMcuFineDeviceCodeNum、//
　　　dwEmuAccCtrlAreaNum、dwmadrEmuAccCtrlStart[32]、dwmadrEmuAccCtrlEnd[32]、//
　　　eEmuAccType[32]、eEmuAccMeans[32]、fTgtPwrVal。//
　　・dwMcuJtagIdcode、wMcuFineDeviceCode を16個の配列に変更。//
    ・以下のメンバ削除。//
　　  dwMcuAreaBlkNum、dwMcuAreaNum[]、dwmadrMcuAreaStartAddr[][]、//
　　  dwmadrMcuAreaEndAddr[][]。//
  ・以下の定義を削除。//
	MCU_MDENR_CFMPTNNUM_MAX、DBG_MCUCODENUM_MAX、XREG_REG_NUM、//
	GEN_REG_0、GEN_REG_1、GEN_REG_2、GEN_REG_3、GEN_REG_4、GEN_REG_5、//
	GEN_REG_6、GEN_REG_7、GEN_REG_8、GEN_REG_9、GEN_REG_10、GEN_REG_11、//
	GEN_REG_12、GEN_REG_13、GEN_REG_14、GEN_REG_15、EV_NUM_MAX_RX、//
	mADD_REG_LIST(list, r) list|=r。//
  ・以下の enum の定義を削除。//
	enum FFWRXENM_FLASHROM_TYPE、enum FFWENM_RM_ENA、//
	enum FFWENM_RM_GETMODE、enum FFWENM_RM_OUTPUT、//
	enum FFWRX_RD_CMDKIND1。//
  ・構造体 FFWRX_DOWNLOAD_WTR_DATA の定義を削除。//
・RevRxNo130301-001 2013/03/26 上田//
	RX64M対応//
・RevRxNo130308-001 2013/03/13 三宅//
  カバレッジ開発対応//
  ・以下の定義を追加。//
    CV_RX_BLKNUM_MAX, enum FFWRXENM_CVB_ENABLE, enum FFWRXENM_CV_BLKNO。//
  ・以下の構造体定義を追加。//
    FFWRX_CVM_DATA, FFWRX_CVB_DATA。//
　・以下の関数宣言を追加。//
　　FFWRXCmd_SetCVM, FFWRXCmd_GetCVM, FFWRXCmd_SetCVB, FFWRXCmd_GetCVB, //
  　FFWRXCmd_GetCVD0, FFWRXCmd_ClrCVD。//
  ・FFWRX_STAT_EML_DATAの定義で、メンバdwCoverageInfo追加。//
・RevRxNo140109-001 2014/01/09 上田//
	RX64Mオプション設定メモリへのダウンロード対応//
・RevRxNo140515-005 2014/06/19 大喜多//
	RX71M対応//
・RevRxE2LNo141104-001 2014/11/18 上田//
	以下のenum定義削除//
	enum FFWRXENM_MAREA_TYPE, enum FFWRXENM_INIT_AREA_TYPE//
・RevRx6z4FpgaNo150525-001 2015/07/14 PA紡車//
  RX6Z4マルチコア対応//
・RevRxNo161003-001 2016/10/03 PA 紡車//
　　RX651-2MB 起動時のバンクモード、起動バンク対応//
・RevRxE2No170201-001 2017/02/01 PA 辻//
	E2 対応//
・RevRxE2No171004-001 2017/10/04 PA 辻//
	E2拡張機能対応//
・RevRxNo180228-001 2018/02/28 PA 辻//
	RX66T-L対応//
・RevRxMultiNo180302-001 2018/03/02 PA高林//
  RXマルチコア対応//
・RevRxNo180625-001,PB18047-CD01-001 2018/06/25 PA 辻//
	RX66T-H/RX72T対応//
・RevRxNo180713-001,PB18047-CD02-001 2018/07/17 PA 紡車//
	RX72M対応//
*/
#ifndef	__FFWE20RX600_H__
#define	__FFWE20RX600_H__

#include "FFWE20.h"

#ifdef	__cplusplus
	extern	"C" {
#endif

/////////// define定義 ///////////
// MCU依存コマンド用//
#define	MCU_AREANUM_MAX			32	// MCUコマンドのSFR,RAM,ROM領域数の最大値//
// RevRxNo120910-001 Modify Line//
#define	MCU_AREANUM_MAX_RX		256	// MCUコマンドのSFR,RAM,ROM領域数の最大値//
#define	MCU_REGBLKNUM_MAX		32	// MCUコマンドのMCUレジスタ情報定義ブロック数の最大値//
#define	MCU_REGADDRNUM_MAX		8	// MCUコマンドのMCUレジスタ情報定義数の最大値//
#define	MCU_AREABLKNUM_MAX		32	// MCUコマンドのMCUアドレス範囲定義ブロック数の最大値//
#define	MCU_AREAADDRNUM_MAX		8	// MCUコマンドのMCUアドレス範囲定義数の最大値//
#define	MCU_AREAPTNUM_MAX		32	// MCUコマンドの内部ROM領域パターン数の最大値//
#define	MCU_BRTABLE_NUM_MAX		16	// ボーレート最大選択数//
#define	MCU_MDENR_PTNNUM_MAX	256	// モードエントリパターンデータの最大数//

#define	DBG_REGBLKNUM_MAX		32	// DBGコマンドのMCUレジスタ情報定義ブロック数の最大値//
#define	DBG_REGADDRNUM_MAX		8	// DBGコマンドのMCUレジスタ情報定義数の最大値//
#define	DBG_AREABLKNUM_MAX		32	// DBGコマンドのMCUアドレス範囲定義ブロック数の最大値//
#define	DBG_AREAADDRNUM_MAX		8	// DBGコマンドのMCUアドレス範囲定義数の最大値//

//#define STAT_UCON_NUM			3	// STATコマンドのユーザケーブル接続情報数//

#define	MCU_SPC_MAX				8	// MCUコマンドのSPC変数の最大値//
// RevRxNo120910-001 Append Line//
#define	MCU_VDET0LEVELNUM_MAX	16	// MCUコマンドの電圧検出0レベル設定数の最大値//
// RevRxNo120910-001 Append Line//
#define	MCU_OCD_ID_MAX			16	// MCUコマンドの対象デバイスJTAG IDコード設定数、対象デバイスFINE デバイスコード設定数の最大値//
// RevRxNo120910-001 Append Line//
#define	MCU_ACCCTRL_AREANUM_MAX	32	// MCUコマンドの個別アクセス制御領域数の最大値//

// RevRxNo120910-001 Append Start//
#define MCU_EXT_MEM_CS			0x00	// MCUコマンドの外部メモリの種別 CS領域//
#define MCU_EXT_MEM_SDRAM		0x01	// MCUコマンドの外部メモリの種別 SDRAM領域//
// RevRxNo120910-001 Append End//

// V.1.02 新デバイス対応 Modify Line
#define STAT_KIND_ALL			0x0F	// STATコマンドの取得情報指定値(SFR/JTAG/FINE/EML全て)//
// V.1.02 新デバイス対応 Append Start
#define STAT_KIND_ALL_J			0x07	// STATコマンドの取得情報指定値(SFR/JTAG/EML全て)//
#define STAT_KIND_ALL_F			0x0D	// STATコマンドの取得情報指定値(SFR/FINE/EML全て)//
#define STAT_KIND_FINE			0x08	// STATコマンドの取得情報指定値(FINE)//
// V.1.02 新デバイス対応 Append End
#define STAT_KIND_SFR			0x04	// STATコマンドの取得情報指定値(SFR)//
#define STAT_KIND_JTAG			0x02	// STATコマンドの取得情報指定値(JTAG)//
#define STAT_KIND_EML			0x01	// STATコマンドの取得情報指定値(EML)//
#define STAT_KIND_NON			0x00	// STATコマンドの取得情報指定値(なし)//

#define	UBCODE_MAX				8	// UBコードの最大値//

// RevRxNo130301-001 Append Line//
#define FMCD_NUM_MAX			64	// フラッシュメモリのコンフィギュレーションデータ設定数の最大値//

// レジスタ操作コマンド用//
// RevRxNo120910-001 Modify Line//
#define XREG_REGNUM_RX			35	// XREGコマンドのレジスタ個数//
// RevRxMultiNo180302-001 Append Line//
#define AREG_REGNUM_RX			36	// AREGコマンドのレジスタ個数//
// RevRxNo180625-001 Append Line//
#define REGBANK_REGNUM			23	// REGBANKコマンドのレジスタ個数//

//当該定義は未使用ですが、構造体定義でパディング規則をここで明示しています//
#define PADDING_4BYTE	4	//構造体パディング：4byte//

// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Line
// プログラムダウンロード関連コマンド//
#define FOVR_RX_BLKNUM_MAX			16		// FOVRコマンドのフラッシュ領域パターン数の最大値//

// イベント設定最大数//
#define EV_ADDR_MAX_RX			8				// 
#define EV_OPC_MAX_RX			4				// 

// イベント番号//
#define PE0							0x00			// 
#define PE1							0x01			// 
#define PE2							0x02			// 
#define PE3							0x03			// 
#define PE4							0x04			// 
#define PE5							0x05			// 
#define PE6							0x06			// 
#define PE7							0x07			// 
#define DE0							0x20			// 
#define DE1							0x21			// 
#define DE2							0x22			// 
#define DE3							0x23			// 

// デバッグコンソール コマンド用 (バッファサイズ)//
#define C2E_BUFF_LEN				256
#define E2C_BUFF_LEN				256

// MCUコマンドで設定する値
#define MCU_TYPE_0			0x00		// RX610 / RX62x//
#define MCU_TYPE_1			0x01		// RX63x//
#define MCU_TYPE_2			0x02		// RX200//
// RevRxNo120910-001 Append Line//
#define MCU_TYPE_3			0x03		// RX100//
// RevRxNo120910-001 Append Line//
#define MCU_TYPE_4			0x04		// RX64x//

#define IF_TYPE_0			0x00		// JTAG//
#define IF_TYPE_1			0x01		// FINE//

// RevRxNo130308-001 Append Start
// カバレッジ計測関連コマンド用//
#define	CV_RX_BLKNUM_MAX	4			// RX用カバレッジ計測領域のブロック数最大値//
// RevRxNo130308-001 Append End

/////////// enum定義 ///////////
//---------------------------
// MCU依存コマンド//
//---------------------------
// STAT
enum FFWRXENM_STAT_MCU {			// ターゲットMCUのステータス情報//
	RX_MCUSTAT_NORMAL=0,				// ターゲットMCUは正常動作状態である//
	RX_MCUSTAT_RESET,					// ターゲットMCUはリセット状態である//
	RX_MCUSTAT_SLEEP,					// ターゲットMCUはスリープ状態である//
	RX_MCUSTAT_SOFT_STDBY,				// ターゲットMCUはソフトウェアスタンバイ状態である//
	RX_MCUSTAT_DEEP_STDBY,				// ターゲットMCUはディープスタンバイ状態である//
};
enum FFWRX_STAT_ENDIAN {			// エンディアン指定状況を格納//
	FFWRX_ENDIAN_LITTLE=0,				// リトルエンディアン//
	FFWRX_ENDIAN_BIG,					// ビッグエンディアン//
};
enum FFWRX_STAT_BAN {				// デバッガ接続可否情報を格納//
	FFWRX_BAN_ENABLE=0,					// デバッガ接続可//
	FFWRX_BAN_DISABLE,					// デバッガ接続禁止//
};
enum FFWRX_STAT_AUTH {				// デバッガ認証状態を格納//
	FFWRX_AUTH_BEFORE=0,				// デバッガ未認証//
	FFWRX_AUTH_ALREADY,					// デバッガ認証済み//
};
enum FFWRX_STAT_DMB {				// デバッグDMAデータ転送状態を格納//
	FFWRX_DDMA_NORMAL=0,				// デバッグDMAデータ転送中でない//
	FFWRX_DDMA_BUSY,					// デバッグDMAデータ転送中である//
};
enum FFWRX_STAT_EXEC {				// ターゲットプログラム実行状態を格納//
	FFWRX_STAT_EXEC_STOP=0,				// ターゲットプログラム停止中//
	FFWRX_STAT_EXEC_RUN					// ターゲットプログラム実行中//
};
enum FFWRX_STAT_AUTHSET {			// デバッガ認証の要否を格納//
	FFWRX_STAT_AUTH_NONSET=0,			// デバッガ認証不要//
	FFWRX_STAT_AUTH_SET					// デバッガ認証必要//
};
enum FFWRX_STAT_PMODE {				// プロセッサモード//
	FFWRX_STAT_PMODE_SNG=0,				// シングルチップモード//
	FFWRX_STAT_PMODE_SNG1,				// シングルチップモード//
	FFWRX_STAT_PMODE_EXMEMDIS,			// 内蔵ROM無効拡張モード//
	FFWRX_STAT_PMODE_EXMEMENA			// 内蔵ROM有効拡張モード//
};

// HPON
enum FFWENM_HPON_CONNECT {		// ターゲットシステムとの接続/切断指定//
	HPON_COLD_CON=0,				// コールドスタート接続//
	HPON_HOT_CON,					// ホットスタート接続//
	HPON_NCON_RESETKEEP,			// 切断(リセット保持、電源供給OFF, EMLE←LOW)//
	HPON_NCON_RESETKEEP2,			// 切断(リセット保持、電源供給OFF実行)//
	HPON_NCON_RESETOFF				// 切断(リセット解除、電源供給OFF実行)//
};

// MSTSM
enum FFWRXENM_MSTS_MASK {			// ターゲットMCU端子のマスク情報//
	RX_MSTSM_OFF=0,						// マスクしない//
	RX_MSTSM_ON							// マスクする//
};
// INSTCODE
enum FFWENM_BRANCH_TYPE {			// 分岐命令種別//
	NOT_BRANCH=0,						// 分岐命令以外//
	BRANCH_NOCOND,						// 無条件分岐:BRA,JMP
	BRANCH_COND,						// 条件分岐:BCnd
	BRANCH_SUB1,						// サブルーチン分岐１:BSR,JSR
	BRANCH_SUB2,						// サブルーチン分岐２:RTS,RTSD
	BRANCH_BRK,							// 割り込み/例外:DBT,INT,BRK,RTE,RTFI
};
// DBG
enum FWENM_ARGERRCHK {		// OCD関連の組合せ制限エラー検出有無//
	OCD_ARGERRCHK_NOUSE = 0,	// 組合せ制限エラー検出なし//
	OCD_ARGERRCHK_USE,			// 組合せ制限エラー検出あり//
};
enum FFWENM_MCU_MULTI {		// FFW-BFW間通信プロトコルにコア指定のパラメータ付加有無//
	EML_MCU_MULTI_NON = 0,		// コア指定のパラメータを付加しない//
	EML_MCU_MULTI_USE,			// コア指定のパラメータを付加する//
};
enum FFWENM_TRCFUNCMODE {	// トレースの利用機能を指定//
	EML_TRCFUNC_TRC = 0,		// トレース用に利用//
	EML_TRCFUNC_CV,				// カバレッジ用に利用//
};
enum FFWENM_ROMCORRECT {	// ROMコレクションの利用を指定//
	EML_ROM_CORRECT_NON = 0,	// ROMコレクション機能未使用//
	EML_ROM_CORRECT_USE,		// ROMコレクション機能使用//
};
enum FFWENM_WRITER {		// 通常モード/ライタモードを指定//
	EML_WRITERMODE_NON = 0,		// 通常モード//
	EML_WRITERMODE_USE,			// ライタモード//
};
enum FFWENM_USEPROGRAM {	// 使用するプログラムを指定//
	EML_HEW = 0,				// E1/E20エミュレータデバッガ//
	EML_FDT,					// FDT
	EML_RXSTICK,				// RX-Stick
	EML_MONITOR,				// シリアルモニタ//
	// RevRxNo120910-001 Append Line//
	EML_EZ_CUBE,				// EZ-CUBEエミュレータデバッガ//
};
enum FFWENM_FLASHWRITE {	// Flash書き換えデバッグを指定//
	EML_DBG_FLASHWRITE_NON = 0,	// Flash書き換えデバッグしない//
	EML_DBG_FLASHWRITE_USE,		// Flash書き換えデバッグする//
};
enum FFWENM_DATA_FLASHWRITE {	// Flash書き換えデバッグを指定//
	EML_DBG_DATA_FLASHWRITE_NON = 0,	// Flash書き換えデバッグしない//
	EML_DBG_DATA_FLASHWRITE_USE,		// Flash書き換えデバッグする//
};
enum FFWENM_RUNREGSET {		// プログラム実行後設定の有無を指定//
	EML_RUNREGSET_NON = 0,		// 設定しない//
	EML_RUNREGSET_USE,			// 設定する//
};

enum FFWENM_OCDREGACSMODE {	// 制御レジスタへのアクセスモードを指定//
	OCD_ACSMODE_NONDUMPFILL = 0,	// DUMP/FILLともにアクセス不可//
	OCD_ACSMODE_FILL,				// FILLのみアクセス可//
	OCD_ACSMODE_DUMPFILL,			// DUMP/FILLともにアクセス可//
	OCD_ACSMODE_DUMP,				// DUMPのみアクセス可//
};
enum FFWENM_SCI_IF {		// 同期/非同期通信を指定//
	SCI_NON=0,					// 指定なし//
	SCI_SCI,					// 同期通信//
	SCI_UART					// 非同期通信//
};
enum FFWRXENM_CLKCHANGE_ENA {	// ダウンロード時のクロック切り替えを指定//
	RX_CLKCHANGE_DIS=0,			// クロック切り替え禁止//
	RX_CLKCHANGE_ENA			// クロック切り替え許可//
};
// RevRxNo120910-001 Append Start//
enum FFWRXENM_BOOTSWAP {		// ブートスワップ機能を使用するプログラムをデバッグする/しないを指定//
	RX_DBG_BOOTSWAP_NON=0,		// ブートスワップ機能を使用したプログラムをデバッグしない//
	RX_DBG_BOOTSWAP_USE			// ブートスワップ機能を使用したプログラムをデバッグする//
};
// RevRxNo120910-001 Append End//
//MCU
enum FFWENM_ACC_SET {		// アキュムレータの有無//
	ACC_SET_OFF=0,				// アキュムレータなし//
	ACC_SET_ON					// アキュムレータあり//
};

enum FFWENM_COPRO_SET {		// コプロセッサの有無//
	COPRO_SET_OFF=0,			// コプロセッサなし//
	COPRO_SET_ON				// コプロセッサあり//
};
// RevRxNo120910-001 Append Start//
enum FFWRXENM_EMU_ACCESS_TYPE {	// 個別アクセス制御領域 アクセス属性//
	RX_EMU_ACCTYPE_RW=0,			// リード/ライト可//
	RX_EMU_ACCTYPE_R,				// リードのみ可//
	RX_EMU_ACCTYPE_W,				// ライトのみ可//
	RX_EMU_ACCTYPE_NON,				// アクセス不可//
};
enum FFWRXENM_EMU_ACCESS_MEANS {	// 個別アクセス制御領域 アクセス手段//
	RX_EMU_ACCMEANS_DDMA=0,				// デバッグDMAアクセス//
	RX_EMU_ACCMEANS_CPU,				// CPUアクセス//
};
// RevRxNo120910-001 Append End//
// RevRxNo130301-001 Append Start//
enum FFWRXENM_CPU_ISA {		// 命令セットアーキテクチャ//
	RX_ISA_RXV1=0,				// RX V1アーキテクチャ//
	RX_ISA_RXV2,				// RX V2アーキテクチャ//
	// RevRxNo180228-001 Append Line//
	RX_ISA_RXV3,				// RX V3アーキテクチャ//
};
enum FFWRXENM_FPU_SET {		// FPUの有無//
	RX_FPU_OFF=0,				// 単精度FPUなし,倍精度FPUなし//
	RX_FPU_ON,					// 単精度FPUあり,倍精度FPUなし//
	// RevRxMultiNo180302-001 Append Line//
	RX_DPFPU_ON,				// 単精度FPUあり,倍精度FPUあり//
};
// RevRxNo130301-001 Append End//

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
//WTR
enum FFWRXENM_WTRTYPE {		// 書き込みプログラムの種別//
	WTRTYPE_0=0,				// ダウンロード用//
	WTRTYPE_1,					// 予約(ブランクチェック用)//
};
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

// JTAGCLK
enum FFWENM_JTAG_TCLK {
	TCLK_50_0MHZ=0,
	TCLK_25_0MHZ,
	TCLK_12_5MHZ,
	TCLK_6_25MHZ,
	TCLK_3_125MHZ,
	TCLK_1_562MHZ,
	TCLK_0_098MHZ,
	TCLK_0_0015MHZ,
	TCLK_66_0MHZ,
	TCLK_33_0MHZ,
	TCLK_16_6MHZ,
	// RevRxE2No170201-001 Append Start
	TCLK_18_181MHZ,
	TCLK_20_0MHZ,
	TCLK_22_222MHZ
	// RevRxE2No170201-001 Append End
};

//---------------------------
// イベント関連コマンド//
//---------------------------

// 2008.11.12 INSERT_BEGIN_E20RX600(+NN) {
enum FFWRX_EV {					// イベント種別//
	FFWRX_PE0=0x00,
	FFWRX_PE1,
	FFWRX_PE2,
	FFWRX_PE3,
	FFWRX_PE4,
	FFWRX_PE5,
	FFWRX_PE6,
	FFWRX_PE7,
	FFWRX_DE0=0x10,
	FFWRX_DE1,
	FFWRX_DE2,
	FFWRX_DE3,
};
enum FFWRX_EV_MODE {		// EV モード//
	EV_MODE_ADDR = 0,		// 命令実行//
	EV_MODE_OPC_RANGE,		// オペランドアクセス(範囲指定)//
	EV_MODE_OPC_MASK,		// オペランドアクセス(マスク設定)//
};

enum FFWRX_EV_ACCESS {	// EV アクセス属性//
	EV_ACCESS_NON = 0,		// 比較なし//
	EV_ACCESS_R,			// リード//
	EV_ACCESS_W,			// ライト//
	EV_ACCESS_RW,			// リードライト//
};

enum FFWRX_EV_DATASIZE {	// EV データサイズ//
	EV_DATASIZE_NON = 0,	// サイズ指定なし//
	EV_DATASIZE_BYTE,		// バイト(1byte)//
	EV_DATASIZE_WORD,		// ワード(2byte)//
	EV_DATASIZE_LWORD,		// ロングワード(4byte)//
	// RevRxMultiNo180302-001 Append Start//
	EV_DATASIZE_DLWORD_L,		// ダブルロングワード(8byte)下位側//
	EV_DATASIZE_DLWORD_H,		// ダブルロングワード(8byte)上位側//
	// RevRxMultiNo180302-001 Append End//
};

enum FFWRX_EV_CMP {		// EV 比較条件//
	EV_CMPMASK_AGREE = 0,	// マスク付き一致//
	EV_CMPMASK_DISAGREE,	// マスク付き不一致//
	EV_CMPRANGE_IN,			// 範囲内//
	EV_CMPRANGE_OUT,		// 範囲外//
};

enum FFWRX_EV_CNTSEL {	// EVCNT 選択モード//
	EVCNT_NON = 0,			// 接続しない//
	EVCNT_ADDR,				// 命令実行//
	EVCNT_OPC,				// オペランドアクセス//
};

enum FFWRX_COMB_PATTERN {	// イベント組合せ条件//
	COMB_OR = 0,				// OR組合せ(ブレーク,トレースともに)//
	COMB_BRK_AND,				// 累積ANDブレーク(トレースはOR組合せ)//
	COMB_BRK_STATE,				// 状態遷移ブレーク(トレースはOR組合せ)//
	COMB_TRC_AND,				// 累積ANDトレース(ブレークはOR組合せ)//
	COMB_TRC_STATE,				// 状態遷移トレース(ブレークはOR組合せ)//
	// RevRxMultiNo180302-001 Append Start//
	COMB_BRK_64,				// 64ビットOAデータ比較組み合わせブレーク(トレースはOR組み合わせ)//
	COMB_TRC_64,				// 64ビットOAデータトレース開始(ブレークはOR組合せ)//
	// RevRxMultiNo180302-001 Append End//
};

enum FFWRX_EVEBRKTSC {	// イベント組合せ条件レジスタ設定値//
	BRKTSC_OR = 0,				// OR組合せ(ブレーク,トレースともに)//
	BRKTSC_BRK_AND,				// 累積ANDブレーク(トレースはOR組合せ)//
	BRKTSC_BRK_STATE,			// 状態遷移ブレーク(トレースはOR組合せ)//
	BRKTSC_BRK_64,				// 64ビットOAデータ比較組み合わせブレーク(トレースはOR組み合わせ)//	// RevRxNo180713-001 Modify Line//
	BRKTSC_DUMMY2,
	BRKTSC_TRC_AND,				// 累積ANDトレース(ブレークはOR組合せ)//
	BRKTSC_TRC_STATE,			// 状態遷移トレース(ブレークはOR組合せ)//
	BRKTSC_TRC_64				// 64ビットOAデータトレース開始(ブレークはOR組合せ)//	// RevRxNo180713-001 Append Line//
};

enum FFWRX_EVCOMBI_PPC_USR_CTRL {		// パフォーマンス計測資源に対するアクセスコントロール設定//
	FFWRX_EVCOMBI_PPC_USR = 0,		// ユーザプログラムで使用//
	FFWRX_EVCOMBI_PPC_EML,			// エミュレータで使用//
};
// 2008.11.12 INSERT_END_E20RX600 }


//---------------------------
// トレース関連コマンド//
//---------------------------
// RM
enum FFWENM_RM_INFOKIND {		// トレース情報種別//
	RM_LOST=0,						// ロスト情報//
	RM_STACK,						// スタックポインタ情報//
	RM_BRCND,						// 分岐成立可否情報//
	RM_DEST,						// 分岐先情報//
	RM_SRC,							// 分岐元情報//
	RM_SRCDEST,						// 分岐元,分岐先情報//
	RM_OPR,							// オペランドアクセス情報//
	RM_HWBR,						// 分岐情報(ハードウェアデバッグモード)//
	RM_STDBY						// スタンバイ情報//
};
enum FFWRX_RM_TRCMODE {			// トレースモード設定//
	MODE1 = 0,						// モード1//
	MODE2,							// モード2//
	MODE3,							// モード3//
	MODE4,							// モード4//
	MODE5,							// モード5//
	MODE6,							// モード6//
	MODE7,							// モード7//
	MODE8,							// モード8//
	MODE9,							// モード9//
	MODE10							// モード10//
};
enum FFWENM_EMLTRCMEMSIZ {		// トレースメモリサイズ指定//
	EML_TRCMEM_64M = 0,				// 64M×16bit (128Mbyte)//
	EML_TRCMEM_32M,					// 32M×16bit (64Mbyte)//
	EML_TRCMEM_16M,					// 16M×16bit (32Mbyte)//
	EML_TRCMEM_08M,					// 8M×16bit (16Mbyte)//
	EML_TRCMEM_04M,					// 4M×16bit (8Mbyte)//
	EML_TRCMEM_02M,					// 2M×16bit (4Mbyte)//
	EML_TRCMEM_01M,					// 1M×16bit (2Mbyte)//
	EML_TRCMEM_512K,				// 512K×16bit (1Mbyte)//
};
enum FFWENM_TRSTP {				// トレース入力の停止/再開を指定//
	TRSTP_RESTART = 0x00,			// トレース入力を再開//
	TRSTP_STOP_KEEP,				// トレース有効/無効状態は保持したまま、トレース入力を停止//
	TRSTP_STOP_CLR = 0x3,			// トレース有効/無効状態はクリア(トレース無効)し、トレース入力を停止//
};

enum FFWENM_TRCCLK_EDGE {		// トレースデータ端子出力エッジ//
	TRC_EDGE_SINGLE=0,				// シングルエッジ出力//
	TRC_EDGE_DOUBLE					// ダブルエッジ出力//
};

enum FFWENM_TRCCLK_RATIO {		// トレースクロック比//
	TRC_RATIO_1_1=0,				// クロック比 1:1//
	TRC_RATIO_2_1,					// クロック比 2:1//
	TRC_RATIO_4_1					// クロック比 4:1//
};

enum FFWENM_TRCCLK_EXTAL {		// トレースクロック動作周波数//
	TRC_EXTAL_X4=0,					// ExTAL×4//
	TRC_EXTAL_X2,					// ExTAL×2//
	TRC_EXTAL_X1,					// ExTAL×1//
	TRC_EXTAL_X1_2,					// ExTAL×1/2//
	TRC_EXTAL_X1_4,					// ExTAL×1/4//
	TRC_EXTAL_X1_8,					// ExTAL×1/8//
	// RevRxNo120910-001 Modify Start//
	TRC_EXTAL_DIS,					// ExTAL×1(EXTAL使わない) for RX63x//
	TRC_EXTAL_ENA					// ExTAL×1/2(EXTAL使う) for RX63x//
	// RevRxNo120910-001 Modify End//
};

//---------------------------
// 時間測定関連コマンド//
//---------------------------

// RevRxNo130308-001 Append Start
//---------------------------
// カバレッジ計測関連コマンド//
//---------------------------
enum FFWRXENM_CVB_ENABLE {			// カバレッジの動作禁止/許可 //
	RX_CVB_DIS=0,						// 動作禁止 //
	RX_CVB_ENA,							// 動作許可 //
};

enum FFWRXENM_CV_BLKNO {			// C0カバレッジデータを取得するブロック番号 //
	RX_BLKNO_0=0,						// ブロック0 //
	RX_BLKNO_1,							// ブロック1 //
	RX_BLKNO_2,							// ブロック2 //
	RX_BLKNO_3,							// ブロック3 //
};
// RevRxNo130308-001 Append End

/////////// 構造体定義 ///////////
//---------------------------
// MCU依存コマンド//
//---------------------------
// MCU空間情報格納構造体//
typedef struct {
	DWORD	dwSfrAreaNum;									// MCUのSFRブロック総数//
	DWORD	dwmadrSfrStartAddr[MCU_AREANUM_MAX_RX];			// MCUのSFRブロック先頭アドレス//
	DWORD	dwmadrSfrEndAddr[MCU_AREANUM_MAX_RX];			// MCUのSFRブロック終了アドレス//
	BYTE	bySfrEndian[MCU_AREANUM_MAX_RX];				// MCUのSFRブロックエンディアン//
	DWORD	dwRamAreaNum;									// MCUの内部RAMブロック総数//
	DWORD	dwmadrRamStartAddr[MCU_AREANUM_MAX_RX];			// MCUの内部RAMブロック先頭アドレス//
	DWORD	dwmadrRamEndAddr[MCU_AREANUM_MAX_RX];			// MCUの内部RAMブロック終了アドレス//
	DWORD	dwmadrFlashRomWriteStart;						// MCU内部FlashROM書き込み先頭アドレス//
	DWORD	dwmadrFlashRomReadStart;						// MCU内部FlashROM読み込み先頭アドレス//
	
	DWORD	dwFlashRomPatternNum;							// MCU内部FlashROM領域パターン数//
	DWORD	dwmadrFlashRomStart[MCU_AREAPTNUM_MAX];			// MCU内部FlashROM領域パターン開始アドレス//
	DWORD	dwFlashRomBlockSize[MCU_AREAPTNUM_MAX];			// MCU内部FlashROMブロック先頭アドレス//
	DWORD	dwFlashRomBlockNum[MCU_AREAPTNUM_MAX];			// MCU内部FlashROMブロック数//
	DWORD	dwDataFlashRomPatternNum;						// MCU内部Data(E2)FlashROM領域パターン数//
	DWORD	dwmadrDataFlashRomStart[MCU_AREAPTNUM_MAX];		// MCU内部Data(E2)FlashROM領域パターン開始アドレス//
	DWORD	dwDataFlashRomBlockSize[MCU_AREAPTNUM_MAX];		// MCU内部Data(E2)FlashROMブロック先頭アドレス//
	DWORD	dwDataFlashRomBlockNum[MCU_AREAPTNUM_MAX];		// MCU内部Data(E2)FlashROMブロック数//
	
	DWORD	dwExtMemBlockNum;								// MCUの外部メモリエリア総数//
	DWORD	dwmadrExtMemBlockStart[MCU_AREANUM_MAX_RX];		// MCUの外部メモリブロック先頭アドレス //
	DWORD	dwmadrExtMemBlockEnd[MCU_AREANUM_MAX_RX];		// MCUの外部メモリブロック終了アドレス//
	enum FFWENM_MACCESS_SIZE	eExtMemCondAccess[MCU_AREANUM_MAX_RX];			// MCUの外部メモリブロックアクセスサイズ //
	BYTE	byExtMemEndian[MCU_AREANUM_MAX_RX];				// MCUの外部メモリブロックエンディアン//
	// RevRxNo120910-001 Append Line//
	BYTE	byExtMemType[MCU_AREANUM_MAX_RX];				// MCUの外部メモリブロック種別//
	DWORD	dwAdrFcuFirmStart;								// FCUファームウェア格納アドレス//
	// RevRxNo120910-001 Modify Line//
	DWORD	dwFcuFirmLen;									// FCUファームウェアサイズ//
	DWORD	dwAdrFcuRamStart;								// FCU-RAMアドレス//
// V.1.02 No.5 ユーザブート/USBブート対応 Append Start
	DWORD	dwmadrUserBootStart;							// ユーザブート領域先頭アドレス//
	DWORD	dwmadrUserBootEnd;								// ユーザブート領域終了アドレス//
// V.1.02 No.5 ユーザブート/USBブート対応 Append End
} FFWRX_MCUAREA_DATA;

// エミュレータ内部制御に必要なMCU情報格納構造体//
typedef struct {
	DWORD	dwadrIspOnReset;		// リセット時のISPレジスタ初期値//
	DWORD	dwadrWorkRam;			// Flash書き込みプログラム格納アドレス//
	DWORD	dwsizWorkRam;			// Flash書き込みプログラム格納サイズ//
	enum FFWENM_TRCCLK_EDGE		eEdge;					// トレースデータ端子出力エッジ//
	enum FFWENM_TRCCLK_RATIO	eRatio;					// トレースクロック比//
	enum FFWENM_TRCCLK_EXTAL	eExTAL;					// トレースクロック動作周波数//
	enum FFWENM_ACC_SET			eAccSet;				// アキュムレータ有無//
	enum FFWENM_COPRO_SET		eCoProSet;				// コプロセッサ有無//
	DWORD	dwSpc[MCU_SPC_MAX];		// SPC変数//
	float	fClkExtal;				// 入力クロック(EXTAL)//
	DWORD	dwmadrIdcodeStart;		// IDコード格納開始アドレス//
	BYTE	byIdcodeSize;			// IDコード格納サイズ//
	// RevRxNo120910-001 Modify Start//
	DWORD	dwMcuJtagIdcodeNum;		// 対象デバイスJTAG IDコード設定数//
	DWORD	dwMcuJtagIdcode[MCU_OCD_ID_MAX];	// 対象デバイスJTAG IDコード//
	// RevRxNo120910-001 Modify End//
	DWORD	dwMcuDeviceCode;		// 対象デバイス デバイスコード//
	
	WORD	wMcuFineMakerCode;		// 対象デバイスFINE メーカコード//
	WORD	wMcuFineFamilyCode;		// 対象デバイスFINE ファミリコード//
	// RevRxNo120910-001 Modify Start//
	DWORD	dwMcuFineDeviceCodeNum;	// 対象デバイスFINE デバイスコード設定数//
	WORD	wMcuFineDeviceCode[MCU_OCD_ID_MAX];	// 対象デバイスFINE デバイスコード//
	// RevRxNo120910-001 Modify End//
	WORD	wMcuFineVersionCode;	// 対象デバイスFINE バージョンコード//
	
	BYTE	byBaudRateSelectNum;							// 選択可能な通信ボーレートの個数//
	DWORD	dwBaudRateVal[MCU_BRTABLE_NUM_MAX];				// 通信ボーレートテーブル(ボーレート即値)//
	BYTE	byModeEntryPtNum;								// モードエントリパターンのデータ個数//
	WORD	wModeEntryPt[MCU_MDENR_PTNNUM_MAX];				// モードエントリパターン//
	WORD	wSendInterValTimeME;							// モードエントリ時の1バイト送信間隔(usec)//
	WORD  	wSendIntervalTimeNORMAL;						// 通常デバッグ時の1バイト送信間隔(usec)//
	WORD	wRcvToSendIntervalTime;							// 通信データ受信完了から送信までの間隔(usec)//

	BYTE	byMcuType;										// MCU種別//
	BYTE	byInterfaceType;								// I/F種別//

// V.1.02 No.12 Class2相当イベント機能の対応 Append Start
	WORD	wPCEvent;										// PCイベント最大数//
	WORD	wOAEvent;										// OAイベント最大数//
	WORD	wCombination;									// AND/OR組み合わせイベント最大数//
	WORD	wPPC;											// PPCチャンネル数//
// V.1.02 No.12 Class2相当イベント機能の対応 Append End
	// RevRxNo120910-001 Append Line//
	WORD	wOcdTrcCycMax;									// OCD内蔵トレース最大サイクル数//
	// RevRxNo120910-001 Append Line//
	float	fTgtPwrVal;										// ユーザ基板の電源電圧//

	DWORD	dwMcuRegInfoBlkNum;													// MCUレジスタ情報定義ブロック数 //
	DWORD	dwMcuRegInfoNum[MCU_REGBLKNUM_MAX];									// MCUレジスタ情報定義数 //
	DWORD	dwmadrMcuRegInfoAddr[MCU_REGBLKNUM_MAX][MCU_REGADDRNUM_MAX];		// MCUレジスタ定義用アドレス //
	enum FFWENM_MACCESS_SIZE	eMcuRegInfoAccessSize[MCU_REGBLKNUM_MAX][MCU_REGADDRNUM_MAX];	// MCUレジスタアクセスサイズ //
	DWORD	dwMcuRegInfoMask[MCU_REGBLKNUM_MAX][MCU_REGADDRNUM_MAX];			// MCUレジスタ定義用マスクデータ //
	DWORD	dwMcuRegInfoData[MCU_REGBLKNUM_MAX][MCU_REGADDRNUM_MAX];			// MCUレジスタ定義用設定データ //
	
	// RevRxNo120910-001 Append Start//
	DWORD	dwVdet0LevelNum;								// 電圧検出0レベル設定数 //
	DWORD	dwVdet0LevelSelMask;							// OFS1レジスタ 電圧検出0レベル選択ビット マスクデータ //
	DWORD	dwVdet0LevelSelData[MCU_VDET0LEVELNUM_MAX];		// OFS1レジスタ 電圧検出0レベル選択値 //
	DWORD	dwVdet0Level[MCU_VDET0LEVELNUM_MAX];			// 電圧検出0レベル //
	// RevRxNo120910-001 Append End//
	// RevRxNo120910-001 Append Start//
	DWORD	dwEmuAccCtrlAreaNum;												// 個別アクセス制御領域数 //
	DWORD	dwmadrEmuAccCtrlStart[MCU_ACCCTRL_AREANUM_MAX];						// 個別アクセス制御領域 開始アドレス //
	DWORD	dwmadrEmuAccCtrlEnd[MCU_ACCCTRL_AREANUM_MAX];						// 個別アクセス制御領域 終了アドレス //
	enum FFWRXENM_EMU_ACCESS_TYPE	eEmuAccType[MCU_ACCCTRL_AREANUM_MAX];		// 個別アクセス制御領域 アクセス属性 //
	enum FFWRXENM_EMU_ACCESS_MEANS	eEmuAccMeans[MCU_ACCCTRL_AREANUM_MAX];		// 個別アクセス制御領域 アクセス手段 //
	// RevRxNo120910-001 Append End//

	// RevRxNo130301-001 Append Start//
	enum FFWRXENM_CPU_ISA	eCpuIsa;	// 命令セットアーキテクチャ //
	enum FFWRXENM_FPU_SET	eFpuSet;	// FPUの有無 //
	// RevRxNo130301-001 Append End//

	// RevRxNo140515-005 Append Line
	float fClkIclk;						// ユーザのICLK動作周波数
} FFWRX_MCUINFO_DATA;

// デバッグ情報格納構造体//
typedef struct {
	enum FWENM_ARGERRCHK		eOcdArgErrChk;			// OCD関連の組合せ制限エラー検出有無//
	enum FFWENM_MCU_MULTI		eMcuMultiCore;			// FFW-BFW間通信プロトコルにコア指定のパラメータ付加有無//
	enum FFWENM_TRCFUNCMODE		eTrcFuncMode;			// トレースの利用機能を指定//
	enum FFWENM_ROMCORRECT		eRomCorrection;			// ROMコレクションの利用を指定//
	enum FFWENM_WRITER			eWriter;				// 通常モード/ライタモードを指定//
	DWORD						dwJtagByPass;			// JTAGカスケード接続時のバイパス情報を指定//
	enum FFWENM_USEPROGRAM		eUseProgram;			// 使用するプログラムを指定//
	enum FFWENM_FLASHWRITE		eDbgFlashWrite;			// Flash書き換えデバッグを指定//
	enum FFWENM_FLASHWRITE		eDbgDataFlashWrite;		// DataFlash書き換えデバッグを指定//
	enum FFWENM_RUNREGSET		eBfrRunRegSet;			// プログラム実行前設定の有無を指定//
	enum FFWENM_RUNREGSET		eAftRunRegSet;			// プログラム実行後設定の有無を指定//
	enum FFWENM_OCDREGACSMODE	eOcdRegAcsMode;			// 制御レジスタへのアクセスモードを指定//
	enum FFWENM_SCI_IF			eSciIF;					// 

	enum FFWRXENM_CLKCHANGE_ENA	eClkChangeEna;			// ダウンロード時のクロック切り替え指定//

	// RevRxNo120910-001 Append Line//
	enum FFWRXENM_BOOTSWAP		eDbgBootSwap;			// ブートスワッププログラムのデバッグ指定//

	DWORD	dwDbgRegInfoBlkNum;							// MCUレジスタ情報定義ブロック数 //
	DWORD	dwDbgRegInfoNum[DBG_REGBLKNUM_MAX];			// MCUレジスタ情報定義数 //
	DWORD	dwmadrDbgRegInfoAddr[DBG_REGBLKNUM_MAX][DBG_REGADDRNUM_MAX];	// MCUレジスタ定義用アドレス //
	enum FFWENM_MACCESS_SIZE	eDbgRegInfoAccessSize[DBG_REGBLKNUM_MAX][DBG_REGADDRNUM_MAX];	// MCUレジスタアクセスサイズ //
	DWORD	dwDbgRegInfoMask[DBG_REGBLKNUM_MAX][DBG_REGADDRNUM_MAX];		// MCUレジスタ定義用マスクデータ //
	DWORD	dwDbgRegInfoData[DBG_REGBLKNUM_MAX][DBG_REGADDRNUM_MAX];		// MCUレジスタ定義用設定データ //
	DWORD	dwDbgAreaBlkNum;							// MCUアドレス範囲定義ブロック数 //
	DWORD	dwDbgAreaNum[DBG_AREABLKNUM_MAX];			// MCUアドレス範囲定義数 //
	DWORD	dwmadrDbgAreaStartAddr[DBG_AREABLKNUM_MAX][DBG_AREAADDRNUM_MAX];// MCUアドレス範囲開始アドレス //
	DWORD	dwmadrDbgAreaEndAddr[DBG_AREABLKNUM_MAX][DBG_AREAADDRNUM_MAX];	// MCUアドレス範囲終了アドレス //

} FFWRX_DBG_DATA;

typedef struct {
	DWORD dwPcInfo;							// プログラムカウンタの内容//
	DWORD dwPswInfo;						// プロセッサステータスワードの内容//
	WORD  wC2eBuffSize;						// C2Eバッファ情報//
	WORD  wE2cBuffSize;						// E2Cバッファ情報//
	DWORD dwExecInfo;						// ターゲットプログラム実行状態//
	DWORD dwEmlMesInfo;						// 全実行時間計測カウンタの内容//
} FFWRX_PROG_DATA;

typedef struct {
	enum FFWRX_STAT_ENDIAN	eEndian;		// エンディアン指定状況を格納//
	BYTE					byStatModePin;	// MODE端子(MD0/1)状態を格納(モニタレジスタを参照)(RX610/RX62x用)//
	BYTE					byStatMdPin;	// MD端子状態を格納(RX63x/RX210用)//
	BYTE					byStatProcMode;	// プロセッサモード状態を格納//
	DWORD					dwPerfInfo;		// パフォーマンス計測動作//
	DWORD					dwAndStateInfo;	// OR/累積AND/状態遷移成立状態//
} FFWRX_STAT_SFR_DATA;

typedef struct {
	enum FFWRX_STAT_BAN		eStatBan;		// デバッガ接続可否情報を格納//
	enum FFWRX_STAT_AUTH	eStatAuth;		// デバッガ認証状態を格納//
	enum FFWRX_STAT_DMB		eStatdDmaBusy;	// デバッグDMAデータ転送状態を格納//
	enum FFWRX_STAT_EXEC	eStatExec;		// ターゲットプログラム実行状態を格納//
	enum FFWRX_STAT_AUTHSET	eStatAuthSet;	// デバッガ認証の要否を格納//
	DWORD					dwJtagIdCode;	// JTAGのIDコードを格納//
} FFWRX_STAT_JTAG_DATA;

typedef struct {
	enum FFWRX_STAT_BAN		eStatBan;		// デバッガ接続可否情報を格納//
	enum FFWRX_STAT_AUTH	eStatAuth;		// デバッガ認証状態を格納//
	enum FFWRX_STAT_DMB		eStatdDmaBusy;	// デバッグDMAデータ転送状態を格納//
	enum FFWRX_STAT_EXEC	eStatExec;		// ターゲットプログラム実行状態を格納//
	enum FFWRX_STAT_AUTHSET	eStatAuthSet;	// デバッガ認証の要否を格納//
	DWORD					dwFineMakerCode;	// FINEのメーカコードを格納//
	DWORD					dwFineFamilyCode;	// FINEのファミリコードを格納//
	DWORD					dwFineDeviceCode;	// FINEのデバイスコードを格納//
} FFWRX_STAT_FINE_DATA;

typedef struct {
	enum FFWE20_STAT_UVCC	eStatUVCC;		// ユーザシステム電源状態を格納//
	// RevRxNo120910-001 Append Line//
	float					fNowUVCC;		// ユーザシステム電圧値を格納//
	enum FFWE20_STAT_UCON	eStatUCON;		// ユーザシステム接続状態を格納//
	float					flTrcClkData;	// トレースクロック実測値を格納//
	DWORD					dwTraceInfo;	// トレース計測動作//
	// RevRxNo130308-001 Append Line//
	DWORD					dwCoverageInfo;	// カバレッジ計測動作//
} FFWRX_STAT_EML_DATA;

// RevRxNo120910-001 Append Start//
typedef struct {
	enum FFWRXENM_STAT_MCU*	peStatMcu;			// ターゲットMCUのステータス情報//
	DWORD					dwSizeOfSfrStruct;	// SFR 情報を格納するFFWRX_STAT_SFR_DATA 構造体サイズを格納//
	FFWRX_STAT_SFR_DATA*	pStatSFR;			// SFR 情報を格納//
	DWORD					dwSizeOfJtagStruct;	// JTAG 情報を格納するFFWRX_STAT_JTAG_DATA 構造体サイズを格納//
	FFWRX_STAT_JTAG_DATA*	pStatJTAG;			// JTAG 情報を格納//
	DWORD					dwSizeOfFineStruct;	// FINE 情報を格納するFFWRX_STAT_FINE_DATA 構造体サイズを格納//
	FFWRX_STAT_FINE_DATA*	pStatFINE;			// FINE 情報を格納//
	DWORD					dwSizeOfEmlStruct;	// エミュレータ情報を格納するFFWRX_STAT_EML_DATA 構造体サイズを格納//
	FFWRX_STAT_EML_DATA*	pStatEML;			// エミュレータ情報を格納//
} FFWRX_STAT_DATA;
// RevRxNo120910-001 Append End//

//コマンド種別ビットフィールド//
typedef struct {
	BYTE				byCmdLen;						//命令バイト長//
	FFWENM_BRANCH_TYPE	branch_type;					//分岐命令種別//
	BYTE				isSubroutineJump;				//サブルーチンジャンプ命令か//
														//（分岐命令：サブルーチン分岐１かどうか）//
	BYTE				isBranch;						//分岐命令//
	BYTE				isRegUpdate;					//レジスタ更新命令//
	BYTE				isMemUpdate;					//メモリ更新命令//
	BYTE				isPswUpdate;					//(1)PSWを直接更新する命令:CLRPSW,SETPSW
	BYTE				isWait;							//(2)WAIT命令:WAIT
	BYTE				isMisc;							//(予約)//
	BYTE				isValidAddr;					//有効な分岐先アドレスが入っているか否か//
	DWORD				dwBranchAddr;					//分岐先アドレス//
	WORD				wUpdateRegLlist;				//更新レジスタリスト（未実装）//
} FFWRX_INST_DATA;
// V.1.02 No.3 起動時のエンディアンおよびデバッグ継続モード設定 Append Start
// PMOD //
enum FFWRXENM_PMODE {				// 端子設定MCU動作モード //
	RX_PMODE_SNG=0,						// シングルチップモード //
	RX_PMODE_ROME,						// 内蔵ROM有効拡張モード //
	RX_PMODE_ROMD8,						// 内蔵ROM無効拡張モード(外部バス8ビット) //
	RX_PMODE_ROMD16,					// 内蔵ROM無効拡張モード(外部バス16ビット) //
	RX_PMODE_USRBOOT,					// ユーザブートモード //
	RX_PMODE_USBBOOT					// USBブートモード //
};
// V.1.02 No.3 起動時のエンディアンおよびデバッグ継続モード設定 Append End

// V.1.02 No.5 ユーザブート/USBブート対応 Append Start
enum FFWRXENM_RMODE {				// レジスタ設定MCU動作モード //
	RX_RMODE_SNG=0,						// シングルチップモード //
	RX_RMODE_ROME,						// 内蔵ROM有効拡張モード //
	RX_RMODE_ROMD,						// 内蔵ROM無効拡張モード //
// RevRXNo161003-001 Append Start
	RX_RMODE_SNG_DUAL_BANK0,			// シングルチップモード&デュアルモード&起動バンク0 //
	RX_RMODE_SNG_DUAL_BANK1,			// シングルチップモード&デュアルモード&起動バンク1 //
	RX_RMODE_SNG_DUAL,					// シングルチップモード&デュアルモード&起動バンク継続 //
	RX_RMODE_ROME_DUAL_BANK0,			// ROM有効拡張モード&デュアルモード&起動バンク0 //
	RX_RMODE_ROME_DUAL_BANK1,			// ROM有効拡張モード&デュアルモード&起動バンク1 //
	RX_RMODE_ROME_DUAL,					// ROM有効拡張モード&デュアルモード&起動バンク継続 //
	RX_RMODE_ROMD_DUAL_BANK0,			// ROM無効拡張モード&デュアルモード&起動バンク0 //
	RX_RMODE_ROMD_DUAL_BANK1,			// ROM無効拡張モード&デュアルモード&起動バンク1 //
	RX_RMODE_ROMD_DUAL,					// ROM無効拡張モード&デュアルモード&起動バンク継続 //
// RevRXNo161003-001 Append End
};
// V.1.02 No.5 ユーザブート/USBブート対応 Append Start

typedef struct {
	BYTE	byUBCodeA[UBCODE_MAX];		// UBコードA
	BYTE	byUBCodeB[UBCODE_MAX];		// UBコードB
} FFWRX_UBCODE_DATA;

// RevRxNo120910-001 Append Start//
enum FFWRXENM_UBAREA_CHANGE_ENA {	// ユーザブートモードまたはUSBブートモード起動時のユーザブート領域/USBブート領域書き換え許可情報 //
	RX_UBAREA_CHANGE_DIS=0,				// ユーザブートモード/USBブートモード起動時にユーザブート/USBブート領域の書き換え不可 //
	RX_UBAREA_CHANGE_ENA,				// ユーザブートモード/USBブートモード起動時にユーザブート/USBブート領域の書き換え可 //
};
typedef struct {
	enum FFWRXENM_UBAREA_CHANGE_ENA	eChangeUbAreaEna;	// ユーザブート領域書き換え許可情報 //
	BYTE	byChangeInfoUBCodeA[UBCODE_MAX];			// UBコードA情報 //
} FFWRX_UBAREA_CHANGEINFO_DATA;
// RevRxNo120910-001 Append End//

// ターゲットMCU端子のマスク情報格納構造体//
typedef struct {
	enum FFWRXENM_MSTS_MASK	eNmi;						// NMI端子入力信号のマスク情報//
	enum FFWRXENM_MSTS_MASK	eInterrupt;					// RESET端子入力信号のマスク情報//
	enum FFWRXENM_MSTS_MASK	eReset;						// RESET端子入力信号のマスク情報//
} FFWRX_MSTSM_DATA;

typedef struct {
	DWORD	dwStartAddr;
	DWORD	dwEndAddr;
} FFWRX_CTRLREG_DATA;

// RevRxNo130301-001 Append Start//
// フラッシュメモリのコンフィギュレーションデータ情報//
typedef struct {
	DWORD	dwSetNum;					// コンフィギュレーションデータ設定数//
	DWORD	dwSetNo[FMCD_NUM_MAX];		// コンフィギュレーションデータ設定先情報//
	DWORD	dwSetData[FMCD_NUM_MAX];	// コンフィギュレーションデータ//
} FFWRX_FMCD_DATA;
// RevRxNo130301-001 Append End//

//---------------------------
// レジスタ操作コマンド//
//---------------------------
// レジスタ情報格納構造体//
// 2008.9.9 INSERT_BEGIN_E20RX600(+4) {
typedef struct {
	enum FFWENM_SETREG_ENABLE	eRegDataSetEnable[XREG_REGNUM_RX];
	DWORD	dwRegData[XREG_REGNUM_RX];
} FFWRX_REG_DATA;
// 2008.9.9 INSERT_END_E20RX600 }

// 倍精度FPUレジスタ情報格納構造体//
// RevRxMultiNo180302-001 Append Start//
typedef struct {
	enum FFWENM_SETREG_ENABLE	eRegDataSetEnable[AREG_REGNUM_RX];
	DWORD	dwRegData[AREG_REGNUM_RX];
} FFWRX_AREG_DATA;
// RevRxMultiNo180302-001 Append End//

// RevRxNo180625-001 Append Start//
// レジスタ退避バンク情報格納構造体//
typedef struct {
	BYTE	byRegBankNum;
	DWORD	dwRegBankData[REGBANK_REGNUM];
} FFWRX_REGBANK_DATA;
// RevRxNo180625-001 Append End//


// フラッシュメモリ初期化ブロック格納構造体//
typedef struct {
	DWORD	dwNum;										// フラッシュブロック領域パターン数//
	DWORD	dwmadrFlashAreaStart[FOVR_RX_BLKNUM_MAX];	// フラッシュブロック領域の先頭アドレス//
	DWORD	dwmadrFlashAreaEnd[FOVR_RX_BLKNUM_MAX];		// フラッシュブロック領域の最終アドレス//
} FFWRX_FOVR_DATA;


//---------------------------
// イベントコマンド//
//---------------------------
// 2008.10.20 INSERT_BEGIN_E20RX600(+NN) {
typedef struct{
	DWORD	dwAddr;			// アドレス値を指定//
} FFWRX_EV_ADDR;

typedef struct{
	BYTE	byKindSelect;	// 指定内容//
	DWORD	dwAddrStart;	// 開始アドレス値//
	DWORD	dwAddrEnd;		// 終了アドレス値//
	DWORD	dwDataValue;	// データ値//
	DWORD	dwDataMask;		// データマスク値//
	BYTE	byAccess;		// アクセス属性//
	BYTE	byDataSize;		// データサイズ//
	BYTE	byAddrCmp;		// アドレス比較条件//
	BYTE	byDataCmp;		// データ比較条件//
	// RevRxNo120910-001 Append Line//
	DWORD	dwBusSel;		// バス選択//
} FFWRX_EV_OPC;

typedef struct{
	FFWRX_EV_ADDR	evAddr;	//命令実行イベント//
	FFWRX_EV_OPC	evOpc;	//オペランドアクセスイベント//
} FFWRX_EV_DATA;

typedef struct{
	BOOL		bEnable;	// イベント成立回数の指定可否//
	FFWRX_EV	eEvNo;		// イベント番号//
	DWORD		dwCount;	// イベント成立回数//
} FFWRX_EVCNT_DATA;

typedef struct{
	BYTE	byEvKind;		// 変更対象イベントを指定//
	DWORD	dwTrcTrg;		// トレース用イベントトリガを指定//
	DWORD	dwPerfTrg;		// パフォーマンス用イベントトリガを指定//
} FFWRX_TRG_DATA;

typedef struct{
	DWORD	dwPreBrk;		// 有効にする実行前PCブレークイベント//
	DWORD	dwBrkpe;		// 有効にする命令実行イベント//
	DWORD	dwBrkde;		// 有効にするオペランドアクセスイベント //
} FFWRX_COMB_BRK;

typedef struct{
	DWORD	dwStartpe;		// トレース開始イベントとして有効な命令実行イベント//
	DWORD	dwEndpe;		// トレース終了イベントとして有効な命令実行イベント//
	DWORD	dwStartde;		// トレース開始イベントとして有効なデータ比較イベント//
	DWORD	dwEndde;		// トレース終了イベントとして有効なデータ比較イベント//
	DWORD	dwPickupde;		// トレース抽出イベントとして有効なデータ比較イベント//
} FFWRX_COMB_TRC;

typedef struct{
	DWORD	dwStartpe[2];	// ﾊﾟﾌｫｰﾏﾝｽ計測開始イベントとして有効な命令実行イベント//
	DWORD	dwEndpe[2];		// ﾊﾟﾌｫｰﾏﾝｽ計測終了イベントとして有効な命令実行イベント//
	DWORD	dwStartde[2];	// ﾊﾟﾌｫｰﾏﾝｽ計測開始イベントとして有効なデータ比較イベント//
	DWORD	dwEndde[2];		// ﾊﾟﾌｫｰﾏﾝｽ計測終了イベントとして有効なデータ比較イベント//
} FFWRX_COMB_PERFTIME;
// 2008.10.20 INSERT_END_E20RX600 }

//---------------------------
// デバッグコンソールコマンド//
//---------------------------
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
// C2Eレジスタ値格納構造体//
typedef struct {
	DWORD dwRegNo;								// データ取得対象のC2Eレジスタ//
	DWORD dwLength;								// 取得するデータ数//
} FFWRX_C2E_DATA;

// E2Cレジスタ値格納構造体//
typedef struct {
	DWORD  dwRegNo;								// データ取得対象のE2Cレジスタ//
	DWORD  dwLength;							// 取得するデータ数//
	BYTE*  pbyData;								// E2Cレジスタに設定するデータ//
} FFWRX_E2C_DATA;
// 2008.11.18 INSERT_END_E20RX600 }


//---------------------------
// トレース関連コマンド//
//---------------------------
// 2008.10.24 INSERT_BEGIN_E20RX600(+NN) {
// トレースサイクル格納構造体(RX用)//
typedef struct {
	DWORD	dwStartCyc;			// 開始サイクル//
	DWORD	dwEndCyc;			// 終了サイクル//
} FFWRX_RCY_DATA;

// トレースモード格納構造体(RX用)//
typedef struct {
	DWORD						dwSetMode;			// トレースモード//
	DWORD						dwSetInitial;		// トレース初期情報//
	DWORD						dwSetInfo;			// トレース出力情報//
	// RevRxNo120910-001 Append Line//
	DWORD						dwTrcSrcSel;		// オペランドアクセス時のトレースソース選択情報//
	DWORD						dwRomStartAddr;		// ROM空間先頭アドレス//
	DWORD						dwRomEndAddr;		// ROM空間最終アドレス//
	DWORD						dwWinTrStartAddr;	// ウィンドウトレース先頭アドレス//
	DWORD						dwWinTrEndAddr;		// ウィンドウトレース最終アドレス//
	enum FFWENM_EMLTRCMEMSIZ	eEmlTrcMemSize;		// トレースメモリサイズ//
} FFWRX_RM_DATA;

// トレースデータ格納構造体(RX用)//
typedef struct {		// スタンバイ情報//
	DWORD	dwStdby;		// スタンバイ情報取得有無//
} FFWRX_RD_STDBY;

typedef struct {		// ロスト情報//
	DWORD	dwLost;			// トレースデータ欠落有無//
} FFWRX_RD_LOST;

typedef struct {		// スタックポインタ情報//
	DWORD	dwStac;			// 割り込みスタック/ユーザスタック//
} FFWRX_RD_STAC;

typedef struct {		// 分岐成立可否情報//
	DWORD	dwBcnd;			// 成立可否//
	DWORD	dwTime;			// タイムスタンプ//
} FFWRX_RD_BCND;

typedef struct {		// 分岐先情報//
	DWORD	dwAddr;			// 分岐先アドレス//
	DWORD	dwTime;			// タイムスタンプ//
	BOOL	bComplete;		// 分岐先アドレス解析完了状態//
} FFWRX_RD_BRANCH_DEST;

typedef struct {		// 分岐元情報//
	DWORD	dwAddr;			// 分岐元アドレス//
	DWORD	dwTime;			// タイムスタンプ//
	BOOL	bComplete;		// 分岐元アドレス解析完了状態//
} FFWRX_RD_BRANCH_SRC;

typedef struct {		// 分岐元,分岐先情報//
	DWORD	dwAddrSrc;		// 分岐元アドレス//
	DWORD	dwAddrDest;		// 分岐先アドレス//
	DWORD	dwTime;			// タイムスタンプ//
	BOOL	bSrcComplete;	// 分岐元アドレス解析完了状態//
	BOOL	bDestComplete;	// 分岐先アドレス解析完了状態//
} FFWRX_RD_BRANCH_SRCDEST;

typedef struct {		// オペランドアクセス情報//
	DWORD	dwAccess;		// アクセス属性//
	DWORD	dwAccessSize;	// アクセスサイズ//
	DWORD	dwEvNum;		// イベント番号//
	DWORD	dwAddr;			// アドレス//
	DWORD	dwData;			// データ//
	DWORD	dwTime;			// タイムスタンプ//
	BOOL	bComplete;		// アドレス解析完了状態//
} FFWRX_RD_OPR_ACCESS;

typedef struct {		// 分岐情報(ハードウェアデバッグモード専用)//
	DWORD	dwType;			// 分岐情報の種類//
	DWORD	dwAddrSrc;		// 分岐元アドレス//
	DWORD	dwAddrDest;		// 分岐先アドレス//
	BOOL	bSrcComplete;	// 分岐元アドレス解析完了状態//
	BOOL	bDestComplete;	// 分岐先アドレス解析完了状態//
} FFWRX_RD_HW_BRANCH;

typedef struct {
	DWORD							dwType;		// トレース情報種別//
	union {
		FFWRX_RD_STDBY			stdby;		// スタンバイ情報//
		FFWRX_RD_LOST			lost;		// ロスト情報//
		FFWRX_RD_STAC			stac;		// スタックポインタ情報//
		FFWRX_RD_BCND			bcnd;		// 分岐成立可否情報//
		FFWRX_RD_BRANCH_DEST	dest;		// 分岐先情報//
		FFWRX_RD_BRANCH_SRC		src;		// 分岐元情報//
		FFWRX_RD_BRANCH_SRCDEST	srcdest;	// 分岐元,分岐先情報//
		FFWRX_RD_OPR_ACCESS		opr;		// オペランドアクセス情報//
		// 以下はハードウェアデバッグモード専用//
		FFWRX_RD_HW_BRANCH		hwbr;		// 分岐情報//
	};
	BYTE							byType1;	// トレース識別子1情報//
} FFWRX_RD_DATA;
// 2008.10.24 INSERT_END_E20RX600 }


//---------------------------
// 時間測定関連コマンド//
//---------------------------
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
// パフォーマンス計測イベント格納構造体//
typedef struct {
	BOOL	bSetConnect;						// カウンタ連結ON/OFF//
	DWORD	dwMpItemNum[2];						// 計測項目番号設定//
	BOOL	bSetOnceMeasure[2];					// 計測対象1回限定設定//
	DWORD	dwOvf[2];							// カウンタオーバーフロー設定//
	BOOL	bSetPerfFunc[2];					// パフォーマンスカウンタ機能ON/OFF//
} FFWRX_PPCCTRL_DATA;

// 区間時間測定データ格納構造体//
typedef struct {
	BOOL	bTimOvf[2];							// 区間実行時間カウンタのオーバフロー状態を格納//
	DWORD	dwTotal[2];							// 実行時間積算値32ビットを格納//
} FFWRX_MPD_DATA;
// 2008.11.18 INSERT_END_E20RX600 }


// RevRxNo130308-001 Append Start
//---------------------------
// カバレッジ計測関連コマンド//
//---------------------------
// カバレッジ機能時のトレースモード格納構造体//
typedef struct {
	DWORD	dwSetTBW;		// トレースデータ出力端子幅//
	DWORD	dwSetTRM;		// トレース情報取得モード//
	DWORD	dwSetTRC;		// トレースクロック比//
} FFWRX_CVM_DATA;

// カバレッジ機能時のカバレッジ計測領域格納構造体//
typedef struct {
	enum FFWRXENM_CVB_ENABLE	eEnable[CV_RX_BLKNUM_MAX];		// カバレッジの動作禁止/許可//
	DWORD						dwmadrBase[CV_RX_BLKNUM_MAX];	// カバレッジ計測領域のベースアドレス//
} FFWRX_CVB_DATA;
// RevRxNo130308-001 Append End


// RevRx6z4FpgaNo150525-001 Append Start
//---------------------------
// マルチコア関連コマンド//
//---------------------------
// クロストリガ情報格納構造体//
typedef struct {
    DWORD         dwSetNum;
    DWORD         dwSrcCore[8];
    DWORD         dwDstCore[8];
    WORD          wCrsTrg[8];
} FFW_CRSTRG_DATA; 
// RevRx6z4FpgaNo150525-001 Append End

//////// FFW I/F関数の宣言 ////////

//----- FFW用宣言 -----//
#ifdef FFWE20_EXPORTS

// MCU依存コマンド//

// 2008.9.9 INSERT_BEGIN_E20RX600(+1) {
extern FFWE100_API	DWORD FFWRXCmd_HPON(DWORD dwPlug);									///< ホットプラグ機能の設定/解除//
// 2008.9.9 INSERT_END_E20RX600 }
// 2008.11.18 INSERT_BEGIN_E20RX600(+2) {
extern FFWE100_API	DWORD FFWRXCmd_IDCODE(BYTE* pbyID);									///< 認証用のIDコードを設定//
// 2008.11.18 INSERT_END_E20RX600 }

// RevRxNo120910-001 Modify Start//
extern FFWE100_API	DWORD FFWRXCmd_SetMCU(DWORD dwSizeOfMcuAreaStruct, const FFWRX_MCUAREA_DATA* pMcuArea, 
										  DWORD dwSizeOfMcuInfoStruct, const FFWRX_MCUINFO_DATA* pMcuInfo);	///< MCU 別情報の設定//
extern FFWE100_API	DWORD FFWRXCmd_GetMCU(DWORD dwSizeOfMcuAreaStruct, FFWRX_MCUAREA_DATA* pMcuArea, 
										  DWORD dwSizeOfMcuInfoStruct, FFWRX_MCUINFO_DATA* pMcuInfo);		///< MCU 別情報の設定//
// RevRxNo120910-001 Modify End//
// RevRxNo120910-001 Modify Start//
extern FFWE100_API	DWORD FFWRXCmd_SetDBG(DWORD dwSizeOfDbgStruct, const FFWRX_DBG_DATA* pDbg);	///< デバッグ情報の設定//
extern FFWE100_API	DWORD FFWRXCmd_GetDBG(DWORD dwSizeOfDbgStruct, FFWRX_DBG_DATA* pDbg);		///< デバッグ情報の参照//
// RevRxNo120910-001 Modify End//
// V.1.02 No.1 FINE通信I/F通信処理追加 Modify Line
// RevRxNo120910-001 Modify Start//
extern FFWE100_API	DWORD FFWRXCmd_GetSTAT(DWORD dwStatKind, DWORD dwSizeOfStatStruct, 
										   FFWRX_STAT_DATA* pStat);						///< ターゲットMCUの状態報告//
// RevRxNo120910-001 Modify End//

// V.1.02 No.3 起動時のエンディアンおよびデバッグ継続モード設定 Append Start
// RevRxNo120910-001 Modify Start//
extern FFWE100_API	DWORD FFWRXCmd_SetPMOD(enum FFWRXENM_PMODE ePmode, enum FFWRXENM_RMODE eRmode, 
										enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA* pUbcode, 
										const FFWRX_UBAREA_CHANGEINFO_DATA* pUbAreaChange);							///< プロセッサモードの設定 //
extern FFWE100_API	DWORD FFWRXCmd_GetPMOD(enum FFWRXENM_PMODE *const pePmode, enum FFWRXENM_RMODE *const peRmode,  
										enum FFWENM_ENDIAN *const peMcuEndian, FFWRX_UBCODE_DATA* pUbcode, 
										FFWRX_UBAREA_CHANGEINFO_DATA *const pUbAreaChange);							///< プロセッサモードの参照 //
// RevRxNo120910-001 Modify End//
// V.1.02 No.3 起動時のエンディアンおよびデバッグ継続モード設定 Append End
extern FFWE100_API	DWORD FFWRXCmd_SetMSTSM(const FFWRX_MSTSM_DATA* pMask);	///< ターゲットMCU端子のマスク情報の設定//
extern FFWE100_API	DWORD FFWRXCmd_GetMSTSM(FFWRX_MSTSM_DATA *const pMask);	///< ターゲットMCU端子のマスク情報の参照//
extern FFWE100_API	DWORD FFWRXCmd_INSTCODE(DWORD dwPC, FFWRX_INST_DATA* pInstResult);		///< 指定したPC位置から始まる命令の命令コードを解析//
// RevRxNo130301-001 Append Start//
extern FFWE100_API	DWORD FFWRXCmd_SetFMCD(const FFWRX_FMCD_DATA* pFmcd);	///< フラッシュメモリコンフィギュレーションデータの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetFMCD(FFWRX_FMCD_DATA *const pFmcd);	///< フラッシュメモリコンフィギュレーションデータの参照//
// RevRxNo130301-001 Append End//

// プログラム実行関連コマンド//
// RevRxNo120910-001 Modify Line//
extern FFWE100_API	DWORD FFWRXCmd_GetPROG(DWORD dwSizeOfProgStruct, FFWRX_PROG_DATA* pProg);	///< プログラム実行中の情報入手//
extern FFWE100_API	DWORD FFWRXCmd_GetBRKF(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb, 
										   BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA);		///< ブレーク要因の参照//
// 2008.9.9 INSERT_BEGIN_E20RX600(+2) {
extern FFWE100_API	DWORD FFWRXCmd_SetXREG(const FFWRX_REG_DATA* pReg);	///< レジスタ値の設定//
extern FFWE100_API	DWORD FFWRXCmd_GetXREG(FFWRX_REG_DATA* pReg);			///< レジスタ値の参照//
// 2008.9.9 INSERT_END_E20RX600 }
// RevRxMultiNo180302-001 Append Start//
extern FFWE100_API	DWORD FFWRXCmd_SetAREG(DWORD dwSizeOfAregStruct, const FFWRX_AREG_DATA* pAreg);	///< 追加レジスタ値の設定//
extern FFWE100_API	DWORD FFWRXCmd_GetAREG(DWORD dwSizeOfAregStruct, FFWRX_AREG_DATA* pAreg);		///< 追加レジスタ値の参照//
// RevRxMultiNo180302-001 Append End//
// RevRxNo180625-001 Append Start//
extern FFWE100_API	DWORD FFWRXCmd_SetREGBANK(const FFWRX_REGBANK_DATA* pRegBank);	///< レジスタ退避バンクの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetREGBANK(FFWRX_REGBANK_DATA* pRegBank);		///< レジスタ退避バンクの参照//
// RevRxNo180625-001 Append End//


// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Start
// プログラムダウンロード関連コマンド//
extern FFWE100_API	DWORD FFWRXCmd_SetFOVR(const FFWRX_FOVR_DATA* pFovr);	///< フラッシュメモリ上書きブロックの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetFOVR(FFWRX_FOVR_DATA* pFovr);			///< フラッシュメモリ上書きブロックの参照//
// V.1.02 No.8 フラッシュROMブロック細分化対応 Append End

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
extern FFWE100_API	DWORD FFWRXCmd_WTRLOAD(enum FFWRXENM_WTRTYPE eWtrType, DWORD dwDataSize, const BYTE* pbyDataBuff);	///< 書き込みプログラムのロード //
extern FFWE100_API	DWORD FFWRXCmd_GetWTRVER(enum FFWRXENM_WTRTYPE eWtrType, char *const psWtrVer);			///< 書き込みプログラムのバージョン参照//
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

// デバッグコンソール関連コマンド//
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
extern FFWE100_API	DWORD FFWRXCmd_GetC2E(FFWRX_C2E_DATA* pC2E, BYTE* pbyData);		///< C2Eレジスタ内データの読み込み//
extern FFWE100_API	DWORD FFWRXCmd_SetE2C(FFWRX_E2C_DATA* pE2C);										///< E2Cレジスタ内データへの書き込み//
extern FFWE100_API	DWORD FFWRXCmd_ClrC2EE2C(void);			///< C2E/E2Cレジスタの初期化//
// 2008.11.18 INSERT_END_E20RX600 }


// イベント関連コマンド //
// 2008.11.12 INSERT_BEGIN_E20RX600(+10) {
extern FFWE100_API DWORD FFWRXCmd_SetEV(enum FFWRX_EV eEvNo, FFWRX_EV_DATA* ev);					///< イベントの設定//
extern FFWE100_API DWORD FFWRXCmd_GetEV(enum FFWRX_EV eEvNo, FFWRX_EV_DATA* ev);					///< イベントの参照//
extern FFWE100_API DWORD FFWRXCmd_SetEVCNT(FFWRX_EVCNT_DATA* evcnt);						///< イベント成立回数の設定//
extern FFWE100_API DWORD FFWRXCmd_GetEVCNT(FFWRX_EVCNT_DATA* evcnt);						///< イベント成立回数の参照//
extern FFWE100_API DWORD FFWRXCmd_SetCOMB(BYTE byEvKind, DWORD dwCombi, FFWRX_COMB_BRK* evBrk,
											 FFWRX_COMB_TRC* evTrc, FFWRX_COMB_PERFTIME* evPerfTime,
											 enum FFWRX_EVCOMBI_PPC_USR_CTRL ePpcUsrCtrl);	///< イベントの組合せ設定//
extern FFWE100_API DWORD FFWRXCmd_GetCOMB(DWORD* dwCombi, FFWRX_COMB_BRK* evBrk,
											 FFWRX_COMB_TRC* evTrc, FFWRX_COMB_PERFTIME* evPerfTime,
											 enum FFWRX_EVCOMBI_PPC_USR_CTRL* ePpcUsrCtrl);	///< イベントの組合せ参照//
extern FFWE100_API DWORD FFWRXCmd_SetEVTRG(BYTE byEvKind, FFWRX_TRG_DATA evTrg);			///< イベントトリガの設定//
extern FFWE100_API DWORD FFWRXCmd_GetEVTRG(FFWRX_TRG_DATA* pEvTrg);			///< イベントトリガの参照//
// 2008.11.12 INSERT_END_E20RX600 }

// トレース関連コマンド//
extern FFWE100_API	DWORD FFWCmd_ClrRD(void);														///< トレースデータのクリア//
extern FFWE100_API  DWORD FFWCmd_GetRD2_SPL(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* rd);

// 2008.10.29 INSERT_BEGIN_E20RX600(+5) {
extern FFWE100_API	DWORD FFWRXCmd_SetRM(const FFWRX_RM_DATA* pTraceMode);					///< トレースモードの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetRM(FFWRX_RM_DATA* pTraceMode);							///< トレースモードの参照//
extern FFWE100_API	DWORD FFWRXCmd_GetRCY(FFWRX_RCY_DATA* pRcy);								///< トレースサイクルの取得//
extern FFWE100_API	DWORD FFWRXCmd_GetRD(DWORD dwStartCyc, DWORD dwEndCyc, DWORD* pdwSetInfo, FFWRX_RD_DATA* rd);	///< トレースデータの取得//
extern FFWE100_API	DWORD FFWRXCmd_SetTRSTP(enum FFWENM_TRSTP eTrstp);							///< トレースの強制停止//
// 2008.10.29 INSERT_END_E20RX600 }


// 時間測定関連コマンド//
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
extern FFWE100_API	DWORD FFWRXCmd_SetPPCCTRL(DWORD dwSetMp, const FFWRX_PPCCTRL_DATA* pPev);	///< PPC計測動作モードの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetPPCCTRL(FFWRX_PPCCTRL_DATA* pPev);						///< PPC計測動作モードの参照//
extern FFWE100_API	DWORD FFWRXCmd_SetPPCEVCNT(DWORD dwSetEvNum);								///< PPC計測カウントイベントの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetPPCEVCNT(DWORD* pdwSetEvNum);								///< PPC計測カウントイベントの参照//
extern FFWE100_API	DWORD FFWRXCmd_GetPPCD(DWORD dwGetMp, FFWRX_MPD_DATA* pMpd);				///< PPC計測データの取得//
extern FFWE100_API	DWORD FFWRXCmd_ClrPPCD(DWORD dwClrMp);										///< PPC計測データのクリア//
// 2008.11.18 INSERT_END_E20RX600 }

// RevRxNo130308-001 Append Start
// カバレッジ計測関連コマンド//
extern FFWE100_API	DWORD FFWRXCmd_SetCVM(const FFWRX_CVM_DATA* pCoverageMode);					///< カバレッジモードの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetCVM(FFWRX_CVM_DATA *const pCoverageMode);					///< カバレッジモードの参照//
extern FFWE100_API	DWORD FFWRXCmd_SetCVB(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvb);			///< カバレッジ計測ベースアドレスの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetCVB(FFWRX_CVB_DATA *const pCvb);							///< カバレッジ計測ベースアドレスの参照//
extern FFWE100_API	DWORD FFWRXCmd_GetCVD0(enum FFWRXENM_CV_BLKNO eBlkNo, 
										   DWORD dwStart, DWORD dwEnd, DWORD dwCvData[]);		///< C0カバレッジデータの取得//
extern FFWE100_API	DWORD FFWRXCmd_ClrCVD(void);												///< カバレッジデータのクリア//
// RevRxNo130308-001 Append End

// JTAG関連コマンド//
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
extern FFWE100_API	DWORD FFWE20Cmd_SetJTAGCLK(BYTE byJtagClk);		///< JTAGテストクロックの設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetJTAGCLK(BYTE* pbyJtagClk);	///< JTAGテストクロックの参照//
// 2008.11.18 INSERT_END_E20RX600 }

// RevRx6z4FpgaNo150525-001 Append Start
// マルチコア関連コマンド //
extern FFWE100_API	DWORD FFWRXCmd_SetCORESEL(const DWORD dwCoreInfo);			///< 制御対象コアの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetCORESEL(DWORD* pdwCoreInfo);				///< 制御対象コアの参照//
extern FFWE100_API	DWORD FFWRXCmd_SetCRSTRG(const FFW_CRSTRG_DATA* pCrsTrg);	///< クロストリガ機能の設定//
extern FFWE100_API	DWORD FFWRXCmd_GetCRSTRG(FFW_CRSTRG_DATA *const pCrsTrg);	///< クロストリガ機能の参照//
extern FFWE100_API	DWORD FFWRXCmd_SetMultiCoreFUNC(DWORD dwFuncNum, const DWORD* pdwFuncInfo);	///< 機能別制御対象コアの設定//
extern FFWE100_API	DWORD FFWRXCmd_GetMultiCoreFUNC(DWORD* pdwFuncNum, DWORD* pdwFuncInfo);		///< 機能別制御対象コアの参照//
// RevRx6z4FpgaNo150525-001 Append End

// RevRxE2No171004-001 Append Start
// E2 拡張機能関連コマンド(RFWから呼び出す)//
extern FFWE100_API	DWORD FFWE2Cmd_SetAspConf(BYTE byBfwSendData);	///< BFWへE2拡張機能の有効状態通知//	
extern FFWE100_API	DWORD FFWE2Cmd_SetMonitorPointAddr(void);		///< BFWへ通過ポイントの設定条件の通知//
// RevRxE2No171004-001 Append End

// 非公開コマンド(デバッグ用)//
extern FFWE100_API	DWORD FFWRXCmd_SetSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, 
										  DWORD dwWriteData);						///< シリアルアクセスレジスタの設定(FINE I/F用)//
extern FFWE100_API	DWORD FFWRXCmd_GetSAR(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, 
										  DWORD *pdwReadData);					///< シリアルアクセスレジスタの参照(FINE I/F用)//
extern FFWE100_API	DWORD FFWRXCmd_IR(DWORD dwLength, BYTE* pbyData);					///< JTAGのIRパスを実行//
extern FFWE100_API	DWORD FFWRXCmd_DR(DWORD dwLength, BYTE* pbyData, DWORD dwPause);	///< JTAGのDRパスを実行//
extern FFWE100_API	DWORD FFWRXCmd_GetDR(DWORD dwLength, BYTE* pbyData, DWORD dwPause);	///< JTAGのDRパスを実行//
extern FFWE100_API	DWORD FFWRXCmd_SEMC(BYTE byLength, DWORD* pbyData);				///< 命令コードの供給//
extern FFWE100_API	DWORD FFWRXCmd_MAST(void);										///< 供給命令コードの実行//
extern FFWE100_API	DWORD FFWRXCmd_GEMD(DWORD *dwData);								///< 供給命令コード実行による結果(C2E_0)内容の取得//
extern FFWE100_API	DWORD FFWRXCmd_SEMP(DWORD dwData);								///< 命令供給コードへのパラメータ渡し(E2C_0)に設定//
extern FFWE100_API	DWORD FFWRXCmd_FCCLR(void);										///< フラッシュROMキャッシュをクリア//
extern FFWE100_API	DWORD FFWRXCmd_FBER(void);										///< フラッシュROMブロックをクリア(FCLRで登録したブロックのみ)//

//----- Target.DLL, NDEB用 -----//
#else

// MCU依存コマンド//
// 2008.9.9 INSERT_BEGIN_E20RX600(+1) {
__EXTERN__	DWORD (*FFWRXCmd_HPON)(DWORD dwPlug);									///< ホットプラグ機能の設定/解除//
// 2008.9.9 INSERT_END_E20RX600 }
// 2008.11.18 INSERT_BEGIN_E20RX600(+2) {
__EXTERN__	DWORD (*FFWRXCmd_IDCODE)(BYTE* pbyID);									///< 認証用のIDコードを設定//
// 2008.11.18 INSERT_END_E20RX600 }
// RevRxNo120910-001 Modify Start//
__EXTERN__	DWORD (*FFWRXCmd_SetMCU)(DWORD dwSizeOfMcuAreaStruct, const FFWRX_MCUAREA_DATA* pMcuArea, 
									 DWORD dwSizeOfMcuInfoStruct, const FFWRX_MCUINFO_DATA* pMcuInfo);	///< MCU 別情報の設定//
__EXTERN__	DWORD (*FFWRXCmd_GetMCU)(DWORD dwSizeOfMcuAreaStruct, FFWRX_MCUAREA_DATA* pMcuArea, 
									 DWORD dwSizeOfMcuInfoStruct, FFWRX_MCUINFO_DATA* pMcuInfo);	///< MCU 別情報の設定//
// RevRxNo120910-001 Modify End//
// RevRxNo120910-001 Modify Start//
__EXTERN__	DWORD (*FFWRXCmd_SetDBG)(DWORD dwSizeOfDbgStruct, const FFWRX_DBG_DATA* pDbg);		///< デバッグ情報の設定//
__EXTERN__	DWORD (*FFWRXCmd_GetDBG)(DWORD dwSizeOfDbgStruct, FFWRX_DBG_DATA* pDbg);			///< デバッグ情報の参照//
// RevRxNo120910-001 Modify End//
// V.1.02 No.1 FINE通信I/F通信処理追加 Modify Line
// RevRxNo120910-001 Modify Start//
__EXTERN__	DWORD (*FFWRXCmd_GetSTAT)(DWORD dwStatKind, DWORD dwSizeOfStatStruct, 
									  FFWRX_STAT_DATA* pStat);						///< ターゲットMCUの状態報告//
// RevRxNo120910-001 Modify End//
// V.1.02 No.3 起動時のエンディアンおよびデバッグ継続モード設定 Append Start
// RevRxNo120910-001 Modify Start//
__EXTERN__	DWORD (*FFWRXCmd_SetPMOD)(enum FFWRXENM_PMODE ePmode, enum FFWRXENM_RMODE eRmode, 
										enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA* pUbcode,
										const FFWRX_UBAREA_CHANGEINFO_DATA* pUbAreaChange);						///< プロセッサモードの設定 //
__EXTERN__	DWORD (*FFWRXCmd_GetPMOD)(enum FFWRXENM_PMODE *const pePmode, enum FFWRXENM_RMODE *const peRmode,  
										enum FFWENM_ENDIAN *const peMcuEndian, FFWRX_UBCODE_DATA* pUbcode,
										FFWRX_UBAREA_CHANGEINFO_DATA *const pUbAreaChange);						///< プロセッサモードの参照 //
// RevRxNo120910-001 Modify End//
// V.1.02 No.3 起動時のエンディアンおよびデバッグ継続モード設定 Append End
__EXTERN__	DWORD (*FFWRXCmd_SetMSTSM)(const FFWRX_MSTSM_DATA* pMask);	///< ターゲットMCU端子のマスク情報の設定//
__EXTERN__	DWORD (*FFWRXCmd_GetMSTSM)(FFWRX_MSTSM_DATA *const pMask);	///< ターゲットMCU端子のマスク情報の参照//
__EXTERN__	DWORD (*FFWRXCmd_INSTCODE)(DWORD dwPC, FFWRX_INST_DATA* pInstResult);		///< 指定したPC位置から始まる命令の命令コードを解析//
// RevRxNo130301-001 Append Start//
__EXTERN__	DWORD (*FFWRXCmd_SetFMCD)(const FFWRX_FMCD_DATA* pFmcd);	///< フラッシュメモリコンフィギュレーションデータの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetFMCD)(FFWRX_FMCD_DATA *const pFmcd);	///< フラッシュメモリコンフィギュレーションデータの参照//
// RevRxNo130301-001 Append End//

// プログラム実行関連コマンド//
// RevRxNo120910-001 Modify Line//
__EXTERN__	DWORD (*FFWRXCmd_GetPROG)(DWORD dwSizeOfProgStruct, FFWRX_PROG_DATA* pProg);	///< プログラム実行中の情報入手//
__EXTERN__	DWORD (*FFWRXCmd_GetBRKF)(DWORD* dwBreakFact, enum FFWRX_COMB_PATTERN* eBrkTrcComb, 
									  BYTE* byOrBrkFactEvPC, BYTE* byOrBrkFactEvOA);		///< ブレーク要因の参照//
// 2008.9.9 INSERT_BEGIN_E20RX600(+2) {
__EXTERN__	DWORD (*FFWRXCmd_SetXREG)(const FFWRX_REG_DATA* pReg);	///< レジスタ値の設定//
__EXTERN__	DWORD (*FFWRXCmd_GetXREG)(FFWRX_REG_DATA* pReg);			///< レジスタ値の参照//
// 2008.9.9 INSERT_END_E20RX600 }
// RevRxMultiNo180302-001 Append Start//
__EXTERN__	DWORD (*FFWRXCmd_SetAREG)(DWORD dwSizeOfAregStruct, const FFWRX_AREG_DATA* pAreg);	///< 追加レジスタ値の設定//
__EXTERN__	DWORD (*FFWRXCmd_GetAREG)(DWORD dwSizeOfAregStruct, FFWRX_AREG_DATA* pAreg);		///< 追加レジスタ値の参照//
// RevRxMultiNo180302-001 Append End//
// RevRxNo180625-001 Append Start//
__EXTERN__	DWORD (*FFWRXCmd_SetREGBANK)(const FFWRX_REGBANK_DATA* pRegBank);	///< レジスタ退避バンクの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetREGBANK)(FFWRX_REGBANK_DATA* pRegBank);			///< レジスタ退避バンクの参照//
// RevRxNo180625-001 Append End//
// V.1.02 No.8 フラッシュROMブロック細分化対応 Append Start
// プログラムダウンロード関連コマンド//
__EXTERN__	DWORD (*FFWRXCmd_SetFOVR)(const FFWRX_FOVR_DATA* pFovr);	///< フラッシュメモリ上書きブロックの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetFOVR)(FFWRX_FOVR_DATA* pFovr);			///< フラッシュメモリ上書きブロックの参照//
// V.1.02 No.8 フラッシュROMブロック細分化対応 Append End

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
__EXTERN__	DWORD (*FFWRXCmd_WTRLOAD)(enum FFWRXENM_WTRTYPE eWtrType, DWORD dwDataSize, const BYTE* pbyDataBuff);	///< 書き込みプログラムのロード //
__EXTERN__	DWORD (*FFWRXCmd_GetWTRVER)(enum FFWRXENM_WTRTYPE eWtrType, char *const psWtrVer);			///< 書き込みプログラムのバージョン参照//
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

//デバッグコンソール関連コマンド //
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
__EXTERN__	DWORD (*FFWRXCmd_GetC2E)(FFWRX_C2E_DATA* pC2E, BYTE* pbyData);		///< C2Eレジスタ内データの読み込み//
__EXTERN__	DWORD (*FFWRXCmd_SetE2C)(FFWRX_E2C_DATA* pE2C);										///< E2Cレジスタ内データへの書き込み//
__EXTERN__	DWORD (*FFWRXCmd_ClrC2EE2C)(void);						///< C2E/E2Cレジスタの初期化//
// 2008.11.18 INSERT_END_E20RX600 }

// イベント関連コマンド//
// 2008.11.12 INSERT_BEGIN_E20RX600(+10) {
__EXTERN__	DWORD (*FFWRXCmd_SetEV)(enum FFWRX_EV eEvNo, FFWRX_EV_DATA* ev);					///< イベントの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetEV)(enum FFWRX_EV eEvNo, FFWRX_EV_DATA* ev);					///< イベントの参照//
__EXTERN__	DWORD (*FFWRXCmd_SetEVCNT)(FFWRX_EVCNT_DATA* evcnt);						///< イベント成立回数の設定//
__EXTERN__	DWORD (*FFWRXCmd_GetEVCNT)(FFWRX_EVCNT_DATA* evcnt);						///< イベント成立回数の参照//
__EXTERN__	DWORD (*FFWRXCmd_SetCOMB)(BYTE byEvKind, DWORD dwCombi, FFWRX_COMB_BRK* evBrk,
										 FFWRX_COMB_TRC* evTrc, FFWRX_COMB_PERFTIME* evPerfTime,
										 enum FFWRX_EVCOMBI_PPC_USR_CTRL ePpcUsrCtrl);	///< イベントの組合せ設定//
__EXTERN__	DWORD (*FFWRXCmd_GetCOMB)(DWORD* dwCombi, FFWRX_COMB_BRK* evBrk,
										 FFWRX_COMB_TRC* evTrc, FFWRX_COMB_PERFTIME* evPerfTime,
										 enum FFWRX_EVCOMBI_PPC_USR_CTRL* ePpcUsrCtrl);	///< イベントの組合せ参照//
__EXTERN__	DWORD (*FFWRXCmd_SetEVTRG)(BYTE byEvKind, FFWRX_TRG_DATA evTrg);			///< イベントトリガの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetEVTRG)(FFWRX_TRG_DATA* pEvTrg);						///< イベントトリガの参照//
// 2008.11.12 INSERT_END_E20RX600 }

// トレース関連コマンド//
__EXTERN__	DWORD (*FFWCmd_ClrRD)(void);														///< トレースデータのクリア//
__EXTERN__	DWORD (*FFWCmd_GetRD2_SPL)(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* rd);

// 2008.10.29 INSERT_BEGIN_E20RX600(+6) {
__EXTERN__	DWORD (*FFWRXCmd_SetRM)(const FFWRX_RM_DATA* pTraceMode);						///< トレースモードの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetRM)(FFWRX_RM_DATA* pTraceMode);							///< トレースモードの参照//
__EXTERN__	DWORD (*FFWRXCmd_GetRCY)(FFWRX_RCY_DATA* pRcy);								///< トレースサイクルの取得//
__EXTERN__	DWORD (*FFWRXCmd_GetRD)(DWORD dwStartCyc, DWORD dwEndCyc, DWORD* pdwSetInfo, FFWRX_RD_DATA* rd);	///< トレースデータの取得//
__EXTERN__	DWORD (*FFWRXCmd_SetTRSTP)(enum FFWENM_TRSTP eTrstp);							///< トレースの強制停止//
// 2008.10.29 INSERT_END_E20RX600 }

// 時間測定関連コマンド//
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
__EXTERN__	DWORD (*FFWRXCmd_SetPPCCTRL)(DWORD dwSetMp, const FFWRX_PPCCTRL_DATA* pPev);	///< PPC計測動作モードの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetPPCCTRL)(FFWRX_PPCCTRL_DATA* pPev);							///< PPC計測動作モードの参照//
__EXTERN__	DWORD (*FFWRXCmd_SetPPCEVCNT)(DWORD dwSetEvNum);								///< PPC計測カウントイベントの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetPPCEVCNT)(DWORD* pdwSetEvNum);								///< PPC計測カウントイベントの参照//
__EXTERN__	DWORD (*FFWRXCmd_GetPPCD)(DWORD dwGetMp, FFWRX_MPD_DATA* pMpd);					///< PPC計測データの取得//
__EXTERN__	DWORD (*FFWRXCmd_ClrPPCD)(DWORD dwClrMp);										///< PPC計測データのクリア//
// 2008.11.18 INSERT_END_E20RX600 }

// RevRxNo130308-001 Append Start
// カバレッジ計測関連コマンド//
__EXTERN__	DWORD (*FFWRXCmd_SetCVM)(const FFWRX_CVM_DATA* pCoverageMode);					///< カバレッジモードの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetCVM)(FFWRX_CVM_DATA *const pCoverageMode);					///< カバレッジモードの参照//
__EXTERN__	DWORD (*FFWRXCmd_SetCVB)(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvb);			///< カバレッジ計測ベースアドレスの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetCVB)(FFWRX_CVB_DATA *const pCvb);							///< カバレッジ計測ベースアドレスの参照//
__EXTERN__	DWORD (*FFWRXCmd_GetCVD0)(enum FFWRXENM_CV_BLKNO eBlkNo, 
									  DWORD dwStart, DWORD dwEnd, DWORD dwCvData[]);		///< C0カバレッジデータの取得//
__EXTERN__	DWORD (*FFWRXCmd_ClrCVD)(void);													///< カバレッジデータのクリア//
// RevRxNo130308-001 Append End

// クロック関連コマンド//
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
__EXTERN__	DWORD (*FFWE20Cmd_SetJTAGCLK)(BYTE byJtagClk);		///< JTAGテストクロックの設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetJTAGCLK)(BYTE* pbyJtagClk);	///< JTAGテストクロックの参照//
// 2008.11.18 INSERT_END_E20RX600 }

// RevRx6z4FpgaNo150525-001 Append Start
// マルチコア関連コマンド //
__EXTERN__	DWORD (*FFWRXCmd_SetCORESEL)(const DWORD dwCoreInfo);			///< 制御対象コアの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetCORESEL)(DWORD* pdwCoreInfo);				///< 制御対象コアの参照//
__EXTERN__	DWORD (*FFWRXCmd_SetCRSTRG)(const FFW_CRSTRG_DATA* pCrsTrg);	///< クロストリガ機能の設定//
__EXTERN__	DWORD (*FFWRXCmd_GetCRSTRG)(FFW_CRSTRG_DATA *const pCrsTrg);	///< クロストリガ機能の参照//
__EXTERN__	DWORD (*FFWRXCmd_SetMultiCoreFUNC)(DWORD dwFuncNum, const DWORD* pdwFuncInfo);	///< 機能別制御対象コアの設定//
__EXTERN__	DWORD (*FFWRXCmd_GetMultiCoreFUNC)(DWORD* pdwFuncNum, DWORD* pdwFuncInfo);		///< 機能別制御対象コアの参照//
// RevRx6z4FpgaNo150525-001 Append End

// RevRxE2No171004-001 Append Start
// E2 拡張機能関連コマンド(RFWから呼び出す)//
__EXTERN__	DWORD (*FFWE2Cmd_SetAspConf)(BYTE byBfwSendData);	///< BFWへE2拡張機能の有効状態通知//	
__EXTERN__	DWORD (*FFWE2Cmd_SetMonitorPointAddr)(void);		///< BFWへ通過ポイントの設定条件の通知//
// RevRxE2No171004-001 Append End

// V.1.02 FINE I/F対応 Append Start
// 非公開コマンド(デバッグ用)//
__EXTERN__	DWORD (*FFWRXCmd_SetSAR)(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, 
										  DWORD dwWriteData);						///< シリアルアクセスレジスタの設定(FINE I/F用)//
__EXTERN__	DWORD (*FFWRXCmd_GetSAR)(BYTE byCommand, enum FFWENM_MACCESS_SIZE eAccessSize, 
										  DWORD *pdwReadData);					///< シリアルアクセスレジスタの参照(FINE I/F用)//
// V.1.02 FINE I/F対応 Append End
__EXTERN__	DWORD (*FFWRXCmd_IR)(DWORD dwLength, BYTE* pbyData);					///< JTAGのIRパスを実行//
__EXTERN__	DWORD (*FFWRXCmd_DR)(DWORD dwLength, BYTE* pbyData, DWORD dwPause);	///< JTAGのDRパスを実行//
__EXTERN__	DWORD (*FFWRXCmd_GetDR)(DWORD dwLength, BYTE* pbyData, DWORD dwPause);	///< JTAGのDRパスを実行//
__EXTERN__	DWORD (*FFWRXCmd_SEMC)(BYTE byLength, DWORD* pbyData);				///< 命令コードの供給//
__EXTERN__	DWORD (*FFWRXCmd_MAST)(void);										///< 供給命令コードの実行//
__EXTERN__	DWORD (*FFWRXCmd_GEMD)(DWORD *dwData);								///< 供給命令コード実行による結果(C2E_0)内容の取得//
__EXTERN__	DWORD (*FFWRXCmd_SEMP)(DWORD dwData);								///< 命令供給コードへのパラメータ渡し(E2C_0)に設定//
__EXTERN__	DWORD (*FFWRXCmd_FCCLR)(void);										///< フラッシュROMキャッシュをクリア//
__EXTERN__	DWORD (*FFWRXCmd_FBER)(void);										///< フラッシュROMブロックをクリア(FCLRで登録したブロックのみ)//

#endif

#ifdef	__cplusplus
	};
#endif

#endif
