///////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_mem.cpp
 * @brief MCU メモリ制御関数
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, S.Ueda, K.Uemori, SDS T.Iwata, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/10/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo111121-003   2012/07/11 橋口
  ・chkVerifyFillData(), chkVerifyWriteData()
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
  ・chkVerifyCwriteData()
    ・新規追加
  ・cwriteDmaAccess()
    ・ベリファイチェック時に使用する関数をchkVerifyWriteData() → 
	  chkVerifyCwriteData() に変更。
・RevNo111121-007   2012/07/11 橋口
  ・dumpDmaAccess(), cdumpDmaAccess()
    ・FCU ファーム領域をリードする場合、以下を検出する処理を追加。
        FFWERR_READ_ROM_FLASHDBG_MCURUN
        FFWERR_ACC_ROM_PEMODE
	→反映ミス修正 2012/09/14
・RevNo120201-001	2012/07/11 橋口
　・MemFill(), MemWrite(), MemCodeWrite(), chkVerifyFillData(), 
　　chkVerifyWriteData(), chkVerifyCwriteData()
　　　ベリファイ結果格納変数初期化処理追加(未初期化メモリリードアクセスの修正)
・RevRxNo120606-004 2012/07/12 橋口
  ・MPU領域リード/ライト対応
・RevNo120727-001 2012/07/27 橋口
	内蔵ROM DUMP 高速化
・RevRxNo121022-001	2012/11/21 SDS 岩田
	EZ-CUBE PROT_MCU_DUMP()分割処理対応
	EZ-CUBE PROT_MCU_WRITE()分割処理対応
・RevRxNo121122-003	2012/11/27 上田
　・フラッシュ書き換えデバッグ中のFCUファーム領域リードデータ設定処理修正。
・RevRxNo121122-004	2012/11/27 上田
　・CWRITEベリファイ処理修正
・RevRxNo121022-001	2012/12/04 SDS 岩田
	・McuDumpDivide()関数のアクセス回数格納変数名をdwReadAccCountに変更
	・EZ-CUBE PROT_MCU_CPUDUMP分割処理対応
・RevRxNo130301-001 2013/08/02 植盛
	RX64M対応
・RevRxNo130730-003 2013/12/12 大喜多, 川上 
　　MemWrite()で、ダウンロード時以外の外部フラッシュ領域へのWRITE処理修正
・RevRxNo130730-009 2013/11/15 大喜多
　　mallocでメモリが確保できなかった場合にエラーを返す処理を追加
・RevRxNo130730-008 2013/11/15 大喜多
	エラーで返る前にメモリ解放処理を追加
・RevRxNo140109-001 2014/01/23 植盛
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo140616-001 2014/06/24 植盛
	MCUコマンド個別制御領域に対する内部処理実装
・RevRxNo140515-005 2014/12/12 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxE2LNo141104-001 2015/01/07 上田
	E2 Lite対応
・RevRxNo150528-001 2015/05/28 PA 紡車
    メモリウェイト仕様追加対応
・RevRxNo150827-001　2015/09/01 PA 紡車
	RX65x ROMウェイト機能対応
・RevRxNo160525-001 2016/06/24 PA 紡車
	RX24T HOCO 64MHz対応
・RevRxNo160711-001 2016/07/11 PA 紡車
　　メモリウェイト処理中のプロテクト復帰処理修正
・RevRxNo161003-001	2016/12/01 PA 辻
	データフラッシュメモリアクセス周波数設定レジスタ対応
・RevRxNo161003-001	2016/11/25 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
・RevRxNo180625-001,PB18047-CD01-001 2018/09/14 PA 紡車
	RX66T-H/RX72T対応
*/
#ifdef _WIN32
#include <windows.h>
#elif __linux__
#endif

#include "ffw_sys.h"
#include "mcu_mem.h"
#include "ffwmcu_brk.h"
#include "ffwmcu_mem.h"
#include "ffwmcu_dwn.h"
#include "domcu_mcu.h"
#include "domcu_prog.h"
#include "domcu_mem.h"							// RevNo111121-003 Append Line
#include "protmcu_mem.h"
#include "domcu_rst.h"

#include "protmcu_mcu.h"

#include "mcu_extflash.h"
#include "mcu_flash.h"
#include "mcu_brk.h"
#include "mcu_sfr.h"
#include "mcu_inram.h"	// RevRxE2LNo141104-001 Append Line
#include "mcu_extram.h"
#include "mcuspec.h"

#include "do_sys.h"		// RevRxNo121022-001 Append Line

// static変数
static enum FFWENM_VERIFY_SET	s_eVerify;		//ベリファイフラグ情報

// RevRxNo140109-001 Append Line
static BOOL		s_bExistConfigDwnDataFlg;	// コンフィギュレーション設定領域へのダウンロードデータ有無フラグ

// RevRxNo140515-005 Append Start
static DWORD s_dwMemwaitData;				// MEMWAITレジスタのユーザ設定値退避用
static BYTE	s_byOpccrData;					// OPCCRレジスタのユーザ設定値退避用
static BYTE s_bySopccrData;					// SOPCCRレジスタのユーザ設定値退避用
static BOOL s_bMemWaitChange;				// メモリウェイト設定変更有無フラグ(TRUE:MEMWAIT変更した、FALSE:MEMWAIT変更していない)
static BOOL s_bOPCModeChange;				// 動作電力制御モード変更有無フラグ(TRUE:OPCCR/SOPCCR変更した、FALSE:OPCCR/SOPCCR変更していない)
static BYTE s_byMemWaitInsert;				// メモリウェイト設定変更ワーニングフラグ(0x00:ウェイト挿入なし、0x01:ROMに挿入、0x02:RAMに挿入)
// RevRxNo140515-005 Append End
// RevRxNo150528-001 Append Line
static float s_fUserICLK;					// 現在のユーザICLK周波数値保存用

// RevRxNo161003-001 Append Start +2
static BYTE	s_byEepfclk;					// 現在のユーザEEPFCLK設定値保存用
static BOOL	s_bEepfclkChg;					// EEPFCLKレジスタを書き換えたかどうかを示す(TRUE:書き換えた、FALSE:書き換えていない)
// RevRxNo161003-001 Append End

// static関数の宣言

static FFWERR dumpDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff);
static FFWERR dumpCpuAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff);		//RevRxNo120606-004 Append Line
static FFWERR fillDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
							FFW_VERIFYERR_DATA* pVerifyErr);
//RevRxNo120606-004 Append Start
static FFWERR fillCpuAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
							FFW_VERIFYERR_DATA* pVerifyErr);
//RevRxNo120606-004 Append End
static FFWERR writeDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							 enum FFWENM_MACCESS_SIZE eAccessSize,  const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);
//RevRxNo120606-004 Append Start
static FFWERR writeCpuAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							 enum FFWENM_MACCESS_SIZE eAccessSize,  const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);
//RevRxNo120606-004 Append End
static FFWERR cdumpDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff);
static FFWERR cwriteDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							 enum FFWENM_MACCESS_SIZE eAccessSize,  const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);
static FFWERR chkVerifyFillData(MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, 
								const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr, DWORD dwWriteDataSize, BYTE byEndian);
// RevNo111121-003 Append Start
static FFWERR chkVerifyCwriteData(MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwLength, 
						const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);
// RevNo111121-003 Append End

// RevRxNo130301-001 Delete

///////////////////////////////////////////////////////////////////////////////
// メモリ操作関連関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ターゲットメモリのDUMP処理
 *   開始アドレス～終了アドレスは、アクセスサイズの倍数を指定する。
 *   開始アドレス、終了アドレス、アクセスサイズのチェックは、本関数呼び出し側で
 *   行うこと。
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eAccessSize アクセスサイズ
 * @param pbyReadBuff リードデータ格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR MemDump(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTotalLength;		//DUMP領域全体のバイトサイズ
	MADDR	madrStartOrgAddr;
	BOOL	bRamEnable = FALSE;
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];	// RevRxNo130301-001 Append Line

	// RevRxNo120606-004 Delete dwDmaStatは未使用なので削除
	//RevNo100715-027 Append Line
	BOOL	bRet;
	// RevRxNo120606-004 Append Start
	DWORD	dwLength;
	BYTE	byDumpAccMeans;		// RevRxNo140616-001 Modify Line
	BYTE	byDumpAccType;		// RevRxNo140616-001 Append Line
	MADDR	madrEndAddrTmp;
	// RevRxNo120606-004 Append End

	// RevRxNo140515-005 Append Line
	BYTE	byMemWaitInsert;

	madrStartOrgAddr = madrStartAddr;

	dwTotalLength = madrEndAddr - madrStartAddr + 1;	// リードバイト数算出

	// RevRxNo140515-005 Append Start
	ferr = ChkMemWaitInsert(madrStartAddr, madrEndAddr, &byMemWaitInsert);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
		s_bMemWaitChange = FALSE;
		s_bOPCModeChange = FALSE;
		ferr = SetMemWait(TRUE);				// メモリウェイト挿入する
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-005 Append End

	while (dwTotalLength != 0) {
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete
		// データフラッシュ読み出し許可設定処理削除
	
		// RevRxNo140616-001 Modify Start
		// RevRxNo120606-004 Append Start
		// DUMPアクセス領域のアクセス方法判定
		ChkAreaAccessMethod(madrStartAddr, madrEndAddr, &dwLength, &byDumpAccMeans, &byDumpAccType);

		if ((byDumpAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byDumpAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_R))) {
			// アクセス属性がリード/ライト可またはリードのみ可の場合

			// DUMPアクセス終了アドレス算出
			madrEndAddrTmp = madrStartAddr+dwLength-1;
			// RevRxNo120606-004 Append End

			// RevRxNo130301-001 Modify Line
			// 内蔵RAMの有効/無効設定		// RevRxNo120606-004 Modify Line
			bRamEnable = ChkRamArea(madrStartAddr, madrEndAddrTmp, &bRamBlockEna[0]);

			//RevNo100715-027 Append Line
			//メモリ操作途中でブレークした場合、有効/無効が対で呼ばれない場合があるので、有効前の状況で判断
			bRet = GetMcuRunState();
			if (bRamEnable == TRUE) {
				// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
				//RevNo100715-027 Modify Line
				if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
					ferr = SetRegRamEnable(TRUE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}

			// RevRxNo120606-004 Modify Start
			if (byDumpAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_DDMA)) {		// DMA使用時
				ferr = dumpDmaAccess(madrStartAddr, madrEndAddrTmp, eAccessSize, pbyBuff);
			} else if (byDumpAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_CPU)) {	// CPUアクセス時
				ferr = dumpCpuAccess(madrStartAddr, madrEndAddrTmp, eAccessSize, pbyBuff);
				// RevRxNo120606-004 Modify End
			} else {															// DMA未使用時でプログラム実行中
			}
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete
			// データフラッシュ読み出し許可設定解除処理削除

			// 内蔵RAMの有効/無効設定を元に戻す
			if (bRamEnable == TRUE) {
				//RevNo100715-027 Modify Line
				// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
				if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
					ferr = SetRegRamEnable(FALSE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}

		} else {
			// アクセス属性がライトのみ可またはアクセス不可の場合

			// DUMP不可領域の格納バッファに0x00を設定
			memset(pbyBuff, 0, dwLength);
		}
		// RevRxNo140616-001 Modify End

		// RevRxNo120606-004 Append Start
		// DUMPバイト数更新
		dwTotalLength -= dwLength;
		// DUMP開始アドレス更新
		madrStartAddr += dwLength;
		if(dwTotalLength != 0){
			// DUMPバッファ位置更新
			pbyBuff += dwLength;
		}
		// RevRxNo120606-004 Append End
	}

	// RevRxNo140515-005 Append Start
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更が必要だった場合
		ferr = SetMemWait(FALSE);				// ユーザ設定値に戻す
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		s_byMemWaitInsert = byMemWaitInsert;	// static変数に結果を格納しておく(0x02だったのが0x01に上書きされることはあるが、どちらかのワーニングが出ればいい)
	}
	// RevRxNo140515-005 Append End

	return ferr;
}

//=============================================================================
/**
 * ターゲットメモリのFILL処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param dwWriteDataSize 書き込みデータサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR MemFill(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
			   enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
			   FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTotalLength;				// FILL領域全体のバイトサイズ
	BOOL	bRamEnable = FALSE;
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];	// RevRxNo130301-001 Append Line

	// RevRxNo120606-004 Delete dwDmaStatは未使用なので削除
	//RevNo100715-027 Append Line
	BOOL	bRet;
	// RevRxNo120606-004 Append Start
	DWORD	dwLength;
	BYTE	byFillAccMeans;		// RevRxNo140616-001 Modify Line
	BYTE	byFillAccType;		// RevRxNo140616-001 Append Line
	MADDR	madrEndAddrTmp;
	// RevRxNo120606-004 Append End

	// RevRxNo140515-005 Append Line
	BYTE	byMemWaitInsert;

	pVerifyErr->eErrorFlag = VERIFY_OK;	// RevNo120201-001 Append Line

	dwTotalLength = madrEndAddr - madrStartAddr + 1;	// ライトバイト数算出

	// RevRxNo140515-005 Append Start
	ferr = ChkMemWaitInsert(madrStartAddr, madrEndAddr, &byMemWaitInsert);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
		s_bMemWaitChange = FALSE;
		s_bOPCModeChange = FALSE;
		ferr = SetMemWait(TRUE);				// メモリウェイト挿入する
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-005 Append End

	while (dwTotalLength != 0) {
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete
		// データフラッシュ読み出し許可設定処理削除

		// RevRxNo140616-001 Modify Start
		// RevRxNo120606-004 Append Start
		// FILLアクセス領域のアクセス方法判定
		ChkAreaAccessMethod(madrStartAddr, madrEndAddr, &dwLength, &byFillAccMeans, &byFillAccType);

		if ((byFillAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byFillAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
			// アクセス属性がリード/ライト可またはライトのみ可の場合

			// FILLアクセス終了アドレス算出
			madrEndAddrTmp = madrStartAddr+dwLength-1;
			// RevRxNo120606-004 Append End

			// RevRxNo130301-001 Modify Line
			// 内蔵RAMの有効/無効設定
			bRamEnable = ChkRamArea(madrStartAddr, madrEndAddrTmp, &bRamBlockEna[0]);		// RevRxNo120606-004 Modify Start
			//RevNo100715-027 Append Line
			//メモリ操作途中でブレークした場合、有効/無効が対で呼ばれない場合があるので、有効前の状況で判断
			bRet = GetMcuRunState();
			if (bRamEnable == TRUE) {
				//RevNo100715-027 Modify Line
				// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
				if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
					ferr = SetRegRamEnable(TRUE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}

			// RevRxNo120606-004 Modify Start
			if (byFillAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_DDMA)) {		// DMA使用時
				ferr = fillDmaAccess(madrStartAddr, madrEndAddrTmp, eVerify, eAccessSize, dwWriteDataSize, pbyWriteBuff, pVerifyErr);
			} else if (byFillAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_CPU)) {	// CPUアクセス時
				ferr = fillCpuAccess(madrStartAddr, madrEndAddrTmp, eVerify, eAccessSize, dwWriteDataSize, pbyWriteBuff, pVerifyErr);
				// RevRxNo120606-004 Modify END
			} else {																// DMA未使用時でプログラム実行中
			}
			if (ferr != FFWERR_OK) {
				// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
				// 内蔵フラッシュROMメモリライト終了処理削除
				return ferr;
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete
			// データフラッシュ読み出し許可設定解除処理削除

			// 内蔵RAMの有効/無効設定を元に戻す
			if (bRamEnable == TRUE) {
				//RevNo100715-027 Modify Line
				// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
				if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
					ferr = SetRegRamEnable(FALSE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}
		}
		// RevRxNo140616-001 Modify End

		// RevRxNo120606-004 Append Start
		dwTotalLength -= dwLength;
		madrStartAddr += dwLength;
		// RevRxNo120606-004 Append End
	}

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
	// 内蔵フラッシュROMメモリライト終了処理削除

	// RevRxNo140515-005 Append Start
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更が必要だった場合
		ferr = SetMemWait(FALSE);				// ユーザ設定値に戻す
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		s_byMemWaitInsert = byMemWaitInsert;	// static変数に結果を格納しておく(0x02だったのが0x01に上書きされることはあるが、どちらかのワーニングが出ればいい)
	}
	// RevRxNo140515-005 Append End

	return ferr;
}

//=============================================================================
/**
 * ターゲットメモリのライト処理
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
FFWERR MemWrite(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
				enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTotalLength;				//ライトエリア全体のバイトサイズ
	const BYTE*	pbyBuff;
	BOOL	bRamEnable = FALSE;
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];	// RevRxNo130301-001 Append Line

	//RevNo100715-027 Append Line
	BOOL	bRet;

	DWORD	dwLength;
	// V.1.02 RevNo110509-003 Append Line
	DWORD	dwLen;
	MADDR	madrAreaEnd;
	BOOL	bExtFlashRomArea;
	BYTE	byWriteAccMeans;			// RevRxNo120606-004 Modify Start, RevRxNo140616-001 Modify Line
	BYTE	byWriteAccType;				// RevRxNo140616-001 Append Line

	// RevRxNo140515-005 Append Line
	BYTE	byMemWaitInsert;

	pVerifyErr->eErrorFlag = VERIFY_OK;	// RevNo120201-001 Append Line

	pbyBuff = pbyWriteBuff;

	dwTotalLength = madrEndAddr - madrStartAddr + 1;	// ライトバイト数算出

	if (dwTotalLength != 0) {
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
		// 旧キャッシュメモリデータ更新処理削除
		// データフラッシュ読み出し許可設定解除処理削除


		// RevRxNo130301-001 Modify Line
		// 内蔵RAMの有効/無効設定
		bRamEnable = ChkRamArea(madrStartAddr, madrEndAddr, &bRamBlockEna[0]);
		//RevNo100715-027 Append Line
		//メモリ操作途中でブレークした場合、有効/無効が対で呼ばれない場合があるので、有効前の状況で判断
		bRet = GetMcuRunState();
		if (bRamEnable == TRUE) {
			// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
			//RevNo100715-027 Modify Line
			if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
				ferr = SetRegRamEnable(TRUE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}

		// RevRxNo140515-005 Append Start
		ferr = ChkMemWaitInsert(madrStartAddr, madrEndAddr, &byMemWaitInsert);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
			s_bMemWaitChange = FALSE;
			s_bOPCModeChange = FALSE;
			ferr = SetMemWait(TRUE);				// メモリウェイト挿入する
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
		// RevRxNo140515-005 Append End

		do {	// 00000000～FFFFFFFFh バイトの場合、dwTotalLength が 0 なので、do 文を使用する。
			// 外部フラッシュROM領域かチェック
			CheckExtFlashRomArea(madrStartAddr, madrEndAddr, &madrAreaEnd, &bExtFlashRomArea);
			dwLength = madrAreaEnd - madrStartAddr + 1;

			// RevRxNo130730-003 Modify Line
			if ((GetDwnpOpenData() == TRUE) && (bExtFlashRomArea == TRUE)) {			// ダウンロード中、かつ、外部フラッシュROM領域の場合
				// V.1.02 RevNo110509-003 Modify Line
				ferr = ExtMemWrite(madrStartAddr, madrAreaEnd, eVerify, eAccessSize, pbyBuff, pVerifyErr, &dwLen);
				if (ferr != FFWERR_OK) {
					// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
					// 内蔵フラッシュROMメモリライト終了処理削除
					return ferr;
				}
			} else {										// 外部フラッシュROM領域以外の場合
				// RevRxNo140616-001 Modify Start
				// RevRxNo120606-004 Modify Start
				// WRITEする領域のアクセス方法判定
				ChkAreaAccessMethod(madrStartAddr, madrAreaEnd, &dwLength, &byWriteAccMeans, &byWriteAccType);

				if ((byWriteAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byWriteAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
					// アクセス属性がリード/ライト可またはライトのみ可の場合

					// WRITEアクセス終了アドレス算出
					madrAreaEnd = madrStartAddr+dwLength-1;

					if(byWriteAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_DDMA)) {
						ferr = writeDmaAccess(madrStartAddr, madrAreaEnd, eVerify, eAccessSize, pbyBuff, pVerifyErr);
					} else if(byWriteAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_CPU)) {
						ferr = writeCpuAccess(madrStartAddr, madrAreaEnd, eVerify, eAccessSize, pbyBuff, pVerifyErr);
					}
					// RevRxNo120606-004 Modify End
					if (ferr != FFWERR_OK) {
						// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
						// 内蔵フラッシュROMメモリライト終了処理削除
						return ferr;
					}
				}
			}
			// RevRxNo140616-001 Modify End

			dwTotalLength -= dwLength;
			madrStartAddr += dwLength;
			pbyBuff += dwLength;
		} while (dwTotalLength);	// dwTotalLength がオーバーフローしていても、dwLength をすでに引いているので問題なし。

		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
		// データフラッシュ読み出し許可設定解除処理削除


		// 内蔵RAMの有効/無効設定を元に戻す
		if (bRamEnable == TRUE) {
			// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
			//RevNo100715-027 Modify Line
			if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
				ferr = SetRegRamEnable(FALSE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}

		// RevRxNo140515-005 Append Start
		if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更が必要だった場合
			ferr = SetMemWait(FALSE);				// ユーザ設定値に戻す
			if( ferr != FFWERR_OK ){
				return ferr;
			}
			s_byMemWaitInsert = byMemWaitInsert;	// static変数に結果を格納しておく(0x02だったのが0x01に上書きされることはあるが、どちらかのワーニングが出ればいい)
		}
		// RevRxNo140515-005 Append End
	}

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
	// 内蔵フラッシュROMメモリライト終了処理削除


	return ferr;
}

//=============================================================================
/**
 * 命令コード並びのDUMP処理
 *   開始アドレス～終了アドレスは、アクセスサイズの倍数を指定する。
 *   開始アドレス、終了アドレス、アクセスサイズのチェックは、本関数呼び出し側で
 *   行うこと。
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eAccessSize アクセスサイズ
 * @param pbyReadBuff リードデータ格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR MemCodeDump(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTotalLength;				//DUMP全体のバイトサイズ
	MADDR	madrStartOrgAddr;
	DWORD	dwDmaStat = TRUE;			// DMA使用状態
	BOOL	bRamEnable = FALSE;
	//RevNo100715-027 Append Line
	BOOL	bRet;
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];	// RevRxNo130301-001 Append Line
	// RevRxNo140616-001 Append Start
	DWORD	dwLength;
	MADDR	madrEndAddrTmp;
	BYTE	byCdumpAccMeans;
	BYTE	byCdumpAccType;
	// RevRxNo140616-001 Append End

	// RevRxNo140515-005 Append Line
	BYTE	byMemWaitInsert;

	madrStartOrgAddr = madrStartAddr;

	dwTotalLength = madrEndAddr - madrStartAddr + 1;	// リードバイト数算出

	// RevRxNo140515-005 Append Start
	byMemWaitInsert = NO_MEM_WAIT_CHG;
	if (GetStepContinueStartFlg() == FALSE) {	// 連続ステップ中でない場合
		ferr = ChkMemWaitInsert(madrStartAddr, madrEndAddr, &byMemWaitInsert);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
		s_bMemWaitChange = FALSE;
		s_bOPCModeChange = FALSE;
		ferr = SetMemWait(TRUE);				// メモリウェイト挿入する
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-005 Append End

	// RevRxNo140616-001 Modify Start
	while (dwTotalLength != 0) {
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
		// データフラッシュ読み出し許可設定処理削除

		// CDUMPアクセス領域のアクセス方法判定
		ChkAreaAccessMethod(madrStartAddr, madrEndAddr, &dwLength, &byCdumpAccMeans, &byCdumpAccType);

		if ((byCdumpAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byCdumpAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_R))) {
			// アクセス属性がリード/ライト可またはリードのみ可の場合

			// CDUMPアクセス終了アドレス算出
			madrEndAddrTmp = madrStartAddr+dwLength-1;

			// RevRxNo130301-001 Modify Line
			// 内蔵RAMの有効/無効設定
			bRamEnable = ChkRamArea(madrStartAddr, madrEndAddrTmp, &bRamBlockEna[0]);
			//RevNo100715-027 Append Line
			//メモリ操作途中でブレークした場合、有効/無効が対で呼ばれない場合があるので、有効前の状況で判断
			bRet = GetMcuRunState();
			if (bRamEnable == TRUE) {
				// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
				//RevNo100715-027 Modify Line
				if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
					ferr = SetRegRamEnable(TRUE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}

			if (dwDmaStat == TRUE) {															// DMA使用時
				ferr = cdumpDmaAccess(madrStartAddr, madrEndAddrTmp, eAccessSize, pbyBuff);
			} else if ((dwDmaStat == FALSE) && (IsMcuRun() == FALSE)) {							// DMA未使用時でプログラム停止中
			} else {																			// DMA未使用時でプログラム実行中
			}
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
			// データフラッシュ読み出し許可設定解除処理削除

			// 内蔵RAMの有効/無効設定を元に戻す
			if (bRamEnable == TRUE) {
				// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
				//RevNo100715-027 Modify Line
				if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
					ferr = SetRegRamEnable(FALSE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}
		} else {
			// アクセス属性がライトのみ可またはアクセス不可の場合

			// DUMP不可領域の格納バッファに0x00を設定
			memset(pbyBuff, 0, dwLength);
		}

		// DUMPバイト数更新
		dwTotalLength -= dwLength;
		// DUMP開始アドレス更新
		madrStartAddr += dwLength;
		if(dwTotalLength != 0){
			// DUMPバッファ位置更新
			pbyBuff += dwLength;
		}
	}
	// RevRxNo140616-001 Modify End

	// RevRxNo140515-005 Append Start
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更が必要だった場合
		ferr = SetMemWait(FALSE);				// ユーザ設定値に戻す
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		s_byMemWaitInsert = byMemWaitInsert;	// static変数に結果を格納しておく(0x02だったのが0x01に上書きされることはあるが、どちらかのワーニングが出ればいい)
	}
	// RevRxNo140515-005 Append End

	return ferr;
}

//=============================================================================
/**
 * 命令コード並びのライト処理
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
FFWERR MemCodeWrite(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
				enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwTotalLength;				// WRITE全体のバイトサイズ
	MADDR	madrStartOrgAddr;
	const BYTE*	pbyBuff;
	DWORD	dwDmaStat = TRUE;			// DMA使用状態
	BOOL	bRamEnable = FALSE;
	//RevNo100715-027 Append Line
	BOOL	bRet;
	BOOL	bRamBlockEna[MCU_AREANUM_MAX_RX];	// RevRxNo130301-001 Append Line

	// RevRxNo140616-001 Append Start
	DWORD	dwLength;
	BYTE	byCwriteAccMeans;
	BYTE	byCwriteAccType;
	MADDR	madrEndAddrTmp;
	// RevRxNo140616-001 Append End

	// RevRxNo140515-005 Append Line
	BYTE	byMemWaitInsert;

	pVerifyErr->eErrorFlag = VERIFY_OK;	// RevNo120201-001 Append Line

	madrStartOrgAddr = madrStartAddr;
	pbyBuff = pbyWriteBuff;

	dwTotalLength = madrEndAddr - madrStartAddr + 1;	// ライトバイト数算出

	// RevRxNo140515-005 Append Start
	byMemWaitInsert = NO_MEM_WAIT_CHG;
	if (GetStepContinueStartFlg() == FALSE) {	// 連続ステップ中でない場合
		ferr = ChkMemWaitInsert(madrStartAddr, madrEndAddr, &byMemWaitInsert);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更必要な場合
		s_bMemWaitChange = FALSE;
		s_bOPCModeChange = FALSE;
		ferr = SetMemWait(TRUE);				// メモリウェイト挿入する
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo140515-005 Append End

	// RevRxNo140616-001 Modify Start
	while (dwTotalLength != 0) {
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
		// データフラッシュ読み出し許可設定処理削除

		// CWRITEアクセス領域のアクセス方法判定
		ChkAreaAccessMethod(madrStartAddr, madrEndAddr, &dwLength, &byCwriteAccMeans, &byCwriteAccType);

		if ((byCwriteAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_RW)) || (byCwriteAccType == static_cast<BYTE>(RX_EMU_ACCTYPE_W))) {
			// アクセス属性がリード/ライト可またはライトのみ可の場合

			// CWRITEアクセス終了アドレス算出
			madrEndAddrTmp = madrStartAddr+dwLength-1;

			// RevRxNo130301-001 Modify Line
			// 内蔵RAMの有効/無効設定
			bRamEnable = ChkRamArea(madrStartAddr, madrEndAddrTmp, &bRamBlockEna[0]);
			//RevNo100715-027 Append Line
			//メモリ操作途中でブレークした場合、有効/無効が対で呼ばれない場合があるので、有効前の状況で判断
			bRet = GetMcuRunState();
			if (bRamEnable == TRUE) {
				// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
				//RevNo100715-027 Modify Line
				if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
					ferr = SetRegRamEnable(TRUE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}

			if (dwDmaStat == TRUE) {															// DMA使用時
				ferr = cwriteDmaAccess(madrStartAddr, madrEndAddrTmp, eVerify, eAccessSize, pbyBuff, pVerifyErr);
			} else if ((dwDmaStat == FALSE) && (IsMcuRun() == FALSE)) {							// DMA未使用時でプログラム停止中
			} else {																			// DMA未使用時でプログラム実行中
			}
			if (ferr != FFWERR_OK) {
				// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
				// 内蔵フラッシュROMメモリライト終了処理削除
				return ferr;
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
			// データフラッシュ読み出し許可設定解除処理削除

			// 内蔵RAMの有効/無効設定を元に戻す
			if (bRamEnable == TRUE) {
				// RAM領域でダウンロード時かS/Wブレークポイント埋め込み/埋め戻しでプログラム停止時
				//RevNo100715-027 Modify Line
				if ((GetDwnpOpenData() == TRUE) || ((GetCmdRetFillFlg() == TRUE) && (bRet == FALSE))) {
					ferr = SetRegRamEnable(FALSE, &bRamBlockEna[0]);		// RevRxNo130301-001 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Delete Line
			// 内蔵フラッシュROMメモリライト終了処理削除
		}
		dwTotalLength -= dwLength;
		madrStartAddr += dwLength;
		pbyBuff += dwLength;
	}
	// RevRxNo140616-001 Modify Start

	// RevRxNo140515-005 Append Start
	if (byMemWaitInsert != NO_MEM_WAIT_CHG) {	// メモリウェイト設定変更が必要だった場合
		ferr = SetMemWait(FALSE);				// ユーザ設定値に戻す
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		s_byMemWaitInsert = byMemWaitInsert;	// static変数に結果を格納しておく(0x02だったのが0x01に上書きされることはあるが、どちらかのワーニングが出ればいい)
	}
	// RevRxNo140515-005 Append End

	return ferr;
}

//=============================================================================
/**
 * ベリファイ有効/無効状態登録
 * @param eVerify	:VERIFY_ON ベリファイ有効　VERIFY_OFF ベリファイ無効
 * @retval なし
 */
