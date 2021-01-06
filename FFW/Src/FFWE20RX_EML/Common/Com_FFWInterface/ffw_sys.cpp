///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffw_sys.cpp
 * @brief システムコマンドの実装ファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, H.Akashi, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120910-001	2012/10/15 三宅
  FFW I/F仕様変更に伴うFFWソース変更。
　・FFWCmd_SetCPUD() の空の関数定義追加。
・RevNo120910-001	2012/10/31 明石
　USB VBUS電圧判定処理追加
・RevNo120910-002	2012/10/31 明石
　MONP ダウンロードデータサイズの奇数バイト対応
・RevNo121017-003	2012/10/31 明石
  VS2008対応エラーコード　WARNING C4996対応変更
・RevRxNo120910-007	2012/11/12 明石
  外部フラッシュ ID不一致時の値返送
・RevRxNo130730-011 2014/02/07 大喜多、上田
  isdigit(),isxdigit(), isupper()の引数をchar型からint型に変更
・RevRxE2LNo141104-001 2015/01/20 上田
	E2 Lite対応
	E1/E20 レベル0 旧バージョン(MP版以外)用関数、処理削除。
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
・RevRxNo170829-001 2017/08/29 山本
	E2拡張対応
・RevRxNo170908-001 2017/09/08 大賀
	FFWE2Cmd_BfwEmlUpdate()対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
・RevRxE1E20No180921-001 2018/09/21 大賀
	E1/E20用BFWのROM容量不足対策(PB18047-CD04-001)
*/
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif

#include "do_sys.h"
#include "ffw_sys.h"
#include "comctrl.h"
#include "ffwdata.h"
#include "dodata.h"
#include "protdata.h"
#include "errdata.h"
#include "mcudata.h"
#include "hwdata.h"
#include "do_closed.h"
#include "prot_sys.h"
#include "ffw_closed.h"
//V.1.02 RevNo110613-001 Append Line
#include "errchk.h"
#include "emudef.h"	// RevRxE2LNo141104-001 Append Line
// RevRxE2No170201-001 Append Start
#include "doe2_sys.h"
#include "ffwe2_sys.h"
// RevRxE2No170201-001 Append End
// RevRxNo170829-001 Append Start
#include "doasp_sys.h"
#include "doasprx_monev.h"
#include "doasp_trg.h"
#include "doasp_pwr.h"
#include "cpuspec.h"
#include "prote2_closed.h"
// RevRxNo170829-001 Append End

// ファイル内static変数の宣言
static DWORD*	s_pdwGageCount=NULL;	// 進捗ゲージカウンタの格納アドレス
static FFW_WINVER_DATA	s_WinVer;		// OSバージョン情報

static WORD		s_wFfwTimeOut;			// FFW用受信タイムアウト
static WORD		s_wBfwTimeOut;			// BFW用受信タイムアウト

static FFWERR	s_ferrDcnfStat;			// FFWCmd_DCNF_SEND実行状況

// MONP用変数
static FFWERR	s_ferrMonpStat;			// MONP_OPEN、MONP_SENDの実行状況
static FFWERR	s_ferrMonpFdtStat;		// MONPFDT_OPEN、MONPFDT_SENDの実行状況
// MONP用変数(E1/E20専用)
static DWORD	s_dwMonpDataSize;		// MONPコマンドでライトするデータの総バイト数
static BYTE		s_byMonpSum[4];			// MONPコマンドでライトする2の補数値
static DWORD	s_dwMonpFdtDataSize;	// MONPFDTコマンドでライトするデータの総バイト数
static BYTE		s_byMonpFdtSum[4];		// MONPFDTコマンドでライトする2の補数値


void	(*CallbackFromFfw2Target)(FFWENM_CALLBACKFUNC_FWRITE_STATUS) = NULL;

static char*	s_chMsgGageCount = NULL;	// 進捗ゲージカウンタに表示するメッセージ文字列格納アドレス

#define	BUFFER_MAX	(0x20000)
static BOOL get_hex_data(char *p, DWORD *value, short cnt);

#define ERROR_MESSAGE_MAX	256					// エラーメッセージは最長256Byte
FFW_ERROR_DETAIL_INFO s_ErrorDetailInfo;		// エラー詳細情報格納用構造体

// [E100]
class CSrecord {
// Construction
public:
	CSrecord() {
	};
	CSrecord(ULONG len, ULONG sendAddr, char *buffer) {
		m_length = len;
		m_startaddr = sendAddr; 
		m_databuffer = new BYTE[len];
		memcpy(m_databuffer, buffer, m_length);
	};
	~CSrecord() {
		delete m_databuffer;
	};
	ULONG	m_startaddr;
	ULONG	m_length;
	BYTE	*m_databuffer;
};

// V.1.02 No.14,15 USB高速化対応 Append Line
static BOOL s_bLevelEml;

///////////////////////////////////////////////////////////////////////////////
// FFW I/F関数定義
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * FFWモジュールを初期化する。
 * @param pSend Communi.DLLのCOM_SendBlock関数のアドレス
 * @param pRecv Communi.DLLのCOM_ReceiveBlock関数のアドレス
 * @param pRecvH Communi.DLLのCOM_ReceiveBlockH関数のアドレス
 * @param pRecvPolling Communi.DLLのCOM_ReceivePolling関数のアドレス
 * @param pSetTimeOut Communi.DLLのCOM_SetTimeOut関数のアドレス
 * @param pLock Communi.DLLのCOM_Lock関数のアドレス
 * @param pUnlock Communi.DLLのCOM_UnLock関数のアドレス
 * @param pTransSize Communi.DLLのCOM_SetTransferSize関数のアドレス
 * @param pdwGageCount 進捗ゲージカウンタの格納アドレス
 * @param pWinVer OSバージョン情報の格納アドレス
 * @param pCallbackFromFfw2Target FFWからのコールバック用
 * @param cMsgGageCount 進捗ゲージカウンタに表示するメッセージ文字列格納用
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_INIT(void* pSend, void* pRecv, void* pRecvH, void* pRecvPolling,
	void* pSetTimeOut, void* pLock, void* pUnlock, void* pTransSize, DWORD* pdwGageCount, FFW_WINVER_DATA* pWinVer, void* pCallbackFromFfw2Target, char cMsgGageCount[])
{
	FFWERR	ferr = FFWERR_OK;
	int	i;
	BYTE	byBfwTransMode;
	FFWE20_EINF_DATA Einf;// RevRxNo170829-001 Append Line

	// OSバージョン情報を設定
	s_WinVer.dwMajorVersion = pWinVer->dwMajorVersion;
	s_WinVer.dwMinorVersion = pWinVer->dwMinorVersion;
	s_WinVer.dwBuildNumber = pWinVer->dwBuildNumber;
	s_WinVer.dwPlatformId = pWinVer->dwPlatformId;
	for (i = 0; i < INIT_WINVER_ADDINFO_NUM; i++) {
		s_WinVer.szCSDVersion[i] = pWinVer->szCSDVersion[i];
	}

	// V.1.02 No.14,15 USB高速化対応 Append Line
	s_bLevelEml = FALSE;
	
	// Communi.DLLの各関数アドレスの設定
	SetCOMFuncAddr(pSend, pRecv, pRecvH, pRecvPolling, pSetTimeOut, pLock, pUnlock, pTransSize);

	// 進捗ゲージカウンタ格納アドレスの設定
	s_pdwGageCount = pdwGageCount;

	// 進捗ゲージカウンタに表示するメッセージ文字列格納アドレスの設定
	s_chMsgGageCount = &cMsgGageCount[0];

	// 内部変数初期化
	InitComctrlData();	// INITコマンド発行時の通信管理/例外処理用内部変数の初期化
	InitProtData();		// BFWプロトコル生成処理用内部変数の初期化
	InitErrorData();	// エラー処理内部変数の初期化
	InitFfwIfData();	// FFWコマンド処理内部変数の初期化
	InitFfwCmdData();	// FFWコマンド処理内部変数の初期化
	InitMcuData();		// ターゲットMCU制御関数内部変数の初期化
	InitHwData();		// エミュレータHW制御関数内部変数の初期化
	InitErrorDetailInfo();	// エラー詳細表示バッファクリア

	ferr = SetCOMTimeOut(s_wFfwTimeOut);	// INIT時、FFWでCommuni.dllにタイムアウト値(30s)を設定
	//RevNo100721-001 Append Start
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	//RevNo100721-001 Append End

	CallbackFromFfw2Target = (void (*)(FFWENM_CALLBACKFUNC_FWRITE_STATUS))pCallbackFromFfw2Target;

	ferr = DO_INIT();
	//RevNo100721-001 Append Start
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	//RevNo100721-001 Append End

	// BFWレベル0への遷移
	// 　前回のデバッガ終了が異常終了し、エミュレータがEML動作モードのままに
	// 　なっている場合を考慮して、BFWにレベル0遷移コマンドを発行する。
	// 　通常はデバッガがデバッガ終了時にTRANS LV0を発行する。
	// 　ただし、E1/E20はUSB通信制御がレベル0とEMLで異なるため
	// 　エミュレータがEML動作中に起動した場合はDO_INIT()でエラーとなる。
	byBfwTransMode = TRANS_MODE_LV0;
	ferr = DO_E20Trans(byBfwTransMode);

	// RevRxNo170829-001 Append Start
	// E2拡張機能関連の初期設定(E2接続時のみ実施)
	// エミュレータ種別情報を取得するGetEINFコマンド発行状態が不明なため、
	// 事前の確認処理を暫定的に追加しておく。本番でどうするか、要確認。
	if (!GetEinfDataSetFlg()) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	else {
		getEinfData(&Einf);
	}

#if defined(E2E2L)	// RevRxE2No171004-001 Append Line
	if (Einf.wEmuStatus == EML_E2) {
		InitAspPinConf();
		InitAspMonEv();
		InitAsp();
		InitAspExTrgOutLevel();
		InitAspPwrMonCorrect();

		ChkE2IndividualCurrentData();
	}
	// RevRxNo170829-001 Append End
#endif		// RevRxE2No171004-001 Append Line

	return ferr;
}

