///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasprx_pwr.cpp
 * @brief パワーモニタコマンドの実装ファイル(RX固有部)
 * @author REL H.Ohga
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/20
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2017/10/20 新規作成 H.Ohga
*/
#include <math.h>
#include "rfw_bitops.h"
#include "doasp_pwr.h"
#include "doasprx_pwr.h"
#include "emudef.h"
#include "doasp_sys.h"
#include "asp_setting.h"
#include "doasp_sys_family.h"
#include "e2_asprx_fpga.h"

//=============================================================================
/**
 * パワーモニタのサンプリングデータ範囲参照
 * @param pdwUAmin パワーモニタのサンプリングデータの最小値情報
 * @param pdwUAmax パワーモニタのサンプリングデータの最大値情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PWRMONOFF パワーモニタ機能が無効状態
 */
//=============================================================================
DWORD DO_GetPwrMonDataRange(DWORD *pdwUAmin, DWORD *pdwUAmax)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwVal, dwAdMin, dwAdMax;
	RFW_ASPCONF_DATA	AspConfData;

	ProtInit_ASP();

	// コンフィグ情報の取得
	GetAspConfData(&AspConfData);

	// パワーモニタが有効状態になっているかチェック
	if (dwRet == RFWERR_OK) {
		if ((AspConfData.dwFunction3 & RF_ASPFUNC_PWRMON) == 0) {
			dwRet = RFWERR_N_BASP_PWRMONOFF;
		}
	}

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// パワーモニタのサンプリングデータの範囲をFPGAレジスタから取得
	if (dwRet == RFWERR_OK) {
		dwVal = FPGA_READ(PWRM_MAXMIN);

		if (dwVal == PWRM_MAXMIN_INIT){
			// 実行前(PWRM_MAXMINが初期状態)ならば、min=max=0にする
			*pdwUAmin = 0;
			*pdwUAmax = 0;
		} else {
			//下位12bitが最小値
			dwAdMin = dwVal & PWRM_RNGMIN_MASK;
			dwAdMin <<= 16; // CalcAd2Current()に渡すため、16ビットシフトする
			*pdwUAmin = CalcAd2Current(dwAdMin); // AD値 -> 電流値(uA)変換

			//上位16bitの内 12bitが最大値
			dwAdMax = dwVal & PWRM_RNGMAX_MASK;
			*pdwUAmax = CalcAd2Current(dwAdMax); // AD値 -> 電流値(uA)変換
		}
	}

	ProtEnd_ASP();

	return dwRet;
}
