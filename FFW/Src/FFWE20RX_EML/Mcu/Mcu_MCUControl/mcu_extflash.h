///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_extflash.h
 * @brief ターゲットMCU制御関数(外部Flashダウンロード)のヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, H.Akashi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/11/06
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-007	2012/10/31 明石
　外部フラッシュ ID不一致時の値返送
・RevRxNo121122-005 2012/11/22 明石
  FFWMCUCmd_WRITEで、外部フラッシュメモリ　メーカIDデバイスID不一致エラー発生後、FFWCmd_GetErrorDetailでエラー詳細情報を
  取り込んだ後、FFWE20Cmd_DWNP_CLOSEを実行すると外部フラッシュメモリ メーカIDデバイスID不一致エラー発生するが、
  この時FFWCmd_GetErrorDetailでエラー詳細情報が取り込めないという不具合の修正
*/
#ifndef	__MCU_EXTFLASH_H__
#define	__MCU_EXTFLASH_H__

#include "ffw_typedef.h"
#include "mcudef.h"
// RevRxNo121122-005	Append Line
#include "ffw_sys.h"
// define定義

// 構造体定義

// RevRxNo120910-007	Append Start
typedef struct st_extf_idsnum {				//	フラッシュメモリ メーカID デバイスID(リード値・期待値)
	BYTE	m_makerIDread[4];				//	メーカIDのリード値
	BYTE	m_makerIDexpected[4];			//	メーカIDの期待値
	BYTE	m_deviceIDread[4];				//	デバイスIDのリード値
	BYTE	m_deviceIDexpected[4];			//	デバイスIDの期待値
} FFW_EXTF_IDSNUM;
typedef struct st_jdec_com {				// フラッシュメモリ メーカID/デバイスID の読み出しコマンドテーブル
	DWORD	m_offsetAddr;					// 読み出しコマンド用オフセットアドレス
	BYTE	m_data[4];						// 読み出しコマンド用データ
} FFW_JDEC_COM;
// RevRxNo120910-007	Append End
// 外部フラッシュダウンロード用定義
#define MAX_USD_FILE				0x04	// 外部フラッシュダウンロードで扱える最大USDファイル数 //
#define	MAX_EXTROM_SECTOR			4096	// 外部Flash登録セクタ数 //
#define MAX_SCRIPT_1LINE_DATA		256
#define	EXTFBLOCKCACHE_BUFFSIZE		0x00080000		// 1単位 64KB*8とする
#define	EXTFBLOCKCACHE_MASK			0x000EFFFF	// 書き込みデータ格納位置判断用マスク値
#define	WORKRAM_BUFFSIZE			0x2400		// ワークRAM領域の最大値
#define EFLASHROM_WRITESIZE			0x100		// 外部フラッシュ書き込み単位
#define EFLASHROM_WRITEPROG_OFFSET	0x00		// ワークRAMの先頭からのオフセット値(書き込みプログラム開始位置)

#define	EFLASHROM_GAGE_LEN			0x100
#define EFLASHROM_UNLOCK_ERASE		0x00
#define EFLASHROM_ERASE				0x01
#define EFLASHROM_WRITE				0x02
#define EFLASHROM_STATUS_CHK		0x03
#define EFLASHROM_CHIP_ERASE		0x04
#define EFLASHROM_CMP_ID			0x05
#define EFLASHROM_EXIT				0x06

#define EFLASHROM_MCU32_ROM32		0x00
#define EFLASHROM_MCU32_ROM16_2		0x01
#define EFLASHROM_MCU32_ROM8_4		0x02
#define EFLASHROM_MCU16_ROM16		0x03
#define EFLASHROM_MCU16_ROM8_2		0x04
#define EFLASHROM_MCU8_ROM8			0x05

#define EFLASHROM_RAM_BF_SCRIPT		0x00
#define EFLASHROM_RAM_AF_SCRIPT		0x01
#define EFLASHROM_ROM_BF_SCRIPT		0x02
#define EFLASHROM_ROM_AF_SCRIPT		0x03

// ExtFlashModule_005 Modify Start
// イレーズステータスチェック結果はエラーコードとは別に定義したため値を変更。
#define EFLASHROM_IN_ERASE			0x0000
#define EFLASHROM_ERASE_FINISH		0x0001
// ExtFlashModule_005 Modify End

#define EFLASHROM_CHECKSUM_OFFSET	0x10
#define EFLASHROM_NO_UNLOCK			0x00
#define EFLASHROM_UNLOCK			0x01

#define EFLASHROM_ROME_RAME			0x01

// ExtFlashModule_002 Append Start
#define EFLASHROM_COMMAND_00		0x00			
#define EFLASHROM_COMMAND_AA		0xAA			
#define EFLASHROM_COMMAND_55		0x55			
#define EFLASHROM_COMMAND_UNLOCK	0x48			
#define EFLASHROM_COMMAND_VERIFY	0x58			
#define EFLASHROM_STATUS_LOCKCHK	0x01	// ExtFlashModule_002a Modify Line
#define EFLASHROM_COMMAND_CUI_STAT	0x70
#define EFLASHROM_STATUS_BLOCK		0x02	// ExtFlashModule_002a Modify Line
#define EFLASHROM_STATUS_COMPLETE	0x80	// ExtFlashModule_002a Modify Line

