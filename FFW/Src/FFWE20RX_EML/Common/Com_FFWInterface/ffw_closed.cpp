///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffw_closed.cpp
 * @brief 非公開コマンドの実装ファイル
 * @author RSD Y.Minami, H.Hashiguchi, Y.Miyake, H.Akashi, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/02/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/10/15 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
　・FFWE20Cmd_CPURF() の空の関数定義追加。
・RevNo120910-002 2012/11/12 明石
　MONPALLコマンド ダウンロードデータサイズの奇数バイト対応
・RevNo121017-003	2012/10/31 明石
  VS2008対応エラーコード　warninig C4996対応変更
・RevRxE2LNo141104-001 2014/11/21 上田
	E2 Lite対応
	E1/E20 レベル0 旧バージョン(MP版以外)用関数削除。
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
*/
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif

#include "ffw_closed.h"
#include "do_closed.h"
#include "ffw_sys.h"
#include "do_sys.h"
#include "prot_sys.h"
//V.1.02 RevNo110613-001 Append Line
#include "errchk.h"
#include "domcu_prog.h"
#include "emudef.h"	// RevRxE2LNo141104-001 Append Line


// 2009.7.3 INSERT_BEGIN_E20RX600(+NN) {
static DWORD	s_dwMonpAllDataSize;		// MONPALLコマンドでライトするデータの総バイト数
static DWORD	s_dwMonpAllLv0DataSize;		// MONPALLコマンドでライトするLV0データの総バイト数
static DWORD	s_dwMonpAllEmlDataSize;		// MONPALLコマンドでライトするEMLデータの総バイト数
static DWORD	s_dwMonpAllFdtDataSize;		// MONPALLコマンドでライトするFDTデータの総バイト数
static BYTE		s_byMonpAllSum[4];			// MONPALLコマンドでライトするLV1領域(LV0+LV1)の2の補数値
static BYTE		s_byMonpAllLv0Sum[4];		// MONPALLコマンドでライトするLV0領域の2の補数値
static BYTE		s_byMonpAllEmlSum[4];		// MONPALLコマンドでライトするEML領域(LV0+EML)の2の補数値
static BYTE		s_byMonpAllFdtSum[4];		// MONPALLコマンドでライトするFDT領域(LV0+FDT)の2の補数値
static DWORD	s_dwMotLv0Ver;				// MOTファイルのLV0バージョン情報格納(FDT分離版(EML/FDT)から1stMP版(LV1)に書き換える場合に使用)

static FFWERR	s_ferrMonpAllStat;			// MONPALL_OPEN、MONPALL_SENDの実行状況
static BYTE		s_byMonAreaFlg;				// MONP書き込み領域フラグ(bit0：レベル0書き込み、bit1：EML書き込み、bit2：FDT書き込み、bit3：COM書き込み)	
// 2009.7.3 INSERT_END_E20RX600 {
static enum FFWENM_EMLMESCLK_SEL s_eTclk;


// RevRxE2LNo141104-001 Append Start
static FFWERR monpAllSend_E1(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);
static FFWERR monpAllSend_E2(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);
static BOOL checkAreaInclude(DWORD dwChkStartAddr, DWORD dwChkEndAddr, DWORD dwAreaStart, DWORD dwAreaEnd);
// RevRxE2LNo141104-001 Append End

///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * シリアル番号の設定
 * @param dwNum シリアル番号バイト数
 * @param cNumber[] シリアル番号格納配列
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetSELID(DWORD dwNum, const char cNumber[])
{
	FFWERR	ferr = FFWERR_OK;
	char filename[MAX_PATH + 1];
	FFWE20_EINF_DATA	Einf;
	DWORD	dwBufSize;	// RevRxE2LNo141104-001 Append Line

	// シリアル番号バイト数の引数エラーチェック
	if (dwNum + 1 > SELID_NUM_MAX) {
		return FFWERR_FFW_ARG;
	}

	if (dwNum == 0) {
		return FFWERR_OK;
	}

	// RevRxE2LNo141104-001 Modify Start
	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		return ferr;
	}
	// BFW動作モードの確認
	if (Einf.byBfwMode != BFW_LV0) {
		// BFWの動作モードがLv0で無い場合、エラー終了
		ferr = FFWERR_BEXE_LEVEL0;
		return ferr;
	}

	// E1/E20の場合、制御CPUフラッシュ書き換えプログラム(MONPPROG)をダウンロード
	if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
		dwBufSize = static_cast<DWORD>(sizeof(filename));
		GetMonpprogFilename(filename, dwBufSize);	// MONPPROGのファイル名(フルパス)を取得

		// MONPPROGのダウンロード
		if((ferr = Load_wtr_e20(filename)) != FFWERR_OK) {
			return ferr;
		}
	}

	// 製品シリアル番号設定
	if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {	// E1/E20の場合
		if( (ferr = DO_E20_SetSELID(dwNum, cNumber)) != FFWERR_OK) {
			return ferr;
		}
	} else if ((Einf.wEmuStatus == EML_E2) || (Einf.wEmuStatus == EML_E2LITE)) {	// E2/E2 Liteの場合
		if( (ferr = DO_E2_SetSELID(dwNum, cNumber)) != FFWERR_OK) {
			return ferr;
		}
	} else {	// E1/E20/E2/E2 Lite以外の場合
		// 何もしない
	}
	// RevRxE2LNo141104-001 Modify End

	return ferr;
}


