////////////////////////////////////////////////////////////////////////////////
/**
 * @file do_common.cpp
 * @brief FFW 共通関数 ソースファイル
 * @author RSO Y.Minami, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/10
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
*/

#include "do_common.h"

//==============================================================================
/**
 * 4Byte データのエンディアン変換を行う。
 * @param dwConv エンディアン変換対象データ
 * @retval エンディアン変換後のデータ
 */
//==============================================================================
DWORD ConvEndian_L(DWORD dwConv)
{

	DWORD		dwData;
	int			i;
	DWORD		dwConvMask[sizeof(DWORD)] = { CONV_ENDIAN_L_BIT0, CONV_ENDIAN_L_BIT1, 
											CONV_ENDIAN_L_BIT2, CONV_ENDIAN_L_BIT3 };

	dwData = 0;
	for (i = 0; i < sizeof(DWORD); ++i) {
		switch (i) {
		case 0:
			dwData |= ((dwConv & dwConvMask[i]) << 24) & dwConvMask[sizeof(DWORD) - i - 1];
			break;

		case 1:
			dwData |= ((dwConv & dwConvMask[i]) << 8) & dwConvMask[sizeof(DWORD) - i - 1];
			break;

		case 2:
			dwData |= ((dwConv & dwConvMask[i]) >> 8) & dwConvMask[sizeof(DWORD) - i - 1];
			break;

		case 3:
			dwData |= ((dwConv & dwConvMask[i]) >> 24) & dwConvMask[sizeof(DWORD) - i - 1];
			break;

		default:		// ここを通らない
			break;
		}
	}

	return dwData;

}


//==============================================================================
/**
 * 2Byte データのエンディアン変換を行う。
 * @param dwConv エンディアン変換対象データ
 * @retval エンディアン変換後のデータ
 */
//==============================================================================
WORD ConvEndian_W(WORD wConv)
{

	WORD		wData;
	int			i;
	WORD		wConvMask[sizeof(WORD)] = { CONV_ENDIAN_W_BIT0, CONV_ENDIAN_W_BIT1 };

	wData = 0;
	for (i = 0; i < sizeof(WORD); ++i) {
		switch (i) {
		case 0:
			wData |= ((wConv & wConvMask[i]) << 8) & wConvMask[sizeof(WORD) - i - 1];
			break;

		case 1:
			wData |= ((wConv & wConvMask[i]) >> 8) & wConvMask[sizeof(WORD) - i - 1];
			break;

		default:		// ここを通らない
			break;
		}
	}

	return wData;

}


//=============================================================================
/**
 * FFW 共通関数用変数初期化
 * @param なし
 * @retval なし
 */
//=============================================================================
void InitFfwCmdData_Common(void)
{
	return;
}
