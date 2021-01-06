////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_rrm.cpp
 * @brief RAMモニタ関連コマンドのソースファイル
 * @author RSO Y.Minami, H.Hashiguchi, Y.Miyake
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/03/22
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130308-001 2013/05/20 三宅
　カバレッジ開発対応
*/
#include "ffwrx_rrm.h"
#include "dorx_rrm.h"
#include "protrx_rrm.h"
#include "prot_cpu.h"
#include "errchk.h"
#include "mcu_sfr.h"
#include "prot_common.h"
#include "dorx_tra.h"
#include "domcu_prog.h"
//RevNo100715-002 Append Line
#include "domcu_mcu.h"

// static 関数　プロトタイプ宣言
static BOOL	optFfwRrmdData( FFW_RRMD_DATA *pRrmd, FFW_RRMD_DATA *rrmd );
static BOOL	setOptRrmdData( DWORD dwPoint,FFW_RRMD_DATA *pRrmd,const FFW_RRMD_DATA rrmd );

//==============================================================================
/**
 * RAM モニタ領域の設定を行う。
 * @param dwSetBlk 設定するイベント種別に応じたイベント番号
 * @param pRrmb 設定するイベント情報を格納するFFW_EV_DATA 構造体のアドレス
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWERR DO_E20_SetRRMB(DWORD dwSetBlk, FFW_RRMB_DATA *pRrmbData)
{
	FFWERR			ferr;
	FFWERR			ferrEnd;
	DWORD			dwShift = 1;
	DWORD			dwEnableBlk = 0;
	//RevNo100715-002 Append Start
	DWORD			dwSetBlkEndian = 0;
	BYTE			byEndian;
	int				nCount;
	//RevNo100715-002 Append End
	// RevRxNo130308-001 Append Start
	FFWMCU_DBG_DATA_RX*	pDbgData;

	pDbgData = GetDbgDataRX();
	if (pDbgData->eTrcFuncMode == EML_TRCFUNC_CV) {	// "カバレッジ用に利用"の場合
		return FFWERR_OK;								// FFWERR_OKを返して関数から抜ける
	}
	// RevRxNo130308-001 Append End

	ProtInit();

	// 動作禁止ブロックのパラメータ設定
	//RevNo100715-002 Modify Line
	for( nCount = 0;nCount < RRM_E20_BLKNUM_MAX;nCount++ ){
		if( pRrmbData->eEnable[nCount] == RRMB_ENA ){
			// 動作許可bitを設定する。
			dwEnableBlk |= (dwShift << nCount);
		}
	}

	//RevNo100715-002 Append Start
	//動作ブロックのエンディアン情報取得
	dwSetBlkEndian = 0;
	for( nCount = 0;nCount < RRM_E20_BLKNUM_MAX;nCount++ ){
		// ここはエミュレータの設定にあわせるので、GetEndianType()を使用
		GetEndianType(pRrmbData->dwmadrBase[nCount], &byEndian);
		if( pRrmbData->eEnable[nCount] == RRMB_ENA ){
			dwSetBlkEndian  |= (DWORD)( byEndian << nCount);
		}
	}
	//RevNo100715-002 Append End

	// E20 RAMモニタベースアドレス設定コマンド送信
	//RevNo100715-002 Modify Line
	ferr = PROT_E20_SetRrmb( RRM_E20_BLKNUM_MAX,dwSetBlk,dwEnableBlk,dwSetBlkEndian ,pRrmbData);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return ferr;
	}

	//外部トレースで、ユーザプログラム停止中のときのみダミーデータを入れる
	if( (GetTrcInramMode() != TRUE) && (IsMcuRun() != TRUE )){
		ferr = SetTrace(TRC_BRK_ENABLE);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
	}

	ferr = ProtEnd();
	return ferr;
}

//==============================================================================
/**
 * RAM モニタのデータ、アクセス履歴を初期化する。
 * @param dwInitBlk RAMモニタ初期化ブロック
 * @param dwInitInfo[] 初期化情報
 * @param bInitFlg0 RAMモニタデータ、アクセス履歴のクリア実行フラグ
 * @param bInitFlg1 アクセス履歴のクリア実行フラグ
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
//RevNo100715-003 Modify Line
FFWERR DO_E20_SetRRMI(DWORD dwInitBlk, const DWORD dwInitInfo[], BOOL bInitFlg0, BOOL bInitFlg1, FFW_RRMB_DATA *pRrmbData)
{
	FFWERR			ferr;
	FFWERR			ferrEnd;
	DWORD			dwData;
	DWORD			dwShift;
	int				nCount;
	//RevNo100715-003 Append Start
	DWORD			dwEndian = 0;
	BYTE			byEndian;
	//RevNo100715-003 Append End

	//ワーニング対策
	dwInitBlk;

	ProtInit();

	//RevNo100715-003 Append Start
	//エンディアンフラグ作成
	for( nCount = 0;nCount < RRM_E20_BLKNUM_MAX;nCount++ ){
		GetEndianType(pRrmbData->dwmadrBase[nCount], &byEndian);
		if( pRrmbData->eEnable[nCount] == RRMB_ENA ){
			dwEndian  |= (DWORD)( byEndian << nCount);
		}
	}
	//RevNo100715-003 Append End

	// RAMモニタデータ、アクセス履歴のクリア指定の場合の初期化
	if( bInitFlg0 ){
		dwData = 0;
		dwShift = 1;

		// 初期化ブロック情報の設定
		for( nCount = 0;nCount < RRM_E20_BLKNUM_MAX;nCount++ ){
			if( dwInitInfo[nCount] & INIT_BIT0 ){
				dwData |= (dwShift << nCount);
			}
		}

		// RAMモニタ初期化コマンド送信
		//RevNo100715-003 Modify Line
		ferr = PROT_E20_SetRrmi( RRM_E20_BLKNUM_MAX,dwData,0x00 ,dwEndian);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return	ferr;
		}
	}

	// アクセス履歴のクリア指定の場合の初期化
	if( bInitFlg1 ){
		dwData = 0;
		dwShift = 1;

		// 初期化ブロック情報の設定
		for( nCount = 0;nCount < RRM_E20_BLKNUM_MAX;nCount++ ){
			if( dwInitInfo[nCount] & INIT_BIT1 ){
				dwData |= (dwShift << nCount);
			}
		}

		// RAMモニタ初期化コマンド送信
		//RevNo100715-003 Modify Line
		ferr = PROT_E20_SetRrmi( RRM_E20_BLKNUM_MAX,dwData,0x01 ,dwEndian);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return	ferr;
		}
	}

	ferr = ProtEnd();
	return ferr;
}


//==============================================================================
/**
 * RAM モニタデータを取得する。
 * @param dwGetBlk RAM モニタデータを取得するブロック0～31 をビットフィールドで指定する。
 * @param rrmd RAM モニタ領域を格納するFFW_RRMD_DATA 構造体の配列を指定する。
 * @param ramMonData RAM モニタデータを格納するFFW_RRMD_GET_DATA 構造体の配列を指定する。
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWERR DO_E20_GetRRMD(DWORD dwGetBlk, const FFW_RRMD_DATA rrmd[], FFW_RRMD_GET_DATA ramMonData[])
{
	FFWERR			ferr = FFWERR_OK;
	FFWERR			ferrEnd;
	int				nBlkCount;			
	int				nPoint;
	WORD			wCount1;
	WORD			wDataCount;
	WORD			wAddress;
	WORD			wSize;
	BYTE			byData[2048];
	FFW_RRMD_DATA	CmdRrmd[RRM_E20_BLKNUM_MAX];
	FFW_RRMD_DATA	rrmdBuf[RRM_E20_BLKNUM_MAX];
	FFW_RRMD_GET_DATA ramMonData_tmp[RRM_E20_BLKNUM_MAX];

	for (nBlkCount = 0; nBlkCount < RRM_E20_BLKNUM_MAX; nBlkCount++) {
		memcpy(&rrmdBuf[nBlkCount], &rrmd[nBlkCount], sizeof(rrmd[0]));
	}

	ProtInit();

	// RAMモニタ実行アドレスを最適化する。
	for( nBlkCount = 0;nBlkCount < RRM_E20_BLKNUM_MAX;nBlkCount++ ){
		// RAMモニタデータを取得するブロックを判定する。
		if( (dwGetBlk >> nBlkCount) & 0x00000001 ){
			// RAMモニタ実行アドレスを最適化する。
			// 例）下記のようなアドレスでGetRRMDコマンドの実行要求があった場合
			//    0h～Fh
			//    10h～15h
			//    3h～1Fh
			//   指定アドレス通り、GetRRMDコマンドを3回実行しなくても0h～1Fhで
			//   1回だけ実行すればよい。
			//
			// OptFfwRrmdData関数はrrmd構造体のGetRRMD実行アドレスを最適化し
			// CmdRrmd構造体に最適化アドレスを設定する関数である。
			optFfwRrmdData( &CmdRrmd[nBlkCount], &rrmdBuf[nBlkCount] );
		}
	}

	// RAMモニタデータ取得処理を実行する。
	for( nBlkCount = 0;nBlkCount < RRM_E20_BLKNUM_MAX;nBlkCount++ ){
		// RAMモニタデータを取得するブロックを判定する。
		if( (dwGetBlk >> nBlkCount) & 0x00000001 ){
			// RAMモニタの指定アドレス分、RAMモニタデータを取得する。
			for( wCount1 = 0;wCount1 < CmdRrmd[nBlkCount].dwAreaNum;wCount1++ ){
			//for( wCount1 = 0;wCount1 < rrmd[nBlkCount].dwAreaNum;wCount1++ ){
				// RAMモニタ開始アドレス、サイズを算出
				wAddress = (WORD)CmdRrmd[nBlkCount].dwStart[wCount1];
				wSize = (WORD)(CmdRrmd[nBlkCount].dwEnd[wCount1] - CmdRrmd[nBlkCount].dwStart[wCount1] + 1);
				//wAddress = (WORD)rrmd[nBlkCount].dwStart[wCount1];
				//wSize = (WORD)(rrmd[nBlkCount].dwEnd[wCount1] - rrmd[nBlkCount].dwStart[wCount1] + 1);

				//RevNo100715-004 Append Start
				//補正したRAMモニタブロックの値が0x400Byte以内であることを確認
				if(wAddress + wSize > (RRM_E20_OFFSET_ADDR+1)){
					return FFWERR_FFW_ARG;
				}
				//RevNo100715-004 Append End

				// RAMモニタデータ取得コマンド送信
				ferr = PROT_E20_GetRrmd( (BYTE)nBlkCount,wAddress,wSize,byData );		
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}

				// 取得データをデータとアクセス属性に分ける。
				nPoint = 0;
				for( wDataCount = 0;wDataCount < wSize;wDataCount++ ){
					ramMonData_tmp[nBlkCount].byAccess[wAddress + wDataCount] = byData[nPoint++];
					ramMonData_tmp[nBlkCount].byData[wAddress + wDataCount] = byData[nPoint++];
				}

			}
		}
	}

	// RAMモニタデータをramMonDataにうつす。
	if( ferr == FFWERR_OK ){
		for( nBlkCount = 0;nBlkCount < RRM_E20_BLKNUM_MAX;nBlkCount++ ){
			nPoint = 0;
			// RAMモニタデータを取得するブロックを判定する。
			if( (dwGetBlk >> nBlkCount) & 0x00000001 ){
				// RAMモニタの指定アドレス分、RAMモニタデータのコピーを行う。
				for( wCount1 = 0;wCount1 < rrmd[nBlkCount].dwAreaNum;wCount1++ ){
					wAddress = (WORD)rrmd[nBlkCount].dwStart[wCount1];
					wSize = (WORD)(rrmd[nBlkCount].dwEnd[wCount1] - rrmd[nBlkCount].dwStart[wCount1] + 1);

					for( wDataCount = 0;wDataCount < wSize;wDataCount++ ){
						ramMonData[nBlkCount].byAccess[nPoint] = ramMonData_tmp[nBlkCount].byAccess[wAddress + wDataCount];
						ramMonData[nBlkCount].byData[nPoint] = ramMonData_tmp[nBlkCount].byData[wAddress + wDataCount];
						nPoint++;
					}
				}		
			}
		}
	}

	ferr = ProtEnd();

	return ferr;
}
//==============================================================================
/**
 * DO_E20_ClrRRML RAMモニタでのロスト発生クリアを行う。
 * @param なし
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_COM 通信異常発生
 */
