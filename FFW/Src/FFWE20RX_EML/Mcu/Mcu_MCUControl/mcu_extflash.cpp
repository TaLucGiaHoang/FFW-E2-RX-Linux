///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_extflash.cpp
 * @brief 外部フラッシュダウンロード　MCU制御
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, H.Akashi, K.Uemori, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/02/07
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・ExtFlashModule_017   2012/07/10 橋口
  ・intRamEnable()
    ・以下レジスタへFILL する際のベリファイ指定変数に
	  初期値が指定されていなかったので、初期値(VERIFY_OFF) を
	  指定するように変更。 →ベース構成改善時に実施済み
        PRCR
        SYSCR1
        MSTPCRC
・RevNo111121-003   2012/07/11 橋口
  ・chkVerifyWriteDataExtFlash()
    ・ベリファイエラー発生時の以下のデータの取得処理を変更。
	  ・ベリファイエラー発生時のアドレス
	    ・アクセスサイズに対して、各アクセスの先頭アドレスと
	      なるように変更。
	  ・ベリファイエラー発生時のリードデータ
	    ・上記アドレスのデータからアクセスサイズ分のデータを
	      取得するように変更。
	  ・ベリファイエラー発生時のライトデータ
	    ・上記アドレスのデータからアクセスサイズ分のデータを
	      取得するように変更。
・ExtFlashModule_016   2012/07/11 橋口
  ・ExtWriteFlashData()
    ・セクタの先頭256 バイトのライトでは、ライトデータが全て0xFF であるかに
	  関わらず、必ずライトするように変更。
・RevNo111215-001	2012/07/11 橋口
  ・resumeWorkRamArea()
    ・SFR アクセス時のベリファイ指定を「ベリファイなし」固定と
	  するように変更。→ベース構成改善時に実施済み
・RevNo120201-001	2012/07/11 橋口
　・chkVerifyWriteDataExtFlash()
　　　ベリファイ結果格納変数初期化処理追加(未初期化メモリリードアクセスの修正)
・ExtFlashModule_018	2012/07/11 橋口 
　・外部フラッシュダウンロードバス設定スクリプト解析処理修正
    → コマンド指定が正しくない場合は"FFWERR_EXTROM_SCRIPT"を返すようにしたが、
	   HEWのエラーメッセージが"xxx用スクリプトが見つかりませんでした。"となるため、
	　 余計な混乱を招くおそれがあるため、今までと同じように無効行として扱うようにした。
・RevRxNo120910-001	2012/09/11 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・ExtWriteFlashStart()で、GetStatData()関数呼び出し時の引数の
    byStatKindをdwStatKindに変更。
・RevNo121017-003	2012/10/31 明石
  VS2008対応エラーコード　WARNING C4996対応変更
・RevRxNo120910-007	2012/11/12 明石
  外部フラッシュ ID不一致時の値返送
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121115-002 2012/11/15 明石
  8bitバス幅の外部フラッシュメモリのメーカーID、デバイスIDの期待値設定処理修正
・RevRxNo121115-003 2012/11/15 明石
  外部フラッシュメモリのデバイスID不一致時のエラー詳細情報16進表記フォーマット修正
・RevRxNo121022-001	2012/11/21 SDS 岩田
	EZ-CUBE PROT_MCU_DUMP()分割処理対応
	EZ-CUBE PROT_MCU_WRITE()分割処理対応
・RevRxNo121122-005 2012/11/22 明石(2012/12/04 SDS 岩田 マージ)
  FFWMCUCmd_WRITEで、外部フラッシュメモリ　メーカIDデバイスID不一致エラー発生後、FFWCmd_GetErrorDetailでエラー詳細情報を
  取り込んだ後、FFWE20Cmd_DWNP_CLOSEを実行すると外部フラッシュメモリ メーカIDデバイスID不一致エラー発生するが、
  この時FFWCmd_GetErrorDetailでエラー詳細情報が取り込めないという不具合の修正
・RevRxNo121213-001 2012/12/13 明石(2012/12/04 SDS 岩田 マージ)
  JEDEC 16-8×2接続時のデバイスIDリード不具合対応
・RevNo130117-001	2013/01/18 上田
　外部フラッシュダウンロード メモリリーク修正
・RevNo130118-001	2013/01/21 上田
　外部フラッシュダウンロード用メモリ解放処理変更
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001 2013/06/10 植盛
	RX64M対応
・RevRxNo130730-010 2013/11/15 大喜多
　　getExtFlashID_cui()のforループ処理修正
・RevRxNo130730-009 2013/11/15 大喜多
　　mallocでメモリが確保できなかった場合にエラーを返す処理を追加
・RevRxNo130730-007 2013/11/15 大喜多
	GetStatData()の戻り値がFFWERR_OKの場合のみpeStatMcuを参照するようにする
・RevRxNo130730-011 2014/02/07 大喜多、上田
	isdigit(),isxdigit(), isupper()の引数をchar型からint型に変更
*/
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif

#include "ffw_sys.h"
#include "mcu_extflash.h"
#include "mcu_flash.h"
#include "mcu_sfr.h"
#include "mcu_mem.h"
#include "ffwmcu_mcu.h"
#include "ffwmcu_dwn.h"
#include "ffwmcu_extflash.h"
#include "domcu_mcu.h"
#include "domcu_mem.h"
#include "domcu_extflash.h"
#include "domcu_rst.h"
#include "protmcu_extflash.h"
#include "protmcu_mem.h"
#include "protmcu_mcu.h"
#include "mcuspec.h"
#include "do_sys.h"		// RevRxNo121022-001 Append Line
#include "mcudef.h"	// RevRxNo130411-001 Append Line


//--------------------------------------------------------------------------------------------------------------
// static変数(外部フラッシュダウンロード用)
static EXTFLASH_ROMCACHE_RX	s_ExtFlashRomCache;				// 外部フラッシュROMデータキャッシュ
static BOOL		s_bExtWriteFlashStartFlg[MAX_USD_FILE];		// 外部フラッシュダウンロード前準備情報　FALSE:準備未、TRUE:準備完
static BOOL		s_bExtFlashDataGetFlg;						// 外部フラッシュメモリデータ取得情報
static BYTE		s_byDiffIDFlg[MAX_USD_FILE];				// メーカID/デバイスID比較済み判断変数
static BOOL		s_byExtFlashWriteFlg;						// 外部フラッシュ領域への書き込み実行済み情報
static BYTE		s_byUsdNum;									// 外部フラッシュダウンロード領域がどのUSDファイルに存在しているかを示す
static WORD		s_wSectNum;									// 外部フラッシュダウンロード領域がどのセクタに存在しているかを示す
static BYTE		s_byLastUsdNum;								// 前回ExtMemWrite処理の外部フラッシュダウンロード領域がどのUSDファイルに存在しているかを示す
static WORD		s_wLastSectNum;								// 前回ExtMemWrite処理の外部フラッシュダウンロード領域がどのセクタに存在しているかを示す
static BYTE		s_bExtIntRamEnableFlg;						// 内蔵RAM有効化判断フラグ(外部フラッシュダウンロード用)
// V.1.02 覚え書き32 RAM停止時の処理対応 Append Line
static BYTE		s_bExtRamStartFlg;							// 内蔵RAM動作化判断フラグ(外部フラッシュダウンロード用)
static BYTE		s_bExtWorkRamResumeFlg;						// ワークRAM領域復帰判断フラグ
static FFWERR	s_ExtFerr;									// 外部フラッシュダウンロードエラー格納変数	
static enum FFWENM_MACCESS_SIZE s_eAccessSize; 				// ワークRAMアクセスサイズ情報
static enum FFWENM_VERIFY_SET	s_ExteVerify;
															// FALSE : 未実行、 TRUE : 実行済み
static DWORD	s_dwExtGageTotal;
static DWORD	s_dwExtGageCnt;
static DWORD	s_dwExtGageBaseCnt;

static WORD		s_wExtRamEnableData;		// RAM有効/無効レジスタデータ退避変数(外部フラッシュダウンロード用)
// V.1.02 覚え書き32 RAM停止時の処理対応 Append Start
static DWORD	s_dwExtMstpcrcData;			// モジュールストップコントロールレジスタC値退避変数(外部フラッシュダウンロード用)
// V.1.02 覚え書き32 RAM停止時の処理対応 Append End

// ExtFlashModule_002b Append Start
static BOOL		s_bSectLock[MAX_USD_FILE][MAX_EXTROM_SECTOR];	// セクタがロックされているかを格納する変数
static BOOL		s_bExistLockSect;							// ロックされているセクタがあったかを格納する変数
// ExtFlashModule_002b Append End

// ExtFlashModule_007 Append Line
static DWORD	s_dwExtWriteDataGageCnt;					// ExtWriteFlashData(セクタイレーズ/ライト処理)の進捗ゲージカウンタ値

// ExtFlashModule_008 Append Line
static BOOL		s_bSendProtEfwriteStart;					// PROT_MCU_EFWRITESTART()送信済みかを示すフラグ

// RevRxNo121122-005	Append Line
static FFW_ERROR_DETAIL_INFO		s_ferdiData;					// メーカID不一致エラーの詳細情報テーブル


// static関数(外部フラッシュダウンロード用)
static FFWERR cwriteDmaAccessExtFlash(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							 enum FFWENM_MACCESS_SIZE eAccessSize,  const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);
static FFWERR chkVerifyWriteDataExtFlash(MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, 
								const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr, BYTE byEndian);
static BOOL setExtFlashDataSetSect(MADDR, MADDR, BYTE *, WORD *);
static FFWERR intRamEnable(MADDR, MADDR);
static FFWERR resumeWorkRamArea(void);
static FFWERR chipErase(BYTE byEndian);
static FFWERR cuiStatusCheck(WORD wSect, BOOL *bResult);
static FFWERR jedecUnlock(WORD wSect, BOOL *bResult);

// ExtFlashModule_002b Append Start
static FFWERR jedecVerifyLockbit(WORD wSect, BOOL* pbLockStatus);
static FFWERR checkJedecLockBit(void);
// ExtFlashModule_002b Append End

// static関数( スクリプトファイル解析用 )
static FFWERR exeScriptCmd(BYTE bUsdNum, BYTE bMode);	// スクリプトファイル解析&コマンド実行
static BOOL setArgument(char *);		// コマンド文字列解析 & 引数分割
static char *cutCmd(char *);			// コマンド抽出
static BOOL cutArg(char *);				// 引数分割
// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete Line
//static char *cutSpace(char *);			// 空白文字削除
static UCHAR get_hex(char *, DWORD *, short);	// ASCII文字をHexに変換

// RevRxNo120910-007	Append Start
static	FFWERR getExtFlashID(BYTE	byUsdNum, FFW_ERROR_DETAIL_INFO* pErrDetailInfo);	// 外部フラッシュメモリ メーカID デバイスID取得関数
static	FFWERR getExtFlashID_cui(BYTE	byUsdNum, FFW_EXTF_IDSNUM* pdata);			// CUI方式外部フラッシュメモリ用
static	FFWERR getExtFlashID_jdec(BYTE	byUsdNum, FFW_EXTF_IDSNUM* pdata);			// JDEC方式外部フラッシュメモリ用
// RevRxNo120910-007	Append End

// グローバル変数( スクリプトファイル解析用 )		2009.5.8 by ookita
BOOL Argc;								// コマンド、引数の総数
char *Argv[256];						// コマンド、各引数の文字列

// static変数( 外部フラッシュダウンロード用 )
static BYTE	s_byWorkRam[WORKRAM_BUFFSIZE];
static BYTE	*s_byExtRomCache;						// 外部フラッシュ1セクタ分のキャッシュ領域へのポインタ
// RevNo130118-001 Append Line
static BOOL s_bNewExtRomCache;						// 外部フラッシュ1セクタ分のキャッシュ領域確保フラグ
//--------------------------------------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// メモリ操作関連関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
 * << 外部フラッシュへのイレーズ・ライト処理事前準備関数 >>
 * 外部フラッシュダウンロード実行前処理を行う。
 *   (1) MCUステータスチェック
 *   (2) 実行前スクリプト実行
 *   (3) ワークRAM退避
 *   (4) 書き込みプログラムをワークRAMにライト
 *   (5) チップイレーズ実行(全セクタ消去指定がある場合のみ)
 * @param madrStartAddr		外部フラッシュダウンロード開始アドレス
 * @param madrEndAddr		外部フラッシュダウンロード終了アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ExtWriteFlashStart(MADDR madrStartAddr, MADDR madrEndAddr)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTotalLength;
	DWORD	dwCnt;
	BYTE	byEndian;
	BYTE	*pbyReadData;
	BYTE	byReadData[WORKRAM_BUFFSIZE];			// 参照データ格納領域
	DWORD	dwmadrProgStartAddr;
	DWORD	dwmadrProgEndAddr;
	FFW_VERIFYERR_DATA VerifyErr;
	DWORD	dwLength;
	DWORD	dwAccessCount;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BOOL	bSameWriteData;
	BYTE	*pbyReadData2;
	BYTE	byReadData2[WORKRAM_BUFFSIZE];			// 参照データ格納領域2
	MADDR	madrWorkEndAddr;
// ExtFlashModule_002b Append Line
	WORD	wCnt;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
	dwAccessCount = 0;				// ワーニング対策
// RevRxNo121022-001 Append Start

	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Append Start
	DWORD						dwAreaNum;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Append Start

	pbyReadData2 = reinterpret_cast<BYTE*>(byReadData2);
	pbyReadData = reinterpret_cast<BYTE*>(byReadData);
	dwTotalLength = madrEndAddr - madrStartAddr + 1;
	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;		//MCU情報

	pMcuInfoData = GetMcuInfoDataRX();

	enum FFWRXENM_STAT_MCU peStatMcu;
	// RevRxNo120910-001 Modify Line
	DWORD dwStatKind;
	FFWRX_STAT_SFR_DATA pStatSFR;
	FFWRX_STAT_JTAG_DATA pStatJTAG;
	FFWRX_STAT_FINE_DATA pStatFINE;
	FFWRX_STAT_EML_DATA pStatEML;

	FFW_DWNP_CHKSUM_DATA*	pChecksumData;			//チェックサム管理変数
	pChecksumData = GetChecksumDataPt();			//チェックサム管理変数アドレス取得

	s_bExtWorkRamResumeFlg = FALSE;					// ワークRAM復帰する必要なしに設定

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
			s_ExtFerr = FFWERR_BMCU_RESET;
			goto err;
		case RX_MCUSTAT_SLEEP:
			s_ExtFerr = FFWERR_BMCU_SLEEP;
			goto err;
		case RX_MCUSTAT_SOFT_STDBY:
			s_ExtFerr = FFWERR_BMCU_STANBY;
			goto err;
		case RX_MCUSTAT_DEEP_STDBY:
			s_ExtFerr = FFWERR_BMCU_DEEPSTANBY;
			goto err;
		}
	}

	// (2) 外部アクセス不可状態の場合、
	//      a) 外部RAM用前スクリプト実行
	if( e_ExtfInfo[s_byUsdNum].byWorkRamType ){		// 外部RAM使用時
		if( (ferr = exeScriptCmd(s_byUsdNum, EFLASHROM_RAM_BF_SCRIPT)) != FFWERR_OK ){	// 実行前外部RAMスクリプト
			s_ExtFerr = ferr;
			goto err;
		}
	}
	//		b) ワークRAM領域退避
	if( e_ExtfInfo[s_byUsdNum].byWorkRamType ){		// 外部RAMの場合
		if( e_ExtfInfo[s_byUsdNum].byWorkAccessSize == MBYTE_ACCESS ){
			s_eAccessSize = MBYTE_ACCESS;
			dwAccessCount = e_ExtfInfo[s_byUsdNum].wWorkSize;
		}else if( e_ExtfInfo[s_byUsdNum].byWorkAccessSize == MWORD_ACCESS ){
			s_eAccessSize = MWORD_ACCESS;
			dwAccessCount = e_ExtfInfo[s_byUsdNum].wWorkSize / 2;
		}else if( e_ExtfInfo[s_byUsdNum].byWorkAccessSize == MLWORD_ACCESS ){
			s_eAccessSize = MLWORD_ACCESS;
			dwAccessCount = e_ExtfInfo[s_byUsdNum].wWorkSize / 4;
		}
	}else{											// 内蔵RAMの場合
		s_eAccessSize = MLWORD_ACCESS;
		dwAccessCount = e_ExtfInfo[s_byUsdNum].wWorkSize / 4;
	}
	madrWorkEndAddr = e_ExtfInfo[s_byUsdNum].dwWorkStart + e_ExtfInfo[s_byUsdNum].wWorkSize -1;
	// ワークRAM指定領域が内蔵RAMかどうかを判断。内蔵RAMの場合、内蔵RAMが有効か無効かをチェックし、
	// 無効の場合は有効にして内蔵RAMに書き込みプログラムをライトする。ダウンロード終了時にRAM無効に戻しておく。
	ferr = intRamEnable(e_ExtfInfo[s_byUsdNum].dwWorkStart, madrWorkEndAddr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify Start
	ferr = GetEndianTypeExtRom(e_ExtfInfo[s_byUsdNum].dwWorkStart,&byEndian);
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}

// RevRxNo121022-001 Append Start
	if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
		ferr = McuDumpDivide(dwAreaNum, e_ExtfInfo[s_byUsdNum].dwWorkStart, bSameAccessSize, s_eAccessSize, bSameAccessCount, dwAccessCount, pbyReadData, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
	} else{
		ferr = PROT_MCU_DUMP(dwAreaNum, &e_ExtfInfo[s_byUsdNum].dwWorkStart, bSameAccessSize, &s_eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadData, byEndian);
	}
// RevRxNo121022-001 Append End

	// V.1.02 覚え書き15の2点目( RevNo101209-02 ) Modify End
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}else{
		s_bExtWorkRamResumeFlg = TRUE;				// ワークRAM復帰する必要あり
		memcpy( s_byWorkRam, pbyReadData, e_ExtfInfo[s_byUsdNum].wWorkSize);	// 取得したワークRAM領域のデータを保存領域に格納
	}

	//      c) 外部ROM用前スクリプト実行
	if( (ferr = exeScriptCmd(s_byUsdNum, EFLASHROM_ROM_BF_SCRIPT)) != FFWERR_OK ){		// 実行前外部ROMスクリプト
		s_ExtFerr = ferr;
		goto err;
	}

	//      d) 書き込みプログラムダウンロード
	dwmadrProgStartAddr = e_ExtfInfo[s_byUsdNum].dwWorkStart + EFLASHROM_WRITEPROG_OFFSET;
	dwmadrProgEndAddr = e_ExtfInfo[s_byUsdNum].dwWorkStart + e_ExtfInfo[s_byUsdNum].wWorkSize - (FLASHROM_WRITESIZE*3) - 1;
	dwLength = dwmadrProgEndAddr - dwmadrProgStartAddr + 1;
	bSameAccessSize = FALSE;
	bSameWriteData = FALSE;

	ferr = GetEndianTypeExtRom(dwmadrProgStartAddr, &byEndian);			// エンディアン判定＆取得
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}
	memcpy(pbyReadData2, (const unsigned char *)&e_ExtfInfo[s_byUsdNum].byWriteProgram, dwLength);
	// cwriteDmaAccess2を使っていたが、内蔵ROMダウンロード用書き込みプログラムをライトするのと同じなので、
	// cwriteDmaAccessを使うようにした。そのため、無駄なデータ並び替えは不要となった。
	// → アクセスサイズがLWORDの場合は並び替え不要だが、BYTE,WORDの場合は並び替え必要。
	// 　　なおかつ、BYTE,WORDの場合はcwriteDmaAccessではベリファイチェックが通らないので、cwriteDmaAccess2に戻す。
	//     → cwriteDmaAccess2()→cwriteDmaAccessExtFlash()に関数名変更
	if (byEndian == FFWRX_ENDIAN_BIG ){
		if( s_eAccessSize == MBYTE_ACCESS){
			ReplaceEndian(pbyReadData2, MLWORD_ACCESS, dwLength);
		}else if( s_eAccessSize == MWORD_ACCESS ){
			ReplaceEndian(pbyReadData2, MLWORD_ACCESS, dwLength);
			ReplaceEndian(pbyReadData2, MWORD_ACCESS, dwLength);
		}
	}
	// V.1.01 No.26 メモリアクセス端数処理対応 Modify End
	ferr = cwriteDmaAccessExtFlash(dwmadrProgStartAddr, dwmadrProgEndAddr, s_ExteVerify, s_eAccessSize, pbyReadData2, &VerifyErr);
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}
	if (VerifyErr.eErrorFlag == VERIFY_ERR) {	
		// ベリファイエラー発生時は、ここで処理を終了する。ファームはエラー値で戻らないが、
		// 上で、VerifayErr.eErrorFlag値を見て、ベリファイエラーとするかそのまま処理を続けるか判断する。
// ExtFlashModule_012 Modify Line
		s_ExtFerr = FFWERR_EXTROM_WPROG_VERIFY;
		goto err;
	}

	s_bExtWriteFlashStartFlg[s_byUsdNum] = TRUE;				// 外部フラッシュダウンロード準備完

// ExtFlashModule_008 Append Line
	s_bSendProtEfwriteStart = FALSE;	// PROT_MCU_EFWRITESTART()未送信に設定


// ExtFlashModule_002b Append Start
	// セクタのロック情報をクリア
	for (dwCnt = 0; dwCnt < MAX_USD_FILE; dwCnt++) {
		if( !e_ExtfInfo[dwCnt].wSectorNum ){	// 未登録の場合
			break;
		}
		for (wCnt = 0; wCnt < e_ExtfInfo[dwCnt].wSectorNum; wCnt++) {
			s_bSectLock[dwCnt][wCnt] = FALSE;	// ロックされていない
		}
	}
// ExtFlashModule_002b Append End

// ExtFlashModule_002 Modify Line
	ferr = chipErase(byEndian);			// 書き込みプログラムのエンディアンをchipErase関数に渡す必要あり
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}

	pChecksumData->byCntMem += 1;		// チェックサム計算メモリ領域加算(USDファイルが追加されるたびに加算される)
	// 進捗ゲージ初期化
	s_dwExtGageTotal = 0;				// 進捗ゲージ初期化
	s_dwExtGageCnt = 0;
	s_dwExtGageBaseCnt = 0;
	dwLength = e_ExtfInfo[s_byUsdNum].dwEndAddr - e_ExtfInfo[s_byUsdNum].dwStartAddr + 1;
	s_dwExtGageTotal = dwLength / EFLASHROM_WRITESIZE;
	s_dwExtGageBaseCnt = (s_dwExtGageTotal / GAGE_COUNT_MAX) + 1;		// 進捗ゲージは1/1000単位で進ませる
	
	// ExtFlashModule_007 Append Line
	s_dwExtWriteDataGageCnt = 0;

	return ferr;
err:
	return ferr;

}


