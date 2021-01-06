///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_mem.cpp
 * @brief メモリ操作関連コマンドの実装ファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, S.Ueda, Y.Miyake, K.Uemori、Y.Kawakami, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/20
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・V.1.03 RevNo111121-008 OFS1ライトデータチェック 2012/07/11 橋口
・RevNo111121-007	2012/07/11 橋口
  ・FFWMCUCmd_MOVE()
    ・以下のWarning を検出する為、DO_MOVE() 実行後にWarningChk_MemAccessErr() を
      実行するように変更。
	    FFWERR_READ_ROM_FLASHDBG_MCURUN
・RevNo121017-002	2012/10/31 上田
　・FFWMCUCmd_WRITE()  <VS2008 ERROR C2148対策>
 　　開始アドレス～終了アドレスが128MB以上の場合引数エラーとするよう変更。
・RevRxNo120910-006	2012/11/12 三宅
  ・FFWMCUCmd_FILL()、FFWMCUCmd_WRITE()、FFWMCUCmd_MOVE()、FFWMCUCmd_CWRITE()で、
    指定ルーチン有効時は、ライト領域と指定ルーチン用ワーク・プログラム領域との
    重なりがあれば、エラー FFWERR_WRITE_SRM_WORKRAM を返すことを追加。
  ・ffwmcu_srm.h のインクルード追加。
・RevRxNo121122-005 2012/11/22 明石(2012/12/04 SDS 岩田 マージ)
  FFWMCUCmd_WRITEで、外部フラッシュメモリ　メーカIDデバイスID不一致エラー発生後、FFWCmd_GetErrorDetailでエラー詳細情報を
  取り込んだ後、FFWE20Cmd_DWNP_CLOSEを実行すると外部フラッシュメモリ メーカIDデバイスID不一致エラー発生するが、
  この時FFWCmd_GetErrorDetailでエラー詳細情報が取り込めないという不具合の修正
・RevRxNo121120-001	2012/11/27 上田
　メモリアクセスコマンドで低消費電力エラーが発生しない不具合修正
・RevRxNo121122-001 2012/11/27 上田
　低消費電力中のFILL/WRITEで実行中のROMアクセスエラーが発生しない不具合修正
・RevRxNo121122-001	2012/11/27 上田(2012/12/04 SDS 岩田 マージ)
　低消費電力中のMOVEで実行中のROMアクセスエラーが発生しない不具合修正
・RevRxNo120910-006	2012/12/10 三宅
　FFWMCUCmd_FILL()、FFWMCUCmd_WRITE()、FFWMCUCmd_MOVE()、FFWMCUCmd_CWRITE() で、
　引数エラーで指定ルーチン用ワーク・プログラムとの重なりの確認を
　ワークRAMとの重なりの確認に変更。
・RevRxNo121122-001	2012/12/11 SDS 岩田(2012/12/19 橋口マージ)
　低消費電力中のCWRITEで実行中のROMアクセスエラーが発生しない不具合修正
・RevRxNo130730-005 2013/12/05 上田
	内蔵ROM有効/無効判定処理改善
・RevRxNo140109-001 2014/01/23 植盛
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo140515-005 2014/07/18 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo140515-011 2014/07/24 川上
　　RX64M IDコード ALL F以外のユーザブート起動対応
・RevRxE2LNo141104-001 2014/11/20 上田
	E2 Lite対応
*/
#include "ffwmcu_mem.h"
#include "domcu_mem.h"
#include "domcu_prog.h"
#include "ffw_sys.h"
#include "errchk.h"
#include "mcu_mem.h"
#include "mcu_flash.h"
// Ver.1.01 2010/08/17 SDS T.Iwata
#include "ffwmcu_dwn.h"
#include "domcu_mcu.h"
#include "mcu_extflash.h"
//RevRxNo120910-006 Append Line
#include "ffwmcu_srm.h"

// RevRxNo140109-001 Append Line
// static変数
static BOOL		s_bWriteErrFlg;		// WRITE処理実行時のエラー有無フラグ

// RevRxNo140109-001 Append Line
// static関数の宣言
static FFWERR chkWriteCmdPara(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff);