//=============================================================================
void SetVerifySet(enum FFWENM_VERIFY_SET eVerify)
{
	s_eVerify = eVerify;
	return;
}

//=============================================================================
/**
 * ベリファイ有効/無効状態取得
 * @param なし
 * @retval VERIFY_ON ベリファイ有効　VERIFY_OFF ベリファイ無効
 */
//=============================================================================
enum FFWENM_VERIFY_SET GetVerifySet(void)
{
	return s_eVerify;
}

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * デバッグDMA機能によるDUMP処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eAccessSize アクセスサイズ
 * @param pbyBuff DUMPデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR dumpDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	DWORD	dwAddAddr;
	DWORD	dwLength;			//メモリアクセス時のバイト長
	DWORD	dwAccessCount;		//メモリアクセス時のアクセスサイズのアクセス回数
	BYTE*	pbyReadBuf;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	DWORD	dwTotalLength;		// メモリアクセス全体のバイトサイズ
	DWORD	dwTotalAccessCount;	// メモリアクセス全体のアクセス回数
	BYTE	byEndian;
	FFW_PB_DATA* pPb;
	DWORD	i;
	DWORD	dwCnt = 0;
	DWORD	dwLoopAddr;
	BYTE	byCmdData;
	BYTE	byDbtCmd = 0x01;	// DBT
	BOOL	bEnableArea = FALSE;
	BOOL	bInRomArea = FALSE;		// RevRxNo140109-001 Append Line
	enum FFWENM_MACCESS_SIZE eAccessSizePb = MBYTE_ACCESS;
	DWORD	dwPbAccessCount = 1;
	DWORD	dwStartOffsetAddr;

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	FFWMCU_DBG_DATA_RX*	pDbgData;
	enum FFWRXENM_MAREA_TYPE eAreaType;
	MADDR madrBlkStartAddr;
	MADDR madrBlkEndAddr;
	DWORD dwBlkNo;
	DWORD dwBlkSize;
	BOOL bReadEna;

	WORD	wReadData[1];		// 参照データ格納領域

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

	BOOL	bFcuReadWarning;			// RevNo111121-007 Append Line

	// RevRxNo161003-001 Append Start
	enum FFWRXENM_RMODE		eRmode;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	// RevRxNo161003-001 Append End

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	DWORD	dwReadLengthMax;
	getEinfData(&einfData);			// エミュレータ情報取得

	// RevRxE2LNo141104-001 Modify Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {	// E2/E2 Liteの場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX_E2;
	} else if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// E1/E20の場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX;
	} else {	// EZ-CUBEの場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX_EZ;
	}
	// RevRxE2LNo141104-001 Modify End
// RevRxNo121022-001 Append End

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーブートマットキャッシュメモリ構造体情報取得

	pDbgData = GetDbgDataRX();
	pbyReadBuf = pbyBuff;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

	//アクセスするバイトサイズ
	dwTotalLength = madrEndAddr - madrStartAddr + 1;

	// データ格納領域を初期化
	memset(pbyReadBuf, 0, dwTotalLength);

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case MWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case MLWORD_ACCESS:
		dwAddAddr = 4;
		break;
	default:
		dwAddAddr = 4;	// アドレス加算値算出
		break;
	}

	//アクセスバイトサイズからアクセス回数に変換
	dwTotalAccessCount = dwTotalLength/dwAddAddr;

	pPb = GetPbData();						// S/Wブレークポイント設定を取得

	// RevRxNo161003-001 Append Start
	eRmode = GetRmodeDataRX();				// レジスタ設定の起動モードを取得
	pMcuDef = GetMcuDefData();
	// RevRxNo161003-001 Append End

	while (dwTotalAccessCount > 0) {
		// 指定領域判定
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Line
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 0, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (bEnableArea == TRUE) {
// RevRxNo121022-001 Modify Start
			if (dwLength > dwReadLengthMax) {
				dwLength = dwReadLengthMax;
			}
// RevRxNo121022-001 Modify End

			//バイトサイズからアクセス回数に変換
			dwAccessCount = dwLength/dwAddAddr;

			ferr = GetEndianType2(madrStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start

			// RevRxNo140109-001 Append Line
			// 指定領域が内部ROMであるかを判定
			bInRomArea = IsMcuInRomArea(eAreaType);

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Modify Line
			if (bInRomArea == TRUE) {	// 内蔵ROM領域の場合	// RevRxNo140109-001 Modify Line
				// プログラム実行中はRAMと同じ扱い(キャッシュを見ない)
				//RevNo120727-001 Modify Line
				if (GetMcuRunState() == TRUE) {
					// ユーザーマット書き換えデバッグ無効 かつ、データマット書き換えデバッグ無効の場合
					if ((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_NON) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_NON)) {
						if (eAreaType == MAREA_DATA) {		// データマット領域の場合
							// 対象ブロック情報取得
							GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
							// 対象ブロックの読み出し許可ビットを確認
							ferr = GetRegDataFlashReadBlockEnable(dwBlkNo,&bReadEna);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
							// 読み出し許可の場合
							if(bReadEna==TRUE){
								// 実MCUをダンプ
								ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
								if (ferr != FFWERR_OK) {
									return ferr;
								}
							}
							// 読み出し禁止の場合
							else{
								// V.1.02 RevNo110308-002 Modify Start
								// 読み出し禁止領域だけ0xFFで埋める必要あり、許可領域は実メモリをDUMPしないといけなかったがそうなっていなかった。
								// V.1.02 RevNo110310-002 Modify Line
								memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
								SetDtfMcurunFlg(TRUE);
								// V.1.02 RevNo110308-002 Modify Start
							}
						}
						else {							// データマット領域以外の場合(ユーザーマット/ユーザーブートマット)
							// 実MCUをダンプ
							ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
					// V.1.02 RevNo110629-001 フラッシュ書き換えデバッグ時のP/Eモード確認処理修正 Append Start
					// 	ユーザマット書き換えデバッグのみ有効の場合
					}else if ((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_NON)) {
						// RevRxNo161003-001 Modify Start
						if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {		// 起動バンク指定付モードでない場合
							// ユーザマット書き換えデバッグ有効時 == ROM P/Eモード状態である可能性が高い
							// ROM P/Eモード中はユーザマットもデータマットもリード不可のためダミーデータを返す。
							memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
							SetFlashDbgMcurunFlg(TRUE);
						} else {	// 起動バンク指定付モードの場合
							// 起動バンク側はリード可能
							//
							if (madrStartAddr >= pMcuDef->dwDualStartupBankStartAddr) {		// DUMPアドレスが起動バンク側先頭アドレス以降の場合								
								// 実MCUをダンプ
								ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
								if (ferr != FFWERR_OK) {
									return ferr;
								}
							} else {	// オプション設定メモリやデータフラッシュなど
								memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
								SetFlashDbgMcurunFlg(TRUE);
							}
						}
						// RevRxNo161003-001 Modify End
					// 	データマット書き換えデバッグのみ有効の場合
					}else if ((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_NON) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)) {
						// データマット書き換えデバッグ有効時 == データフラッシュP/Eモード状態である可能性が高い
						// データフラッシュP/Eモード中はデータマットのみリード不可のためデータマット領域以外は実メモリをDUMPする。
						if ((eAreaType != MAREA_DATA) && (eAreaType != MAREA_CONFIG)) {
							// データマット領域でない且つコンフィギュレーション設定領域ではない場合	// RevRxNo140109-001 Modify Line

							// 実MCUをダンプ
							ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}else{		// データマット領域またはコンフィギュレーション設定領域の場合 → ダミーデータ返す
							memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
							SetFlashDbgMcurunFlg(TRUE);
						}

					// ユーザーマット書き換えデバッグ有効 かつ、データマット書き換えデバッグ有効の場合					
					// V.1.02 RevNo110629-001 フラッシュ書き換えデバッグ時のP/Eモード確認処理修正 Append End
					}else{
						// RevRxNo161003-001 Modify Start
						if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {		// 起動バンク指定付モードでない場合
							// V.1.02 RevNo110308-002 Modify Start
							// 読み出し禁止領域だけ0xFFで埋める必要あり、許可領域は実メモリをDUMPしないといけなかったがそうなっていなかった。
							// V.1.02 RevNo110310-002 Modify Line
							memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
							SetFlashDbgMcurunFlg(TRUE);
							// V.1.02 RevNo110308-002 Modify Start
						} else {									// 起動バンク指定付モードの場合
							// 起動バンク側はリード可能
							//
							if (madrStartAddr >= pMcuDef->dwDualStartupBankStartAddr) {		// DUMPアドレスが起動バンク側先頭アドレス以降の場合								
								// 実MCUをダンプ
								ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
								if (ferr != FFWERR_OK) {
									return ferr;
								}
							} else {	// オプション設定メモリやデータフラッシュなど
								memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
								SetFlashDbgMcurunFlg(TRUE);
							}
						}
						// RevRxNo161003-001 Modify End
					}
				}
	
				// プログラム停止中
				else{
					if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) || (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
// データフラッシュ書き換えデバッグ有効時ののみP/Eモードかどうかの確認を行うとしていたが、最終的にどちらかの書き換えデバッグが
// 有効だったら、内蔵フラッシュ(データマット、ユーザマット、ユーザブートマット)へのアクセスはしないようにすべきとの判断でコメントとした。2011.3.9
//					if(pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE){
// → 2011.6.29 ツール技若槻さんから指摘で、データマットがP/Eモードエントリ状態でもユーザマットはリード可能ということが判明。処理修正。
						// FENTRYRレジスタ値取得
						ferr = GetFentryrRegData(&wReadData[0]);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						// V.1.02 RevNo110629-001 フラッシュ書き換えデバッグ時のP/Eモード確認処理修正 Append & Modify Start
						// データマット書き換えデバッグのみ有効の場合
						if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_NON) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
							// データフラッシュP/Eモード状態の場合、データマット領域へのアクセスは禁止のためエラーを返す。
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE) == MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE){	// FENTRYDビットが立っている場合
								if ((eAreaType == MAREA_DATA) || (eAreaType == MAREA_CONFIG)) {
									// DUMP領域がデータマット領域またはコンフィギュレーション設定領域の場合	// RevRxNo140109-001 Modify Line
									return FFWERR_ACC_ROM_PEMODE;
								}
							}
						// ユーザマット書き換えデバッグのみ有効の場合
						}else if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_NON)){
							// ROM P/Eモード状態の場合、ユーザマット、データマット領域ともアクセスは禁止のためエラーを返す。
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_USERMAT_PEMODE) != MCU_REG_FLASH_FENTRYR_NO_PEMODE){			// FENTRYxビットが立っている場合
								// RevRxNo161003-001 Modify Start
								if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {		// 起動バンク指定付モードでない場合
									return FFWERR_ACC_ROM_PEMODE;
								} else {									// 起動バンク指定付モードの場合
									// 起動バンク側はリード可能
									//
									if (madrStartAddr >= pMcuDef->dwDualStartupBankStartAddr) {		// DUMPアドレスが起動バンク側先頭アドレス以降の場合										
										GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロックの情報取得
										if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
											ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
											if (ferr != FFWERR_OK) {
												return ferr;
											}
										}
										GetFlashRomCacheData(eAreaType,madrStartAddr, dwAccessCount, eAccessSize, pbyReadBuf);
									} else {	// オプション設定メモリやデータフラッシュなど
										return FFWERR_ACC_ROM_PEMODE;
									}
								}
								// RevRxNo161003-001 Modify End
							}
						// ユーザマット & データマット書き換えデバッグ有効の場合
						}else if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
							// ROM P/Eモード状態の場合、ユーザマット、データマット領域ともアクセスは禁止のためエラーを返す。
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_USERMAT_PEMODE) != MCU_REG_FLASH_FENTRYR_NO_PEMODE){			// FENTRYxビットが立っている場合
								// RevRxNo161003-001 Modify Start
								if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {		// 起動バンク指定付モードでない場合
									return FFWERR_ACC_ROM_PEMODE;
								} else {									// 起動バンク指定付モードの場合
									// 起動バンク側はリード可能
									//									
									if (madrStartAddr >= pMcuDef->dwDualStartupBankStartAddr) {		// DUMPアドレスが起動バンク側先頭アドレス以降の場合																	
										GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);	// 対象ブロックの情報取得
										if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
											ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
											if (ferr != FFWERR_OK) {
												return ferr;
											}
										}
										GetFlashRomCacheData(eAreaType,madrStartAddr, dwAccessCount, eAccessSize, pbyReadBuf);
									} else {	// オプション設定メモリやデータフラッシュなど
										return FFWERR_ACC_ROM_PEMODE;
									}
								}
								// RevRxNo161003-001 Modify End
							}
							// データフラッシュP/Eモード状態の場合、データマット領域へのアクセスは禁止のためエラーを返す。
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE) == MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE){	// FENTRYDビットが立っている場合
								if ((eAreaType == MAREA_DATA) || (eAreaType == MAREA_CONFIG)) {
									// DUMP領域がデータマット領域またはコンフィギュレーション設定領域の場合	// RevRxNo140109-001 Modify Line

									return FFWERR_ACC_ROM_PEMODE;
								}
							}
						}
						// V.1.02 RevNo110629-001 フラッシュ書き換えデバッグ時のP/Eモード確認処理修正 Append & Modify Start
					}
					if (eAreaType == MAREA_DATA) {		// データマット領域の場合
						// 対象ブロック情報取得
						GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
						
						ferr = SetRegDataFlashReadEnable(TRUE);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						if(dm_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
							// RevRxNo161003-001 Append Start +4
							ferr = SetEepfclk(TRUE);		// EEPFCLKレジスタ設定
							if (ferr != FFWERR_OK) {
								return ferr;
							}
							// RevRxNo161003-001 Append End

							ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
							if (ferr != FFWERR_OK) {
								return ferr;
							}

							// RevRxNo161003-001 Append Start +4
							ferr = SetEepfclk(FALSE);		// EEPFCLKレジスタ復帰
							if (ferr != FFWERR_OK) {
								return ferr;
							}
							// RevRxNo161003-001 Append End
						}
						else{
							GetFlashRomCacheData(eAreaType,madrStartAddr, dwAccessCount, eAccessSize, pbyReadBuf);
						}
						ferr = SetRegDataFlashReadEnable(FALSE);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
					}
					else if (eAreaType == MAREA_USER) {		// ユーザーマット領域の場合
						// 対象ブロック情報取得
						GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

						if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
							ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
						GetFlashRomCacheData(eAreaType,madrStartAddr, dwAccessCount, eAccessSize, pbyReadBuf);
					}
					else if (eAreaType == MAREA_USERBOOT){		// ユーザーブートマット領域の場合	// RevRxNo140109-001 Modify Line
						// 対象ブロック情報取得
						GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

						if(GetUbmCacheSet() == FALSE){
							ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
						GetFlashRomCacheData(eAreaType,madrStartAddr, dwAccessCount, eAccessSize, pbyReadBuf);

					// RevRxNo140109-001 Append Start
					} else {	// コンフィギュレーション設定領域の場合
						// 実MCUをダンプ
						ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
					}
					// RevRxNo140109-001 Append Start
				}
			}
			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

			else {
// RevNo111121-007 Append Start
				bFcuReadWarning = FALSE;
				if (IsMcuFcuFirmArea(madrStartAddr) == TRUE) {
						// 指定アドレスがFCU ファーム領域の場合

					//RevNo120727-001 Modify Line
					if (GetMcuRunState() == TRUE) {
							// ユーザプログラム実行中の場合

						if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {
								// プログラムROM のフラッシュ書き換えデバッグが「有効」の場合

							// RevRxNo121122-003 Modify Line
							memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
							SetFlashDbgMcurunFlg(TRUE);
							bFcuReadWarning = TRUE;
						}
					} else {
							// ユーザプログラム停止中の場合

						if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {
								// プログラムROM のフラッシュ書き換えデバッグが「有効」の場合

							// FENTRYRレジスタ値取得
							ferr = GetFentryrRegData(&wReadData[0]);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_USERMAT_PEMODE) != MCU_REG_FLASH_FENTRYR_NO_PEMODE){
									// FENTRYxビットが立っている場合

								return FFWERR_ACC_ROM_PEMODE;
							}
						}
					}
				}
