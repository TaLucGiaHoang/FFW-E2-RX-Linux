///////////////////////////////////////////////////////////////////////////////
/**
* @file asprh_setting.cpp
* @brief ASPRH FPGA setting
* @author RSD S.Nagai
* @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
* @author Renesas System Design Co., Ltd. All rights reserved.
* @date 2017/03/28
*/
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/11/29 新規作成 S.Nagai
*/

#include "rfw_bitops.h"
#include "asp_setting.h"
#include "asprh_setting.h"
extern "C" {
	int		exe_asp_run(USHORT);
	int		DcuGetReg(USHORT, ULONG *);
	ULONG	GetG3PeMode(void);
	BOOL	GetSftTrcEnable(void);
	BOOL	isSftTrcEnable(void);
	BOOL	IsReceiveBrkCmd(void);
	ULONG	get_lpdoptinfodata(int);
	BOOL	IsSwTrcNotRecord_PreInfo(void);
	ULONG	GetDeviceKindInfo(void);
}
#include "e2_asp_fpga.h"
#include "e2_asprh_fpga.h"
#include "doasprh_swtrc.h"
#include "doasp_exe.h"
#include "run.h"
#include "reg_dcu.h"
#include "doasp_sys_family.h"
#include "mtimeout.h"
#include "asynchro.h"
#include "ex_dev.h"

// ファイル内static変数の宣言

// ファイル内static関数の宣言
static void SetCmdboxCommon_woSTRC(ULONG ulHDAT, ULONG ulLDAT);
static void SetCmdboxLPD1pin(ULONG ulHDAT, ULONG ulLDAT);
static void SetCmdboxLPD4pin_woDeepStop(ULONG ulHDAT, ULONG ulLDAT);
static void SetCmdboxLPD4pin_wDeepStop(ULONG ulHDAT, ULONG ulLDAT);


//=============================================================================
/**
* RH固有のRUN開始処理
* @note
* @param なし
* @retval なし
*/
//=============================================================================
void AspRun_RH(void)
{
	WORD	wMode;
	int		nErr;

	// 拡張機能有効ビットをセット
	wMode = MODE_ASP;

	if ((!IsAspSwTrcEna() || !GetSftTrcEnable() || !isSftTrcEnable() || (IsSwTrcNotRecord_PreInfo() != FALSE))) {
		// ソフトトレースがOFFの場合
		// または全コアのソフトトレーススイッチがOFFの場合
		// コマンドボックスをON、自走トリガONでリードデータはSFIFOとする
		wMode |= MODE_CMDBOX_EN | MODE_CMDBOX_AUTO | MODE_CMDBOX_FIFO;
	} else {
		// ソフトトレースONの場合
		// 強制ブレークでもコマンドボックスを使うため、イネーブルにする
		wMode |= MODE_CMDBOX_EN | MODE_SWTRC;
	}

	// コマンドボックスのリセット解除
	FPGA_CLR_PAT(RH850_RST_CTRL, RH850RSTCTRL_RST_HCB);

	// CCUARTの有効化
	FPGA_WRITE(CC_UART_EN, CC_UART_EN_ENBIT);

	nErr = exe_asp_run(wMode);
	// エラーを返す口がないのでエラーは読み捨て
}


