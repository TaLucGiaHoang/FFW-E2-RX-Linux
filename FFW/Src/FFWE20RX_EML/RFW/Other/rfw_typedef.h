///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfw_typedef.h
 * @brief E2エミュレータファームウェアのデータ型定義ファイル
 * @author RSO Y.Minami, H.Hashiguchi, REL K.Sasaki
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/31
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・2017/10/31 佐々木(広)
	RX用FFWの型定義と被っていたため、define値(ASP_RX)でインクルード対象
	を切り替えるように修正
*/
#ifndef	__RFW_TYPEDEF_H__
#define	__RFW_TYPEDEF_H__

#ifdef _WIN32
#include    <windows.h>
//#include "StdAfx.h"
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif
typedef	DWORD	RFWERR;	// FFWで定義しているエラーを扱う変数に使用
typedef volatile UINT64	VUINT64;	// 最適化禁止	:64ビットアクセスのみ

#ifndef	ASP_RX
typedef	DWORD	FFWERR;	// FFWで定義しているエラーを扱う変数に使用
typedef	WORD	BFWERR;	// BFWで定義しているエラーを扱う変数に使用

typedef DWORD	EADDR;	// 制御CPUのアドレスを扱う変数に使用
#if ASP_ARM
typedef UINT64	MADDR;	// ターゲットMCUのアドレスを扱う変数に使用
#endif
typedef volatile BYTE	VBYTE;		// 最適化禁止	:8ビットアクセスのみ
typedef volatile WORD	VWORD;		// 最適化禁止	:16ビットアクセスのみ
typedef volatile DWORD	VDWORD;		// 最適化禁止	:32ビットアクセスのみ

typedef DWORD	DCSFACC;	// dcsf_xx.hで定義するDCSFレジスタ、メモリのアドレスに使用(FFW用)
//typedef VDWORD*	DCSFACC;	// BFW用
#endif	// ASP_RX

#endif	// __RFW_TYPEDEF_H__
