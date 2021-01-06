///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasprh_swtrc.cpp
 * @brief 外部ソフトトレースイベントコマンドの実装ファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/10/20 新規作成 S.Nagai
*/
#include "rfw_bitops.h"
#include "doasprh_swtrc.h"
#include "rfwasprh_swtrc.h"
#include "emudef.h"
#include "doasp_sys.h"
#include "doasp_sys_family.h"

// ファイル内static変数の宣言
static BYTE	s_bySwTrcEvSw[4];
static RFW_SWTRC_DATA s_SwTrcData[4];

// ファイル内static関数の宣言


///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
* ソフトトレースイベントの設定
* @param byChNo チャネル番号
* @param pSwTrcData ソフトウェアトレースイベント情報
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN  ユーザプログラム実行中
* @retval RFWERR_N_BASP_SWTRCOFF  ソフトトレースは無効状態
*/
//=============================================================================
DWORD DO_SetSwtrcEv(BYTE byChNo, const RFW_SWTRC_DATA *pSwTrcData)
{
	DWORD	dwRet = RFWERR_OK;
	RFW_ASPCONF_DATA	AspConfData;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// SetConfigurationでソフトトレースを使う設定になっているかチェック
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		if ((AspConfData.dwFunction3 & RF_ASPCONF_EXT_STRACE) == 0) {
			dwRet = RFWERR_N_BASP_SWTRCOFF;
		}
	}

	if (dwRet == RFWERR_OK) {
		// ソフトトレースイベントの保存
		s_bySwTrcEvSw[byChNo] = RF_ASPSWTRC_ON;
		memcpy((void*)&s_SwTrcData[byChNo], pSwTrcData, sizeof(s_SwTrcData[byChNo]));

		// E2トリガのテーブル設定
		// ソフトトレースイベントチャネルは20～23でパルス型
		SetE2TrgTable(20 + byChNo, RFW_EVENTLINK_PULSE);
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* ソフトトレースイベントの取得
* @param byChNo チャネル番号
* @param pSwTrcData ソフトウェアトレースイベント情報
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_CHNOUSE　指定したチャネルは使用していない
*/
//=============================================================================
DWORD DO_GetSwtrcEv(BYTE byChNo, RFW_SWTRC_DATA *const pSwTrcData)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	if (s_bySwTrcEvSw[byChNo] == RF_ASPSWTRC_OFF) {
		// 指定したチャネルは無効
		dwRet = RFWERR_N_BASP_CHNOUSE;
	} else {
		memcpy((void*)pSwTrcData, &s_SwTrcData[byChNo], sizeof(s_SwTrcData[byChNo]));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* ソフトトレースイベントの削除
* @param byChNo チャネル番号
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
* @retval RFWERR_N_BASP_CHNOUSE　指定したチャネルは使用していない
* @retval RFWERR_N_BASP_FACT_CANTCLR　E2アクションにリンクされているためクリアできない
*/
//=============================================================================
DWORD DO_ClrSwtrcEv(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	UINT64	u64SingleEvent;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 指定したチャネルが使われているかチェック
	if (dwRet == RFWERR_OK) {
		if (s_bySwTrcEvSw[byChNo] == RF_ASPSWTRC_OFF) {
			// 指定したチャネルは無効
			dwRet = RFWERR_N_BASP_CHNOUSE;
		}
	}

	// ソフトトレースイベントはch20～23
	u64SingleEvent = 0x100000;
	u64SingleEvent <<= byChNo;

	// アクションとリンクされていないかチェック
	if (dwRet == RFWERR_OK) {
		dwRet = CheckSingleEvent(RFW_ASPFUNC_CLR, u64SingleEvent, 0, 0);
	}

	if (dwRet == RFWERR_OK) {
		// CANモニタイベント管理用変数の初期化
		InitAspSwTrcEv(byChNo);

		// E2トリガのテーブル初期化
		ClrE2TrgTable(find_first_bit(u64SingleEvent));
	}

		ProtEnd_ASP();

	return dwRet;
}


///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* ソフトトレースイベント管理用変数の初期化
* @param byChNo チャネル番号
* @retval なし
*/
//=============================================================================
void InitAspSwTrcEv(BYTE byChNo)
{
	BYTE	byData;
	if (byChNo == 0xff) {
		for (byData = 0; byData < 4; byData++) {
			s_bySwTrcEvSw[byData] = RF_ASPSWTRC_OFF;
			memset((void*)&s_SwTrcData[byData], 0x00, sizeof(s_SwTrcData[byData]));
		}
	} else {
		s_bySwTrcEvSw[byChNo] = RF_ASPSWTRC_OFF;
		memset((void*)&s_SwTrcData[byChNo], 0x00, sizeof(s_SwTrcData[byChNo]));
	}
}


///////////////////////////////////////////////////////////////////////////////
// グローバル関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* ソフトトレースイベント有効/無効の参照
* @param なし
* @retval TRUE  ソフトトレースイベント有効
* @retval FALSE ソフトトレースイベント無効
*/
//=============================================================================
BOOL IsAspSwTrcEvEna(void)
{
	DWORD	dwCnt;
	for (dwCnt = 0; dwCnt < 4; dwCnt++) {
		if (s_bySwTrcEvSw[dwCnt] == RF_ASPSWTRC_ON) {
			return TRUE;
		}
	}

	return FALSE;
}


//=============================================================================
/**
* ch指定のソフトトレースイベント有効/無効の取得
* @param byChNo チャネル番号
* @retval 0 ソフトトレースイベント有効
* @retval 1 ソフトトレースイベント無効
*/
//=============================================================================
BYTE GetAspSwTrcEvEna(BYTE byChNo)
{
	return s_bySwTrcEvSw[byChNo];
}


//=============================================================================
/**
* ソフトトレースイベント条件の取得
* @param byChNo チャネル番号
* @param pSwTrcData ソフトウェアトレースイベント情報
* @retval なし
*/
//=============================================================================
void GetSwTrcData(BYTE byChNo, RFW_SWTRC_DATA *pSwTrcData)
{
	memcpy((void*)pSwTrcData, &s_SwTrcData[byChNo], sizeof(s_SwTrcData[byChNo]));
}


//=============================================================================
/**
* ソフトトレース有効/無効の参照
* @param なし
* @retval TRUE  ソフトトレース有効
* @retval FALSE ソフトトレース無効
*/
//=============================================================================
BOOL IsAspSwTrcEna(void)
{
	RFW_ASPCONF_DATA	AspConfData;

	// SetConfigurationでソフトトレースを使う設定になっているかチェック
	GetAspConfData(&AspConfData);
	if ((AspConfData.dwFunction3 & RF_ASPCONF_EXT_STRACE) == 0) {
		return FALSE;
	}
	return TRUE;
}

