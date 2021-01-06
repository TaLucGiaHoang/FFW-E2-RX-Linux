///////////////////////////////////////////////////////////////////////////////
/**
 * @file asp_version.cpp
 * @brief E2拡張機能部バージョン取得
 * @author tssr ogata
 * @author Copyright (C) 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/3/29
 */
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <cstring>
#if defined(__GNUC__)
#include <stdint.h>
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t UINT64;
#endif
#include "mcudef.h"
#include "asp_version.h"
using namespace std;

#if defined(__GNUC__)
#define strcpy_s strcpy
#undef DECLARE
#define DECLARE(func) int func##_
#endif

/*
 * binaryファイルでversion文字列が見つけやすいようにrcsid形式で保存
 */
#define _STR(s)	#s
#define _RCSID(key, s)	"$" key ": " _STR(s) " $"
static const char* spComVer = _RCSID("AspCom", RFW_ASP_COM_VERSION);
#if defined(ASP_RH)
static const char* spFamilyVer = _RCSID("AspFamily", RFW_ASP_RH_VERSION);
static const DWORD sdwFamilyVer = RFW_ASP_RH_VERSION;
#elif defined(ASP_RL)
static const char* spFamilyVer = _RCSID("AspFamily", RFW_ASP_RL_VERSION);
static const DWORD sdwFamilyVer = RFW_ASP_RL_VERSION;
#else
#error
#endif

//=============================================================================
/**
 * "$keyword: 0xXXXXXXXX $"の文字列を"X.XX.XX.XXX"形式に変換
 * @param s 変換前文字列
 * @retval 変換後文字列
 */
//=============================================================================
string ToVerStr(const string &s)
{
	string ver = s.substr(s.find_first_of(" ", 0), s.find_last_of(" ", s.size()) - s.find_first_of(" ", 0));
	ver = ver.substr(3);
	ver.insert(1, ".");
	ver.insert(4, ".");
	ver.insert(7, ".");
	return ver;
}

//=============================================================================
/**
 * ASP共通部のversion文字列取得
 * @retval version文字列
 */
//=============================================================================
string GetAspComVer(void)
{
	return ToVerStr(string(spComVer));
}

//=============================================================================
/**
 * ASP固有部のversion文字列取得
 * @retval version文字列
 */
//=============================================================================
string GetAspFamilyVer(void)
{
	return ToVerStr(string(spFamilyVer));
}

//=============================================================================
/**
 * ASP versionの取得
 * @param pAspVer
 * @retval RFWERR_OK
 */
//=============================================================================
DECLARE(RFWASP_GetVersion)(RFW_ASPVERSION_DATA* pVer)
{
	int rc = RFWERR_N_RFW_ARG;
	if (pVer) {
		strcpy_s(pVer->szComVer, GetAspComVer().c_str());
		pVer->dwComVer = RFW_ASP_COM_VERSION;
		strcpy_s(pVer->szFamilyVer, GetAspFamilyVer().c_str());
		pVer->dwFamilyVer = sdwFamilyVer;
		rc = RFWERR_OK;
	}
	return rc;
}