// RevNo111121-007 Append End
// RevNo111121-007 Modify Start
				if (bFcuReadWarning == FALSE) {
					ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, 
								bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);	// RevNo111121-007 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
// RevNo111121-007 Modify End
			}

			// S/Wブレークポイントが含まれるかチェック
			for (dwCnt = 0, dwLoopAddr = madrStartAddr; dwLoopAddr < (madrStartAddr + dwLength); dwLoopAddr++, dwCnt++) {
				OffsetCWriteAddr(byEndian, dwLoopAddr, &dwStartOffsetAddr);	// 退避命令はコード並びで取得しているため、
																			// S/Wブレーク設定アドレスの判定はCWRITE時のアドレスで行なう
				for (i = 0; i < pPb->dwSetNum; i++) {
					if ((pPb->dwmadrAddr[i] == dwStartOffsetAddr) && (pPb->eFillState[i] == PB_FILL_COMPLETE)) {
						// V.1.02 RevNo110418-003 Append Start
						if (eAreaType == MAREA_DATA) {		// データマット領域の場合
							ferr = SetRegDataFlashReadEnable(TRUE);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
						// V.1.02 RevNo110418-003 Append End

						// S/Wブレーク設定アドレスに現在書き込まれている内容を取得
						// ※DO_DUMP処理中なので直接PROT_MCU_DUMPを呼び出す
						ferr = PROT_MCU_DUMP(1, &dwLoopAddr, FALSE, &eAccessSizePb, FALSE, &dwPbAccessCount, &byCmdData, byEndian);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						// V.1.02 RevNo110418-003 Append Start
						if (eAreaType == MAREA_DATA) {		// データマット領域の場合
							ferr = SetRegDataFlashReadEnable(FALSE);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
						// V.1.02 RevNo110418-003 Append End

						// DBT命令の場合だけ退避命令コードで置換
						if (byCmdData == byDbtCmd) {
							memcpy(&pbyReadBuf[dwCnt], &pPb->byCmdData[i], sizeof(BYTE));
						}
						break;
					}
				}
			}
		} else {	// 有効領域ではない場合
			// V.1.02 RevNo110304-001 Modify Start
			dwAccessCount = dwLength/dwAddAddr;
			// break;  <- ここでブレークすると予約領域以降の有効領域の処理ができない
			// V.1.02 RevNo110304-001 Modify End
		}

		dwTotalAccessCount -= dwAccessCount;			// DUMP回数更新
		dwLength = dwAccessCount * dwAddAddr;
		madrStartAddr += dwLength;			// DUMP開始アドレス更新
		if (dwTotalLength > 0) {
			pbyReadBuf += dwLength;			// DUMPデータ格納バッファアドレス更新
		}
	}

	return ferr;
}
// RevRxNo120606-004 Append Start
//=============================================================================
/**
 * デバッグDMA機能によるDUMP処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eAccessSize アクセスサイズ
 * @param pbyBuff DUMPデータ格納バッファアドレス
 * @return FFWエラーコード
 */
//=============================================================================
static FFWERR dumpCpuAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	DWORD	dwAddAddr;
	DWORD	dwLength;
	BYTE*	pbyReadBuf;
	BOOL	bSameAccessSize;
	BOOL	bSameLength;
	DWORD	dwTotalLength;
	BYTE	byEndian;
	BOOL	bEnableArea = FALSE;

	DWORD	dwTotalReadCnt;				// 全DUMP回数を格納
	DWORD	dwReadCnt;					// DUMP回数を格納
	BOOL	bRet;						
	
	enum FFWRXENM_MAREA_TYPE eAreaType;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	DWORD	dwReadLengthMax;
	getEinfData(&einfData);			// エミュレータ情報取得

	// RevRxE2LNo141104-001 Modify Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {	// E2/E2 Liteの場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX_E2;
	} else if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// E1/E20の場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX;
	} else {	// EZ-CUBEの場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX_EZ;
	}
	// RevRxE2LNo141104-001 Modify End
// RevRxNo121022-001 Append End

	pbyReadBuf = pbyBuff;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameLength = FALSE;

	dwTotalLength = madrEndAddr - madrStartAddr + 1;
	// 実行時のダミーデータが0なので、0初期化実施削除する場合は注意
	memset(pbyReadBuf, 0, dwTotalLength);

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case MWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case MLWORD_ACCESS:
		dwAddAddr = 4;
		break;
	default:
		dwAddAddr = 4;
		break;
	}
	
	// DUMP回数算出
	// 端数処理はDO_DUMP()層で行う。端数切り上げ処理を行うとメモリアクセス異常を起こす可能性があるのでやらない
	dwTotalReadCnt = dwTotalLength / dwAddAddr;

	// プログラム実行情報取得
	bRet = GetMcuRunState();
	if(bRet == TRUE){
		//プログラム実行中の場合は正常終了 DUMPデータはダミーデータ(0)
		memset(pbyReadBuf, 0, dwTotalLength);
		return FFWERR_OK;
	}

	while (dwTotalReadCnt > 0) {
		// 指定領域判定
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 0, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (bEnableArea == TRUE) {
// RevRxNo121022-001 Modify Start
			if (dwLength > dwReadLengthMax) {
				dwLength = dwReadLengthMax;
			}
// RevRxNo121022-001 Modify End

			// 今回のリード回数を算出
			// 端数処理はDO_DUMP()層で行う。端数切り上げ処理を行うとメモリアクセス異常を起こす可能性があるのでやらない
			dwReadCnt = dwLength / dwAddAddr;

			ferr = GetEndianType2(madrStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			ferr = PROT_MCU_CPUDUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameLength, &dwReadCnt, pbyReadBuf, byEndian);	// RevRxNo120606-004 Modify Line DUMP回数を渡すところにDUMPバイトサイズを渡していた。
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			// DUMP回数更新
			dwTotalReadCnt -= dwReadCnt;
			// DUMP開始アドレス更新
			madrStartAddr += dwLength;
			if (dwTotalReadCnt > 0) {
				// DUMPデータ格納バッファアドレス更新
				pbyReadBuf += dwLength;
			}
		}
	}

	return ferr;
}
// RevRxNo120606-004 Append End

//=============================================================================
/**
 * デバッグDMA機能によるFILL処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param dwWriteDataSize 書き込みデータサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR fillDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
							FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr2 = FFWERR_OK;	// RevRxNo161003-001 Append Line
	DWORD	dwAddAddr;
	DWORD	dwLength;			//アクセスバイトサイズ
	DWORD	dwAccessCount;		//アクセス回数
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	DWORD	dwTotalLength;		//全体のアクセスバイトサイズ
	DWORD	dwTotalAccessCount;	//全体のアクセス回数
	FFW_PB_DATA* pPb;
	DWORD	i;
	DWORD	dwCnt = 0;
	DWORD	dwLoopAddr;
	BYTE	byEndian;
	DWORD	dwStartOffsetAddr;
	DWORD	dwLoopMaxAddr = 0;
	BOOL	bEnableArea = FALSE;
	BOOL	bInRomArea = FALSE;		// RevRxNo140109-001 Append Line

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	FFWMCU_DBG_DATA_RX*	pDbgData;
	WORD	wReadData[1];		// 参照データ格納領域
	enum FFWRXENM_MAREA_TYPE eAreaType;

	pDbgData = GetDbgDataRX();
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

	s_eVerify = eVerify;	// ベリファイチェック有無

// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	dwTotalLength = madrEndAddr - madrStartAddr + 1;

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
	dwTotalAccessCount = dwTotalLength/dwAddAddr;	// FILL回数算出
	
	pPb = GetPbData();						// S/Wブレークポイント設定を取得

// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	while (dwTotalAccessCount > 0) {
		// 指定領域判定
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Line
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 1, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// V.1.01 No.26 メモリアクセス端数データ処理統一　Appned Start
		dwLoopAddr = madrStartAddr;
		ferr = GetEndianType2(dwLoopAddr, &byEndian);							// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// V.1.01 No.26 メモリアクセス端数データ処理統一　Appned End
		if (bEnableArea == TRUE) {
			if (dwLength > PROT_FILL_LENGTH_MAX) {
				dwLength = PROT_FILL_LENGTH_MAX;
			}

			//アクセスバイトサイズをアクセス回数に変換
			dwAccessCount = dwLength/dwAddAddr;
			if (dwAccessCount < 1) {
				dwAccessCount = 1;
			}

			dwLoopMaxAddr = (madrStartAddr + dwLength ) - 1;		// ループ最終アドレスを算出

			// S/Wブレークポイントが含まれるかチェック
// V.1.01 No.26 メモリアクセス端数データ処理統一　Modify Line
			for (; dwLoopAddr <= dwLoopMaxAddr; dwLoopAddr++, dwCnt++) {
				if (dwCnt > dwWriteDataSize - 1) {		// 書込みデータサイズを超えた場合、元に戻す
					dwCnt = 0;
				}
// V.1.01 No.26 メモリアクセス端数データ処理統一　Delete Line
//				GetEndianType2(dwLoopAddr, &byEndian);							// エンディアン判定＆取得
				OffsetCWriteAddr(byEndian, dwLoopAddr, &dwStartOffsetAddr);		// 退避命令はコード並びで取得しているため、
																				// S/Wブレーク設定アドレスの判定はCWRITE時のアドレスで行なう
				for (i = 0; i < pPb->dwSetNum; i++) {
					if ((pPb->dwmadrAddr[i] == dwStartOffsetAddr) && (pPb->eFillState[i] == PB_FILL_COMPLETE)) {
						pPb->eFillState[i] = PB_FILL_SET;
						memcpy(&pPb->byCmdData[i], &pbyWriteBuff[dwCnt], sizeof(BYTE));	// DBT命令は1BYTE命令
						break;
					}
				}
				if ((dwLoopAddr + 1) < 1) {		// 最終アドレスの場合、ループ抜ける
					break;
				}
			}

			ferr = GetEndianType2(madrStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start

			// RevRxNo140109-001 Append Line
			// 指定領域が内部ROMであるかを判定
			bInRomArea = IsMcuInRomArea(eAreaType);

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Modify Line
			if (bInRomArea == TRUE) {	// 内蔵ROM領域の場合	// RevRxNo140109-001 Modify Line
				if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) || (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
					// FENTRYRレジスタ値取得
					ferr = GetFentryrRegData(&wReadData[0]);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					if(wReadData[0] != MCU_REG_FLASH_FENTRYR_NO_PEMODE){
						return FFWERR_ACC_ROM_PEMODE;
					}
				}
				if(eAreaType == MAREA_DATA){		// データマット領域の場合
					// RevRxNo161003-001 Append Start +4
					ferr = SetEepfclk(TRUE);		// EEPFCLKレジスタ設定
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					// RevRxNo161003-001 Append End

					ferr = SetRegDataFlashReadEnable(TRUE);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
					
				ferr = SetFlashRomCacheMem(eAreaType, madrStartAddr, dwAccessCount, eAccessSize, pbyWriteBuff, dwWriteDataSize);
				if (ferr != FFWERR_OK) {
					// RevRxNo161003-001 Append Start +10
					if(eAreaType == MAREA_DATA){		// データマット領域の場合
						ferr2 = SetEepfclk(FALSE);		// EEPFCLKレジスタ復帰
						if (ferr2 != FFWERR_OK) {
							return ferr2;
						}

						ferr2 = SetRegDataFlashReadEnable(FALSE);
						if (ferr2 != FFWERR_OK) {
							return ferr2;
						}
					}
					// RevRxNo161003-001 Append End

					return ferr;
				}
				
				if(eAreaType == MAREA_DATA){		// データマット領域の場合
					// RevRxNo161003-001 Append Start +4
					ferr = SetEepfclk(FALSE);		// EEPFCLKレジスタ復帰
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					// RevRxNo161003-001 Append End

					ferr = SetRegDataFlashReadEnable(FALSE);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}
			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

			else {							// フラッシュROM領域以外の場合
				ferr = PROT_MCU_FILL(eVerify, madrStartAddr, eAccessSize, dwAccessCount, dwWriteDataSize, pbyWriteBuff, pVerifyErr, byEndian);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				// ベリファイチェック
				if (s_eVerify == VERIFY_ON) {
					ferr = chkVerifyFillData(madrStartAddr, eAccessSize, dwAccessCount, pbyWriteBuff, pVerifyErr, dwWriteDataSize, byEndian);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						return ferr;
					}
				}
			}
		} else {
			dwAccessCount = dwLength/dwAddAddr;
		}

// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
		dwTotalAccessCount -= dwAccessCount;		// FILL回数更新
		dwLength = dwAccessCount * dwAddAddr;
		madrStartAddr += dwLength;	// FILL開始アドレス更新
	}

	return ferr;
}
// RevRxNo120606-004 Append Start
//=============================================================================
/**
 * デバッグDMA機能によるFILL処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param dwWriteDataSize 書き込みデータサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @return FFWエラーコード
 */
//=============================================================================
static FFWERR fillCpuAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
							FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAddAddr;
	DWORD	dwLength;
	DWORD	dwTotalLength;
	BYTE	byEndian;
	BOOL	bEnableArea = FALSE;
	BOOL	bRet;
	DWORD	dwFillCnt;

	enum FFWRXENM_MAREA_TYPE eAreaType;

	s_eVerify = eVerify;	// ベリファイチェック有無

	//アクセスバイト数算出
	dwTotalLength = madrEndAddr - madrStartAddr + 1;

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

	// プログラム実行情報取得
	bRet = GetMcuRunState();
	if(bRet == TRUE){
		// 実行中は実行中エラーを返す
		return FFWERR_BMCU_RUN;
	}

	if(dwTotalLength > 0) {
		// 指定領域判定
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 1, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		ferr = GetEndianType2(madrStartAddr, &byEndian);							// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			return ferr;
		}	
		if (bEnableArea == TRUE) {

			// Fill回数算出
			// dwTotalLengthが端数の場合(LWORDアクセスのときに3Byteライト)は切り上げが必要であるが、
			// DO_FILL()層で端数処理を実施しているため、切り上げ処理不要
			dwFillCnt = dwTotalLength / dwAddAddr;

			ferr = PROT_MCU_CPUFILL(eVerify, madrStartAddr, eAccessSize, dwFillCnt, dwWriteDataSize, pbyWriteBuff, pVerifyErr, byEndian);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ベリファイチェック
			if (s_eVerify == VERIFY_ON) {
				ferr = chkVerifyFillData(madrStartAddr, eAccessSize, dwFillCnt, pbyWriteBuff, pVerifyErr, dwWriteDataSize, byEndian);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					return ferr;
				}
			}
		}
	}

	return ferr;
}
// RevRxNo120606-004 Append End
//=============================================================================
/**
 * デバッグDMA機能によるWRITE処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR writeDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							 enum FFWENM_MACCESS_SIZE eAccessSize,  const BYTE* pbyWriteBuff,
							 FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr2 = FFWERR_OK;	// RevRxNo161003-001 Append Line
	DWORD	dwAreaNum;
	DWORD	dwAddAddr = 4;
	DWORD	dwLength;			//アクセスバイトサイズ
	DWORD	dwAccessCount;		//アクセス回数
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BOOL	bSameWriteData;
	const BYTE*	pbyBuff;
	FFWMCU_DBG_DATA_RX*	pDbgData;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	DWORD	dwTotalLength;

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	WORD	wReadData[1];		// 参照データ格納領域

	FFW_PB_DATA* pPb;
	DWORD	i;
	DWORD	dwCnt = 0;
	DWORD	dwLoopAddr;
	BYTE	byEndian;
	BOOL	bRetFillFlg;
	DWORD	dwStartOffsetAddr;
	BOOL	bEnableArea = FALSE;
	BOOL	bInRomArea = FALSE;		// RevRxNo140109-001 Append Line
	BOOL	bExtArea = FALSE;
	enum FFWENM_MACCESS_SIZE eEscAccessSize;
	enum FFWRXENM_MAREA_TYPE eAreaType;
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Modify Start
	EXTRAM_DWNP_DATA	*extram_ptr;
	EXTRAM_DWNP_DATA	*extram_tmpptr;
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Modify End

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	DWORD	dwWriteLengthMax = 0;
	DWORD	dwExtWriteLengthMax = 0;
	DWORD	dwInRamWriteLengthMax = 0;	// RevRxE2LNo141104-001 Append Line
	BOOL	bWriteCache;				// RevRxE2LNo141104-001 Append Line
	BYTE*	pbyInramBuff;				// RevRxE2LNo141104-001 Append Line


	getEinfData(&einfData);			// エミュレータ情報取得

	if (einfData.wEmuStatus == EML_EZCUBE) {		// EZ-CUBEの場合
		dwWriteLengthMax = PROT_WRITE_LENGTH_MAX_EZ;
		dwExtWriteLengthMax = PROT_EXT_WRITE_LENGTH_MAX_EZ;
		dwInRamWriteLengthMax = PROT_INRAM_WRITE_LENGTH_MAX_EZ;	// RevRxE2LNo141104-001 Append Line

	} else {		// E1/E20/E2/E2 Liteの場合
		dwWriteLengthMax = PROT_WRITE_LENGTH_MAX;
		dwExtWriteLengthMax = PROT_EXT_WRITE_LENGTH_MAX;

		// RevRxE2LNo141104-001 Append Start
		if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
			dwInRamWriteLengthMax = PROT_INRAM_WRITE_LENGTH_MAX_E2;
		} else {
			dwInRamWriteLengthMax = PROT_INRAM_WRITE_LENGTH_MAX;
		}
		// RevRxE2LNo141104-001 Append End
	}
// RevRxNo121022-001 Append End

	pDbgData = GetDbgDataRX();

	pbyBuff = pbyWriteBuff;
	dwAreaNum = 1;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	dwTotalLength = madrEndAddr - madrStartAddr + 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	bSameWriteData = FALSE;

	s_eVerify = eVerify;	// ベリファイチェック有無

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
		break;
	}

	pPb = GetPbData();						// S/Wブレークポイント設定を取得

// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	while (dwTotalLength > 0) {
		// 指定領域判定
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Line
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 1, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// V.1.01 No.26 メモリアクセス端数データ処理統一　Appned Start
		dwLoopAddr = madrStartAddr;
		ferr = GetEndianType2(dwLoopAddr, &byEndian);						// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			return ferr;
		}
// V.1.01 No.26 メモリアクセス端数データ処理統一　Appned End
		if (bEnableArea) {
			// RevRxE2LNo141104-001 Append Start
			if ((eAreaType == MAREA_INRAM) && GetDwnpOpenData()) {
				// 指定領域が内部RAM且つダウンロード中の場合
				if (dwLength > dwInRamWriteLengthMax) {
					dwLength = dwInRamWriteLengthMax;
				}
			// RevRxE2LNo141104-001 Append End

			// RevRxNo140109-001 Modify Start
			// RevRxE2LNo141104-001 Modify Line
			} else if ((eAreaType == MAREA_EXTRAM) && GetDwnpOpenData()) {
				// 指定領域が外部RAM且つダウンロード中の場合

				if (dwLength > dwExtWriteLengthMax) {
					dwLength = dwExtWriteLengthMax;
				}
			} else {
				if (dwLength > dwWriteLengthMax) {
					dwLength = dwWriteLengthMax;
				}
			}
			// RevRxNo140109-001 Modify End

// V.1.02 No.14,15 ダウンロード高速化対応 Modify Start
// V.1.01 No.26 メモリアクセス端数データ処理統一　Appned Start
			// DWNPコマンド実行中の場合
			if (GetDwnpOpenData()) {
				eEscAccessSize = eAccessSize;
				// 外部メモリのアクセスサイズ取得
				// V.1.02 RevNo110510-003 Modify Line
				GetExtAccessSize(madrStartAddr, madrStartAddr, &eAccessSize, &bExtArea);
				if (eEscAccessSize != eAccessSize) {	// 元々のアクセスサイズと異なる場合
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
				}
			}
			//アクセスバイトサイズをアクセス回数に変換
			dwAccessCount = dwLength/dwAddAddr;
			if (dwAccessCount < 1) {
				dwAccessCount = 1;
			}
// V.1.01 No.26 メモリアクセス端数データ処理統一　Appned End

			// DWNPコマンド実行中でない場合
			// 命令コード復帰/埋め込みフラグ状態を取得
			bRetFillFlg = GetCmdRetFillFlg();
			if (bRetFillFlg == FALSE) {
				// S/Wブレークポイントが含まれるかチェック
// V.1.01 No.26 メモリアクセス端数データ処理統一　Modify Line
				for ( ; dwLoopAddr < (madrStartAddr + dwLength); dwLoopAddr++, dwCnt++) {
// V.1.01 No.26 メモリアクセス端数データ処理統一　Delete Line
					OffsetCWriteAddr(byEndian, dwLoopAddr, &dwStartOffsetAddr);		// 退避命令はコード並びで取得しているため、
																					// S/Wブレーク設定アドレスの判定はCWRITE時のアドレスで行なう
					for (i = 0; i < pPb->dwSetNum; i++) {
						if ((pPb->dwmadrAddr[i] == dwStartOffsetAddr) && (pPb->eFillState[i] == PB_FILL_COMPLETE)) {
							pPb->eFillState[i] = PB_FILL_SET;
							memcpy(&pPb->byCmdData[i], &pbyWriteBuff[dwCnt], sizeof(BYTE));	// DBT命令は1BYTE命令
							break;
						}
					}
				}
			}

			ferr = GetEndianType2(madrStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start

			// RevRxNo140109-001 Append Line
			// 指定領域が内部ROMであるかを判定
			bInRomArea = IsMcuInRomArea(eAreaType);

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Modify Line
			if (bInRomArea) {	// 内蔵ROM領域の場合	// RevRxNo140109-001 Modify Line
				// RevRxNo140109-001 Append Start
				if (eAreaType == MAREA_CONFIG) {		// コンフィギュレーション設定領域の場合
					s_bExistConfigDwnDataFlg = TRUE;	// コンフィギュレーション設定領域へのダウンロードデータあり設定
				}
				// RevRxNo140109-001 Append End

				if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) || (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
					// FENTRYRレジスタ値取得
					ferr = GetFentryrRegData(&wReadData[0]);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					if(wReadData[0] != MCU_REG_FLASH_FENTRYR_NO_PEMODE){
						return FFWERR_ACC_ROM_PEMODE;
					}
				}
				if(eAreaType == MAREA_DATA){		// データマット領域の場合
					// RevRxNo161003-001 Append Start +4
					ferr = SetEepfclk(TRUE);		// EEPFCLKレジスタ設定
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					// RevRxNo161003-001 Append End

					ferr = SetRegDataFlashReadEnable(TRUE);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
				ferr = SetFlashRomCacheMem(eAreaType, madrStartAddr, dwAccessCount, eAccessSize, pbyBuff, 0);
				if (ferr != FFWERR_OK) {
					// RevRxNo161003-001 Append Start +10
					if(eAreaType == MAREA_DATA){		// データマット領域の場合
						ferr2 = SetEepfclk(FALSE);		// EEPFCLKレジスタ復帰
						if (ferr2 != FFWERR_OK) {
							return ferr2;
						}

						ferr2 = SetRegDataFlashReadEnable(FALSE);
						if (ferr2 != FFWERR_OK) {
							return ferr2;
						}
					}
					// RevRxNo161003-001 Append End

					return ferr;
				}
				if(eAreaType == MAREA_DATA){		// データマット領域の場合
					// RevRxNo161003-001 Append Start +4
					ferr = SetEepfclk(FALSE);		// EEPFCLKレジスタ復帰
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					// RevRxNo161003-001 Append End

					ferr = SetRegDataFlashReadEnable(FALSE);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End
			} else {									// 内蔵ROM領域以外の場合
				// RevRxE2LNo141104-001 Append Start
				// FFWキャッシュへのデータ保存判定
				if (GetDwnpOpenData()) {
					if ((eAreaType == MAREA_INRAM) && ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE))) {
															// E2/E2 Liteで内部RAMへのダウンロード時
						bWriteCache = TRUE;
					} else if (eAreaType == MAREA_EXTRAM) {	// 外部RAMへのダウンロード時
						bWriteCache = TRUE;
					} else {								// 上記以外のダウンロード時
						bWriteCache = FALSE;
					}
				} else {	// ダウンロード以外
					bWriteCache = FALSE;
				}
				// RevRxE2LNo141104-001 Append End

				// FFWキャッシュまたはMCUへのライト処理
				if (bWriteCache == FALSE) {	// MCUへライトする場合	// RevRxE2LNo141104-001 Modify Line
					ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
								bSameAccessCount, &dwAccessCount, bSameWriteData, pbyBuff, pVerifyErr, byEndian);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					// ベリファイチェック
					if (s_eVerify == VERIFY_ON) {
						ferr = ChkVerifyWriteData(madrStartAddr, eAccessSize, dwAccessCount, pbyBuff, pVerifyErr, byEndian);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						if (pVerifyErr->eErrorFlag == VERIFY_ERR) {			// ベリファイエラー発生時、処理を終了する。
							return ferr;
						}
					}
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned Start
				} else {	// FFWキャッシュへライトする場合(E2/E2 Liteの内部RAMダウンロード、または、外部RAMダウンロード時)
					// RevRxE2LNo141104-001 Append Start
					if (eAreaType == MAREA_INRAM) {	// 内部RAMへのダウンロード
						SetInRamDwnpInfoPtr(eVerify, madrStartAddr, eAccessSize, dwAccessCount, byEndian);	// 内部RAMダウンロード情報格納

						pbyInramBuff = GetInRamBuffPtr(madrStartAddr);
						memcpy(pbyInramBuff, pbyBuff, dwLength);	// 内部RAMダウンロードデータをFFWキャッシュにライト
					// RevRxE2LNo141104-001 Append End

					} else {	// 外部RAMへのダウンロード	// RevRxE2LNo141104-001 Modify Line
						extram_ptr = (EXTRAM_DWNP_DATA *)new(EXTRAM_DWNP_DATA);
						extram_ptr->pbyBuff = (BYTE *)new(BYTE[dwLength]);
						memcpy( extram_ptr->pbyBuff, pbyBuff, dwLength );
						extram_ptr->eVerify = eVerify;
						extram_ptr->dwCount = dwAccessCount;
						extram_ptr->eAccessSize = eAccessSize;
						extram_ptr->madrStartAddr = madrStartAddr;
						extram_ptr->byEndian = byEndian;
						extram_ptr->next = NULL;
						if( ExtramDwnpTop == NULL ){	// 外部RAMダウンロード用構造体が作ってない場合
							ExtramDwnpTop = extram_ptr;
						}else{
							extram_tmpptr = ExtramDwnpTop;
							while( extram_tmpptr->next != NULL ){
								extram_tmpptr = extram_tmpptr->next;
							}
							extram_tmpptr->next = extram_ptr;
						}
					}
				}
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Appned End
			}
		} else {
			//アクセスバイトサイズをアクセス回数に変換
			dwAccessCount = dwLength / dwAddAddr;
		}

// V.1.01 No.26 メモリアクセス端数データ処理統一　Modify Start
		dwLength = dwAccessCount * dwAddAddr;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
		dwTotalLength -= dwLength;
// V.1.01 No.26 メモリアクセス端数データ処理統一　Modify End
		madrStartAddr += dwLength;							// WRITE開始アドレス更新
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
		if (dwTotalLength > 0) {
			pbyBuff += dwLength;
		}
	}

	return ferr;
}
// RevRxNo120606-004 Append Start
//=============================================================================
/**
 * デバッグDMA機能によるWRITE処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @return FFWエラーコード
 */