//=============================================================================
/**
 * << 外部フラッシュへのセクタイレーズ・ライト処理終了関数 >>
 * 終了時は正常にデバイスを読み出せるよう以下処理を実施する。
 *   JEDECデバイス：リセットコマンド発行
 *   CUIデバイス：リードアレイコマンド発行
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ExtWriteFlashEnd(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr2 = FFWERR_OK;

// ExtFlashModule_007 Append Start
	s_dwExtWriteDataGageCnt = GAGE_COUNT_MAX;
	SetGageCount(s_dwExtWriteDataGageCnt);
// ExtFlashModule_007 Append End

	if( s_byUsdNum == 0xFF ){						// 外部フラッシュダウンロードなしの場合
		return s_ExtFerr;							// 外部フラッシュへのダウンロード未実行なので何もせず終了
	}
	if( !s_bExtWriteFlashStartFlg[s_byUsdNum] ){	// 外部フラッシュダウンロード準備未の場合
		return s_ExtFerr;							// 外部フラッシュへのダウンロード未実行なので何もせず終了
	}

	if( s_ExtFerr == FFWERR_OK ){					// エラーが発生していない場合
		// BFWMCUCmd_EFWRITEEND送信
		// V.1.02 No.24 外部フラッシュダウンロード禁止対応(ExtFlashModule_012) Append Line
		// ダウンロード禁止指定があった場合はPROT_MCU_EFWRITESTART()を発行していない場合があるので
		if (s_bSendProtEfwriteStart == TRUE) {		// PROT_MCU_EFWRITESTART()送信済みの場合
			ferr = PROT_MCU_EFWRITEEND();
			if (ferr != FFWERR_OK) {
				//退避していたユーザRAMを復帰
				ferr2 = resumeWorkRamArea();
				return ferr;
			}
		}
		//退避していたユーザRAMを復帰
		ferr2 = resumeWorkRamArea();
		if( ferr2 != FFWERR_OK) {
			return ferr2;
		}
		s_wLastSectNum = 0xFFFF;								// 前回処理した最後のセクタ番号初期化
		s_bExtWriteFlashStartFlg[s_byUsdNum] = FALSE;				// 外部フラッシュダウンロード準備未に設定(USD番号初期値)
		s_ExteVerify = VERIFY_OFF;										// ベリファイチェックなし

		// 外部ROM/RAM用後スクリプト実行
		if( e_ExtfInfo[s_byUsdNum].byWorkRamType ){					// 外部RAM使用時
			if( (ferr = exeScriptCmd(s_byUsdNum, EFLASHROM_RAM_AF_SCRIPT)) != FFWERR_OK ){	// 実行後外部RAMスクリプト
				return ferr;
			}
		}
		if( (ferr = exeScriptCmd(s_byUsdNum, EFLASHROM_ROM_AF_SCRIPT)) != FFWERR_OK ){		// 実行後外部ROMスクリプト
			return ferr;
		}
		return ferr;
	}else{											// 既にエラーが発生している場合
		// ExtFlashModule_008 Modify Line
		// PROT_MCU_EFWRITEEND()発行するための判断を変更
		if (s_bSendProtEfwriteStart == TRUE) {		// PROT_MCU_EFWRITESTART()送信済みの場合
			// BFWMCUCmd_EFWRITEEND送信
			ferr = PROT_MCU_EFWRITEEND();			// 終了処理を実行しておく
			resumeWorkRamArea();					// ワークRAM復帰
			s_wLastSectNum = 0xFFFF;								// 前回処理した最後のセクタ番号初期化
			s_bExtWriteFlashStartFlg[s_byUsdNum] = FALSE;				// 外部フラッシュダウンロード準備未に設定(USD番号初期値)
			s_ExteVerify = VERIFY_OFF;										// ベリファイチェックなし
			// 外部ROM/RAM用後スクリプト実行
			if( e_ExtfInfo[s_byUsdNum].byWorkRamType ){					// 外部RAM使用時
				ferr = exeScriptCmd(s_byUsdNum, EFLASHROM_RAM_AF_SCRIPT);
				if(ferr != FFWERR_OK){
					return ferr;
				}
			}
			ferr = exeScriptCmd(s_byUsdNum, EFLASHROM_ROM_AF_SCRIPT);
			if(ferr != FFWERR_OK){
				return ferr;
			}
		}else{										// スクリプト読み込みエラーかステータスエラーの場合
			resumeWorkRamArea();					// ワークRAM復帰
		}
		return s_ExtFerr;
	}

}

//=============================================================================
/**
 * << 外部フラッシュへのセクタイレーズ・ライト処理関数 >>
 * 必要に応じてセクタイレーズし、256バイト単位でデータをライトする。
 * JEDECデバイスの場合のみ、セクタがロックされているかを事前にチェックし、
 * ロックされている場合はライトしないようにする(ロック解除指定なしの場合)。
 * @param eVerify         べリファイ指定(VERIFY_OFF/VERIFY_ON)
 * @param pVerifyErr      ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ExtWriteFlashData(enum FFWENM_VERIFY_SET eVerify, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wBuffDivCnt;						// バッファ書き込み用バッファを256バイトで分割した数を格納
	BYTE	byWriteData[EFLASHROM_WRITESIZE];	// 書き込みデータ格納領域
	DWORD	dwSect, i, n;						// 作業変数
	MADDR	dwmadrWriteStartAddr;
	BYTE	byMode;								// モード格納
	BYTE	byEndian, byExtRomEndian;
	BYTE	*bReadBuff;
	DWORD	dwLength;
	DWORD	dwAccessCount;						// ベリファイ時のDUMPアクセス回数
/// ExtFlashModule_011 E20->E100 Modify Line 
	BYTE	byEraseStat = FALSE;				// イレーズ後ステータスチェック判断用
	enum FFWENM_MACCESS_SIZE eAccessSize;
	FFWMCU_DBG_DATA_RX*	pDbgData;				// デバッグ情報格納構造体へのポインタ
	DWORD	dwChkCache, dwChkData;				// チェックサム計算用変数
	char	byGageMes[EFLASHROM_GAGE_LEN];		// 進捗ゲージ表示文字列格納領域
// ExtFlashModule_002b Append Line
	BOOL	bLockStatus;

	// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
	WORD	wEraseStatus;
	// RevRxNo120910-007	Append Start
	// RevRxNo121122-005	Delete Line
	FFWERR						ferrExt = FFWERR_OK;;	// 外部フラッシュデバイスID取得関連エラーコード
	// RevRxNo120910-007	Append End
	// RevNo121017-003	Append Line
	int							ierr;
	
	//ワーニング対策
	pVerifyErr;
	eVerify;

	FFW_DWNP_CHKSUM_DATA*	pChecksumData;			//チェックサム管理変数
	pChecksumData = GetChecksumDataPt();			//チェックサム管理変数アドレス取得

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
	eAccessSize = MWORD_ACCESS;		// ワーニング対策
	dwAccessCount = 0;				// ワーニング対策
// RevRxNo121022-001 Append Start

	if( s_ExtFerr != FFWERR_OK ){				// 既にエラーが発生している場合
		// RevNo130117-001 Append Start
		// RevNo130118-001 Modify Line
		if(s_bNewExtRomCache == TRUE){				// 確保したキャッシュメモリの解放
			delete [] s_byExtRomCache;

			// RevNo130118-001 Append Line
			s_bNewExtRomCache = FALSE;
		}
		// RevNo130117-001 Append End
		return s_ExtFerr;						// 何もせず戻る
	}

	// byModeの意味
	// #define EFLASHROM_UNLOCK_ERASE		0x00 : ロック解除付きセクタイレーズ
	// #define EFLASHROM_ERASE				0x01 : ロック解除なしセクタイレーズ
	// #define EFLASHROM_WRITE				0x02 : ライト
	// #define EFLASHROM_STATUS_CHK			0x03 : イレーズ後ステータスチェック付きライト
	// #define EFLASHROM_CHIP_ERASE			0x04 : チップイレーズ
	// #define EFLASHROM_CMP_ID				0x05 : メーカ・デバイスID比較
	// #define EFLASHROM_EXIT				0x06 : 終了処理
	// RevNo121017-003	Modify Line
	ierr = sprintf_s(byGageMes, _countof(byGageMes), "External Flash [%02x]", s_byUsdNum );

// ExtFlashModule_007 Append Line
	SetGageCount(s_dwExtWriteDataGageCnt);	// 進捗ゲージカウンタ用変数設定

	// ワークRAM領域のエンディアン確認
	ferr = GetEndianTypeExtRom(e_ExtfInfo[s_byUsdNum].dwWorkStart, &byEndian);	// エンディアン判定＆取得
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}

	// [ 書き込み処理シーケンス ]
	// (1) 書き込みプログラム初期化処理実行
	if( e_ExtfInfo[s_byUsdNum].wBuffSize != EFLASHROM_WRITESIZE ){
		wBuffDivCnt = (WORD)(e_ExtfInfo[s_byUsdNum].wBuffSize / EFLASHROM_WRITESIZE);	// この情報をどのように利用するかは未定
	}
	ferr = PROT_MCU_EFWRITESTART(byEndian, e_ExtfInfo[s_byUsdNum].byConnect, s_eAccessSize, e_ExtfInfo[s_byUsdNum].dwWorkStart, 
					e_ExtfInfo[s_byUsdNum].dwBuffStart, e_ExtfInfo[s_byUsdNum].wBuffSize, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum],
					e_ExtfInfo[s_byUsdNum].wCommandAddr8_1, e_ExtfInfo[s_byUsdNum].wCommandAddr8_2, e_ExtfInfo[s_byUsdNum].wCommandAddr16_1, 
					e_ExtfInfo[s_byUsdNum].wCommandAddr16_2, e_ExtfInfo[s_byUsdNum].wCommandAddr32_1, e_ExtfInfo[s_byUsdNum].wCommandAddr32_2 );
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}
	// ExtFlashModule_008 Append Line
	s_bSendProtEfwriteStart = TRUE;	// PROT_MCU_EFWRITESTART()送信済み

	// (2) メーカ・デバイスID比較済み
	//     → 比較済みでなければ比較処理実行
	if( !s_byDiffIDFlg[s_byUsdNum] ){
		byMode = EFLASHROM_CMP_ID;
		// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
		ferr = PROT_MCU_EFWRITE(byMode, 0, (DWORD)e_ExtfInfo[s_byUsdNum].wMakerID, (DWORD)e_ExtfInfo[s_byUsdNum].wDeviceID, 0x0, NULL, &wEraseStatus);
		if (ferr != FFWERR_OK) {
			s_ExtFerr = ferr;
			// RevRxNo121122-005	Modify Start
			// RevRxNo120910-007	Append Start
			// getExtFlashID(s_byUsdNum,&data)をコールする。
			ferrExt = getExtFlashID(s_byUsdNum, &s_ferdiData);
			if( ferrExt == FFWERR_OK ) {
				// (ⅰ）getExtFlashID の戻り値がFFWERR_OKの場合
				//	<1>	data.dwErrorCodeにエラーコード（FFWERR_EXTROM_ID）をセットする。
				s_ferdiData.dwErrorCode = FFWERR_EXTROM_ID;
				//	<2>	SetErrorDetailInfo関数をコールする。　SetErrorDetailInfo（&data,6)
				//	SetErrorDetailInfo(&s_ferdiData,EFLASHERR_MAX);
				// RevRxNo121122-005	Modify End
			}else {
				// (ⅱ）getExtFlashID の戻り値がFFWERR_OK以外の場合
				// 		エラーコード格納の変数ferrに戻り値を上書きする。
				ferr = ferrExt;
			}
			// RevRxNo120910-007	Append End
			goto err;
		}
		s_byDiffIDFlg[s_byUsdNum] = TRUE;		// 比較済みに設定
	}

	// (3) 既にイレーズされているか確認
	//     → イレーズされていなければイレーズ実行
	if( !e_ExtfInfo[s_byUsdNum].byChipErase ){		// チップイレーズ指定がない場合のみセクタイレーズ実施
		// チップイレーズ指定なしで指定セクタ未イレーズの場合
		if( e_ExtfInfo[s_byUsdNum].byUnlock == EFLASHROM_NO_UNLOCK ){	// ロック解除指定なし

			// ExtFlashModule_002b Append Start
			// JEDECの場合、ロックがかかっているかの確認
			if (e_ExtfInfo[s_byUsdNum].byWriteProgramType == EXTF_WRITEPROG_JEDEC) {
				ferr = jedecVerifyLockbit(s_wSectNum, &bLockStatus);
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					goto err;
				}
				if (bLockStatus == TRUE) {
					// ExtFlashModule_002b Modify Line
					s_bSectLock[s_byUsdNum][s_wSectNum] = TRUE;	// このセクタはロックがかかっている
					s_bExistLockSect = TRUE;	// ロックされているセクタあり
				}
			}
			// ExtFlashModule_002b Append End

			byMode = EFLASHROM_ERASE;
		}else{															// ロック解除指定あり
			byMode = EFLASHROM_UNLOCK_ERASE;
		}
		byEraseStat = FALSE;		// イレーズ未実行
		// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
		ferr = PROT_MCU_EFWRITE(byMode, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], NULL, &wEraseStatus);
		if (ferr != FFWERR_OK) {
			s_ExtFerr = ferr;
			goto err;
		}
		byEraseStat = TRUE;			// イレーズ実行完
	}		

	
	// 外部フラッシュイレーズ & ライト(セクタサイズ分繰り返す)
	ferr = GetEndianTypeExtRom(e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], &byExtRomEndian);			// エンディアン判定＆取得
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}
	dwSect = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] / EFLASHROM_WRITESIZE;
	for( i = 0; i < dwSect; i++ ){
		// ExtFlashModule_002b Append Line
		if (s_bSectLock[s_byUsdNum][s_wSectNum] == FALSE) {	// ロックされていない場合のみライト処理を実行する

			// (4) ライトプログラム実行( 256バイト分のダウンロードデータを書き込みも実施 )
			memcpy( &byWriteData, &s_byExtRomCache[i*EFLASHROM_WRITESIZE], EFLASHROM_WRITESIZE );
			dwmadrWriteStartAddr = e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum] + i*EFLASHROM_WRITESIZE;
// ExtFlashModule_016 Modify Start
			if (i == 0) {
				// セクタの先頭256 バイトのライトでは、ライトデータが全て0xFF であるかに
				// 関わらず、必ずライトする。
				n = 0;
			} else {
				for( n = 0; n < EFLASHROM_WRITESIZE; n++ ){
					if( byWriteData[n] != 0xFF ){		// 書き込みデータが0xFF以外であればブレーク
						break;
					}
				}
			}
// ExtFlashModule_016 Modify End
			if( n != EFLASHROM_WRITESIZE ){		// 書き込みデータに0xFF以外があった場合のみ書き込み
				// byConnectの意味
				// #define EFLASHROM_MCU32_ROM32		0x00 : MCU32bit - ROM32bit			※RXは未対応
				// #define EFLASHROM_MCU32_ROM16_2		0x01 : MCU32bit - ROM16bit * 2		※RXは未対応
				// #define EFLASHROM_MCU32_ROM8_4		0x02 : MCU32bit - ROM8bit * 4		※RXは未対応
				// #define EFLASHROM_MCU16_ROM16		0x03 : MCU16bit - ROM16bit
				// #define EFLASHROM_MCU16_ROM8_2		0x04 : MCU16bit - ROM8bit * 2
				// #define EFLASHROM_MCU8_ROM8			0x05 : MCU8bit  - ROM8bit
				
				// ダウンロード先外部ROM領域のエンディアン確認
				if (byEndian == FFWRX_ENDIAN_BIG) {			// CPUがBIGの場合
					if( byExtRomEndian == FFWRX_ENDIAN_LITTLE ){				// 外部ROMがLITTLEの場合
						if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM16 || e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM8_2 ){	// MCU16の場合
							ReplaceEndian(byWriteData, MWORD_ACCESS, EFLASHROM_WRITESIZE);		// 2byte単位で反転する必要有り 
						}
					}else{														// 外部ROMがBIGの場合
					}
				}else{										// CPUがLITTLEの場合
					if( byExtRomEndian == FFWRX_ENDIAN_BIG ){					// 外部ROMがBIGの場合
						if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM16 || e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM8_2 ){	// MCU16の場合
							ReplaceEndian(byWriteData, MWORD_ACCESS, EFLASHROM_WRITESIZE);		// 2byte単位で反転する必要有り 
						}
					}
				}

				if( byEraseStat ){			// セクタイレーズ直後のライト
					byMode = EFLASHROM_STATUS_CHK;		// イレーズ後ステータスチェック付きライト処理実行
					byEraseStat = FALSE;		// イレーズ未実行
				}else{
					byMode = EFLASHROM_WRITE;			// ライト処理実行
				}
				// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
				ferr = PROT_MCU_EFWRITE(byMode, e_ExtfInfo[s_byUsdNum].wSectEraseTimeout, 0, 0, dwmadrWriteStartAddr, byWriteData, &wEraseStatus);
				if (ferr != FFWERR_OK) {
					// add 2010.4.26
					if( ferr == FFWERR_EXTROM_ERASE ){		// イレーズエラーで返ってきた場合
						if( e_ExtfInfo[s_byUsdNum].byWriteProgramType ){			// CUIの場合セクタイレーズ原因を確認
							BOOL	bResult;
							ferr = cuiStatusCheck( (WORD)i, &bResult );
							if (ferr != FFWERR_OK) {
								s_ExtFerr = ferr;
								goto err;
							}
							if (bResult){			// ロックがかかっている場合(ダウンロード処理続行)
								// ExtFlashModule_002b Modify Start
								s_bSectLock[s_byUsdNum][s_wSectNum] = TRUE;	// このセクタはロックがかかっている
								s_bExistLockSect = TRUE;	// ロックされているセクタあり
								// ExtFlashModule_002b Modify End
							}else{					// ロックがかかっていない場合(イレーズエラーで処理終了)
								ferr = FFWERR_EXTROM_ERASE;
								s_ExtFerr = ferr;
								goto err;
							}
						}else{														// JEDECの場合
							s_ExtFerr = ferr;
							goto err;
						}
					}else{
						s_ExtFerr = ferr;
						goto err;
					}		
				}

			// ExtFlashModule_002b Append Line
			}
		}
		s_dwExtGageCnt++;
		if (!(s_dwExtGageCnt % s_dwExtGageBaseCnt)) {
			// ExtFlashModule_007 Modify Start 
			s_dwExtWriteDataGageCnt = s_dwExtGageCnt / s_dwExtGageBaseCnt;
			SetGageCount(s_dwExtWriteDataGageCnt);
			// ExtFlashModule_007 Modify End 
			SetMsgGageCount(byGageMes);
		}
		// ※ 1セクタ分の書き込みが完了するまで(3)(4)を繰り返す
	}

	// 1セクタ終了処理(JEDEC:デバイスリセット、CUI:リードアレイモード)
	ferr = PROT_MCU_EFWRITEEND();
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		goto err;
	}

	s_byExtFlashWriteFlg = TRUE;								// 外部フラッシュへの書き込み実行済み	
	s_bExtFlashDataGetFlg = FALSE;								// 1セクタ分のデータ未取得にする
	// (5) ベリファイ指定有りの場合、ベリファイ実行
	dwChkCache = 0;		// 初期化
	dwChkData = 0;		// 初期化
	if( s_ExteVerify ){
		// １セクタ分のリードデータ格納領域確保
		bReadBuff = (unsigned char *)new(unsigned char[e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum]]);
		// V.1.02 覚え書き27 接続形態に合わせたアクセスサイズでリード対応 Mofdify Strat
		// BYTEでが正常に読み出せない場合もあるので、接続形態に合わせたアクセスサイズでリードするよう修正する
		// なおかつ、PROT_MCU_DUMPでも512バイト以上のデータを取得できるのでPROT_MCU_DUMPを使用する。
		// PROT_MCU_DUMPで取得したデータはバイト並びで格納されるのでアクセスサイズで変換する必要なし。
		if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM16 ){		// 16bitの場合
			eAccessSize = MWORD_ACCESS;
			dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] / 2;
		}else if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM8_2 ){	// 16bitの場合 	
			eAccessSize = MWORD_ACCESS;
			dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] / 2;
		}else if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU8_ROM8 ){	// 8bitの場合 	
			eAccessSize = MBYTE_ACCESS;
			dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum];
		}
		ferr = GetEndianTypeExtRom(e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum],&byEndian);
		if (ferr != FFWERR_OK) {
			// RevNo130118-001 Modify Line
			delete [] bReadBuff;					// 確保したベリファイ用メモリの解放
			s_ExtFerr = ferr;
			goto err;
		}

// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr = McuDumpDivide(1, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], FALSE, eAccessSize, FALSE, dwAccessCount, bReadBuff, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
		} else{
			ferr = PROT_MCU_DUMP(1, &e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], FALSE, &eAccessSize, FALSE, &dwAccessCount, bReadBuff, byEndian );
		}
// RevRxNo121022-001 Append End

		// V.1.02 覚え書き27 接続形態に合わせたアクセスサイズでリード対応 Mofdify End
		if (ferr != FFWERR_OK) {
			// RevNo130118-001 Modify Line
			delete [] bReadBuff;					// 確保したベリファイ用メモリの解放
			s_ExtFerr = ferr;
			goto err;
		}

		// V.1.02 RevNo110517-002 Append Line
		dwLength = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum];
		for( i = 0; i < dwLength; i++ ){
			if( s_byExtRomCache[i] != bReadBuff[i] ){	// ベリファイチェック
				ferr = FFWERR_EXTROM_VERIFY;
				// RevNo130118-001 Modify Line
				delete [] bReadBuff;				// 確保したベリファイ用メモリの解放
				s_ExtFerr = ferr;
				goto err;
			}
		}

		pDbgData = GetDbgDataRX();
		if (pDbgData->eWriter == EML_WRITERMODE_USE) {		// ライタモードの場合
			// 構造体に格納
			for( i = 0; i < dwLength; i++ ){
				// チェックサム値計算
				dwChkCache += s_byExtRomCache[i];			// 書き込んだ値のチェックサム
				dwChkData += bReadBuff[i];				// 読み出した値のチェックサム
			}
			pChecksumData->byMemType[pChecksumData->byCntMem-1] = (BYTE)(s_byUsdNum+EFLASHROM_CHECKSUM_OFFSET);
			pChecksumData->pdwCheckSum[pChecksumData->byCntMem-1] += dwChkData;	// チェックサム値格納(セクタ分加算)
			if( dwChkCache != dwChkData ){		// 不一致だったらCHECKSUM_NON_AGREEを設定。最後まで一致していればCHECKSUM_AGREEのまま。
				pChecksumData->eCheck[pChecksumData->byCntMem-1] = CHECKSUM_NON_AGREE;
			}
		}
		
		// RevNo130118-001 Modify Line
		delete [] bReadBuff;			// 確保したベリファイ用メモリの解放

	}else{			// ベリファイ指定がない場合
		pDbgData = GetDbgDataRX();
		if (pDbgData->eWriter == EML_WRITERMODE_USE) {		// ライタモードの場合
			// ベリファイチェックがなくてもチェックサムは計算する
			// １セクタ分のリードデータ格納領域確保
			bReadBuff = (unsigned char *)new(unsigned char[e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum]]);
			// V.1.02 覚え書き27 接続形態に合わせたアクセスサイズでリード対応 Mofdify Strat
			// BYTEでが正常に読み出せない場合もあるので、接続形態に合わせたアクセスサイズでリードするよう修正する
			// なおかつ、PROT_MCU_DUMPでも512バイト以上のデータを取得できるのでPROT_MCU_DUMPを使用する。
			// PROT_MCU_DUMPで取得したデータはバイト並びで格納されるのでアクセスサイズで変換する必要なし。
			if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM16 ){		// 16bitの場合
				eAccessSize = MWORD_ACCESS;
				dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] / 2;
			}else if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM8_2 ){	// 16bitの場合 	
				eAccessSize = MWORD_ACCESS;
				dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] / 2;
			}else if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU8_ROM8 ){	// 8bitの場合 	
				eAccessSize = MBYTE_ACCESS;
				dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum];
			}
			ferr = GetEndianTypeExtRom(e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum],&byEndian);
			if (ferr != FFWERR_OK) {
				// RevNo130118-001 Modify Line
				delete [] bReadBuff;				// 確保したベリファイ用メモリの解放
				s_ExtFerr = ferr;
				goto err;
			}
// RevRxNo121022-001 Append Start
			if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
				ferr = McuDumpDivide(1, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], FALSE, eAccessSize, FALSE, dwAccessCount, bReadBuff, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
			} else{
				ferr = PROT_MCU_DUMP(1, &e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], FALSE, &eAccessSize, FALSE, &dwAccessCount, bReadBuff, byEndian );
			}
// RevRxNo121022-001 Append End

			// V.1.02 覚え書き27 接続形態に合わせたアクセスサイズでリード対応 Mofdify End
			if (ferr != FFWERR_OK) {
				// RevNo130118-001 Modify Line
				delete [] bReadBuff;				// 確保したベリファイ用メモリの解放
				s_ExtFerr = ferr;
				goto err;
			}
			// V.1.02 RevNo110517-002 Append Line
			dwLength = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum];
			// 構造体に格納
			for( i = 0; i < dwLength; i++ ){
				// チェックサム値計算
				dwChkCache += s_byExtRomCache[i];			// 書き込んだ値のチェックサム
				dwChkData += bReadBuff[i];				// 読み出した値のチェックサム
			}
			pChecksumData->byMemType[pChecksumData->byCntMem-1] = (BYTE)(s_byUsdNum+EFLASHROM_CHECKSUM_OFFSET);
			pChecksumData->pdwCheckSum[pChecksumData->byCntMem-1] += dwChkData;	// チェックサム値格納(セクタ分加算)
			if( dwChkCache != dwChkData ){		// 不一致だったらCHECKSUM_NON_AGREEを設定。最後まで一致していればCHECKSUM_AGREEのまま。
				pChecksumData->eCheck[pChecksumData->byCntMem-1] = CHECKSUM_NON_AGREE;
			}
			// RevNo130118-001 Modify Line
			delete [] bReadBuff;				// 確保したベリファイ用メモリの解放
		}
	}
	// RevNo130118-001 Modify Line
	if(s_bNewExtRomCache == TRUE){				// 確保したキャッシュメモリの解放
		delete [] s_byExtRomCache;

		// RevNo130118-001 Append Line
		s_bNewExtRomCache = FALSE;
	}
	return FFWERR_OK;
err:
	// RevNo130118-001 Modify Line
	if(s_bNewExtRomCache == TRUE){				// 確保したキャッシュメモリの解放
		delete [] s_byExtRomCache;

		// RevNo130118-001 Append Line
		s_bNewExtRomCache = FALSE;
	}
	return ferr;
	
}


//=============================================================================
/**
 * << 外部フラッシュメモリへのライト処理関数 >>
 *   開始アドレス～終了アドレスは、アクセスサイズの倍数を指定する。
 *   開始アドレス、終了アドレス、アクセスサイズのチェックは、本関数呼び出し側で
 *   行うこと。
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ExtMemWrite(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
				enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr, DWORD *dwLen )
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTotalLength;			// 書き込みデータの全体バイトサイズ
	BYTE	byUsdNum;				// 指定アドレスがどのUSDファイルに存在しているかを示す
	WORD	wSectNum;				// 指定アドレスがどのセクタに存在しているかを示す
	DWORD	dwCnt;
	FFW_VERIFYERR_DATA VerifyErr;
	DWORD	dwLength;				// 書き込みデータのバイトサイズ
	DWORD	dwAccessCount;			// DUMP時のアクセスサイズによるアクセス回数
	MADDR	madrRealAddr;
	MADDR	madrSectEndAddr;
	const BYTE*	pbyBuff;
	// V.1.02 覚え書き27 接続形態に合わせたアクセスサイズでリード対応 Append Line
	BYTE	byEndian;

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応
	pVerifyErr;							//ワーニング対策

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
	dwAccessCount = 0;				// ワーニング対策
// RevRxNo121022-001 Append Start

	pbyBuff = pbyWriteBuff;
	dwTotalLength = madrEndAddr - madrStartAddr + 1;	// ライトバイト数算出
	*dwLen = 0;

	if (dwTotalLength != 0) {			// 書き込みデータあり
		if (GetDwnpOpenData() == TRUE) {				// DWNPコマンド実行中の場合(ダウンロードの場合)
			// ダウンロード領域判断( 対応USDファイル番号、対応セクタ番号 )
			if( setExtFlashDataSetSect(madrStartAddr, madrEndAddr, &byUsdNum, &wSectNum) == FALSE ){
				return ferr;
			}
			// 書き込み処理を実行すべきかどうか判断
			if( !s_bExtWriteFlashStartFlg[byUsdNum] ){		// 外部フラッシュダウンロード準備未の場合
				if( s_byUsdNum != byUsdNum ){				// 前回処理したUSDと同じではない(次のUSDファイルデータ)
					s_wLastSectNum = 0xFFFF;				// 前回処理した最後のセクタ番号初期化
					// データ書き込み処理実行
					if( !GetExtFlashWriteFlg() ){			// 外部フラッシュへの書き込み未実行の場合
						ferr = ExtWriteFlashData(eVerify, &VerifyErr);
						if (ferr != FFWERR_OK) {
							s_ExtFerr = ferr;
							return ferr;
						}
						ferr = ExtWriteFlashEnd();
						if (ferr != FFWERR_OK) {
							s_ExtFerr = ferr;
							return ferr;
						}
					}
				}
				s_byUsdNum = byUsdNum;
				s_wSectNum = wSectNum;
				s_ExteVerify = eVerify;
				// ExtFlashModule_006 Append Line
				SetGageCount(0);	// 進捗ゲージカウンタ用変数初期化
				// 外部フラッシュダウンロード実行前準備(ワークRAM退避、外部バス設定、書き込みプログラムダウンロード)
				ferr = ExtWriteFlashStart(madrStartAddr, madrEndAddr);
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					return ferr;
				}
			}
			// ダウンロード領域判断( 対応USDファイル番号、対応セクタ番号 )
			if( (s_byUsdNum == byUsdNum) && (s_wSectNum != wSectNum) ){
				// USDファイルは同じで、前回処理したセクタと同じではない(次のセクタ情報)
				// データ書き込み処理実行
				if( !GetExtFlashWriteFlg() ){			// 外部フラッシュへの書き込み未実行の場合
					ferr = ExtWriteFlashData(eVerify, &VerifyErr);
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						return ferr;
					}
				}
			}
			while( dwTotalLength ){					// 書き込むべきデータがある場合
				// ダウンロード領域判断( 対応USDファイル番号、対応セクタ番号 )
				if( setExtFlashDataSetSect(madrStartAddr, madrEndAddr, &byUsdNum, &wSectNum) == FALSE ){
					return ferr;
				}
				if( s_byUsdNum != byUsdNum ){		// 前回のUSDファイル領域ではない場合
					// ダウンロードデータが2つのUSDファイル領域をまたいでいる場合のみ以下の処理実行
					ferr = ExtWriteFlashEnd();		// 前USDファイルでのダウンロード処理を終了させておく
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						return ferr;
					}
					s_byUsdNum = byUsdNum;
					s_wSectNum = wSectNum;
					s_wLastSectNum = 0xFFFF;				// 前回処理した最後のセクタ番号初期化
					// ExtFlashModule_006 Append Line
					SetGageCount(0);	// 進捗ゲージカウンタ用変数初期化
					// 外部フラッシュダウンロード実行前準備(ワークRAM退避、外部バス設定、書き込みプログラムダウンロード)
					ferr = ExtWriteFlashStart(madrStartAddr, madrEndAddr);
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						return ferr;
					}
				}else{
					s_byUsdNum = byUsdNum;
					s_wSectNum = wSectNum;
				}
				// V.1.02 No.24 外部フラッシュダウンロード禁止対応(ExtFlashModule_012) Modify Start
				if( (s_wLastSectNum != wSectNum) && (e_ExtfInfo[s_byUsdNum].byEraseInfo[s_wSectNum] != EXTF_NOT_DOWNLOAD)){			// 前回処理したセクタと同じではない(次のセクタ情報)
					// キャッシュ領域確保
					s_byExtRomCache = (unsigned char *)new(unsigned char[e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum]]);
					// RevNo130118-001 Append Line
					s_bNewExtRomCache = TRUE;
				}
				s_wLastSectNum = wSectNum;
				// (3) 上書きダウンロードかイレーズダウンロードかダウンロード禁止か判断
				if( e_ExtfInfo[s_byUsdNum].byEraseInfo[s_wSectNum] == EXTF_NOT_DOWNLOAD ){			// ダウンロード禁止
					madrSectEndAddr = e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum] + e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] - 1;
					madrRealAddr = madrStartAddr - e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum];
					if( madrEndAddr > madrSectEndAddr ){				// セクタを越えるデータがある場合
						dwLength = madrSectEndAddr+1 - madrStartAddr;
						*dwLen += dwLength;
						pbyWriteBuff += dwLength;
						s_byExtFlashWriteFlg = FALSE;					// 外部フラッシュへの書き込み未実行	
						madrStartAddr = madrStartAddr + dwLength;		// 書き込みデータ開始アドレスを更新
					}else{
						dwLength = dwTotalLength;
						*dwLen += dwLength;
						pbyWriteBuff += dwLength;
						s_byExtFlashWriteFlg = TRUE;					// 外部フラッシュへの書き込み実行	
					}
					dwTotalLength = dwTotalLength - dwLength;
					continue;
				}else if( e_ExtfInfo[s_byUsdNum].byEraseInfo[s_wSectNum] == EXTF_ERASE_DOWNLOAD ){	// イレーズダウンロード
				// V.1.02 No.24 外部フラッシュダウンロード禁止対応(ExtFlashModule_012) Modify End
					if( !s_bExtFlashDataGetFlg ){								// キャッシュメモリ初期化済みでない場合
						// キャッシュメモリを0xFFで初期化
						for( dwCnt = 0; dwCnt < e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum]; dwCnt++ ){
							s_byExtRomCache[dwCnt] = 0xFF;
						}
						s_bExtFlashDataGetFlg = TRUE;							// キャッシュメモリ初期化済み
					}
				}else{															// 上書きダウンロード
					// (4) 上書きダウンロードの場合
					//      外部ROMデータ取得済みか確認
					//        → 取得してない場合は1セクタ分データ取得
					if( !s_bExtFlashDataGetFlg ){								// 1セクタ分のデータ未取得の場合
						// 1セクタ分データ取得
						// V.1.02 覚え書き27 接続形態に合わせたアクセスサイズでリード対応 Mofdify Strat
						// BYTEでが正常に読み出せない場合もあるので、接続形態に合わせたアクセスサイズでリードするよう修正する
						// なおかつ、PROT_MCU_DUMPでも512バイト以上のデータを取得できるのでPROT_MCU_DUMPを使用する。
						// PROT_MCU_DUMPで取得したデータはバイト並びで格納されるのでアクセスサイズで変換する必要なし。
						if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM16 ){		// 16bitの場合
							eAccessSize = MWORD_ACCESS;
							dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] / 2;
						}else if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM8_2 ){	// 16bitの場合 	
							eAccessSize = MWORD_ACCESS;
							dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] / 2;
						}else if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU8_ROM8 ){	// 8bitの場合 	
							eAccessSize = MBYTE_ACCESS;
							dwAccessCount = e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum];
						}
						ferr = GetEndianTypeExtRom(e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum],&byEndian);
						if (ferr != FFWERR_OK) {
							s_ExtFerr = ferr;
							return ferr;
						}

// RevRxNo121022-001 Append Start
						if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
							ferr = McuDumpDivide(1, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], FALSE, eAccessSize, FALSE, dwAccessCount, s_byExtRomCache, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
						} else{
							ferr = PROT_MCU_DUMP(1, &e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], FALSE, &eAccessSize, FALSE, &dwAccessCount, s_byExtRomCache, byEndian );
						}
// RevRxNo121022-001 Append End

						// V.1.02 覚え書き27 接続形態に合わせたアクセスサイズでリード対応 Mofdify End
						if (ferr != FFWERR_OK) {
							s_ExtFerr = ferr;
							return ferr;
						}
						s_bExtFlashDataGetFlg = TRUE;				// 1セクタ分のデータ取得済みにする
					}
				}
				// (5) ダウンロードデータを格納(s_byExtRomCache[]の指定位置に指定データをライトする)
				madrSectEndAddr = e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum] + e_ExtfInfo[s_byUsdNum].dwSectSize[s_wSectNum] - 1;
				madrRealAddr = madrStartAddr - e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum];
				if( madrEndAddr > madrSectEndAddr ){				// セクタを越えるデータがある場合
					dwLength = madrSectEndAddr+1 - madrStartAddr;
					*dwLen += dwLength;
					for( dwCnt = 0; dwCnt < dwLength; dwCnt++, madrRealAddr++ ){
						s_byExtRomCache[madrRealAddr] = *pbyWriteBuff++;
					}
					s_byExtFlashWriteFlg = FALSE;					// 外部フラッシュへの書き込み未実行	
					// データ書き込み処理実行
					ferr = ExtWriteFlashData(eVerify, &VerifyErr);
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						return ferr;
					}
					madrStartAddr = madrStartAddr + dwLength;		// 書き込みデータ開始アドレスを更新
				}else{
					dwLength = dwTotalLength;
					*dwLen += dwLength;
					for( dwCnt = 0; dwCnt < dwLength; dwCnt++, madrRealAddr++ ){
						s_byExtRomCache[madrRealAddr] = *pbyWriteBuff++;
					}
					s_byExtFlashWriteFlg = FALSE;					// 外部フラッシュへの書き込み未実行	
				}
				dwTotalLength = dwTotalLength - dwLength;
			}
		}else{								// ダウンロードでない場合(通常のWRITE)
			// 2009/12MP版では対応しない
		}
	}

	return ferr;
}




///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
 * << デバッグDMA機能によるCodeWRITE処理
 *			(外部フラッシュダウンロード用書き込みプログラム書き込み専用)関数 >>
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR cwriteDmaAccessExtFlash(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							 enum FFWENM_MACCESS_SIZE eAccessSize,  const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	DWORD	dwAddAddr;
	DWORD	dwLength;			//パイトサイズ
	DWORD	dwAccessCount;		//WRITE時のアクセスサイズによるアクセス回数
	BOOL	bSameAccessSize;
	BOOL	bSameLength;
	BOOL	bSameWriteData;
	// V.1.02 RevNo110304-001 Deleate & Append Line		Delete:BOOL	bFlashRomArea = FALSE;
	enum FFWRXENM_MAREA_TYPE	eAreaType;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	DWORD	dwTotalLength;		//全体のバイトサイズ
	DWORD	dwTotalAccessCount;	//全体のアクセス回数
	BYTE	byEndian;
	const BYTE*	pbyBuff;
	BOOL	bEnableArea = FALSE;
	enum FFWENM_VERIFY_SET eVerifyTmp;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	DWORD	dwWriteLengthMax = 0;
	getEinfData(&einfData);			// エミュレータ情報取得

	if (einfData.wEmuStatus == EML_EZCUBE){		// EZ-CUBEの場合
		dwWriteLengthMax = PROT_WRITE_LENGTH_MAX_EZ;
	}
	else{		// E1/E20の場合
		dwWriteLengthMax = PROT_WRITE_LENGTH_MAX;
	}
// RevRxNo121022-001 Append End
	
	pbyBuff = pbyWriteBuff;
	dwAreaNum = 1;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	dwTotalLength = madrEndAddr - madrStartAddr + 1;
	bSameAccessSize = FALSE;
	bSameLength = FALSE;
	bSameWriteData = FALSE;

	SetVerifySet(eVerify);

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
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	//全体のバイトサイズを全体のアクセス回数に変換
	dwTotalAccessCount = dwTotalLength/dwAddAddr;	// WRITE回数算出

	// V.1.01 No.26 メモリアクセス端数処理対応に伴う変更 modify Start
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	while (dwTotalAccessCount > 0) {
		// 指定領域判定
		// V.1.02 RevNo110304-001 Modify Line
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 1, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		ferr = GetEndianTypeExtRom(madrStartAddr, &byEndian);				// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (bEnableArea == TRUE) {
// RevRxNo121022-001 Modify Start
			if (dwLength > dwWriteLengthMax) {
				dwLength = dwWriteLengthMax;
			}
// RevRxNo121022-001 Modify End
			// バイトサイズをアクセス回数に変換
			dwAccessCount = dwLength/dwAddAddr;
			if (dwAccessCount < 1) {
				dwAccessCount = 1;
			}

			byEndian = FFWRX_ENDIAN_LITTLE;
			ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
						bSameLength, &dwAccessCount, bSameWriteData, pbyBuff, pVerifyErr,byEndian);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ベリファイチェック
			eVerifyTmp = GetVerifySet();
			if (eVerifyTmp == VERIFY_ON) {
				ferr = chkVerifyWriteDataExtFlash(madrStartAddr, eAccessSize, dwAccessCount, pbyBuff, pVerifyErr, byEndian);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {			// ベリファイエラー発生時、処理を終了する。
					return ferr;
				}
			}
		} else {
			// バイトサイズをアクセス回数に変換
			dwAccessCount = dwLength/dwAddAddr;
		}

// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
		dwTotalAccessCount -= dwAccessCount;					// WRITE回数更新
		dwLength = dwAccessCount * dwAddAddr;
		madrStartAddr += dwLength;								// WRITE開始アドレス更新
		pbyBuff += dwLength;
	}
	// V.1.01 No.26 メモリアクセス端数処理対応に伴う変更 modify End

	return ferr;
}

//=============================================================================
/**
 * << ベリファイチェック処理(外部フラッシュダウンロード時CWRITE用)関数 >>
 * @param madrStartAddr 開始アドレス
 * @param eAccessSize   アクセスサイズ(取得サイズ計算,エラー返却用)
 * @param dwAccessCount      データ長
 * @param pbyWriteBuff  ライトデータ格納用バッファアドレス
 * @param pVerifyErr    ベリファイ結果格納用バッファアドレス
  * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR chkVerifyWriteDataExtFlash(MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, 
						const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr, BYTE byEndian)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BYTE*	pbyReadBuf;
	BYTE*	pbyReadBufEsc;
	DWORD	i;
// RevNo111121-003 Delete Line  madrSearchAddr を削除
	DWORD	dwSizeCnt = 0;
	DWORD	dwChkCnt = 0;
	DWORD	dwAccessSizeCnt;		// RevNo111121-003 Append Line

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
// RevNo111121-003 Delete Line  madrSearchAddr を削除

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwSizeCnt = 1;
		dwAccessSizeCnt = 1;		// RevNo111121-003 Append Line
		break;
	case MWORD_ACCESS:
		dwSizeCnt = 2;
		dwAccessSizeCnt = 2;		// RevNo111121-003 Append Line
		break;
	case MLWORD_ACCESS:
		dwSizeCnt = 4;
		dwAccessSizeCnt = 4;		// RevNo111121-003 Append Line
		break;
// RevNo111121-003 Append Start
	default:
		dwSizeCnt = 4;
		dwAccessSizeCnt = 4;
		break;
// RevNo111121-003 Append End
	}

	// ベリファイチェックする最大Byte数をセット
	dwChkCnt = dwAccessCount * dwSizeCnt;

	pbyReadBufEsc = (BYTE*)malloc(dwChkCnt);
	// RevRxNo130730-009 Append Start
	if( pbyReadBufEsc == NULL ){
		return FFWERR_MEMORY_NOT_SECURED;
	}
	// RevRxNo130730-009 Append End
	pbyReadBuf = pbyReadBufEsc;
	memset(pbyReadBuf, 0, dwChkCnt);

	// 実際に書き込まれたデータ取得(DUMP)
// RevRxNo121022-001 Append Start
	// 呼び出し側で、アクセスバイト数の分割処理を行っている。
	// ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないのでBFWMCUCmd_DUMPコマンドの
	// リードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
	ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, 
				bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
	if (ferr != FFWERR_OK) {
		free(pbyReadBufEsc);
		return ferr;
	}

	// BIGエンディアンの場合は並び替え必要
	if( byEndian == FFWRX_ENDIAN_BIG ){
		if (s_eAccessSize == MWORD_ACCESS){			// WORDアクセス
			ReplaceEndian(pbyReadBuf, MWORD_ACCESS, dwChkCnt);
		}else if (s_eAccessSize == MLWORD_ACCESS){			// LWORDアクセス
			ReplaceEndian(pbyReadBuf, MLWORD_ACCESS, dwChkCnt);
		}
	}

	pVerifyErr->eErrorFlag = VERIFY_OK;	// RevNo120201-001 Append Line

	// ベリファイチェック(1Byteづつチェック)
	for (i = 0; i < dwChkCnt; i++) {
		// ベリファイエラー発生時
		if (pbyReadBuf[i] != pbyWriteBuff[i]) {
			pVerifyErr->eErrorFlag = VERIFY_ERR;				// ベリファイエラー発生有無情報
			pVerifyErr->eAccessSize = eAccessSize;				// ベリファイエラー発生時のアクセスサイズ
// RevNo111121-003 Modify Start
			// ベリファイエラー発生時のアドレス
			pVerifyErr->dwErrorAddr = madrStartAddr + (i - (i % dwAccessSizeCnt));

			// ベリファイエラー発生時のライトデータ
			switch (eAccessSize) {
			case MBYTE_ACCESS:
				pVerifyErr->dwErrorWriteData = (DWORD)pbyWriteBuff[i];
				break;
			case MWORD_ACCESS:
				pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteBuff[i - (i % dwAccessSizeCnt) + 1] << 8) | 
												pbyWriteBuff[i - (i % dwAccessSizeCnt)]);
				break;
			case MLWORD_ACCESS:
				pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteBuff[i - (i % dwAccessSizeCnt) + 3] << 24) | 
												(pbyWriteBuff[i - (i % dwAccessSizeCnt) + 2] << 16) | 
												(pbyWriteBuff[i - (i % dwAccessSizeCnt) + 1] << 8) | 
												pbyWriteBuff[i - (i % dwAccessSizeCnt)]);
				break;
			}

			// ベリファイエラー発生時のリードデータ
			switch (eAccessSize) {
			case MBYTE_ACCESS:
				pVerifyErr->dwErrorReadData = (DWORD)pbyReadBuf[i];
				break;
			case MWORD_ACCESS:
				pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 8) | 
												pbyReadBuf[i - (i % dwAccessSizeCnt)]);
				break;
			case MLWORD_ACCESS:
				pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt) + 3] << 24) | 
												(pbyReadBuf[i - (i % dwAccessSizeCnt) + 2] << 16) | 
												(pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 8) | 
												pbyReadBuf[i - (i % dwAccessSizeCnt)]);
				break;
			}
// RevNo111121-003 Modify End
			break;
		}
// RevNo111121-003 Delete Line  madrSearchAddr を削除
	}

	free(pbyReadBufEsc);

	return ferr;
}


//=============================================================================
/**
 * << 外部フラッシュダウンロード時使用ワークRAM領域の復帰関数 >>
 * ワークRAM領域に退避しておいたデータを復帰。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR resumeWorkRamArea(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_VERIFYERR_DATA	VerifyErr;
	MADDR	madrEndAddr;

// RevNo110114-02( ExtFlashModule_013 ) Append Start
	DWORD	dwAreaNum;
	DWORD	dwAccessCount;			//WRITE時のアクセスサイズによるアクセス回数
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BOOL	bSameWriteData;
	BYTE	byEndian;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	bSameWriteData = FALSE;
// RevNo110114-02( ExtFlashModule_013 ) Append End

// ExtFlashModule_001 Delete Line
//	FFW_VERIFYERR_DATA* pVerifyErr;
// V.1.02 新デバイス対応(ExtFlashModule_014) Append&Modify Start
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWRX_MCUINFO_DATA* pMcuInfo;	// RevRxNo130301-001 Append Start
	MADDR						madrRWaddr;			// 参照/設定するレジスタのアドレス
	WORD						wWriteData[1];		// 設定データ格納領域
	DWORD						dwWriteData[1];		// 設定データ格納領域
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	BOOL						bProtectFree = FALSE;		// プロテクト解除記録用 
	BOOL						bEsc;
	WORD						wData;
	DWORD						dwData;
// V.1.02 新デバイス対応(ExtFlashModule_014) Append&Modify End
	// RevRxNo130301-001 Append Start
	MADDR						madrSyscr1;			// SYSCR1アドレス値
	enum FFWENM_MACCESS_SIZE	eSyscr1Size;		// SYSCR1アクセスサイズ
	MADDR						madrMstpcrc;		// MSTPCRCアドレス値
	enum FFWENM_MACCESS_SIZE	eMstpcrcSize;		// MSTPCRCアクセスサイズ
	// RevRxNo130301-001 Append End

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
	dwAccessCount = 0;				// ワーニング対策
// RevRxNo121022-001 Append End

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	pMcuInfo = GetMcuInfoDataRX();	// RevRxNo130301-001 Append Line

// RevNo110114-02( ExtFlashModule_013 ) Modify Start
	madrEndAddr = e_ExtfInfo[s_byUsdNum].dwWorkStart + e_ExtfInfo[s_byUsdNum].wWorkSize - 1;

	// RevRxNo130301-001 Modify Start
	// SYSCR1/MSTPCRC設定で使用する内部変数を初期化
	// 二次元配列に格納されているアドレスとサイズ情報は、どの配列でも同じ値である。
	// そのため、[3][0]/[4][0]の値を変数に設定する。
	madrSyscr1 = pMcuInfo->dwmadrMcuRegInfoAddr[3][0];
	eSyscr1Size = pMcuInfo->eMcuRegInfoAccessSize[3][0];
	madrMstpcrc = pMcuInfo->dwmadrMcuRegInfoAddr[4][0];
	eMstpcrcSize = pMcuInfo->eMcuRegInfoAccessSize[4][0];
	// RevRxNo130301-001 Modify End

	//退避していたユーザRAMを復帰
	if( s_bExtWorkRamResumeFlg ){		// 復帰する必要がある場合のみ復帰
		// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Mpdify Start
		// BFW側の処理修正により1024バイト以上のデータも処理できるようになったのでPROT_MCU_WRITEにして問題なし
		if( s_eAccessSize == MBYTE_ACCESS ){
			dwAccessCount = e_ExtfInfo[s_byUsdNum].wWorkSize;
		}else if( s_eAccessSize == MWORD_ACCESS ){
			dwAccessCount = e_ExtfInfo[s_byUsdNum].wWorkSize / 2;
		}else if( s_eAccessSize == MLWORD_ACCESS ){
			dwAccessCount = e_ExtfInfo[s_byUsdNum].wWorkSize / 4;
		}
		ferr = GetEndianTypeExtRom(e_ExtfInfo[s_byUsdNum].dwWorkStart,&byEndian);
		if (ferr != FFWERR_OK) {
			return ferr;
		}

// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr = McuWriteDivide(s_ExteVerify, dwAreaNum, e_ExtfInfo[s_byUsdNum].dwWorkStart, bSameAccessSize, s_eAccessSize,
					bSameAccessCount, dwAccessCount, bSameWriteData, s_byWorkRam, &VerifyErr, byEndian, PROT_WRITE_LENGTH_MAX_EZ);
		} else{
			ferr = PROT_MCU_WRITE(s_ExteVerify, dwAreaNum, &e_ExtfInfo[s_byUsdNum].dwWorkStart, bSameAccessSize, &s_eAccessSize,
					bSameAccessCount, &dwAccessCount, bSameWriteData, s_byWorkRam, &VerifyErr, byEndian);
		}
// RevRxNo121022-001 Append End

		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// V.1.02 覚え書き15の2点目( RevNo101209-02 )(RevNo110317-001) Mpdify End
	}

// V.1.02 覚え書き32 RAM停止時の処理対応 Modify Start(ソースコードDR指摘事項 No.7対応)
	if( s_bExtIntRamEnableFlg || s_bExtRamStartFlg ){		// 内蔵RAM無効を有効に変えている or RAM停止を動作に変えている場合
		//指定ワークRAM領域が内蔵RAM内に収まる場合

// V.1.02 新デバイス対応(ExtFlashModule_014) Append Start
// RX630/RX210 SYSCR1プロテクト解除処理追加
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
			bEsc = TRUE;
			ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
// V.1.02 新デバイス対応(ExtFlashModule_014) Append End
		if( s_bExtIntRamEnableFlg ){		// RAM無効を有効に変えていた場合
			eAccessSize = eSyscr1Size;				// RevRxNo130301-001 Modify Line
			pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);
			madrRWaddr = madrSyscr1;				// RevRxNo130301-001 Modify Line
			// レジスタ書き込みデータをセット
			wData = s_wExtRamEnableData;
			memcpy(pbyWriteData, &wData, sizeof(WORD));
// ExtFlashModule_009 Modify Line (変数定義修正。pVerifyErr -> VerifyErr, ライトデータサイズ修正 1 -> 2 )
// V.1.02 FFW自身のDUMP処理PROT_MCU_DUMP化対応 Modify Line 
			ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		if( s_bExtRamStartFlg ){		// RAM停止を動作に変えていた場合
			eAccessSize = eMstpcrcSize;				// RevRxNo130301-001 Modify Line
			pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
			madrRWaddr = madrMstpcrc;				// RevRxNo130301-001 Modify Line
			// V.1.02 覚え書き32 RAM停止時の処理対応 Append Line
			dwData = s_dwExtMstpcrcData;
			memcpy(pbyWriteData, &dwData, sizeof(DWORD));
			ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}

// V.1.02 新デバイス対応(ExtFlashModule_014) Append Start
// RX630/RX210 SYSCR1プロテクト設定処理追加
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
			if( bProtectFree ){		// プロテクト解除した場合(プロテクト設定する必要あり)
				bEsc = FALSE;
				ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
				// V.1.02 RevNo031123-001 Modify End
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}
// V.1.02 新デバイス対応(ExtFlashModule_014) Append End
		s_bExtIntRamEnableFlg = FALSE;	// 内蔵RAM無効を有効にはしていないに設定
		s_bExtRamStartFlg = FALSE;		// 内蔵RAM停止を動作にはしていないに設定
// V.1.02 FFW自身のDUMP処理PROT_MCU_DUMP化対応 Modify Line 
	}
// RevNo110114-02( ExtFlashModule_013 ) Modify End
// V.1.02 覚え書き32 RAM停止時の処理対応 Modify End(ソースコードDR指摘事項 No.7対応)
	return ferr;
}


//=============================================================================
/**
 * << 外部フラッシュダウンロード時使用ワークRAM領域判断&内蔵RAM有効化関数 >>
 * 指定されたワークRAM領域が、内蔵RAM領域か外部領域かを判断し、
 * 内蔵RAM領域の場合のみ、内蔵RAM有効/無効を確認し、無効の場合は有効にする。
 * @param madrStartAddr ワークRAM開始アドレス
 * @param madrEndAddr ワークRAM終了アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR intRamEnable(MADDR madrStartAddr, MADDR madrEndAddr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWRX_MCUAREA_DATA* pMcuArea;
	DWORD	dwCnt;
// V.1.02 新デバイス対応(ExtFlashModule_014) Append&Modify Start
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWRX_MCUINFO_DATA* pMcuInfo;	// RevRxNo130301-001 Append Start
	MADDR						madrRWaddr;			// 参照/設定するレジスタのアドレス
	// RevRxNo130301-001 Delete Line
	WORD						wReadData[1];		// 参照データ格納領域
	WORD						wWriteData[1];		// 設定データ格納領域
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	WORD						wData;				// 実際に書き込むデータ
	BOOL						bProtectFree = FALSE;		// プロテクト解除記録用 
	BOOL						bEsc;
// V.1.02 覚え書き32 RAM停止時の処理対応 Append Start(ソースコードDR指摘事項 No.7対応)
	DWORD						dwReadData[1];		// 参照データ格納領域
	DWORD						dwWriteData[1];		// 設定データ格納領域
	BYTE						byRamDisable = FALSE;		// RAM無効記録用 
	BYTE						byRamStop = FALSE;			// RAM停止記録用 
	DWORD						dwData;				// 実際に書き込むデータ
// V.1.02 覚え書き32 RAM停止時の処理対応 Append End(ソースコードDR指摘事項 No.7対応)
// V.1.02 新デバイス対応(ExtFlashModule_014) Append&Modify End
	// RevRxNo130301-001 Append Start
	BOOL						bRamEnable = FALSE;
	BOOL						bRamBlockEna[MCU_AREANUM_MAX_RX];
	MADDR						madrSyscr1;			// SYSCR1アドレス値
	enum FFWENM_MACCESS_SIZE	eSyscr1Size;		// SYSCR1アクセスサイズ
	WORD						wSyscr1Data;		// SYSCR1設定値
	WORD						wSyscr1Mask;		// SYSCR1マスク値
	MADDR						madrMstpcrc;		// MSTPCRCアドレス値
	enum FFWENM_MACCESS_SIZE	eMstpcrcSize;		// MSTPCRCアクセスサイズ
	DWORD						dwMstpcrcData;		// MSTPCRC設定値
	DWORD						dwMstpcrcMask;		// MSTPCRCマスク値
	// RevRxNo130301-001 Append End

// ExtFlashModule_009 Modify Line ( 変数定義修正。pVerifyErr -> VerifyErr )
	FFW_VERIFYERR_DATA VerifyErr;
	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	pMcuArea = GetMcuAreaDataRX();
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	pMcuInfo = GetMcuInfoDataRX();	// RevRxNo130301-001 Append Line

	// RevRxNo130301-001 Modify Start
	// SYSCR1/MSTPCRC設定で使用する内部変数を初期化
	// 二次元配列に格納されているアドレスとサイズ情報は、どの配列でも同じ値である。
	// そのため、[3][0]/[4][0]の値を変数に設定する。
	madrSyscr1 = pMcuInfo->dwmadrMcuRegInfoAddr[3][0];
	eSyscr1Size = pMcuInfo->eMcuRegInfoAccessSize[3][0];
	wSyscr1Data = 0x0000;
	wSyscr1Mask = 0x0000;
	madrMstpcrc = pMcuInfo->dwmadrMcuRegInfoAddr[4][0];
	eMstpcrcSize = pMcuInfo->eMcuRegInfoAccessSize[4][0];
	dwMstpcrcData = 0x00000000;
	dwMstpcrcMask = 0x00000000;
	wData = 0x0000;
	dwData = 0x00000000;

	// 内蔵RAMの有効/無効設定
	bRamEnable = ChkRamArea(madrStartAddr, madrEndAddr, &bRamBlockEna[0]);

	if (bRamEnable == TRUE) {	// 指定領域がRAM領域と重複する場合
		for (dwCnt = 0; dwCnt < pMcuArea->dwRamAreaNum; dwCnt++) {
			if (bRamBlockEna[dwCnt] == TRUE) {		// i番目のRAM領域と重複している場合

				// i番目のRAM領域に対応したSYSCR1/MSTPCRC値の設定値を格納
				// SYSCR1関連
				wSyscr1Data |= static_cast<WORD>(pMcuInfo->dwMcuRegInfoData[3][dwCnt]);	// SYSCR1設定値(RAM領域毎に値が異なるため、論理和)
				wSyscr1Mask |= static_cast<WORD>(pMcuInfo->dwMcuRegInfoMask[3][dwCnt]);	// SYSCR1マスク値(RAM領域毎に値が異なるため、論理和)

				// MSTPCRC関連
				dwMstpcrcData |= pMcuInfo->dwMcuRegInfoData[4][dwCnt];			// MSTPCRC設定値(RAM領域毎に値が異なるため、論理和)
				dwMstpcrcMask |= pMcuInfo->dwMcuRegInfoMask[4][dwCnt];			// MSTPCRCマスク値(RAM領域毎に値が異なるため、論理和)
			}
		}

		// SYSCR1レジスタの読み出し
		madrRWaddr = madrSyscr1;
		eAccessSize = eSyscr1Size;
		pbyReadData = reinterpret_cast<BYTE*>(wReadData);
		ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT,pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// 内蔵RAM無効時、SYSCR1設定値を生成
		if ((wReadData[0] & wSyscr1Mask) != wSyscr1Data) {
			memcpy(&s_wExtRamEnableData, pbyReadData, sizeof(WORD));		// 設定内容を記憶
			byRamDisable = TRUE;
			// SYSCR1レジスタ書き込みデータをセット
			wData = (s_wExtRamEnableData & ~(wSyscr1Mask)) | wSyscr1Data;
		}

		// モジュールストップコントロールレジスタCの読み出し
		madrRWaddr = madrMstpcrc;
		eAccessSize = eMstpcrcSize;
		pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
		ferr = GetMcuSfrReg(madrRWaddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// 内蔵RAM停止時、MSTPCRC設定値を生成
		if ((dwReadData[0] & dwMstpcrcMask) != dwMstpcrcData) {
			memcpy(&s_dwExtMstpcrcData, pbyReadData, sizeof(DWORD));		// 設定内容を記憶
			byRamStop = TRUE;
			// モジュールストップコントロールレジスタC書き込みデータをセット
			dwData = (s_dwExtMstpcrcData & ~(dwMstpcrcMask)) | dwMstpcrcData;
		}

		if( byRamDisable || byRamStop ){		// 設定する必要ありの場合
			// V.1.02 新デバイス対応(ExtFlashModule_014) Append Start
			// RX630/RX210 SYSCR1プロテクト解除処理追加

			// RevRxNo121026-001, RevRxNo130411-001 Modify Line
			if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
				bEsc = TRUE;
				ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
			// V.1.02 新デバイス対応(ExtFlashModule_014) Append End

			if( byRamDisable ){		// SYSCR1設定要の場合
				eAccessSize = eSyscr1Size;
				pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);
				madrRWaddr = madrSyscr1;
				// ExtFlashModule_009 Modify Line (変数定義修正。pVerifyErr -> VerifyErr, ライトデータサイズ修正 1 -> 2 )
				memcpy(pbyWriteData, &wData, sizeof(WORD));
				ferr = SetMcuSfrReg(madrRWaddr,eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_bExtIntRamEnableFlg = TRUE;	// RAM有効
			}
			if( byRamStop ){		// MSTPCRC設定要の場合
				eAccessSize = eMstpcrcSize;
				pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
				madrRWaddr = madrMstpcrc;
				memcpy(pbyWriteData, &dwData, sizeof(DWORD));
				ferr = SetMcuSfrReg(madrRWaddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_bExtRamStartFlg = TRUE;		// RAM動作
			}

			// V.1.02 新デバイス対応(ExtFlashModule_014) Append Start
			// RX630/RX210 SYSCR1プロテクト設定処理追加
			// RevRxNo121026-001, RevRxNo130411-001 Modify Line
			if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
				if( bProtectFree ){		// プロテクト解除した場合(プロテクト設定する必要あり)
					bEsc = FALSE;
					ferr = EscMcuPrcrProtect(bEsc,MCU_REG_SYSTEM_PRCR_PRC1,&bProtectFree);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}
			// V.1.02 新デバイス対応(ExtFlashModule_014) Append End
			// V.1.02 覚え書き32 RAM停止時の処理対応 Modify End(ソースコードDR指摘事項 No.7対応)
		}
	}
	// RevRxNo130301-001 Modify End

	return ferr;
}

//=============================================================================
/**
 * << 外部フラッシュダウンロード時チップイレーズ処理実行関数 >>
 * チップイレーズ指定がある場合のみ全セクタ消去を実施する。
 *    JEDECデバイスの場合:チップイレーズコマンド発行
 *    CUIデバイスの場合:セクタイレーズコマンドを全セクタに対して実行
 * 内蔵RAM領域の場合のみ、内蔵RAM有効/無効を確認し、無効の場合は有効にする。
 * @param byEndian エンディアン
 * @retval FFWエラーコード
 */
