///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_trg.cpp
 * @brief 外部トリガコマンドの実装ファイル
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
#include "doasp_trg.h"
#include "rfwasp_trg.h"
#include "emudef.h"
#include "doasp_sys.h"
#include "asp_setting.h"
#include "doasp_exe.h"
#include "doasp_sys_family.h"


// ファイル内static変数の宣言
static BOOL		s_bExTrgInCh0Ena;
static BOOL		s_bExTrgInCh1Ena;
static BYTE		s_byExTrgInCh0Type;
static BYTE		s_byExTrgInCh1Type;
static RFW_EXTRGOUT_DATA s_ExTriggerOut[2];
static BOOL		s_bExTrgOutCh0Ena;
static BOOL		s_bExTrgOutCh1Ena;
static BYTE		s_byExTrgOutLevel[2];

// ファイル内static関数の宣言


///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 外部トリガ入力のE2トリガ設定
 * @param byChNo チャネル番号 
 * @param byDetectType トリガ入力検出条件
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PINDISABLE　指定のASP端子は無効状態
 * @retval RFWERR_N_BASP_LFACT_CANTCHG　E2トリガはレベル型にリンクされているため変更不可
 * @retval RFWERR_N_BASP_PFACT_CANTCHG　E2トリガはパルス型にリンクされているため変更不可
 */
