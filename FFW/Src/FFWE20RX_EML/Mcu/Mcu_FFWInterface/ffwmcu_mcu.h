///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_mcu.h
 * @brief MCU依存コマンドのヘッダファイル
 * @author RSO Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, S.Ueda, Y.Kawakami, K.Uemori, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/09/19
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RX220 WS対応 RX210でRX220対応 2012/07/11 橋口
・RevNo111128-001	2012/07/11 橋口
  ・以下関数のextern 宣言を新規追加。
      GetMdeRegData()
      SetMdeRegData()
  ・STATコマンドのプロセッサモード状態の定義を追加。
・RevRxNo111128-001		2012/07/11 橋口
  ・STATコマンドのMODE端子状態の定義を追加。
・RevRxNo120606-003 2012/07/12 橋口
　・SYSCR0レジスタがないMCU対応
・RevNo121105-001	2012/11/05 上田
　・MCU_REGINFO_DFLRE_NO, MCU_REGINFO_DFLWE_NOのdefine定義追加。
・RevNo121213-001	2012/12/13 上田
　・MCU_REG_BLK_MAX, MCU_REG_NUM_MAXのdefine定義削除。
・RevRxNo130301-001 2013/04/12 上田
	RX64M対応
・RevRxNo130809-001 2013/08/09 上田
	MCU, DBG, STATコマンドのパラメータ可変長対応処理改善
・RevRxNo140515-006 2014/06/19 川上
	RX231対応
・RevRxNo140515-012 2014/06/20 川上
	ホットプラグ起動中のカバレッジ機能エラー処理修正
・RevRxNo130730-001 2014/06/24 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140617-001	2014/06/17 大喜多
	TrustedMemory機能対応
・RevRxNo140515-005 2014/06/26 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo150528-001	2015/05/28 PA 紡車
	メモリウェイト仕様追加対応
・RevRxNo150827-005 2015/12/03 PA 紡車
	RX651対応
・RevRxNo150827-002 2015/09/04 PA 紡車
	RX651オプション設定メモリアドレス、配置変更対応
・RevRxNo150928-001 2015/09/28 PA 辻
	ROMキャッシュインバリデート Warning対応
	対応見送りのため、コメントアウト
・RevRxNo151028-001 2015/10/28 南
	RX130 電圧監視リセット0対応のため、
	SPC_RX100_LVD_LEVEL_CHKの定義を追加
・RevRxNo161003-001 2016/10/03 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
・RevRxNo180625-001,PB18047-CD01-001 2018/06/25 PA 辻、PA 紡車
	RX66T-H/RX72T対応
*/
#ifndef	__FFWMCU_MCU_H__
#define	__FFWMCU_MCU_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義
// RevRxNo130809-001 Append Start
// MCUコマンド 構造体サイズ定義
#define FFWRX_MCUAREA_DATA_SIZE_V20000		0x00002230	// V.2.00.00のFFWRX_MCUAREA_DATA構造体サイズ
#define FFWRX_MCUINFO_DATA_SIZE_V20000		0x00001628	// V.2.00.00のFFWRX_MCUINFO_DATA構造体サイズ
#define FFWRX_MCUINFO_DATA_SIZE_V20200		0x00001630	// V.2.02.00のFFWRX_MCUINFO_DATA構造体サイズ
// RevRxNo140515-005 Append Line 
#define FFWRX_MCUINFO_DATA_SIZE_V20300		0x00001634	// V.2.03.00のFFWRX_MCUINFO_DATA構造体サイズ
// DBGコマンド 構造体サイズ定義
#define FFWRX_DBG_DATA_SIZE_V20000			0x00001944	// V.2.00.00のFFWRX_DBG_DATA構造体サイズ
// STATコマンド 構造体サイズ定義
#define FFWRX_STAT_DATA_SIZE_V20000			0x00000024	// V.2.00.00のFFWRX_STAT_DATA構造体サイズ
#define FFWRX_STAT_SFR_DATA_SIZE_V20000		0x00000010	// V.2.00.00のFFWRX_STAT_SFR_DATA構造体サイズ
#define FFWRX_STAT_JTAG_DATA_SIZE_V20000	0x00000018	// V.2.00.00のFFWRX_STAT_JTAG_DATA構造体サイズ
#define FFWRX_STAT_FINE_DATA_SIZE_V20000	0x00000020	// V.2.00.00のFFWRX_STAT_FINE_DATA構造体サイズ
#define FFWRX_STAT_EML_DATA_SIZE_V20000		0x00000014	// V.2.00.00のFFWRX_STAT_EML_DATA構造体サイズ
#define FFWRX_STAT_EML_DATA_SIZE_V20200		0x00000018	// V.2.02.00のFFWRX_STAT_EML_DATA構造体サイズ
// RevRxNo130809-001 Append End

