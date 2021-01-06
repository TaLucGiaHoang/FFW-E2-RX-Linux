////////////////////////////////////////////////////////////////////////////////
/**
 * @file mcurx_tra.h
 * @brief RX共通トレース関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda, K.Okita
 * @author Copyright (C) 2009(2010-2013) Renesas Electronics Corporation and
 * @author Renesas Solutions Corp. All rights reserved.
 * @date 2014/06/19
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo140515-005 2014/06/19 大喜多
	RX71M対応
*/
#ifndef	__MCURX_TRA_H__
#define	__MCURX_TRA_H__


#include "ffw_typedef.h"
#include "mcudef.h"

// 定数定義
//RevNo100715-014 Append Start
#define TRC_SET_OCDREGNUM		10	// RevRxNo130301-001 Modify Line
// V.1.02 No.31 Class2 トレース対応 Append Line
#define TRC_SET_CLASS2_OCDREGNUM 4

#define BUF_RM_TRC128_H				0x00085800

//SCKCR
#define FPGA_ICK_VAL				0x00000003

// RevRxNo140515-005 Append Start
#define MCU_REG_SYSTEM_MACJCR						0x00080200		// 電流ジャンプ対策制御レジスタ
// RevRxNo140515-005 Append End



// 内蔵トレースヘッダ情報
#define TRCRM_HDR_TDV			0x00000001
#define TRCRM_HDR_LOST			0x00000002
#define TRCRM_HDR_MODE			0x00000004
#define TRCRM_HDR_BTR			0x00000008
#define TRCRM_HDR_CMD1			0x000000F0
#define TRCRM_HDR_CMD2			0x00000F00
#define TRCRM_HDR_TIME			0xFFFFF000

#define TRCRM_HDR_CLS2_RW		0x00000001
#define TRCRM_HDR_CLS2_ATTR		0x00000006
	#define TRCRM_HDR_CLS2_ATTR_TIME	0x00000006
#define TRCRM_HDR_CLS2_CMD		0x00000030

#define TRCRM_HDR_CLS2_BTR3		0x00000002
#define TRCRM_HDR_CLS2_BTR2		0x00000004

#define TRCRM_HDR_CLS2_DTR1		0x00000002
#define TRCRM_HDR_CLS2_DTR2		0x00000004
#define TRCRM_HDR_CLS2_DTR4		0x00000006
#define TRCRM_HDR_CLS2_DTW1		0x00000003
#define TRCRM_HDR_CLS2_DTW2		0x00000005
#define TRCRM_HDR_CLS2_DTW4		0x00000007

#define TRCRM_HDR_CLS2_CH1		0x00000010
#define TRCRM_HDR_CLS2_CH2		0x00000020

//外部ダミーパケット出力時のセットデータ
#define DUMMY_BTR_HEADER			0x00000071
#define DUMMY_BTR_DSTADDR			0x00000000
#define DUMMY_BTR_SRCADDR			0x00000000
#define DUMMY_DTW_HEADER			0x000000C1
#define DUMMY_DTW_ADDRESS			0x00084000
#define DUMMY_DTW_DATA				0x00000000
#define DUMMY_DTW_ADDRESS_00000000H	0x00000000
#define DUMMY_DTW_DATA_00000000H	0x00000023
#define DUMMY_DTW_ADDRESS_FFFFFFFFH	0xFFFFFFFF

//クリア時の初期値
#define INIT_OCD_REG_EV_EVETSPCP			0		//トレース開始実行PCイベント選択
#define INIT_OCD_REG_EV_EVETSOP				0		//トレース開始オペランドイベント選択
#define INIT_OCD_REG_EV_EVETEPCP			0		//トレース終了実行PCイベント選択
#define INIT_OCD_REG_EV_EVETEOP				0		//トレース終了オペランドイベント選択
#define INIT_OCD_REG_EV_EVETXOP				0		//トレース抽出オペランドイベント選択
#define INIT_OCD_REG_EV_EVETTRG				0		//トレーストリガ出力

#define INIT_OCD_REG_RM_TRCR				0x00030000		//トレースコントロールレジスタ
#define INIT_OCD_REG_RM_TACR				0				//トレース動作中コントロールレジスタ
#define INIT_OCD_REG_RM_TRCS				0				//トレース圧縮方式設定レジスタ
#define INIT_OCD_REG_RM_TISR				0				//トレース命令選別設定レジスタ
#define INIT_OCD_REG_RM_RSAR				0xFE000000		//ROM空間先頭アドレスレジスタ
#define INIT_OCD_REG_RM_REAR				0xFFFFFFFF		//ROM空間終了アドレスレジスタ
#define INIT_OCD_REG_RM_WSAR				0x00000000		//ウィンドウトレース先頭アドレスレジスタ
#define INIT_OCD_REG_RM_WEAR				0xFFFFFFFF		//ウィンドウトレース終了アドレスレジスタ
#define INIT_OCD_REG_RM_TSSR				0x00000001		//トレースソース選択レジスタ	// RevRxNo130301-001 Append Line

// プロトタイプ宣言
extern FFWERR ClrTrcOCDReg(void);
extern FFWERR ClrTrcOcdMemCls3(void);
extern FFWERR SetTrcOcdData(void);
// V.1.02 No.31 Class2 トレース対応 Append Start
extern FFWERR SetTrcCls2OcdData(void);
extern FFWERR GetTrcOcdMemInputInfo(DWORD* pdwTrstp);
extern FFWERR ClrTrcOcdMemCls2(void);

extern FFWERR SetTrcOcdRst(void);
extern FFWERR SetTrcOcdStp(enum FFWENM_TRSTP eTrstp);
extern FFWERR CheckTrcOcdStop(void);

extern FFWERR CheckTrcClkFrq(DWORD dwTrcfrq, DWORD* pdwTrfs);

extern FFWERR SetTrcOCDDummyDataBTR1(void);
extern FFWERR ClrTOIBit(void);

//内蔵トレース用
extern FFWERR GetAllTrcBuffDataTrbf(DWORD* pdwRd,DWORD dwRdCount);
extern FFWERR OpenTrcBuff(DWORD dwTbsr);
extern FFWERR GetTrcOcdOffFlg(BOOL* pbOFF);
extern FFWERR GetTrcOcdMemInptPntCls2(DWORD* pdwTbip);

extern void InitMcuRxData_Tra(void);	///< トレース関連コマンド用FFW内部変数の初期化

#endif	// __MCURX_TRA_H__
