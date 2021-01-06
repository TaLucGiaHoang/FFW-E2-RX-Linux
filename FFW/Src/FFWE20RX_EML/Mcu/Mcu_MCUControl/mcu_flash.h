///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_flash.h
 * @brief ターゲットMCU(内蔵Flash)制御関数のヘッダファイル
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, K.Uemori, S.Ueda, Y.Kawakami, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/05/11
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo111121-007	2012/07/11 橋口
  ・IsMcuFcuFirmArea() を新規追加。
・RevRxNo120606-008 2012/07/13 橋口
　・USBブートコードではない場合でもUSBブートエラーを出してしまうため、修正
・RevRxNo120910-008	2012/11/01 三宅
  ・MAX_FLASH_TRANS_SIZE の定義追加。
  ・DF_READENA_BLK_NUM_TYPE0～2 の定義で「;」があったのを削除。
・RevRxNo120910-004	2012/11/01 三宅
　・GetOfs1Lvd1renChangeFlgPtr()の関数宣言追加。
・RevRxNo120910-005	2012/11/12 明石
　ユーザブートモード起動時のUSBブート領域非消去指定対応
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121022-001	2012/11/27 SDS 岩田
   EZ-CUBE PROT_MCU_FWRITE()分割処理対応
・RevRxNo121206-001 2012/12/06 植盛
  ブートスワップ時のキャッシュ処理対応
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001 2013/04/26 上田
	RX64M対応
・RevRxNo130301-002 2013/11/18 上田
	RX64M MP対応
・RevRxNo140109-001 2014/01/17 上田
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo140515-011 2014/07/14 川上
	RX64M IDコード ALL F以外のユーザブート起動対応
・RevRxNo130730-001 2014/07/17 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140617-001	2014/06/26 大喜多
	TrustedMemory機能対応
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
・RevRxNo150827-002 2015/08/28 PA 紡車
	RX651オプション設定メモリアドレス、配置変更対応
・RevRxNo150827-003 2015/12/01 PA 紡車
	RV40F Phase2対応
・RevRxNo160527-001 2016/06/10 PA 辻
	RX651 正式対応
・RevRxNo161003-001 2016/12/01 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxNo170511-001 2017/05/11 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応不具合修正
*/
#ifndef	__MCU_FLASH_H__
#define	__MCU_FLASH_H__

#include "ffw_typedef.h"
#include "mcudef.h"

// define定義

#define FLASHROM_WRITESIZE		0x100

// RevRxNo130301-001 Append Start
// Extra領域書き換え用定義
#define EXTRA_A_PROG_SIZE	16	// ExtraA領域書き換え単位(16バイト)
#define EXTRA_A_WRITE_FLG_NUM	(MCU_EXTRA_A_AREA_SIZE_RX650 / EXTRA_A_PROG_SIZE)	// Extra領域書き換えバイト数単位の書き換えフラグ数	// RevRxNo150827-002 Modify Line
// RevRxNo130301-001 Append End

// V.1.02 No.8 フラッシュROMブロック細分化対応 Modify Line
#define FILLDATA_CACHEBLOCK_MAX		1024		// FILL対象フラッシュブロック最大数( RX630データフラッシュブロック最大数 )
#define FILLDATA_CACHEBUFF_MAX		0x2000	// FILL対象フラッシュデータ最大数

// V.1.02 新デバイス対応( レジスタ数増加に対応（多めに確保) ) Modify Start 
#define MAX_FLASHACCESS_SFR_BYTE	8	// フラッシュアクセス時に書き換わるSFRレジスタ数(バイトアクセス)
//RevNo010804-001 Append Line
#define MAX_FLASHACCESS_SFR_WORD	24	// フラッシュアクセス時に書き換わるSFRレジスタ数(ワードアクセス)
#define MAX_FLASHACCESS_SFR_LWORD	4	// フラッシュアクセス時に書き換わるSFRレジスタ数(ロングワードアクセス)
// V.1.02 新デバイス対応( レジスタ数増加に対応（多めに確保) ) Modify End 

// RevRxNo130411-001 Delete：データフラッシュリード許可レジスタの1ビットに対応するブロック数定義をmcudef.hへ移動

// RevRxNo120910-008 Append Line
#define MAX_FLASH_TRANS_SIZE	0x10000		// ユーザマットのフラッシュライト時のBFWへ送信の最大サイズ(64K)

