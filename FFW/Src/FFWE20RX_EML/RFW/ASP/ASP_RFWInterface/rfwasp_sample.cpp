///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_sample.cpp
 * @brief 非公開コマンドの実装ファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2017/02/07
*/
#include <windows.h>
#include <stdio.h>

#include "rfwasp_sample.h"
#include "doasp_sample.h"
#include "do_syserr.h"
#include "emudef.h"
#include "asp_setting.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"
#include "rfw_bitops.h"
#include "rfwasp_family.h"

///////////////////////////////////////////////////////////////////////////////
// RFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/*
 * SetSamplingSource
 * @param dwSrc サンプリングする対象
 * @retval RFWERR_OK
 * @retval RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_SetSamplingSource)(DWORD dwSrc)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetSamplingSource_log_enter(dwSrc);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETSAMPSRC);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if ((dwSrc & (DWORD)(~(RF_ASPSRC_POWER | RF_ASPSRC_MON_CH0 | RF_ASPSRC_MON_CH1 | RF_ASPSRC_FAMILY))) != 0) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetSamplingSource(dwSrc);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETSAMPSRC);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_SetSamplingSource_log_leave(dwSrc, dwRet);
	return dwRet;
}


//=============================================================================
/*
 * GetSample
 * @param dwType 記録データ種別
 * @param dwKeyType 0:時間指定 1:変化点数指定
 * @param u64Start 開始時間/開始変化点
 * @param u64End 終了時間/終了変化点
 * @param pData 格納領域
 * @param dwSize 格納領域のサイズ
 * @param pdwStoreCnt 格納領域に格納した個数
 * @param pdwExistCnt 指定範囲に存在する個数/0の場合はdwSizeまで格納したらそれ以上検索しない
 * @param pu64Read 格納した次の有効時間/変化点数
 * @retval RFWERR_OK
 * @retval RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_GetSample)(
    DWORD dwType,
    DWORD dwKeyType,
    UINT64 u64Start,
    UINT64 u64End,
    void *pData,
    DWORD dwSize,
    DWORD *pdwStoreCnt,
    DWORD *pdwExistCnt,
    UINT64 *pu64Read)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetSample_log_enter(dwType, dwKeyType, u64Start, u64End, pData, dwSize, pdwStoreCnt, pdwExistCnt, pu64Read);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETSAMP);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}


	// 品種固有のエラーチェック処理
	if (dwRet == RFWERR_OK) {
		dwRet = checkGetSample(dwType, dwKeyType, u64Start, u64End);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	if (dwRet == RFWERR_OK) {
		if (population_count(dwType) != 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (dwKeyType != 0 && dwKeyType != 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (u64Start > u64End) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (!pData || !dwSize) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (!pdwStoreCnt) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspGetSample(
		            dwType,
		            dwKeyType,
		            u64Start,
		            u64End,
		            pData,
		            dwSize,
		            pdwStoreCnt,
		            pdwExistCnt,
		            pu64Read);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETSAMP);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_GetSample_log_leave(dwType, dwKeyType, u64Start, u64End, pData, dwSize, pdwStoreCnt, pdwExistCnt, pu64Read, dwRet);
	return dwRet;
}


//=============================================================================
/*
 * SetSaveFileName
 * @param fname ファイル名
 * @retval RFWERR_OK
 * @retval RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_SetSaveFileName)(const char *fname)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetSaveFileName_log_enter(fname);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETSAVEFILENAME);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		if (!fname) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspSetSaveFileName(fname);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETSAVEFILENAME);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_SetSaveFileName_log_leave(fname, dwRet);
	return dwRet;
}

//=============================================================================
/*
 * SaveFile
 * @param なし
 * @retval RFWERR_OK
 * @retval RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_SaveFile)(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SaveFile_log_enter();

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SAVEFILE);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspSaveFile();
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SAVEFILE);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_SaveFile_log_leave(dwRet);
	return dwRet;
}

//=============================================================================
/*
 * LoadSample
 * @param fname ロードするファイル名
 * @retval RFWERR_OK
 * @retval RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_LoadSample)(const char *fname)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_LoadSample_log_enter(fname);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_LOADSAMPLE);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		if (!fname) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspLoadSample(fname);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_LOADSAMPLE);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_LoadSample_log_leave(fname, dwRet);
	return dwRet;
}

//=============================================================================
/*
 * UnLoadSample
 * @param なし
 * @retval RFWERR_OK
 * @retval RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_UnLoadSample)(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_UnLoadSample_log_enter();

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_UNLOADSAMPLE);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspUnLoadSample();
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_UNLOADSAMPLE);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_UnLoadSample_log_leave(dwRet);
	return dwRet;
}


//=============================================================================
/*
 * SetRealTimeBufferSize
 * @param dwSize PCストレージの時に確保するバッファサイズ
 * @return RFWERR_OK
 * @return RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_SetRealTimeBufferSize)(DWORD dwSize)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETBUFSIZE);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		// ダミー関数
		if (dwSize == 0) {
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETBUFSIZE);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	return dwRet;
}


//=============================================================================
/*
 * SetRealTimeSamplingType
 * @param dwType 記録データ種別
 * @return RFWERR_OK
 * @return RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_SetRealTimeSamplingType)(DWORD dwType)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETSAMPTYPE);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		// ダミー関数
		if (dwType == 0) {
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETSAMPTYPE);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	return dwRet;
}


//=============================================================================
/*
 * StopSample
 * @param なし
 * @return RFWERR_OK
 * @return RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_StopSample)(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_STOPSAMP);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		// ダミー関数
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_STOPSAMP);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	return dwRet;
}


//=============================================================================
/*
 * RestartSample
 * @param なし
 * @return RFWERR_OK
 * @return RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_RestartSample)(void)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_RESTARTSAMP);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		// ダミー関数
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_RESTARTSAMP);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	return dwRet;
}


//=============================================================================
/*
 * GetAvailableTime
 * @param dwType 記録データ種別
 * @param pu64Start 有効なデータ範囲のタイムスタンプ
 * @param pu64End 有効なデータ範囲のタイムスタンプ
 * @param pu64sample有効なデータ範囲の記録数
 * @return RFWERR_OK
 * @return RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_GetAvailableTime)(DWORD dwType, UINT64 *const pu64Start, UINT64 *const pu64End, UINT64 *const pu64Sample)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetAvailableTime_log_enter(dwType, pu64Start, pu64End, pu64Sample);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETAVAILTIME);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// 品種固有のエラーチェック処理
	if (dwRet == RFWERR_OK) {
		dwRet = checkGetAvailableTime(dwType);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if ((population_count(dwType) != 1)
		        || (pu64Start == NULL)
		        || (pu64End == NULL)
		        || (pu64Sample == NULL)) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetAvailableTime(dwType, pu64Start, pu64End, pu64Sample);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETAVAILTIME);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_GetAvailableTime_log_leave(dwType, pu64Start, pu64End, pu64Sample, dwRet);
	return dwRet;
}


//=============================================================================
/*
 * GetSampleDataSize
 * @param dwType 記録データ種別
 * @param dwKeyType 0:時間指定 1:変化点数指定
 * @param u64Start 開始時間/開始変化点
 * @param u64End 終了時間/終了変化点
 * @param pdwDataSize データサイズ
 * @return RFWERR_OK
 * @return RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_GetSampleDataSize)(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, UINT64 *const pu64DataSize)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetSampleDataSize_log_enter(dwType, dwKeyType, u64Start, u64End, pu64DataSize);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETSAMPSIZE);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// 品種固有のエラーチェック処理
	if (dwRet == RFWERR_OK) {
		dwRet = checkGetSampleDataSize(dwType, dwKeyType, u64Start, u64End);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (population_count(dwType) != 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (dwKeyType != 0 && dwKeyType != 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (u64Start > u64End) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (!pu64DataSize) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspGetSampleDataSize(dwType, dwKeyType, u64Start, u64End, pu64DataSize);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETSAMPSIZE);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_GetSampleDataSize_log_leave(dwType, dwKeyType, u64Start, u64End, pu64DataSize, dwRet);
	return dwRet;
}


//=============================================================================
/*
 * GetSampleRealTime
 * @param dwType 記録データ種別
 * @param dwKeyType 0:時間指定 1:変化点数指定
 * @param u64Back 現在の時間から遡る時間/変化点
 * @param pData 格納領域
 * @param dwSize 格納領域のサイズ
 * @param pdwStoreCnt 格納領域に格納した個数
 * @param pdwExistCnt 指定範囲に存在する個数/0の場合はdwSizeまで格納したらそれ以上検索しない
 * @return RFWERR_OK
 * @return RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_GetSampleRealTime)(DWORD dwType, DWORD dwKeyType, UINT64 u64Back, void *pData, DWORD dwSize, DWORD *const pdwStoreCnt, DWORD *const pdwExistCnt)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETSAMPNOW);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// 品種固有のエラーチェック処理
	if (dwRet == RFWERR_OK) {
		dwRet = checkGetSampleRealTime(dwType, dwKeyType, u64Back);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	if (dwRet == RFWERR_OK) {
		// ダミー関数
		if (dwType == 0) {
		}
		if (dwKeyType == 0) {
		}
		if (u64Back == 0) {
		}
		if (dwSize == 0) {
		}
		pData = 0;
		*pdwStoreCnt = 0;
		*pdwExistCnt = 0;
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETSAMPNOW);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	return dwRet;
}


//=============================================================================
/*
 * GetSampleState
 * @param pdwState 記録動作状態
 * @param pu64Num 記録したデータのバイト数
 * @param pu64Max 最大記録バイト数
 * @return RFWERR_OK
 * @return RFWERR_ERR
 */
