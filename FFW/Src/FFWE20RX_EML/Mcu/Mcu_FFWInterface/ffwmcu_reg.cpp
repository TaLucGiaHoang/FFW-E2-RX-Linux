////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwmcu_reg.cpp
 * @brief レジスタ操作コマンドのソースファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/09/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/09/14 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
    ・FFWRXCmd_GetXREG()で、
      RX64x以外の場合、FFWはACC0G, ACC0H, ACC0L, ACC1G, ACC1H, ACC1L, EXTBの
      設定内容を無視する。
      RX64xの場合は、FFWはACCH, ACCLの設定内容を無視する。
    ・FFWRXCmd_GetXREG()で、
      RX64x以外の場合、ACC0G, ACC0H, ACC0L, ACC1G, ACC1H, ACC1L, EXTBの
      レジスタ参照値として0x0000_0000を格納する。
      RX64xの場合は、ACCH, ACCLのレジスタ参照値として0x0000_0000を格納する。
・RevRxNo121112-001 2012/11/13 植盛
  ・Class2のFPSWレジスタアクセス
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130301-001	2013/05/16 植盛、上田
  RX64M対応
・RevRxE2LNo141104-001 2014/11/20 上田
	E2 Lite対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
・RevRxNo180625-001,PB18047-CD01-001 2018/06/25 PA 辻
	RX66T-H/RX72T対応
*/
#include "ffwmcu_reg.h"
#include "domcu_reg.h"
#include "domcu_mem.h"
#include "domcu_prog.h"
#include "ffwmcu_mcu.h"
// V.1.02 RevNo110613-001 Append Line
#include "errchk.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line

// FFW内部変数
// 2008.9.9 INSERT_BEGIN_E20RX600(+1) {
static FFWMCU_REG_DATA_RX s_RegState_RX;	// MCUレジスタ管理変数(RX用)
// 2008.9.9 INSERT_END_E20RX600 }