//=============================================================================
static FFWERR writeCpuAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							 enum FFWENM_MACCESS_SIZE eAccessSize,  const BYTE* pbyWriteBuff,
							 FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	DWORD	dwAddAddr;
	DWORD	dwLength;
	BOOL	bSameAccessSize;
	BOOL	bSameLength;
	BOOL	bSameWriteData;
	const BYTE*	pbyBuff;
	DWORD	dwTotalLength;

	BYTE	byEndian;
	BOOL	bEnableArea = FALSE;
	enum FFWRXENM_MAREA_TYPE eAreaType;

	DWORD	dwTotalWriteCnt;		//全ライトする回数
	DWORD	dwWriteCnt;				//ライトする回数
	BOOL	bRet;

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
	bSameAccessSize = FALSE;
	bSameLength = FALSE;
	bSameWriteData = FALSE;

	// アクセスバイト数算出
	dwTotalLength = madrEndAddr - madrStartAddr + 1;

	s_eVerify = eVerify;	// ベリファイチェック有無

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

	// dwTotalLengthが端数の場合(LWORDアクセスのときに3Byteライト)は切り上げが必要であるが、
	// DO_WRITE()層で端数処理を実施しているため、切り上げ処理不要
	dwTotalWriteCnt = dwTotalLength / dwAddAddr;

	// プログラム実行情報取得
	bRet = GetMcuRunState();
	if(bRet == TRUE){
		// 実行中は実行中エラーを返す
		return FFWERR_BMCU_RUN;
	}

	while (dwTotalWriteCnt > 0) {
		// 指定領域判定
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 1, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		ferr = GetEndianType2(madrStartAddr, &byEndian);						// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (bEnableArea == TRUE) {

// RevRxNo121022-001 Modify Start
			if (dwLength > dwWriteLengthMax) {
				dwLength = dwWriteLengthMax;
			}
// RevRxNo121022-001 Modify End

			// dwLengthが端数の場合(LWORDアクセスのときに3Byteライト)は切り上げが必要であるが、
			// DO_WRITE()層で端数処理を実施しているため、切り上げ処理不要
			dwWriteCnt = dwLength / dwAddAddr;

			ferr = PROT_MCU_CPUWRITE(eVerify, dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
					bSameLength, &dwWriteCnt, bSameWriteData, pbyBuff, pVerifyErr, byEndian);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// ベリファイチェック
			if (s_eVerify == VERIFY_ON) {
				ferr = ChkVerifyWriteData(madrStartAddr, eAccessSize, dwWriteCnt, pbyBuff, pVerifyErr, byEndian);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {			// ベリファイエラー発生時、処理を終了する。
					return ferr;
				}
			}
			dwTotalWriteCnt -= dwWriteCnt;
			madrStartAddr += dwLength;							// WRITE開始アドレス更新
			if (dwTotalWriteCnt > 0) {
				pbyBuff += dwLength;
			}
		}
	}

	return ferr;
}
// RevRxNo120606-004 Append End
//=============================================================================
/**
 * デバッグDMA機能によるCodeDUMP処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eAccessSize アクセスサイズ
 * @param pbyBuff DUMPデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR cdumpDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	DWORD	dwAddAddr;
	DWORD	dwLength;				//アクセスバイトサイズ
	DWORD	dwAccessCount;			//アクセス回数
	BYTE*	pbyReadBuf;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	DWORD	dwTotalLength;			//アクセスバイトサイズ全体
	DWORD	dwTotalAccessCount;		//アクセス回数全体
	BYTE	byEndian;
	FFW_PB_DATA* pPb;
	DWORD	i;
	DWORD	dwCnt = 0;
	DWORD	dwLoopAddr;
	BYTE	byCmdData;
	BYTE	byDbtCmd = 0x01;	// DBT
	BOOL	bEnableArea = FALSE;
	BOOL	bInRomArea = FALSE;		// RevRxNo140109-001 Append Line
	enum FFWENM_MACCESS_SIZE eAccessSizePb = MBYTE_ACCESS;
	DWORD	dwPbAccessCount = 1;
	DWORD	dwStartOffsetAddr;

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start
	FFWMCU_DBG_DATA_RX*	pDbgData;
	enum FFWRXENM_MAREA_TYPE eAreaType;
	MADDR madrBlkStartAddr;
	MADDR madrBlkEndAddr;
	DWORD dwBlkNo;
	DWORD dwBlkSize;
	BOOL bReadEna;

	WORD	wReadData[1];		// 参照データ格納領域

	USER_ROMCACHE_RX		*um_ptr;	// ユーザーマットキャッシュメモリ構造体ポインタ
	DATA_ROMCACHE_RX		*dm_ptr;	// データマットキャッシュメモリ構造体ポインタ
	USERBOOT_ROMCACHE_RX	*ubm_ptr;	// ユーザーブートマットキャッシュメモリ構造体ポインタ

	BOOL	bFcuReadWarning;			// RevNo111121-007 Append Line

	// RevRxNo161003-001 Append Start
	enum FFWRXENM_RMODE		eRmode;
	FFWMCU_MCUDEF_DATA* pMcuDef;
	// RevRxNo161003-001 Append End

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	DWORD	dwReadLengthMax;
	getEinfData(&einfData);			// エミュレータ情報取得

	// RevRxE2LNo141104-001 Modify Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {	// E2/E2 Liteの場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX_E2;
	} else if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// E1/E20の場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX;
	} else {	// EZ-CUBEの場合
		dwReadLengthMax = PROT_DUMP_LENGTH_MAX_EZ;
	}
	// RevRxE2LNo141104-001 Modify End
// RevRxNo121022-001 Append End

	um_ptr = GetUserMatCacheData();				// ユーザーマットキャッシュメモリ構造体情報取得
	dm_ptr = GetDataMatCacheData();				// データマットキャッシュメモリ構造体情報取得
	ubm_ptr = GetUserBootMatCacheData();		// ユーザーブートマットキャッシュメモリ構造体情報取得

	pDbgData = GetDbgDataRX();
	pbyReadBuf = pbyBuff;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

	dwTotalLength = madrEndAddr - madrStartAddr + 1;

	// データ格納領域を初期化
	memset(pbyReadBuf, 0, dwTotalLength);

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case MWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case MLWORD_ACCESS:
		dwAddAddr = 4;
		break;
	default:
		dwAddAddr = 4;	// アドレス加算値算出
		break;
	}
	//アクセスバイトサイズをアクセス回数に変換
	dwTotalAccessCount = dwTotalLength / dwAddAddr;

	pPb = GetPbData();						// S/Wブレークポイント設定を取得

	// RevRxNo161003-001 Append Start
	eRmode = GetRmodeDataRX();				// レジスタ設定の起動モードを取得
	pMcuDef = GetMcuDefData();
	// RevRxNo161003-001 Append End

	while (dwTotalAccessCount > 0) {
		// 指定領域判定
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Line
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 0, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (bEnableArea == TRUE) {
// RevRxNo121022-001 Modify Start
			if (dwLength > dwReadLengthMax) {
				dwLength = dwReadLengthMax;
			}
// RevRxNo121022-001 Modify End

			//アクセスバイトサイズをアクセス回数に変換
			dwAccessCount = dwLength / dwAddAddr;

			ferr = GetEndianType2(madrStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start

			// RevRxNo140109-001 Append Line
			// 指定領域が内部ROMであるかを判定
			bInRomArea = IsMcuInRomArea(eAreaType);

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Modify Line
			if (bInRomArea == TRUE) {	// 内蔵ROM領域の場合	// RevRxNo140109-001 Modify Line
				// プログラム実行中はRAMと同じ扱い(キャッシュを見ない)
				//RevNo120727-001 Modify Line
				if (GetMcuRunState() == TRUE) {
					// ユーザーマット書き換えデバッグ無効 かつ、データマット書き換えデバッグ無効の場合
					if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_NON) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_NON)){
						if (eAreaType == MAREA_DATA) {		// データマット領域の場合
							// 対象ブロック情報取得
							GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
							// 対象ブロックの読み出し許可ビットを確認
							ferr = GetRegDataFlashReadBlockEnable(dwBlkNo,&bReadEna);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
							// 読み出し許可の場合
							if(bReadEna==TRUE){
								// 実MCUをダンプ
								ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
								if (ferr != FFWERR_OK) {
									return ferr;
								}
							}
							// 読み出し禁止の場合
							else{
								// V.1.02 RevNo110308-002 Modify Start
								// 読み出し禁止領域だけ0xFFで埋める必要あり、許可領域は実メモリをDUMPしないといけなかったがそうなっていなかった。
								// V.1.02 RevNo110310-002 Modify Line
								memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
								SetDtfMcurunFlg(TRUE);
								// V.1.02 RevNo110308-002 Modify Start
							}
						}
						else {							// データマット領域以外の場合(ユーザーマット/ユーザーブートマット)
							// 実MCUをダンプ
							ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
					// V.1.02 RevNo110629-001 フラッシュ書き換えデバッグ時のP/Eモード確認処理修正 Append Start
					// 	ユーザマット書き換えデバッグのみ有効の場合
					}else if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_NON)){
						// RevRxNo161003-001 Modify Start
						if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {		// 起動バンク指定付モードでない場合
							// ユーザマット書き換えデバッグ有効時 == ROM P/Eモード状態である可能性が高い
							// ROM P/Eモード中はユーザマットもデータマットもリード不可のためダミーデータを返す。
							memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
							SetFlashDbgMcurunFlg(TRUE);
						} else {	// 起動バンク指定付モードの場合
							// 起動バンク側はリード可能
							//
							if (madrStartAddr >= pMcuDef->dwDualStartupBankStartAddr) {		// DUMPアドレスが起動バンク側先頭アドレス以降の場合
								// 実MCUをダンプ
								ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
								if (ferr != FFWERR_OK) {
									return ferr;
								}
							} else {	// オプション設定メモリやデータフラッシュなど
								memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
								SetFlashDbgMcurunFlg(TRUE);
							}
						}
						// RevRxNo161003-001 Modify End
					// 	データマット書き換えデバッグのみ有効の場合
					}else if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_NON) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
						// データマット書き換えデバッグ有効時 == データフラッシュP/Eモード状態である可能性が高い
						// データフラッシュP/Eモード中はデータマットのみリード不可のためデータマット領域以外は実メモリをDUMPする。
						if ((eAreaType != MAREA_DATA) && (eAreaType != MAREA_CONFIG)) {
							// データマット領域でない且つコンフィギュレーション設定領域ではない場合	// RevRxNo140109-001 Modify Line

							// 実MCUをダンプ
							ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}else{		// データマット領域またはコンフィギュレーション設定領域の場合 → ダミーデータ返す
							memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
							SetFlashDbgMcurunFlg(TRUE);
						}

					// ユーザーマット書き換えデバッグ有効 かつ、データマット書き換えデバッグ有効の場合					
					// V.1.02 RevNo110629-001 フラッシュ書き換えデバッグ時のP/Eモード確認処理修正 Append End
					}
					// ユーザーマット書き換えデバッグ有効 または、データマット書き換えデバッグ有効の場合					
					else{
						// RevRxNo161003-001 Modify Start
						if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {		// 起動バンク指定付モードでない場合
							// V.1.02 RevNo110308-002 Modify Start
							// 読み出し禁止領域だけ0xFFで埋める必要あり、許可領域は実メモリをDUMPしないといけなかったがそうなっていなかった。
							// V.1.02 RevNo110310-002 Modify Line
							memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
							SetFlashDbgMcurunFlg(TRUE);
							// V.1.02 RevNo110308-002 Modify Start
						} else {									// 起動バンク指定付モードの場合
							// 起動バンク側はリード可能
							//
							if (madrStartAddr >= pMcuDef->dwDualStartupBankStartAddr) {		// DUMPアドレスが起動バンク側先頭アドレス以降の場合
								// 実MCUをダンプ
								ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
								if (ferr != FFWERR_OK) {
									return ferr;
								}
							} else {	// オプション設定メモリやデータフラッシュなど
								memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
								SetFlashDbgMcurunFlg(TRUE);
							}
						}
						// RevRxNo161003-001 Modify End
					}
				}
	
				// プログラム停止中
				else{
					// データマット書き換えデバッグ有効の場合
					if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) || (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
// データフラッシュ書き換えデバッグ有効時ののみP/Eモードかどうかの確認を行うとしていたが、最終的にどちらかの書き換えデバッグが
// 有効だったら、内蔵フラッシュ(データマット、ユーザマット、ユーザブートマット)へのアクセスはしないようにすべきとの判断でコメントとした。2011.3.9
//					if(pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE){
// → 2011.6.29 ツール技若槻さんから指摘で、データマットがP/Eモードエントリ状態でもユーザマットはリード可能ということが判明。処理修正。
						// FENTRYRレジスタ値取得
						ferr = GetFentryrRegData(&wReadData[0]);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						// V.1.02 RevNo110629-001 フラッシュ書き換えデバッグ時のP/Eモード確認処理修正 Append & Modify Start
						// データマット書き換えデバッグのみ有効の場合
						if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_NON) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
							// データフラッシュP/Eモード状態の場合、データマット領域へのアクセスは禁止のためエラーを返す。
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE) == MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE){	// FENTRYDビットが立っている場合
								if ((eAreaType == MAREA_DATA) || (eAreaType == MAREA_CONFIG)) {
									// DUMP領域がデータマット領域またはコンフィギュレーション設定領域の場合	// RevRxNo140109-001 Modify Line

									return FFWERR_ACC_ROM_PEMODE;
								}
							}
						// ユーザマット書き換えデバッグのみ有効の場合
						}else if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_NON)){
							// ROM P/Eモード状態の場合、ユーザマット、データマット領域ともアクセスは禁止のためエラーを返す。
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_USERMAT_PEMODE) != MCU_REG_FLASH_FENTRYR_NO_PEMODE){			// FENTRYxビットが立っている場合
								// RevRxNo161003-001 Modify Start
								if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {		// 起動バンク指定付モードでない場合
									return FFWERR_ACC_ROM_PEMODE;
								} else {									// 起動バンク指定付モードの場合
									// 起動バンク側はリード可能
									//
									if (madrStartAddr >= pMcuDef->dwDualStartupBankStartAddr) {		// DUMPアドレスが起動バンク側先頭アドレス以降の場合
										// 実MCUをダンプ
										ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
										if (ferr != FFWERR_OK) {
											return ferr;
										}
									} else {	// オプション設定メモリやデータフラッシュなど
										return FFWERR_ACC_ROM_PEMODE;
									}
								}
								// RevRxNo161003-001 Modify End
							}
						// ユーザマット & データマット書き換えデバッグ有効の場合
						}else if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) && (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
							// ROM P/Eモード状態の場合、ユーザマット、データマット領域ともアクセスは禁止のためエラーを返す。
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_USERMAT_PEMODE) != MCU_REG_FLASH_FENTRYR_NO_PEMODE){			// FENTRYxビットが立っている場合
								// RevRxNo161003-001 Modify Start
								if (eRmode < RX_RMODE_SNG_DUAL_BANK0) {		// 起動バンク指定付モードでない場合
									return FFWERR_ACC_ROM_PEMODE;
								} else {									// 起動バンク指定付モードの場合
									// 起動バンク側はリード可能
									//
									if (madrStartAddr >= pMcuDef->dwDualStartupBankStartAddr) {		// DUMPアドレスが起動バンク側先頭アドレス以降の場合
										// 実MCUをダンプ
										ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
										if (ferr != FFWERR_OK) {
											return ferr;
										}
									} else {	// オプション設定メモリやデータフラッシュなど
										return FFWERR_ACC_ROM_PEMODE;
									}
								}
								// RevRxNo161003-001 Modify End
							}
							// データフラッシュP/Eモード状態の場合、データマット領域へのアクセスは禁止のためエラーを返す。
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE) == MCU_REG_FLASH_FENTRYR_DATAMAT_PEMODE){	// FENTRYDビットが立っている場合
								if ((eAreaType == MAREA_DATA) || (eAreaType == MAREA_CONFIG)) {
									// DUMP領域がデータマット領域またはコンフィギュレーション設定領域の場合	// RevRxNo140109-001 Modify Line

									return FFWERR_ACC_ROM_PEMODE;
								}
							}
						}
						// V.1.02 RevNo110629-001 フラッシュ書き換えデバッグ時のP/Eモード確認処理修正 Append & Modify Start
					}
					if (eAreaType == MAREA_DATA) {		// データマット領域の場合
						// 対象ブロック情報取得
						GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);
						ferr = SetRegDataFlashReadEnable(TRUE);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						if(dm_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
							// RevRxNo161003-001 Append Start +4
							ferr = SetEepfclk(TRUE);		// EEPFCLKレジスタ設定
							if (ferr != FFWERR_OK) {
								return ferr;
							}
							// RevRxNo161003-001 Append End

							ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
							if (ferr != FFWERR_OK) {
								return ferr;
							}

							// RevRxNo161003-001 Append Start +4
							ferr = SetEepfclk(FALSE);		// EEPFCLKレジスタ復帰
							if (ferr != FFWERR_OK) {
								return ferr;
							}
							// RevRxNo161003-001 Append End
						}
						else{
							GetFlashRomCacheData(eAreaType,madrStartAddr, dwAccessCount, eAccessSize, pbyReadBuf);
						}
						ferr = SetRegDataFlashReadEnable(FALSE);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
					}
					else if (eAreaType == MAREA_USER) {		// ユーザーマット領域の場合
						// 対象ブロック情報取得
						GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

						if(um_ptr->pbyCacheSetFlag[dwBlkNo] == FALSE){
							ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
						GetFlashRomCacheData(eAreaType,madrStartAddr, dwAccessCount, eAccessSize, pbyReadBuf);
					}
					else if (eAreaType == MAREA_USERBOOT){		// ユーザーブートマット領域の場合	// RevRxNo140109-001 Modify Line
						// 対象ブロック情報取得
						GetFlashRomBlockInf( eAreaType, madrStartAddr, &madrBlkStartAddr, &madrBlkEndAddr, &dwBlkNo, &dwBlkSize);

						if(GetUbmCacheSet() == FALSE){
							ferr = UpdateFlashRomCache(eAreaType, dwBlkNo, madrBlkStartAddr, madrBlkEndAddr, dwBlkSize);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
						GetFlashRomCacheData(eAreaType,madrStartAddr, dwAccessCount, eAccessSize, pbyReadBuf);

					// RevRxNo140109-001 Append Start
					} else {	// コンフィギュレーション設定領域の場合
						// 実MCUをダンプ
						ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
					}
					// RevRxNo140109-001 Append Start
				}
			}
			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

			else {							// フラッシュROM領域以外の場合
// RevNo111121-007 Append Start
				bFcuReadWarning = FALSE;
				if (IsMcuFcuFirmArea(madrStartAddr) == TRUE) {
						// 指定アドレスがFCU ファーム領域の場合
					//RevNo120727-001 Modify Line
					if (GetMcuRunState() == TRUE) {
							// ユーザプログラム実行中の場合

						if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {
								// プログラムROM のフラッシュ書き換えデバッグが「有効」の場合

							// RevRxNo121122-003 Modify Line
							memset(pbyReadBuf, 0xFF, dwLength);	// リードデータを0xFFhに設定
							SetFlashDbgMcurunFlg(TRUE);
							bFcuReadWarning = TRUE;
						}
					} else {
							// ユーザプログラム停止中の場合

						if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {
								// プログラムROM のフラッシュ書き換えデバッグが「有効」の場合

							// FENTRYRレジスタ値取得
							ferr = GetFentryrRegData(&wReadData[0]);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
							if((wReadData[0] & MCU_REG_FLASH_FENTRYR_USERMAT_PEMODE) != MCU_REG_FLASH_FENTRYR_NO_PEMODE){
									// FENTRYxビットが立っている場合

								return FFWERR_ACC_ROM_PEMODE;
							}
						}
					}
				}
// RevNo111121-007 Append End
// RevNo111121-007 Modify Start
				if (bFcuReadWarning == FALSE) {
					ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, 
								bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);			// RevNo111121-007 Modify Line
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
// RevNo111121-007 Modify End
			}

			// S/Wブレークポイントが含まれるかチェック
			for (dwCnt = 0, dwLoopAddr = madrStartAddr; dwLoopAddr < (madrStartAddr + dwLength); dwLoopAddr++, dwCnt++) {
				OffsetCWriteAddr(byEndian, dwLoopAddr, &dwStartOffsetAddr);	// 退避命令はコード並びで取得しているため、
																			// S/Wブレーク設定アドレスの判定はCWRITE時のアドレスで行なう
				for (i = 0; i < pPb->dwSetNum; i++) {
					if ((pPb->dwmadrAddr[i] == dwStartOffsetAddr) && (pPb->eFillState[i] == PB_FILL_COMPLETE)) {
						// V.1.02 RevNo110418-003 Append Start
						if (eAreaType == MAREA_DATA) {		// データマット領域の場合
							ferr = SetRegDataFlashReadEnable(TRUE);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
						// V.1.02 RevNo110418-003 Append End

						// S/Wブレーク設定アドレスに現在書き込まれている内容を取得
						// ※DO_CDUMP処理中なので直接PROT_MCU_DUMPを呼び出す
						ferr = PROT_MCU_DUMP(1, &dwLoopAddr, FALSE, &eAccessSizePb, FALSE, &dwPbAccessCount, &byCmdData, byEndian);
						if (ferr != FFWERR_OK) {
							return ferr;
						}
						// V.1.02 RevNo110418-003 Append Start
						if (eAreaType == MAREA_DATA) {		// データマット領域の場合
							ferr = SetRegDataFlashReadEnable(FALSE);
							if (ferr != FFWERR_OK) {
								return ferr;
							}
						}
						// V.1.02 RevNo110418-003 Append End

						// DBT命令の場合だけ退避命令コードで置換
						if (byCmdData == byDbtCmd) {
							memcpy(&pbyReadBuf[dwCnt], &pPb->byCmdData[i], sizeof(BYTE));
						}
						break;
					}
				}
			}
		} else {	// 有効領域ではない場合
			// V.1.02 RevNo110304-001 Modify Start
			//アクセスバイトサイズをアクセス回数に変更
			dwAccessCount = dwLength / dwAddAddr;
			// break;  <- ここでブレークすると予約領域以降の有効領域の処理ができない
			// V.1.02 RevNo110304-001 Modify End
		}

		dwTotalAccessCount -= dwAccessCount;			// DUMP回数更新
		dwLength = dwAccessCount * dwAddAddr;
		madrStartAddr += dwLength;			// DUMP開始アドレス更新
		if (dwTotalLength > 0) {
			pbyReadBuf += dwLength;			// DUMPデータ格納バッファアドレス更新
		}
	}

	return ferr;
}

//=============================================================================
/**
 * デバッグDMA機能によるCodeWRITE処理
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR cwriteDmaAccess(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
							 enum FFWENM_MACCESS_SIZE eAccessSize,  const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr2 = FFWERR_OK;	// RevRxNo161003-001 Append Line
	DWORD	dwAreaNum;
	DWORD	dwAddAddr;
	DWORD	dwLength;				//アクセスバイトサイズ
	DWORD	dwAccessCount;			//アクセス回数
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BOOL	bSameWriteData;
	BYTE	byEndian;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	DWORD	dwTotalLength;			//アクセスバイトサイズ全体
	DWORD	dwTotalAccessCount;		//アクセス回数全体
	const BYTE*	pbyBuff;
	FFW_PB_DATA* pPb;
	DWORD	i;
	DWORD	dwCnt = 0;
	DWORD	dwLoopAddr;
	BOOL	bRetFillFlg;
	BOOL	bEnableArea = FALSE;
	BOOL	bInRomArea = FALSE;		// RevRxNo140109-001 Append Line

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start
	FFWMCU_DBG_DATA_RX*	pDbgData;

	WORD	wReadData[1];		// 参照データ格納領域

	enum FFWRXENM_MAREA_TYPE eAreaType;
	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

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

	pDbgData = GetDbgDataRX();
	pbyBuff = pbyWriteBuff;
	dwAreaNum = 1;
// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	dwTotalLength = madrEndAddr - madrStartAddr + 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	bSameWriteData = FALSE;

	s_eVerify = eVerify;	// ベリファイチェック有無

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
	//アクセスサイズからアクセス回数に変換
	dwTotalAccessCount = dwTotalLength / dwAddAddr;	// WRITE回数算出

	pPb = GetPbData();						// S/Wブレークポイント設定を取得

// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
	while (dwTotalAccessCount > 0) {
		// 指定領域判定
		// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Line
		ferr = ChkBlockArea(madrStartAddr, madrEndAddr, &dwLength, &eAreaType, 1, &bEnableArea);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (bEnableArea == TRUE) {
// RevRxNo121022-001 Modify Start
			if (dwLength > dwWriteLengthMax) {
				dwLength = dwWriteLengthMax;
			}
// RevRxNo121022-001 Modify End
			//アクセスバイトサイズからアクセス回数に変換
			dwAccessCount = dwLength / dwAddAddr;
			if (dwAccessCount < 1) {
				dwAccessCount = 1;
			}

			// 命令コード復帰/埋め込みフラグ状態を取得
			bRetFillFlg = GetCmdRetFillFlg();
			if (bRetFillFlg == FALSE) {
				// S/Wブレークポイントが含まれるかチェック
				for (dwLoopAddr = madrStartAddr; dwLoopAddr < (madrStartAddr + dwLength); dwLoopAddr++, dwCnt++) {
					for (i = 0; i < pPb->dwSetNum; i++) {
						if ((pPb->dwmadrAddr[i] == dwLoopAddr) && (pPb->eFillState[i] == PB_FILL_COMPLETE)) {
							pPb->eFillState[i] = PB_FILL_SET;
							memcpy(&pPb->byCmdData[i], &pbyWriteBuff[dwCnt], sizeof(BYTE));
							break;
						}
					}
				}
			}

			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify Start

			// RevRxNo140109-001 Append Line
			// 指定領域が内部ROMであるかを判定
			bInRomArea = IsMcuInRomArea(eAreaType);

// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Modify Line
			if (bInRomArea == TRUE) {	// 内蔵ROM領域の場合	// RevRxNo140109-001 Modify Line
				if((pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) || (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE)){
					// FENTRYRレジスタ値取得
					ferr = GetFentryrRegData(&wReadData[0]);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					if(wReadData[0] != MCU_REG_FLASH_FENTRYR_NO_PEMODE){
						return FFWERR_ACC_ROM_PEMODE;
					}
				}
				
				if(eAreaType == MAREA_DATA){		// データマット領域の場合
					// RevRxNo161003-001 Append Start +4
					ferr = SetEepfclk(TRUE);		// EEPFCLKレジスタ設定
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					// RevRxNo161003-001 Append End

					ferr = SetRegDataFlashReadEnable(TRUE);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
					
				ferr = SetFlashRomCacheMem(eAreaType, madrStartAddr, dwAccessCount, eAccessSize, pbyBuff, 0);
				if (ferr != FFWERR_OK) {
					// RevRxNo161003-001 Append Start +10
					if(eAreaType == MAREA_DATA){		// データマット領域の場合
						ferr2 = SetEepfclk(FALSE);		// EEPFCLKレジスタ復帰
						if (ferr2 != FFWERR_OK) {
							return ferr2;
						}

						ferr2 = SetRegDataFlashReadEnable(FALSE);
						if (ferr2 != FFWERR_OK) {
							return ferr2;
						}
					}
					// RevRxNo161003-001 Append End

					return ferr;
				}
	
				if(eAreaType == MAREA_DATA){		// データマット領域の場合
					// RevRxNo161003-001 Append Start +4
					ferr = SetEepfclk(FALSE);		// EEPFCLKレジスタ復帰
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					// RevRxNo161003-001 Append End

					ferr = SetRegDataFlashReadEnable(FALSE);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			
			}
			// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Modify End

			else {												// フラッシュROM領域以外の場合
				byEndian = FFWRX_ENDIAN_LITTLE;
				ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
							bSameAccessCount, &dwAccessCount, bSameWriteData, pbyBuff, pVerifyErr,byEndian);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				// ベリファイチェック
				if (s_eVerify == VERIFY_ON) {
// RevNo111121-003 Modify Start
					// RevRxNo121122-004 Modify Line
					ferr = chkVerifyCwriteData(madrStartAddr, eAccessSize, dwAccessCount, pbyBuff, pVerifyErr);
// RevNo111121-003 Modify End
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {			// ベリファイエラー発生時、処理を終了する。
						return ferr;
					}
				}
			}
		} else {
			//アクセスバイトサイズをアクセス回数に変換
			dwAccessCount = dwLength / dwAddAddr;
		}

// RevNo110415-02 int型で表現できない数値があるためDWORD型へ変更 Modify Line
		dwTotalAccessCount -= dwAccessCount;								// WRITE回数更新
		dwLength = dwAccessCount * dwAddAddr;
		madrStartAddr += dwLength;								// WRITE開始アドレス更新
		pbyBuff += dwLength;
	}

	return ferr;
}

//=============================================================================
/**
 * ベリファイチェック処理(FILLデータ用)
 * @param madrStartAddr 開始アドレス
 * @param eAccessSize   アクセスサイズ(取得サイズ計算,エラー返却用)
 * @param dwAccessCount FILL回数
 * @param pbyWriteBuff  ライトデータ格納用バッファアドレス
 * @param pVerifyErr    ベリファイ結果格納用バッファアドレス
 * @param dwWriteDataSize 書き込みデータサイズ
 * @param byEndian      エンディアン情報
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR chkVerifyFillData(MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, 
						const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr, DWORD dwWriteDataSize, BYTE byEndian)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameAccessCount;
	BYTE*	pbyReadBuf;
	BYTE*	pbyReadBufEsc;
	DWORD	i, j;
// RevNo111121-003 Delete Line  madrSearchAddr を削除
	DWORD	dwSizeCnt = 0;
	DWORD	dwChkLength = 0;		//チェックするバイトサイズ
	BYTE*	pbyWriteData;
	DWORD	dwAccessSizeCnt;		// RevNo111121-003 Append Line
	BYTE	byErrAddrEndian;		// RevNo111121-003 Append Line
// RevRxNo120606-004 Append Start
	MADDR	madrEndAddr;
	DWORD	dwLengthTmp;
	BYTE	byFillAccMeans;			// RevRxNo140616-001 Modify Line
	BYTE	byFillAccType;			// RevRxNo140616-001 Append Line
// RevRxNo120606-004 Append End

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
// RevNo111121-003 Delete Line  madrSearchAddr を削除
	pbyWriteData = (BYTE*)pbyWriteBuff;

// RevRxNo121022-001 Append Start
	FFWE20_EINF_DATA	einfData;
	getEinfData(&einfData);			// エミュレータ情報取得
// RevRxNo121022-001 Append Start

	// ベリファイチェックする最大Byte数をセット
	// V.1.01 No.26 メモリアクセス端数処理対応に伴う修正 Modify Start
	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwChkLength = dwAccessCount;
		dwAccessSizeCnt = 1;		// RevNo111121-003 Append Line
		break;
	case MWORD_ACCESS:
		dwChkLength = dwAccessCount * 2;
		dwAccessSizeCnt = 2;		// RevNo111121-003 Append Line
		break;
	case MLWORD_ACCESS:
		dwChkLength = dwAccessCount * 4;
		dwAccessSizeCnt = 4;		// RevNo111121-003 Append Line
		break;
	// RevNo111121-003 Append Start
	default:
		dwChkLength = dwAccessCount * 4;
		dwAccessSizeCnt = 4;
		break;
	// RevNo111121-003 Append End
	}
	dwSizeCnt = dwWriteDataSize;
	// V.1.01 No.26 メモリアクセス端数処理対応に伴う修正 Modify End

	pbyReadBufEsc = (BYTE*)malloc(dwChkLength);
	// RevRxNo130730-009 Append Start
	if( pbyReadBufEsc == NULL ){
		return FFWERR_MEMORY_NOT_SECURED;
	}
	// RevRxNo130730-009 Append End
	pbyReadBuf = pbyReadBufEsc;
	memset(pbyReadBuf, 0, dwChkLength);

	// 実際に書き込まれたデータ取得(DUMP)
	// RevRxNo120606-004 Modify Start
	// 終了アドレスを算出
	madrEndAddr = madrStartAddr + dwChkLength -1;
	ChkAreaAccessMethod(madrStartAddr,madrEndAddr,&dwLengthTmp,&byFillAccMeans, &byFillAccType);	// RevRxNo140616-001 Modify Line

	// V.1.01 No.26 メモリアクセス端数処理対応に伴う修正 Modify Start
	// (1) ベリファイチェックはバイト単位で比較する
	// (2) chkVerifyFillData()が呼び出された時点のpbyWriteBuff[]のデータ並びはバイト並び
	// (3) DO_DUMP()でデータを取得すると、リトル並び変換されてくる
	// 従って、この関数の中ではDO_DUMPではなくPROT_MCU_DUMPを発行すべき
	if(byFillAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_DDMA)) {	// RevRxNo140616-001 Modify Line
// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){				// EZ-CUBEの場合
			ferr = McuDumpDivide(dwAreaNum, madrStartAddr, bSameAccessSize, eAccessSize,
					bSameAccessCount, dwAccessCount, pbyReadBuf, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
		} else{
			ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
					bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
		}
// RevRxNo121022-001 Append End
	} else if(byFillAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_CPU)) {	// RevRxNo140616-001 Modify Line
// RevRxNo121022-001 Append Start
		if (einfData.wEmuStatus == EML_EZCUBE){			// EZ-CUBEの場合
			ferr = McuCpuDumpDivide(dwAreaNum, madrStartAddr, bSameAccessSize, eAccessSize,
					bSameAccessCount, dwAccessCount, pbyReadBuf, byEndian, PROT_DUMP_LENGTH_MAX_EZ);
		} else{
			ferr = PROT_MCU_CPUDUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
					bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
		}
// RevRxNo121022-001 Append End
	}
	// RevRxNo120606-004 Modify End
	if (ferr != FFWERR_OK) {
		free(pbyReadBufEsc);	// RevRxNo130730-008 Append Line
		return ferr;
	}
	// V.1.01 No.26 メモリアクセス端数処理対応に伴う修正 Modify End

	pVerifyErr->eErrorFlag = VERIFY_OK;	// RevNo120201-001 Append Line

	// ベリファイチェック(1Byteづつチェック)
	for (i = 0, j = 0; i < dwChkLength; i++, j++) {
		// アクセスサイズ分カウントしたらカウンタリセット
		if (dwSizeCnt == j) {
			j = 0;
		}
		// ベリファイエラー発生時
		if (pbyReadBuf[i] != pbyWriteData[j]) {
			pVerifyErr->eErrorFlag = VERIFY_ERR;				// ベリファイエラー発生有無情報
			pVerifyErr->eAccessSize = eAccessSize;				// ベリファイエラー発生時のアクセスサイズ
// RevNo111121-003 Modify Start
			// ベリファイエラー発生時のアドレス
			pVerifyErr->dwErrorAddr = madrStartAddr + (i - (i % dwAccessSizeCnt));

			ferr = GetEndianType2(pVerifyErr->dwErrorAddr, &byErrAddrEndian);
			if (ferr != FFWERR_OK) {
				free(pbyReadBufEsc);	// RevRxNo130730-008 Append Line
				return ferr;
			}

			// ベリファイエラー発生時のライトデータ
			switch (eAccessSize) {
			case MBYTE_ACCESS:
				pVerifyErr->dwErrorWriteData = (DWORD)pbyWriteData[j];
				break;
			case MWORD_ACCESS:
				if (byErrAddrEndian == FFWRX_ENDIAN_LITTLE) {
						// MCU のエンディアンがリトルエンディアンの場合
					pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteData[j - (j % dwAccessSizeCnt) + 1] << 8) | 
													pbyWriteData[j - (j % dwAccessSizeCnt)]);
				} else {
						// MCU のエンディアンがビッグエンディアンの場合
					pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteData[j - (j % dwAccessSizeCnt)] << 8) | 
													pbyWriteData[j - (j % dwAccessSizeCnt) + 1]);
				}
				break;
			case MLWORD_ACCESS:
				if (byErrAddrEndian == FFWRX_ENDIAN_LITTLE) {
						// MCU のエンディアンがリトルエンディアンの場合
					pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteData[j - (j % dwAccessSizeCnt) + 3] << 24) | 
													(pbyWriteData[j - (j % dwAccessSizeCnt) + 2] << 16) | 
													(pbyWriteData[j - (j % dwAccessSizeCnt) + 1] << 8) | 
													pbyWriteData[j - (j % dwAccessSizeCnt)]);
				} else {
						// MCU のエンディアンがビッグエンディアンの場合
					pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteData[j - (j % dwAccessSizeCnt)] << 24) | 
													(pbyWriteData[j - (j % dwAccessSizeCnt) + 1] << 16) | 
													(pbyWriteData[j - (j % dwAccessSizeCnt) + 2] << 8) | 
													pbyWriteData[j - (j % dwAccessSizeCnt) + 3]);
				}
				break;
			}

			// ベリファイエラー発生時のリードデータ
			switch (eAccessSize) {
			case MBYTE_ACCESS:
				pVerifyErr->dwErrorReadData = (DWORD)pbyReadBuf[i];
				break;
			case MWORD_ACCESS:
				if (byErrAddrEndian == FFWRX_ENDIAN_LITTLE) {
						// ベリファイエラー発生時のアドレスのエンディアンがリトルエンディアンの場合
					pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 8) | 
													pbyReadBuf[i - (i % dwAccessSizeCnt)]);
				} else {
						// ベリファイエラー発生時のアドレスのエンディアンがビッグエンディアンの場合
					pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt)] << 8) | 
													pbyReadBuf[i - (i % dwAccessSizeCnt) + 1]);
				}
				break;
			case MLWORD_ACCESS:
				if (byErrAddrEndian == FFWRX_ENDIAN_LITTLE) {
						// ベリファイエラー発生時のアドレスのエンディアンがリトルエンディアンの場合
					pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt) + 3] << 24) | 
													(pbyReadBuf[i - (i % dwAccessSizeCnt) + 2] << 16) | 
													(pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 8) | 
													pbyReadBuf[i - (i % dwAccessSizeCnt)]);
				} else {
						// ベリファイエラー発生時のアドレスのエンディアンがビッグエンディアンの場合
					pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt)] << 24) | 
													(pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 16) | 
													(pbyReadBuf[i - (i % dwAccessSizeCnt) + 2] << 8) | 
													pbyReadBuf[i - (i % dwAccessSizeCnt) + 3]);
				}
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
 * ベリファイチェック処理(WRITE用)
 * @param madrStartAddr 開始アドレス
 * @param eAccessSize   アクセスサイズ(取得サイズ計算,エラー返却用)
 * @param dwAccessCount      ライト回数
 * @param pbyWriteBuff  ライトデータ格納用バッファアドレス
 * @param pVerifyErr    ベリファイ結果格納用バッファアドレス
 * @param byEndian      エンディアン指定(0:リトル、1:ビッグ)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ChkVerifyWriteData(MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, 
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
	DWORD	dwChkLength = 0;		//チェックするバイトサイズ
	DWORD	dwAccessSizeCnt;		// RevNo111121-003 Append Line
	BYTE	byErrAddrEndian;		// RevNo111121-003 Append Line
// RevRxNo120606-004 Append Start
	MADDR	madrEndAddr;
	DWORD	dwLengthTmp;
	BYTE	byWriteAccMeans;		// RevRxNo140616-001 Modify Line
	BYTE	byWriteAccType;			// RevRxNo140616-001 Append Line
// RevRxNo120606-004 Append End

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
	dwChkLength = dwAccessCount * dwSizeCnt;

	pbyReadBufEsc = (BYTE*)malloc(dwChkLength);
	// RevRxNo130730-009 Append Start
	if( pbyReadBufEsc == NULL ){
		return FFWERR_MEMORY_NOT_SECURED;
	}
	// RevRxNo130730-009 Append End
	pbyReadBuf = pbyReadBufEsc;
	memset(pbyReadBuf, 0, dwChkLength);

	// 実際に書き込まれたデータ取得(DUMP)
	// RevRxNo120606-004 Modify Start
	// 終了アドレスを算出
	madrEndAddr = madrStartAddr + dwChkLength -1;
	ChkAreaAccessMethod(madrStartAddr,madrEndAddr,&dwLengthTmp,&byWriteAccMeans, &byWriteAccType);	// RevRxNo140616-001 Modify Line
	if (byWriteAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_DDMA)) {		// RevRxNo140616-001 Modify Line
// RevRxNo121022-001 Append Start
		// 呼び出し側で、アクセスバイト数の分割処理を行っている。
		// ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないのでBFWMCUCmd_DUMPコマンドの
		// リードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
		ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, 
					bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
	} else if (byWriteAccMeans == static_cast<BYTE>(RX_EMU_ACCMEANS_CPU)) {	// RevRxNo140616-001 Modify Line
// RevRxNo121022-001 Append Start
		// 呼び出し側で、アクセスバイト数の分割処理を行っている。
		// ヘッダを含めて0x800hバイトを超える、PROT_MCU_CPUDUMP()関数が呼ばれることはないのでBFWMCUCmd_DUMPコマンドの
		// リードバイト数最大値でアクセスバイト数を分割してPROT_MCU_CPUDUMP()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
		ferr = PROT_MCU_CPUDUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, 
					bSameAccessCount, &dwAccessCount, pbyReadBuf, byEndian);
	}
	// RevRxNo120606-004 Modify End
	if (ferr != FFWERR_OK) {
		free(pbyReadBufEsc);
		return ferr;
	}

	pVerifyErr->eErrorFlag = VERIFY_OK;	// RevNo120201-001 Append Line

	// ベリファイチェック(1Byteづつチェック)
	for (i = 0; i < dwChkLength; i++) {
		// ベリファイエラー発生時
		if (pbyReadBuf[i] != pbyWriteBuff[i]) {
			pVerifyErr->eErrorFlag = VERIFY_ERR;				// ベリファイエラー発生有無情報
			pVerifyErr->eAccessSize = eAccessSize;				// ベリファイエラー発生時のアクセスサイズ
// RevNo111121-003 Modify Start
			// ベリファイエラー発生時のアドレス
			pVerifyErr->dwErrorAddr = madrStartAddr + (i - (i % dwAccessSizeCnt));

			ferr = GetEndianType2(pVerifyErr->dwErrorAddr, &byErrAddrEndian);
			if (ferr != FFWERR_OK) {
				free(pbyReadBufEsc);	// RevRxNo130730-008 Append Line
				return ferr;
			}

			// ベリファイエラー発生時のライトデータ
			switch (eAccessSize) {
			case MBYTE_ACCESS:
				pVerifyErr->dwErrorWriteData = (DWORD)pbyWriteBuff[i];
				break;
			case MWORD_ACCESS:
				if (byErrAddrEndian == FFWRX_ENDIAN_LITTLE) {
						// ベリファイエラー発生時のアドレスのエンディアンがリトルエンディアンの場合
					pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteBuff[i - (i % dwAccessSizeCnt) + 1] << 8) | 
													pbyWriteBuff[i - (i % dwAccessSizeCnt)]);
				} else {
						// ベリファイエラー発生時のアドレスのエンディアンがビッグエンディアンの場合
					pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteBuff[i - (i % dwAccessSizeCnt)] << 8) | 
													pbyWriteBuff[i - (i % dwAccessSizeCnt) + 1]);
				}
				break;
			case MLWORD_ACCESS:
				if (byErrAddrEndian == FFWRX_ENDIAN_LITTLE) {
						// ベリファイエラー発生時のアドレスのエンディアンがリトルエンディアンの場合
					pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteBuff[i - (i % dwAccessSizeCnt) + 3] << 24) | 
													(pbyWriteBuff[i - (i % dwAccessSizeCnt) + 2] << 16) | 
													(pbyWriteBuff[i - (i % dwAccessSizeCnt) + 1] << 8) | 
													pbyWriteBuff[i - (i % dwAccessSizeCnt)]);
				} else {
						// ベリファイエラー発生時のアドレスのエンディアンがビッグエンディアンの場合
					pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteBuff[i - (i % dwAccessSizeCnt)] << 24) | 
													(pbyWriteBuff[i - (i % dwAccessSizeCnt) + 1] << 16) | 
													(pbyWriteBuff[i - (i % dwAccessSizeCnt) + 2] << 8) | 
													pbyWriteBuff[i - (i % dwAccessSizeCnt) + 3]);
				}
				break;
			}

			// ベリファイエラー発生時のリードデータ
			switch (eAccessSize) {
			case MBYTE_ACCESS:
				pVerifyErr->dwErrorReadData = (DWORD)pbyReadBuf[i];
				break;
			case MWORD_ACCESS:
				if (byErrAddrEndian == FFWRX_ENDIAN_LITTLE) {
						// ベリファイエラー発生時のアドレスのエンディアンがリトルエンディアンの場合
					pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 8) | 
													pbyReadBuf[i - (i % dwAccessSizeCnt)]);
				} else {
						// ベリファイエラー発生時のアドレスのエンディアンがビッグエンディアンの場合
					pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt)] << 8) | 
													pbyReadBuf[i - (i % dwAccessSizeCnt) + 1]);
				}
				break;
			case MLWORD_ACCESS:
				if (byErrAddrEndian == FFWRX_ENDIAN_LITTLE) {
						// ベリファイエラー発生時のアドレスのエンディアンがリトルエンディアンの場合
					pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt) + 3] << 24) | 
													(pbyReadBuf[i - (i % dwAccessSizeCnt) + 2] << 16) | 
													(pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 8) | 
													pbyReadBuf[i - (i % dwAccessSizeCnt)]);
				} else {
						// ベリファイエラー発生時のアドレスのエンディアンがビッグエンディアンの場合
					pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt)] << 24) | 
													(pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 16) | 
													(pbyReadBuf[i - (i % dwAccessSizeCnt) + 2] << 8) | 
													pbyReadBuf[i - (i % dwAccessSizeCnt) + 3]);
				}
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
// RevNo111121-003 Append Start
//=============================================================================
/**
 * ベリファイチェック処理(CWRITE用)
 * @param madrStartAddr 開始アドレス
 * @param eAccessSize   アクセスサイズ(取得サイズ計算,エラー返却用)
 * @param dwAccessCount CWRITEアクセス回数
 * @param pbyWriteBuff  ライトデータ格納用バッファアドレス
 * @param pVerifyErr    ベリファイ結果格納用バッファアドレス
  * @return FFWエラーコード
 */