//=============================================================================
// ExtFlashModule_002 Modify Line
static FFWERR chipErase(BYTE byEndian)
{

	FFWERR	ferr = FFWERR_OK;
	DWORD	dwCnt;
	WORD	wSect;													// CUIチップイレーズ時のセクタイレーズ番号格納
	WORD	wTimeout;
	DWORD	dwExtGageCoefficient;
	char	byGageMes[EFLASHROM_GAGE_LEN];							// 進捗ゲージ表示文字列格納領域
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// FILLアクセスサイズ
	WORD	wWriteData[1];							// FILLデータ格納領域
	BYTE*	pbyWriteData;							// FILLデータを格納する領域へのポインタ
	pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);
	eAccessSize = MWORD_ACCESS;
	BOOL	bResult;								// ステータスチェック結果(CUI)、セクタロック解除結果(JEDEC)格納変数
	// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
	WORD	wEraseStatus;	
	// RevRxNo120910-007	Append Start
	// RevRxNo121125-005	Delete Line
	FFWERR						ferrExt = FFWERR_OK;				// 外部フラッシュデバイスID取得関連エラーコード
	// RevRxNo120910-007	Append End
	// RevNo121017-003	Append Line
	int	ierr;

	if( e_ExtfInfo[s_byUsdNum].byChipErase ){						// チップイレーズ指定ありの場合
		if( e_ExtfInfo[s_byUsdNum].byWriteProgramType ){			// CUIの場合セクタイレーズを全セクタ分実行
			// 進捗ゲージ初期化
			s_dwExtGageTotal = 0;									// 進捗ゲージ初期化
			s_dwExtGageCnt = 0;
			s_dwExtGageBaseCnt = 0;
			if( e_ExtfInfo[s_byUsdNum].wSectEraseTimeout > 50 ){
				s_dwExtGageTotal = (e_ExtfInfo[s_byUsdNum].wSectEraseTimeout / 50) * e_ExtfInfo[s_byUsdNum].wSectorNum;
			}else{
				s_dwExtGageTotal = e_ExtfInfo[s_byUsdNum].wSectorNum;
			}
			if( s_dwExtGageTotal == 0 ){
				s_dwExtGageTotal = 1;								// 0で割るとバンドルしない例外で終了してしまうため1にしておく
			}
			s_dwExtGageBaseCnt = 1;									// 進捗ゲージは1/1000単位で進ませる
			if( s_dwExtGageTotal < GAGE_COUNT_MAX ){				// s_dwExtGageTotalが1000より小さい場合
				dwExtGageCoefficient = GAGE_COUNT_MAX / s_dwExtGageTotal;
			}else{
				dwExtGageCoefficient = s_dwExtGageTotal / GAGE_COUNT_MAX;
			}
			// ExtFlashModule_007 Append Line
			SetGageCount(0);	// 進捗ゲージカウンタ用変数初期化

			// RevNo121017-003	Modify Line
			ierr = sprintf_s(byGageMes, _countof(byGageMes), "External Flash [%02x] Chip Erase", s_byUsdNum );

			// add IDCheck 2010.3.9 start
			ferr = PROT_MCU_EFWRITESTART(byEndian, e_ExtfInfo[s_byUsdNum].byConnect, s_eAccessSize, e_ExtfInfo[s_byUsdNum].dwWorkStart, 
							e_ExtfInfo[s_byUsdNum].dwBuffStart, e_ExtfInfo[s_byUsdNum].wBuffSize, e_ExtfInfo[s_byUsdNum].dwSectAddr[0], 0,0,0,0,0,0 );
																	// 0,0,0,0,0,0 : JEDECコマンド設定用のためCUIの場合は0としている
			if (ferr != FFWERR_OK) {
				s_ExtFerr = ferr;
				goto err;
			}
			// ExtFlashModule_008 Append Line
			s_bSendProtEfwriteStart = TRUE;	// PROT_MCU_EFWRITESTART()送信済み

			if( !s_byDiffIDFlg[s_byUsdNum] ){
				// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
				ferr = PROT_MCU_EFWRITE(EFLASHROM_CMP_ID, 0, (DWORD)e_ExtfInfo[s_byUsdNum].wMakerID, (DWORD)e_ExtfInfo[s_byUsdNum].wDeviceID, 0, NULL, &wEraseStatus);
																	// 2番目の引数0 : セクタイレーズタイムアウト値(未使用のため0としている)
																	// 5番目の引数0 : セクタアドレス(未使用のため0としている)
																	// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					// RevRxNo121122-005	Modify Start
					// RevRxNo120910-007	Append Start
					// getExtFlashID(s_byUsdNum,&data)をコールする。
					ferrExt = getExtFlashID(s_byUsdNum, &s_ferdiData);
					if( ferrExt == FFWERR_OK ) {
						// (ⅰ）getExtFlashID の戻り値がFFWERR_OKの場合
						//	<1>	data.dwErrorCodeにエラーコード（FFWERR_EXTROM_ID）をセットする。
						s_ferdiData.dwErrorCode = FFWERR_EXTROM_ID;
						// RevRxNo121122-005	Modify End
					}else {
						// (ⅱ）getExtFlashID の戻り値がFFWERR_OK以外の場合
						// 		エラーコード格納の変数ferrに戻り値を上書きする。
						ferr = ferrExt;
					}
					// RevRxNo120910-007	Append End
					goto err;
				}
				s_byDiffIDFlg[s_byUsdNum] = TRUE;					// 比較済みに設定
			}
			// add IDCheck 2010.3.9 end
			for( wSect = 0; wSect < e_ExtfInfo[s_byUsdNum].wSectorNum; wSect++ ){
				ferr = PROT_MCU_EFWRITESTART(byEndian, e_ExtfInfo[s_byUsdNum].byConnect, s_eAccessSize, e_ExtfInfo[s_byUsdNum].dwWorkStart, 
								e_ExtfInfo[s_byUsdNum].dwBuffStart, e_ExtfInfo[s_byUsdNum].wBuffSize, e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], 0,0,0,0,0,0 );
																	// 0,0,0,0,0,0 : JEDECコマンド設定用のためCUIの場合は0としている
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					goto err;
				}
				// ExtFlashModule_008 Append Line
				s_bSendProtEfwriteStart = TRUE;	// PROT_MCU_EFWRITESTART()送信済み

				if( e_ExtfInfo[s_byUsdNum].byUnlock == 0 ){			// ロック解除指定なし
					// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
					ferr = PROT_MCU_EFWRITE(EFLASHROM_ERASE, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], NULL, &wEraseStatus);
																	// 0, 0, 0 : セクタイレーズタイムアウト値, メーカIDD, デバイスID(未使用のため0としている)
																	// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						goto err;
					}
				}else{												// ロック解除指定あり
					// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
					ferr = PROT_MCU_EFWRITE(EFLASHROM_UNLOCK_ERASE, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], NULL, &wEraseStatus);
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						goto err;
					}
				}
				if( e_ExtfInfo[s_byUsdNum].wSectEraseTimeout > 50 ){
					wTimeout = (WORD)(e_ExtfInfo[s_byUsdNum].wSectEraseTimeout / 50);
				}else{
					wTimeout = 1;
				}
				for( dwCnt = 0; dwCnt < (DWORD)wTimeout; dwCnt++ ) {
					// イレーズステータスチェック
					// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
					ferr = PROT_MCU_EFWRITE(EFLASHROM_STATUS_CHK, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], NULL, &wEraseStatus);
