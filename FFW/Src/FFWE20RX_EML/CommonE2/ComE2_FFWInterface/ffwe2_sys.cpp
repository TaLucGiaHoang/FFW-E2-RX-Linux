///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwe2_sys.cpp
 * @brief システムコマンドの実装ファイル
 * @author PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#include "ffwe2_sys.h"
#include "doe2_sys.h"
#include "do_sys.h"
#include "emudef.h"
#include "ffwmcu_mcu.h"		// getIdCodeResultState()呼び出し用
#include "prot_sys.h"		// TRANS_MODE_EML/TRANS_MODE_LV0用
// RevRxE2No171004-001 Append Start
#include "domcu_prog.h"
#include "doasp_pwr.h"
// RevRxE2No171004-001 Append End

// ファイル内static変数の宣言
static enum FFWENM_EXPWR2_MODE s_eNowExPwr2Mode = EXPWR2_DISABLE;	// 電源供給モード保存変数(E2用)
// RevRxE2No171004-001 Append Line
static BYTE s_byNowExPwr2Val = 0;	// 電源供給電圧値保存変数(E2用)(RFWで使用)

///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ユーザ基板への電源供給を設定(リニア指定)
 * @param eExPwr2Mode 電源供給モード設定
 * @param byExPwr2Val 電源供給電圧
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_EXTPWR2 (enum FFWENM_EXPWR2_MODE eExPwr2Mode, BYTE byExPwr2Val)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferr2;
	FFWE20_EINF_DATA	einfGetUvcc;
	float	fCmpUvcc;
	FFWE20_EINF_DATA	Einf;
	BYTE	bySupply;
	BYTE	byBfwTransMode;

	// エミュレータ情報をEINFコマンド処理を実行して取得する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		return ferr;
	}

	if (Einf.wEmuStatus != EML_E2) {	// E2でない場合はエラー
		return FFWERR_FUNC_UNSUPORT;
	}

	// パラメータのエラーチェック
	if ((eExPwr2Mode != EXPWR2_DISABLE) && (eExPwr2Mode != EXPWR2_USER) && (eExPwr2Mode != EXPWR2_ASP) &&
		(eExPwr2Mode != EXPWR2_TARGET_ASPIN) && (eExPwr2Mode != EXPWR2_USER_ASP)) {	// 電源供給モード設定値のエラーチェック
		return FFWERR_FUNC_UNSUPORT;
	}
	if ((eExPwr2Mode == EXPWR2_USER) || (eExPwr2Mode == EXPWR2_ASP) || (eExPwr2Mode == EXPWR2_USER_ASP)) {
																					//電源供給するモードの場合
		if ((byExPwr2Val != 0x00) && ((byExPwr2Val < 0x12) || (byExPwr2Val > 0x32))) {	// 電源供給電圧設定値のエラーチェック
			return FFWERR_FUNC_UNSUPORT;
		}
	}

	// ユーザシステム電源状態確認(E2は、BFW レベル0でチェックしていないためFFWで実施する)
	// 起動時の関数発行手順で、EXTPWR2発行前に必ずEINFを発行することになっているのでその情報を利用する
	// EINFに合わせて1.20以上とする
	if (Einf.flNowUVCC >= UVCC_1_20V) {	// 1.20V以上の電源が既に供給されている場合
		if (Einf.byBfwMode == TRANS_MODE_LV0) {	// LV0動作モードの場合
			byBfwTransMode = TRANS_MODE_EML;
			if ((ferr = DO_E20Trans(byBfwTransMode)) != FFWERR_OK) {		// DO_GetPwrSrc()呼び出しのため、BFWプログラムをEMLに遷移
				return ferr;
			}
			if ((ferr = DO_GetPwrSrc(&bySupply)) != FFWERR_OK) {			// 電源供給ソース情報取得
				return ferr;
			}
			if ((ferr = DO_E20Trans(Einf.byBfwMode)) != FFWERR_OK) {	// BFWプログラムをLV0に遷移(元に戻す)
				return ferr;
			}
		} else {									// EML動作モードの場合
			if ((ferr = DO_GetPwrSrc(&bySupply)) != FFWERR_OK) {			// 電源供給ソース情報取得
				return ferr;
			}
		}

		if (bySupply == EXT_POWER) {				// エミュレータからユーザシステムに電源供給されていない(外部電源から供給)
			if ((eExPwr2Mode == EXPWR2_USER) || (eExPwr2Mode == EXPWR2_USER_ASP)) {
														// エミュレータからユーザシステムへ電源供給するモードの場合
				if ((byExPwr2Val != 0x00)) {				// 電源供給電圧が0Vでない場合
					return FFWERR_BTARGET_POWERSUPPLY;
				}
			}
		}
	}

	// RevRxE2No171004-001 Append Start