//=============================================================================
static FFWERR chkVerifyCwriteData(MADDR madrStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwAccessCount, 
						const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAreaNum;
	BOOL	bSameAccessSize;
	BOOL	bSameLength;
	BYTE*	pbyReadBuf;
	BYTE*	pbyReadBufEsc;
	DWORD	i;
	DWORD	dwSizeCnt = 0;
	DWORD	dwChkLength = 0;
	DWORD	dwAccessSizeCnt;
	MADDR	madrTmpStartAddr;
	BYTE	byEndian;

	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameLength = FALSE;

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwSizeCnt = 1;
		dwAccessSizeCnt = 1;
		break;
	case MWORD_ACCESS:
		dwSizeCnt = 2;
		dwAccessSizeCnt = 2;
		break;
	case MLWORD_ACCESS:
		dwSizeCnt = 4;
		dwAccessSizeCnt = 4;
		break;
	default:
		dwSizeCnt = 4;
		dwAccessSizeCnt = 4;
		break;
	}

	// ベリファイチェックする最大Byte数をセット
	dwChkLength = dwAccessCount * dwSizeCnt;

	pbyReadBufEsc = (BYTE*)malloc(dwChkLength);
	// RevRxNo130730-009 Append Start
	if( pbyReadBufEsc == NULL ){
		return FFWERR_MEMORY_NOT_SECURED;
	}
	// RevRxNo130730-009 Append End
	pbyReadBuf = pbyReadBufEsc;
	memset(pbyReadBuf, 0, dwChkLength);

	// 実際に書き込まれたデータ取得(リトルエンディアン固定でDUMP)
