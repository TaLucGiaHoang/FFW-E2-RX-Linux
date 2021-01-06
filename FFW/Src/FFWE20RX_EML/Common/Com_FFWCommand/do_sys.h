///////////////////////////////////////////////////////////////////////////////
/**
 * @file do_sys.h
 * @brief システム関連コマンドのヘッダファイル
 * @author RSD Y.Minami, H.Hashiguchi, H.Akashi, S.Ueda, SDS T.Iwata, PA M.Tsuji, PA K.Tsumu
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo121022-001	2012/10/22 SDS 岩田
    EZ-CUBEエミュレータ定義追加
・RevRxNo121022-001	2012/11/1 SDS 岩田
    エミュレータ種別のEZ-CUBEエミュレータ定義名変更
・RevNo120910-002	2012/11/12 明石
　　MONP MONPALLコマンド ダウンロードデータサイズの奇数バイト対応
・RevRxE2LNo141104-001 2015/01/21 上田
	E2 Lite対応
	E1/E20 レベル0 旧バージョン(MP版以外)用関数削除。
・RevRxE2No170201-001 2017/02/01 PA 辻、PA 紡車
	E2エミュレータ対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/
#ifndef	__DO_SYS_H__
#define	__DO_SYS_H__

#include "ffw_typedef.h"
#include "mcudef.h"


// define定義
//#define INIT_USBTRANS_SIZE	0x10000		// USB転送サイズ
#define INIT_USBTRANS_SIZE	0x2000		// USB転送サイズ
#define WIN2KFULL_USBTRANS_SIZE	0x00400	// Windows2000 Full-speed時のUSB転送サイズ
#define WIN2KHI_USBTRANS_SIZE	0x00400	// Windows2000 Hi-speed時のUSB転送サイズ


#define WIN_VER_PLATFORM_WIN32s			0
#define WIN_VER_PLATFORM_WIN32_WINDOWS	1
#define WIN_VER_PLATFORM_WIN32_NT		2

#define EML_E1		0x0001
#define EML_E20		0x0002
// RevRxNo121022-001 Append Line
#define EML_EZCUBE	0x0003
#define EML_E2LITE	0x0004	// RevRxE2LNo141104-001 Append Line
#define EML_E2		0x0005	// RevRxE2LNo141104-001 Append Line

// RevRxE2No1702001-001 Append Start
#define ASCI_NULL		0x00
#define ASCI_SEMICOLON	0x3B
#define ASCI_I			0x49
#define ASCI_D			0x44
#define ASCI_EQUAL		0x3D
// RevRxE2No1702001-001 Append End
#define ASCI_a			0x61
#define ASCI_b			0x62
#define ASCI_c			0x63
#define ASCI_d			0x64
#define ASCI_e			0x65

#define UVCC_1_49V	1.49f	// RevRxE2LNo141104-001 Modify Line
#define UVCC_1_20V	1.20f	// RevRxE2No170201-001 Modify Line

// RevNo120910-002	Append Start
// MONP 及びMONPALL用定義
#define	FFW_MONP_FLBUFSIZ		0x40000		// MONPコマンド用バッファサイズ
#define	FFW_ALL_TOPADDR			0x00000000	// Level0、EML、FDT全領域書き込み開始アドレス
#define	FFW_EML_TOPADDR			0x00010000	// EML書き込み開始アドレス
#define	FFW_FDT_TOPADDR			0x00030000	// FDT書き込み開始アドレス
#define	FFW_EML_SIZE			0x20000		// EML領域書き込みサイズ
#define	FFW_EMLALL_SIZE			0x40000		// Level0、EML、FDT全領域書き込みサイズ
#define	FFW_EML_SIZE_INIT		0x00000		// 書き込み長初期値
#define	FFW_SEND_UNITCNT		0x2000		// 書き込みデータサイズ(8192バイト)
#define	FFW_MONP_BUFDEFAULT		0xFF		// 書き込みバッファデータ
// RevNo120910-002	Append End

// RevRxE2LNo141104-001 Append Start
#define	SELID_AREA_NONDATA_E2	0xFF		// シリアル番号未設定領域データ(E2/E2 Lite用)
#define	SELID_AREA_ENDCODE_E2	0xAA		// シリアル番号領域終了コード(E2/E2 Lite用)
// RevRxE2LNo141104-001 Append End

// enum定義
enum FFWENM_USB_SPEED {		// USB転送種別
	USB_SPEED_FULL=0,			// Full-Speed
	USB_SPEED_HIGH				// Hi-Speed
};

// グローバル関数の宣言
extern FFWERR DO_INIT(void);									///< INIT処理
extern FFWERR DO_SetUSBTS(DWORD dwTransSize);					///< USB転送サイズの設定
extern FFWERR DO_GetUSBTS(DWORD* pdwTransSize);					///< USB転送サイズの参照
extern FFWERR DO_SetTMOUT(WORD wFfwTimeOut, WORD wBfwTimeOut);	///< FFW, BFWのタイムアウト時間の設定
extern FFWERR DO_GetTMOUT(WORD* pwBfwTimeOut);					///< BFWのタイムアウト時間の参照
extern FFWERR DO_GetE20EINF(FFWE20_EINF_DATA* pEinf);				///< エミュレータ情報の取得

extern FFWERR DO_MONP_OPEN(DWORD dwTotalLength);				///< モニタプログラム(BFW EML)データの送信開始を通知
extern FFWERR DO_MONP_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW EML)データの送信
extern FFWERR DO_MONP_CLOSE(void);								///< モニタプログラム(BFW EML)データの送信終了を通知
extern FFWERR DO_MONPFDT_OPEN(DWORD dwTotalLength);				///< モニタプログラム(BFW FDT/PRG)データの送信開始を通知
extern FFWERR DO_MONPFDT_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW FDT/PRG)データの送信
extern FFWERR DO_MONPFDT_CLOSE(void);							///< モニタプログラム(BFW FDT/PRG)データの送信終了を通知
// RevRxE2LNo141104-001 Append Start

#include "prot_sys.h"

extern FFWERR MonpOpen(enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd);	///< MONPxxx OPEN処理実行
extern FFWERR MonpSend(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< MONPxxx SEND処理実行
extern FFWERR MonpClose(enum FFWENM_BFWLOAD_CMD_NO eBfwLoadCmd);	///< MONPxxx CLOSE処理実行
extern void NewBfwDataBuff(void);								///< BFWコード格納用バッファの確保と初期化
extern void DeleteBfwDataBuff(void);							///< BFWコード格納用バッファの解放
extern BYTE* GetBfwCodeBuffPtr(DWORD dwAddr);					///< BFWコード格納用バッファポインタの取得
// RevRxE2LNo141104-001 Append End

// 2008.11.13 INSERT_BEGIN_E20RX600(+1) {
extern FFWERR DO_E20DCNF_OPEN(DWORD dwTotalLength, DWORD dwType);	///< DCSFコンフィギュレーション開始を通知
// 2008.11.13 INSERT_END_E20RX600 }
extern FFWERR DO_DCNF_SEND(DWORD dwLength, const BYTE* pbyDcsfBuff);	///< DCSFコンフィギュレーションデータの送信
extern FFWERR DO_DCNF_CLOSE(void);								///< DCSFコンフィギュレーションの終了

extern float ConvVbusVal(WORD wVbusVal, WORD wEmuStatus);	///< USBバスパワー電圧のA/D変換値→電圧値への変換 // RevRxE2LNo141104-001 Append Line
extern float ConvUvccVal(WORD wUvccVal, WORD wEmuStatus);	///< ユーザシステム電圧のA/D変換値→電圧値への変換 // RevRxE2LNo141104-001 Append Line
extern FFWERR getEinfData(FFWE20_EINF_DATA* pEinf);			///< FFW内で保持しているエミュレータ情報取得
extern BOOL GetEinfDataSetFlg(void);						///< FFW内で保持しているエミュレータ情報の設定状況取得 // RevRxE2LNo141104-001 Append Line

extern FFWERR SetUsbTransferSize_INIT(DWORD dwTransSize);		///< FFWCmd_INIT時のUSB転送サイズ設定
extern FFWERR SetUsbTransferSize_USBTS(DWORD dwTransSize);		///< FFWCmd_USBTS時のUSB転送サイズ設定
extern void InitFfwCmdData_Sys(void);							///< システムコマンド用FFW内部変数の初期化

// RevRxE2No171004-001 Append Line
extern DWORD GetUsbTransferSize(void);						///< USB転送サイズを取得

extern BOOL getFirstConnectInfo(void);						///< 初回起動時に接続した状態を取得
extern void setFirstConnectInfo(BOOL bSet);					///< 初回起動時に接続した状態を設定

extern FFWERR DO_EXPWR(enum FFWENM_EXPWR_ENABLE eExPwrEnable, enum FFWENM_EXPWR_VAL eExPwrVal);	///< ターゲットシステムへの電源供給(固定値指定)

// 2009.6.22 INSERT_BEGIN_E20RX600(+5) {
extern FFWERR DO_E20Trans(BYTE byBfwTransMode);						///< BFW 指定内部動作モードへの遷移
extern FFWERR DO_E20SUMCHECK(FFWE20_SUM_DATA* pSum);					///< 各領域のサム値を取得
// RevRxE2LNo141104-001 Modify Start
// DO_E20SetSELID_OPEN(), DO_E20SetSELID_SEND(), DO_E20SetSELID_CLOSE()をDO_E20_SetSELID()に変更
extern FFWERR DO_E20_SetSELID(DWORD dwNum, const char cNumber[]);	///< シリアル番号設定(E1/E20用)
// RevRxE2LNo141104-001 Modify End
extern FFWERR DO_E20GetSELID(DWORD* pdwNum,char* pbyNumber);///< シリアル番号を取得
// 2009.6.22 INSERT_END_E20RX600 }
extern FFWERR DO_E2_SetSELID(DWORD dwNum, const char cNumber[]);	///< シリアル番号設定(E2/E2 Lite用)	// RevRxE2LNo141104-001 Append Line

extern FFWERR E20TransLV0(void);						///< BFW LEVEL0への遷移

#endif	// __DO_SYS_H__