// ExtFlashModule_005 Modify Start
// イレーズステータスチェック結果はエラーコードとは別に受信するよう変更。
					if (ferr == FFWERR_OK) {
						if (wEraseStatus == EFLASHROM_ERASE_FINISH) {	// イレーズ完
							break;
						}
					} else if (ferr == FFWERR_EXTROM_ERASE) {			// イレーズエラー発生時
						ferr = cuiStatusCheck(wSect, &bResult);				// ステータスチェック実行
						if (ferr != FFWERR_OK) {
							s_ExtFerr = ferr;
							goto err;
						}
						if( bResult ){		// ロックがかかっている場合(以降の処理を続行する)
							ferr = FFWERR_OK;
// ExtFlashModule_002b Modify Start
							s_bSectLock[s_byUsdNum][wSect] = TRUE;	// このセクタはロックがかかっている
							s_bExistLockSect = TRUE;	// ロックされているセクタあり
// ExtFlashModule_002b Modify End
							break;
						}else{				// ロックがかかっていない場合(ここでエラー終了する)
							s_ExtFerr = FFWERR_EXTROM_ERASE;
							goto err;
						}
					} else {		// イレーズエラー以外のエラー発生時
						s_ExtFerr = ferr;
						goto err;
					}
// ExtFlashModule_005 Modify End

					COM_WaitMs(50);									// 50ms待ち
					s_dwExtGageCnt++;
					if( !(s_dwExtGageCnt % s_dwExtGageBaseCnt) ){
						if( s_dwExtGageTotal < GAGE_COUNT_MAX ){	// トータルが1000より小さい場合
							SetGageCount((s_dwExtGageCnt * dwExtGageCoefficient) / s_dwExtGageBaseCnt);
						}else{										// トータルが100より大きい場合
							SetGageCount((s_dwExtGageCnt / dwExtGageCoefficient) / s_dwExtGageBaseCnt);
						}
						SetMsgGageCount(byGageMes);
					}
				}
				if( dwCnt == (DWORD)wTimeout ){
					s_ExtFerr = FFWERR_EXTROM_ERASE;
					goto err;
				}
			}		
			// 進捗100%ゲージを300ms間表示
			for( dwCnt = 0; dwCnt < 3; dwCnt++ ){
				SetGageCount(1000 / 1);
				SetMsgGageCount(byGageMes);
				COM_WaitMs(100);									// 100ms待ち
			}
		}else{														// JEDECの場合、チップイレーズコマンド発行
			if( e_ExtfInfo[s_byUsdNum].byUnlock == 0 ){				// ロック解除指定なし
				// 進捗ゲージ初期化
				s_dwExtGageTotal = 0;								// 進捗ゲージ初期化
				s_dwExtGageCnt = 0;
				s_dwExtGageBaseCnt = 0;
				s_dwExtGageTotal = e_ExtfInfo[s_byUsdNum].wChipEraseTimeout;
				if( s_dwExtGageTotal == 0 ){
					s_dwExtGageTotal = 1;							// 0で割るとバンドルしない例外で終了してしまうため1にしておく
				}
				s_dwExtGageBaseCnt = 1;								// 進捗ゲージは1/1000単位で進ませる
				if( s_dwExtGageTotal < GAGE_COUNT_MAX ){			// s_dwExtGageTotalが1000より小さい場合
					dwExtGageCoefficient = GAGE_COUNT_MAX / s_dwExtGageTotal;
				}else{
					dwExtGageCoefficient = s_dwExtGageTotal / GAGE_COUNT_MAX;
				}
				// ExtFlashModule_007 Append Line
				SetGageCount(0);	// 進捗ゲージカウンタ用変数初期化

				// RevNo121017-003	Modify Line
				ierr = sprintf_s(byGageMes, _countof(byGageMes), "External Flash [%02x] Chip Erase", s_byUsdNum );

				ferr = PROT_MCU_EFWRITESTART(byEndian, e_ExtfInfo[s_byUsdNum].byConnect, s_eAccessSize, e_ExtfInfo[s_byUsdNum].dwWorkStart, 
								e_ExtfInfo[s_byUsdNum].dwBuffStart, e_ExtfInfo[s_byUsdNum].wBuffSize, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum],
								e_ExtfInfo[s_byUsdNum].wCommandAddr8_1, e_ExtfInfo[s_byUsdNum].wCommandAddr8_2, e_ExtfInfo[s_byUsdNum].wCommandAddr16_1, 
								e_ExtfInfo[s_byUsdNum].wCommandAddr16_2, e_ExtfInfo[s_byUsdNum].wCommandAddr32_1, e_ExtfInfo[s_byUsdNum].wCommandAddr32_2 );
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					goto err;
				}
				// ExtFlashModule_008 Append Line
				s_bSendProtEfwriteStart = TRUE;	// PROT_MCU_EFWRITESTART()送信済み

				// add IDCheck 2010.3.9 begin
				if( !s_byDiffIDFlg[s_byUsdNum] ){
					// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
					ferr = PROT_MCU_EFWRITE(EFLASHROM_CMP_ID, 0, (DWORD)e_ExtfInfo[s_byUsdNum].wMakerID, (DWORD)e_ExtfInfo[s_byUsdNum].wDeviceID, 0, NULL, &wEraseStatus);
																	// 2番目の引数0 : セクタイレーズタイムアウト値(未使用のため0としている)
																	// 5番目の引数0 : セクタアドレス(未使用のため0としている)
																	// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						// RevRxNo121122-005	Modify Start
						// RevRxNo120910-007	Append Start
						// getExtFlashID(s_byUsdNum,&data)をコールする。
						ferrExt = getExtFlashID(s_byUsdNum, &s_ferdiData);
						if( ferrExt == FFWERR_OK ) {
							// (ⅰ）getExtFlashID の戻り値がFFWERR_OKの場合
							//	<1>	data.dwErrorCodeにエラーコード（FFWERR_EXTROM_ID）をセットする。
							s_ferdiData.dwErrorCode = FFWERR_EXTROM_ID;
							// RevRxNo121122-005	Modify End
						}else {
							// (ⅱ）getExtFlashID の戻り値がFFWERR_OK以外の場合
							// 		エラーコード格納の変数ferrに戻り値を上書きする。
							ferr = ferrExt;
						}
						// RevRxNo120910-007	Append End
						goto err;
					}
					s_byDiffIDFlg[s_byUsdNum] = TRUE;				// 比較済みに設定
				}
				// add IDCheck 2010.3.9 end

				// ExtFlashModule_002b Append Start
				// ロック状態の確認
				ferr = checkJedecLockBit();
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					goto err;
				}
				// ExtFlashModule_002b Append End

				// チップイレーズ
				// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
				ferr = PROT_MCU_EFWRITE(EFLASHROM_CHIP_ERASE, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], NULL, &wEraseStatus);
																	// 0, 0, 0 : セクタイレーズタイムアウト値, メーカIDD, デバイスID(未使用のため0としている)
																	// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					goto err;
				}
				for( dwCnt = 0; dwCnt < (DWORD)e_ExtfInfo[s_byUsdNum].wChipEraseTimeout; dwCnt++ ) {
					// ステータスチェックの間隔を1ｓ間隔くらいにする必要あり（無駄なUSB通信をなくすため）
					// チップイレーズステータスチェック
					// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
					ferr = PROT_MCU_EFWRITE(EFLASHROM_STATUS_CHK, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], NULL, &wEraseStatus);
