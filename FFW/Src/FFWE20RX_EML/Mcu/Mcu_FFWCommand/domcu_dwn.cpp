///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_dwn.cpp
 * @brief ダウンロード関連コマンドの実装ファイル
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, H.Akashi, Y.Miyake, S.Ueda, Y.Kawakami, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2015) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2015/08/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120615-001	2012/07/03 橋口
  ライタモード時のHOCO切り替え処理改善
・RevNo121017-007	2012/10/18 明石
  strWtrNotLoadMsgバッファを領域が小さすぎるので領域を1バイト拡大した。
・RevNo121017-003	2012/10/30 明石
　VS2008 warning C4996対策
・RevRxNo120910-006	2012/11/21 三宅(2012/12/04 SDS 岩田 マージ)
  Start/Stop機能の高速化。
  ・DO_DWNP_CLOSE()で、指定ルーチン用ワーク・プログラムのライト処理追加。
  ・ffwmcu_srm.h のインクルード追加。
・RevRxNo121221-001	2012/12/21 上田
　　WTRダウンロード処理改善
・RevRxNo140515-011 2014/07/14 川上
　　RX64M IDコード ALL F以外のユーザブート起動対応
・RevRxNo140617-001	2014/07/14 大喜多
	TrustedMemory機能対応
・RevRxE2LNo141104-001 2015/03/6 上田, 大喜多
	E2 Lite対応
・RevRxNo150827-002 2015/08/28 PA 紡車
	RX651オプション設定メモリアドレス、配置変更対応
*/

#include "domcu_dwn.h"
#include "ffwmcu_dwn.h"
#include "ffw_sys.h"
#include "ffwmcu_prog.h"
#include "domcu_mcu.h"
#include "domcu_mem.h"
#include "errchk.h"
#include "prot_common.h"
#include "domcu_brk.h"
#include "ffwmcu_brk.h"
#include "mcu_inram.h"	// RevRxE2LNo141104-001 Append Line
#include "mcu_extram.h"
#include "mcu_extflash.h"
#include "mcu_flash.h"
// RevRxNo120910-006 Append Line
#include "ffwmcu_srm.h"
#include "mcuspec.h"		//	RevRxNo140515-011 Append Line

// ファイル内static変数の宣言
static BOOL		s_bFlashDataClearFlg;		// フラッシュROMブロックデータ初期化フラグ

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
static BOOL		s_bDownloadWtrLoad;				// ダウンロード用WTRプログラムのロード実行フラグ
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

