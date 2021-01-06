///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_rec.h
 * @brief ASP記録ファイル処理
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef __RFWASP_REC_H__
#define __RFWASP_REC_H__

#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include "rfw_bitops.h"
#include "rfwasp_fio.h"
#include "rfwasp_recdata.h"
#include "rfwasp_recframe.h"
#include "rfwasp_converter.h"

class fpga_format_err : public std::runtime_error
{
public:
	explicit fpga_format_err(const std::string& s):
		std::runtime_error(s) {}
};

class fpga_start_bit_twice_err : public fpga_format_err
{
public:
	explicit fpga_start_bit_twice_err(const std::string& s):
		fpga_format_err(s) {}
};

class none_fpga_start_bit_err : public fpga_format_err
{
public:
	explicit none_fpga_start_bit_err(const std::string& s):
		fpga_format_err(s) {}
};

class illegal_fpga_data_sequence_err : public fpga_format_err
{
public:
	explicit illegal_fpga_data_sequence_err(const std::string& s):
		fpga_format_err(s) {}
};

class illegal_fpga_size_err : public fpga_format_err
{
public:
	explicit illegal_fpga_size_err(const std::string& s):
		fpga_format_err(s) {}
};

class fpga_runtime_err : public std::runtime_error
{
public:
	explicit fpga_runtime_err(const std::string& s):
		std::runtime_error(s) {}
};

class sram_ovf_err : public fpga_runtime_err
{
public:
	explicit sram_ovf_err(const std::string& s):
		fpga_runtime_err(s) {}
};

class sfifo_ovf_err : public fpga_runtime_err
{
public:
	explicit sfifo_ovf_err(const std::string& s):
		fpga_runtime_err(s) {}
};

class clk_ovf_err : public fpga_runtime_err
{
public:
	explicit clk_ovf_err(const std::string& s):
		fpga_runtime_err(s) {}
};

class asp_fio_err : public std::runtime_error
{
public:
	explicit asp_fio_err(const std::string& s):
		std::runtime_error(s) {}
};

class asp_fopen_err : public asp_fio_err
{
public:
	explicit asp_fopen_err(const std::string& s):
		asp_fio_err(s) {}
};

class asp_fseek_err : public asp_fio_err
{
public:
	explicit asp_fseek_err(const std::string& s):
		asp_fio_err(s) {}
};

class asp_fwrite_err : public asp_fio_err
{
public:
	explicit asp_fwrite_err(const std::string& s):
		asp_fio_err(s) {}
};

class asp_fread_err : public asp_fio_err
{
public:
	explicit asp_fread_err(const std::string& s):
		asp_fio_err(s) {}
};

class asp_fclose_err : public asp_fio_err
{
public:
	explicit asp_fclose_err(const std::string& s):
		asp_fio_err(s) {}
};

const char ASP_REC_MAGIC[] = "LPRUPDE";
const DWORD	ASP_REC_VER = 1;

/* ヘッダのサイズ 4kバウンダリにする */
static const int RFWASP_REC_HEAD_SIZE = 4096;

/**
 * 記録ファイルヘッダ
 *
 * 記録ファイルの構造
 *
 *  |                      |          |
 *  |:--------------------:|:--------:|
 *  | ヘッダ部(4kbyte)     | RFWASP_REC_HEAD |
 *  | block(1Mbyte)        | RFWASP_REC_BLOCK (RFWASP_REC_BLOCK_HEAD) |
 *  | block(1Mbyte)        | RFWASP_REC_BLOCK (RFWASP_REC_BLOCK_HEAD) |
 *  | block(1Mbyte)        | RFWASP_REC_BLOCK (RFWASP_REC_BLOCK_HEAD) |
 *  | block(1Mbyte)        | RFWASP_REC_BLOCK (RFWASP_REC_BLOCK_HEAD) |
 *  | ... 最大block数まで  | ... |
 *
 */
struct RFWASP_REC_HEAD {
	BYTE		magic[8];					/**< MAGIC NUMBER */
	DWORD		version;					/**< ファイルバージョン */
	UINT64		mtime;						/**< 最終記録時間(time_tが環境で違うのでUINT64) */
	DWORD		io_ch_type[4];				/**< IO_CH0_TX/RX, IO_CH1_TX/RXがロジアナ/UART/SPI/I2C/CANのどれになっているか */
	UINT64		max_blkcnt;				    /**< リングの場合の最大ブロック個数,無制限の場合は0 */
	UINT64		first_blk;					/**< リングの最初のブロック,無制限では0 */
	UINT64		last_blk;					/**< リングの最後のブロック,無制限ではUINT64_MAXまで */
	UINT64		last_clk[MAX_FRAME_TYPE];	/**< ファイル中の最後のクロック */
	UINT64		last_cnt[MAX_FRAME_TYPE];	/**< ファイル中の最後のカウント */
	INT64		clk_calib[MAX_FRAME_TYPE];	/**< クロック補正値 */

