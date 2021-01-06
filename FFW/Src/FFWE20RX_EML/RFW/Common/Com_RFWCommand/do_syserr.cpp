///////////////////////////////////////////////////////////////////////////////
/**
 * @file do_syserr.cpp
 * @brief システムコマンドの実装ファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/09/09
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/09/09
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "do_syserr.h"
#include "emudef.h"
#include "errmessage.h"
//#include "errchk.h"
//#include "mcucarm_mcu.h"

// ファイル内static変数の宣言

// RFWエラー番号登録
// 呼び出し元threadごとにエラー番号を保持するためthrad local storageにする
static __declspec(thread) DWORD s_dwRfwErrType = RF_ERR_COMMON;		// エラー番号格納変数(共通鰓格納)
static __declspec(thread) DWORD s_dwRfwErrNoCore[CORE_MAX];			// エラー番号格納変数(コア個別エラー格納用)
static __declspec(thread) DWORD s_dwRfwErrCoreInfo[CORE_MAX];		// エラー発生コア番号格納変数(コア個別エラー格納用)
static __declspec(thread) DWORD s_dwRfwErrNoCoreCnt;				// エラー番号格納変数(コア個別エラー格納用) 情報格納カウンタ

static __declspec(thread) DWORD s_dwRfwErrNoCoreTmp[CORE_MAX];		// エラー番号格納TMP変数(コア個別エラー格納用)
static __declspec(thread) DWORD s_dwRfwErrCoreInfoTmp[CORE_MAX];	// エラー発生コア番号格納TMP変数(コア個別エラー格納用)
static __declspec(thread) DWORD s_dwRfwErrNoCoreTmpCnt;				// エラー番号格納TMP変数(コア個別エラー格納用) 情報格納カウンタ

static __declspec(thread) DWORD s_dwRetWarning;						// ワーニング発生時のエラー番号格納変数

// static関数の宣言



//==============================================================================
/**
 * 共通RFWエラー番号の登録
*  [注意事項]
*   本関数の引数dwRfwErrNoには、RFWERR_OKを指定してもよい。
*   RFWERR_OK を指定した場合は、RFW_GetErroInfoエラー管理情報に反映しない。
*
*   ワーニング情報が既に登録されている場合
*   dwRfwErrNoがエラーの場合、ワーニング状態はクリアする。
*
* @param DWORD dwRfwErrNo RFW共通エラー番号
* @return DWORD 引数dwRfwErrNoで登録したエラー番号。
*         ワーニングの場合は、RFWERR_OKに置き換えて戻り値を返送する。
*/
//==============================================================================
DWORD SetCommErrNo(DWORD dwRfwErrNo)
{
	DWORD dwRet = RFWERR_OK;		// 関数の戻り値で返送するエラーコード(WarringをRFWERR_OKに置き換えしたもの)
	BOOL bNowWarrning = FALSE;		// 今回登録するエラーのワーニング該当有無情報

	// RFWERR_OKでない場合のみエラー情報を登録
	if (dwRfwErrNo != RFWERR_OK) {
		//---------------------------------
		// 今回のエラー情報種別判定
		//---------------------------------
		// ワーニング情報登録とワーニング判定
		if (SetWarning(dwRfwErrNo)) {
			// ワーニング
			bNowWarrning = TRUE;
		} else {
			// エラー
			bNowWarrning = FALSE;
		}

		//---------------------------------
		// エラー情報の登録
		//---------------------------------
		// 現在ワーニング発生中でエラーを上書きする場合
		if (((WarningChk() & RFWERR_WARRING_BIT) == RFWERR_WARRING_BIT)
		        && (bNowWarrning == FALSE)) {
			// ワーニング情報をクリア
			ClrWarning();
		}

		// エラー情報の登録
		s_dwRfwErrNoCore[0] = dwRfwErrNo;

		// 関数戻り値を更新
		dwRet = dwRfwErrNo;
		if (bNowWarrning) {
			// ワーニングの場合は、RFWERR_OKに置き換える。
			dwRet = RFWERR_OK;
		}

		// コア個別エラー
		s_dwRfwErrType = RF_ERR_COMMON;

		// RFWERR_OKで且つ、ワーニング発生時にワーニング情報を登録
	} else {
		// ワーニング発生中
		if ((WarningChk() & RFWERR_WARRING_BIT) == RFWERR_WARRING_BIT) {
			// ワーニング情報の登録
			s_dwRfwErrNoCore[0] = WarningChk();
			// 関数戻り値
			dwRet = RFWERR_OK;
			// コア個別エラー
			s_dwRfwErrType = RF_ERR_COMMON;
		}
	}

	return dwRet;
}



