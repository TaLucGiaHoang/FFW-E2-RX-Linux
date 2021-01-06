////////////////////////////////////////////////////////////////////////////////
/**
 * @file dorx_tra.h
 * @brief RX共通トレース関連コマンドのヘッダファイル
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2016/10/05
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RX220 WS対応 RX210でRX220対応 2012/07/11 橋口
・RevRxNo120606-005 2012/06/07 橋口
  RX Class2トレース オペランドトレースイベント番号がずれる不具合対応
・RevRxNo130301-001	2013/06/14 上田
  RX64M対応
・RevRxNo130408-001	2014/04/17 上田
	外部トレースデータ8ビット幅対応
・RevRxE2LNo141104-001 2014/11/17 上田
	E2 Lite対応
・RevRxNo161003-001	2016/10/05 PA 辻
	ROMデータトレースアクセスタイプ出力対応
*/
#ifndef	__DORX_TRA_H__
#define	__DORX_TRA_H__


#include "ffw_typedef.h"
#include "mcudef.h"


///// define定義
///// 共通
// コマンド種別
#define TRC_CMD_BRANCH		0				// 分岐命令
#define TRC_CMD_ACCESS		1				// データアクセス命令
#define TRC_CMD_HWDBG		2				// ハードウェアデバッグ
#define TRC_CMD_TIMESTAMP	3				// タイムスタンプ


///// トレースモード設定関連
//RM SetModeビット定義
#define RM_SETMODE_MODE			0xFF000000
	#define FUNCS_FREETRC		0x00000000
	#define FUNCS_FULLTRC		0x00000001
#define RM_SETMODE_TDOS			0x0000000C	// RevRxNo130301-001 Append Line
	#define TDOS_OUT_NOTBRK		0x00000000
	#define TDOS_OUT_BRK		0x00000001
	#define TDOS_IN_NOTBRK		0x00000002
	#define TDOS_IN_BRK			0x00000003
	#define TMWM_NOTWRAP		0x00000000
	#define TMWM_WRAP			0x00000001
#define RM_SETMODE_TRM			0x00000001
	#define TRM_REALTIME		0x00000000
	#define TRM_TRCFULL			0x00000001

//RM SetInitialビット定義
//RevNo110309-004 Append Line
#define RM_SETINITIAL_DLY			0xF0000000
// RevRxNo130301-001 Append Start
#define RM_SETINITIAL_TRV			0x00010000
	#define RM_SETINITIAL_TRV_DIS		0x00000000
// RevRxNo130301-001 Append End
// RevRxNo130408-001 Append Start
#define RM_SETINITIAL_TBW			0x0000F000
	#define RM_SETINITIAL_TBW_D4S1		0x00000000	// DATA 4ビット、SYNC 1ビット
	#define RM_SETINITIAL_TBW_D8S2		0x00001000	// DATA 8ビット、SYNC 2ビット
	#define RM_SETINITIAL_TBW_D8S1		0x00004000	// DATA 8ビット、SYNC 1ビット(未サポート)
// RevRxNo130408-001 Append End
#define RM_SETINITIAL_TRFS			0x000000F0
	#define RM_SETINITIAL_TRFS_1_2		0x00000010
// RevRxNo130301-001 Append Start
#define RM_SETINITIAL_TRE			0x00000100
	#define RM_SETINITIAL_TRE_DIS		0x00000000
// RevRxNo130301-001 Append End

//RM SetInfoビット定義
#define RM_SETINFO_RODE				0x00000400	//10bit
#define RM_SETINFO_BSS				0x00008000	//15bit
#define RM_SETINFO_TBE				0x00020000	//17bit
#define RM_SETINFO_TDE				0x00040000	//18bit
#define RM_SETINFO_TRI				0x00080000	//19bit
#define RM_SETINFO_TMO				0x00100000	//20bit
#define RM_SETINFO_TDA				0x00200000	//21bit
#define RM_SETINFO_TSE				0x00400000	//22bit
#define RM_SETINFO_TPE				0x0C000000	//26-27bit
#define RM_SETINFO_CLS2_TMSTMP		0x04000000
#define RM_SETINFO_RODTE			0x80000000	//31bit		// RevRxNo161003-001 Append Line

///// トレースデータ取得関連
// オペランドアクセス情報
#define TRC_OPR_ACS_R		0				// アクセス属性(リード)
#define TRC_OPR_ACS_W		1				// アクセス属性(ライト)
#define TRC_OPR_ACSSIZE_B	1				// アクセスサイズ(バイト)
#define TRC_OPR_ACSSIZE_W	2				// アクセスサイズ(ワード)
#define TRC_OPR_ACSSIZE_L	4				// アクセスサイズ(ロングワード)
// RevRxNo120606-005 Append Start
#define TRC_OPR_EVNUM_DE0	0x00000001		// トレース結果オペランドイベント番号DE0
#define TRC_OPR_EVNUM_DE1	0x00000002		// トレース結果オペランドイベント番号DE1
#define TRC_OPR_EVNUM_DE2	0x00000003		// トレース結果オペランドイベント番号DE2
#define TRC_OPR_EVNUM_DE3	0x00000004		// トレース結果オペランドイベント番号DE3
// RevRxNo120606-005 Append End