//=============================================================================
/**
 * シリアル番号の参照
 * @param pdwNum シリアル番号バイト数格納アドレス
 * @param cNumber シリアル番号格納アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetSELID(DWORD* pdwNum, char cNumber[])
{
	FFWERR	ferr;

	ferr = DO_E20GetSELID(pdwNum,&cNumber[0]);

	return ferr;
}

//=============================================================================
/**
 * モニタCPU 空間のリード
 * @param dweadrStartAddr モニタCPUアドレス
 * @param eAccessSize アクセスサイズ
 * @param dwReadCount 繰り返し回数
 * @param pbyReadBuff リードデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_CPUR(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize,
							  DWORD dwReadCount, BYTE *const pbyReadBuff)
{
	FFWERR	ferr;
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	if (eAccessSize == EWORD_ACCESS) {
		if (dweadrStartAddr % 2) {	// アクセスサイズがワードサイズで、リードアドレスが奇数アドレスの場合
			return FFWERR_FFW_ARG;
		}
	} else if (eAccessSize == ELWORD_ACCESS) {
		if (dweadrStartAddr % 4) {	// アクセスサイズがロングワードサイズで、リードアドレスが4バイト境界以外の場合
			return FFWERR_FFW_ARG;
		}
	}

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

	ferr = DO_CPUR(dweadrStartAddr, eAccessSize, dwReadCount, pbyReadBuff);

	return ferr;
}

//=============================================================================
/**
 * モニタCPU 空間のライト
 * @param dweadrStartAddr モニタCPUアドレス
 * @param eAccessSize アクセスサイズ
 * @param dwWriteCount 繰り返し回数
 * @param pbyWriteBuff ライトデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_CPUW(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
							  DWORD dwWriteCount, BYTE* pbyWriteBuff)
{//拡張機能対応でpbyWriteBuffのcost宣言を暫定で外す
	FFWERR ferr;
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	if (eAccessSize == EWORD_ACCESS) {
		if (dweadrStartAddr % 2) {	// アクセスサイズがワードサイズで、ライトアドレスが奇数アドレスの場合
			return FFWERR_FFW_ARG;
		}
	} else if (eAccessSize == ELWORD_ACCESS) {
		if (dweadrStartAddr % 4) {	// アクセスサイズがロングワードサイズで、ライトアドレスが4バイト境界以外の場合
			return FFWERR_FFW_ARG;
		}
	}

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

	ferr = DO_CPUW(dweadrStartAddr, eAccessSize, dwWriteCount, pbyWriteBuff);

	return ferr;
}

//=============================================================================
/**
 * モニタCPU 空間の単一データライト(FILL)
 * @param dweadrStartAddr モニタCPUアドレス
 * @param eAccessSize アクセスサイズ
 * @param dwWriteCount 繰り返し回数
 * @param pbyWriteBuff ライトデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_CPUF(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
							  DWORD dwWriteCount, const BYTE* pbyWriteBuff)
{
	FFWERR ferr;
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	if (eAccessSize == EWORD_ACCESS) {
		if (dweadrStartAddr % 2) {	// アクセスサイズがワードサイズで、ライトアドレスが奇数アドレスの場合
			return FFWERR_FFW_ARG;
		}
	} else if (eAccessSize == ELWORD_ACCESS) {
		if (dweadrStartAddr % 4) {	// アクセスサイズがロングワードサイズで、ライトアドレスが4バイト境界以外の場合
			return FFWERR_FFW_ARG;
		}
	}

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

	ferr = DO_CPUF(dweadrStartAddr, eAccessSize, dwWriteCount, pbyWriteBuff);

	return ferr;
}

//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムのダウンロード開始
 * @param dwTotalLength FFWCmd_MONPALL_SENDで送信するBFWデータの総バイト数
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONPALL_OPEN(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	char filename[MAX_PATH + 1];
	FFWE20_EINF_DATA	Einf;
	DWORD	dwBufSize;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Modify Start
	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		s_ferrMonpAllStat = ferr;
		return ferr;
	}
	// BFW動作モードの確認
	if (Einf.byBfwMode != BFW_LV0) {
		// BFWの動作モードがLv0で無い場合、エラー終了
		ferr = FFWERR_BEXE_LEVEL0;
		s_ferrMonpAllStat = ferr;
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	// 書き込み領域フラグ
	s_byMonAreaFlg = BFW_MONP_AREA_NON;

	// E1/E20用変数の初期化
	// EML領域の最終3バイトデータ格納変数の初期化
	s_byMonpAllEmlSum[0] = 0xff;		// 2FFFChのライトデータ
	s_byMonpAllEmlSum[1] = 0xff;		// 2FFFDhのライトデータ
	s_byMonpAllEmlSum[2] = 0xff;		// 2FFFEhのライトデータ
	// LV0＋EML領域の総和の2の補数格納変数の初期化
	s_byMonpAllEmlSum[3] = 0x00;		// 2FFFFhのライトデータ
	// FDT領域の最終3バイトデータ格納変数の初期化
	s_byMonpAllFdtSum[0] = 0xff;		// 3FFFChのライトデータ
	s_byMonpAllFdtSum[1] = 0xff;		// 3FFFDhのライトデータ
	s_byMonpAllFdtSum[2] = 0xff;		// 3FFFEhのライトデータ
	// LV0＋FDT領域の総和の2の補数格納変数の初期化
	s_byMonpAllFdtSum[3] = 0x00;		// 3FFFFhのライトデータ
	// Level 0の総和格納変数を初期化
	s_byMonpAllLv0Sum[3] = 0x00;
	// 各領域で書き込みデータのあったデータ数格納変数の初期化
	s_dwMonpAllLv0DataSize = 0;
	s_dwMonpAllEmlDataSize = 0;
	s_dwMonpAllFdtDataSize = 0;
	// MOTファイル内レベル0バージョン格納変数の初期化
	s_dwMotLv0Ver = 0x00000000;


	// RevRxE2LNo141104-001 Modify Start
	// 制御CPUフラッシュ書き換えプログラム(MONPPROG)をダウンロード
	dwBufSize = static_cast<DWORD>(sizeof(filename));
	GetMonpprogFilename(filename, dwBufSize);	// MONPPROGのファイル名(フルパス)を取得

	if ((ferr = Load_wtr_e20(filename)) != FFWERR_OK) {	// MONPPROGのダウンロード
		s_ferrMonpAllStat = ferr;
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	// MONPALL_OPENコマンド送信
	ferr = DO_MONPALL_OPEN(dwTotalLength);

	// MONP_OPENの実行状況をコピー
	s_ferrMonpAllStat = ferr;

	return ferr;
}

//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムデータの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONPALL_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwBfwAreaEndAddr;
	FFWE20_EINF_DATA einfData;	// RevRxE2LNo141104-001 Append Line

	pEmuDef = GetEmuDefData();	// RevRxE2LNo141104-001 Append Line
	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// パラメータエラーチェック
	if (dwLength > MONP_LENGTH_MAX) {
		s_ferrMonpAllStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	// RevRxE2LNo141104-001 Append Start
	dwBfwAreaEndAddr = pEmuDef->dwBfwAreaStartAdr + pEmuDef->dwBfwAreaSize - 1;
	if ((dweadrAddr < pEmuDef->dwBfwAreaStartAdr) || (dweadrAddr > dwBfwAreaEndAddr)) {	
		// 開始アドレスがBFW領域開始アドレス～BFW領域終了アドレス の範囲外の場合
		s_ferrMonpAllStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	if (dwLength > (dwBfwAreaEndAddr - dweadrAddr + 1)) {
		// データバイト数が、(開始アドレス～BFW領域終了アドレス)を超える場合
		s_ferrMonpAllStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	// RevRxE2LNo141104-001 Append End

	// MONPALL SEND処理実行
	// RevRxE2LNo141104-001 Modify Start
	if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {
		ferr = monpAllSend_E1(dweadrAddr, dwLength, pbyBfwBuff);

	} else {
		ferr = monpAllSend_E2(dweadrAddr, dwLength, pbyBfwBuff);
	}
	// RevRxE2LNo141104-001 Modify End

	// MONPALL_SENDの実行状況をコピー
	s_ferrMonpAllStat = ferr;

	return ferr;
}

//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONPALL_CLOSE(void)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwCount;
	DWORD	dwLength;
	BYTE	bySum;
	FFWE20_EINF_DATA einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// MONPALL_OPEN、MONPALL_SENDの実行状況を確認
	if (s_ferrMonpAllStat == FFWERR_OK){
		if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// RevRxE2LNo141104-001 Append Line
			// MONPALL_OPEN、MONPALL_SENDコマンドが正常終了した場合は、チェックサム値をライトする
			if (s_byMonAreaFlg == BFW_MONP_AREA_EML) {			// EML領域への書き込み終了
				// LV0, EML領域の空きデータの総和値を算出
				dwLength = 0x2FFFF - (s_dwMonpAllLv0DataSize + s_dwMonpAllEmlDataSize);
				bySum = (BYTE)(dwLength * 0xFF);
				bySum = (BYTE)(bySum + s_byMonpAllEmlSum[3] + s_byMonpAllLv0Sum[3]);
				s_byMonpAllEmlSum[3] = (BYTE)((~bySum) + 1);
				ferr = DO_MONPALL_SEND( 0x2fffc,4,s_byMonpAllEmlSum );

			} else if (s_byMonAreaFlg == BFW_MONP_AREA_FDT) {		// FDT領域への書き込み終了
				// LV0, FDT領域の空きデータの総和値を算出
				dwLength = 0x1FFFF - (s_dwMonpAllLv0DataSize + s_dwMonpAllFdtDataSize);
				bySum = (BYTE)(dwLength * 0xFF);
				bySum = (BYTE)(bySum + s_byMonpAllFdtSum[3] + s_byMonpAllLv0Sum[3]);
				s_byMonpAllFdtSum[3] = (BYTE)((~bySum) + 1);
				ferr = DO_MONPALL_SEND( 0x3fffc,4,s_byMonpAllFdtSum );
			}
		}	// RevRxE2LNo141104-001 Append Line
	}

	ferr = DO_MONPALL_CLOSE();

	dwCount = GAGE_COUNT_MAX;
	SetGageCount(dwCount);

	return ferr;
}


//==============================================================================
/**
 * 全実行時間測定クロックの設定
 * @param eTclk      全実行時間測定のクロックソースを格納
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_SetEMLMECLK(enum FFWENM_EMLMESCLK_SEL eTclk)
{	
	FFWERR		ferr;
	BYTE		byBuff;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// RevRxE2No170201-001 Modify Line
	if ((einfData.wEmuStatus == EML_E2LITE) || (einfData.wEmuStatus == EML_E2)) {
		return FFWERR_FUNC_UNSUPORT;
	}
	// RevRxE2LNo141104-001 Append End

	if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
		// プログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	//引数チェック
	byBuff = (BYTE)eTclk;
	if ( (TCLK_8_250MHZ >= byBuff) && (byBuff >= TCLK_1_031MHZ ) ){
		return FFWERR_FFW_ARG;
	}

	ferr = DO_SetEMLMECLK(eTclk);
	if(ferr != FFWERR_OK){
		return ferr;
	}
	// s_eTclkに設定値格納
	s_eTclk = eTclk;
	
	return ferr;
}
//==============================================================================
/**
 * 全実行時間測定クロックの参照
 * @param peTclk     全実行時間測定のクロックソースを格納
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetEMLMECLK(enum FFWENM_EMLMESCLK_SEL* peTclk)
{
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// RevRxE2No170201-001 Modify Line
	if ((einfData.wEmuStatus == EML_E2LITE) || (einfData.wEmuStatus == EML_E2)) {
		return FFWERR_FUNC_UNSUPORT;
	}
	// RevRxE2LNo141104-001 Append End

	if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
		// プログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	memcpy(peTclk,&s_eTclk,sizeof(enum FFWENM_EMLMESCLK_SEL));
	
	return FFWERR_OK;
}
//==============================================================================
/**
 * 全実行時間測定データの取得
 * @param  pdwEmlTime SCI FPGA 計測時間
 * @param  pbyEmlOverflow SCI FPGA オーバーフロー結果
 * @param  pu64PcTime PC資源による計測時間
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetEMLMED(DWORD* pdwEmlTime, BYTE* pbyEmlOverflow, UINT64* pu64PcTime)
{	
	FFWERR ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// RevRxE2No170201-001 Modify Line
	if ((einfData.wEmuStatus == EML_E2LITE) || (einfData.wEmuStatus == EML_E2)) {
		return FFWERR_FUNC_UNSUPORT;
	}
	// RevRxE2LNo141104-001 Append End

	if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
		// プログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	ferr = DO_GetEMLMED(pdwEmlTime, pbyEmlOverflow, pu64PcTime);
	
	return ferr;
}
//==============================================================================
/**
 * 全実行時間測定データのクリア
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_ClrEMLMED(void)
{
	FFWERR ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	if (einfData.wEmuStatus == EML_E2LITE) {
		return FFWERR_FUNC_UNSUPORT;
	}
	// RevRxE2LNo141104-001 Append End

	if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
		// プログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	ferr = DO_ClrEMLMED();
	
	return ferr;
}

//=============================================================================
/**
 * E20 USB FIFOバッファの取得（出荷検査用コマンド）
 * @param pFifo USB FIFOバッファ取得データ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetD0FIFO( BYTE *pFifo )
{
	FFWERR ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		return FFWERR_FUNC_UNSUPORT;
	}
	// RevRxE2LNo141104-001 Append End

	ferr = DO_E20GetD0FIFO( pFifo );

	return ferr;
}

// RevRxNo120910-001 Append Start
//=============================================================================
/**
 * モニタCPU空間のリードモディファイライト
 * @param dweadrStartAddr        モニタCPU空間のライトアドレス
 * @param eAccessSize            アクセスサイズ
 * @param dwWriteCount           アクセスサイズ単位でのライト回数
 * @param dwWriteData            書き込みデータを格納
 * @param dwMaskData             マスクデータを格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_CPURF(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									   DWORD dwWriteCount, DWORD dwWriteData, DWORD dwMaskData)
{
// RevRxE2LNo141104-001 Modify Start
	FFWERR ferr = FFWERR_OK;
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	if (eAccessSize == EWORD_ACCESS) {
		if (dweadrStartAddr % 2) {	// アクセスサイズがワードサイズで、ライトアドレスが奇数アドレスの場合
			return FFWERR_FFW_ARG;
		}
	} else if (eAccessSize == ELWORD_ACCESS) {
		if (dweadrStartAddr % 4) {	// アクセスサイズがロングワードサイズで、ライトアドレスが4バイト境界以外の場合
			return FFWERR_FFW_ARG;
		}
	}

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

	dwMaskData = dwMaskData ^ 0xffffffff;

	ferr = DO_CPURF(dweadrStartAddr, eAccessSize, dwWriteCount, dwWriteData, dwMaskData);

	return ferr;
// RevRxE2LNo141104-001 Modify End
}
// RevRxNo120910-001 Append End


///////////////////////////////////////////////////////////////////////////////
// FFW内部関数定義
///////////////////////////////////////////////////////////////////////////////
// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * MONP書き込み領域フラグの参照
 * @param なし
 * @retval MONP書き込み領域フラグ値
 *         bit0:レベル0書き込み有無(1=書き込みあり/0=書き込みなし)
 *         bit1:EML書き込み有無(1=書き込みあり/0=書き込みなし)
 *         bit2:FDT/PRG書き込み有無(1=書き込みあり/0=書き込みなし)
 *         bit3:COM書き込み有無(1=書き込みあり/0=書き込みなし)
 */