//=============================================================================
DECLARE(RFWASP_GetSampleState)(DWORD *const pdwState, UINT64 *const pu64Num, UINT64 *const pu64Max)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_GetSampleState_log_enter(pdwState, pu64Num, pu64Max);
	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_GETSAMPSTAT);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	// パラメータチェック
	if (dwRet == RFWERR_OK) {
		if (!pdwState) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (!pu64Num) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else if (!pu64Max) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		} else {
			// パラメータチェックは以上
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_GetSampleState(pdwState, pu64Num, pu64Max);
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_GETSAMPSTAT);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_GetSampleState_log_leave(pdwState, pu64Num, pu64Max, dwRet);
	return dwRet;
}

//=============================================================================
/*
 * SetClkCalibration
 * @param dwType 記録種別
 * @param i64Calib 補正値
 * @return RFWERR_OK
 * @return RFWERR_ERR
*/
//=============================================================================
DECLARE(RFWASP_SetClkCalibration)(DWORD dwType, INT64 i64Calib)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwRetTmp = RFWERR_OK;

	RFWASP_SetClkCalibration_log_enter(dwType, i64Calib);

	// ワーニング情報のクリア
	ClrWarning();

	// 排他制御のチェック
	dwRet = CmdExecutCheck_ASP(ASPCMD_SETCLKCALIBRATION);
	if (dwRet != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRet);
	}

	// 品種固有のエラーチェック処理
	if (dwRet == RFWERR_OK) {
		dwRet = checkSetClkCalibration(dwType, i64Calib);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 拡張機能が有効かチェック
	if (dwRet == RFWERR_OK) {
		if (IsAspOn() == FALSE) {
			dwRet = SetCommErrNo(RFWERR_N_BASP_ASPOFF);
		}
	}

	if (dwRet == RFWERR_OK) {
		if (population_count(dwType) != 1) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	/* 補正値はFPGAクロックに合わせて48bit以内 */
	if (dwRet == RFWERR_OK) {
		if (i64Calib > 0xffffffffffffLL || i64Calib < -0xffffffffffffLL) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_SetClkCalibration(dwType, i64Calib);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 排他制御のための終了制御
	dwRetTmp = EndOfCmdExecut_ASP(ASPCMD_SETCLKCALIBRATION);
	if (dwRetTmp != RFWERR_OK) {	// 排他制御エラーの格納
		dwRet = SetCommErrNo(dwRetTmp);
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	RFWASP_SetClkCalibration_log_leave(dwType, i64Calib, dwRet);
	return dwRet;
}

/**
 * Debug用rawデータファイル追加
 */
DECLARE(RFWASP_AddRaw)(const char *rawfile, const char* datfile)
{
	DWORD	dwRet = RFWERR_OK;

	// ワーニング情報のクリア
	ClrWarning();

	if (dwRet == RFWERR_OK) {
		if (!rawfile || !datfile) {
			dwRet = SetCommErrNo(RFWERR_N_RFW_ARG);
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = DO_AspAddRaw(rawfile, datfile);
		if (dwRet != RFWERR_OK) {
			dwRet = SetCommErrNo(dwRet);
		}
	}

	// 共通エラーへワーニングを登録する
	if ((dwRet == RFWERR_OK) && (WarningChk() != RFWERR_OK)) {
		SetCommErrNo(WarningChk());
	}

	// 関数戻り値作成
	if ((dwRet != RFWERR_OK) || (WarningChk() != RFWERR_OK)) {
		dwRet = RFWERR_ERR;
	}

	return dwRet;
}

