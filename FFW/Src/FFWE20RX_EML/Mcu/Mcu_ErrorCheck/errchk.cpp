///////////////////////////////////////////////////////////////////////////////
/**
 * @file errchk.cpp
 * @brief エラーチェック処理
 * @author RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, H.Akashi, S.Ueda, K.Uemori, Y.Kawakami, SDS T.Iwata, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/02/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・V.1.03 RevRxNo111121-001 内蔵ROM無効拡張起動時のROMへのS/Wブレーク設定禁止 2012/07/11 橋口
・V.1.03 RevNo111121-008 OFS1ライトデータチェック 2012/07/11 橋口
・V.1.03 RevRxNo111214-001 PBチェック時のチェック順変更 2012/07/11 橋口
・V.1.03 RevNo120123-001 MOVE時のオプション設定メモリデータ比較修正 2012/07/11 橋口
・V.1.03 RevNo120124-001 WRITE時のオプション設定メモリデータ比較修正 2012/07/11 橋口
・RevNo120614-001 2012/07/12 橋口 FFWERR_MCU_CONNECT_BANエラー変換対応
・RevRxNo120614-001 2012/07/12 橋口 クロック分周、切り替え前のビット確認
・RevRxNo120910-001	2012/09/11 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
  ・ErrorChk_Fill(), ErrorChk_Write(), ErrorChk_Move(), ErrorChk_CWrite()で、
    GetStatData()関数呼び出し時の引数のbyStatKindをdwStatKindに変更。
・RevNo121017-001	2012/10/18 明石
  VS2008対応エラーコード ERROR C4430対策
・RevRxNo121022-001	2012/10/22 SDS 岩田
    EZ-CUBE用エラー変換対応
・RevRxNo120910-004	2012/11/01 三宅
  ・checkOptionSettingMemoryFill()、
　　checkOptionSettingMemoryWrite()、
　　checkOptionSettingMemoryCWrite()、
　　checkOptionSettingMemoryMove() で、
　　・「OFS1 LVDASが有効設定になっているか確認」は、RX100以外の場合のみ実施するように変更。
　　・RX100の場合、OFS1.STUPLVD1RENが有効設定になっているか確認する処理を追加。
　　・関数の最後で、”OFS1.STUPLVD1RENが有効設定”になっている場合、
　　　エラーFFWERR_WRITE_OFS1_LVD1REN_ENAを返す。
  ・checkOptionSettingMemoryDwnp() で、
　　・「OFS1 LVDASが有効設定になっているか確認」は、RX100以外の場合のみ実施するように変更。
・RevRxNo120910-003 2012/11/01 三宅
　・cmpOfs1Vdsel2TargetVcc()で、
    ・電圧検出0レベル値のMCUファイル定義化による変更。
　・checkOptionSettingMemoryFill()、
　　checkOptionSettingMemoryWrite()、
　　checkOptionSettingMemoryCWrite()、
　　checkOptionSettingMemoryMove()、
　　checkOptionSettingMemoryDwnp() で、
　　・byVdsel生成時のマスクを削除。
・RevRxNo121120-001	2012/11/27 上田
　　メモリアクセスコマンドで低消費電力エラーが発生しない不具合修正
・RevRxNo121122-001 2012/11/27 上田
　低消費電力中のFILL/WRITEで実行中のROMアクセスエラーが発生しない不具合修正
・RevRxNo121122-001	2012/11/27 上田(2012/12/04 SDS 岩田 マージ)
　低消費電力中のMOVEで実行中のROMアクセスエラーが発生しない不具合修正
 ・RevRxEzNo121213-002 2012/12/18 橋口
  EZ-CUBE 性能向上対応
・RevRxNo121122-001	2012/12/11 SDS 岩田(2012/12/19 橋口マージ)
　低消費電力中のCWRITEで実行中のROMアクセスエラーが発生しない不具合修正
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130411-001 2014/01/21 植盛
	enum FFWRXENM_MDE_TYPE型のメンバ名変更(RX_MDE_OSM→RX_MDE_FLASH)
・RevRxNo130730-001 2014/06/18 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxNo140617-001	2014/06/17 大喜多
	TrustedMemory機能対応
・RevRxNo140515-010 2014/06/27 川上
	フラッシュ書き換えデバッグ有効時のS/Wブレーク設定改善	
・RevRxNo140515-005 2014/07/18 大喜多
	RX71M対応(メモリウェイト必要領域対応)
・RevRxNo130730-001 2014/07/22 植盛
	ユーザプログラムによるオプション設定メモリ書き換え後の再設定処理追加
・RevRxE2LNo141104-001 2014/12/22 上田
	E2 Lite対応
・RevRxNo150928-001 2015/09/28 PA 辻
	ROMキャッシュインバリデート Warning対応
	対応見送りのため、コメントアウト
・RevRxNo160527-001 2016/06/10 PA 辻
	RX651 正式対応
・RevRxNo161003-001 2016/10/28 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
*/

#include "errchk.h"
#include "BFWE20ERR.h"
#include "ffwrx_rrm.h"
#include "ffwmcu_brk.h"
#include "ffwmcu_mem.h"
#include "domcu_mcu.h"
#include "domcu_mem.h"
#include "domcu_prog.h"
#include "mcu_sfr.h"
#include "mcu_extflash.h"
#include "mcu_flash.h"
#include "mcuspec.h"
#include "do_sys.h"		// RevNo111121-008 Append Line
#include "mcudef.h"		// RevRxNo130411-001 Append Line
#include "mcu_mem.h"	// RevRxNo140515-005 Append line
#include "domcu_rst.h"	// RevRxNo130730-001 Append line
#include "FFWE20RX600.h"	// RevRxNo161003-001 Append Line
#include "doasp_sys_family.h"	// RevRxE2No171004-001 Append Line

// V.1.02 新デバイス対応 Append Start
//------------------------------------------------------------------------------
// static 関数宣言
//------------------------------------------------------------------------------
// V.1.03 RevNo111121-008 Modify Start
static FFWERR checkOptionSettingMemoryFill(MADDR madrStart, MADDR madrEnd, DWORD dwDataSize, const BYTE* pbyWriteBuff);
static FFWERR checkOptionSettingMemoryWrite(MADDR madrStart, MADDR madrEnd, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff);
static FFWERR checkOptionSettingMemoryCWrite(MADDR madrStart, MADDR madrEnd, const BYTE* pbyWriteBuff);
static FFWERR checkOptionSettingMemoryMove(MADDR dwmadrSrcStartAddr, MADDR dwmadrSrcEndAddr, MADDR dwmadrDisStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize);
static FFWERR checkOptionSettingMemoryDwnp(MADDR madrStart, MADDR madrEnd, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff);
// V.1.03 RevNo111121-008 Modify End
// V.1.03 RevNo111121-008 Append Start
static BOOL cmpOptionSettingMemory2FillData(MADDR madrStart,MADDR madrEnd,MADDR madrChkStart,MADDR madrChkEnd,BYTE* pbyFillData, BYTE* byChkData,BYTE* pbyChkMask,DWORD* dwFillCnt);
//RevNo120124-001 Modify Line
static BOOL cmpOptionSettingMemory2WriteData(MADDR madrStart,MADDR madrEnd,MADDR madrChkStart,MADDR madrChkEnd,enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff,BYTE* pbyChkData,BYTE* pbyChkMask,DWORD* pdwWriteCnt);
static BOOL cmpOptionSettingMemory2CWriteData(MADDR madrStart,MADDR madrEnd,MADDR madrChkStart,MADDR madrChkEnd,const BYTE* pbyWriteBuff,BYTE* pbyChkData,BYTE* pbyChkMask,BYTE* pbyWriteData);
static FFWERR cmpOptionSettingMemory2MoveData(MADDR dwmadrSrcStartAddr,MADDR dwmadrSrcEndAddr,MADDR dwmadrDisStartAddr,MADDR madrChkStart,MADDR madrChkEnd, enum FFWENM_MACCESS_SIZE eAccessSize,BYTE* pbyChkData,BYTE* pbyChkMask,BYTE* pbyMoveData,BOOL* pbMatch);
// RevRxNo130730-001 Delete
static void	replaceEndianUBCode2Byte(BYTE bySetUbcode, FFWENM_ENDIAN eEndian,BYTE* pbyData);
// V.1.03 RevNo111121-008 Append End
static void checkArea(MADDR madrStart, MADDR madrEnd, MADDR madrChkStart, MADDR madrChkEnd, MADDR* pmadrTmpEnd, BOOL* pbChkArea);
// V.1.02 新デバイス対応 Append End

// V.1.02 RevNo110613-001 Append Line
// RevNo121017-001	Modify Line
static FFWERR	s_ferrBFWWarning = FFWERR_OK;

