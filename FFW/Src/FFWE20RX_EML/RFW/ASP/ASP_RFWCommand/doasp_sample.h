///////////////////////////////////////////////////////////////////////////////
/**
 * @file doasp_sample.h
 * @brief データ取得関連のヘッダファイル
 * @author TSSR M.Ogata
 * @author Copyright (C) 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2017/02/07 新規作成
*/
#ifndef	__DOASP_SAMPLE_H__
#define	__DOASP_SAMPLE_H__

#include "rfw_typedef.h"
#include "mcudef.h"

// define定義


// グローバル関数の宣言
#ifdef	__cplusplus
extern	"C" {
#endif
extern DWORD DO_AspIn(DWORD dwReadByte, DWORD *const pbyReadBuff);
extern DWORD DO_AspSetPort(BYTE byUsbRdyN, BYTE byBfwRdyN);
extern DWORD DO_AspGetSample(
    DWORD dwType,
    DWORD dwKeyType,
    UINT64 u64Start,
    UINT64 u64End,
    void *pData,
    DWORD dwSize,
    DWORD *pdwStoreCnt,
    DWORD *pdwExistCnt,
    UINT64 *pu64Read);
extern DWORD DO_AspSetSaveFileName(const char *fname);
extern DWORD DO_AspSaveFile(void);
extern DWORD DO_AspLoadSample(const char *fname);
extern DWORD DO_AspUnLoadSample(void);
extern DWORD AspSramSave(DWORD dwSrambyte);
extern DWORD DO_GetAvailableTime(DWORD dwType, UINT64 *const pu64Start, UINT64 *const pu64End, UINT64 *const pu64Sample);
extern DWORD DO_AspGetSampleDataSize(
    DWORD dwType,
    DWORD dwKeyType,
    UINT64 u64Start,
    UINT64 u64End,
    UINT64 *pu64DataSize);
extern DWORD DO_GetSampleState(DWORD *const pdwState, UINT64 *const pu64Num, UINT64 *const pu64Max);
extern DWORD DO_SetClkCalibration(DWORD dwType, INT64 i64Calib);
extern DWORD DO_AspAddRaw(const char* rawfile, const char* datfile);
extern void DO_ClrAspSaveFileName(void);
#ifdef	__cplusplus
}
#endif

#endif	// __DOASP_SAMPLE_H__
