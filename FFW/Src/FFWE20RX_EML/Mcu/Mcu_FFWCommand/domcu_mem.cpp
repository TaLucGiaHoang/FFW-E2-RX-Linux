///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_mem.cpp
 * @brief メモリ操作関連コマンドの実装ファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, H.Akashi, S.Ueda, SDS T.Iwata
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2013/11/21
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo111121-003   2012/07/10 橋口
  ・OffsetStartAddr() 
    ・domcu_mem.cpp 以外でも使用する為、プロトタイプ宣言をdomcu_mem.h へ移動。
・RevNo121017-002	2012/10/18 明石
　VS2008対応エラーコード　ERROR C2148暫定対策
・RevNo121017-002	2012/10/31 上田
　VS2008対応エラーコード　ERROR C2148恒久対策
　・DO_WRITE()
 　　レングスが0の場合のWRITEデータ格納メモリ確保処理削除。
・RevRxNo121022-001	2012/11/21 SDS 岩田
  ・EZ-CUBE PROT_MCU_DUMP()分割処理対応
・RevRxNo130730-009 2013/11/15 大喜多
　　mallocでメモリが確保できなかった場合にFFWERR_MEMORY_NOT_SECUREDを返すよう変更
・RevRxNo130730-008 2013/11/21 大喜多
	エラーで返る前にメモリ解放処理を追加
*/
#include "domcu_mem.h"
#include "ffwmcu_mem.h"
#include "ffwmcu_dwn.h"
#include "domcu_mcu.h"
#include "domcu_dwn.h"
#include "prot_common.h"
#include "mcu_mem.h"
#include "do_sys.h"
#include "protmcu_mem.h"

#include <malloc.h>