//=============================================================================
/**
 * BFWエラーコードをFFWエラーコードに変換する。
 * @param berr BFWエラーコード
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR TransError(BFWERR berr)
{
	FFWERR	ferr;
	FFWE20_EINF_DATA	einfData;	// RevRxE2No171004-001 Append Line

	getEinfData(&einfData);			// RevRxE2No171004-001 Append Line

	switch (berr) {
	///// 正常終了 /////
	case BFWERR_OK:
		ferr = FFWERR_OK;
		break;

	///// コマンドエラー /////
	case BFWERR_BFW_ARG:	// 引数の指定に誤りがある
		ferr = FFWERR_FFW_ARG;
		break;
	case BFWERR_BFW_CMD:	// 未定義のコマンドコードを受信した
		ferr = FFWERR_BFW_CMD;
		break;

	///// エミュレータ内部資源のアクセスエラー /////
	// 共通項目

	// ベース基板に関するエラー	
	case BFWERR_BFWFROM_ERASE:	// BFW格納用 フラッシュROMのイレーズエラー
		ferr = FFWERR_BFWFROM_ERASE;
		break;
	case BFWERR_BFWFROM_WRITE:	// BFW格納用 フラッシュROMの書き込みエラー
		ferr = FFWERR_BFWFROM_WRITE;
		break;
	// RevRxE2LNo141104-001 Append Start
	case BFWERR_BFWFROM_VERIFY:	// BFW格納用 フラッシュROMのベリファイエラー
		ferr = FFWERR_BFWFROM_VERIFY;
		break;
	// RevRxE2LNo141104-001 Append End
	//RevNo100715-009 Modify Start
	case BFWERR_FPGA_CONFIG:	// DCSFコンフィグレーションエラー
		ferr = FFWERR_FPGA_CONFIG;
	//RevNo100715-009 Modify End
		break;
	// RevRxE2LNo141104-001 Append Start
	case BFWERR_LID_UNDEFINE:	// ライセンスIDが定義されていない
		ferr = FFWERR_LID_UNDEFINE;
		break;
	// RevRxE2LNo141104-001 Append End

	// MCU基板に関するエラー
	// RevRxE2No171004-001 Append Start
	case BFWERR_NID_BLANK:			// ニックネームがブランク状態で設定されていない
		ferr = FFWERR_NID_BLANK;
		break;
	case BFWERR_NIDSIZE_OVER:		// ニックネームの文字数が許容範囲を超えている
		ferr = FFWERR_NIDSIZE_OVER;
		break;
	case BFWERR_FLAG_BLANK:			// フラグがブランク状態で設定されていない
		ferr = FFWERR_FLAG_BLANK;
		break;
	// RevRxE2No171004-001 Append End
	case BFWERR_MCUROM_ERASE:		// ターゲットMCU内蔵 フラッシュROMのイレーズエラー
		ferr = FFWERR_MCUROM_ERASE;
		break;
	case BFWERR_MCUROM_WRITE:		// ターゲットMCU内蔵 フラッシュROMの書き込みエラー
		ferr = FFWERR_MCUROM_WRITE;
		break;
	case BFWERR_MCUROM_PCLK_ILLEGAL:// ターゲットMCU内蔵 フラッシュROMの周辺クロック指定範囲外エラー
		ferr = FFWERR_MCUROM_PCLK_ILLEGAL;
		break;
	case BFWERR_CLK_CHG_DISENA:		// ダウンロード時のクロックソース切り替え不可エラー
		ferr = FFWERR_CLK_CHG_DISENA;
		break;
	case BFWERR_EXTAL_OUT:			// ダウンロード時のEXTAL切り替え不可エラー
		ferr = FFWERR_EXTAL_OUT;
		break;

	// シリアル情報関連エラー
	case BFWERR_SNSIZE_OVER:			// シリアル番号の文字数が許容範囲を超えている
		// RevRxE2No171004-001 Modify Start
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			ferr = FFWERR_EX2BFPGA_CONFIG;		// 2nd拡張ボードFPGAコンフィグレーションエラー
		}
		else {
			ferr = FFWERR_SNSIZE_OVER;
		}
		break;
		// RevRxE2No171004-001 Modify End
	case BFWERR_SN_BLANK:				// シリアル番号がブランク状態で設定されていない
		// RevRxE2No171004-001 Modify Start
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			ferr = FFWERR_TMRCTRL_OVER;			// 既に全てのタイマ(コンペアマッチA,B)が使用されている。
		}
		else {
			ferr = FFWERR_SN_BLANK;
		}
		// RevRxE2No171004-001 Modify End
		break;
	// RevRxE2LNo141104-001 Append End

	// ベース基板に関するエラー2
	case BFWERR_TMRCTRL_OVER:			// 既に全てのタイマ(コンペアマッチA,B)が使用されている。
		// RevRxE2No171004-001 Modify Start
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			ferr = FFWERR_EX2BEROM_ERASE;		// 2nd拡張ボードEEPROMのイレーズエラー
		}
		else {
			ferr = FFWERR_TMRCTRL_OVER;
		}
		// RevRxE2No171004-001 Modify End
		break;
	// RevRxNo121022-001 Append Start
	case BFWERR_BFW_MCU_UNMATCH:
		// RevRxE2No171004-001 Modify Start
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			ferr = FFWERR_SPBFPGAFROM_WRITE;	// SPボードFPGA格納用フラッシュROMの書き込みエラー
		}
		else {
			// [EZ-CUBE]エミュレータに格納されたファームウェアがデバッグ対象のMCU用でありません。
			ferr = FFWERR_BFW_MCU_UNMATCH;
		}
		// RevRxE2No171004-001 Modify End
		break;
	case BFWERR_BFW_VER_UNMATCH:
		// RevRxE2No171004-001 Modify Start
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			ferr = FFWERR_SPBFPGAFROM_VERIFY;	// SPボードFPGA格納用フラッシュROMのベリファイエラー
		}
		else {
			// [EZ-CUBE]エミュレータに格納されたファームウェアのバージョンが一致していません。
			ferr = FFWERR_BFW_VER_UNMATCH;
		}
		// RevRxE2No171004-001 Modify End
		break;
	// RevRxNo121022-001 Append End

	// RevRxE2No171004-001 Append Start
	// 拡張ボードに関するエラー
	case BFWERR_EX1BFWFROM_ERASE:		// 拡張ボードF/W格納用フラッシュROMのイレーズエラー
		ferr = FFWERR_EX1BFWFROM_ERASE;
		break;
	case BFWERR_EX1BFWFROM_WRITE:		// 拡張ボードF/W格納用フラッシュROMの書き込みエラー
		ferr = FFWERR_EX1BFWFROM_WRITE;
		break;
	case BFWERR_EX1BFWFROM_VERIFY:		// 拡張ボードF/W格納用フラッシュROMのベリファイエラー
		ferr = FFWERR_EX1BFWFROM_VERIFY;
		break;
	case BFWERR_EX1BFPGA_CONFIG:		// 拡張ボードFPGAコンフィグレーションエラー
		ferr = FFWERR_EX1BFPGA_CONFIG;
		break;
	case BFWERR_EX1BEROM_ERASE:			// 拡張ボードEEPROMのイレーズエラー
		ferr = FFWERR_EX1BEROM_ERASE;
		break;
	case BFWERR_EX1BEROM_WRITE:			// 拡張ボードEEPROMの書き込みエラー
		ferr = FFWERR_EX1BEROM_WRITE;
		break;
	case BFWERR_EX1BEROM_VERIFY:		// 拡張ボードEEPROMのベリファイエラー
		ferr = FFWERR_EX1BEROM_VERIFY;
		break;
	case BFWERR_EX1BEROM_READ:			// 拡張ボードEEPROMのリードエラー
		ferr = FFWERR_EX1BEROM_READ;
		break;
	case BFWERR_EX1BEROM_NORES:			// 拡張ボードEEPROMの応答なしエラー
		ferr = FFWERR_EX1BEROM_NORES;
		break;
	case BFWERR_EEPROM_WRITE:			// EEPROMのライトエラー
		ferr = FFWERR_EEPROM_WRITE;
		break;
	case BFWERR_EEPROM_READ:			// EEPROMのリードエラー
		ferr = FFWERR_EEPROM_READ;
		break;

	// 2nd拡張ボードに関するエラー
	case BFWERR_SPBFPGAFROM_ERASE:		// SPボードFPGA格納用フラッシュROMのイレーズエラー
		ferr = FFWERR_SPBFPGAFROM_ERASE;
		break;
	case BFWERR_EX2BEROM_WRITE:			// 2nd拡張ボードEEPROMの書き込みエラー
		ferr = FFWERR_EX2BEROM_WRITE;
		break;
	case BFWERR_EX2BEROM_VERIFY:		// 2nd拡張ボードEEPROMのベリファイエラー
		ferr = FFWERR_EX2BEROM_VERIFY;
		break;
	case BFWERR_EX2BEROM_READ:			// 2nd拡張ボードEEPROMのリードエラー
		ferr = FFWERR_EX2BEROM_READ;
		break;
	case BFWERR_EX2BEROM_NORES:			// 2nd拡張ボードEEPROMの応答なしエラー
		ferr = FFWERR_EX2BEROM_NORES;
		break;

	case BFWERR_PODFPGAFROM_ERASE:		// エミュレーションポッドFPGA格納用フラッシュROMのイレーズエラー
		ferr = FFWERR_PODFPGAFROM_ERASE;
		break;
	case BFWERR_PODFPGAFROM_WRITE:		// エミュレーションポッドFPGA格納用フラッシュROMの書き込みエラー
		ferr = FFWERR_PODFPGAFROM_WRITE;
		break;
	case BFWERR_PODFPGAFROM_VERIFY:		// エミュレーションポッドFPGA格納用フラッシュROMのベリファイエラー
		ferr = FFWERR_PODFPGAFROM_VERIFY;
		break;

	case BFWERR_N_UNSUPPORT:			// 指定機能はサポートされていません。
		ferr = FFWERR_N_UNSUPPORT;
		break;
	// RevRxE2No171004-001 Append End

		// JTAG通信に関するエラー
	case BFWERR_JTAG_ILLEGAL_IR:		// JTAGのIR値が想定外
		ferr = FFWERR_JTAG_ILLEGAL_IR;
		break;

	// V.1.01 V.1.01.00.002の変更マージ Append Start
	case BFWERR_DAUTH:			// ID認証プログラムが完了しない。
		ferr = FFWERR_DAUTH_FAIL;
		break;
	// V.1.01 V.1.01.00.002の変更マージ Append End

	// RevNo120614-001 Append Start
	case BFWERR_MCU_CONNECT_BAN:			// 接続できませんでした。MCUはエミュレータ接続禁止状態です。
		ferr = FFWERR_MCU_CONNECT_BAN;
		break;
	// RevNo120614-001 Append End

	// RevRxE2No171004-001 Append Start
	case BFWERR_N_AMCU_IABORT:			// MCUに命令アボートが発生した
		ferr = FFWERR_N_AMCU_IABORT;
		break;
	case BFWERR_N_AMCU_DABORT:			// MCUにデータアボートが発生した
		ferr = FFWERR_N_AMCU_DABORT;
		break;
	// RevRxE2No171004-001 Append End

	// システム状態に関するエラー
	case BFWERR_LV1_DAMAGE:				// レベル1の内容が破損している
		ferr = FFWERR_LV1_DAMAGE;
		break;
	case BFWERR_BEXE_LEVEL0:			// BFW 内部動作モードがレベル0の状態でない
		ferr = FFWERR_BEXE_LEVEL0;
		break;

	case BFWERR_EML_ENDCODE:			// EML領域にエンドコードがない
		ferr = FFWERR_EML_ENDCODE;
		break;
	case BFWERR_FDT_ENDCODE:			// FDT領域にエンドコードがない
		ferr = FFWERR_FDT_ENDCODE;
		break;
	case BFWERR_EML_TO_FDT:				// EML動作モードからFDT動作モードへの遷移はできない。
		ferr = FFWERR_EML_TO_FDT;
		break;
	case BFWERR_FDT_TO_EML:				// FDT動作モードからEML動作モードへの遷移はできない。
		ferr = FFWERR_FDT_TO_EML;
		break;
	case BFWERR_MONP_ILLEGALEML:		// 不正なエミュレータの接続を検出した。
		ferr = FFWERR_MONP_ILLEGALEML;
		break;

	///// デバッグ機能のエラー /////
	// 共通項目

	// メモリアクセスに関するエラー
/////////////////////////////////////////////////////////////////////
// Ver.1.01 2010/08/17 SDS T.Iwata
	case BFWERR_FWRITE_NOTEXE_ILGLERR:	// MCU内蔵フラッシュROMへのアクセス違反などが発生しているため、フラッシュ書き換えが実行できない
		ferr = FFWERR_FWRITE_NOTEXE_ILGLERR;
		break;
	case BFWERR_FWRITE_FAIL_ILGLERR:	// MCU内蔵フラッシュROMへのアクセス違反が発生し、フラッシュ書き換えが正常に実行できなかった
		ferr = FFWERR_FWRITE_FAIL_ILGLERR;
		break;
/////////////////////////////////////////////////////////////////////

	// RevRxNo120614-001 Append Start
	case BFWERR_ACC_ROM_PEMODE:
		ferr = FFWERR_ACC_ROM_PEMODE;	// フラッシュ書き換えモード中のため、MCU内蔵ROM領域の読み出し/書き込み実行はできない
		break;
	// RevRxNo120614-001 Append End

	// V.1.02 RevNo110613-001 Append Start
	case BFWERR_CLKCHG_DMB_CLR:
		ferr = FFWERR_CLKCHG_DMB_CLR;	// メモリアクセス中のクロック切り替えなどによる通信エラーが発生したためリカバリした(Warning)
		break;
	// V.1.02 RevNo110613-001 Append End

	// RevRxNo160527-001 Append Start +3
	case BFWERR_FAW_FSPR_ALREADY_PROTECT:	// FAWレジスタFSPRビットによるプロテクトがすでに設定されている(Warning)
		ferr = FFWERR_FAW_FSPR_ALREADY_PROTECT;
		break;
	// RevRxNo160527-001 Append End

	///// コマンド実行前のステータスチェックエラー /////
	// プログラム実行状態に関するエラー
	case BFWERR_BMCU_RUN:		// ユーザプログラム実行中のためコマンド処理を実行できない
		ferr = FFWERR_BMCU_RUN;
		break;
	case BFWERR_BMCU_STOP:		// ユーザプログラム停止中のためコマンド処理を実行できない
		ferr = FFWERR_BMCU_STOP;
		break;

	// ターゲットステータスに関するエラー
	case BFWERR_BTARGET_POWER:	// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない
		ferr = FFWERR_BTARGET_POWER;
		break;
	case BFWERR_BTARGET_RESET:	// ターゲットシステム上のリセット端子が 'L'アクティブ状態のためコマンド処理を実行できない
		ferr = FFWERR_BTARGET_RESET;
		break;
	case BFWERR_BTARGET_NOCONNECT:	// ターゲットシステムと接続されていない
		ferr = FFWERR_BTARGET_NOCONNECT;
		break;
	case BFWERR_BTARGET_POWERSUPPLY:	// ターゲットシステムの電圧は既に供給されている
		ferr = FFWERR_BTARGET_POWERSUPPLY;
		break;

	// MCUのCPUステータスに関するエラー
	case BFWERR_BMCU_RESET:		// MCUがリセット状態のためコマンド処理を実行できない
		ferr = FFWERR_BMCU_RESET;
		break;
	case BFWERR_BMCU_STANBY:	// MCUのスタンバイモード中で内部クロックが停止状態である。
		ferr = FFWERR_BMCU_STANBY;
		break;
	case BFWERR_BMCU_SLEEP:		// MCUのスリープモード中で内部クロックが停止状態である。
		ferr = FFWERR_BMCU_SLEEP;
		break;

	//V.1.02 RevNo110621-001 Append Start
	case BFWERR_BMCU_AUTH_DIS:		// MCU内部リセットが発生したため、コマンド処理を実行できない。
		ferr = FFWERR_BMCU_AUTH_DIS;
		break;
	//V.1.02 RevNo110621-001 Append End

	// MCUのバスステータスに関するエラー

	// MCU端子状態に関するエラー
	case BFWERR_EMLE_NOT_HIGH:		// MCUのEMLE端子が 'L'状態のためJTAG動作がしない
		ferr = FFWERR_EMLE_NOT_HIGH;
		break;
	//RevNo100715-009 Append Start
	case BFWERR_ATARGET_TRSTN:		// MCUのTRSTN端子が コントロールできない
		ferr = FFWERR_ATARGET_TRSTN;
		break;
	case BFWERR_ATARGET_TMS:		// MCUのTMS端子が コントロールできない
		ferr = FFWERR_ATARGET_TMS;
		break;
	//RevNo100715-009 Append End
	case BFWERR_ATARGET_UB:			// MCUのUB端子が コントロールできないので、エミュレータとMCUが接続できない
		// RevNo110405-004 Modify Line
		ferr = FFWERR_ATARGET_UB;
		break;
	case BFWERR_ATARGET_MD:			// MCUのMD端子が コントロールできないので、エミュレータとMCUが接続できない
		// RevNo110405-004 Modify Line
		ferr = FFWERR_ATARGET_MD;
		break;
	// RevRxE2LNo141104-001 Append Start
	case BFWERR_ATARGET_EMLE:			// MCUのEMLE端子が コントロールできないので、エミュレータとMCUが接続できない
		ferr = FFWERR_ATARGET_EMLE;
	// RevRxE2LNo141104-001 Append End

	// RevRxE2No171004-001 Append Start
	case BFWERR_N_BFWWAIT_TIMEOUT:		// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない
		ferr = FFWERR_N_BFWWAIT_TIMEOUT;
		break;
	// RevRxE2No171004-001 Append End

	// SCI通信に関するエラー
	case BFWERR_BBR_UNDER:				// 設定可能なボーレート値より小さい
		ferr = FFWERR_BBR_UNDER;
		break;
	case BFWERR_BBR_OVER:				// 設定可能なボーレート値より大きい
		ferr = FFWERR_BBR_OVER;
		break;
	case BFWERR_ASCI_COMERR:			// ターゲット通信エラーが発生した
		ferr = FFWERR_ASCI_COMERR;
		break;
	case BFWERR_ASCI_COMOV:				// 通信中断が発生した。
		ferr = FFWERR_ASCI_COMOV;
		break;
	case BFWERR_ASCI_FER:				// フレーミングエラーが発生した。
		ferr = FFWERR_ASCI_FER;
		break;
	case BFWERR_ASCI_ORER:				// オーバーランエラーが発生した。
		ferr = FFWERR_ASCI_ORER;
		break;
	case BFWERR_ASCI_PER:				// パリティエラーが発生した。
		ferr = FFWERR_ASCI_PER;
		break;
	case BFWERR_ASCI_TRANSMIT:			// シリアルデータ送信中にタイムアウトエラーが発生した。
		ferr = FFWERR_ASCI_TRANSMIT;
		break;
	case BFWERR_ASCI_RECEIVE:			// シリアルデータ受信中にタイムアウトエラーが発生した。
		ferr = FFWERR_ASCI_RECEIVE;
		break;
	case BFWERR_ATARGET_RESET:			// ユーザリセット解除中にタイムアウトエラーが発生した。
		ferr = FFWERR_ATARGET_RESET;
		break;
	case BFWERR_ASCI_HALFEMP:			// シリアルデータ送信(送信用FIFOの空き(512バイト)待ち)中にタイムアウトエラーが発生した。
		ferr = FFWERR_ASCI_HALFEMP;
		break;
	// FINE通信に関するエラー
	case BFWERR_BAUDRATE_MEASUREMENT:	// 通信ボーレート確認コマンドによるボーレート計測モードへの遷移に失敗した。
		ferr = FFWERR_BAUDRATE_MEASUREMENT;
		break;

// RevRxNo150928-001 Append Start +6
#if 0
	// MCU内蔵資源のアクセスエラー
	// ROMキャッシュに関係するエラー
	case BFWERR_ROMC_BAN_INVALIDATE_PERMIT:	// ROMキャッシュ動作を禁止に変更後インバリデート実施し、ROMキャッシュ動作を許可に戻した(Warning)
		ferr = FFWERR_ROMC_BAN_INVALIDATE_PERMIT;
		break;
	case BFWERR_ROMC_INVALIDATE:		// ROMキャッシュインバリデートを実施した(Warning)
		ferr = FFWERR_ROMC_INVALIDATE;
		break;
#endif
// RevRxNo150928-001 Append End

	// デバッグ動作中のエラー
	// ブレークに関係するエラー
	case BFWERR_STOP_AMCU_RESET:	// リセットがはいっていたため、ブレークできなかった。
		ferr = FFWERR_STOP_AMCU_RESET;
		break;
	// デバッグコンソールに関するエラー
	case BFWERR_C2E_BUFF_EMPY:	// C2Eバッファが空の状態である。
		ferr = FFWERR_C2E_BUFF_EMPY;
		break;
	///// 外部フラッシュダウンロード関連エラー /////
	case BFWERR_EXTROM_ERASE:			// 外部フラッシュイレーズエラー
		ferr = FFWERR_EXTROM_ERASE;
		break;
	case BFWERR_EXTROM_WRITE:			// 外部フラッシュライトエラー
		ferr =FFWERR_EXTROM_WRITE;
		break;
	case BFWERR_EXTROM_IDREAD:			// 外部フラッシュメーカ・デバイスID読み出しエラー
		ferr = FFWERR_EXTROM_BUS;
		break;
	case BFWERR_EXTROM_IDUNMATCH:		// 外部フラッシュメーカ・デバイスID不一致エラー
		ferr = FFWERR_EXTROM_ID;
		break;

	///// コマンド実行後(EFWタイムアウト発生後)のステータスチェックエラー
	// MCUのCPUステータスに関するエラー
	case BFWERR_AMCU_POWER:		// タイムアウトが発生した。MCU供給電源がOFF状態である。
		ferr = FFWERR_AMCU_POWER;
		break;
	case BFWERR_AMCU_RESET:		// タイムアウトが発生した。MCUがリセット状態である。
		ferr = FFWERR_AMCU_RESET;
		break;
	case BFWERR_AMCU_STANBY:	// タイムアウトが発生した。MCUのスタンバイモード中で内部クロックが停止状態である。
		ferr = FFWERR_AMCU_STANBY;
		break;
	case BFWERR_AMCU_SLEEP:		// タイムアウトが発生した。MCUのスリープモード中で内部クロックが停止状態である。
		ferr = FFWERR_AMCU_SLEEP;
		break;

	case BFWERR_EL3_AMCU_AUTH_DIS:	// タイムアウトが発生した。MCUとのデバッガ認証切れ状態である。
		ferr = FFWERR_EL3_AMCU_AUTH_DIS;
		break;
	case BFWERR_EL3_BMCU_AUTH_DIS:	// MCUとのデバッガ認証切れ状態である。
		// Rev110303-003 Modify Line
		ferr = FFWERR_EL3_BMCU_AUTH_DIS;
		break;
	case BFWERR_EL1_AMCU_NO_DBG:	// MCUとエミュレータの接続に失敗した。
		ferr = FFWERR_AMCU_NO_DBG;
		break;

	//RevNo100715-039 Append Start
	//ターゲット状態に関するエラー
	case BFWERR_EL3_ATARGET_NOCONNECT:	// タイムアウトエラーが発生した。ターゲットが外れている。
		ferr = FFWERR_ATARGET_NOCONNECT;
		break;
	case BFWERR_EL3_ATARGET_POWER:		// タイムアウトエラーが発生した。ターゲット電源がOFFである。
		ferr = FFWERR_ATARGET_POWER;
		break;
	//RevNo100715-009 Modify Start
	case BFWERR_EL3_ATARGET_RESET:		// タイムアウトエラーが発生した。ターゲットリセットがLoである。
		ferr = FFWERR_TO_ATARGET_RESET;
	//RevNo100715-009 Modify Start
		break;

	// RevRxE2No171004-001 Append Start
	// ターゲットとの通信エラー
	case BFWERR_SU_SWD_TRANSFER:		// モニタプログラム処理でタイムアウトが発生した。エラーが特定できない。
		ferr = FFWERR_SU_SWD_TRANSFER;
		break;
	case BFWERR_SU_DAP_STICKYERR:		// DAPのSTICKYエラーが発生しています。エラーが特定できません。
		ferr = FFWERR_SU_DAP_STICKYERR;
		break;
	case BFWERR_SU_DAP_WAIT_TIMEOUT:	// DAPとの通信で、ACKがWAITからOKにならなかった。
		ferr = FFWERR_SU_DAP_WAIT_TIMEOUT;
		break;
	case BFWERR_SU_DAP_PROTOCOL_ERR:	// DAPとの通信で、想定外のACK(FAULT/WAIT/OK以外)が返ってきた、もしくはプロトコルエラーが発生した。
		ferr = FFWERR_SU_DAP_PROTOCOL_ERR;
		break;
	case BFWERR_SU_SWD_FAULT:			// SWDのACKでFAULTが発生した。
		ferr = FFWERR_SU_SWD_FAULT;
		break;

	// ASPに関するエラー
	case BFWERR_ASP_BREAK:				// ASP停止要求
		ferr = FFWERR_ASP_BREAK;
		break;
	case BFWERR_ASP_OVF:				// ASPオーバーフロー
		ferr = FFWERR_ASP_OVF;
		break;
	// RevRxE2No171004-001 Append End

	///// タイムアウトエラー /////
	//RevNo100715-028 Modify Start
	case BFWERR_OFW_TIMEOUT:	// OFW処理でタイムアウトが発生した。エラーが特定できない。
		ferr = FFWERR_OFW_TIMEOUT;
	//RevNo100715-009 Modify End
		break;
	case BFWERR_BFW_TIMEOUT:	// BFW処理でタイムアウトが発生した。
		ferr = FFWERR_BFW_TIMEOUT;
		break;
	///// 指定ルーチン実行エラー /////
	case BFWERR_SRM_TIMEOUT:		// 指定ルーチン実行でタイムアウトが発生した。
		ferr = FFWERR_SRM_TIMEOUT;
		break;
		///// 通過ポイント用モニタプログラム実行エラー /////
	case BFWERR_MON_POINT_TIMEOUT:		// 通過ポイント用モニタプログラム実行でタイムアウトが発生した。
		ferr = FFWERR_MON_POINT_TIMEOUT;
		break;

		// RevRxE2No171004-001 Append Start
	case BFWERR_SU_MON_TIMEOUT:				// モニタプログラム処理でタイムアウトが発生した。エラーが特定できない。
		ferr = FFWERR_SU_MON_TIMEOUT;
		break;
	case BFWERR_SU_BFW_TIMEOUT:				// BFW処理でタイムアウトが発生した。
		ferr = FFWERR_SU_BFW_TIMEOUT;
		break;
	case BFWERR_SU_DAP_STICKYERR_TIMEOUT:	// タイムアウトが発生しました。DAPのSTICKYエラーが発生しています。エラーが特定できません。
		ferr = FFWERR_SU_DAP_STICKYERR_TIMEOUT;
		break;
	// RevRxE2No171004-001 Append End

	// RevRxNo121022-001 Append Start
	// EZ-CUBE RX内部処理用
	case BFWERR_FUNC_UNSUPORT: // サポートしていない機能である。
		ferr =  FFWERR_FUNC_UNSUPORT;
		break;
	case BFWERR_USB_COMCODE_UNMATCH: // BFWDLL、BFW間でのUSB通信時のコマンドコード不一致
		ferr =  FFWERR_BFW_CMD; // 未定義のコマンドコードを受信した
		break;
	// RevRxNo121022-001 Append End

	///// 上記に当てはまらない場合 /////
	default:
		// RevRxE2No171004-001 Modify Start
		if (einfData.wEmuStatus == EML_E2) {	// E2エミュレータの場合
			switch (berr) {
			case 0x0106:		// シリアル番号がブランク状態で設定されていない
				ferr = FFWERR_SN_BLANK;
				break;
			case 0x010A:		// シリアル番号の文字数が許容範囲を超えている
				ferr = FFWERR_SNSIZE_OVER;
				break;
			case 0x3033:		// タイムアウトエラーが発生した。ターゲットリセットがLoである。
				ferr = FFWERR_TO_ATARGET_RESET;
				break;
			case 0x3031:		// タイムアウトエラーが発生した。ターゲットが外れている。
				ferr = FFWERR_ATARGET_NOCONNECT;
				break;
			case 0x3032:		// タイムアウトエラーが発生した。ターゲット電源がOFFである。
				ferr = FFWERR_ATARGET_POWER;
				break;
			default:
				ferr = FFWERR_COMDATA_ERRCODE;	// エラーコードが異常である
				break;
			}
		} else {
			ferr = FFWERR_COMDATA_ERRCODE;	// エラーコードが異常である
		}
		// RevRxE2No171004-001 Modify End
		break;
	}

	return ferr;
}

//=============================================================================
/**
 * FFWエラーコードの種類が通信エラーであるかを確認する
 * @param ferr FFWエラーコード
 * @retval TRUE 通信エラー(0x7Fxxxxxx)である
 * @retcal FALSE 通信エラー(0x7Fxxxxxx)でない
 */
//=============================================================================
BOOL	CheckComError(FFWERR ferr)
{
	BOOL	bComErr;
	FFWERR	ferrData;

	ferrData = ferr & FFWERR_COM_MASK;

	if (ferrData == FFWERR_COM_MASK) {	// 通信エラー(0x7Fxxxxxx)の場合
		bComErr = TRUE;
	} else {
		bComErr = FALSE;
	}

	return bComErr;
}


//=============================================================================
// ターゲットMCUに依存するパラメータチェック
//=============================================================================
//=============================================================================
/**
 * MCUアクセスサイズのエラーチェック関数
 *	FFWE20RXの場合、アクセスサイズが、
 *  MBYTE_ACCESS, MWORD_ACCESS, MLWORD_ACCESSでない場合、引数エラーを返送する。
 *	なお、特にチェックの必要がないMCUの場合は、何もせずFFWERR_OKを返すこと。
 * @param eAccessSize MCUアクセスサイズ
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_FFW_ARG 引数エラー
 */