// 2008.9.9 INSERT_BEGIN_E20RX600(+124) {
//==============================================================================
/**
 * MCU レジスタ値の設定を行う。
 * @param pReg 設定・参照する全レジスタ値を格納するFFWRX_REG_DATA 構造体のアドレス
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数の指定に誤りがある。
 * @retval FFWERR_BMCU_RUN ユーザプログラム実行中のため、コマンド処理を実行できない
 */
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_SetXREG(const FFWRX_REG_DATA* pReg)
{
	// V.1.02 RevNo110613-001 Append Line
	FFWERR	ferr = FFWERR_OK;
	int i;
	// RevRxNo120910-001 Append Line
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo120910-001 Append Line
	pMcuInfo = GetMcuInfoDataRX();	// MCU情報取得

	/* プログラム実行状態をチェック */
	if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
		// プログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	// PSWレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_PSW] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_PSW] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_PSW] = pReg->dwRegData[REG_NUM_RX_PSW];
		if (s_RegState_RX.dwRegData[REG_NUM_RX_PSW] & SM_BIT) {
			s_RegState_RX.dwRegData[REG_NUM_RX_R0] = s_RegState_RX.dwRegData[REG_NUM_RX_USP];
		} else {
			s_RegState_RX.dwRegData[REG_NUM_RX_R0] = s_RegState_RX.dwRegData[REG_NUM_RX_ISP];
		}
	}

	// 汎用レジスタ
	for (i = REG_NUM_RX_R0; i <= REG_NUM_RX_R15; i++) {
		if (pReg->eRegDataSetEnable[i] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[i] > REG_MAX_GENERAL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			s_RegState_RX.dwRegData[i] = pReg->dwRegData[i];
			if ((i == REG_NUM_RX_R0) && (pReg->eRegDataSetEnable[REG_NUM_RX_R0] == REG_ACCESS_ON)) {
				if (s_RegState_RX.dwRegData[REG_NUM_RX_PSW] & SM_BIT) {
					s_RegState_RX.dwRegData[REG_NUM_RX_USP] = s_RegState_RX.dwRegData[REG_NUM_RX_R0];
				} else {
					s_RegState_RX.dwRegData[REG_NUM_RX_ISP] = s_RegState_RX.dwRegData[REG_NUM_RX_R0];
				}
			}
		}
	}


	// ISPレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_ISP] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_ISP] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_ISP] = pReg->dwRegData[REG_NUM_RX_ISP];

		// SMビットが0の時はISPレジスタの値をR0レジスタに反映する
		if (!(s_RegState_RX.dwRegData[REG_NUM_RX_PSW] & SM_BIT)) {
			s_RegState_RX.dwRegData[REG_NUM_RX_R0] = s_RegState_RX.dwRegData[REG_NUM_RX_ISP];
		}
	}

	// USPレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_USP] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_USP] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_USP] = pReg->dwRegData[REG_NUM_RX_USP];

		// SMビットが1の時はUSPレジスタの値をR0レジスタに反映する
		if (s_RegState_RX.dwRegData[REG_NUM_RX_PSW] & SM_BIT) {
			s_RegState_RX.dwRegData[REG_NUM_RX_R0] = s_RegState_RX.dwRegData[REG_NUM_RX_USP];
		}
	}

	// PCレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_PC] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_PC] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_PC] = pReg->dwRegData[REG_NUM_RX_PC];
	}

	// INTBレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_INTB] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_INTB] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_INTB] = pReg->dwRegData[REG_NUM_RX_INTB];
	}

	// BPCレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_BPC] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_BPC] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_BPC] = pReg->dwRegData[REG_NUM_RX_BPC];
	}

	// BPSWレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_BPSW] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_BPSW] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_BPSW] = pReg->dwRegData[REG_NUM_RX_BPSW];
	}

	// FINTVレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_FINTV] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_FINTV] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_FINTV] = pReg->dwRegData[REG_NUM_RX_FINTV];
	}

	// FPSWレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_FPSW] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_FPSW] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_FPSW] = pReg->dwRegData[REG_NUM_RX_FPSW];
	}

	// CPENレジスタ
	if (pReg->eRegDataSetEnable[REG_NUM_RX_CPEN] == REG_ACCESS_ON) {
		// レジスタ値を設定する場合
		if (pReg->dwRegData[REG_NUM_RX_CPEN] > REG_MAX_CONTROL) {
			// 設定値が不正の場合
			return FFWERR_FFW_ARG;
		}
		s_RegState_RX.dwRegData[REG_NUM_RX_CPEN] = pReg->dwRegData[REG_NUM_RX_CPEN];
	}

	// RevRxNo120910-001 Modify Start
	// RevRxNo130411-001 Modify Line
	if (pMcuInfo->eCpuIsa == RX_ISA_RXV1) {	// RX V1アーキテクチャの場合
		// ACCHレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_ACCH] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_ACCH] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			s_RegState_RX.dwRegData[REG_NUM_RX_ACCH] = pReg->dwRegData[REG_NUM_RX_ACCH];
		}

		// ACCLレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_ACCL] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_ACCL] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			s_RegState_RX.dwRegData[REG_NUM_RX_ACCL] = pReg->dwRegData[REG_NUM_RX_ACCL];
		}
		
	} else {									// RX V2/RX V3アーキテクチャの場合
		// ACC0Gレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_ACC0G] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_ACC0G] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}

			// RevRxNo130301-001 Modify Line
			s_RegState_RX.dwRegData[REG_NUM_RX_ACC0G] = pReg->dwRegData[REG_NUM_RX_ACC0G] & MSK_REG_ACCG;
		}

		// ACC0Hレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_ACC0H] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_ACC0H] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			s_RegState_RX.dwRegData[REG_NUM_RX_ACC0H] = pReg->dwRegData[REG_NUM_RX_ACC0H];
		}

		// ACC0Lレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_ACC0L] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_ACC0L] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			s_RegState_RX.dwRegData[REG_NUM_RX_ACC0L] = pReg->dwRegData[REG_NUM_RX_ACC0L];
		}

		// ACC1Gレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_ACC1G] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_ACC1G] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			// RevRxNo130301-001 Modify Line
			s_RegState_RX.dwRegData[REG_NUM_RX_ACC1G] = pReg->dwRegData[REG_NUM_RX_ACC1G] & MSK_REG_ACCG;
		}

		// ACC1Hレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_ACC1H] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_ACC1H] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			s_RegState_RX.dwRegData[REG_NUM_RX_ACC1H] = pReg->dwRegData[REG_NUM_RX_ACC1H];
		}

		// ACC1Lレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_ACC1L] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_ACC1L] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			s_RegState_RX.dwRegData[REG_NUM_RX_ACC1L] = pReg->dwRegData[REG_NUM_RX_ACC1L];
		}

		// EXTBレジスタ
		if (pReg->eRegDataSetEnable[REG_NUM_RX_EXTB] == REG_ACCESS_ON) {
			// レジスタ値を設定する場合
			if (pReg->dwRegData[REG_NUM_RX_EXTB] > REG_MAX_CONTROL) {
				// 設定値が不正の場合
				return FFWERR_FFW_ARG;
			}
			s_RegState_RX.dwRegData[REG_NUM_RX_EXTB] = pReg->dwRegData[REG_NUM_RX_EXTB];
		}
	}
	// RevRxNo120910-001 Modify End