//=============================================================================
/**
 * USB転送サイズの設定
 * @param dwTransSize Communi.DLLのCOM_ReceiveBlockH関数でBFWに要求するデータバイト数
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWCmd_SetUSBTS(DWORD dwTransSize)
{
	FFWERR	ferr = FFWERR_OK;

	if (dwTransSize < USBTS_SIZE_MIN) {	// 512バイトより小さい場合
		return FFWERR_FFW_ARG;
	}
	if (dwTransSize > USBTS_SIZE_MAX) {	// 8Kバイトより大きい場合
		return FFWERR_FFW_ARG;
	}
	if ((dwTransSize & 0x000001ff) != 0) {	// 512バイトの倍数でない場合
		return FFWERR_FFW_ARG;
	}

	ferr = DO_SetUSBTS(dwTransSize);

	return ferr;
}

//=============================================================================
/**
 * USB転送サイズの取得	
 * @param pdwTransSize Communi.DLLのCOM_ReceiveBlockH関数でBFWに要求する
 *                     データバイト数の格納アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWCmd_GetUSBTS(DWORD* pdwTransSize)
{
	FFWERR	ferr = FFWERR_OK;

	ferr = DO_GetUSBTS(pdwTransSize);

	return ferr;
}

//=============================================================================
/**
 * FFW, BFWのタイムアウト時間の設定
 * @param wFfwTimeOut FFWがBFWからデータを受信する際の受信タイムアウト時間
 * @param wBfwTimeOut BFWがEFW実行要求を出してからEFW処理実行完了確認までのタイムアウト時間
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWCmd_SetTMOUT(WORD wFfwTimeOut, WORD wBfwTimeOut)
{
	FFWERR	ferr = FFWERR_OK;

	if (wBfwTimeOut >= wFfwTimeOut) {
		return FFWERR_FFW_ARG;
	}

	ferr = DO_SetTMOUT(wFfwTimeOut, wBfwTimeOut);

	if (ferr == FFWERR_OK) {
		s_wFfwTimeOut = wFfwTimeOut;
		s_wBfwTimeOut = wBfwTimeOut;
	}

	return ferr;
}

//=============================================================================
/**
 * FFW, BFWのタイムアウト時間の参照
 * @param pwFfwTimeOut FFWがBFWからデータを受信する際の受信タイムアウト時間の格納アドレス
 * @param pwBfwTimeOut BFWがEFW実行要求を出してからEFW処理実行完了確認までのタイムアウト時間の格納アドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWCmd_GetTMOUT(WORD* pwFfwTimeOut, WORD* pwBfwTimeOut)
{
	FFWERR ferr = FFWERR_OK;

	*pwFfwTimeOut = s_wFfwTimeOut;	// FFWのタイムアウト時間を格納
	*pwBfwTimeOut = s_wBfwTimeOut;	// BFWのタイムアウト時間(FFW内部変数)を格納	// RevRxE2LNo141104-001 Append Line
									// E2/E2 Liteでは、EML動作中の場合のみDO_GetTMOUT()でBFWから取得した値に更新される。

	ferr = DO_GetTMOUT(pwBfwTimeOut);	// BFWのタイムアウト時間を取得

	return ferr;
}

// 2008.11.13 INSERT_BEGIN_E20RX600(+6) {
//=============================================================================
/**
 * エミュレータ情報の取得(RX用)
 * @param pEinf エミュレータハードウェア情報を格納する構造体のアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_GetEINF(FFWE20_EINF_DATA* pEinf)
{
	FFWERR ferr = FFWERR_OK;

	// RevRxE2LNo141104-001 Modify Start
		// E1/E20 レベル0 V.0.05.00.000未満の処理を削除。
	if ((ferr = DO_GetE20EINF(pEinf)) != FFWERR_OK) {
		return ferr;
	}
	// RevRxE2LNo141104-001 Modify End

	// RevRxE2No170201-001 Modify Start
	if (pEinf->wEmuStatus == EML_E2) {
		// USBバスパワー電圧値が4.00未満かチェックをする。
		if (pEinf->flNowVbus < USB_VBUS_THRESHHOLD_4_00V) {
			ferr = FFWERR_USB_VBUS_LOW;
		}
	} else {
		// RevNo120910-001	Append Start
		// RevRxE2LNo141104-001 Modify Start
		// USBバスパワー電圧値が4.35未満かチェックをする。
		if (pEinf->flNowVbus < USB_VBUS_THRESHHOLD) {
			ferr = FFWERR_USB_VBUS_LOW;
		}
		// RevRxE2LNo141104-001 Modify End
		// RevNo120910-001	Append End
	}
	// RevRxE2No170201-001 Modify End

	return ferr;
}
// 2008.11.13 INSERT_END_E20RX600 }

// 2008.11.13 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * BFW Level EMLのダウンロード開始
 * @param dwTotalLength FFWE20Cmd_MONP_SENDで送信するBFWデータの総バイト数
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONP_OPEN(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	char filename[MAX_PATH + 1];
	FFWE20_SUM_DATA		pSum;
	FFWE20_EINF_DATA	Einf;
	DWORD	dwBufSize;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Modify Start
	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		s_ferrMonpStat = ferr;
		return ferr;
	}
	// BFW動作モードの確認
	if (Einf.byBfwMode != BFW_LV0) {
		// BFWの動作モードがLv0で無い場合、エラー終了
		ferr = FFWERR_BEXE_LEVEL0;
		s_ferrMonpStat = ferr;
		return ferr;
	}

	// E1/E20の場合、Level0のサム値を取得
	if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
		DO_E20SUMCHECK(&pSum);
		s_byMonpSum[3] = pSum.bySumLevel0;
	}

	// 制御CPUフラッシュ書き換えプログラム(MONPPROG)をダウンロード
	dwBufSize = static_cast<DWORD>(sizeof(filename));
	GetMonpprogFilename(filename, dwBufSize);	// MONPPROGのファイル名(フルパス)を取得

	if ((ferr = Load_wtr_e20(filename)) != FFWERR_OK) {	// MONPPROGのダウンロード
		s_ferrMonpStat = ferr;
		return ferr;
	}

	// MONP_OPENコマンド送信
	ferr = DO_MONP_OPEN(dwTotalLength);
	if (ferr == FFWERR_OK) {
		if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
			// コマンド正常終了時、Level0、1領域の2の補数格納変数を初期化
			s_byMonpSum[0] = 0xff;		// 2FFFChのライトデータ
			s_byMonpSum[1] = 0xff;		// 2FFFDhのライトデータ
			s_byMonpSum[2] = 0xff;		// 2FFFEhのライトデータ

			s_dwMonpDataSize = 0;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// MONP_OPENの実行状況をコピー
	s_ferrMonpStat = ferr;

	return ferr;
}


//=============================================================================
/**
 * BFW Level EMLデータの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONP_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwBfwAreaEndAddr;
	FFWE20_EINF_DATA einfData;	// RevRxE2LNo141104-001 Append Line

	pEmuDef = GetEmuDefData();
	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// パラメータエラーチェック
	if (dwLength > MONP_LENGTH_MAX) {
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	// RevRxE2LNo141104-001 Append Start
	dwBfwAreaEndAddr = pEmuDef->dwBfwAreaStartAdr + pEmuDef->dwBfwAreaSize - 1;
	if ((dweadrAddr < pEmuDef->dwBfwAreaStartAdr) || (dweadrAddr > dwBfwAreaEndAddr)) {	
		// 開始アドレスがBFW領域開始アドレス～BFW領域終了アドレス の範囲外の場合
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	if (dwLength > (dwBfwAreaEndAddr - dweadrAddr + 1)) {
		// データバイト数が、(開始アドレス～BFW領域終了アドレス)を超える場合
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	// RevRxE2LNo141104-001 Append End


	// RevRxE2LNo141104-001 Append Start
	// E1/E20以外の場合
	if ((einfData.wEmuStatus != EML_E1) && (einfData.wEmuStatus != EML_E20)) {
			ferr = DO_MONP_SEND(dweadrAddr, dwLength, pbyBfwBuff);

	// E1/E20の場合、書き換え対象領域のサム値を算出して、BFWコード送信
	} else {
	// RevRxE2LNo141104-001 Append End
		// LV0の内容はサム値算出時に加算しない。
		if ((dweadrAddr < 0x10000) && (dweadrAddr + dwLength -1 >= 0x10000)) {
			for(DWORD dwCount = 0; dwCount < dweadrAddr+dwLength-0x10000; dwCount++ ){
				s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + dwCount + (0x10000 - dweadrAddr)));
			}
			s_dwMonpDataSize += dweadrAddr+dwLength-0x10000;
			// RevNo120910-002	Modify Start
			//	BFW MONP側で0x10000以前のデータは書き込みしない様になっているので対策不要
			//	ferr = DO_MONP_SEND(0x10000, dweadrAddr+dwLength-0x10000, (BYTE *)&(pbyBfwBuff[(0x10000 - dweadrAddr)]));
			ferr = DO_MONP_SEND(dweadrAddr, dwLength, pbyBfwBuff);
			// RevNo120910-002	Modify End
		} else {
			// EMLの10000h～2FFFBhの内容の総和値を算出
			if ((dweadrAddr >= 0x10000) && (dweadrAddr < 0x30000)) {
				// 最終4バイトを含む場合
				if ((dweadrAddr <= 0x2FFFC) && (0x2FFFC <= (dweadrAddr + dwLength - 1))) {
					s_dwMonpDataSize += dwLength;
					// 0x2FFFF番地へのライトデータが含まれている場合、0x2FFFF番地のデータはサム値計算に使用しないため、
					// 0x2FFFF番地のデータは無視をする
					if( (dweadrAddr <= 0x2FFFF) && (0x2FFFF <= (dweadrAddr + dwLength - 1)) ){
						s_dwMonpDataSize -= 1;
					}
					for(DWORD dwCount = 0; dwCount < dwLength-(dweadrAddr+dwLength-0x2FFFC); dwCount++ ){
						s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + dwCount));
					}
					s_byMonpSum[0] = *(pbyBfwBuff + (0x2FFFC - dweadrAddr));
					s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + (0x2FFFC - dweadrAddr)));
					if( (dweadrAddr <= 0x2FFFD) && (0x2FFFD <= (dweadrAddr + dwLength - 1)) ){
						s_byMonpSum[1] = *(pbyBfwBuff + (0x2FFFD - dweadrAddr));
						s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + (0x2FFFD - dweadrAddr)));
						if( (dweadrAddr <= 0x2FFFE) && (0x2FFFE <= (dweadrAddr + dwLength - 1)) ){
							s_byMonpSum[2] = *(pbyBfwBuff + (0x2FFFE - dweadrAddr));
							s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + (0x2FFFE - dweadrAddr)));
						}
					}
					ferr = DO_MONP_SEND(dweadrAddr, dwLength-(dweadrAddr+dwLength-0x2FFFC), pbyBfwBuff);		// 最終4バイトはサム値と一緒にMONP_CLOSEでプログラムする。
				}
				else {
					for(DWORD dwCount = 0; dwCount < dwLength; dwCount++ ){
						s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + dwCount));
					}
					s_dwMonpDataSize += dwLength;
					ferr = DO_MONP_SEND(dweadrAddr, dwLength, pbyBfwBuff);
				}
			}
		}
	}	// RevRxE2LNo141104-001 Append Line

	// MONP_SENDの実行状況をコピー
	s_ferrMonpStat = ferr;

	return ferr;
}

//=============================================================================
/**
 * BFW Level EMLのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONP_CLOSE(void)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwLength;
	BYTE	bySum;
	FFWE20_EINF_DATA einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// MONP_OPEN、MONP_SENDの実行状況を確認
	if (s_ferrMonpStat == FFWERR_OK) {
		if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// RevRxE2LNo141104-001 Append Line
			// MONPALL_OPEN、MONPALL_SENDコマンドが正常終了した場合は、チェックサム値をライトする
			// LV0, EML領域の空きデータの総和値を算出
			dwLength = 0x1FFFF - s_dwMonpDataSize;
			bySum = (BYTE)(dwLength * 0xFF);
			bySum = (BYTE)(bySum + s_byMonpSum[3]);
			s_byMonpSum[3] = (BYTE)((~bySum) + 1);
			ferr = DO_MONP_SEND(0x2fffc, 4, s_byMonpSum);
		}	// RevRxE2LNo141104-001 Append Line
	}

	// MONP_CLOSEコマンド送信
	ferr = DO_MONP_CLOSE();

	*s_pdwGageCount = GAGE_COUNT_MAX;

	return ferr;
}

//=============================================================================
/**
 * FDT領域BFW Level EMLのダウンロード開始
 * @param dwTotalLength FFWE20Cmd_MONPFDT_SENDで送信するBFWデータの総バイト数
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONPFDT_OPEN(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	char filename[MAX_PATH + 1];
	FFWE20_EINF_DATA	Einf;
	FFWE20_SUM_DATA		pSum;
	DWORD	dwBufSize;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Modify Start
	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		s_ferrMonpFdtStat = ferr;
		return ferr;
	}
	// BFW動作モードの確認
	if (Einf.byBfwMode != BFW_LV0) {
		// BFWの動作モードがLv0で無い場合、エラー終了
		ferr = FFWERR_BEXE_LEVEL0;
		s_ferrMonpFdtStat = ferr;
		return ferr;
	}

	// E1/E20の場合、Level0のサム値を取得
	if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
		DO_E20SUMCHECK(&pSum);
		s_byMonpFdtSum[3] = pSum.bySumLevel0;
	}

	// 制御CPUフラッシュ書き換えプログラム(MONPPROG)をダウンロード
	dwBufSize = static_cast<DWORD>(sizeof(filename));
	GetMonpprogFilename(filename, dwBufSize);	// MONPPROGのファイル名(フルパス)を取得

	if ((ferr = Load_wtr_e20(filename)) != FFWERR_OK) {	// MONPPROGのダウンロード
		s_ferrMonpFdtStat = ferr;
		return ferr;
	}

	// MONPFDT_OPENコマンド送信
	ferr = DO_MONPFDT_OPEN(dwTotalLength);
	if (ferr == FFWERR_OK) {
		if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
			// コマンド正常終了時、EML領域(LV0+EML)の2の補数格納変数を初期化
			s_byMonpFdtSum[0] = 0xff;		// 3FFFChのライトデータ
			s_byMonpFdtSum[1] = 0xff;		// 3FFFDhのライトデータ
			s_byMonpFdtSum[2] = 0xff;		// 3FFFEhのライトデータ

			s_dwMonpFdtDataSize = 0;
		}
	}
	// RevRxE2LNo141104-001 Modify End

	// MONPFDT_OPENの実行状況をコピー
	s_ferrMonpFdtStat = ferr;

	return ferr;
}

//=============================================================================
/**
 * FDT領域BFW Level EMLデータの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONPFDT_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwBfwAreaEndAddr;
	FFWE20_EINF_DATA einfData;	// RevRxE2LNo141104-001 Append Line

	pEmuDef = GetEmuDefData();
	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// パラメータエラーチェック
	if (dwLength > MONP_LENGTH_MAX) {
		s_ferrMonpFdtStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	// RevRxE2LNo141104-001 Append Start
	dwBfwAreaEndAddr = pEmuDef->dwBfwAreaStartAdr + pEmuDef->dwBfwAreaSize - 1;
	if ((dweadrAddr < pEmuDef->dwBfwAreaStartAdr) || (dweadrAddr > dwBfwAreaEndAddr)) {	
		// 開始アドレスがBFW領域開始アドレス～BFW領域終了アドレス の範囲外の場合
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	if (dwLength > (dwBfwAreaEndAddr - dweadrAddr + 1)) {
		// データバイト数が、(開始アドレス～BFW領域終了アドレス)を超える場合
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	// RevRxE2LNo141104-001 Append End


	// RevRxE2LNo141104-001 Append Start
	// E1/E20以外の場合
	if ((einfData.wEmuStatus != EML_E1) && (einfData.wEmuStatus != EML_E20)) {
		ferr = DO_MONPFDT_SEND(dweadrAddr, dwLength, pbyBfwBuff);


	// E1/E20の場合、書き換え対象領域のサム値を算出して、BFWコード送信
	} else {
	// RevRxE2LNo141104-001 Append End
		// LV0およびEMLの内容はサム値算出時に加算しない。
		if ((dweadrAddr < 0x30000) && (dweadrAddr + dwLength -1 >= 0x30000)) {
			for(DWORD dwCount = 0; dwCount < dweadrAddr+dwLength-0x30000; dwCount++ ){
				s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + dwCount + (0x30000 - dweadrAddr)));
			}
			s_dwMonpFdtDataSize += dweadrAddr+dwLength-0x30000;
			ferr = DO_MONPFDT_SEND(dweadrAddr, dwLength, pbyBfwBuff);
		}
		// FDTの30000h～3FFFBhの内容の総和値を算出
		else if (dweadrAddr >= 0x30000) {
			// 最終4バイトを含む場合
			if ((dweadrAddr <= 0x3FFFC) && (0x3FFFC <= (dweadrAddr + dwLength - 1))) {
				s_dwMonpFdtDataSize += dwLength;
				// 0x3FFFF番地へのライトデータが含まれている場合、0x3FFFF番地のデータはサム値計算に使用しないため、
				// 0x3FFFF番地のデータは無視をする
				if( (dweadrAddr <= 0x3FFFF) && (0x3FFFF <= (dweadrAddr + dwLength - 1)) ){
					s_dwMonpFdtDataSize -= 1;
				}
				for(DWORD dwCount = 0; dwCount < dwLength-(dweadrAddr+dwLength-0x3FFFC); dwCount++ ){
					s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + dwCount));
				}
				s_byMonpFdtSum[0] = *(pbyBfwBuff + (0x3FFFC - dweadrAddr));
				s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + (0x3FFFC - dweadrAddr)));
				if( (dweadrAddr <= 0x3FFFD) && (0x3FFFD <= (dweadrAddr + dwLength - 1)) ){
					s_byMonpFdtSum[1] = *(pbyBfwBuff + (0x3FFFD - dweadrAddr));
					s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + (0x3FFFD - dweadrAddr)));
					if( (dweadrAddr <= 0x3FFFE) && (0x3FFFE <= (dweadrAddr + dwLength - 1)) ){
						s_byMonpFdtSum[2] = *(pbyBfwBuff + (0x3FFFE - dweadrAddr));
						s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + (0x3FFFE - dweadrAddr)));
					}
				}
				ferr = DO_MONPFDT_SEND(dweadrAddr, dwLength-(dweadrAddr+dwLength-0x3FFFC), pbyBfwBuff);		// 最終4バイトはサム値と一緒にMONP_CLOSEでプログラムする。
			}
			else {
				for(DWORD dwCount = 0; dwCount < dwLength; dwCount++ ){
					s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + dwCount));
				}
				s_dwMonpFdtDataSize += dwLength;
				ferr = DO_MONPFDT_SEND(dweadrAddr, dwLength, pbyBfwBuff);
			}
		}
	}	// RevRxE2LNo141104-001 Append Line

	// MONP_SENDの実行状況をコピー
	s_ferrMonpFdtStat = ferr;

	return ferr;
}

//=============================================================================
/**
 * BFW Level EMLのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_MONPFDT_CLOSE(void)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwLength;
	BYTE	bySum;
	FFWE20_EINF_DATA einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// MONP_OPEN、MONP_SENDの実行状況を確認
	if (s_ferrMonpFdtStat == FFWERR_OK) {
		if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {	// RevRxE2LNo141104-001 Append Line
			// MONP_OPEN、MONP_SENDコマンドが正常終了した場合は、チェックサム値をライトする
			// FDT領域内の空きデータの総和値を算出
			dwLength = 0xFFFF - s_dwMonpFdtDataSize;
			bySum = (BYTE)(dwLength * 0xFF);
			bySum = (BYTE)(bySum + s_byMonpFdtSum[3]);
			s_byMonpFdtSum[3] = (BYTE)((~bySum) + 1);
			ferr = DO_MONPFDT_SEND( 0x3fffc,4,s_byMonpFdtSum );
			// エラーが発生してもMONP_CLOSEは実行させる！
		}	// RevRxE2LNo141104-001 Append Line
	}

	// MONP_CLOSEコマンド送信
	ferr = DO_MONPFDT_CLOSE();

	*s_pdwGageCount = GAGE_COUNT_MAX;

// TargetがFDT遷移コマンドを発行することによりFDTへ遷移する。

	return ferr;
}

//=============================================================================
/**
 * DCSFコンフィギュレーションの開始(RX用)
 * @param dwTotalLength FFWCmd_DCNF_SENDで送信するBFWデータの総バイト数
 * @param dwType ダウンロードするFPGAファイルの種類
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_DCNF_OPEN(DWORD dwTotalLength, DWORD dwType)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// E2 Liteの場合、何もせず正常終了を返す。
	if (einfData.wEmuStatus == EML_E2LITE) {
		return FFWERR_OK;
	}
	// RevRxE2LNo141104-001 Append End

	ferr = DO_E20DCNF_OPEN(dwTotalLength, dwType);

	s_ferrDcnfStat = FFWERR_OK;

	return ferr;
}

//=============================================================================
/**
 * DCSFコンフィギュレーションデータの送信
 * @param dwLength DCSF論理データのバイト数
 * @param pbyDcsfBuff DCSF論理データの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_DCNF_SEND(DWORD dwLength, const BYTE* pbyDcsfBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// E2 Liteの場合、何もせず正常終了を返す。
	if (einfData.wEmuStatus == EML_E2LITE) {
		return FFWERR_OK;
	}
	// RevRxE2LNo141104-001 Append End

	ferr = DO_DCNF_SEND(dwLength, pbyDcsfBuff);
	s_ferrDcnfStat = ferr;

	return ferr;
}

//=============================================================================
/**
 * DCSFコンフィギュレーションの終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_DCNF_CLOSE(void)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// E2 Liteの場合、何もせず正常終了を返す。
	if (einfData.wEmuStatus == EML_E2LITE) {
		return FFWERR_OK;
	}
	// RevRxE2LNo141104-001 Append End

	// FFWCmd_DCNF_SENDの実行状況が正常な場合、H/Wの初期化を行う。
	if (s_ferrDcnfStat == FFWERR_OK) {
		ferr = DO_DCNF_CLOSE();
	}

	*s_pdwGageCount = GAGE_COUNT_MAX;

	return ferr;
}
// 2008.11.13 INSERT_END_E20RX600 }

//=============================================================================
/**
 * ユーザ基板への電源供給を設定(固定値指定)
 * @param eExPwrEnable 電源供給有効/無効設定
 * @param eExPwrVal 供給電源電圧
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_EXTPWR (enum FFWENM_EXPWR_ENABLE eExPwrEnable, enum FFWENM_EXPWR_VAL eExPwrVal)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;
	FFWE20_EINF_DATA	einfGetUvcc;
	float	fCmpUvcc;			// RevRxE2LNo141104-001 Append Line
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2No170201-001 Append Start
	enum FFWENM_EXPWR2_MODE eExPwr2Mode;	// 電源供給モード(E2用)
	BYTE					byExPwr2Val;	// 電源供給電圧(E2用)
	enum FFWENM_EXPWR2_MODE eNowExPwr2Mode;	// 現電源供給モード(E2用)
	// RevRxE2No170201-001 Append End

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	// RevRxE2No170201-001 Modify Start
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	} else {
		// E2では、電源供給しない設定にした場合にすぐにEINFコマンドを発行すると
		// 供給停止が反映される前に電圧値を取得してしまう可能性があるため、必ず最新の電圧値を取得する
		getEinfData(&einfData);		// エミュレータ情報取得
		if (einfData.wEmuStatus == EML_E2) {
			if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
				return ferr;
			}
		}
	}
	// RevRxE2No170201-001 Modify End
	// RevRxE2LNo141104-001 Append End

	getEinfData(&einfData);		// エミュレータ情報取得

	// RevRxE2LNo141104-001 Append Start
	// エミュレータのサポート供給電圧値チェック
	if (eExPwrEnable == EXPWR_ENABLE) {
		if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {
			if ((eExPwrVal != EXPWR_V330) && (eExPwrVal != EXPWR_V500)) {
				return FFWERR_FUNC_UNSUPORT;
			}
		} else if (einfData.wEmuStatus == EML_E2LITE) {
			if (eExPwrVal != EXPWR_V330) {
				return FFWERR_FUNC_UNSUPORT;
			}
		// RevRxE2No170201-001 Append Start
		} else if (einfData.wEmuStatus == EML_E2) {
			if ((eExPwrVal != EXPWR_V180) && (eExPwrVal != EXPWR_V330) && (eExPwrVal != EXPWR_V500)) {
				return FFWERR_FUNC_UNSUPORT;
			}
		// RevRxE2No170201-001 Append End
		} else {
			// 何もしない
		}
	}
	// RevRxE2LNo141104-001 Append End

	// V.1.02 覚え書き23の1点目(低電圧供給中の3.3Vor5V供給回避) Append Start
	// 起動時の関数発行手順で、EXTPWR発行前に必ずEINFを発行することになっているのでその情報を利用する
	// RevNo110223-001 Modify Line
	// ソースコードDR指摘事項 No.1対応 Modify Line(EINFに合わせて1.49以上とする)
	// RevRxE2LNo141104-001 Modify Start
	// E1/E20用BFW レベル0不具合対応。E2 LiteはBFW レベル0のEXTPWRコマンドでチェックするためFFWでは実施しない。
	if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {
		if ((eExPwrEnable == EXPWR_ENABLE) && (einfData.flNowUVCC >= UVCC_1_49V)) {		// 1.49V以上の電源が既に供給されている場合
			return FFWERR_BTARGET_POWERSUPPLY;
		}
	// RevRxE2No170201-001 Append Start
	} else if (einfData.wEmuStatus == EML_E2) {		// E2は、BFW レベル0でチェックしていないためFFWで実施する
		if ((eExPwrEnable == EXPWR_ENABLE) && (einfData.flNowUVCC >= UVCC_1_20V)) {		// 1.20V以上の電源が既に供給されている場合
			return FFWERR_BTARGET_POWERSUPPLY;
		}
	// RevRxE2No170201-001 Append End
	}
	// RevRxE2LNo141104-001 Modify End
	// V.1.02 覚え書き23の1点目(低電圧供給中の3.3Vor5V供給回避) Append End

	// RevRxE2No170201-001 Modify Start
	if (einfData.wEmuStatus == EML_E2) {
		// 電源供給モード変換
		if (eExPwrEnable == EXPWR_DISABLE) {		// 電源供給しない
			eExPwr2Mode = EXPWR2_DISABLE;
		} else if (eExPwrEnable == EXPWR_ENABLE) {	// 電源供給する
			eExPwr2Mode = EXPWR2_USER;
		} else {
			return FFWERR_FUNC_UNSUPORT;
		}

		// 電源供給電圧設定値変換(0.1V単位で供給する電圧値を10倍した値に変換)
		if (eExPwrVal == EXPWR_V330) {			// 3.3V供給時
			byExPwr2Val = 0x21;
		} else if (eExPwrVal == EXPWR_V500) {	// 5.0V供給時
			byExPwr2Val = 0x32;
		} else {								// 1.8V供給時
			byExPwr2Val = 0x12;
		}

		eNowExPwr2Mode = GetExPwr2Mode();
		if ((eNowExPwr2Mode != EXPWR2_DISABLE) && (eNowExPwr2Mode != EXPWR2_USER)) {
			ferr = DO_EXPWR2(EXPWR2_DISABLE, byExPwr2Val);	// 一旦電源供給停止
		}
		ferr = DO_EXPWR2(eExPwr2Mode, byExPwr2Val);
		if (ferr == FFWERR_OK) {
			// RevRxE2No171004-001 Modify Line
			SaveExPwr2Setting(eExPwr2Mode, byExPwr2Val);	// 電源供給設定を保存
		}
	} else {
		ferr = DO_EXPWR(eExPwrEnable, eExPwrVal);
	}
	// RevRxE2No170201-001 Modify End

	// RevRxE2LNo141104-001 Append Start
	if (ferr == FFWERR_OK) {
		// E2/E2 Liteエミュレータから電源供給後の電圧値確認
		if ((eExPwrEnable == EXPWR_ENABLE) && ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE))) {
			if ((ferr = DO_GetE20EINF(&einfGetUvcc)) != FFWERR_OK) {
				return ferr;
			}
			if (eExPwrVal == EXPWR_V330) {			// 3.3V供給時
				fCmpUvcc = (float)3.3;
			} else if (eExPwrVal == EXPWR_V500) {	// 5.0V供給時
				fCmpUvcc = (float)5.0;
			} else {								// 1.8V供給時
				fCmpUvcc = (float)1.8;
			}
			fCmpUvcc = fCmpUvcc * (float)0.9;
			if (einfGetUvcc.flNowUVCC < fCmpUvcc) {	// (供給電圧値×0.9)未満の場合
				ferr = FFWERR_TARGET_POWERLOW;	// ユーザシステムの電圧が供給電圧より低い(Warning)を設定
			}
		}
	}
	// RevRxE2LNo141104-001 Append End

	// 電源供給しない場合、初回接続状態を未接続にする
	if (eExPwrEnable == EXPWR_DISABLE) {
		setFirstConnectInfo(FALSE);	// 未接続状態を設定
	}

	return ferr;
}
// 2008.11.18 INSERT_END_E20RX600 }

// 2009.6.22 INSERT_BEGIN_E20RX600(+NN) {
//=============================================================================
/**
 * BFWプログラムをFDTに遷移させる。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_TRANS_FDT(void)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	byBfwTransMode;
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

	byBfwTransMode = TRANS_MODE_FDT;
	ferr = DO_E20Trans(byBfwTransMode);

	return ferr;
}

//=============================================================================
/**
 * BFWプログラムをLevel1に遷移させる。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_TRANS_EML(void)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	byBfwTransMode;
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

	byBfwTransMode = TRANS_MODE_EML;
	ferr = DO_E20Trans(byBfwTransMode);
	// RevRxE2LNo141104-001 Modify Start
	if (ferr == FFWERR_OK) {
		// V.1.02 No.14,15 USB高速化対応 Append Line
		s_bLevelEml = TRUE;
	}
	// RevRxE2LNo141104-001 Modify End

	return ferr;
}

//=============================================================================
/**
 * BFWプログラムをLevel0に遷移させる。
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_TRANS_LV0(void)
{
	FFWERR	ferr = FFWERR_OK;
	BYTE	byBfwTransMode;
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

	byBfwTransMode = TRANS_MODE_LV0;
	ferr = DO_E20Trans(byBfwTransMode);
	// V.1.02 No.14,15 USB高速化対応 Append Line
	s_bLevelEml = FALSE;

	return ferr;
}

//=============================================================================
/**
 * Level0 + 1、データフラッシュ領域のサムチェックとLevel0のサム値を取得する。
 * @param pSum Level0領域のサム値
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE20Cmd_SUMCHECK(FFWE20_SUM_DATA* pSum)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;	// RevRxE2LNo141104-001 Append Line
	BOOL	bSetEinfData;		// RevRxE2LNo141104-001 Append Line
	FFWE20_EINF_DATA	Einf;	// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// エミュレータ情報未取得の場合、EINFコマンド処理を実行して取得する
	bSetEinfData = GetEinfDataSetFlg();
	if (bSetEinfData == FALSE) {
		if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxE2LNo141104-001 Append End

	getEinfData(&einfData);		// RevRxE2LNo141104-001 Append Line

	// RevRxE2LNo141104-001 Append Start
	// E2/E2 Liteの場合、何もせず正常終了を返す。
	if ((einfData.wEmuStatus == EML_E2) || (einfData.wEmuStatus == EML_E2LITE)) {
		pSum->bySumLevel0 = 0x00;
		pSum->bySumEml = 0x00;
		pSum->bySumFdt = 0x00;
		pSum->bySumBlockA = 0x00;
		return FFWERR_OK;
	}
	// RevRxE2LNo141104-001 Append End

	ferr = DO_E20SUMCHECK(pSum);

	return ferr;
}


// RevRxNo120910-001 Append Start
//=============================================================================
/**
 * モニタCPU空間のデータ設定(未対応)
 * @param pCpud        モニタCPU空間の設定データを格納
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWCmd_SetCPUD(const FFW_CPUD_DATA* pCpud)
{
	pCpud;		// コンパイル時のWarning対策

	return FFWERR_OK;
}
// RevRxNo120910-001 Append End


//=============================================================================
/* 最後のエラー取得
 * @param pdwErrorCode     該当するFFWエラーコード
 * @param szErrorMessage[][]	エラーパラメータ
 * @retval FFWエラーコード
 */