//=============================================================================
FFWERR	ErrorChk_McuAccessSize(enum FFWENM_MACCESS_SIZE eAccessSize)
{
	FFWERR	ferr;

// 2008.8.18 MODIFY_BEGIN_E20RX600(-1,+1) {
	if ((eAccessSize == MBYTE_ACCESS) || (eAccessSize == MWORD_ACCESS) || (eAccessSize == MLWORD_ACCESS)) {
// 2008.8.18 MODIFY_END_E20RX600 }
		ferr = FFWERR_OK;
	} else {
		ferr = FFWERR_FFW_ARG;
	}

	return ferr;
}

//=============================================================================
/**
 * PBコマンドのエラーチェック関数
 *	FFWE20RXの場合、以下のエラーを判断する。
 *		(a)指定アドレスがRAM領域、フラッシュROM領域以外である。--> FFWERR_PBAREA_OUT
 *
 *	なお、特にチェックの必要がないMCUの場合は、何もせずFFWERR_OKを返すこと。
 *
 * @param eSw 設定／解除の指定
 * @param madrBrkAddr PB設定アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR	ErrorChk_Pb(enum FFWENM_PB_SW eSw, MADDR madrBrkAddr)
{
	FFWERR	ferr;
	enum FFWENM_PROGCMD_NO	eProgCmd;
	DWORD					dwBMode;
	DWORD					dwBFactor;
	FFWMCU_DBG_DATA_RX*		pDbgData;

	//ワーニング対策
	eSw;

	eProgCmd = GetMcuRunCmd();
	GetBmMode(&dwBMode, &dwBFactor);
	pDbgData = GetDbgDataRX();					// デバッグ情報取得

	// RevRxNo140617-001 Append Start
	if (GetPmodeInRomDisFlg() == FALSE) {		// ROM無効拡張モードでない場合
		if (GetTMEnable() == TRUE) {			// TM機能有効の場合
			if (ChkTmArea(madrBrkAddr) == TRUE){		// PB設定アドレスがTM領域内の場合
				return FFWERR_PBAREA_OUT;
			}
		}
	}
	// RevRxNo140617-001 Append End

	// V.1.02 No.26 データフラッシュCPU書き換えデバッグ対象化 Append & Modify Start
	// RevRxNo140515-010 Modify Start
	if(IsMcuRomAddr(madrBrkAddr) == TRUE){		// 指定アドレスがROM領域を含む場合
		// プログラムROM書き換えデバッグ指定ありの場合
		if (pDbgData->eDbgFlashWrite == EML_DBG_FLASHWRITE_USE) {
			return FFWERR_PB_ROMAREA_FLASHDEBUG;
		}
		// データフラッシュ書き換えデバッグ指定ありの場合
		else if (pDbgData->eDbgDataFlashWrite == EML_DBG_DATA_FLASHWRITE_USE) {
			return FFWERR_PB_ROMAREA_FLASHDEBUG;
		}
	// RevRxNo140515-010 Modify End
	// V.1.02 No.26 データフラッシュCPU書き換えデバッグ対象化 Append & Modify End

		// RevRxNo111121-001 Append Start
		// 起動モードがROM無効モードの場合はROMへのS/Wブレーク設定禁止
		// RevRxNo161003-001 Modify Line
		if((GetRmodeDataRX() == RX_RMODE_ROMD) || (GetRmodeDataRX() == RX_RMODE_ROMD_DUAL_BANK0) || 
			(GetRmodeDataRX() == RX_RMODE_ROMD_DUAL_BANK1) || (GetRmodeDataRX() == RX_RMODE_ROMD_DUAL)){
			return FFWERR_PBAREA_OUT;
		}
		// RevRxNo111121-001 Append End

		// RevRxNo111214-001 Modify Start
		if (GetMcuRunState()) {	// プログラム実行中 // RevRxE2LNo141104-001 Modify Line
			if ((eProgCmd == PROGCMD_GB) || (eProgCmd == PROGCMD_RSTG)) {	// GB, RSTG実行中の場合
				if (dwBMode & BMODE_SWB_BIT) {	// ブレークモードがS/W ブレーク許可の場合
					return FFWERR_PB_ROMAREA_MCURUN;
				}
			}
		}
		// RevRxNo111214-001 Modify End

		if(GetPmodeInRomDisFlg() == TRUE){
		// MCU内蔵ROM無効時はS/Wブレーク設定禁止
			return FFWERR_PBAREA_OUT;
		}
	}

	if (IsMcuRamAddr(madrBrkAddr) == TRUE) {		// 指定アドレスがMCU内蔵RAM領域である場合
		ferr = FFWERR_OK;
	} else if (IsMcuRomAddr(madrBrkAddr) == TRUE) {	// 指定アドレスがMCU内蔵ROM領域である場合
		ferr = FFWERR_OK;
	} else {
		ferr = FFWERR_PBAREA_OUT;
	}

	return ferr;
}


//=============================================================================
/**
 * PBCLコマンドのエラーチェック関数
 *	以下のエラーを判断する。
 *		(a)ソフトウェアブレーク実現手段がBRK命令埋め込みの場合
 *			(a-1)ROMエミュレーションメモリ種別がFLASHで、プログラム実行中で、
 *				MCU内蔵フラッシュROM領域にPBが設定されている。--> FFWERR_PB_ROMAREA_MCURUN
 *
 *	なお、特にチェックの必要がないMCUの場合は、何もせずFFWERR_OKを返すこと。
 *
 * @param なし
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_PB_ROMAREA_MCURUN ユーザプログラム実行中にMCU内蔵ROM領域へのソフトウェアブレークは設定/解除できない
 */
//=============================================================================
FFWERR ErrorChk_Pbcl(void)
{
	enum FFWENM_PROGCMD_NO	eProgCmd;
	FFW_PB_DATA*			pPb;
	DWORD					i;
	BOOL					bSetRom;
	DWORD					dwBMode;
	DWORD					dwBFactor;

	eProgCmd = GetMcuRunCmd();
	GetBmMode(&dwBMode, &dwBFactor);
	pPb = GetPbData();

	if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
		if ((eProgCmd == PROGCMD_GB) || (eProgCmd == PROGCMD_RSTG)) {	// GB, RSTG実行中の場合

			if (dwBMode & BMODE_SWB_BIT) {	// ブレークモードがS/W ブレーク許可の場合

				bSetRom = FALSE;
				for (i = 0; i < pPb->dwSetNum; i++) {
					if (IsMcuRomAddr(pPb->dwmadrAddr[i]) == TRUE) {
						bSetRom = TRUE;
						break;
					}
				}
				if (bSetRom == TRUE) {	// 内部ROM領域にPBが設定されている場合
					return FFWERR_PB_ROMAREA_MCURUN;
				}
			}
		}
	}

	return FFWERR_OK;

}
// 2008.11.12 MODIFY_END_E20RX600 }

//=============================================================================
/**
 * DUMPコマンドのエラーチェック関数
 *	FFWERR_OKを返送する。
 *	特にチェックの必要がないMCUの場合は、何もせずFFWERR_OKを返すこと。
 * @param madrStart 開始アドレス
 * @param madrEnd 終了アドレス
 * @retval FFWERR_OK 正常終了
 */
//=============================================================================
FFWERR	ErrorChk_Dump(MADDR madrStart, MADDR madrEnd)
{
	FFWERR	ferr = FFWERR_OK;

	//ワーニング対策
	madrStart;
	madrEnd;

	return ferr;
}

//=============================================================================
/**
 * FILLコマンドのエラーチェック関数
 *	以下のエラーを判断する。
 *		(a)プログラム実行中 かつ MCU内蔵ROM領域を含む --> FFWERR_WRITE_ROMAREA_MCURUN
 * 
 *	なお、特にチェックの必要がないMCUの場合は、何もせずFFWERR_OKを返すこと。
 * @param madrStart 開始アドレス
 * @param madrEnd 終了アドレス
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_WRITE_ROMAREA_MCURUN ユーザプログラム実行中にMCU内蔵ROM領域への書き込みは実行できない
 */