#define INIT_SFRAREA_NUM		1				// SFR領域数初期値
#define INIT_RAMAREA_NUM		1				// MCU内蔵RAM領域数初期値
#define INIT_FLASHBLOCK_NUM		1				// MCU内蔵フラッシュROM領域数初期値
#define INIT_FLASHBLOCK_SIZE	0x10000			// MCU内蔵フラッシュROMブロックサイズ初期値
#define INIT_EXTROM_NUM			8				// 外部ROM領域数初期値

#define INIT_JSRCODE_NUM		4				// ジャンプサブルチーン命令コードの定義数初期値
#define INIT_RTSCODE_NUM		1				// リターンサブルチーン命令コードの定義数初期値

#define INIT_MCUD_NUM			0				// ターゲットMCU空間に設定するデータ数初期値

#define	INIT_PMODE				RX_PMODE_SNG		// プロセッサモード情報初期値

#define HPON_CON_DBG			0xFF						// FPGAコンフィグ(デバッグ用)

#define SPC_MCU_FPGA			0x80000000
#define SPC_RX610_1STCUT		0x20000000
//RX220WS対応 Appned Start
//SPC定義
#define SPC_TRC_RX220_WS			0x00070000
#define SPC_TRC_RX220_BUFFSIZE		0x00030000
#define SPC_TRC_RX220_RX210			0x00040000
#define SPC_TRC_RX220_32CYC			0x00020000
#define SPC_TRC_RX220_0CYC			0x00010000
#define SPC_TRC_RX220_BUFF64_32CYC			0x00060000
#define SPC_TRC_RX220_BUFF64_0CYC			0x00050000
// RevRxNo120606-003 Appned Start
#define SPC_SYSCR0_READ				0x00100000
#define SPC_SYSCR0_READ_OK			0x00000000
#define SPC_SYSCR0_READ_NG			0x00100000
// RevRxNo120606-003 Append End
#define SPC_FLASH_SC32_MF3			0x00200000	//RevRxNo140515-006 Append Line
#define SPC_FLASH_RV40F2			0x00400000	//RevRxNo150827-005 Append Line
#define SPC_FLASH_TYPE_MASK			0x00600000	//RevRxNo150827-005 Append Line

// RevRxNo161003-001 Append Start
#define	SPC_EEPFCLK_EXIST			0x80000000
#define	SPC_BANKMD_EXIST			0x20000000
#define	SPC_BANKSEL_EXIST			0x10000000
#define SPC_ROM_DATA_TRC_ACCESS_TYPE_EXIST	0x40000000
// RevRxNo161003-001 Append End