//=============================================================================
/**
* ソフトトレース有効時のHW設定(実行時)
* @param なし
* @retval なし
*/
//=============================================================================
void EnableSwTrcEv(void)
{
	RFW_SWTRC_DATA		SwTrcData;
	BYTE	byCnt;
	DWORD	dwTracEvlfact1Data = 0;

	for (byCnt = 0; byCnt < 4; byCnt++) {
		if (GetAspSwTrcEvEna(byCnt) == RF_ASPSWTRC_ON) {
			GetSwTrcData(byCnt, &SwTrcData);
			switch (SwTrcData.byTraceType) {
			case RF_ASPSWTRC_PC:
			default:
				dwTracEvlfact1Data |= (DWORD)(TRACE_EVLFACT1_PC << (byCnt * 8));
				break;
			case RF_ASPSWTRC_TAG:
				dwTracEvlfact1Data |= (DWORD)(TRACE_EVLFACT1_TAG << (byCnt * 8));
				break;
			case RF_ASPSWTRC_REG:
				dwTracEvlfact1Data |= (DWORD)(TRACE_EVLFACT1_REG << (byCnt * 8));
				dwTracEvlfact1Data |= (DWORD)(SwTrcData.byRegister << ((byCnt * 8) + 3));
				break;
			}
			FPGA_WRITE(TRACE_EVLP0 + (byCnt * 4), SwTrcData.dwData);
			FPGA_WRITE(TRACE_EVLPMSK0 + (byCnt * 4), SwTrcData.dwMask);
		}
	}

	FPGA_WRITE(TRACE_EVLFACT1, dwTracEvlfact1Data);

	for (byCnt = 0; byCnt < 4; byCnt++) {
		if (GetAspSwTrcEvEna(byCnt) == RF_ASPSWTRC_ON) {
			FPGA_SET_PAT(TRACE_CTRL, (TRACE_CTRL_TRACEEVLEN0 << byCnt));
		}
	}
}


//=============================================================================
/**
* ソフトトレース有効時のHW設定(実行時)
* @param なし
* @retval なし
*/
//=============================================================================
void SwTrcResetRelease(void)
{
	// モジュールリセット解除
	FPGA_CLR_PAT(RH850_RST_CTRL, RH850RSTCTRL_RST_ST);
}


//=============================================================================
/**
* ブレーク要求コマンドのHW設定(実行時)
* @param なし
* @retval なし
*/
//=============================================================================
void EnableBrkEv(void)
{
	ULONG	ulDbgCtrlpData;
	ULONG	ulHDAT;
	ULONG	ulLDAT;
	ULONG	ulDeviceInfo;

	ulDeviceInfo = GetDeviceKindInfo();

	// DBG_CTRLPの値をリードしてコマンドボックス設定値を決める
	DcuGetReg(G3_DBG_CTRLP, &ulDbgCtrlpData);
	ulLDAT = ulDbgCtrlpData & 0x0000FFFF;
	ulHDAT = ulDbgCtrlpData >> 16;

	if (IsAspSwTrcEna() && GetSftTrcEnable() && isSftTrcEnable() && (IsSwTrcNotRecord_PreInfo() == FALSE)) {
		// ソフトトレースON

		// 送信停止要求コマンドの設定
		if (get_lpdoptinfodata(0) == 3) {
			// LPD 4pinの場合
			if (((ulDeviceInfo & FL_SDISERIESALLID) == FL_SDIINFO_F1x_ALL)
			||  ((ulDeviceInfo & FL_SDISERIESALLID) == FL_SDIINFO_D1x_ALL)) {
				// DeepStopがある品種
				SetCmdboxLPD4pin_wDeepStop(ulHDAT, ulLDAT);
			} else {
				SetCmdboxLPD4pin_woDeepStop(ulHDAT, ulLDAT);
			}
		} else {
			// LPD 1pinの場合
			SetCmdboxLPD1pin(ulHDAT, ulLDAT);
		}

		// ソフトトレース停止後のステータス検出で使用するため設定する
		EnableDetectBrk_CMDBOX();

		// ブレーク要求コマンドを有効にする
		FPGA_MDF_PAT(COMBOX_TRG, COMBOX_TRG_HWTRG << COMBOX_TRG_BREAK_BIT, COMBOX_TRG_MSK << COMBOX_TRG_BREAK_BIT);
	} else {
		// ソフトトレースOFF
		SetCmdboxCommon_woSTRC(ulHDAT, ulLDAT);

		// ブレーク要求コマンドを有効にする
		FPGA_MDF_PAT(COMBOX_TRG, COMBOX_TRG_HWTRG << COMBOX_TRG_BREAK_BIT, COMBOX_TRG_MSK << COMBOX_TRG_BREAK_BIT);
	}

	// コマンドボックス自体の有効化は別途行う
}