static DWORD	s_dwDownloadWtrMemSize;	// RevRxNo121221-001 Append Line
static BYTE		*s_pbyDownloadWtrMem = NULL;
static BYTE		*s_pbyDownloadWtrVerMem = NULL;

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ユーザプログラムのダウンロード開始
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_DWNP_OPEN(void)
{
	FFWERR			ferrEnd;
	FFWRX_MCUAREA_DATA* pMcuArea;
	FFWMCU_DBG_DATA_RX*	pDbgData;

	ProtInit();

	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Move Start
	// DO_DWNP_OPEN()後に実施していた初期化処理を前に持ってきた
	SetUmCacheDataInit(FALSE);			// ユーザーマットキャッシュメモリ初期化未実行に設定
	SetUbmCacheDataInit(FALSE);			// ユーザーブートマットキャッシュメモリ初期化未実行に設定
	SetDataFlashInitFlag(FALSE);		// データマット初期化未実行に設定
	SetDataFlashEraseFlag(FALSE);		// データマットのみの初期化実行フラグをセット
	SetIdcodeChange(FALSE);				// IDコード書き換えフラグ初期化	RevRxNo140515-011 Append Line			
	// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Move End

	// RevRxNo140617-001 Append Line
	SetDwnpDataIncludeTMArea(FALSE);	// ダウンロードデータにTM領域データが含まれていたフラグをFALSEで初期化

	ClrResultCheckSum();		// チェックサム結果をクリア

	InitExtFlashWrite();		// 外部フラッシュダウンロード用変数初期化

	pDbgData = GetDbgDataRX();	// デバッグ情報取得

	// ライタモードの場合、必ずキャッシュクリア
	pMcuArea = GetMcuAreaDataRX();
	if (pDbgData->eWriter == EML_WRITERMODE_USE) {
		InitFlashRomCacheMemFlg();
		// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append Start
		// Hocoへの切り替えはBFWで実施するため削除 RevRxNo120615-001 Delete
		SetDwnpOpenData(TRUE);		// ここでDWNP中フラグを設定しておく必要あり
		InitFlashRomCacheWriter();	// ライタモード時用フラッシュキャッシュ初期化処理
		// V.1.02 No.25 内蔵ROMチェックサム、ベリファイチェック処理修正 Append End
	}

// V.1.02 RevNo110317-001 Delete Start
// ダウンロード時には埋め込まれたS/Wブレークは消えることを想定しているが、
// RAMの場合は消えていない(埋め込まれたまま)なのでPB_FILL_COMPLETEでなくなると
// 良くない。内蔵ROMの場合はここでPB_FILL_SETに戻してやる必要があるが、判別できない
// ので、writeDmaAccessでDWNP中でもS/Wチェックをするよう改定
#if 0
	pPb = GetPbData();			// S/Wブレークポイント設定を取得
	for (i = 0; i < pPb->dwSetNum; i++) {
		if (pPb->eFillState[i] == PB_FILL_COMPLETE) {
			UpdatePbData(pPb->dwmadrAddr[i], 0);	// S/Wブレーク設定状態更新
		}
	}
#endif
// V.1.02 RevNo110317-001 Delete End

	ferrEnd = ProtEnd();
	return FFWERR_OK;
}