#if defined(E2E2L)
	// パワーモニタ有効かつユーザプログラム実行中の供給電圧または電源供給モードの変更不可
	if (IsAspPwrMonEna() && GetMcuRunState()) {
		if ((byExPwr2Val != s_byNowExPwr2Val) || (eExPwr2Mode != s_eNowExPwr2Mode)) {
			return FFWERR_ASP_NO_CHG_POWERSUPPLY;
		}
	}
#endif
	// RevRxE2No171004-001 Append End

	if (eExPwr2Mode != EXPWR2_DISABLE) {	// 電源供給する場合
		if (getIdCodeResultState() == FALSE) {	// 起動時(ID認証結果が未設定状態)
			if ((Einf.flNowUVCC >= UVCC_1_20V) || (s_eNowExPwr2Mode != EXPWR2_DISABLE)) {
													// 1.20V以上の電源が既に供給されている場合、または、既に電源供給設定済みの場合
				ferr = DO_EXPWR2(EXPWR2_DISABLE, byExPwr2Val);	// 一旦ユーザシステムへの電源供給停止
			}
			ferr = DO_EXPWR2(eExPwr2Mode, byExPwr2Val);
		} else {								// 接続中
			if (eExPwr2Mode == s_eNowExPwr2Mode) {	// 電源供給モードが現電源供給モードと同じ
				ferr = DO_CHGPWR(byExPwr2Val);			// 電源供給電圧のみ変更
			} else {								// 電源供給モードを変更
				ferr = DO_EXPWR2(EXPWR2_DISABLE, byExPwr2Val);	// 一旦ユーザシステムへの電源供給停止
				ferr = DO_EXPWR2(eExPwr2Mode, byExPwr2Val);
			}
		}
	} else {								// 電源供給しない場合
		ferr = DO_EXPWR2(eExPwr2Mode, byExPwr2Val);
	}

	if (ferr == FFWERR_OK) {
		// RevRxE2No171004-001 Modify Line
		SaveExPwr2Setting(eExPwr2Mode, byExPwr2Val);	// 電源供給設定を保存
	}

	// エミュレータから電源供給後の電圧値確認
	if ((ferr2 = DO_GetE20EINF(&einfGetUvcc)) != FFWERR_OK) {
		return ferr2;
	}
	if (ferr == FFWERR_OK) {
		if ((eExPwr2Mode == EXPWR2_USER) || (eExPwr2Mode == EXPWR2_USER_ASP)) {
												// エミュレータからユーザシステムに電源供給する場合
			fCmpUvcc = (float)byExPwr2Val / 10;
			fCmpUvcc = fCmpUvcc * (float)0.9;
			if (einfGetUvcc.flNowUVCC < fCmpUvcc) {	// (供給電圧値×0.9)未満の場合
				ferr = FFWERR_TARGET_POWERLOW;			// ユーザシステムの電圧が供給電圧より低い(Warning)を設定
			}
		}
	}

	return ferr;
}

// RevRxE2No171004-001 Modify Start
//=============================================================================
/**
 * 電源供給設定を保存(E2用)
 * @param eExPwr2Mode 電源供給モード設定
 * @param byExPwr2Val 電源供給電圧設定
 * @retval なし
 */
//=============================================================================
void SaveExPwr2Setting(enum FFWENM_EXPWR2_MODE eExPwr2Mode, BYTE byExPwr2Val)
{
	s_eNowExPwr2Mode = eExPwr2Mode;		// 電源供給モード設定を保存
	s_byNowExPwr2Val = byExPwr2Val;		// 電源供給電圧設定を保存

	if (eExPwr2Mode == EXPWR2_DISABLE || eExPwr2Mode == EXPWR2_TARGET_ASPIN) {	// 電源供給しない場合
		s_byNowExPwr2Val = 0;
	}

	if (((eExPwr2Mode == EXPWR2_USER) || (eExPwr2Mode == EXPWR2_ASP) || (eExPwr2Mode == EXPWR2_USER_ASP)) && (byExPwr2Val == 0x00)) {	// 電源供給電圧が0V指定の場合
		s_eNowExPwr2Mode = EXPWR2_DISABLE;	// LV0の仕様に合わせて電源供給OFF設定を保持
	}

	return;
}
// RevRxE2No171004-001 Modify End

//=============================================================================
/**
 * 電源供給モード設定を取得(E2用)
 * @param なし
 * @retval s_eNowExPwr2Mode 現電源供給モード設定
 */
//=============================================================================
enum FFWENM_EXPWR2_MODE GetExPwr2Mode(void)
{
	return s_eNowExPwr2Mode;
}

// RevRxE2No171004-001 Append Start
//=============================================================================
/**
* 電源供給電圧設定を取得(E2用)
* @param なし
* @retval s_eNowExPwr2Mode 現電源供給電圧設定
*/
//=============================================================================
BYTE GetExPwr2Val(void)
{
	return s_byNowExPwr2Val;
}
// RevRxE2No171004-001 Append End