//==============================================================================
/**
* ソフトウェアトレースOFFにおけるコマンドボックスのブレーク要求
* @note ソフトウェアトレースOFFの場合はLPD 1pin/4pinに関係なく1方式のみ
* @param ulHDAT  RUN前のDBG_CTRLPの上位16bitデータ
* @param ulLDAT  RUN前のDBG_CTRLPの下位16bitデータ
* @retval なし
*/
//==============================================================================
static void SetCmdboxCommon_woSTRC(ULONG ulHDAT, ULONG ulLDAT)
{
	DWORD dwCnt;

	// ブレーク要求コマンドの設定
	if (GetG3PeMode() == 2) {
		// シングルコアの場合
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x890C));	// 0x0600890C
	} else {
		// マルチコアの場合(ブロードキャストアクセスする)
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0xB90C));	// 0x0600B90C
	}
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT | ulHDAT));	// 0x080000B3
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT | ulLDAT | G3_DBG_CTRLP_FBK));	// 0x0A009010
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x0C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001 | COMBOX_CMD_END));	// 0x84000001

	// 残りのブレークコマンドエリアは0でフィルする
	for (dwCnt = 0x10; dwCnt < 0x50; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_BREAK + dwCnt, 0);
	}
}


//==============================================================================
/**
* LPD 1pinにおけるコマンドボックスのブレーク要求
* @note LPD 1pinはDeepStop有無に関係なく1方式のみ
* @param ulHDAT  RUN前のDBG_CTRLPの上位16bitデータ
* @param ulLDAT  RUN前のDBG_CTRLPの下位16bitデータ
* @retval なし
*/
//==============================================================================
static void SetCmdboxLPD1pin(ULONG ulHDAT, ULONG ulLDAT)
{
	DWORD dwCnt;

	// 当初の予定通り、送信停止要求から行う
	// ただし、タイミング次第で再実行後トレースが取れなくなる制限付き
	// LPD 1pinは回避方法なし
	// 送信停止要求コマンドの設定
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0002));	// 0x04000002
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_TRACEOFF | 0x0000));	// 0x02000000
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_TRACERESET | 0x0000));	// 0x18000000
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x0C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_RDATASW | 0x0000));		// 0x1A000000
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x10, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_AUTORUN | 0x0000));		// 0x1C000000

	// 残りの送信停止要求コマンドエリアは0でフィルする
	for (dwCnt = 0x14; dwCnt < 0x30; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_STPREQ + dwCnt, 0);
	}

	// ブレーク要求コマンドの設定
	if (GetG3PeMode() == 2) {
		// シングルコアの場合
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x890C));	// 0x0600890C
	} else {
		// マルチコアの場合(ブロードキャストアクセスする)
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0xB90C));	// 0x0600B90C
	}
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT | ulHDAT));	// 0x080000B3
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT | ulLDAT | G3_DBG_CTRLP_FBK));	// 0x0A009010
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x0C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001 | COMBOX_CMD_END));	// 0x84000001

	// 残りのブレークコマンドエリアは0でフィルする
	for (dwCnt = 0x10; dwCnt < 0x50; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_BREAK + dwCnt, 0);
	}
}


