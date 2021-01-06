///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_per.cpp
 * @brief 時間計測コマンドの実装ファイル
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
#include "doasp_per.h"
#include "doasp_sys.h"
#include "emudef.h"
#include "asp_setting.h"
#include "doasp_sys_family.h"


// ファイル内static変数の宣言
static BOOL	s_bPerSwitch[2];
static RFW_ASPPERCOND_DATA s_AspPerCond[2];

// ファイル内static関数の宣言


///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* 時間測定条件の設定
* @param byChNo チャネル番号
* @param pAspPerCond 時間計測条件
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
* @retval RFWERR_N_BASP_SGLLNK_ILLEGAL
* @retval RFWERR_N_BASP_FACT_NONSEL
* @retval RFWERR_N_BASP_FACT_NOSUPPORT
* @retval RFWERR_N_BASP_FACT_DISABLE
* @retval RFWERR_N_BASP_LFACT_CANTSET
* @retval RFWERR_N_BASP_PFACT_CANTSET
* @retval RFWERR_N_BASP_FACT_CANTCLR
*/
//=============================================================================
DWORD DO_SetAspPerCond(BYTE byChNo, const RFW_ASPPERCOND_DATA *pAspPerCond)
{
	DWORD	dwRet = RFWERR_OK;
	UINT64	u64mode;
	DWORD	dwChNo = 0;
	BOOL	bChangeFlag = FALSE;
	BOOL	bTypeFlag = FALSE;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// パルス型要因の場合
	if ((pAspPerCond->byMode & RF_ASPPER_MODE_TRG) == RF_ASPPER_MODE_TRG_PULSE) {
		u64mode = (UINT64)1 << RF_ASPE2ACTCH_PERCH0_START;
		if (dwRet == RFWERR_OK) {
			dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, pAspPerCond->u64StartEvent, RFW_EVENTLINK_PULSE, RF_ASPE2ACTCH_PERCH0_START);
		}
		if (dwRet == RFWERR_OK) {
			dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, pAspPerCond->u64EndEvent, RFW_EVENTLINK_PULSE, RF_ASPE2ACTCH_PERCH0_END);
		}
	// レベル型要因の場合
	} else {
		u64mode = (UINT64)1 << RF_ASPE2ACTCH_PERCH0_SECT;
		if (dwRet == RFWERR_OK) {
			dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, pAspPerCond->u64SectionEvent, RFW_EVENTLINK_LEVEL, RF_ASPE2ACTCH_PERCH0_SECT);
		}
	}

	if (dwRet == RFWERR_OK) {
		if (byChNo == 0) {
			dwChNo = 0;
		} else {
			dwChNo = 3;
			u64mode <<= 3;
		}
		// 時間計測条件の保存
		if (s_bPerSwitch[byChNo]) {
			// 条件変更
			bChangeFlag = TRUE;
			if ((s_AspPerCond[byChNo].byMode & RF_ASPPER_MODE_TRG) != (pAspPerCond->byMode & RF_ASPPER_MODE_TRG)) {
				bTypeFlag = TRUE;
			}
		}
		else {
			s_bPerSwitch[byChNo] = TRUE;
		}
		memcpy((void*)&s_AspPerCond[byChNo], pAspPerCond, sizeof(*pAspPerCond));
	}

	// トリガイベント情報の更新
	if (dwRet == RFWERR_OK) {
		// 変更の場合は、元のアクション情報を一旦消す
		if (bChangeFlag) {
			if (byChNo == 0) {
				// ch0はbit0-2がアクションチャネル
				ClearSingleEvent(((UINT64)1 << RF_ASPE2ACTCH_PERCH0_START) | ((UINT64)1 << RF_ASPE2ACTCH_PERCH0_END) | ((UINT64)1 << RF_ASPE2ACTCH_PERCH0_SECT));
			} else {
				// ch1はbit3-5がアクションチャネル
				ClearSingleEvent(((UINT64)1 << RF_ASPE2ACTCH_PERCH1_START) | ((UINT64)1 << RF_ASPE2ACTCH_PERCH1_END) | ((UINT64)1 << RF_ASPE2ACTCH_PERCH1_SECT));
			}
			if (bTypeFlag) {
				// レベル型アクションへの変更
				if ((pAspPerCond->byMode & RF_ASPPER_MODE_TRG) == RF_ASPPER_MODE_TRG_LEVEL) {
					// パルス型アクションの削除
					ClrE2ActTable(dwChNo);
					ClrE2ActTable(dwChNo + 1);

				// パルス型アクションへの変更
				} else {
					// レベル型アクションの削除
					ClrE2ActTable(dwChNo + 2);
				}
			}
		}

		// ここからは設定情報の保存
		// パルス型要因の場合
		if ((pAspPerCond->byMode & RF_ASPPER_MODE_TRG) == RF_ASPPER_MODE_TRG_PULSE) {
			// 開始・終了トリガにアクション情報を登録
			SaveSingleEvent(find_first_bit(pAspPerCond->u64StartEvent), u64mode);
			u64mode <<= 1;
			SaveSingleEvent(find_first_bit(pAspPerCond->u64EndEvent), u64mode);

			// E2アクションのテーブル設定
			// アクションは開始と終了の2つある(両方ともパルス型)
			SetE2ActTable(dwChNo, RFW_EVENTLINK_PULSE, pAspPerCond->u64StartEvent);
			SetE2ActTable(dwChNo+1, RFW_EVENTLINK_PULSE, pAspPerCond->u64EndEvent);

			// E2トリガのテーブル設定
			// 時間測定がトリガとなる側は1chでパルス型
			SetE2TrgTable(byChNo, RFW_EVENTLINK_PULSE);

		// レベル型要因の場合
		} else {
			// セクショントリガにアクション情報を登録
			SaveSingleEvent(find_first_bit(pAspPerCond->u64SectionEvent), u64mode);

			// E2アクションのテーブル設定
			// セクションの場合は1つだけ(レベル型)
			SetE2ActTable(dwChNo+2, RFW_EVENTLINK_LEVEL, pAspPerCond->u64SectionEvent);

			// E2トリガのテーブル設定
			// 時間測定がトリガとなる側は1chでパルス型
			SetE2TrgTable(byChNo, RFW_EVENTLINK_PULSE);
		}

		// 計測時間のクリアを行う
		ClrAspPerData(byChNo);
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* 時間測定条件の取得
* @param byChNo チャネル番号
* @param pAspPerCond 時間計測条件
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_CHNOUSE　指定したチャネルは使用していない
*/
//=============================================================================
DWORD DO_GetAspPerCond(BYTE byChNo, RFW_ASPPERCOND_DATA *pAspPerCond)
{
	DWORD dwRet = RFWERR_OK;

	ProtInit_ASP();

	if (!s_bPerSwitch[byChNo]) {
		// 指定したチャネルは無効
		dwRet = RFWERR_N_BASP_CHNOUSE;
	} else {
		memcpy((void*)pAspPerCond, &s_AspPerCond[byChNo], sizeof(s_AspPerCond[byChNo]));
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* 時間測定条件の削除
* @param byChNo チャネル番号
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
* @retval RFWERR_N_BASP_CHNOUSE　指定したチャネルは使用していない
* @retval RFWERR_N_BASP_FACT_CANTCLR　E2アクションにリンクされているためクリアできない
*/
//=============================================================================
DWORD DO_ClrAspPerCond(BYTE byChNo)
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

	// 指定したchが使われているかチェック
	if (dwRet == RFWERR_OK) {
		if (!s_bPerSwitch[byChNo]) {
			dwRet = RFWERR_N_BASP_CHNOUSE;
		}
	}

	// チャネルからイベント情報に変換
	if (byChNo == 0) {
		u64SingleEvent = (UINT64)1 << RF_ASPE2TRGCH_PERCH0;
	} else {
		u64SingleEvent = (UINT64)1 << RF_ASPE2TRGCH_PERCH1;
	}

	// アクションとリンクされていないかチェック
	if (dwRet == RFWERR_OK) {
		dwRet = CheckSingleEvent(RFW_ASPFUNC_CLR, u64SingleEvent, 0, 0);
	}

	if (dwRet == RFWERR_OK) {
		// パフォーマンス計測管理用変数の初期化
		InitAspPerCond(byChNo);

		// 時間計測のためのトリガ側のアクション情報を初期化
		if (byChNo == 0) {
			// ch0はbit0-2がアクションチャネル
			ClearSingleEvent(((UINT64)1 << RF_ASPE2ACTCH_PERCH0_START) | ((UINT64)1 << RF_ASPE2ACTCH_PERCH0_END) | ((UINT64)1 << RF_ASPE2ACTCH_PERCH0_SECT));
		} else {
			// ch1はbit3-5がアクションチャネル
			ClearSingleEvent(((UINT64)1 << RF_ASPE2ACTCH_PERCH1_START) | ((UINT64)1 << RF_ASPE2ACTCH_PERCH1_END) | ((UINT64)1 << RF_ASPE2ACTCH_PERCH1_SECT));
		}

		// E2アクションのテーブル初期化
		// 時間計測1ch分でアクション3ch分を初期化
		ClrE2ActTable((DWORD)(byChNo*3));
		ClrE2ActTable((DWORD)(byChNo*3) + 1);
		ClrE2ActTable((DWORD)(byChNo*3) + 2);

		// E2トリガのテーブル初期化
		ClrE2TrgTable((DWORD)byChNo);

		// 計測時間のクリアを行う
		ClrAspPerData(byChNo);
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* 時間測定の結果取得
* @param byChNo チャネル番号
* @param pAspPerformanceData 時間計測結果
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
* @retval RFWERR_N_BASP_CHNOUSE　指定したチャネルは使用していない
* @retval RFWERR_N_BASP_TSTAMP_OVER　タイムスタンプがオーバーフローしたため計測できなかった
*/
//=============================================================================
DWORD DO_GetAspPerData(BYTE byChNo, RFW_ASPPER_DATA *pAspPerformanceData)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 指定したchが使われているかチェック
	if (dwRet == RFWERR_OK) {
		if (!s_bPerSwitch[byChNo]) {
			// 指定したチャネルは無効
			dwRet = RFWERR_N_BASP_CHNOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		// 測定結果の読み出し
		dwRet = GetAspPerData(byChNo, pAspPerformanceData);
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* 時間測定の結果消去
* @param byChNo チャネル番号
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
* @retval RFWERR_N_BASP_CHNOUSE　指定したチャネルは使用していない
*/
//=============================================================================
DWORD DO_ClrAspPerData(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 指定したchが使われているかチェック
	if (dwRet == RFWERR_OK) {
		if (!s_bPerSwitch[byChNo]) {
			// 指定したチャネルは無効
			dwRet = RFWERR_N_BASP_CHNOUSE;
		}
	}

	if (dwRet == RFWERR_OK) {
		// 測定結果のクリア
		ClrAspPerData(byChNo);
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
* 時間測定用内部管理変数の初期化
* @note 初期化するch番号を指定する。0xFFを指定した場合は全chを初期化する。
* @param byChNo チャネル番号
* @retval なし
*/
//=============================================================================
void InitAspPerCond(BYTE byChNo)
{
	BYTE	byData;
	if (byChNo == 0xFF) {
		for (byData = 0; byData < 2; byData++) {
			s_bPerSwitch[byData] = FALSE;
			s_AspPerCond[byData].byMode = 0;
			s_AspPerCond[byData].u64StartEvent = 0;
			s_AspPerCond[byData].u64EndEvent = 0;
			s_AspPerCond[byData].u64SectionEvent = 0;
			s_AspPerCond[byData].u64Threshold = 0xFFFFFFFFFFFFFFFF;
		}
	} else if (byChNo == 0){
		s_bPerSwitch[0] = FALSE;
		s_AspPerCond[0].byMode = 0;
		s_AspPerCond[0].u64StartEvent = 0;
		s_AspPerCond[0].u64EndEvent = 0;
		s_AspPerCond[0].u64SectionEvent = 0;
		s_AspPerCond[0].u64Threshold = 0xFFFFFFFFFFFFFFFF;
	} else {
		s_bPerSwitch[1] = FALSE;
		s_AspPerCond[1].byMode = 0;
		s_AspPerCond[1].u64StartEvent = 0;
		s_AspPerCond[1].u64EndEvent = 0;
		s_AspPerCond[1].u64SectionEvent = 0;
		s_AspPerCond[1].u64Threshold = 0xFFFFFFFFFFFFFFFF;
	}
}


///////////////////////////////////////////////////////////////////////////////
// グローバル関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* 時間測定有効/無効の参照
* @param なし
* @retval TRUE  時間測定有効
* @retval FALSE 時間測定無効
*/
//=============================================================================
BOOL IsAspPerformanceEna(void)
{
	if (s_bPerSwitch[0]) {
		return TRUE;
	}
	else if (s_bPerSwitch[1]) {
		return TRUE;
	}
	else {
		// ch0と1のみ
	}

	return FALSE;
}


//=============================================================================
/**
* ch指定の時間測定有効/無効の取得
* @param byChNo チャネル番号
* @retval TRUE  時間測定有効
* @retval FALSE 時間測定無効
*/
//=============================================================================
BOOL GetPerformanceEna(BYTE byChNo)
{
	return s_bPerSwitch[byChNo];
}


//=============================================================================
/**
* 時間測定の条件の取得
* @param byChNo チャネル番号
* @param pAspPerCond 時間計測条件
* @retval なし
*/
//=============================================================================
void GetPerformanceCondData(BYTE byChNo, RFW_ASPPERCOND_DATA *pAspPerCond)
{
	memcpy((void*)pAspPerCond, &s_AspPerCond[byChNo], sizeof(s_AspPerCond[byChNo]));
}