//=============================================================================
// V.1.02 新デバイス対応 Modify Line
FFWERR	ErrorChk_Fill(MADDR madrStart, MADDR madrEnd, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff)
{
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo120910-001 Modify Line
	DWORD					dwStatKind;
	enum FFWRXENM_STAT_MCU	eStatMcu;
	FFWRX_STAT_SFR_DATA		pStatSFR;
	FFWRX_STAT_JTAG_DATA	pStatJTAG;
	FFWRX_STAT_FINE_DATA	pStatFINE;
	FFWRX_STAT_EML_DATA		pStatEML;
	// RevRxNo140617-001 Append Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	MADDR	madrCmpStart[MCU_AREANUM_MAX_RX];	// 比較用先頭アドレス
	MADDR	madrCmpEnd[MCU_AREANUM_MAX_RX];		// 比較用終了アドレス
	DWORD	dwCmpCnt;
	BOOL	bAreaOverlap;						// 領域重複確認結果
	pMcuInfoData = GetMcuInfoDataRX();
	// RevRxNo140617-001 Append End

	// 処理高速化のため記述修正
	if (IsNotMcuRomArea(madrStart, madrEnd) == FALSE) {
		//内蔵ROMアドレスへのアクセス
		if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
			//本来ROM有効/無効状態はSetPmodeInRomReg2Flg()で取得するべきであるが、
			//FFWERR_WRITE_ROMAREA_MCURUNエラーを出したいので、STATでROM無効状態を取得
			// RevRxNo120910-001 Modify Line
			dwStatKind = STAT_KIND_SFR;		// SFR情報のみ取得
			// RevRxNo120910-001 Modify Line
			ferr = GetStatData(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// 内蔵ROM無効拡張モード以外の時
			if (pStatSFR.byStatProcMode != FFWRX_STAT_PMODE_EXMEMDIS) {
			// プログラム実行中 かつ MCU内蔵ROM領域を含む場合
				return FFWERR_WRITE_ROMAREA_MCURUN;
			}
		}
	}

	// RevRxNo121122-001 Append Start
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo121122-001 Append End

	// V.1.02 新デバイス対応 Append Start
	// V.1.03 RevNo111121-008 Modify Line
	// フラッシュROMのMDEレジスタ、UBコード領域への書き込みデータチェック
	ferr = checkOptionSettingMemoryFill(madrStart, madrEnd, dwWriteDataSize, pbyWriteBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// V.1.02 新デバイス対応 Append End

	// RevRxNo140617-001 Append Start
	if (GetPmodeInRomDisFlg() == FALSE) {		// ROM無効拡張モードでない場合
		if (GetTMEnable() == TRUE) {			// TM機能有効の場合
			// TM領域の先頭/終了アドレスを比較用配列に格納
			dwCmpCnt = 1;
			madrCmpStart[0] = pMcuInfoData->dwmadrMcuRegInfoAddr[8][0];
			madrCmpEnd[0] = pMcuInfoData->dwmadrMcuRegInfoAddr[9][0];
			bAreaOverlap = ChkAreaOverlap(madrStart, madrEnd, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);
			if (bAreaOverlap == TRUE) {		// FILL領域がTM領域と重複していた場合
				ferr = FFWERR_WRITE_TMAREA;
			}
		}
	}
	// RevRxNo140617-001 Append End

	return ferr;
}

//=============================================================================
/**
 * WRITEコマンドのエラーチェック関数
 *	以下のエラーを判断する。
 *		(a)プログラム実行中 かつ MCU内蔵ROM領域を含む --> FFWERR_WRITE_ROMAREA_MCURUN
 * 
 *	なお、特にチェックの必要がないMCUの場合は、何もせずFFWERR_OKを返すこと。
 * @param madrStart 開始アドレス
 * @param madrEnd 終了アドレス
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_WRITE_ROMAREA_MCURUN ユーザプログラム実行中にMCU内蔵ROM領域への書き込みは実行できない
 */
//=============================================================================
// V.1.02 新デバイス対応 Modify Line
FFWERR	ErrorChk_Write(MADDR madrStart, MADDR madrEnd, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff)
{

// V.1.02 新デバイス対応 Append Start
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo120910-001 Modify Line
	DWORD					dwStatKind;
	enum FFWRXENM_STAT_MCU	eStatMcu;
	FFWRX_STAT_SFR_DATA		pStatSFR;
	FFWRX_STAT_JTAG_DATA	pStatJTAG;
	FFWRX_STAT_FINE_DATA	pStatFINE;
	FFWRX_STAT_EML_DATA		pStatEML;
	// RevRxNo140617-001 Append Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	MADDR	madrCmpStart[MCU_AREANUM_MAX_RX];	// 比較用先頭アドレス
	MADDR	madrCmpEnd[MCU_AREANUM_MAX_RX];		// 比較用終了アドレス
	DWORD	dwCmpCnt;
	BOOL	bAreaOverlap;						// 領域重複確認結果
	pMcuInfoData = GetMcuInfoDataRX();
	// RevRxNo140617-001 Append End

	if (IsNotMcuRomArea(madrStart, madrEnd) == FALSE) {
		//内蔵ROMアドレスへのアクセス
		if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
			//本来ROM有効/無効状態はSetPmodeInRomReg2Flg()で取得するべきであるが、
			//FFWERR_WRITE_ROMAREA_MCURUNエラーを出したいので、STATでROM無効状態を取得
			// RevRxNo120910-001 Modify Line
			dwStatKind = STAT_KIND_SFR;		// SFR情報のみ取得
			// RevRxNo120910-001 Modify Line
			ferr = GetStatData(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// 内蔵ROM無効拡張モード以外の時
			if (pStatSFR.byStatProcMode != FFWRX_STAT_PMODE_EXMEMDIS) {
			// プログラム実行中 かつ MCU内蔵ROM領域を含む場合
				return FFWERR_WRITE_ROMAREA_MCURUN;
			}
		}
	}

	// RevRxNo121122-001 Append Start
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo121122-001 Append End

	// V.1.03 RevNo111121-008 Modify Line
	// フラッシュROMのMDEレジスタ、UBコード領域への書き込みデータチェック
	ferr = checkOptionSettingMemoryWrite(madrStart, madrEnd, eAccessSize, pbyWriteBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
// V.1.02 新デバイス対応 Append End

	// RevRxNo140617-001 Append Start
	if (GetPmodeInRomDisFlg() == FALSE) {		// ROM無効拡張モードでない場合
		if (GetTMEnable() == TRUE) {			// TM機能有効の場合
			// TM領域の先頭/終了アドレスを比較用配列に格納
			dwCmpCnt = 1;
			madrCmpStart[0] = pMcuInfoData->dwmadrMcuRegInfoAddr[8][0];
			madrCmpEnd[0] = pMcuInfoData->dwmadrMcuRegInfoAddr[9][0];
			bAreaOverlap = ChkAreaOverlap(madrStart, madrEnd, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);
			if (bAreaOverlap == TRUE) {		// WRITE領域がTM領域と重複していた場合
				ferr = FFWERR_WRITE_TMAREA;
			}
		}
	}
	// RevRxNo140617-001 Append End
	return ferr;
}

//=============================================================================
/**
 * MOVEコマンドのエラーチェック関数
 * @param madrSrcStartAddr 転送元開始アドレス
 * @param madrSrcEndAddr 転送元終了アドレス
 * @param madrDisStartAddr 転送先開始アドレス
 * @param madrDisEndAddr 転送先終了アドレス
 * @retval FFWERR_WRITE_ROMAREA_MCURUN ユーザプログラム実行中にMCU内蔵ROM領域への書き込みは実行できない
 */
//=============================================================================
// V.1.02 新デバイス対応 Modify Line
FFWERR	ErrorChk_Move(MADDR madrSrcStartAddr, MADDR madrSrcEndAddr, MADDR madrDisStartAddr, MADDR madrDisEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize)
{

// V.1.02 新デバイス対応 Append Start
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo120910-001 Modify Line
	DWORD					dwStatKind;
	enum FFWRXENM_STAT_MCU	eStatMcu;
	FFWRX_STAT_SFR_DATA		pStatSFR;
	FFWRX_STAT_JTAG_DATA	pStatJTAG;
	FFWRX_STAT_FINE_DATA	pStatFINE;
	FFWRX_STAT_EML_DATA		pStatEML;
	// RevRxNo140617-001 Append Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	MADDR	madrCmpStart[MCU_AREANUM_MAX_RX];	// 比較用先頭アドレス
	MADDR	madrCmpEnd[MCU_AREANUM_MAX_RX];		// 比較用終了アドレス
	DWORD	dwCmpCnt;
	BOOL	bAreaOverlap;						// 領域重複確認結果
	pMcuInfoData = GetMcuInfoDataRX();
	// RevRxNo140617-001 Append End

	if (IsNotMcuRomArea(madrDisStartAddr, madrDisEndAddr) == FALSE) {
		//内蔵ROMアドレスへのアクセス
		if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
			//本来ROM有効/無効状態はSetPmodeInRomReg2Flg()で取得するべきであるが、
			//FFWERR_WRITE_ROMAREA_MCURUNエラーを出したいので、STATでROM無効状態を取得
			// RevRxNo120910-001 Modify Line
			dwStatKind = STAT_KIND_SFR;		// SFR情報のみ取得
			// RevRxNo120910-001 Modify Line
			ferr = GetStatData(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// 内蔵ROM無効拡張モード以外の時
			if (pStatSFR.byStatProcMode != FFWRX_STAT_PMODE_EXMEMDIS) {
			// プログラム実行中 かつ MCU内蔵ROM領域を含む場合
				return FFWERR_WRITE_ROMAREA_MCURUN;
			}
		}
	}
	
	// RevRxNo121122-001 Append Start
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo121122-001 Append End

	// V.1.03 RevNo111121-008 Modify Line
	// フラッシュROMのMDEレジスタ、UBコード領域への書き込みデータチェック
	ferr = checkOptionSettingMemoryMove(madrSrcStartAddr, madrSrcEndAddr, madrDisStartAddr, eAccessSize);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
// V.1.02 新デバイス対応 Append End

	// RevRxNo140617-001 Append Start
	if (GetPmodeInRomDisFlg() == FALSE) {		// ROM無効拡張モードでない場合
		if (GetTMEnable() == TRUE) {			// TM機能有効の場合
			// TM領域の先頭/終了アドレスを比較用配列に格納
			dwCmpCnt = 1;
			madrCmpStart[0] = pMcuInfoData->dwmadrMcuRegInfoAddr[8][0];
			madrCmpEnd[0] = pMcuInfoData->dwmadrMcuRegInfoAddr[9][0];
			bAreaOverlap = ChkAreaOverlap(madrDisStartAddr, madrDisEndAddr, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);
			if (bAreaOverlap == TRUE) {		// 転送先領域がTM領域と重複していた場合
				ferr = FFWERR_WRITE_TMAREA;
			}
		}
	}
	// RevRxNo140617-001 Append End
	return ferr;
}

// V.1.02 新デバイス対応 Append Start
//=============================================================================
/**
 * CWRITEコマンドのエラーチェック関数
 *	FFWE100RX610の場合、以下のエラーを判断する。
 *		(a)ROMエミュレーションメモリ種別がFLASHで、プログラム実行中 かつ MCU内蔵ROM領域を含む --> FFWERR_WRITE_ROMAREA_MCURUN
 * 
 *	なお、特にチェックの必要がないMCUの場合は、何もせずFFWERR_OKを返すこと。
 * @param madrStart 開始アドレス
 * @param madrEnd 終了アドレス
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_WRITE_ROMAREA_MCURUN ユーザプログラム実行中にMCU内蔵ROM領域への書き込みは実行できない
 */
//=============================================================================
FFWERR	ErrorChk_CWrite(MADDR madrStart, MADDR madrEnd, const BYTE* pbyWriteBuff)
{
	FFWERR	ferr = FFWERR_OK;
	// RevRxNo120910-001 Modify Line
	DWORD					dwStatKind;
	enum FFWRXENM_STAT_MCU	eStatMcu;
	FFWRX_STAT_SFR_DATA		pStatSFR;
	FFWRX_STAT_JTAG_DATA	pStatJTAG;
	FFWRX_STAT_FINE_DATA	pStatFINE;
	FFWRX_STAT_EML_DATA		pStatEML;
	// RevRxNo140617-001 Append Start
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfoData;
	MADDR	madrCmpStart[MCU_AREANUM_MAX_RX];	// 比較用先頭アドレス
	MADDR	madrCmpEnd[MCU_AREANUM_MAX_RX];		// 比較用終了アドレス
	DWORD	dwCmpCnt;
	BOOL	bAreaOverlap;						// 領域重複確認結果
	pMcuInfoData = GetMcuInfoDataRX();
	// RevRxNo140617-001 Append End

	if (IsNotMcuRomArea(madrStart, madrEnd) == FALSE) {
		//内蔵ROMへのライト
		if (GetMcuRunState()) {	// RevRxE2LNo141104-001 Modify Line
			//本来ROM有効/無効状態はSetPmodeInRomReg2Flg()で取得するべきであるが、
			//FFWERR_WRITE_ROMAREA_MCURUNエラーを出したいので、STATでROM無効状態を取得
			// RevRxNo120910-001 Modify Line
			dwStatKind = STAT_KIND_SFR;		// SFR情報のみ取得
			// RevRxNo120910-001 Modify Line
			ferr = GetStatData(dwStatKind, &eStatMcu, &pStatSFR, &pStatJTAG, &pStatFINE, &pStatEML);
			if (ferr != FFWERR_OK) {
				return ferr;
			}
			// 内蔵ROM無効拡張モード以外の時
			if (pStatSFR.byStatProcMode != FFWRX_STAT_PMODE_EXMEMDIS) {
			// プログラム実行中 かつ MCU内蔵ROM領域を含む場合
				return FFWERR_WRITE_ROMAREA_MCURUN;
			}
		}
	}

	// RevRxNo121122-001 Append Start
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	// RevRxNo121122-001 Append End

	// V.1.03 RevNo111121-008 Modify Line
	// フラッシュROMのMDEレジスタ、UBコード領域への書き込みデータチェック
	ferr = checkOptionSettingMemoryCWrite(madrStart, madrEnd, pbyWriteBuff);
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo140617-001 Append Start
	if (GetPmodeInRomDisFlg() == FALSE) {		// ROM無効拡張モードでない場合
		if (GetTMEnable() == TRUE) {			// TM機能有効の場合
			// TM領域の先頭/終了アドレスを比較用配列に格納
			dwCmpCnt = 1;
			madrCmpStart[0] = pMcuInfoData->dwmadrMcuRegInfoAddr[8][0];
			madrCmpEnd[0] = pMcuInfoData->dwmadrMcuRegInfoAddr[9][0];
			bAreaOverlap = ChkAreaOverlap(madrStart, madrEnd, &madrCmpStart[0], &madrCmpEnd[0], dwCmpCnt);
			if (bAreaOverlap == TRUE) {		// CWRITE領域がTM領域と重複していた場合
				ferr = FFWERR_WRITE_TMAREA;
			}
		}
	}
	// RevRxNo140617-001 Append End

	return ferr;
}
// V.1.02 新デバイス対応 Append End
// V.1.03 RevNo111121-008 Append Start
//=============================================================================
/**
 * DWNP中のWRITEコマンドのエラーチェック関数
 *	以下のエラーを判断する。
 *		(a)プログラム実行中 かつ MCU内蔵ROM領域を含む --> FFWERR_WRITE_ROMAREA_MCURUN
 * 
 *	なお、特にチェックの必要がないMCUの場合は、何もせずFFWERR_OKを返すこと。
 * @param madrStart 開始アドレス
 * @param madrEnd 終了アドレス
 * @retval FFWERR_OK 正常終了
 * @retval FFWERR_WRITE_ROMAREA_MCURUN ユーザプログラム実行中にMCU内蔵ROM領域への書き込みは実行できない
 */
//=============================================================================
FFWERR	ErrorChk_Dwnp(MADDR madrStart, MADDR madrEnd, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff)
{
	FFWERR ferr = FFWERR_OK;
	// RevRxNo180228-001 Append Start
	FFWMCU_FWCTRL_DATA*	pFwCtrl;
	FFWMCU_MCUDEF_DATA* pMcuDef;

	pFwCtrl = GetFwCtrlData();
	pMcuDef = GetMcuDefData();
	// RevRxNo180228-001 Append End

	// DWNP中のプログラム実行中チェックはDWNP_OPENの先頭で行っているので、
	// IsMcuRun()がTRUEでくることはない。削除してもいいはず。2011.6.10
	// プログラム実行中 かつ ダウンロード領域に内蔵ROM領域が含まれる場合
	if (IsNotMcuRomArea(madrStart, madrEnd) == FALSE) {
		if (GetMcuRunState() == TRUE) {				//RevRxEzNo121213-002 Modify Line
			// 内蔵ROM無効拡張モード以外の場合、エラーを返す。			
			if( GetPmodeInRomDisFlg() != TRUE ){
				return FFWERR_WRITE_ROMAREA_MCURUN;
			}
		}
		
		// フラッシュROMのOFS1レジスタへの書き込みデータチェック
		ferr = checkOptionSettingMemoryDwnp(madrStart, madrEnd, eAccessSize, pbyWriteBuff);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	// RevRxNo180228-001 Append Start
	} else {
		if (pFwCtrl->eOfsType == RX_OFS_EXTRA) {	// OFSレジスタをExtra領域で設定するMCUの場合
			if (pFwCtrl->eLvdErrType == RX_LVD_LEVEL_CHK) {	// LVDx有効時の電圧レベルチェックを実施するMCUの場合
				// ダウンロードデータにOFS1レジスタへの書き込みデータが含まれるか判断
				if ((madrStart <= pMcuDef->madrOfs1StartAddr) && ((pMcuDef->madrOfs1StartAddr + 3) <= madrEnd)) {	// OFS1レジスタへの書き込みデータの場合
					// フラッシュROMのOFS1レジスタへの書き込みデータチェック
					ferr = checkOptionSettingMemoryDwnp(madrStart, madrEnd, eAccessSize, pbyWriteBuff);
					if (ferr != FFWERR_OK) {
						return ferr;
					}
				}
			}
		}
	// RevRxNo180228-001 Append End
	}

	return ferr;
}
// V.1.03 RevNo111121-008 Append End

// ExtFlashModule_002b Append Start
//=============================================================================
/**
 * 外部フラッシュダウンロード時のWarning発生確認
 * @param なし
 * @retval FFWERR_EXTROM_NOTWRITE_LOCKSECT	外部フラッシュROMにロックされているために書き込み/消去ができないセクタがあった
 * @retval FFWERR_OK						正常終了
 */
//=============================================================================
FFWERR WarningChk_ExtFlashWriteErr(void)
{
	BOOL	bExistLockSect;
	FFWERR	ferr;

	bExistLockSect = GetExistLockSectFlg();

	if (bExistLockSect == TRUE) {
		ferr = FFWERR_EXTROM_NOTWRITE_LOCKSECT;	// 外部フラッシュROMにロックされているために書き込み/消去ができないセクタがあった(Warning)
	} else {
		ferr = FFWERR_OK;
	}
	return ferr;
}
// ExtFlashModule_002b Append End

// V.1.02 新デバイス対応 Append Start
//=============================================================================
/**
 * 内蔵フラッシュダウンロード時のWarning発生確認
 * @param なし
 * @retval FFWERR_ROM_MDE_CHANGE			MDEを書き換えた		
 * @retval FFWERR_ROM_UBCODE_CHANGE			UBコードを書き換えた
 * @retval FFWERR_ROM_MDE_UBCODE_CHANGE		MDEもUBコードも書き換えた
 * @retval FFWERR_OK						正常終了
 */
//=============================================================================
FFWERR WarningChk_FlashWriteErr(void)
{
	BOOL	bEndianChange;
	BOOL	bUbcodeChange;
	BOOL	bBankmdChange;					// RevRxNo161003-001 Append Line
	FFWERR	ferr;

	bEndianChange = GetEndianChangeFlg();
	bUbcodeChange = GetUbcodeChangeFlg();
	bBankmdChange = GetBankModeChangeFlg();	// RevRxNo161003-001 Append Line

	// RevRxNo161003-001 Modify Start
	// RX65xはユーザブートが存在しないため、bUbcodeChangeを判断に使用している既存の判断文は
	// 変更しないよう、bBankmdChangeの判断を入れないようにした。
	if (bEndianChange && bUbcodeChange) {			// MDEもUBコードも書き換えた
		// V.1.02 RevNo110228-003 Modify Line
		ferr = FFWERR_WRITE_MDE_UBCODE_CHANGE;
	}else if (bEndianChange && (bBankmdChange == FALSE) && (bUbcodeChange == FALSE)) {	// MDEのみ書き換えた
		// V.1.02 RevNo110228-003 Modify Line
		ferr = FFWERR_WRITE_MDE_CHANGE;
	}else if ((bEndianChange == FALSE) && bBankmdChange && (bUbcodeChange == FALSE)) {	// BANKMDのみ書き換えた
		ferr = FFWERR_WRITE_BANKMD_CHANGE;
	}else if (bEndianChange && bBankmdChange && (bUbcodeChange == FALSE)) {				// MDE & BANKMDを書き換えた
		ferr = FFWERR_WRITE_MDE_BANKMD_CHANGE;
	}else if ((bEndianChange == FALSE) && bUbcodeChange) {								// UBコードのみ書き換えた
		// V.1.02 RevNo110228-003 Modify Line
		ferr = FFWERR_WRITE_UBCODE_CHANGE;
	} else {
		ferr = FFWERR_OK;
	}
	// RevRxNo161003-001 Modify End

	return ferr;
}
// V.1.02 新デバイス対応 Append End

// V.1.02 RevNo110308-002 Append Start
//=============================================================================
/**
 * メモリアクセス時のWarning発生確認
 * @param なし
 * @retval FFWERR_READ_DTF_MCURUN					
 * @retval FFWERR_READ_ROM_FLASHDBG_MCURUN		
 * @retval FFWERR_OK						正常終了
 */
//=============================================================================
FFWERR WarningChk_MemAccessErr(void)
{
	BOOL	bDFBanBlockRead;
	BOOL	bFlashChangePERead;
	FFWERR	ferr;

	bDFBanBlockRead = GetDtfMcurunFlg();
	bFlashChangePERead = GetFlashDbgMcurunFlg();

	if( bDFBanBlockRead == TRUE ){
		ferr = FFWERR_READ_DTF_MCURUN;
	}else if( bFlashChangePERead == TRUE ){
		ferr = FFWERR_READ_ROM_FLASHDBG_MCURUN;
	} else {
		ferr = FFWERR_OK;
	}
	return ferr;
}
// V.1.02 RevNo110308-002 Append End

//=============================================================================
/**
 * メモリアクセス時のメモリウェイト挿入Warning発生確認
 * @param なし
 * @retval FFWERR_ROM_WAIT_TMP_CHANGE					
 * @retval FFWERR_RAM_WAIT_TMP_CHANGE		
 * @retval FFWERR_OK						正常終了
 */
//=============================================================================
FFWERR WarningChk_MemWaitInsertErr(void)
{
	BYTE	byMemWaitInsert;
	FFWERR	ferr = FFWERR_OK;

	byMemWaitInsert = GetMemWaitInsertFlg();

	if (byMemWaitInsert == MEM_WAIT_ROM) {			// ROMへのメモリウェイト挿入の場合
		ferr = FFWERR_ROM_WAIT_TMP_CHANGE;
	} else if (byMemWaitInsert == MEM_WAIT_RAM) {	// RAMへのメモリウェイト挿入の場合
		ferr = FFWERR_RAM_WAIT_TMP_CHANGE;
	} else {
		// 上記以外はあり得ない
	}
	return ferr;
}


// RevRxNo130730-001 Append Start
//=============================================================================
/**
 * オプション設定メモリ書き戻し時のWarning発生確認
 * @param なし
 * @retval FFWERR_OK					正常終了
 * @retval FFWERR_ROM_MDE_CHANGE		MDEレジスタを指定された内容に書き換えた
 * @retval FFWERR_ROM_OFS1_CHANGE		OFS1レジスタを書き換えた
 * @retval FFWERR_ROM_MDE_OFS1_CHANGE	MDEレジスタおよびOFS1レジスタを書き換えた
 */
//=============================================================================
FFWERR WarningChk_OsmFwrite(void)
{
	FFWERR	ferr = FFWERR_OK;
	
	ferr = GetOsmFwriteWarningFlg();

	return ferr;
}
// RevRxNo130730-001 Append End


// V.1.02 RevNo110613-001 Append Start
//=============================================================================
/**
 * BFW処理内でWarning発生したか確認
 * @param なし
 * @retval FFWERR_CLKCHG_DMB_CLR dDMAワーニング
 * @retval FFWERR_ROMC_BAN_INVALIDATE_PERMIT ROMキャッシュ動作禁止ワーニング
 * @retval FFWERR_ROMC_INVALIDATE ROMキャッシュインバリデートワーニング
 * @retval FFWERR_OK						正常終了
 */
//=============================================================================
FFWERR WarningChk_BFW(void)
{
	FFWERR	ferr = FFWERR_OK;
	
	ferr = s_ferrBFWWarning;
	// 変数クリア
	ClrBFWWarning();

	return ferr;
}

//=============================================================================
/**
 * BFW処理内でWarning発生した場合、内部変数にワーニング内容を保持
 * @param なし
 * @retval TRUE			エラーがBFWからのワーニングだった
 * @retval FALSE		エラーがエラーもしくはFFWERR_OKだった
 */
//=============================================================================
BOOL SetBFWWarning(FFWERR ferr){

	BOOL bWarningCode = FALSE;

	switch(ferr){
	case FFWERR_CLKCHG_DMB_CLR:		// エラーがFFWERR_CLKCHG_DMB_CLR(ワーニング)の場合
	// RevRxNo150928-001 Append Start +2
#if 0
	case FFWERR_ROMC_BAN_INVALIDATE_PERMIT:	// エラーがFFWERR_ROMC_BAN_INVALIDATE_PERMIT(ワーニング)の場合
	case FFWERR_ROMC_INVALIDATE:	// エラーがFFWERR_ROMC_INVALIDATE(ワーニング)の場合
#endif
	// RevRxNo150928-001 Append End
	// RevRxNo160527-001 Append Line
	case FFWERR_FAW_FSPR_ALREADY_PROTECT:	// エラーがFFWERR_FAW_FSPR_ALREADY_PROTECT(ワーニング)の場合
		bWarningCode = TRUE;
		break;
	default:
		bWarningCode = FALSE;
		break;
	}

	// ワーニングの場合はエラーを内部管理変数へ保持しておく
	if(bWarningCode == TRUE){
		s_ferrBFWWarning = ferr;
	}

	return bWarningCode;
}
//=============================================================================
/**
 * BFW処理内でWarning発生した場合、内部変数にワーニング内容を保持
 * @param なし
 * @retval TRUE			エラーがBFWからのワーニングだった
 * @retval FALSE		エラーがエラーもしくはFFWERR_OKだった
 */
//=============================================================================
void ClrBFWWarning(void){

	s_ferrBFWWarning = FFWERR_OK;

	return ;
}
// V.1.02 RevNo110613-001 Append End

// RevRxNo121120-001 Append Start
//=============================================================================
/**
 * MCUステータス情報をコマンド実行前エラーに変換する
 * @param eStatMcu		MCUステータス情報
 * @return FFWERR　		FFWERR_BMCU_RESET: コマンド実行前リセットエラー
 * @return				FFWERR_BMCU_SLEEP: コマンド実行前スリープエラー
 * @return				FFWERR_BMCU_STANBY: コマンド実行前スタンバイエラー
 * @return				FFWERR_BMCU_DEEPSTANBY: コマンド実行前ディープスタンバイエラー
 * @return				FFWERR_OK: MCU通常状態
 */
//=============================================================================
FFWERR SetMcuStat2BmcuErr(enum FFWRXENM_STAT_MCU eStatMcu){

	FFWERR ferr = FFWERR_OK;

	switch (eStatMcu) {
	case RX_MCUSTAT_RESET:
		ferr = FFWERR_BMCU_RESET;
		break;
	case RX_MCUSTAT_SLEEP:
		ferr = FFWERR_BMCU_SLEEP;
		break;
	case RX_MCUSTAT_SOFT_STDBY:
		ferr = FFWERR_BMCU_STANBY;
		break;
	case RX_MCUSTAT_DEEP_STDBY:
		ferr = FFWERR_BMCU_DEEPSTANBY;
		break;
	default:
		ferr = FFWERR_OK;
		break;
	}

	return ferr;
}
// RevRxNo121120-001 Append End

// V.1.02 新デバイス対応 Append Start
//=============================================================================
/**
 * フラッシュROMのオプション設定メモリ(MDE、UBコード領域、OFS1)へのFILLデータのエラーチェックを行う。
 * @param madrStart		FILL開始アドレス
 * @param madrEnd		FILL終了アドレス
 * @param dwDataSize	書き込みデータサイズ
 * @param pbyWriteBuff	書き込みデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
// V.1.03 RevNo111121-008 Modify Line
static FFWERR checkOptionSettingMemoryFill(MADDR madrStart, MADDR madrEnd, DWORD dwDataSize, const BYTE* pbyWriteBuff)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	byChkData[OSM_CHKDATA_LENGTH];					// MDEレジスタ、UBコード領域の比較データ(正常データ)格納バッファ	// V.1.03 RevNo111121-008 Modify Line
	BYTE	byChkMask[OSM_CHKMASK_LENGTH];	// MDEレジスタデータ比較用マスク値		// V.1.03 RevNo111121-008 Modify Line
	BYTE	byFillData[8];					// FILLデータ格納バッファ
	DWORD	dwFillCnt;
	MADDR	madrChkStart, madrChkEnd;
	BOOL	bMatchMde = TRUE;
	BOOL	bMatchUbcode = TRUE;
	DWORD	i;
	DWORD	dwData;
	// V.1.03 RevNo111121-008 Append Start
	DWORD	dwMaskData;
	BOOL	bMatch = FALSE;
	BOOL	bOverVdsel = TRUE;
	BYTE	byVdsel;
	// V.1.03 RevNo111121-008 Append End
	// RevRxNo120910-004 Append Start
	BOOL	bStuplvd1renFlg = FALSE;  		// "OFS1.STUPLVD1RENが有効設定フラグ"(TRUE:有効"0"設定、FALSE:無効"1"設定)
	BOOL	bMatchStuplvd1ren = TRUE;		// "OFS1.STUPLVD1RENが一致フラグ"(TRUE:一致、FALSE:不一致)
	// RevRxNo120910-004 Append End

	FFWENM_ENDIAN				eEndian;
	enum FFWRXENM_PMODE	ePmode;				// 起動時に指定された端子設定起動モード
	FFWRX_UBCODE_DATA *pUbcode;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	ePmode = GetPmodeDataRX();
	pUbcode = GetUBCodeDataRX();
	eEndian = GetEndianDataRX();			// エンディアン情報取得

	// RevRxNo130411-001 Delete
	// MDEレジスタ, UBコード領域, OFS1レジスタへのライトデータチェックの実施有無は、
	// FFWMCU_FWCTRL_DATA構造体のeMdeType, bUbcodeExist, eOfsTypeで判断するように変更したため、
	// MCU_TYPE_0のときに何もせず正常終了する処理を削除。

	// 内蔵ROM無効状態の場合、何もせず正常終了する。( 事前にSetPmodeInRomReg2Flg()を実行しておくこと )
	if (GetPmodeInRomDisFlg()) {
		return FFWERR_OK;
	}

	// 8バイト分のFILLデータ生成(アドレス順に格納する)
	if (dwDataSize == MDATASIZE_1BYTE) {
		for (i = 0; i < 8; i++) {
			byFillData[i] = *pbyWriteBuff;
		}
	} else {
		if (eEndian == ENDIAN_LITTLE) {	// リトルエンディアンの場合
			if (dwDataSize == MDATASIZE_2BYTE) {
				for (i = 0; i < 8; i+=2) {
					byFillData[i]   = *(pbyWriteBuff + 1);
					byFillData[i+1] = *pbyWriteBuff;
				}
			} else if (dwDataSize == MDATASIZE_4BYTE) {
				for (i = 0; i < 8; i+=4) {
					byFillData[i]   = *(pbyWriteBuff + 3);
					byFillData[i+1] = *(pbyWriteBuff + 2);
					byFillData[i+2] = *(pbyWriteBuff + 1);
					byFillData[i+3] = *pbyWriteBuff;
				}
			} else {
				for (i = 0; i < 8; i++) {
					byFillData[i] = *(pbyWriteBuff + (7-i));
				}
			}

		} else {						// ビッグエンディアンの場合
			if (dwDataSize == MDATASIZE_2BYTE) {
				for (i = 0; i < 8; i+=2) {
					byFillData[i] = *pbyWriteBuff;
					byFillData[i+1] = *(pbyWriteBuff + 1);
				}
			} else if (dwDataSize == MDATASIZE_4BYTE) {
				for (i = 0; i < 8; i+=4) {
					byFillData[i] = *pbyWriteBuff;
					byFillData[i+1] = *(pbyWriteBuff + 1);
					byFillData[i+2] = *(pbyWriteBuff + 2);
					byFillData[i+3] = *(pbyWriteBuff + 3);
				}
			} else {
				for (i = 0; i < 8; i++) {
					byFillData[i] = *(pbyWriteBuff + i);
				}
			}
		}
	}

	///// MDEレジスタへのライトデータ確認 /////
	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eMdeType == RX_MDE_FLASH) {	// MDEをフラッシュROMで設定するMCUの場合	// RevRxNo130411-001 Modify Line
		// MDEレジスタの比較データ(正常データ)設定
		// V.1.03 RevNo111121-008 Modify Start
		if (eEndian == ENDIAN_LITTLE) {
			dwData = MCU_OSM_MDE_LITTLE_DATA;
		} else {
			dwData = MCU_OSM_MDE_BIG_DATA;
		}
		dwMaskData = MCU_OSM_MDE_MASK_DATA;
		//エンディアンデータをBYTE並びに変換
		ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
		//エンディアンマスクデータをBYTE並びに変換
		ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);
		// V.1.03 RevNo111121-008 Modify End

		// 領域確認アドレス設定
		if (ePmode != RX_PMODE_USRBOOT) {			// シングルチップ起動の場合
			madrChkStart = MCU_OSM_MDES_START;
			madrChkEnd = MCU_OSM_MDES_END;
		} else {									// ユーザブート起動の場合
			madrChkStart = MCU_OSM_MDEB_START;
			madrChkEnd = MCU_OSM_MDEB_END;
		}

		// V.1.03 RevNo111121-008 Modify Start
		// Fillデータの比較を実施
		bMatchMde = cmpOptionSettingMemory2FillData(madrStart,madrEnd,madrChkStart,madrChkEnd,&byFillData[0],&byChkData[0],&byChkMask[0],&dwFillCnt);
		// V.1.03 RevNo111121-008 Modify End
	}

	// ユーザブートモード時、UBコードA,B領域へのライトデータ確認
	// RevRxNo130411-001 Append Line
	if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
		if (ePmode == RX_PMODE_USRBOOT) {

			///// UBコードA領域へのライトデータ確認 /////
			// UBコードA領域の比較データ(正常データ)設定
			// V.1.03 RevNo111121-008 Modify Start
			replaceEndianUBCode2Byte(SET_UBCODE_A,eEndian,&byChkData[0]);

			//マスクデータ作成
			memset(&byChkMask[0],0xFF,OSM_CHKMASK_LENGTH);
			// V.1.03 RevNo111121-008 Modify End

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_UBCODE_A_START;
			madrChkEnd = MCU_OSM_UBCODE_A_END;

			// V.1.03 RevNo111121-008 Modify Line
			// Fillデータの比較を実施
			bMatchUbcode = cmpOptionSettingMemory2FillData(madrStart,madrEnd,madrChkStart,madrChkEnd,&byFillData[0],&byChkData[0],&byChkMask[0],&dwFillCnt);

			///// UBコードB領域へのライトデータ確認 /////
			if (bMatchUbcode != FALSE) {	// UBコードAへのライトデータが不一致でない場合のみ確認する
				// V.1.03 RevNo111121-008 Modify Start
				replaceEndianUBCode2Byte(SET_UBCODE_B,eEndian,&byChkData[0]);

				//マスクデータ作成
				memset(&byChkMask[0],0xFF,OSM_CHKMASK_LENGTH);
				// V.1.03 RevNo111121-008 Modify End

				madrChkStart = MCU_OSM_UBCODE_B_START;
				madrChkEnd = MCU_OSM_UBCODE_B_END;

				// V.1.03 RevNo111121-008 Modify Line
				// Fillデータの比較を実施
				bMatchUbcode = cmpOptionSettingMemory2FillData(madrStart,madrEnd,madrChkStart,madrChkEnd,&byFillData[0],&byChkData[0],&byChkMask[0],&dwFillCnt);
			}
		}
	}

	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eOfsType == RX_OFS_FLASH) {	// OFSレジスタをフラッシュROMで設定するMCUの場合
		// RevRxNo120910-004 Append Line, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_LEVEL_CHK) {	// LVDx有効時の電圧レベルチェックを実施するMCUの場合
			// V.1.03 RevNo111121-008 Append Start
			// OFS1 LVDASが有効設定になっているか確認		
			dwData = MCU_OSM_OFS1_LVDAS_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_LVDAS_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// Fillデータの比較を実施
			bMatch = cmpOptionSettingMemory2FillData(madrStart,madrEnd,madrChkStart,madrChkEnd,&byFillData[0],&byChkData[0],&byChkMask[0],&dwFillCnt);

			// VDSEL有効設定の場合
			if(bMatch == FALSE){
				// VDSEL値取得
				// RevRxNo120910-003 modify Line	// マスクを削除
				byVdsel = (BYTE)(byFillData[dwFillCnt]);
				// ターゲット電圧とVDSEL設定値の比較
				ferr = CmpOfs1Vdsel2TargetVcc(byVdsel,&bOverVdsel);		// RevRxNo130730-001 Modify Line
				if(ferr != FFWERR_OK){
					return ferr;
				}
			}
			// V.1.03 RevNo111121-008 Append End
		// RevRxNo120910-004 Append Line
		}

		// RevRxNo120910-004 Append Start
		// RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_ENA_CHK) {	// LVDx有効設定チェックを実施するMCUの場合
			// OFS1 STUPLVD1RENが有効設定になっているか確認
			dwData = MCU_OSM_OFS1_STUPLVD1REN_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_STUPLVD1REN_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// Fillデータの比較を実施
			bMatchStuplvd1ren = cmpOptionSettingMemory2FillData(madrStart,madrEnd,madrChkStart,madrChkEnd,&byFillData[0],&byChkData[0],&byChkMask[0],&dwFillCnt);

			// STUPLVD1REN有効設定の場合
			if (bMatchStuplvd1ren == FALSE) {
				bStuplvd1renFlg = TRUE;  // "OFS1.STUPLVD1RENが有効設定フラグ"に"有効設定(TRUE)"を設定
			}
		}
		// RevRxNo120910-004 Append End
	}

	if (bMatchMde == FALSE) {			// MDE不一致の場合
		ferr = FFWERR_WRITE_MDE_NOTMATCH;
	} else if (bMatchUbcode == FALSE) {	// UBコード不一致の場合
		ferr = FFWERR_WRITE_UBCODE_NOTMATCH;
	// V.1.03 RevNo111121-008 Modify Start
	} else if (bOverVdsel == FALSE) {						// ターゲット電圧がVDSEL以下の設定の場合
		ferr = FFWERR_WRITE_OFS1_TARGETVCC_UNDERVDSEL;
	// RevRxNo120910-004 Append Start
	} else if (bStuplvd1renFlg == TRUE) {					// OFS1.STUPLVD1RENが有効("0")設定の場合
		ferr = FFWERR_WRITE_OFS1_LVD1REN_ENA;	// エラー：OFS1レジスタへの起動時電圧監視1リセット有効設定は不可である
	// RevRxNo120910-004 Append End
	}
	// V.1.03 RevNo111121-008 Modify End

	return ferr;
}
// V.1.03 RevNo111121-008 Append Start
//=============================================================================
/**
 * Fillデータの比較を実施
 * @param madrStart		Fill開始アドレス
 * @param madrEnd		Fill終了アドレス
 * @param madrChkStart		比較開始アドレス
 * @param madrChkEnd		比較終了アドレス
 * @param pbyFillData[]	Fillデータ格納ポインタ(BYTE並び)
 * @param pbyChkData[]	比較データ格納ポインタ(BYTE並び)
 * @param pbyChkMaskData[]	比較マスクデータ格納ポインタ(BYTE並び)
 * @param pdwFillCnt	データ位置格納ポインタ
 * @return FFWエラーコード
 */
//=============================================================================
static BOOL cmpOptionSettingMemory2FillData(MADDR madrStart,MADDR madrEnd,MADDR madrChkStart,MADDR madrChkEnd,BYTE* pbyFillData, BYTE* byChkData,BYTE* pbyChkMask,DWORD* pdwFillCnt)
{
	DWORD	dwTotalLength;
	MADDR	madrTmpStart;
	MADDR	madrTmpEnd;
	DWORD	dwTmpLen,dwChkCnt;
	DWORD	i;
	BYTE	byData;
	BOOL	bChkArea;
	BOOL	bMatch = TRUE;
	DWORD	dwFillCnt = 0;

	dwTotalLength = madrEnd - madrStart + 1;
	madrTmpStart = madrStart;

	while (dwTotalLength) {
		checkArea(madrTmpStart, madrEnd, madrChkStart, madrChkEnd, &madrTmpEnd, &bChkArea);
		dwTmpLen = madrTmpEnd - madrTmpStart + 1;

		if (bChkArea == TRUE) {	// オプション設定レジスタを含む場合
			dwChkCnt = madrTmpStart - madrChkStart;		// オプション設定レジスタデータバッファ位置算出
			dwFillCnt = (madrTmpStart - madrStart) % 8;	// FILLデータバッファ位置算出

			for (i = 0; i < dwTmpLen; i++) {
				byData = (BYTE)(pbyFillData[dwFillCnt] & pbyChkMask[dwChkCnt]);	// オプション設定レジスタの確認ビットでマスク
				if (byData != byChkData[dwChkCnt]) {
					bMatch = FALSE;	// オプション設定レジスタへのライトデータが不一致
					break;
				}
				dwFillCnt++;
				if (dwFillCnt == 8) {
					dwFillCnt = 0;
				}
				dwChkCnt++;
			}
		}
		if (bMatch == FALSE) {
			break;
		}

		madrTmpStart += dwTmpLen;
		dwTotalLength -= dwTmpLen;
	}

	*pdwFillCnt = dwFillCnt;

	return bMatch;
}
// V.1.03 RevNo111121-008 Append End

//=============================================================================
/**
 * フラッシュROMのオプション設定メモリ(MDE、UBコード領域、OFS1)のWRITEデータのエラーチェックを行う。
 * @param madrStart		WRITE開始アドレス
 * @param madrEnd		WRITE終了アドレス
 * @param eAccessSize	アクセスサイズ
 * @param pbyWriteBuff	書き込みデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR checkOptionSettingMemoryWrite(MADDR madrStart, MADDR madrEnd, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff)
{

	FFWERR	ferr = FFWERR_OK;
	BYTE	byChkData[OSM_CHKDATA_LENGTH];	// オプション設定レジスタの比較データ(正常データ)格納バッファ	// V.1.03 RevNo111121-008 Modify Line
	BYTE	byChkMask[OSM_CHKMASK_LENGTH];	// オプション設定レジスタデータ比較用マスク値					// V.1.03 RevNo111121-008 Modify Line
	DWORD	dwWriteCnt;
	MADDR	madrChkStart, madrChkEnd;
	BOOL	bMatchMde = TRUE;
	BOOL	bMatchUbcode = TRUE;
	BOOL	bMatch = FALSE;					// V.1.03 RevNo111121-008 Append Line
	BOOL	bOverVdsel = TRUE;
	BYTE	byVdsel;						// V.1.03 RevNo111121-008 Modify Line
	DWORD	dwData;
	DWORD	dwMaskData;						// V.1.03 RevNo111121-008 Appned Line
	// RevRxNo120910-004 Append Start
	BOOL	bStuplvd1renFlg = FALSE;  		// "OFS1.STUPLVD1RENが有効設定フラグ"(TRUE:有効"0"設定、FALSE:無効"1"設定)
	BOOL	bMatchStuplvd1ren = TRUE;		// "OFS1.STUPLVD1RENが一致フラグ"(TRUE:一致、FALSE:不一致)
	// RevRxNo120910-004 Append End

	FFWENM_ENDIAN				eEndian;
	enum FFWRXENM_PMODE	ePmode;				// 起動時に指定された端子設定起動モード
	FFWRX_UBCODE_DATA *pUbcode;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	ePmode = GetPmodeDataRX();
	pUbcode = GetUBCodeDataRX();
	eEndian = GetEndianDataRX();			// エンディアン情報取得

	// RevRxNo130411-001 Delete
	// MDEレジスタ, UBコード領域, OFS1レジスタへのライトデータチェックの実施有無は、
	// FFWMCU_FWCTRL_DATA構造体のeMdeType, bUbcodeExist, eOfsTypeで判断するように変更したため、
	// MCU_TYPE_0のときに何もせず正常終了する処理を削除。

	// 内蔵ROM無効状態の場合、何もせず正常終了する。( 事前にSetPmodeInRomReg2Flg()を実行しておくこと )
	if (GetPmodeInRomDisFlg()) {
		return FFWERR_OK;
	}

	///// MDEレジスタへのライトデータ確認 /////
	// MDEレジスタの比較データ(正常データ)設定
	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eMdeType == RX_MDE_FLASH) {	// MDEをフラッシュROMで設定するMCUの場合	// RevRxNo130411-001 Modify Line
		// V.1.03 RevNo111121-008 Modify Start
		if (eEndian == ENDIAN_LITTLE) {
			dwData = MCU_OSM_MDE_LITTLE_DATA;
		} else {
			dwData = MCU_OSM_MDE_BIG_DATA;
		}
		dwMaskData = MCU_OSM_MDE_MASK_DATA;
		//エンディアンデータをBYTE並びに変換
		ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
		//エンディアンマスクデータをBYTE並びに変換
		ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

		// V.1.03 RevNo111121-008 Modify End
		// 領域確認アドレス設定
		if (ePmode != RX_PMODE_USRBOOT) {		// シングルチップ起動の場合
			madrChkStart = MCU_OSM_MDES_START;
			madrChkEnd = MCU_OSM_MDES_END;
		} else {								// ユーザブート起動の場合
			madrChkStart = MCU_OSM_MDEB_START;
			madrChkEnd = MCU_OSM_MDEB_END;
		}

		//Writeデータとの比較
		// V.1.03 RevNo111121-008 Modify Line
		bMatchMde = cmpOptionSettingMemory2WriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,eAccessSize,pbyWriteBuff,&byChkData[0],&byChkMask[0],&dwWriteCnt); 	//RevNo120124-001 Modify Line
	}

	// ユーザブートモード時、UBコードA,B領域へのライトデータ確認
	// RevRxNo130411-001 Append Line
	if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
		if (ePmode == RX_PMODE_USRBOOT) {
			///// UBコードA領域へのライトデータ確認 /////
			// UBコードA領域の比較データ(正常データ)設定
			// V.1.03 RevNo111121-008 Modify Start
			replaceEndianUBCode2Byte(SET_UBCODE_A,eEndian,&byChkData[0]);

			//マスクデータ作成
			memset(&byChkMask[0],0xFF,OSM_CHKMASK_LENGTH);
			// V.1.03 RevNo111121-008 Modify End

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_UBCODE_A_START;
			madrChkEnd = MCU_OSM_UBCODE_A_END;

			// V.1.03 RevNo111121-008 Modify Line
			bMatchUbcode = cmpOptionSettingMemory2WriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,eAccessSize,pbyWriteBuff,&byChkData[0],&byChkMask[0],&dwWriteCnt); 	//RevNo120124-001 Modify Line

			///// UBコードB領域へのライトデータ確認 /////
			if (bMatchUbcode != FALSE) {	// UBコードAへのライトデータが不一致でない場合のみ確認する
				// V.1.03 RevNo111121-008 Modify Start
				replaceEndianUBCode2Byte(SET_UBCODE_B,eEndian,&byChkData[0]);
				//マスクデータ作成
				memset(&byChkMask[0],0xFF,OSM_CHKMASK_LENGTH);
				// V.1.03 RevNo111121-008 Modify End
				
				//アドレス設定
				madrChkStart = MCU_OSM_UBCODE_B_START;
				madrChkEnd = MCU_OSM_UBCODE_B_END;

				// V.1.03 RevNo111121-008 Modify Line
				bMatchUbcode = cmpOptionSettingMemory2WriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,eAccessSize,pbyWriteBuff,&byChkData[0],&byChkMask[0],&dwWriteCnt); 	//RevNo120124-001 Modify Line
			}
		}
	}

	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eOfsType == RX_OFS_FLASH) {	// OFSレジスタをフラッシュROMで設定するMCUの場合
		// RevRxNo120910-004 Append Line, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_LEVEL_CHK) {	// LVDx有効時の電圧レベルチェックを実施するMCUの場合
			// V.1.03 RevNo111121-008 Append Start
			// OFS1 LVDASが有効設定になっているか確認		
			dwData = MCU_OSM_OFS1_LVDAS_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_LVDAS_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// WRITEデータの比較を実施
			bMatch = cmpOptionSettingMemory2WriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,eAccessSize,pbyWriteBuff,&byChkData[0],&byChkMask[0],&dwWriteCnt); 	//RevNo120124-001 Modify Line
			// VDSEL有効設定の場合
			if(bMatch == FALSE){
				// VDSEL値取得
				// RevRxNo120910-003 modify Line	// マスクを削除
				byVdsel = (BYTE)(pbyWriteBuff[dwWriteCnt]);
				// ターゲット電圧とVDSEL設定値の比較
				ferr = CmpOfs1Vdsel2TargetVcc(byVdsel,&bOverVdsel);		// RevRxNo130730-001 Modify Line
				if(ferr != FFWERR_OK){
					return ferr;
				}
			}
			// V.1.03 RevNo111121-008 Append End
		// RevRxNo120910-004 Append Line
		}

		// RevRxNo120910-004 Append Start
		// RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_ENA_CHK) {	// LVDx有効設定チェックを実施するMCUの場合
			// OFS1 STUPLVD1RENが有効設定になっているか確認		
			dwData = MCU_OSM_OFS1_STUPLVD1REN_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_STUPLVD1REN_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// WRITEデータの比較を実施
			bMatchStuplvd1ren = cmpOptionSettingMemory2WriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,eAccessSize,pbyWriteBuff,&byChkData[0],&byChkMask[0],&dwWriteCnt);

			// STUPLVD1REN有効設定の場合
			if (bMatchStuplvd1ren == FALSE) {
				bStuplvd1renFlg = TRUE;  // "OFS1.STUPLVD1RENが有効設定フラグ"に"有効設定(TRUE)"を設定
			}
		}
		// RevRxNo120910-004 Append End
	}

	if (bMatchMde == FALSE) {			// MDE不一致の場合
		ferr = FFWERR_WRITE_MDE_NOTMATCH;
	} else if (bMatchUbcode == FALSE) {	// UBコード不一致の場合
		ferr = FFWERR_WRITE_UBCODE_NOTMATCH;
	// V.1.03 RevNo111121-008 Modify Start
	} else if (bOverVdsel == FALSE) {						// ターゲット電圧がVDSEL以下の設定の場合
		ferr = FFWERR_WRITE_OFS1_TARGETVCC_UNDERVDSEL;
	// RevRxNo120910-004 Append Start
	} else if (bStuplvd1renFlg == TRUE) {					// OFS1.STUPLVD1RENが有効("0")設定の場合
		ferr = FFWERR_WRITE_OFS1_LVD1REN_ENA;	// エラー：OFS1レジスタへの起動時電圧監視1リセット有効設定は不可である
	// RevRxNo120910-004 Append End
	}
	// V.1.03 RevNo111121-008 Modify End

	return ferr;
}

// V.1.03 RevNo111121-008 Append Start
//=============================================================================
/**
 * WRITEデータの比較を実施
 * @param madrStart		WRITE開始アドレス
 * @param madrEnd		WRITE終了アドレス
 * @param madrChkStart		比較開始アドレス
 * @param madrChkEnd		比較終了アドレス
 * @param dwDataSize		WRITEデータサイズ
 * @param pbyWriteBuff[]	WRITEデータ格納ポインタ(BYTE並び)
 * @param pbyChkData[]		比較データ格納ポインタ(BYTE並び)
 * @param pbyChkMaskData[]	比較マスクデータ格納ポインタ(BYTE並び)
 * @param pdwWriteCnt	データ位置格納ポインタ
 * @return FFWエラーコード
 */
//=============================================================================
static BOOL cmpOptionSettingMemory2WriteData(MADDR madrStart,MADDR madrEnd,MADDR madrChkStart,MADDR madrChkEnd,enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff,BYTE* pbyChkData,BYTE* pbyChkMask,DWORD* pdwWriteCnt) 	//RevNo120124-001 Modify Line
{
	BOOL	bMatch = TRUE;
	BOOL	bChkArea;
	BYTE*	pbyWriteData;
	BYTE	byData,byChkData,byChkMask;
	DWORD	dwTotalLength;
	DWORD	dwTmpLen;
	DWORD	dwChkCnt;
	DWORD	dwData;
	DWORD	dwWriteCnt = 0;
	DWORD	i;
	MADDR	madrTmpStart;
	MADDR	madrTmpEnd;
	FFWENM_ENDIAN				eEndian;

	//RevNo120124-001 Append Line
	DWORD	dwDataSize;
	DWORD	dwWriteByteCnt;

	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwDataSize = 1;
		break;
	case MWORD_ACCESS:
		dwDataSize = 2;
		break;
	case MLWORD_ACCESS:
		dwDataSize = 4;
		break;
	default:
		dwDataSize = 4;
		break;	// ここは通らない
	}
	//RevNo120124-001 Append End

	eEndian = GetEndianDataRX();			// エンディアン情報取得

	dwTotalLength = madrEnd - madrStart + 1;
	madrTmpStart = madrStart;

	while (dwTotalLength) {
		checkArea(madrTmpStart, madrEnd, madrChkStart, madrChkEnd, &madrTmpEnd, &bChkArea);
		dwTmpLen = madrTmpEnd - madrTmpStart + 1;

		if (bChkArea == TRUE) {
			dwChkCnt = madrTmpStart - madrChkStart;			// オプション設定レジスタデータバッファ位置算出

			//pbyWriteBuffのデータが入っているデータ数を求める
			dwWriteByteCnt = madrEnd - madrStart+1;			//RevNo120124-001 Append Line

			for (i = 0; i < dwTmpLen; i++) {
				// ライトデータバッファ位置算出
				if (eEndian == ENDIAN_LITTLE) {
					dwData = (madrTmpStart + i) - madrStart;
					dwWriteCnt = (((dwData / dwDataSize) + 1) * dwDataSize) - (dwData % dwDataSize) - 1;
					// RevNo120124-001 Append Start
					// 端数データ(WORDアクセス:奇数個のデータ,LWORDアクセス:4n個以外のデータ数)の場合に、
					// pbyWriteBuffに対して、端数部はバイト並びの前詰めでライトデータが格納されている。
					// このためLITTLEエンディアンでうまく比較できない場合がある。
					// 端数位置を比較するときに、比較順を以下のように補正する。
					//              LWORD データ数:3    データ数:2  データ数:1 WORD データ数:1 
					// pbyWriteBuff       |0|1|2|3|     |0|1|2|3|   |0|1|2|3|       |0|1|
					// 比較順 　　　        |3|2|1|         |2|1|         |1|         |1|
					// 補正比較順 　      |1|2|3|       |1|2|       |1|             |1|
					// 端数比較の条件は、以下の1かつ2の場合
					// 1.比較データ位置がデータ数のデータサイズの切れ目の最大位置以上のとき ((dwWriteByteCnt / dwDataSize) * dwDataSize) < dwWriteCnt )
					// 2.データ数をdwDataSizeで割ったときに余りが出る場合 ((dwWriteByteCnt % dwDataSize) != 0)
					if( (((dwWriteByteCnt / dwDataSize) * dwDataSize) < dwWriteCnt )&&((dwWriteByteCnt % dwDataSize) != 0)) {
						if(eAccessSize == MLWORD_ACCESS){
							// 端数データ部は下位2ビットを反転
							dwWriteCnt = (dwWriteCnt & 0xfffffffc) | (~dwWriteCnt & 0x00000003);
						}
						if(eAccessSize == MWORD_ACCESS){
							// 端数データ部は下位1ビットを反転
							dwWriteCnt = (dwWriteCnt & 0xfffffffe) | (~dwWriteCnt & 0x00000001);
						}
					}
					// RevNo120124-001 Append End
				} else {
					dwWriteCnt = (madrTmpStart + i) - madrStart;
				}
				pbyWriteData = const_cast<BYTE*>(pbyWriteBuff + dwWriteCnt);

				byChkMask = pbyChkMask[dwChkCnt];
				byData = (BYTE)(*pbyWriteData & byChkMask);	// オプション設定レジスタの確認ビットでマスク
				byChkData = pbyChkData[dwChkCnt];
				if (byData != byChkData) {
					bMatch = FALSE;	// オプション設定レジスタへのライトデータが不一致
					break;
				}
				dwChkCnt++;
			}
		}
		if (bMatch == FALSE) {
			break;
		}

		madrTmpStart += dwTmpLen;
		dwTotalLength -= dwTmpLen;
	}

	*pdwWriteCnt = dwWriteCnt;

	return bMatch;
}
// V.1.03 RevNo111121-008 Append End

//=============================================================================
/**
 * フラッシュROMのオプション設定メモリ(MDE、UBコード領域、OFS1)へのCWRITEデータのエラーチェックを行う。
 * @param madrStart		CWRITE開始アドレス
 * @param madrEnd		CWRITE終了アドレス
 * @param pbyWriteBuff	書き込みデータ格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
static FFWERR checkOptionSettingMemoryCWrite(MADDR madrStart, MADDR madrEnd, const BYTE* pbyWriteBuff)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	byChkData[OSM_CHKDATA_LENGTH];	// オプション設定レジスタの比較データ(正常データ)格納バッファ	// V.1.03 RevNo111121-008 Modify Line
	BYTE	byChkMask[OSM_CHKMASK_LENGTH];	// オプション設定レジスタデータ比較用マスク値					// V.1.03 RevNo111121-008 Modify Line
	BYTE	byWriteData;
	MADDR	madrChkStart, madrChkEnd;
	BOOL	bMatchMde = TRUE;
	BOOL	bMatchUbcode = TRUE;
	BOOL	bMatch		= FALSE;			// V.1.03 RevNo111121-008 Modify Line
	BOOL	bOverVdsel = TRUE;
	DWORD	dwData,dwMaskData;
	BYTE	byVdsel;						// V.1.03 RevNo111121-008 Modify Line
	// RevRxNo120910-004 Append Start
	BOOL	bStuplvd1renFlg = FALSE;  		// "OFS1.STUPLVD1RENが有効設定フラグ"(TRUE:有効"0"設定、FALSE:無効"1"設定)
	BOOL	bMatchStuplvd1ren = TRUE;		// "OFS1.STUPLVD1RENが一致フラグ"(TRUE:一致、FALSE:不一致)
	// RevRxNo120910-004 Append End

	FFWENM_ENDIAN				eEndian;
	enum FFWRXENM_PMODE	ePmode;				// 起動時に指定された端子設定起動モード
	FFWRX_UBCODE_DATA *pUbcode;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	ePmode = GetPmodeDataRX();
	pUbcode = GetUBCodeDataRX();
	eEndian = GetEndianDataRX();			// エンディアン情報取得

	// RevRxNo130411-001 Delete
	// MDEレジスタ, UBコード領域, OFS1レジスタへのライトデータチェックの実施有無は、
	// FFWMCU_FWCTRL_DATA構造体のeMdeType, bUbcodeExist, eOfsTypeで判断するように変更したため、
	// MCU_TYPE_0のときに何もせず正常終了する処理を削除。

	// 内蔵ROM無効状態の場合、何もせず正常終了する。( 事前にSetPmodeInRomReg2Flg()を実行しておくこと )
	if (GetPmodeInRomDisFlg()) {
		return FFWERR_OK;
	}

	///// MDEレジスタへのライトデータ確認 /////
	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eMdeType == RX_MDE_FLASH) {	// MDEをフラッシュROMで設定するMCUの場合	// RevRxNo130411-001 Modify Line
		// MDEレジスタの比較データ(正常データ)設定
		// V.1.03 RevNo111121-008 Modify Start
		if (eEndian == ENDIAN_LITTLE) {
			dwData = MCU_OSM_MDE_LITTLE_DATA;
		} else {
			dwData = MCU_OSM_MDE_BIG_DATA;
		}
		dwMaskData = MCU_OSM_MDE_MASK_DATA;
		//エンディアンデータをBYTE並びに変換
		ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
		//エンディアンマスクデータをBYTE並びに変換
		ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);
		// V.1.03 RevNo111121-008 Modify End

		// 領域確認アドレス設定
		if (ePmode != RX_PMODE_USRBOOT) {		// シングルチップ起動の場合
			madrChkStart = MCU_OSM_MDES_START;
			madrChkEnd = MCU_OSM_MDES_END;
		} else {								// ユーザブート起動の場合
			madrChkStart = MCU_OSM_MDEB_START;
			madrChkEnd = MCU_OSM_MDEB_END;
		}

		//CWriteデータとの比較
		// V.1.03 RevNo111121-008 Modify Line
		bMatchMde = cmpOptionSettingMemory2CWriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,pbyWriteBuff,&byChkData[0],&byChkMask[0],&byWriteData);
	}

	// ユーザブートモード時、UBコードA,B領域へのライトデータ確認
	// RevRxNo130411-001 Append Line
	if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
		if (ePmode == RX_PMODE_USRBOOT) {

			///// UBコードA領域へのライトデータ確認 /////
			// UBコードA領域の比較データ(正常データ)設定
			// V.1.03 RevNo111121-008 Modify Start
			replaceEndianUBCode2Byte(SET_UBCODE_A,eEndian,&byChkData[0]);
			//マスクデータ作成
			memset(&byChkMask[0],0xFF,OSM_CHKMASK_LENGTH);
			// V.1.03 RevNo111121-008 Modify End

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_UBCODE_A_START;
			madrChkEnd = MCU_OSM_UBCODE_A_END;

			// V.1.03 RevNo111121-008 Modify Line
			bMatchUbcode = cmpOptionSettingMemory2CWriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,pbyWriteBuff,&byChkData[0],&byChkMask[0],&byWriteData);

			///// UBコードB領域へのライトデータ確認 /////
			if (bMatchUbcode != FALSE) {	// UBコードAへのライトデータが不一致でない場合のみ確認する
				// V.1.03 RevNo111121-008 Modify Start
				replaceEndianUBCode2Byte(SET_UBCODE_B,eEndian,&byChkData[0]);

				//マスクデータ作成
				memset(&byChkMask[0],0xFF,OSM_CHKMASK_LENGTH);
				// V.1.03 RevNo111121-008 Modify End

				//アドレス設定
				madrChkStart = MCU_OSM_UBCODE_B_START;
				madrChkEnd = MCU_OSM_UBCODE_B_END;

				// V.1.03 RevNo111121-008 Modify Line
				bMatchUbcode = cmpOptionSettingMemory2CWriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,pbyWriteBuff,&byChkData[0],&byChkMask[0],&byWriteData);
			}
		}
	}

	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eOfsType == RX_OFS_FLASH) {	// OFSレジスタをフラッシュROMで設定するMCUの場合
		// RevRxNo120910-004 Append Line, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_LEVEL_CHK) {	// LVDx有効時の電圧レベルチェックを実施するMCUの場合
			// V.1.03 RevNo111121-008 Append Start
			// OFS1 LVDASが有効設定になっているか確認		
			dwData = MCU_OSM_OFS1_LVDAS_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_LVDAS_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// CWriteデータの比較を実施
			bMatch = cmpOptionSettingMemory2CWriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,pbyWriteBuff,&byChkData[0],&byChkMask[0],&byWriteData);
			// VDSEL有効設定の場合
			if(bMatch == FALSE){
				// VDSEL値取得
				// RevRxNo120910-003 modify Line	// マスクを削除
				byVdsel = (BYTE)(byWriteData);
				// ターゲット電圧とVDSEL設定値の比較
				ferr = CmpOfs1Vdsel2TargetVcc(byVdsel,&bOverVdsel);		// RevRxNo130730-001 Modify Line
				if(ferr != FFWERR_OK){
					return ferr;
				}
			}
			// V.1.03 RevNo111121-008 Append End
		// RevRxNo120910-004 Append Line
		}

		// RevRxNo120910-004 Append Start
		// RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_ENA_CHK) {	// LVDx有効設定チェックを実施するMCUの場合
			// OFS1 STUPLVD1RENが有効設定になっているか確認		
			dwData = MCU_OSM_OFS1_STUPLVD1REN_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_STUPLVD1REN_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// CWriteデータの比較を実施
			bMatchStuplvd1ren = cmpOptionSettingMemory2CWriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,pbyWriteBuff,&byChkData[0],&byChkMask[0],&byWriteData);

			// STUPLVD1REN有効設定の場合
			if (bMatchStuplvd1ren == FALSE) {
				bStuplvd1renFlg = TRUE;  // "OFS1.STUPLVD1RENが有効設定フラグ"に"有効設定(TRUE)"を設定
			}
		}
		// RevRxNo120910-004 Append End
	}

	if (bMatchMde == FALSE) {			// MDE不一致の場合
		ferr = FFWERR_WRITE_MDE_NOTMATCH;
	} else if (bMatchUbcode == FALSE) {	// UBコード不一致の場合
		ferr = FFWERR_WRITE_UBCODE_NOTMATCH;
	// V.1.03 RevNo111121-008 Modify Start
	} else if (bOverVdsel == FALSE) {						// ターゲット電圧がVDSEL以下の設定の場合
		ferr = FFWERR_WRITE_OFS1_TARGETVCC_UNDERVDSEL;
	// RevRxNo120910-004 Append Start
	} else if (bStuplvd1renFlg == TRUE) {					// OFS1.STUPLVD1RENが有効("0")設定の場合
		ferr = FFWERR_WRITE_OFS1_LVD1REN_ENA;	// エラー：OFS1レジスタへの起動時電圧監視1リセット有効設定は不可である
	// RevRxNo120910-004 Append End
	}
	// V.1.03 RevNo111121-008 Modify End

	return ferr;
}
// V.1.03 RevNo111121-008 Append Start
//=============================================================================
/**
 * CWRITEデータの比較を実施
 * @param madrStart		CWRITE開始アドレス
 * @param madrEnd		CWRITE終了アドレス
 * @param madrChkStart		比較開始アドレス
 * @param madrChkEnd		比較終了アドレス
 * @param pbyWriteBuff[]	CWRITEデータ格納ポインタ(BYTE並び)
 * @param pbyChkData[]	比較データ格納ポインタ(BYTE並び)
 * @param pbyChkMaskData[]	比較マスクデータ格納ポインタ(BYTE並び)
 * @param pdwCwriteCnt	データ位置格納ポインタ
 * @return FFWエラーコード
 */
//=============================================================================
static BOOL cmpOptionSettingMemory2CWriteData(MADDR madrStart,MADDR madrEnd,MADDR madrChkStart,MADDR madrChkEnd,const BYTE* pbyWriteBuff,BYTE* pbyChkData,BYTE* pbyChkMask,BYTE* pbyCwriteData)
{
	
	DWORD	dwTotalLength;
	MADDR	madrCwriteStart,madrCwriteEnd;
	MADDR	madrAddr;
	MADDR	madrTmpStart,madrTmpEnd;
	BYTE*	pbyCwriteBuff;
	BYTE*	pbyWriteData;
	BOOL	bBigEndFlg;
	BOOL	bMatch = TRUE;
	BOOL	bChkArea;
	DWORD	dwData;
	DWORD	dwChkCnt;
	DWORD	dwTmpLen;
	DWORD	i;
	DWORD	dwWriteCnt = 0;
	BYTE	byData;
	BYTE	byWriteData = 0;
	FFWENM_ENDIAN				eEndian;

	dwTotalLength = madrEnd - madrStart + 1;
	madrCwriteStart = madrStart;
	madrCwriteEnd = madrEnd;
	pbyCwriteBuff = const_cast<BYTE*>(pbyWriteBuff);

	eEndian = GetEndianDataRX();			// エンディアン情報取得

	bBigEndFlg = FALSE;
	// ビッグエンディアンの場合
	if (eEndian == ENDIAN_BIG) {
		// 開始アドレスが4の倍数でない場合、4の倍数の区切りまで処理する。
		dwData = madrCwriteStart & 0x000000003;	// CWRITEアドレス下位2ビット
		while ((dwData != 0x00000000) && (dwTotalLength > 0)) {
			// CWRITEアドレスをMCUアドレスに変換
				// 開始アドレスは、小さい順に処理をする。
			if (dwData == 0x00000001) {
				madrAddr = madrCwriteStart + 1;
			} else if (dwData == 0x00000002) {
				madrAddr = madrCwriteStart - 1;
			} else {	// dwData == 0x00000003
				madrAddr = madrCwriteStart - 3;
			}

			if ((madrChkStart <= madrAddr) && (madrAddr <= madrChkEnd)) {	
				dwChkCnt = madrAddr - madrChkStart;				// オプション設定レジスタデータバッファ位置算出
				pbyWriteData = pbyCwriteBuff;					// CWRITEデータバッファ位置
				byData = (BYTE)(*pbyWriteData & pbyChkMask[dwChkCnt]);	// オプション設定レジスタの確認ビットでマスク
				if (byData != pbyChkData[dwChkCnt]) {
					bMatch = FALSE;	// オプション設定レジスタへのライトデータが不一致
					byWriteData = *pbyWriteData;
					break;
				}
			}
			madrCwriteStart++;
			pbyCwriteBuff++;
			dwTotalLength--;

			dwData = madrCwriteStart & 0x000000003;	// CWRITEアドレス下位2ビット
		}

		if (bMatch != FALSE) {
			// 終了アドレスが(4の倍数+3)でない場合、端数分は後で処理する。
			if (dwTotalLength > 0) {
				dwData = madrCwriteEnd & 0x000000003;	// CWRITEアドレス下位2ビット
				if (dwData != 0x00000003) {
					madrCwriteEnd = madrCwriteEnd - (dwData + 1);
					dwTotalLength = dwTotalLength - (dwData + 1);
					bBigEndFlg = TRUE;
				}
			}
		}
	}


	// リトルエンディアン または ビッグエンディアンで開始アドレスが4の倍数の場合の処理
	madrTmpStart = madrCwriteStart;
	while (dwTotalLength > 0) {
		if (bMatch == FALSE) {
			break;
		}
		checkArea(madrTmpStart, madrCwriteEnd, madrChkStart, madrChkEnd, &madrTmpEnd, &bChkArea);
		dwTmpLen = madrTmpEnd - madrTmpStart + 1;

		if (bChkArea == TRUE) {	// オプション設定レジスタアドレスが含まれる場合
			// オプション設定レジスタデータバッファ位置算出
			dwChkCnt = madrTmpStart - madrChkStart;

			for (i = 0; i < dwTmpLen; i++) {
				// ライトデータバッファ位置算出
				if (eEndian == ENDIAN_LITTLE) {
					dwWriteCnt = (madrTmpStart + i) - madrCwriteStart;
				} else {
					madrAddr = (madrTmpStart + i);
					dwData = madrAddr & 0x000000003;	// CWRITEアドレス下位2ビット
					if (dwData == 0x00000000) {
						madrAddr = madrAddr + 3;
					} else if (dwData == 0x00000001) {
						madrAddr = madrAddr + 1;
					} else if (dwData == 0x00000002) {
						madrAddr = madrAddr - 1;
					} else {
						madrAddr = madrAddr - 3;
					}
					dwWriteCnt = madrAddr - madrCwriteStart;
				}
				pbyWriteData = pbyCwriteBuff + dwWriteCnt;

				byData = (BYTE)(*pbyWriteData & pbyChkMask[dwChkCnt]);	// オプション設定レジスタの確認ビットでマスク
				if (byData != pbyChkData[dwChkCnt]) {
					bMatch = FALSE;	// オプション設定レジスタへのライトデータが不一致
					byWriteData = *pbyWriteData;
					break;
				}
				dwChkCnt++;
			}
		}
		madrTmpStart += dwTmpLen;
		dwTotalLength -= dwTmpLen;
	}

	madrCwriteStart = madrCwriteEnd + 1;
	pbyCwriteBuff = const_cast<BYTE*>(pbyWriteBuff) + (madrCwriteStart - madrStart);

	// ビッグエンディアンで終了アドレスが(4の倍数+3)でない場合
	if ((bBigEndFlg == TRUE) && (bMatch == TRUE)) {
		dwTotalLength = madrEnd - madrCwriteStart + 1;

		dwData = madrCwriteStart & 0x000000003;	// CWRITEアドレス下位2ビット
		while (dwTotalLength > 0) {
			if (dwData == 0x00000000) {
				madrAddr = madrCwriteStart + 3;
			} else if (dwData == 0x00000001) {
				madrAddr = madrCwriteStart + 1;
			} else {	// dwData == 0x00000002
				madrAddr = madrCwriteStart - 1;
			}

			if ((madrChkStart <= madrAddr) && (madrAddr <= madrChkEnd)) {	
				dwChkCnt = madrAddr - madrChkStart;				// オプション設定レジスタデータバッファ位置算出
				pbyWriteData = pbyCwriteBuff;					// CWRITEデータバッファ位置
				byData = (BYTE)(*pbyWriteData & pbyChkMask[dwChkCnt]);	// オプション設定レジスタの確認ビットでマスク
				if (byData != pbyChkData[dwChkCnt]) {
					bMatch = FALSE;	// オプション設定レジスタへのライトデータが不一致
					byWriteData = *pbyWriteData;
					break;
				}
			}
			madrCwriteStart++;
			pbyCwriteBuff++;
			dwTotalLength--;

			dwData = madrCwriteStart & 0x000000003;	// CWRITEアドレス下位2ビット
		}
	}

	*pbyCwriteData = byWriteData;

	return bMatch;
}
// V.1.03 RevNo111121-008 Append End

//=============================================================================
/**
 * フラッシュROMのオプション設定メモリ(MDE、UBコード領域、OFS1)へのMOVEデータのエラーチェックを行う。
 * @param dwmadrSrcStartAddr 転送元開始アドレス
 * @param dwmadrSrcEndAddr 転送元終了アドレス
 * @param dwmadrDisStartAddr 転送先開始アドレス
 * @param eAccessSize アクセスサイズ
 * @retval FFWエラーコード
 */
//=============================================================================
// V.1.03 RevNo111121-008 Modify Start
static FFWERR checkOptionSettingMemoryMove(MADDR dwmadrSrcStartAddr, MADDR dwmadrSrcEndAddr, 
								 MADDR dwmadrDisStartAddr, enum FFWENM_MACCESS_SIZE eAccessSize)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	byChkData[OSM_CHKDATA_LENGTH];	// オプション設定レジスタの比較データ(正常データ)格納バッファ	// V.1.03 RevNo111121-008 Modify Line
	BYTE	byChkMask[OSM_CHKMASK_LENGTH];	//オプション設定レジスタデータ比較用マスク値					// V.1.03 RevNo111121-008 Modify Line
	BYTE	byUnmatchData;
	BYTE	byVdsel;						// V.1.03 RevNo111121-008 Modify Line
	MADDR	madrChkStart, madrChkEnd;
	BOOL	bMatchMde = TRUE;
	BOOL	bMatchUbcode = TRUE;
	BOOL	bMatch = FALSE;					// V.1.03 RevNo111121-008 Modify Line
	BOOL	bOverVdsel = TRUE;				// V.1.03 RevNo111121-008 Modify Line
	DWORD	dwData;
	DWORD	dwMaskData;
	// RevRxNo120910-004 Append Start
	BOOL	bStuplvd1renFlg = FALSE;  		// "OFS1.STUPLVD1RENが有効設定フラグ"(TRUE:有効"0"設定、FALSE:無効"1"設定)
	BOOL	bMatchStuplvd1ren = TRUE;		// "OFS1.STUPLVD1RENが一致フラグ"(TRUE:一致、FALSE:不一致)
	// RevRxNo120910-004 Append End

	FFWENM_ENDIAN				eEndian;
	enum FFWRXENM_PMODE	ePmode;				// 起動時に指定された端子設定起動モード
	FFWRX_UBCODE_DATA *pUbcode;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	ePmode = GetPmodeDataRX();
	pUbcode = GetUBCodeDataRX();
	eEndian = GetEndianDataRX();			// エンディアン情報取得

	// RevRxNo130411-001 Delete
	// MDEレジスタ, UBコード領域, OFS1レジスタへのライトデータチェックの実施有無は、
	// FFWMCU_FWCTRL_DATA構造体のeMdeType, bUbcodeExist, eOfsTypeで判断するように変更したため、
	// MCU_TYPE_0のときに何もせず正常終了する処理を削除。

	// 内蔵ROM無効状態の場合、何もせず正常終了する。( 事前にSetPmodeInRomReg2Flg()を実行しておくこと )
	if (GetPmodeInRomDisFlg()) {
		return FFWERR_OK;
	}

	///// MDEレジスタへのライトデータ確認 /////
	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eMdeType == RX_MDE_FLASH) {	// MDEをフラッシュROMで設定するMCUの場合	// RevRxNo130411-001 Modify Line
		// MDEレジスタの比較データ(正常データ)設定
		// V.1.03 RevNo111121-008 Modify Start
		if (eEndian == ENDIAN_LITTLE) {
			dwData = MCU_OSM_MDE_LITTLE_DATA;
		} else {
			dwData = MCU_OSM_MDE_BIG_DATA;
		}
		dwMaskData = MCU_OSM_MDE_MASK_DATA;
		//エンディアンデータをBYTE並びに変換
		ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
		//エンディアンマスクデータをBYTE並びに変換
		ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);
		// V.1.03 RevNo111121-008 Modify End

		// 領域確認アドレス設定
		if (ePmode != RX_PMODE_USRBOOT) {		// シングルチップ起動
			madrChkStart = MCU_OSM_MDES_START;
			madrChkEnd = MCU_OSM_MDES_END;
		} else {								// ユーザブート起動
			madrChkStart = MCU_OSM_MDEB_START;
			madrChkEnd = MCU_OSM_MDEB_END;
		}
		//Moveデータとの比較
		// V.1.03 RevNo111121-008 Modify Line
		ferr = cmpOptionSettingMemory2MoveData(dwmadrSrcStartAddr,dwmadrSrcEndAddr,dwmadrDisStartAddr,madrChkStart,madrChkEnd, eAccessSize,&byChkData[0],&byChkMask[0],&byUnmatchData,&bMatchMde);
		if(ferr != FFWERR_OK){
			return ferr;
		}
		// V.1.03 RevNo111121-008 Modify End
	}

	// ユーザブートモード時、UBコードA,B領域へのライトデータ確認
	// RevRxNo130411-001 Append Line
	if (pFwCtrl->bUbcodeExist == TRUE) {	// UBコードがあるMCUの場合
		if (ePmode == RX_PMODE_USRBOOT) {
			///// UBコードA領域へのライトデータ確認 /////
			// UBコードA領域の比較データ(正常データ)設定
			// V.1.03 RevNo111121-008 Modify Start
			replaceEndianUBCode2Byte(SET_UBCODE_A,eEndian,&byChkData[0]);

			//マスクデータ作成
			memset(&byChkMask[0],0xFF,OSM_CHKMASK_LENGTH);
			// V.1.03 RevNo111121-008 Modify End

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_UBCODE_A_START;
			madrChkEnd = MCU_OSM_UBCODE_A_END;
			//Moveデータとの比較
			// V.1.03 RevNo111121-008 Modify Start
			ferr = cmpOptionSettingMemory2MoveData(dwmadrSrcStartAddr,dwmadrSrcEndAddr,dwmadrDisStartAddr,madrChkStart,madrChkEnd,eAccessSize,&byChkData[0],&byChkMask[0],&byUnmatchData,&bMatchUbcode);
			if(ferr != FFWERR_OK){
				return ferr;
			}
			// V.1.03 RevNo111121-008 Modify End

			///// UBコードB領域へのライトデータ確認 /////
			if (bMatchUbcode != FALSE) {	// UBコードAへのライトデータが不一致でない場合のみ確認する
				// V.1.03 RevNo111121-008 Modify Start
				replaceEndianUBCode2Byte(SET_UBCODE_B,eEndian,&byChkData[0]);
				//マスクデータ作成
				memset(&byChkMask[0],0xFF,OSM_CHKMASK_LENGTH);
				// V.1.03 RevNo111121-008 Modify End
				madrChkStart = MCU_OSM_UBCODE_B_START;
				madrChkEnd = MCU_OSM_UBCODE_B_END;
			
				//Moveデータとの比較
				// V.1.03 RevNo111121-008 Modify Start
				ferr = cmpOptionSettingMemory2MoveData(dwmadrSrcStartAddr,dwmadrSrcEndAddr,dwmadrDisStartAddr,madrChkStart,madrChkEnd,eAccessSize,&byChkData[0],&byChkMask[0],&byUnmatchData,&bMatchUbcode);
				if(ferr != FFWERR_OK){
					return ferr;
				}
				// V.1.03 RevNo111121-008 Modify End
			}
		}
	}

	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eOfsType == RX_OFS_FLASH) {	// OFSレジスタをフラッシュROMで設定するMCUの場合
		// RevRxNo120910-004 Append Line, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_LEVEL_CHK) {	// LVDx有効時の電圧レベルチェックを実施するMCUの場合
			// V.1.03 RevNo111121-008 Append Start
			// OFS1 LVDASが有効設定になっているか確認
			dwData = MCU_OSM_OFS1_LVDAS_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_LVDAS_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// Moveデータの比較を実施
			ferr = cmpOptionSettingMemory2MoveData(dwmadrSrcStartAddr,dwmadrSrcEndAddr,dwmadrDisStartAddr,madrChkStart,madrChkEnd,eAccessSize,&byChkData[0],&byChkMask[0],&byUnmatchData,&bMatch);
			if(ferr != FFWERR_OK){
				return ferr;
			}
			// VDSEL有効設定の場合
			if(bMatch == FALSE){
				// VDSEL値取得
				// RevRxNo120910-003 modify Line	// マスクを削除
				byVdsel = (BYTE)(byUnmatchData);
				// ターゲット電圧とVDSEL設定値の比較
				ferr = CmpOfs1Vdsel2TargetVcc(byVdsel,&bOverVdsel);		// RevRxNo130730-001 Modify Line
				if(ferr != FFWERR_OK){
					return ferr;
				}
			}
			// V.1.03 RevNo111121-008 Append End
		// RevRxNo120910-004 Append Line
		}

		// RevRxNo120910-004 Append Start
		// RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_ENA_CHK) {	// LVDx有効設定チェックを実施するMCUの場合
			// OFS1 STUPLVD1RENが有効設定になっているか確認		
			dwData = MCU_OSM_OFS1_STUPLVD1REN_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_STUPLVD1REN_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// Moveデータの比較を実施
			ferr = cmpOptionSettingMemory2MoveData(dwmadrSrcStartAddr,dwmadrSrcEndAddr,dwmadrDisStartAddr,madrChkStart,madrChkEnd,eAccessSize,&byChkData[0],&byChkMask[0],&byUnmatchData,&bMatchStuplvd1ren);
			if(ferr != FFWERR_OK){
				return ferr;
			}

			// STUPLVD1REN有効設定の場合
			if (bMatchStuplvd1ren == FALSE) {
				bStuplvd1renFlg = TRUE;  // "OFS1.STUPLVD1RENが有効設定フラグ"に"有効設定(TRUE)"を設定
			}
		}
		// RevRxNo120910-004 Append End
	}

	if (bMatchMde == FALSE) {			// MDE不一致の場合
		ferr = FFWERR_WRITE_MDE_NOTMATCH;
	} else if (bMatchUbcode == FALSE) {	// UBコード不一致の場合
		ferr = FFWERR_WRITE_UBCODE_NOTMATCH;
	// V.1.03 RevNo111121-008 Modify Start
	} else if (bOverVdsel == FALSE) {						// ターゲット電圧がVDSEL以下の設定の場合
		ferr = FFWERR_WRITE_OFS1_TARGETVCC_UNDERVDSEL;
	// RevRxNo120910-004 Append Start
	} else if (bStuplvd1renFlg == TRUE) {					// OFS1.STUPLVD1RENが有効("0")設定の場合
		ferr = FFWERR_WRITE_OFS1_LVD1REN_ENA;	// エラー：OFS1レジスタへの起動時電圧監視1リセット有効設定は不可である
	// RevRxNo120910-004 Append End
	}
	// V.1.03 RevNo111121-008 Modify End

	return ferr;
}
// V.1.03 RevNo111121-008 Append Start
//=============================================================================
/**
 * MOVEデータの比較を実施
 * @param madrSrcStart		Move Source 開始アドレス
 * @param madrSrcEnd		Move Source 終了アドレス
 * @param madrDisStart		Move Disteination 開始アドレス
 * @param pbyChkData[]		比較データ格納ポインタ(BYTE並び)
 * @param pbyChkMaskData[]	比較マスクデータ格納ポインタ(BYTE並び)
 * @param pbyMoveData		データが違う場合のエラーデータ
 * @param pbMatch			比較結果格納フラグ
 * @return FFWエラーコード
 */
