///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfw_rec.cpp
 * @brief ASP記録ファイル処理
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
#include "rfwasp_rec.h"
#if !defined(ASP_CONSOLE)
#include "doasp_sys_family.h"
#else
bool IsSaveSramRawData()
{
	return true;
}
#endif

using namespace std;

//=============================================================================
/**
 * Debug Dump
 */
//=============================================================================
void RFWASP_REC_HEAD::Dump()
{
	int i;
	char	buf[64];

	cout << "magic " << string((char*)magic) << endl;
	cout << "version " << version << endl;

	time_t t = mtime;
#if defined(_MSC_VER)
	struct tm ltm;
	localtime_s(&ltm, &t);
	strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M", &ltm);
#else
	strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M", localtime(&t));
#endif
	cout << "mtime " << buf << endl;

	for (i = 0; i < 4; ++i) {
		cout << "io_ch[" << i << "] = " << io_ch_type[i] << endl;
	}
	cout << "max_blkcnt = " << max_blkcnt << endl;
	cout << "first_blk = " << first_blk << endl;
	cout << "last_blk = " << last_blk << endl;
	for (i = 0; i < MAX_FRAME_TYPE; ++i) {
		cout << "last_clk[" << i << "] = " << last_clk[i] << "(" << clk_to_nsec(calib_clk(last_clk[i], RecFrame::_calib[i]), OFF) << "ns)" << endl;
	}
	for (i = 0; i < MAX_FRAME_TYPE; ++i) {
		cout << "last_cnt[" << i << "] = " << last_cnt[i] << endl;
	}
	for (i = 0; i < MAX_FRAME_TYPE; ++i) {
		cout << "clk_calib[" << i << "] = " << clk_calib[i] << endl;
	}
}

//=============================================================================
/**
 * ヘッダのmagicナンバが有効か?
 * @retval true 有効
 * @retval false 無効
 */
//=============================================================================
bool RFWASP_REC_HEAD::IsValid()
{
	if (::memcmp(magic, ASP_REC_MAGIC, sizeof(ASP_REC_MAGIC)) != 0)
		return false;
	return true;
}

//=============================================================================
/**
 * ヘッダのversionが有効か?
 * @retval true 有効
 * @retval false 無効
 */
//=============================================================================
bool RFWASP_REC_HEAD::IsValidVer()
{
	return (version == ASP_REC_VER);
}

//=============================================================================
/**
 * debug dump
 */
//=============================================================================
void RFWASP_REC_BLOCK_HEAD::Dump()
{
	int	i;

	cout << "data_size = "  << data_size << endl;

	for (i = 0; i < MAX_FRAME_TYPE; ++i) {
		cout << "clk[" << i << "] = " << clk[i] << "(" << clk_to_nsec(calib_clk(clk[i], RecFrame::_calib[i]), OFF) << "ns)" << endl;
	}
	for (i = 0; i < MAX_FRAME_TYPE; ++i) {
		cout << "cnt[" << i << "] = " << cnt[i] << endl;
	}
	for (i = 0; i < MAX_FRAME_TYPE; ++i) {
		cout << "exist[" << i << "] = " << exist[i] << endl;
	}
}

//=============================================================================
/**
 * debug dump
 * @param bDumpData data部をdumpするか
 * @param bytecnt byte位置
 */
//=============================================================================
void RFWASP_REC_BLOCK::Dump(bool bDumpData, UINT64 &bytecnt)
{
	RecFrame	frame;
	UINT64		clk_hi[MAX_FRAME_TYPE];
	UINT64		clk_mid[MAX_FRAME_TYPE];
	UINT64		cnt[MAX_FRAME_TYPE];
	head.Dump();
	for (int type = 0; type < MAX_FRAME_TYPE; ++type) {
		clk_hi[type] = head.clk[type] & (mk_bit_mask<UINT64>(20) << 28);
		clk_mid[type] = head.clk[type] & (mk_bit_mask<UINT64>(20) << 8);
		cnt[type]  = head.cnt[type];
	}
	if (bDumpData) {
		for (unsigned int i = 0; i < head.data_size; ++i) {
			if (frame.AddFpga(data[i])) {
				cout << cnt[frame._type] << "(" << bytecnt << "), ";
				bytecnt += frame._rec.size() * sizeof(DWORD);
				frame.Dump();
				if (frame._bclk_hi) {
					clk_hi[frame._type] = frame._clk_hi;
				}
				if (frame._bclk_mid) {
					clk_mid[frame._type] = frame._clk_mid;
				}
				++cnt[frame._type];
				UINT64 clk = clk_hi[frame._type] + clk_mid[frame._type] + frame._clk_low;
				UINT64 nsec = clk_to_nsec(calib_clk(clk, RecFrame::_calib[frame._type]), OFF);
				cout << clk << "(" << hex << uppercase << setw(16) << nsec << "/" << dec << nsec << ")" << endl;
				cout << dec << nouppercase;
				frame.Init();
			}
		}
	}
}