//=============================================================================
FFWERR FFWCmd_GetErrorDetail(DWORD* pdwErrorCode, 
							 char szErrorMessage[ERROR_MESSAGE_NUM_MAX][ERROR_MESSAGE_LENGTH_MAX])

{
	FFW_ERROR_DETAIL_INFO *pErrInfo;
	int i;
	// RevNo121017-003	Append Line
	errno_t ernerr;
	
	pErrInfo = GetErrorDetailInfo();

	// RevRxNo120910-007	Modify Line
	//	(2)	引数の*pdwErrrorCodeにｐErrorInfo->dwErrorCodeをロードする
	*pdwErrorCode = pErrInfo->dwErrorCode;

	for (i = 0; i < ERROR_MESSAGE_NUM_MAX; i++) {
		// RevRxNo120910-007	Modify Line
		// RevNo121017-003	Modify Line
		ernerr = strcpy_s(szErrorMessage[i], ERROR_MESSAGE_LENGTH_MAX, pErrInfo->szErrorMessage[i]);
	}
	
	InitErrorDetailInfo();

	return FFWERR_OK;
}


// RevRxNo170908-001 Append Start
typedef struct CSrecord_E2 CSrecord_E2;

//BFW解析用の構造体定義
struct CSrecord_E2 {
	CSrecord_E2 *(*Add)(CSrecord_E2 *first, DWORD length, DWORD addr, char *buffer);
	CSrecord_E2 *first;
	CSrecord_E2 *last;
	CSrecord_E2 *next;
	DWORD m_length;
	DWORD m_startaddr;
	BYTE m_databuffer[4];
};

CSrecord_E2 *m_CSrecord_E2_Add(CSrecord_E2 *first, DWORD length, DWORD addr, char *buffer)
{
	CSrecord_E2 *temp;

	temp = (CSrecord_E2 *)malloc(sizeof(CSrecord_E2) - 4 + length);
	if( temp == NULL ){
		return NULL;
	}

	temp->first = NULL;
	temp->last = NULL;
	temp->next = NULL;
	temp->m_length = length;
	temp->m_startaddr = addr;
	memcpy(temp->m_databuffer, buffer, length);

	if( first->first == NULL ){
		first->first = temp;
		first->last = temp;
		first->next = NULL;
	}
	else{
		first->last->next = temp;
		first->last = temp;
	}

	return temp;
}

CSrecord_E2 m_CSrecord_E2 = {&m_CSrecord_E2_Add, NULL, NULL, NULL, 0, 0, {0, 0, 0, 0}};

void free_CSRecord_E2()
{
	CSrecord_E2	*current, *next;

	current = m_CSrecord_E2.first;

	while(current != NULL){
		next = current->next;
		free(current);
		current = next;
	}

	m_CSrecord_E2.first = NULL;
}

static BYTE		s_byEmlVersionFlg;		// BFW EMLバージョンアドレスフラグ
static DWORD	s_dwEmlVersion;			// BFWファイル内の EMLバージョン保持
static BYTE		s_byEmlTargetMcuFlg;	// BFW EMLターゲットMCUアドレスフラグ
static WORD		s_wEmlTargetMcu;		// BFWファイル内の EMLターゲットMCU保持
static WORD		s_wEmlTargetMcuSub;		// BFWファイル内の EMLターゲットMCUサブ保持

static BYTE		s_byVerMcuRead;		// BFW EMLバージョン/MCU情報読み込み済みフラグ
#define	BFW_EML_VERSION_LOC		0x1	//bit0
#define	BFW_TARGET_MCU_LOC		0x2 //bit1

#define	BFW_LOAD_NONEED	0	//更新不要
#define	BFW_LOAD_NEED	1	//更新必要

//=============================================================================
/**
 * BFW Level EMLのダウンロード開始
 * @param dwTotalLength Monp_Send_E2で送信するBFWデータの総バイト数
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD Monp_Open_E2(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	char filename[MAX_PATH + 1];
	FFWE20_EINF_DATA	Einf;
	DWORD	dwBufSize;

	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		return ferr;
	}
	// BFW動作モードの確認
	if (Einf.byBfwMode != BFW_LV0) {
		// BFWの動作モードがLv0で無い場合、エラー終了
		ferr = FFWERR_BEXE_LEVEL0;
		return ferr;
	}

	// 制御CPUフラッシュ書き換えプログラム(MONPPROG)をダウンロード
	dwBufSize = static_cast<DWORD>(sizeof(filename));
	GetMonpprogFilename(filename, dwBufSize);	// MONPPROGのファイル名(フルパス)を取得

	if ((ferr = Load_wtr_e20(filename)) != FFWERR_OK) {	// MONPPROGのダウンロード
		return ferr;
	}

	// MONP_OPENコマンド送信
	ferr = DO_MONP_OPEN(dwTotalLength);

	//ゲージカウンタを0(%)にする
	*s_pdwGageCount = 0;

	return ferr;
}


//=============================================================================
/**
 * BFW Level EMLデータの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD Monp_Send_E2(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwBfwAreaEndAddr;

	pEmuDef = GetEmuDefData();

	// パラメータエラーチェック
	if (dwLength > MONP_LENGTH_MAX) {
		return FFWERR_FFW_ARG;
	}
	dwBfwAreaEndAddr = pEmuDef->dwBfwAreaStartAdr + pEmuDef->dwBfwAreaSize - 1;
	if ((dweadrAddr < pEmuDef->dwBfwAreaStartAdr) || (dweadrAddr > dwBfwAreaEndAddr)) {	
		// 開始アドレスがBFW領域開始アドレス～BFW領域終了アドレス の範囲外の場合
		return FFWERR_FFW_ARG;
	}
	if (dwLength > (dwBfwAreaEndAddr - dweadrAddr + 1)) {
		// データバイト数が、(開始アドレス～BFW領域終了アドレス)を超える場合
		return FFWERR_FFW_ARG;
	}

	ferr = DO_MONP_SEND(dweadrAddr, dwLength, pbyBfwBuff);

	return ferr;
}

//=============================================================================
/**
 * BFW Level EMLのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD Monp_Close_E2(void)
{
	FFWERR	ferr = FFWERR_OK;

	// MONP_CLOSEコマンド送信
	ferr = DO_MONP_CLOSE();

	*s_pdwGageCount = GAGE_COUNT_MAX;

	return ferr;
}


static BOOL get_hex_dw(char *p, DWORD *value, short cnt)
{
	long	val;					/* 数値 */

	for(val = 0L; cnt > 0; cnt--){
		val *= 0x10L;				/* 桁上げ */
		if(isdigit(*p)){		/* 0x0 ～ 0x9 */
			val += *p - '0';
		}else if(isxdigit(*p)){	/* 0xA ～ 0xF */
			if(isupper(*p))
				val += (*p - 'A') + 0xA;
			else
				return FALSE;		/* 小文字の場合 */
		}else{
			return FALSE;			/* 16進数でない場合 */
		}
		p++;						/* 次の文字へ */
	}
	*value = val;					/* 変換した数値を格納 */
	return TRUE;					/* 正常終了 */
}


//=============================================================================
/**
 * データを読み込み格納する
 * @param fp ファイルオープン
 * @param dataCnt データ数
 * @param data 格納先
 * @param checkSum チェックサム
 * @retval FFWERR_OK 正常終了
 * @retval FALSE 異常終了
 */
//=============================================================================
static BOOL read_data_from_file_e2(FILE *fp, UCHAR dataCnt, char *data, DWORD *checkSum)
{
	char	buf[0x10 * 2];		/* ファイルの入力バッファ */
	UCHAR	cnt;				/* 一度に処理するデータ数 */
	UCHAR	i;					/* 作業変数 */
	DWORD	value;				/* 作業変数 */
	DWORD	dwEmlTargetMcu;

	while (dataCnt > 0) {
		/* 1度に処理するデータ数 */
		if (dataCnt > 0x10) {
			cnt = 0x10;
		} else {
			cnt = dataCnt;
		}
		/* データの読み込み */
		if (fread(buf, 1, cnt * 2, fp) != (size_t)(cnt * 2)) {
			return FALSE;
		}
		// BFW EMLバージョンアドレスの場合、EMLバージョンを保持
		if (s_byEmlVersionFlg != FALSE) {
			get_hex_dw(&buf[0], &s_dwEmlVersion, 8);
		}
		// BFW EMLターゲットMCUアドレスの場合、ターゲットMCUを保持
		if (s_byEmlTargetMcuFlg != FALSE) {
			get_hex_dw(&buf[0], &dwEmlTargetMcu, 8);
			s_wEmlTargetMcuSub = (WORD)(dwEmlTargetMcu & 0x0000FFFF);
			s_wEmlTargetMcu = (WORD)((dwEmlTargetMcu & 0xFFFF0000) >> 16);
		}
		/* データの格納 */
		for (i = 0; i < cnt; i++) {
			/* データを数値変換、格納 */
			if (get_hex_dw(&buf[i * 2], &value, 2) == FALSE) {
				return FALSE;
			}
			*data = (UCHAR)value;
			/* チェックサムに加算 */
			*checkSum += *data;
			data++;		/* 次のデータへ */
		}
		dataCnt -= cnt;		/* データ数を更新 */
	}
	return TRUE;
}

//=============================================================================
/**
 * Sファイル/MOTファイルからHEXデータを取得しm_CSrecord_E2変数に格納する
 * @param fp ファイルポインタ
* @param byVercheck バージョンチェック情報。BFW_LOAD_NONEED(0):更新不要, BFW_LOAD_NEED(1):更新必要
 * @retval TRUE  正常終了
 * @retval FALSE 異常終了
 */