//=============================================================================
BYTE GetMonpAreaFlg(void)
{
	return s_byMonAreaFlg;
}
// RevRxE2LNo141104-001 Append End

///////////////////////////////////////////////////////////////////////////////
// static関数定義
///////////////////////////////////////////////////////////////////////////////
// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムデータの送信(E1/E20用)
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR monpAllSend_E1(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	bySum;
	DWORD	dwEmlLen;

	if ((dweadrAddr + dwLength - 1) < 0x10000) {
		for(DWORD dwCount = 0; dwCount < dwLength; dwCount++ ){
			s_byMonpAllLv0Sum[3] = (BYTE)(s_byMonpAllLv0Sum[3] + (*(pbyBfwBuff + dwCount)));
		}
		s_dwMonpAllLv0DataSize += dwLength;
		ferr = DO_MONPALL_SEND(dweadrAddr, dwLength, pbyBfwBuff);
		// MOTファイルのLV0バージョン情報チェック
		if ((dweadrAddr <= 0xFFF0) && (0xFFF0 <= (dweadrAddr + dwLength - 1))) {
			s_dwMotLv0Ver |= *(pbyBfwBuff + (0xFFF0 - dweadrAddr));
			s_dwMotLv0Ver = s_dwMotLv0Ver << 8;
		}
		if ((dweadrAddr <= 0xFFF1) && (0xFFF1 <= (dweadrAddr + dwLength - 1))) {
			s_dwMotLv0Ver |= *(pbyBfwBuff + (0xFFF1 - dweadrAddr));
			s_dwMotLv0Ver = s_dwMotLv0Ver << 8;
		}
		if ((dweadrAddr <= 0xFFF2) && (0xFFF2 <= (dweadrAddr + dwLength - 1))) {
			s_dwMotLv0Ver |= *(pbyBfwBuff + (0xFFF2 - dweadrAddr));
			s_dwMotLv0Ver = s_dwMotLv0Ver << 8;
		}
		if ((dweadrAddr <= 0xFFF3) && (0xFFF3 <= (dweadrAddr + dwLength - 1))) {
			s_dwMotLv0Ver |= *(pbyBfwBuff + (0xFFF3 - dweadrAddr));
		}
	}

	// MOTファイルにEML領域(10000h番地～2FFFEh番地)の内容が含まれているか確認
	if((dweadrAddr >= 0x10000) && ((dweadrAddr + dwLength - 1) < 0x2FFFF)){
		s_byMonAreaFlg |= BFW_MONP_AREA_EML;	// EML領域書き込み有り
	}
	// MOTファイルにFDT領域(30000h番地～3FFFEh番地)の内容が含まれているか確認
	if((dweadrAddr >= 0x30000) && ((dweadrAddr + dwLength - 1) < 0x3FFFF)){
		s_byMonAreaFlg |= BFW_MONP_AREA_FDT;	// FDT領域書き込み有り
	}

	// EML領域の書き込み
	if ((s_byMonAreaFlg & BFW_MONP_AREA_EML) == BFW_MONP_AREA_EML) {
		// FDT領域のデータが送られてきた場合、EMLの最終4バイトの書き込みを行う。
		if ((s_byMonAreaFlg & BFW_MONP_AREA_FDT) == BFW_MONP_AREA_FDT) {
			dwEmlLen = 0x2FFFF - (s_dwMonpAllLv0DataSize + s_dwMonpAllEmlDataSize);
			bySum = (BYTE)(dwEmlLen * 0xFF);
			bySum = (BYTE)(bySum + s_byMonpAllEmlSum[3] + s_byMonpAllLv0Sum[3]);
			s_byMonpAllEmlSum[3] = (BYTE)((~bySum) + 1);
			ferr = DO_MONPALL_SEND( 0x2fffc,4,s_byMonpAllEmlSum );
			s_byMonAreaFlg = (BYTE)(s_byMonAreaFlg & ~BFW_MONP_AREA_EML);	// EML領域書き込み無しに設定
		}
		else {
			// EMLの最終4バイトを含む場合
			if ((dweadrAddr <= 0x2FFFC) && (0x2FFFC <= (dweadrAddr + dwLength - 1))) {
				s_dwMonpAllEmlDataSize += dwLength;
				// 0x2FFFF番地へのライトデータが含まれている場合、0x2FFFF番地のデータはサム値計算に使用しないため、
				// 0x2FFFF番地のデータは無視をする
				if( (dweadrAddr <= 0x2FFFF) && (0x2FFFF <= (dweadrAddr + dwLength - 1)) ){
					s_dwMonpAllEmlDataSize -= 1;
				}
				for(DWORD dwCount = 0; dwCount < dwLength-(dweadrAddr+dwLength-0x2FFFC); dwCount++ ){
					s_byMonpAllEmlSum[3] = (BYTE)(s_byMonpAllEmlSum[3] + *(pbyBfwBuff + dwCount));
				}
				s_byMonpAllEmlSum[0] = *(pbyBfwBuff + (0x2FFFC - dweadrAddr));
				s_byMonpAllEmlSum[3] = (BYTE)(s_byMonpAllEmlSum[3] + *(pbyBfwBuff + (0x2FFFC - dweadrAddr)));
				if( (dweadrAddr <= 0x2FFFD) && (0x2FFFD <= (dweadrAddr + dwLength - 1)) ){
					s_byMonpAllEmlSum[1] = *(pbyBfwBuff + (0x2FFFD - dweadrAddr));
					s_byMonpAllEmlSum[3] = (BYTE)(s_byMonpAllEmlSum[3] + *(pbyBfwBuff + (0x2FFFD - dweadrAddr)));
					if( (dweadrAddr <= 0x2FFFE) && (0x2FFFE <= (dweadrAddr + dwLength - 1)) ){
						s_byMonpAllEmlSum[2] = *(pbyBfwBuff + (0x2FFFE - dweadrAddr));
						s_byMonpAllEmlSum[3] = (BYTE)(s_byMonpAllEmlSum[3] + *(pbyBfwBuff + (0x2FFFE - dweadrAddr)));
					}
				}
				ferr = DO_MONPALL_SEND(dweadrAddr, dwLength-(dweadrAddr+dwLength-0x2FFFC), pbyBfwBuff);		// 最終4バイトはサム値と一緒にプログラムする。
			}
			else {
				for(DWORD dwCount = 0; dwCount < dwLength; dwCount++ ){
					s_byMonpAllEmlSum[3] = (BYTE)(s_byMonpAllEmlSum[3] + *(pbyBfwBuff + dwCount));
				}
				s_dwMonpAllEmlDataSize += dwLength;
				ferr = DO_MONPALL_SEND(dweadrAddr, dwLength, pbyBfwBuff);
			}
		}
	}
	// FDT領域の書き込み
	if (s_byMonAreaFlg == BFW_MONP_AREA_FDT) {
		// FDTの最終4バイトを含む場合
		if ((dweadrAddr <= 0x3FFFC) && (0x3FFFC <= (dweadrAddr + dwLength - 1))) {
			s_dwMonpAllFdtDataSize += dwLength;
			// 0x3FFFF番地へのライトデータが含まれている場合、0x3FFFF番地のデータはサム値計算に使用しないため、
			// 0x3FFFF番地のデータは無視をする
			if( (dweadrAddr <= 0x3FFFF) && (0x3FFFF <= (dweadrAddr + dwLength - 1)) ){
				s_dwMonpAllFdtDataSize -= 1;
			}
			for(DWORD dwCount = 0; dwCount < dwLength-(dweadrAddr+dwLength-0x3FFFC); dwCount++ ){
				s_byMonpAllFdtSum[3] = (BYTE)(s_byMonpAllFdtSum[3] + *(pbyBfwBuff + dwCount));
			}
			s_byMonpAllFdtSum[0] = *(pbyBfwBuff + (0x3FFFC - dweadrAddr));
			s_byMonpAllFdtSum[3] = (BYTE)(s_byMonpAllFdtSum[3] + *(pbyBfwBuff + (0x3FFFC - dweadrAddr)));
			if( (dweadrAddr <= 0x3FFFD) && (0x3FFFD <= (dweadrAddr + dwLength - 1)) ){
				s_byMonpAllFdtSum[1] = *(pbyBfwBuff + (0x3FFFD - dweadrAddr));
				s_byMonpAllFdtSum[3] = (BYTE)(s_byMonpAllFdtSum[3] + *(pbyBfwBuff + (0x3FFFD - dweadrAddr)));
				if( (dweadrAddr <= 0x3FFFE) && (0x3FFFE <= (dweadrAddr + dwLength - 1)) ){
					s_byMonpAllFdtSum[2] = *(pbyBfwBuff + (0x3FFFE - dweadrAddr));
					s_byMonpAllFdtSum[3] = (BYTE)(s_byMonpAllFdtSum[3] + *(pbyBfwBuff + (0x3FFFE - dweadrAddr)));
				}
			}
			ferr = DO_MONPALL_SEND(dweadrAddr, dwLength-(dweadrAddr+dwLength-0x3FFFC), pbyBfwBuff);		// 最終4バイトはサム値と一緒にMONP_CLOSEでプログラムする。
		}
		else {
			for(DWORD dwCount = 0; dwCount < dwLength; dwCount++ ){
				s_byMonpAllFdtSum[3] = (BYTE)(s_byMonpAllFdtSum[3] + *(pbyBfwBuff + dwCount));
			}
			s_dwMonpAllFdtDataSize += dwLength;
			ferr = DO_MONPALL_SEND(dweadrAddr, dwLength, pbyBfwBuff);
		}
	}

	return ferr;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * LEVEL0領域を含むモニタプログラムデータの送信(E2/E2 Lite用)
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR monpAllSend_E2(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	FFWE20_EINF_DATA einfData;
	DWORD	dwEndAddr;
	DWORD	dwAreaStart, dwAreaEnd;
	BOOL	bArea;

	pEmuDef = GetEmuDefData();
	getEinfData(&einfData);

	dwEndAddr = dweadrAddr + dwLength - 1;	// BFWコードデータ送信終了アドレス

	// BFWコードデータの領域チェック
	// レベル0領域
	dwAreaStart = pEmuDef->dwBfwLv0StartAdr;
	dwAreaEnd = pEmuDef->dwBfwLv0StartAdr + pEmuDef->dwBfwLv0Size - 1;
	bArea = checkAreaInclude(dweadrAddr, dwEndAddr, dwAreaStart, dwAreaEnd);
	if (bArea) {
		s_byMonAreaFlg |= BFW_MONP_AREA_LV0;	// レベル0領域書き込みあり
	}
	// EML領域
	dwAreaStart = pEmuDef->dwBfwEmlStartAdr;
	dwAreaEnd = pEmuDef->dwBfwEmlStartAdr + pEmuDef->dwBfwEmlSize - 1;
	bArea = checkAreaInclude(dweadrAddr, dwEndAddr, dwAreaStart, dwAreaEnd);
	if (bArea) {
		s_byMonAreaFlg |= BFW_MONP_AREA_EML;	// EML領域書き込みあり
	}
	// PRG領域
	dwAreaStart = pEmuDef->dwBfwPrgStartAdr;
	dwAreaEnd = pEmuDef->dwBfwPrgStartAdr + pEmuDef->dwBfwPrgSize - 1;
	bArea = checkAreaInclude(dweadrAddr, dwEndAddr, dwAreaStart, dwAreaEnd);
	if (bArea) {
		s_byMonAreaFlg |= BFW_MONP_AREA_FDT;	// PRG領域書き込みあり
	}
	if (einfData.wEmuStatus == EML_E2LITE) {
		// COM領域
		dwAreaStart = pEmuDef->dwBfwComStartAdr;
		dwAreaEnd = pEmuDef->dwBfwComStartAdr + pEmuDef->dwBfwComSize - 1;
		bArea = checkAreaInclude(dweadrAddr, dwEndAddr, dwAreaStart, dwAreaEnd);
		if (bArea) {
			s_byMonAreaFlg |= BFW_MONP_AREA_COM;	// COM領域書き込みあり
		}
	}

	// MONPALL SEND処理実行
	ferr = DO_MONPALL_SEND(dweadrAddr, dwLength, pbyBfwBuff);

	return ferr;
}
// RevRxE2LNo141104-001 Append End

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * 指定開始/終了アドレスが指定領域に一部でも含まれるかの判定
 * @param dwChkStartAddr 指定開始アドレス
 * @param dwChkEndAddr 指定終了アドレス
 * @param dwAreaStart 領域開始アドレス
 * @param dwAreaEnd 領域終了アドレス
 * @retval TRUE 指定開始/終了アドレスが指定領域に一部でも含まれる
 * @retval FALSE 指定開始/終了アドレスが指定領域に含まれない
 */
//=============================================================================
static BOOL checkAreaInclude(DWORD dwChkStartAddr, DWORD dwChkEndAddr, DWORD dwAreaStart, DWORD dwAreaEnd)
{
	BOOL	bArea;

	if ((dwAreaStart <= dwChkStartAddr) && (dwChkStartAddr <= dwAreaEnd)) {	// 開始アドレスが指定領域に含まれる
		bArea = TRUE;

	} else if ((dwAreaStart <= dwChkEndAddr) && (dwChkEndAddr <= dwAreaEnd)) {	// 終了アドレスが指定領域に含まれる
		bArea = TRUE;

	} else if ((dwChkStartAddr < dwAreaStart) && (dwAreaEnd < dwChkEndAddr)) {	// 開始～終了アドレスが指定領域を含む
		bArea = TRUE;

	} else {
		bArea = FALSE;
	}

	return bArea;
}
// RevRxE2LNo141104-001 Append End

///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 非公開コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfData_Closed(void)
{
	return;
}