#if 0	
	//デバッグ用
	FFWERR ferr;
	ferr = DO_MCU_SetRXXREG(&s_RegState_RX);
#endif

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Apped End

}


//==============================================================================
/**
 * MCU レジスタ値の参照を行う。
 * @param pReg 設定・参照する全レジスタ値を格納するFFWRX_REG_DATA 構造体のアドレス
 * @retval FFWERR_OK 正常終了
 *
 * ※本関数の処理は、コールドスタート起動/ホットプラグ起動を考慮する必要がある。
 */
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_GetXREG(FFWRX_REG_DATA* pReg)
{
	// V.1.02 RevNo110613-001 Append Line
	FFWERR	ferr = FFWERR_OK;

	BOOL	bHotPlugState = FALSE;
	BOOL	bIdCodeResultState = FALSE;
	// RevRxNo120910-001 Append Line
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo120910-001 Append Line
	pMcuInfo = GetMcuInfoDataRX();	// MCU情報取得

	bHotPlugState = getHotPlugState();	// ホットプラグ設定状態を取得
	bIdCodeResultState = getIdCodeResultState();	// ID認証結果状態を取得

	// ホットプラグ未設定状態かID認証結果未設定状態の場合
	if ((bHotPlugState == FALSE) || (bIdCodeResultState == FALSE)) {
		if (GetMcuRunState()) {	// プログラム実行中エラー確認 // RevRxE2LNo141104-001 Modify Line
			return FFWERR_BMCU_RUN;
		}
	}

	memcpy(pReg, &s_RegState_RX, sizeof(FFWMCU_REG_DATA_RX));

	// RevRxNo130411-001 Modify Line
	if (pMcuInfo->eCpuIsa == RX_ISA_RXV1) {	// RX V1アーキテクチャの場合、ACC0GからEXTBのレジスタ参照値として0x0000_0000を格納する。
		// ACC0Gレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_ACC0G] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_ACC0G] = REG_NO_USE_VALUE;
		// ACC0Hレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_ACC0H] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_ACC0H] = REG_NO_USE_VALUE;
		// ACC0Lレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_ACC0L] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_ACC0L] = REG_NO_USE_VALUE;
		// ACC1Gレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_ACC1G] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_ACC1G] = REG_NO_USE_VALUE;
		// ACC1Hレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_ACC1H] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_ACC1H] = REG_NO_USE_VALUE;
		// ACC1Lレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_ACC1L] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_ACC1L] = REG_NO_USE_VALUE;
		// EXTBレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_EXTB] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_EXTB] = REG_NO_USE_VALUE;
	} else {								// RX V2/RX V3アーキテクチャの場合、、ACCH, ACCLのレジスタ参照値として0x0000_0000を格納する。
		// ACCHレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_ACCH] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_ACCH] = REG_NO_USE_VALUE;
		// ACCLレジスタ
		pReg->eRegDataSetEnable[REG_NUM_RX_ACCL] = REG_ACCESS_OFF;
		pReg->dwRegData[REG_NUM_RX_ACCL] = REG_NO_USE_VALUE;
	}
	// RevRxNo120910-001 Append End

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Apped End

}

//==============================================================================
/**
 * MCU レジスタ値の設定を行う。
 * @param pReg FFW MCUレジスタ管理変数のポインタ
 * @retval なし
 */
//==============================================================================
void SetRegData_RX(FFWMCU_REG_DATA_RX* pReg)
{
	memcpy(&s_RegState_RX, pReg, sizeof(FFWMCU_REG_DATA_RX));

	return;
}

//==============================================================================
/**
 * MCU レジスタ値の設定を取得。
 * @param なし
 * @retval FFW MCUレジスタ管理変数のポインタ
 */