// RevRxNo121022-001 Append Start
	// 呼び出し側で、アクセスバイト数の分割処理を行っている。
	// ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないのでBFWMCUCmd_DUMPコマンドの
	// リードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
	ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, 
				bSameLength, &dwAccessCount, pbyReadBuf, FFWRX_ENDIAN_LITTLE);
	if (ferr != FFWERR_OK) {
		free(pbyReadBufEsc);
		return ferr;
	}

	pVerifyErr->eErrorFlag = VERIFY_OK;	// RevNo120201-001 Append Line

	// ベリファイチェック(1Byteづつチェック)
	for (i = 0; i < dwChkLength; i++) {
		// ベリファイエラー発生時
		if (pbyReadBuf[i] != pbyWriteBuff[i]) {
			pVerifyErr->eErrorFlag = VERIFY_ERR;				// ベリファイエラー発生有無情報
			pVerifyErr->eAccessSize = eAccessSize;				// ベリファイエラー発生時のアクセスサイズ

			// ベリファイエラー発生時のアドレス
			madrTmpStartAddr = madrStartAddr + (i - (i % dwAccessSizeCnt));

			// ベリファイエラー発生アドレスがビッグエンディアンの場合、
			// 補正されたアドレスを元に戻す
			// ※元に戻したアドレスも同じエンディアンである
			ferr = GetEndianType2(madrTmpStartAddr, &byEndian);
			if (ferr != FFWERR_OK) {
				free(pbyReadBufEsc);	// RevRxNo130730-008 Append Line
				return ferr;
			}

			if (byEndian == FFWRX_ENDIAN_BIG) {
				OffsetStartAddr(eAccessSize, madrTmpStartAddr, (MADDR*)&(pVerifyErr->dwErrorAddr));
			} else {
				pVerifyErr->dwErrorAddr = (DWORD)madrTmpStartAddr;
			}

			// ベリファイエラー発生時のライトデータ
			switch (eAccessSize) {
			case MBYTE_ACCESS:
				pVerifyErr->dwErrorWriteData = (DWORD)pbyWriteBuff[i];
				break;
			case MWORD_ACCESS:
				pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteBuff[i - (i % dwAccessSizeCnt)] << 8) | 
												pbyWriteBuff[i - (i % dwAccessSizeCnt) + 1]);
				break;
			case MLWORD_ACCESS:
				pVerifyErr->dwErrorWriteData = (DWORD)((pbyWriteBuff[i - (i % dwAccessSizeCnt)] << 24) | 
												(pbyWriteBuff[i - (i % dwAccessSizeCnt) + 1] << 16) | 
												(pbyWriteBuff[i - (i % dwAccessSizeCnt) + 2] << 8) | 
												pbyWriteBuff[i - (i % dwAccessSizeCnt)] + 3);
				break;
			}

			// ベリファイエラー発生時のリードデータ
			switch (eAccessSize) {
			case MBYTE_ACCESS:
				pVerifyErr->dwErrorReadData = (DWORD)pbyReadBuf[i];
				break;
			case MWORD_ACCESS:
				pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt)] << 8) | 
												pbyReadBuf[i - (i % dwAccessSizeCnt) + 1]);
				break;
			case MLWORD_ACCESS:
				pVerifyErr->dwErrorReadData = (DWORD)((pbyReadBuf[i - (i % dwAccessSizeCnt)] << 24) | 
												(pbyReadBuf[i - (i % dwAccessSizeCnt) + 1] << 16) | 
												(pbyReadBuf[i - (i % dwAccessSizeCnt) + 2] << 8) | 
												pbyReadBuf[i - (i % dwAccessSizeCnt) + 3]);
				break;
			}
			break;
		}
	}

	free(pbyReadBufEsc);

	return ferr;
}
// RevNo111121-003 Append End

// RevRxNo130301-001 Modify Start
//=============================================================================
/**
 * 指定開始/終了アドレスがRAM領域と重複するかを判定する。
 * @param  madrStartAddr 開始アドレス
 * @param  madrEndAddr   終了アドレス
 * @param  bRamBlockEna	 RAM領域重複有無情報格納配列ポインタ
 * @retval TRUE          RAM領域と重複する
 * @retval FALSE         RAM領域と重複しない
 */
//=============================================================================
BOOL ChkRamArea(MADDR madrStartAddr, MADDR madrEndAddr, BOOL* bRamBlockEna)
{
	BOOL	bRamArea;
	DWORD	i;
	FFWMCU_MCUAREA_DATA_RX*	pMcuArea;

	pMcuArea = GetMcuAreaDataRX();

	bRamArea = FALSE;

	for (i = 0; i < pMcuArea->dwRamAreaNum; i++) {

		if (madrEndAddr < pMcuArea->dwmadrRamStartAddr[i]) {
			// 終了アドレスがRAMの先頭アドレスより小さいかの判定

			bRamBlockEna[i] = FALSE;	// RAM領域と重複しない
		} else if (madrStartAddr > pMcuArea->dwmadrRamEndAddr[i]) {
			// 開始アドレスがRAMの終了アドレスより大きいかの判定

			bRamBlockEna[i] = FALSE;	// RAM領域と重複しない
		} else {
			// 指定領域がRAM領域と重複する

			bRamArea = TRUE;
			bRamBlockEna[i] = TRUE;	// RAM領域と重複する
		}
	}

	return bRamArea;
}
// RevRxNo130301-001 Modify End

//=============================================================================
/**
 * ターゲットMCU制御関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData_Mem(void)
{
	s_eVerify = VERIFY_OFF;

	return;
}


// RevRxNo121022-001 Append Start
//=============================================================================
/**
 * BFWMCUCmd_WRITEコマンドのライトバイト数最大値でアクセスバイト数を分割して
 * PROT_MCU_WRITE()関数を呼び出す(EZ-CUBE用)
 *
 * @param eVerify			べリファイ指定(VERIFY_OFF/VERIFY_ON)(BFWではベリファイをしない)
 * @param dwAreaNum			ライト領域数
 * @param madrWriteAddr		ライト開始アドレス
 * @param bSameAccessSize	同一アクセスサイズ指定フラグ
 * @param eAccessSize		アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameAccessCount	同一ライトアクセス回数指定フラグ
 * @param dwAccessCount		1領域のライトアクセス回数(1～0x10000)
 * @param bSameWriteData	同一ライトデータ指定フラグ
 * @param pbyWriteData		ライトデータ格納用バッファへのポインタ
 * @param pVerifyErrorInfo	ベリファイ結果格納用バッファアドレス(BFWではベリファイをしない)
 * @param byEndian			エンディアン指定(0:リトル、1:ビッグ)
 * @param dwWriteMaxLength	BFWMCUCmd_WRITEコマンドのライトバイト数最大値
 *
 *   ※以下の引数は、必ず固定値で渡すこと。
 *   ・dwAreaNum = 1
 *   ・bSameAccessSize = FALSE
 *   ・bSameLength = FALSE
 *   ・bSameWriteData = FALSE
 *
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR McuWriteDivide(enum FFWENM_VERIFY_SET eVerify, DWORD dwAreaNum, MADDR madrWriteAddr, 
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

		ferr = PROT_MCU_WRITE(eVerify, dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
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


// RevRxNo121022-001 Append Start
//=============================================================================
/**
 * BFWMCUCmd_DUMPコマンドのリードバイト数最大値でアクセスバイト数を分割して
 * PROT_MCU_DUMP()関数を呼び出す(EZ-CUBE用)
 *
 * @param dwAreaNum			リード領域数(1～0x1000)
 * @param madrReadAddr		リード開始アドレス
 * @param bSameAccessSize	同一アクセスサイズ指定フラグ
 * @param eAccessSize		アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameAccessCount	同一リードアクセス回数指定フラグ
 * @param dwAccessCount		1領域のリードアクセス回数(1～0x10000)
 * @param pbyReadData		リードデータ格納用バッファへのポインタ
 * @param byEndian			エンディアン指定(0:リトル、1:ビッグ)
 * @param dwDumpMaxLength	BFWMCUCmd_DUMPコマンドのリードバイト数最大値
 *
 *   ※以下の引数は、必ず固定値で渡すこと。
 *   ・dwAreaNum = 1
 *   ・bSameAccessSize = FALSE
 *   ・bSameAccessCount = FALSE
 *
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR McuDumpDivide(DWORD dwAreaNum, MADDR madrReadAddr, 
					 BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize,
					 BOOL bSameAccessCount, DWORD dwAccessCount, BYTE* pbyReadData,
					 BYTE byEndian, DWORD dwDumpMaxLength)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAddAddr;			// アドレス加算値
	DWORD	dwTotalLength;		// トータルアクセスバイトサイズ
	DWORD	dwLength=0;			// アクセスバイトサイズ
	DWORD	dwReadAccCount;		// アクセス回数		// RevRxNo121022-001(20121203) Modify Line
	MADDR	madrStartAddr;
	BYTE*	pbyBuff;			// DUMPデータ格納バッファのポインタ

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
	madrStartAddr = madrReadAddr;
	// DUMPデータ格納バッファのポインタを取得
	pbyBuff = pbyReadData;

	while (dwTotalLength > 0) {
		if (dwTotalLength > dwDumpMaxLength) {
			dwLength = dwDumpMaxLength;
		}
		else{
			dwLength = dwTotalLength;
		}

		//アクセスバイトサイズをアクセス回数に変換
		// RevRxNo121022-001(20121203) Modify Start
		dwReadAccCount = dwLength/dwAddAddr;
		if (dwReadAccCount < 1) {
			dwReadAccCount = 1;
		}
		// RevRxNo121022-001(20121203) Modify End

		ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
								bSameAccessCount, &dwReadAccCount, pbyBuff, byEndian);	// RevRxNo121022-001(20121203) Modify Line

		if (ferr != FFWERR_OK) {
			return ferr;
		}

		dwTotalLength -= dwLength;			// 残りDUMPサイズ更新
		madrStartAddr += dwLength;			// DUMP開始アドレス更新

		if (dwTotalLength > 0) {
			pbyBuff += dwLength;			// DUMPデータ格納バッファアドレス更新
		}
	}

	return ferr;

}
// RevRxNo121022-001 Append End

// RevRxNo121022-001 Append Start
//=============================================================================
/**
 * BFWMCUCmd_DUMPコマンドのリードバイト数最大値でアクセスバイト数を分割して
 * PROT_MCU_CPUDUMP()関数を呼び出す(EZ-CUBE用)
 *
 * @param dwAreaNum			リード領域数(1～0x1000)
 * @param madrReadAddr		リード開始アドレス
 * @param bSameAccessSize	同一アクセスサイズ指定フラグ
 * @param eAccessSize		アクセスサイズ(BYTE_ACCESS/WORD_ACCESS/LWORD_ACCESS)
 * @param bSameAccessCount	同一リードアクセス回数指定フラグ
 * @param dwAccessCount		1領域のリードアクセス回数(1～0x10000)
 * @param pbyReadData		リードデータ格納用バッファへのポインタ
 * @param byEndian			エンディアン指定(0:リトル、1:ビッグ)
 * @param dwDumpMaxLength	BFWMCUCmd_DUMPコマンドのリードバイト数最大値
 *
 *   ※以下の引数は、必ず固定値で渡すこと。
 *   ・dwAreaNum = 1
 *   ・bSameAccessSize = FALSE
 *   ・bSameAccessCount = FALSE
 *
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR McuCpuDumpDivide(DWORD dwAreaNum, MADDR madrReadAddr, 
					 BOOL bSameAccessSize, enum FFWENM_MACCESS_SIZE eAccessSize,
					 BOOL bSameAccessCount, DWORD dwAccessCount, BYTE* pbyReadData,
					 BYTE byEndian, DWORD dwDumpMaxLength)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwAddAddr;			// アドレス加算値
	DWORD	dwTotalLength;		// トータルアクセスバイトサイズ
	DWORD	dwLength=0;			// アクセスバイトサイズ
	DWORD	dwReadAccCount;		// アクセス回数
	MADDR	madrStartAddr;
	BYTE*	pbyBuff;			// DUMPデータ格納バッファのポインタ

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
	madrStartAddr = madrReadAddr;
	// DUMPデータ格納バッファのポインタを取得
	pbyBuff = pbyReadData;

	while (dwTotalLength > 0) {
		if (dwTotalLength > dwDumpMaxLength) {
			dwLength = dwDumpMaxLength;
		}
		else{
			dwLength = dwTotalLength;
		}

		//アクセスバイトサイズをアクセス回数に変換
		dwReadAccCount = dwLength/dwAddAddr;
		if (dwReadAccCount < 1) {
			dwReadAccCount = 1;
		}

		ferr = PROT_MCU_CPUDUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize,
								bSameAccessCount, &dwReadAccCount, pbyBuff, byEndian);

		if (ferr != FFWERR_OK) {
			return ferr;
		}

		dwTotalLength -= dwLength;			// 残りDUMPサイズ更新
		madrStartAddr += dwLength;			// DUMP開始アドレス更新

		if (dwTotalLength > 0) {
			pbyBuff += dwLength;			// DUMPデータ格納バッファアドレス更新
		}
	}

	return ferr;

}
// RevRxNo121022-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * コンフィギュレーション設定領域へのダウンロードデータ有無フラグクリア
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrExistConfigDwnDataFlg(void)
{
	s_bExistConfigDwnDataFlg = FALSE;		// コンフィギュレーション設定領域へのダウンロードデータなし
}
// RevRxNo140109-001 Append End

// RevRxNo140109-001 Append Start
//=============================================================================
/**
 * コンフィギュレーション設定領域へのダウンロードデータ有無フラグ取得
 * @param なし
 * @retval TRUE :ダウンロードデータあり
 * @retval FALSE:ダウンロードデータなし
 */
