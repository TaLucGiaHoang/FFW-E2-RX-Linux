///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_rec.h
 * @brief ASP記録ファイルフレーム
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef __RFWASP_REC_FRAME_H__
#define __RFWASP_REC_FRAME_H__

#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include "rfw_bitops.h"
#include "rfwasp_fio.h"
#include "rfwasp_recdata.h"

inline std::string src_locate(const char* file, int line)
{
	std::ostringstream s;
#if defined(_MSC_VER)
	std::string fullpath(file);
	std::string basename;
	std::string::size_type pos = fullpath.find_last_of("\\");
	if (pos == std::string::npos) {
		basename = fullpath;
	} else {
		basename = fullpath.substr(pos + 1);
	}
	s << basename << ":" << line;
#else
	s << file << ":" << line;
#endif
	return s.str();
}

#define SRC_LOCATE	src_locate(__FILE__, __LINE__)

/*
 * ASPデータのFrameType
 */
enum FrameType {
	SRAM_OVF	= 0x00,
	IO_CH0_TX	= 0x01,
	IO_CH0_RX	= 0x02,
	IO_CH1_TX	= 0x03,
	IO_CH1_RX	= 0x04,
	POWER		= 0x05,
	EVL			= 0x06,
	PC_MEM		= 0x07,
	SPECIFIC_8	= 0x08,
	SPECIFIC_9	= 0x09,
	SPECIFIC_A	= 0x0a,
	SPECIFIC_B	= 0x0b,
	SPECIFIC_C	= 0x0c,
	SPECIFIC_D	= 0x0d,
	SPECIFIC_E	= 0x0e,
	DUMMY		= 0x0f,
	MAX_FRAME_TYPE
};

/*
 * FrameTypeのIO_CH0_TX/RX IO_CH1_TX/RXは
 * ロジアナ/UART/I2C/SPI/CANで共用になっている
 * どれに対応するかの定義
 */
enum FrameIOType {
	IO_LOGIC_ANALYZER,
	IO_UART,
	IO_I2C,
	IO_SPI,
	IO_CAN
};

static inline UINT64 calib_clk(UINT64 clk, INT64 calib)
{
	UINT64	cclk;
	cclk = clk + calib;
	/*
	 * wraparround check
	 * FPGAのクロックが48bitのため+方向のwarparroundは発生しない
	 *
	 */
	if (calib < 0) {
		if (cclk > clk) {
			cclk = 0;
		}
	}
	return cclk;
}

const UINT64 max_clk = 0xffffffffffff;
const UINT64 max_nsec = (max_clk * 100) / 12;
enum ROUNDING {
	OFF,	// 四捨五入
	UP,		// 切上げ
	DOWN	// 切捨て
};

static inline UINT64 clk_to_nsec(UINT64 clk, ROUNDING r = OFF)
{
	if (clk > max_clk) {
		return max_nsec;
	} else {
		/* clk * 1000 / 120 */
		UINT64 off = 0;
		switch (r) {
		case OFF: 	off = 6;	break;
		case UP:	off = 11;	break;
		case DOWN:	off = 0;	break;
		}
		return (clk * 100 + off) / 12;
	}
}

static inline UINT64 nsec_to_clk(UINT64 nsec, ROUNDING r = OFF)
{
	if (nsec > max_nsec) {
		return max_clk;
	} else {
		/* nsec * 120 / 1000 */
		UINT64 off = 0;
		switch (r) {
		case OFF: 	off = 50; 	break;
		case UP:	off = 99;	break;
		case DOWN:	off = 0;	break;
		}
		return (nsec * 12 + off) / 100;
	}
}

static inline UINT64 nsec_to_clk_start(UINT64 nsec)
{
	/* 開始側nsec補正 */
	UINT64 clk = nsec_to_clk(nsec, OFF);
	UINT64 nsec2 = clk_to_nsec(clk, OFF);
	if (nsec2 < nsec) {
		return clk + 1;
	} else {
		return clk;
	}
}

static inline UINT64 nsec_to_clk_end(UINT64 nsec)
{
	/* 終了側nsec補正 */
	UINT64 clk = nsec_to_clk(nsec, OFF);
	UINT64 nsec2 = clk_to_nsec(clk, OFF);
	if (nsec2 > nsec) {
		return clk - 1;
	} else {
		return clk;
	}
}

/**
 * @brief ダミーフレームか?
 */
static inline bool IsDummyFrame(DWORD dwFpga)
{
	return (dwFpga == 0xffffffff);
}

