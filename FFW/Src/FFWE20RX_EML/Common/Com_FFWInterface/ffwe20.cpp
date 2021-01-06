///////////////////////////////////////////////////////////////////////////////
/**
 * @file ffwe20.cpp
 * @brief E1/E20エミュレータ用FFWモジュール
 * @author RSO Y.Minami, K.Okita, H.Hashiguchi
 * @author Copyright (C) 2009(2010-2012) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2012/07/11
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevNo120201-001	2012/07/11 橋口
　・DllMain()
　　　DLL_PROCESS_DETACHの場合のDO_E20Trans()呼び出し削除
*/
#ifdef _WIN32
#include "StdAfx.h"
#elif __linux__
#include "ffw_linux.h"
#include "addition.h"
#endif
// V.1.02 RevNo110509-002 Appned Start
#include "prot_sys.h"
#include "do_sys.h"
// V.1.02 RevNo110509-002 Appned End



///////////////////////////////////////////////////////////////////////////////
// DLLエントリポイント 
///////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	printf("[%s] Microsoft function, not implemted\n", __func__);
	lpReserved;
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:	// no break
		SetHandle ( static_cast<HINSTANCE>(hModule) );
	case DLL_THREAD_ATTACH:		// no break
	case DLL_THREAD_DETACH:		// no break
		// V.1.02 RevNo110506-001 Append Line
		break;
	case DLL_PROCESS_DETACH:
		// V.1.02 RevNo110506-001 Append Line → RevNo110509-001 Delete Line
//		DeleteFlashRomCacheMem();

		// V.1.02 RevNo110509-002 Appned Line → RevNo120201-001 Delete Line
//		// FFWを解放する際にエミュレータBFWのレベルをレベル0に戻しておく(次回再起動時のため)
//		DO_E20Trans(TRANS_MODE_LV0);
		break;
	default:
		break;
	}

    return TRUE;
}
	
// モジュール(DLL)のハンドルの設定と参照
static HINSTANCE hInstanceDll = NULL;

void SetHandle(HINSTANCE hInstance)
{
	printf("[%s] Microsoft function, not implemted\n", __func__);
	hInstanceDll = hInstance;
}

HINSTANCE GetHandle()
{
	printf("[%s] Microsoft function, not implemted\n", __func__);
	return hInstanceDll;
}