// 構造体定義
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete
// 旧キャッシュメモリ管理構造体削除

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Line
#define FCLR_BLKNUM_MAX_RX				3073	// FCLRコマンドのフラッシュブロック数の最大値(ユーザマット(2048)、データマット(1024)、ユーザーブート(1)合わせて)
// フラッシュメモリ初期化ブロック格納構造体
typedef struct {
	DWORD	dwNum;								// フラッシュブロックアドレス数
	DWORD	dwmadrBlkStart[FCLR_BLKNUM_MAX_RX];	// フラッシュブロックの先頭アドレス
	BYTE	byFlashType[FCLR_BLKNUM_MAX_RX];	// フラッシュブロックの種類( 0:コードフラッシュ、1:コードフラッシュ以外 )
} FFW_FCLR_DATA_RX;
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減 Append Start
#define CACHE_BLOCK_AREA_NUM_RX				32	// ブロック領域の設定ブロック数
// ユーザーマット用キャッシュメモリ管理構造体
#define	MATCH_USBBOOT			0xFF		// RevRxNo120606-008	Append line
// UBコード比較用フラグ設定値(マッチ設定)
#define	MATCH_ARGBOOT			0xFF		// RevRxNo120910-005	Append line

typedef struct{
	BYTE	*pbyCacheMem;				// キャッシュメモリ
	BYTE	*pbyFormerMem;				// 差分ダウンロード用メモリ
	BYTE	*pbyCacheSetFlag;			// キャッシュメモリ データ格納状態、格納済み：TRUE、未格納：FALSE
	BYTE	*pbyFormerSetFlag;			// 差分ダウンロード用メモリ データ格納状態、格納済み：TRUE、未格納：FALSE
	BYTE	*pbyBlockWriteFlag;			// ブロック書き換え有無、書き換え有り：TRUE、書き換えなし：FALSE
	BYTE	*pbyBlockAreaWriteFlag;		// ブロック領域書き換え有無、書き換え有り：TRUE、書き換えなし：FALSE
} USER_ROMCACHE_RX;

// データマット用キャッシュメモリ管理構造体
typedef struct{
	BYTE	*pbyCacheMem;				// キャッシュメモリ
	BYTE	*pbyFormerMem;				// 差分ダウンロード用メモリ
	BYTE	*pbyCacheSetFlag;			// キャッシュメモリ データ格納状態、格納済み：TRUE、未格納：FALSE
	BYTE	*pbybFormerSetFlag;			// 差分ダウンロード用メモリ データ格納状態、格納済み：TRUE、未格納：FALSE
	BYTE	*pbyBlockWriteFlag;			// ブロック書き換え有無、書き換え有り：TRUE、書き換えなし：FALSE
	BYTE	*pbyBlockAreaWriteFlag;		// ブロック領域書き換え有無、書き換え有り：TRUE、書き換えなし：FALSE
	BYTE	*pbyProgSizeWriteFlag;		// 書き込みサイズ単位書き換え有無、書き換え有り：TRUE、書き換えなし：FALSE
	BYTE	*pbyDataFlashInitFlag;		// FCLR対象領域初期化実行状態、実行済み：TRUE、未実行：FALSE
} DATA_ROMCACHE_RX;

// ユーザーブートマット用キャッシュメモリ管理構造体
typedef struct{
	BYTE	*pbyCacheMem;				// キャッシュメモリ
	BYTE	*pbyFormerMem;				// 差分ダウンロード用メモリ
} USERBOOT_ROMCACHE_RX;
// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End


// enum定義
// V.1.02 No.22 書き込みプログラムダウンロード化 Modify Start
// ロードするFlash書き込みプログラムの種別
enum FFWRX_WTR_NO {
	WTR_NOTUSE = 0,
	WTR_RX610,
	WTR_RX62N,
	WTR_RX630,
	WTR_RX210
};
// V.1.02 No.22 書き込みプログラムダウンロード化 Modify Start

// RevRxE2LNo141104-001 Append Start(FFWE20RX600.hから移動)
enum FFWRXENM_INIT_AREA_TYPE {	// ターゲットMCUの領域指定
	INIT_UM=0,						// ユーザーマット
	INIT_DM_UBM						// データマット/ユーザーブートマット
};
// RevRxE2LNo141104-001 Append End


// グローバル関数の宣言

extern FFWERR WriteFlashStart(enum FFWENM_VERIFY_SET eVerify);					///< フラッシュROM領域へのWRITE処理開始
extern FFWERR WriteFlashEnd(void);						///< フラッシュROM領域へのWRITE処理終了

extern FFWERR WriteFlashData(enum FFWENM_MACCESS_SIZE eAccessSize, enum FFWENM_VERIFY_SET eVerify, FFW_VERIFYERR_DATA* pVerifyErr);