// 2008.10.10 INSERT_BEGIN_E20RX600(+18) {
// 識別子1
#define	STDBY					0x0000			// スタンバイ状態
#define	LOST					0x0001			// ロスト情報
#define	MODE					0x0004			// スタックポインタ情報
#define	BTR1					0x0006			// 分岐成立可否情報
#define	BTR2					0x0007			// 分岐先アドレス情報
#define	BTR3					0x0008			// 分岐元アドレス情報
#define	DTR1					0x0009			// 1バイトリード情報
#define	DTR2					0x000A			// 2バイトリード情報
#define	DTR4					0x000B			// 4バイトリード情報
#define	DTW1					0x000C			// 1バイトライト情報
#define	DTW2					0x000D			// 2バイトライト情報
#define	DTW4					0x000E			// 4バイトライト情報

// 識別子3
#define	COMP_4BIT				0x00			// 4bit長に圧縮
#define	COMP_8BIT				0x01			// 8bit長に圧縮
#define	COMP_16BIT				0x02			// 16bit長に圧縮
#define	COMP_NON				0x03			// 非圧縮(32bit長で出力)

// ビット数
#define	BITNUM_DWORD			32				// DWORDのビット数
// 2008.10.10 INSERT_END_E20RX600 }


// ハードウェアデバッグ情報
#define TRC_HW_NORMAL		0				// 分岐情報の種類(一般)
#define TRC_HW_SUB			1				// 分岐情報の種類(サブルーチン)
#define TRC_HW_EXCEPT		2				// 分岐情報の種類(例外)


///// トレースサイクル取得関連
#define RCY_CYCLE_ALL_FF		0xFFFFFFFF	//トレースサイクルALL FF
#define RCY_CYCLE_NON			0xFFFFFFFF	//トレースサイクルがない場合


///// トレース停止関連
#define TRC_END_WAIT_TIME	100				// トレース吐き出し終了時間 200μsecなので 1msとしておく


///// 外部トレース関連
// RevRxNo130408-001 Modify Start
#define RCY_CYCLE_MAX_D4S1		0x04000400		//トレースサイクル数論理上の最大値(DATA 4ビット、SYNC 1ビット)
#define RCY_CYCLE_MAX_D8S2		0x08000800		//トレースサイクル数論理上の最大値(DATA 8ビット、SYNC 2ビット) 
// RevRxNo130408-001 Modify End
#define RD_TRC_BLOCK			0x00000800		// トレースデータの1ブロックサイズ
#define RD_TRCMEM_INDEX			0x00010001		// トレースメモリのインデックス領域(+トレースバッファ1ブロック分)
#define RD_TRCMEM_SIZE			0x08000000		// トレースメモリの最大サイズ(64MB*2Byte)
// RevRxNo130408-001 Append Start
#define RD_BUFSIZE_TRCBLOCK_D4S1	0x00000800	// 1トレースメモリブロックのトレースメモリデータ格納バッファサイズ(DATA 4ビット、SYNC 1ビット)
#define RD_BUFSIZE_TRCBLOCK_D8S2	0x00001000	// 1トレースメモリブロックのトレースメモリデータ格納バッファサイズ(DATA 8ビット、SYNC 2ビット)
// RevRxNo130408-001 Append End
#define TRCMEM_BLKDIV_SIZE		0x400			// RCY-RD高速化 トレースサイクル数分割取得時の分割まとめサイズ
#define TRC_FPGA_TMWBA_TMWBS	0x0200
#define TRC_FPGA_TMWBA_ADDR		0x01FF
#define TRC_FPGA_BUFF_FULL		0x03E4			//トレースFPGAのTRCTL0.TMFULLビットが立つのはトレースメモリがいっぱいで、トレースバッファがTRC_FPGA_BUFF_FULL以上のとき

// 外部トレースフォーマット
#define TRCDATA_TRDATA03		0x000F
#define TRCDATA_CMD_INF			0xF000
// RevRxNo130408-001 Modify Start
	// DATA 4ビット、SYNC 1ビット用定義
	#define TRCDATA_CMD_INF_STNBY_D4S1		0x0000	//無効データ
	#define TRCDATA_CMD_INF_IDNT1_D4S1		0x1000	//識別子1
	#define TRCDATA_CMD_INF_IDNT2_D4S1		0x2000	//識別子2
	#define TRCDATA_CMD_INF_IDNT3_1_D4S1	0x4000	//識別子3-1
	#define TRCDATA_CMD_INF_IDNT3_2_D4S1	0x6000	//識別子3-2
	#define TRCDATA_CMD_INF_DATA_D4S1		0x8000	//データ
	// DATA 8ビット、SYNC 2ビット用定義
	#define TRCDATA_CMD_INF_STNBY_D8S2		0x0000	//無効データ
	#define TRCDATA_CMD_INF_IDNT1_D8S2		0x1000	//識別子1
	#define TRCDATA_CMD_INF_IDNT2_D8S2		0x2000	//識別子2
	#define TRCDATA_CMD_INF_IDNT3_1_D8S2	0x3000	//識別子3-1
	#define TRCDATA_CMD_INF_IDNT3_2_D8S2	0x4000	//識別子3-2
	#define TRCDATA_CMD_INF_DATA_D8S2		0x5000	//データ
