///////////////////////////////////////////////////////////////////////////////
/**
 * @file errchk.h
 * @brief エラーチェック処理のヘッダファイル
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi, K.Uemori
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/07/22
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・V.1.03 RevNo111121-008 2012/07/11 橋口 OFS1 VDSELチェック対応
・RevRxNo121120-001	2012/11/20 橋口
　　メモリアクセスコマンドで低消費電力エラーが発生しない不具合修正
・RevRxNo130730-001 2014/06/18 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140515-005 2014/07/18 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo130730-001 2014/07/22 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
*/
#ifndef	__ERRCHK_H__
#define	__ERRCHK_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義
#define FFWERR_COM_MASK	0x7F000000
// RevNo111121-008 Append Start
#define OSM_CHKDATA_LENGTH 8	//オプション設定レジスタ比較時に比較値を格納するバイト列の長さ
#define OSM_CHKMASK_LENGTH 8	//オプション設定レジスタ比較時にマスクで使用するバイト列の長さ
//ReplaceEndianUBCode2ByteでもらうUBCODEの種類
#define	SET_UBCODE_A	1
#define	SET_UBCODE_B	2
// RevNo111121-008 Append End

// グローバル関数の宣言

// FFWモジュール内部関数
extern FFWERR TransError(BFWERR);									///< BFWエラーコードをFFWエラーコードに変換する
extern BOOL	CheckComError(FFWERR ferr);								///< FFWエラーコードの種類が通信エラーであるかを確認する
extern FFWERR ErrorChk_McuAccessSize(enum FFWENM_MACCESS_SIZE eAccessSize);	///< MCUアクセスサイズのエラーチェック
extern FFWERR ErrorChk_Pb(enum FFWENM_PB_SW eSw, MADDR madrBrkAddr);	///< PBコマンドのエラーチェック
extern FFWERR ErrorChk_Pbcl(void);									///< PBCLコマンドのエラーチェック
extern FFWERR ErrorChk_Dump(MADDR madrStart, MADDR madrEnd);		///< DUMPコマンドのエラーチェック
// V.1.02 新デバイス対応 Modify Start
extern FFWERR ErrorChk_Fill(MADDR madrStart, MADDR madrEnd, 
							DWORD dwWriteDataSize, const BYTE* pbyWriteBuff);	///< FILLコマンドのエラーチェック
extern FFWERR ErrorChk_Write(MADDR madrStart, MADDR madrEnd, 
							 enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff);	///< WRITEコマンドのエラーチェック
extern FFWERR ErrorChk_Move(MADDR madrSrcStartAddr, MADDR madrSrcEndAddr, 
							MADDR madrDisStartAddr, MADDR madrDisEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize);	///< MOVEアドレスのエラーチェック
// V.1.02 新デバイス対応 Modify End
// V.1.02 新デバイス対応 Append Line
extern FFWERR ErrorChk_CWrite(MADDR madrStart, MADDR madrEnd, const BYTE* pbyWriteBuff);	///< CWRITEコマンドのエラーチェック	// RevNo101129_002 Modify Line
// V.1.03 RevNo111121-008 Append Line
extern FFWERR	ErrorChk_Dwnp(MADDR madrStart, MADDR madrEnd, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff);
// ExtFlashModule_002b Append Line
extern FFWERR WarningChk_ExtFlashWriteErr(void);					///< 外部フラッシュダウンロード時のWarning発生確認
// V.1.02 新デバイス対応 Append Line
extern FFWERR WarningChk_FlashWriteErr(void);						///< 内蔵フラッシュダウンロード時のWarning発生確認
// V.1.02 RevNo110308-002 Append Line
extern FFWERR WarningChk_MemAccessErr(void);						///< メモリアクセス時のWarning発生確認
extern void InitErrorData_Errchk(void);								///< エラーチェック処理内部変数初期化

// RevRxNo140515-005 Append Line
extern FFWERR WarningChk_MemWaitInsertErr(void);					///< メモリウェイト挿入Warning発生確認

// RevRxNo130730-001 Append Line
extern FFWERR WarningChk_OsmFwrite(void);							///< オプション設定メモリ書き戻しWarning発生確認

// V.1.02 RevNo110613-001 Append Start
extern FFWERR WarningChk_BFW(void);
extern BOOL SetBFWWarning(FFWERR ferr);
extern void ClrBFWWarning(void);
// V.1.02 RevNo110613-001 Append End

extern FFWERR SetMcuStat2BmcuErr(enum FFWRXENM_STAT_MCU eStatMcu);	// RevRxNo121120-001 Append Line

extern FFWERR CmpOfs1Vdsel2TargetVcc(BYTE byVdsel,BOOL* pbOverVdsel);	// RevRxNo130730-001 Append Line

#endif	// __ERRCHK_H__