	static const int _real_size	=
	    sizeof(BYTE) * 8 + \
	    sizeof(DWORD) + \
	    sizeof(UINT64) + \
	    sizeof(DWORD) * 4 + \
	    sizeof(UINT64) + \
	    sizeof(UINT64) + \
	    sizeof(UINT64) + \
	    sizeof(UINT64) * MAX_FRAME_TYPE + \
	    sizeof(UINT64) * MAX_FRAME_TYPE + \
	    sizeof(INT64) * MAX_FRAME_TYPE;

	BYTE		_pad[RFWASP_REC_HEAD_SIZE - _real_size];		// 4K padding

	RFWASP_REC_HEAD()
	{
		Init();
	}

	void Init()
	{
		::memset(this, 0, sizeof(RFWASP_REC_HEAD));
		version = ASP_REC_VER;
		::memcpy(magic, ASP_REC_MAGIC, sizeof(ASP_REC_MAGIC));
	}

	/*
	 * @brief 論理ブロック番号(リングのfirst...last)から物理ブロック番号に変換
	 */
	UINT64 lblk_to_blk(UINT64 lblk)
	{
		if (lblk == (std::numeric_limits<UINT64>::max)()) {
			return last_blk;
		} else if (max_blkcnt == 0) {
			return lblk;
		} else {
			if (lblk == max_blkcnt) {
				return last_blk;
			}
			UINT64 blk = lblk + first_blk;
			if (blk >= max_blkcnt) {
				blk -= max_blkcnt;
			}
			return blk;
		}
	}

	/*
	 * @brief 有効ブロック数
	 */
	static bool non_zero(UINT64 n) {return (n != 0);}
	UINT64 get_blk_cnt()
	{
		if (std::find_if(&last_clk[0], &last_clk[MAX_FRAME_TYPE], non_zero) == &last_clk[MAX_FRAME_TYPE]) {
			/* last_clkが全部0ならば有効ブロックはない */
			return 0;
		} else {
			if (first_blk <= last_blk) {
				return last_blk - first_blk + 1;
			} else {
				return max_blkcnt;
			}
		}
	}

	void Dump();
	bool IsValid();		/* 有効なファイルヘッダか? */
	bool IsValidVer();	/* 有効なファイルversionか? */
};

/* Headerが4Kバウンダリかcompile時にcheckする */
typedef char _chk_rfwasp_rec_head_size[(sizeof(RFWASP_REC_HEAD) % 4096 == 0) * 2 - 1];

/** 記録ファイル内ブロックのヘッダ */
struct RFWASP_REC_BLOCK_HEAD {
	DWORD	data_size;				/**< DWORD単位で有効な個数(Frame数ではない) */
	UINT64	clk[MAX_FRAME_TYPE];	/**< BLOCK中の最小clk,有効Frameがない場合は前のBlockの最終clk */
	UINT64	cnt[MAX_FRAME_TYPE];	/**< BLOCK中の最小cnt,有効Frameがない場合は前のBLOCKの最終cnt */
	bool	exist[MAX_FRAME_TYPE];	/**< 有効Frameがあるか? */
	RFWASP_REC_BLOCK_HEAD() : data_size(0), clk(), cnt(), exist()
	{
	}
	void Dump();
	void GetClkCnt(FrameType type, UINT64 &clk_hi, UINT64 &clk_mid, UINT64 &rcnt) const
	{
		clk_hi = clk[type] & (mk_bit_mask<UINT64>(20) << 28);
		clk_mid = clk[type] & (mk_bit_mask<UINT64>(20) << 8);
		rcnt = cnt[type];
	}
};

/*
 * BLOCK HEADER込みの1ブロックのバイト数
 * 4Kバウンダリにする。
 */
static const DWORD RFWASP_REC_BLOCK_SIZE = 1024 * 1024;

/*
 * BLOCK内のDWORD単位の最大データ数
 * global time stamp hi + global time stamp mid + データフレーム最大8
 * で 10以上であること
 */
static const DWORD RFWASP_REC_BLOCK_DATA_SIZE = (RFWASP_REC_BLOCK_SIZE - sizeof(RFWASP_REC_BLOCK_HEAD)) / sizeof(DWORD);