//=============================================================================
static BOOL Load_Mot_E2(FILE *fp, BYTE *byVercheck)
{
	UCHAR	recType;	/* [ レコードタイプ ] */
	UCHAR	dataCnt;	/* [ データ数 ]       */
	ULONG	offset;		/* [ 先頭アドレス ]   */
	DWORD	checkSum;	/* [ チェックサム ]   */
	char	buf[128];	/* ファイルの入力バッファ */
	DWORD	value;		/* 作業変数 */
	DWORD	sum;		/* チェックサム */
	ULONG	addr;		/* 開始アドレス */
	ULONG	sendAddr;	/* [ 開始アドレス ] */
	UINT	sendLength;	/* [ レングス ]     */
	char	buffer[BUFFER_MAX];

	*byVercheck = BFW_LOAD_NONEED;

	FFWE20_EINF_DATA	pEminfo;
	DWORD	dwEmlVersionAddr;
	DWORD	dwEmlTargetMcuAddr;

	s_byEmlVersionFlg = FALSE;
	s_byEmlTargetMcuFlg = FALSE;
	s_dwEmlVersion = 0;
	s_wEmlTargetMcu = 0;
	s_wEmlTargetMcuSub = 0;
	s_byVerMcuRead = 0;

	// エミュレータ種別取得のため、GETEMINFOコマンド処理を実行する
	// pEminfoの初期化込み
	DO_GetE20EINF(&pEminfo);

	// エミュレータ毎のBFW EMLバージョン格納領域とEMLターゲットMCU格納領域を設定
	if (pEminfo.wEmuStatus == EML_E2LITE) {
		dwEmlVersionAddr = BFWEML_VERSION_E2L;
		dwEmlTargetMcuAddr = BFWEML_TARGETMCU_E2L;
	} else if (pEminfo.wEmuStatus == EML_E2) {
		dwEmlVersionAddr = BFWEML_VERSION_E2;
		dwEmlTargetMcuAddr = BFWEML_TARGETMCU_E2;
	} else {
		dwEmlVersionAddr = 0;
		dwEmlTargetMcuAddr = 0;
	}

	/* S0スタートレコードのチェック */
	if (fgets((char *)buf, 128, fp) == NULL) {
		return FALSE;
	}
	/* 初期化処理 */
	sendAddr = (ULONG) - 1L;	/* [ 開始アドレス ] */
	sendLength = 0;				/* [ レングス ]     */
	checkSum = 0;				/* [チェックサム] */

	do {
		sum = 0;	/* 初期化 */

		/* スタートマーク、レコードタイプ、　*/
		/* データ数の読み込み		     */
		if (fread(buf, 1, 1 + 1 + 2 , fp) != 4) {
			return FALSE;
		}
		/* スタートマークのチェック */
		if (buf[0] != 'S') {
			return FALSE;
		}
		/* レコードタイプ、データ数の取得 */
		if (get_hex_dw(&buf[1], &value, 1 + 2) == FALSE) {
			return FALSE;
		}
		recType = (UCHAR)(value >> 8);
		dataCnt = (UCHAR)(value >> 0);
		/* レコードタイプのチェック */
		switch (recType) {
		case 0x00:
			if (dataCnt != 0x03) {
				return FALSE;
			}
			/* 先頭アドレス の読み込み */
			if (fread(buf, 1, 4, fp) != 4) {
				return FALSE;
			}
			/* 先頭アドレスの取得 */
			if (get_hex_dw(buf, &value, 4) == FALSE) {
				return FALSE;
			}
			offset  = (UINT)(value);
			if (offset != 0x0000) {
				return FALSE;
			}
			/* チェックサムに加算 */
			sum = sum + dataCnt;
			break;
		case 0x01:
			/* 先頭アドレス の読み込み */
			if (fread(buf, 1, 4, fp) != 4) {
				return FALSE;
			}
			/* 先頭アドレスの取得 */
			if (get_hex_dw(buf, &value, 4) == FALSE) {
				return FALSE;
			}
			/* BFW EMLバージョンアドレスだった場合フラグを立てる */
			if (value == dwEmlVersionAddr) {
				s_byEmlVersionFlg = TRUE;
				s_byVerMcuRead |= BFW_EML_VERSION_LOC;
			} else {
				s_byEmlVersionFlg = FALSE;
			}
			/* BFW EMLターゲットMCUアドレスだった場合フラグを立てる */
			if (value == dwEmlTargetMcuAddr) {
				s_byEmlTargetMcuFlg = TRUE;
				s_byVerMcuRead |= BFW_TARGET_MCU_LOC;
			} else {
				s_byEmlTargetMcuFlg = FALSE;
			}
			offset  = (UINT)(value);
			/* チェックサムに加算 */
			sum = sum + dataCnt;
			sum = sum + (offset >> 8);
			sum = sum + (offset >> 0);
			addr = offset;	/* 開始アドレス */
			/************** [ MONPコマンドの送信 ] **************/
			if (addr != (sendAddr + sendLength)) {
				/* データのアドレスが連続でない場合 */
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);

				/* 開始アドレス、レングスを再設定 */
				sendAddr = addr;
				sendLength = 0;
			}
			if ((sendLength + (dataCnt - (2 + 1))) > BUFFER_MAX) {
				/* 連続するデータがバッファの最大を越える場合 */
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);

				/* 開始アドレス、レングスを再設定 */
				sendAddr += sendLength;
				sendLength = 0;
			}
			/****************************************************/
			/* データの読み込み、格納 */
			if (read_data_from_file_e2(fp, (UCHAR)(dataCnt - (2 + 1)), &buffer[sendLength], &sum) != TRUE) {
				return FALSE;
			}
			/* バッファのデータ数の更新 */
			sendLength += (dataCnt - (2 + 1));
			break;
		case 0x02:
			/* 先頭アドレス の読み込み */
			if (fread(buf, 1, 6, fp) != 6) {
				return FALSE;
			}
			/* 先頭アドレスの取得 */
			if (get_hex_dw(buf, &value, 6) == FALSE) {
				return FALSE;
			}
			/* BFW EMLバージョンアドレスだった場合フラグを立てる */
			if (value == dwEmlVersionAddr) {
				s_byEmlVersionFlg = TRUE;
				s_byVerMcuRead |= BFW_EML_VERSION_LOC;
			} else {
				s_byEmlVersionFlg = FALSE;
			}
			/* BFW EMLターゲットMCUアドレスだった場合フラグを立てる */
			if (value == dwEmlTargetMcuAddr) {
				s_byEmlTargetMcuFlg = TRUE;
				s_byVerMcuRead |= BFW_TARGET_MCU_LOC;
			} else {
				s_byEmlTargetMcuFlg = FALSE;
			}
			offset  = (ULONG)value;
			/* チェックサムに加算 */
			sum = sum + dataCnt;
			sum = sum + (offset >> 16);
			sum = sum + (offset >>  8);
			sum = sum + (offset >>  0);
			addr = offset;	/* 開始アドレス */
			/************** [ MONPコマンドの送信 ] **************/
			if (addr != (sendAddr + sendLength)) {
				/* データのアドレスが連続でない場合 */
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);

				/* 開始アドレス、レングスを再設定 */
				sendAddr = addr;
				sendLength = 0;
			}
			if ((sendLength + (dataCnt - (3 + 1))) > BUFFER_MAX) {
				/* 連続するデータがバッファの最大を越える場合 */
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);

				/* 開始アドレス、レングスを再設定 */
				sendAddr += sendLength;
				sendLength = 0;
			}
			/****************************************************/
			/* データの読み込み、格納 */
			if (read_data_from_file_e2(fp, (UCHAR)(dataCnt - (3 + 1)), &buffer[sendLength], &sum) != TRUE) {
				return FALSE;
			}
			/* バッファのデータ数の更新 */
			sendLength += (dataCnt - (3 + 1));
			break;
		case 0x03:
			/* 先頭アドレス の読み込み */
			if (fread(buf, 1, 8, fp) != 8) {
				return FALSE;
			}
			/* 先頭アドレスの取得 */
			if (get_hex_dw(buf, &value, 8) == FALSE) {
				return FALSE;
			}
			/* BFW EMLバージョンアドレスだった場合フラグを立てる */
			if (value == dwEmlVersionAddr) {
				s_byEmlVersionFlg = TRUE;
				s_byVerMcuRead |= BFW_EML_VERSION_LOC;
			} else {
				s_byEmlVersionFlg = FALSE;
			}
			/* BFW EMLターゲットMCUアドレスだった場合フラグを立てる */
			if (value == dwEmlTargetMcuAddr) {
				s_byEmlTargetMcuFlg = TRUE;
				s_byVerMcuRead |= BFW_TARGET_MCU_LOC;
			} else {
				s_byEmlTargetMcuFlg = FALSE;
			}
			offset  = (ULONG)(value);
			/* チェックサムに加算 */
			sum = sum + dataCnt;
			sum = sum + (offset >> 24);
			sum = sum + (offset >> 16);
			sum = sum + (offset >>  8);
			sum = sum + (offset >>  0);
			addr = offset;	/* 開始アドレス */
			/************** [ MONPコマンドの送信 ] **************/
			if (addr != (sendAddr + sendLength)) {
				/* データのアドレスが連続でない場合 */
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);

				/* 開始アドレス、レングスを再設定 */
				sendAddr = addr;
				sendLength = 0;
			}
			if ((sendLength + (dataCnt - (4 + 1))) > BUFFER_MAX) {
				/* 連続するデータがバッファの最大を越える場合 */
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);

				/* 開始アドレス、レングスを再設定 */
				sendAddr += sendLength;
				sendLength = 0;
			}
			/****************************************************/
			/* データの読み込み、格納 */
			if (read_data_from_file_e2(fp, (UCHAR)(dataCnt - (4 + 1)), &buffer[sendLength], &sum) != TRUE) {
				return FALSE;
			}
			/* バッファのデータ数の更新 */
			sendLength += (dataCnt - (4 + 1));
			break;
		case 0x07:
			if (dataCnt != 0x05) {
				return FALSE;
			}
			/* 先頭アドレス の読み込み */
			if (fread(buf, 1, 8, fp) != 8) {
				return FALSE;
			}
			/* 先頭アドレスの取得 */
			if (get_hex_dw(buf, &value, 8) == FALSE) {
				return FALSE;
			}
			offset  = (ULONG)(value);
			/* チェックサムに加算 */
			sum = sum + dataCnt;
			sum = sum + (offset >> 24);
			sum = sum + (offset >> 16);
			sum = sum + (offset >>  8);
			sum = sum + (offset >>  0);
			{
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);
			}
			break;
		case 0x08:
			if (dataCnt != 0x04) {
				return FALSE;
			}
			/* 先頭アドレス の読み込み */
			if (fread(buf, 1, 6, fp) != 6) {
				return FALSE;
			}
			/* 先頭アドレスの取得 */
			if (get_hex_dw(buf, &value, 6) == FALSE) {
				return FALSE;
			}
			offset  = (ULONG)(value);
			/* チェックサムに加算 */
			sum = sum + dataCnt;
			sum = sum + (offset >> 16);
			sum = sum + (offset >>  8);
			sum = sum + (offset >>  0);
			{
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);
			}
			break;
		case 0x09:
			if (dataCnt != 0x03) {
				return FALSE;
			}
			/* 先頭アドレス の読み込み */
			if (fread(buf, 1, 4, fp) != 4) {
				return FALSE;
			}
			/* 先頭アドレスの取得 */
			if (get_hex_dw(buf, &value, 4) == FALSE) {
				return FALSE;
			}
			offset  = (UINT)(value);
			/* チェックサムに加算 */
			sum = sum + dataCnt;
			sum = sum + (offset >>  8);
			sum = sum + (offset >>  0);
			{
				m_CSrecord_E2.Add(&m_CSrecord_E2, sendLength, sendAddr, buffer);
			}
			break;
		default:
			/* レコードタイプが異常な場合 */
			return FALSE;
		}

		// バージョン情報と、MCU(サブ)情報の両方を読み込んだ後で、
		// ・BFWファイル内のEMLバージョンと、エミュレータのバージョンが同じ
		// ・BFWファイル内のMCU情報と、エミュレータのMCU情報が同じ
		// ・BFWファイル内のMCUサブ情報と、エミュレータのMCUサブ情報が同じ
		// の3つの条件が揃ったら、BFWは更新しない
		if( (s_byVerMcuRead & BFW_EML_VERSION_LOC) == BFW_EML_VERSION_LOC && 
			(s_byVerMcuRead & BFW_TARGET_MCU_LOC) == BFW_TARGET_MCU_LOC) {
			if( (s_dwEmlVersion == pEminfo.dwBfwEMLVer ) &&
			    (s_wEmlTargetMcu == pEminfo.wEmlTargetCpu) && 
			    (s_wEmlTargetMcuSub == pEminfo.wEmlTargetCpuSub) ){
					// byVercheck=BFW_LOAD_NEED(1)にして戻る
					*byVercheck = BFW_LOAD_NEED;
					return FALSE;
			}

			// 一度チェックしたらs_byVerMcuReadを0にしておく
			s_byVerMcuRead = 0;
		}

		/* チェックサムの読み込み、取得 */
		if (fread(buf, 1, 2, fp) != 2) {
			return FALSE;
		}
		if (get_hex_dw(buf, &value, 2) == FALSE) {
			return FALSE;
		}
		checkSum = value;
		/* チェックサムのチェック */
		sum = (sum & 0x000000FF);
		sum = (0xFF - sum);		/* 1の補数 */
		if (checkSum != sum) {
			return FALSE;
		}
		/* 改行文字の読み込みとチェック */
		if (fread(buf, 1, 1, fp) != 1) {
			return FALSE;
		}
		if (buf[0] != '\n') {
			return FALSE;
		}
	} while ((recType != 0x07) && (recType != 0x08) && (recType != 0x09));	/* エンドレコードまで */

	/* EOFのチェック */
	if (fread(buf, 1, 1, fp) == 1) {
		return FALSE;
	}
	return TRUE;
}

#define	BFW_MODIFIED		0
#define	BFW_LOAD_FAIL		1
#define	BFW_ALREADY_LOADED	2
#define	BFW_NOT_LEVEL0		3
#define	BFW_FROM_ERASE		4
#define	BFW_FROM_WRITE		5
#define	BFW_FROM_VERIFY		6
#define BFW_MONP_FILE		7
#define	BFW_ERR_COM			8

//=============================================================================
/**
 * 指定されたBFWファイルを使ってBFW(LevelEML)の更新を行う
 * @param filename BFWファイル名
 * @retval BFWロード情報。
 */
//=============================================================================
static BYTE Monp_E2( char *filename )
{
	char		extent[10];
	FILE		*fp;
	DWORD		dwRet, dwRetC;
	BOOL		bRet;
	DWORD		dwTotalLength = 0;	// 総バイト数格納変数
	CSrecord_E2	*tmpCSrd;			// ファイルデータ格納変数
	FFWE20_EINF_DATA pEinf;
	BYTE		byVercheck;
	BYTE		byRet = BFW_MODIFIED;

	// ファイル名から拡張子を取り出す
	_splitpath_s(filename, NULL, 0, NULL, 0, NULL, 0, extent, sizeof(extent));

	// (1)ファイル拡張子のチェック
	_strupr_s( extent, sizeof(extent) );	// データの大文字変換

	// 拡張子がモトローラSフォーマットであるか確認
	if(( strcmp(extent, ".S") != 0 ) && ( strcmp(extent, ".MOT") != 0 )){
		return BFW_LOAD_FAIL ;
	}

	// (2)BFW MOTファイルのオープン
	// WTRファイルオープン
	if( fopen_s( &fp, filename,"rt" ) != 0 ){
		// ファイルが正常にオープン出来なかった場合はエラー処理
		return BFW_LOAD_FAIL ;
	}

	// MOTファイルからHEXデータを取得しm_CSrecord_E2変数に格納する。
	bRet = Load_Mot_E2( fp, &byVercheck );
	fclose( fp );	// ファイルクローズ
	// MOTファイルのロード処理チェック
	if( bRet == FALSE ){
		// エラー終了の場合、処理を中断
		if( byVercheck == BFW_LOAD_NEED ){
			byRet = BFW_ALREADY_LOADED;
		} else { //BFW_LOAD_NONEED
			byRet = BFW_LOAD_FAIL;
		}

		free_CSRecord_E2(); // m_CSrecord_E2が確保した領域を開放する
		return byRet;
	}

	// (3)MONP_OPENコマンド送信

	getEinfData(&pEinf);

	// Stopダイアログ作成処理

	// MONP_OPENコマンドをFFWに送信
	if( (dwRet = Monp_Open_E2( dwTotalLength )) != FFWERR_OK ){
		// エラー終了の場合、処理を中断
		// BFWコードダウンロードの終了
		if(dwRet == FFWERR_BFWFROM_ERASE){
			if((dwRetC = Monp_Close_E2()) != FFWERR_OK) {
				dwRet = dwRetC;
			}
		}
		if (dwRet == FFWERR_BEXE_LEVEL0) {
			byRet = BFW_NOT_LEVEL0;
		} else if (dwRet == FFWERR_BFWFROM_ERASE) {
			byRet = BFW_FROM_ERASE;
		} else if (dwRet == FFWERR_BFWFROM_WRITE) {
			byRet = BFW_FROM_WRITE;
		} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
			byRet = BFW_FROM_VERIFY;
		} else if (dwRet == FFWERR_MONP_FILE_NOT_FOUND) {
			byRet = BFW_MONP_FILE;
		} else if (dwRet == FFWERR_COM) {
			byRet = BFW_ERR_COM;
		} else {
			byRet = BFW_LOAD_FAIL;
		}

		free_CSRecord_E2(); // m_CSrecord_E2が確保した領域を開放する
		return byRet;
	}

	// MONP_OPENが正常終了すると、モニタCPUはWTRプログラムの先頭にジャンプする。
	// WTRプログラムがコマンド受信待ち状態になるまで500ms程待つ
	Sleep( 500 );

	// (4)MONP_SENDコマンド送信
	// Stopダイアログ表示
	// BFW Level EMLの送信
	for( tmpCSrd = m_CSrecord_E2.first; tmpCSrd != NULL; tmpCSrd = tmpCSrd->next ){
		// レングスが0以上かチェック（レングスが0の場合はファイルデータ無し）	
		if( tmpCSrd->m_length > 0 ){
			if( (dwRet = Monp_Send_E2( (DWORD)tmpCSrd->m_startaddr,	// ライトアドレス
									 (DWORD)tmpCSrd->m_length,		// ライトレングス
									 tmpCSrd->m_databuffer ))		// ライトデータ
										!= FFWERR_OK ){
				// BFWコードダウンロードの終了
				if((dwRetC = Monp_Close_E2()) != FFWERR_OK) {
					dwRet = dwRetC;
				}

				if (dwRet == FFWERR_BFWFROM_ERASE) {
					byRet = BFW_FROM_ERASE;
				} else if (dwRet == FFWERR_BFWFROM_WRITE) {
					byRet = BFW_FROM_WRITE;
				} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
					byRet = BFW_FROM_VERIFY;
				} else if (dwRet == FFWERR_COM) {
					byRet = BFW_ERR_COM;
				} else {
					byRet = BFW_LOAD_FAIL;
				}
				free_CSRecord_E2(); // m_CSrecord_E2が確保した領域を開放する
				return byRet;
			}
		}
	}

	// (5)MONP_CLOSEコマンド送信
	//通常モード (直接DO_MONP()が呼ばれる)
	// ダウンロード完了通知
	if( (dwRet = Monp_Close_E2()) != FFWERR_OK ){
		// エラー終了の場合、処理を中断
		if (dwRet == FFWERR_BFWFROM_WRITE) {
			byRet = BFW_FROM_WRITE;
		} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
			byRet = BFW_FROM_VERIFY;
		} else if (dwRet == FFWERR_COM) {
			byRet = BFW_ERR_COM;
		} else {
			byRet = BFW_LOAD_FAIL;
		}
		free_CSRecord_E2(); // m_CSrecord_E2が確保した領域を開放する
		return byRet;
	}

	// MONP_CLOSEが正常終了すると、モニタCPUはBFWプログラムの先頭にジャンプする。
	// BFWプログラムがコマンド受信待ち状態になるまで500ms程待つ
	Sleep( 500 );

	byRet = BFW_MODIFIED;

	// m_CSrecord_E2が確保した領域を開放する
	free_CSRecord_E2();

	return byRet;
}


