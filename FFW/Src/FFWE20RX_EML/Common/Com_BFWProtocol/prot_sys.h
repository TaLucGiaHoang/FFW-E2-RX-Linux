///////////////////////////////////////////////////////////////////////////////
/**
 * @file prot_sys.h
 * @brief BFWコマンド プロトコル生成関数(システム関連)ヘッダファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, S.Ueda
 * @author Copyright (C) 2009(2010-2014) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2014/11/25
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxE2LNo141104-001 2014/11/25 上田
	E2 Lite対応
	E1/E20 レベル0 旧バージョン(MP版以外)用関数削除。
*/
#ifndef	__PROT_SYS_H__
#define	__PROT_SYS_H__

#include "ffw_typedef.h"
#include "mcudef.h"
// #include "do_sys.h"

/////////// define定義 ///////////
// RevRxE2LNo141104-001 Append Line
#define EINF_MCUID_CODE_E2	0x20	// EINFコマンド EML, PRGターゲットMCU IDコードの未使用データ格納値(空白文字)(E2/E2 Lite用)

#define BFWCMD_MONP_SEND	0x0000	// BFWCmd_MONPのMONP終了コード種別 (実行2)
#define BFWCMD_MONP_CLOSE	0x0001	// BFWCmd_MONPのMONP終了コード種別 (実行3)
#define BFWCMD_DCNF_OPEN	0x0000	// BFWCmd_DCNFのDCNF終了コード種別 (実行1)
#define BFWCMD_DCNF_SEND	0x0001	// BFWCmd_DCNFのDCNF終了コード種別 (実行2)
// 2009.06.22 INSERT_BEGIN_E20RX600(+2) {
#define BFWCMD_SETSELID_SEND			0x0000		// BFWCmd_SetSELIDの終了コード種別 (実行3)
#define BFWCMD_SETSELID_CLOSE			0x0001		// BFWCmd_SetSELIDの終了コード種別 (実行4)
#define BFWCMD_SETSELID_STARTADDR_E2	0x00000000	// BFWCmd_SetSELIDの開始アドレス(E2/E2 Lite用)(0x00000000固定)	// RevRxE2LNo141104-001 Append Line
// 2009.06.22 INSERT_END_E20RX600 }
#define TRANS_MODE_LV0	0x00	// LV0コマンド入力待ち状態への遷移
#define TRANS_MODE_EML	0x01	// EMLコマンド入力待ち状態への遷移
#define TRANS_MODE_FDT	0x02	// FDTコマンド入力待ち状態への遷移


// RevRxE2LNo141104-001 Append Start
// MONPxxx, SELIDコマンドのエミュレータ識別コード
#define	REWRITE_EMUCODE_E1	0x453031FF
#define	REWRITE_EMUCODE_E20	0x453230FF
#define	REWRITE_EMUCODE_E2	0x4532FFFF
#define	REWRITE_EMUCODE_E2L	0x45324CFF
// RevRxE2LNo141104-001 Append End

// enum定義
enum FFWENM_BFWLOAD_CMD_NO {	// BFW書き換えコマンド種別
	BFWLOAD_CMD_MONP=0,				// MONPコマンド用
	BFWLOAD_CMD_MONPALL,			// MONPALLコマンド用
	BFWLOAD_CMD_MONPFDT,			// MONPFDTコマンド用
	BFWLOAD_CMD_MONPCOM,			// MONPCOMコマンド用	// RevRxE2LNo141104-001 Append Line
};

/////////// 関数の宣言 ///////////
extern FFWERR PROT_SetTMOUT(WORD wBfwTimeOut);		///< BFWCmd_SetTMOUTコマンドの発行
extern FFWERR PROT_GetTMOUT(WORD* pwBfwTimeOut);	///< BFWCmd_GetTMOUTコマンドの発行
// 2008.11.13 INSERT_BEGIN_E20RX600(+1) {
extern FFWERR PROT_GetE20EINF(FFWE20_EINF_DATA* pEinf);				///< BFWCmd_GetRXEINFコマンドの発行
// V.1.02 No.14,15 USB高速化対応 Append Line
extern FFWERR PROT_GetE20EINF_EML(FFWE20_EINF_DATA* pEinf);				///< BFWCmd_GetRXEINF_EMLコマンドの発行
// 2008.11.13 INSERT_END_E20RX600 }
extern FFWERR PROT_MONP_OPEN(enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd);	///< BFWCmd_MONPコマンドの発行開始	// RevRxE2LNo141104-001 Modify Line
extern FFWERR PROT_MONP_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff, enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd);
																		///< BFWCmd_MONPコマンドの発行
