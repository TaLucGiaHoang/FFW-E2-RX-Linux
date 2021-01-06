/**
 * @file rfw_bitops.h
 * @brief bit操作template
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015-2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/10/13
 */
#ifndef __rfw_bitops_h__
#define __rfw_bitops_h__

#include <limits.h>		/* CHAR_BIT */

#if defined(__GNUC__)
#include <inttypes.h>
typedef uint8_t		BYTE;
typedef uint16_t    WORD;
typedef uint32_t    DWORD;
typedef uint64_t    UINT64;
typedef int64_t     INT64;
#elif defined(_MSC_VER)
#include <tchar.h>
#include "windows.h"
#include <stdlib.h>		/* _byteswap_xx */
#include <intrin.h>		/* BitScanForward */
#pragma warning(disable:4351)
#endif

#if (CHAR_BIT == 8)
#define BITS_PER_BYTE	(CHAR_BIT)
#define BITS_PER_WORD	(BITS_PER_BYTE * sizeof(WORD))
#define BITS_PER_DWORD	(BITS_PER_BYTE * sizeof(DWORD))
#define BITS_PER_UINT64	(BITS_PER_BYTE * sizeof(UINT64))
#else
#error
#endif

/**
 * @brief  endian swap WORD
 * @param  x  WORD
 * @return WORD
 */
static inline WORD byte_swap(WORD x)
{
#if defined(__GNUC__)
	return __builtin_bswap16(x);
#elif defined(_MSC_VER)
	return _byteswap_ushort(x);
#else
	return (x << 8) | (x >> 8);
#endif
}

/**
 * @brief  endian swap DWORD
 * @param  x  DWORD
 * @return DWORD
 */
static inline DWORD byte_swap(DWORD x)
{
#if defined(__GNUC__)
	return __builtin_bswap32(x);
#elif defined(_MSC_VER)
	return _byteswap_ulong(x);
#else
	return
	    ((x << 24) & 0xff000000) |
	    ((x << 8)  & 0x00ff0000) |
	    ((x >> 24) & 0x000000ff) |
	    ((x >> 8)  & 0x0000ff00);
#endif
}

/**
 * @brief  endian swap UINT64
 * @param  x  UINT64
 * @return UINT64
 */
static inline UINT64 byte_swap(UINT64 x)
{
#if defined(__GNUC__)
	return __builtin_bswap64(x);
#elif defined(_MSC_VER)
	return _byteswap_uint64(x);
#else
	return
	    ((x << 56) & 0xff00000000000000ULL) |
	    ((x << 40) & 0x00ff000000000000ULL) |
	    ((x << 24) & 0x0000ff0000000000ULL) |
	    ((x << 8)  & 0x000000ff00000000ULL) |
	    ((x >> 56) & 0x00000000000000ffULL) |
	    ((x >> 40) & 0x000000000000ff00ULL) |
	    ((x >> 24) & 0x0000000000ff0000ULL) |
	    ((x >> 8)  & 0x00000000ff000000ULL);
#endif
}

/**
 *
 * @brief  find first bit
 * @param  x  DWORD
 * @return DWORD
 */
static inline DWORD find_first_bit(DWORD x)
{
#if defined(__GNUC__)
	if (x == 0)
		return 0;
	return __builtin_ctzl(x);
#elif defined(_MSC_VER)
	DWORD i;
	_BitScanForward(&i, x);
	return i;
#else
	DWORD  i;
	if (x == 0)
		return 0;
	for (i = 1; !(x & 1); i++)
		x >>= 1;
	return i;
#endif
}

/**
 * @brief  find first bit
 * @param  x  UINT64
 * @return DWORD
 */
static inline DWORD find_first_bit(UINT64 x)
{
#if defined(__GNUC__)
	if (x == 0)
		return 0;
	return __builtin_ctzll(x);
#elif defined(_MSC_VER)
#if defined(_WIN64)
	DWORD i;
	_BitScanForward64(&i, x);
	return i;
#else
	DWORD i;
	if (_BitScanForward(&i, static_cast<DWORD>(x)))
		return i;
	_BitScanForward(&i, static_cast<DWORD>(x>>32));
	return i + 32;
#endif
#else
	DWORD  i;
	if (x == 0)
		return 0;
	for (i = 1; !(x & 1); i++)
		x >>= 1;
	return i;
#endif
}

/**
 * @brief  population count
 * @param  x  DWORD
 * @return DWORD
 */
