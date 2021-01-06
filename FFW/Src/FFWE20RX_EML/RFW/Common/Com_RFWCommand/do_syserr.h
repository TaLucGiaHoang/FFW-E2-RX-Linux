///////////////////////////////////////////////////////////////////////////////
/**
 * @file do_syserr.h
 * @brief システム関連コマンドのヘッダファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/09/09
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/09/09
*/
#ifndef	__DO_SYSERR_H__
#define	__DO_SYSERR_H__

#include "rfw_typedef.h"
#include "mcudef.h"


// define定義

// RFWERR_WARRING_BIT
#define ERROR_MAX				0xFFFFFFFF
#define RFWERR_WARRING_BIT		(0x10000000) // Warningビットの位置定義


// グローバル関数の宣言
extern DWORD SetCommErrNo(DWORD dwRfwErrNo);						///< 共通RFWエラー番号の登録
extern DWORD GetCommErrNo(void);									///< 共通RFWエラー番号の取得
extern DWORD GetCommErrType(void);									///< RFWエラーの種別取得
extern void GetCommErrMsg(DWORD dwRfwErrNo, BYTE byMsgLanguage, WCHAR* swzRfwErrMsg);	///< 共通RFWエラーメッセージの取得
extern DWORD SetTmpCoreErrNo(DWORD dwCoreInfo, DWORD dwRfwErrNo);	///< コア個別RFWエラー番号をテンポラリに登録
extern DWORD GetTmpCoreErrNo_WarningClr(DWORD dwCoreInfo);			///< 登録したエラー番号をテンポラリから取得(ワーニング置き換えあり)
extern DWORD GetTmpCoreErrNo(DWORD dwCoreInfo);						///< 登録したエラー番号をテンポラリから取得(ワーニング置き換えなし)
extern void SetCoreErrNo(DWORD dwCoreNum, const DWORD *pdwCoreInfo);///< テンポラリのエラー情報をエラー管理情報に反映
extern void ClrTmpCoreErrNo(void);									///< テンポラリのエラー情報をクリア
extern DWORD*  GetRfwErrNoCorePtr(void);							///< エラー番号格納変数(コア個別エラー格納用)ポインタの取得
extern DWORD*  GetRfwErrCoreInfoPtr(void);							///< エラー発生コア番号格納変数ポインタの取得
extern DWORD GetRfwErrNoCoreCnt(void);								///< RFWエラーの情報登録数を取得
extern void InitRfwCmdData_SysErr(void);							///< システムコマンド用RFW内部変数の初期化
extern void ClrWarning(void);										///< Warning発生した場合、内部変数にワーニング内容をクリア
extern DWORD WarningChk(void);										///< Warning発生したか確認
extern BOOL SetWarning(DWORD dwRet);								///< Warning発生した場合、内部変数にワーニング内容を保持

#endif	// __DO_SYSERR_H__