//ExtFlashModule_005 Modify Start
// イレーズステータスチェック結果はエラーコードとは別に受信するよう変更。
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						goto err;
					}
					if (wEraseStatus == EFLASHROM_ERASE_FINISH) {	// イレーズ完
						break;
					}
//ExtFlashModule_005 Modify End
					COM_WaitMs(1000);								// 1秒待ち
					s_dwExtGageCnt++;
					if( !(s_dwExtGageCnt % s_dwExtGageBaseCnt) ){
						if( s_dwExtGageTotal < GAGE_COUNT_MAX ){	// トータルが1000より小さい場合
							SetGageCount((s_dwExtGageCnt * dwExtGageCoefficient) / s_dwExtGageBaseCnt);
						}else{										// トータルが100より大きい場合
							SetGageCount((s_dwExtGageCnt / dwExtGageCoefficient) / s_dwExtGageBaseCnt);
						}
						SetMsgGageCount(byGageMes);
					}
				}
				if( dwCnt == (DWORD)e_ExtfInfo[s_byUsdNum].wChipEraseTimeout ){
					s_ExtFerr = FFWERR_EXTROM_ERASE;
					goto err;
				}
				// 進捗100%ゲージを300ms間表示
				for( dwCnt = 0; dwCnt < 3; dwCnt++ ){
					SetGageCount(1000 / 1);
					SetMsgGageCount(byGageMes);
					COM_WaitMs(100);								// 100ms待ち
				}
			// add UnlockChipErase for JEDEC 2010.3.10 start
			}else{		// ロック解除付きイレーズ指定ありの場合
				// 進捗ゲージ初期化
				s_dwExtGageTotal = 0;								// 進捗ゲージ初期化
				s_dwExtGageCnt = 0;
				s_dwExtGageBaseCnt = 0;
				s_dwExtGageTotal = e_ExtfInfo[s_byUsdNum].wChipEraseTimeout;
				if( s_dwExtGageTotal == 0 ){
					s_dwExtGageTotal = 1;							// 0で割るとバンドルしない例外で終了してしまうため1にしておく
				}
				s_dwExtGageBaseCnt = 1;								// 進捗ゲージは1/1000単位で進ませる
				if( s_dwExtGageTotal < GAGE_COUNT_MAX ){			// s_dwExtGageTotalが1000より小さい場合
					dwExtGageCoefficient = GAGE_COUNT_MAX / s_dwExtGageTotal;
				}else{
					dwExtGageCoefficient = s_dwExtGageTotal / GAGE_COUNT_MAX;
				}
				// ExtFlashModule_007 Append Line
				SetGageCount(0);	// 進捗ゲージカウンタ用変数初期化

				// RevNo121017-003	Modify Line
				ierr = sprintf_s(byGageMes, _countof(byGageMes), "External Flash [%02x] Chip Erase", s_byUsdNum );

				ferr = PROT_MCU_EFWRITESTART(byEndian, e_ExtfInfo[s_byUsdNum].byConnect, s_eAccessSize, e_ExtfInfo[s_byUsdNum].dwWorkStart, 
								e_ExtfInfo[s_byUsdNum].dwBuffStart, e_ExtfInfo[s_byUsdNum].wBuffSize, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum],
								e_ExtfInfo[s_byUsdNum].wCommandAddr8_1, e_ExtfInfo[s_byUsdNum].wCommandAddr8_2, e_ExtfInfo[s_byUsdNum].wCommandAddr16_1, 
								e_ExtfInfo[s_byUsdNum].wCommandAddr16_2, e_ExtfInfo[s_byUsdNum].wCommandAddr32_1, e_ExtfInfo[s_byUsdNum].wCommandAddr32_2 );
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					goto err;
				}
				// ExtFlashModule_008 Append Line
				s_bSendProtEfwriteStart = TRUE;	// PROT_MCU_EFWRITESTART()送信済み

				// add IDCheck 2010.3.9 begin
				if( !s_byDiffIDFlg[s_byUsdNum] ){
					// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
					ferr = PROT_MCU_EFWRITE(EFLASHROM_CMP_ID, 0, (DWORD)e_ExtfInfo[s_byUsdNum].wMakerID, (DWORD)e_ExtfInfo[s_byUsdNum].wDeviceID, 0x0, NULL, &wEraseStatus);
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						// RevRxNo121125-005	Modify Start
						// RevRxNo120910-007	Append Start
						// getExtFlashID(s_byUsdNum,&data)をコールする。
						ferrExt = getExtFlashID(s_byUsdNum, &s_ferdiData);
						if( ferrExt == FFWERR_OK ) {
							// (ⅰ）getExtFlashID の戻り値がFFWERR_OKの場合
							//	<1>	data.dwErrorCodeにエラーコード（FFWERR_EXTROM_ID）をセットする。
							s_ferdiData.dwErrorCode = FFWERR_EXTROM_ID;
							// RevRxNo121125-005	Append End
						}else {
							// (ⅱ）getExtFlashID の戻り値がFFWERR_OK以外の場合
							// 		エラーコード格納の変数ferrに戻り値を上書きする。
							ferr = ferrExt;
						}
						// RevRxNo120910-007	Append End
						goto err;
					}
					s_byDiffIDFlg[s_byUsdNum] = TRUE;				// 比較済みに設定
				}
				// add IDCheck 2010.3.9 end

				// add UnLock 2010.3.17 start
				// Verify Lock bitコマンド発行( JEDECの場合、ロックがかけられるのは16bitモードのみ )
				if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM16 ){	// 接続形態がMCU16-ROM16の場合のみロック確認&解除実施
					// リセットコマンド(シーケンスクリア)(念のため)
					// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
					ferr = PROT_MCU_EFWRITE(EFLASHROM_EXIT, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[0], NULL, &wEraseStatus);
																	// 0, 0, 0 : セクタイレーズタイムアウト値, メーカIDD, デバイスID(未使用のため0としている)
																	// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						goto err;
					}

					// 全セクタロック解除
					for( wSect = 0; wSect < e_ExtfInfo[s_byUsdNum].wSectorNum; wSect++ ){
						ferr = jedecUnlock(wSect, &bResult);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						if( bResult ){		// セクタロック解除成功
							// 何もしない
						}else{				// 解除したのにロックかかっている
							s_ExtFerr = FFWERR_EXTROM_ERASE;
							goto err;
						}
					}
				}
				// add UnLock 2010.3.17 end

				// チップイレーズ
				// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
				ferr = PROT_MCU_EFWRITE(EFLASHROM_CHIP_ERASE, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], NULL, &wEraseStatus);
																	// 0, 0, 0 : セクタイレーズタイムアウト値, メーカIDD, デバイスID(未使用のため0としている)
																	// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
				if (ferr != FFWERR_OK) {
					s_ExtFerr = ferr;
					goto err;
				}

				for( dwCnt = 0; dwCnt < (DWORD)e_ExtfInfo[s_byUsdNum].wChipEraseTimeout; dwCnt++ ) {
					// ステータスチェックの間隔を1ｓ間隔くらいにする必要あり（無駄なUSB通信をなくすため）
					// チップイレーズステータスチェック
					// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
					ferr = PROT_MCU_EFWRITE(EFLASHROM_STATUS_CHK, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[s_wSectNum], NULL, &wEraseStatus);
					//ExtFlashModule_005 Modify Start
					// イレーズステータスチェック結果はエラーコードとは別に受信するよう変更。
					if (ferr != FFWERR_OK) {
						s_ExtFerr = ferr;
						goto err;
					}
					if (wEraseStatus == EFLASHROM_ERASE_FINISH) {	// イレーズ完
						break;
					}
					//ExtFlashModule_005 Modify End
					COM_WaitMs(1000);								// 1秒待ち
					s_dwExtGageCnt++;
					if( !(s_dwExtGageCnt % s_dwExtGageBaseCnt) ){
						if( s_dwExtGageTotal < GAGE_COUNT_MAX ){	// トータルが1000より小さい場合
							SetGageCount((s_dwExtGageCnt * dwExtGageCoefficient) / s_dwExtGageBaseCnt);
						}else{										// トータルが100より大きい場合
							SetGageCount((s_dwExtGageCnt / dwExtGageCoefficient) / s_dwExtGageBaseCnt);
						}
						SetMsgGageCount(byGageMes);
					}
				}
				if( dwCnt == (DWORD)e_ExtfInfo[s_byUsdNum].wChipEraseTimeout ){
					s_ExtFerr = FFWERR_EXTROM_ERASE;
					goto err;
				}
				// 進捗100%ゲージを300ms間表示
				for( dwCnt = 0; dwCnt < 3; dwCnt++ ){
					SetGageCount(1000 / 1);
					SetMsgGageCount(byGageMes);
					COM_WaitMs(100);								// 100ms待ち
				}

			}
			// add UnlockChipErase for JEDEC 2010.3.10 end
		}
	}

err:
	return ferr;
}