void RFWASP_REC_BLOCK::ToRaw(UINT64 &bytecnt)
{
	RecFrame	frame;
	UINT64		clk_hi[MAX_FRAME_TYPE];
	UINT64		clk_mid[MAX_FRAME_TYPE];
	UINT64		cnt[MAX_FRAME_TYPE];
	for (int type = 0; type < MAX_FRAME_TYPE; ++type) {
		clk_hi[type] = head.clk[type] & (mk_bit_mask<UINT64>(20) << 28);
		clk_mid[type] = head.clk[type] & (mk_bit_mask<UINT64>(20) << 8);
		cnt[type]  = head.cnt[type];
	}
	for (unsigned int i = 0; i < head.data_size; ++i) {
		if (frame.AddFpga(data[i])) {
			if (frame._bclk_hi) {
				clk_hi[frame._type] = frame._clk_hi;
			}
			if (frame._bclk_mid) {
				clk_mid[frame._type] = frame._clk_mid;
			}
			UINT64 clk = clk_hi[frame._type] + clk_mid[frame._type] + frame._clk_low;
			frame.ToRaw(bytecnt, cnt[frame._type], clk);
			bytecnt += frame._rec.size() * sizeof(DWORD);
			++cnt[frame._type];
			cout << dec << nouppercase;
			frame.Init();
		}
	}
}


//=============================================================================
/**
 * constructor
 */
//=============================================================================
RfwAspRec::RfwAspRec():
	fname(),
	_head(),
	_cnt(),
	_clk_hi(),
	_clk_mid(),
	_clk_low(),
	_first_clk(),
	_first_cnt(),
	_fio(),
	_blk(0),
	_frame(),
	_valid_timestamp(),
#if defined(RFWASP_THREAD)
#else
	_fsave()
#endif
{
}

//=============================================================================
/**
 * destructor
 */
//=============================================================================
RfwAspRec::~RfwAspRec()
{
#if defined(RFWASP_THREAD)
#else
	_fsave.Close();
#endif
}

//=============================================================================
/**
 * ブロック書き出し
 */
//=============================================================================
void RfwAspRec::SaveBlock()
{
	for (int i = 0; i < MAX_FRAME_TYPE; ++i) {
		_head.last_clk[i] = _clk_hi[i] + _clk_mid[i] + _clk_low[i];
		_head.last_cnt[i] = (_cnt[i] == 0) ? 0 : _cnt[i] - 1;
	}
#if defined(RFWASP_THREAD)
#else
	if (!_fsave.Seek(sizeof(RFWASP_REC_HEAD) + _head.last_blk * sizeof(RFWASP_REC_BLOCK))) {
		delete _blk;
		_blk = 0;
		throw asp_fseek_err(SRC_LOCATE);
	}
	if (!_fsave.Write(_blk, sizeof(RFWASP_REC_BLOCK))) {
		delete _blk;
		_blk = 0;
		throw asp_fwrite_err(SRC_LOCATE);
	}
	delete _blk;
#endif
	_blk = 0;
}

//=============================================================================
/**
 * 新規ブロック追加
 */
//=============================================================================
void RfwAspRec::NewBlock()
{
	if (_head.max_blkcnt && _head.get_blk_cnt() >= _head.max_blkcnt) {
		++_head.first_blk;
		if (_head.first_blk >= _head.max_blkcnt) {
			_head.first_blk = 0;
		}
		++_head.last_blk;
		if (_head.last_blk >= _head.max_blkcnt) {
			_head.last_blk = 0;
		}
	} else {
		++_head.last_blk;
	}
	_blk = new RFWASP_REC_BLOCK();
	for (DWORD i = 0; i < MAX_FRAME_TYPE; ++i) {
		_blk->head.clk[i] = _clk_hi[i] + _clk_mid[i] + _clk_low[i];
		_blk->head.cnt[i] = _cnt[i];
	}
}

//=============================================================================
/**
 * 記録ファイル作成開始
 * @param filename 記録ファイルパス
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspRec::AddFpgaStart(const char *filename)
{
	Init();
	if (_fsave.Open(filename, true) == false) {
		return false;
	}
	if (IsSaveSramRawData()) {
		std::string	s(filename);
		s += ".raw";
		if (_fraw.Open(s.c_str(), true) == false) {
			return false;
		}
	}
	return _fsave.Seek(sizeof(RFWASP_REC_HEAD));
}

//=============================================================================
/**
 * 記録ファイル作成終了
 */