// RevRxNo130408-001 Modify End

// RevRxNo130408-001 Append Start
// E20トレースメモリの使用サイズ
#define E20_TMSIZE_128MB	0x0000	// 128Mバイト
#define E20_TMSIZE_64MB		0x0001	// 64Mバイト
#define E20_TMSIZE_32MB		0x0002	// 32Mバイト
#define E20_TMSIZE_16MB		0x0003	// 16Mバイト
#define E20_TMSIZE_8MB		0x0004	// 8Mバイト
#define E20_TMSIZE_4MB		0x0005	// 4Mバイト
#define E20_TMSIZE_2MB		0x0006	// 2Mバイト
#define E20_TMSIZE_1MB		0x0007	// 1Mバイト
// RevRxNo130408-001 Append End


///// 内部トレース関連 (Class 3用)
#define TRC_BUF_MAX			0x1000
#define CLS2_TRC_BUF_MAX	0x200	// V.1.02 No.31 Class2 トレース対応 Append Line

///// 内部トレース関連 (Class2用)
// Class2のトレースメモリ段数
#define TRC_CYC_MAX_CLS2		64
// RX220 WS対応 Append Start
#define TRC_CYC_CLS2_CYC32		32
#define TRC_CYC_CLS2_CYC0		0
// RX220 WS対応 Append End
#define CLR_TRC_MEM_UPPER_ADDR		0x00000FFF	// Class2のトレースメモリで上位側をクリアする



///// 構造体定義
typedef struct {
//	BOOL	bValid;							// 有効ブロックか否か(TRUE:無効ブロック、FALSE:有効ブロック)
	DWORD	dwStartCycle;					// 開始サイクル数
	DWORD	dwEndCycle;						// 終了サイクル数
} RD_INDEX;


///// enum定義
enum FFWENM_TRCBRK_MODE {		// トレースブレークモード
	TRC_BRK_DISABLE=0,				// トレースブレーク無効
	TRC_BRK_ENABLE					// トレースブレーク有効
};



///// プロトタイプ宣言（コマンド処理部）
FFWERR DO_SetRXRM(const FFWRX_RM_DATA* pTraceMode);
FFWERR DO_SetRXCls2RM(const FFWRX_RM_DATA* pTraceMode);	// V.1.02 No.31 Class2 トレース対応 Append Line
FFWERR DO_GetRXRD(DWORD dwStartCyc, DWORD dwEndCyc, DWORD* pdwSetInfo, FFWRX_RD_DATA* pRd);
FFWERR DO_GetRD2_SPL(DWORD dwBlockStart, DWORD dwBlockEnd, WORD* rd);
FFWERR DO_ClrRD(void);
FFWERR DO_GetRXRCY(FFWRX_RCY_DATA *pRcy);
FFWERR DO_SetRXTRSTP(enum FFWENM_TRSTP eTrstp, BOOL bSetDmmyBTR1);

///// プロトタイプ宣言（その他の関数）
// トレースモード設定関連
extern WORD GetE20TrcMemSize(void);	// RevRxNo130408-001 Append Line
extern FFWERR SetTrace(enum FFWENM_TRCBRK_MODE eTrcBrk);
extern BOOL GetTrcInramMode(void);
extern BOOL GetTrcFreeMode(void);	// RX220 WS対応 Append Line

// トレースサイクル取得関連
extern DWORD GetTrcCycMaxCls2(void);	// RX220 WS対応 Append Line
extern BOOL GetSetRcyFlg(FFWRX_RCY_DATA* rcy);	// V.1.02 No.31 Class2 トレース対応 Append Line

// トレースデータ取得関連
extern DWORD GetRdBufSizeTraceBlock(void);	// RevRxNo130408-001 Append Line
extern DWORD GetRdBufSizeTraceBuffer(WORD wTmwba);	// RevRxNo130408-001 Append Line
extern void DeleteTrcDataInram(void);	// V.1.02 RevNo110609-002 Append Line
extern BOOL GetTrcOffSwFlg(void);

// トレースデータクリア関連
extern FFWERR ClrTrcMemData(void);	// RevRxE2LNo141104-001 Append Line

// トレース関連FFW内部処理用関数
extern FFWERR ClrTrcReg(void);
extern float GetTRCLKCalc(DWORD dwTrclk);
extern  FFWERR StopE20Trc(FFWRX_STAT_EML_DATA* pStatEML,BOOL* pbStopTrc);	// V.1.02 No.34 トレース終了イベントでのトレース停止対応 Append Line
extern  void SetClrTrcMemSize(void);	// RevNo110125-01(V.1.02 覚書 No.27) トレースメモリクリア不具合対応 Append Line

// 初期化関数
extern void InitFfwCmdRxData_Tra(void);	///< トレース関連コマンド用FFW内部変数の初期化

#endif	// __DORX_TRA_H__
