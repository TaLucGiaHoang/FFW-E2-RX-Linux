///////////////////////////////////////////////////////////////////////////////
/**
 * @file domcu_mcu.h
 * @brief MCU依存コマンドのヘッダファイル
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, H.Akashi, S.Ueda, K.Uemori
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/12/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・V.1.03 RevNo111121-008 2012/07/11 橋口 OFS1 VDSELチェック対応
・RevRxNo120606-004 2012/06/11 橋口
  ・MPU領域リード/ライト対応
・RevRxNo120910-001	2012/09/11 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・DO_GetRXSTAT()の関数宣言での、引数のbyStatKindをdwStatKindに変更。
・RevRxNo120910-005	2012/11/12 明石
　ユーザブートモード起動時のUSBブート領域非消去指定対応
・RevRxNo130301-001 2013/08/02 植盛
	RX64M対応
・RevRxNo140109-001 2014/01/16 上田
	RX64Mオプション設定メモリへのダウンロード対応
・RevRxNo140617-001	2014/06/17 大喜多
	TrustedMemory機能対応
・RevRxNo140616-001 2014/06/16 植盛
	MCUコマンド個別制御領域に対する内部処理実装
・RevRxNo130730-001 2014/06/20 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
・RevRxNo161003-001 2016/12/01 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
*/
#ifndef	__DOMCU_MCU_H__
#define	__DOMCU_MCU_H__

#include "ffw_typedef.h"
#include "mcudef.h"
// #include "ffwmcu_mcu.h"

#define	STAT_EML_TRCINFO_RUN	0x00000001

//RevRxNo120606-004	Append Start
#define DDMA_ACCESS				0x00		// dDMAアクセス範囲 
#define CPU_ACCESS				0x01		// CPUアクセス範囲 
//RevRxNo120606-004	Append End


// enum定義
// RevRxE2LNo141104-001 Append Start(FFWE20RX600.hから移動、変更)
enum FFWRXENM_MAREA_TYPE {		// ターゲットMCUの領域指定
	MAREA_ETC=0,					// その他(以下以外)
	MAREA_USER,						// ユーザーマット
	MAREA_DATA,						// データマット
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Modify Line
	MAREA_USERBOOT,					// ユーザーブートマット
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Append Start
	MAREA_EXTRAM,					// 外部RAM
	MAREA_EXTROM,					// 外部ROM
// V.1.02 No.27 外部RAMダウンロード進捗表示対応 Append End
// RevRxNo140109-001 Append Line
	MAREA_CONFIG,					// コンフィギュレーション設定領域
	MAREA_INRAM						// 内蔵RAM領域
};
// RevRxE2LNo141104-001 Append End

// RevRxNo161003-001 Append Start
enum FFWRXENM_BANKMD {			// バンクモード情報
	RX_LINEAR=0,					// リニアモード
	RX_DUAL							// デュアルモード
};
enum FFWRXENM_BANKSEL {			// 起動バンク情報
	RX_BANKSEL_BANK0=0,				// 起動バンク0
	RX_BANKSEL_BANK1,				// 起動バンク1
	RX_BANKSEL_BANKCONT				// 起動バンク継続
};
// RevRxNo161003-001 Append End

#include "ffwmcu_mcu.h"
// グローバル関数の宣言
extern FFWERR DO_SetRXMCU(const FFWRX_MCUAREA_DATA* pMcuArea, const FFWRX_MCUINFO_DATA* pMcuInfo);	///< MCU情報の設定
extern FFWERR DO_GetRXMCU(FFWRX_MCUAREA_DATA* pMcuArea, FFWRX_MCUINFO_DATA* pMcuInfo);

extern FFWERR DO_SetRXDBG(const FFWMCU_DBG_DATA_RX* pDbg);	///< デバッグ情報の設定
extern FFWERR DO_GetRXDBG(FFWMCU_DBG_DATA_RX* pDbg);
// V.1.02 No.1 FINE通信I/F通信処理追加 Modify Line
// RevRxNo120910-001 Modify Start
extern FFWERR DO_GetRXSTAT(DWORD dwStatKind, enum FFWRXENM_STAT_MCU* peStatMcu, FFWRX_STAT_SFR_DATA* pStatSFR, 
							FFWRX_STAT_JTAG_DATA* pStatJTAG, FFWRX_STAT_FINE_DATA* pStatFINE, FFWRX_STAT_EML_DATA* pStatEML);	///< ターゲットMCUのステータス情報の取得
extern FFWERR DO_GetRXINSTCODE(DWORD dwPC, FFWRX_INST_DATA* pInstResult);	///< 命令コード解析結果の参照
// RevRxNo120910-005	Modify Start
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append Start
extern FFWERR DO_SetRXPMOD(enum FFWRXENM_PMODE ePmode, enum FFWRXENM_RMODE eRmode, enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA *pUbcode,
						   const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange);	///< プロセッサモードの設定
// V.1.02 No.3 起動時のエンディアン情報&デバッグ継続モード設定 Append End
// RevRxNo120910-005	Modify End

// RevRxNo130730-001 Append Start
extern FFWERR CheckOsm_RC03F_MF3(enum FFWRXENM_PMODE ePmode, enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA *pUbcode, 
								 const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange, BOOL* pbEndianChange, BOOL* pbUbcodeChange, 
								 BOOL* pbDebugCont, BOOL* pbLvdChange, FFWERR* pferrWarning, BOOL bPmodCmd);
// RevRxNo161003-001 Modify Line
extern FFWERR CheckOsm_RV40F(enum FFWRXENM_PMODE ePmode, enum FFWENM_ENDIAN eMcuEndian, const FFWRX_UBCODE_DATA *pUbcode, 
							 const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange, BOOL* pbEndianChange, BOOL* pbBankmdChange, BOOL* pbBankselChange, BOOL* pbUbcodeChange, 
							 BOOL* pbDebugCont, BOOL* pExtraChange, BOOL* pbLvdChange, FFWERR* pferrWarning, BOOL bPmodCmd);