//=============================================================================
void RfwAspRec::AddFpgaFin(void)
{
	if (_blk) {
		SaveBlock();
	}
	/* Write File Header */
	for (int i = 0; i < MAX_FRAME_TYPE; ++i) {
		_head.clk_calib[i] = RecFrame::_calib[i];
	}

#if defined(RFWASP_THREAD)
#else
	time_t tnow = time(0);
	_head.mtime = tnow;

	if (!_fsave.Seek(0)) {
		throw asp_fseek_err(SRC_LOCATE);
	}
	if (!_fsave.Write(&_head, sizeof(_head))) {
		throw asp_fwrite_err(SRC_LOCATE);
	}
	if (!_fsave.Close()) {
		throw asp_fclose_err(SRC_LOCATE);
	}
#endif
	if (IsSaveSramRawData()) {
		if (!_fraw.Close()) {
			throw asp_fclose_err(SRC_LOCATE);
		}
	}
}

//=============================================================================
/**
 * @brief 次のblockを読む
 * @param blk ブロック番号
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspRec::GetNextBlock(UINT64 &blk)
{
	if (blk == _head.last_blk) {
		return false;
	}
	++blk;
	if (_head.max_blkcnt && blk > _head.max_blkcnt) {
		blk = 0;
	}
	ReadBlock(blk);
	return true;
}

//=============================================================================
/*
 * @brief block読み込み
 * @param blk ブロック番号
 * @return 読み込んだブロック
 */
//=============================================================================
RFWASP_REC_BLOCK* RfwAspRec::ReadBlock(UINT64 blk)
{
	bool eof;
	_blk = new RFWASP_REC_BLOCK();

	if (!_fio.Seek(sizeof(RFWASP_REC_HEAD) +
	               blk * sizeof(RFWASP_REC_BLOCK))) {
		delete _blk;
		_blk = 0;
		throw asp_fseek_err(SRC_LOCATE);
	}

	if (!_fio.Read(_blk, sizeof(RFWASP_REC_BLOCK), eof)) {
		delete _blk;
		_blk = 0;
		throw asp_fread_err(SRC_LOCATE);
	}
	return _blk;
}

//=============================================================================
/*
 * @brief FIFOデータからブロックを組み立ててsave
 * @param pdwFpga FIFOデータ
 * @param len FIFOデータ長(DWORD単位)
 * @return ブロックアドレス
 */
//=============================================================================
RFWASP_REC_BLOCK* RfwAspRec::AddFpgaData(const DWORD* pdwFpga, DWORD len)
{
	if (IsSaveSramRawData()) {
		_fraw.Write(pdwFpga, len * sizeof(DWORD));
	}
	if (!_blk) {
		NewBlock();
		_head.last_blk = 0;
	}

	for (DWORD i = 0; i < len; ++i) {
		UINT64 clk = 0;
		DWORD fpga = byte_swap(pdwFpga[i]);
		try {
			if (_frame.AddFpga(fpga) == true) {
				/* global timestamp hiが来るまで捨てる */
				if (_valid_timestamp[_frame._type] == false && _frame._bclk_hi == false) {
					_frame.Init();
					continue;
				}

				/* blk満タンなら新規blk追加 */
				if (_frame._rec.size() + _blk->head.data_size >= RFWASP_REC_BLOCK_DATA_SIZE) {
					SaveBlock();
					NewBlock();
				}

				/* timestamp check */
				clk = ((_frame._bclk_hi) ? _frame._clk_hi : _clk_hi[_frame._type])
					+ ((_frame._bclk_mid) ? _frame._clk_mid : _clk_mid[_frame._type])
					+ _frame._clk_low;
				if (_pre_clk[_frame._type] >= clk) {
					throw clk_ovf_err(SRC_LOCATE);
				} else {
					_pre_clk[_frame._type] = clk;
				}

				if (_frame._bclk_hi) {
					_valid_timestamp[_frame._type] = true;
					_clk_hi[_frame._type] = _frame._clk_hi;
				}
				if (_frame._bclk_mid) {
					_clk_mid[_frame._type] = _frame._clk_mid;
				}
				_clk_low[_frame._type] = _frame._clk_low;

				/* blkに格納 */
				for (std::vector<DWORD>::iterator it = _frame._rec.begin(); it !=  _frame._rec.end(); ++it) {
					_blk->data[_blk->head.data_size++] = *it;
				}

				if (_blk->head.exist[_frame._type] == false) {
					/* block内で最初のFrameだったらblockヘッダ更新 */
					_blk->head.clk[_frame._type] = _clk_hi[_frame._type] + _clk_mid[_frame._type] + _clk_low[_frame._type];
					_blk->head.cnt[_frame._type] = _cnt[_frame._type];
					_blk->head.exist[_frame._type] = true;
				}
				++_cnt[_frame._type];

				/* SRAM OVFの場合は終了する */
				if (_frame._type == SRAM_OVF) {
					throw sram_ovf_err(SRC_LOCATE);
				}
				/* SFIFO OVFの場合は終了する */
				if (_frame._bsfifo_ovf) {
					throw sfifo_ovf_err(SRC_LOCATE);
				}
				_frame.Init();
			}
		} catch (none_fpga_start_bit_err &e) {
			e.what();
			if (std::find(&_valid_timestamp[0], &_valid_timestamp[MAX_FRAME_TYPE], true) != &_valid_timestamp[MAX_FRAME_TYPE]) {
				/* global Timestmap hiが既にあったら異常 */
				throw;
			} else {
				/* global timestap hiまでは捨てる */
				continue;
			}
		} catch (std::runtime_error &e) {
			e.what();
			throw;
		}
	}
	return _blk;
}