//==============================================================================
/**
 * 共通RFWエラー番号の取得
 * @param なし
 * @retval DWORD s_dwRfwErrNoCore[0]  共通エラー番号
 */
//==============================================================================
DWORD GetCommErrNo(void)
{
	return s_dwRfwErrNoCore[0];
}

//==============================================================================
/**
 * RFWエラーの種別取得
 * @param なし
 * @retval DWORD s_dwRfwErrType　エラーの種別(共通/コア個別)
 */
//==============================================================================
DWORD GetCommErrType(void)
{
	return s_dwRfwErrType;
}

//==============================================================================
/**
 * RFWエラーの情報登録数
 * @param なし
 * @retval DWORD s_dwRfwErrNoCoreCnt　個別エラーの登録総数
 */
//==============================================================================
DWORD GetRfwErrNoCoreCnt(void)
{
	return s_dwRfwErrNoCoreCnt;
}

//==============================================================================
/**
 * 共通RFWエラーメッセージの取得
 * @param DWORD dwRfwErrNo RFWエラー番号
 * @param BYTE byMsgLanguage メッセージ文字列の言語
 * @param WCHAR* swzRfwErrMsg エラーメッセージ格納ポインタ
 * @retval なし
 */
//==============================================================================
void GetCommErrMsg(DWORD dwRfwErrNo, BYTE byMsgLanguage, WCHAR* swzRfwErrMsg)
{
	const ERRMSG_DATA* pErrMsg = GetErrMsgPtr();

	for (;;) {
		if ((pErrMsg->dwRfwErrNo == dwRfwErrNo) || (pErrMsg->dwRfwErrNo == RFWERR_N_NOMESSAGE_ERROR)) {
			if (byMsgLanguage == RF_MSG_ENG) {
				wcscpy_s(swzRfwErrMsg, ERRMSGINFO_ERRMSG_MAX, pErrMsg->pswzErrMsgEn);
			} else {
				wcscpy_s(swzRfwErrMsg, ERRMSGINFO_ERRMSG_MAX, pErrMsg->pswzErrMsgJp);
			}
			break;
		}
		++pErrMsg;
	}
}

//==============================================================================
/**
*  コア個別RFWエラー番号をテンポラリに登録
* @param DWORD dwCoreInfo コア番号格納変数
* @param DWORD dwRfwErrNo エラー番号格納変数
* @return DWORD 引数dwRfwErrNoで登録したエラー番号。
*         ワーニングの場合は、RFWERR_OKに置き換えて戻り値を返送する。
*         RFW_GetErrInfoのエラー管理情報へは、ワーニング情報を含めたエラーを反映する。
*/
//==============================================================================
DWORD SetTmpCoreErrNo(DWORD dwCoreInfo, DWORD dwRfwErrNo)
{

	// エラー番号格納変数(コア個別エラー格納用)　にエラー番号登録
	s_dwRfwErrNoCoreTmp[s_dwRfwErrNoCoreTmpCnt] = dwRfwErrNo;
	// エラー発生コア番号格納変数(コア個別エラー格納用) にコア番号登録
	s_dwRfwErrCoreInfoTmp[s_dwRfwErrNoCoreTmpCnt] = dwCoreInfo;
	// エラー番号格納変数(コア個別エラー格納用) 情報格納カウンタ
	s_dwRfwErrNoCoreTmpCnt++;

	// ワーニングの判定
	if (SetWarning(dwRfwErrNo)) {
		// 関数戻り値のエラー番号をRFWERR_OKに置き換える
		dwRfwErrNo = RFWERR_OK;
	}

	return dwRfwErrNo;

}