//=============================================================================
static FFWERR cmpOptionSettingMemory2MoveData(MADDR dwmadrSrcStartAddr,MADDR dwmadrSrcEndAddr,MADDR dwmadrDisStartAddr,MADDR madrChkStart,MADDR madrChkEnd,enum FFWENM_MACCESS_SIZE eAccessSize,BYTE* pbyChkData,BYTE* pbyChkMask,BYTE* pbyMoveData,BOOL* pbMatch)
{
	FFWERR	ferr = FFWERR_OK;
	// RevNo120123-001 Modify Start
	BYTE*	pbyDumpData;	// DUMPデータ格納バッファポインタ
	BYTE	byDumpDataTmp=0;	//不一致データ格納バッファ 
	DWORD	dwDumpDataByteLength;	//DUMPデータリードバッファのバイトサイズ
	// RevNo120123-001 Modify End
	DWORD	dwChkCnt;
	DWORD	dwMoveCnt = 0;
	DWORD	dwBufCnt;
	DWORD	dwBufCntEnd;	//DUMP終了アドレス補正バッファ	// RevNo120123-001 Append Line
	DWORD	dwTotalLength;
	DWORD	dwTmpLen;
	MADDR	madrEnd;
	MADDR	madrTmpStart, madrTmpEnd;
	BOOL	bChkArea;
	BOOL	bMatch = TRUE;
	DWORD	i;
	BYTE	byData;
	DWORD	dwAddrOffset;
	DWORD	dwDataSize;
	MADDR	madrReadStart;
	MADDR	madrReadEnd;

	FFWENM_ENDIAN				eEndian;

	eEndian = GetEndianDataRX();			// エンディアン情報取得
	
	switch (eAccessSize) {
	case MBYTE_ACCESS:
		dwDataSize = 1;
		break;
	case MWORD_ACCESS:
		dwDataSize = 2;
		break;
	case MLWORD_ACCESS:
		dwDataSize = 4;
		break;
	default:
		dwDataSize = 4;
		break;	// ここは通らない
	}

	// V.1.03 RevNo111121-008 Modify End
	dwTotalLength = dwmadrSrcEndAddr - dwmadrSrcStartAddr + 1;
	madrTmpStart = dwmadrDisStartAddr;
	madrEnd = dwmadrDisStartAddr + (dwTotalLength - 1);

	while (dwTotalLength) {
		checkArea(madrTmpStart, madrEnd, madrChkStart, madrChkEnd, &madrTmpEnd, &bChkArea);
		dwTmpLen = madrTmpEnd - madrTmpStart + 1;

		if (bChkArea == TRUE) {	// 転送先がオプション設定レジスタの場合
			// データバッファ位置算出
			dwChkCnt = madrTmpStart - madrChkStart;


			// 転送元データDUMP開始アドレス、終了アドレスを設定
			// なお、DUMP開始アドレスは、転送元先頭アドレスからアクセスサイズの区切りになるよう設定する。
			dwAddrOffset = madrTmpStart - dwmadrDisStartAddr;	// MOVE先頭アドレスからのオフセットアドレス
			madrReadStart = dwmadrSrcStartAddr + dwAddrOffset;
			// MOVEデータ(DUMPデータ)格納バッファ先頭アドレスからのオフセットを計算
			dwBufCnt = 0;
			if (eAccessSize == MWORD_ACCESS) {
				if (dwAddrOffset & 0x00000001) {	// MOVE先頭アドレスから奇数番目の場合
					dwBufCnt = 1;
					madrReadStart -= 1;			// 偶数番目のアドレスからDUMP
				}
			} else if (eAccessSize == MLWORD_ACCESS) {
				if (dwAddrOffset & 0x00000003) {	// MOVE先頭アドレスから4の倍数番目でない場合
					dwBufCnt = dwAddrOffset & 0x00000003;
					madrReadStart -= dwBufCnt;	// 4の倍数番目のアドレスからDUMP
				}
			}

			dwAddrOffset = madrTmpEnd - dwmadrDisStartAddr;
			madrReadEnd = dwmadrSrcStartAddr + dwAddrOffset;	// 転送元データDUMP終了アドレス
			
			// RevNo120123-001 Append Start
			//DUMPバイト数を算出
			dwDumpDataByteLength = madrReadEnd - madrReadStart + 1;

			//DUMPBYTE数が、WORD:奇数個　LWORD:4の倍数ではない場合、データの比較処理が以下のように動作するため
			//LITTLEエンディアンでうまく比較できない場合がある。
			//このため、DUMPデータ数をWORD:偶数個　LWORD:4の倍数個に増やす必要がある。
			// ex. DUMPでアドレス0～5(DUMPバイト長が4n+2)のデータをLWORDでDUMPした場合
			// DO_DUMPでDUMPしたデータは、以下のように取得される。(データはアドレス)
			// pbyDumpData |0|1|2|3|4|5|6|7|
			// データ　　　|3|2|1|0|4|5|x|x|(xはDUMPデータではないため値は不定)
			// LITTLEエンディアンではデータ比較時に、
			// pbyDumpData[3]→pbyDumpData[2]→pbyDumpData[1]→pbyDumpData[0]→pbyDumpData[7]→pbyDumpData[6]・・・
			// の順で比較するため、5番地のデータ比較がpbyDumpData[7]の不定値と比較してしまい、正常データでもエラーになる。
			// このため、DUMPデータ数をWORD:偶数個　LWORD:4の倍数個に増やす必要がある。DUMPでアドレス0～7(DUMPバイト長が4n)の場合
			// pbyDumpData |0|1|2|3|4|5|6|7|
			// データ　　　|3|2|1|0|7|6|5|4|
			if (eAccessSize == MWORD_ACCESS) {
				if (dwDumpDataByteLength & 0x00000001) {	// DUMPデータが奇数個の場合
					madrReadEnd += 1;			// 偶数個のアドレスまでDUMP
					//DUMPバイト数が変わるので再度算出
					dwDumpDataByteLength = madrReadEnd - madrReadStart + 1;
				}
			} else if (eAccessSize == MLWORD_ACCESS) {
				if (dwDumpDataByteLength & 0x00000003) {	// MOVE先頭アドレスから4倍数個でない場合
					dwBufCntEnd = dwDumpDataByteLength & 0x00000003;
					dwBufCntEnd = 4-dwBufCntEnd;
					madrReadEnd += dwBufCntEnd;	// 4の倍数番目のアドレスからDUMP
					//DUMPバイト数が変わるので再度算出
					dwDumpDataByteLength = madrReadEnd - madrReadStart + 1;
				}
			}
			
			pbyDumpData = new BYTE [dwDumpDataByteLength];
			// RevNo120123-001 Append End

			// 転送元データをDUMP
			// RevNo120123-001 Modify Start
			if ((ferr = DO_DUMP(madrReadStart, madrReadEnd, eAccessSize, &pbyDumpData[0])) != FFWERR_OK) {
				delete [] pbyDumpData;
				return ferr;
			}
			// RevNo120123-001 Modiy End

			for (i = 0; i < dwTmpLen; i++) {
				// ライトデータバッファ位置算出
				if (eEndian == ENDIAN_LITTLE) {
					dwMoveCnt = (((dwBufCnt / dwDataSize) + 1) * dwDataSize) - (dwBufCnt % dwDataSize) - 1;
				} else {
					dwMoveCnt = dwBufCnt;
				}

				// RevNo120123-001 Modify Start
				byData = (BYTE)(pbyDumpData[dwMoveCnt] & pbyChkMask[dwChkCnt]);	// オプション設定レジスタの確認ビットでマスク
				if (byData != pbyChkData[dwChkCnt]) {
					byDumpDataTmp = pbyDumpData[dwMoveCnt];
					// RevNo120123-001 Modify End
					bMatch = FALSE;	// オプション設定レジスタへのライトデータが不一致
					break;
				}
				dwBufCnt++;
				dwChkCnt++;
			}
			delete [] pbyDumpData;			// RevNo120123-001 Append Line
		}

		if (bMatch == FALSE) {
			break;
		}

		madrTmpStart += dwTmpLen;
		dwTotalLength -= dwTmpLen;
	}

	*pbMatch = bMatch;
	*pbyMoveData = byDumpDataTmp;	// RevNo120123-001 Modify Line

	return ferr;
}
//=============================================================================
/**
 * フラッシュROMのオプション設定メモリ(MDE、UBコード領域、OFS1)へのDWNP時のWRITEデータのエラーチェックを行う。
 * @param madrStart		WRITE開始アドレス
 * @param madrEnd		WRITE終了アドレス
 * @param eAccessSize	アクセスサイズ
 * @param pbyWriteBuff	書き込みデータ格納バッファアドレス
 * @return FFWエラーコード
 */