//=============================================================================
/**
 * BFW(LevelEML)の自動更新
 * @param szMcuFilename  MCUファイル名(絶対パス付き)
 * @param bBfwUpdate		BFWの更新有無判定結果(TRUE:更新した、FALSE:更新なし)
 * @retval FFWエラーコード
 */
//=============================================================================
FFWE100_API	DWORD FFWE2Cmd_BfwEmlUpdate(char szMcuFilename[], BOOL *bBfwUpdate)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;
	char	*p;
	char	fwiname[MAX_PATH + 1];
	char	bfwname[MAX_PATH + 1];
	char	filetmp[MAX_PATH + 1];
	BYTE	byData;
	BYTE	byMerr;
	FILE	*fp;

	*bBfwUpdate = FALSE;	//FALSE(更新なし)で初期化しておく
	DO_GetE20EINF(&einfData);

	// E1/E20/EZ-Cubeの場合、何もせずに未サポートエラーを返す。
	if ((einfData.wEmuStatus != EML_E2) && (einfData.wEmuStatus != EML_E2LITE)) {
		return FFWERR_FUNC_UNSUPORT;
	}

	//引数で渡された.mcuファイルが存在するか確認
	if( fopen_s( &fp, szMcuFilename,"rt" ) != 0 ){
		return FFWERR_MCU_FILE_NOT_FOUND;
	}
	fclose(fp);

	// 既にBFWがEMLモードならば、
	// FFWERR_BEXE_LEVEL0を返す
	if (einfData.byBfwMode != BFW_LV0) {
		return FFWERR_BEXE_LEVEL0;
	}

	//szMcuFilenameから、フォルダ名だけを取り出し、fwinameに保存する
	//.mcuファイルパスの、後ろから2つ目の'\'を取ると、FWIファイルがあるフォルダになる
	strcpy_s(fwiname, MAX_PATH, szMcuFilename);
#ifdef _WIN32
	p = strrchr(fwiname, '\\');
	fwiname[p-fwiname] = '\0';
	p = strrchr(fwiname, '\\');
	fwiname[p-fwiname] = '\0';
#else // __linux__
	p = strrchr(fwiname, '/');
	fwiname[p-fwiname] = '\0';
	p = strrchr(fwiname, '/');
	fwiname[p-fwiname] = '\0';
#endif

	//.mcuファイル内の[ENVIRONMENT] FirmwareSettingsの設定値であるファイル名を取り出し、
	//  fwinameの末尾にくっつける
	GetPrivateProfileString("ENVIRONMENT", "FirmwareSettings", "", filetmp, MAX_PATH, szMcuFilename);
	strcat_s(fwiname, MAX_PATH, "\\");
	strcpy_s(bfwname, MAX_PATH, fwiname); //BFWはFWIと同じフォルダにあるためコピーする
	strcat_s(fwiname, MAX_PATH, filetmp);

	//.fwiファイルが存在するか確認
	if (fopen_s(&fp, fwiname,"rt") != 0) {
		return FFWERR_BFW_FILE_NOT_FOUND;
	}
	fclose(fp);

	//FWIファイルを開き、
	//  - エミュレータがE2ならば、BFWFileNameE2の設定値をbfwnameに保存する
	//  - エミュレータがE2Lite or E2OB-miniならば、BFWFileNameE2Lの設定値をbfwnameに保存する
	if (einfData.wEmuStatus == EML_E2){
		GetPrivateProfileString("FILES", "BFWFileNameE2", "", filetmp, MAX_PATH, fwiname);
	} else {
		ferr = DO_CPUR(E2_OB_MINI_MAGIC_ADDR, EBYTE_ACCESS, 1, &byData);
		if (ferr != FFWERR_OK) {
		    return ferr;
		}
		if (byData == E2_OB_MINI_MAGIC_CODE) { //0x32
			GetPrivateProfileString("FILES", "BFWFileNameE2OBM", "", filetmp, MAX_PATH, fwiname);
		} else {
			GetPrivateProfileString("FILES", "BFWFileNameE2L", "", filetmp, MAX_PATH, fwiname);
		}
	}
	strcat_s(bfwname, MAX_PATH, filetmp);

	//BFWファイルをロードする。もしエラーが発生したなら、エラーを返す
	byMerr = Monp_E2(bfwname);
	if (byMerr == BFW_LOAD_FAIL){
		// BFW更新なし。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_BFW_FILE_NOT_FOUND;
	} else if (byMerr == BFW_ALREADY_LOADED){
		*bBfwUpdate = FALSE; //BFW更新なし
		ferr = FFWERR_OK;
	} else if (byMerr == BFW_FROM_ERASE) {
		// ROM ERASEエラー。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_BFWFROM_ERASE;
	} else if (byMerr == BFW_FROM_WRITE) {
		// ROM WRITEエラー。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_BFWFROM_WRITE;
	} else if (byMerr == BFW_FROM_VERIFY) {
		// ROM VERIFYエラー。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_BFWFROM_VERIFY;
	} else if (byMerr == BFW_MONP_FILE) {
		// MONPファイルオープンエラー。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_MONP_FILE_NOT_FOUND;
	} else if (byMerr == BFW_ERR_COM) {
		// 致命的通信エラー発生。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_COM;
	} else { // byMerr == BFW_LOADED
		*bBfwUpdate = TRUE; //BFW更新あり
		ferr = FFWERR_OK;
	}

	return ferr;
}

// RevRxE1E20No180921-001 Append Start

#define	MODE_EML	0
#define	MODE_FDT	2

typedef struct CSrecord_E1 CSrecord_E1;

//BFW解析用の構造体定義
struct CSrecord_E1 {
	CSrecord_E1 *(*Add)(CSrecord_E1 *first, DWORD length, DWORD addr, char *buffer);
	CSrecord_E1 *first;
	CSrecord_E1 *last;
	CSrecord_E1 *next;
	DWORD m_length;
	DWORD m_startaddr;
	BYTE m_databuffer[4];
};

CSrecord_E1 *s_CSrecord_E1_Add(CSrecord_E1 *first, DWORD length, DWORD addr, char *buffer)
{
	CSrecord_E1 *temp;

	temp = (CSrecord_E1 *)malloc(sizeof(CSrecord_E1) - 4 + length);
	if( temp == NULL ){
		return NULL;
	}

	temp->first = NULL;
	temp->last = NULL;
	temp->next = NULL;
	temp->m_length = length;
	temp->m_startaddr = addr;
	memcpy(temp->m_databuffer, buffer, length);

	if( first->first == NULL ){
		first->first = temp;
		first->last = temp;
		first->next = NULL;
	}
	else{
		first->last->next = temp;
		first->last = temp;
	}

	return temp;
}

static CSrecord_E1 s_CSrecord_E1 = {&s_CSrecord_E1_Add, NULL, NULL, NULL, 0, 0, {0, 0, 0, 0}};

void free_CSRecord_E1()
{
	CSrecord_E1	*current, *next;

	current = s_CSrecord_E1.first;

	while(current != NULL){
		next = current->next;
		free(current);
		current = next;
	}

	s_CSrecord_E1.first = NULL;
}


//=============================================================================
/**
 * BFW Level EMLのダウンロード開始
 * @param dwTotalLength FFWE20Cmd_MONP_SENDで送信するBFWデータの総バイト数
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD Monp_Open_E1E20(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	char	filename[MAX_PATH + 1];
	FFWE20_SUM_DATA		pSum;
	FFWE20_EINF_DATA	Einf;
	DWORD	dwBufSize;

	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		s_ferrMonpStat = ferr;
		return ferr;
	}

	if (Einf.byBfwMode != BFW_LV0) {
		// BFWの動作モードがLv0で無い場合、エラー終了
		ferr = FFWERR_BEXE_LEVEL0;
		s_ferrMonpStat = ferr;
		return ferr;
	}

	// E1/E20の場合、Level0のサム値を取得
	if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
		DO_E20SUMCHECK(&pSum);
		s_byMonpSum[3] = pSum.bySumLevel0;
	}

	// 制御CPUフラッシュ書き換えプログラム(MONPPROG)をダウンロード
	dwBufSize = static_cast<DWORD>(sizeof(filename));
	GetMonpprogFilename(filename, dwBufSize);	// MONPPROGのファイル名(フルパス)を取得

	if ((ferr = Load_wtr_e20(filename)) != FFWERR_OK) {	// MONPPROGのダウンロード
		s_ferrMonpStat = ferr;
		return ferr;
	}

	// MONP_OPENコマンド送信
	ferr = DO_MONP_OPEN(dwTotalLength);
	if (ferr == FFWERR_OK) {
		if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
			// コマンド正常終了時、Level0、1領域の2の補数格納変数を初期化
			s_byMonpSum[0] = 0xff;		// 2FFFChのライトデータ
			s_byMonpSum[1] = 0xff;		// 2FFFDhのライトデータ
			s_byMonpSum[2] = 0xff;		// 2FFFEhのライトデータ

			s_dwMonpDataSize = 0;
		}
	}

	// MONP_OPENの実行状況をコピー
	s_ferrMonpStat = ferr;

	//ゲージカウンタを0(%)にする
	*s_pdwGageCount = 0;

	return ferr;
}

//=============================================================================
/**
 * BFW Level EMLデータの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD Monp_Send_E1E20(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwBfwAreaEndAddr;
	FFWE20_EINF_DATA einfData;

	pEmuDef = GetEmuDefData();
	getEinfData(&einfData);

	// パラメータエラーチェック
	if (dwLength > MONP_LENGTH_MAX) {
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}

	dwBfwAreaEndAddr = pEmuDef->dwBfwAreaStartAdr + pEmuDef->dwBfwAreaSize - 1;
	if ((dweadrAddr < pEmuDef->dwBfwAreaStartAdr) || (dweadrAddr > dwBfwAreaEndAddr)) {	
		// 開始アドレスがBFW領域開始アドレス～BFW領域終了アドレス の範囲外の場合
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	if (dwLength > (dwBfwAreaEndAddr - dweadrAddr + 1)) {
		// データバイト数が、(開始アドレス～BFW領域終了アドレス)を超える場合
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}

	// E1/E20以外の場合
	if ((einfData.wEmuStatus != EML_E1) && (einfData.wEmuStatus != EML_E20)) {
			ferr = DO_MONP_SEND(dweadrAddr, dwLength, pbyBfwBuff);

	// E1/E20の場合、書き換え対象領域のサム値を算出して、BFWコード送信
	} else {
		// LV0の内容はサム値算出時に加算しない。
		if ((dweadrAddr < 0x10000) && (dweadrAddr + dwLength -1 >= 0x10000)) {
			for(DWORD dwCount = 0; dwCount < dweadrAddr+dwLength-0x10000; dwCount++ ){
				s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + dwCount + (0x10000 - dweadrAddr)));
			}
			s_dwMonpDataSize += dweadrAddr+dwLength-0x10000;
			ferr = DO_MONP_SEND(dweadrAddr, dwLength, pbyBfwBuff);
		} else {
			// EMLの10000h～2FFFBhの内容の総和値を算出
			if ((dweadrAddr >= 0x10000) && (dweadrAddr < 0x30000)) {
				// 最終4バイトを含む場合
				if ((dweadrAddr <= 0x2FFFC) && (0x2FFFC <= (dweadrAddr + dwLength - 1))) {
					s_dwMonpDataSize += dwLength;
					// 0x2FFFF番地へのライトデータが含まれている場合、0x2FFFF番地のデータはサム値計算に使用しないため、
					// 0x2FFFF番地のデータは無視をする
					if( (dweadrAddr <= 0x2FFFF) && (0x2FFFF <= (dweadrAddr + dwLength - 1)) ){
						s_dwMonpDataSize -= 1;
					}
					for(DWORD dwCount = 0; dwCount < dwLength-(dweadrAddr+dwLength-0x2FFFC); dwCount++ ){
						s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + dwCount));
					}
					s_byMonpSum[0] = *(pbyBfwBuff + (0x2FFFC - dweadrAddr));
					s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + (0x2FFFC - dweadrAddr)));
					if( (dweadrAddr <= 0x2FFFD) && (0x2FFFD <= (dweadrAddr + dwLength - 1)) ){
						s_byMonpSum[1] = *(pbyBfwBuff + (0x2FFFD - dweadrAddr));
						s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + (0x2FFFD - dweadrAddr)));
						if( (dweadrAddr <= 0x2FFFE) && (0x2FFFE <= (dweadrAddr + dwLength - 1)) ){
							s_byMonpSum[2] = *(pbyBfwBuff + (0x2FFFE - dweadrAddr));
							s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + (0x2FFFE - dweadrAddr)));
						}
					}
					// 最終4バイトはサム値と一緒にMONP_CLOSEでプログラムする。
					ferr = DO_MONP_SEND(dweadrAddr, dwLength-(dweadrAddr+dwLength-0x2FFFC), pbyBfwBuff);
				}
				else {
					for(DWORD dwCount = 0; dwCount < dwLength; dwCount++ ){
						s_byMonpSum[3] = (BYTE)(s_byMonpSum[3] + *(pbyBfwBuff + dwCount));
					}
					s_dwMonpDataSize += dwLength;
					ferr = DO_MONP_SEND(dweadrAddr, dwLength, pbyBfwBuff);
				}
			}
		}
	}

	// MONP_SENDの実行状況をコピー
	s_ferrMonpStat = ferr;

	return ferr;
}

//=============================================================================
/**
 * BFW Level EMLのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD Monp_Close_E1E20(void)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwLength;
	BYTE	bySum;
	FFWE20_EINF_DATA einfData;

	getEinfData(&einfData);

	// MONP_OPEN、MONP_SENDの実行状況を確認
	if (s_ferrMonpStat == FFWERR_OK) {
		if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {
			// MONPALL_OPEN、MONPALL_SENDコマンドが正常終了した場合は、チェックサム値をライトする
			// LV0, EML領域の空きデータの総和値を算出
			dwLength = 0x1FFFF - s_dwMonpDataSize;
			bySum = (BYTE)(dwLength * 0xFF);
			bySum = (BYTE)(bySum + s_byMonpSum[3]);
			s_byMonpSum[3] = (BYTE)((~bySum) + 1);
			ferr = DO_MONP_SEND(0x2fffc, 4, s_byMonpSum);
		}
	}

	// MONP_CLOSEコマンド送信
	ferr = DO_MONP_CLOSE();

	*s_pdwGageCount = GAGE_COUNT_MAX;

	return ferr;
}

//=============================================================================
/**
 * FDT領域BFW Level EMLのダウンロード開始
 * @param dwTotalLength FFWE20Cmd_MONPFDT_SENDで送信するBFWデータの総バイト数
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD MonpFdt_Open_E1E20(DWORD dwTotalLength)
{
	FFWERR	ferr = FFWERR_OK;
	char	filename[MAX_PATH + 1];
	FFWE20_EINF_DATA	Einf;
	FFWE20_SUM_DATA		pSum;
	DWORD	dwBufSize;

	// エミュレータ種別、BFW動作モード取得のため、EINFコマンド処理を実行する
	if ((ferr = DO_GetE20EINF(&Einf)) != FFWERR_OK) {
		s_ferrMonpFdtStat = ferr;
		return ferr;
	}

	if (Einf.byBfwMode != BFW_LV0) {
		// BFWの動作モードがLv0で無い場合、エラー終了
		ferr = FFWERR_BEXE_LEVEL0;
		s_ferrMonpStat = ferr;
		return ferr;
	}
	// E1/E20の場合、Level0のサム値を取得
	if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
		DO_E20SUMCHECK(&pSum);
		s_byMonpFdtSum[3] = pSum.bySumLevel0;
	}

	// 制御CPUフラッシュ書き換えプログラム(MONPPROG)をダウンロード
	dwBufSize = static_cast<DWORD>(sizeof(filename));
	GetMonpprogFilename(filename, dwBufSize);	// MONPPROGのファイル名(フルパス)を取得

	if ((ferr = Load_wtr_e20(filename)) != FFWERR_OK) {	// MONPPROGのダウンロード
		s_ferrMonpFdtStat = ferr;
		return ferr;
	}

	// MONPFDT_OPENコマンド送信
	ferr = DO_MONPFDT_OPEN(dwTotalLength);
	if (ferr == FFWERR_OK) {
		if ((Einf.wEmuStatus == EML_E1) || (Einf.wEmuStatus == EML_E20)) {
			// コマンド正常終了時、EML領域(LV0+EML)の2の補数格納変数を初期化
			s_byMonpFdtSum[0] = 0xff;		// 3FFFChのライトデータ
			s_byMonpFdtSum[1] = 0xff;		// 3FFFDhのライトデータ
			s_byMonpFdtSum[2] = 0xff;		// 3FFFEhのライトデータ

			s_dwMonpFdtDataSize = 0;
		}
	}

	// MONPFDT_OPENの実行状況をコピー
	s_ferrMonpFdtStat = ferr;

	//ゲージカウンタを0(%)にする
	*s_pdwGageCount = 0;

	return ferr;
}

//=============================================================================
/**
 * FDT領域BFW Level EMLデータの送信
 * @param dweadrAddr BFWコードデータの開始アドレス
 * @param dwLength BFWコードデータのバイト数(0x000_00000～0x0020_0000)
 * @param pbyBfwBuff BFWデータコードの格納バッファアドレス
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD MonpFdt_Send_E1E20(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff)
{
	FFWERR	ferr = FFWERR_OK;
	FFW_EMUDEF_DATA* pEmuDef;
	DWORD	dwBfwAreaEndAddr;
	FFWE20_EINF_DATA einfData;

	pEmuDef = GetEmuDefData();
	getEinfData(&einfData);

	// パラメータエラーチェック
	if (dwLength > MONP_LENGTH_MAX) {
		s_ferrMonpFdtStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}

	dwBfwAreaEndAddr = pEmuDef->dwBfwAreaStartAdr + pEmuDef->dwBfwAreaSize - 1;
	if ((dweadrAddr < pEmuDef->dwBfwAreaStartAdr) || (dweadrAddr > dwBfwAreaEndAddr)) {	
		// 開始アドレスがBFW領域開始アドレス～BFW領域終了アドレス の範囲外の場合
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}
	if (dwLength > (dwBfwAreaEndAddr - dweadrAddr + 1)) {
		// データバイト数が、(開始アドレス～BFW領域終了アドレス)を超える場合
		s_ferrMonpStat = FFWERR_FFW_ARG;
		return FFWERR_FFW_ARG;
	}

	// E1/E20以外の場合
	if ((einfData.wEmuStatus != EML_E1) && (einfData.wEmuStatus != EML_E20)) {
		ferr = DO_MONPFDT_SEND(dweadrAddr, dwLength, pbyBfwBuff);

	// E1/E20の場合、書き換え対象領域のサム値を算出して、BFWコード送信
	} else {
		// LV0およびEMLの内容はサム値算出時に加算しない。
		if ((dweadrAddr < 0x30000) && (dweadrAddr + dwLength -1 >= 0x30000)) {
			for(DWORD dwCount = 0; dwCount < dweadrAddr+dwLength-0x30000; dwCount++ ){
				s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + dwCount + (0x30000 - dweadrAddr)));
			}
			s_dwMonpFdtDataSize += dweadrAddr+dwLength-0x30000;
			ferr = DO_MONPFDT_SEND(dweadrAddr, dwLength, pbyBfwBuff);
		}
		// FDTの30000h～3FFFBhの内容の総和値を算出
		else if (dweadrAddr >= 0x30000) {
			// 最終4バイトを含む場合
			if ((dweadrAddr <= 0x3FFFC) && (0x3FFFC <= (dweadrAddr + dwLength - 1))) {
				s_dwMonpFdtDataSize += dwLength;
				// 0x3FFFF番地へのライトデータが含まれている場合、0x3FFFF番地のデータはサム値計算に使用しないため、
				// 0x3FFFF番地のデータは無視をする
				if( (dweadrAddr <= 0x3FFFF) && (0x3FFFF <= (dweadrAddr + dwLength - 1)) ){
					s_dwMonpFdtDataSize -= 1;
				}
				for(DWORD dwCount = 0; dwCount < dwLength-(dweadrAddr+dwLength-0x3FFFC); dwCount++ ){
					s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + dwCount));
				}
				s_byMonpFdtSum[0] = *(pbyBfwBuff + (0x3FFFC - dweadrAddr));
				s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + (0x3FFFC - dweadrAddr)));
				if( (dweadrAddr <= 0x3FFFD) && (0x3FFFD <= (dweadrAddr + dwLength - 1)) ){
					s_byMonpFdtSum[1] = *(pbyBfwBuff + (0x3FFFD - dweadrAddr));
					s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + (0x3FFFD - dweadrAddr)));
					if( (dweadrAddr <= 0x3FFFE) && (0x3FFFE <= (dweadrAddr + dwLength - 1)) ){
						s_byMonpFdtSum[2] = *(pbyBfwBuff + (0x3FFFE - dweadrAddr));
						s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + (0x3FFFE - dweadrAddr)));
					}
				}
				// 最終4バイトはサム値と一緒にMONP_CLOSEでプログラムする。
				ferr = DO_MONPFDT_SEND(dweadrAddr, dwLength-(dweadrAddr+dwLength-0x3FFFC), pbyBfwBuff);
			}
			else {
				for(DWORD dwCount = 0; dwCount < dwLength; dwCount++ ){
					s_byMonpFdtSum[3] = (BYTE)(s_byMonpFdtSum[3] + *(pbyBfwBuff + dwCount));
				}
				s_dwMonpFdtDataSize += dwLength;
				ferr = DO_MONPFDT_SEND(dweadrAddr, dwLength, pbyBfwBuff);
			}
		}
	}

	// MONP_SENDの実行状況をコピー
	s_ferrMonpFdtStat = ferr;

	return ferr;
}

//=============================================================================
/**
 * BFW Level FDTのダウンロード終了
 * @param なし
 * @retval FFWエラーコード
 */