//==============================================================================
FFWERR DO_E20_ClrRRML(void)
{
	FFWERR			ferr;
	FFWERR						ferrEnd;

	ProtInit();
	// RAMモニタ ロスト発生クリアコマンドの送受信
	ferr = PROT_E20_ClrRrml();
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		return	ferr;
	}

	ferr = ProtEnd();

	return	ferr;
}
///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////

//==============================================================================
/**
 * RRMDアドレス実行最適化。
 * @param  なし
 * @retval TRUE 最適化を行った　FALSE最適化しなかった
 */
//==============================================================================
static BOOL	optFfwRrmdData( FFW_RRMD_DATA *pRrmd, FFW_RRMD_DATA *rrmd )
{
	FFW_RRMD_DATA	tmpRrmd;
	DWORD			dwCount;
	DWORD			dwPoint;
	BOOL			bOptFlg = FALSE;

	tmpRrmd.dwAreaNum = rrmd->dwAreaNum;
	for( dwCount = 0;dwCount < tmpRrmd.dwAreaNum;dwCount++ ){
		tmpRrmd.dwStart[dwCount] = rrmd->dwStart[dwCount];
		tmpRrmd.dwEnd[dwCount] = rrmd->dwEnd[dwCount];
	}

	for( dwPoint = 0;dwPoint < tmpRrmd.dwAreaNum; ){
		bOptFlg = setOptRrmdData( dwPoint,pRrmd,tmpRrmd );
		if( bOptFlg == FALSE ){
			dwPoint++;
		}
		
		// 最適化後の構造体情報をコピー
		tmpRrmd.dwAreaNum = pRrmd->dwAreaNum;
		for( dwCount = 0;dwCount < tmpRrmd.dwAreaNum;dwCount++ ){
			tmpRrmd.dwStart[dwCount] = pRrmd->dwStart[dwCount];
			tmpRrmd.dwEnd[dwCount] = pRrmd->dwEnd[dwCount];
		}
	}

	return bOptFlg;
}