//=============================================================================
/*
 * @brief 記録ファイルopen
 * @param name 記録ファイルパス
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspRec::Open(const char* name)
{
	bool eof;
	int	type;
	RFWASP_REC_BLOCK_HEAD	blk_head;

	Init();

	if (_fio.Open(name, false) == false) {
		throw asp_fopen_err(SRC_LOCATE);
	}
	if (_fio.Read(&_head, sizeof(_head), eof) == false) {
		_fio.Close();
		throw asp_fread_err(SRC_LOCATE);
	}
	/* magic & file version check */
	if (!_head.IsValid() || !_head.IsValidVer()) {
		_fio.Close();
		return false;
	}
	for (int i = 0; i < MAX_FRAME_TYPE; ++i) {
		RecFrame::_calib[i] = _head.clk_calib[i];
	}

	/*
	 * リングの場合、有効な範囲が不明になるので再設定する
	 * 書込み中に有効範囲を補正するには捨てるブロックの読み出しが
	 * 必要になるので行わない
	 * (メモリ上にブロックヘッダを全部置ければよいが..)
	 */

	/* 最初のclk/cntを探す */
	for (UINT64 lblk = 0; lblk < _head.get_blk_cnt(); ++lblk) {
		get_blk_head(_head.lblk_to_blk(lblk), blk_head);
		for (int i = 0; i < MAX_FRAME_TYPE; ++i) {
			if (_head.last_clk[i] && blk_head.exist[i]) {
				if (_first_clk[i] == 0) {
					_first_clk[i] = blk_head.clk[i];
				}
				if (_first_cnt[i] == 0) {
					_first_cnt[i] = blk_head.cnt[i];
				}
			}
		}
		for (type = 0; type < MAX_FRAME_TYPE; ++type) {
			if (_first_clk[type] == 0 && _head.last_clk[type])
				break;
		}
		if (type == MAX_FRAME_TYPE)
			break;
	}
	/* 最初のclkが見つからない場合はlastも0にする */
	for (type = 0; type < MAX_FRAME_TYPE; ++type) {
		if (_first_clk[type] == 0 && _head.last_cnt[type]) {
			_head.last_cnt[type] = 0;
			_head.last_clk[type] = 0;
		}
	}
	return true;
}

//=============================================================================
/*
 * @brief ヘッダ部から有効なデータ範囲を返す
 * @param type FrameType
 * @param pu64start	開始クロック
 * @param pu64end 終了クロック
 * @param pu64cnt 記録回数
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspRec::GetAvail(FrameType type, UINT64* pu64start, UINT64* pu64end, UINT64* pu64cnt)
{
	if (!IsOpen())
		return false;

	if (_head.last_clk[type]) {
		*pu64start = _first_clk[type];
		*pu64end = _head.last_clk[type];
		*pu64cnt = _head.last_cnt[type] - _first_cnt[type] + 1;
	} else {
		*pu64start = 0;
		*pu64end = 0;
		*pu64cnt = 0;
	}
	return true;
}

//=============================================================================
/*
 * @brief 記録ファイルclose
 */
//=============================================================================
void RfwAspRec::Close()
{
	if (!_fio.Close()) {
		throw asp_fclose_err(SRC_LOCATE);
	}
}