//=============================================================================
DWORD MonpFdt_Close_E1E20(void)
{
	FFWERR	ferr = FFWERR_OK;
	DWORD	dwLength;
	BYTE	bySum;
	FFWE20_EINF_DATA einfData;

	getEinfData(&einfData);

	// MONP_OPEN、MONP_SENDの実行状況を確認
	if (s_ferrMonpFdtStat == FFWERR_OK) {
		if ((einfData.wEmuStatus == EML_E1) || (einfData.wEmuStatus == EML_E20)) {
			// MONP_OPEN、MONP_SENDコマンドが正常終了した場合は、チェックサム値をライトする
			// FDT領域内の空きデータの総和値を算出
			dwLength = 0xFFFF - s_dwMonpFdtDataSize;
			bySum = (BYTE)(dwLength * 0xFF);
			bySum = (BYTE)(bySum + s_byMonpFdtSum[3]);
			s_byMonpFdtSum[3] = (BYTE)((~bySum) + 1);
			ferr = DO_MONPFDT_SEND( 0x3fffc,4,s_byMonpFdtSum );
			// エラーが発生してもMONP_CLOSEは実行させる！
		}
	}

	// MONP_CLOSEコマンド送信
	ferr = DO_MONPFDT_CLOSE();

	*s_pdwGageCount = GAGE_COUNT_MAX;

	// TargetがFDT遷移コマンドを発行することによりFDTへ遷移する。

	return ferr;
}

static BOOL isExist_fdt = FALSE;

static BOOL read_data_from_file(FILE *fp, UCHAR dataCnt, char *data, UCHAR *checkSum)
{
	char	buf[0x10 * 2];		/* ファイルの入力バッファ */
	UCHAR	cnt;				/* 一度に処理するデータ数 */
	UCHAR	i;					/* 作業変数 */
	DWORD	value;				/* 作業変数 */

	while(dataCnt > 0){
		/* 1度に処理するデータ数 */
		if(dataCnt > 0x10){
			cnt = 0x10;
		}else{
			cnt = dataCnt;
		}
		/* データの読み込み */
		if(fread(buf, 1, cnt * 2, fp) != (size_t)(cnt * 2)){
			return FALSE;
		}
		/* データの格納 */
		for(i = 0; i < cnt; i++){
			/* データを数値変換、格納 */
			if(get_hex_data(&buf[i * 2], &value, 2) == FALSE){
				return FALSE;
			}
			*data = (UCHAR)value;
			/* チェックサムに加算 */
			*checkSum += *data;
			data++;		/* 次のデータへ */
		}
		dataCnt -= cnt;		/* データ数を更新 */
	}
	return TRUE;
}

static BOOL Load_Mot_E1E20(FILE *fp)
{
	UCHAR	recType;	/* [ レコードタイプ ] */
	UCHAR	dataCnt;	/* [ データ数 ]       */
	ULONG	offset;		/* [ 先頭アドレス ]   */
	UCHAR	checkSum;	/* [ チェックサム ]   */
	char	buf[128];	/* ファイルの入力バッファ */
	DWORD	value;		/* 作業変数 */
	UCHAR	sum;		/* チェックサム */
	ULONG	addr;		/* 開始アドレス */
	ULONG	sendAddr;	/* [ 開始アドレス ] */
	UINT	sendLength;	/* [ レングス ]     */
	int		dataNum, i;
	char	buffer[BUFFER_MAX + 1];

	isExist_fdt = FALSE; //FDT領域フラグをFALSEに初期化する

	/* S0スタートレコードのチェック */
	if (fgets((char *)buf, 128, fp) == NULL) {
		return FALSE;
	}
	/* 初期化処理 */
	sendAddr = (ULONG) -1L;		/* [ 開始アドレス ] */
	sendLength = 0;		/* [ レングス ]     */
	do{
		sum = 0;	/* 初期化 */

		/* スタートマーク、レコードタイプ、　*/
		/* データ数の読み込み		     */
		if (fread(buf, 1, 1 + 1 + 2 , fp) != 4) {
			return FALSE;
		}
		/* スタートマークのチェック */
		if (buf[0] != 'S') {
			return FALSE;
		}
		/* レコードタイプ、データ数の取得 */
		if (get_hex_data(&buf[1], &value, 1 + 2) == FALSE) {
			return FALSE;
		}	
		recType = (UCHAR)(value >> 8);
		dataCnt = (UCHAR)(value >> 0);
		/* レコードタイプのチェック */
		switch(recType) {
			case 0x00:
				if (dataCnt != 0x03) {
					return FALSE;
				}
		   		/* 先頭アドレス の読み込み */
		    	if (fread(buf, 1, 4, fp) != 4) {
					return FALSE;
				}
		    	/* 先頭アドレスの取得 */
		    	if (get_hex_data(buf, &value, 4) == FALSE) {
					return FALSE;
				}
	    		offset  = (UINT)(value);
				if (offset != 0x0000) {
					return FALSE;
				}
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
				dataNum = (int)((dataCnt - 3) * 2);
				if (fread(buf, 1, dataNum, fp) != (size_t)dataNum) {
					return FALSE;
				}
				for (i = 0; i < dataNum; i += 2) {
		    		if (get_hex_data(&buf[i], &value, 2) == FALSE) {
						return FALSE;
					}
					sum += (UCHAR)(value & 0xff);
				}
				break;
			case 0x01:
		   		/* 先頭アドレス の読み込み */
	    		if (fread(buf, 1, 4, fp) != 4) {
					return FALSE;
				}
	    		/* 先頭アドレスの取得 */
	    		if (get_hex_data(buf, &value, 4) == FALSE) {
					return FALSE;
				}
	    		offset  = (UINT)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 8);
	    		sum += (UCHAR)(offset >> 0);
				addr = offset;	/* 開始アドレス */
				/************** [ MONPコマンドの送信 ] **************/
				if (addr != (sendAddr + sendLength)) {
					/* データのアドレスが連続でない場合 */
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);

					/* 開始アドレス、レングスを再設定 */
					sendAddr = addr;
					sendLength = 0;
				}
				if ((sendLength + (dataCnt-(2+1))) > BUFFER_MAX) {
					/* 連続するデータがバッファの最大を越える場合 */
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);

					/* 開始アドレス、レングスを再設定 */
					sendAddr += sendLength;
					sendLength = 0;
				}
				/****************************************************/
				/* データの読み込み、格納 */
				if (read_data_from_file(fp, (dataCnt-(2+1)), &buffer[sendLength], &sum) != TRUE) {
					return FALSE;
				}
				/* バッファのデータ数の更新 */
				sendLength += (dataCnt-(2+1));
				break;
			case 0x02:
		   		/* 先頭アドレス の読み込み */
	    		if (fread(buf, 1, 6, fp) != 6) {
					return FALSE;
				}
	    		/* 先頭アドレスの取得 */
	    		if (get_hex_data(buf, &value, 6) == FALSE) {
					return FALSE;
				}
	    		offset  = (ULONG)value;
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 16);
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				addr = offset;	/* 開始アドレス */
				/************** [ MONPコマンドの送信 ] **************/
				if (addr != (sendAddr + sendLength)) {
					/* データのアドレスが連続でない場合 */
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);

					/* 開始アドレス、レングスを再設定 */
					sendAddr = addr;
					sendLength = 0;
				}
				if ((sendLength + (dataCnt-(3+1))) > BUFFER_MAX) {
					/* 連続するデータがバッファの最大を越える場合 */
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);

					/* 開始アドレス、レングスを再設定 */
					sendAddr += sendLength;
					sendLength = 0;
				}
				/****************************************************/
				/* データの読み込み、格納 */
				if (read_data_from_file(fp, (dataCnt-(3+1)), &buffer[sendLength], &sum) != TRUE) {
					return FALSE;
				}
				/* バッファのデータ数の更新 */
				sendLength += (dataCnt-(3+1));
				break;
			case 0x03:
	    		/* 先頭アドレス の読み込み */
	    		if (fread(buf, 1, 8, fp) != 8) {
					return FALSE;
				}
		   		/* 先頭アドレスの取得 */
	    		if (get_hex_data(buf, &value, 8) == FALSE) {
					return FALSE;
				}
	    		offset  = (ULONG)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 24);
	    		sum += (UCHAR)(offset >> 16);
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				addr = offset;	/* 開始アドレス */
				/************** [ MONPコマンドの送信 ] **************/
				if (addr != (sendAddr + sendLength)) {
					/* データのアドレスが連続でない場合 */
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);

					/* 開始アドレス、レングスを再設定 */
					sendAddr = addr;
					sendLength = 0;
				}
				if ((sendLength + (dataCnt-(4+1))) > BUFFER_MAX) {
					/* 連続するデータがバッファの最大を越える場合 */
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);

					/* 開始アドレス、レングスを再設定 */
					sendAddr += sendLength;
					sendLength = 0;
				}
				/****************************************************/
				/* データの読み込み、格納 */
				if (read_data_from_file(fp, (dataCnt-(4+1)), &buffer[sendLength], &sum) != TRUE) {
					return FALSE;
				}
				/* バッファのデータ数の更新 */
				sendLength += (dataCnt-(4+1));
				break;
			case 0x07:
				if (dataCnt != 0x05) {
					return FALSE;
				}
	    		/* 先頭アドレス の読み込み */
	    		if (fread(buf, 1, 8, fp) != 8) {
					return FALSE;
				}
	    		/* 先頭アドレスの取得 */
	    		if (get_hex_data(buf, &value, 8) == FALSE) {
					return FALSE;
				}
	    		offset  = (ULONG)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 24);
	    		sum += (UCHAR)(offset >> 16);
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				{
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);
				}
				break;
			case 0x08:
				if (dataCnt != 0x04) {
					return FALSE;
				}
	    		/* 先頭アドレス の読み込み */
	    		if (fread(buf, 1, 6, fp) != 6) {
					return FALSE;
				}
	    		/* 先頭アドレスの取得 */
	    		if (get_hex_data(buf, &value, 6) == FALSE) {
					return FALSE;
				}
	    		offset  = (ULONG)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >> 16);
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				{
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);
				}
				break;
			case 0x09:
				if (dataCnt != 0x03) {
					return FALSE;
				}
	    		/* 先頭アドレス の読み込み */
		   		if (fread(buf, 1, 4, fp) != 4) {
					return FALSE;
				}
	    		/* 先頭アドレスの取得 */
	    		if (get_hex_data(buf, &value, 4) == FALSE) {
					return FALSE;
				}
	    		offset  = (UINT)(value);
	    		/* チェックサムに加算 */
	    		sum += dataCnt;
	    		sum += (UCHAR)(offset >>  8);
	    		sum += (UCHAR)(offset >>  0);
				{
					s_CSrecord_E1.Add(&s_CSrecord_E1, sendLength, sendAddr, buffer);
				}
				break;
			default:
				/* レコードタイプが異常な場合 */
				return FALSE;
		}

		/* チェックサムの読み込み、取得 */
		if (fread(buf, 1, 2, fp) != 2) {
			return FALSE;
		}
		if (get_hex_data(buf, &value, 2) == FALSE) {
			return FALSE;
		}
		checkSum = (UCHAR)value;
		/* チェックサムのチェック */
		sum = (UCHAR)(~sum);		/* 1の補数 */
		if (checkSum != sum) {
			return FALSE;
		}
		/* 改行文字の読み込みとチェック */
		if (fgetc(fp) != '\n') {
			return FALSE;
		}

		// FDT領域がある場合は、isExist_fdtをTRUEにする
		if (isExist_fdt == FALSE && sendAddr >= BFWPRG_START_E20) {
			isExist_fdt = TRUE;
		}
	}while ((recType != 0x07) && (recType != 0x08) && (recType != 0x09));	/* エンドレコードまで */

	/* EOFのチェック */
	if (fread(buf, 1, 1, fp) == 1) {
		return FALSE;
	}
	return TRUE;
}