//=============================================================================
BOOL GetExistConfigDwnDataFlg(void)
{
	return s_bExistConfigDwnDataFlg;
}
// RevRxNo140109-001 Append End

// RevRxNo140515-005 Append Start
//=============================================================================
/**
 * MEMWAITレジスタ設定を変更したかどうかを示すフラグs_bMemWaitChangeを設定する。
 * @param bMemWaitChange(TRUE：MEMWAIT変更した、FALSE：MEMWAIT変更していない)
 * @retval なし
 */
//=============================================================================
void SetMemWaitChange(BOOL bMemWaitChange)
{
	s_bMemWaitChange = bMemWaitChange;
	return;
}

//=============================================================================
/**
 * OPCCR/SOPCCRレジスタ設定を変更したかどうかを示すフラグs_bOPCModeChangeを設定する。
 * @param bOPCModeChange(TRUE：OPCCR/SOPCCRを変更した、FALSE：OPCCR/SOPCCRを変更していない)
 * @retval なし
 */
//=============================================================================
void SetOPCModeChange(BOOL bOPCModeChange)
{
	s_bOPCModeChange = bOPCModeChange;
	return;
}

//=============================================================================
/**
 * 指定領域(一部またはすべて)がMCUコマンドで指定されたウェイトサイクルが必要な
 * 領域を含むか判定し、ウェイトサイクルが必要な場合、現在のICLK周波数を算出する。
 * @param madrStartAddr		指定領域先頭アドレス
 * @param madrEndAddr		指定領域終了アドレス
 * @retval なし
 */
//=============================================================================
FFWERR ChkMemWaitInsert(MADDR madrStartAddr, MADDR madrEndAddr, BYTE* pbyMemWaitInsert)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUAREA_DATA_RX*		pMcuAreaData;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	float	fSpcICLK;							// SPC変数で定義しているメモリウェイトが不要な周波数最大値

	MADDR						madrStartAddrTmp;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域
	DWORD						dwDataMask;
	DWORD						dwData;

	pMcuAreaData = GetMcuAreaDataRX();
	pMcuInfoData = GetMcuInfoDataRX();

	dwReadData[0] = 0;		// 0クリアしておく(MCUによってMEMWAITレジスタサイズが違うため)
	*pbyMemWaitInsert = NO_MEM_WAIT_CHG;			// メモリウェイト設定変更不要にしておく
	// RevRxNo150528-001 Append Line
	s_fUserICLK = 0.0;		// ユーザICLK周波数値を0.0で初期化しておく
	if (GetMcuRunState() == FALSE) {			// ユーザプログラム停止中の場合
		if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT) == SPC_MEMORY_WAIT) {	// ウェイトサイクル挿入必要の場合
			// RevRxNo150528-001 Append Start
			ChkMwAreaOverlap(madrStartAddr, madrEndAddr, pbyMemWaitInsert);		// 指定アドレスがウェイト必要な領域か判断
			if (*pbyMemWaitInsert == NO_MEM_WAIT_CHG) {		// メモリウェイト挿入不要領域だった
				return ferr;
			}
			// RevRxNo150528-001 Append End

			// MEMWAITレジスタをリードして退避しておく
			madrStartAddrTmp = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
			eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
			pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
			if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX700) {	// RX700系の場合
				ferr = GetMcuSfrReg_CPU(madrStartAddrTmp, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			} else {
				ferr = GetMcuSfrReg(madrStartAddrTmp, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			}
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// RevRxNo150528-001 Modify Start
			// RX24T以外は今までと同じチェックでOK、RX24TはpMcuInfoData->dwMcuRegInfoxxx[5][1]と比較する必要あり
			// RevRxNo150827-001 Modify Line
			if (((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX24T) ||	// RX24Tの場合
				  ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX651)) {	// RX65xの場合
				dwDataMask = pMcuInfoData->dwMcuRegInfoMask[5][1];
				dwData = pMcuInfoData->dwMcuRegInfoData[5][1];
			} else {
				dwDataMask = pMcuInfoData->dwMcuRegInfoMask[5][0];
				dwData = pMcuInfoData->dwMcuRegInfoData[5][0];
			}
			if ((dwReadData[0] & dwDataMask) != dwData) {	// ウェイトなしの設定
				ferr = CalcUserICLK(&s_fUserICLK);			// 現在の周波数算出
				// RevRxNo150827-001 Modify Line
				if (((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX24T) ||	// RX24Tの場合
					  ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX651)) {	// RX65xの場合
					// MEMWAITレジスタ値が0x00、0x01の場合にここに来る
					if (dwReadData[0] == 0x00) {			// MEMWAITレジスタ値が0x00の場合
						fSpcICLK = static_cast<float>(pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_CYCLE);		// SPC変数で指定している周波数1をfloat型に変換
						if (s_fUserICLK <= fSpcICLK) {				// メモリウェイト不要な周波数である場合
							*pbyMemWaitInsert = NO_MEM_WAIT_CHG;	// メモリウェイト設定変更不要に戻しておく
						}
					} else {		// MEMWAITレジスタ値が0x01の場合
						fSpcICLK = static_cast<float>((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_CYCLE2) >> 16);		// SPC変数で指定している周波数2をfloat型に変換
						if (s_fUserICLK <= fSpcICLK) {				// メモリウェイト不要な周波数である場合
							*pbyMemWaitInsert = NO_MEM_WAIT_CHG;	// メモリウェイト設定変更不要に戻しておく
						}
					}
				} else {		// RX24T以外の場合
					fSpcICLK = static_cast<float>(pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_CYCLE);		// SPC変数で指定している周波数をfloat型に変換
					if (s_fUserICLK <= fSpcICLK) {					// メモリウェイト不要な周波数である場合
						*pbyMemWaitInsert = NO_MEM_WAIT_CHG;		// メモリウェイト設定変更不要に戻しておく
					}
				}
			} else {		// ウェイトあり(メモリウェイト設定変更不要)
				*pbyMemWaitInsert = NO_MEM_WAIT_CHG;		// メモリウェイト設定変更不要に戻しておく
			}
			// RevRxNo150528-001 Modify End
			if (*pbyMemWaitInsert != NO_MEM_WAIT_CHG) {		// メモリウェイト設定変更が必要な場合
				s_dwMemwaitData = dwReadData[0];			// MEMWAITレジスタのユーザ設定値を退避しておく
			}
		}
	} else {			// ユーザプログラム実行中の場合
		// ユーザプログラム実行中は現在の周波数が分からないため、メモリウェイト
		// 処理は実施しない。
	}
	return ferr;
}

// RevRxNo150528-001 Append Start
//=============================================================================
/**
 * 指定領域(一部またはすべて)がMCUコマンドで指定されたウェイトサイクルが必要な
 * 領域を含むか判定する
 * @param madrStartAddr		指定領域先頭アドレス
 * @param madrEndAddr		指定領域終了アドレス
 * @retval なし
 */
//=============================================================================
FFWERR ChkMwAreaOverlap(MADDR madrStartAddr, MADDR madrEndAddr, BYTE* pbyMemWaitInsert)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUAREA_DATA_RX*		pMcuAreaData;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	MADDR	madrCmpStart[MCU_AREANUM_MAX_RX];	// 比較用先頭アドレス
	MADDR	madrCmpEnd[MCU_AREANUM_MAX_RX];		// 比較用終了アドレス
	DWORD	dwCmpCnt;
	DWORD	i;
	BOOL	bAreaOverlap;						// 領域重複確認結果

	pMcuAreaData = GetMcuAreaDataRX();
	pMcuInfoData = GetMcuInfoDataRX();

	if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT) == SPC_MEMORY_WAIT) {	// ウェイトサイクル挿入必要の場合
		switch(pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_AREA){
		case SPC_MEMORY_WAIT_AREA_ROM:		// ROMへのウェイトサイクル挿入の場合
			dwCmpCnt = pMcuAreaData->dwFlashRomPatternNum;
			for (i = 0; i < dwCmpCnt; i++) { 
				madrCmpStart[i] = pMcuAreaData->dwmadrFlashRomStart[i];
				madrCmpEnd[i] = pMcuAreaData->dwmadrFlashRomStart[i] + (pMcuAreaData->dwFlashRomBlockSize[i] * pMcuAreaData->dwFlashRomBlockNum[i]) - 1;
			}
			bAreaOverlap = ChkAreaOverlap(madrStartAddr, madrEndAddr, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);
			if (bAreaOverlap == TRUE) {					// 指定領域がROM領域と重複していた場合
				*pbyMemWaitInsert = MEM_WAIT_ROM;
			}
			break;
		case SPC_MEMORY_WAIT_AREA_RAM:		// RAMへのウェイトサイクル挿入の場合
			// RevRxNo180625-001 Modify Start
			// ウェイト挿入が必要なRAM領域数の最大は2であるため、2つ分まで処理できるようにしておく。
			// 領域数が増えた場合のこと考慮したいが、MCUレジスタ定義情報配列の何番目にRAM領域情報を
			// 格納することになるのか確定できないため、処理を先に入れておくことはできない。
			dwCmpCnt = ((pMcuInfoData->dwSpc[2] & SPC_MEMORY_WAIT_RAM_NUM) >> 18) + 1;
			for (i = 0; i < dwCmpCnt; i++) {
				if (i == 0) {
					madrCmpStart[i] = pMcuInfoData->dwmadrMcuRegInfoAddr[6][0];
					madrCmpEnd[i] = pMcuInfoData->dwmadrMcuRegInfoAddr[7][0];
				} else if (i == 1) {
					madrCmpStart[i] = pMcuInfoData->dwmadrMcuRegInfoAddr[12][0];
					madrCmpEnd[i] = pMcuInfoData->dwmadrMcuRegInfoAddr[13][0];
				} else {
					// ここには来ない
				}
			}
			// RevRxNo180713-001 Modify End
			bAreaOverlap = ChkAreaOverlap(madrStartAddr, madrEndAddr, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);
			if (bAreaOverlap == TRUE) {					// 指定領域がメモリウェイトが必要なRAM領域と重複していた場合
				*pbyMemWaitInsert = MEM_WAIT_RAM;
			}
			break;
		case SPC_MEMORY_WAIT_AREA_ALL:		// ROM/RAMへのウェイトサイクル挿入の場合
			// RevRxNo180625-001 Modify Start
			// ウェイト挿入が必要なRAM領域数の最大は2であるため、2つ分まで処理できるようにしておく。
			// 領域数が増えた場合のこと考慮したいが、MCUレジスタ定義情報配列の何番目にRAM領域情報を
			// 格納することになるのか確定できないため、処理を先に入れておくことはできない。
			dwCmpCnt = ((pMcuInfoData->dwSpc[2] & SPC_MEMORY_WAIT_RAM_NUM) >> 18) + 1;
			for (i = 0; i < dwCmpCnt; i++) {
				if (i == 0) {
					madrCmpStart[i] = pMcuInfoData->dwmadrMcuRegInfoAddr[6][0];
					madrCmpEnd[i] = pMcuInfoData->dwmadrMcuRegInfoAddr[7][0];
				} else if (i == 1) {
					madrCmpStart[i] = pMcuInfoData->dwmadrMcuRegInfoAddr[12][0];
					madrCmpEnd[i] = pMcuInfoData->dwmadrMcuRegInfoAddr[13][0];
				} else {
					// ここには来ない
				}
			}
			// RevRxNo180625-001 Modify End
			bAreaOverlap = ChkAreaOverlap(madrStartAddr, madrEndAddr, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);
			if (bAreaOverlap == TRUE) {					// 指定領域がメモリウェイトが必要なRAM領域と重複していた場合
				*pbyMemWaitInsert = MEM_WAIT_RAM;
			}else{										// 指定領域がメモリウェイトが必要なRAM領域と重複していない場合
				dwCmpCnt = pMcuAreaData->dwFlashRomPatternNum;
				for (i = 0; i < dwCmpCnt; i++) { 
					madrCmpStart[i] = pMcuAreaData->dwmadrFlashRomStart[i];
					madrCmpEnd[i] = pMcuAreaData->dwmadrFlashRomStart[i] + (pMcuAreaData->dwFlashRomBlockSize[i] * pMcuAreaData->dwFlashRomBlockNum[i]) - 1;
				}
				bAreaOverlap = ChkAreaOverlap(madrStartAddr, madrEndAddr, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);
				if (bAreaOverlap == TRUE) {				// 指定領域がROM領域と重複していた場合
					*pbyMemWaitInsert = MEM_WAIT_ROM;
				}
			}
			break;
		default:						// ウェイトサイクル挿入なしの場合
			break;						// 何もしない
		}
	}

	return ferr;
}
// RevRxNo150528-001 Append End

//=============================================================================
/**
 * メモリウェイト設定を変更する
 * @param bMemwaitSet(TRUE：ウェイトサイクル挿入、FALSE：ユーザ値に戻す)
 * @retval なし
 * 補足：MEMWAITレジスタは、RX71MはLWORDだが、RX231はBYTEである。このため、
 *       大きい方のサイズに合わせて処理をする。
 */