//==============================================================================
FFWMCU_REG_DATA_RX* GetRegData_RX(void)
{
	FFWMCU_REG_DATA_RX*	pRet;

	pRet = &s_RegState_RX;

	return pRet;
}
//=============================================================================
/**
 * レジスタ内部管理変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwRegData(void)
{
	// RevRxNo121112-001 Append Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	pMcuInfo = GetMcuInfoDataRX();	// MCU情報取得
	// RevRxNo121112-001 Append End

	/* MCU レジスタの管理変数を初期化 */
	s_RegState_RX.dwRegData[REG_NUM_RX_R0] = INIT_REG_GENERAL;
	s_RegState_RX.dwRegData[REG_NUM_RX_ISP] = INIT_REG_CONTROL;
	s_RegState_RX.dwRegData[REG_NUM_RX_USP] = INIT_REG_CONTROL;
	s_RegState_RX.dwRegData[REG_NUM_RX_PSW] = INIT_REG_CONTROL;
	s_RegState_RX.dwRegData[REG_NUM_RX_BPSW] = INIT_REG_CONTROL;

	// RevRxNo121112-001 Modify Start
	// RevRxNo130411-001 Modify Line
	if (pMcuInfo->eFpuSet == RX_FPU_ON) {	// FPUありの場合
		s_RegState_RX.dwRegData[REG_NUM_RX_FPSW] = REG_RX_FPSW_INIT_VAL;
	} else {
		s_RegState_RX.dwRegData[REG_NUM_RX_FPSW] = INIT_REG_CONTROL;
	}
	// RevRxNo121112-001 Modify End

	// RevRxNo130301-001 Append Start
	// RevRxNo180228-001 Modify Start
	if (pMcuInfo->eCpuIsa == RX_ISA_RXV1) {	// RX V1アーキテクチャの場合
		s_RegState_RX.dwRegData[REG_NUM_RX_EXTB] = INIT_REG_CONTROL;
	} else {								// RX V2/RX V3アーキテクチャの場合
		s_RegState_RX.dwRegData[REG_NUM_RX_EXTB] = REG_RX_EXTB_INIT_VAL;
	}
	// RevRxNo180228-001 Modify End
	// RevRxNo121112-001 Modify End

	return;

}
//=============================================================================
/**
 * レジスタ操作コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfMcuData_Reg(void)
{
	// RevRxNo121112-001 Append Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	pMcuInfo = GetMcuInfoDataRX();	// MCU情報取得
	// RevRxNo121112-001 Append End

	/* MCU レジスタの管理変数を初期化 */

	s_RegState_RX.dwRegData[REG_NUM_RX_R0] = INIT_REG_GENERAL;
	s_RegState_RX.dwRegData[REG_NUM_RX_ISP] = INIT_REG_CONTROL;
	s_RegState_RX.dwRegData[REG_NUM_RX_USP] = INIT_REG_CONTROL;
	s_RegState_RX.dwRegData[REG_NUM_RX_PSW] = INIT_REG_CONTROL;
	s_RegState_RX.dwRegData[REG_NUM_RX_BPSW] = INIT_REG_CONTROL;

	// RevRxNo121112-001 Modify Start
	// RevRxNo130411-001 Modify Line
	if (pMcuInfo->eFpuSet == RX_FPU_ON) {	// FPUありの場合
		s_RegState_RX.dwRegData[REG_NUM_RX_FPSW] = REG_RX_FPSW_INIT_VAL;
	} else {
		s_RegState_RX.dwRegData[REG_NUM_RX_FPSW] = INIT_REG_CONTROL;
	}
	// RevRxNo121112-001 Modify End

	// RevRxNo130301-001 Append Start
	// RevRxNo180228-001 Modify Start
	if (pMcuInfo->eCpuIsa == RX_ISA_RXV1) {	// RX V1アーキテクチャの場合
		s_RegState_RX.dwRegData[REG_NUM_RX_EXTB] = INIT_REG_CONTROL;
	} else {								// RX V2/RX V3アーキテクチャの場合
		s_RegState_RX.dwRegData[REG_NUM_RX_EXTB] = REG_RX_EXTB_INIT_VAL;
	}
	// RevRxNo180228-001 Modify End
	// RevRxNo121112-001 Modify End

	return;

}
// 2008.9.9 INSERT_END_E20RX600 }

