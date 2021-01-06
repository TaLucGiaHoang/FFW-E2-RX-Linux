////////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_reg.cpp
 * @brief レジスタ操作コマンドのソースファイル
 * @author RSO Y.Minami, H.Hashiguchi, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/09/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo180625-001,PB18047-CD01-001 2018/06/25 PA 辻
	RX66T-H/RX72T対応
*/
#include "domcu_reg.h"
#include "prot_common.h"
#include "protmcu_reg.h"
#include "mcu_rst.h"

//=============================================================================
/**
 * レジスタ値クリア　BFW/FFWの内部変数初期化
 * @param byClrCmd クリア処理呼び出しコマンド
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR ClrFwRegData(BYTE byClrCmd){

	FFWERR					ferr = FFWERR_OK;
	DWORD					dwRestVectAddr;
	FFWMCU_REG_DATA_RX*		pRegDataRX;
	BYTE					byRestVect;
	BOOL					bZeroClrReg;


	//レジスタ格納ポインタ取得
	pRegDataRX = GetRegData_RX();							// レジスタ情報取得		

	//リセットベクタはキャッシュorメモリからとる判定
	switch(byClrCmd){
	case REGCLRCMD_PMOD:
		byRestVect = RESTVECT_MEM_DATA;
		bZeroClrReg = FALSE;
		break;
	case REGCLRCMD_SREST:
	case REGCLRCMD_CLR:
		byRestVect = RESTVECT_MEM_DATA;
		bZeroClrReg = TRUE;
		break;
	case REGCLRCMD_REST:
		byRestVect = RESTVECT_CACHE_DATA;
		bZeroClrReg = FALSE;
		break;
	default:
		byRestVect = RESTVECT_MEM_DATA;
		bZeroClrReg = FALSE;
		break;			
	}

	//レジスタを0クリアする場合
	if(bZeroClrReg == TRUE){
		memset(pRegDataRX, 0, sizeof(FFWMCU_REG_DATA_RX));
	}

	//FFW内の内部変数初期化
	InitFfwRegData();

	//リセットアドレス取得
	ferr = GetMcuRestAddr(byRestVect,&dwRestVectAddr);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	//PC値更新
	pRegDataRX->dwRegData[REG_NUM_RX_PC] = dwRestVectAddr;

	//BFWへ設定
	ferr = PROT_MCU_SetRXXREG(pRegDataRX);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;

}

//=============================================================================
/**
 * レジスタ操作コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Reg(void)
{

}

// RevRxNo180625-001 Append Start
//=============================================================================
/**
* レジスタ退避バンクの設定を行う。
* @param pRegBank 設定するレジスタ退避バンク情報を格納するFFWRX_REGBANK_DATA構造体アドレス
* @retval FFWエラーコード
*/
//=============================================================================
FFWERR DO_SetREGBANK(const FFWRX_REGBANK_DATA* pRegBank)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_SetREGBANK(pRegBank);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
* レジスタ退避バンクの参照を行う。
* @param pRegBank 参照するレジスタ退避バンク情報を格納するFFWRX_REGBANK_DATA構造体アドレス
* @retval FFWエラーコード
*/
//=============================================================================
FFWERR DO_GetREGBANK(FFWRX_REGBANK_DATA* pRegBank)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;

	ProtInit();

	ferr = PROT_MCU_GetREGBANK(pRegBank);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}
// RevRxNo180625-001 Append End