/** 記録ファイル内のブロック */
struct RFWASP_REC_BLOCK {
	RFWASP_REC_BLOCK_HEAD	head;
	DWORD					data[RFWASP_REC_BLOCK_DATA_SIZE];
	static const int _real_size  = (sizeof(RFWASP_REC_BLOCK_HEAD) + RFWASP_REC_BLOCK_DATA_SIZE * sizeof(DWORD));

	/* 4Kバウンダリかcompile時にcheckする */
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4200)
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma	GCC diagnostic ignored "-Wpedantic"
#endif
	BYTE					_pad[RFWASP_REC_BLOCK_SIZE - _real_size];
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

	RFWASP_REC_BLOCK() : head()
	{
	}
	void Dump(bool bDumpData, UINT64 &bytecnt);
	void ToRaw(UINT64 &bytecnt);
};

/* BLOCKが4Kバウンダリかcompile時にcheckする */
typedef char _chk_rfwasp_rec_block_size[(sizeof(RFWASP_REC_BLOCK) % 4096 == 0) * 2 - 1];

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif


class RfwAspRec
{
public:
	RfwAspRec();
	virtual ~RfwAspRec();

	struct clk_tag {};
	struct cnt_tag {};

	/**
	 * @brief BLOCKのbinary search (指定値以上:指定値含む)
	 * @param type FrameType
	 * @param u64val search value
	 * @return BLOCK first_blockを0とした論理block番号
	 * @return std::numeric_limits<UINT64>::max() blockが見つからない
	 */
	template <class TAG> UINT64 lower_bound_lblk(FrameType type, UINT64 u64val, TAG tag)
	{
		UINT64 len = _head.get_blk_cnt();
		UINT64 first = 0;
		UINT64 half;
		UINT64 middle;
		UINT64 curval;

		while (len > 0) {
			half = len / 2;
			middle = first + half;
			curval = get_blk_val(type, _head.lblk_to_blk(middle), tag);
#if defined(_debug_asp)
			std::cout
			        << "middle(" << middle << "), "
			        << "curval(" << curval << ")" << std::endl;
#endif
			if (curval < u64val) {
				first = middle + 1;
				len = len - half - 1;
			} else {
				len = half;
			}
		}
		if (first >= _head.get_blk_cnt()) {
			return (std::numeric_limits<UINT64>::max)();
		}
		return first;
	}

	/**
	 * @brief BLOCKのbinary search (指定値より大きい:指定値含まず)
	 * @param type FrameType
	 * @param u64val search value
	 * @return BLOCK first_blkを0とした論理block番号
	 * @return std::numeric_limits<UINT64>::max blockが見つからない
	 */
	template <class TAG> UINT64 upper_bound_lblk(FrameType type, UINT64 u64val, TAG tag)
	{
		UINT64 len = _head.get_blk_cnt();
		UINT64 first = 0;
		UINT64 half;
		UINT64 middle;
		UINT64 curval;

		while (len > 0) {
			half = len / 2;
			middle = first + half;
			curval = get_blk_val(type, _head.lblk_to_blk(middle), tag);
#if defined(_DEBUG_ASP)
			std::cout
			        << "middle(" << middle << "), "
			        << "curval(" << curval << ")" << std::endl;
#endif
			if (u64val < curval) {
				len = half;
			} else {
				first = middle + 1;
				len = len - half - 1;
			}
		}
		if (first >= _head.get_blk_cnt()) {
			return (std::numeric_limits<UINT64>::max)();
		}
		return first;
	}

	/**
	 * @brief 検索値のある先頭のblkを探す
	 * @param type FrameType
	 * @param u64val search value
	 * @param tag clk/cnt
	 * @return 物理ブロック番号
	 */
	template <class TAG> UINT64 find_blk(FrameType type, UINT64 u64val, TAG tag)
	{
		UINT64 	lblk = lower_bound_lblk(type, u64val, tag);
		UINT64	blk;
		if (lblk != (std::numeric_limits<UINT64>::max)()) {
			if (u64val < get_blk_val(type, _head.lblk_to_blk(lblk), tag)) {
				/*
				 * 検索値以上のblockの場合、一つ前のblcokを返す
				 */
				if (lblk > 0) {
					--lblk;
				}
			}
		}
		blk = _head.lblk_to_blk(lblk);
		return blk;
	}