//=============================================================================
/**
 * << 外部フラッシュダウンロード時CUIデバイスのロック確認関数 >>
 * CUIデバイスにロックがかかっている場合、イレーズエラーが発生しその時点で処理
 * を終了してしまうため、JEDECデバイスと処理を同じくするため、ロックがかかって
 * いる場合はその旨を呼び出し側に返しイレーズ処理を続行させる。
 * @param wSect ロック確認対象セクタ番号
 * @param bResult ロック確認結果格納変数へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
// ExtFlashModule_002 Append Start
static FFWERR cuiStatusCheck(WORD wSect, BOOL *bResult)
{

	FFWERR	ferr = FFWERR_OK;
	FFW_VERIFYERR_DATA			VerifyErr;			// ベリファイ結果格納構造体(VERIFY_OFFのため未使用)
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// FILLアクセスサイズ
	WORD	wWriteData[1];							// FILLデータ格納領域
	DWORD	dwWriteDataSize;						// FILLデータサイズ
	BYTE*	pbyWriteData;							// FILLデータを格納する領域へのポインタ
	pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);
	eAccessSize = MWORD_ACCESS;
	BYTE	bReadBuff[2];							// DUMPデータ格納領域
	MADDR	madrCommandAddr;						// コマンド発行アドレス

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	madrCommandAddr = e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect];	// コマンド発行アドレス設定

	// ロックがかかているためにイレーズエラーとなった場合は、セクタイレーズを続行するべき
	if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM16 ){	// 接続形態がMCU16-ROM16の場合
		//ステータスチェックコマンド発行
		// 0x0070発行
		dwWriteDataSize = 2;							// FILLデータサイズは2固定(E1/E20は未使用のため)
		eAccessSize = MWORD_ACCESS;
		pbyWriteData[0] = EFLASHROM_COMMAND_00;
		pbyWriteData[1] = EFLASHROM_COMMAND_CUI_STAT;
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_FILL(madrCommandAddr, (madrCommandAddr + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_DUMP(madrCommandAddr, (madrCommandAddr + 1), eAccessSize, bReadBuff);	// DUMP処理実行
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// ExtFlashModule_002a Modify Line
		if( (bReadBuff[1] & EFLASHROM_STATUS_BLOCK) && (bReadBuff[1] & EFLASHROM_STATUS_COMPLETE) ){	// ロックかかっている かつ イレーズが完了している
			*bResult = TRUE;			// ロックがかかっている(そのためイレーズできなかった)
		}else{
			*bResult = FALSE;			// ロックはかかっていない(別の原因でイレーズできなかった)
		}
	}else if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU8_ROM8 ){	// 接続形態がMCU8-ROM8の場合
		//ステータスチェックコマンド発行
		// 0x70発行
		dwWriteDataSize = 1;								// FILLデータサイズは2固定(E1/E20は未使用のため)
		eAccessSize = MBYTE_ACCESS;
		pbyWriteData[0] = EFLASHROM_COMMAND_CUI_STAT;
		pbyWriteData[1] = EFLASHROM_COMMAND_00;				// BYTEアクセスで未使用のため0で初期化
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_FILL(madrCommandAddr, madrCommandAddr, VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_DUMP(madrCommandAddr, madrCommandAddr, eAccessSize, bReadBuff);	// DUMP処理実行
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// ExtFlashModule_002a Modify Line
		if( (bReadBuff[0] & EFLASHROM_STATUS_BLOCK) && (bReadBuff[0] & EFLASHROM_STATUS_COMPLETE) ){	// ロックかかっている かつ イレーズが完了している
			*bResult = TRUE;			// ロックがかかっている(そのためイレーズできなかった)
		}else{
			*bResult = FALSE;			// ロックはかかっていない(別の原因でイレーズできなかった)
		}
	}else if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM8_2 ){	// 接続形態がMCU16-ROM8の場合
		//ステータスチェックコマンド発行
		// 0x7070発行
		dwWriteDataSize = 2;							// FILLデータサイズは2固定(E1/E20は未使用のため)
		eAccessSize = MWORD_ACCESS;
		pbyWriteData[0] = EFLASHROM_COMMAND_CUI_STAT;
		pbyWriteData[1] = EFLASHROM_COMMAND_CUI_STAT;				// BYTEアクセスで未使用のため0で初期化
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_FILL(madrCommandAddr, (madrCommandAddr + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_DUMP(madrCommandAddr, (madrCommandAddr + 1), eAccessSize, bReadBuff);	// DUMP処理実行
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// ExtFlashModule_002a Modify Line
		if( ((bReadBuff[0] & EFLASHROM_STATUS_BLOCK) && (bReadBuff[0] & EFLASHROM_STATUS_COMPLETE))
				&& ((bReadBuff[1] & EFLASHROM_STATUS_BLOCK) && (bReadBuff[1] & EFLASHROM_STATUS_COMPLETE)) ){
			// ロックかかっている かつ イレーズが完了している
			*bResult = TRUE;			// ロックがかかっている(そのためイレーズできなかった)
		}else{
			*bResult = FALSE;			// ロックはかかっていない(別の原因でイレーズできなかった)
		}
	}
	return ferr;

}
// ExtFlashModule_002 Append End


//=============================================================================
/**
 * << 外部フラッシュダウンロード時JEDECデバイスセクタロック解除関数 >>
 * ロック解除つきチップイレーズ処理用。チップイレーズコマンド発行前に全セクタ
 * に対してロック解除コマンドを発行し、ロックを解除する。
 * @param wSect ロック解除対象セクタ番号
 * @param bResult ロック解除結果格納変数へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
// ExtFlashModule_002 Append Start
static FFWERR jedecUnlock(WORD wSect, BOOL *bResult)
{

	FFWERR	ferr = FFWERR_OK;
	MADDR	dwmadrCommandAddr1;
	MADDR	dwmadrCommandAddr2;
	FFW_VERIFYERR_DATA			VerifyErr;			// ベリファイ結果格納構造体(VERIFY_OFFのため未使用)
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// FILLアクセスサイズ
	WORD	wWriteData[1];							// FILLデータ格納領域
	DWORD	dwWriteDataSize;						// FILLデータサイズ
	BYTE*	pbyWriteData;							// FILLデータを格納する領域へのポインタ
	pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);
	eAccessSize = MWORD_ACCESS;
	BYTE	bReadBuff[2];							// DUMPデータ格納領域
	// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
	WORD	wEraseStatus;

	eAccessSize = MWORD_ACCESS;						// FILLアクセスサイズはワード固定
	dwWriteDataSize = 2;							// FILLデータサイズは2バイト固定
	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	dwmadrCommandAddr1 = e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect] | (e_ExtfInfo[s_byUsdNum].wCommandAddr16_1 << 1);
	dwmadrCommandAddr2 = e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect] | (e_ExtfInfo[s_byUsdNum].wCommandAddr16_2 << 1);
	// ClearLockbitCommand発行
	// 0x00AA発行
	pbyWriteData[0] = EFLASHROM_COMMAND_00;
	pbyWriteData[1] = EFLASHROM_COMMAND_AA;
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
	ferr = DO_FILL(dwmadrCommandAddr1, (dwmadrCommandAddr1 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// 0x0055発行
	pbyWriteData[0] = EFLASHROM_COMMAND_00;
	pbyWriteData[1] = EFLASHROM_COMMAND_55;
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
	ferr = DO_FILL(dwmadrCommandAddr2, (dwmadrCommandAddr2 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// 0x0048発行
	pbyWriteData[0] = EFLASHROM_COMMAND_00;
	pbyWriteData[1] = EFLASHROM_COMMAND_UNLOCK;
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
	ferr = DO_FILL(dwmadrCommandAddr1, (dwmadrCommandAddr1 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// 0x0000発行(ロック解除)
	pbyWriteData[0] = EFLASHROM_COMMAND_00;
	pbyWriteData[1] = EFLASHROM_COMMAND_00;
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
	ferr = DO_FILL(e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], (e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect] + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// リセットコマンド(シーケンスクリア)
	// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
	ferr = PROT_MCU_EFWRITE(EFLASHROM_EXIT, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], NULL, &wEraseStatus);
												// 0, 0, 0 : セクタイレーズタイムアウト値, メーカIDD, デバイスID(未使用のため0としている)
												// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		return ferr;
	}

	// VerifyLockbitCommand発行
	// 0x00AA発行
	pbyWriteData[0] = EFLASHROM_COMMAND_00;
	pbyWriteData[1] = EFLASHROM_COMMAND_AA;
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
	ferr = DO_FILL(dwmadrCommandAddr1, (dwmadrCommandAddr1 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// 0x0055発行
	pbyWriteData[0] = EFLASHROM_COMMAND_00;
	pbyWriteData[1] = EFLASHROM_COMMAND_55;
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
	ferr = DO_FILL(dwmadrCommandAddr2, (dwmadrCommandAddr2 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// 0x0058発行
	pbyWriteData[0] = EFLASHROM_COMMAND_00;
	pbyWriteData[1] = EFLASHROM_COMMAND_VERIFY;
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
	ferr = DO_FILL(dwmadrCommandAddr1, (dwmadrCommandAddr1 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// ロック状態取得
// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
	ferr = DO_DUMP(e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], (e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect] + 1), eAccessSize, bReadBuff);	// DUMP処理実行
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if( bReadBuff[1] & EFLASHROM_STATUS_LOCKCHK ){		// 解除したのにロックかかっている
		*bResult = FALSE;		// セクタロック解除失敗
	}else{
		*bResult = TRUE;		// セクタロック解除成功
	}

	// リセットコマンド(シーケンスクリア)
	// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
	ferr = PROT_MCU_EFWRITE(EFLASHROM_EXIT, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], NULL, &wEraseStatus);
												// 0, 0, 0 : セクタイレーズタイムアウト値, メーカIDD, デバイスID(未使用のため0としている)
												// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
	if (ferr != FFWERR_OK) {
		s_ExtFerr = ferr;
		return ferr;
	}

	return ferr;

}
// ExtFlashModule_002 Append End


//=============================================================================
/**
 * << 外部フラッシュダウンロード時JEDECデバイスセクタロック確認関数 >>
 * JEDECデバイスはセクタにロックがかかっている状態でイレーズ・ライトを実行して
 * もエラーが発生しないため、本当にデータがライトできたかどうかがわかりにくい。
 * ロックがかかっているセクタがあった場合はユーザにその旨を通知するため、ロック
 * 確認を実行する。
 * @param wSect ロック解除対象セクタ番号
 * @param pbLockStatus ロック解除結果格納変数へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
// ExtFlashModule_002b Append Start
// JEDEC方式デバイス ベリファイロック(ロック状態の確認)
static FFWERR jedecVerifyLockbit(WORD wSect, BOOL* pbLockStatus)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	dwmadrCommandAddr1;
	MADDR	dwmadrCommandAddr2;
	FFW_VERIFYERR_DATA			VerifyErr;			// ベリファイ結果格納構造体(VERIFY_OFFのため未使用)
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// FILLアクセスサイズ
	WORD	wWriteData[1];							// FILLデータ格納領域
	DWORD	dwWriteDataSize;						// FILLデータサイズ
	BYTE*	pbyWriteData;							// FILLデータを格納する領域へのポインタ
	BYTE	bReadBuff[2];							// DUMPデータ格納領域
	// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
	WORD	wEraseStatus;

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	// 接続形態がMCU16-ROM16以外の場合
	if (e_ExtfInfo[s_byUsdNum].byConnect != EFLASHROM_MCU16_ROM16) {
		*pbLockStatus = FALSE;	// ロックかかっていない


	// 接続形態がMCU16-ROM16の場合ロック確認実施
	} else {
		pbyWriteData = reinterpret_cast<BYTE*>(wWriteData);

		eAccessSize = MWORD_ACCESS;						// FILLアクセスサイズはワード固定
		dwWriteDataSize = 2;							// FILLデータサイズは2バイト固定

		dwmadrCommandAddr1 = e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect] | (e_ExtfInfo[s_byUsdNum].wCommandAddr16_1 << 1);
		dwmadrCommandAddr2 = e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect] | (e_ExtfInfo[s_byUsdNum].wCommandAddr16_2 << 1);

		// VerifyLockbitCommand発行
		// 0x00AA発行
		pbyWriteData[0] = EFLASHROM_COMMAND_00;
		pbyWriteData[1] = EFLASHROM_COMMAND_AA;
	// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_FILL(dwmadrCommandAddr1, (dwmadrCommandAddr1 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// 0x0055発行
		pbyWriteData[0] = EFLASHROM_COMMAND_00;
		pbyWriteData[1] = EFLASHROM_COMMAND_55;
	// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_FILL(dwmadrCommandAddr2, (dwmadrCommandAddr2 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// 0x0058発行
		pbyWriteData[0] = EFLASHROM_COMMAND_00;
		pbyWriteData[1] = EFLASHROM_COMMAND_VERIFY;
	// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_FILL(dwmadrCommandAddr1, (dwmadrCommandAddr1 + 1), VERIFY_OFF, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// ロック状態取得
	// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
		ferr = DO_DUMP(e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], (e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect] + 1), eAccessSize, bReadBuff);	// DUMP処理実行
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if( bReadBuff[1] & EFLASHROM_STATUS_LOCKCHK ){
			*pbLockStatus = TRUE;	// ロックかかっている
		}else{
			*pbLockStatus = FALSE;	// ロックかかっていない
		}

		// リセットコマンド(シーケンスクリア)
		// ExtFlashModule_005 Modify Line ( PROT_MCU_EFWRITE()の引数に &wEraseStatusを追加 )
		ferr = PROT_MCU_EFWRITE(EFLASHROM_EXIT, 0, 0, 0, e_ExtfInfo[s_byUsdNum].dwSectAddr[wSect], NULL, &wEraseStatus);
													// 0, 0, 0 : セクタイレーズタイムアウト値, メーカIDD, デバイスID(未使用のため0としている)
													// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
		if (ferr != FFWERR_OK) {
			return ferr;
		}

	}

	return ferr;
}
// ExtFlashModule_002b Append End


//=============================================================================
/**
 * << 外部フラッシュダウンロード時JEDECデバイス全セクタロック確認関数 >>
 * チップイレーズコマンド発行前に全セクタに対してロック確認を実行する。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
// ExtFlashModule_002b Append Start
// JEDEC方式デバイス 全セクタのロック状態の確認
static FFWERR checkJedecLockBit(void)
{
	FFWERR	ferr = FFWERR_OK;
	WORD	wSect;
	BOOL	bLockStatus;

	// ロックがかかっているかを確認する
	if( e_ExtfInfo[s_byUsdNum].byConnect == EFLASHROM_MCU16_ROM16 ){	// 接続形態がMCU16-ROM16の場合のみロック確認実施
		for (wSect = 0; wSect < e_ExtfInfo[s_byUsdNum].wSectorNum; wSect++) {
			ferr = jedecVerifyLockbit(wSect, &bLockStatus);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			if (bLockStatus == TRUE) {
				s_bSectLock[s_byUsdNum][wSect] = TRUE;	// このセクタはロックがかかっている
				s_bExistLockSect = TRUE;	// ロックされているセクタあり
			}
		}
	}

	return ferr;
}
// ExtFlashModule_002b Append End


//=============================================================================
/**
 * << 外部フラッシュダウンロード用USDファイル番号、セクタ番号取得関数 >>
 * チップイレーズコマンド発行前に全セクタに対してロック確認を実行する。
 * @param  madrStartAddr　ダウンロードデータ開始アドレス
 * @param  madrEndAddr　ダウンロードデータ終了アドレス
 * @param  pbUsdNum  ダウンロードデータに対応するUSDファイル番号格納変数へのポインタ
 * @param  pwSectNum　ダウンロードデータに対応するセクタ番号格納変数へのポインタ
 * @retval FFWエラーコード
 */
//=============================================================================
static BOOL setExtFlashDataSetSect(MADDR madrStartAddr, MADDR madrEndAddr, BYTE *pbUsdNum, WORD *pwSectNum)
{
	BYTE	byUsd;
	WORD	bySect;

	madrEndAddr;

	for( byUsd = 0; byUsd < MAX_USD_FILE; byUsd++ ){
		if( !e_ExtfInfo[byUsd].wSectorNum ){	// 未登録の場合
			break;
		}
		for( bySect = 0; bySect < e_ExtfInfo[byUsd].wSectorNum; bySect++ ){
			if( e_ExtfInfo[byUsd].dwSectAddr[bySect] <= madrStartAddr 
					&& madrStartAddr <= (e_ExtfInfo[byUsd].dwSectAddr[bySect]+e_ExtfInfo[byUsd].dwSectSize[bySect]-1)) {
				*pwSectNum = bySect;
				*pbUsdNum = byUsd;
				return TRUE;
			}
		}
	}
	return FALSE;

}


//=============================================================================
/**
 * << 外部フラッシュダウンロード用スクリプトファイル解析関数 >>
 * 外部フラッシュダウンロード前、後に実行する外部バス設定スクリプトを実行する。
 * @param  bUsdNum  スクリプト解析対象USDファイル番号
 * @param  wMode スクリプトの種類
 *          0 : 実行前外部RAMバス設定用
 *          1 : 実行後外部RAMバス設定用
 *          2 : 実行前外部フラッシュバス設定用
 *          3 : 実行後外部フラッシュバス設定用
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR exeScriptCmd(BYTE bUsdNum, BYTE bMode)
{
	FFWERR	ferr = FFWERR_OK;
	FILE	*fp;						// ファイル
	char	data_buffer[MAX_SCRIPT_1LINE_DATA];			// データバッファ
	BYTE	filename[MAX_PATH_SIZE];
	char	*p;
	MADDR 	madrStartAddr;
	MADDR 	madrEndAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	enum FFWENM_VERIFY_SET		eVerify = VERIFY_OFF;			// ベリファイON/OFF
	DWORD	dwWriteDataSize = 4;		// 初期値はLWORD(4)にしておく
	BYTE	cnt, len;					// 位置確認用
	DWORD	dwWriteData[1];				// 設定データ格納領域
	DWORD	dwdata;						// 作業用変数
	BYTE	*pbyWriteData;
	// RevNo121017-003	Append Line
	errno_t	ernerr;

//ExtFlashModule_009 Modify Line 
//    変数定義修正 pVerifyErr -> &VerifyErr
	FFW_VERIFYERR_DATA	VerifyErr;

	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);

	// スクリプトファイルオープンファイル名コピー
	switch( bMode ){
		case 0:			// for RAM Before
			// RAM用前スクリプトファイルオープン用処理
			memcpy( filename, e_ExtfInfo[bUsdNum].byExtRamBfScript, MAX_PATH_SIZE );
			break;
		case 1:			// for RAM After
			// RAM用後スクリプトファイルオープン
			memcpy( filename, e_ExtfInfo[bUsdNum].byExtRamAfScript, MAX_PATH_SIZE );
			break;
		case 2:			// for ROM Before
			// ROM用前スクリプトファイルオープン
			memcpy( filename, e_ExtfInfo[bUsdNum].byExtRomBfScript, MAX_PATH_SIZE );
			break;
		case 3:			// for ROM After
			// ROM用後スクリプトファイルオープン
			memcpy( filename, e_ExtfInfo[bUsdNum].byExtRomAfScript, MAX_PATH_SIZE );
			break;
		default:
			break;
	}

	if( filename[0] == '\0' ){		// 実行スクリプト指定がない場合
		return ferr;
	}
	p = (char *)&filename[0];
	// 指定スクリプトファイルオープン
	// RevNo121017-003	Modify Start
	ernerr = fopen_s( &fp , p , "r");
	if( fp == NULL ) {
		return FFWERR_EXTROM_SCRIPT;			// 外部フラッシュROMスクリプト読み込みエラー
	}
	// RevNo121017-003	Modify End

	//データの取得
		for(;;){
GetLine:
		if( (fgets(data_buffer,MAX_SCRIPT_1LINE_DATA,fp)) == NULL ){		// 1行分のデータ取得
			// 全行取得済み
			break;
		}
		// RevNo121017-003	Modyfy Line
		ernerr = _strupr_s( data_buffer, _countof(data_buffer) );			// data_bufferの中身を全て大文字に変換
		// 先頭の空白行削除
		for( cnt = 0; data_buffer[cnt] == ' ' || data_buffer[cnt] == '\t'; cnt++ );
		if( data_buffer[cnt] == '!' ){					// コメント行
			goto GetLine;
		}
		if( data_buffer[cnt] == '\n' ){					// 改行
			goto GetLine;
		}
		// コマンド文字列解析 & 引数分割
		if( setArgument(&data_buffer[cnt]) != TRUE ){
			goto GetLine;								// コマンドがMF/MEMORY_FILL以外の場合は次の行取得へ
		}
		// コマンド発行( MF/MEMORY_FILLコマンドのみ )
		// Argv[0] : コマンド			Argc == 1
		// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append Line
		if( Argc > 3 ){		// MFの引数が必要分指定されている場合のみ以下の処理を実施(引数が足りないのにArgv[x]をリードするとクラッシュする)
			// Argv[1] : 開始アドレス		Argc == 2
			len = (BYTE)strlen( Argv[1] );
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Modify Start
			if( get_hex( Argv[1], &madrStartAddr, len ) != TRUE){	// 16進数以外のデータがあった場合は次の行へ
				goto GetLine;
			}				
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Modify End
			// Argv[2] : 終了アドレス		Argc == 3
			len = (BYTE)strlen( Argv[2] );
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Modify Start
			if( get_hex( Argv[2], &madrEndAddr, len ) != TRUE){		// 16進数以外のデータがあった場合は次の行へ
				goto GetLine;
			}				
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Modify End
			// Argv[3] : 書き込みデータ		Argc == 4
			len = (BYTE)strlen( Argv[3] );
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Modify Start
			if( get_hex( Argv[3], &dwdata, len ) != TRUE){		// 16進数以外のデータがあった場合は次の行へ
				goto GetLine;
			}				
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Modify End
			// Argv[4] : アクセスサイズ		Argc == 5
			if( Argc > 4 ){					// アクセスサイズ指定あり
				len = (BYTE)strlen( Argv[4] );
				if( len == 0x04 ){				// 4文字(アクセスサイズ指定あり)の場合
					if( strcmp( Argv[4], "BYTE" ) == 0 ){			// BYTE指定の場合
						eAccessSize = MBYTE_ACCESS;
						// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete( dwLength不要のため削除 )
					}else if( strcmp( Argv[4], "WORD" ) == 0 ){		// WORD指定の場合
						eAccessSize = MWORD_ACCESS;
						// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete( dwLength不要のため削除 )
					}else if( strcmp( Argv[4], "LONG" ) == 0 ){		// LONG指定の場合
						eAccessSize = MLWORD_ACCESS;
						// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete( dwLength不要のため削除 )
					}else{											// 上記以外の文字列
						goto GetLine;
					}
				// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete
				// アクセスサイズなしのベリファイ指定は受け付けないようにするためlen == 0x01の場合の処理削除
				}else{							// 4文字(アクセスサイズ指定あり)でない場合
					goto GetLine;
				}
				// Argv[5] : ベリファイ指定		Argc == 6
				if( Argc > 5 ){					// ベリファイ指定あり
					if( strcmp( Argv[5], "N" ) == 0 ){				// ベリファイなしの場合
						eVerify = VERIFY_OFF;
					}else if( strcmp( Argv[5], "V" ) == 0 ){		// ベリファイありの場合
						eVerify = VERIFY_ON;
					// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append Start
					}else{		// 指定文字が"N"でも"V"でもない場合は無効行とする
						goto GetLine;
					// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append End
					}
				}else{							// ベリファイ指定なし
					eVerify = VERIFY_OFF;
				}
			}else{							// アクセスサイズ指定、ベリファイ指定なし
				eAccessSize = MBYTE_ACCESS;
				// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete( dwLength不要のため削除 )
				eVerify = VERIFY_OFF;
			}
			
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete
			// 書き込み開始アドレスと終了アドレスが等しい場合の処理は不要のため削除

			// 書き込みデータ設定
			pbyWriteData[0] = 0;
			pbyWriteData[1] = 0;
			pbyWriteData[2] = 0;
			pbyWriteData[3] = 0;
			if(eAccessSize == 0) {				// BYTEサイズの場合
				dwWriteDataSize = 1;
				pbyWriteData[0] = (BYTE)dwdata;
			}else if(eAccessSize == 1) {		// WORDサイズの場合
				dwWriteDataSize = 2;
				pbyWriteData[0] = (BYTE)(dwdata>>8);
				pbyWriteData[1] = (BYTE)dwdata;
			}else if(eAccessSize == 2) {		// LONGサイズの場合
				dwWriteDataSize = 4;
				pbyWriteData[0] = (BYTE)(dwdata>>24);
				pbyWriteData[1] = (BYTE)(dwdata>>16);
				pbyWriteData[2] = (BYTE)(dwdata>>8);
				pbyWriteData[3] = (BYTE)dwdata;
			}

			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append Start
			if (madrStartAddr > madrEndAddr) {	// 開始アドレスが終了アドレスよりも大きい場合
				goto GetLine;
			}
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append End
			// コマンド発行
			// V.1.01 No.26 メモリアクセス端数データ処理統一 Modify Line
			ferr = DO_FILL(madrStartAddr, madrEndAddr, eVerify, eAccessSize, dwWriteDataSize, pbyWriteData, &VerifyErr);
			// ExtFlashModule_012 Append Start
			if (ferr != FFWERR_OK) {
				fclose(fp);		// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append Line
				return ferr;
			}
			if (VerifyErr.eErrorFlag == VERIFY_ERR) {	
				ferr = FFWERR_EXTROM_BUSSET_BAT_VRIFYERR;	// バッチファイル内でベリファイエラー発生
				fclose(fp);
				return ferr;
			}
			// ExtFlashModule_012 Append End
		// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append Start
		}else{		// MFの引数が必要分指定されていない場合
			goto GetLine;
		}
		// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append End
		// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete( この時点のgoto GetLineは不要のため削除 )
	}
	fclose(fp);

	return ferr;


}

//=============================================================================
/**
 * << スクリプトファイル コマンド文字列解析 & 引数分割関数 >>
 * スクリプトファイルに書かれているコマンド文字列解析とコマンド引数の分割を行う
 * @param  cmd_str  コマンド文字列
 * @retval TRUE/FALSE
 */
//=============================================================================
static int setArgument(char *cmd_str)
{
	char *arg_str;
	// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete( char *s;, int i; )

	Argc = 0;
	arg_str = cutCmd(cmd_str);
	// コマンドがMF/MEMORY_FILLかどうか比較
	if( strcmp( Argv[0], "MF" ) != 0 ){						// MFでない場合
		if( strcmp( Argv[0], "MEMORY_FILL" ) != 0 ){		// MEMORY_FILLでない場合
			return FALSE;						// 引数を解析する必要なし
		}
	}

	for (; *arg_str == ' ' || *arg_str == '\t'; arg_str++);	// 空白文字読み飛ばし

	if (*arg_str != '\0' && *arg_str != '\n') {				// 引数有り
		if (cutArg(arg_str) != TRUE) {		// 引数分割(この時点では空白文字付き)
			return FALSE;
		}
	}else{													// 引数なし
		return FALSE;
	}

	// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete Start
	// 以下の処理は、ここに至るまでの処理で対応できていて意味がないため削除する。
//	for (i = 0; i < Argc; i++) {
//		Argv[i] = CutSpace(Argv[i]);		// 各引数に付いている空白文字を削除
//	}

//	for (s = Argv[0]; *s != '\0'; s++) {
//		if (*s == ',') {
//			return FALSE;
//		}
//	}
//	if (Argv[Argc - 1] == NULL) {
//		return FALSE;
//	}
	// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete End
	return TRUE;
}

//=============================================================================
/**
 * << コマンド抽出関数 >>
 * @param  *s  コマンド文字列を格納した配列へのポインタ
 * @retval コマンド文字列格納配列のコマンドの先頭位置を返す
 */
//=============================================================================
static char *cutCmd(char *s)
{
	for (; *s == ' ' || *s == '\t'; s++)
		;
	Argv[Argc++] = s++;
	for (; *s != ' ' && *s != '\t' && *s != '\n' && *s != '\0'; s++)
		;
	if (*s != '\0') {
		*s++ = '\0';
	}
	return s;
}

//=============================================================================
/**
 * << コマンドの複数引数文字列を1つ1つ分割する関数 >>
 * @param  *s  コマンド文字列
 * @retval TRUE
 */
//=============================================================================
static int cutArg(char *s)
{
	Argv[Argc++] = s;
	while (*s != '\0' && *s != '\n') {
		if (*s == ' ' || *s == '\t') {
			*s++ = '\0';
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append & Mpdify Start
			// コマンド引数間に複数SP/TABがあっても正常に解析できるようにする
			for (; *s == ' ' || *s == '\t'; s++){
				;
			}
			if (*s == '\0' || *s == '\n') {		// EOFか改行コードの場合はここで引数取得処理終了
				break;
			}
			if( Argc < 6 ){		// 必要な引数分の取得ができていない場合は引数格納
								// Argc = コマンド格納後(1)、開始格納後(2)、終了格納後(3)、データ格納後(4)、アクセスサイズ格納後(5)、ベリファイ格納後(6)
				Argv[Argc++] = s;	// 引数の先頭位置へのポインタを入れた後、Argcをインクリメントする
			}else{
				break;			// 必要な引数分取得できたらここで解析終了
			}
			// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Append & Mpdify End
		} else {
			s++;
		}
	}
	*s = '\0';

	return TRUE;
}

// ExtFlashModule_018 外部フラッシュダウンロードバス設定スクリプト解析処理修正 Delete( *CutSpace()関数削除 ) 

//=============================================================================
/**
 * << ASCII文字の16進数変換関数 >>
 * @param  *p  ASCII文字列格納配列へのポインタ
 * @param  *value  16進数変換後の値格納変数へのポインタ
 * @param  cnt  16進数への変換文字数
 * @retval TRUE/FALSE
 */
//=============================================================================
static UCHAR get_hex(char *p, DWORD *value, short cnt)
{
	long	val;					/* 数値 */
	int		n;						// RevRxNo130730-011 Append Line

	for(val = 0L; cnt > 0; cnt--){
		val *= 0x10L;				/* 桁上げ */
		// RevRxNo130730-011 Modify Start
		n = (int)*p & 0x000000ff;
		if(isdigit(n)){		/* 0x0 ～ 0x9 */
			val += *p - '0';
		}else if(isxdigit(n)){	/* 0xA ～ 0xF */
			if(isupper(n))
		// RevRxNo130730-011 Modify End
				val += (*p - 'A') + 0xA;
			else
				return(FALSE);		/* 小文字の場合 */
		}else{
			return(FALSE);			/* 16進数でない場合 */
		}
		p++;						/* 次の文字へ */
	}
	*value = val;					/* 変換した数値を格納 */
	return(TRUE);					/* 正常終了 */
}

//==============================================================================
/**
 * << 外部フラッシュ領域への書き込み実行済み情報取得関数 >>
 * @param  なし
 * @retval s_byExtFlashWriteFlgの値
 */
//==============================================================================
BOOL GetExtFlashWriteFlg(void)
{
	return s_byExtFlashWriteFlg;
}