//=============================================================================
/**
 * ユーザプログラムのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_DWNP_CLOSE(FFW_DWNP_CHKSUM_DATA* chksum)
{
	FFWERR	ferr;
	FFWERR			ferrEnd;
	FFW_VERIFYERR_DATA	VerifyErr;
	BOOL	bFlashWrite = FALSE;
	// RevRxNo120910-006 Append Start
	FFW_SRM_DATA* pSrmData;
	BYTE*	pbyRwBuff;
	MADDR	dwmadrWorkProgramStartAddr;				// ワーク・プログラム スタートアドレス
	MADDR	dwmadrWorkProgramEndAddr;				// ワーク・プログラム エンドアドレス
	DWORD	dwCacheIdcode;						// IDコードのキャッシュメモリ開始位置
	// RevRxNo120910-006 Append End
	// RevRxNo140515-011 Append Start
	BYTE*	pbyCacheID;
	USER_ROMCACHE_RX	*um_ptr;	// ユーザマットキャッシュメモリ構造体ポインタ
	um_ptr = GetUserMatCacheData();
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo150827-002 Append Line
	pFwCtrl = GetFwCtrlData();		
	pMcuDef = GetMcuDefData();		// RevRxNo150827-002 Append Line
	// RevRxNo140515-011 Append End

	ProtInit();
	VerifyErr.eErrorFlag = VERIFY_OK;	//メモリエラー対応

	// 外部フラッシュ書き込み確認
	if( !GetExtFlashWriteFlg() ){			// 外部フラッシュへの書き込み未実行の場合
		// 外部フラッシュダウンロード終了処理実行
		ferr = ExtWriteFlashData(GetExtFlashVerify(), &VerifyErr);
	}
	// 外部フラッシュダウンロード終了処理実行
	ferr = ExtWriteFlashEnd();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	// RevRxE2LNo141104-001 Append Start
	if (GetInRamDwnpCacheFlg()) {	// 内部RAMダウンロードデータ用キャッシュがある場合
		ferr = WriteInRamData();
		if (ferr != FFWERR_OK) {
			//ExtWriteFlashEnd();	//不要なため削除 2015/3/6
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned Start
	if( ExtramDwnpTop != NULL ){	// 外部RAMダウンロード用キャッシュにデータがある場合
		// RevRxE2LNo141104-001 Modify Start
		ferr = ExtWriteRamData();
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		// RevRxE2LNo141104-001 Modify End
	}
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned End

	// フラッシュROMキャッシュ変更ありの場合、フラッシュROMへの書込み実施
	bFlashWrite = GetStateFlashRomCache();
	if (bFlashWrite == TRUE) {
		SetFlashDataClearFlg();										// フラッシュROMブロック初期化情報を"初期化済み"に設定しておく
		// フラッシュROMへのライト処理実施
		ferr = WriteFlashExec(VERIFY_ON);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}
	//RevRxNo140515-011 Append Start
	if (GetIdcodeChange()) {				// IDコードが書き換えられた場合
		if (pFwCtrl->eIdcodeType == RX_IDCODE_FLASH) {		// IDコード設定種別がフラッシュROM上の場合
			dwCacheIdcode = (MCU_IDCODE_ADDR_START - GetCacheStartAddr(MAREA_USER));
			pbyCacheID = &um_ptr->pbyCacheMem[dwCacheIdcode];
		}else{		  // IDコード設定種別がExtra領域上の場合
			dwCacheIdcode = (pMcuDef->dwOsisStartOffset - pMcuDef->dwExtraAStartOffset);	// RevRxNo150827-002 Modify Line
			pbyCacheID = GetExtraCacheAddr(dwCacheIdcode);
		}
		SetIdCode(pbyCacheID);		// IDコードを内部変数へ格納
	}
	//RevRxNo140515-011 Append End

	ClrFlashDataClearFlg();	// フラッシュROMブロック初期化情報を"未初期化"に設定

	// チェックサム結果取得
	GetCheckSum(chksum);

// RevRxNo120910-006 Append Start	SRM高速化対応(バイトアクセスでワーク・プログラム領域ライト処理追加)
	// ワーク・プログラム領域は、RAM領域に配置なので、ここに以下のコードを置いている。
	// 注意：ワーク・プログラム領域をROM領域に配置することも許す仕様に変更になった場合は、
	// 　　　以下コードの配置箇所や実施する/しないの仕様検討が必要。
	pSrmData = GetSrmData();
	// Startルーチン有効、またはStopルーチン有効時
	if(((pSrmData->dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((pSrmData->dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)){
		pbyRwBuff = GetSrmWorkProgramCode(pSrmData);
		dwmadrWorkProgramStartAddr = pSrmData->dwmadrRamWorkAreaStartAddr + WORKPROGRAMSTARTADDR;
		dwmadrWorkProgramEndAddr   = dwmadrWorkProgramStartAddr + SRM_WORK_PROGRAM_LENGTH - 0x00000001;
		ferr = DO_CWRITE(dwmadrWorkProgramStartAddr, 
		                 dwmadrWorkProgramEndAddr, VERIFY_OFF, pbyRwBuff, &VerifyErr);
		if(ferr != FFWERR_OK){
			ferrEnd = ProtEnd();
			return ferr;				// FFWERR_OK でなければ、以下は実行しない。
		}
	}
// RevRxNo120910-006 Append End

	ferr = ProtEnd();

	return ferr;
}

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
//=============================================================================
/**
 * Flash書き込みプログラムのロード
 * @param	eWtrType	WTRプログラム種別
 * @param	dwDataSize	WTRプログラムデータサイズ
 * @param	pbyDataBuff	WTRプログラム格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_WTRLOAD(enum FFWRXENM_WTRTYPE eWtrType, DWORD dwDataSize, const BYTE* pbyDataBuff)
{
	
	FFWERR	ferr = FFWERR_OK;

	if(eWtrType == WTRTYPE_0){

		// ダウンロード用WTRプログラム格納メモリの開放
		DeleteWtrLoadMem(eWtrType);

		// ダウンロード用WTRプログラムをロード未実行済に設定
		SetDownloadWtrLoad(FALSE);

		// ダウンロード用WTRプログラム格納メモリ確保
		// RevRxNo121221-001 Modify Start
		s_dwDownloadWtrMemSize = dwDataSize-WRT_VER_MEM_LENGTH;	// WTRプログラム格納メモリサイズ
		s_pbyDownloadWtrMem = new BYTE [s_dwDownloadWtrMemSize];
		// RevRxNo121221-001 Modify End

		// ダウンロード用WTRプログラムバージョン格納メモリ確保
		s_pbyDownloadWtrVerMem = new BYTE [WRT_VER_MEM_LENGTH];


		// ダウンロード用WTRプログラム格納メモリ初期化
		// RevRxNo121221-001 Modify Line
		memset(s_pbyDownloadWtrMem, 0xFF, s_dwDownloadWtrMemSize);

		// ダウンロード用WTRプログラムバージョン格納メモリ初期化
		memset(s_pbyDownloadWtrVerMem, 0xFF, WRT_VER_MEM_LENGTH);
	
		// ダウンロード用WTRプログラム格納メモリにWTRプログラムを設定
		// RevRxNo121221-001 Modify Line
		memcpy(s_pbyDownloadWtrMem, pbyDataBuff, s_dwDownloadWtrMemSize);
	
		// ダウンロード用WTRプログラムバージョン格納メモリにWTRプログラムバージョンを設定
		memcpy(s_pbyDownloadWtrVerMem, (pbyDataBuff + dwDataSize-WRT_VER_MEM_LENGTH), WRT_VER_MEM_LENGTH);
	
		// ダウンロード用WTRプログラムをロード済みに設定
		SetDownloadWtrLoad(TRUE);
	}
	return ferr;
}
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End


// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
//=============================================================================
/**
 * Flash書き込みプログラムのバージョン参照
 * @param	eWtrType	WTRプログラム種別
 * @param	psWtrVer	WTRプログラムバージョン格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
 FFWERR DO_WTRVER(enum FFWRXENM_WTRTYPE eWtrType, char *const psWtrVer)
{
	FFWERR	ferr = FFWERR_OK;
	// RevNo121017-007	Modify Start
	// "NotLoad 00000000"+NULLコードをコピーするには領域が小さすぎるので領域を1拡大した。
	char	strWtrNotLoadMsg[WRT_NOT_LOAD_STRING_SIZE];			
	// RevNo121017-003	Append Line
	errno_t ernerr;

	 // ダウンロード用WTRプログラムのロード状態を参照
	if(eWtrType == WTRTYPE_0){
		if(GetDownloadWtrLoad() == TRUE){
			memcpy(psWtrVer, GetDownloadWtrVerAddr(), WRT_VER_MEM_LENGTH);
		}
		else{
			// RevNo121017-003	Modify Line
			ernerr = strcpy_s( strWtrNotLoadMsg, WRT_NOT_LOAD_STRING_SIZE, "NotLoad 00000000" );
			// RevNo110216-004 Modify Line
			memcpy(psWtrVer, strWtrNotLoadMsg, WRT_NOT_LOAD_STRING_SIZE);
		}
	}

	return ferr;
}

//=============================================================================
/**
 * フラッシュメモリ初期化ブロックの設定
 * @param pFclr ダウンロード時に初期化するフラッシュメモリ情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
void DO_SetFCLR(void){

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modidy Line
	InitFlashRomCacheMemFlg();	// フラッシュROMキャッシュメモリ管理フラグ初期化
}


//=============================================================================
/**
 * フラッシュROMブロックデータ初期化情報を取得する。
 * @param なし
 * @retval TRUE フラッシュROMブロックデータ初期化済み
 * @retval FALSE フラッシュROMブロックデータを未初期化
 */