// RevRxNo130730-001 Append End

extern FFWERR DO_RXHPON(DWORD dwPlug);				///< ホットプラグ機能の設定/解除
extern FFWERR DO_E20SetHotPlug(void);							///< HPONホットプラグ前のハードウェアの設定
extern FFWERR DO_RXIDCODE(BYTE* pbyID, BYTE* pbyResult);			///< 認証IDコードの設定

extern BOOL IsMcuRamAddr(MADDR madr);						///< 指定アドレスがMCU内蔵RAM領域であるかを判定する
extern BOOL IsMcuRomAddr(MADDR madr);						///< 指定アドレスがMCU内蔵ROM領域であるかを判定する
extern BOOL IsProgramRomAddr(MADDR madr);					///< 指定アドレスがMCU内蔵ROM(プログラムROM)領域であるかを判定する
extern BOOL IsDataRomAddr(MADDR madr);						///< 指定アドレスがMCU内蔵ROM(データフラッシュ)領域であるかを判定する
extern BOOL IsNotMcuRomArea(MADDR madrStart, MADDR madrEnd);	///< 指定領域がMCU内蔵ROM領域以外であるかを判定する
extern MADDR	GetFlashRomBlockEndAddr(MADDR madr);		///< 指定アドレスを含むフラッシュROMブロックの終了アドレスを取得する

extern void GetEndianType(MADDR madrStartAddr, BYTE* pbyEndian);									///< 設定されているエンディアンを取得する
extern FFWERR GetEndianType2(MADDR madrStartAddr, BYTE* pbyEndian);
extern FFWERR GetEndianTypeExtRom(MADDR madrStartAddr, BYTE* pbyEndian);
// V.1.02 RevNo110510-003 Modify Line
extern void GetExtAccessSize(MADDR madrStartAddr, MADDR madrEndAddr, enum FFWENM_MACCESS_SIZE* eAccessSize, BOOL* pbExtArea);	///< 外部メモリのアクセスサイズを取得する
extern void OffsetCWriteAddr(BYTE byEndian, MADDR madrStartAddr, DWORD* pdwStartAddr);				///< 開始アドレスを補正する
extern void ReplaceEndian(BYTE pbyBuff[], FFWENM_MACCESS_SIZE eAccessSize, DWORD dwLen);			///< 取得データをエンディアンに合わせる
// V.1.03 RevNo111121-008 Append Line
extern void ReplaceEndianLWord2Byte(DWORD* pdwData,BYTE* pbyData,FFWENM_ENDIAN eEndian,DWORD dwCount);

// RevNo010804-001 Append Line
extern DWORD GetDataFlashBlockNum(void);

extern FFWERR ChkBlockArea(MADDR madrStartAddr, MADDR madrEndAddr, DWORD* pdwLen, enum FFWRXENM_MAREA_TYPE *eAreaType, BYTE byDumpFill, BOOL* bEnableArea); // 指定開始アドレスがどの領域に属するかを判定する。
extern BOOL IsMcuInRomArea(enum FFWRXENM_MAREA_TYPE eAreaType);	// RevRxNo140109-001 Append Line

// RevRxNo120910-005	Append Line
extern	void SetUbAreaChangeRX(const FFWRX_UBAREA_CHANGEINFO_DATA *pUbAreaChange);


// V.1.02 RevNo110509-001 Append Start
extern BOOL GetPmodInfo(void);
extern void SetPmodInfo(BOOL bSet);
// V.1.02 RevNo110509-001 Append End

// V.1.02 RevNo110407-001 Append Start
extern BOOL GetHponOffInfo(void);							///< HPON OFF処理中かどうかの情報を取得
extern void SetHponOffInfo(BOOL bSet);						///< HPON OFF処理中かどうかの情報を設定
// V.1.02 RevNo110407-001 Append End

extern void ChkAreaAccessMethod(MADDR madrStartAddr, MADDR madrEndAddr, DWORD* pdwLen, BYTE* pbyAccMeans, BYTE* pbyAccType);	//RevRxNo120606-004	Append Line, RevRxNo140616-001 Modify Line
extern FFWERR ChkIncludeArea(MADDR madrStart, MADDR madrEnd, MADDR* madrCmpStart, MADDR* madrCmpEnd, DWORD dwCmpCnt);	// RevRxNo130301-001 Append Line

// RevRxNo140617-001 Append Start
extern BOOL GetTMEnable(void);				//TM機能有効無効情報を返す(TRUE:TM機能有効、FALSE:TM機能無効)
extern DWORD GetTMINFData(void);			//TM識別データ値(TMINF)を返す
extern BOOL ChkTmArea(MADDR madrAddr);		//指定アドレスがTM領域内かチェック(TRUE:TM領域内、FALSE:TM領域外)
extern BOOL ChkAreaOverlap(MADDR madrStart, MADDR madrEnd, MADDR* madrCmpStart, MADDR* madrCmpEnd, DWORD dwCmpCnt);		// 指定領域が比較領域と重複するかチェック
// RevRxNo140617-001 Append End

// RevRxNo161003-001 Append Line
extern BOOL GetTMEnableDual(void);			//FFEE0000h～FFEEFFFFhのTM機能有効無効情報を返す(TRUE:FFEE0000h～FFEEFFFFhのTM機能有効、FALSE:FFEE0000h～FFEEFFFFhのTM機能無効)

extern void InitFfwCmdMcuData_Mcu(void);	///< MCU依存コマンド用FFW内部変数の初期化

#endif	// __DOMCU_MCU_H__