//=============================================================================
DWORD DO_SetExTriggerIn(BYTE byChNo, BYTE byDetectType)
{
	DWORD	dwRet = RFWERR_OK;
	UINT64	u64ExTrgInBit;
	int		i;
	RFW_ASPCONF_DATA	AspConfData;
	RFW_ASPE2ACT_INFO	AspE2ActInfo;
	DWORD	dwTrgChNo;
	DWORD	dwFactType;

	ProtInit_ASP();

	dwFactType = 0;

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// コンフィグ情報の取得
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		// 外部トリガ入力機能が有効かチェック
		if (byChNo == RF_ASPTRGIN_CH0) {
			if ((AspConfData.dwFunction2 & RF_ASPTRGIN_CH0_BIT) != RF_ASPTRGIN_CH0_BIT) {
				dwRet = RFWERR_N_BASP_PINDISABLE;
			}
		} else {
			if ((AspConfData.dwFunction2 & RF_ASPTRGIN_CH1_BIT) != RF_ASPTRGIN_CH1_BIT) {
				dwRet = RFWERR_N_BASP_PINDISABLE;
			}
		}
	}

	// 外部トリガ入力ch要因ビット
	if (dwRet == RFWERR_OK) {
		if (byChNo == RF_ASPTRGIN_CH0) {
			u64ExTrgInBit = 4;
		} else {
			u64ExTrgInBit = 8;
		}
		// 外部トリガ入力検出条件
		if (byDetectType > RF_ASPTRGIN_HLEVEL) {
			dwFactType = RFW_EVENTLINK_PULSE;		// パルス型
		} else {
			dwFactType = RFW_EVENTLINK_LEVEL;		// レベル型
		}
		// 外部トリガ入力をE2トリガとするE2アクションのチェック
		for (i = 0; i < RFW_EVENTLINK_CH_MAX; i++) {
			GetE2ActTable((DWORD)i, &AspE2ActInfo);
			if ((AspE2ActInfo.u64LinkFact & u64ExTrgInBit) == u64ExTrgInBit) {
				// E2アクションのタイプチェック
				if (AspE2ActInfo.dwActType == RFW_EVENTLINK_PULSE) {
					// パルス型の場合
					if (dwFactType == RFW_EVENTLINK_LEVEL) {
						dwRet = RFWERR_N_BASP_PFACT_CANTCHG;
						break;
					}
				} else if (AspE2ActInfo.dwActType == RFW_EVENTLINK_LEVEL) {
					// レベル型の場合
					if (dwFactType == RFW_EVENTLINK_PULSE) {
						dwRet = RFWERR_N_BASP_LFACT_CANTCHG;
						break;
					}
				} else {
					// どちらでも可なアクションのためエラーなし
				}
			}
		}
	}

	if (dwRet == RFWERR_OK) {
		// 外部トリガ入力設定状態の保存
		if (byChNo == RF_ASPTRGIN_CH0) {
			s_bExTrgInCh0Ena = TRUE;
			s_byExTrgInCh0Type = byDetectType;
			dwTrgChNo = 2;
		} else {
			s_bExTrgInCh1Ena = TRUE;
			s_byExTrgInCh1Type = byDetectType;
			dwTrgChNo = 3;
		}
		// E2トリガ情報の更新
		SetE2TrgTable(dwTrgChNo, dwFactType);

		// 外部トリガの端子の設定はこのタイミングで行う
		SetExTrgIn(byChNo, byDetectType);
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ入力のE2トリガ参照
 * @param byChNo チャネル番号
 * @param byDetectType トリガ入力検出条件
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BASP_PINNOUSE　指定のASP端子は未使用状態
 */
//=============================================================================
DWORD DO_GetExTriggerIn(BYTE byChNo, BYTE *const pbyDetectType)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// 外部トリガ入力の検出条件参照
	if (dwRet == RFWERR_OK) {
		// 外部トリガ入力chのチェック
		if (byChNo == RF_ASPTRGIN_CH0) {
			if (s_bExTrgInCh0Ena == TRUE) {
				*pbyDetectType = s_byExTrgInCh0Type;
			} else {
				dwRet = RFWERR_N_BASP_PINNOUSE;
			}
		} else {
			if (s_bExTrgInCh1Ena == TRUE) {
				*pbyDetectType = s_byExTrgInCh1Type;
			} else {
				dwRet = RFWERR_N_BASP_PINNOUSE;
			}
		}
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ入力のE2トリガ消去
 * @param byChNo チャネル番号
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PINNOUSE　指定のASP端子は未使用状態
 * @retval RFWERR_N_BASP_FACT_CANTCLR　E2トリガはE2アクションにリンクされているため消去不可
 */
//=============================================================================
DWORD DO_ClrExTriggerIn(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwTrgChNo;
	RFW_ASPE2TRG_INFO AspE2TrgInfo;

	ProtInit_ASP();

	dwTrgChNo = 0;

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// 外部トリガ入力が設定されているかチェック
	if (dwRet == RFWERR_OK) {
		// 外部トリガ入力chのチェック
		if (byChNo == RF_ASPTRGIN_CH0) {
			if (s_bExTrgInCh0Ena == FALSE) {
				dwRet = RFWERR_N_BASP_PINNOUSE;
			}
		} else {
			if (s_bExTrgInCh1Ena == FALSE) {
				dwRet = RFWERR_N_BASP_PINNOUSE;
			}
		}
	}

	// 外部トリガ入力がE2アクションにリンクされているかチェック
	if (dwRet == RFWERR_OK) {
		// 外部トリガ入力chのチェック
		if (byChNo == RF_ASPTRGIN_CH0) {
			dwTrgChNo = 2;
		} else {
			dwTrgChNo = 3;
		}
		GetE2TrgTable(dwTrgChNo, &AspE2TrgInfo);
		if (AspE2TrgInfo.u64LinkAct != 0) {
			dwRet = RFWERR_N_BASP_FACT_CANTCLR;
		}
	}

	// 外部トリガ入力消去
	if (dwRet == RFWERR_OK) {
		// 外部トリガ入力管理用変数の初期化
		if (byChNo == RF_ASPTRGIN_CH0) {
			s_bExTrgInCh0Ena = FALSE;
			s_byExTrgInCh0Type = 0;
		} else {
			s_bExTrgInCh1Ena = FALSE;
			s_byExTrgInCh1Type = 0;
		}
		// E2トリガのテーブル初期化
		ClrE2TrgTable(dwTrgChNo);

		// 外部トリガの端子の設定はこのタイミングで行う
		ClrExTrgIn(byChNo);
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ出力のE2アクション設定
 * @param byChNo チャネル番号 
 * @param pExTriggerOut 外部トリガ出力情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PINDISABLE　指定のASP端子は無効状態
 * @retval RFWERR_N_BASP_FACT_NONSEL　E2トリガが指定されていない
 * @retval RFWERR_N_BASP_SGLLNK_ILLEGAL　シングルE2トリガに複数個の要因は設定できない
 * @retval RFWERR_N_BASP_FACT_DISABLE　指定のE2トリガは無効状態
 * @retval RFWERR_N_BASP_PFACT_CANTSET　レベル型動作にパルス型イベントは設定できない
 * @retval RFWERR_N_BASP_LFACT_CANTSET　パルス型動作にレベル型イベントは設定できない
 * @retval RFWERR_N_BASP_FACT_NOSUPPORT　指定のE2トリガは未サポート
 */
//=============================================================================
DWORD DO_SetExTriggerOut(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut)
{
	DWORD	dwRet = RFWERR_OK;
	RFW_ASPCONF_DATA	AspConfData;
	BYTE	byActNo;
	DWORD	dwActType = 0;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	// コンフィグ情報の取得
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		// 外部トリガ出力機能が有効かチェック
		if (byChNo == RF_ASPTRGOUT_CH0) {
			if ((AspConfData.dwFunction2 & RF_ASPTRGOUT_CH0_BIT) != RF_ASPTRGOUT_CH0_BIT) {
				dwRet = RFWERR_N_BASP_PINDISABLE;
			}
		} else {
			if ((AspConfData.dwFunction2 & RF_ASPTRGOUT_CH1_BIT) != RF_ASPTRGOUT_CH1_BIT) {
				dwRet = RFWERR_N_BASP_PINDISABLE;
			}
		}
	}

	// アクティブレベル設定済みかチェック
	if (dwRet == RFWERR_OK) {
		// 外部トリガ出力機能が有効かチェック
		if (s_byExTrgOutLevel[byChNo] == 0xFF) {
			dwRet = RFWERR_N_BASP_NONSEL_ACTLEVEL;
		}
	}

	// マルチE2トリガ指定のチェック(本開発の対象外)
	if (dwRet == RFWERR_OK) {
	}

	// シングルE2トリガ指定のチェック
	if (dwRet == RFWERR_OK) {
		if (pExTriggerOut->byOutputType == RF_ASPTRGOUT_PULSE) {
			// パルス型の場合
			dwActType = RFW_EVENTLINK_PULSE;
			if (byChNo == RF_ASPTRGOUT_CH0) {
				dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, pExTriggerOut->u64FactorSingleEvent, RFW_EVENTLINK_PULSE, RF_ASPE2ACTCH_EXTRGINCH0);
			} else {
				dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, pExTriggerOut->u64FactorSingleEvent, RFW_EVENTLINK_PULSE, RF_ASPE2ACTCH_EXTRGINCH1);
			}
		} else {
			// レベル型の場合
			dwActType = RFW_EVENTLINK_LEVEL;
			if (byChNo == RF_ASPTRGOUT_CH0) {
				dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, pExTriggerOut->u64FactorSingleEvent, RFW_EVENTLINK_LEVEL, RF_ASPE2ACTCH_EXTRGINCH0);
			} else {
				dwRet = CheckSingleEvent(RFW_ASPFUNC_SET, pExTriggerOut->u64FactorSingleEvent, RFW_EVENTLINK_LEVEL, RF_ASPE2ACTCH_EXTRGINCH1);
			}
		}
	}

	if (dwRet == RFWERR_OK) {
		if (byChNo == RF_ASPTRGOUT_CH0) {
			s_bExTrgOutCh0Ena = TRUE;
			byActNo = RF_ASPE2ACTCH_EXTRGINCH0;
		} else {
			s_bExTrgOutCh1Ena = TRUE;
			byActNo = RF_ASPE2ACTCH_EXTRGINCH1;
		}
		// 外部トリガ出力設定状態の保存
		s_ExTriggerOut[byChNo].byOutputType = pExTriggerOut->byOutputType;
		if (pExTriggerOut->byOutputType == RF_ASPTRGOUT_PULSE) {
			// パルス型の場合は、パルス幅を更新
			s_ExTriggerOut[byChNo].wPulseWidth = pExTriggerOut->wPulseWidth;
		} else {
			// レベル型の場合は、パルス幅を初期化
			s_ExTriggerOut[byChNo].wPulseWidth = 0x0001;
		}
		s_ExTriggerOut[byChNo].u64FactorSingleEvent = pExTriggerOut->u64FactorSingleEvent;
		s_ExTriggerOut[byChNo].dwFactorMultiEvent = pExTriggerOut->dwFactorMultiEvent;

		// 指定シングルE2トリガのアクション情報に外部トリガ出力をリンク
		SaveSingleEvent(find_first_bit(pExTriggerOut->u64FactorSingleEvent), (UINT64)((UINT64)1 << byActNo));

		// E2アクションのテーブル設定
		SetE2ActTable((DWORD)byActNo, dwActType, pExTriggerOut->u64FactorSingleEvent);

		// 外部トリガの端子の設定はこのタイミングで行う
		SetExTrgOut(byChNo, pExTriggerOut);
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ出力のE2アクション参照
 * @param byChNo チャネル番号
 * @param pExTriggerOut 外部トリガ出力情報
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BASP_PINNOUSE　指定のASP端子は未使用状態
 */
//=============================================================================
DWORD DO_GetExTriggerOut(BYTE byChNo, RFW_EXTRGOUT_DATA *const pExTriggerOut)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	if (dwRet == RFWERR_OK) {
		// 外部トリガ出力chのチェック
		if (byChNo == RF_ASPTRGOUT_CH0) {
			if (s_bExTrgOutCh0Ena == FALSE) {
				dwRet = RFWERR_N_BASP_PINNOUSE;
			}
		} else {
			if (s_bExTrgOutCh1Ena == FALSE) {
				dwRet = RFWERR_N_BASP_PINNOUSE;
			}
		}
	}

	// 外部トリガ出力のE2アクション取得
	if (dwRet == RFWERR_OK) {
		memcpy((void*)pExTriggerOut, &s_ExTriggerOut[byChNo], sizeof(RFW_EXTRGOUT_DATA));
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * 外部トリガ出力のE2アクション消去
 * @param byChNo チャネル番号
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
 * @retval RFWERR_N_BASP_PINNOUSE　指定のASP端子は未使用状態
 */
//=============================================================================
DWORD DO_ClrExTriggerOut(BYTE byChNo)
{
	DWORD	dwRet = RFWERR_OK;
	DWORD	dwActChNo;

	ProtInit_ASP();

	dwActChNo = 0;

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}
	
	// 外部トリガ出力が設定されているかチェック
	if (dwRet == RFWERR_OK) {
		// 外部トリガ出力chのチェック
		if (byChNo == RF_ASPTRGOUT_CH0) {
			dwActChNo = 6;
			if (s_bExTrgOutCh0Ena == FALSE) {
				dwRet = RFWERR_N_BASP_PINNOUSE;
			}
		} else {
			dwActChNo = 7;
			if (s_bExTrgOutCh1Ena == FALSE) {
				dwRet = RFWERR_N_BASP_PINNOUSE;
			}
		}
	}

	// 外部トリガ出力のE2アクション消去
	if (dwRet == RFWERR_OK) {
		// 外部トリガ出力管理用変数の初期化
		InitAspExTrgOut(byChNo);

		// E2アクションのテーブル初期化
		ClrE2ActTable(dwActChNo);

		// RFW_ASPE2TRG_INFO.u64LinkActにリンクされている本E2アクション情報を削除
		ClearSingleEvent((UINT64)((UINT64)1 << dwActChNo));

		// 外部トリガの端子の設定はこのタイミングで行う
		ClrExTrgOut(byChNo);
	}

	ProtEnd_ASP();

	return dwRet;
}


//=============================================================================
/**
* 外部トリガ出力のアクティブレベル設定
* @param byChNo チャネル番号
* @param byActiveLevel 外部トリガ出力アクティブレベル
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BMCU_RUN　ユーザプログラム実行中
* @retval RFWERR_N_BASP_PINDISABLE　指定のASP端子は無効状態
*/
//=============================================================================
DWORD DO_SetExTriggerOutLevel(BYTE byChNo, BYTE byActiveLevel)
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

	// コンフィグ情報の取得
	if (dwRet == RFWERR_OK) {
		GetAspConfData(&AspConfData);
		// 外部トリガ出力機能が有効かチェック
		if (dwRet == RFWERR_OK) {
			if (byChNo == RF_ASPTRGOUT_CH0) {
				if ((AspConfData.dwFunction2 & RF_ASPTRGOUT_CH0_BIT) != RF_ASPTRGOUT_CH0_BIT) {
					dwRet = RFWERR_N_BASP_PINDISABLE;
				}
			} else {
				if ((AspConfData.dwFunction2 & RF_ASPTRGOUT_CH1_BIT) != RF_ASPTRGOUT_CH1_BIT) {
					dwRet = RFWERR_N_BASP_PINDISABLE;
				}
			}
		}
	}

	if (dwRet == RFWERR_OK) {
		// 外部トリガの端子の設定はこのタイミングで行う
		SetExTrgOutLevel(byChNo, byActiveLevel);
		s_byExTrgOutLevel[byChNo] = byActiveLevel;
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
* 外部トリガ出力のアクティブレベル参照
* @param byChNo チャネル番号
* @param pbyActiveLevel 外部トリガ出力アクティブレベル
* @retval RFWERR_OK  正常動作
* @retval RFWERR_N_BASP_PINNOUSE　指定のASP端子は未使用状態
*/
//=============================================================================
DWORD DO_GetExTriggerOutLevel(BYTE byChNo, BYTE *const pbyActiveLevel)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	if (dwRet == RFWERR_OK) {
		// 外部トリガ出力chのチェック
		if (s_byExTrgOutLevel[byChNo] == 0xFF) {
			dwRet = RFWERR_N_BASP_PINNOUSE;
		}
	}

	// 外部トリガ出力のE2アクション取得
	if (dwRet == RFWERR_OK) {
		*pbyActiveLevel = s_byExTrgOutLevel[byChNo];
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
* 外部トリガ入力の管理用変数初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitAspExTrgIn(void)
{
	s_bExTrgInCh0Ena = FALSE;
	s_bExTrgInCh1Ena = FALSE;
	s_byExTrgInCh0Type = 0;
	s_byExTrgInCh1Type = 0;
}

//=============================================================================
/**
* 外部トリガ出力の管理用変数初期化
* @param byChNo チャネル番号
* @retval なし
*/
//=============================================================================
void InitAspExTrgOut(BYTE byChNo)
{
	if (byChNo == RF_ASPTRGOUT_CH0) {
		s_bExTrgOutCh0Ena = FALSE;
		s_ExTriggerOut[0].byOutputType = 0;
		s_ExTriggerOut[0].u64FactorSingleEvent = 0;
		s_ExTriggerOut[0].dwFactorMultiEvent = 0;
		s_ExTriggerOut[0].wPulseWidth = 0x0001;
	}
	if (byChNo == RF_ASPTRGOUT_CH1) {
		s_bExTrgOutCh1Ena = FALSE;
		s_ExTriggerOut[1].byOutputType = 0;
		s_ExTriggerOut[1].u64FactorSingleEvent = 0;
		s_ExTriggerOut[1].dwFactorMultiEvent = 0;
		s_ExTriggerOut[1].wPulseWidth = 0x0001;
	}
}

//=============================================================================
/**
* 外部トリガ入出力の管理用変数初期化
* @param なし
* @retval なし
*/
//=============================================================================
void InitAspExTrg(void)
{
	InitAspExTrgIn();
	InitAspExTrgOut(RF_ASPTRGOUT_CH0);
	InitAspExTrgOut(RF_ASPTRGOUT_CH1);
}

//=============================================================================
/**
* 外部トリガ出力のアクティブレベル初期化
* @note ASP ON/OFFでは初期化しない
* @note ターゲット接続時のみ初期化する
* @param なし
* @retval なし
*/
//=============================================================================
void InitAspExTrgOutLevel(void)
{
	s_byExTrgOutLevel[RF_ASPTRGOUT_CH0] = 0xFF;
	s_byExTrgOutLevel[RF_ASPTRGOUT_CH1] = 0xFF;
}

///////////////////////////////////////////////////////////////////////////////
// グローバル関数
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
* 外部トリガ出力のイネーブル情報取得
* @param byChNo チャネル番号
* @retval TRUE  イネーブル
* @retval FALSE ディセーブル
*/
//=============================================================================
BOOL IsAspExTrgOutEna(BYTE byChNo)
{
	if (byChNo == RF_ASPTRGOUT_CH0) {
		return s_bExTrgOutCh0Ena;
	}
	else {
		return s_bExTrgOutCh1Ena;
	}
}

//=============================================================================
/**
* 外部トリガ入力のイネーブル情報取得
* @param byChNo チャネル番号
* @retval TRUE  イネーブル
* @retval FALSE ディセーブル
*/
//=============================================================================
BOOL IsAspExTrgInEna(BYTE byChNo)
{
	if (byChNo == RF_ASPTRGIN_CH0) {
		return s_bExTrgInCh0Ena;
	}
	else {
		return s_bExTrgInCh1Ena;
	}
}