extern FFWERR WriteFlashExec(enum FFWENM_VERIFY_SET eVerify);		///< フラッシュROM領域へのWRITE処理実行(開始～終了)
extern FFWERR EraseFlashRomBlock(void);								///< フラッシュROMブロックをクリア(FCLRで登録したブロックのみ)

// V.1.02 RevNo110308-002 Append Start
extern void ClrMemAccWarningFlg(void);
extern BOOL GetDtfMcurunFlg(void);
extern void SetDtfMcurunFlg(BOOL bDFBanBlockRead);
extern BOOL GetFlashDbgMcurunFlg(void);
extern void SetFlashDbgMcurunFlg(BOOL bFlashChangeRead);
// V.1.02 RevNo110308-002 Append End

#include "domcu_mcu.h"
extern BOOL GetFlashRomBlockInf( enum FFWRXENM_MAREA_TYPE eAreaType, MADDR madrStartAddr, MADDR *pdwBlkStartAddr, MADDR *pdwBlkEndAddr, DWORD *pdwBlkNo, DWORD *pdwBlkLength); //指定アドレス ブロック情報取得
extern BOOL GetFlashRomBlockNoInf( enum FFWRXENM_MAREA_TYPE eAreaType, DWORD dwBlockNo, MADDR *pdwBlkStartAddr, MADDR *pdwBlkEndAddr, DWORD *pdwBlkLength); //指定ブロック情報取得
extern BOOL GetDmProgSizeWriteFlagInf(MADDR madrStartAddr, DWORD *pdwStartBlkNo); //指定アドレス 書き込みサイズ書き込みフラグの開始ブロック番号を情報取得

 
extern FFWERR GetFentryrRegData(WORD *pwRegData);	// フラッシュP/Eモードエントリレジスタ値取得

// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append Line
extern FFWERR SetRegDataFlashReadEnable(BOOL bEnable);
extern void ClrResultCheckSum(void);												///< チェックサム結果をクリア
extern void GetCheckSum(FFW_DWNP_CHKSUM_DATA* pChecksumData);		///< チェックサム結果を取得
extern FFW_DWNP_CHKSUM_DATA* GetChecksumDataPt(void);
extern FFWERR SetResultCheckSum(void);												///< チェックサム結果を格納

extern BOOL ChkDataFlashRomFclr(void);

extern BOOL GetStateFlashRomCache(void);							///< フラッシュROMキャッシュ変更状態を取得

// V.1.02 新デバイス対応 Append Start
extern void ClrMdeUbcodeChaneFlg(void);
extern BOOL GetEndianChangeFlg(void);
extern BOOL GetUbcodeChangeFlg(void);
// V.1.02 新デバイス対応 Append End
// RevRxNo161003-001 Append Start
extern void ClrBankmdBankselChaneFlg(void);
extern BOOL GetBankModeChangeFlg(void);
extern BOOL GetBankSelChangeFlg(void);
extern BOOL GetBankSelNotChangeFlg(void);
// RevRxNo161003-001 Append End

extern FFWERR GetRegDataFlashReadBlockEnable(DWORD dwBlockNo,BOOL *bResult);	// 対象ブロックのデータフラッシュ許可レジスタ確認


// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Dalete
// 旧フラッシュROMデータキャッシュメモリ関連、FILL対象データキャッシュメモリ関連変数、関数削除

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Line
extern USER_ROMCACHE_RX			*g_UserRomCacheTop;
extern DATA_ROMCACHE_RX			*g_DataRomCacheTop;
extern USERBOOT_ROMCACHE_RX		*g_UserBootRomCacheTop;

extern USER_ROMCACHE_RX* GetUserMatCacheData(void);				// ユーザーマットキャッシュメモリ構造体情報取得
extern DATA_ROMCACHE_RX* GetDataMatCacheData(void);				// データマットキャッシュメモリ構造体情報取得
extern USERBOOT_ROMCACHE_RX* GetUserBootMatCacheData(void);		// ユーザーブートマットキャッシュメモリ構造体情報取得
extern BYTE* GetExtraCacheAddr(DWORD dwCacheCnt);				// Extra領域キャッシュメモリアドレス取得　RevRxNo140515-011 Append Line				

extern FFWERR UpdateFlashRomCache(enum FFWRXENM_MAREA_TYPE eAreaType, DWORD dwBlkNo, MADDR madrBlkStartAddr, MADDR madrBlkEndAddr, DWORD dwBlkSize); // 1ブロック分のキャッシュメモリを更新
extern FFWERR SetFlashRomCacheMem(enum FFWRXENM_MAREA_TYPE eAreaType, MADDR madrStartAddr, DWORD dwAccessCount, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, DWORD dwWriteDataSize); // キャッシュメモリにライトデータ設定

