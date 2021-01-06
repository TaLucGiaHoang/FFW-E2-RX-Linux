/**
 * @file rfw_fio.h
 * @brief ASP記録ファイルI/O
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015-2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/10/13
 */
#ifndef __RFWASP_FIO_H__
#define __RFWASP_FIO_H__

#include <stdio.h>
#include <string>
#include "rfw_bitops.h"


class RfwAspFio
{
public:
	explicit RfwAspFio();
	virtual ~RfwAspFio() throw();

	bool Open(const char *fname, bool bWrite, DWORD dwReadAhead = 0);
	bool Close(void);
	bool Seek(UINT64 off);
	bool Write(const void *p, DWORD dwSize);

	/*
	 * @brief 先読み付Read
	 * @param p read buffer
	 * @param dwSize read size
	 * @param beof eof flga
	 * @return read size
	 */
	DWORD ReadAhead(void *p, DWORD dwSize, bool &beof);

	/*
	 * @brief 先読みせずに直接Read
	 * @param p void*
	 * @param dwSize read size
	 * @param beof eof flag
	 * @return readsize
	 */
	DWORD Read(void *p, DWORD dwSize, bool &beof);

	bool IsOpen() const
	{
		return (_fp != 0);
	}

private:
	bool ReadAhead();
	DWORD AvailCount()
	{
		return _readahead - _pos;
	}
	std::string	_fname;
	FILE *_fp;
	DWORD	_readahead_max;
	DWORD	_readahead;
	DWORD	_pos;
	bool	_beof;
	BYTE *_data;
};

#endif

