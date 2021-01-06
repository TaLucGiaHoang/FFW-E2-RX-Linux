///////////////////////////////////////////////////////////////////////////////
/**
 * @file hw_fpga.cpp
 * @brief HW FPGA共通部アクセス実装ファイル
 * @author RSD H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/18
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/11/18 上田
	E2 Lite対応
*/
#include "hw_fpga.h"
#include "do_common.h"
#include "prot_cpu.h"
// ファイル内static変数の宣言

///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////

//==============================================================================
/**
 * FPGAへの書き込み
 * @param eadrWriteAddr 書き込み対象アドレス
 * @param wData         書き込みデータ
 * @retval FFWERR_OK    正常終了
 * @retval FFWERR_COM   通信異常発生
 */
//==============================================================================
FFWERR WriteFpgaReg(EADDR eadrWriteAddr, WORD wData)
{
	FFWERR						ferr;
	enum FFWENM_EACCESS_SIZE	eCpuAccessSize;			// 設定する領域へのアクセスサイズ
	BYTE*						pbyWriteFpga;			// 設定データを格納する領域のアドレス(FPGA用)
	WORD						wWriteData[1];			// 設定データ格納領域(FPGA用)
	WORD						wDataTmp;
	DWORD						dwLength;

	// レジスタ設定
	eCpuAccessSize = EWORD_ACCESS;
	pbyWriteFpga = reinterpret_cast<BYTE*>(wWriteData);
	dwLength = 1;

	wDataTmp = ConvEndian_W(wData);
	
	memset(pbyWriteFpga, 0, sizeof(WORD));
	memcpy(pbyWriteFpga, &wDataTmp, sizeof(WORD));

	// RevRxE2LNo141104-001 Modify Line
	// エミュレータ種別、BFW動作モードに対応したCPU_FILLコマンドを発行(エミュレータ種別はFFW内部変数を参照)
	ferr = ProtCpuFill(eCpuAccessSize, eadrWriteAddr, dwLength, pbyWriteFpga);

	if (ferr != FFWERR_OK) {
		return ferr;
	}

	return ferr;
}

//==============================================================================
/**
 * FPGAからの読み込み
 * @param eadrReadAddr 読み込み対象アドレス
 * @param pwData       読み込みデータ
 * @retval FFWERR_OK   正常終了
 * @retval FFWERR_COM  通信異常発生
 */
//==============================================================================
FFWERR ReadFpgaReg(EADDR eadrReadAddr, WORD* pwData)
{
	FFWERR						ferr;
	enum FFWENM_EACCESS_SIZE	eCpuAccessSize;			// 設定する領域へのアクセスサイズ
	BYTE*						pbyReadFpga;			// 参照データを格納する領域のアドレス(FPGA用)
	WORD						wReadData[1];			// 参照データ格納領域(FPGA用)
	DWORD						dwAreaNum;
	DWORD						dwLength;
	WORD						wDataTmp;

	// レジスタ設定
	eCpuAccessSize = EWORD_ACCESS;
	pbyReadFpga = reinterpret_cast<BYTE*>(wReadData);
	dwAreaNum = 1;
	dwLength = 1;

	memset(pbyReadFpga, 0, sizeof(WORD));

	// RevRxE2LNo141104-001 Modify Line
	// エミュレータ種別、BFW動作モードに対応したCPU_Rコマンドを発行(エミュレータ種別はFFW内部変数を参照)
	ferr = ProtCpuRead(eCpuAccessSize, dwAreaNum, &eadrReadAddr, &dwLength, pbyReadFpga);
	if (ferr != FFWERR_OK) {
		return ferr;
	}
	memcpy(&wDataTmp, pbyReadFpga, sizeof(WORD));
	*pwData = ConvEndian_W(wDataTmp);

	return ferr;
}

///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * E1E20共通FPGA制御用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFpgaData(void)
{

	return;
	
}


