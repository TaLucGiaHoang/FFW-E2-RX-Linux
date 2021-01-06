///////////////////////////////////////////////////////////////////////////////
/**
 * @file emudef.cpp
 * @brief エミュレータ依存のサブルーチンファイル
 * @author RSD S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2014(2017) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/02/01
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxE2LNo141104-001 2014/11/07 上田
	新規作成
・RevRxE2No170201-001 2017/02/01 PA 辻
	E2エミュレータ対応
・RevRxNo170515-001 2017/05/15 PA 紡車
　　E2 OB-mini対応
  ・RevRxNo170517-001 2017/05/17 南
  　　E2 OB-mini対応
	*/
// RevRxNo170517-001 Append Start
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#elif __linux__
#endif
// RevRxNo170517-001 Append End

#include "emudef.h"
#include "cpuspec.h"
#include "prot_sys.h"
#include "do_closed.h"	// RevRxNo170515-001 Append Line


// ファイル内static変数の宣言
// MCU固有情報管理構造体定義(初期化)
static FFW_EMUDEF_DATA	s_EmuDefData = {
	BFWAREA_START_E20,		// BFWコード領域開始アドレス
	BFWAREA_SIZE_E20,		// BFWコード領域サイズ
	BFWLV0_START_E20,		// BFWレベル0領域開始アドレス
	BFWLV0_SIZE_E20,		// BFWレベル0領域サイズ
	BFWEML_START_E20,		// BFW EML領域開始アドレス
	BFWEML_SIZE_E20, 		// BFW EML領域サイズ
	BFWPRG_START_E20,		// BFW FDT/PRG領域開始アドレス
	BFWPRG_SIZE_E20,		// BFW FDT/PRG領域サイズ
	0x00000000,				// BFW COM領域開始アドレス
	0x00000000,				// BFW COM領域サイズ
	SELIDAREA_START_E20,	// 製品シリアル番号設定領域開始アドレス
	SELIDAREA_SIZE_E20,		// 製品シリアル番号設定領域サイズ
	REWRITE_EMUCODE_E20,	// BFW書き換え時のエミュレータ識別コード
	"monpprog.s"			// BFW書き換え制御プログラムのファイル名
};

static bool existMonpFile(char* szE2OBminiMonpName);	// RevRxNo170517-001 Append Line

//=============================================================================
/**
 * エミュレータ種別に対応したエミュレータ固有情報を設定する。
 * @param なし
 * @retval なし
 */