//==============================================================================
/**
* テンポラリのエラー情報をクリア
*　SetTmpCoreErrNo関数で登録したエラー番号をクリア
* @param なし
* @return なし
*/
//==============================================================================
void ClrTmpCoreErrNo(void)
{
	// コア個別エラーのTMP初期化
	memset(s_dwRfwErrNoCoreTmp, RFWERR_OK, sizeof(s_dwRfwErrNoCoreTmp));
	memset(s_dwRfwErrCoreInfoTmp, RF_PE1, sizeof(s_dwRfwErrCoreInfoTmp));
	s_dwRfwErrNoCoreTmpCnt = 0;
}

//==============================================================================
/**
* 登録したエラー番号をテンポラリから取得(ワーニング置き換えあり)
*
* @param DWORD dwCoreInfo コア番号格納変数
* @return DWORD dwRfwErrNoCoreTmp SetTmpCoreErrNo関数で登録したエラー番号で
*         dwCoreInfoに対応したエラー番号。
*         ワーニングの場合は、RFWERR_OKに置き換えて戻り値を返送する。
*/
//==============================================================================
DWORD GetTmpCoreErrNo_WarningClr(DWORD dwCoreInfo)
{
	DWORD dwCnt;
	DWORD dwRfwErrNoCoreTmp = RFWERR_OK;

	for (dwCnt = 0; dwCnt < s_dwRfwErrNoCoreTmpCnt; dwCnt++) {
		if (s_dwRfwErrCoreInfoTmp[dwCnt] == dwCoreInfo) {
			dwRfwErrNoCoreTmp = s_dwRfwErrNoCoreTmp[dwCnt];
		}
	}

	// ワーニングの判定
	if (SetWarning(dwRfwErrNoCoreTmp)) {
		// 関数戻り値のエラー番号をRFWERR_OKに置き換える
		dwRfwErrNoCoreTmp = RFWERR_OK;
	}

	return dwRfwErrNoCoreTmp;
}

//==============================================================================
/**
* 登録したエラー番号をテンポラリから取得(ワーニング置き換えなし)
*
* @param DWORD dwCoreInfo コア番号格納変数
* @return  DWORD dwRfwErrNoCoreTmp SetTmpCoreErrNo関数で登録したエラー番号で
*                dwCoreInfoに対応したエラー番号。
*/
//==============================================================================
DWORD GetTmpCoreErrNo(DWORD dwCoreInfo)
{
	DWORD dwCnt;
	DWORD dwRfwErrNoCoreTmp = RFWERR_OK;

	for (dwCnt = 0; dwCnt < s_dwRfwErrNoCoreTmpCnt; dwCnt++) {
		if (s_dwRfwErrCoreInfoTmp[dwCnt] == dwCoreInfo) {
			dwRfwErrNoCoreTmp = s_dwRfwErrNoCoreTmp[dwCnt];
		}
	}

	return dwRfwErrNoCoreTmp;
}