// static 関数宣言
static FFWERR changeOrderDumpBuff(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff);
static FFWERR changeOrderWriteBuff(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, BYTE* pbyBuff);
// RevNo111121-003 OffsetStartAddr() のプロトタイプ宣言をdomcu_mem.h に移動

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ターゲットメモリ内容を取得する。
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eAccessSize アクセスサイズ
 * @param pbyReadBuff リードデータ格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_DUMP(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE *const pbyReadBuff)
{
	FFWERR	ferr;
	FFWERR	ferrEnd;
	BYTE*	pbyBuff;

	DWORD	dwTotalLength;				// DUMP全体のバイトサイズ
	BYTE*	pbyReadBufEsc;
	BOOL	bDumpEndFlag = FALSE;		// Dump 終了フラグ(FALSE:終了でない、TRUE:終了)
	MADDR	madrEndAddrRemainder;		// 最終アドレスを 4 で割った余り
	MADDR	madrTempStartAddr;			// そのときの先頭アドレス
	MADDR	madrTempEndAddr;			// そのときの最終アドレス

	ProtInit();

	madrEndAddrRemainder = madrEndAddr % 4;		// 最終余り。

	madrTempStartAddr = madrStartAddr;			// madrStartAddr を初期値。
	madrTempEndAddr   = madrEndAddr;			// madrEndAddr   を初期値。

	// 確保ポインタと操作ポインタは別
	// Target側から渡された領域を破壊しないよう3Byte余分にメモリ確保しておく
	dwTotalLength = madrEndAddr - madrStartAddr + 1;	// リードバイト数算出
	pbyReadBufEsc = (BYTE*)malloc(dwTotalLength + 0x3);
	if (pbyReadBufEsc == NULL) {
		ferrEnd = ProtEnd();
		// RevRxNo130730-009 Modify Line( エラーコードをFFWERR_FFW_ARG→FFWERR_MEMORY_NOT_SECUREDに変更 )
		return FFWERR_MEMORY_NOT_SECURED;				// メモリ確保失敗
	}
	pbyBuff = pbyReadBufEsc;
	memset(pbyBuff, 0, dwTotalLength + 0x3);

	// 先頭余り部の Dump
	if ((madrTempStartAddr % 4) == 1) {	// 先頭アドレス 4 で割った余りが 1 の場合
		ferr = MemDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト DUMP 処理実行。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
			bDumpEndFlag = TRUE;				// Dump 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	if ((bDumpEndFlag == FALSE) && ((madrTempStartAddr % 4) == 2)) {	// 先頭アドレス 4 で割った余りが 2 の場合
		if ((eAccessSize == MBYTE_ACCESS) || (madrTempStartAddr == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
			ferr = MemDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト DUMP 処理実行。
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
				bDumpEndFlag = TRUE;				// Dump 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
			pbyBuff += 1;			// バッファアドレスを更新。
		} else {	// ワードアクセス以上かつ２バイト以上ある場合
			ferr = MemDump(madrTempStartAddr, madrTempStartAddr + 1, MWORD_ACCESS, pbyBuff);		// 1 ワード DUMP 処理実行。
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if ((madrTempStartAddr + 1) == madrEndAddr) {	// 終了アドレスなら、
				bDumpEndFlag = TRUE;					// Dump 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 2;	// そのときの先頭アドレスを更新。
			pbyBuff += 2;			// バッファアドレスを更新。
		}
	}
	if ((bDumpEndFlag == FALSE) && ((madrTempStartAddr % 4) == 3)) {	// 先頭アドレス 4 で割った余りが 3 の場合
		ferr = MemDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト DUMP 処理実行。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
			bDumpEndFlag = TRUE;				// Dump 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	// 先頭余り部があるとき終わり。
	// Dump 終了でなければ、残りの先頭アドレスは 4 で割り切れる状態になっている。

	// 本体部の Dump。
	if ((bDumpEndFlag == FALSE) && 									// Dump 終了フラッグが「終了でない」、かつ
				((madrEndAddr - madrTempStartAddr) >= 3L )) {			// 本体部先頭から End アドレスまで 3 以上ある場合(すなわち本体部がある場合)
		if (madrEndAddrRemainder != 3) {								// 最終アドレス余りが 3 でない場合
			madrTempEndAddr = madrEndAddr - madrEndAddrRemainder - 1;		// 本体部の最終アドレス。余り分と 1 を引く。
		} else {													// 最終アドレス余りが 3 の場合
			madrTempEndAddr = madrEndAddr;								// 本体部の最終アドレス。madrEndAddr のまま。
			bDumpEndFlag = TRUE;	// Dump 終了フラッグ設定(終了)。
		}
		ferr = MemDump(madrTempStartAddr, madrTempEndAddr, eAccessSize, pbyBuff);	// 本体部の DUMP 処理実行。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		pbyBuff = pbyBuff + (madrTempEndAddr - madrTempStartAddr + 1);	// pbyBuff 更新。
		madrTempStartAddr = madrTempEndAddr + 1;						// 最終余り部の先頭アドレス設定
	}

	// 最終余り部の Dump
	if (bDumpEndFlag == FALSE) {	// Dump 終了フラッグが「終了でない」場合。（先頭アドレス 4 で割った余りは 0 のはずである。）
		if ((eAccessSize == MBYTE_ACCESS) || (madrTempStartAddr == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
			ferr = MemDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト DUMP 処理実行。
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
				bDumpEndFlag = TRUE;				// Dump 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
			pbyBuff += 1;			// バッファアドレスを更新。
		} else {	// ワードアクセス以上かつ２バイト以上ある場合
			ferr = MemDump(madrTempStartAddr, madrTempStartAddr + 1, MWORD_ACCESS, pbyBuff);		// 1 ワード DUMP 処理実行。
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if ((madrTempStartAddr + 1) == madrEndAddr) {	// 終了アドレスなら、
				bDumpEndFlag = TRUE;					// Dump 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 2;	// そのときの先頭アドレスを更新。
			pbyBuff += 2;			// バッファアドレスを更新。
		}
	}
	if ((bDumpEndFlag == FALSE) && ((madrTempStartAddr % 4) == 1)) {	// 先頭アドレス 4 で割った余りが 1 の場合
		ferr = MemDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト DUMP 処理実行。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
			bDumpEndFlag = TRUE;				// Dump 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	if ((bDumpEndFlag == FALSE) && ((madrTempStartAddr % 4) == 2)) {	// 先頭アドレス 4 で割った余りが 2 の場合
		ferr = MemDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト DUMP 処理実行。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
	}	// 最大でもここまでである。

	// DUMP データの並べ替え
	pbyBuff = pbyReadBufEsc;
	ferr = changeOrderDumpBuff(madrStartAddr, madrEndAddr, eAccessSize, pbyBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		free(pbyReadBufEsc);
		return ferr;
	}

	memcpy(pbyReadBuff, pbyBuff, dwTotalLength);

	free(pbyReadBufEsc);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	return ferr;
}

//=============================================================================
/**
 * ターゲットメモリに単一データを書き込む。
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param dwWriteDataSize 書き込みデータサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_FILL(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
					  enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff,
					  FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	DWORD	i;
	MADDR	madrAreaEnd;
	BYTE	byEndian;
	DWORD	dwLength;
	// V.1.02 RevNo110304-001 Deleate & Append Line		Delete:BOOL	bFlashRomArea = FALSE;
	enum FFWRXENM_MAREA_TYPE	eAreaType;
	BOOL	bEnableArea = FALSE;

	DWORD	dwSeparateNum;				// アドレスを区切る値(1, 2, 4 バイトデータサイズ時は 4 で区切り、8 バイトデータサイズ時は 8 で区切る)
	BOOL	bFillEndFlag = FALSE;		// Fill 終了フラグ(FALSE:終了でない、TRUE:終了)
	MADDR	madrStartRemainder;			// 先頭アドレスを 4 で割った余り
	MADDR	madrEndRemainder;			// 最終アドレスを 4 で割った余り
	MADDR	madrMainStartAddr;			// 先頭の余りを除いた本体部の先頭アドレス
	MADDR	madrMainEndAddr;			// 最終の余りを除いた本体部の最終アドレス
	BYTE	byBeforeEndian;				// 前の領域のエンディアンを格納
	BYTE	byWriteBuffL[8];			// リトルエンディアン時のFILLバッファ
	BYTE	byWriteBuffB[8];			// ビッグエンディアン時のFILLバッファ
	BYTE	byWriteBuffLtoL[8];			// リトルエンディアン領域からリトルエンディアン領域時のFILLバッファ
	BYTE	byWriteBuffBtoB[8];			// ビッグエンディアン領域からビッグエンディアン領域時のFILLバッファ
	BYTE	byWriteBuffLtoBFirst[8];	// リトルエンディアン領域からビッグエンディアン領域時の１回目FILLバッファ
	BYTE	byWriteBuffBtoLFirst[8];	// ビッグエンディアン領域からリトルエンディアン領域時の１回目FILLバッファ
	BYTE	byWriteBuffRemainder[8];	// 余り部のFILLバッファ
	MADDR	madrStartAddrTemp;			// 更新していくスタートアドレス
	MADDR	madrStartRemainder_DataSize;	// スタートアドレスをデータサイズで割った余り

	ProtInit();

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start

	// 1, 2, 4 バイトデータサイズ時は 4 で区切り、8 バイトデータサイズ時は 8 で区切る。
	if (dwWriteDataSize <= 4) {
		dwSeparateNum = 4;
	} else {
		dwSeparateNum = 8;
	}

	// byWriteBuffL[]、byWriteBuffB[] 設定
	for (i = 0; i < dwWriteDataSize; i++) {
		byWriteBuffL[i] = *(pbyWriteBuff + dwWriteDataSize - 1 - i);	// リトルエンディアン時のFILLバッファ
		byWriteBuffB[i] = *(pbyWriteBuff + i);							// ビッグエンディアン時のFILLバッファ
	}
	// データサイズ 1,2 のときは、バッファサイズ 4 まで拡張する。4 で区切るため。
	if (dwWriteDataSize == 1) {
		for (i = 1; i < 4; i++) {
			byWriteBuffL[i] = byWriteBuffL[0];
			byWriteBuffB[i] = byWriteBuffB[0];
		}
	}
	if (dwWriteDataSize == 2) {
			byWriteBuffL[2] = byWriteBuffL[0];
			byWriteBuffB[2] = byWriteBuffB[0];
			byWriteBuffL[3] = byWriteBuffL[1];
			byWriteBuffB[3] = byWriteBuffB[1];
	}

	madrStartRemainder = madrStartAddr % dwSeparateNum;	// 先頭アドレス値の余り。
	madrEndRemainder   = madrEndAddr % dwSeparateNum;	// 最終アドレス値の余り。

	madrMainStartAddr = madrStartAddr;					// madrStartAddr を初期値としておく。
	madrMainEndAddr   = madrEndAddr;					// madrEndAddr   を初期値としておく。

	ferr = GetEndianType2(madrMainStartAddr, &byEndian);		// エンディアン判定＆取得
	if(ferr != FFWERR_OK){
		return ferr;
	}
	byBeforeEndian = byEndian;							// 前の領域のエンディアンを格納

	// 先頭余り部の Fill
	if (madrStartRemainder != 0) {													// 先頭余り部がある場合
		if ((madrEndAddr - madrStartAddr) < (dwSeparateNum - madrStartRemainder)) {	// 先頭余り部より FILL長が以下のとき
			bFillEndFlag = TRUE;													// Fill 終了フラッグ設定(終了)。
			madrStartRemainder = dwSeparateNum - (madrEndAddr - madrStartAddr + 1);
		}
	}

	if (madrStartRemainder != 0) {					// 先頭余り部がある場合
		// 先頭余り部のFillバッファにデータ格納
		if (byEndian == FFWRX_ENDIAN_LITTLE) {
			for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
				byWriteBuffRemainder[i] = byWriteBuffL[i];
			}
		} else {
			for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
				byWriteBuffRemainder[i] = byWriteBuffB[i];
			}
		}
		for (i = 0; i < (dwSeparateNum - madrStartRemainder); ) {
			if ((madrStartAddr + i) % 4 == 0) {	// 先頭アドレス 4 で割った余りが 0 の場合
				if ((eAccessSize == MBYTE_ACCESS) || ((madrStartAddr + i) == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
					ferr = MemFill(madrStartAddr + i, madrStartAddr + i, eVerify, MBYTE_ACCESS, 1, &byWriteBuffRemainder[i % dwWriteDataSize], pVerifyErr);	// FILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 1;
				} else if ((eAccessSize == MWORD_ACCESS) || ((madrStartAddr + i + 2) >= madrEndAddr)){	// ワードアクセスまたは３バイト以下である場合
					ferr = MemFill(madrStartAddr + i, madrStartAddr + i + 1, eVerify, MWORD_ACCESS, 2, &byWriteBuffRemainder[i % dwWriteDataSize], pVerifyErr);	// ワードFILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 2;
				} else {	// ロングワードアクセスかつ４バイト以上である場合
					ferr = MemFill(madrStartAddr + i, madrStartAddr + i + 3, eVerify, MLWORD_ACCESS, 4, &byWriteBuffRemainder[i % dwWriteDataSize], pVerifyErr);	// ロングワードFILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 4;
				}
			} else if (((madrStartAddr + i) % 2) == 0) {	// 先頭アドレス 2 で割った余りが 0 の場合
				if ((eAccessSize == MBYTE_ACCESS) || ((madrStartAddr + i) == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
					ferr = MemFill(madrStartAddr + i, madrStartAddr + i, eVerify, MBYTE_ACCESS, 1, &byWriteBuffRemainder[i % dwWriteDataSize], pVerifyErr);	// FILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 1;
				} else {	// ワードアクセス以上かつ２バイト以上ある場合
					ferr = MemFill(madrStartAddr + i, madrStartAddr + i + 1, eVerify, MWORD_ACCESS, 2, &byWriteBuffRemainder[i % dwWriteDataSize], pVerifyErr);	// ワードFILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 2;
				}
			} else if (((madrStartAddr + i) % 2) == 1) {	// 先頭アドレス 2 で割った余りが 1 の場合
				ferr = MemFill(madrStartAddr + i, madrStartAddr + i, eVerify, MBYTE_ACCESS, 1, &byWriteBuffRemainder[i % dwWriteDataSize], pVerifyErr);	// FILL処理実行
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					return ferr;
				}
				i += 1;
			}
		}
		madrMainStartAddr = madrStartAddr - madrStartRemainder + dwSeparateNum;		// madrMainStartAddr を 4 (8バイトデータ時は 8)の倍数へ更新。
	}	// 先頭余り部があるとき終わり。
	// FILL 終了でなければ、残りの先頭アドレスは 4 (8バイトデータ時は 8)で割り切れる状態になっている。

	// 本体部用にFILLバッファを更新する。（本体部はデータサイズ単位でFILLするが、最終端数部は dwSeparateNum 単位でFILLする。）
	 // エンディアンが同じとき用バッファ
	for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
		byWriteBuffLtoL[i] = byWriteBuffL[(dwSeparateNum - madrStartRemainder + i) % dwSeparateNum];	// リトルエンディアン領域からリトルエンディアン領域時のFILLバッファ
		byWriteBuffBtoB[i] = byWriteBuffB[(dwSeparateNum - madrStartRemainder + i) % dwSeparateNum];	// ビッグエンディアン領域からビッグエンディアン領域時のFILLバッファ
	}
	 // エンディアンが変わったとき用バッファ
	madrStartRemainder_DataSize = madrStartAddr % dwWriteDataSize;	// データサイズで割った先頭アドレス値の余り。
	if (madrStartRemainder_DataSize != 0) {	// データサイズで割った余りがある場合
		for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
			if (i < madrStartRemainder_DataSize) {	// データサイズで割った余りが次のエンディアン区切りまでの残り
				byWriteBuffLtoBFirst[i] = byWriteBuffL[(dwSeparateNum - madrStartRemainder + i) % dwSeparateNum];	// リトルエンディアン領域からビッグエンディアン領域時のFILLバッファ
				byWriteBuffBtoLFirst[i] = byWriteBuffB[(dwSeparateNum - madrStartRemainder + i) % dwSeparateNum];	// ビッグエンディアン領域からリトルエンディアン領域時のFILLバッファ
			} else {
				byWriteBuffLtoBFirst[i] = byWriteBuffB[(dwSeparateNum - madrStartRemainder + i) % dwSeparateNum];
				byWriteBuffBtoLFirst[i] = byWriteBuffL[(dwSeparateNum - madrStartRemainder + i) % dwSeparateNum];
			}
		}
	} else {						// 余りがない場合
		for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
			byWriteBuffLtoBFirst[i] = byWriteBuffB[i];
			byWriteBuffBtoLFirst[i] = byWriteBuffL[i];
		}
	}

	madrStartAddrTemp = madrMainStartAddr;	// 本体部の実行がなかった場合のため、ここで設定しておく。

	// 本体部の FILL。
	if ((bFillEndFlag == FALSE) && 										// FILL 終了フラッグが「終了でない」、かつ
				((madrEndAddr - madrMainStartAddr) >= (dwSeparateNum - 1))) {	// 本体部先頭から End アドレスまで 3 (8バイトデータ時は 7)以上ある場合(すなわち本体部がある場合)
		if (madrEndRemainder != (dwSeparateNum - 1)) {					// 最終アドレス余りが 3 (8バイトデータ時は 7)でない場合
			madrMainEndAddr = madrEndAddr - madrEndRemainder - 1;		// 本体部の最終アドレス。余り分と 1 を引く。
		} else {														// 最終アドレス余りが 3 (8バイトデータ時は 7)の場合
			madrMainEndAddr = madrEndAddr;								// 本体部の最終アドレス。madrEndAddr のまま。
		}
		madrAreaEnd = 0;
		for ( ; madrMainEndAddr != madrAreaEnd; ) {
			// 指定領域判定
			// V.1.02 RevNo110304-001 Modify Line
			ferr = ChkBlockArea(madrStartAddrTemp, madrMainEndAddr, &dwLength, &eAreaType, 1, &bEnableArea);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			ferr = GetEndianType2(madrStartAddrTemp, &byEndian);		// エンディアン判定＆取得
			if(ferr != FFWERR_OK){
				ferrEnd = ProtEnd();
				return ferr;
			}
			madrAreaEnd = madrStartAddrTemp + dwLength - 1;
			if ((byBeforeEndian == FFWRX_ENDIAN_LITTLE) && (byEndian == FFWRX_ENDIAN_LITTLE)) {	// 前がリトル、今もリトルの場合
				ferr = MemFill(madrStartAddrTemp, madrAreaEnd, eVerify, eAccessSize, dwWriteDataSize, &byWriteBuffLtoL[0], pVerifyErr);	// FILL処理実行
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					return ferr;
				}		
			}
			if ((byBeforeEndian == FFWRX_ENDIAN_BIG) && (byEndian == FFWRX_ENDIAN_BIG)) {			// 前がビッグ、今もビッグの場合
				ferr = MemFill(madrStartAddrTemp, madrAreaEnd, eVerify, eAccessSize, dwWriteDataSize, &byWriteBuffBtoB[0], pVerifyErr);	// FILL処理実行
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					return ferr;
				}		
			}
			if ((byBeforeEndian == FFWRX_ENDIAN_LITTLE) && (byEndian == FFWRX_ENDIAN_BIG)) {		// 前がリトル、今はビッグの場合
				madrAreaEnd = madrStartAddrTemp + dwWriteDataSize - 1;				// 1 データサイズ分のFILLのみ実施
				ferr = MemFill(madrStartAddrTemp, madrAreaEnd, eVerify, eAccessSize, dwWriteDataSize, &byWriteBuffLtoBFirst[0], pVerifyErr);	// FILL処理実行
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					return ferr;
				}		
			}
			if ((byBeforeEndian == FFWRX_ENDIAN_BIG) && (byEndian == FFWRX_ENDIAN_LITTLE)) {		// 前がビッグ、今はリトルの場合
				madrAreaEnd = madrStartAddrTemp + dwWriteDataSize - 1;				// 1 データサイズ分のFILLのみ実施
				ferr = MemFill(madrStartAddrTemp, madrAreaEnd, eVerify, eAccessSize, dwWriteDataSize, &byWriteBuffBtoLFirst[0], pVerifyErr);	// FILL処理実行
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					return ferr;
				}		
			}
			// 次の繰り返し用設定
			madrStartAddrTemp = madrAreaEnd + 1;
			byBeforeEndian = byEndian;	// 前の領域のエンディアンを格納
		}

	}

	// 最終余り部の Fill
	if ((bFillEndFlag == FALSE) && (madrEndRemainder != (dwSeparateNum - 1))) {	// Dump 終了フラッグが「終了でない」、かつ
																				// 最終アドレス余りが 3 (8バイトデータ時は 7)でない場合
		// 指定領域判定
		// V.1.02 RevNo110304-001 Modify Line
		ferr = ChkBlockArea(madrStartAddrTemp, madrEndAddr, &dwLength, &eAreaType, 1, &bEnableArea);
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		ferr = GetEndianType2(madrStartAddrTemp, &byEndian);										// エンディアン判定＆取得
		if(ferr != FFWERR_OK){
			ferrEnd = ProtEnd();
			return ferr;
		}
		// 最終余り部のFillバッファにデータ格納
		if ((byBeforeEndian == FFWRX_ENDIAN_LITTLE) && (byEndian == FFWRX_ENDIAN_LITTLE)) {	// 前がリトル、今もリトルの場合
			for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
				byWriteBuffRemainder[i] = byWriteBuffLtoL[i];
			}
		}
		if ((byBeforeEndian == FFWRX_ENDIAN_BIG) && (byEndian == FFWRX_ENDIAN_BIG)) {		// 前がビッグ、今もビッグの場合
			for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
				byWriteBuffRemainder[i] = byWriteBuffBtoB[i];
			}
		}
		if ((byBeforeEndian == FFWRX_ENDIAN_LITTLE) && (byEndian == FFWRX_ENDIAN_BIG)) {	// 前がリトル、今はビッグの場合
			for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
				byWriteBuffRemainder[i] = byWriteBuffLtoBFirst[i];
			}
		}
		if ((byBeforeEndian == FFWRX_ENDIAN_BIG) && (byEndian == FFWRX_ENDIAN_LITTLE)) {	// 前がビッグ、今はリトルの場合
			for (i = 0; i < dwSeparateNum; i++) {	// 4 か 8 まで拡張する。
				byWriteBuffRemainder[i] = byWriteBuffBtoLFirst[i];
			}
		}

		for (i = 0; i <= madrEndRemainder; ) {
			if (((madrStartAddrTemp + i) % 4) == 0) {	// 先頭アドレス 4 で割った余りが 0 の場合
				if ((eAccessSize == MBYTE_ACCESS) || ((madrStartAddrTemp + i) == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
					ferr = MemFill(madrStartAddrTemp + i, madrStartAddrTemp + i, eVerify, MBYTE_ACCESS, 1, &byWriteBuffRemainder[i], pVerifyErr);	// FILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 1;
				} else if ((eAccessSize == MWORD_ACCESS) || ((madrStartAddrTemp + i + 2) >= madrEndAddr)){	// ワードアクセスまたは３バイト以下である場合
					ferr = MemFill(madrStartAddrTemp + i, madrStartAddrTemp + i + 1, eVerify, MWORD_ACCESS, 2, &byWriteBuffRemainder[i], pVerifyErr);	// ワードFILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 2;
				} else {	// ロングワードアクセスかつ４バイト以上である場合
					ferr = MemFill(madrStartAddrTemp + i, madrStartAddrTemp + i + 3, eVerify, MLWORD_ACCESS, 4, &byWriteBuffRemainder[i], pVerifyErr);	// ロングワードFILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 4;
				}
			} else if (((madrStartAddrTemp + i) % 2) == 0) {	// 先頭アドレス 2 で割った余りが 0 の場合
				if ((eAccessSize == MBYTE_ACCESS) || ((madrStartAddrTemp + i) == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
					ferr = MemFill(madrStartAddrTemp + i, madrStartAddrTemp + i, eVerify, MBYTE_ACCESS, 1, &byWriteBuffRemainder[i], pVerifyErr);	// FILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 1;
				} else {	// ワードアクセス以上かつ２バイト以上ある場合
					ferr = MemFill(madrStartAddrTemp + i, madrStartAddrTemp + i + 1, eVerify, MWORD_ACCESS, 2, &byWriteBuffRemainder[i], pVerifyErr);	// ワードFILL処理実行
					if (ferr != FFWERR_OK) {
						ferrEnd = ProtEnd();
						return ferr;
					}
					if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
						ferrEnd = ProtEnd();
						return ferr;
					}
					i += 2;
				}
			} else if (((madrStartAddrTemp + i) % 2) == 1) {	// 先頭アドレス 2 で割った余りが 1 の場合
				ferr = MemFill(madrStartAddrTemp + i, madrStartAddrTemp + i, eVerify, MBYTE_ACCESS, 1, &byWriteBuffRemainder[i], pVerifyErr);	// FILL処理実行
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					return ferr;
				}
				i += 1;
			}
		}
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	return ferr;
}

//=============================================================================
/**
 * ターゲットメモリ内容を変更する。
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_WRITE(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
					  enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	BOOL	bWriteEndFlag = FALSE;		// Write 終了フラグ(FALSE:終了でない、TRUE:終了)
	MADDR	madrEndAddrRemainder;		// 最終アドレスを 4 で割った余り
	MADDR	madrTempStartAddr;			// そのときの先頭アドレス
	MADDR	madrTempEndAddr;			// そのときの最終アドレス
	DWORD	dwTotalLength;				// WRITE全体のバイトサイズ
	BYTE	*pbyBuffOrg = NULL;			// 動的メモリ確保用
	BYTE	*pbyBuff;					// ポインタ変更用

	enum FFWENM_MACCESS_SIZE	eExtRamAccessSize;	// 外部RAM領域リード用アクセスサイズ
	MADDR	madrReadStartAddr;			// 外部RAM領域リード用先頭アドレス
	BYTE	byReadBuf[8];				// 外部RAM領域リード用データバッファ(4バイトでOKのはずだが念のため)
	DWORD	dwAccessCount;				// 外部RAM領域リード用データ長
	DWORD	dwModifyLength;				// 外部RAM領域モディファイ用バイトデータ長
	BOOL	bExtRamArea;				// 外部RAM領域かどうか判断
	DWORD	dwReadCnt;					// 外部RAM領域リード用カウンタ
	DWORD	dwModifyStartIndex;			// リードモディファイ開始インデックス
	BYTE	byEndian;

	ProtInit();

	// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append End

	// WRITE データの並べ替え
	dwTotalLength = madrEndAddr - madrStartAddr + 1;
	// RevNo121017-002	Modify Line
	pbyBuffOrg = (BYTE*) new BYTE[dwTotalLength + 3L];			// 動的メモリ確保
																// 「+ 3」しているのは、リトルエンディアンでロングワードのとき、
																// FFWMCUCmd_WRITE からのバッファは、領域長 + 3 があり得るため。
	pbyBuff = &pbyBuffOrg[0];	// ポインタ変更用

	ferr = changeOrderWriteBuff(madrStartAddr, madrEndAddr, eAccessSize, pbyWriteBuff, pbyBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		delete [] pbyBuffOrg;	// メモリ開放
		return ferr;
	}

	madrEndAddrRemainder = madrEndAddr % 4;	// 最終余り。

	madrTempStartAddr = madrStartAddr;		// madrStartAddr を初期値。
	madrTempEndAddr   = madrEndAddr;		// madrEndAddr   を初期値。

	// 先頭余り部の Write
	if ((madrTempStartAddr % 4) == 1) {				// 先頭アドレス 4 で割った余りが 1 の場合
		// 外部RAM領域判定
		// V.1.02 RevNo110510-003 Modify Line
		GetExtAccessSize(madrTempStartAddr, madrTempStartAddr, &eExtRamAccessSize, &bExtRamArea);
		// 外部RAM領域、DWNPコマンド発行時、バイトアクセス以外
		if (bExtRamArea == TRUE && GetDwnpOpenData() == TRUE && eExtRamAccessSize != MBYTE_ACCESS) {
			// ワードアクセス時
			if (eExtRamAccessSize == MWORD_ACCESS) {
				madrReadStartAddr = madrTempStartAddr - 1;
				dwModifyStartIndex = 1;	// 上書きは1バイト目から
				dwModifyLength = 2;
			}
			// ロングワードアクセス時
			else {
				madrReadStartAddr = madrTempStartAddr - 1;
				dwModifyStartIndex = 1;	// 上書きは3バイト目から
				dwModifyLength = 4;
			}
			dwAccessCount = 1;		// PROT_MCU_DUMPの引数として渡すレングスはバイト数ではなくアクセス回数
			// リードモディファイ用のデータをダンプ
			// (バイト並びでデータがほしいのでPROT_MCU_DUMPを直接呼出し) => 正しいエンディアン情報必要 2010.8.23
			ferr = GetEndianType2(madrReadStartAddr, &byEndian);			// エンディアン判定＆取得
			if(ferr != FFWERR_OK){
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
// RevRxNo121022-001 Append Start
			// 端数処理の為、ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないので、
			// BFWMCUCmd_DUMPコマンドのリードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を
			// 呼び出す必要はない。
// RevRxNo121022-001 Append End
			ferr = PROT_MCU_DUMP(1, &madrReadStartAddr, FALSE, &eExtRamAccessSize, 
						FALSE, &dwAccessCount, &byReadBuf[0], byEndian);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			// ライトデータを上書き
			for (dwReadCnt = dwModifyStartIndex; dwReadCnt < dwModifyLength; dwReadCnt++) {
				byReadBuf[dwReadCnt] = *(pbyBuff);
				if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
					bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
				}
				madrTempStartAddr += 1;						// そのときの先頭アドレスを更新。
				pbyBuff += 1;								// バッファアドレスを更新。
				if (bWriteEndFlag == TRUE) {
					break;									// ライトデータがない事になるので抜ける
				}
			}
			ferr = MemWrite(madrReadStartAddr, (madrReadStartAddr + (dwModifyLength - 1)), eVerify, eExtRamAccessSize, &byReadBuf[0], pVerifyErr);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
		}
		// それ以外の場合
		else {
			ferr = MemWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
				bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 1;						// そのときの先頭アドレスを更新。
			pbyBuff += 1;								// バッファアドレスを更新。
		}
	}
	if ((bWriteEndFlag == FALSE) && ((madrTempStartAddr % 4) == 2)) {				// 先頭アドレス 4 で割った余りが 2 の場合
		// 外部RAM領域判定
		// V.1.02 RevNo110510-003 Modify Start
		if ((eAccessSize == MBYTE_ACCESS) || (madrTempStartAddr == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
			GetExtAccessSize(madrTempStartAddr, madrTempStartAddr, &eExtRamAccessSize, &bExtRamArea);
		}else{
			GetExtAccessSize(madrTempStartAddr, madrTempStartAddr+1, &eExtRamAccessSize, &bExtRamArea);
		}
		// 外部RAM領域、DWNPコマンド発行時、バイトアクセス以外
		if (bExtRamArea == TRUE && GetDwnpOpenData() == TRUE && eExtRamAccessSize != MBYTE_ACCESS) {
			// ワードアクセス時
			if (eExtRamAccessSize == MWORD_ACCESS) {
				madrReadStartAddr = madrTempStartAddr;
				dwModifyStartIndex = 0;	// 上書きは0バイト目から
				dwModifyLength = 2;
			}
			// ロングワードアクセス時
			else {
				madrReadStartAddr = madrTempStartAddr - 2;
				dwModifyStartIndex = 2;	// 上書きは2バイト目から
				dwModifyLength = 4;
			}
			dwAccessCount = 1;		// PROT_MCU_DUMPの引数として渡すレングスはバイト数ではなくアクセス回数
			// リードモディファイ用のデータをダンプ
			// (バイト並びでデータがほしいのでPROT_MCU_DUMPを直接呼出し) => 正しいエンディアン情報必要 2010.8.23
			ferr = GetEndianType2(madrReadStartAddr, &byEndian);			// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
// RevRxNo121022-001 Append Start
			// 端数処理の為、ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないので、
			// BFWMCUCmd_DUMPコマンドのリードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を
			// 呼び出す必要はない。
// RevRxNo121022-001 Append End
			ferr = PROT_MCU_DUMP(1, &madrReadStartAddr, FALSE, &eExtRamAccessSize, 
						FALSE, &dwAccessCount, &byReadBuf[0], byEndian);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			// ライトデータを上書き
			for (dwReadCnt = dwModifyStartIndex; dwReadCnt < dwModifyLength; dwReadCnt++) {
				byReadBuf[dwReadCnt] = *(pbyBuff);
				if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
					bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
				}
				madrTempStartAddr += 1;						// そのときの先頭アドレスを更新。
				pbyBuff += 1;								// バッファアドレスを更新。
				if (bWriteEndFlag == TRUE) {
					break;									// ライトデータがない事になるので抜ける
				}
			}
			ferr = MemWrite(madrReadStartAddr, (madrReadStartAddr + (dwModifyLength - 1)), eVerify, eExtRamAccessSize, &byReadBuf[0], pVerifyErr);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
		}
		// それ以外の場合
		else {
			if ((eAccessSize == MBYTE_ACCESS) || (madrTempStartAddr == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
				ferr = MemWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
					bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
				}
				madrTempStartAddr += 1;						// そのときの先頭アドレスを更新。
				pbyBuff += 1;								// バッファアドレスを更新。
			} else {	// ワードアクセス以上かつ２バイト以上ある場合
				ferr = MemWrite(madrTempStartAddr, madrTempStartAddr + 1, eVerify, MWORD_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;						// メモリ開放
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;						// メモリ開放
					return ferr;
				}
				if ((madrTempStartAddr + 1) == madrEndAddr) {	// 終了アドレスなら、
					bWriteEndFlag = TRUE;						// Write 終了フラッグ設定(終了)。
				}
				madrTempStartAddr += 2;							// そのときの先頭アドレスを更新。
				pbyBuff += 2;									// バッファアドレスを更新。
			}
		}
	}
	if ((bWriteEndFlag == FALSE) && ((madrTempStartAddr % 4) == 3)) {	// 先頭アドレス 4 で割った余りが 3 の場合
		// 外部RAM領域判定
		// V.1.02 RevNo110510-003 Modify Line
		GetExtAccessSize(madrTempStartAddr, madrTempStartAddr, &eExtRamAccessSize, &bExtRamArea);
		// 外部RAM領域、DWNPコマンド発行時、バイトアクセス以外
		if (bExtRamArea == TRUE && GetDwnpOpenData() == TRUE && eExtRamAccessSize != MBYTE_ACCESS) {
			// ワードアクセス時
			if (eExtRamAccessSize == MWORD_ACCESS) {
				madrReadStartAddr = madrTempStartAddr - 1;
				dwModifyStartIndex = 1;	// 上書きは3バイト目から
				dwModifyLength = 2;
			}
			// ロングワードアクセス時
			else {
				madrReadStartAddr = madrTempStartAddr - 3;
				dwModifyStartIndex = 3;	// 上書きは3バイト目から
				dwModifyLength = 4;
			}
			dwAccessCount = 1;		// PROT_MCU_DUMPの引数として渡すレングスはバイト数ではなくアクセス回数
			// リードモディファイ用のデータをダンプ
			// (バイト並びでデータがほしいのでPROT_MCU_DUMPを直接呼出し) => 正しいエンディアン情報必要 2010.8.23
			ferr = GetEndianType2(madrReadStartAddr, &byEndian);			// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
// RevRxNo121022-001 Append Start
			// 端数処理の為、ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないので、
			// BFWMCUCmd_DUMPコマンドのリードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を
			// 呼び出す必要はない。
// RevRxNo121022-001 Append End
			ferr = PROT_MCU_DUMP(1, &madrReadStartAddr, FALSE, &eExtRamAccessSize, 
						FALSE, &dwAccessCount, &byReadBuf[0], byEndian);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			// ライトデータを上書き
			for (dwReadCnt = dwModifyStartIndex; dwReadCnt < dwModifyLength; dwReadCnt++) {
				byReadBuf[dwReadCnt] = *(pbyBuff);
				if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
					bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
				}
				madrTempStartAddr += 1;						// そのときの先頭アドレスを更新。
				pbyBuff += 1;								// バッファアドレスを更新。
				if (bWriteEndFlag == TRUE) {
					break;									// ライトデータがない事になるので抜ける
				}
			}
			ferr = MemWrite(madrReadStartAddr, (madrReadStartAddr + (dwModifyLength - 1)), eVerify, eExtRamAccessSize, &byReadBuf[0], pVerifyErr);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
		}
		// それ以外の場合
		else {
			ferr = MemWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
				bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 1;						// そのときの先頭アドレスを更新。
			pbyBuff += 1;								// バッファアドレスを更新。
		}
	}
	// 先頭余り部があるとき終わり。
	// Write 終了でなければ、残りの先頭アドレスは 4 で割り切れる状態になっている。

	// 本体部の Write。
	if ((bWriteEndFlag == FALSE) && 									// Write 終了フラッグが「終了でない」、かつ
				((madrEndAddr - madrTempStartAddr) >= 3L )) {			// 本体部先頭から End アドレスまで 3 以上ある場合(すなわち本体部がある場合)
		if (madrEndAddrRemainder != 3) {								// 最終アドレス余りが 3 でない場合
			madrTempEndAddr = madrEndAddr - madrEndAddrRemainder - 1;	// 本体部の最終アドレス。余り分と 1 を引く。
		} else {														// 最終アドレス余りが 3 の場合
			madrTempEndAddr = madrEndAddr;								// 本体部の最終アドレス。madrEndAddr のまま。
			bWriteEndFlag = TRUE;										// Write 終了フラッグ設定(終了)。
		}
		ferr = MemWrite(madrTempStartAddr, madrTempEndAddr, eVerify, eAccessSize, pbyBuff, pVerifyErr);	// 本体部の WRITE 処理実行。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			delete [] pbyBuffOrg;										// メモリ開放
			return ferr;
		}
		if (pVerifyErr->eErrorFlag == VERIFY_ERR) {						// ベリファイエラー発生時、処理を終了する。
			ferrEnd = ProtEnd();
			delete [] pbyBuffOrg;										// メモリ開放
			return ferr;
		}
		pbyBuff = pbyBuff + (madrTempEndAddr - madrTempStartAddr + 1);	// pbyBuff 更新。
		madrTempStartAddr = madrTempEndAddr + 1;						// 最終余り部の先頭アドレス設定
	}

	// 最終余り部の Write
	if ((bWriteEndFlag == FALSE) && ((madrEndAddr % 4) == 0)) {	// 先頭アドレス 4 で割った余りが 0 の場合
		// 外部RAM領域判定
		// V.1.02 RevNo110510-003 Modify Line
		GetExtAccessSize(madrTempStartAddr, madrEndAddr, &eExtRamAccessSize, &bExtRamArea);
		// 外部RAM領域、DWNPコマンド発行時、バイトアクセス以外
		if (bExtRamArea == TRUE && GetDwnpOpenData() == TRUE && eExtRamAccessSize != MBYTE_ACCESS) {
			// ワードアクセス時
			if (eExtRamAccessSize == MWORD_ACCESS) {
				madrReadStartAddr = madrTempStartAddr;
				dwModifyStartIndex = 0;	// 上書きは1バイト目から
				dwModifyLength = 2;
			}
			// ロングワードアクセス時
			else {
				madrReadStartAddr = madrTempStartAddr;
				dwModifyStartIndex = 0;	// 上書きは1バイト目から
				dwModifyLength = 4;
			}
			dwAccessCount = 1;		// PROT_MCU_DUMPの引数として渡すレングスはバイト数ではなくアクセス回数
			// リードモディファイ用のデータをダンプ
			// (バイト並びでデータがほしいのでPROT_MCU_DUMPを直接呼出し) => 正しいエンディアン情報必要 2010.8.23
			ferr = GetEndianType2(madrReadStartAddr, &byEndian);			// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
// RevRxNo121022-001 Append Start
			// 端数処理の為、ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないので、
			// BFWMCUCmd_DUMPコマンドのリードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を
			// 呼び出す必要はない。
// RevRxNo121022-001 Append End
			ferr = PROT_MCU_DUMP(1, &madrReadStartAddr, FALSE, &eExtRamAccessSize, 
						FALSE, &dwAccessCount, &byReadBuf[0], byEndian);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			// ライトデータを上書き
			for (dwReadCnt = dwModifyStartIndex; dwReadCnt < dwModifyLength; dwReadCnt++) {
				byReadBuf[dwReadCnt] = *(pbyBuff);
				if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
					bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
				}
				if (bWriteEndFlag == TRUE) {
					break;									// ライトデータがない事になるので抜ける
				}
			}
			ferr = MemWrite(madrReadStartAddr, (madrReadStartAddr + (dwModifyLength - 1)), eVerify, eExtRamAccessSize, &byReadBuf[0], pVerifyErr);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
		}
		// それ以外の場合( ライトしなければならないデータは1バイトのみ )
		else {
			ferr = MemWrite(madrTempStartAddr, madrEndAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
				bWriteEndFlag = TRUE;				// Write 終了フラッグ設定(終了)。
			}
		}
	}
	if ((bWriteEndFlag == FALSE) && ((madrEndAddr % 4) == 1)) {	// 先頭アドレス 4 で割った余りが 1 の場合
		// 外部RAM領域判定
		// V.1.02 RevNo110510-003 Modify Line
		GetExtAccessSize(madrTempStartAddr, madrEndAddr, &eExtRamAccessSize, &bExtRamArea);
		// 外部RAM領域、DWNPコマンド発行時、バイトアクセス以外
		if (bExtRamArea == TRUE && GetDwnpOpenData() == TRUE && eExtRamAccessSize != MBYTE_ACCESS) {
			// ワードアクセス時
			if (eExtRamAccessSize == MWORD_ACCESS) {
				madrReadStartAddr = madrTempStartAddr;
				dwModifyStartIndex = 0;	// 上書きは1バイト目から
				dwModifyLength = 2;
			}
			// ロングワードアクセス時
			else {
				madrReadStartAddr = madrTempStartAddr;
				dwModifyStartIndex = 0;	// 上書きは1バイト目から
				dwModifyLength = 4;
			}
			dwAccessCount = 1;		// PROT_MCU_DUMPの引数として渡すレングスはバイト数ではなくアクセス回数
			// リードモディファイ用のデータをダンプ
			// (バイト並びでデータがほしいのでPROT_MCU_DUMPを直接呼出し) => 正しいエンディアン情報必要 2010.8.23
			ferr = GetEndianType2(madrReadStartAddr, &byEndian);			// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
// RevRxNo121022-001 Append Start
			// 端数処理の為、ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないので、
			// BFWMCUCmd_DUMPコマンドのリードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を
			// 呼び出す必要はない。
// RevRxNo121022-001 Append End
			ferr = PROT_MCU_DUMP(1, &madrReadStartAddr, FALSE, &eExtRamAccessSize, 
						FALSE, &dwAccessCount, &byReadBuf[0], byEndian);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			// ライトデータを上書き
			for (dwReadCnt = dwModifyStartIndex; dwReadCnt < dwModifyLength; dwReadCnt++) {
				byReadBuf[dwReadCnt] = *(pbyBuff);
				if ((madrTempStartAddr + dwReadCnt) == madrEndAddr) {		// 終了アドレスなら、
					bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
				}
				pbyBuff += 1;								// バッファアドレスを更新。
				if (bWriteEndFlag == TRUE) {
					break;									// ライトデータがない事になるので抜ける
				}
			}
			ferr = MemWrite(madrReadStartAddr, (madrReadStartAddr + (dwModifyLength - 1)), eVerify, eExtRamAccessSize, &byReadBuf[0], pVerifyErr);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
		}
		// それ以外の場合( ライトしなければならないデータは2バイトのみ )
		else {
			if (eAccessSize == MBYTE_ACCESS) {	// バイトアクセスの場合
				ferr = MemWrite(madrTempStartAddr, madrEndAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
					bWriteEndFlag = TRUE;				// Write 終了フラッグ設定(終了)。
				}
			} else {	// ワードアクセス、ロングアクセスの場合
				ferr = MemWrite(madrTempStartAddr, madrEndAddr, eVerify, MWORD_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if ((madrTempStartAddr + 1) == madrEndAddr) {	// 終了アドレスなら、
					bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
				}
			}
		}
	}

	if ((bWriteEndFlag == FALSE) && ((madrEndAddr % 4) == 2)) {	// 先頭アドレス 4 で割った余りが 2 の場合
		// 外部RAM領域判定
		// V.1.02 RevNo110510-003 Modify Line
		GetExtAccessSize(madrTempStartAddr, madrEndAddr, &eExtRamAccessSize, &bExtRamArea);
		// 外部RAM領域、DWNPコマンド発行時、バイトアクセス以外
		if (bExtRamArea == TRUE && GetDwnpOpenData() == TRUE && eExtRamAccessSize != MBYTE_ACCESS) {
			// ワードアクセス時
			if (eExtRamAccessSize == MWORD_ACCESS) {
				madrReadStartAddr = madrTempStartAddr;
				dwModifyStartIndex = 0;	// 上書きは1バイト目から
				dwModifyLength = 4;
				dwAccessCount = 2;		// PROT_MCU_DUMPの引数として渡すレングスはバイト数ではなくアクセス回数
			}
			// ロングワードアクセス時
			else {
				madrReadStartAddr = madrTempStartAddr;
				dwModifyStartIndex = 0;	// 上書きは1バイト目から
				dwModifyLength = 4;
				dwAccessCount = 1;		// PROT_MCU_DUMPの引数として渡すレングスはバイト数ではなくアクセス回数
			}
			// リードモディファイ用のデータをダンプ
			// (バイト並びでデータがほしいのでPROT_MCU_DUMPを直接呼出し) => 正しいエンディアン情報必要 2010.8.23
			ferr = GetEndianType2(madrReadStartAddr, &byEndian);			// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
// RevRxNo121022-001 Append Start
			// 端数処理の為、ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないので、
			// BFWMCUCmd_DUMPコマンドのリードバイト数最大値でアクセスバイト数を分割してPROT_MCU_DUMP()関数を
			// 呼び出す必要はない。
// RevRxNo121022-001 Append End
			ferr = PROT_MCU_DUMP(1, &madrReadStartAddr, FALSE, &eExtRamAccessSize, 
						FALSE, &dwAccessCount, &byReadBuf[0], byEndian);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			// ライトデータを上書き
			for (dwReadCnt = dwModifyStartIndex; dwReadCnt < dwModifyLength; dwReadCnt++) {
				byReadBuf[dwReadCnt] = *(pbyBuff);
				if ((madrTempStartAddr + dwReadCnt) == madrEndAddr) {		// 終了アドレスなら、
					bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
				}
				pbyBuff += 1;								// バッファアドレスを更新。
				if (bWriteEndFlag == TRUE) {
					break;									// ライトデータがない事になるので抜ける
				}
			}
			ferr = MemWrite(madrReadStartAddr, (madrReadStartAddr + (dwModifyLength - 1)), eVerify, eExtRamAccessSize, &byReadBuf[0], pVerifyErr);
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				delete [] pbyBuffOrg;					// メモリ開放
				return ferr;
			}
		}
		// それ以外の場合( ライトしなければならないデータは3バイト )
		else {
			if (eAccessSize == MBYTE_ACCESS) {	// バイトアクセスの場合
				ferr = MemWrite(madrTempStartAddr, madrEndAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
					bWriteEndFlag = TRUE;				// Write 終了フラッグ設定(終了)。
				}
			} else {	// ワードアクセス、ロングアクセスの場合
				// まず2バイト分をワードでライト
				ferr = MemWrite(madrTempStartAddr, madrEndAddr-1, eVerify, MWORD_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				madrTempStartAddr += 2;
				pbyBuff += 2;			// バッファアドレスを更新。

				// 残り1バイトをバイトでライト
				ferr = MemWrite(madrTempStartAddr, madrEndAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト WRITE 処理実行。
				if (ferr != FFWERR_OK) {
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
				if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
					ferrEnd = ProtEnd();
					delete [] pbyBuffOrg;					// メモリ開放
					return ferr;
				}
			}
		}
	}	// 最大でもここまでである。

	delete [] pbyBuffOrg;					// メモリ開放
	
	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	return ferr;
}

//=============================================================================
/**
 * メモリコピー
 * @param madrSrcStartAddr 転送元開始アドレス
 * @param madrSrcEndAddr 転送元終了アドレス
 * @param madrDisStartAddr 転送先開始アドレス
 * @param madrDisEndAddr 転送先終了アドレス
 * @param eVerify ベリファイ有無
 * @param eAccessSize アクセスサイズ
 * @param pVerifyErr ベリファイ結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MOVE(MADDR madrSrcStartAddr, MADDR madrSrcEndAddr, MADDR madrDisStartAddr, MADDR madrDisEndAddr, 
			   enum FFWENM_VERIFY_SET eVerify, enum FFWENM_MACCESS_SIZE eAccessSize, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	BYTE*	pbyRwBuff;
	BYTE*	pbyRwBuffEsc;
	DWORD	dwRwTotalLen = 0;

	ProtInit();

	dwRwTotalLen = madrSrcEndAddr - madrSrcStartAddr + 1;

	// 確保ポインタと操作ポインタは別
	pbyRwBuffEsc = (BYTE*)malloc(dwRwTotalLen + 0xA);	// アドレス補正により前後+0x3Byte,オフセット操作用で+0xAしておく
	if (pbyRwBuffEsc == NULL) {
		ferrEnd = ProtEnd();
		// RevRxNo130730-009 Modify Line( エラーコードをFFWERR_FFW_ARG→FFWERR_MEMORY_NOT_SECUREDに変更 )
		return FFWERR_MEMORY_NOT_SECURED;				// メモリ確保失敗
	}
	pbyRwBuff = pbyRwBuffEsc;
	memset(pbyRwBuff, 0, dwRwTotalLen + 0xA);			// アドレス補正により前後+0x3Byte,オフセット操作用で+0xAしておく

	// DUMP処理
	ferr = DO_DUMP(madrSrcStartAddr, madrSrcEndAddr, eAccessSize, pbyRwBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		free(pbyRwBuffEsc);
		return ferr;
	}

	// WRITE処理
	ferr = DO_WRITE(madrDisStartAddr, madrDisEndAddr, eVerify, eAccessSize, pbyRwBuff, pVerifyErr);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		free(pbyRwBuffEsc);
		return ferr;
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	free(pbyRwBuffEsc);

	return ferr;

}


//=============================================================================
/**
 * メモリ比較
 * @param madrSrcStartAddr 比較元開始アドレス
 * @param madrSrcEndAddr 比較元終了アドレス
 * @param madrDisStartAddr 比較先開始アドレス
 * @param eAccessSize アクセスサイズ
 * @param dwCompDataSize 比較単位サイズ
 * @param pCompError 比較結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MCMP(MADDR madrSrcStartAddr, MADDR madrSrcEndAddr, MADDR madrDisStartAddr, 
			   enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwCompDataSize, 
			   FFW_COMPERR_DATA* pCompError)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	BYTE*	pbySrcBuff;
	BYTE*	pbySrcBuffEsc;
	BYTE*	pbyDisBuff;
	BYTE*	pbyDisBuffEsc;
	DWORD	dwCmpLen = 0;
	MADDR	madrDisEndAddr;
	int		nResult = 0;
	DWORD	dwChkSize;
	DWORD	dwErrAddrCnt = 0;		// エラー発生時のアドレス算出用カウンタ
	DWORD	dwLoopCnt = 0;

	ProtInit();

	// 比較先の終了アドレスを先に算出
	madrDisEndAddr = madrDisStartAddr + (madrSrcEndAddr - madrSrcStartAddr);

	// 確保ポインタと操作ポインタは別(比較元用)
	dwCmpLen = madrSrcEndAddr - madrSrcStartAddr + 1;
	pbySrcBuffEsc = (BYTE*)malloc(dwCmpLen + 0xA);	// アドレス補正により前後+0x3Byte,オフセット操作用で+0xAしておく
	if (pbySrcBuffEsc == NULL) {
		ferrEnd = ProtEnd();
		// RevRxNo130730-009 Modify Line( エラーコードをFFWERR_FFW_ARG→FFWERR_MEMORY_NOT_SECUREDに変更 )
		return FFWERR_MEMORY_NOT_SECURED;				// メモリ確保失敗
	}
	pbySrcBuff = pbySrcBuffEsc;
	memset(pbySrcBuff, 0, dwCmpLen + 0xA);			// アドレス補正により前後+0x3Byte,オフセット操作用で+0xAしておく

	// 比較元のDUMP処理
	ferr = DO_DUMP(madrSrcStartAddr, madrSrcEndAddr, eAccessSize, pbySrcBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		free(pbySrcBuffEsc);
		return ferr;
	}

	// 確保ポインタと操作ポインタは別(比較先用)
	dwCmpLen = madrDisEndAddr - madrDisStartAddr + 1;
	pbyDisBuffEsc = (BYTE*)malloc(dwCmpLen + dwCompDataSize + 0xA);	// アドレス補正により前後+0x3Byte,オフセット操作用で+0xAしておく
	if (pbyDisBuffEsc == NULL) {
		ferrEnd = ProtEnd();	// RevRxNo130730-008 Append Line
		free(pbySrcBuffEsc);	// RevRxNo130730-008 Append Line
		// RevRxNo130730-009 Modify Line( エラーコードをFFWERR_FFW_ARG→FFWERR_MEMORY_NOT_SECUREDに変更 )
		return FFWERR_MEMORY_NOT_SECURED;				// メモリ確保失敗
	}
	pbyDisBuff = pbyDisBuffEsc;
	memset(pbyDisBuff, 0, dwCmpLen + dwCompDataSize + 0xA);			// アドレス補正により前後+0x3Byte,オフセット操作用で+0xAしておく

	// 比較先のDUMP処理
	ferr = DO_DUMP(madrDisStartAddr, madrDisEndAddr, eAccessSize, pbyDisBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		free(pbySrcBuffEsc);
		free(pbyDisBuffEsc);
		return ferr;
	}

	// 比較対象領域、DUMP結果チェックサイズを比較元の補正前に戻す
	dwCmpLen = madrSrcEndAddr - madrSrcStartAddr + 1;
	dwChkSize = madrSrcEndAddr - madrSrcStartAddr + 1;

	// 取得したDUMPデータ同士を比較
	while (dwCmpLen > dwLoopCnt) {				// 比較範囲を超えたら終了
		// 端数が出た場合は端数で比較
		if (dwChkSize < dwCompDataSize) {
			nResult = memcmp(pbySrcBuff, pbyDisBuff, dwChkSize);
			dwLoopCnt += dwChkSize;				// ループカウンタを端数分プラス
		}
		// 端数が出ていない間は比較単位サイズで比較
		else {
			nResult = memcmp(pbySrcBuff, pbyDisBuff, dwCompDataSize);
			dwLoopCnt += dwCompDataSize;		// ループカウンタを比較単位サイズ分プラス
		}

		// 一致していない場合
		if (nResult != 0) {
			pCompError->eErrorFlag = COMP_ERR;													// コンペア結果NGを格納
			pCompError->dwErrorSrcAddr = madrSrcStartAddr + (dwCompDataSize * dwErrAddrCnt);	// エラー発生時の比較元アドレス
			pCompError->dwErrorDisAddr = madrDisStartAddr + (dwCompDataSize * dwErrAddrCnt);	// エラー発生時の比較先アドレス
			if (dwChkSize < dwCompDataSize) {
				memcpy(pCompError->byErrorSrcData, pbySrcBuff, dwChkSize);						// エラー発生時の比較元データ
				memcpy(pCompError->byErrorDisData, pbyDisBuff, dwChkSize);						// エラー発生時の比較先データ
			} else {
				memcpy(pCompError->byErrorSrcData, pbySrcBuff, dwCompDataSize);					// エラー発生時の比較元データ
				memcpy(pCompError->byErrorDisData, pbyDisBuff, dwCompDataSize);					// エラー発生時の比較先データ
			}
			break;	// whileループを抜ける
		}
		// 一致している場合
		else {
			pCompError->eErrorFlag = COMP_OK;	// コンペア結果OKを格納
			if (dwChkSize >= dwCompDataSize) {
				dwChkSize -= dwCompDataSize;	// チェックサイズを比較単位サイズ分マイナス
			} else {
				dwChkSize = 0;
			}
			// 比較データが残っている場合
			if (dwChkSize > 0) {
				pbySrcBuff += dwCompDataSize;	// 比較元データを比較単位サイズ分プラス
				pbyDisBuff += dwCompDataSize;	// 比較先データを比較単位サイズ分プラス
				dwErrAddrCnt++;					// エラー発生時のアドレス算出用カウンタをインクリメント
			}
		}
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	free(pbySrcBuffEsc);
	free(pbyDisBuffEsc);

	return ferr;
}


//=============================================================================
/**
 * メモリ検索
 * @param madrStartAddr 検索開始アドレス
 * @param madrEndAddr 検索終了アドレス
 * @param eSearchEqual 検索方法
 * @param eSerchForward 検索方向
 * @param eSearchPatern 検索方式
 * @param eAccessSize アクセスサイズ
 * @param dwSearchDataSize 検索データサイズ
 * @param pbySearchDataBuff 検索データ格納バッファアドレス
 * @param pbySearchMaskBuff マスクデータ格納バッファアドレス
 * @param pSearchAns 検索結果格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_MSER(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MSEARCH_EQU eSearchEqual, 
			 enum FFWENM_MSEARCH_DIR eSerchForward, enum FFWENM_MSEARCH_PATERN eSearchPatern,
			 enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwSearchDataSize,
			 const BYTE* pbySearchDataBuff, const BYTE* pbySearchMaskBuff,
			 FFW_SEARCH_DATA* pSearchAns)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	enum FFWENM_MSEARCH_PATERN eWkSearchPatern;
	BYTE*	pbySearchBuff;
	BYTE*	pbySearchBuffEsc;
	int		nSearchPos = 0;
	DWORD	dwChkSize;
	DWORD	dwCmpLen = 0;
	DWORD	i;
	BOOL	bSearchFlg = FALSE;
	BYTE*	pbyWorkSearchData;
	BYTE*	pbyWorkSearchMask;
	DWORD	dwLoopCnt = 0;

	ProtInit();

	// 検索結果を初期化
	pSearchAns->eSerchFlag = NON_MATCH;
	pSearchAns->dwMacthAddr = 0;

	// 検索方式を再設定
	if (eSearchEqual == NOTEQU_SEARCH) {		// 不一致検索時
		// 検索データサイズが2Byte以上で検索方式が1アドレス単位の場合
		if (dwSearchDataSize > 1 && eSearchPatern == PATERN_SEARCH) {
			eWkSearchPatern = DATASIZE_SEARCH;	// 常に検索データサイズ単位を設定
		} else {
			eWkSearchPatern = eSearchPatern;	// 指定された検索方式を設定
		}
	} else {									// 一致検索時
		eWkSearchPatern = eSearchPatern;		// 指定された検索方式を設定
	}

	// 検索対象領域、データチェックサイズを算出
	dwCmpLen = madrEndAddr - madrStartAddr + 1;
	dwChkSize = madrEndAddr - madrStartAddr + 1;

	// 確保ポインタと操作ポインタは別
	pbySearchBuffEsc = (BYTE*)malloc(dwCmpLen + 0xA);	// アドレス補正により前後+0x3Byte,オフセット操作用で+0xAしておく
	if (pbySearchBuffEsc == NULL) {
		ferrEnd = ProtEnd();
		// RevRxNo130730-009 Modify Line( エラーコードをFFWERR_FFW_ARG→FFWERR_MEMORY_NOT_SECUREDに変更 )
		return FFWERR_MEMORY_NOT_SECURED;				// メモリ確保失敗
	}
	pbySearchBuff = pbySearchBuffEsc;
	memset(pbySearchBuff, 0, dwCmpLen + 0xA);			// アドレス補正により前後+0x3Byte,オフセット操作用で+0xAしておく

	// 検索対象領域をDUMP
	ferr = DO_DUMP(madrStartAddr, madrEndAddr, eAccessSize, pbySearchBuff);
	if (ferr != FFWERR_OK) {
		ferrEnd = ProtEnd();
		free(pbySearchBuffEsc);
		return ferr;
	}

	pbyWorkSearchData = (BYTE*)pbySearchDataBuff;
	pbyWorkSearchMask = (BYTE*)pbySearchMaskBuff;

	// 検索方向が終了アドレスからの場合
	if (eSerchForward == BACK_SEARCH) {
		// 検索開始位置を終了アドレス+1から検索データサイズ分前に移動した位置に合わせる
		//    ↓開始アドレス       ↓終了アドレス
		// --+--+--+--+--+--+--+--+--+--
		// ～|XX|XX|XX|XX|XX|XX|XX|XX|～
		// --+--+--+--+--+--+--+--+--+--
		//                ｜    ↑ ↑
		//                ｜    ｜ 検索データサイズが1の場合
		//                ｜    検索データサイズが2の場合
		//                検索データサイズが4の場合
		nSearchPos = dwCmpLen - dwSearchDataSize;
	}

	while (dwCmpLen > dwLoopCnt) {				// 検索範囲を超えたら終了
		switch (eSearchEqual) {
		case EQU_SEARCH:						// 一致検索
			if (dwChkSize < dwSearchDataSize) {	// データチェックサイズに端数が出た場合、不一致と扱う
				bSearchFlg = FALSE;
			} else {							// データチェックサイズに端数が出ていない場合
				for (i = 0; i < dwSearchDataSize; i++) {
					// マスク後のデータ比較で一致しない場合
					if ((pbySearchBuff[i + nSearchPos] & pbyWorkSearchMask[i]) != 
						(pbyWorkSearchData[i] & pbyWorkSearchMask[i])) {
						bSearchFlg = FALSE;
						break;
					}
				}
				// マスク後のデータ比較で一致した場合
				if (i == dwSearchDataSize) {
					pSearchAns->eSerchFlag = MATCH;
					pSearchAns->dwMacthAddr = madrStartAddr + nSearchPos;
					bSearchFlg = TRUE;
				}
			}
			break;
		case NOTEQU_SEARCH:						// 不一致検索
			if (dwChkSize < dwSearchDataSize) {	// データチェックサイズに端数が出た場合
				pSearchAns->eSerchFlag = MATCH;
				pSearchAns->dwMacthAddr = madrStartAddr + nSearchPos;
				bSearchFlg = TRUE;
			} else {							// データチェックサイズに端数が出ていない場合
				for (i = 0; i < dwSearchDataSize; i++) {
					// マスク後のデータ比較で一致しない場合
					if ((pbySearchBuff[i + nSearchPos] & pbyWorkSearchMask[i]) != 
						(pbyWorkSearchData[i] & pbyWorkSearchMask[i])) {
						pSearchAns->eSerchFlag = MATCH;
						pSearchAns->dwMacthAddr = madrStartAddr + nSearchPos;
						bSearchFlg = TRUE;
						break;
					}
				}
			}
			break;
		}

		// 検索に成功した場合、whileループを抜ける
		if (bSearchFlg == TRUE) {
			break;
		}

		// 検索方式が1アドレス単位の場合
		if (eWkSearchPatern == PATERN_SEARCH) {
			// 検索開始位置を1アドレス分移動
			if (eSerchForward == BACK_SEARCH) {
				nSearchPos--;						// 終了アドレスからの場合、前に移動
			} else {
				nSearchPos++;						// 開始アドレスからの場合、後ろに移動
			}
			// データチェックサイズをデクリメント
			dwChkSize--;
			dwLoopCnt++;							// ループカウンタをインクリメント
		}
		// 検索方式が検索データサイズ単位の場合
		else {
			// データチェックサイズを検索データサイズ分マイナス
			dwChkSize -= dwSearchDataSize;
			dwLoopCnt += dwSearchDataSize;				// ループカウンタを検索データサイズ分プラス
			if (dwChkSize < dwSearchDataSize) {			// データチェックサイズに端数が出た場合
				// 検索開始位置を検索データサイズ分移動
				if (eSerchForward == BACK_SEARCH) {
					nSearchPos -= dwChkSize;				// 終了アドレスからの場合、前に移動
				} else {
					nSearchPos += dwSearchDataSize;		// 開始アドレスからの場合、後ろに移動
				}
			} else {									// データチェックサイズに端数が出ていない場合
				// 検索開始位置を検索データサイズ分移動
				if (eSerchForward == BACK_SEARCH) {
					nSearchPos -= dwSearchDataSize;		// 終了アドレスからの場合、前に移動
				} else {
					nSearchPos += dwSearchDataSize;		// 開始アドレスからの場合、後ろに移動
				}
			}
		}
	}

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}

	free(pbySearchBuffEsc);

	return ferr;
}


//=============================================================================
/**
 * ターゲットメモリ内容を取得する。(命令コード順に並び替え)
 * @param dwmadrStartAddr 開始アドレス
 * @param dwmadrEndAddr   終了アドレス
 * @param pbyReadBuff     リードデータ格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_CDUMP(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, const BYTE* pbyReadBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	BYTE*	pbyBuff;
	BYTE*	pbyReadBufEsc;
	BYTE*	pbyRetBuff;					// Targetへの返却用
	DWORD	dwTotalLength;				// CDUMP全体のバイトサイズ
	BOOL	bDumpEndFlag = FALSE;		// Dump 終了フラグ(FALSE:終了でない、TRUE:終了)
	MADDR	madrStartAddrRemainder;		// 先頭アドレスを 4 で割った余り
	MADDR	madrEndAddrRemainder;		// 最終アドレスを 4 で割った余り

	MADDR	madrTempStartAddr;			// そのときの先頭アドレス
	MADDR	madrTempEndAddr;			// そのときの最終アドレス
	enum FFWENM_MACCESS_SIZE eAccessSize;
	BYTE	byEndian;
	MADDR	madrOffsetStartAddr;		// そのときの先頭アドレス(補正後)
	
	eAccessSize = MLWORD_ACCESS;	// ロングワードアクセスするようにする。

	ProtInit();

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start

	// 確保ポインタと操作ポインタは別
	// Target側から渡された領域を破壊しないよう3Byte余分にメモリ確保しておく
	dwTotalLength = dwmadrEndAddr - dwmadrStartAddr + 1;	// リードバイト数算出
	pbyReadBufEsc = (BYTE*)malloc(dwTotalLength + 0x3);
	if (pbyReadBufEsc == NULL) {
		ferrEnd = ProtEnd();
		// RevRxNo130730-009 Modify Line( エラーコードをFFWERR_FFW_ARG→FFWERR_MEMORY_NOT_SECUREDに変更 )
		return FFWERR_MEMORY_NOT_SECURED;				// メモリ確保失敗
	}
	pbyBuff = pbyReadBufEsc;
	memset(pbyBuff, 0, dwTotalLength + 0x3);

	madrStartAddrRemainder = dwmadrStartAddr % 4;	// 先頭余り。
	madrEndAddrRemainder   = dwmadrEndAddr % 4;		// 最終余り。

	madrTempStartAddr = dwmadrStartAddr;			// dwmadrStartAddr を初期値。
	madrTempEndAddr   = dwmadrEndAddr;				// dwmadrEndAddr   を初期値。
	
	// 先頭余り部の Dump
	if ((madrTempStartAddr % 4) == 1) {	// 先頭アドレス 4 で割った余りが 1 の場合
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
			OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);					// RevNo111121-003 Modify Line
			ferr = MemCodeDump(madrOffsetStartAddr, madrOffsetStartAddr, MBYTE_ACCESS, pbyBuff);	// 1 バイト CDUMP 処理実行。
		} else {
			ferr = MemCodeDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト CDUMP 処理実行。
		}
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (madrTempStartAddr == dwmadrEndAddr) {	// 終了アドレスなら、
			bDumpEndFlag = TRUE;					// Dump 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	if ((bDumpEndFlag == FALSE) && ((madrTempStartAddr % 4) == 2)) {					// 先頭アドレス 4 で割った余りが 2 の場合
		if ((eAccessSize == MBYTE_ACCESS) || (madrTempStartAddr == dwmadrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
			ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
				OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);					// 開始アドレス補正		// RevNo111121-003 Modify Line
				ferr = MemCodeDump(madrOffsetStartAddr, madrOffsetStartAddr, MBYTE_ACCESS, pbyBuff);	// 1 バイト CDUMP 処理実行。
			} else {
				ferr = MemCodeDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト CDUMP 処理実行。
			}
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if (madrTempStartAddr == dwmadrEndAddr) {	// 終了アドレスなら、
				bDumpEndFlag = TRUE;					// Dump 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
			pbyBuff += 1;			// バッファアドレスを更新。
		} else {					// ワードアクセス以上かつ２バイト以上ある場合
			ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
				OffsetStartAddr(MWORD_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);						// 開始アドレス補正			// RevNo111121-003 Modify Line
				ferr = MemCodeDump(madrOffsetStartAddr, madrOffsetStartAddr + 1, MWORD_ACCESS, pbyBuff);	// 1 ワード CDUMP 処理実行。
				ReplaceEndian(pbyBuff, MWORD_ACCESS, sizeof(WORD));	// ビッグ時は並び替えが必要
			} else {
				ferr = MemCodeDump(madrTempStartAddr, madrTempStartAddr + 1, MWORD_ACCESS, pbyBuff);		// 1 ワード CDUMP 処理実行。
			}
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if ((madrTempStartAddr + 1) == dwmadrEndAddr) {	// 終了アドレスなら、
				bDumpEndFlag = TRUE;						// Dump 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 2;	// そのときの先頭アドレスを更新。
			pbyBuff += 2;			// バッファアドレスを更新。
		}
	}
	if ((bDumpEndFlag == FALSE) && ((madrTempStartAddr % 4) == 3)) {	// 先頭アドレス 4 で割った余りが 3 の場合
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
			OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);					// 開始アドレス補正			// RevNo111121-003 Modify Line
			ferr = MemCodeDump(madrOffsetStartAddr, madrOffsetStartAddr, MBYTE_ACCESS, pbyBuff);	// 1 バイト CDUMP 処理実行。
		} else {
			ferr = MemCodeDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト CDUMP 処理実行。
		}
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (madrTempStartAddr == dwmadrEndAddr) {	// 終了アドレスなら、
			bDumpEndFlag = TRUE;					// Dump 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	// 先頭余り部があるとき終わり。
	// Dump 終了でなければ、残りの先頭アドレスは 4 で割り切れる状態になっている。

	// 本体部の CDump。
	if ((bDumpEndFlag == FALSE) && 											// Dump 終了フラッグが「終了でない」、かつ
				((dwmadrEndAddr - madrTempStartAddr) >= 3L )) {				// 本体部先頭から End アドレスまで 3 以上ある場合(すなわち本体部がある場合)
		if (madrEndAddrRemainder != 3) {									// 最終アドレス余りが 3 でない場合
			madrTempEndAddr = dwmadrEndAddr - madrEndAddrRemainder - 1;		// 本体部の最終アドレス。余り分と 1 を引く。
		} else {															// 最終アドレス余りが 3 の場合
			madrTempEndAddr = dwmadrEndAddr;								// 本体部の最終アドレス。dwmadrEndAddr のまま。
			bDumpEndFlag = TRUE;	// Dump 終了フラッグ設定(終了)。
		}
		ferr = MemCodeDump(madrTempStartAddr, madrTempEndAddr, eAccessSize, pbyBuff);	// 本体部の CDUMP 処理実行。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は並び替えが必要
			ReplaceEndian(pbyBuff, eAccessSize, (madrTempEndAddr - madrTempStartAddr + 1));
		}
		pbyBuff = pbyBuff + (madrTempEndAddr - madrTempStartAddr + 1);	// pbyBuff 更新。
		madrTempStartAddr = madrTempEndAddr + 1;						// 最終余り部の先頭アドレス設定
	}

	// 最終余り部の CDump
	if (bDumpEndFlag == FALSE) {	// Dump 終了フラッグが「終了でない」場合。（先頭アドレス 4 で割った余りは 0 のはずである。）
		if ((eAccessSize == MBYTE_ACCESS) || (madrTempStartAddr == dwmadrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
			ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
				OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);					// 開始アドレス補正			// RevNo111121-003 Modify Line
				ferr = MemCodeDump(madrOffsetStartAddr, madrOffsetStartAddr, MBYTE_ACCESS, pbyBuff);	// 1 バイト CDUMP 処理実行。
			} else {
				ferr = MemCodeDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト CDUMP 処理実行。
			}
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if (madrTempStartAddr == dwmadrEndAddr) {	// 終了アドレスなら、
				bDumpEndFlag = TRUE;					// Dump 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
			pbyBuff += 1;			// バッファアドレスを更新。
		} else {					// ワードアクセス以上かつ２バイト以上ある場合
			ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
				OffsetStartAddr(MWORD_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);						// 開始アドレス補正			// RevNo111121-003 Modify Line
				ferr = MemCodeDump(madrOffsetStartAddr, madrOffsetStartAddr + 1, MWORD_ACCESS, pbyBuff);	// 1 ワード CDUMP 処理実行。
				ReplaceEndian(pbyBuff, MWORD_ACCESS, sizeof(WORD));	// ビッグ時は並び替えが必要
			} else {
				ferr = MemCodeDump(madrTempStartAddr, madrTempStartAddr + 1, MWORD_ACCESS, pbyBuff);		// 1 ワード CDUMP 処理実行。
			}
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				free(pbyReadBufEsc);
				return ferr;
			}
			if ((madrTempStartAddr + 1) == dwmadrEndAddr) {	// 終了アドレスなら、
				bDumpEndFlag = TRUE;						// Dump 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 2;	// そのときの先頭アドレスを更新。
			pbyBuff += 2;			// バッファアドレスを更新。
		}
	}
	if ((bDumpEndFlag == FALSE) && ((madrTempStartAddr % 4) == 1)) {	// 先頭アドレス 4 で割った余りが 1 の場合
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
			OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);						// 開始アドレス補正			// RevNo111121-003 Modify Line
			ferr = MemCodeDump(madrOffsetStartAddr, madrOffsetStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト CDUMP 処理実行。
		} else {
			ferr = MemCodeDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);			// 1 バイト CDUMP 処理実行。
		}
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (madrTempStartAddr == dwmadrEndAddr) {	// 終了アドレスなら、
			bDumpEndFlag = TRUE;					// Dump 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	if ((bDumpEndFlag == FALSE) && ((madrTempStartAddr % 4) == 2)) {	// 先頭アドレス 4 で割った余りが 2 の場合
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
			OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);					// 開始アドレス補正			// RevNo111121-003 Modify Line
			ferr = MemCodeDump(madrOffsetStartAddr, madrOffsetStartAddr, MBYTE_ACCESS, pbyBuff);	// 1 バイト CDUMP 処理実行。
		} else {
			ferr = MemCodeDump(madrTempStartAddr, madrTempStartAddr, MBYTE_ACCESS, pbyBuff);		// 1 バイト CDUMP 処理実行。
		}
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			free(pbyReadBufEsc);
			return ferr;
		}
	}	// 最大でもここまでである。

	pbyBuff = pbyReadBufEsc;
	pbyRetBuff = (BYTE*)pbyReadBuff;
	memcpy(pbyRetBuff, pbyBuff, dwTotalLength);

	free(pbyReadBufEsc);

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	return ferr;
}

//=============================================================================
/**
 * ターゲットメモリ内容を変更する。(命令コード順に並び替え)
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr 終了アドレス
 * @param eVerify ベリファイ有無
 * @param pbyWriteBuff ライトデータ格納用バッファアドレス
 * @param pVerifyErr ベリファイ結果格納用バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR DO_CWRITE(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
				 const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr)
{
	FFWERR	ferr = FFWERR_OK;
	FFWERR	ferrEnd;
	BOOL	bWriteEndFlag = FALSE;		// Write 終了フラグ(FALSE:終了でない、TRUE:終了)
	MADDR	madrEndAddrRemainder;		// 最終アドレスを 4 で割った余り
	MADDR	madrTempStartAddr;			// そのときの先頭アドレス
	MADDR	madrTempEndAddr;			// そのときの最終アドレス
	BYTE	*pbyBuff;					// 内部処理の変数名を WRITE と合わせるため。
	enum FFWENM_MACCESS_SIZE eAccessSize;
	BYTE	byEndian;
	MADDR	madrOffsetStartAddr;		// そのときの先頭アドレス(補正後)

	eAccessSize = MLWORD_ACCESS;		// ロングワードアクセスするようにする。

	ProtInit();

	pbyBuff = (BYTE*)pbyWriteBuff;

// V.1.02 No.17 内蔵ROMダウンロード時のPCメモリ使用量削減対応 Append Start

	madrEndAddrRemainder = madrEndAddr % 4;	// 最終余り。

	madrTempStartAddr = madrStartAddr;		// madrStartAddr を初期値。
	madrTempEndAddr   = madrEndAddr;		// madrEndAddr   を初期値。

	// 先頭余り部の CWrite
	if ((madrTempStartAddr % 4) == 1) {		// 先頭アドレス 4 で割った余りが 1 の場合
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
			OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);										// RevNo111121-003 Modify Line
			ferr = MemCodeWrite(madrOffsetStartAddr, madrOffsetStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);	// 1 バイト CWRITE 処理実行。
		} else {
			ferr = MemCodeWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト CWRITE 処理実行。
		}
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
			bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	if ((bWriteEndFlag == FALSE) && ((madrTempStartAddr % 4) == 2)) {				// 先頭アドレス 4 で割った余りが 2 の場合
		if ((eAccessSize == MBYTE_ACCESS) || (madrTempStartAddr == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
			ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
				OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);										// RevNo111121-003 Modify Line
				ferr = MemCodeWrite(madrOffsetStartAddr, madrOffsetStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);	// 1 バイト CWRITE 処理実行。
			} else {
				ferr = MemCodeWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト CWRITE 処理実行。
			}
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
				bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
			pbyBuff += 1;			// バッファアドレスを更新。
		} else {	// ワードアクセス以上かつ２バイト以上ある場合
			ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正と並び替えが必要
				OffsetStartAddr(MWORD_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);							// RevNo111121-003 Modify Line
				if (IsNotMcuRomArea(madrOffsetStartAddr, madrOffsetStartAddr + 1) == FALSE) {
					ReplaceEndian(pbyBuff, MWORD_ACCESS, sizeof(WORD));		// ROM領域でバイトアクセス以外は並び替え
				}
				ferr = MemCodeWrite(madrOffsetStartAddr, madrOffsetStartAddr + 1, eVerify, MWORD_ACCESS, pbyBuff, pVerifyErr);	// 1 バイト CWRITE 処理実行。
			} else {
				ferr = MemCodeWrite(madrTempStartAddr, madrTempStartAddr + 1, eVerify, MWORD_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト CWRITE 処理実行。
			}
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				return ferr;
			}
			if ((madrTempStartAddr + 1) == madrEndAddr) {	// 終了アドレスなら、
				bWriteEndFlag = TRUE;						// Write 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 2;	// そのときの先頭アドレスを更新。
			pbyBuff += 2;			// バッファアドレスを更新。
		}
	}
	if ((bWriteEndFlag == FALSE) && ((madrTempStartAddr % 4) == 3)) {	// 先頭アドレス 4 で割った余りが 3 の場合
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
			OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);										// RevNo111121-003 Modify Line
			ferr = MemCodeWrite(madrOffsetStartAddr, madrOffsetStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);	// 1 バイト CWRITE 処理実行。
		} else {
			ferr = MemCodeWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト CWRITE 処理実行。
		}
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
			bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	// 先頭余り部があるとき終わり。
	// CWrite 終了でなければ、残りの先頭アドレスは 4 で割り切れる状態になっている。

	// 本体部の CWrite。
	if ((bWriteEndFlag == FALSE) && 									// Write 終了フラッグが「終了でない」、かつ
				((madrEndAddr - madrTempStartAddr) >= 3L )) {			// 本体部先頭から End アドレスまで 3 以上ある場合(すなわち本体部がある場合)
		if (madrEndAddrRemainder != 3) {								// 最終アドレス余りが 3 でない場合
			madrTempEndAddr = madrEndAddr - madrEndAddrRemainder - 1;	// 本体部の最終アドレス。余り分と 1 を引く。
		} else {														// 最終アドレス余りが 3 の場合
			madrTempEndAddr = madrEndAddr;								// 本体部の最終アドレス。madrEndAddr のまま。
			bWriteEndFlag = TRUE;	// Write 終了フラッグ設定(終了)。
		}
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);					// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {								// ビッグ、ROM領域、バイトアクセス以外は並び替えが必要
			if (IsNotMcuRomArea(madrTempStartAddr, madrTempEndAddr) == FALSE) {
				ReplaceEndian(pbyBuff, MLWORD_ACCESS, (madrTempEndAddr - madrTempStartAddr + 1));
			}
		}
		ferr = MemCodeWrite(madrTempStartAddr, madrTempEndAddr, eVerify, MLWORD_ACCESS, pbyBuff, pVerifyErr);	// 本体部の CWRITE 処理実行。
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
			ferrEnd = ProtEnd();
			return ferr;
		}
		pbyBuff = pbyBuff + (madrTempEndAddr - madrTempStartAddr + 1);	// pbyBuff 更新。
		madrTempStartAddr = madrTempEndAddr + 1;						// 最終余り部の先頭アドレス設定
	}

	// 最終余り部の CWrite
	if (bWriteEndFlag == FALSE) {	// Write 終了フラッグが「終了でない」場合。（先頭アドレス 4 で割った余りは 0 のはずである。）
		if ((eAccessSize == MBYTE_ACCESS) || (madrTempStartAddr == madrEndAddr)) {	// バイトアクセスまたは領域は１バイトのみ場合
			ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
				OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);										// RevNo111121-003 Modify Line
				ferr = MemCodeWrite(madrOffsetStartAddr, madrOffsetStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);	// 1 バイト CWRITE 処理実行。
			} else {
				ferr = MemCodeWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト CWRITE 処理実行。
			}
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (madrTempStartAddr == madrEndAddr) {		// 終了アドレスなら、
				bWriteEndFlag = TRUE;					// Write 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
			pbyBuff += 1;			// バッファアドレスを更新。
		} else {					// ワードアクセス以上かつ２バイト以上ある場合
			ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正と並び替えが必要
				OffsetStartAddr(MWORD_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);											// RevNo111121-003 Modify Line
				if (IsNotMcuRomArea(madrOffsetStartAddr, madrOffsetStartAddr + 1) == FALSE) {
					ReplaceEndian(pbyBuff, MWORD_ACCESS, sizeof(WORD));		// ROM領域でバイトアクセス以外は並び替え
				}
				ferr = MemCodeWrite(madrOffsetStartAddr, madrOffsetStartAddr + 1, eVerify, MWORD_ACCESS, pbyBuff, pVerifyErr);	// 1 バイト CWRITE 処理実行。
			} else {
				ferr = MemCodeWrite(madrTempStartAddr, madrTempStartAddr + 1, eVerify, MWORD_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト CWRITE 処理実行。
			}
			if (ferr != FFWERR_OK) {
				ferrEnd = ProtEnd();
				return ferr;
			}
			if (pVerifyErr->eErrorFlag == VERIFY_ERR) {		// ベリファイエラー発生時、処理を終了する。
				ferrEnd = ProtEnd();
				return ferr;
			}
			if ((madrTempStartAddr + 1) == madrEndAddr) {	// 終了アドレスなら、
				bWriteEndFlag = TRUE;						// Write 終了フラッグ設定(終了)。
			}
			madrTempStartAddr += 2;	// そのときの先頭アドレスを更新。
			pbyBuff += 2;			// バッファアドレスを更新。
		}
	}
	if ((bWriteEndFlag == FALSE) && ((madrTempStartAddr % 4) == 1)) {	// 先頭アドレス 4 で割った余りが 1 の場合
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
			OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);										// RevNo111121-003 Modify Line
			ferr = MemCodeWrite(madrOffsetStartAddr, madrOffsetStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);	// 1 バイト CWRITE 処理実行。
		} else {
			ferr = MemCodeWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト CWRITE 処理実行。
		}
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (madrTempStartAddr == madrEndAddr) {	// 終了アドレスなら、
			bWriteEndFlag = TRUE;				// Write 終了フラッグ設定(終了)。
		}
		madrTempStartAddr += 1;	// そのときの先頭アドレスを更新。
		pbyBuff += 1;			// バッファアドレスを更新。
	}
	if ((bWriteEndFlag == FALSE) && ((madrTempStartAddr % 4) == 2)) {	// 先頭アドレス 4 で割った余りが 2 の場合
		ferr = GetEndianType2(madrTempStartAddr, &byEndian);	// エンディアン判定＆取得
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (byEndian == FFWRX_ENDIAN_BIG) {				// ビッグ時は開始アドレス補正が必要
			OffsetStartAddr(MBYTE_ACCESS, madrTempStartAddr, &madrOffsetStartAddr);										// RevNo111121-003 Modify Line
			ferr = MemCodeWrite(madrOffsetStartAddr, madrOffsetStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);	// 1 バイト CWRITE 処理実行。
		} else {
			ferr = MemCodeWrite(madrTempStartAddr, madrTempStartAddr, eVerify, MBYTE_ACCESS, pbyBuff, pVerifyErr);		// 1 バイト CWRITE 処理実行。
		}
		if (ferr != FFWERR_OK) {
			ferrEnd = ProtEnd();
			return ferr;
		}
		if (pVerifyErr->eErrorFlag == VERIFY_ERR) {	// ベリファイエラー発生時、処理を終了する。
			ferrEnd = ProtEnd();
			return ferr;
		}
	}	// 最大でもここまでである。

	ferrEnd = ProtEnd();
	if (ferr == FFWERR_OK) {
		ferr = ferrEnd;
	}
	return ferr;
}

//=============================================================================
/**
 * DUMPバッファをアドレス順からTargetへ渡す順に並べ替える。
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr   終了アドレス
 * @param eAccessSize   アクセスサイズ
 * @param pbyBuff       バッファのポインタ
 * @retval 				FFWエラーコード
 */
//=============================================================================
static FFWERR changeOrderDumpBuff(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, BYTE* pbyBuff)
{
	MADDR	madrAreaEnd;
	BYTE	byTempBuff[8];			// バッファ・テンポラリ
	DWORD	dwCnt;					// 0 から 7 を動かす。
	DWORD	i;
	DWORD	dwAddAddr;				// アクセスサイズのバイト数
	BOOL	bAboveSize;				// Dump 個数がアクセスサイズ以上の場合:TRUE、未満の場合:FALSE。
	DWORD	dwLengthRemainder;		// Dump 個数をアクセスサイズで割った余り
	MADDR	madrStartAddrTemp;		// 更新していくスタートアドレス
	MADDR	madrEndAddrTemp;		// アクセスサイズの余り dwLengthRemainder を引いたエンドアドレス
	DWORD	dwAreaLengthRemainder;	// madrAreaEnd までの個数をアクセスサイズで割った余り
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwLength;				// バイトサイズ
	BYTE	byEndian;
	// V.1.02 RevNo110304-001 Deleate & Append Line		Delete:BOOL	bFlashRomArea = FALSE;
	enum FFWRXENM_MAREA_TYPE	eAreaType;
	BOOL	bEnableArea = FALSE;
	
	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case MWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case MLWORD_ACCESS:
		dwAddAddr = 4;
		break;
	case MDOUBLE_ACCESS:
		dwAddAddr = 8;
		break;
	default:
		dwAddAddr = 4;
		break;
	}

	if ((madrEndAddr - madrStartAddr + 1) >= dwAddAddr) {
		bAboveSize = TRUE;	// Dump 個数がアクセスサイズ以上
	} else {
		bAboveSize = FALSE;	// Dump 個数がアクセスサイズ未満
	}

	dwLengthRemainder = (madrEndAddr - madrStartAddr + 1) % dwAddAddr;
	madrStartAddrTemp = madrStartAddr;
	madrEndAddrTemp   = madrEndAddr - dwLengthRemainder;	// 下記で madrAreaEnd が madrStartAddrTemp にアクセスサイズ倍数足したもので
															// なくても、アクセスサイズの区切りまではバッファを持たせるため。

	if (bAboveSize == TRUE) {	// Dump 個数がアクセスサイズ以上のときのみ、実施。
		madrAreaEnd = 0;
		dwCnt = 0;
		do {
			// 指定領域判定
			// V.1.02 RevNo110304-001 Modify Line
			ferr = ChkBlockArea(madrStartAddrTemp, madrEndAddrTemp, &dwLength, &eAreaType, 1, &bEnableArea);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = GetEndianType2(madrStartAddrTemp, &byEndian);					// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			madrAreaEnd = madrStartAddrTemp + dwLength - 1;
			dwAreaLengthRemainder = (madrAreaEnd - madrStartAddrTemp +1) % dwAddAddr;	// madrAreaEnd までの個数をアクセスサイズで割った余り
			if (dwAreaLengthRemainder != 0) {								// 余りが 0 でないとき
				madrAreaEnd = madrAreaEnd - dwAreaLengthRemainder + dwAddAddr;	// madrStartAddrTemp からのアクセスサイズ区切りまで更新
			}																// 余りが 0 のとき、そのまま。
			if (byEndian == FFWRX_ENDIAN_LITTLE) {							// リトルエンディアン時
				i = 0;
				do {
					for (dwCnt = 0; dwCnt < dwAddAddr; dwCnt++) {
						byTempBuff[dwCnt] = *(pbyBuff + dwCnt);				// バッファ・テンポラリに格納
					}
					for (dwCnt = 0; dwCnt < dwAddAddr; dwCnt++) {
						*pbyBuff = byTempBuff[dwAddAddr - 1 - dwCnt];		// データサイズ分ひっくり返す。
						pbyBuff++;											// バッファ更新
					}
					i = i + dwAddAddr;
				} while (i != (madrAreaEnd - madrStartAddrTemp + 1));		// (madrAreaEnd - madrStartAddrTemp + 1) がオーバーフローしても、
																			// i を足した後なので、問題なし。
			} else {														// ビッグエンディアン時
				// *pbyBuff の並べ替え無し
				pbyBuff = pbyBuff + (madrAreaEnd - madrStartAddrTemp +1);	// バッファ更新。
			}
			// 次の繰り返し用設定
			madrStartAddrTemp = madrAreaEnd + 1;
		} while (madrEndAddrTemp != madrAreaEnd);
	}
	
	return ferr;
}

//=============================================================================
/**
 * WRITEバッファをTargetから来た順からアドレス順へ並べ替える。
 * @param madrStartAddr 開始アドレス
 * @param madrEndAddr   終了アドレス
 * @param eAccessSize   アクセスサイズ
 * @param pbyWriteBuff  ライトデータ格納用バッファアドレス
 * @param pbyBuff       バッファのポインタ
 * @retval 				FFWエラーコード
 */
//=============================================================================
static FFWERR changeOrderWriteBuff(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize, 
                                  const BYTE* pbyWriteBuff, BYTE* pbyBuff)
{
	MADDR	madrAreaEnd;
	BYTE	byTempBuff[8];			// バッファ・テンポラリ
	DWORD	dwCnt;					// 0 から 7 を動かす。
	DWORD	i;
	DWORD	dwAddAddr;				// アクセスサイズのバイト数
	BOOL	bAboveSize;				// Dump 個数がアクセスサイズ以上の場合:TRUE、未満の場合:FALSE。
	DWORD	dwLengthRemainder;		// Dump 個数をアクセスサイズで割った余り
	MADDR	madrStartAddrTemp;		// 更新していくスタートアドレス
	MADDR	madrEndAddrTemp;		// アクセスサイズの余り dwLengthRemainder を引いたエンドアドレス
	DWORD	dwAreaLengthRemainder;	// madrAreaEnd までの個数をアクセスサイズで割った余り
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwLength;
	BYTE	byEndian;
	// V.1.02 RevNo110304-001 Deleate & Append Line		Delete:BOOL	bFlashRomArea = FALSE;
	enum FFWRXENM_MAREA_TYPE	eAreaType;
	BOOL	bEnableArea = FALSE;
	
	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwAddAddr = 1;
		break;
	case MWORD_ACCESS:
		dwAddAddr = 2;
		break;
	case MLWORD_ACCESS:
		dwAddAddr = 4;
		break;
	case MDOUBLE_ACCESS:
		dwAddAddr = 8;
		break;
	default:
		dwAddAddr = 4;
		break;
	}

	if ((madrEndAddr - madrStartAddr + 1) >= dwAddAddr) {
		bAboveSize = TRUE;	// Write 個数がアクセスサイズ以上
	} else {
		bAboveSize = FALSE;	// Write 個数がアクセスサイズ未満
	}

	dwLengthRemainder = (madrEndAddr - madrStartAddr + 1) % dwAddAddr;
	madrStartAddrTemp = madrStartAddr;
	madrEndAddrTemp   = madrEndAddr - dwLengthRemainder;	// 下記で madrAreaEnd が madrStartAddrTemp にアクセスサイズ倍数足したもので
															// なくても、アクセスサイズの区切りまではバッファを持たせるため。

	if (bAboveSize == TRUE) {	// Write 個数がアクセスサイズ以上のときのみ、実施。
		madrAreaEnd = 0;
		dwCnt = 0;
		do {
			// 指定領域判定
			// V.1.02 RevNo110304-001 Modify Line
			ferr = ChkBlockArea(madrStartAddrTemp, madrEndAddrTemp, &dwLength, &eAreaType, 1, &bEnableArea);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			ferr = GetEndianType2(madrStartAddrTemp, &byEndian);								// エンディアン判定＆取得
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			madrAreaEnd = madrStartAddrTemp + dwLength - 1;
			dwAreaLengthRemainder = (madrAreaEnd - madrStartAddrTemp +1) % dwAddAddr;	// madrAreaEnd までの個数をアクセスサイズで割った余り
			if (dwAreaLengthRemainder != 0) {											// 余りが 0 でないとき
				madrAreaEnd = madrAreaEnd - dwAreaLengthRemainder + dwAddAddr;			// madrStartAddrTemp からのアクセスサイズ区切りまで更新
			}																			// 余りが 0 のとき、そのまま。
			if (byEndian == FFWRX_ENDIAN_LITTLE) {										// リトルエンディアン時
				i = 0;
				do {
					for (dwCnt = 0; dwCnt < dwAddAddr; dwCnt++) {
						byTempBuff[dwCnt] = *(pbyWriteBuff + dwCnt);					// バッファ・テンポラリに格納
					}
					for (dwCnt = 0; dwCnt < dwAddAddr; dwCnt++) {
						*(pbyBuff + dwCnt) = byTempBuff[dwAddAddr - 1 - dwCnt];			// データサイズ分ひっくり返す。
					}
					pbyBuff += dwAddAddr;												// バッファ更新
					pbyWriteBuff += dwAddAddr;
					i += dwAddAddr;
				} while (i != (madrAreaEnd - madrStartAddrTemp + 1));	// (madrAreaEnd - madrStartAddrTemp + 1) がオーバーフローしても、
																		// i を足した後なので、問題なし。
			} else {													// ビッグエンディアン時
				i = 0;
				do {
					*pbyBuff = *pbyWriteBuff;							// *pbyBuff の並べ替え無し
					pbyBuff += 1;										// バッファ更新
					pbyWriteBuff += 1;
					i += 1;
				} while (i != (madrAreaEnd - madrStartAddrTemp + 1));	// (madrAreaEnd - madrStartAddrTemp + 1) がオーバーフローしても、
																		// i を足した後なので、問題なし。
			}
			// 次の繰り返し用設定
			madrStartAddrTemp = madrAreaEnd + 1;
		} while (madrEndAddrTemp != madrAreaEnd);
	}

	// Write 個数のアクセスサイズ余り部分の並べ替え
	// ※E1/E20ではTargetから渡される余り部分のライトデータはバイト並びなのでリトル時もひっくり返さない
	if (dwLengthRemainder != 0) {
		for (dwCnt = 0; dwCnt < dwLengthRemainder; dwCnt++) {		// 余り分繰り返す。
			*(pbyBuff + dwCnt) = *(pbyWriteBuff + dwCnt);			// バッファ・テンポラリに格納
		}
	}

	return ferr;
}

//=============================================================================
/**
 * 指定開始アドレスを補正する。(CodeDUMP/WRITEの端数アドレス用)
 * @param eAccessSize   アクセスサイズ
 * @param madrStartAddr 補正前の開始アドレス
 * @param pdwStartAddr  補正後の開始アドレス
 * @retval なし
 */
//=============================================================================
void OffsetStartAddr(enum FFWENM_MACCESS_SIZE eAccessSize, MADDR madrStartAddr, DWORD* pdwStartAddr)
{
	DWORD dwStart;

	dwStart = madrStartAddr & 0x0000000F;

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		if (dwStart == 0x0 || dwStart == 0x4 || dwStart == 0x8 || dwStart == 0xC) {
			(*pdwStartAddr) = madrStartAddr + 3;
		} else if (dwStart == 0x1 || dwStart == 0x5 || dwStart == 0x9 || dwStart == 0xD) {
			(*pdwStartAddr) = madrStartAddr + 1;
		} else if (dwStart == 0x2 || dwStart == 0x6 || dwStart == 0xA || dwStart == 0xE) {
			(*pdwStartAddr) = madrStartAddr - 1;
		} else if (dwStart == 0x3 || dwStart == 0x7 || dwStart == 0xB || dwStart == 0xF) {
			(*pdwStartAddr) = madrStartAddr - 3;
		}
		break;
	case MWORD_ACCESS:
		if (dwStart == 0x0 || dwStart == 0x4 || dwStart == 0x8 || dwStart == 0xC) {
			(*pdwStartAddr) = madrStartAddr + 2;
		} else if (dwStart == 0x2 || dwStart == 0x6 || dwStart == 0xA || dwStart == 0xE) {
			(*pdwStartAddr) = madrStartAddr - 2;
		}
		break;
	case MLWORD_ACCESS:
		(*pdwStartAddr) = madrStartAddr;	// ロングワードアクセス時は補正不要
		break;
	default:
		(*pdwStartAddr) = madrStartAddr;
		break;
	}

	return;
}
//=============================================================================
/**
 * メモリ操作コマンド用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdMcuData_Mem(void)
{
	return;
}