//=============================================================================
static FFWERR checkOptionSettingMemoryDwnp(MADDR madrStart, MADDR madrEnd, enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff)
{

	FFWERR	ferr = FFWERR_OK;
	BYTE	byChkData[OSM_CHKDATA_LENGTH];	// オプション設定レジスタの比較データ(正常データ)格納バッファ	// V.1.03 RevNo111121-008 Modify Line
	BYTE	byChkMask[OSM_CHKMASK_LENGTH];	// オプション設定レジスタデータ比較用マスク値					// V.1.03 RevNo111121-008 Modify Line
	DWORD	dwWriteCnt;
	MADDR	madrChkStart, madrChkEnd;
	BOOL	bMatch = FALSE;					// V.1.03 RevNo111121-008 Modify Line
	BOOL	bOverVdsel = TRUE;				// V.1.03 RevNo111121-008 Modify Line
	BYTE	byVdsel;						// V.1.03 RevNo111121-008 Modify Line
	DWORD	dwData;
	DWORD	dwMaskData;						// V.1.03 RevNo111121-008 Modify Line

	FFWENM_ENDIAN				eEndian;
	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ宣言削除
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWMCU_MCUDEF_DATA* pMcuDef;	// RevRxNo180228-001 Append Line

	// RevRxNo130411-001 Delete：FFWMCU_MCUINFO_DATA構造体ポインタ取得処理削除
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line
	eEndian = GetEndianDataRX();			// エンディアン情報取得
	pMcuDef = GetMcuDefData();	// RevRxNo180228-001 Append Line

	// RevRxNo130411-001 Delete
	// OFS1レジスタへのライトデータチェックの実施有無は、
	// FFWMCU_FWCTRL_DATA構造体のeOfsType, eLvdErrTypeで判断するように変更したため、
	// MCU_TYPE_0のときに何もせず正常終了する処理を削除。

	// 内蔵ROM無効状態の場合、何もせず正常終了する。( 事前にSetPmodeInRomReg2Flg()を実行しておくこと )
	if (GetPmodeInRomDisFlg()) {
		return FFWERR_OK;
	}

	// RevRxNo130411-001 Append Line
	if (pFwCtrl->eOfsType == RX_OFS_FLASH) {	// OFSレジスタをフラッシュROMで設定するMCUの場合
		// RevRxNo120910-004 Append Line, RevRxNo130411-001 Modify Line
		if (pFwCtrl->eLvdErrType == RX_LVD_LEVEL_CHK) {	// LVDx有効時の電圧レベルチェックを実施するMCUの場合
			// V.1.03 RevNo111121-008 Append Start
			// OFS1 LVDASが有効設定になっているか確認		
			dwData = MCU_OSM_OFS1_LVDAS_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_LVDAS_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData,&byChkData[0],eEndian,1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData,&byChkMask[0],eEndian,1);

			// 領域確認アドレス設定
			madrChkStart = MCU_OSM_OFS1_START;
			madrChkEnd = MCU_OSM_OFS1_END;

			// WRITEデータの比較を実施
			bMatch = cmpOptionSettingMemory2WriteData(madrStart,madrEnd,madrChkStart,madrChkEnd,eAccessSize,pbyWriteBuff,&byChkData[0],&byChkMask[0],&dwWriteCnt); 	//RevNo120124-001 Modify Line
			// VDSEL有効設定の場合
			if(bMatch == FALSE){
				// VDSEL値取得
				// RevRxNo120910-003 modify Line	// マスクを削除
				byVdsel = (BYTE)(pbyWriteBuff[dwWriteCnt]);
				// ターゲット電圧とVDSEL設定値の比較
				ferr = CmpOfs1Vdsel2TargetVcc(byVdsel,&bOverVdsel);		// RevRxNo130730-001 Modify Line
				if(ferr != FFWERR_OK){
					return ferr;
				}
			}
		// RevRxNo120910-004 Append Line
		}
	// RevRxNo180228-001 Append Start
	} else if (pFwCtrl->eOfsType == RX_OFS_EXTRA) {	// OFSレジスタをExtra領域で設定するMCUの場合
		if (pFwCtrl->eLvdErrType == RX_LVD_LEVEL_CHK) {	// LVDx有効時の電圧レベルチェックを実施するMCUの場合
														// OFS1 LVDASが有効設定になっているか確認		
			dwData = MCU_OSM_OFS1_LVDAS_DIS_DATA;
			dwMaskData = MCU_OSM_OFS1_LVDAS_MASK_DATA;
			//エンディアンデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwData, &byChkData[0], eEndian, 1);
			//エンディアンマスクデータをBYTE並びに変換
			ReplaceEndianLWord2Byte(&dwMaskData, &byChkMask[0], eEndian, 1);

			// 領域確認アドレス設定
			madrChkStart = pMcuDef->madrOfs1StartAddr;
			madrChkEnd = pMcuDef->madrOfs1StartAddr + 3;

			// WRITEデータの比較を実施
			bMatch = cmpOptionSettingMemory2WriteData(madrStart, madrEnd, madrChkStart, madrChkEnd, eAccessSize, pbyWriteBuff, &byChkData[0], &byChkMask[0], &dwWriteCnt);
																																						// VDSEL有効設定の場合
			if (bMatch == FALSE) {
				// VDSEL値取得
				// マスクを削除
				byVdsel = (BYTE)(pbyWriteBuff[dwWriteCnt]);
				// ターゲット電圧とVDSEL設定値の比較
				ferr = CmpOfs1Vdsel2TargetVcc(byVdsel, &bOverVdsel);
				if (ferr != FFWERR_OK) {
					return ferr;
				}
			}
		}
	// RevRxNo180228-001 Append End
	}

	if (bOverVdsel == FALSE) {						// ターゲット電圧がVDSEL以下の設定の場合
		ferr = FFWERR_WRITE_OFS1_TARGETVCC_UNDERVDSEL;
	}

	return ferr;
}
// V.1.03 RevNo111121-008 Append End