//==============================================================================
/**
 * 外部フラッシュ領域への書き込み後のベリファイチェック指定情報取得
 * @param  なし
 * @retval s_byExtFlashWriteFlgの値
 */
//==============================================================================
enum FFWENM_VERIFY_SET GetExtFlashVerify(void)
{
	return s_ExteVerify;
}

// ExtFlashModule_002b Append Start
//=============================================================================
/**
 * ロックされているセクタがあったかを格納する変数のクリア
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrExistLockSectFlg(void)
{
	s_bExistLockSect = FALSE;	// ロックされているセクタなし
}
// ExtFlashModule_002b Append End


// ExtFlashModule_002b Append Start
//=============================================================================
/**
 * ロックされているセクタがあったかを格納する変数の参照
 * @param なし
 * @retval なし
 */
//=============================================================================
BOOL GetExistLockSectFlg(void)
{
	return s_bExistLockSect;
}
// ExtFlashModule_002b Append End


//==============================================================================
/*	関数名	void	COM_WaitMs(DWORD ms)
*	引数	ms		ウエイトする時間
*	戻り値	なし
*	機能	指定時間ウエイトする
*			GetTickCount()はシステムを起動した後の経過時間を15～16ms単位で取得
*/
//==============================================================================
void	COM_WaitMs(DWORD ms)
{
	DWORD	s_time, time;

	s_time = GetTickCount();
//	while (1) {
	for(;;){
		time = GetTickCount();
		if (s_time > time) {
			// PCシステム起動から48日経過して、タイマカウンタが1周した
			if ((0 - s_time) + time > ms) {
				break;
			}
		} else {
			// タイマカウンタは1周していない
			if ((time - s_time) > ms) {
				break;
			}
		}
	}
}

//=============================================================================
/**
 * << 指定アドレス範囲の外部フラッシュROM領域判定関数 >>
 * 指定開始アドレスと領域属性(外部フラッシュROM領域/外部フラッシュROM領域以外)が
 * 同じ領域の終了アドレスとその領域属性を返送する。 　
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param pmadrAreaEnd 開始アドレスと同じ領域属性の領域の終了アドレスを格納するバッファへのポインタ
 * @param pbExtFlashRomArea 開始アドレスの領域属性を格納するバッファへのポインタ(TRUE:外部フラッシュROM領域/FALSE:外部フラッシュROM領域以外)
 * @retval なし
 */
//=============================================================================
void CheckExtFlashRomArea(MADDR madrStartAddr, MADDR madrEndAddr, MADDR* pmadrAreaEnd, BOOL* pbExtFlashRomArea)
{
	DWORD	dwCnt;
	BOOL	bAreaChk;
	DWORD	dwUsdFileNum;

	dwUsdFileNum = MAX_USD_FILE;

	bAreaChk = FALSE;
	for (dwCnt = 0; dwCnt < dwUsdFileNum; dwCnt++) {
		if (!e_ExtfInfo[dwCnt].wSectorNum){
			break;
		}
		if (madrEndAddr < e_ExtfInfo[dwCnt].dwStartAddr) {	// madrStartAddr <= madrEndAddr < e_ExtfInfo[dwCnt].dwStartAddr <= e_ExtfInfo[dwCnt].dwEndAddr
			*pmadrAreaEnd = madrEndAddr;
			*pbExtFlashRomArea = FALSE;	// 外部フラッシュROM領域でない
			bAreaChk = TRUE;	// 領域判定済み
			break;
		}
		if (madrStartAddr < e_ExtfInfo[dwCnt].dwStartAddr) {	// madrStartAddr < e_ExtfInfo[dwCnt].dwStartAddr <= madrEndAddr
			*pmadrAreaEnd = e_ExtfInfo[dwCnt].dwStartAddr - 1;
			*pbExtFlashRomArea = FALSE;	// 外部フラッシュROM領域でない
			bAreaChk = TRUE;	// 領域判定済み
			break;
		}
		if (madrStartAddr <= e_ExtfInfo[dwCnt].dwEndAddr) {
			if (madrEndAddr <= e_ExtfInfo[dwCnt].dwEndAddr) {	// e_ExtfInfo[dwCnt].dwStartAddr <= madrStartAddr <= madrEndAddr <= e_ExtfInfo[dwCnt].dwEndAddr
				*pmadrAreaEnd = madrEndAddr;
				*pbExtFlashRomArea = TRUE;	// 外部フラッシュROM領域である
				bAreaChk = TRUE;	// 領域判定済み
				break;
			} else {		// e_ExtfInfo[dwCnt].dwStartAddr <= madrStartAddr <= e_ExtfInfo[dwCnt].dwEndAddr < madrEndAddr
				*pmadrAreaEnd = e_ExtfInfo[dwCnt].dwEndAddr;
				*pbExtFlashRomArea = TRUE;	// 外部フラッシュROM領域である
				bAreaChk = TRUE;	// 領域判定済み
				break;
			}
		}
	}
	if (bAreaChk != TRUE) {	// 領域判定済みでない場合(e_ExtfInfo[dwCnt].dwStartAddr <= e_ExtfInfo[dwCnt].dwEndAddr < madrStartAddr <= madrEndAddr)
		*pmadrAreaEnd = madrEndAddr;
		*pbExtFlashRomArea = FALSE;	// 外部フラッシュROM領域でない
	}

	return;
}

//------------------------------------------------------------------------------------------------------

//=============================================================================
/**
 * 外部CS領域のエンディアン取得
 * @param dwCsNum			: 外部CS領域 0:CS0～7:CS7
 * @param pwEndianState	:ENDIAN_LITTLE/ENDIAN_BIG
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR GetMcuExtCsEndian(DWORD dwCsNum,WORD* pwEndianState)
{

	FFWERR	ferr;
	MADDR	madrCsiCtrl[INIT_EXTROM_NUM] = {MCU_REG_BSC_CS0CR,MCU_REG_BSC_CS1CR, MCU_REG_BSC_CS2CR, MCU_REG_BSC_CS3CR, MCU_REG_BSC_CS4CR, MCU_REG_BSC_CS5CR, MCU_REG_BSC_CS6CR, MCU_REG_BSC_CS7CR};
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	WORD						wReadData[1];		// 参照データ格納領域

	*pwEndianState = GetEndianState();		// リセット時CPUエンディアン状態を取得
	eAccessSize = MWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	ferr = GetMcuSfrReg(madrCsiCtrl[dwCsNum], eAccessSize,SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if( (wReadData[0] & MCU_REG_BSC_CSCR_EMODE) ==  MCU_REG_BSC_CSCR_EMODE){				// CSCNT.bit8が1の場合(CPUエンディアンと違う)
		if (*pwEndianState == FFWRX_ENDIAN_BIG){		// CPUがBIGエンディアンの場合
			(*pwEndianState) = FFWRX_ENDIAN_LITTLE;
		}else{					// CPUがLITTLEエンディアンの場合
			(*pwEndianState) = FFWRX_ENDIAN_BIG;
		}
	}
	return ferr;

}

//==============================================================================
/**
 * << 外部フラッシュ領域ダウンロード用変数初期化処理関数 >>
 * @param  なし
 * @retval TRUE
 */
//==============================================================================
void InitExtFlashWrite(void)
{
	int	i;
	// RevRxNo121122-005	Append Line
	errno_t ernerr;
	

	// -------------------------------------------------------------------------------------------------------
	// 外部フラッシュダウンロード関連構造体、変数初期化
	s_bExtFlashDataGetFlg = FALSE;								// 外部フラッシュデータ未取得
	s_byUsdNum = 0xFF;
	s_wSectNum = 0xFFFF;
	s_wLastSectNum = 0xFFFF;
	for( i = 0; i < MAX_USD_FILE; i++ ){
		s_bExtWriteFlashStartFlg[i] = FALSE;					// 外部フラッシュダウンロード準備未
		s_byDiffIDFlg[i] = FALSE;								// メーカID/デバイスID比較済みフラグ初期化
	}
	s_byExtFlashWriteFlg = TRUE;								// 外部フラッシュへの書き込み実行済み(いきなり書き込みに行かないように)				
	s_ExteVerify = VERIFY_OFF;									// ベリファイチェックなし
	s_bExtIntRamEnableFlg = FALSE;								// 内蔵RAM無効モードを有効にはしてないに設定
// V.1.02 覚え書き32 RAM停止時の処理対応 Append Line
	s_bExtRamStartFlg = FALSE;									// 内蔵RAM動作化にはしていないに設定
	s_bExtWorkRamResumeFlg = FALSE;								// ワークRAM復帰する必要なしに設定
	s_ExtFerr = FFWERR_OK;										// エラーなしに設定しておく

	// ExtFlashModule_008 Append Line
	s_bSendProtEfwriteStart = FALSE;							// PROT_MCU_EFWRITESTART()未送信
	// RevRxNo121122-005	Append Start
	s_ferdiData.dwErrorCode = FFWERR_OK;
	for (i = 0; i < EFLASHERR_MAX; i++) {
		ernerr = strcpy_s(s_ferdiData.szErrorMessage[i], ERROR_MESSAGE_LENGTH_MAX, "No Message");
	}

	// RevNo130118-001 Append Line
	s_bNewExtRomCache = FALSE;									// 外部フラッシュ1セクタ分のキャッシュ領域確保フラグ初期化

	// RevRxNo121122-005	Append End
	// -------------------------------------------------------------------------------------------------------

	return;
}

//=============================================================================
/**
 * << ターゲットMCU制御関数用変数初期化(外部フラッシュダウンロード用)関数 >>
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData_Extflash(void)
{
	int i;

	// -------------------------------------------------------------------------------------------------------
	// 外部フラッシュダウンロード関連構造体、変数初期化
	for( i = 0; i < MAX_USD_FILE; i++ ){
		e_ExtfInfo[i].wSectorNum = 0;							// 全セクタ数クリア(USDファイル登録なし)
	}	
	s_bExtFlashDataGetFlg = FALSE;								// 外部フラッシュデータ未取得
	s_byUsdNum = 0xFF;
	s_wSectNum = 0xFFFF;
	s_wLastSectNum = 0xFFFF;
	for( i = 0; i < MAX_USD_FILE; i++ ){
		s_bExtWriteFlashStartFlg[i] = FALSE;					// 外部フラッシュダウンロード準備未
		s_byDiffIDFlg[i] = FALSE;								// メーカID/デバイスID比較済みフラグ初期化
	}
	s_byExtFlashWriteFlg = TRUE;								// 外部フラッシュへの書き込み実行済み(いきなり書き込みに行かないように)				
	s_ExteVerify = VERIFY_OFF;									// ベリファイチェックなし
	s_bExtIntRamEnableFlg = FALSE;								// 内蔵RAM無効モードを有効にはしてないに設定
// V.1.02 覚え書き32 RAM停止時の処理対応 Append Line
	s_bExtRamStartFlg = FALSE;									// 内蔵RAM動作化にはしていないに設定
	s_bExtWorkRamResumeFlg = FALSE;								// ワークRAM復帰する必要なしに設定
	s_ExtFerr = FFWERR_OK;										// エラーなしに設定しておく

	// ExtFlashModule_008 Append Line
	s_bSendProtEfwriteStart = FALSE;							// PROT_MCU_EFWRITESTART()未送信

	// RevNo130118-001 Append Line
	s_bNewExtRomCache = FALSE;									// 外部フラッシュ1セクタ分のキャッシュ領域確保フラグ初期化
	// -------------------------------------------------------------------------------------------------------

	return;
}

// RevRxNo120910-007	Append Start
//==============================================================================
/**
 * フラッシュメモリのメーカIDとデバイスIDを取得する。
 * @param	BYTE	byUsdNum	USDファイル番号
 * @param	FFW_ERROR_DETAIL_INFO *pErrDetailInfo	エラー詳細情報テーブルポインタ
 * @relval	FFWエラーコード
 */
//==============================================================================
FFWERR getExtFlashID(BYTE	byUsdNum, FFW_ERROR_DETAIL_INFO* pErrDetailInfo)
{
	FFWERR	ferr;									// FFWエラーコード
	FFW_EXTF_IDSNUM	fexidData;						// メーカID デバイスID
	DWORD	dwDataSize = EXTF_DATA_WORD;			// データサイズ 初期値はワーニング対策（未初期化変数参照の可能性あり Warning C4701)
	DWORD	i;										// カウンタ
	WORD	wMakerID_read;							// メーカIDのリード値
	WORD	wMakerID_expected;						// メーカIDの期待値
	WORD	wDeviceID_read;							// デバイスIDのリード値
	WORD	wDeviceID_expected;						// デバイスIDの期待値
	WORD	wTemp;									// 計算用テンポラリ
	MADDR	madrTopSectStartAddr;					// フラッシュメモリの先頭アドレス
	MADDR	madrLastSectEndAddr;					// フラッシュメモリの最終アドレス
	int		ierr;									// sprintf_s 関数戻り値

	// 指定された開始アドレスと終了アドレスからのフラッシュメモリのメーカIDデバイスIDを取り込んで文字列でdataで指定された
	// テーブルにセットする。
	if( e_ExtfInfo[ byUsdNum ].byWriteProgramType == EXTF_WRITEPROG_CUI ) {
		// (1)	書き込み方式がCUIの場合
			//	A.	メーカIDとデバイスIDの期待値と実際の値を取得する。(getExtFlashID_cui(byUsdNum,pdata)
			ferr = getExtFlashID_cui( byUsdNum, &fexidData);
			if( ferr != FFWERR_OK) {
				//		(A)通信エラー等で取り込めない時 FFWERR_COM等、下位の処理で発生したエラーコード
				//			getExtFlashID_cui関数の戻り値を戻り値としてセットし処理を抜ける。
				return	ferr;
			}
	} else if ( e_ExtfInfo[ byUsdNum ].byWriteProgramType == EXTF_WRITEPROG_JEDEC ) {
		// (2)	書き込み方式がJDEC方式の場合
			//	A.	メーカIDとデバイスIDの期待値と実際の値を取得する。(getExtFlashID_jdec(byUsdNum,pdata)
			ferr = getExtFlashID_jdec( byUsdNum, &fexidData);
			if( ferr != FFWERR_OK) {
				//		(A)通信エラー等で取り込めない時 FFWERR_COM等、下位の処理で発生したエラーコード
				//			getExtFlashID_jdec関数の戻り値を戻り値としてセットし処理を抜ける。
				return	ferr;
			}
	}
	// (3)	IDのデータサイズを e_Exinfo->byConnectから取得する。
	if( e_ExtfInfo[ byUsdNum ].byConnect == EFLASHROM_MCU8_ROM8 ) {
		//	A.	e_ExtfInfo->byConnect = EFLASHROM_MCU8_ROM8ならば
		dwDataSize = EXTF_DATA_BYTE;
	} else if ((EFLASHROM_MCU16_ROM16 <= e_ExtfInfo[byUsdNum].byConnect) && (e_ExtfInfo[byUsdNum].byConnect <= EFLASHROM_MCU16_ROM8_2)) {
		//	B.	EFLASHROM_MCU16_ROM16 ≦ e_ExtfInfo->byConnect ≦ EFLASHROM_MCU16_ROM8_2ならば
		//		IDのデータサイズは2とする
		dwDataSize = EXTF_DATA_WORD;
	}
	// (4)	メーカID(wMakerID_read) =0とする。
	wMakerID_read = 0;
	for (i = 0; i < dwDataSize ; i++ ) {
	// (5)	i= 0～IDデータサイズまでA～Cを繰り返す
		//	A.	wMakerID_readを8bit左シフトする。
		wMakerID_read = 	wMakerID_read << 8;
		//	B.	wTemp =pdata->makeID_read[i++]	
		wTemp = static_cast<WORD>(fexidData.m_makerIDread[i]);
		//	C.	wMakerID_readとwTempとORをとってwMakerID_readに格納する。
		wMakerID_read |= (wTemp & 0x00FF);
	}
	// (6)	デバイスID(wDeviceID_read) =0とする。
	wDeviceID_read = 0;
	for (i = 0; i < dwDataSize ; i++ ) {
	// (7)	i= 0～IDデータサイズまでA～Cを繰り返す	
		wDeviceID_read = 	wDeviceID_read << 8;
		//	A.	wDveviceID_readを8bit左シフトする。
		wTemp = static_cast<WORD>(fexidData.m_deviceIDread[i]);
		//	B.	wTemp =pdata->deviceID_read[i++]
		wDeviceID_read |= (wTemp & 0x00FF);
		//	C.	wDeviceID_readとwTempとORをとってwDeviceID_readに格納する。
	}
	// (8)	メーカID(wMakerID_expected) =0とする。
	wMakerID_expected = 0;
	// RevRxNo121115-002	Modify Line
	for (i = 0; i < EXTF_DATA_WORD ; i++ ) {
	// (9)	i= 0～ワードサイズまでA～Cを繰り返す
		//	A.	wMakerID_expectedを8bit左シフトする。
		wMakerID_expected = 	wMakerID_expected << 8;
		//	B.	wTemp =pdata->makeID_expected[i++]	
		wTemp = static_cast<WORD>(fexidData.m_makerIDexpected[i]);
		//	C.	wMakerID_expectedとwTempとORをとってwMakerID_expectedに格納する。
		wMakerID_expected |= (wTemp & 0x00FF);
	}
	// (10)	デバイスID(wDeviceID_expected) =0とする。
	wDeviceID_expected = 0;
	// RevRxNo121115-002	Modify Line
	for (i = 0; i < EXTF_DATA_WORD ; i++ ) {
	// (11)	i= 0～ワードサイズまでA～Cを繰り返す	
		wDeviceID_expected = 	wDeviceID_expected << 8;
		//	A.	wDveviceID_expectedを8bit左シフトする。
		wTemp = static_cast<WORD>(fexidData.m_deviceIDexpected[i]);
		//	B.	wTemp =pdata->deviceID_expected[i++]
		wDeviceID_expected |= (wTemp & 0x00FF);
		//	C.	wDeviceID_expectedとwTempとORをとってwDeviceID_expectedに格納する。
	}
	// (12)	フラッシュメモリの先頭アドレスを求める。(外部ROM開始アドレス)
	madrTopSectStartAddr = static_cast<MADDR>(e_ExtfInfo[byUsdNum].dwStartAddr);
	// (13)	フラッシュメモリの最終アドレスを求める。(外部ROM終了アドレス)
	madrLastSectEndAddr = static_cast<MADDR>(e_ExtfInfo[byUsdNum].dwEndAddr);

	// エラー詳細情報のテーブルに情報を登録する。
	
	// RevRxNo121115-003	Modify Start
	// (14)	フラッシュメモリの先頭アドレスをエラー詳細情報テーブルに登録する。
	ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_TS_STARTADDR], ERROR_MESSAGE_LENGTH_MAX,"0x%08x", madrTopSectStartAddr);
	// (15)	フラッシュメモリの最終アドレスをエラー詳細情報テーブルに登録する。
	ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_LS_ENDADDR], ERROR_MESSAGE_LENGTH_MAX,"0x%08x", madrLastSectEndAddr);
	// RevRxNo121115-003	Modify End

	if( dwDataSize == EXTF_DATA_WORD ) {
		// RevRxNo121115-003	Modify Start
		//(16)IDデータサイズが2ならば以下処理を実行	
		//	A.	メーカID(wMakerID_read)を文字列変換する。(IDのサイズが2バイトなので書式は0x%04x)にする
		//	B.	文字列に変換したデータを引数でしたpErrDetailInfoのszErrorMessageメンバのポインタpErrDetailInfo->szErrorMessage[1]のさす領域に
		//	コピーする。
		//	※A.Bは説明の為、2行に分けているが一行で実施(sprintf_s関数)
		ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_MAKERID_READ], ERROR_MESSAGE_LENGTH_MAX,"0x%04x", wMakerID_read );
		//	C.	メーカID(wMakerID_expected)を文字列変換する。(IDのサイズが2バイトなので書式は0x%04x)にする
		//	D.	文字列に変換したデータを引数でしたpErrDetailInfoのszErrorMessageメンバのポインタpErrDetailInfo->szErrorMessage[2]のさす領域に
		//	コピーする。
		//	※C,Dは説明の為、2行に分けているが一行で実施(sprintf_s関数)
		ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_MAKERID_EXP], ERROR_MESSAGE_LENGTH_MAX,"0x%04x", wMakerID_expected );
		//	E	デバイスID(wDeviceID_read)を文字列変換する。(IDのサイズが2バイトなので書式は0x%04x)にする
		//	F	文字列に変換したデータを引数でしたpErrDetailInfoのszErrorMessageメンバのポインタpErrDetailInfo->szErrorMessage[3]のさす領域に
		//	コピーする。
		//	※E,Fは説明の為、2行に分けているが一行で実施(sprintf_s関数)
		ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_DEVICEID_READ], ERROR_MESSAGE_LENGTH_MAX,"0x%04x", wDeviceID_read );
		//	G	デバイスID(wDeviceID_expected)を文字列変換する。(IDのサイズが2バイトなので書式は0x%04x)にする
		//	H.	文字列に変換したデータを引数でしたpErrDetailInfoのszErrorMessageメンバのポインタpErrDetailInfo->szErrorMessage[4]のさす領域に
		//	コピーする。
		//	※G,Hは説明の為、2行に分けているが一行で実施(sprintf_s関数)
		ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_DEVICEID_EXP], ERROR_MESSAGE_LENGTH_MAX,"0x%04x", wDeviceID_expected );
		// RevRxNo121115-003	Modify End
		
	} else if( dwDataSize == EXTF_DATA_BYTE ) {
		// RevRxNo121115-003	Modify Start
		//(17)	IDデータサイズが1ならば以下処理を実行
		//	A.	メーカID(wMakerID_read)を文字列変換する。(IDのサイズが1バイトなので書式は0x%02x)にする
		//	B.	文字列に変換したデータを引数でしたpErrDetailInfoのszErrorMessageメンバのポインタpErrDetailInfo->szErrorMessage[1]のさす領域に
		//	コピーする。
		//	※A.Bは説明の為、2行に分けているが一行で実施(sprintf_s関数)
		ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_MAKERID_READ], ERROR_MESSAGE_LENGTH_MAX,"0x%02x", wMakerID_read );
		//	C.	メーカID(wMakerID_expected)を文字列変換する。(IDのサイズが1バイトなので書式は0x%02x)にする
		//	D.	文字列に変換したデータを引数でしたpErrDetailInfoのszErrorMessageメンバのポインタpErrDetailInfo->szErrorMessage[2]のさす領域に
		//	コピーする。
		//	※C,Dは説明の為、2行に分けているが一行で実施(sprintf_s関数)
		// RevRxNo121115-002	Modify Line
		ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_MAKERID_EXP], ERROR_MESSAGE_LENGTH_MAX,"0x%02x", ( 0x00ff & wMakerID_expected) );
		//	E	デバイスID(wDeviceID_read)を文字列変換する。(IDのサイズが1バイトなので書式は0x%02x)にする
		//	F	文字列に変換したデータを引数でしたpErrDetailInfoのszErrorMessageメンバのポインタpErrDetailInfo->szErrorMessage[3]のさす領域に
		//	コピーする。
		//	※E,Fは説明の為、2行に分けているが一行で実施(sprintf_s関数)
		ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_DEVICEID_READ], ERROR_MESSAGE_LENGTH_MAX,"0x%02x", wDeviceID_read );
		//	G	デバイスID(wDeviceID_expected)を文字列変換する。(IDのサイズが1バイトなので書式は0x%02x)にする。
		//	H.	文字列に変換したデータを引数でしたpErrDetailInfoのszErrorMessageメンバのポインタpErrDetailInfo->szErrorMessage[4]のさす領域に
		//	コピーする。
		//	※G,Hは説明の為、2行に分けているが一行で実施(sprintf_s関数)
		// RevRxNo121115-002	Modify Line
		ierr = sprintf_s(pErrDetailInfo->szErrorMessage[EFLASHERR_DEVICEID_EXP], ERROR_MESSAGE_LENGTH_MAX,"0x%02x", ( 0x00ff & wDeviceID_expected) );
		// RevRxNo121115-003	Modify End
	}
	// (18)	FFWERR_OKを戻り値としてセットし処理を抜ける。
	return	FFWERR_OK;
}
// RevRxNo120910-007	Append End
// RevRxNo120910-007	Append Start
//==============================================================================
/**
 * フラッシュメモリのメーカIDとデバイスIDを取得する(cui方式)
 * @param	BYTE			byUsdNum	USDファイル番号
 * @param	FFW_EXTF_IDSNUM *pdata		フラッシュメモリ メーカID デバイスID出力先構造体ポインタ
 * @relval	FFWエラーコード
 */