//==============================================================================
/**
 * RRMDアドレス実行最適化 実行部。
 * @param  なし
 * @retval TRUE
 */
//==============================================================================
static BOOL setOptRrmdData( DWORD dwPoint,FFW_RRMD_DATA *pRrmd,const FFW_RRMD_DATA rrmd )
{
	DWORD	dwCount;
	DWORD	dwOptStartAddr;
	DWORD	dwOptEndAddr;
	DWORD	dwOutStartAddr[1024];
	DWORD	dwOutEndAddr[1024];
	int		nCount;
	BOOL	bOptFlg = FALSE;

	// RAMモニタアドレスの実行アドレス数が1の場合
	if( rrmd.dwAreaNum == 1 ){
		pRrmd->dwStart[0] = rrmd.dwStart[0];
		pRrmd->dwEnd[0] = rrmd.dwEnd[0];
		pRrmd->dwAreaNum = rrmd.dwAreaNum;
		return FALSE;
	}

	nCount			= 0;
	dwOptStartAddr	= rrmd.dwStart[dwPoint];
	dwOptEndAddr	= rrmd.dwEnd[dwPoint];

	for( dwCount = 0;dwCount < rrmd.dwAreaNum;dwCount++ ){
		pRrmd->dwStart[dwCount]	= rrmd.dwStart[dwCount];
		pRrmd->dwEnd[dwCount]	= rrmd.dwEnd[dwCount];
	}

	// RAMモニタアドレスの実行数が複数ある場合
	for( dwCount = 0;dwCount < rrmd.dwAreaNum;dwCount++ ){
		if( dwPoint >= dwCount ){
			continue;
		}

		if( dwOptStartAddr <= rrmd.dwStart[dwCount] &&
			dwOptEndAddr >= rrmd.dwStart[dwCount] ){
			// 開始アドレスが領域内に含まれている
			if( dwOptStartAddr <= rrmd.dwEnd[dwCount] &&
				dwOptEndAddr >= rrmd.dwEnd[dwCount] ){
				// 終了アドレスが領域内に含まれている。
				//    |------------|  A 領域
				//       |------|     B 領域
				// 何もしない
			}
			else{
				// 終了アドレスは領域内に含まれていない。
				//    |------------|         A 領域
				//         |-------------|   B 領域
				bOptFlg = TRUE;
				dwOptEndAddr = rrmd.dwEnd[dwCount];
			}
		}
		else if( dwOptStartAddr <= rrmd.dwEnd[dwCount] &&
				 dwOptEndAddr >= rrmd.dwEnd[dwCount] ){
			// 終了アドレスが領域内に含まれている
			//       |------------|  A 領域
			//   |---------|         B 領域
			bOptFlg = TRUE;
			dwOptStartAddr = rrmd.dwStart[dwCount];
		}
		else if( (rrmd.dwStart[dwCount] <= dwOptStartAddr && rrmd.dwEnd[dwCount] >= dwOptStartAddr) &&
				 (rrmd.dwStart[dwCount] <= dwOptEndAddr && rrmd.dwEnd[dwCount] >= dwOptEndAddr) ){				 
			// B領域がA領域を含む場合
			//       |------------|      A 領域
			//   |--------------------|  B 領域
			bOptFlg = TRUE;
			dwOptStartAddr = rrmd.dwStart[dwCount];
			dwOptEndAddr = rrmd.dwEnd[dwCount];
		}
		else{
			if( (rrmd.dwStart[dwCount] != 0x00) &&
				(dwOptEndAddr == rrmd.dwStart[dwCount] - 1) ){
				// B領域の先頭が00h以外かつ
				// A領域の後ろにB領域が並んでいる場合
				//   |-----|            A領域
				//          |--------|  B領域
				bOptFlg = TRUE;
				dwOptEndAddr = rrmd.dwEnd[dwCount];
			}
			else if( (rrmd.dwEnd[dwCount] != 0xFFFFFFFF) &&
					 (dwOptStartAddr == rrmd.dwEnd[dwCount] + 1) ){
				// B領域の最終がFFFF_FFFFh以外かつ
				// A領域の前にB領域が並んでいる場合
				//          |--------|  A領域
				//   |-----|            B領域
				bOptFlg = TRUE;
				dwOptStartAddr = rrmd.dwStart[dwCount];
			}
			else{
				// A領域、B領域共に交わらない場合
				//         |----------|        A 領域
				//                      |---|  B 領域
				//   |---|                     B 領域
				dwOutStartAddr[nCount]	= rrmd.dwStart[dwCount];
				dwOutEndAddr[nCount]	= rrmd.dwEnd[dwCount];
				nCount++;
			}
		}
	}

	// 最適化アドレスを構造体にコピー
	pRrmd->dwStart[dwPoint]	= dwOptStartAddr;
	pRrmd->dwEnd[dwPoint]	= dwOptEndAddr;

	// 最適化アドレスに含まれないアドレスを構造体にコピー
	for( int i = 0;i < nCount;i++ ){
		dwPoint++;
		pRrmd->dwStart[dwPoint] = dwOutStartAddr[i];
		pRrmd->dwEnd[dwPoint] = dwOutEndAddr[i];
	}

	pRrmd->dwAreaNum = dwPoint + 1;

	return 	bOptFlg;		
}


//==============================================================================
/**
 * RAMモニタ関連レジスタを初期化する。
 * @param  なし
 * @retval FFWエラー
 */
//==============================================================================
FFWERR ClrRramReg(void)
{
	FFWERR	ferr;
	int		i;
	DWORD				dwInitBlk;
	FFW_RRMB_DATA		RrmbData;
	FFW_RRMB_DATA*		pRrmbData;

	// FPGA上のRAMモニタベースアドレスクリア
	dwInitBlk = 0x000F;
	for (i = 0; i < RRM_E20_BLKNUM_MAX; ++i) {
		RrmbData.eEnable[i] = RRMB_DIS;
		RrmbData.dwmadrBase[i] = 0x00000000;
	}
	pRrmbData = &RrmbData;
	ferr = DO_E20_SetRRMB( dwInitBlk, pRrmbData );
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	
	return FFWERR_OK;
}


//=============================================================================
/**
 * RAMモニタ関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdRxData_Rrm(void)
{
	return;
}