extern FFWERR GetFlashCacheRestVect(enum FFWRXENM_PMODE ePmode);
extern FFWERR GetFlashCacheRestVectDual(enum FFWRXENM_PMODE ePmode);	// RevRxNo170511-001 Append Line
extern FFWERR ClrFlashCacheOfsDbg2Sng(enum FFWENM_ENDIAN eMcuEndian );
extern FFWERR SetFlashCacheOfsDbg(enum FFWENM_ENDIAN eMcuEndian );
extern FFWERR SetFlashCacheOfsLvd(enum FFWENM_ENDIAN eMcuEndian);	// RevRxNo130730-001 Append Line
extern FFWERR SetFlashCacheMdeEndian(enum FFWRXENM_PMODE ePmode,enum FFWENM_ENDIAN eMcuEndian);
extern FFWERR SetFlashCacheUbcode(enum FFWENM_ENDIAN eMcuEndian,const FFWRX_UBCODE_DATA *pUbcode);

// RevRxNo140109-001 Append Start
extern FFWERR UpdateExtraCache(void);
extern void SetExtraCacheSet(BOOL bCacheSet);
extern BOOL GetExtraCacheSet(void);
// RevRxNo140109-001 Append End
// RevRxNo130301-001 Append Start
extern FFWERR SetExtraCache(enum FFWENM_ENDIAN eMcuEndian);
extern BOOL CheckExtraCacheData(enum FFWENM_ENDIAN eMcuEndian, DWORD dwSetNo, DWORD dwSetData);
extern BOOL CheckExtraCacheMdeEndian(enum FFWENM_ENDIAN eMcuEndian);	// RevRxNo130301-002 Append Line
extern void SetExtraCacheOfsDbg(enum FFWENM_ENDIAN eMcuEndian);
extern void SetExtraCacheOfsLvd(enum FFWENM_ENDIAN eMcuEndian);		// RevRxNo130730-001 Append Line
extern void ClrExtraCacheOfsDbg2Sng(enum FFWENM_ENDIAN eMcuEndian);
// RevRxNo130301-001 Append End
extern BOOL CheckExtraCacheFawFspr(enum FFWENM_ENDIAN eMcuEndian);	// RevRxNo150827-003 Append Line
// RevRxNo161003-001 Append Start
extern FFWERR CheckExtraCacheMdeBankmd(BOOL* pbBankmdChange, BOOL bPmodCmd);
extern FFWERR CheckExtraCacheBanksel(enum FFWRXENM_BANKSEL eBankSel, BOOL* pbBankselChange, BOOL* pbBankselNotChange, BOOL bPmodCmd);
// RevRxNo161003-001 Append End

extern FFWERR GetRegDataFlashReadBlockEnable(DWORD dwBlockNo,BOOL *bResult);	// 対象ブロックのデータフラッシュ許可レジスタ確認
extern void ClearAllBlockWriteFlg(void);	// 全ブロック書き換えフラグ/ブロック領域書き換えフラグをクリア
extern BOOL ChkDataFlashRomFclr(void);

extern BOOL GetDataFlashInitFlag(void); // データマット初期化実行状態の参照
extern void SetDataFlashInitFlag(BOOL bDataFlashInitFlag); // データマット初期化実行状態の設定
extern BOOL GetDataFlashEraseFlag(void);	 // データマットのみのイレーズ初期化フラグの参照
extern void SetDataFlashEraseFlag(BOOL bDataFlashEraseFlag);	 // データマットのみのイレーズ初期化フラグの設定

extern BOOL GetUmCacheDataInit(void);//ユーザーマットキャッシュメモリ初期化実行状態の参照
extern void SetUmCacheDataInit(BOOL bUmCacheDataInitFlg);//ユーザーマットキャッシュメモリ初期化実行状態の設定

extern BOOL GetUbmCacheDataInit(void);//ユーザーブートマットキャッシュメモリ初期化実行状態の参照
extern void SetUbmCacheDataInit(BOOL bUmCacheDataInitFlg);//ユーザーブートマットキャッシュメモリ初期化実行状態の設定

extern void MaskMdeUbCodeArea(void);

extern BOOL GetFlashRomInitInfo(FFW_FCLR_DATA_RX* pAddFclrData); // フラッシュROM初期化が必要かチェック/初期化情報登録
extern void GetFlashRomCacheData(enum FFWRXENM_MAREA_TYPE eAreaType, MADDR madrStartAddr, DWORD dwDumpNum, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyReadBuff);

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
extern void NewFlashRomCacheMem(const FFWRX_MCUAREA_DATA* pMcuArea);	// キャッシュメモリ構造体の領域確保処理
extern void DeleteFlashRomCacheMem(void);						// キャッシュメモリ構造体の領域確保チェック/ 領域開放
extern void InitFlashRomCacheMemFlg();							// キャッシュメモリ管理フラグを初期化
extern void InitCacheDataSetFlg(enum FFWRXENM_MAREA_TYPE eAreaType);	// 各領域のキャッシュメモリデータ設定フラグのクリア

