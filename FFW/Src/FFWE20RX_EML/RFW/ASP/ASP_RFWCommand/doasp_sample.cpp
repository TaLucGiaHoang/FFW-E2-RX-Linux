///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_sample.cpp
 * @brief データ取得の実装ファイル
 * @author TSSR M.Ogata, PA M.Tsuji
 * @author Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2017/02/07 新規作成 TSSR M.Ogata
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#include "doasp_sample.h"
#include "rfwasp_sample.h"
#include "protasp_cpu.h"
#include "emudef.h"
#include "rfwasp_rec.h"
#include "rfwasp_converter.h"
#include "doasp_sys_family.h"
#include "asp_setting.h"
#include "doasp_sys.h"
#include "RFWASP.h"
#include "e2_asp_fpga.h"

// ファイル内static変数の宣言
RfwAspRec	s_AspRec;
char s_fname[MAX_PATH];

// ファイル内static関数の宣言

///////////////////////////////////////////////////////////////////////////////
// static関数
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// 初期化関数
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * ASP FIFO read
 * @param dwReadByte リードバイト数
 * @param pbyReadBuff リードデータ格納バッファアドレス
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DWORD DO_AspIn(DWORD dwReadByte, DWORD *const pbyReadBuff)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();
	// エミュレータ種別、BFW動作モードに対応したCPU_Rコマンドを発行(エミュレータ種別はRFW内部変数を参照)
	dwRet = ProtAspIn(dwReadByte, pbyReadBuff);
	ProtEnd_ASP();
	return dwRet;
}

//=============================================================================
/**
 * ASP Set Port
 * @param byUsbRdyN
 * @param byBfwRdyN
 * @retval RFWERR_OK  正常動作
 * @retval RFWERR_ERR エラーあり
 */
//=============================================================================
DWORD DO_AspSetPort(BYTE byUsbRdyN, BYTE byBfwRdyN)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();
	// エミュレータ種別、BFW動作モードに対応したCPU_Rコマンドを発行(エミュレータ種別はRFW内部変数を参照)
	dwRet = ProtAspSetPort(byUsbRdyN, byBfwRdyN);
	ProtEnd_ASP();
	return dwRet;
}

//=============================================================================
/**
 * ASP GetSample
 * @param dwType 記録データ種別
 * @param dwKeyType 0:時間指定 1:変化点数指定
 * @param u64Start 開始時間/開始変化点
 * @param u64End 終了時間/終了変化点
 * @param pData 格納領域
 * @param dwSize 格納領域のサイズ
 * @param pdwStoreCnt 格納領域に格納した個数
 * @param pdwExistCnt 指定範囲に存在する個数/0の場合はdwSizeまで格納したらそれ以上検索しない
 * @param pu64Read 格納した次の有効時間/変化点数
 * @retval RFWERR_OK
 * @retval RFWERR_N_BMCU_RUN
 * @retval RFWERR_N_BASP_LOAD
 * @retval RFWERR_N_BASP_F_FORMAT
 * @retval RFWERR_N_BASP_F_READ
 * @retval RFWERR_N_BASP_F_SEEK
 */
