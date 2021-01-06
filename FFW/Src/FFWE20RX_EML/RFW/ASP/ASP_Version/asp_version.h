///////////////////////////////////////////////////////////////////////////////
/**
 * @file asp_version.h
 * @brief E2拡張機能部バージョン定義ヘッダファイル
 * @author RSD S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/8/11 新規作成 S.Nagai
*/
#ifndef	__ASP_VERSION_H__
#define	__ASP_VERSION_H__

/////////// define定義 ///////////
// RFW ASP 共通部バージョン
#define RFW_ASP_COM_VERSION		0x10100002	// V.1.01.00.002
// E2拡張機能共通部(\RFW\ASP以下)を追加、修正した場合はこちらのバージョンを上げてください

// E2拡張機能固有部(\RFW\ASPRLや\RFW\ASPRHなど)を追加、修正した場合は
// 固有部ごとに以下のバージョンを上げてください
// RFA ASP RL固有部バージョン
#define RFW_ASP_RL_VERSION		0x10000002	// V.1.00.00.002

// RFA ASP RH固有部バージョン
#define RFW_ASP_RH_VERSION		0x10000007	// V.1.00.00.007

// RFA ASP RX固有部バージョン
#define RFW_ASP_RX_VERSION		0x00000001	// V.0.00.00.001

// RFA ASP ARM固有部バージョン
#define RFW_ASP_ARM_VERSION		0x00000001	// V.0.00.00.001(未開発)

#endif	// __ASP_VERSION_H__
