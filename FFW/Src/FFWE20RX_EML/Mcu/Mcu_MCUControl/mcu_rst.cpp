////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcu_rst.cpp
 * @brief リセット関連MCUへの処理のソースファイル
 * @author RSO H.Hashiguchi, K.Uemori, S.Ueda, SDS T.Iwata, Y.Miyake, PA K.Tsumu
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2016/10/28
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121026-001 2012/11/07 植盛
  RX100量産対応
・RevRxNo121022-001	2012/11/21 SDS 岩田
	EZ-CUBE PROT_MCU_DUMP()分割処理対応
・RevRxNo130411-001	2013/04/12 上田
	F/W内のMCU個別制御管理方法変更
・RevRxNo130308-001	2013/08/05 三宅
　カバレッジ開発対応
・RevRxNo130301-001	2013/09/05 上田
	RX64M対応
・RevRxNo130730-005 2013/11/11 上田
	内蔵ROM有効/無効判定処理改善
・RevRxNo161003-001 2016/10/28 PA 紡車
　　RX651-2MB 起動時のバンクモード、起動バンク対応
*/
#include "mcu_rst.h"

#include "ffwmcu_mcu.h"
#include "domcu_prog.h"
#include "protmcu_mcu.h"
#include "protmcu_rst.h"
#include "domcu_mem.h"
#include "protmcu_mem.h"
#include "mcuspec.h"
#include "domcu_mcu.h"
#include "mcudef.h"	// RevRxNo130411-001 Append Line
#include "do_sys.h"			// RevRxNo130308-001-033 Append Line
#include "hwrx_fpga_cv.h"	// RevRxNo130308-001-033 Append Line


//==============================================================================
/**
 * ターゲットMCU をH/W リセットする。
 * @param なし
 * @retval FFWERR_OK 正常終了
 */
//==============================================================================
FFWERR McuRest(void)
{

	FFWERR						ferr;

	// V.1.02 No.4 ユーザブート/USBブート対応 Append Start
	// RX630/RX210のユーザブート起動の場合はリセットコマンドを発行せず、HPON→IDCODE処理となる
	FFWMCU_MCUINFO_DATA_RX*		pMcuInfo;
	enum FFWRXENM_PMODE	ePmode;
	FFWMCU_FWCTRL_DATA*	pFwCtrl;	// RevRxNo130411-001 Append Line
	FFWE20_EINF_DATA	einfData;	// RevRxNo130308-001-033 Append Line

	pMcuInfo = GetMcuInfoDataRX();
	ePmode = GetPmodeDataRX();
	pFwCtrl = GetFwCtrlData();	// RevRxNo130411-001 Append Line


	// RevRxNo130411-001 Modify Line
	if ((ePmode == RX_PMODE_USRBOOT) && (pFwCtrl->bAuthClrUsrBootReset == TRUE)) {
			// ユーザブート起動で、ユーザブートモードでのリセット時にデバッガ認証が切れるMCUの場合
		ferr = PROT_RXHPON(0x00, ePmode);	// コールドスタート&ユーザブートモードでHPON(接続)
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// IDCODE再設定
		BYTE byIDBuff[16];
		BYTE byResult = 0;
		GetIdCode(byIDBuff);	 // pbyID[]に保存しておいたIDコードを格納する
		ferr = PROT_RXIDCODE(byIDBuff, &byResult);
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevNo110422-001 Delete
		// リセット後のレジスタ初期化処理はすべて共通なので、削除

	} else {
	// V.1.02 No.4 ユーザブート/USBブート対応 Append End
		ferr = PROT_MCU_REST();			// コマンド発行（リセット）
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		// RevNo110422-001 Delete
		// リセット後のレジスタ初期化処理はすべて共通なので、削除
	}

	// RevNo110427-001 Append Start
	//RX630/RX210のFINE通信内蔵ROM無効モードのリセットは、認証切れを起こしているかもしれない。
	//認証切れを起こした場合は、ブートが動いて認証を行うのでエンディアンがブート(ユーザブート)のエンディアンで動く
	//このため再度リセットを入れて、エンディアンを認識させる必要がある。
	//ただし、USRBOOT時は不要
	enum FFWRXENM_RMODE eRmode;
	eRmode = GetRmodeDataRX();
	// RevRxNo161003-001 Modify Line
	if((((eRmode == RX_RMODE_ROMD) || (eRmode == RX_RMODE_ROMD_DUAL_BANK0) || (eRmode == RX_RMODE_ROMD_DUAL_BANK1) || (eRmode == RX_RMODE_ROMD_DUAL)) || 
		(GetPmodeInRomDisFlg())) && (pMcuInfo->byInterfaceType == IF_TYPE_1)) {
		// RevRxNo121026-001, RevRxNo130411-001 Modify Line
		if (ePmode != RX_PMODE_USRBOOT) {
			ferr = PROT_MCU_REST();			// コマンド発行（リセット）
			if (ferr != FFWERR_OK) {
				return ferr;
			}
		}
	}

	// MCUをリセット後、SetPmodeInRomReg2Flg()発行前に
	// 内蔵ROM無効拡張モード状態参照レジスタリードフラグをTRUE(リード必要)に設定
	// RevRxNo130730-005 Append Line
	SetReadPmodeInRomDisRegFlg(TRUE);

	// MCUリセットにより内蔵ROM有効/無効状態が変化する可能性があるため、
	// SetPmodeInRomReg2Flg()を発行する。
	ferr = SetPmodeInRomReg2Flg();	// 内蔵ROM無効拡張モードフラグの設定
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	// RevRxNo130308-001-033 Append Start
	getEinfData(&einfData);						// エミュレータ情報取得
	if (einfData.wEmuStatus == EML_E20) {		// E20の場合のみ
		// TRCTL0.RSTFを"0"クリアする。上でマイコンがリセットされ、RSTFが"1"になるため。
		ferr = ClrFpgaTrctl0_Rstf();	// RevRxNo130308-001-038 modify Line
		if (ferr != FFWERR_OK) {
			return ferr;
		}
	}
	// RevRxNo130308-001-033 Append End

	return ferr;

}