//=============================================================================
FFWERR SetMemWait(BOOL bMemwaitSet)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	MADDR						madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	DWORD						dwDataMask;
	DWORD						dwData;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE						byReadData[1];		// 参照データ格納領域
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	DWORD						dwWriteData[1];		// 設定データ格納領域
	BYTE						byWriteData[1];		// 設定データ格納領域
	// RevRxNo150528-001 Modify Line
	BOOL						bProtectFree = FALSE;	// プロテクト解除記録用 
	FFWMCU_FWCTRL_DATA*			pFwCtrl;
	// RevRxNo150528-001 Append Line
	float						fSpcICLK;			// SPC変数で指定する判定周波数格納変数

	pFwCtrl = GetFwCtrlData();
	pMcuInfoData = GetMcuInfoDataRX();

	dwDataMask = pMcuInfoData->dwMcuRegInfoMask[5][0];
	dwData = pMcuInfoData->dwMcuRegInfoData[5][0];

	if (bMemwaitSet == TRUE) {					// ウェイトサイクル挿入指定の場合
		if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX700) {	// RX700系の場合
			// MEMWAITレジスタをウェイトありに設定
			madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
			eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
			dwWriteData[0] = (s_dwMemwaitData & ~dwDataMask) | dwData;		// ユーザ値をマスクして、メモリウェイトを設定する。
			pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
			ferr = SetMcuSfrReg_CPU(madrStartAddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_bMemWaitChange = TRUE;
			// RevRxNo180228-001 Append Start
		} else if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX66T) {	// RX66x系の場合
			// MEMWAITレジスタをウェイトありに設定
			madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
			eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
			dwWriteData[0] = (s_dwMemwaitData & ~dwDataMask) | dwData;		// ユーザ値をマスクして、メモリウェイトを設定する。
			pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
			ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_bMemWaitChange = TRUE;
		// RevRxNo180228-001 Append End
		// RevRxNo150827-001 Append Start
		} else if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX651) {	// RX65xの場合
			// ROMWTレジスタをウェイトありに設定
			madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
			eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
			fSpcICLK = static_cast<float>((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_CYCLE2) >> 16);			// SPC変数で指定している周波数2をfloat型に変換
			if (s_fUserICLK > fSpcICLK) {		// ユーザICLK周波数 > ウェイト挿入判定周波数2
				madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][1];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][1];
				dwDataMask = pMcuInfoData->dwMcuRegInfoMask[5][1];
				dwData = pMcuInfoData->dwMcuRegInfoData[5][1];
			} else {
				// 再設定不要
			}
			dwWriteData[0] = (s_dwMemwaitData & ~dwDataMask) | dwData;		// ユーザ値をマスクして、メモリウェイトを設定する。
			pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
			ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_bMemWaitChange = TRUE;
		// RevRxNo150827-001 Append End
		// RevRxNo150528-001 Modify Start
		} else if (((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX231) || 
					((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX24T)) {	// RX231系/RX24Tの場合
			// プロテクト解除
			if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
				ferr = EscMcuPrcrProtect(TRUE,MCU_REG_SYSTEM_PRCR_PRC0,&bProtectFree);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}

			// SOPCCRをリード
			madrStartAddr = MCU_REG_SYSTEM_SOPCCR;
			eAccessSize = MBYTE_ACCESS;
			pbyReadData = reinterpret_cast<BYTE*>(byReadData);
			ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_bySopccrData = byReadData[0];
			// OPCCRをリード
			madrStartAddr = MCU_REG_SYSTEM_OPCCR;
			eAccessSize = MBYTE_ACCESS;
			pbyReadData = reinterpret_cast<BYTE*>(byReadData);
			ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_byOpccrData = byReadData[0];
			if ((s_bySopccrData & MCU_REG_SYSTEM_SOPCCR_LOW) == MCU_REG_SYSTEM_SOPCCR_LOW) {	// 低速動作モードの場合
				// SOPCCRレジスタを高速動作モードまたは中速動作モードに設定
				madrStartAddr = MCU_REG_SYSTEM_SOPCCR;
				eAccessSize = MBYTE_ACCESS;
				pbyWriteData = reinterpret_cast<BYTE*>(byWriteData);
				byWriteData[0] = MCU_REG_SYSTEM_SOPCCR_HIGHMID;			// 高速動作モードまたは中速動作モードに設定
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				// OPCCRレジスタを高速動作モードに設定
				madrStartAddr = MCU_REG_SYSTEM_OPCCR;
				eAccessSize = MBYTE_ACCESS;
				byWriteData[0] = MCU_REG_SYSTEM_OPCCR_HIGH_RX231;		// 高速動作モードに設定
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_bOPCModeChange = TRUE;
			} else if ((s_byOpccrData & MCU_REG_SYSTEM_OPCCR_OPCM) != MCU_REG_SYSTEM_OPCCR_HIGH_RX231) {	// 高速動作モードでない場合
				// OPCCRレジスタを高速動作モードに設定
				madrStartAddr = MCU_REG_SYSTEM_OPCCR;
				eAccessSize = MBYTE_ACCESS;
				pbyWriteData = reinterpret_cast<BYTE*>(byWriteData);
				byWriteData[0] = MCU_REG_SYSTEM_OPCCR_HIGH_RX231;		// 高速動作モードに設定
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_bOPCModeChange = TRUE;
			} else {
				// それ以外は動作モード変更必要なし
			}

			// MEMWAITレジスタをウェイトありに設定
			madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
			eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
			if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX24T) {	// RX24Tの場合
				fSpcICLK = static_cast<float>((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_CYCLE2) >> 16);		// SPC変数で指定している周波数2をfloat型に変換
				if (s_fUserICLK > fSpcICLK) {		// ユーザICLK周波数 > ウェイト挿入判定周波数2
					madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][1];
					eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][1];
					dwDataMask = pMcuInfoData->dwMcuRegInfoMask[5][1];
					dwData = pMcuInfoData->dwMcuRegInfoData[5][1];
				} else {
					// 再設定不要
				}
			}
			dwWriteData[0] = (s_dwMemwaitData & ~dwDataMask) | dwData;		// ユーザ値をマスクして、メモリウェイトを設定する。
			pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
			ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			s_bMemWaitChange = TRUE;

			// プロテクト復帰
			if( bProtectFree ){		// プロテクト解除した場合(プロテクト設定する必要あり)	// RevRxNo160711-001 Modify Line
				ferr = EscMcuPrcrProtect(FALSE,MCU_REG_SYSTEM_PRCR_PRC0,&bProtectFree);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		// RevRxNo150528-001 Modify End

		} else {
			// 現状、RX71MとRX231以外にメモリウェイト必要なものは存在しない。
		}
	} else {		// ウェイトサイクル設定解除(ユーザ値に戻す)
		if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX700) {	// RX700系の場合
			if (s_bMemWaitChange == TRUE) {		// ウエイトありに変更している
				// MEMWAITレジスタをユーザ値に戻す
				madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
				dwWriteData[0] = s_dwMemwaitData;
				pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
				ferr = SetMcuSfrReg_CPU(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
//				s_bMemWaitChange = FALSE;		// FFW層でワーニング発生確認をするためここでは初期化しない
			}
		// RevRxNo180228-001 Append Start
		} else if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX66T) {	// RX66x系の場合
			if (s_bMemWaitChange == TRUE) {		// ウエイトありに変更している
				// MEMWAITレジスタをユーザ値に戻す
				madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
				dwWriteData[0] = s_dwMemwaitData;
				pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		// RevRxNo180228-001 Append End
		// RevRxNo150827-001 Append Start
		} else if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX651) {	// RX65xの場合
			if (s_bMemWaitChange == TRUE) {		// ウエイトありに変更している
				// ROMWTレジスタをユーザ値に戻す
				// 現状メモリウェイト関連レジスタはMEMWAIT 1つであるため、MCUレジスタ定義の配列が複数ある場合でも、
				// アドレスは同じであるため0番目の配列に入っているアドレスを使用する。今後メモリウェイト関連レジスタが
				// 複数になった場合は、ユーザ値に戻す場合にどの配列のアドレスに書き込むのか検討が必要となる。
				madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
				dwWriteData[0] = s_dwMemwaitData;
				pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		// RevRxNo150827-001 Append End
		// RevRxNo150528-001 Modify Start
		} else if (((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX231) || 
					((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX24T)) {	// RX231系/RX24Tの場合
			// プロテクト解除
			if (pFwCtrl->bProtRegExist == TRUE) {	// プロテクトレジスタがあるMCUの場合
				ferr = EscMcuPrcrProtect(TRUE,MCU_REG_SYSTEM_PRCR_PRC0,&bProtectFree);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}

			if (s_bMemWaitChange == TRUE) {		// ウエイトありに変更している
				// MEMWAITレジスタをユーザ値に戻す
				// 現状メモリウェイト関連レジスタはMEMWAIT 1つであるため、MCUレジスタ定義の配列が複数ある場合でも、
				// アドレスは同じであるため0番目の配列に入っているアドレスを使用する。今後メモリウェイト関連レジスタが
				// 複数になった場合は、ユーザ値に戻す場合にどの配列のアドレスに書き込むのか検討が必要となる。
				madrStartAddr = pMcuInfoData->dwmadrMcuRegInfoAddr[5][0];
				eAccessSize = pMcuInfoData->eMcuRegInfoAccessSize[5][0];
				dwWriteData[0] = s_dwMemwaitData;
				pbyWriteData = reinterpret_cast<BYTE*>(dwWriteData);
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
//				s_bMemWaitChange = FALSE;		// FFW層でワーニング発生確認をするためここでは初期化しない
			}
			if (s_bOPCModeChange) {		// 動作モードを変更している
				// OPCCRレジスタをユーザ値に戻す
				madrStartAddr = MCU_REG_SYSTEM_OPCCR;
				eAccessSize = MBYTE_ACCESS;
				byWriteData[0] = s_byOpccrData;
				pbyWriteData = reinterpret_cast<BYTE*>(byWriteData);
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				// SOPCCRレジスタをユーザ値に戻す
				madrStartAddr = MCU_REG_SYSTEM_SOPCCR;
				eAccessSize = MBYTE_ACCESS;
				byWriteData[0] = s_bySopccrData;
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_bOPCModeChange = FALSE;
			}

			// プロテクト復帰
			if( bProtectFree ){		// プロテクト解除した場合(プロテクト設定する必要あり)	// RevRxNo160711-001 Modify Line
				ferr = EscMcuPrcrProtect(FALSE,MCU_REG_SYSTEM_PRCR_PRC0,&bProtectFree);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		// RevRxNo150528-001 Modify End
		} else {
			// 現状、RX71MとRX231以外にメモリウェイト必要なものは存在しない。
		}
	}
	return ferr;
}

//=============================================================================
/**
 * SCKCR/SCKCR3/PLLCR/HOHOCR2レジスタをリードし、現在のICLK周波数を算出する。
 * @param なし
 * @retval fUserIclk(現在のユーザICLK周波数を格納)
 */
//=============================================================================
FFWERR CalcUserICLK(float *pfUserIclk)
{
	FFWERR						ferr = FFWERR_OK;
	float						fIclkDiv;			// SCKCRのICLK分周値格納変数(計算用)
	float						fPllDiv;			// PLLCRのPLL分周値(計算用)
	float						fPllDouble;			// PLLCRのPLL逓倍数(計算用)
	float						fHocoFreq;			// HOCO周波数(計算用)
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	MADDR						madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域
	WORD						wReadData[1];		// 参照データ格納領域
	BYTE						byReadData[1];		// 参照データ格納領域

	pMcuInfoData = GetMcuInfoDataRX();

	*pfUserIclk = 0.0;			// ユーザICLK周波数格納変数を0.0で初期化

	// SCKCRをリード(bit27-24 ICLKの分周値)
	madrStartAddr = MCU_REG_SYSTEM_SCKCR;
	eAccessSize = MLWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
	ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// ICLK分周値を計算しておく
	switch (dwReadData[0] & MCU_REG_SYSTEM_SCKCR_ICK) {
	case MCU_REG_SYSTEM_SCKCR_ICK_1:	// 1分周の場合
		fIclkDiv = 1;
		break;
	case MCU_REG_SYSTEM_SCKCR_ICK_2:	// 2分周の場合
		fIclkDiv = 2;
		break;
	case MCU_REG_SYSTEM_SCKCR_ICK_4:	// 4分周の場合
		fIclkDiv = 4;
		break;
	case MCU_REG_SYSTEM_SCKCR_ICK_8:	// 8分周の場合
		fIclkDiv = 8;
		break;
	case MCU_REG_SYSTEM_SCKCR_ICK_16:	// 16分周の場合
		fIclkDiv = 16;
		break;
	case MCU_REG_SYSTEM_SCKCR_ICK_32:	// 32分周の場合
		fIclkDiv = 32;
		break;
	case MCU_REG_SYSTEM_SCKCR_ICK_64:	// 64分周の場合
		fIclkDiv = 64;
		break;
	default:							// 上記以外の場合は1分周にしておく
		fIclkDiv = 1;
		break;
	}
	// SCKCR3をリード(bit10-8 クロック種別)
	madrStartAddr = MCU_REG_SYSTEM_SCKCR3;
	eAccessSize = MWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo150827-001 Modify Line
	// RevRxNo180228-001 Modify Line
	if (((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX700) ||	// RX700系の場合
		 ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX66T) ||	// RX66x系の場合
		 ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX651)) {	// RX65xの場合
		if ((wReadData[0] & MCU_REG_SYSTEM_SCKCR3_CLKSEL) == MCU_REG_SYSTEM_SCKCR3_PLL) {			// PLLの場合
			// PLLCRをリード(bit1-0 PLLの分周値、bit13-8 PLLの逓倍数)
			madrStartAddr = MCU_REG_SYSTEM_PLLCR;
			eAccessSize = MWORD_ACCESS;
			pbyReadData = reinterpret_cast<BYTE*>(wReadData);
			ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// PLL分周値を計算しておく
			switch (wReadData[0] & MCU_REG_SYSTEM_PLLCR_DIV) {
			case MCU_REG_SYSTEM_PLLCR_1DIV:		// PLL 1分周の場合
				fPllDiv = 1;
				break;
			case MCU_REG_SYSTEM_PLLCR_2DIV:		// PLL 2分周の場合
				fPllDiv = 2; 
				break;
			case MCU_REG_SYSTEM_PLLCR_3DIV: 	// PLL 3分周の場合
				fPllDiv = 3;
				break;
			default:							// 上記以外の場合は1分周にしておく
				fPllDiv = 1;
				break;
			}
			// PLL逓倍数を計算しておく
//			fPllDouble = ((static_cast<float>)((wReadData[0] >> 8) & MCU_REG_SYSTEM_PLLCR_STC_MASK) + (static_cast<float>)1) / (static_cast<float>)2;
			fPllDouble = ((float)((wReadData[0] >> 8) & MCU_REG_SYSTEM_PLLCR_STC_MASK) + (float)1) / (float)2;

			if( (wReadData[0] & MCU_REG_SYSTEM_PLLCR_PLLSRCSEL) == MCU_REG_SYSTEM_PLLCR_HOCO){	// PLLクロックソースがHOCOの場合
				// HOCOCR2をリード
				madrStartAddr = MCU_REG_SYSTEM_HOCOCR2;
				eAccessSize = MBYTE_ACCESS;
				pbyReadData = reinterpret_cast<BYTE*>(byReadData);
				ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				// HOCO周波数を計算しておく
				switch (byReadData[0] & MCU_REG_SYSTEM_HOCOCR2_HCFRQ) {
				case MCU_REG_SYSTEM_HOCOCR2_16_RX700:	// 16MHzの場合
					fHocoFreq = 16.0;
					break;
				case MCU_REG_SYSTEM_HOCOCR2_18_RX700:	// 18MHzの場合
					fHocoFreq = 18.0;
					break;
				case MCU_REG_SYSTEM_HOCOCR2_20_RX700:	// 20MHzの場合
					fHocoFreq = 20.0;
					break;
				default:								// 上記以外の場合は16MHzにしておく
					fHocoFreq = 16.0;
					break;
				}	

				// 現在の周波数を算出
				*pfUserIclk = fHocoFreq / fPllDiv * fPllDouble / fIclkDiv;
			} else {	// PLLクロックソースがEXTALの場合
				// 現在の周波数を算出
				*pfUserIclk = pMcuInfoData->fClkExtal / fPllDiv * fPllDouble / fIclkDiv;
			}
		}
	// RevRxNo150528-001 Modify Line
	} else if (((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX231) || 
				((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX24T)) {		// RX231系/RX24Tの場合
		if ((wReadData[0] & MCU_REG_SYSTEM_SCKCR3_CLKSEL) == MCU_REG_SYSTEM_SCKCR3_HOCO) {					// HOCOの場合
			// HOCOCR2をリード
			madrStartAddr = MCU_REG_SYSTEM_HOCOCR2;
			eAccessSize = MBYTE_ACCESS;
			pbyReadData = reinterpret_cast<BYTE*>(byReadData);
			ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// HOCO周波数を計算しておく
			switch (byReadData[0] & MCU_REG_SYSTEM_HOCOCR2_HCFRQ) {
			case MCU_REG_SYSTEM_HOCOCR2_32_RX231:	// 32MHzの場合
				fHocoFreq = 32.0;
				break;
			// RevRxNo160525-001 Modify Start
			case MCU_REG_SYSTEM_HOCOCR2_54_RX231:	// 54MHzの場合
				if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX24T) {	// RX24Tの場合
					fHocoFreq = 64.0;
				} else {	// RX231/RX23Tの場合
					fHocoFreq = 54.0;
				}
				break;
			// RevRxNo160525-001 Modify End
			default:								// 上記以外の場合は32MHzにしておく
				fHocoFreq = 32.0;
				break;
			}	

			// 現在の周波数を算出
			*pfUserIclk = fHocoFreq / fIclkDiv;
		} else if ((wReadData[0] & MCU_REG_SYSTEM_SCKCR3_CLKSEL) == MCU_REG_SYSTEM_SCKCR3_PLL) {	// PLLの場合
			// PLLCRをリード(bit1-0 PLLの分周値、bit13-8 PLLの逓倍数)
			madrStartAddr = MCU_REG_SYSTEM_PLLCR;
			eAccessSize = MWORD_ACCESS;
			pbyReadData = reinterpret_cast<BYTE*>(wReadData);
			ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// PLL分周値を計算しておく
			switch (wReadData[0] & MCU_REG_SYSTEM_PLLCR_DIV) {
			case MCU_REG_SYSTEM_PLLCR_1DIV:		// PLL 1分周の場合
				fPllDiv = 1;
				break;
			case MCU_REG_SYSTEM_PLLCR_2DIV:		// PLL 2分周の場合
				fPllDiv = 2; 
				break;
			case MCU_REG_SYSTEM_PLLCR_4DIV: 	// PLL 4分周の場合
				fPllDiv = 4;
				break;
			default:							// 上記以外の場合は1分周にしておく
				fPllDiv = 1;
				break;
			}

			// PLL逓倍数を計算しておく
			fPllDouble = ((float)((wReadData[0] >> 8) & MCU_REG_SYSTEM_PLLCR_STC_MASK) + (float)1) / (float)2;

			// 現在の周波数を算出
			// RevRxNo160525-001 Modify Start
			if ((pMcuInfoData->dwSpc[1] & SPC_MEMORY_WAIT_MCUTYPE) == SPC_MEMORY_WAIT_MCUTYPE_RX24T) {	// RX24Tの場合
				// RX24TはクロックソースがHOCOの場合あり
				if ((wReadData[0] & MCU_REG_SYSTEM_PLLCR_PLLSRCSEL_RX24T) == MCU_REG_SYSTEM_PLLCR_HOCO_RX24T) {	// クロックソースがHOCOの場合
					// HOCOCR2をリード
					madrStartAddr = MCU_REG_SYSTEM_HOCOCR2;
					eAccessSize = MBYTE_ACCESS;
					pbyReadData = reinterpret_cast<BYTE*>(byReadData);
					ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
					// HOCO周波数を計算
					switch (byReadData[0] & MCU_REG_SYSTEM_HOCOCR2_HCFRQ) {
					case MCU_REG_SYSTEM_HOCOCR2_32_RX231:	// 32MHzの場合
						fHocoFreq = 32.0;
						break;
					case MCU_REG_SYSTEM_HOCOCR2_54_RX231:	// 64MHzの場合
						fHocoFreq = 64.0;
						break;
					default:								// 上記以外の場合は32MHzにしておく
						fHocoFreq = 32.0;
						break;
					}	
					*pfUserIclk = fHocoFreq / fPllDiv * fPllDouble / fIclkDiv;
				} else {	// クロックソースがEXTALの場合
					*pfUserIclk = pMcuInfoData->fClkExtal / fPllDiv * fPllDouble / fIclkDiv;
				}
			} else {	// RX231/RX23Tの場合
				*pfUserIclk = pMcuInfoData->fClkExtal / fPllDiv * fPllDouble / fIclkDiv;
			}
			// RevRxNo160525-001 Modify End
		} else {
			// HOCO,PLL以外の場合は、周波数がメモリウェイトが必要な値になることがないため計算不要。
		}
	} else {
		// 現状、RX71MとRX231以外にメモリウェイト必要なものは存在しない。
	}
	return ferr;
}

//=============================================================================
/**
 * FFWERR_ROM_WAIT_TMP_CHANGE/FFWERR_RAM_WAIT_TMP_CHANGEワーニング発生記録用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrMemWaitInsertWarningFlg(void)
{
	s_byMemWaitInsert = NO_MEM_WAIT_CHG;		// ワーニング発生していない
}

//=============================================================================
/**
 * FFWERR_ROM_WAIT_TMP_CHANGE/FFWERR_RAM_WAIT_TMP_CHANGEワーニング発生記録用変数の参照
 * @param なし
 * @retval BYTE NO_MEM_WAIT_CHG		メモリウェイト一時的設定不要
 *				MEM_WAIT_ROM	ROMへのメモリウェイト一時的設定あり
 *				MEM_WAIT_RAM	RAMへのメモリウェイト一時的設定あり
*/
//=============================================================================
BYTE GetMemWaitInsertFlg(void)
{
	return s_byMemWaitInsert;
}

// RevRxNo140515-005 Append End

// RevRxNo161003-001 Append Start +54+104+30
//=============================================================================
/**
 * EEPFCLKレジスタを変更する
 * @param bmode(TRUE：EEPFCLK設定、FALSE：ユーザ値に戻す)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR SetEepfclk(BOOL bMode)
{
	FFWERR	ferr = FFWERR_OK;
	MADDR	madrStartAddr;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE*						pbyWriteData;		// 設定データを格納する領域のアドレス
	BYTE						byReadData[1];		// 参照データ格納領域
	BYTE						byWriteData[1];		// 設定データ格納領域
	BYTE						byFclk;				// 算出したFCLK値格納変数
	FFWMCU_FWCTRL_DATA*			pFwCtrl;

	pFwCtrl = GetFwCtrlData();

	pMcuInfoData = GetMcuInfoDataRX();
	if (pFwCtrl->bEepfclkExist)			{	// EEPFCLKレジスタが存在する場合
		if (bMode) {							// EEPFCLKレジスタ設定の場合
			s_byEepfclk = 0x00;
			s_bEepfclkChg = FALSE;
			// EEPFCLKレジスタ値取得
			madrStartAddr = MCU_REG_EEPFCLK;
			eAccessSize = MBYTE_ACCESS;
			pbyReadData = reinterpret_cast<BYTE*>(byReadData);
			ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			memcpy(&s_byEepfclk, pbyReadData, sizeof(BYTE));
			ferr = CalcUserFCLK(&byFclk);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			if (s_byEepfclk != byFclk) {	// 現在のEEPFCLK設定値が算出したFCLK値と異なる場合
				// 算出したFCLK値をEEPFCLKレジスタに設定
				byWriteData[0] = byFclk;
				pbyWriteData = reinterpret_cast<BYTE*>(byWriteData);
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
				s_bEepfclkChg = TRUE;
			}
		} else {								// ユーザ値に戻す
			if (s_bEepfclkChg) {					// EEPFCLKレジスタを変更している
				madrStartAddr = MCU_REG_EEPFCLK;
				eAccessSize = MBYTE_ACCESS;
				byWriteData[0] = s_byEepfclk;
				pbyWriteData = reinterpret_cast<BYTE*>(byWriteData);
				ferr = SetMcuSfrReg(madrStartAddr, eAccessSize, pbyWriteData);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}
	}
	return ferr;
}

//=============================================================================
/**
 * SCKCR/SCKCR3/PLLCRレジスタをリードし、現在のFCLK周波数を算出する。
 * @param pbyFclk(現在のユーザFCLK周波数を格納)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR CalcUserFCLK(BYTE *pbyFclk)
{
	FFWERR						ferr = FFWERR_OK;
	float						fFclkDiv;			// SCKCRのFCLK分周値格納変数(計算用)
	float						fPllDiv;			// PLLCRのPLL入力分周値(計算用)
	float						fPllDouble;			// PLLCRのPLL逓倍数(計算用)
	float						fHocoFreq;			// HOCO周波数(計算用)
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	MADDR						madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域
	WORD						wReadData[1];		// 参照データ格納領域

	pMcuInfoData = GetMcuInfoDataRX();

	*pbyFclk = 0x00;			// ユーザFCLK周波数格納変数を0x00で初期化

	// SCKCRをリード(bit31-28 FCLKの分周値)
	madrStartAddr = MCU_REG_SYSTEM_SCKCR;
	eAccessSize = MLWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
	ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// FCLK分周値を計算しておく
	switch (dwReadData[0] & MCU_REG_SYSTEM_SCKCR_FCK) {
	case MCU_REG_SYSTEM_SCKCR_FCK_1:	// 1分周の場合
		fFclkDiv = 1;
		break;
	case MCU_REG_SYSTEM_SCKCR_FCK_2:	// 2分周の場合
		fFclkDiv = 2;
		break;
	case MCU_REG_SYSTEM_SCKCR_FCK_4:	// 4分周の場合
		fFclkDiv = 4;
		break;
	case MCU_REG_SYSTEM_SCKCR_FCK_8:	// 8分周の場合
		fFclkDiv = 8;
		break;
	case MCU_REG_SYSTEM_SCKCR_FCK_16:	// 16分周の場合
		fFclkDiv = 16;
		break;
	case MCU_REG_SYSTEM_SCKCR_FCK_32:	// 32分周の場合
		fFclkDiv = 32;
		break;
	case MCU_REG_SYSTEM_SCKCR_FCK_64:	// 64分周の場合
		fFclkDiv = 64;
		break;
	default:							// 上記以外の場合は1分周にしておく
		fFclkDiv = 1;
		break;
	}
	// SCKCR3をリード(bit10-8 クロック種別)
	madrStartAddr = MCU_REG_SYSTEM_SCKCR3;
	eAccessSize = MWORD_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(wReadData);
	ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	if ((wReadData[0] & MCU_REG_SYSTEM_SCKCR3_CLKSEL) == MCU_REG_SYSTEM_SCKCR3_PLL) {			// PLLの場合
		// PLLCRをリード(bit1-0 PLLの分周値、bit13-8 PLLの逓倍数)
		madrStartAddr = MCU_REG_SYSTEM_PLLCR;
		eAccessSize = MWORD_ACCESS;
		pbyReadData = reinterpret_cast<BYTE*>(wReadData);
		ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// PLL分周値を計算しておく
		switch (wReadData[0] & MCU_REG_SYSTEM_PLLCR_DIV) {
		case MCU_REG_SYSTEM_PLLCR_1DIV:		// PLL 1分周の場合
			fPllDiv = 1;
			break;
		case MCU_REG_SYSTEM_PLLCR_2DIV:		// PLL 2分周の場合
			fPllDiv = 2; 
			break;
		case MCU_REG_SYSTEM_PLLCR_3DIV: 	// PLL 3分周の場合
			fPllDiv = 3;
			break;
		default:							// 上記以外の場合は1分周にしておく
			fPllDiv = 1;
			break;
		}
		// PLL逓倍数を計算しておく
		fPllDouble = ((float)((wReadData[0] >> 8) & MCU_REG_SYSTEM_PLLCR_STC_MASK) + (float)1) / (float)2;

		if( (wReadData[0] & MCU_REG_SYSTEM_PLLCR_PLLSRCSEL) == MCU_REG_SYSTEM_PLLCR_HOCO){	// PLLクロックソースがHOCOの場合
			ferr = CalcUserHOCO(&fHocoFreq);		// HOCO周波数を計算しておく
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// 現在の周波数を算出
			*pbyFclk = static_cast<BYTE>(fHocoFreq / fPllDiv * fPllDouble / fFclkDiv) + 1;
		} else {	// PLLクロックソースがEXTALの場合
			// 現在の周波数を算出
			*pbyFclk = static_cast<BYTE>(pMcuInfoData->fClkExtal / fPllDiv * fPllDouble / fFclkDiv) + 1;
		}
	} else if ((wReadData[0] & MCU_REG_SYSTEM_SCKCR3_CLKSEL) == MCU_REG_SYSTEM_SCKCR3_LOCO) {	// LOCOの場合
		// 現在の周波数を算出
		*pbyFclk = 1;				// 周波数が1MHz以下のため 1MHzを設定
	} else if ((wReadData[0] & MCU_REG_SYSTEM_SCKCR3_CLKSEL) == MCU_REG_SYSTEM_SCKCR3_HOCO) {	// HOCOの場合
		ferr = CalcUserHOCO(&fHocoFreq);		// HOCO周波数を計算しておく
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// 現在の周波数を算出
		*pbyFclk = static_cast<BYTE>(fHocoFreq / fFclkDiv) + 1;
	} else if ((wReadData[0] & MCU_REG_SYSTEM_SCKCR3_CLKSEL) == MCU_REG_SYSTEM_SCKCR3_SUB) {	// サブクロック発振器の場合
		*pbyFclk = 1;				// 周波数が1MHz以下のため 1MHzを設定
	} else if ((wReadData[0] & MCU_REG_SYSTEM_SCKCR3_CLKSEL) == MCU_REG_SYSTEM_SCKCR3_EXTAL) {	// EXTALの場合
		// 現在の周波数を算出
		*pbyFclk = static_cast<BYTE>(pMcuInfoData->fClkExtal / fFclkDiv) + 1;
	} else {
		return FFWERR_MCUROM_PCLK_ILLEGAL;
	}
	return ferr;
}

//=============================================================================
/**
 * HOCOCR2レジスタをリードし、現在のHOCO周波数を算出する。
 * @param pfHocoFreq(現在のユーザHOCO周波数を格納)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR CalcUserHOCO(float *pfHocoFreq)
{
	FFWERR						ferr = FFWERR_OK;
	MADDR						madrStartAddr;
	enum FFWENM_MACCESS_SIZE	eAccessSize;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	BYTE						byReadData[1];		// 参照データ格納領域

	// HOCOCR2をリード
	madrStartAddr = MCU_REG_SYSTEM_HOCOCR2;
	eAccessSize = MBYTE_ACCESS;
	pbyReadData = reinterpret_cast<BYTE*>(byReadData);
	ferr = GetMcuSfrReg(madrStartAddr, eAccessSize, SFR_ACCESS_COUNT, pbyReadData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// HOCO周波数を計算しておく
	switch (byReadData[0] & MCU_REG_SYSTEM_HOCOCR2_HCFRQ) {
	case MCU_REG_SYSTEM_HOCOCR2_16_RX700:	// 16MHzの場合
		*pfHocoFreq = 16.0;
		break;
	case MCU_REG_SYSTEM_HOCOCR2_18_RX700:	// 18MHzの場合
		*pfHocoFreq = 18.0;
		break;
	case MCU_REG_SYSTEM_HOCOCR2_20_RX700:	// 20MHzの場合
		*pfHocoFreq = 20.0;
		break;
	default:								// 上記以外の場合は16MHzにしておく
		*pfHocoFreq = 16.0;
		break;
	}	

	return ferr;
}	
// RevRxNo161003-001 Append End