/**
 * @brief スタートフレームか?
 */
static inline bool IsStartFrame(DWORD dwFpga)
{
	return (dwFpga & 0x1);
}

/**
 * @brief フレームタイプ取得
 */
static inline FrameType GetFrameType(DWORD dwFpga)
{
	return static_cast<FrameType>(bit_extract<BYTE, DWORD>(dwFpga, 4, 4));
}

/**
 * @brief フレームサイズ取得
 */
static inline int GetFrameSize(DWORD dwFpga, bool &except)
{
	DWORD szbit = bit_extract<DWORD, DWORD>(dwFpga, 1, 3);
	except = (szbit == 7) ? true : false;
	return
	    (szbit == 0) ? 0 :
	    (szbit == 1) ? 1 :
	    (szbit == 2) ? 2 :
	    (szbit == 3) ? 3 :
	    (szbit == 4) ? 5 :
	    (szbit == 5) ? 8 :
	    (szbit == 7) ? 1 : 	/* 例外フレーム */
	    -1;
}

/**
 * @brief グローバルタイムスタンプHIか?
 */
static inline bool IsHiClkFrame(FrameType type, DWORD fpga)
{
	return
	    bit_extract<WORD, DWORD>(fpga, 0, 12) == ((static_cast<WORD>(type) << 4) + 0x101);
}

/**
 * @brief グローバルタイムスタンプMiddleか?
 * @note グローバルタイムスタンプHIが既にあればStartBitは立たない
 */
static inline bool IsMidClkFrame(FrameType type, DWORD fpga, bool bClkHi)
{
	return
	    bit_extract<WORD, DWORD>(fpga, 0, 12) == ((static_cast<WORD>(type) << 4) + ((bClkHi) ? 0x0 : 0x1));
}

/**
 * @brief グローバルタイムスタンプHiをUINT64にして取得
 */
static inline UINT64 GetHiClk(DWORD fpga)
{
	return bit_extract<UINT64, DWORD>(fpga, 12, 20) << 28;
}

/**
 * @brief グローバルタイムスタンプMiddleをUINT64にして取得
 */
static inline UINT64 GetMidClk(DWORD fpga)
{
	return bit_extract<UINT64, DWORD>(fpga, 12, 20) << 8;
}

/**
 * @brief タイムスタンプlowをUINT64にして取得
 */
static inline UINT64 GetLowClk(DWORD fpga)
{
	return bit_extract<UINT64, DWORD>(fpga, 8, 8);
}

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif


/**
 * FPGAデータのフレーム
 *  グローバルタイムスタンプがある場合はそれも含んで1フレーム
 */
struct RecFrame {
	std::vector<DWORD> _rec;	/**< 生のFPGAデータをendian変換したもの */
	int			_start_pos;		/**< 生のFPGAデータでデータが始まる位置(グローバルタイムスタンプの有無で位置変わる) */
	FrameType	_type;			/**< frametype */
	UINT64		_clk_hi;		/**< グローバルタイムスタンプHIがある場合 */
	UINT64		_clk_mid;		/**< グローバルタイムスタンプMIDがある場合 */
	UINT64		_clk_low;		/**< クロックLow,常に存在する */
	int			_remain_sz;		/**< 有効Frame完成で0 */
	bool		_bstart;		/**< StartBit有 */
	bool		_bclk;			/**< 有効Frame完成でfalse */
	bool		_bclk_hi;		/**< グローバルタイムスタンプHIが有効? */
	bool		_bclk_mid;		/**< グローバルタイムスタンプMIDが有効? */
	bool		_bsfifo_ovf;	/**< SFIFO OVF */
	static INT64	_calib[MAX_FRAME_TYPE]; /**< clk補正値 */

	RecFrame();

	/**
	 * グローバルタイムスタンプを抜いた実際のデータ位置のデータを返す
	 */
	DWORD operator[](int n) const
	{
		return _rec[_start_pos + n];
	}

	static bool disp(RecFrame &rec, UINT64 clk);

	/**
	 * ASP FIFOをDWORD単位で受け取って有効な形にまとめる
	 * @return true 有効なframe完成 false frame未完成
	 */
	bool AddFpga(DWORD fpga);

	void Init();
	void Dump();
	void ToRaw(UINT64 off, UINT64 cnt, UINT64 clk);

};

static inline UINT64 calib_clk(UINT64 clk, FrameType type)
{
	return calib_clk(clk, RecFrame::_calib[type]);
}


#endif