//=============================================================================
DWORD DO_AspGetSample(
    DWORD dwType,
    DWORD dwKeyType,
    UINT64 u64Start,
    UINT64 u64End,
    void *pData,
    DWORD dwSize,
    DWORD *pdwStoreCnt,
    DWORD *pdwExistCnt,
    UINT64 *pu64Read)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	if (!s_AspRec.IsOpen()) {
		dwRet = RFWERR_N_BASP_LOAD;
	}

	if (dwRet == RFWERR_OK) {
		AspConverter::set_cnv_buf(pData, dwSize);
		AspConverter::_pexist_cnt = pdwExistCnt;
		try {
			if (dwKeyType == 0) {
				s_AspRec.GetFrame(
						AspConverter::to_frametype(dwType),
						calib_clk(nsec_to_clk_start(u64Start), -RecFrame::_calib[AspConverter::to_frametype(dwType)]),
						calib_clk(nsec_to_clk_end(u64End), -RecFrame::_calib[AspConverter::to_frametype(dwType)]),
						pu64Read,
						RfwAspRec::clk_tag(),
						AspConverter::extract);
			} else {
				s_AspRec.GetFrame(
						AspConverter::to_frametype(dwType),
						u64Start,
						u64End,
						pu64Read,
						RfwAspRec::cnt_tag(),
						AspConverter::extract);

			}
		} catch (fpga_format_err &e) {
			e.what();
			/* 何かbugあり */
			dwRet = RFWERR_N_BASP_F_FORMAT;
		} catch (asp_fread_err &e) {
			e.what();
			dwRet = RFWERR_N_BASP_F_READ;
		} catch (asp_fseek_err &e) {
			e.what();
			dwRet = RFWERR_N_BASP_F_SEEK;
		}
	}

	if (dwRet == RFWERR_OK) {
		*pdwStoreCnt = AspConverter::_store_cnt;
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * ASP GetSampleDataSize
 * @param dwType 記録データ種別
 * @param dwKeyType 0:時間指定 1:変化点数指定
 * @param u64Start 開始時間/開始変化点
 * @param u64End 終了時間/終了変化点
 * @param pu64DataSize データサイズ
 * @retval RFWERR_OK
 * @retval RFWERR_N_BMCU_RUN
 * @retval RFWERR_N_BASP_LOAD
 * @retval RFWERR_N_BASP_F_FORMAT
 * @retval RFWERR_N_BASP_F_READ
 * @retval RFWERR_N_BASP_F_SEEK
 */
//=============================================================================
DWORD DO_AspGetSampleDataSize(
    DWORD dwType,
    DWORD dwKeyType,
    UINT64 u64Start,
    UINT64 u64End,
    UINT64 *pu64DataSize)
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	if (!s_AspRec.IsOpen()) {
		dwRet =  RFWERR_N_BASP_LOAD;
	}
	if (dwRet == RFWERR_OK) {
		AspConverter::set_cnv_buf(0, 0);
		try {
			if (dwKeyType == 0) {
				s_AspRec.GetFrame(
						AspConverter::to_frametype(dwType),
						calib_clk(nsec_to_clk_start(u64Start), -RecFrame::_calib[AspConverter::to_frametype(dwType)]),
						calib_clk(nsec_to_clk_end(u64End), -RecFrame::_calib[AspConverter::to_frametype(dwType)]),
						0,
						RfwAspRec::clk_tag(),
						AspConverter::extract);
			} else {
				s_AspRec.GetFrame(
						AspConverter::to_frametype(dwType),
						u64Start,
						u64End,
						0,
						RfwAspRec::cnt_tag(),
						AspConverter::extract);
			}
		} catch (fpga_format_err &e) {
			e.what();
			/* 何かbugあり */
			dwRet = RFWERR_N_BASP_F_FORMAT;
		} catch (asp_fread_err &e) {
			e.what();
			dwRet = RFWERR_N_BASP_F_READ;
		} catch (asp_fseek_err &e) {
			e.what();
			dwRet = RFWERR_N_BASP_F_SEEK;
		}
	}

	if (dwRet == RFWERR_OK) {
		*pu64DataSize = AspConverter::_cnv_cnt;
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * Asp SetSaveFileName
 * @param fname ファイル名
 * @retval RFWERR_OK
 */
//=============================================================================
DWORD DO_AspSetSaveFileName(const char *fname)
{
	ProtInit_ASP();

	strcpy_s(s_fname, sizeof(s_fname), fname);

	ProtEnd_ASP();

	return RFWERR_OK;
}

void DO_ClrAspSaveFileName(void)
{
	s_fname[0] = 0;
}


//=============================================================================
/**
 * Asp SaveFile
 * @param none
 * @retval RFWERR_OK
 * @retval RFWERR_N_BMCU_RUN
 * @retval RFWERR_N_BASP_F_OPEN
 * @retval RFWERR_N_BASP_FPGA_START2
 * @retval RFWERR_N_BASP_FPGA_SEQ
 * @retval RFWERR_N_BASP_FPGA_SIZE
 * @retval RFWERR_N_BASP_F_OPEN
 * @retval RFWERR_N_BASP_F_SEEK
 * @retval RFWERR_N_BASP_F_READ
 * @retval RFWERR_N_BASP_F_WRITE
 * @retval RFWERR_N_BASP_F_CLOSE
 */
//=============================================================================
DWORD DO_AspSaveFile()
{
	DWORD	dwRet = RFWERR_OK;

	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	if (dwRet == RFWERR_OK) {
		dwRet = Save_Asp();
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * Asp LoadSample
 * @param fname ファイル名
 * @retval RFWERR_OK
 * @retval RFWERR_N_BASP_ALREADY_LOAD
 * @retval RFWERR_N_BASP_F_FORMAT
 * @retval RFWERR_N_BASP_F_OPEN
 * @retval RFWERR_N_BASP_F_READ
 */
//=============================================================================
DWORD DO_AspLoadSample(const char *fname)
{
	DWORD dwRet = RFWERR_OK;

	ProtInit_ASP();

	if (s_AspRec.IsOpen()) {
		dwRet =  RFWERR_N_BASP_ALREADY_LOAD;
	}
	if (dwRet == RFWERR_OK) {
		try {
			if (s_AspRec.Open(fname)) {
				dwRet = RFWERR_OK;
			} else {
				dwRet = RFWERR_N_BASP_F_FORMAT;
			}
		} catch (asp_fopen_err &e) {
			e.what();
			dwRet = RFWERR_N_BASP_F_OPEN;
		} catch (asp_fread_err &e) {
			e.what();
			dwRet = RFWERR_N_BASP_F_READ;
		}
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * ASP UnLoadSample
 * @param なし
 * @retval RFWERR_OK
 * @retval RFWERR_N_BASP_LOAD
 */
//=============================================================================
DWORD DO_AspUnLoadSample(void)
{
	DWORD dwRet = RFWERR_OK;

	ProtInit_ASP();
	if (!s_AspRec.IsOpen()) {
		dwRet =  RFWERR_N_BASP_LOAD;
	}
	if (dwRet == RFWERR_OK) {
		s_AspRec.Close();
	}
	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * AspSramSave
 * @note SRAMに入っているデータ数分のみ取り出してセーブする
 * @note この関数が呼ばれる時点でDTFIFOが有効となって、
 * @note データの移動が発生しているためDTFIFO有効前に読み出し、
 * @note パラメータで読み出しサイズを渡す
 * @param dwSrambyte リードするSRAMのサイズ(バイト)
 * @retval RFWERR_OK
 * @retval RFWERR_N_BASP_FPGA_START2
 * @retval RFWERR_N_BASP_FPGA_SEQ
 * @retval RFWERR_N_BASP_FPGA_SIZE
 * @retval RFWERR_N_BASP_F_OPEN
 * @retval RFWERR_N_BASP_F_SEEK
 * @retval RFWERR_N_BASP_F_READ
 * @retval RFWERR_N_BASP_F_WRITE
 * @retval RFWERR_N_BASP_F_CLOSE
 */
//=============================================================================
DWORD AspSramSave(DWORD dwSrambyte)
{
	DWORD	rc = RFWERR_OK;
	static const int insize = 1024 * 1024;	// 最大1MBサイズ単位で読み出す
	DWORD	dwSize;

	if (s_AspRec.AddFpgaStart(s_fname) == false) {
		return RFWERR_N_BASP_F_OPEN;
	}

	DWORD *srambuf = new DWORD[insize / sizeof(DWORD)];
	/*
	 * SRAMカウントは信用できるため利用して、入っているデータ分のみ読み出す
	 * データサイズはパラメータとして受け取る
	 */

	while (dwSrambyte > 0) {
		// SRAMに入っているデータが1MBに満たない場合は、SRAMに入っているデータ分だけ読みだす
		dwSize = min(insize, dwSrambyte);
		DO_AspIn(dwSize, srambuf);
		try {
			s_AspRec.AddFpgaData(srambuf, dwSize / sizeof(DWORD));
		} catch (none_fpga_start_bit_err &e) {
			e.what();
			rc = RFWERR_N_BASP_FPGA_START2;
			break;
		} catch (fpga_start_bit_twice_err &e) {
			e.what();
			rc = RFWERR_N_BASP_FPGA_START2;
			break;
		} catch (illegal_fpga_data_sequence_err &e) {
			e.what();
			rc = RFWERR_N_BASP_FPGA_SEQ;
			break;
		} catch (illegal_fpga_size_err &e) {
			e.what();
			rc = RFWERR_N_BASP_FPGA_SIZE;
			break;
		} catch (asp_fopen_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_OPEN;
			break;
		} catch (asp_fseek_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_SEEK;
			break;
		} catch (asp_fread_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_READ;
			break;
		} catch (asp_fwrite_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_WRITE;
			break;
		} catch (asp_fclose_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_CLOSE;
			break;
		} catch (sram_ovf_err &e) {
			e.what();
			rc = RFWERR_N_BASP_SRAM_OVF;
			break;
		} catch (sfifo_ovf_err &e) {
			e.what();
			rc = RFWERR_N_BASP_SFIFO_OVF;
			break;
		} catch (clk_ovf_err &e) {
			e.what();
			rc = RFWERR_N_BASP_TSTAMP_OVER;
			break;
		}
		dwSrambyte -= dwSize;
	}

	try {
		s_AspRec.AddFpgaFin();
	} catch (asp_fseek_err &e) {
		e.what();
		if (rc != RFWERR_OK) {
			rc = RFWERR_N_BASP_F_SEEK;
		}
	} catch (asp_fwrite_err &e) {
		e.what();
		if (rc != RFWERR_OK) {
			rc = RFWERR_N_BASP_F_WRITE;
		}
	} catch (asp_fclose_err &e) {
		e.what();
		if (rc != RFWERR_OK) {
			rc = RFWERR_N_BASP_F_CLOSE;
		}
	}
	delete [] srambuf;
	return rc;
}

//=============================================================================
/**
 * GetAvailableTime
 * @param dwType 		記録データ種別
 * @param pu64Start		有効な開始時間
 * @param pu64End		有効な終了時間
 * @param pu64Sample	カウント
 * @retval RFWERR_OK
 * @retval RFWERR_N_BMCU_RUN
 * @retval RFWERR_N_BASP_LOAD
 */
//=============================================================================
DWORD DO_GetAvailableTime(DWORD dwType, UINT64 *const pu64Start, UINT64 *const pu64End, UINT64 *const pu64Sample)
{
	DWORD dwRet = RFWERR_OK;
	ProtInit_ASP();

	// ターゲットがRUN中かチェック
	if (dwRet == RFWERR_OK) {
		if (IsStatRun() == TRUE) {
			dwRet = RFWERR_N_BMCU_RUN;
		}
	}

	if (dwRet == RFWERR_OK) {
		if (s_AspRec.GetAvail(AspConverter::to_frametype(dwType), pu64Start, pu64End, pu64Sample) == true) {
			*pu64Start = clk_to_nsec(calib_clk(*pu64Start, AspConverter::to_frametype(dwType)), OFF);
			*pu64End = clk_to_nsec(calib_clk(*pu64End, AspConverter::to_frametype(dwType)), OFF);
			dwRet = RFWERR_OK;
		} else {
			dwRet = RFWERR_N_BASP_LOAD;
		}
	}

	ProtEnd_ASP();

	return dwRet;
}

//=============================================================================
/**
 * DO_GetSampleState
 * @param pdwState 記録動作状態
 * @param pu64Num 記録したデータのバイト数
 * @param pu64Max 最大記録バイト数
 * @retval RFWERR_OK
 * @retval RFWERR_N_UNSUPPORT
 */
//=============================================================================
DWORD DO_GetSampleState(DWORD *const pdwState, UINT64 *const pu64Num, UINT64 *const pu64Max)
{
	DWORD dwRet = RFWERR_OK;
	BOOL bFull = FALSE;
	BOOL bOvf = FALSE;

	RFW_ASPCONF_DATA	AspConfData;

	ProtInit_ASP();

	// コンフィグ情報の取得
	GetAspConfData(&AspConfData);

	if (AspConfData.dwStorage == RF_ASPSTORAGE_PC) {
		dwRet = RFWERR_N_UNSUPPORT;
	}

	if (dwRet == RFWERR_OK) {
		if ((FPGA_READ(ASPCTRL) & ASPCTRL_ASPRUN) != 0) {
			*pdwState = RF_ASPSAMPLE_RUN;
		} else {
			*pdwState = RF_ASPSAMPLE_STOP;
		}
		*pu64Max = SRAM_MAX_BYTE;
		*pu64Num = (FPGA_READ(SRAM_STAT) & SRAM_STAT_SRAMCOUNT_MASK) * sizeof(WORD);

		// SRAMフル状態の確認
		if (IsSramFullStat_family()) {
			bFull = TRUE;
		}
		if ((FPGA_READ(SRAM_STAT) & SRAM_STAT_SRAM_LIMIT) != 0) {
			bFull = TRUE;
		}

		if ((FPGA_READ(SRAM_STAT) & SRAM_STAT_SRAM_OVF) != 0) {
			// SRAMオーバーフロー状態
			bOvf = TRUE;
		}
		if (bFull && (AspConfData.dwStorage == RF_ASPSTORAGE_E2_FULLSTOP)) {
			*pdwState |= RF_ASPSAMPLE_FULLSTOP;
		} else if (bFull && (AspConfData.dwStorage == RF_ASPSTORAGE_E2_FULLBREAK)) {
			*pdwState |= RF_ASPSAMPLE_FULLBREAK;
		} else if (bOvf && (AspConfData.dwStorage == RF_ASPSTORAGE_E2)) {
			*pdwState |= RF_ASPSAMPLE_WRAPAROUND;
		} else {
			// PCストレージは未サポート
		}
	}

	ProtEnd_ASP();

	return dwRet;
}

/**
 * Asp SetClkCalibration
 * @param dwType
 * @param i64Calib
 * @return RFWERR_OK
 */
DWORD DO_SetClkCalibration(DWORD dwType, INT64 i64Calib)
{
	ProtInit_ASP();

	RecFrame::_calib[AspConverter::to_frametype(dwType)] = i64Calib;

	ProtEnd_ASP();

	return RFWERR_OK;
}

/**
 * Asp AddRawFile
 * @param fname
 * @return RFWERR_OK
 */
DWORD DO_AspAddRaw(const char *rawfile, const char *datfile)
{
	DWORD	rc = RFWERR_OK;
	static const int insize = 1024 * 1024;	// 最大1MBサイズ単位で読み出す
	DWORD	dwSize;

	if (s_AspRec.AddFpgaStart(datfile) == false) {
		return RFWERR_N_BASP_F_OPEN;
	}

	RfwAspFio raw;
	if (raw.Open(rawfile, false) == false) {
		return RFWERR_N_BASP_F_OPEN;
	}

	bool	beof = false;
	DWORD *srambuf = new DWORD[insize / sizeof(DWORD)];

	while (!beof) {
		dwSize = raw.Read(srambuf, insize, beof);
		try {
			s_AspRec.AddFpgaData(srambuf, dwSize / sizeof(DWORD));
		} catch (none_fpga_start_bit_err &e) {
			e.what();
			rc = RFWERR_N_BASP_FPGA_START2;
			break;;
		} catch (fpga_start_bit_twice_err &e) {
			e.what();
			rc = RFWERR_N_BASP_FPGA_START2;
			break;
		} catch (illegal_fpga_data_sequence_err &e) {
			e.what();
			rc = RFWERR_N_BASP_FPGA_SEQ;
			break;
		} catch (illegal_fpga_size_err &e) {
			e.what();
			rc = RFWERR_N_BASP_FPGA_SIZE;
			break;
		} catch (asp_fopen_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_OPEN;
			break;
		} catch (asp_fseek_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_SEEK;
			break;
		} catch (asp_fread_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_READ;
			break;
		} catch (asp_fwrite_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_WRITE;
			break;
		} catch (asp_fclose_err &e) {
			e.what();
			rc = RFWERR_N_BASP_F_CLOSE;
			break;
		} catch (sram_ovf_err &e) {
			e.what();
			rc = RFWERR_N_BASP_SRAM_OVF;
			break;
		} catch (sfifo_ovf_err &e) {
			e.what();
			rc = RFWERR_N_BASP_SFIFO_OVF;
			break;
		} catch (clk_ovf_err &e) {
			e.what();
			rc = RFWERR_N_BASP_TSTAMP_OVER;
			break;
		}
	}
	raw.Close();

	try {
		s_AspRec.AddFpgaFin();
	} catch (asp_fseek_err &e) {
		e.what();
		if (rc != RFWERR_OK) {
			rc = RFWERR_N_BASP_F_SEEK;
		}
	} catch (asp_fwrite_err &e) {
		e.what();
		if (rc != RFWERR_OK) {
			rc = RFWERR_N_BASP_F_WRITE;
		}
	} catch (asp_fclose_err &e) {
		e.what();
		if (rc != RFWERR_OK) {
			rc = RFWERR_N_BASP_F_CLOSE;
		}
	}
	delete [] srambuf;
	return rc;
}