static BYTE DO_MONP_E1E20(int mode, int eml_sts=0xff)
{
	DWORD	dwRet, dwRetC;
	DWORD	i;
	BYTE	byRet;

	// (3)MONP_OPENコマンド送信
	DWORD	dwTotalLength = 0;	// 総バイト数格納変数
	CSrecord_E1 *tmpCSrd;			// ファイルデータ格納変数

	//接続エミュレータの判定
	FFWE20_EINF_DATA pEinf;
	DWORD dwEmlEndCode = 0xFFFFFFFF;
	DWORD dwFdtEndCode = 0xFFFFFFFF;
	DWORD dwEmlTergetLv0 = 0xFFFFFFFF;
	DWORD dwFdtTergetLv0 = 0xFFFFFFFF;
	DWORD dwEmlVersion = 0xFFFFFFFF;
	DWORD dwFdtVersion = 0xFFFFFFFF;
	WORD wEmlTargetMcu = 0xFFFF;
	WORD wEmlTargetMcuSub = 0xFFFF;
	char cEmlTargetMcuId[EINF_BFW_NAME_NUM+1];
	char cFdtTgtMucId[EINF_BFW_NAME_NUM+1];

	if ((dwRet = DO_GetE20EINF(&pEinf)) != FFWERR_OK) {
		if (dwRet != FFWERR_USB_VBUS_LOW) {
			return BFW_LOAD_FAIL;
		}
	}

	// MOTファイルデータのレングスを加算し、総バイト数を算出
	for( tmpCSrd = s_CSrecord_E1.first; tmpCSrd != NULL; tmpCSrd = tmpCSrd->next ){
		//ENDCODE値を参照
		if (tmpCSrd->m_length > 0) {
			BYTE*	pbyWriteBuf;

			if (mode == MODE_EML) {		// EML
				pbyWriteBuf = tmpCSrd->m_databuffer;
				//EML対応エンドコード情報
				if ((tmpCSrd->m_startaddr <= BFWEML_ENDCODE_E20) && (BFWEML_ENDCODE_E20+3 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length))) {
					dwEmlEndCode = (pbyWriteBuf[BFWEML_ENDCODE_E20 - tmpCSrd->m_startaddr] << 24) 
								| (pbyWriteBuf[BFWEML_ENDCODE_E20 - tmpCSrd->m_startaddr + 1] << 16)
								| (pbyWriteBuf[BFWEML_ENDCODE_E20 - tmpCSrd->m_startaddr + 2] << 8)
								| (pbyWriteBuf[BFWEML_ENDCODE_E20 - tmpCSrd->m_startaddr + 3] << 0);
				}
				//MOTファイルのEML対応LV0バージョン情報
				if ((tmpCSrd->m_startaddr <= BFWEML_LV0_E20) && (BFWEML_LV0_E20+3 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length))) {
					dwEmlTergetLv0 = (pbyWriteBuf[BFWEML_LV0_E20 - tmpCSrd->m_startaddr] << 24) 
								| (pbyWriteBuf[BFWEML_LV0_E20 - tmpCSrd->m_startaddr + 1] << 16)
								| (pbyWriteBuf[BFWEML_LV0_E20 - tmpCSrd->m_startaddr + 2] << 8)
								| (pbyWriteBuf[BFWEML_LV0_E20 - tmpCSrd->m_startaddr + 3] << 0);
				}
				// EMLのバージョン情報取得
				// 0x10000から4バイトがEMLのバージョン
				if ((tmpCSrd->m_startaddr <= BFWEML_VERSION_E20) && (BFWEML_VERSION_E20+3 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length))) {
					dwEmlVersion = (pbyWriteBuf[BFWEML_VERSION_E20 - tmpCSrd->m_startaddr] << 24) 
								| (pbyWriteBuf[BFWEML_VERSION_E20 - tmpCSrd->m_startaddr + 1] << 16)
								| (pbyWriteBuf[BFWEML_VERSION_E20 - tmpCSrd->m_startaddr + 2] << 8)
								| (pbyWriteBuf[BFWEML_VERSION_E20 - tmpCSrd->m_startaddr + 3] << 0);
				}
				// EMLのターゲットMCU情報取得
				// 0x10004から2バイトがEMLのターゲットMCU情報
				if ((tmpCSrd->m_startaddr <= BFWEML_MCU_E20) && (BFWEML_MCU_E20+1 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length))) {
					wEmlTargetMcu = (pbyWriteBuf[BFWEML_MCU_E20 - tmpCSrd->m_startaddr] << 8) 
								| (pbyWriteBuf[BFWEML_MCU_E20 - tmpCSrd->m_startaddr + 1] << 0);
				}
				// EMLのターゲットMCUサブ情報取得
				// 0x10006から2バイトがEMLのターゲットMCUサブ情報
				if ((tmpCSrd->m_startaddr <= BFWEML_MCUSUB_E20) && (BFWEML_MCUSUB_E20+1 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length))) {
					wEmlTargetMcuSub = (pbyWriteBuf[BFWEML_MCUSUB_E20 - tmpCSrd->m_startaddr] << 8) 
								| (pbyWriteBuf[BFWEML_MCUSUB_E20 - tmpCSrd->m_startaddr + 1] << 0);
				}
				// EMLのターゲットMCU ID情報取得
				// 0x10008から16バイトがEMLのターゲットMCUサブ情報
				if ((tmpCSrd->m_startaddr <= BFWEML_MCUID_E20) && BFWEML_MCUID_E20 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length -1)) {
					for (i = 0; i < 16; i++) {
						*(cEmlTargetMcuId + i) = pbyWriteBuf[BFWEML_MCUID_E20 - tmpCSrd->m_startaddr + i];
					}
					*(cEmlTargetMcuId + i) = '\0';
				}
			}
			else if (mode == MODE_FDT) {		// FDT
				pbyWriteBuf = tmpCSrd->m_databuffer;
				//FDT対応エンドコード情報
				if ((tmpCSrd->m_startaddr <= BFWPRG_ENDCODE_E20) && (BFWPRG_ENDCODE_E20+3 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length))) {
					dwFdtEndCode = (pbyWriteBuf[BFWPRG_ENDCODE_E20 - tmpCSrd->m_startaddr] << 24) 
								| (pbyWriteBuf[BFWPRG_ENDCODE_E20 - tmpCSrd->m_startaddr + 1] << 16)
								| (pbyWriteBuf[BFWPRG_ENDCODE_E20 - tmpCSrd->m_startaddr + 2] << 8)
								| (pbyWriteBuf[BFWPRG_ENDCODE_E20 - tmpCSrd->m_startaddr + 3] << 0);
				}
				//MOTファイルのFDT対応LV0バージョン情報
				if ((tmpCSrd->m_startaddr <= BFWPRG_LV0_E20) && (BFWPRG_LV0_E20+3 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length))) {
					dwFdtTergetLv0 = (pbyWriteBuf[BFWPRG_LV0_E20 - tmpCSrd->m_startaddr] << 24) 
								| (pbyWriteBuf[BFWPRG_LV0_E20 - tmpCSrd->m_startaddr + 1] << 16)
								| (pbyWriteBuf[BFWPRG_LV0_E20 - tmpCSrd->m_startaddr + 2] << 8)
								| (pbyWriteBuf[BFWPRG_LV0_E20 - tmpCSrd->m_startaddr + 3] << 0);
				}
				// FDTのバージョン情報取得
				// 0x3FF80から4バイトがFDTのバージョン
				if ((tmpCSrd->m_startaddr <= BFWPRG_VERSION_E20) && (BFWPRG_VERSION_E20+3 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length))) {
					dwFdtVersion = (pbyWriteBuf[BFWPRG_VERSION_E20 - tmpCSrd->m_startaddr] << 24) 
								| (pbyWriteBuf[BFWPRG_VERSION_E20 - tmpCSrd->m_startaddr + 1] << 16)
								| (pbyWriteBuf[BFWPRG_VERSION_E20 - tmpCSrd->m_startaddr + 2] << 8)
								| (pbyWriteBuf[BFWPRG_VERSION_E20 - tmpCSrd->m_startaddr + 3] << 0);
				}
				// FDT対応ターゲットMCU ID情報
				// 0x3FF84から16バイトがFDT MCU ID情報
				if ((tmpCSrd->m_startaddr <= BFWPRG_MCUID_E20) && BFWPRG_MCUID_E20 <= (tmpCSrd->m_startaddr + tmpCSrd->m_length -1)) {
					for (i = 0; i < 16; i++) {
						*(cFdtTgtMucId + i) = pbyWriteBuf[BFWPRG_MCUID_E20 - tmpCSrd->m_startaddr + i];
					}
					*(cFdtTgtMucId + i) = '\0';
				}
			}
		}
		// データレングスを加算
		dwTotalLength += tmpCSrd->m_length;
	}

	if ((pEinf.dwBfwLV0Ver != dwEmlTergetLv0) && (mode == MODE_EML)) {
		//エミュレータのLV0バージョンとMOTファイルのEML対応LV0バージョンが一致しない。
		//LV0が異なるのでMONPEMLできないようにする。
		return BFW_LOAD_FAIL;
	}
	if ((pEinf.dwBfwLV0Ver != dwFdtTergetLv0) && (mode == MODE_FDT)) {
		//エミュレータのLV0バージョンとMOTファイルのFDT対応LV0バージョンが一致しない。
		//LV0が異なるのでMONPFDTできないようにする。
		return BFW_LOAD_FAIL;
	}

	if (mode == MODE_EML) {
		// mode=0で以下の条件が全部揃っていれば、更新しない
		if ((pEinf.wEmlTargetCpu    == wEmlTargetMcu)    &&	// MCU情報が同じ
		    (pEinf.wEmlTargetCpuSub == wEmlTargetMcuSub) &&	// MCUサブ情報が同じ
		    (strncmp(pEinf.cEmlTgtMucId, cEmlTargetMcuId, 16) == 0)  &&	// MCU ID情報が同じ
		    (pEinf.dwBfwEMLVer      == dwEmlVersion)) {		// EMLバージョンが同じ
			return BFW_ALREADY_LOADED;
		}
	}

	if (mode == MODE_FDT) {
		if (eml_sts != BFW_MODIFIED) { // EMLが更新済みなら、FDTは必ず更新する
			// mode=2で以下の条件が全部揃っていれば、更新しない
			if ((strncmp(pEinf.cFdtTgtMucId, cFdtTgtMucId, 16) == 0) && // FDTターゲットMCUID
			    (pEinf.dwBfwFdtVer == dwFdtVersion)) {	// FDTバージョンが同じ
				return BFW_ALREADY_LOADED;
			}
		}
	}

	if ((mode == MODE_EML) && (dwEmlEndCode != 0xAAAAAAAA)) {
		//E1またはE20なのにMOTファイル上のEMLENDCODEが0xAAAAAAAAでない。
		return BFW_LOAD_FAIL;
	}
	if ((mode == MODE_FDT) && (dwFdtEndCode != 0xBBBBBBBB)) {
		//E1またはE20なのにMOTファイル上のFDTENDCODEが0xBBBBBBBBでない。
		return BFW_LOAD_FAIL;
	}

	// MONP_OPENコマンドをFFWに送信
	if (mode == MODE_EML) {
		// EML領域のみ
		if ((dwRet = Monp_Open_E1E20(dwTotalLength)) != FFWERR_OK) {
			// エラー終了の場合、処理を中断
			// エラー表示処理
			// BFWコードダウンロードの終了
			if (dwRet == FFWERR_BFWFROM_ERASE) {
				if ((dwRetC = Monp_Close_E1E20()) != FFWERR_OK) {
					dwRet = dwRetC;
				}
			}

			if (dwRet == FFWERR_BFWFROM_ERASE) {
				return BFW_FROM_ERASE;
			} else if (dwRet == FFWERR_BFWFROM_WRITE) {
				return BFW_FROM_WRITE;
			} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
				return BFW_FROM_VERIFY;
			} else if (dwRet == FFWERR_MONP_FILE_NOT_FOUND) {
				return BFW_MONP_FILE;
			} else if (dwRet == FFWERR_COM) {
				return BFW_ERR_COM;
			} else {
				return BFW_LOAD_FAIL;
			}
		}
	} else if (mode == MODE_FDT) {
		// FDT領域のみ
		if ((dwRet = MonpFdt_Open_E1E20(dwTotalLength)) != FFWERR_OK) {
			// エラー終了の場合、処理を中断
			// エラー表示処理
			// BFWコードダウンロードの終了
			if (dwRet == FFWERR_BFWFROM_ERASE) {
				if ((dwRetC = MonpFdt_Close_E1E20()) != FFWERR_OK) {
					dwRet = dwRetC;
				}
			}

			if (dwRet == FFWERR_BFWFROM_ERASE) {
				return BFW_FROM_ERASE;
			} else if (dwRet == FFWERR_BFWFROM_WRITE) {
				return BFW_FROM_WRITE;
			} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
				return BFW_FROM_VERIFY;
			} else if (dwRet == FFWERR_MONP_FILE_NOT_FOUND) {
				return BFW_MONP_FILE;
			} else if (dwRet == FFWERR_COM) {
				return BFW_ERR_COM;
			} else {
				return BFW_LOAD_FAIL;
			}
		} 
	}

	// MONP_OPENが正常終了すると、モニタCPUはWTRプログラムの先頭にジャンプする。
	// WTRプログラムがコマンド受信待ち状態になるまで500ms程待つ
	Sleep(500);

	// (4)MONP_SENDコマンド送信
	// モニタプログラムの送信
	long nlength = 0;
	for( tmpCSrd = s_CSrecord_E1.first; tmpCSrd != NULL; tmpCSrd = tmpCSrd->next ){
		// レングスが0以上かチェック（レングスが0の場合はファイルデータ無し）
		if (tmpCSrd->m_length > 0) {
			if (mode == MODE_EML) {
				// EML領域のみ
				if ((dwRet = Monp_Send_E1E20((DWORD)tmpCSrd->m_startaddr,	// ライトアドレス
												(DWORD)tmpCSrd->m_length,		// ライトレングス
												tmpCSrd->m_databuffer))		// ライトデータ
												!= FFWERR_OK) {
					// BFWコードダウンロードの終了
					if ((dwRetC = Monp_Close_E1E20()) != FFWERR_OK) {
						dwRet = dwRetC;
					}

					if (dwRet == FFWERR_BFWFROM_ERASE) {
						return BFW_FROM_ERASE;
					} else if (dwRet == FFWERR_BFWFROM_WRITE) {
						return BFW_FROM_WRITE;
					} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
						return BFW_FROM_VERIFY;
					} else if (dwRet == FFWERR_COM) {
						return BFW_ERR_COM;
					} else {
						return BFW_LOAD_FAIL;
					}
				}
			} else if (mode == MODE_FDT) {
				// FDT領域のみ
				if ((dwRet = MonpFdt_Send_E1E20((DWORD)tmpCSrd->m_startaddr,	// ライトアドレス
												(DWORD)tmpCSrd->m_length,		// ライトレングス
												tmpCSrd->m_databuffer))		// ライトデータ
												!= FFWERR_OK) {
					// BFWコードダウンロードの終了
					if ((dwRetC = MonpFdt_Close_E1E20()) != FFWERR_OK) {
						dwRet = dwRetC;
					}

					if (dwRet == FFWERR_BFWFROM_ERASE) {
						return BFW_FROM_ERASE;
					} else if (dwRet == FFWERR_BFWFROM_WRITE) {
						return BFW_FROM_WRITE;
					} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
						return BFW_FROM_VERIFY;
					} else if (dwRet == FFWERR_COM) {
						return BFW_ERR_COM;
					} else {
						return BFW_LOAD_FAIL;
					}
				}
			}
			nlength += tmpCSrd->m_length;
		}
	}

	// (5)MONP_CLOSEコマンド送信
	if (mode == MODE_EML) {
		//EMLモード (直接DO_MONP()が呼ばれる)
		// ダウンロード完了通知
		if ((dwRet = Monp_Close_E1E20()) != FFWERR_OK) {
			// エラー終了の場合、処理を中断
			if (dwRet == FFWERR_BFWFROM_WRITE) {
				byRet = BFW_FROM_WRITE;
			} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
				byRet = BFW_FROM_VERIFY;
			} else if (dwRet == FFWERR_COM) {
				return BFW_ERR_COM;
			} else {
				return BFW_LOAD_FAIL;
			}
		}
	} else if (mode == MODE_FDT) {
		//FDTモード  (DO_MONPFDT()経由でDO_MONP()が呼ばれる)
		// ダウンロード完了通知
		if ((dwRet = MonpFdt_Close_E1E20()) != FFWERR_OK) {
			// エラー終了の場合、処理を中断
			if (dwRet == FFWERR_BFWFROM_WRITE) {
				byRet = BFW_FROM_WRITE;
			} else if (dwRet == FFWERR_BFWFROM_VERIFY) {
				byRet = BFW_FROM_VERIFY;
			} else if (dwRet == FFWERR_COM) {
				return BFW_ERR_COM;
			} else {
				return BFW_LOAD_FAIL;
			}
		}
	}

	// MONP_CLOSEが正常終了すると、モニタCPUはBFWプログラムの先頭にジャンプする。
	// BFWプログラムがコマンド受信待ち状態になるまで500ms程待つ
	Sleep(500);

	return BFW_MODIFIED;
}

static BYTE MONP_E1E20(char *filename)
{
	char	extent[10];
	BYTE	byRet_eml = BFW_ALREADY_LOADED;
	BYTE	byRet_fdt = BFW_ALREADY_LOADED;
	BYTE	byRet;

	_splitpath_s(filename, NULL, 0, NULL, 0, NULL, 0, extent, sizeof(extent));

	// (1)ファイル拡張子のチェック
	_strupr_s( extent, sizeof(extent) );	// データの大文字変換

	// 拡張子がモトローラSフォーマットであるか確認
	if ((strcmp(extent, ".S") != 0) && (strcmp(extent, ".MOT") != 0)) {
		return BFW_LOAD_FAIL;
	}

	// (2)BFW MOTファイルのオープン
	FILE	*fp;

	// ファイル名称に作業パスを付加
	// strcpy(filename, GetInPath(filename)); // 元々パス付ファイル名のため処理を省く
	// WTRファイルオープン
	if (fopen_s(&fp, filename,"rt") != 0) {
		// ファイルが正常にオープン出来なかった場合はエラー処理
		return BFW_LOAD_FAIL;
	}

	// MOTファイルからHEXデータを取得しCsRecord_E1変数に格納する。
	// EML&FDTをまとめて格納
	byRet = (BYTE)Load_Mot_E1E20(fp);

	fclose(fp);	// ファイルクローズ
	if (byRet == FALSE) {
		free_CSRecord_E1(); // s_CSrecord_E1が確保した領域を開放する
		return BFW_LOAD_FAIL;
	}

	// EML領域の書き換え
	byRet_eml = DO_MONP_E1E20(MODE_EML);

	// ALREADY_LOADED/MODIFIED以外なら、即リターン
	if (byRet_eml != BFW_ALREADY_LOADED && byRet_eml != BFW_MODIFIED) {
		byRet = byRet_eml;
		free_CSRecord_E1(); // s_CSrecord_E1が確保した領域を開放する
		return byRet;
	}

	if (isExist_fdt == TRUE) {
		// FDT領域の書き換え
		byRet_fdt = DO_MONP_E1E20(MODE_FDT, byRet_eml);
	}

	// ALREADY_LOADED/MODIFIED以外なら、即リターン
	if (byRet_fdt != BFW_ALREADY_LOADED && byRet_fdt != BFW_MODIFIED) {
		byRet = byRet_fdt;
		free_CSRecord_E1(); // s_CSrecord_E1が確保した領域を開放する
		return byRet;
	}

	if (byRet_eml == BFW_ALREADY_LOADED && byRet_fdt == BFW_ALREADY_LOADED) {
		byRet = BFW_ALREADY_LOADED;
	} else {
		byRet = BFW_MODIFIED;
	}

	// s_CSrecord_E1が確保した領域を開放する
	free_CSRecord_E1();

	return byRet;
}