// RevRxNo140617-001,RevRxNo140515-005 Append Start
#define SPC_TM_EXIST				0x00800000
#define SPC_MEMORY_WAIT				0x00008000
#define SPC_MEMORY_WAIT_AREA		0x00006000
#define SPC_MEMORY_WAIT_AREA_NON	0x00000000
#define SPC_MEMORY_WAIT_AREA_ROM	0x00002000
#define SPC_MEMORY_WAIT_AREA_RAM	0x00004000
#define SPC_MEMORY_WAIT_AREA_ALL	0x00006000
#define SPC_MEMORY_WAIT_MCUTYPE			0x00001C00
#define SPC_MEMORY_WAIT_MCUTYPE_RX700	0x00000000
#define SPC_MEMORY_WAIT_MCUTYPE_RX231	0x00000400
#define SPC_MEMORY_WAIT_MCUTYPE_RX24T	0x00000800		// RevRxNo150528-001 Append Line
#define SPC_MEMORY_WAIT_MCUTYPE_RX651	0x00000C00		// RevRxNo150827-005 Append Line
#define SPC_MEMORY_WAIT_MCUTYPE_RX66T	0x00001000		// RevRxNo180228-001 Append Line
#define SPC_MEMORY_WAIT_CYCLE		0x000003FF
#define SPC_MEMORY_WAIT_CYCLE2		0x03FF0000			// RevRxNo150528-001 Append Line
// RevRxNo140617-001,RevRxNo140515-005 Append End
// RevRx6z4FpgaNo150525-001 Append Start
#define SPC_MULTI_CORE				0x08000000		// bit27(マルチコア指定)
#define SPC_RX6Z4_FPGA				0x04000000		// bit26(システム関連レジスタアクセス不可指定)
// RevRx6z4FpgaNo150525-001 Append End

// SPC[1].bit27(RX100シリーズでの電圧検出0リセットサポート)
#define SPC_RX100_LVD_LEVEL_CHK				0x08000000	// RevRxNo151028-001 Append Line

//RX220WS対応 Appned End

// SPC[2].bit31(ROMコードプロテクト配置)
#define SPC_ROMCODE_PROTECT			0x80000000		// RevRxNo180228-001 Append Line

// SPC[2].bit20-18(メモリウェイト対象RAM領域数)
#define SPC_MEMORY_WAIT_RAM_NUM		0x001C0000		// RevRxNo180625-001 Append Line

// RevRxNo180625-001 Append Start
// SPC[2].bit16(レジスタ一括退避機能のサポート有無)
#define SPC_REGBANK_EXIST			0x00010000
#define SPC_REGBANK_START_NUM		0x0000FF00
#define SPC_REGBANK_END_NUM			0x000000FF
// RevRxNo180625-001 Append End

// RevRxNo150928-001 Append Line
#if 0
#define SPC_ROMC_EXIST		0x01000000	// ROMキャッシュ搭載品種
#endif

// RevNo121213-001 Delete

// RevNo121105-001 Append Start
// MCUレジスタ情報定義
#define	MCU_REGINFO_DFLRE_NO	0x00	// データフラッシュ読み出し許可レジスタ情報格納番号
#define	MCU_REGINFO_DFLWE_NO	0x01	// データフラッシュ書き込み/消去許可レジスタ情報格納番号
// RevNo121105-001 Append End



// RevRxNo111128-001 Append Start
#define STAT_MD_PIN_USRBOOT		0x00	// STATコマンドのMD端子状態 ユーザブートモード/USB ブートモード
#define STAT_MD_PIN_SNG			0x01	// STATコマンドのMD端子状態 シングルチップモード
// RevRxNo111128-001 Append End

// RevNo111128-001 Append Start
#define STAT_PROC_MODE_SNG1		0x00	// STATコマンドのプロセッサモード シングルチップモード
#define STAT_PROC_MODE_SNG2		0x01	// STATコマンドのプロセッサモード シングルチップモード
#define STAT_PROC_MODE_ROMD		0x02	// STATコマンドのプロセッサモード 内蔵ROM 無効拡張モード
#define STAT_PROC_MODE_ROME		0x03	// STATコマンドのプロセッサモード 内蔵ROM 有効拡張モード
// RevNo111128-001 Append End

// RevRxNo130301-001 Append Start
// RevRxNo150827-002 Modify Line
#define FMCD_SETNO_MIN			0x00000000	// FMCDコマンドの設定先情報 最小値
#define FMCD_SETNO_MAX			0x0000007F	// FMCDコマンドの設定先情報 最大値
// RevRxNo130301-001 Append End


// 構造体型のtypedef
typedef FFWRX_MCUAREA_DATA	FFWMCU_MCUAREA_DATA_RX;
typedef FFWRX_MCUINFO_DATA	FFWMCU_MCUINFO_DATA_RX;
typedef FFWRX_DBG_DATA		FFWMCU_DBG_DATA_RX;
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append Line
typedef FFWRX_UBCODE_DATA	FFWMCU_UBCODE_DATA_RX;