// RevRxNo180625-001 Append Start
//==============================================================================
/**
* レジスタ退避バンクの設定を行う。
* @param pRegBank 設定するレジスタ退避バンク情報を格納するFFWRX_REGBANK_DATA構造体のアドレス
* @retval FFWエラーコード
*/
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_SetREGBANK(const FFWRX_REGBANK_DATA* pRegBank)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	BYTE byRegBankStartNum;
	BYTE byRegBankEndNum;

	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuInfoData = GetMcuInfoDataRX();	// MCU情報取得

	// プログラム実行状態をチェック
	if (GetMcuRunState()) {			// プログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	// SPC変数で定義しているレジスタ一括退避機能のサポートチェック
	if ((pMcuInfoData->dwSpc[2] & SPC_REGBANK_EXIST) != SPC_REGBANK_EXIST) {
															// レジスタ一括退避機能がサポートされていない場合
		return FFWERR_FUNC_UNSUPORT;
	}

	// SPC変数で定義しているバンク番号の範囲チェック
	byRegBankStartNum = (BYTE)((pMcuInfoData->dwSpc[2] & SPC_REGBANK_START_NUM) >> 8);	// レジスタ退避バンク開始番号取得
	byRegBankEndNum = (BYTE)(pMcuInfoData->dwSpc[2] & SPC_REGBANK_END_NUM);				// レジスタ退避バンク終了番号取得
	if ((pRegBank->byRegBankNum < byRegBankStartNum) || (pRegBank->byRegBankNum > byRegBankEndNum)) {
															// バンク番号の範囲外の値が指定された場合
		return FFWERR_FFW_ARG;
	}

	ferr = DO_SetREGBANK(pRegBank);

	if (ferr == FFWERR_OK) {
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
}


//==============================================================================
/**
* レジスタ退避バンクの参照を行う。
* @param pRegBank 参照するレジスタ退避バンク情報を格納するFFWRX_REGBANK_DATA構造体のアドレス
* @retval FFWエラーコード
*/
//==============================================================================
FFWE100_API	DWORD FFWRXCmd_GetREGBANK(FFWRX_REGBANK_DATA* pRegBank)
{
	FFWERR	ferr = FFWERR_OK;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	BYTE byRegBankStartNum;
	BYTE byRegBankEndNum;

	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	pMcuInfoData = GetMcuInfoDataRX();	// MCU情報取得

	// プログラム実行状態をチェック
	if (GetMcuRunState()) {			// プログラム実行中の場合
		return FFWERR_BMCU_RUN;
	}

	// SPC変数で定義しているレジスタ一括退避機能のサポートチェック
	if ((pMcuInfoData->dwSpc[2] & SPC_REGBANK_EXIST) != SPC_REGBANK_EXIST) {
															// レジスタ一括退避機能がサポートされていない場合
		return FFWERR_FUNC_UNSUPORT;
	}

	// SPC変数で定義しているバンク番号の範囲チェック
	byRegBankStartNum = (BYTE)((pMcuInfoData->dwSpc[2] & SPC_REGBANK_START_NUM) >> 8);	// レジスタ退避バンク開始番号取得
	byRegBankEndNum = (BYTE)(pMcuInfoData->dwSpc[2] & SPC_REGBANK_END_NUM);				// レジスタ退避バンク終了番号取得
	if ((pRegBank->byRegBankNum < byRegBankStartNum) || (pRegBank->byRegBankNum > byRegBankEndNum)) {
															// バンク番号の範囲外の値が指定された場合
		return FFWERR_FFW_ARG;
	}

	ferr = DO_GetREGBANK(pRegBank);

	if (pMcuInfoData->eFpuSet == RX_FPU_OFF) {	// 単精度FPUなしの場合、FPSWのレジスタ参照値として0x0000_0000を格納する。
		// FPSWレジスタ
		pRegBank->dwRegBankData[REGBANK_REG_NUM_FPSW] = REG_NO_USE_VALUE;
	}

	if (pMcuInfoData->eCpuIsa == RX_ISA_RXV1) {	// RX V1アーキテクチャの場合、ACC0GからACC1Lのレジスタ参照値として0x0000_0000を格納する。
		// ACC0Gレジスタ
		pRegBank->dwRegBankData[REGBANK_REG_NUM_ACC0G] = REG_NO_USE_VALUE;
		// ACC0Hレジスタ
		pRegBank->dwRegBankData[REGBANK_REG_NUM_ACC0H] = REG_NO_USE_VALUE;
		// ACC0Lレジスタ
		pRegBank->dwRegBankData[REGBANK_REG_NUM_ACC0L] = REG_NO_USE_VALUE;
		// ACC1Gレジスタ
		pRegBank->dwRegBankData[REGBANK_REG_NUM_ACC1G] = REG_NO_USE_VALUE;
		// ACC1Hレジスタ
		pRegBank->dwRegBankData[REGBANK_REG_NUM_ACC1H] = REG_NO_USE_VALUE;
		// ACC1Lレジスタ
		pRegBank->dwRegBankData[REGBANK_REG_NUM_ACC1L] = REG_NO_USE_VALUE;
	}

	if (ferr == FFWERR_OK) {
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
}
// RevRxNo180625-001 Append End