///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * メモリダンプ
 * @param dwmadrStartAddr 開始アドレス
 * @param dwmadrEndAddr 終了アドレス
 * @param eAccessSize アクセスサイズ
 * @param pbyReadBuff リードデータ格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_DUMP(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, 
								 BYTE *const pbyReadBuff)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	DWORD	dwTotalLength;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	pMcuDef = GetMcuDefData();

	// 引数チェック
	if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (dwmadrEndAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}

	if (dwmadrStartAddr > dwmadrEndAddr) {	// 開始アドレスが終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}

	// アクセスサイズのエラーチェック
	if ((ferr = ErrorChk_McuAccessSize(eAccessSize)) != FFWERR_OK) {
		return ferr;
	}

	dwTotalLength = dwmadrEndAddr - dwmadrStartAddr + 1;	// リードバイト数算出

	// RevRxE2LNo141104-001 Delete:MCUステータス確認処理削除(ターゲット/MCUステータス確認はBFWで実施する)。

	// メモリアクセスが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		// RevRxE2LNo141104-001 Append Line
		memset(pbyReadBuff, 0xFF, dwTotalLength);	// エラー発生時は0xFFで埋める
		return ferr;
	}

	ferr = ErrorChk_Dump(dwmadrStartAddr, dwmadrEndAddr);	// MCUに依存するエラーチェック
	if (ferr != FFWERR_OK) {
		// RevRxE2LNo141104-001 Append Line
		memset(pbyReadBuff, 0xFF, dwTotalLength);	// エラー発生時は0xFFで埋める
		return ferr;
	}

	// V.1.02 RevNo110308-002 Append Line
	ClrMemAccWarningFlg();		// FFWERR_READ_DTF_MCURUN/FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング記録変数初期化

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
 	ferr = DO_DUMP(dwmadrStartAddr, dwmadrEndAddr, eAccessSize, pbyReadBuff);	// DUMP処理実行
	if (ferr != FFWERR_OK) {
		memset(pbyReadBuff, 0xFF, dwTotalLength);	// エラー発生時は0xFFで埋める
		return ferr;
	}
	// V.1.02 RevNo110308-002 Append Line
	ferr = WarningChk_MemAccessErr();				// メモリアクセス時のWarning確認

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return	ferr;
}