#if 0
//==============================================================================
/**
* テンポラリのエラー情報をエラー管理情報に反映
* SetTmpCoreErrNo関数で登録したエラー番号をRFW_GetErrInfoのエラー管理情報に登録
*   s_dwRfwErrNoCore[CORE_MAX];			// エラー番号格納変数(コア個別エラー格納用)
*   s_dwRfwErrCoreInfo[CORE_MAX];		// エラー発生コア番号格納変数(コア個別エラー格納用)
*   s_dwRfwErrNoCoreCnt;				// エラー番号格納変数(コア個別エラー格納用) 情報格納カウンタ
*
* @param DWORD dwCoreNum コア数
* @param const DWORD *pdwCoreInfo コア番号格納配列ポインタ
* @return なし
*/
//==============================================================================
void SetCoreErrNo(DWORD dwCoreNum, const DWORD *pdwCoreInfo)
{

	BOOL bRfwWarring = FALSE;
	BOOL bCoreNumNG = FALSE;
	DWORD dwCnt;
	DWORD dwTmpCnt;
	DWORD dwAllCoreNum;				// 実装コア数
	RFW_MCUFILE_DBGINFO *pDbgInfo;
	DWORD*	pdwAllDbgCoreInfo;		// デバッグ対象コア情報(パワーダウンコア含む)

	DWORD dwCount1;
	DWORD dwCount2;
	DWORD dwRetTmp = RFWERR_OK;
	DWORD dwAllDbgCoreNum;
	DWORD dwStoreCnt = 0;
	DWORD dwCoreInfoTmp[CORE_MAX];


	pDbgInfo = GetMCUFileDbgInfo();
	// 実装コア数の確認
	dwAllCoreNum = pDbgInfo->dwCoreNum;

	// dwCoreInfoTmpの初期化
	memset(dwCoreInfoTmp, 0x00, sizeof(dwCoreInfoTmp));


	// pdwAllDbgCoreInfoのエリア確保
	if (dwAllCoreNum != 0) {
		pdwAllDbgCoreInfo = new DWORD [dwAllCoreNum];
	} else if (dwCoreNum != 0) {
		pdwAllDbgCoreInfo = new DWORD [dwCoreNum];
	} else {
		pdwAllDbgCoreInfo = new DWORD [CORE_MAX];
	}

	// デバッグ対象コアの確認(パワーダウンコア含む)
	GetDbgCoreAllDbgConnectInfo(dwAllCoreNum, &pdwAllDbgCoreInfo[0], &dwAllDbgCoreNum);

	//RFW_CONNECT前の対応
	if (dwAllDbgCoreNum == 0) {
		dwAllDbgCoreNum = dwCoreNum;
		for (dwCount2 = 0; dwCount2 < dwAllDbgCoreNum; ++dwCount2) {
			*(pdwAllDbgCoreInfo + dwCount2) =  *(pdwCoreInfo  + dwCount2);
		}
	} 


	//[注意] これ以降は、pdwCoreInfoを使用しない。
	//  ソートしたdwCoreInfoTmpを使用する。


	// RFW_Connet前は コア数不正チェックは行わない。
	if (dwAllDbgCoreNum != 0) {
		// dwCoreNum,dwCoreInfo
		if ((dwCoreNum < 1) || (dwCoreNum > dwAllDbgCoreNum)) {
			// 同時制御コア数範囲外の指定エラー
			dwRetTmp = RFWERR_N_RFW_ARG;
			// dwCoreNum不正時は、引数エラーソートをしない。
			bCoreNumNG = TRUE;

		} else {
			// 指定対象コアがデバッグ対象コアであるかをチェック
			for (dwCount1 = 0; dwCount1 < dwCoreNum; ++dwCount1) {
				if (dwRetTmp == RFWERR_OK) {
					dwRetTmp = RFWERR_N_RFW_ARG;
					for (dwCount2 = 0; dwCount2 < dwAllDbgCoreNum; ++dwCount2) {
						if ( *(pdwCoreInfo+dwCount1) == pdwAllDbgCoreInfo[dwCount2]) {
							dwRetTmp = RFWERR_OK;
						}
					}
				}
			}
			if (dwRetTmp == RFWERR_OK) {
				// 複数の指定対象コア内に同一コア番号が存在するかをチェック
				if (dwCoreNum > 1) {
					for (dwCount1 = 0; dwCount1 < (dwCoreNum - 1); ++dwCount1) {
						for (dwCount2 = (dwCount1 + 1); dwCount2 < dwCoreNum; ++dwCount2) {
							if ( *(pdwCoreInfo + dwCount1) == *(pdwCoreInfo + dwCount2)) {
								dwRetTmp = RFWERR_N_RFW_ARG;
							}
						}
					}
				}
			}
		}

		// 引数エラー格納
		if (dwRetTmp != RFWERR_OK) {	// CoreNum,CoreInfoの引数エラー格納

			for (dwCount2 = 0; dwCount2 < dwAllCoreNum; ++dwCount2) {
				s_dwRfwErrCoreInfo[dwCount2] =  pdwAllDbgCoreInfo[dwCount2];
				s_dwRfwErrNoCore[dwCount2] = RFWERR_N_RFW_ARG;

			}
			// コア個別エラー
			s_dwRfwErrType = RF_ERR_CORE;

			// dwCoreNum不正時は、デバッグ有効コア分を登録
			if (bCoreNumNG) {
				s_dwRfwErrNoCoreCnt = dwAllCoreNum;
			} else {
				s_dwRfwErrNoCoreCnt = dwCoreNum;
			}
			delete [] pdwAllDbgCoreInfo;
			return;
		}
	}


	//[注意] これ以降は、pdwCoreInfoを使用しない。
	//  ソートしたdwCoreInfoTmpを使用する。

	// 操作コアのソート
	if (dwAllDbgCoreNum != 0) {
		// コア番号のソート
		dwStoreCnt = 0;
		dwCount2 = RF_PE1;
		for (dwCount1 = 0; dwCount1 < CORE_MAX; ++dwCount1) {
			for (dwCount2=0; dwCount2 < dwCoreNum; ++dwCount2) {
				if (*(pdwCoreInfo+dwCount2) == dwCount1) {
					dwCoreInfoTmp[dwStoreCnt++]= *(pdwCoreInfo+dwCount2);
					break;
				}
				// CORE_MAXを超えるCORE_NUMが指定されている場合、ソートを中断
				if (dwCount2 == (CORE_MAX-1)) {
					break;
				}
			}
			if (dwStoreCnt == dwCoreNum) {
				break;
			}
		}
	}



	// デバッグ対象コアの確認
	dwAllCoreNum = pDbgInfo->dwCoreNum;

	// TMPエラーがすべてRFWERR_OKか？
	for (dwCnt = 0; dwCnt < s_dwRfwErrNoCoreTmpCnt; dwCnt++) {
		if (s_dwRfwErrNoCoreTmp[dwCnt] != RFWERR_OK) {
			break;
		}
	}

	// TMPエラーがすべてRFWERR_OKの場合
	if (dwCnt == s_dwRfwErrNoCoreTmpCnt) {
		// TMPエラーがすべてRFWERR_OKでワーニング発生時
		if ((WarningChk() & RFWERR_WARRING_BIT) == RFWERR_WARRING_BIT) {
			bRfwWarring = TRUE;
			// TMPエラーがすべてRFWERR_OKでワーニングなし時
		} else {
			delete [] pdwAllDbgCoreInfo;
			// エラー更新をしないで終了
			return;
		}
		// RFWERR_OK以外のエラーがTMPに含まれる場合
	}

	// エラー番号格納変数(コア個別エラー格納用) 情報格納カウンタをデバッグ対象コア数に設定
	s_dwRfwErrNoCoreCnt = dwAllCoreNum;

	// ワーニング発生時は、デバッグ対象コア分ワーニング情報を格納する。
	if ((WarningChk() & RFWERR_WARRING_BIT) == RFWERR_WARRING_BIT) {
		for (dwCnt = 0; dwCnt < dwAllCoreNum; ++dwCnt) {
			s_dwRfwErrNoCore[dwCnt] = WarningChk();
			s_dwRfwErrCoreInfo[dwCnt] = pDbgInfo->dwCoreInfo[dwCnt];
		}
		// ワーニング未発生の場合は、デバッグ対象コア分RFWERR_OKを格納する。
	} else {
		for (dwCnt = 0; dwCnt < dwAllCoreNum; ++dwCnt) {
			s_dwRfwErrNoCore[dwCnt] = RFWERR_OK;
		}
		// 操作しているコア数がない場合(RFW_Connectの場合など)
		if (dwCoreNum == 0) {
			for (dwCnt = 0; dwCnt < CORE_MAX; ++dwCnt) {
				s_dwRfwErrCoreInfo[dwCnt] = dwCnt;
			}
		// 操作しているコア数がある場合(RFW_Connect正常時以降)
		} else {

			// 操作したコア分に対してコア番号を割り当てる。
			for (dwCnt = 0; dwCnt < CORE_MAX; ++dwCnt) {
				if  (dwCnt < dwCoreNum) { 
					//s_dwRfwErrCoreInfo[dwCnt] = *(pdwCoreInfo + dwCnt);
					s_dwRfwErrCoreInfo[dwCnt] = dwCoreInfoTmp[dwCnt];
				} else {
					// 該当しないコア番号をいれておく
					s_dwRfwErrCoreInfo[dwCnt] = CORE_MAX;
				}		
			}
		}
	}

	// すべてのコア番号を登録していないケースに対応
	// s_dwRfwErrCoreInfoTmpに未登録のコア番号を定義する。
	if (s_dwRfwErrNoCoreTmpCnt < dwCoreNum) {

		// s_dwRfwErrCoreInfoTmpにコア番号が定義されていない数
		dwStoreCnt = dwCoreNum - s_dwRfwErrNoCoreTmpCnt; 
		// s_dwRfwErrCoreInfoTmpにコア番号を定義した数
		dwCount1 = 0;

		for (dwTmpCnt=0; dwTmpCnt < dwCoreNum; dwTmpCnt++) {
			for (dwCnt=dwCount1; dwCnt < dwCoreNum; dwCnt++) {
				if (s_dwRfwErrCoreInfo[dwTmpCnt] == s_dwRfwErrCoreInfoTmp[dwCnt]) {
					break;
				}
				if (s_dwRfwErrCoreInfo[dwTmpCnt] != s_dwRfwErrCoreInfoTmp[dwCnt]) {
					if (dwStoreCnt <= dwCnt) {
						s_dwRfwErrCoreInfoTmp[dwStoreCnt] = s_dwRfwErrCoreInfo[dwTmpCnt];
						dwStoreCnt++;
						dwCount1++;
						break;
					} 
				}
			}	
			if ((dwCoreNum - s_dwRfwErrNoCoreTmpCnt) == dwCount1) {	
				break;	
			}
		}

	}

	// エラー番号格納変数(コア個別エラー格納用) 情報格納カウンタを登録
	if (s_dwRfwErrNoCoreTmpCnt > 0) {
		dwStoreCnt = 0; 
		for (dwCnt = 0; dwStoreCnt < dwCoreNum; ++dwCnt) {
			for (dwTmpCnt = 0; dwStoreCnt < dwCoreNum; dwTmpCnt++) {
				if (*(pdwCoreInfo+dwStoreCnt) == s_dwRfwErrCoreInfoTmp[dwTmpCnt]) {
					s_dwRfwErrCoreInfo[dwStoreCnt] = s_dwRfwErrCoreInfoTmp[dwTmpCnt];
					s_dwRfwErrNoCore[dwStoreCnt] = s_dwRfwErrNoCoreTmp[dwTmpCnt];
					dwStoreCnt++;
					break;
				}
				if (dwTmpCnt == CORE_MAX) {
					break;
				}
			}
			if (dwCnt == CORE_MAX) {
				break;
			}
		}
	}
	// コア個別エラー
	s_dwRfwErrType = RF_ERR_CORE;

	delete [] pdwAllDbgCoreInfo;

	return;
}
#endif