// グローバル関数の宣言
extern FFWMCU_MCUAREA_DATA_RX* GetMcuAreaDataRX(void);	///< MCU空間情報格納構造体のポインタ取得
extern FFWMCU_MCUINFO_DATA_RX* GetMcuInfoDataRX(void);	///< MCU空間情報格納構造体のポインタ取得
extern FFWMCU_DBG_DATA_RX* GetDbgDataRX(void);			///< RXDBG情報管理構造体のポインタ取得

// RevRxNo130730-001 Append Line
extern FFWRX_UBAREA_CHANGEINFO_DATA*	GetUBCodeDataAreaChangeRX(void);	///< ユーザブート領域の書き換え情報の管理構造体ポインタ取得

// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append Start
extern enum FFWRXENM_PMODE GetPmodeDataRX(void);		///< ユーザ指定端子設定作モード情報取得 >
extern void SetPmodeDataRX(FFWRXENM_PMODE);				///< ユーザ指定端子設定作モード情報設定 >
extern enum FFWRXENM_RMODE GetRmodeDataRX(void);		///< ユーザ指定レジスタ設定ード情報取得 >
extern void SetRmodeDataRX(FFWRXENM_RMODE);				///< ユーザ指定レジスタ設定ード情報設定 >
extern enum FFWENM_ENDIAN GetEndianDataRX(void);		///< ユーザ指定エンディアン情報取得 >
extern void SetEndianDataRX(FFWENM_ENDIAN);				///< ユーザ指定エンディアン情報設定 >
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append End
extern FFWRX_UBCODE_DATA* GetUBCodeDataRX(void);			///< UBコード情報格納構造体のポインタ取得
extern void SetUBCodeDataRX(const FFWRX_UBCODE_DATA *pUbcode);	///< UBコード情報格納構造体へのUBコード情報設定
// RevRxNo130301-001 Append Line
extern FFWRX_FMCD_DATA* GetFmcdData(void);				///< フラッシュメモリのコンフィギュレーションデータ情報格納構造体のポインタ取得
// RevRxNo161003-001 Append Start
#include "domcu_mcu.h"
extern enum FFWRXENM_BANKMD GetBankModeDataRX(void);		///< バンクモード情報取得 >
extern void SetBankModeDataRX(FFWRXENM_BANKMD);				///< バンクモード情報設定 >
extern enum FFWRXENM_BANKSEL GetBankSelDataRX(void);		///< 起動バンク情報取得 >
extern void SetBankSelDataRX(FFWRXENM_BANKSEL);				///< 起動バンク情報設定 >
// RevRxNo161003-001 Append End

extern BOOL getHotPlugState(void);								///< ホットプラグ設定状態を取得
extern void setHotPlugState(BOOL bSet);							///< ホットプラグ設定状態を設定
// RevRxNo140515-012 Append Line
extern void	setHotPlgStartFlg(BOOL bSet);						///< ホットプラグ起動フラグを設定
extern BOOL getIdCodeResultState(void);							///< ID認証結果状態を取得
extern void setIdCodeResultState(BOOL bSet);					///< ID認証結果状態を設定
// V.1.02 No.4 ユーザブートモード起動対応 Append Start
extern void SetIdCode(BYTE *pbyID);			// ユーザ設定ID格納
extern void GetIdCode(BYTE *pbyIDBuff);		// 保存しておいたユーザ設定IDコードデータ取得
// V.1.02 No.4 ユーザブートモード起動対応 Append End
// RevNo111128-001 Append Start
extern enum FFWRX_STAT_ENDIAN GetMdeRegData();
extern void SetMdeRegData(enum FFWRX_STAT_ENDIAN eEndian);
// RevNo111128-001 Append End
extern void InitFfwIfMcuData_Mcu(void);	///< MCU依存コマンド用FFW内部変数の初期化

#endif	// __FFWMCU_MCU_H__