extern BOOL GetNewCacheMem(enum FFWRXENM_MAREA_TYPE eAreaType);	// キャッシュメモリ確保フラグ参照
extern void SetNewCacheMem(enum FFWRXENM_MAREA_TYPE eAreaType,BOOL bNewCacheMem);	// キャッシュメモリ確保フラグ設定
extern MADDR GetCacheStartAddr(enum FFWRXENM_MAREA_TYPE eAreaType); // キャッシュメモリMCU先頭アドレス取得
extern MADDR GetCacheEndAddr(enum FFWRXENM_MAREA_TYPE eAreaType); // キャッシュメモリMCU終了アドレス取得
extern DWORD GetCacheBlockNum(enum FFWRXENM_MAREA_TYPE eAreaType); // キャッシュメモリMCUブロック数取得
extern DWORD GetCacheProgSize(enum FFWRXENM_MAREA_TYPE eAreaType); // キャッシュメモリMCUブロック数取得

extern BOOL GetIdcodeChange(void);						// IDコード書き換えフラグ取得　RevRxNo140515-011 Append Line
extern void SetIdcodeChange(BOOL bIdcodeChange);		// IDコード書き換えフラグ設定　RevRxNo140515-011 Append Line
extern BOOL GetUbmCacheSet(void);						// ユーザーブートマットキャッシュメモリデータ格納済みフラグ取得
extern void SetUbmCacheSet(BOOL bUbmCacheSet);			// ユーザーブートマットキャッシュメモリデータ格納済みフラグ設定
extern BOOL GetUbmBlockWrite(void);						// ユーザーブートマットブロック書き換えフラグ取得
extern void SetUbmBlockWrite(BOOL bUbmBlockWriteFlag);	// ユーザーブートマットブロック書き換えフラグ設定
// RevRxNo140109-001 Delete: InitExtraCacheMemFlg()

extern BOOL CheckExtraCacheOfsDbg(enum FFWENM_ENDIAN eMcuEndian);	// RevRxNo130730-001 Append Line

// RevRxNo120910-005	Modify Line
extern FFWERR CheckMemUbcode(const FFWRX_UBCODE_DATA *pUbcode, BOOL* pbUbcodeChange,  const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange);	
extern FFWERR CheckOfs1LvdDis(BOOL* pbLvdChange, BYTE byOfs1LvdData); // RevRxNo130730-001 Append Line
																														//UBコードチェック
// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append Start
extern FFWERR  InitFlashRomCacheWriter(void);
// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append End

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End
extern void NewEscWorkRamCacheMem(DWORD dwSize);
extern void DeleteEscWorkRamCacheMem(void);

extern FFWERR SetDbgClkChg(enum FFWRXENM_CLKCHANGE_ENA	eClkChangeEna);

extern BOOL IsMcuFcuFirmArea(MADDR madrAddr);	// RevNo111121-007 Append Line

extern void InitMcuData_Flash(void);						///< ターゲットMCU制御関数用変数初期化
// RevRxNo120910-004 Append Line
extern BOOL* GetOfs1Lvd1renChangeFlgPtr(void);

// RevRxNo121022-001 Append Start
// RevRxNo130301-001 Modify Line
extern FFWERR McuFlashWriteDivide(BYTE byWriteArea, enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, MADDR madrWriteAddr,
						   BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize,
						   BOOL bSameAccessCount, DWORD dwAccessCount, 
						   BOOL bSameWriteData, BYTE* pbyWriteData, 
						   FFW_VERIFYERR_DATA* pVerifyErrorInfo, BYTE byEndian, DWORD dwWriteMaxLength);
// RevRxNo121022-001 Append End

// RevRxNo121206-001 Append Line
extern void SwapFlashRomCacheData(void);
extern FFWERR SwapFlashWriteFlag(void);		// RevRxNo130730-001 Append Line

// RevRxNo140617-001 Append Start
extern void	SetDwnpDataIncludeTMArea(BOOL bInclude);
extern BOOL	GetDwnpDataIncludeTMArea(void);
// RevRxNo140617-001 Append End

extern BOOL* GetFawFsprChangeFlgPtr(void);	// RevRxNo160527-001 Append Line

#endif	// __MCU_FALSH_H__
