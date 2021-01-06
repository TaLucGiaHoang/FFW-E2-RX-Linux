///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffw_typedef.h
 * @brief E1/E20エミュレータファー??ウェアの??ータ型定義ファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
///////////////////////////////////////////////////////////////////////////////
/*
??改定履歴
・ソース構???改??	2012/07/10 橋口
　　ベ???ス:FFWE20RX600.dll V.1.02.00.015
*/
#ifndef	__FFW_TYPEDEF_H__
#define	__FFW_TYPEDEF_H__

#ifdef _WIN32
#include "StdAfx.h"
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif


typedef	DWORD	FFWERR;	// FFWで定義して??るエラーを扱??変数に使用
typedef	WORD	BFWERR;	// BFWで定義して??るエラーを扱??変数に使用

typedef DWORD	EADDR;	// モニタCPUのアドレスを扱??変数に使用
typedef DWORD	MADDR;	// ターゲ????MCUのアドレスを扱??変数に使用

typedef volatile BYTE	VBYTE;	// 最適化禁止	:8ビットアクセスのみ
typedef volatile WORD	VWORD;	//			 	:16ビットアクセスのみ
typedef volatile DWORD	VDWORD;	//				:32ビットアクセスのみ

typedef DWORD	DCSFACC;	// dcsf_xx.hで定義するDCSFレジスタ、メモリのアドレスに使用(FFW用)
//typedef VDWORD*	DCSFACC;	// BFW用


#endif	// __FFW_TYPEDEF_H__