	/**
	 * @brief blockからclk/cnt指定で値取得
	 * @param type FrameType
	 * @param first 開始値
	 * @param last 終了値
	 * @param next 次有効データ
	 * @param tag clk_tag/cnt_tag
	 * @param extract RecFrameからAPIへの変換/DataSizeカウント
	 * @param tag clk/count
	 */
	template <class TAG, class Extract> void GetFrame(
	    FrameType type,
	    UINT64 first,
	    UINT64 last,
	    UINT64 *next,
	    TAG tag,
	    Extract extract)
	{
		UINT64 clk_hi;
		UINT64 clk_mid;
		UINT64 cnt;
		UINT64 clk = 0;
		UINT64 blknum;
		enum { SEARCH_START, SEARCH_NEXT, SEARCH_END } sts;

		AspConverter::_store_cnt = 0;
		if (AspConverter::_pexist_cnt) {
			*AspConverter::_pexist_cnt	= 0;
		}
		if (next) {
			*next = 0;
		}

		if (_head.get_blk_cnt() == 0) {
			return;
		}

		blknum = find_blk(type, first, tag);
		ReadBlock(blknum);
		_blk->head.GetClkCnt(type, clk_hi, clk_mid, cnt);

		sts = SEARCH_START;
		for (;;) {
			if (_blk->head.exist[type]) {
				/* block内にデータ存在 */
				cnt = _blk->head.cnt[type];
				_frame.Init();
				for (DWORD i = 0; i < _blk->head.data_size; ++i) {
					if (_frame.AddFpga(_blk->data[i]) == true) {
						if (type == _frame._type) {
							if (_frame._bclk_hi) {
								clk_hi = _frame._clk_hi;
							}
							if (_frame._bclk_mid) {
								clk_mid = _frame._clk_mid;
							}
							clk = clk_hi + clk_mid + _frame._clk_low;
							if (match(clk, cnt, first, last, tag)) {
								/* 見つかったらAPI構造体へ変換 */
								if (extract(_frame, clk) == false) {
									/* 変換先bufferがfullなら */
									SetNext(next, type, clk, cnt, tag);
									if (AspConverter::_pexist_cnt == 0) {
										sts = SEARCH_END;
										break;
									} else {
										sts = SEARCH_NEXT;
									}
								}
								if (AspConverter::_pexist_cnt) {
									++*AspConverter::_pexist_cnt;
								}
							}
							if (fin(clk, cnt, last, tag)) {
								SetNext(next, type, clk, cnt, tag);
								sts = SEARCH_END;
								break;
							}
							++cnt;
						}
						_frame.Init();
					}
				}
			}
			delete _blk;
			_blk = 0;
			if (sts == SEARCH_END) {
				break;
			}
			if (GetNextBlock(blknum) == true) {
				_blk->head.GetClkCnt(type, clk_hi, clk_mid, cnt);
			} else {
				sts = SEARCH_END;
				break;
			}
		}
	}

	/**
	 * @brief 記録ファイルへの記録開始
	 */
	bool AddFpgaStart(const char* fname);

	/**
	 * @brief Blockを領域確保してFPGA-FIFOの中身を格納
	 * @param pdwFpga FPGA data
	 * @param len DWORD単位のサイズ
	 */
	RFWASP_REC_BLOCK* AddFpgaData(const DWORD *pdwFpga, DWORD len);

	/**
	 * @brief FPGA-FIFOの追加終了/ファイルヘッダ書込み
	 */
	void AddFpgaFin(void);

	/**
	 * @brief 記録ファイル open
	 */
	bool Open(const char *path);

	/**
	 * @brief 記録ファイル close
	 */
	void Close();

	/**
	 * @brief 記録ファイルOpen?
	 */
	bool IsOpen() const
	{
		return _fio.IsOpen();
	}

	/**
	 * @brief 最大ブロックサイズ設定
	 * @param blksize 0:無制限
	 */
	void SetBlkSize(UINT64 blksize)
	{
		_head.max_blkcnt = blksize;
	}

	/**
	 * @brief
	 * 記録ファイル中の有効な時間/記録数を返す
	 */
	bool GetAvail(FrameType type, UINT64* pu64start, UINT64* pu64end, UINT64* pu64cnt);

private:
	std::string	fname;
	RFWASP_REC_HEAD _head;
	UINT64	_cnt[MAX_FRAME_TYPE];
	UINT64	_clk_hi[MAX_FRAME_TYPE];
	UINT64	_clk_mid[MAX_FRAME_TYPE];
	UINT64	_clk_low[MAX_FRAME_TYPE];
	UINT64	_pre_clk[MAX_FRAME_TYPE];
	UINT64	_first_clk[MAX_FRAME_TYPE];
	UINT64	_first_cnt[MAX_FRAME_TYPE];
	RfwAspFio	_fio;
	RFWASP_REC_BLOCK *_blk;
	RecFrame	_frame;
	bool	_valid_timestamp[MAX_FRAME_TYPE];
#if defined(RFWASP_THREAD)
#else
	RfwAspFio	_fsave;
#endif
	RfwAspFio	_fraw;