// RevRxNo130301-001 Append Start
//==============================================================================
/**
 * OCDのデバッグモジュールリセット
 * @param なし
 * @retval FFWエラーコード
 */
//==============================================================================
FFWERR OcdDbgModuleReset(void)
{
	FFWERR	ferr = FFWERR_OK;

	ferr = PROT_MCU_DBGREST();	// DBGRESTコマンド発行
	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}
// RevRxNo130301-001 Append End

//=============================================================================
/**
 * リセットベクタ取得
 * @param byRestVect RESTVECT_MEM_DATA:メモリの値を取得 RESTVECT_CACHE_DATA:メモリの値を取得
 * @param pdwRestVect リセットベクタ値格納ポインタ
 * @retval なし
 */
//=============================================================================
FFWERR GetMcuRestAddr(BYTE byRestVect,DWORD* pdwRestVect)
{

	FFWERR						ferr = FFWERR_OK;
	FFWMCU_MCUAREA_DATA_RX*		pMcuArea;			// ROMレス品判断のためMCU領域情報が必要
	enum FFWENM_MACCESS_SIZE	eAccessSize;		// 設定する領域へのアクセスサイズ
	MADDR						madrStartAddr,madrEndAddr;
	BYTE*						pbyReadData;		// 参照データを格納する領域のアドレス
	DWORD						dwReadData[1];		// 参照データ格納領域
	DWORD						dwAccessCount;
	DWORD						dwAreaNum;
	BOOL						bSameAccessSize;
	BOOL						bSameAccessCount;
	BYTE						byEndian;
	enum FFWRXENM_PMODE	ePmode;
	ePmode = GetPmodeDataRX();
	pMcuArea = GetMcuAreaDataRX();
	
	if(ePmode == RX_PMODE_USRBOOT){
		madrStartAddr = pMcuArea->dwmadrUserBootEnd-0x03;		// ユーザブートリセットベクタ
	} else {
		madrStartAddr = MCU_SNG_MODE_REST_VECT_ADDR;		// ユーザブートリセットベクタ
	}
	madrEndAddr = madrStartAddr + 3;
	pbyReadData = reinterpret_cast<BYTE*>(dwReadData);
	dwAccessCount = 1;
	dwAreaNum = 1;
	bSameAccessSize = FALSE;
	bSameAccessCount = FALSE;
	eAccessSize = MLWORD_ACCESS;
	memset(pbyReadData, 0, sizeof(WORD));
	//リセットベクタはリトルで取得
	byEndian = FFWRX_ENDIAN_LITTLE;
	if(byRestVect == RESTVECT_MEM_DATA){
// RevRxNo121022-001 Append Start
		// ヘッダを含めて0x800hバイトを超える、PROT_MCU_DUMP()関数が呼ばれることはないので、BFWMCUCmd_DUMPコマンドのリードバイト数最大値で
		// アクセスバイト数を分割してPROT_MCU_DUMP()関数を呼び出す必要はない。
// RevRxNo121022-001 Append End
		ferr = PROT_MCU_DUMP(dwAreaNum, &madrStartAddr, bSameAccessSize, &eAccessSize, bSameAccessCount, &dwAccessCount, pbyReadData, byEndian);
	} else if(byRestVect == RESTVECT_CACHE_DATA){
		ferr = DO_CDUMP(madrStartAddr, madrEndAddr, pbyReadData);
	}

	*pdwRestVect = dwReadData[0];

	return ferr;
}


//=============================================================================
/**
 * リセット関連コマンド用FFW内部変数の初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitMcuData_Rst(void)
{

	return;

}