//==============================================================================
/**
* LPD 4pinにおけるコマンドボックスのブレーク要求
* ソフトウェアトレースON、DeepStopなしの場合
* @note 通常方式では送信停止要求とトレースデータが競合すると
* @note 次回実行時にソフトトレースが吐き出されないため、
* @note 処理順を入れ替えて回避する
* @param ulHDAT  RUN前のDBG_CTRLPの上位16bitデータ
* @param ulLDAT  RUN前のDBG_CTRLPの下位16bitデータ
* @retval なし
*/
//==============================================================================
static void SetCmdboxLPD4pin_woDeepStop(ULONG ulHDAT, ULONG ulLDAT)
{
	DWORD dwCnt;

	// 当初は送信停止要求で止めようとしたが、LPD 4pinで動作が不正になるため
	// LPD 4pinではいきなりブレークコマンドを送信することにした
	// さらに、送信停止要求がトレースデータにぶつからないように
	// ブレークの直前にソフトトレースをディセーブルにする
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_TRACEOFF | 0x0000));	// 0x02000000
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_TRACERESET | 0x0000));	// 0x1A000000

	// ブレーク要求コマンドの設定
	if (GetG3PeMode() == 2) {
		// シングルコアの場合
		FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x89A6));	// 0x060089A6
	} else {
		// マルチコアの場合(ブロードキャストアクセスする)
		FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0xB9A6));	// 0x0600B9A6
	}

	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x0C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT | 0x0014));	// 0x08000014
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x10, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT | 0x0032));	// 0x0A000032
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x14, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001));		// 0x04000001

	// 残りの送信停止要求コマンドエリアは0でフィルする
	for (dwCnt = 0x18; dwCnt < 0x30; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_STPREQ + dwCnt, 0);
	}

	// ブレーク要求コマンドの設定
	if (GetG3PeMode() == 2) {
		// シングルコアの場合
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x890C));	// 0x0600890C
	} else {
		// マルチコアの場合(ブロードキャストアクセスする)
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0xB90C));	// 0x0600B90C
	}
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT | ulHDAT));	// 0x080000B3
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT | ulLDAT | G3_DBG_CTRLP_FBK));	// 0x0A009010
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x0C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001));	// 0x04000001
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x10, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0002));	// 0x04000002
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x14, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_RDATASW | 0x0000));		// 0x1A000000
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x18, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_AUTORUN | 0x0000));		// 0x1C000000
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x1C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0000 | COMBOX_CMD_END));	// 0x84000000

	// 残りのブレークコマンドエリアは0でフィルする
	for (dwCnt = 0x20; dwCnt < 0x50; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_BREAK + dwCnt, 0);
	}
}


//==============================================================================
/**
* LPD 4pinにおけるコマンドボックスのブレーク要求
* ソフトウェアトレースON、DeepStopありの場合
* @note 通常方式ではDeepStop解除できないため、途中にウェイトを挟む
* @param ulHDAT  RUN前のDBG_CTRLPの上位16bitデータ
* @param ulLDAT  RUN前のDBG_CTRLPの下位16bitデータ
* @retval なし
*/
//==============================================================================
static void SetCmdboxLPD4pin_wDeepStop(ULONG ulHDAT, ULONG ulLDAT)
{
	DWORD dwCnt;

	// 当初は送信停止要求で止めようとしたが、LPD 4pinで動作が不正になるため
	// LPD 4pinではいきなりブレークコマンドを送信することにした
	// さらに、送信停止要求がトレースデータにぶつからないように
	// ウェイトを入れる。そのためコマンド領域2に後半のコマンドを配置
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_TRACEOFF | 0x0000));	// 0x02000000
	FPGA_WRITE(COMBOX_ADDR_STPREQ + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_TRACERESET | 0x0000));	// 0x1A000000

	// 残りの送信停止要求コマンドエリアは0でフィルする
	for (dwCnt = 0x08; dwCnt < 0x30; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_STPREQ + dwCnt, 0);
	}

	// ブレーク要求コマンドの設定
	if (GetG3PeMode() == 2) {
		// シングルコアの場合
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x890C));	// 0x0600890C
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x10, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x883C));	// 0x0600883C
	} else {
		// マルチコアの場合(ブロードキャストアクセスする)
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0xB90C));	// 0x0600B90C
		FPGA_WRITE(COMBOX_ADDR_BREAK + 0x10, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0xB83C));	// 0x0600B83C
	}
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT | ulHDAT));	// 0x080000B3
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT | ulLDAT | G3_DBG_CTRLP_FBK));	// 0x0A009010
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x0C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001));	// 0x04000001

	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x14, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT));	// 0x08000000
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x18, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT));	// 0x0A000000
	FPGA_WRITE(COMBOX_ADDR_BREAK + 0x1C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001 | COMBOX_CMD_END));	// 0x04000001																														
																														
	// 残りのブレークコマンドエリアは0でフィルする
	for (dwCnt = 0x20; dwCnt < 0x50; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_BREAK + dwCnt, 0);
	}

	// コマンドエリア2にウェイト後の送信停止要求処理以降を配置する
	FPGA_WRITE(COMBOX_ADDR_COMMAND2 + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0002));	// 0x04000002
	FPGA_WRITE(COMBOX_ADDR_COMMAND2 + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_RDATASW | 0x0000));		// 0x1A000000
	FPGA_WRITE(COMBOX_ADDR_COMMAND2 + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_AUTORUN | 0x0000));		// 0x1C000000
	FPGA_WRITE(COMBOX_ADDR_COMMAND2 + 0x10, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0000 | COMBOX_CMD_END));	// 0x84000000

	// 残りの格納コマンド エリア2は0でフィルする
	for (dwCnt = 0x14; dwCnt < 0x40; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_COMMAND2 + dwCnt, 0);
	}

	// コマンドエリア2を有効にする
	FPGA_MDF_PAT(COMBOX_TRG, COMBOX_TRG_SWTRG << COMBOX_TRG_COMMAND2_BIT, COMBOX_TRG_MSK << COMBOX_TRG_COMMAND2_BIT);
}


