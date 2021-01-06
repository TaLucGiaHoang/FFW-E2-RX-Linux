///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfw_recframe.cpp
 * @brief ASP記録ファイル処理
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
 ///////////////////////////////////////////////////////////////////////////////
#include "rfwasp_converter.h"
#include "rfwasp_rec.h"
using namespace std;

INT64 RecFrame::_calib[MAX_FRAME_TYPE]; /**< clk補正値 */

RecFrame::RecFrame():
	_rec(),
	_start_pos(0),
	_type(MAX_FRAME_TYPE),
	_clk_hi(0),
	_clk_mid(0),
	_clk_low(0),
	_remain_sz(0),
	_bstart(false),
	_bclk(false),
	_bclk_hi(false),
	_bclk_mid(false),
	_bsfifo_ovf(false)
{
}

void RecFrame::Init()
{
	_rec.clear();
	_start_pos = 0;
	_remain_sz = 0;
	_type = MAX_FRAME_TYPE;
	_clk_hi = 0;
	_clk_mid = 0;
	_clk_low = 0;
	_bstart = false;
	_bclk = false;
	_bclk_hi = false;
	_bclk_mid = false;
	_bsfifo_ovf = false;
}

/*
 * @brief DWORDを繰り返し追加して有効レコードを作製する
 * @param fpga FIFOデータ
 * @return 有効なレコードになったらtrue
 */
bool RecFrame::AddFpga(DWORD fpga)
{
	if (IsDummyFrame(fpga))
		return false;

	if (IsStartFrame(fpga) && GetFrameType(fpga) == SRAM_OVF) {
		/* SRAM_OVFは割り込んで発生するので前のフレームは捨てる */
		Init();
	}

	if (_remain_sz) {
		if (IsStartFrame(fpga)) {
			Init();
			throw fpga_start_bit_twice_err(SRC_LOCATE);
		} else {
			_rec.push_back(fpga);
			--_remain_sz;
		}
	} else {
		if (_bstart) {
			if (IsStartFrame(fpga)) {
				Init();
				throw fpga_start_bit_twice_err(SRC_LOCATE);
			}
		} else {
			if (IsStartFrame(fpga)) {
				_bstart = true;
			} else {
				Init();
				throw none_fpga_start_bit_err(SRC_LOCATE);
			}
		}

		FrameType type = GetFrameType(fpga);
		if (_type == MAX_FRAME_TYPE) {
			_type = type;
		}
		if (type != _type) {
			Init();
			throw illegal_fpga_data_sequence_err(SRC_LOCATE);
		}

		if (IsHiClkFrame(_type, fpga)) {
			_rec.push_back(fpga);
			_clk_hi = GetHiClk(fpga);
			_bclk = true;
			_bclk_hi = true;
			++_start_pos;
		} else if (IsMidClkFrame(_type, fpga, _bclk_hi)) {
			_rec.push_back(fpga);
			_clk_mid = GetMidClk(fpga);
			_bclk = true;
			_bclk_mid = true;
			++_start_pos;
		} else {
			bool except;
			_remain_sz = GetFrameSize(fpga, except);
			if (_remain_sz == -1) {
				Init();
				throw illegal_fpga_size_err(SRC_LOCATE);
			}
			_rec.push_back(fpga);
			_clk_low = GetLowClk(fpga);
			_bclk = false;
			--_remain_sz;
			if (except && _type != SRAM_OVF) {
				_bsfifo_ovf = true;
			}
		}
	}
	if (_bclk == false && _remain_sz == 0)
		return true;
	else
		return false;
}

void RecFrame::Dump()
{
	cout << "type(" << _type << "), ";
	if (_bsfifo_ovf) {
		cout << "sfifo_ovf, ";
	}
	if (_bclk_hi) {
		cout << "clk_hi(" << _clk_hi << "), ";
	}
	if (_bclk_mid) {
		cout << "clk_mid(" << _clk_mid << "), ";
	}
	cout << "clk_low(" << _clk_low << "), ";
	for (std::vector<DWORD>::iterator it = _rec.begin(); it != _rec.end(); ++it) {
		cout << hex << setw(8) << setfill('0') << *it << ", ";
	}
	cout << dec;
}

void RecFrame::ToRaw(UINT64 off, UINT64 cnt, UINT64 clk)
{
	cout << "\t/* off(" << off << ") cnt(" << cnt << ") type(" << _type << ") clk(" << hex << clk << ") */" << endl;
	for (std::vector<DWORD>::iterator it = _rec.begin(); it != _rec.end(); ++it) {
		cout << "\t0x" << hex << setw(8) << setfill('0') << *it << "," << endl;
	}
}

bool RecFrame::disp(RecFrame &rec, UINT64 clk)
{
	cout << clk << " ";
	rec.Dump();
	cout << endl;
	return true;
}