//=============================================================================
FFWERR SetEmuDefData(void)		// RevRxNo170515-001 Modify Line
{
	FFWERR	ferr = FFWERR_OK;	// RevRxNo170515-001 Append Line
	FFWE20_EINF_DATA	einfData;
	errno_t	ernerr;
	BYTE	byData;				// RevRxNo170515-001 Append Line
	char szMonpName[MAX_PATH];	// RevRxNo170517-001 Append Line

	getEinfData(&einfData);

	switch (einfData.wEmuStatus) {
	case EML_E20:	// no break
	case EML_E1:
		s_EmuDefData.dwBfwAreaStartAdr 	= BFWAREA_START_E20;	// BFWコード領域開始アドレス
		s_EmuDefData.dwBfwAreaSize 		= BFWAREA_SIZE_E20;		// BFWコード領域サイズ
		s_EmuDefData.dwBfwLv0StartAdr 	= BFWLV0_START_E20;		// BFWレベル0領域開始アドレス
		s_EmuDefData.dwBfwLv0Size 		= BFWLV0_SIZE_E20;		// BFWレベル0領域サイズ
		s_EmuDefData.dwBfwEmlStartAdr 	= BFWEML_START_E20;		// BFW EML領域開始アドレス
		s_EmuDefData.dwBfwEmlSize 		= BFWEML_SIZE_E20; 		// BFW EML領域サイズ
		s_EmuDefData.dwBfwPrgStartAdr 	= BFWPRG_START_E20;		// BFW FDT/PRG領域開始アドレス
		s_EmuDefData.dwBfwPrgSize 		= BFWPRG_SIZE_E20;		// BFW FDT/PRG領域サイズ
		s_EmuDefData.dwBfwComStartAdr 	= 0x00000000;			// BFW COM領域開始アドレス
		s_EmuDefData.dwBfwComSize 		= 0x00000000;			// BFW COM領域サイズ
		s_EmuDefData.dwSelIdStartAdr 	= SELIDAREA_START_E20;	// 製品シリアル番号設定領域開始アドレス
		s_EmuDefData.dwSelIdSize  		= SELIDAREA_SIZE_E20;	// 製品シリアル番号設定領域サイズ

		if (einfData.wEmuStatus == EML_E20) {
			s_EmuDefData.dwBfwRewriteEmuCode = REWRITE_EMUCODE_E20;	// BFW書き換え時のエミュレータ識別コード
		} else {
			s_EmuDefData.dwBfwRewriteEmuCode = REWRITE_EMUCODE_E1;	// BFW書き換え時のエミュレータ識別コード
		}
		ernerr = strcpy_s(s_EmuDefData.szMonpprogFileName, MONPPROG_FILENAME_NUM_MAX, "monpprog.s");	// BFW書き換え制御プログラムのファイル名
		break;

	case EML_E2LITE:
		s_EmuDefData.dwBfwAreaStartAdr 	= BFWAREA_START_E2L;		// BFWコード領域開始アドレス
		s_EmuDefData.dwBfwAreaSize 		= BFWAREA_SIZE_E2L;			// BFWコード領域サイズ
		s_EmuDefData.dwBfwLv0StartAdr 	= BFWLV0_START_E2L;			// BFWレベル0領域開始アドレス
		s_EmuDefData.dwBfwLv0Size 		= BFWLV0_SIZE_E2L;			// BFWレベル0領域サイズ
		s_EmuDefData.dwBfwEmlStartAdr 	= BFWEML_START_E2L;			// BFW EML領域開始アドレス
		s_EmuDefData.dwBfwEmlSize 		= BFWEML_SIZE_E2L; 			// BFW EML領域サイズ
		s_EmuDefData.dwBfwPrgStartAdr 	= BFWPRG_START_E2L;			// BFW FDT/PRG領域開始アドレス
		s_EmuDefData.dwBfwPrgSize 		= BFWPRG_SIZE_E2L;			// BFW FDT/PRG領域サイズ
		s_EmuDefData.dwBfwComStartAdr 	= BFWCOM_START_E2L;			// BFW COM領域開始アドレス
		s_EmuDefData.dwBfwComSize 		= BFWCOM_SIZE_E2L;			// BFW COM領域サイズ
		s_EmuDefData.dwSelIdStartAdr 	= SELIDAREA_START_E2L;		// 製品シリアル番号設定領域開始アドレス
		s_EmuDefData.dwSelIdSize  		= SELIDAREA_SIZE_E2L;		// 製品シリアル番号設定領域サイズ
		s_EmuDefData.dwBfwRewriteEmuCode = REWRITE_EMUCODE_E2L;		// BFW書き換え時のエミュレータ識別コード
		ernerr = strcpy_s(s_EmuDefData.szMonpprogFileName, MONPPROG_FILENAME_NUM_MAX, "monpprog_e2lite.s");	// BFW書き換え制御プログラムのファイル名
		// RevRxNo170515-001 Append Start
		// E2 OB-miniの場合にMONPPROGファイル名を差し替え
		ferr = DO_CPUR(E2_OB_MINI_MAGIC_ADDR, EBYTE_ACCESS, 1, &byData);	//参照アドレスはEMLKINDセクションの次アドレス
		if (ferr != FFWERR_OK) {
			return ferr;
		}
		if (byData == E2_OB_MINI_MAGIC_CODE) {							//E2 OB-miniのマジックコードであればE2 OB-miniと判断できる
			// RevRxNo170517-001 Modify Start
			//Device_CustomフォルダにMONPPROGが存在するか確認
#ifdef __linux__
			ernerr = strcpy_s(szMonpName, _countof(szMonpName), "../../../../../Device_Custom/RX/MCU/E2L/monpprog_e2obm.s");
#else // _WIN32
			ernerr = strcpy_s(szMonpName, _countof(szMonpName), "..\\..\\..\\..\\..\\Device_Custom\\RX\\MCU\\E2L\\monpprog_e2obm.s");
#endif
			if (existMonpFile(szMonpName) == FALSE) {
				//Device_Customフォルダに存在しないので、Deviceフォルダを確認
				ernerr = strcpy_s(szMonpName, _countof(szMonpName), "..\\..\\..\\..\\..\\Device\\RX\\MCU\\E2L\\monpprog_e2obm.s");
				if (existMonpFile(szMonpName) == FALSE) {
					//Deviceフォルダにも存在しないので、相対パスなしのMONPPROGファイル名を登録(FFWと同じフォルダと扱われる)
					ernerr = strcpy_s(szMonpName, _countof(szMonpName), "monpprog_e2obm.s");
				}
			}
			ernerr = strcpy_s(s_EmuDefData.szMonpprogFileName, _countof(s_EmuDefData.szMonpprogFileName), szMonpName);
			// RevRxNo170517-001 Modify End
		}
		// RevRxNo170515-001 Append End

		break;

	// RevRxE2No170201-001 Append Start
	case EML_E2:
		s_EmuDefData.dwBfwAreaStartAdr = BFWAREA_START_E2;			// BFWコード領域開始アドレス
		s_EmuDefData.dwBfwAreaSize = BFWAREA_SIZE_E2;				// BFWコード領域サイズ
		s_EmuDefData.dwBfwLv0StartAdr = BFWLV0_START_E2;			// BFWレベル0領域開始アドレス
		s_EmuDefData.dwBfwLv0Size = BFWLV0_SIZE_E2;					// BFWレベル0領域サイズ
		s_EmuDefData.dwBfwEmlStartAdr = BFWEML_START_E2;			// BFW EML領域開始アドレス
		s_EmuDefData.dwBfwEmlSize = BFWEML_SIZE_E2; 				// BFW EML領域サイズ
		s_EmuDefData.dwBfwPrgStartAdr = BFWPRG_START_E2;			// BFW FDT/PRG領域開始アドレス
		s_EmuDefData.dwBfwPrgSize = BFWPRG_SIZE_E2;					// BFW FDT/PRG領域サイズ
		s_EmuDefData.dwBfwComStartAdr = BFWCOM_START_E2;			// BFW COM領域開始アドレス
		s_EmuDefData.dwBfwComSize = BFWCOM_SIZE_E2;					// BFW COM領域サイズ
		s_EmuDefData.dwSelIdStartAdr = SELIDAREA_START_E2;			// 製品シリアル番号設定領域開始アドレス
		s_EmuDefData.dwSelIdSize = SELIDAREA_SIZE_E2;				// 製品シリアル番号設定領域サイズ
		s_EmuDefData.dwBfwRewriteEmuCode = REWRITE_EMUCODE_E2;		// BFW書き換え時のエミュレータ識別コード
		ernerr = strcpy_s(s_EmuDefData.szMonpprogFileName, MONPPROG_FILENAME_NUM_MAX, "monpprog_e2.s");	// BFW書き換え制御プログラムのファイル名
		break;
	// RevRxE2No170201-001 Append End

	case EML_EZCUBE:
		s_EmuDefData.dwBfwAreaStartAdr 	= 0x00000000;	// BFWコード領域開始アドレス(未使用)
		s_EmuDefData.dwBfwAreaSize 		= 0x00000000;	// BFWコード領域サイズ(未使用)
		s_EmuDefData.dwBfwLv0StartAdr 	= 0x00000000;	// BFWレベル0領域開始アドレス(未使用)
		s_EmuDefData.dwBfwLv0Size 		= 0x00000000;	// BFWレベル0領域サイズ(未使用)
		s_EmuDefData.dwBfwEmlStartAdr 	= 0x00000000;	// BFW EML領域開始アドレス(未使用)
		s_EmuDefData.dwBfwEmlSize 		= 0x00000000; 	// BFW EML領域サイズ(未使用)
		s_EmuDefData.dwBfwPrgStartAdr 	= 0x00000000;	// BFW FDT/PRG領域開始アドレス(未使用)
		s_EmuDefData.dwBfwPrgSize 		= 0x00000000;	// BFW FDT/PRG領域サイズ(未使用)
		s_EmuDefData.dwBfwComStartAdr 	= 0x00000000;	// BFW COM領域開始アドレス(未使用)
		s_EmuDefData.dwBfwComSize 		= 0x00000000;	// BFW COM領域サイズ(未使用)
		s_EmuDefData.dwSelIdStartAdr 	= 0x00000000;	// 製品シリアル番号設定領域開始アドレス(未使用)
		s_EmuDefData.dwSelIdSize  		= 0x00000000;	// 製品シリアル番号設定領域サイズ(未使用)
		s_EmuDefData.dwBfwRewriteEmuCode = 0x00000000;	// BFW書き換え時のエミュレータ識別コード(未使用)
		ernerr = strcpy_s(s_EmuDefData.szMonpprogFileName, MONPPROG_FILENAME_NUM_MAX, "");	// BFW書き換え制御プログラムのファイル名
		break;

	default:
		break;	// ここは通らない
	}

	return ferr;	// RevRxNo170515-001 Modify Line
}


