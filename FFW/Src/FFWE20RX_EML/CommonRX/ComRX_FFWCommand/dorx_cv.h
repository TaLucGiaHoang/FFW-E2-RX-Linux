////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_cv.h
 * @brief カバレッジ計測関連コマンドのヘッダファイル
 * @author RSO Y.Miyake, S.Ueda
 * @author Copyright (C) 2013(2014) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/06/27
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・RevRxNo130308-001	2013/06/03 三宅
　カバレッジ開発対応
・RevRxNo140627-001	2014/06/27 上田
　分岐アドレス差分出力指定修正
*/
#ifndef	__DORX_CV_H__
#define	__DORX_CV_H__


#include "ffw_typedef.h"
#include "mcudef.h"
#include "ffwrx_cv.h"


//カバレッジ機能時のトレースモードの設定処理
#define CVM_SETMODE_REALTIME		0x04000000
#define CVM_SETMODE_TRCFULL			0x05000001
#define CVM_SETINITIAL_DATA4_SYNC1	0x00010301
#define CVM_SETINITIAL_DATA8_SYNC2	0x00011301
#define CVM_SETINITIAL_DATA8_SYNC1	0x00014301
#define CVM_SETINFO_RATIO_1_1		0x1002C000	// RevRxNo140627-001 Modify Line
#define CVM_SETINFO_RATIO_2_1		0x1082C000	// RevRxNo140627-001 Modify Line
#define CVM_SETINFO_RATIO_4_1		0x1102C000	// RevRxNo140627-001 Modify Line
#define CVM_TRCSRCSEL_CPU_SEL		0x00000001	// RevRxNo130308-001-026 Append Line
#define CVM_DEFAULT					0x00000000
#define CVM_DEFAULT_FFFFFFFF		0xFFFFFFFF
#define CVM_BIT_31_28_0MASK			0x0FFFFFFF
// RevRxNo130308-001-022 Append Start
#define CVM_BIT_3_0_1MASK			0x0000000F	// dwSetTBWのTBWビットマスク用
#define CVM_BIT_0_0_1MASK			0x00000001	// dwSetTRMのTRMビットマスク用
#define CVM_BIT_2_0_1MASK			0x00000007	// dwSetTRCのTRCビットマスク用
// RevRxNo130308-001-022 Append End
// RevRxNo130308-001-011 Append Line
#define CVM_BIT_15_12_1MASK			0x0000F000
//C0カバレッジデータの取得処理
#define CVD0_TRACE_MEM_BLOCK_SIZE	0x00800000
#define CVD0_1K_SIFT				10			// 1k単位なので、1kで割るための、10ビット右シフト用
#define CVD0_MCU_BLOCK_REMAINDER	0x000001FF	// マイコンアドレスで512単位での余り生成用

// プロトタイプ宣言（コマンド処理部）
extern FFWERR DO_SetRXCVM(const FFWRX_CVM_DATA* pCoverageMode);
extern FFWERR DO_SetRXCVB(DWORD dwSetBlk, const FFWRX_CVB_DATA* pCvbData);
extern FFWERR DO_GetRXCVD0(enum FFWRXENM_CV_BLKNO eBlkNo, DWORD dwStart, DWORD dwEnd, DWORD dwCvData[]);
extern FFWERR DO_ClrRXCVD(void);
extern FFWERR ClrCvReg(void);
extern void InitFfwCmdMcuData_Cv(void);	///< カバレッジ計測関連コマンド用FFW内部変数の初期化
// RevRxNo130308-001-007 Append Line
extern FFWERR SetGoTrace_CV(void);


#endif	// __DORX_CV_H__