//=============================================================================
/**
 * 単一データのメモリ書き込み
 * @param dwmadrStartAddr 開始アドレス
 * @param dwmadrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param dwWriteDataSize 書き込みデータサイズ
 * @param pbyWriteBuff 書き込みデータ格納バッファアドレス
 * @param pVerifyErr ベリファイ結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_FILL(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
								 enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
								 FFW_VERIFYERR_DATA* pVerifyErr)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	// RevRxNo120910-006 Append Start
	FFW_SRM_DATA* pSrmData;
	MADDR	dwmadrWorkRamStartAddr;				// ワーク・プログラム スタートアドレス
	MADDR	dwmadrWorkRamEndAddr;				// ワーク・プログラム エンドアドレス
	// RevRxNo120910-006 Append end

	SetIdcodeChange(FALSE);				// IDコード書き換えフラグ初期化	RevRxNo140515-011 Append Line	
	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	pMcuDef = GetMcuDefData();

	// 引数チェック
	if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (dwmadrEndAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (dwmadrStartAddr > dwmadrEndAddr) {	// 開始アドレスが終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}

	// アクセスサイズのエラーチェック
	if ((ferr = ErrorChk_McuAccessSize(eAccessSize)) != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo120910-006 Append Start
	pSrmData = GetSrmData();
	// Startルーチン有効、またはStopルーチン有効時
	if(((pSrmData->dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((pSrmData->dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)){
		// RevRxNo120910-006 Modify Start
		dwmadrWorkRamStartAddr = pSrmData->dwmadrRamWorkAreaStartAddr;
		dwmadrWorkRamEndAddr   = dwmadrWorkRamStartAddr + SRM_WORKRAM_LENGTH - 0x00000001;
		// RevRxNo120910-006 Modify End
		if (!((dwmadrEndAddr < dwmadrWorkRamStartAddr) || (dwmadrWorkRamEndAddr < dwmadrStartAddr))) {	// 本体部の判定。
			return FFWERR_WRITE_SRM_WORKRAM;	// エラー：指定ルーチン実行機能用ワークRAM領域への書き込みは実行できない
		}
	}
	// RevRxNo120910-006 Append End

	switch (dwWriteDataSize) {
	case MDATASIZE_1BYTE:	// no break
	case MDATASIZE_2BYTE:	// no break
	case MDATASIZE_4BYTE:	// no break
	case MDATASIZE_8BYTE:
		break;
	default:
		return FFWERR_FFW_ARG;
	}

	if (eAccessSize == MWORD_ACCESS) {	// 書き込みデータサイズがアクセスサイズの倍数であるかをチェック
		if ((dwWriteDataSize % 2) != 0) {
			return FFWERR_FFW_ARG;
		}
	}
// 2008.8.18 INSERT_BEGIN_E20RX600(+5) {
	else if (eAccessSize == MLWORD_ACCESS) {	// ロングワードアクセスのとき
		if ((dwWriteDataSize % 4) != 0) {
			return FFWERR_FFW_ARG;
		}
	}
// 2008.8.18 INSERT_END_E20RX600 }


	// RevRxNo121122-001 Delete 
	// 内蔵ROM無効拡張モードフラグをここで行うと、実行中のROM領域へのアクセスで「FFWERR_WRITE_ROMAREA_MCURUNエラー」
	// より先に「FFWERR_BMCU_STANBY(低消費電力モード)エラー」が出るためErrorChk_Fill()内に移動

	// V.1.02 新デバイス対応 Modify Line
	ferr = ErrorChk_Fill(dwmadrStartAddr, dwmadrEndAddr, dwWriteDataSize, pbyWriteBuff);	// MCUに依存するエラーチェック	// RevNo090310_003
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxE2LNo141104-001 Delete:MCUステータス確認処理削除(ターゲット/MCUステータス確認はBFWで実施する)。

	// ベリファイエラー内容初期化
	pVerifyErr->eErrorFlag = VERIFY_OK;
	pVerifyErr->eAccessSize = MBYTE_ACCESS;
	pVerifyErr->dwErrorAddr = 0;
	pVerifyErr->dwErrorWriteData = 0;
	pVerifyErr->dwErrorReadData = 0;

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_FILL(dwmadrStartAddr, dwmadrEndAddr, eVerify, eAccessSize, dwWriteDataSize, pbyWriteBuff, pVerifyErr);	// FILL処理実行

	// メモリライトが発生するFFW I/F発行時、メモリライト処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグをTRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * メモリ書き込み
 * @param dwmadrStartAddr 開始アドレス
 * @param dwmadrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pbyWriteBuff 書き込みデータ格納バッファアドレス
 * @param pVerifyErr ベリファイ結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_WRITE(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify,
								  enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff,
								  FFW_VERIFYERR_DATA* pVerifyErr)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR ferr = FFWERR_OK;
	// RevRxNo140109-001 Delete
	// RevRxNo121122-005	Append Line
	FFW_ERROR_DETAIL_INFO*	pFerdiData;				// デバイスID メーカID不一致エラーの詳細情報テーブルのポインタ

	if (GetDwnpOpenData() == FALSE) {			// DWNP中は実施しない
		SetIdcodeChange(FALSE);				// IDコード書き換えフラグ初期化	RevRxNo140515-011 Append Line
	}
	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	// RevRxNo140109-001 Modify Start
	ferr = chkWriteCmdPara(dwmadrStartAddr, dwmadrEndAddr, eAccessSize, pbyWriteBuff);
	if (ferr != FFWERR_OK) {	// WRITEコマンドのパラメータでエラーが発生
		s_bWriteErrFlg = TRUE;		// WRITE処理中にエラーが発生した
		return ferr;
	}
	// RevRxNo140109-001 Modify End

	// ベリファイエラー内容初期化
	pVerifyErr->eErrorFlag = VERIFY_OK;
	pVerifyErr->eAccessSize = MBYTE_ACCESS;
	pVerifyErr->dwErrorAddr = 0;
	pVerifyErr->dwErrorWriteData = 0;
	pVerifyErr->dwErrorReadData = 0;

// ExtFlashModule_002b Append Line
	ClrExistLockSectFlg();	// ロックされているセクタがあったかを示すフラグをクリア

	ferr = DO_WRITE(dwmadrStartAddr, dwmadrEndAddr, eVerify, eAccessSize, pbyWriteBuff, pVerifyErr);	// 処理実行

	// メモリライトが発生するFFW I/F発行時、メモリライト処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグをTRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// ExtFlashModule_002b Append Start
	if (ferr != FFWERR_OK) {	// FFWERR_OK でない場合。
		// RevRxNo121122-005	Append Start
		if(ferr == FFWERR_EXTROM_ID) {
			pFerdiData = GetExtFlashErrorDetailInfo();
			SetErrorDetailInfo(pFerdiData,EFLASHERR_MAX);
		}
		// RevRxNo121122-005	Append End

		// RevRxNo140109-001 Append Line
		s_bWriteErrFlg = TRUE;		// WRITE処理中にエラーが発生した
		return	ferr;
	} else {					// FFWERR_OK の場合
		ferr = WarningChk_ExtFlashWriteErr();	// 外部フラッシュダウンロード時のエラー確認

		// RevRxNo140515-005 Append Start
		if ( GetDwnpOpenData() != TRUE ){		// DWNP中ではない場合のみWarning表示
			if(ferr == FFWERR_OK){
				ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
			}
		}
		// RevRxNo140515-005 Append End

		// V.1.02 RevNo110613-001 Apped Start
		if(ferr == FFWERR_OK){
			ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
		}
		// V.1.02 RevNo110613-001 Apped End
		return ferr;
	// ExtFlashModule_002b Append End
	}
}

//=============================================================================
/**
 * メモリコピー
 * @param dwmadrSrcStartAddr 転送元開始アドレス
 * @param dwmadrSrcEndAddr 転送元終了アドレス
 * @param dwmadrDisStartAddr 転送先開始アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pVerifyErr ベリファイ結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_MOVE(DWORD dwmadrSrcStartAddr, DWORD dwmadrSrcEndAddr, DWORD dwmadrDisStartAddr, 
								 enum FFWENM_VERIFY_SET eVerify, enum FFWENM_MACCESS_SIZE eAccessSize, 
								 FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	MADDR	madrDisEndAddr;
	DWORD	dwMoveLength;
	DWORD	dwRange;
	// RevRxNo120910-006 Append Start
	FFW_SRM_DATA* pSrmData;
	MADDR	dwmadrWorkRamStartAddr;				// ワーク・プログラム スタートアドレス
	MADDR	dwmadrWorkRamEndAddr;				// ワーク・プログラム エンドアドレス
	// RevRxNo120910-006 Append end

	SetIdcodeChange(FALSE);				// IDコード書き換えフラグ初期化	RevRxNo140515-011 Append Line
	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	pMcuDef = GetMcuDefData();

	// 引数チェック
	if (dwmadrSrcStartAddr > pMcuDef->madrMaxAddr) {	// 転送元開始アドレス
		return FFWERR_FFW_ARG;
	}
	if (dwmadrSrcEndAddr > pMcuDef->madrMaxAddr) {	// 転送元終了アドレス
		return FFWERR_FFW_ARG;
	}
	if (dwmadrSrcStartAddr > dwmadrSrcEndAddr) {	// 転送元開始アドレスが転送元終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}
	if (dwmadrDisStartAddr > pMcuDef->madrMaxAddr) {	// 転送先開始アドレス
		return FFWERR_FFW_ARG;
	}

	// 指定領域チェック(16MB以上ならエラー)
	dwRange = dwmadrSrcEndAddr - dwmadrSrcStartAddr + 1;
	if (dwRange > MEM_MAX_RANGE || dwRange == 0) {	// 0xFFFFFFFF - 0x00000000 + 1 の場合、dwRange == 0 になる
		return FFWERR_FFW_ARG;
	}

	dwMoveLength = dwmadrSrcEndAddr - dwmadrSrcStartAddr + 1;
	madrDisEndAddr = dwmadrDisStartAddr + dwMoveLength - 1;	// 転送先終了アドレス
	if (madrDisEndAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (dwmadrDisStartAddr > madrDisEndAddr) {		// 転送先開始アドレスが転送先終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}

	// 転送元の合計サイズがアクセスサイズ未満の場合
	if ((eAccessSize == MWORD_ACCESS) && (dwMoveLength < 2)) {		// ワード指定で2Byte未満
		return FFWERR_FFW_ARG;
	}
	if ((eAccessSize == MLWORD_ACCESS) && (dwMoveLength < 4)) {		// ロングワード指定で4Byte未満
		return FFWERR_FFW_ARG;
	}

	// RevRxNo120910-006 Append Start
	pSrmData = GetSrmData();
	// Startルーチン有効、またはStopルーチン有効時
	if(((pSrmData->dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((pSrmData->dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)){
		// RevRxNo120910-006 Modify Start
		dwmadrWorkRamStartAddr = pSrmData->dwmadrRamWorkAreaStartAddr;
		dwmadrWorkRamEndAddr   = dwmadrWorkRamStartAddr + SRM_WORKRAM_LENGTH - 0x00000001;
		// RevRxNo120910-006 Modify End
		if (!((madrDisEndAddr < dwmadrWorkRamStartAddr) || (dwmadrWorkRamEndAddr < dwmadrDisStartAddr))) {	// 本体部の判定。
			return FFWERR_WRITE_SRM_WORKRAM;	// エラー：指定ルーチン実行機能用ワークRAM領域への書き込みは実行できない
		}
	}
	// RevRxNo120910-006 Append End

	// RevRxNo121122-001 Delete 
	// 内蔵ROM無効拡張モードフラグをここで行うと、実行中のROM領域へのアクセスで「FFWERR_WRITE_ROMAREA_MCURUNエラー」
	// より先に「FFWERR_BMCU_STANBY(低消費電力モード)エラー」が出るためErrorChk_Move()内に移動

	// V.1.02 新デバイス対応 Modify Line
	ferr = ErrorChk_Move(dwmadrSrcStartAddr, dwmadrSrcEndAddr, dwmadrDisStartAddr, madrDisEndAddr, eAccessSize);
				// MCUに依存するアドレスチェック
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxE2LNo141104-001 Delete:MCUステータス確認処理削除(ターゲット/MCUステータス確認はBFWで実施する)。

	// ベリファイエラー内容初期化
	pVerifyErr->eErrorFlag = VERIFY_OK;
	pVerifyErr->eAccessSize = MBYTE_ACCESS;
	pVerifyErr->dwErrorAddr = 0;
	pVerifyErr->dwErrorWriteData = 0;
	pVerifyErr->dwErrorReadData = 0;

// RevNo111121-007 Append Start
	// FFWERR_READ_DTF_MCURUN/FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング記録変数初期化
	ClrMemAccWarningFlg();
// RevNo111121-007 Append End


	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_MOVE(dwmadrSrcStartAddr, dwmadrSrcEndAddr, dwmadrDisStartAddr, madrDisEndAddr, eVerify, eAccessSize, pVerifyErr);	// MOVE処理実行

	// メモリライトが発生するFFW I/F発行時、メモリライト処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグをTRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

// RevNo111121-007 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemAccessErr();		// メモリアクセス時のWarning確認  RevNo111121-007 Append Line
	}
// RevNo111121-007 Append End

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * メモリ比較
 * @param dwmadrSrcStartAddr 比較元開始アドレス
 * @param dwmadrSrcEndAddr 比較元終了アドレス
 * @param dwmadrDisStartAddr 比較先開始アドレス
 * @param eAccessSize アクセスサイズ
 * @param dwCompDataSize 比較単位サイズ
 * @param pCompError 比較結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_MCMP(DWORD dwmadrSrcStartAddr, DWORD dwmadrSrcEndAddr, DWORD dwmadrDisStartAddr, 
								 enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwCompDataSize, FFW_COMPERR_DATA* pCompError)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	DWORD	dwRange;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	pMcuDef = GetMcuDefData();

	// 引数チェック
	if (dwmadrSrcStartAddr > pMcuDef->madrMaxAddr) {
		// 比較元開始アドレスがMCU 空間アドレスの最大値を超える場合
		return FFWERR_FFW_ARG;
	}
	if (dwmadrSrcEndAddr > pMcuDef->madrMaxAddr) {
		// 比較元終了アドレスがMCU 空間アドレスの最大値を超える場合
		return FFWERR_FFW_ARG;
	}
	if (dwmadrDisStartAddr > pMcuDef->madrMaxAddr) {
		// 比較先開始アドレスがMCU 空間アドレスの最大値を超える場合
		return FFWERR_FFW_ARG;
	}
	if (dwmadrSrcStartAddr > dwmadrSrcEndAddr) {
		// 比較元開始アドレスが比較元終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}
	if ((dwmadrSrcEndAddr - dwmadrSrcStartAddr) > (pMcuDef->madrMaxAddr - dwmadrDisStartAddr)) {
		// 比較先開始アドレスからの比較対象領域がMCU 空間アドレスの最大値を越える場合
		return FFWERR_FFW_ARG;
	}

	// 指定領域チェック(16MB以上ならエラー)
	dwRange = (dwmadrSrcEndAddr - dwmadrSrcStartAddr + 1) * 2;	// 比較元＋比較先なので×２する
	if (dwRange > MEM_MAX_RANGE || dwRange == 0) {	// 0xFFFFFFFF - 0x00000000 + 1 の場合、dwRange == 0 になる
		return FFWERR_FFW_ARG;
	}

	// アクセスサイズのエラーチェック
	if ((ferr = ErrorChk_McuAccessSize(eAccessSize)) != FFWERR_OK) {
		return ferr;
	}

	// 比較データサイズチェック
	if ((dwCompDataSize != 1) && (dwCompDataSize != 2) && 
		(dwCompDataSize != 4) && (dwCompDataSize != 8)) {
		// 比較データサイズが1, 2, 4, 8 のいずれかでない場合
		return FFWERR_FFW_ARG;
	}

	// 比較データサイズがアクセスサイズの倍数であるかをチェック
	if (eAccessSize == MWORD_ACCESS) {
		if ((dwCompDataSize % 2) != 0) {
			return FFWERR_FFW_ARG;
		}
	} else if (eAccessSize == MLWORD_ACCESS) {
		if ((dwCompDataSize % 4) != 0) {
			return FFWERR_FFW_ARG;
		}
	}

	// ユーザプログラム実行中エラー確認
	if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
		// ユーザプログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	// RevRxE2LNo141104-001 Delete:MCUステータス確認処理削除(ターゲット/MCUステータス確認はBFWで実施する)。

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// メモリアクセスが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_MCMP(dwmadrSrcStartAddr, dwmadrSrcEndAddr, dwmadrDisStartAddr, 
				eAccessSize, dwCompDataSize, pCompError);

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * メモリ検索
 * @param dwmadrStartAddr 検索開始アドレス
 * @param dwmadrEndAddr 検索終了アドレス
 * @param eSearchEqual 検索方法
 * @param eSerchForward 検索方向
 * @param eSearchPatern 検索方式
 * @param eAccessSize アクセスサイズ
 * @param dwSearchDataSize 検索データサイズ
 * @param pbySearchDataBuff 検索データ格納バッファアドレス
 * @param pbySearchMaskBuff マスクデータ格納バッファアドレス
 * @param pSearchAns 検索結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_MSER(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_MSEARCH_EQU eSearchEqual, 
								 enum FFWENM_MSEARCH_DIR eSerchForward, enum FFWENM_MSEARCH_PATERN eSearchPatern, 
								 enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwSearchDataSize, const BYTE* pbySearchDataBuff, 
								 const BYTE* pbySearchMaskBuff, FFW_SEARCH_DATA* pSearchAns)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	DWORD	dwRange;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	pMcuDef = GetMcuDefData();

	// 引数チェック
	if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {
		// 開始アドレスがMCU 空間アドレスの最大値を超える場合
		return FFWERR_FFW_ARG;
	}
	if (dwmadrEndAddr > pMcuDef->madrMaxAddr) {
		// 終了アドレスがMCU 空間アドレスの最大値を超える場合
		return FFWERR_FFW_ARG;
	}
	if (dwmadrStartAddr > dwmadrEndAddr) {
		// 開始アドレスが終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}
	if (eSearchEqual != EQU_SEARCH && eSearchEqual != NOTEQU_SEARCH) {
		// 検索方法が指定外の場合
		return FFWERR_FFW_ARG;
	}
	if (eSerchForward != FORWARD_SEARCH && eSerchForward != BACK_SEARCH) {
		// 検索方向が指定外の場合
		return FFWERR_FFW_ARG;
	}
	if (eSearchPatern != PATERN_SEARCH && eSearchPatern != DATASIZE_SEARCH) {
		// 検索方式が指定外の場合
		return FFWERR_FFW_ARG;
	}

	// 指定領域チェック(16MB以上ならエラー)
	dwRange = dwmadrEndAddr - dwmadrStartAddr + 1;
	if (dwRange > MEM_MAX_RANGE || dwRange == 0) {	// 0xFFFFFFFF - 0x00000000 + 1 の場合、dwRange == 0 になる
		return FFWERR_FFW_ARG;
	}

	// アクセスサイズのエラーチェック
	if ((ferr = ErrorChk_McuAccessSize(eAccessSize)) != FFWERR_OK) {
		return ferr;
	}

	// 検索データサイズがアクセスサイズの倍数であるかをチェック
	if (eAccessSize == MWORD_ACCESS) {
		if ((dwSearchDataSize % 2) != 0) {
			return FFWERR_FFW_ARG;
		}
	} else if (eAccessSize == MLWORD_ACCESS) {
		if ((dwSearchDataSize % 4) != 0) {
			return FFWERR_FFW_ARG;
		}
	}

	// 検索データサイズが開始 ～ 終了アドレスの領域サイズを超えるかをチェック
	if (dwSearchDataSize > dwmadrEndAddr - dwmadrStartAddr + 1) {
		return FFWERR_FFW_ARG;
	}

	// 検索方式に該当する検索データサイズになっているかをチェック
	switch (eSearchPatern) {
	case PATERN_SEARCH:		// 1 アドレス単位で検索
		if ((dwSearchDataSize < MSER_DSIZE_MIN) || (dwSearchDataSize > MSER_DSIZE_MAX)) {
			// 検索データサイズが設定可能な値でない場合
			return FFWERR_FFW_ARG;
		}
		break;

	case DATASIZE_SEARCH:	// 検索データサイズ単位で検索
		if ((dwSearchDataSize != 1) && (dwSearchDataSize != 2) && 
			(dwSearchDataSize != 4) && (dwSearchDataSize != 8)) {
			// 検索データサイズが1, 2, 4, 8 のいずれかでない場合
			return FFWERR_FFW_ARG;
		}
		break;

	default:		// ここは通らない
		break;
	}

	// ユーザプログラム実行中エラー確認
	if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
		// ユーザプログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	// RevRxE2LNo141104-001 Delete:MCUステータス確認処理削除(ターゲット/MCUステータス確認はBFWで実施する)。

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// メモリアクセスが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_MSER(dwmadrStartAddr, dwmadrEndAddr, eSearchEqual, 
				eSerchForward, eSearchPatern, eAccessSize, dwSearchDataSize, 
				pbySearchDataBuff, pbySearchMaskBuff, pSearchAns);

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWCmdのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * 命令コードの並びでダンプ
 * @param dwmadrStartAddr 開始アドレス
 * @param dwmadrEndAddr   終了アドレス
 * @param pbyReadBuff     リードデータ格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_CDUMP(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, const BYTE* pbyReadBuff)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	DWORD	dwTotalLength;
	BYTE*	pbyRetBuff;	// RevRxE2LNo141104-001 Append Line

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	pMcuDef = GetMcuDefData();

	// 引数チェック
	if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (dwmadrEndAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}

	if (dwmadrStartAddr > dwmadrEndAddr) {	// 開始アドレスが終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}

	dwTotalLength = dwmadrEndAddr - dwmadrStartAddr + 1;	// リードバイト数算出

	// RevRxE2LNo141104-001 Delete:MCUステータス確認処理削除(ターゲット/MCUステータス確認はBFWで実施する)。

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start
	// メモリアクセスが発生するFFW I/Fの先頭で、SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		// RevRxE2LNo141104-001 Append Start
		pbyRetBuff = const_cast<BYTE*>(pbyReadBuff);
		memset(pbyRetBuff, 0xFF, dwTotalLength);
		// RevRxE2LNo141104-001 Append End
		return ferr;
	}

	// V.1.02 RevNo110308-002 Append Line
	ClrMemAccWarningFlg();		// FFWERR_READ_DTF_MCURUN/FFWERR_READ_ROM_FLASHDBG_MCURUNワーニング記録変数初期化

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_CDUMP(dwmadrStartAddr, dwmadrEndAddr, pbyReadBuff);	// DUMP処理実行

	if (ferr != FFWERR_OK) {
		// V.1.02 覚え書き23の2点目(CDUMPでエラーが出た場合のmemcpy処理間違い修正) Append&Modify Start
		pbyRetBuff = const_cast<BYTE*>(pbyReadBuff);
		memset(pbyRetBuff, 0xFF, dwTotalLength);
		// V.1.02 覚え書き23の2点目(CDUMPでエラーが出た場合のmemcpy処理間違い修正) Append&Modify End
		return ferr;
	}
	// V.1.02 RevNo110308-002 Append Line
	ferr = WarningChk_MemAccessErr();				// メモリアクセス時のWarning確認

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}

//=============================================================================
/**
 * 命令コードの並びで書き込み
 * @param dwmadrStartAddr 開始アドレス
 * @param dwmadrEndAddr   終了アドレス
 * @param eVerify         ベリファイ有無
 * @param pbyWriteBuff    書き込みデータ格納バッファアドレス
 * @param pVerifyErr      ベリファイ結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWMCUCmd_CWRITE(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify,
																		const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	// RevRxNo120910-006 Append Start
	FFW_SRM_DATA* pSrmData;
	MADDR	dwmadrWorkRamStartAddr;				// ワーク・プログラム スタートアドレス
	MADDR	dwmadrWorkRamEndAddr;				// ワーク・プログラム エンドアドレス
	// RevRxNo120910-006 Append end

	SetIdcodeChange(FALSE);				// IDコード書き換えフラグ初期化	RevRxNo140515-011 Append Line
	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();
	ClrMemWaitInsertWarningFlg();	// RevRxNo140515-005 Append Line

	pMcuDef = GetMcuDefData();

	// 引数チェック
	if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (dwmadrEndAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}

	if (dwmadrStartAddr > dwmadrEndAddr) {	// 開始アドレスが終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}

	// RevRxNo120910-006 Append Start	SRM高速化対応(引数エラー処理追加)
	pSrmData = GetSrmData();
	// Startルーチン有効、またはStopルーチン有効時
	if(((pSrmData->dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((pSrmData->dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)){
		// RevRxNo120910-006 Modify Start
		dwmadrWorkRamStartAddr = pSrmData->dwmadrRamWorkAreaStartAddr;
		dwmadrWorkRamEndAddr   = dwmadrWorkRamStartAddr + SRM_WORKRAM_LENGTH - 0x00000001;
		// RevRxNo120910-006 Modify End
		if (!((dwmadrEndAddr < dwmadrWorkRamStartAddr) || (dwmadrWorkRamEndAddr < dwmadrStartAddr))) {
			return FFWERR_WRITE_SRM_WORKRAM;	// エラー：指定ルーチン実行機能用ワークRAM領域への書き込みは実行できない
		}
	}
	// RevRxNo120910-006 Append End

	// ベリファイエラー内容初期化
	pVerifyErr->eErrorFlag = VERIFY_OK;
	pVerifyErr->eAccessSize = MBYTE_ACCESS;
	pVerifyErr->dwErrorAddr = 0;
	pVerifyErr->dwErrorWriteData = 0;
	pVerifyErr->dwErrorReadData = 0;

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped Start

	// RevRxNo121122-001 Delete 
	// 内蔵ROM無効拡張モードフラグをここで行うと、実行中のROM領域へのアクセスで「FFWERR_WRITE_ROMAREA_MCURUNエラー」
	// より先に「FFWERR_BMCU_STANBY(低消費電力モード)エラー」が出るためErrorChk_CWrite()内に移動

	// V.1.02 新デバイス対応 Append Start
	ferr = ErrorChk_CWrite(dwmadrStartAddr, dwmadrEndAddr, pbyWriteBuff);	// MCUに依存するエラーチェック
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.02 新デバイス対応 Append End

	// RevRxE2LNo141104-001 Delete:MCUステータス確認処理削除(ターゲット/MCUステータス確認はBFWで実施する)。

	// V.1.01 No.17 RAMへのダウンロード高速化 Apped End
	ferr = DO_CWRITE(dwmadrStartAddr, dwmadrEndAddr, eVerify, pbyWriteBuff, pVerifyErr);	// 処理実行

	// メモリライトが発生するFFW I/F発行時、メモリライト処理後に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグをTRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// RevRxNo140515-005 Append Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_MemWaitInsertErr();		// メモリウェイト挿入Warning確認
	}
	// RevRxNo140515-005 Append End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;
}
// 2008.11.18 INSERT_END_E20RX600 }

//=============================================================================
/**
 * メモリ操作コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Mem(void)
{
	s_bWriteErrFlg = FALSE;		// WRITE処理中にエラーは発生しなかった	// RevRxNo140109-001 Append Line

	return;
}

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * WRITE処理実行時のエラー有無フラグクリア
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrWriteErrFlg(void)
{
	s_bWriteErrFlg = FALSE;		// WRITE処理中にエラーは発生しなかった
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * WRITE処理実行時のエラー有無フラグ値取得
 * @param なし
 * @retval TRUE :WRITE処理中にエラーが発生した
 * @retval FALSE:WRITE処理中にエラーは発生しなかった
 */