extern FFWERR PROT_MONP_CLOSE(enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd);	///< BFWCmd_MONPコマンド終了
//extern FFWERR PROT_DCNF_OPEN(void);										///< BFWCmd_DCNFコマンドの発行開始
// 2008.11.13 INSERT_BEGIN_E20RX600(+1) {
extern FFWERR PROT_DCNF_OPEN(DWORD dwType);						///< BFWCmd_DCNFコマンドの発行開始
// 2008.11.13 INSERT_END_E20RX600 }
extern FFWERR PROT_DCNF_SEND(DWORD dwLength, const BYTE* pbyDcsfBuff);	///< BFWCmd_DCNFコマンドの発行
extern FFWERR PROT_SetUSBT(DWORD dwTransSize);							///< BFWCmd_SetUSBTコマンドの発行
extern FFWERR PROT_GetUSBT(DWORD* pdwTransSize);						///< BFWCmd_GetUSBTコマンドの発行
////////////////////
#include "do_sys.h"
////////////////////
extern FFWERR PROT_GetUSBS(enum FFWENM_USB_SPEED* peUsbSpeedType, BOOL bInit);	///< BFWCmd_GetUSBSコマンドの発行
// 2009.06.22 INSERT_BEGIN_E20RX600(+5) {
extern FFWERR PROT_E20TRANS_LV1(void);								///< BFWCmd_TRANS_LV1コマンドの発行
extern FFWERR PROT_E20TRANS_LV0(void);								///< BFWCmd_TRANS_LV1コマンドの発行
extern FFWERR PROT_E20TRANS(BYTE byBfwTransMode);					///< BFWCmd_TRANSコマンドの発行
// V.1.02 No.14,15 USB高速化対応 Append Start
extern FFWERR PROT_E20TRANS_EML(BYTE byBfwTransMode);				///< BFWCmd_TRANSコマンドの発行
extern FFWERR PROT_E20SUMCHECK(FFWE20_SUM_DATA* pSum);				///< BFWCmd_SUMCHECKコマンドの発行
extern FFWERR PROT_E20SetSELID_OPEN(void);							///< BFWCmd_SetSELIDコマンドの発行開始(E1/E20用)	// RevRxE2LNo141104-001 Modify Line
extern FFWERR PROT_E20SetSELID_SEND(DWORD dwAddr,DWORD dwLength,const BYTE* pbyNumber);	///< BFWCmd_SetSELIDコマンドのシリアル番号書込み(E1/E20用)
extern FFWERR PROT_E20SetSELID_CLOSE(void);							///< BFWCmd_SetSELIDコマンドの発行終了(E1/E20用)
extern FFWERR PROT_E2_SetSELID(DWORD dwLength, const BYTE* pbyNumber);	///< BFWCmd_SetSELIDコマンドの発行(E2/E2 Lite用)	// RevRxE2LNo141104-001 Append Line
extern FFWERR PROT_E20GetSELID( DWORD *pdwNum,char* pbyNumber );
// 2009.06.22 INSERT_END_E20RX600 }

extern FFWERR PROT_EXPWR(enum FFWENM_EXPWR_ENABLE eExPwrEnable, enum FFWENM_EXPWR_VAL eExPwrVal);	///<ターゲットシステムへの電源供給(固定値指定)
// V.1.02 No.14,15 USB高速化対応 Append Line
extern FFWERR PROT_EXPWR_EML(enum FFWENM_EXPWR_ENABLE eExPwrEnable, enum FFWENM_EXPWR_VAL eExPwrVal);	///<ターゲットシステムへの電源供給

extern FFWERR PROT_E20GetD0FIFO( BYTE* );

extern void InitProtData_Sys(void);	///< システムコマンド用変数初期化


#endif	// __PROT_SYS_H__