//=============================================================================
/**
 * 領域A(madrStart～madrEnd)が領域B(madrChkStart～madrChkEnd)の領域であるかを判定する。
 * 領域Aと領域Bが異なる場合、領域Aの開始アドレスと同じ領域属性の終了アドレスを返送する。
 * @param madrStart		領域A開始アドレス
 * @param madrEnd		領域A終了アドレス
 * @param madrChkStart	領域B開始アドレス
 * @param madrChkEnd	領域B終了アドレス
 * @param pmadrTmpEnd	領域A開始アドレスと同じ領域属性の終了アドレス格納変数へのポインタ
 * @param pbChkArea		領域A開始アドレスの属性を格納する変数へのポインタ
 *						(領域A開始アドレスが領域Bである場合TRUEを返す)
 * @retval なし
 */
//=============================================================================
static void checkArea(MADDR madrStart, MADDR madrEnd, MADDR madrChkStart, MADDR madrChkEnd, 
					  MADDR* pmadrTmpEnd, BOOL* pbChkArea)
{
	BOOL	bAreaChk;

	bAreaChk = FALSE;
	if (madrEnd < madrChkStart) {			// madrStart <= madrEnd < madrChkStart <= madrChkEnd
		*pmadrTmpEnd = madrEnd;
		*pbChkArea = FALSE;		// チェック対象領域でない
		bAreaChk = TRUE;		// 領域判定済み

	} else if (madrStart < madrChkStart) {	// madrStart < madrChkStart <= madrEnd
		*pmadrTmpEnd = madrChkStart - 1;
		*pbChkArea = FALSE;		// チェック対象領域でない
		bAreaChk = TRUE;		// 領域判定済み

	} else if (madrStart <= madrChkEnd) {
		if (madrEnd <= madrChkEnd) {		// madrChkStart <= madrStart <= madrEnd <= madrChkEnd
			*pmadrTmpEnd = madrEnd;
			*pbChkArea = TRUE;	// チェック対象領域である
			bAreaChk = TRUE;	// 領域判定済み
		} else {							// madrChkStart<= madrStart <= madrChkEnd < madrEnd
			*pmadrTmpEnd = madrChkEnd;
			*pbChkArea = TRUE;	// チェック対象領域である
			bAreaChk = TRUE;	// 領域判定済み
		}
	}
	if (bAreaChk != TRUE) {		// 領域判定済みでない場合(madrChkStart <= madrChkEnd < madrStart <= madrEnd)
		*pmadrTmpEnd = madrEnd;
		*pbChkArea = FALSE;		// チェック対象領域でない
	}
	return;
}
// V.1.02 新デバイス対応 Append End
// V.1.03 RevNo111121-008 Append Start
//=============================================================================
/**
 * UBコードをバイト並びのデータ列に渡す
 * @param bySetUbcode		
 * @param pbOverVdsel	比較結果格納ポインタ ターゲット電源がVDSEL設定以上である:TRUE 以下:FALSE
 * @return FFWERR
 */