	/**
	 * 内部変数初期化
	 */
	void Init()
	{
		_head.Init();
		std::fill_n(_cnt, static_cast<size_t>(MAX_FRAME_TYPE), 0);
		std::fill_n(_clk_hi, static_cast<size_t>(MAX_FRAME_TYPE), 0);
		std::fill_n(_clk_mid, static_cast<size_t>(MAX_FRAME_TYPE), 0);
		std::fill_n(_clk_low, static_cast<size_t>(MAX_FRAME_TYPE), 0);
		std::fill_n(_pre_clk, static_cast<size_t>(MAX_FRAME_TYPE), 0);
		std::fill_n(_first_clk, static_cast<size_t>(MAX_FRAME_TYPE), 0);
		std::fill_n(_first_cnt, static_cast<size_t>(MAX_FRAME_TYPE), 0);
		std::fill_n(_valid_timestamp, static_cast<size_t>(MAX_FRAME_TYPE), false);
		_frame.Init();
	}

	/**
	 * @brief ブロックヘッダ読み込み
	 */
	void get_blk_head(UINT64 blk, RFWASP_REC_BLOCK_HEAD &head)
	{
		bool	eof;
		if (!_fio.Seek(sizeof(RFWASP_REC_HEAD) +
		               blk * sizeof(RFWASP_REC_BLOCK))) {
			throw asp_fseek_err(SRC_LOCATE);
		}
		if (!_fio.Read(&head, sizeof(RFWASP_REC_BLOCK_HEAD), eof)) {
			throw asp_fread_err(SRC_LOCATE);
		}
	}

	/**
	 * @brief ブロックヘッダから時間取得
	 */
	UINT64 get_blk_val(FrameType type, UINT64 blk, clk_tag)
	{
		RFWASP_REC_BLOCK_HEAD	head;
		get_blk_head(blk, head);
		return head.clk[type];
	}

	/**
	 * @brief ブロックヘッダからカウント取得
	 */
	UINT64 get_blk_val(FrameType type, UINT64 blk, cnt_tag)
	{
		RFWASP_REC_BLOCK_HEAD	head;
		get_blk_head(blk, head);
		return head.cnt[type];
	}

	/**
	 * @brief ブロックセーブ
	 */
	void SaveBlock();

	/**
	 * @brief 新規ブロック確保
	 */
	void NewBlock();

	/**
	 * @brief 次のblock読み込み
	 * @param blk 物理ブロック番号
	 * @return  最後のblockならばfalse
	 */
	bool GetNextBlock(UINT64 &blk);

	/**
	 * @brief 指定block read
	 * @param blk 物理ブロック番号
	 * @return RFWASP_REC_BLOCK*
	 */
	RFWASP_REC_BLOCK *ReadBlock(UINT64 blk);


#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4100)
#endif

	/**
	 * @brief 検索用helper clkでmatchするか
	 */
	bool match(UINT64 clk, UINT64 cnt, UINT64 first, UINT64 last, clk_tag)
	{
		if (clk >= first && clk <= last)
			return true;
		else
			return false;
	}

	/**
	 * @brief 検索用helper clkが最終か
	 */
	bool fin(UINT64 clk, UINT64 cnt, UINT64 last, clk_tag)
	{
		if (clk > last) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * @brief 検索用helper cntでmatchするか
	 */
	bool match(UINT64 clk, UINT64 cnt, UINT64 first, UINT64 last, cnt_tag)
	{
		if (cnt >= first && cnt <= last) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * @brief 検索用helper cntが最終か
	 */
	bool fin(UINT64 clk, UINT64 cnt, UINT64 last, cnt_tag)
	{
		if (cnt > last) {
			return true;
		} else {
			return false;
		}
	}

	void SetNext(UINT64 *next, FrameType type, UINT64 clk, UINT64 cnt, clk_tag)
	{
		if (next != 0 && *next == 0) {
			*next = clk_to_nsec(calib_clk(clk, type), OFF);
		}
	}

	void SetNext(UINT64 *next, FrameType type, UINT64 clk, UINT64 cnt, cnt_tag)
	{
		if (next != 0 && *next == 0) {
			*next = cnt;
		}
	}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
};

#endif
