///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwe2_closed.cpp
 * @brief 非公開コマンドの実装ファイル
 * @author RSD S.Ueda
 * @author Copyright (C) 2014 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/14
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/14 上田
	新規作成
*/
#include "ffwe2_closed.h"
#include "doe2_closed.h"
#include "ffw_sys.h"
#include "do_sys.h"
#include "emudef.h"


//=============================================================================
/**
 * BFW COMモジュールのダウンロード開始
 * @param dwTotalLength FFWE2Cmd_MONPCOM_SENDで送信するBFWデータの総バイト数
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_MONPCOM_OPEN(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	char filename[MAX_PATH + 1];
	FFWE20_EINF_DATA	Einf;
	DWORD	dwBufSize;

	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		return ferr;
	}

	// エミュレータ種別の確認
	if (Einf.wEmuStatus != EML_E2LITE) {
		return FFWERR_FUNC_UNSUPORT;
	}

	// BFW動作モードの確認
	if (Einf.byBfwMode != BFW_LV0) {
		// BFWの動作モードがLv0で無い場合、エラー終了
		return FFWERR_BEXE_LEVEL0;
	}

	// 制御CPUフラッシュ書き換えプログラム(MONPPROG)をダウンロード
	dwBufSize = static_cast<DWORD>(sizeof(filename));
	GetMonpprogFilename(filename, dwBufSize);	// MONPPROGのファイル名(フルパス)を取得

	if ((ferr = Load_wtr_e20(filename)) != FFWERR_OK) {	// MONPPROGのダウンロード
		return ferr;
	}

	// MONPCOM_OPENコマンド送信
	ferr = DO_MONPCOM_OPEN(dwTotalLength);

	return ferr;
}


//=============================================================================
/**
 * BFW COMモジュールデータの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_MONPCOM_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwBfwAreaEndAddr;
	FFWE20_EINF_DATA einfData;

	pEmuDef = GetEmuDefData();
	getEinfData(&einfData);

	// エミュレータ種別の確認
	if (einfData.wEmuStatus != EML_E2LITE) {
		return FFWERR_FUNC_UNSUPORT;
	}

	// パラメータエラーチェック
	if (dwLength > MONP_LENGTH_MAX) {
		return FFWERR_FFW_ARG;
	}
	dwBfwAreaEndAddr = pEmuDef->dwBfwAreaStartAdr + pEmuDef->dwBfwAreaSize - 1;
	if ((dweadrAddr < pEmuDef->dwBfwAreaStartAdr) || (dweadrAddr > dwBfwAreaEndAddr)) {	
		// 開始アドレスがBFW領域開始アドレス～BFW領域終了アドレス の範囲外の場合
		return FFWERR_FFW_ARG;
	}
	if (dwLength > (dwBfwAreaEndAddr - dweadrAddr + 1)) {
		// データバイト数が、(開始アドレス～BFW領域終了アドレス)を超える場合
		return FFWERR_FFW_ARG;
	}

	// MONPCOM_SENDコマンド実行
	ferr = DO_MONPCOM_SEND(dweadrAddr, dwLength, pbyBfwBuff);

	return ferr;
}

//=============================================================================
/**
 * BFW COMモジュールのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_MONPCOM_CLOSE(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA einfData;

	getEinfData(&einfData);

	// エミュレータ種別の確認
	if (einfData.wEmuStatus != EML_E2LITE) {
		return FFWERR_FUNC_UNSUPORT;
	}

	// MONPCOM_CLOSEコマンド送信
	ferr = DO_MONPCOM_CLOSE();

	SetGageCount(GAGE_COUNT_MAX);

	return ferr;
}

//=============================================================================
/**
 * エミュレータACT-LED制御
 * @param dwActLedControl ACT-LED点灯/消灯指定
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_SetACTLED(DWORD dwActLedControl)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	Einf;

	// エミュレータ種別取得
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		return ferr;
	}

	// 対応エミュレータチェック
	if ((Einf.wEmuStatus != EML_E2) && (Einf.wEmuStatus != EML_E2LITE)) {
		return FFWERR_FUNC_UNSUPORT;
	}

	// 引数エラーチェック
	if ((dwActLedControl != ACTLED_OFF) && (dwActLedControl != ACTLED_ON)) {
		return FFWERR_FFW_ARG;
	}

	ferr = DO_SetACTLED(dwActLedControl);

	return ferr;
}


//=============================================================================
/**
 * ライセンス情報の設定
 * @param dwOffsetAddr ライセンス情報設定開始アドレス(オフセットアドレス)
 * @param dwLength ライセンス情報設定データのバイト数
 * @param pbyData ライセンス情報設定データの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_SetLID(DWORD dwOffsetAddr, DWORD dwLength, const BYTE* pbyData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;

	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	ferr = DO_GetE20EINF(&einfData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 対応エミュレータチェック
	if ((einfData.wEmuStatus != EML_E2) && (einfData.wEmuStatus != EML_E2LITE)) {
		return FFWERR_FUNC_UNSUPORT;
	}

	// BFW動作モードチェック
	if (einfData.byBfwMode != BFW_LV0) {
		return FFWERR_BEXE_LEVEL0;
	}

	// 引数エラーチェック
	if (dwOffsetAddr > (LID_AREA_SIZE - 1)) {
		return FFWERR_FFW_ARG;
	}
	if ((dwLength < LID_LENGTH_MIN) || (dwLength > LID_LENGTH_MAX)) {
		return FFWERR_FFW_ARG;
	}

	ferr = DO_SetLID(dwOffsetAddr, dwLength, pbyData);

	return ferr;
}

//=============================================================================
/**
 * ライセンス情報の参照
 * @param dwOffsetAddr ライセンス情報参照開始アドレス(オフセットアドレス)
 * @param dwLength ライセンス情報参照データのバイト数
 * @param pbyData ライセンス情報参照データの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_GetLID(DWORD dwOffsetAddr, DWORD dwLength, BYTE *const pbyData)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;

	// エミュレータ種別取得のため、EINFコマンド処理を実行する
	ferr = DO_GetE20EINF(&einfData);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// 対応エミュレータチェック
	if ((einfData.wEmuStatus != EML_E2) && (einfData.wEmuStatus != EML_E2LITE)) {
		return FFWERR_FUNC_UNSUPORT;
	}

	// 引数エラーチェック
	if (dwOffsetAddr > (LID_AREA_SIZE - 1)) {
		return FFWERR_FFW_ARG;
	}
	if ((dwLength < LID_LENGTH_MIN) || (dwLength > LID_LENGTH_MAX)) {
		return FFWERR_FFW_ARG;
	}

	ferr = DO_GetLID(dwOffsetAddr, dwLength, pbyData);

	return ferr;
}


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
void InitFfwIfE2Data_Closed(void)
{
	return;
}
