////////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwrx_rrm.cpp
 * @brief RAMモニタ関連コマンドのソースファイル
 * @author RSD Y.Minami, H.Hashiguchi, Y.Miyake, Y.Kawakami, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/07
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121022-001	2012/10/22 SDS 岩田
　　FFWE20Cmd_SetRRMB(), FFWE20Cmd_GetRRMB(), FFWE20Cmd_SetRRMI(),
　　FFWE20Cmd_GetRRMD(), FFWE20Cmd_ClrRRML()関数にEZ-CUBE用処理を追加
・RevRxNo121022-001	2012/11/1 SDS 岩田
　　FFWE20Cmd_SetRRMB(), FFWE20Cmd_GetRRMB(), FFWE20Cmd_SetRRMI(),
　　FFWE20Cmd_GetRRMD(), FFWE20Cmd_ClrRRML()関数のエミュレータ種別 EZ-CUBE判定処理の定義名を変更
・RevRxNo130308-001 2013/05/20 三宅
	カバレッジ開発対応
・RevRxNo140515-001 2014/06/27 川上
	RRML, CLRコマンド内のユーザI/F接続形態判定処理改善
・RevRxE2LNo141104-001 2014/11/07 上田
	E2 Lite対応
*/
#include "ffwrx_rrm.h"
#include "errchk.h"
#include "dorx_rrm.h"
#include "do_sys.h"
// RevRxNo130308-001 Append Line
#include "domcu_mcu.h"

// FFW 内部変数
static FFW_RRMB_DATA s_Rrmb;					// RAM モニタベースアドレスの管理変数

//==============================================================================
/**
 * RAM モニタ領域の設定を行う。
 * @param dwSetBlk 設定するRAMモニタブロック(ビットフィールドで指定)
 * @param pRrmb 設定するRAMモニタ領域情報を格納するFFW_RRMB_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWE20Cmd_SetRRMB(DWORD dwSetBlk, const FFW_RRMB_DATA* pRrmb)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR				ferr = FFWERR_OK;
	int					i, j;
	FFW_RRMB_DATA		RrmbData;
	FFW_RRMB_DATA*		pRrmbData;
	DWORD				dwShift;
	FFW_RRMB_DATA*		pSetRrmb;		// 設定中のRAM モニタ設定情報
	FFWMCU_MCUDEF_DATA* pMcuDef;
	FFWE20_EINF_DATA	einfData;
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;
	
	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RRAM_UNSUPPORT;						// 「RAMモニタ機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	getEinfData(&einfData);		// エミュレータ情報取得

	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続以外の場合
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
		return FFWERR_RRAM_UNSUPPORT;
	}
	// RevRxE2LNo141104-001 Modify End

	
	pMcuDef = GetMcuDefData();

	// RAMモニタ領域設定対象ブロック情報のチェック
	if (!dwSetBlk) {
		// RAM モニタ領域の設定対象のブロックがない場合（dwSetBlk == 0）
		// 何もすることが無いので正常終了とする。
		return FFWERR_OK;
	}

	//RevNo100715-004 Append Start
	// RAMモニタブロック設定が4以上の場合はエラーを返す。
	if(dwSetBlk > MAX_BLOCK_NUM){
		return FFWERR_FFW_ARG;
	}
	//RevNo100715-004 Append End

	// 引数pRrmbの内容をRrmbDataにコピーする。
	memcpy(&RrmbData, pRrmb, sizeof(FFW_RRMB_DATA));
	dwShift = 1;
	
	for (i = 0; i < RRM_E20_BLKNUM_MAX; ++i) {
		if (dwSetBlk & (dwShift << i)) {
			// 指定ブロックの設定内容を変更する場合
			if (RrmbData.eEnable[i] == RRMB_ENA) {
				// 指定ブロックのRAM モニタ動作を許可にする場合
				if (RrmbData.dwmadrBase[i] > pMcuDef->madrMaxAddr) {
					// RAMモニタベースアドレスがMCUの最大アドレスを超えている場合
					return FFWERR_FFW_ARG;
				}
				// 指定ブロックアドレス0～9bitのアドレスはクリア
				RrmbData.dwmadrBase[i] = RrmbData.dwmadrBase[i] & 0xFFFFFC00;
			}
		}
	}

	/* 引数チェック（RAM モニタ設定領域のベースアドレスの重複チェック） */
	/* ※１：変更対象ブロック同士の重複チェック */
	dwShift = 1;
	for (i = 0; i < RRM_E20_BLKNUM_MAX; ++i) {
		for (j = i + 1; j < RRM_E20_BLKNUM_MAX; ++j) {
			if ( ((dwSetBlk & (dwShift << i)) && (dwSetBlk & (dwShift << j))) && 
				 ((RrmbData.eEnable[i] == RRMB_ENA) && (RrmbData.eEnable[j] == RRMB_ENA)) ) {
				// RAM モニタ領域の設定対象になっている場合
				// 設定対象の領域が動作許可に設定する場合
				if (RrmbData.dwmadrBase[i] == RrmbData.dwmadrBase[j]) {
					// 引数チェック（ベースアドレスが重複しているか）
					return FFWERR_RRMAREA_SAME;
				}
			}
		}
	}

	/* 引数チェック（RAM モニタ設定領域のベースアドレスの重複チェック） */
	/* ※２：設定変更対象でない、かつ現在動作許可状態ブロックとの重複チェック */
	pSetRrmb = GetRrmbData();
	dwShift = 1;
	for (i = 0; i < RRM_E20_BLKNUM_MAX; ++i) {
		for (j = 0; j < RRM_E20_BLKNUM_MAX; ++j) {
			if (((dwSetBlk & (dwShift << i)) && (!(dwSetBlk & (dwShift << j)))) && 
				((RrmbData.eEnable[i] == RRMB_ENA) && (pSetRrmb->eEnable[j] == RRMB_ENA))) {
				// 以前設定したブロックに対して重複チェックを行う必要がある場合
				if (RrmbData.dwmadrBase[i] == pSetRrmb->dwmadrBase[j]) {
					// ベースアドレスが重複している場合
					return FFWERR_RRMAREA_SAME;
				}
			}
		}
	}

	pRrmbData = &RrmbData;
	ferr = DO_E20_SetRRMB( dwSetBlk,pRrmbData );
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// FFW 内部変数に設定内容を保存
	dwShift = 1;
	for (i = 0; i < RRM_E20_BLKNUM_MAX; ++i) {
		if (dwSetBlk & (dwShift << i)) {
			s_Rrmb.eEnable[i] = RrmbData.eEnable[i];
			s_Rrmb.dwmadrBase[i] = RrmbData.dwmadrBase[i];
		}
	}

	// V.1.02 RevNo110613-001 Modify Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}

	return ferr;
	// V.1.02 RevNo110613-001 Modify End

}