//=============================================================================
BOOL GetFlashDataClearFlg(void)
{
	return s_bFlashDataClearFlg;
}

//=============================================================================
/**
 * フラッシュROMブロックデータ初期化フラグを初期化済みに設定する。
 * @param なし
 * @retval なし
 */
//=============================================================================
void SetFlashDataClearFlg(void)
{
	s_bFlashDataClearFlg = TRUE;
}

//=============================================================================
/**
 * フラッシュROMブロックデータ初期化フラグを未初期化に設定する。
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrFlashDataClearFlg(void)
{
	s_bFlashDataClearFlg = FALSE;
}

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
//=============================================================================
/**
 * ダウンロード用WTRプログラム格納済みフラグを設定する。
 * @param BOOL bDownloadWtrLoad
 * @retval なし
 */
//=============================================================================
void SetDownloadWtrLoad(BOOL bDownloadWtrLoad)
{
	s_bDownloadWtrLoad = bDownloadWtrLoad;
}
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
//=============================================================================
/**
 * ダウンロード用WTRプログラム格納済みフラグを参照する。
 * @param なし
 * @retval s_bDownloadWtrLoad
 */
//=============================================================================
BOOL GetDownloadWtrLoad(void)
{
	return s_bDownloadWtrLoad;
}
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

// RevRxNo121221-001 Append Start
//=============================================================================
/**
 * ダウンロード用WTRプログラム格納メモリのサイズを返送する
 * @param BOOL bDownloadWtrLoad
 * @retval なし
 */