//=============================================================================
BOOL GetWriteErrFlg(void)
{
	return s_bWriteErrFlg;
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * WRITEコマンドのパラメータチェック
 * @param dwmadrStartAddr 開始アドレス
 * @param dwmadrEndAddr 終了アドレス
 * @param eAccessSize アクセスサイズ
 * @param pbyWriteBuff 書き込みデータ格納バッファアドレス
 * @param pVerifyErr ベリファイ結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR chkWriteCmdPara(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr,
							  enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff)
{
	FFWERR					ferr = FFWERR_OK;
	FFWMCU_MCUDEF_DATA* 	pMcuDef;
	// RevNo121017-002 Append Start
	DWORD					dwLength;	
	// RevRxNo120910-006 Append Start
	FFW_SRM_DATA* 			pSrmData;
	MADDR					dwmadrWorkRamStartAddr;				// ワーク・プログラム スタートアドレス
	MADDR					dwmadrWorkRamEndAddr;				// ワーク・プログラム エンドアドレス
	// RevRxNo120910-006 Append end

	pMcuDef = GetMcuDefData();

	// 引数チェック
	if (dwmadrStartAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (dwmadrEndAddr > pMcuDef->madrMaxAddr) {
		return FFWERR_FFW_ARG;
	}
	if (dwmadrStartAddr > dwmadrEndAddr) {	// 開始アドレスが終了アドレスよりも大きい場合
		return FFWERR_FFW_ARG;
	}
	// RevNo121017-002 Append Start
	dwLength = dwmadrEndAddr - dwmadrStartAddr + 1;
	// 開始アドレスから終了アドレスまでのレングスがWRITE_LENGTH_MAX(128MB)を超える場合
	if ((dwLength > WRITE_LENGTH_MAX) || (dwLength == 0)) {
		return FFWERR_FFW_ARG;
	}
	// RevNo121017-002 Append End

	// アクセスサイズのエラーチェック
	if ((ferr = ErrorChk_McuAccessSize(eAccessSize)) != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo120910-006 Append Start
	pSrmData = GetSrmData();
	// Startルーチン有効、またはStopルーチン有効時
	if(((pSrmData->dwSrMode & SRM_START_FUNC_BIT) == SRM_START_FUNC_BIT) || ((pSrmData->dwSrMode & SRM_STOP_FUNC_BIT) == SRM_STOP_FUNC_BIT)){
		// RevRxNo120910-006 Modify Start
		dwmadrWorkRamStartAddr = pSrmData->dwmadrRamWorkAreaStartAddr;
		dwmadrWorkRamEndAddr   = dwmadrWorkRamStartAddr + SRM_WORKRAM_LENGTH - 0x00000001;
		// RevRxNo120910-006 Modify End
		if (!((dwmadrEndAddr < dwmadrWorkRamStartAddr) || (dwmadrWorkRamEndAddr < dwmadrStartAddr))) {	// 本体部の判定。
			return FFWERR_WRITE_SRM_WORKRAM;	// エラー：指定ルーチン実行機能用ワークRAM領域への書き込みは実行できない
		}
	}
	// RevRxNo120910-006 Append End

// V.1.01 No.17 RAMへのダウンロード高速化対応 Append Start
// Ver.1.01 2010/08/17 SDS T.Iwata

	// DWNP実行中である場合
	if ( GetDwnpOpenData() == TRUE ){
		// V.1.03 RevNo111121-008 Modify Start
		// ダウンロード時のチェックを以下にまとめる
		ferr = ErrorChk_Dwnp(dwmadrStartAddr, dwmadrEndAddr, eAccessSize, pbyWriteBuff);
		if(ferr != FFWERR_OK){
			return ferr;
		}
		// V.1.03 RevNo111121-008 Modify End
	}
	// DWNP実行中でない場合
	else{
		// RevRxNo121122-001 Delete 
		// 内蔵ROM無効拡張モードフラグをここで行うと、実行中のROM領域へのアクセスで「FFWERR_WRITE_ROMAREA_MCURUNエラー」
		// より先に「FFWERR_BMCU_STANBY(低消費電力モード)エラー」が出るためErrorChk_WRITE()内に移動

		// V.1.02 新デバイス対応 Modify Line
		ferr = ErrorChk_Write(dwmadrStartAddr, dwmadrEndAddr, eAccessSize, pbyWriteBuff);	// MCUに依存するエラーチェック
		if (ferr != FFWERR_OK) {
			return ferr;
		}

		// RevRxE2LNo141104-001 Delete:MCUステータス確認処理削除(ターゲット/MCUステータス確認はBFWで実施する)。
	}
// V.1.01 No.17 RAMへのダウンロード高速化対応 Append End

	return ferr;
}
// RevRxNo140109-001 Append End