//==============================================================================
FFWERR getExtFlashID_cui(BYTE	byUsdNum, FFW_EXTF_IDSNUM* pdata)
{
	FFWERR	ferr;
	DWORD	dwCDataSize[EFLASHROM_MCU8_ROM8+1] = { sizeof(DWORD), sizeof(DWORD), sizeof(DWORD), sizeof(WORD), sizeof(WORD), sizeof(BYTE)};
																					//	データサイズ変換テーブル
	enum FFWENM_MACCESS_SIZE	eAccessSize[EFLASHROM_MCU8_ROM8+1] = { MLWORD_ACCESS, MLWORD_ACCESS, MLWORD_ACCESS, MWORD_ACCESS
																		, MWORD_ACCESS, MBYTE_ACCESS};
																					//	アクセスサイズ変換テーブル
	DWORD	dwCDeviceCount[EFLASHROM_MCU8_ROM8+1] = {	1,	2,	4,	1,	2,	1};		//	デバイス数変換テーブル
	MADDR	madrDIDOffset[EFLASHROM_MCU8_ROM8+1] = {	4,	4,	4,	2,	4,	2};		//	オフセットアドレス変換テーブル

	FFW_VERIFYERR_DATA			VerifyErr;		//	DO_FILL用ベリファイ結果格納構造体(VERIFY_OFFのため未使用)
	enum FFWENM_MACCESS_SIZE	eAccess;		//	アクセスサイズ
	DWORD	dwDataSize;							//	データサイズ
	MADDR	madrSectAddr;						// 	セクタアドレス
	MADDR	madrDeviceAddr;						// 	デバイスIDリードアドレス
	DWORD	dwDeviceCount;						//	デバイス数
	MADDR	madrOffset;							//	デバイスID取得用オフセットアドレス
	MADDR	madrEndAddr;						//	メーカID取得用終了アドレス
	BYTE	byCuiCom[2];						//	CUI方式コマンドテーブル
	BYTE	byCuiComEnd[2];						//	CUI方式エンドコマンドテーブル
	BYTE	byReadData[2];						//	CUI方式コマンドリードデータ
	WORD	wTemp;								//	計算用テンポラリ
	BYTE	byConnect;							//	接続形態情報()
	// RevRxNo130730-010 Modify Line( 変数iを削除 )
	DWORD	j;									//	カウンタ

	// 外部フラッシュメモリ情報から接続形態情報をロード
	byConnect = e_ExtfInfo[byUsdNum].byConnect;
	// (1)	フラッシュメモリ情報テーブルの接続情報(e_Exinfo->byConnect)を確認する。
	if( byConnect < EFLASHROM_MCU16_ROM16 ) {
		//	A.	 e_Exinfo->byConnect < 3 (EFLASHROM_MCU32_ROM32～EFLASHROM_MCU32_ROM8_4)の場合	
		//		戻り値をFFWERR_OKにセットして処理を抜ける。
		return	FFWERR_OK;
	}
	//	(2) 	接続情報から、アクセスデータサイズ(enum),データサイズ,フラッシュ先頭セクタアドレスを求める
	eAccess = eAccessSize[byConnect];						//	アクセスサイズ		 eAccess		eAccessSize[e_Exitnfo->byConnect]
	dwDataSize = dwCDataSize[byConnect];					//	データサイズ		 dwDataSize		dwCDataSize[e_Exinfo-.byConnect]
	madrSectAddr = static_cast<MADDR>(e_ExtfInfo[byUsdNum].dwSectAddr[0]);	//	セクタ先頭アドレス	 madrSectAddr	(MADDR) e_ExtfInfo->dwSectAddr[0]
	dwDeviceCount = dwCDeviceCount[byConnect]; 				//	デバイス数			 dwDeviceCount	dwCDeviceCount[e_Exinfo->byConnect]
	madrOffset = madrDIDOffset[byConnect]; 					//	デバイスIDオフセット madrOffset		madrDISOffset[e_Exinfo->byConnect]
	//(3)	フラッシュメモリ情報テーブルの接続情報(e_Exinfo->byConnect)を確認する。
	if( byConnect == EFLASHROM_MCU16_ROM16 ){
		//	A.	接続情報(e_ExtfInfo[byUsdNum].byConnect==EFLASHROM_MCU16_ROM16)の場合
		byCuiCom[0] = 		0x00;			//	byCuiCom[0]			= 0x00
		byCuiCom[1] = 		0x90;			//	byCuiCom[1]			= 0x90
		byCuiComEnd[0] = 	0x00;			//	byCuicuiComEnd[0]	= 0x00
		byCuiComEnd[1] = 	0xFF;			//	byCuiComEnd[1]		= 0xFF
	} else if ( byConnect == EFLASHROM_MCU16_ROM8_2 ){
		//	B.	接続情報(e_ExtfInfo->byConnect==EFLASHROM_MCU16_ROM8_2)の場合
		byCuiCom[0]= 		0x90;			//	byCuiCom[0]			= 0x90
		byCuiCom[1]= 		0x90;			//	byCuiCom[1]			= 0x90
		byCuiComEnd[0]= 	0xFF;			//	byCuiComEnd[0]		= 0xFF
		byCuiComEnd[1]= 	0xFF;			//	byCuiComEnd[1]		= 0xFF
	} else if ( byConnect == EFLASHROM_MCU8_ROM8 ){
		//	C.	接続情報(e_ExtfInfo->byConnect==EFLASHROM_MCU8_ROM8)の場合
		byCuiCom[0]= 0x90;					//	byCuiCom[0]			= 0x90
		byCuiComEnd[0]= 0xFF;				//	byCuiComEnd[0]		= 0xFF
	}
	//	(4)	madrEndAddr = madrSectAddr+dwDataSize-1
	madrEndAddr = madrSectAddr+dwDataSize-1;
	// RevRxNo130730-010 Modify Line( for文削除、コメント(5)の説明修正 )
	//	(5)	メーカーIDを読み出す
	//	A.	DO_FILLでコマンド設定する。
	ferr = DO_FILL(madrSectAddr, madrEndAddr, VERIFY_OFF, eAccess, dwDataSize, byCuiCom, &VerifyErr);
	//		DO_FILL(madrSectAddr ,.madrEndAddr,VERIFY_OFF, eAccess, dwDataSize byCuiCom );
	if (ferr != FFWERR_OK) {
		//	(A)	DO_FILLの戻り値がFFWERR_EOK以外
		return ferr;					//	DO_FILLの戻り値を戻り値を戻り値にセットして処理を抜ける
	}
	//	B.	DO_DUMP(madrSectAddr,madrSectAddr+dwDataSize-1,eAccess, ＆(pdata->m_makerIDread「i]) );
	ferr = DO_DUMP(madrSectAddr,(madrSectAddr+dwDataSize-1), eAccess, byReadData );	// DUMP処理実行
	if (ferr != FFWERR_OK) {
		//	(A)	DO_DUMPの戻り値がFFWERR_EOK以外
		return ferr;					//	DO_DUMPの戻り値を戻り値を戻り値にセットして処理を抜ける
	}
	//	C. 読み込んだデータを出力バッファにコピー
	// RevRxNo130730-010 Modify Line( j = i; → j = 0; に変更 )
	for (j = 0; j < dwDataSize ; j++) {
		pdata->m_makerIDread[j] = byReadData[j];
	}
	//	D.	DO_FILLでコマンド終了設定する。
	ferr = DO_FILL(madrSectAddr, madrEndAddr, VERIFY_OFF, eAccess, dwDataSize, byCuiComEnd, &VerifyErr);
	//		DO_FILL(madrSectAddr ,madrEndAddr,VERIFY_OFF, eAccess, dwDataSize cuiComEnd );
	if (ferr != FFWERR_OK) {
		//	(A)	DO_FILLの戻り値がFFWERR_EOK以外
		return ferr;					//	DO_FILLの戻り値を戻り値を戻り値にセットして処理を抜ける
	}
	// RevRxNo130730-010 Delete Line( for文の括弧を削除 )

	//	(6)	madrDeviceAddr = madrSectAddr+madrOffset(デバイスIDを読み出すアドレスをセットする。)
	madrDeviceAddr = madrSectAddr + madrOffset;	

	// RevRxNo130730-010 Modify Line( for文削除、コメント(7)の説明修正 )
	//	(7)	デバイスIDを読み出す
	//	A.	DO_FILLでコマンド設定する。
	ferr = DO_FILL(madrSectAddr, madrEndAddr, VERIFY_OFF, eAccess, dwDataSize, byCuiCom, &VerifyErr);
	//		DO_FILL(madrSectAddr ,.madrEndAddr,VERIFY_OFF, eAccess, dwDataSize byCuiCom );
	if (ferr != FFWERR_OK) {
		//	(A)	DO_FILLの戻り値がFFWERR_EOK以外
		return ferr;					//	DO_FILLの戻り値を戻り値を戻り値にセットして処理を抜ける
	}
	//	B.	DO_DUMP(madrDeviceAddr ,(madrDeviceAddr+dwDataSize-1),eAccess, byReadData)
	ferr = DO_DUMP(madrDeviceAddr,(madrDeviceAddr+dwDataSize-1), eAccess, byReadData);	// DUMP処理実行
	if (ferr != FFWERR_OK) {
		//	(A)	DO_DUMPの戻り値がFFWERR_EOK以外
		return ferr;					//	DO_DUMPの戻り値を戻り値を戻り値にセットして処理を抜ける
	}
	//	C.読み込んだデータを出力バッファにコピーする。
	// RevRxNo130730-010 Modify Line( j = i; → j = 0; に変更 )
	for (j = 0; j < dwDataSize ; j++) {
		pdata->m_deviceIDread[j] = byReadData[j];
	}
	//	D.	DO_FILLでコマンド終了設定する。
	ferr = DO_FILL(madrSectAddr, madrEndAddr, VERIFY_OFF, eAccess, dwDataSize, byCuiComEnd, &VerifyErr);
	//		DO_FILL(madrSectAddr ,madrEndAddr,VERIFY_OFF, eAccess, dwDataSize cuiComEnd );
	if (ferr != FFWERR_OK) {
		//	(A)	DO_FILLの戻り値がFFWERR_EOK以外
		return ferr;					//	DO_FILLの戻り値を戻り値を戻り値にセットして処理を抜ける
	}
	// RevRxNo130730-010 Delete Line( for文の括弧を削除 )
	
	//(8) 	フラッシュメモリ情報テーブルのメーカID情報(期待値)e_ExtfInfo->wMakerIDをローカル変数 wTempにロード
	wTemp = e_ExtfInfo[ byUsdNum ].wMakerID;
	//(9) 	 pdata->makerID_expected[1] =wTemp & 0x00ff
	pdata->m_makerIDexpected[1] =static_cast<BYTE>(wTemp & 0x00ff);
	//(10) 	 pdata->makerID_expected[0] =(wTemp>>8) & 0x00ff
	pdata->m_makerIDexpected[0] =static_cast<BYTE>((wTemp>>8) & 0x00ff);
	//(11)	フラッシュメモリ情報テーブルのデバイスID情報(期待値)e_ExtfInfo->wDeviceIDをローカル変数 wTempにロード
	wTemp = e_ExtfInfo[ byUsdNum ].wDeviceID;
	//(12)	 pdata->deviceID_expected[1] =wTemp & 0x00ff
	pdata->m_deviceIDexpected[1] =static_cast<BYTE>(wTemp & 0x00ff);
	//(13)	 pdata->deviceID_expected[0] =(wTemp>>8) & 0x00ff
	pdata->m_deviceIDexpected[0] =static_cast<BYTE>((wTemp>>8) & 0x00ff);
	//(14)	戻り値FFWERR_OKにして処理を抜ける
	return	FFWERR_OK;
}
// RevRxNo120910-007	Append End
// RevRxNo120910-007	Append Start
//==============================================================================
/**
 * フラッシュメモリのメーカIDとデバイスIDを取得する(jdec方式)
 * @param	BYTE	byUsdNum	USDファイル番号
 * @param	FFW_EXTF_IDSNUM *pdata		フラッシュメモリ メーカID デバイスID出力先構造体ポインタ
 * @relval	FFWエラーコード
 */
//==============================================================================
FFWERR getExtFlashID_jdec(BYTE	byUsdNum, FFW_EXTF_IDSNUM* pdata)
{
	FFWERR	ferr;
	DWORD	dwCDataSize[EFLASHROM_MCU8_ROM8+1] = { sizeof(DWORD), sizeof(DWORD), sizeof(DWORD), sizeof(WORD), sizeof(WORD), sizeof(BYTE)};
																					//	データサイズ変換テーブル
	enum FFWENM_MACCESS_SIZE	eAccessSize[EFLASHROM_MCU8_ROM8+1] = { MLWORD_ACCESS, MLWORD_ACCESS, MLWORD_ACCESS, MWORD_ACCESS
																		, MWORD_ACCESS, MBYTE_ACCESS};
	// RevRxNo121213-001	Append Line
	DWORD	dwCDeviceCount[EFLASHROM_MCU8_ROM8+1] = {	1,	2,	4,	1,	2,	1};		//	デバイス数変換テーブル
																	//	アクセスサイズ変換テーブル
	FFW_VERIFYERR_DATA			VerifyErr;							//	DO_FILL用ベリファイ結果格納構造体(VERIFY_OFFのため未使用)
	FFW_JDEC_COM			fjcJdecCom[3];							//	JDECコマンドテーブル
	enum FFWENM_MACCESS_SIZE	eAccess;							//	アクセスサイズ
	DWORD	dwDataSize;												//	データサイズ
	MADDR	madrCommandAddr;										//	コマンドアドレス
	MADDR	madrSectAddr;											// 	セクタアドレス
	MADDR	madrEndAddr;											//	メーカID取得用終了アドレス
	WORD	wTemp;													//	計算用テンポラリ
	WORD	wEraseStatus;											//	イレースステータス
	BYTE	byConnect;												//	接続形態情報()
	// RevRxNo121213-001	Append Line
	DWORD	dwDeviceCount;											//	デバイス数
	DWORD	i;														//	コマンド送信カウンタ


	// 外部フラッシュメモリ情報から接続形態情報をロード
	byConnect = e_ExtfInfo[byUsdNum].byConnect;
	// (1)	フラッシュメモリ情報テーブルの接続情報(e_Exinfo->byConnect)を確認する。
	if( byConnect < EFLASHROM_MCU16_ROM16) {
		//	A.	 e_Exinfo->byConnect < 3 (EFLASHROM_MCU32_ROM32～EFLASHROM_MCU32_ROM8_4)の場合	
		//		戻り値をFFWERR_OKにセットして処理を抜ける。
		return	FFWERR_OK;
	}
	// (2) 	接続情報から、アクセスデータサイズ(enum),データサイズ,フラッシュ先頭セクタアドレスを求める
	eAccess = eAccessSize[ byConnect ];					//	アクセスサイズ		 eAccess			eAccessSize[e_Exitnfo->byConnect]
	dwDataSize = dwCDataSize[ byConnect ];				//	データサイズ		 dwDataSize		dwCDataSize[e_Exinfo-.byConnect]
	madrSectAddr = static_cast<MADDR>(e_ExtfInfo[byUsdNum].dwSectAddr[0]);		//	セクタ先頭アドレス	 madrSectAddr	(MADDR) e_ExtfInfo->dwSectAddr[0]
	// RevRxNo121213-001	Append Line
	dwDeviceCount = dwCDeviceCount[byConnect]; 									//	デバイス数			 dwDeviceCount	dwCDeviceCount[e_Exinfo->byConnect]
	// (3)	フラッシュメモリ情報テーブルの接続情報(e_Exinfo->byConnect)を確認する。
	if( byConnect == EFLASHROM_MCU16_ROM16){
		//	A.	接続情報(e_ExtfInfo[byUsdNum].byConnect==EFLASHROM_MCU16_ROM16)の場合
		fjcJdecCom[0].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr16_1 * dwDataSize;
		fjcJdecCom[0].m_data[0]		=	0x00;
		fjcJdecCom[0].m_data[1]		=	0xAA;
		fjcJdecCom[1].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr16_2 * dwDataSize;
		fjcJdecCom[1].m_data[0]		=	0x00;
		fjcJdecCom[1].m_data[1]		=	0x55;
		fjcJdecCom[2].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr16_1 * dwDataSize;
		fjcJdecCom[2].m_data[0]		=	0x00;
		fjcJdecCom[2].m_data[1]		=	0x90;
	} else if( byConnect == EFLASHROM_MCU16_ROM8_2 ){
		//	B.	接続情報(e_ExtfInfo->byConnect==EFLASHROM_MCU16_ROM8_2)の場合
		fjcJdecCom[0].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr8_1 * dwDataSize;
		fjcJdecCom[0].m_data[0]		=	0xAA;
		fjcJdecCom[0].m_data[1]		=	0xAA;
		fjcJdecCom[1].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr8_2 * dwDataSize;
		fjcJdecCom[1].m_data[0]		=	0x55;
		fjcJdecCom[1].m_data[1]		=	0x55;
		fjcJdecCom[2].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr8_1 * dwDataSize;
		fjcJdecCom[2].m_data[0]		=	0x90;
		fjcJdecCom[2].m_data[1]		=	0x90;
	} else if( byConnect == EFLASHROM_MCU8_ROM8 ){
		//	C.	接続情報(e_ExtfInfo->byConnect==EFLASHROM_MCU8_ROM8)の場合
		fjcJdecCom[0].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr8_1 * dwDataSize;
		fjcJdecCom[0].m_data[0]		=	0xAA;
		fjcJdecCom[1].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr8_2 * dwDataSize;
		fjcJdecCom[1].m_data[0]		=	0x55;
		fjcJdecCom[2].m_offsetAddr	=	e_ExtfInfo[byUsdNum].wCommandAddr8_1 * dwDataSize;
		fjcJdecCom[2].m_data[0]		=	0x90;
	}

	// (4)	リセットコマンド(シーケンスクリア)
	ferr = PROT_MCU_EFWRITE(EFLASHROM_EXIT, 0, 0, 0, madrSectAddr, NULL, &wEraseStatus);
										// 0, 0, 0 : セクタイレーズタイムアウト値, メーカIDD, デバイスID(未使用のため0としている)
										// NULL : ダウンロードデータ格納領域へのポインタ(未使用のためNULLとしている)
	if (ferr != FFWERR_OK) {
		// リセット時にエラー発生したら	発生したエラーコードを戻り値にセットして抜ける。
		return ferr;
	}

	//	(5）iを0から2まで変更して、以下を繰り返し実行する。（JDECコマンドシーケンス送信）
	for( i = 0 ;i < 3 ; i++ ) {
		//	(a)	madrCommandAddr　= madrSectAddr | jdecCom[i].offset_addr
		madrCommandAddr	=	madrSectAddr | fjcJdecCom[i].m_offsetAddr;
		//	(b)	madrEndAddr　= madrCommandAddr+dwDataSize-1
		madrEndAddr		=	madrCommandAddr + dwDataSize - 1;
		//	(c)	DO_FILLでコマンド設定する
		ferr = DO_FILL(madrCommandAddr, madrEndAddr, VERIFY_OFF, eAccess, dwDataSize, fjcJdecCom[i].m_data, &VerifyErr);
		//		DO_FILL(madrCommandAddr ,madrEndAddr,VERIFY_OFF, eAccess, dwDataSize fjcJdecCom[i].m_data, &VerifyErr );
		if (ferr != FFWERR_OK) {
			//	(A)	DO_FILLの戻り値がFFWERR_EOK以外
			return ferr;					//	DO_FILLの戻り値を戻り値を戻り値にセットして処理を抜ける
		}
	}
	//	(6)	メーカーIDを読み出す。(セクタ先頭アドレスのデータを読み込む）
	ferr = DO_DUMP(madrSectAddr,madrSectAddr+dwDataSize - 1, eAccess, &(pdata->m_makerIDread[0]) );	// DUMP処理実行
	if (ferr != FFWERR_OK) {
			//	(A)	DO_DUMPの戻り値がFFWERR_EOK以外
		return ferr;					//	DO_DUMPの戻り値を戻り値を戻り値にセットして処理を抜ける
	}
	//	(7)	デバイスIDを読み出す。(セクタ先頭アドレスのデータを読み込む）
	// RevRxNo121213-001	Append Line
	ferr = DO_DUMP(madrSectAddr + (2 * dwDeviceCount) ,madrSectAddr + (2 * dwDeviceCount) + dwDataSize - 1, eAccess, &(pdata->m_deviceIDread[0]) );	// DUMP処理実行
	if (ferr != FFWERR_OK) {
			//	(A)	DO_DUMPの戻り値がFFWERR_EOK以外
		return ferr;					//	DO_DUMPの戻り値を戻り値を戻り値にセットして処理を抜ける
	}

	//	(8) フラッシュメモリ情報テーブルのメーカID情報(期待値)e_ExtfInfo->wMakerIDをローカル変数 wTempにロード
	wTemp = e_ExtfInfo[ byUsdNum ].wMakerID;
	//	(9)  pdata->makerID_expected[1] =wTemp & 0x00ff
	pdata->m_makerIDexpected[1] =static_cast<BYTE>(wTemp & 0x00ff);
	//	(10) pdata->makerID_expected[0] =(wTemp>>8) & 0x00ff
	pdata->m_makerIDexpected[0] =static_cast<BYTE>((wTemp>>8) & 0x00ff);
	//	(11)	フラッシュメモリ情報テーブルのデバイスID情報(期待値)e_ExtfInfo->wDeviceIDをローカル変数 wTempにロード
	wTemp = e_ExtfInfo[ byUsdNum ].wDeviceID;
	//	(12)	 pdata->deviceID_expected[1] =wTemp & 0x00ff
	pdata->m_deviceIDexpected[1] =static_cast<BYTE>(wTemp & 0x00ff);
	//	(13)	 pdata->deviceID_expected[0] =(wTemp>>8) & 0x00ff
	pdata->m_deviceIDexpected[0] =static_cast<BYTE>((wTemp>>8) & 0x00ff);
	//	(14)	戻り値FFWERR_OKにして処理を抜ける
	return	FFWERR_OK;
}
// RevRxNo120910-007	Append End

// RevRxNo121122-005	Append Start
//==============================================================================
/**
 * デバイスID メーカID不一致エラーの詳細情報を取得
 * @param	
 * @relval	デバイスID メーカID不一致エラーの詳細情報テーブルのポインタ
 */
//==============================================================================
FFW_ERROR_DETAIL_INFO* GetExtFlashErrorDetailInfo(void)
{
	return	&s_ferdiData;
}
// RevRxNo121122-005	Append End