//==============================================================================
/**
* E2拡張機能使用時の強制ブレークはコマンドボックスから投げる
* @note 3秒待ってブレークしなかったらタイムアウト
* @param なし
* @retval TRUE  ブレークした
* @retval FALSE ブレークしていない
*/
//==============================================================================
BOOL ReleaseBrkCmd(void)
{
	UINT64	u64Timeout;

	// ブレークコマンドをソフトトリガで送信
	FPGA_SET_PAT(COMBOX_TRG, COMBOX_TRG_CMDGETBREAK);

	u64Timeout = GetTickCount64();
	for (;;) {
		// ブレークするまで待ち
		if (IsReceiveBrkCmd()) {
			return TRUE;
		}
		if ((GetTickCount64() - u64Timeout) >= 3000) {
			// 3秒でタイムアウト
			return FALSE;
		}
	}
}

//=============================================================================
/**
* コマンドボックスを使ったブレーク検出のHW設定(実行時)
* @note  格納コマンドエリア1領域に配置し、ステータス取得を繰り返す
* @note  現状PCモニタ、RAMモニタはサポートしていないので、他のコマンドは配置しない
* @note  配置が必要になった場合、順番やサンプリング間隔など調整が必要
* @param なし
* @retval なし
*/
//=============================================================================
void EnableDetectBrk_CMDBOX(void)
{
	DWORD dwCnt;

	// ステータス取得コマンドの設定
	FPGA_WRITE(COMBOX_ADDR_COMMAND1 + 0x00, (DWORD)(COMBOX_CMD_READ | COMBOX_CMD_REG_CBUFCMD | COMBOX_CMD_TYPE_STATUS | 0xC90D));					// 0x420FC90D
	FPGA_WRITE(COMBOX_ADDR_COMMAND1 + 0x04, (DWORD)(COMBOX_CMD_READ | COMBOX_CMD_REG_FSMCTRL | COMBOX_CMD_TYPE_STATUS | 0x0001 | COMBOX_CMD_END));	// 0xC40F0001

	// 残りの格納コマンド エリア1は0でフィルする
	for (dwCnt = 0x8; dwCnt < 0x80; dwCnt += 0x4) {
		FPGA_WRITE(COMBOX_ADDR_COMMAND1 + dwCnt, 0);
	}

	// 格納コマンドエリア1を自走トリガにする
	FPGA_MDF_PAT(COMBOX_TRG, COMBOX_TRG_AUTORUN << COMBOX_TRG_COMMAND1_BIT, COMBOX_TRG_MSK << COMBOX_TRG_COMMAND1_BIT);
	// 無限リピート、間隔は空けずにコマンドを投げる
	FPGA_WRITE(COM_REP, 0x00000000);
	FPGA_WRITE(COM_INTERVAL, 0x00000001);

	// ブレーク判断のステータスを設定する
	FPGA_WRITE(COM_STSCHK_CMPV, 0x00000002);
	FPGA_WRITE(COM_STSCHK_CMPMSK, 0xFFFFFFFD);

	// コマンドボックス自体の有効化は別途行う
}