#define EXTF_CHIPERASE_INFO_MAX		1	// チップ消去指定最大値
#define EXTF_SECTERASE_INFO_MAX		1	// セクタ消去指定最大値
#define EXTF_WORKRAM_TYPE_MAX		1	// ワークRAM領域種別最大値
#define EXTF_FASTWRITEMODE_MAX		2	// 高速書き込みモード情報最大値
#define EXTF_UNLOCK_MAX				1	// ロックビット解除情報最大値
#define EXTF_CONNECT_MAX			5	// 接続形態情報最大値
#define EXTF_WPROG_TYPE_MAX			1	// 書き込みプログラムタイプ最大値

#define EXTF_WRITEPROG_JEDEC		0	// 書き込みプログラム:JEDEC方式
#define EXTF_WRITEPROG_CUI			1	// 書き込みプログラム:CUI方式
// ExtFlashModule_002 Append End

// V.1.02 No.24 外部フラッシュダウンロード禁止対応(ExtFlashModule_012) Append Start
#define EXTF_RMW_DOWNLOAD			0x00	// 上書きダウンロード
#define EXTF_ERASE_DOWNLOAD			0x01	// イレーズ後ダウンロード
#define EXTF_NOT_DOWNLOAD			0x02	// ダウンロード禁止
// V.1.02 No.24 外部フラッシュダウンロード禁止対応(ExtFlashModule_012) Append End

// 外部フラッシュダウンロード用構造体定義
typedef	struct {
	WORD	wSectNum;						// キャッシュ登録セクタ番号
	BYTE	bCacheChange;					// キャッシュ変更状態	変更：TRUE、変更無：FALSE
	BYTE	bCacheSet;						// キャッシュ登録状態、TRUE：登録、FALSE：未登録
	DWORD	dwStartAddr;					// キャッシュ登録先頭アドレス
	DWORD	dwSize;							// キャッシュ登録サイズ
	BYTE	*bCacheData;
} EXTFLASH_ROMCACHE_RX[MAX_EXTROM_SECTOR];

// RevRxNo120910-007	Append Start

// 外部フラッシュメモリアクセスデータサイズ
#define	EXTF_DATA_BYTE				1		// 外部フラッシュメモリアクセスデータサイズ (1byte)
#define	EXTF_DATA_WORD				2		// 外部フラッシュメモリアクセスデータサイズ (2byte)
#define	EXTF_DATA_DWORD				4		// 外部フラッシュメモリアクセスデータサイズ (4byte)

// エラー詳細情報テーブル
enum FFWENM_EFLASH_ID_ERROR {				// 外部フラッシュROM のメーカ・デバイスID不一致エラーのエラー詳細情報番号エラー
	EFLASHERR_TS_STARTADDR = 0,				// 外部フラッシュ先頭アドレス
	EFLASHERR_LS_ENDADDR,					// 外部フラッシュ終了アドレス
	EFLASHERR_MAKERID_READ,					// メーカIDのリード値
	EFLASHERR_MAKERID_EXP,					// メーカIDの期待値
	EFLASHERR_DEVICEID_READ,				// デバイスIDのリード値
	EFLASHERR_DEVICEID_EXP,					// デバイスIDの期待値
	EFLASHERR_MAX							// 詳細情報数
};
// RevRxNo120910-007	Append End

// グローバル関数の宣言

extern FFWERR ExtMemWrite(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, enum FFWENM_MACCESS_SIZE eAccessSize,
					   const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr, DWORD *dwLen);
																	///< 外部フラッシュメモリのライト処理
extern FFWERR ExtWriteFlashStart(MADDR madrStartAddr, MADDR madrEndAddr);
																	///< 外部フラッシュROM領域へのWRITE処理開始
extern FFWERR ExtWriteFlashEnd(void);								///< 外部フラッシュROM領域へのWRITE処理終了
extern FFWERR ExtWriteFlashData(enum FFWENM_VERIFY_SET eVerify, FFW_VERIFYERR_DATA* pVerifyErr);

extern void InitExtFlashWrite(void);
extern BOOL GetExtFlashWriteFlg(void);
extern enum FFWENM_VERIFY_SET GetExtFlashVerify(void);
extern void COM_WaitMs(DWORD);

extern void CheckExtFlashRomArea(MADDR madrStartAddr, MADDR madrEndAddr, MADDR* pmadrAreaEnd, BOOL* pbExtFlashRomArea);

// ExtFlashModule_002b Append Start
extern void ClrExistLockSectFlg(void);
extern BOOL GetExistLockSectFlg(void);
// ExtFlashModule_002b Append End

extern FFWERR GetMcuExtCsEndian(DWORD dwCnt,WORD* pwEndianState);

extern void InitMcuData_Extflash(void);						///< ターゲットMCU制御関数用変数初期化

// RevRxNo121122-005	Append Line
extern	FFW_ERROR_DETAIL_INFO* GetExtFlashErrorDetailInfo(void);	///< デバイスID メーカID不一致エラーの詳細情報を取得

#endif	// __MCU_EXTFLASH_H__