FFWE100_API	DWORD FFWE20Cmd_BfwEmlUpdate(char szMcuFilename[], BOOL *bBfwUpdate)
{
	FFWERR	ferr = FFWERR_OK;
	FFWE20_EINF_DATA	einfData;
	char	*p;
	char	fwiname[MAX_PATH + 1];
	char	bfwname[MAX_PATH + 1];
	char	filetmp[MAX_PATH + 1];
	BYTE	byMerr;
	FILE	*fp;

	*bBfwUpdate = FALSE;	//FALSE(更新なし)で初期化しておく
	DO_GetE20EINF(&einfData);

	// E2/E2Lite/EZ-Cubeの場合、何もせずに未サポートエラーを返す。
	if ((einfData.wEmuStatus != EML_E1) && (einfData.wEmuStatus != EML_E20)) {
		return FFWERR_FUNC_UNSUPORT;
	}

	//引数で渡された.mcuファイルが存在するか確認
	if (fopen_s(&fp, szMcuFilename,"rt") != 0) {
		return FFWERR_MCU_FILE_NOT_FOUND;
	}
	fclose(fp);

	// 既にBFWがEMLモードならば、
	// FFWERR_BEXE_LEVEL0を返す
	if (einfData.byBfwMode != BFW_LV0) {
		return FFWERR_BEXE_LEVEL0;
	}

	//szMcuFilenameから、フォルダ名だけを取り出し、fwinameに保存する
	//.mcuファイルパスの、後ろから2つ目の'\'を取ると、FWIファイルがあるフォルダになる
	strcpy_s(fwiname, MAX_PATH, szMcuFilename);
#ifdef _WIN32
	p = strrchr(fwiname, '\\');
	fwiname[p-fwiname] = '\0';
	p = strrchr(fwiname, '\\');
	fwiname[p-fwiname] = '\0';
#else // __linux__
	p = strrchr(fwiname, '/');
	fwiname[p-fwiname] = '\0';
	p = strrchr(fwiname, '/');
	fwiname[p-fwiname] = '\0';
#endif

	//.mcuファイル内の[ENVIRONMENT] FirmwareSettingsの設定値であるファイル名を取り出し、
	//  fwinameの末尾にくっつける
	GetPrivateProfileString("ENVIRONMENT", "FirmwareSettings", "", filetmp, MAX_PATH, szMcuFilename);
	strcat_s(fwiname, MAX_PATH, "\\");
	strcpy_s(bfwname, MAX_PATH, fwiname); //BFWはFWIと同じフォルダにあるためコピーする
	strcat_s(fwiname, MAX_PATH, filetmp);

	//.fwiファイルが存在するか確認
	if (fopen_s(&fp, fwiname,"rt") != 0) {
		return FFWERR_BFW_FILE_NOT_FOUND;
	}
	fclose(fp);

	//FWIファイルを開き、
	//  - エミュレータがE1ならば、BFWFileNameE1の設定値をbfwnameに保存する
	//  - エミュレータがE20ならば、BFWFileNameE20の設定値をbfwnameに保存する
	if (einfData.wEmuStatus == EML_E1) {
		GetPrivateProfileString("FILES", "BFWFileNameE1", "", filetmp, MAX_PATH, fwiname);
	} else {
		GetPrivateProfileString("FILES", "BFWFileNameE20", "", filetmp, MAX_PATH, fwiname);
	}
	strcat_s(bfwname, MAX_PATH, filetmp);

	//BFWファイルをロードする。もしエラーが発生したなら、エラーを返す

	byMerr = MONP_E1E20(bfwname);
	if (byMerr == BFW_LOAD_FAIL) {
		// BFW更新なし。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_BFW_FILE_NOT_FOUND;
	} else if (byMerr == BFW_ALREADY_LOADED) {
		*bBfwUpdate = FALSE; //BFW更新なし
		ferr = FFWERR_OK;
	} else if (byMerr == BFW_FROM_ERASE) {
		// ROM ERASEエラー。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_BFWFROM_ERASE;
	} else if (byMerr == BFW_FROM_WRITE) {
		// ROM WRITEエラー。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_BFWFROM_WRITE;
	} else if (byMerr == BFW_FROM_VERIFY) {
		// ROM VERIFYエラー。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_BFWFROM_VERIFY;
	} else if (byMerr == BFW_MONP_FILE) {
		// MONPファイルオープンエラー。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_MONP_FILE_NOT_FOUND;
	} else if (byMerr == BFW_ERR_COM) {
		// 致命的通信エラー発生。bBfwUpdate=FALSEは初期化時に済。
		ferr = FFWERR_COM;
	} else { // byMerr == BFW_MODIFIED
		*bBfwUpdate = TRUE; //BFW更新あり
		ferr = FFWERR_OK;
	}

	return ferr;
}
// RevRxE1E20No180921-001 Append End


#if defined(E2E2L)
void ChkE2IndividualCurrentData(void)
{
	BYTE byBuff[10] = {0};
	DWORD dwRet;
	double dblData = 0.027;

	dwRet = PROT_GetLID(0x0000071E, 0x0A, byBuff);

	if (dwRet == FFWERR_OK) {
		// Bigエンディアン(上位、下位の順)で返るので、リトルのWORD形式に並べ直し、設定
		SetE2BaseCurrentVal(dblData,
							(((WORD)byBuff[0]) << 8) | byBuff[1],
							(((WORD)byBuff[2]) << 8) | byBuff[3],
							(((WORD)byBuff[4]) << 8) | byBuff[5],
							(((WORD)byBuff[6]) << 8) | byBuff[7],
							(((WORD)byBuff[8]) << 8) | byBuff[9]);
	}
	else {
		// LID領域がブランク状態だった場合、暫定値を使用する。
		SetE2BaseCurrentVal(dblData,  0x000D, 0x001A, 0x0036, 0x04F, 0x059);
	}
}
#endif
// RevRxNo170908-001 Append End

//=============================================================================
/**
 * OSバージョン情報の参照
 * @param なし
 * @retval OSバージョン情報格納構造体アドレス
*/
//=============================================================================
FFW_WINVER_DATA* GetWinVersion()
{
	return &s_WinVer;
}

//=============================================================================
/**
 * 進捗ゲージカウンタ値の設定
 *   進捗ゲージカウンタのクリアは、Target.DLLが行う。
 * @param dwCount 進捗ゲージカウンタ値(0～1000)
 * @retval なし
 */
//=============================================================================
void SetGageCount(DWORD dwCount)
{
	*s_pdwGageCount = dwCount;
}

//=============================================================================
/**
 * 進捗ゲージカウンタに表示するメッセージ文字列の設定
 * @param pchMsgGageCount メッセージ文字列の先頭アドレス
 * @retval なし
 */
//=============================================================================
void SetMsgGageCount(char* pchMsgGageCount)
{
	// RevNo121017-003	Modify Start
	errno_t ernerr;

	ernerr = strcpy_s(s_chMsgGageCount, GAGE_COUNT_MSG_MAX, pchMsgGageCount);
	// RevNo121017-003	Modify End
}

//=============================================================================
/**
 * FFW受信タイムアウト値の参照
 * @param なし
 * @retval FFW受信タイムアウト値(単位:秒)
*/
//=============================================================================
WORD GetFfwTimeOut(void)
{
	return s_wFfwTimeOut;
}

// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * BFWタイムアウト値の参照
 * @param なし
 * @retval BFWタイムアウト値(単位:秒)
*/
//=============================================================================
WORD GetBfwTimeOut(void)
{
	return s_wBfwTimeOut;
}
// RevRxE2LNo141104-001 Append End

//=============================================================================
/**
 * システムコマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwIfData_Sys(void)
{
	s_wFfwTimeOut = INIT_FFW_TMOUT;		// FFW用受信タイムアウト
	s_wBfwTimeOut = INIT_BFW_TMOUT;		// BFW用受信タイムアウト
	s_ferrDcnfStat = FFWERR_OK;			// FFWCmd_DCNF_SEND実行状況
}


// RevRxE2LNo141104-001 Append Start
//=============================================================================
/**
 * 制御CPUフラッシュ書き換えプログラム(MONPPROG)のファイル名(フルパス)取得
 * @param pFilename ファイル名格納アドレス
 * @param dwBufSize ファイル名格納領域のサイズ((MAX_PATH+1)以上)
 * @retval なし
 */
//=============================================================================
void GetMonpprogFilename(char* pFilename, DWORD dwBufSize)
{
	char filename[MAX_PATH + 1];
	HINSTANCE hFfwDll;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	errno_t	ernerr;
	int		ierr;
	FFW_EMUDEF_DATA* pEmuDef;

	pEmuDef = GetEmuDefData();

	// ffw.dllのフルパスを得る。
	hFfwDll = GetHandle();
	GetModuleFileName(hFfwDll, filename, sizeof(filename));

	// ffw.dllのフルパスからファイル名を削除
	// RevNo121017-003	Modify Line
	ernerr = _splitpath_s(filename, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);

	// パス名生成(ファイル名の前まで)
	// RevNo121017-003	Modify Line
	ierr = sprintf_s(filename, _countof(filename), "%s%s", drive, dir);

	// パス名生成(ファイル名付加)
	// RevNo121017-003	Modify Line
	ernerr = strcat_s(filename, _countof(filename), pEmuDef->szMonpprogFileName);

	// 生成したファイル名(フルパス)を指定されたアドレスに格納
	ernerr = strcpy_s(pFilename, dwBufSize, filename);

	return;
}
// RevRxE2LNo141104-001 Append End


//=============================================================================
/**
 * 制御CPUフラッシュ書き換えプログラム(MONPPROG)ファイルのダウンロード
 * @param *filename　ファイル名
 * @retval FFWエラーコード
 */
//=============================================================================
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 2009.1.23 SDS Ueda 追加
// RevRxE2LNo141104-001 Modify Line
FFWERR Load_wtr_e20( char *filename )
{
	FFWERR ferr = FFWERR_OK;
	FILE	*fp;						//ファイル
	char	data_buffer[256];			//データバッファ
	int		count;						//data_bufferのカウンタ(配列番号)
	DWORD	dwHexData;
	int	i;
	DWORD	dwAddr;
	DWORD	dwLength;
	DWORD	dwLenOfs;
	DWORD	dwCpuWAddr;
	BYTE	byDataBuffer[CPU_W_SIZE];
	int		nAddrPoint,nDataPoint;
	DWORD	dwCount;
	BOOL	bAddrSet = FALSE;
	BOOL	bCpu_wFlg = FALSE;
	BOOL	bDataSet = FALSE;
	// RevNo121017-003	Append Line
	errno_t	ernerr;

	// ファイルオープン
	// RevNo121017-003	Modify Start
	ernerr = fopen_s( &fp, filename, "r");
	if( fp == NULL ) {
		return FFWERR_MONP_FILE_NOT_FOUND;
	}
	// RevNo121017-003	Modify End

	dwCount = 0;
	dwCpuWAddr = 0;
	//データの取得
	while ( (fgets(data_buffer,256,fp)) != NULL ) {
		if ( data_buffer[1] == '1' ) {
			// S1フォーマット
			nAddrPoint = 4;
			nDataPoint = 8;
			dwLenOfs = 3;
		}
		else if ( data_buffer[1] == '2' ) {
			// S2フォーマット
			nAddrPoint = 6;
			nDataPoint = 10;
			dwLenOfs = 4;
		}
		else if ( data_buffer[1] == '3' ) {
			// S3フォーマット
			nAddrPoint = 8;
			nDataPoint = 12;
			dwLenOfs = 5;
		}
		else{
			// それ以外のフォーマットの場合
			continue;	
		}

		// HEXファイルのレングスを取得
		get_hex_data ( &data_buffer[2], &dwLength,2 );
		// HEXファイル1ラインのデータ数を算出
		dwLength -= dwLenOfs;

		// HEXファイルのアドレスを取得
		get_hex_data ( &data_buffer[4], &dwAddr, (short)nAddrPoint );
		
		if( bAddrSet != TRUE ){
			// CPU_W実行時のライトアドレスを設定
			bAddrSet = TRUE;
			dwCpuWAddr = dwAddr;
		}
		else{
			// HEXファイル内でアドレスが跳んでいないか確認
			DWORD	dwNextAddr = dwCpuWAddr + dwCount;
			if( dwNextAddr != dwAddr ){
				// HEXファイル内でアドレスが移動しているので、CPU_W用のバッファへの書き込みは終了
				// CPU_Wコマンドを実行する。
				bCpu_wFlg = bDataSet = TRUE;
				goto CPU_W_EXCUTE;
			}

			// データレングス長が1000hバイトを超えているか確認
			if( (dwCount + dwLength) >= CPU_W_SIZE ){
				bCpu_wFlg = bDataSet = TRUE;
				goto CPU_W_EXCUTE;	
			}
		}
		
		// HEWファイルのデータを取得
		for ( count=nDataPoint, i = 0; data_buffer[count+3] != '\0'; count+=2, i++ ) {
			get_hex_data ( &data_buffer[count], &dwHexData, 2 );	
			byDataBuffer[dwCount++] = (const BYTE)dwHexData;
		}

CPU_W_EXCUTE:
		// CPU_Wコマンド送信処理
		if( bCpu_wFlg == TRUE ){
			bCpu_wFlg = FALSE;
			if( (ferr = DO_CPUW( dwCpuWAddr, EBYTE_ACCESS, dwCount, byDataBuffer )) != FFWERR_OK) {
				break;
			}

			bAddrSet = FALSE;
			dwCount = 0;
		}

		if( bDataSet == TRUE ){
			bDataSet = FALSE;
			bAddrSet = TRUE;

			dwCpuWAddr = dwAddr;
			// HEWファイルのデータを取得
			for ( count=nDataPoint, i = 0; data_buffer[count+3] != '\0'; count+=2, i++ ) {
				get_hex_data ( &data_buffer[count], &dwHexData, 2 );	
				byDataBuffer[dwCount++] = (const BYTE)dwHexData;
			}
		}
	}

	if( dwCount != 0 && ferr == FFWERR_OK ){
		ferr = DO_CPUW( dwCpuWAddr, EBYTE_ACCESS, dwCount, byDataBuffer );
	} 

	fclose(fp);

	return ferr;
}

//=============================================================================
/**
 * HEXファイルデータをバイナリ値に変換
 * @param p　HEXデータ
 * @param value　変換値格納データ
 * @param cnt 変換文字数 
 * @retval TRUE:正常終了
 */
//=============================================================================
static BOOL get_hex_data(char *p, DWORD *value, short cnt)
{
	long	val;					/* 数値 */
	int		n;						// RevRxNo130730-011 Append Line

	for(val = 0L; cnt > 0; cnt--){
		val *= 0x10L;				/* 桁上げ */
		// RevRxNo130730-011 Modify Start
		n = (int)*p & 0x000000ff;
		if(isdigit(n)){		/* 0x0 ～ 0x9 */
			val += *p - '0';
		}else if(isxdigit(n)){	/* 0xA ～ 0xF */
			if(isupper(n))
		// RevRxNo130730-011 Modify End
				val += (*p - 'A') + 0xA;
			else
				return FALSE;		/* 小文字の場合 */
		}else{
			return FALSE;			/* 16進数でない場合 */
		}
		p++;						/* 次の文字へ */
	}
	*value = val;					/* 変換した数値を格納 */
	return TRUE;					/* 正常終了 */
}

//=============================================================================
/**
 * 最後に発生したエラー情報（FFW内部変数）を正常状態に初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitErrorDetailInfo(void) 
{
	BYTE i;
	// RevNo121017-003	Append Line
	errno_t ernerr;

	s_ErrorDetailInfo.dwErrorCode = FFWERR_OK;

	for (i = 0; i < ERROR_MESSAGE_NUM_MAX; i++) {
		// RevNo121017-003	Modify Line
		ernerr = strcpy_s(s_ErrorDetailInfo.szErrorMessage[i], ERROR_MESSAGE_LENGTH_MAX, "No Message");
	}
}

//=============================================================================
/**
 * 最後に発生したエラー情報（FFW内部変数）を取得
 * @param なし
 * @retval FFW内部で保持しているFFWR8C_LAST_ERROR_INFO構造体のポインタ
 */
//=============================================================================
FFW_ERROR_DETAIL_INFO* GetErrorDetailInfo(void)
{
	return &s_ErrorDetailInfo;
}

//=============================================================================
/**
 * エラー詳細情報を設定する
 * @param  pErrDetailInfo エラー詳細情報構造体ポインタ
 * @param  byMessageNum   セットするエラーメッセージ個数
 * @retval なし
 */
//=============================================================================
void SetErrorDetailInfo(FFW_ERROR_DETAIL_INFO* pErrDetailInfo, BYTE byMessageNum) 
{
	BYTE i;
	// RevNo121017-003	Append Line
	errno_t		ernerr;

	// RevRxNo120910-007	Append Line
	//	(1)	引数pErrDetailInfo->dwErrorCodeで与えられたエラーコードを管理変数
	//		（s_ErrorDetailInfo．dwErrorCode）にセットする。
	s_ErrorDetailInfo.dwErrorCode = pErrDetailInfo->dwErrorCode;	// 引数で与えられたエラーコードを管理変数へコピー

	for (i = 0; i < byMessageNum; i++) {
		// RevNo121017-003	Modify Line
		ernerr = strcpy_s(s_ErrorDetailInfo.szErrorMessage[i], ERROR_MESSAGE_LENGTH_MAX, pErrDetailInfo->szErrorMessage[i]);
	}
}


// V.1.02 No.14,15 USB高速化対応 Append Line
//=============================================================================
/**
 * レベルEML情報取得
 * @param なし
 * @retval レベルEML情報( TRUE:レベルEML, FALSE:レベル0 )
 */
//=============================================================================
BOOL GetLevelEML(void)
{
	return s_bLevelEml;
}
//=============================================================================
/**
 * レベルEML情報設定
 * @param レベルEML情報( TRUE:レベルEML, FALSE:レベル0 )
 * @retval なし
 */
//=============================================================================
void SetLevelEML(BOOL bLvEml)
{
	s_bLevelEml = bLvEml;
	return;
}
// V.1.02 No.4 ユーザブートモード起動対応 Append End