//=============================================================================
DWORD GetDownloadWtrDataSize(void)
{
	return s_dwDownloadWtrMemSize;
}
// RevRxNo121221-001 Append End

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
//=============================================================================
/**
 * ダウンロード用WTRプログラム格納メモリのポインタアドレスを返送する
 * @param BOOL bDownloadWtrLoad
 * @retval なし
 */
//=============================================================================
BYTE* GetDownloadWtrDataAddr()
{
	return s_pbyDownloadWtrMem;
}
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
//=============================================================================
/**
 * ダウンロード用WTRプログラムバージョン格納メモリのポインタアドレスを返送する
 * @param BOOL bDownloadWtrLoad
 * @retval なし
 */
//=============================================================================
BYTE* GetDownloadWtrVerAddr()
{
	return s_pbyDownloadWtrVerMem;
}
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

// V.1.02 No.22 書き込みプログラムダウンロード化 Append Start
//=============================================================================
/**
 * WTRプログラムロード用メモリの開放
 * @param なし
 * @retval なし
 */
//=============================================================================
void DeleteWtrLoadMem(enum FFWRXENM_WTRTYPE eWtrType)
{

	if(eWtrType == WTRTYPE_0){
		// ダウンロード用WTRプログラム格納メモリの開放
		if(s_pbyDownloadWtrMem != NULL){
			delete s_pbyDownloadWtrMem;
			s_pbyDownloadWtrMem = NULL;
		}
		// RevRxNo121221-001 Append Line
		s_dwDownloadWtrMemSize = 0;

		// ダウンロード用WTRプログラムバージョン格納メモリの開放
		if(s_pbyDownloadWtrVerMem != NULL){
			delete s_pbyDownloadWtrVerMem;
			s_pbyDownloadWtrVerMem = NULL;
		}
		// ダウンロード用WTRプログラムをロード未実行済に設定
		SetDownloadWtrLoad(FALSE);
	}
}
// V.1.02 No.22 書き込みプログラムダウンロード化 Append End

//=============================================================================
/**
 * プログラムダウンロードコマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Dwn(void)
{
	s_bFlashDataClearFlg = FALSE;
	// V.1.02 No.22 書き込みプログラムダウンロード化 Append Line
	SetDownloadWtrLoad(FALSE);
	// RevRxNo121221-001 Append Line
	s_dwDownloadWtrMemSize = 0;
}