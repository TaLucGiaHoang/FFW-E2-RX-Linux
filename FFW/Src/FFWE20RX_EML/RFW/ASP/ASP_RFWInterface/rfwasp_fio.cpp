///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfw_fio.cpp
 * @brief ASP記録ファイルI/O
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <algorithm>
#include "rfwasp_fio.h"

//=============================================================================
/**
 * Constructor
 */
//=============================================================================
RfwAspFio::RfwAspFio() : _fp(0), _readahead_max(0), _readahead(0), _pos(0), _beof(false), _data(0)
{
}

//=============================================================================
/**
 * Destructor
 */
//=============================================================================
RfwAspFio::~RfwAspFio() throw()
{
	Close();
	delete [] _data;
}

//=============================================================================
/**
 * ファイルオープン
 * @param fname ファイルパス
 * @param bWrite WriteOpenフラグ
 * @param dwReadAhead readaheadサイズ
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspFio::Open(const char *fname, bool bWrite, DWORD dwReadAhead)
{
	if (bWrite == false) {
		_readahead_max = dwReadAhead;
		try {
			_data = new BYTE[dwReadAhead];
		} catch (std::bad_alloc) {
			return false;
		}
	}

	_fname = fname;
#if defined(_MSC_VER)
	errno_t err = fopen_s(&_fp, fname, (bWrite) ? "wb" : "rb");
	if (err != 0)
		return false;
	else
		return true;
#else
	_fp = fopen(fname, (bWrite) ? "wb" : "rb");
	if (_fp != 0) {
		return true;
	} else {
		return false;
	}
#endif
}

//=============================================================================
/**
 * ファイルクローズ
 * @param なし
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspFio::Close(void)
{
	bool rc = true;
	if (_fp) {
		if (fclose(_fp) != 0) {
			rc = false;
		};
	}
	_fp = 0;
	return rc;
}

//=============================================================================
/**
 * ファイルシーク
 * @param off ファイル先頭からのオフセット
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspFio::Seek(UINT64 off)
{
	int	rc;

	if (!_fp) {
		return false;
	}
	_readahead = _pos = 0;
#if defined(__GNUC__)
	rc = fseeko(_fp, off, SEEK_SET);
#elif defined(_MSC_VER)
	rc = _fseeki64(_fp, off, SEEK_SET);
#endif
	return rc == 0;
}

//=============================================================================
/**
 * ファイルライト
 * @param p 書込みバッファ
 * @param dwSize 書込みサイズ
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspFio::Write(const void *p, DWORD dwSize)
{
	DWORD	rc;
	if (!_fp) {
		return false;
	}

	rc = fwrite(p, 1, dwSize, _fp);
	if (rc != dwSize) {
		return false;
	}
	return true;
}

//=============================================================================
/**
 * ファイル先読み
 * @param なし
 * @retval true 成功
 * @retval false 失敗
 */
//=============================================================================
bool RfwAspFio::ReadAhead()
{
	size_t	sz;

	if (!_fp) {
		return false;
	}

	_pos = 0;
	sz = fread(_data, 1, _readahead_max, _fp);
	if (sz != _readahead_max) {
		if (ferror(_fp)) {
			_readahead = 0;
			return false;
		}
		if (feof(_fp)) {
			_beof = true;
		} else {
			_beof = false;
		}
	}
	_readahead = sz;
	return true;
}

//=============================================================================
/**
 * ファイルリード(先読み付き)
 * @param p	readバッファ
 * @param swSize readサイズ
 * @param beof EOF状態
 * @return readサイズ
 */
//=============================================================================
DWORD RfwAspFio::ReadAhead(void *p, DWORD dwSize, bool &beof)
{
	DWORD	copy_sz = 0;
	DWORD	remain_sz = 0;
	if (!_fp) {
		return 0;
	}

	if (AvailCount() > dwSize) {
		copy_sz = dwSize;
		::memcpy(p, &_data[_pos], copy_sz);
		_pos += copy_sz;
	} else {
		if (AvailCount() > 0) {
			copy_sz = AvailCount();
			::memcpy(p, &_data[_pos], copy_sz);
			dwSize -= copy_sz;
		}
		if (ReadAhead() == false) {
			beof = _beof;
			return copy_sz;
		}
#ifdef _MSC_VER
#undef min
#endif
		remain_sz = std::min(dwSize, AvailCount());
		::memcpy((char*)p + copy_sz, &_data[_pos], remain_sz);
		_pos += remain_sz;
	}
	beof = _beof;
	return copy_sz + remain_sz;
}

//=============================================================================
/**
 * ファイルリード
 * @param p	readバッファ
 * @param swSize readサイズ
 * @param beof EOF状態
 * @return readサイズ
 */
//=============================================================================
DWORD RfwAspFio::Read(void *p, DWORD dwSize, bool &beof)
{
	size_t	sz;

	if (!_fp) {
		return 0;
	}

	sz = fread(p, 1, dwSize, _fp);
	if (feof(_fp)) {
		_beof = true;
	} else {
		_beof = false;
	}
	beof = _beof;
	return sz;
}