//=============================================================================
/**
* 強制ブレークの時にコマンドボックスを止める設定
* 強制ブレークの前にコマンドボックスを無効化する必要あり
* @param なし
* @retval なし
*/
//=============================================================================
void StopCMDBOX(void)
{
	DWORD	dwComboxCtrl;
	UINT64	t;

	// コマンドボックスを停止
	FPGA_CLR_PAT(COMBOX_CTRL, COMBOX_CTRL_CMDBOXEN);

	// 送信中のコマンドボックスが完了するまで待つ
	w_timer(TIMER_COUNT_INIT, &t);
	for(;;) {
		dwComboxCtrl = FPGA_READ(COMBOX_CTRL);
		if ((dwComboxCtrl & COMBOX_CTRL_CMDBOXEN) == 0) {
			break;
		}
		if (w_timer(GetTimerCount_1sec(), &t)) {
			// タイムアウト1秒
			break;
		}
	}

	// リードデータの格納先をRBUFに戻す
	FPGA_CLR_PAT(COMBOX_CTRL, COMBOX_CTRL_CMDRDATASW);

	return;
}


//=============================================================================
/**
* トレース停止コマンドのHW設定(実行時)
* @param なし
* @retval なし
*/
//=============================================================================
void EnableStopTrcEv(void)
{
	// トレース停止コマンドの設定
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x00, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x8D82));		// 0x06008D82
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x04, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT | 0x7000));	// 0x08007000
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x08, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT | 0x0000));	// 0x0A000000
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x0C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001));		// 0x04000001
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x10, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x8A82));		// 0x06008A82
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x14, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT | 0x7000));	// 0x08007000
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x18, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT | 0x0000));	// 0x0A000000
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x1C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001));		// 0x04000001
	if (GetG3PeMode() == 2) {
		// シングルコアの場合
		FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x20, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0x8882));	// 0x06008882
	}
	else {
		// マルチコアの場合(ブロードキャストアクセスする)
		FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x20, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_CBUFCMD | 0xB882));	// 0x0600B882
	}
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x24, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFHDAT | 0x7000));	// 0x08007000
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x28, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_TBUFLDAT | 0x0000));	// 0x0A000000
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x2C, (DWORD)(COMBOX_CMD_WRITE | COMBOX_CMD_REG_FSMCTRL | 0x0001));		// 0x04000001
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x30, (DWORD)(COMBOX_CMD_READ | COMBOX_CMD_REG_CBUFCMD | 0xC891));		// 0x4600C891
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x34, (DWORD)(COMBOX_CMD_READ | COMBOX_CMD_REG_FSMCTRL | 0x0001 | COMBOX_CMD_END));		// 0x84000001

	// 残りは0でフィルする
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x38, 0);
	FPGA_WRITE(COMBOX_ADDR_TRCSTP + 0x3C, 0);

	// トレース停止コマンドを有効にする
	FPGA_MDF_PAT(COMBOX_TRG, COMBOX_TRG_HWTRG << COMBOX_TRG_TRCSTP_BIT, COMBOX_TRG_MSK << COMBOX_TRG_TRCSTP_BIT);

	// コマンドボックス自体の有効化は別途行う
}


//=============================================================================
/**
* ファミリ固有機能のモジュールリセットとレジスタリセットを行う
* @param なし
* @retval なし
*/
//=============================================================================
void Reset_Asp_family(void)
{
	FPGA_SET_PAT(RH850_RST_CTRL, RH850RSTCTRL_RST_HCB | RH850RSTCTRL_RST_ST);
	FPGA_SET_PAT(RH850_RST_CTRL, RH850RSTCTRL_RST_HCBREG | RH850RSTCTRL_RST_STREG);
	FPGA_CLR_PAT(RH850_RST_CTRL, RH850RSTCTRL_RST_HCBREG | RH850RSTCTRL_RST_STREG);
}



//=============================================================================
/**
* コマンドボックスのブレークコマンドが発行されたかどうかチェックする関数
* @param なし
* @retval TRUE  ：コマンドが発行された
* @retval FALSE ：コマンドが発行されていない
*/
//=============================================================================
BOOL IsReleaseBrkCmd(void)
{
	DWORD	dwComboxCtrl;
	dwComboxCtrl = FPGA_READ(COMBOX_CTRL);
	if ((dwComboxCtrl & COMBOX_CTRL_CMDBREAKFLG) != 0) {
		return TRUE;
	}
	return FALSE;
}