//=============================================================================
static void	replaceEndianUBCode2Byte(BYTE bySetUbcode, FFWENM_ENDIAN eEndian,BYTE* pbyData){

	FFWRX_UBCODE_DATA *pUbcode;
	pUbcode = GetUBCodeDataRX();

	// UBコードAをバイト並びデータに渡す
	if(bySetUbcode == SET_UBCODE_A){
		if (eEndian == ENDIAN_LITTLE) {
			pbyData[0] = pUbcode->byUBCodeA[3];
			pbyData[1] = pUbcode->byUBCodeA[2];
			pbyData[2] = pUbcode->byUBCodeA[1];
			pbyData[3] = pUbcode->byUBCodeA[0];
			pbyData[4] = pUbcode->byUBCodeA[7];
			pbyData[5] = pUbcode->byUBCodeA[6];
			pbyData[6] = pUbcode->byUBCodeA[5];
			pbyData[7] = pUbcode->byUBCodeA[4];
		} else {
			pbyData[0] = pUbcode->byUBCodeA[0];
			pbyData[1] = pUbcode->byUBCodeA[1];
			pbyData[2] = pUbcode->byUBCodeA[2];
			pbyData[3] = pUbcode->byUBCodeA[3];
			pbyData[4] = pUbcode->byUBCodeA[4];
			pbyData[5] = pUbcode->byUBCodeA[5];
			pbyData[6] = pUbcode->byUBCodeA[6];
			pbyData[7] = pUbcode->byUBCodeA[7];
		}
	}

	// UBコードBをバイト並びデータに渡す
	if(bySetUbcode == SET_UBCODE_B){
		if (eEndian == ENDIAN_LITTLE) {
			pbyData[0] = pUbcode->byUBCodeB[3];
			pbyData[1] = pUbcode->byUBCodeB[2];
			pbyData[2] = pUbcode->byUBCodeB[1];
			pbyData[3] = pUbcode->byUBCodeB[0];
			pbyData[4] = pUbcode->byUBCodeB[7];
			pbyData[5] = pUbcode->byUBCodeB[6];
			pbyData[6] = pUbcode->byUBCodeB[5];
			pbyData[7] = pUbcode->byUBCodeB[4];
		} else {
			pbyData[0] = pUbcode->byUBCodeB[0];
			pbyData[1] = pUbcode->byUBCodeB[1];
			pbyData[2] = pUbcode->byUBCodeB[2];
			pbyData[3] = pUbcode->byUBCodeB[3];
			pbyData[4] = pUbcode->byUBCodeB[4];
			pbyData[5] = pUbcode->byUBCodeB[5];
			pbyData[6] = pUbcode->byUBCodeB[6];
			pbyData[7] = pUbcode->byUBCodeB[7];
		}
	}
}

//=============================================================================
/**
 * ターゲット電源値とOFS1のVDSEL値を比較する。
 * @param byVdsel		OFS1.VDSEL値(2bit)
 * @param pbOverVdsel	比較結果格納ポインタ ターゲット電源がVDSEL設定以上である:TRUE 以下:FALSE
 * @return FFWERR
 */
//=============================================================================
// RevRxNo130730-001 Modify Line
FFWERR CmpOfs1Vdsel2TargetVcc(BYTE byVdsel,BOOL* pbOverVdsel)
{
	
	FFWERR ferr = FFWERR_OK;
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	FFWE20_EINF_DATA			einfData;
	// RevRxNo120910-003 Append Start
	DWORD	dwVdsel;			// byVdselをDWORD型にキャストして使用する。
	float	fVdet0Level;		// MCUファイル記載のDWORD値をfloat型に変換して格納用
	DWORD	dwMatchIndex;		// 一致した配列番号を格納用
	BOOL	bMatchFlg = FALSE;	// 一致フラグ(TRUE:一致した、FALSE:一致しなかった)
	DWORD	i;
	float	f1000Value = 1000;	// 1000をfloat変数に設定
	// RevRxNo120910-003 Append End

	pMcuInfo = GetMcuInfoDataRX();

	*pbOverVdsel = TRUE;
	//ターゲット電圧取得
	ferr = DO_GetE20EINF(&einfData);
	if(ferr != FFWERR_OK){
		return ferr;
	}

	// RevRxNo120910-003 Modify Start
	dwVdsel = (static_cast<DWORD>(byVdsel)) & 0x000000FF;
	if (pMcuInfo->dwVdet0LevelNum != 0) {	// dwVdet0LevelNum が 0 でない場合
		for (i = 0; i < pMcuInfo->dwVdet0LevelNum; i++) {
			if ((dwVdsel & pMcuInfo->dwVdet0LevelSelMask) == 
				(pMcuInfo->dwVdet0LevelSelData[i] & pMcuInfo->dwVdet0LevelSelMask)) {
				dwMatchIndex = i;	// 一致した配列番号を格納
				bMatchFlg = TRUE;	// 一致フラグに、"一致した(TRUE)"を設定
				break;
			}
		}
		if (bMatchFlg == FALSE) {	// 一致しなかった場合
			return ferr;			// *pbOverVdsel = TRUE のまま関数から抜ける。
		}
		fVdet0Level = (static_cast<float>(pMcuInfo->dwVdet0Level[i])) / f1000Value;	// floatにキャストして1000fで割る。
		if(einfData.flNowUVCC <= fVdet0Level){	// ターゲット電圧がpMcuInfo->dwVdet0Level[i]以下の場合
			*pbOverVdsel = FALSE;	// 「比較結果格納フラグ」に、"電圧以下(FALSE)"を設定
		}
	}
	// RevRxNo120910-003 Modify End

	return ferr;
}
// V.1.03 RevNo111121-008 Append End

//=============================================================================
/**
 * エラーチェック処理内部変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitErrorData_Errchk(void)
{
	// V.1.02 RevNo110613-001 Append Line
	s_ferrBFWWarning = FFWERR_OK;
	return;
}

