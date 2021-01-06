////////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_runset.cpp
 * @brief プログラム実行中の設定変更コマンドのソースファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/

#include "domcu_runset.h"
#include "ffwmcu_brk.h"


// FFW 内部変数
static DWORD s_EvCmdExeFlg;						// 各コマンド呼び出し確認フラグ


//=============================================================================
/**
 * FFW 内部変数を更新する。
 * @param なし
 * @retval なし
 */
//=============================================================================
void UpdateRunsetData(void)
{

	int				eCmdNum;		// コマンド番号
	BOOL			bRet;
	/// SetBM コマンド関連の内部変数を更新 
	eCmdNum = RUNSET_CMDNUM_BM;
	bRet = GetEvCmdExeFlg(eCmdNum);
	if (bRet) {
				// SetBM コマンドが発行されている場合
		SetBmData();
	}

	//!!!!RUN中の変更を対応する際にはイベント関連のコマンドも考慮する必要がある!!!!

	return;

}

//==============================================================================
/**
 * 指定したコマンドが発行されているかを取得する。
 * @param eCmdNum 発行有無を調べるコマンドの番号
 * @retval TRUE 指定したコマンドが発行されている
 * @retval FALSE 指定したコマンドが発行されていない
 */
//==============================================================================
BOOL GetEvCmdExeFlg(int eCmdNum)
{

	DWORD	dwShift;
	BOOL	bRet;

	dwShift = 1;
	if (s_EvCmdExeFlg & (dwShift << eCmdNum)) {
				// 指定したコマンドが発行されている場合
		bRet = TRUE;
	} else {
				// 指定したコマンドが発行されていない場合
		bRet = FALSE;
	}

	return bRet;

}
//==============================================================================
/**
 * プログラム実行中の連続コマンド対象コマンドが発行されたことを示すフラグを設定する。
 * @param eCmdNum 発行したコマンドの番号
 * @retval なし
 */
//==============================================================================
void SetEvCmdExeFlg(int eCmdNum)
{

	DWORD	dwShift;

	dwShift = 1;
	s_EvCmdExeFlg |= (dwShift << eCmdNum);

	return;

}

//=============================================================================
/**
 * プログラム実行中の設定変更コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Runset(void)
{


	/* 各コマンド呼び出し確認フラグを初期化 */
	s_EvCmdExeFlg = INIT_CMDEXE_FLG;


	return;

}