static inline DWORD population_count(DWORD x)
{
#if defined(__GNUC__)
	return __builtin_popcountl(x);
/* SSE4.2以上のCPUでないと使えない
#elif defined(_MSC_VER)
return _popcnt(x);
*/
#else
	x = (x & 0x55555555) + (x >> 1 & 0x55555555);
	x = (x & 0x33333333) + (x >> 2 & 0x33333333);
	x = (x & 0x0f0f0f0f) + (x >> 4 & 0x0f0f0f0f);
	x = (x & 0x00ff00ff) + (x >> 8 & 0x00ff00ff);
	return (x & 0x0000ffff) + (x >> 16 & 0x0000ffff);
#endif
}

/**
 * @brief  population count
 * @param  x  UINT64
 * @return DWORD
 */
static inline DWORD population_count(UINT64 x)
{
#if defined(__GNUC__)
	return __builtin_popcountll(x);
/* SSE4.2以上のCPUでないと使えない
#elif defined(_MSC_VER)
return _popcnt64(x);
*/
#else
	x = (x & 0x5555555555555555) + (x >> 1 & 0x5555555555555555);
	x = (x & 0x3333333333333333) + (x >> 2 & 0x3333333333333333);
	x = (x & 0x0f0f0f0f0f0f0f0f) + (x >> 4 & 0x0f0f0f0f0f0f0f0f);
	x = (x & 0x00ff00ff00ff00ff) + (x >> 8 & 0x00ff00ff00ff00ff);
	x = (x & 0x0000ffff0000ffff) + (x >> 16 & 0x0000ffff0000ffff);
	return (x & 0x00000000ffffffff) + (x >> 32 & 0x00000000ffffffff);
#endif
}

/**
 * @brief  make bit mask
 * @param  width  mask bit count
 * @return T
 */
template<class T> T mk_bit_mask(int width)
{
	return (T)(~0) >> (sizeof(T) * 8 - width);
}

/**
 * @brief  bit extract
 * @param  d  source value
 * @param  start_bit  first bit position
 * @param  bit_width  bit count
 * @return T
 */
template<class T, class U> T bit_extract(U d, int start_bit, int bit_width)
{
	return static_cast<T>((d & (mk_bit_mask<U>(bit_width) << start_bit)) >> start_bit);
}

/**
 * @brief  make bit pattern
 * @param  d  source value
 * @param  start_bit  first bit
 * @param  bit_width  bit count
 * @return T
 */
template<class T, class U> U bit_ptn(T d, int start_bit, int bit_width)
{
	return static_cast<T>((d & mk_bit_mask<T>(bit_width)) << start_bit);
}

/**
 * @brief  bit or
 * @param  d  destination
 * @param  x  or value
 * @param  start_bit  first bit position
 * @param  bit_width  bit count
 * @return distination
 */
template<class T, class U> U bit_insert(U &d, T x, int start_bit, int bit_width)
{
	d |= bit_ptn<T, U>(x, start_bit, bit_width);
	return d;
}

/**
 * @brief  bit replace
 * @param  d  destination
 * @prama  x  replace value
 * @param  start_bit  first bit position
 * @param  bit_width  bit count
 * @return distination
 */
template<class T, class U> U bit_replace(U &d, T x, int start_bit, int bit_width)
{
	d = (d & ~(mk_bit_mask<U>(bit_width) << start_bit))
	    | bit_ptn<T, U>(x, start_bit, bit_width);
	return d;
}

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

/* 40bit UINT64 */
typedef struct {
	DWORD	l;
	BYTE	h;
	typedef BYTE htype;
}
UINT64_40;

/* 48bit UINT64 */
typedef struct {
	DWORD	l;
	WORD	h;
	typedef WORD htype;
}
UINT64_48;

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

/**
 * @brief  UINT64 to UINT64_40/48
 * @param  val destination reference
 * @prama  u64val source value;
 * @return UINT64
 */
template <class T> UINT64 u64compress(T &val, UINT64 u64val)
{
	val.l = bit_extract<DWORD, UINT64>(u64val, 0, BITS_PER_DWORD);
	val.h = bit_extract<typename T::htype, UINT64>(u64val, BITS_PER_DWORD, sizeof(typename T::htype));
	return u64val;
}

/**
 * @brief  UINT64_40/48 to UINT64
 * @param  val source reference
 * @return UINT64
 */
template <class T> UINT64 u64decompress(const T &val)
{
	return static_cast<UINT64>(val.l)
	       + (static_cast<UINT64>(val.h) << BITS_PER_DWORD);
}

#endif