//=============================================================================
/**
 * Warning発生したか確認
 * @param なし
 * @retval DWORD s_dwRetWarning ワーニング対象のエラー番号
 */
//=============================================================================
DWORD WarningChk(void)
{
	DWORD	dwRet = RFWERR_OK;

	dwRet = s_dwRetWarning;
	// 変数クリア
	//ClrWarning();

	return dwRet;
}

//=============================================================================
/**
 * Warning発生した場合、内部変数にワーニング内容を保持
 * @param dwRet RFWエラーコード
 * @retval BOOL ワーニング該当有無 (TRUE:エラーがBFWからのワーニング/FALSE: エラーもしくはRFWERR_OK
 */
//=============================================================================
BOOL SetWarning(DWORD dwRet)
{
	BOOL bWarningCode = FALSE;

	switch (dwRet) {
	// エラーがRFWERR_W_SBPOINT_DELETE(ワーニング)の場合、RFW内部処理としてはワーニングとして扱わない
	case RFWERR_W_SBPOINT_DELETE:
		bWarningCode = FALSE;
		break;
	case RFWERR_W_BMCU_PDOWN:
		bWarningCode = FALSE;
		break;
	case RFWERR_W_BMCU_OSDLK:
		bWarningCode = FALSE;
		break;
	default:
		// ワーニングビットがセットされている場合、RFW内部処理としてはワーニングとして扱う
		if ((dwRet & RFWERR_WARRING_BIT) == RFWERR_WARRING_BIT) {
			bWarningCode = TRUE;
		} else {
			bWarningCode = FALSE;
		}
		break;
	}

	// ワーニングの場合はエラーを内部管理変数へ保持しておく
	if (bWarningCode != FALSE) {
		s_dwRetWarning = dwRet;
	}

	return bWarningCode;
}