//=============================================================================
/**
 * FFW内で定義しているエミュレータ固有情報を構造体のポインタとして返送する。
 * @param なし
 * @retval エミュレータ固有情報管理構造体へのポインタ
 */
//=============================================================================
FFW_EMUDEF_DATA* GetEmuDefData(void)
{
	return	&s_EmuDefData;
}

// RevRxNo170517-001 Append Start
//=============================================================================
/**
* MONPファイルが存在するかチェックする。(E2 OB-mini専用)
* @param MONPFILE名
* @retval TRUE: ファイルが存在する
*         FALSE:ファイルが存在しない
*/
//=============================================================================
static bool existMonpFile(char* szMonpName)
{
	char filename[MAX_PATH + 1];
	HINSTANCE hFfwDll;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	errno_t	ernerr;
	int		ierr;

	// ffw.dllのフルパスを得る。
	hFfwDll = GetHandle();
	GetModuleFileName(hFfwDll, filename, sizeof(filename));

	// ffw.dllのフルパスからファイル名を削除
	ernerr = _splitpath_s(filename, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);

	// パス名生成(ファイル名の前まで)
	ierr = sprintf_s(filename, _countof(filename), "%s%s", drive, dir);

	// パス名生成(ファイル名付加)
	ernerr = strcat_s(filename, _countof(filename), szMonpName);

	if (PathFileExists(filename)) {
		//ファイルが存在する
		return TRUE;
	}
	return FALSE;

}
// RevRxNo170517-001 Append End