//==============================================================================
/**
 * RAM モニタ領域のベースアドレスを参照する。
 * @param pRrmb RAM モニタ領域を格納するFFW_RRMB_DATA 構造体のアドレス
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWE20Cmd_GetRRMB(FFW_RRMB_DATA* pRrmb)
{
	FFWE20_EINF_DATA	einfData;
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;
	
	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RRAM_UNSUPPORT;						// 「RAMモニタ機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	getEinfData(&einfData);		// エミュレータ情報取得
	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続以外の場合
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
		return FFWERR_RRAM_UNSUPPORT;
	}
	// RevRxE2LNo141104-001 Modify End

	//RevNo100715-004 Modify Line
	//BFWコマンドではなく、static変数の値で設定状態を参照
	memcpy(pRrmb,&s_Rrmb,sizeof(FFW_RRMB_DATA));
	return FFWERR_OK;
}


//==============================================================================
/**
 * RAM モニタのデータ、アクセス履歴、初期化抜け検出結果を初期化する。
 * @param dwInitBlk RAM モニタデータ、アクセス履歴、初期化抜け検出結果を
 * 初期化ブロック0～31をビットフィールドで指定する。
 * @param dwInitInfo 初期化情報が格納されている配列
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWE20Cmd_SetRRMI(DWORD dwInitBlk, const DWORD dwInitInfo[])
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR			ferr = FFWERR_OK;
	FFW_RRMB_DATA*	pRrmb;			// RAM モニタ領域を格納するFFW_RRMB_DATA 構造体へのポインタ
	DWORD			dwShift;
	BOOL			bInitFlg0 = FALSE;
	BOOL			bInitFlg1 = FALSE;
	int				nCount;
	FFWE20_EINF_DATA	einfData;
	//RevNo100715-003 Modify Line
	FFW_RRMB_DATA*		pSetRrmb;		// 設定中のRAM モニタ設定情報
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;

	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RRAM_UNSUPPORT;						// 「RAMモニタ機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	getEinfData(&einfData);		// エミュレータ情報取得
	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続以外の場合
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
		return FFWERR_RRAM_UNSUPPORT;
	}
	// RevRxE2LNo141104-001 Modify End

	if( !dwInitBlk ){
		// RAM モニタの初期化対象のブロックがない為、
		// 正常終了とする
		return FFWERR_OK;
	}
	// RAMモニタ初期化ブロック設定が4以上の場合はエラーを返す。
	if(dwInitBlk > MAX_BLOCK_NUM){
		return FFWERR_FFW_ARG;
	}
	// 引数、動作ブロックチェック
	pRrmb = GetRrmbData();		// RAMモニタブロック情報の取得
	dwShift = 1;
	for (nCount = 0; nCount < RRM_E20_BLKNUM_MAX; ++nCount) {
		if (dwInitBlk & (dwShift << nCount)) {
			if (dwInitInfo[nCount] & CHECK_INIT_INFO) {
				// 引数チェック（初期化情報）
				return FFWERR_FFW_ARG;
			}

			if (pRrmb->eEnable[nCount] == RRMB_DIS) {
				// 指定ブロックが動作禁止である
				return FFWERR_RRMBLK_DIS;
			}
		}
	}

	// 初期化方法の設定フラグ設定
	dwShift = 1;
	for( nCount = 0;nCount < RRM_E20_BLKNUM_MAX;nCount++ ){
		// 初期化有効bitの確認
		if( (dwInitBlk >> nCount) & 0x00000001 ){
			if (dwInitInfo[nCount] & INIT_BIT0) {
				bInitFlg0 = TRUE;
			}
			if (dwInitInfo[nCount] & INIT_BIT1) {
				bInitFlg1 = TRUE;
			}
		}
	}

	//RevNo100715-003 Modify Start
	//現在設定中のRRMデータを取得
	pSetRrmb = GetRrmbData();
	// RAMモニタデータ初期化コマンド送信
	ferr = DO_E20_SetRRMI( dwInitBlk,dwInitInfo,bInitFlg0,bInitFlg1 ,pSetRrmb);
	//RevNo100715-003 Modify End

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return	ferr;

}


//==============================================================================
/**
 * RAM モニタデータを取得する。
 * @param dwGetBlk RAM モニタデータを取得するブロック0～31 をビットフィールドで指定する。
 * @param rrmd RAM モニタ領域を格納するFFW_RRMD_DATA 構造体の配列を指定する。
 * @param ramMonData RAM モニタデータを格納するFFW_RRMD_GET_DATA 構造体の配列を指定する。
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API DWORD FFWE20Cmd_GetRRMD(DWORD dwGetBlk, const FFW_RRMD_DATA rrmd[], FFW_RRMD_GET_DATA ramMonData[])
{

	// V.1.02 RevNo110613-001 Modify Line
	FFWERR			ferr = FFWERR_OK;
	int				i;
	DWORD			j;
	FFW_RRMB_DATA*	pRrmb;		// RAM モニタ領域を格納するFFW_RRMB_DATA 構造体へのポインタ
	DWORD			dwMask;
	DWORD			dwSize;		// 指定した領域サイズの合計

	FFWE20_EINF_DATA	einfData;
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;
	
	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RRAM_UNSUPPORT;						// 「RAMモニタ機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	getEinfData(&einfData);		// エミュレータ情報取得
	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続以外の場合
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
		return FFWERR_RRAM_UNSUPPORT;
	}
	// RevRxE2LNo141104-001 Modify End
 
	if (!dwGetBlk) {
							// RAM モニタデータの取得対象のブロックがない為、
							// 正常終了とする。
		return FFWERR_OK;
	}

	//RevNo100715-004 Append Start
	// RAMモニタブロック設定が4以上の場合はエラーを返す。
	if(dwGetBlk > MAX_BLOCK_NUM){
		return FFWERR_FFW_ARG;
	}
	//RevNo100715-004 Append End

	dwMask = 1;
	pRrmb = GetRrmbData();
	for (i = 0; i < RRM_E20_BLKNUM_MAX; ++i) {
		if (dwGetBlk & (dwMask << i)) {
			if (rrmd[i].dwAreaNum > MAX_AREA_NUM) {
							// 引数チェック（RAM モニタデータを参照する領域の数）
				return FFWERR_FFW_ARG;
			}

			dwSize = 0;
			for (j = 0; j < rrmd[i].dwAreaNum; ++j) {
				if (rrmd[i].dwStart[j] > RRM_E20_OFFSET_ADDR) {
								// 引数チェック
								// RAM モニタデータを参照する領域の開始アドレスへのオフセット）
					return FFWERR_FFW_ARG;
				}

				if (rrmd[i].dwEnd[j] > RRM_E20_OFFSET_ADDR) {
								// 引数チェック
								// RAM モニタデータを参照する領域の終了アドレスへのオフセット）
					return FFWERR_FFW_ARG;
				}

				if (rrmd[i].dwStart[j] > rrmd[i].dwEnd[j]) {
								// 引数チェック（開始アドレスの終了アドレスの比較）
					return FFWERR_FFW_ARG;
				}

				dwSize += (rrmd[i].dwEnd[j] - rrmd[i].dwStart[j]) + 1;
			}

			if (dwSize > E20_MAX_AREA_SIZE) {
								// 引数チェック（RAM データ領域1 ブロックのサイズ）
				return FFWERR_FFW_ARG;
			}

			if (pRrmb->eEnable[i] == RRMB_DIS) {
								// 指定ブロックが動作禁止である
				return FFWERR_RRMBLK_DIS;
			}
		}
	}

	ferr = DO_E20_GetRRMD(dwGetBlk, rrmd, ramMonData);

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End

	return ferr;

}

//==============================================================================
/**
 * RAM モニタでのロスト発生をクリアする。
 * @param  なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWE100_API	DWORD FFWE20Cmd_ClrRRML(void)
{
	// V.1.02 RevNo110613-001 Modify Line
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;
	// RevRxNo130308-001 Append Line
	FFWMCU_DBG_DATA_RX*	pDbgData;
	
	// V.1.02 RevNo110613-001 Apped Line
	// ワーニング発生後にエラーが出た場合、ワーニングがクリアできないのでIFの先頭でクリア
	ClrBFWWarning();

	// RevRxNo130308-001 Append Start
 	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_RRAM_UNSUPPORT;						// 「RAMモニタ機能はサポートしていない。」エラーを返す。
	}
	// RevRxNo130308-001 Append End

	getEinfData(&einfData);		// エミュレータ情報取得
	// RevRxE2LNo141104-001 Modify Start
	// E20+38ピンケーブル接続以外の場合
	if (!((einfData.wEmuStatus == EML_E20) && (einfData.eStatUIF == CONNECT_IF_38PIN))) {
		return FFWERR_RRAM_UNSUPPORT;
	}
	// RevRxE2LNo141104-001 Modify End

	// RAMモニタ ロスト発生のクリア処理を実行
	ferr = DO_E20_ClrRRML();

	// V.1.02 RevNo110613-001 Apped Start
	if(ferr == FFWERR_OK){
		ferr = WarningChk_BFW();				// BFWコマンドのWarning確認
	}
	// V.1.02 RevNo110613-001 Apped End
	
	return ferr;
}

//==============================================================================
/**
 * FFW RAM モニタベースアドレス管理変数に設定されている値を取得する。
 * @param なし
 * @retval FFW RAM モニタベースアドレス管理変数のポインタ
 */
//==============================================================================
FFW_RRMB_DATA* GetRrmbData(void)
{

	FFW_RRMB_DATA*	pRet;

	pRet = &s_Rrmb;

	return pRet;

}

//=============================================================================
/**
 * RAMモニタ関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfRxData_Rrm(void)
{

	int		i;

	/* RAM モニタ領域の設定を初期化 */
	for (i = 0; i < RRM_E20_BLKNUM_MAX; ++i) {
		s_Rrmb.eEnable[i] = INIT_RRMB_DATA;
	}

	return;

}