//=============================================================================
/**
 * Warning発生した場合、内部変数にワーニング内容をクリア
 * @param なし
 * @retval なし
 */
//=============================================================================
void ClrWarning(void)
{
	// ワーニング情報格納バッファの初期化
	s_dwRetWarning = RFWERR_OK;

	// コア個別エラーのTMP初期化
	ClrTmpCoreErrNo();

	return;
}

//=============================================================================
/**
 * エラー番号格納変数(コア個別エラー格納用)ポインタの取得
 * @param なし
 * @retval DWORD* s_dwRfwErrNoCore配列ポインタ
 */
//=============================================================================
DWORD*  GetRfwErrNoCorePtr(void)
{
	return &s_dwRfwErrNoCore[0];
}

//=============================================================================
/**
 * エラー発生コア番号格納変数ポインタの取得
 * @param なし
 * @retval DWORD* s_dwRfwErrCoreInfo配列ポインタ
 */
//=============================================================================
DWORD*  GetRfwErrCoreInfoPtr(void)
{
	return &s_dwRfwErrCoreInfo[0];
}

//=============================================================================
/**
 * システムコマンド用RFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitRfwCmdData_SysErr(void)
{
	// 共通エラー
	s_dwRfwErrType = RF_ERR_COMMON;

	memset(s_dwRfwErrNoCore, RFWERR_OK, sizeof(s_dwRfwErrNoCore));
	memset(s_dwRfwErrCoreInfo, RF_PE1, sizeof(s_dwRfwErrCoreInfo));
	s_dwRfwErrNoCoreCnt = 0;

	memset(s_dwRfwErrNoCoreTmp, RFWERR_OK, sizeof(s_dwRfwErrNoCoreTmp));
	memset(s_dwRfwErrCoreInfoTmp, RF_PE1, sizeof(s_dwRfwErrCoreInfoTmp));
	s_dwRfwErrNoCoreTmpCnt = 0;

	return;
}
