///////////////////////////////////////////////////////////////////////////////
/**
 * @file FFWE20.h//
 * @brief E1/E20/E2 Lite/E2エミュレータFFW API関数のヘッダファイル//
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, Y.Miyake, S.Ueda, PA K.Tsumu, REL H.Ohga//
 * @author (C) 2009-2015,2017,2018 Renesas Electronics Corporation. All rights reserved.//
 * @date 2018/09/05//
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴//
・ソース構成改善	2012/07/10 橋口//
　　ベース:FFWE20RX600.dll V.1.02.00.015//
・RevRxNo120910-001 2012/09/27 三宅//
  FFW I/F仕様変更に伴うFFWソース変更。//
  ・以下の関数宣言追加。//
    FFWMCUCmd_SetMCUD, FFWMCUCmd_GetMCUD, FFWMCUCmd_SetRSST, FFWMCUCmd_SetRSEN, //
    FFWMCUCmd_SetSSST, FFWMCUCmd_SetSSEN, FFWE20Cmd_CPUF, FFWCmd_SetCPUD。//
  ・以下の構造体の定義追加。//
    FFW_MCUD_DATA, FFW_CPUD_DATA。//
  ・MCUD_NUM_MAX の定義がコメントだったのを、定義するように変更。//
  ・以下の定義を削除。//
	  EINF_BOARD_LOTNO_NUM, EINF_OPFPGA_FUNC_NUM_MAX, //
	  EINF_CABLE_NUM_MAX, RRM_E20_SET_BLKNUM_MAX。//
  ・以下の enum の定義を削除。//
	  enum FFWENM_CONNECT_STAT, enum FFWENM_CABLECONNECT_STAT。//
・RevRxE2LNo141104-001 2014/11/13 上田//
　・E2 Lite対応//
　・BFW固有アドレス定義等不要な定義の削除//
・RevRx6z4FpgaNo150525-001 2015/08/20 PA紡車//
  RX6Z4マルチコア対応//
・RevRxNo170908-001 2017/09/08 大賀//
  FFWE2Cmd_BfwEmlUpdate()対応//
・RevRxNo180831-001,PB18047-CD04-001  2018/08/31 大賀//
  FFWE20Cmd_BfwEmlUpdate()対応//
*/
#ifndef	__FFWE20_H__
#define	__FFWE20_H__

#ifndef	FFWE100_API	// RevRxE2LNo141104-001 Append Line//
#ifdef _WIN32
#ifdef FFWE20_EXPORTS
	#define FFWE100_API __declspec(dllexport)
#else
	#define FFWE100_API __declspec(dllimport)
	#ifdef FFWE100_INSTANCE
		#define	__EXTERN__
	#else
		#define	__EXTERN__	extern
	#endif
#endif
#else // __linux__
#define FFWE20_EXPORTS
#define FFWE100_API 
#define __EXTERN__	extern
#endif
#endif	// RevRxE2LNo141104-001 Append Line//

#ifdef	__cplusplus
	extern	"C" {
#endif

/////////// define定義 ///////////
// システムコマンド用//
#define INIT_WINVER_ADDINFO_NUM		128		// OSバージョンの追加情報文字数//
#define EINF_BFW_NAME_NUM			16		// BFW名のバイト数		// RevRxNo170920-002 Delete Line//
#define CPUD_NUM_MAX				256		// CPUD設定データ個数最大値//

// エラー詳細情報用//
#define ERROR_MESSAGE_NUM_MAX		16
#define ERROR_MESSAGE_LENGTH_MAX	32

// MCU依存コマンド用//
// RevRxNo120910-001 Modify Line//
#define MCUD_NUM_MAX				256		// MCUD設定データ個数最大値//
//#define EVAD_NUM_MAX				256		// EVAD設定データ個数最大値//
// プログラムダウンロード関連コマンド//
#define FCLR_BLKNUM_MAX				255		// FCLRコマンドのフラッシュブロック数の最大値//
#define DWNP_FLASHTYPE_MAX			16		// DWNPコマンド時のFlashROM情報最大個数//
// ブレーク関連コマンド用//
#define PB_SETNUM_MAX				256		// ソフトウェアブレークポイント設定点数最大値//

// メモリ操作関連コマンド//
#define MCMP_DATASIZE_MAX			8		// 比較データサイズ最大値//

// RAMモニタ関連コマンド用//
#define RRM_BLKNUM_MAX				32		// RAMモニタ領域のブロック数最大値//
#define RRM_AREANUM_MAX				1024	// RAMモニタデータ取得領域数最大値//
#define	RRM_E20_BLKNUM_MAX			4		// E20用RAMモニタ領域のブロック数最大値//

// 外部Flash用定義 //
#define EXTF_WRITE_PROGRAM_MAX		8192	// 書き込みプログラムのバイト数最大値 //
#define EXTF_FILL_DATA_MAX			256		// 外部ROMアクセス前後のFILL情報数最大値 //
#define EXTF_FROM_SECTOR_MAX		4096	// セクタ領域数最大値 //
#define FFW_EXTF_DATA_NUM_MAX		4		// 外部フラッシュダウンロード情報数最大値 //
#define MAX_PATH_SIZE				1024	// 外部Flash用スクリプトパスの文字数 //

/////////// enum定義 ///////////
//---------------------------
// 共通//
//---------------------------
enum FFWENM_EACCESS_SIZE {	// モニタCPUのアクセスサイズ//
	EBYTE_ACCESS=0,				// バイトサイズ(1バイト)//
	EWORD_ACCESS,				// ワードサイズ(2バイト)//
	ELWORD_ACCESS				// ロングワードサイズ(4バイト)//
};

enum FFWENM_MACCESS_SIZE {	// ターゲットMCUのアクセスサイズ//
	MBYTE_ACCESS=0,				// バイトサイズ(1バイト)//
	MWORD_ACCESS,				// ワードサイズ(2バイト)//
	MLWORD_ACCESS,				// ロングワードサイズ(4バイト)//
	MDOUBLE_ACCESS,				// ダブルサイズ(8バイト)//
};
enum FFWENM_ENDIAN {		// エンディアン //
	ENDIAN_LITTLE=0,			// リトルエンディアン //
	ENDIAN_BIG					// ビッグエンディアン //
};

//---------------------------
// システムコマンド//
//---------------------------
// EINF
enum FFWENM_EML_POWER {			// エミュレータに対する電源供給//
	EML_PWR_OFF=0,					// 電源未供給状態//
	EML_PWR_ON						// 電源供給状態//
};
enum FFWE20_STAT_UVCC {				// ユーザシステム電源状態//
	UVCC_OFF=0,						// 電源OFF//
	UVCC_ON,						// 電源ON//
};
enum FFWE20_BFW_MODE {				// BFW動作モード//
	BFW_LV0=0,						// LV0動作モード//
	BFW_EML,						// EML動作モード//
	BFW_FDT,						// FDT動作モード//
};
enum FFWE20_STAT_UCON {				// ユーザシステム接続状態//
	TRGT_EML_NCON=0,				// 未接続//
	TRGT_EML_CON,					// 接続//
};
enum BFWENM_CONNECT_IF {			// ターゲットボードとの接続形態//
	CONNECT_IF_NON=0,				// ケーブル未接続//
	CONNECT_IF_38PIN,				// JTAG 38ピン//
	CONNECT_IF_14PIN,				// JTAG 14ピン//
	CONNECT_IF_RESERVE,				// 予約//
	CONNECT_IF_SELF,				// 38pinケーブル＋セルフチェックコネクタ接続//
	CONNECT_IF_LOOPBACK,			// 38pinケーブル＋38pin→14pin変換コネクタ＋14pin→38pin変換コネクタ＋セルフチェックコネクタ接続//
	CONNECT_IF_IRREGULAR,			// ケーブル接続異常//
};
enum BFWENM_DEBUG_IF {				// 通信制御FPGAデバッグインタFフェース種別//
	SCI_DEBUG_RESERVE=0,			// RESERVE//
	SCI_DEBUG_RX_JTAG,				// RX-JTAG//
	SCI_DEBUG_SCI,					// SCI//
};
enum BFWENM_MCU_TYPE {				// 通信制御FPGAデバッグインタフェース種別//
	TRACE_MCU_RESERVE=0,			// RESERVE//
	TRACE_MCU_RX610,				// RX-610//
};

// EXPWR
enum FFWENM_EXPWR_ENABLE {		// ユーザ基板への電源供給有無を設定//
	EXPWR_DISABLE = 0,				// 電源供給しない//
	EXPWR_ENABLE,					// 電源供給する//
};
enum FFWENM_EXPWR_VAL {			// ユーザ基板への電源供給電圧を設定//
	EXPWR_NON = 0,					// 供給なし//
	EXPWR_V330,						// 3.3V供給//
	EXPWR_V500,						// 5.0V供給//
	EXPWR_V180,						// 1.8V供給//	// RevRxE2LNo141104-001 Append Line//
};

// DWNP
enum FFWENM_CALLBACKFUNC_FWRITE_STATUS {	// フラッシュダウンロードステータス//
	FWRITE_START = 0x100,	// イレーズ/ライト開始//
	FWRITE_END,				// イレーズ/ライト終了//
	FWRITE_ERROR,			// イレーズ/ライトエラー//
};


//---------------------------
// ブレーク関連コマンド//
//---------------------------
// PB
enum FFWENM_PB_SW {				// ソフトウェアブレークの設定／解除//
	PB_SET=0,						// 設定//
	PB_CLR							// 解除//
};

enum FFWENM_PB_FILLSTATE {		// ソフトウェアブレークの設定状態//
	PB_FILL_NON=0,					// 埋め込み不要//
	PB_FILL_CLR,					// 埋め込み解除待ち//
	PB_FILL_SET,					// 埋め込み設定待ち//
	PB_FILL_COMPLETE				// 埋め込み済み//
};

//---------------------------
// プログラム実行関連コマンド//
//---------------------------
// 共通//
enum FFWENM_STARTADDR_SET {		// 実行開始アドレスの指定有無//
	ADDR_SETON=0,					// 実行開始アドレスを指定する//
	ADDR_SETOFF						// 実行開始アドレスを指定しない//
};
// STEP
enum FFWENM_RETURN_STEP {		// リターンサブルーチン命令のステップ実行情報//
	STEP_NONRTSCODE=0,				// リターンサブルチン命令以外の命令でステップ実行を開始した//
	STEP_RTSCODE					// リターンサブルチン命令でステップ実行を開始した//
};
enum FFWENM_SUBCALL_STEP {		// サブルーチンコール命令のステップ実行情報//
	STEP_NONSUBCALLCODE=0,			// サブルーチンコール命令以外の命令でステップ実行を開始した//
	STEP_SUBCALLCODE				// サブルーチンコール命令でステップ実行を開始した//
};
enum FFWENM_STEP_RUN {			// ステップ実行の完了情報//
	STEP_NOT_GOTO_RUNSTATE=0,		// 一定時間内にステップ実行が完了した//
	STEP_GOTO_RUNSTATE				// 一定時間内にステップ実行が完了しなかった。プログラム実行状態を継続する。//
};

//---------------------------
// レジスタ操作関連コマンド//
//---------------------------
enum FFWENM_SETREG_ENABLE {		// レジスタ設定有無情報//
	REG_ACCESS_OFF=0,				// レジスタ値を設定しない//
	REG_ACCESS_ON					// レジスタ値を設定する//
};

//---------------------------
// メモリ操作コマンド//
//---------------------------
// 共通//
enum FFWENM_VERIFY_SET {		// ベリファイ指定//
	VERIFY_OFF=0,					// ベリファイ無効//
	VERIFY_ON						// ベリファイ有効//
};
enum FFWENM_VERIFY_RESULT {		// ベリファイ結果//
	VERIFY_OK=0,					// 正常終了//
	VERIFY_ERR						// ベリファイエラー//
};

// MCMP
enum FFWENM_COMP_RESULT {		// コンペア結果//
	COMP_OK=0,						// 正常終了//
	COMP_ERR						// コンペアエラー//
};

// MSER
enum FFWENM_MSEARCH_EQU {		// 検索条件//
	EQU_SEARCH=0,					// 一致検索//
	NOTEQU_SEARCH					// 不一致検索//
};
enum FFWENM_MSEARCH_DIR {		// 検索方向//
	FORWARD_SEARCH=0,				// 検索開始アドレスから順に検索する//
	BACK_SEARCH						// 検索終了アドレスから検索対象データを検索する//
};
enum FFWENM_MSEARCH_PATERN {	// 検索方式//
	PATERN_SEARCH=0,				// 1アドレス単位で検索を行う//
	DATASIZE_SEARCH					// 検索データサイズ単位で検索を行う//
};
enum FFWENM_MSEARCH_RESULT {	// 検索結果//
	MATCH=0,						// 指定データを検出した//
	NON_MATCH						// 指定データを検出できなかった//
};

//---------------------------
// RAM モニタ関連コマンド//
//---------------------------

enum FFWENM_RRMB_ENABLE {		// RAM モニタ領域の動作状態//
	RRMB_DIS=0,						// 動作禁止//
	RRMB_ENA						// 動作許可//
};

//---------------------------
// 時間測定関連コマンド//
//---------------------------
// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
enum FFWENM_EMLMESCLK_SEL {		// 全実行時間測定のクロックソース//
	TCLK_8_250MHZ = 0,				// 8.250MHz//
	TCLK_4_125MHZ,					// 4.125MHz//
	TCLK_2_063MHZ,					// 2.063MHz//
	TCLK_1_031MHZ,					// 1.031MHz//
};
// 2008.11.18 INSERT_END_E20RX600 }

//----------------------------------------
// 3.5	プログラムダウンロード関連コマンド//
//----------------------------------------
enum FFWENM_CHKSUM {
	CHECKSUM_NON_AGREE=0,
	CHECKSUM_AGREE
};

/////////// 構造体定義 ///////////
//---------------------------
// システムコマンド//
//---------------------------
// OSバージョン情報//
typedef struct {
	DWORD	dwMajorVersion;							// OSのメジャーバージョン情報//
	DWORD	dwMinorVersion;							// OSのマイナーバージョン情報//
	DWORD	dwBuildNumber;							// OSのビルドナンバー情報//
	DWORD	dwPlatformId;							// OSのプラットフォームID情報//
	char	szCSDVersion[INIT_WINVER_ADDINFO_NUM];	// OSバージョンの追加情報//
} FFW_WINVER_DATA;
// エミュレータ情報(E1/E20用)//
typedef struct {
	WORD	wEmuStatus;								// エミュレータ種別//
	WORD	wBoardRev;								// エミュレータ基板REV//
	enum FFWENM_EML_POWER	eEmlPwrSupply;			// エミュレータへの電源(ACアダプタ)供給（E20 REV.Aのみ）//
	WORD	wVbusVal;								// USBバスパワー(A/D変換値)//
	float	flNowVbus;								// USBバスパワー電圧//

	WORD	wTagetCable;							// ユーザI/F接続形態(A/D変換値)//
	enum BFWENM_CONNECT_IF	eStatUIF;				// ユーザI/F接続形態//
	enum FFWE20_STAT_UCON	eStatUCON;				// ユーザシステム接続状態//
	WORD	wUvccVal;								// ユーザシステム電圧(A/D変換値)//
	float   flNowUVCC;								// ユーザシステム電圧//
	enum FFWE20_STAT_UVCC	eStatUVCC;				// ユーザシステム電源状態//
	BYTE	byBfwMode;								// BFW動作モード//

	DWORD	dwBfwLV0Ver;							//※// LV0バージョン//
	DWORD	dwBfwEMLVer;							//※// EMLバージョン//
	WORD	wEmlTargetCpu;							//※// EMLターゲットMCU情報//
	WORD	wEmlTargetCpuSub;						//※// EMLターゲットMCUサブ情報//
	char	cEmlTgtMucId[EINF_BFW_NAME_NUM];		//※// EMLターゲットMCU ID//
	DWORD	dwEmlTargetLv0Ver;						//※// EML対応LV0バージョン//
	DWORD	dwBfwFdtVer;							//※// FDTバージョン//
	char	cFdtTgtMucId[EINF_BFW_NAME_NUM];		//※// FDTターゲットMCU ID//
	DWORD	dwFdtTargetLv0Ver;						//※// FDT対応LV0バージョン//

	BYTE	byFpga1Status;							// 通信制御FPGAコンフィグレーション状態//
	BYTE	byFpga1DbgIf;							// デバッグインタフェース//
	enum BFWENM_DEBUG_IF	eDebugIF;				// デバッグインタフェース種別//
	BYTE	byFpga1Rev;								// 通信制御FPGAバージョン//
	BYTE	byFpga2DbgIf;							// トレース対象MCU//
	enum BFWENM_MCU_TYPE	eMCUType;				// トレース対象MCU種別//
	BYTE	byFpga2Rev;								// トレース制御FPGAバージョン//
	BYTE	byFpgaVer;								// FPGA全体バージョン//
	BYTE	byTraceFlg;								// トレース有無フラグ//
} FFWE20_EINF_DATA;

// 外部フラッシュダウンロード情報格納構造体//
typedef struct FFW_EXTF_DATA_ST{
	WORD	wSectorNum;								// 外部ROM総セクタ数//
	BYTE	byChipErase;							// チップイレーズ情報//
	WORD	wChipEraseTimeout;						// チップ消去タイムアウト時間//
	DWORD	dwSectAddr[EXTF_FROM_SECTOR_MAX];			// 各セクタ開始アドレス//
	DWORD	dwSectSize[EXTF_FROM_SECTOR_MAX];			// 各セクタサイズ//
	BYTE	byEraseInfo[EXTF_FROM_SECTOR_MAX];			// 各セクタ消去情報//
	DWORD	dwStartAddr;							// 外部ROM開始アドレス//
	DWORD	dwEndAddr;								// 外部ROM終了アドレス//
	BYTE	byExtRomBfScript[MAX_PATH_SIZE];		// 外部ROMアクセス前スクリプトパス//
	BYTE	byExtRomAfScript[MAX_PATH_SIZE];		// 外部ROMアクセス後スクリプトパス//
	DWORD	dwMaxSectorSize;						// 最大セクタのサイズ(KB単位)//
	DWORD	dwWorkStart;							// ワークRAM先頭アドレス//
	WORD	wWorkSize;								// ワークRAMサイズ//
	BYTE	byWorkAccessSize;						// ワークRAMアクセスサイズ//
	BYTE	byWorkRamType;							// ワークRAMタイプ//
	BYTE	byExtRamBfScript[MAX_PATH_SIZE];		// 外部RAMアクセス前スクリプトパス//
	BYTE	byExtRamAfScript[MAX_PATH_SIZE];		// 外部RAMアクセス後スクリプトパス//
	DWORD	dwBuffStart;							// バッファ先頭アドレス//
	WORD	wBuffSize;								// バッファサイズ//
	WORD	wMakerID;								// メーカID情報//
	WORD	wDeviceID;								// デバイスID情報//
	WORD	wSectEraseTimeout;						// セクタイレーズタイムアウト時間//
	BYTE	byFastWriteMode;						// 高速書き込みモード情報//
	BYTE	byUnlock;								// ロックビット解除除法//
	BYTE	byConnect;								// 接続形態情報//
	BYTE	byWriteProgramType;						// 書き込みプログラムタイプ//
	WORD	wCommandAddr8_1;						// コマンド設定アドレス8bit(1st)//
	WORD	wCommandAddr8_2;						// コマンド設定アドレス8bit(2nd)//
	WORD	wCommandAddr16_1;						// コマンド設定アドレス16bit(1st)//
	WORD	wCommandAddr16_2;						// コマンド設定アドレス16bit(2nd)//
	WORD	wCommandAddr32_1;						// コマンド設定アドレス32bit(1st)//
	WORD	wCommandAddr32_2;						// コマンド設定アドレス32bit(2nd)//
	BYTE	byWriteProgram[EXTF_WRITE_PROGRAM_MAX];	// 書き込みプログラムデータ格納領域//
} FFW_EXTF_DATA[FFW_EXTF_DATA_NUM_MAX];

// フラッシュ領域サム値格納構造体//
typedef struct {
	BYTE	bySumLevel0;						// レベル0領域サム値//
	BYTE	bySumEml;							// レベル0＋EML領域サム値//
	BYTE	bySumFdt;							// レベル0＋FDT領域サム値//
	BYTE	bySumBlockA;						// ブロックA領域サム値//
} FFWE20_SUM_DATA;

// RevRxNo120910-001 Append Start//
// モニタCPU空間の設定データ格納構造体//
typedef struct {
	DWORD	dwCpudBlkNum;							// CPUDの設定データ個数//
	DWORD	dweadrAddr[CPUD_NUM_MAX];				// CPU空間の書き込みアドレス//
	enum	FFWENM_EACCESS_SIZE eAccessSize[CPUD_NUM_MAX];	// アクセスサイズ//
	DWORD	dwWriteData[CPUD_NUM_MAX];				// CPU空間の書き込みデータ//
	DWORD	dwDataMask[CPUD_NUM_MAX];				// CPU空間の書き込みデータのマスク//
} FFW_CPUD_DATA;
// RevRxNo120910-001 Append End//

// RevRxNo120910-001 Append Start//
//---------------------------
// MCU依存コマンド//
//---------------------------
// ターゲットMCU空間に設定するデータ格納構造体//
typedef struct {
	DWORD	dwSetDataNum;							// 設定データ情報総数//
	DWORD	dwSetTimming[MCUD_NUM_MAX];				// 設定タイミング情報//
	DWORD	dwSetCondition[MCUD_NUM_MAX];			// 設定条件//
	DWORD	dwmadrWriteAddr[MCUD_NUM_MAX];			// 設定アドレス//
	enum	FFWENM_MACCESS_SIZE eAccessSize[MCUD_NUM_MAX];	// アクセスサイズ//
	DWORD	dwWriteData[MCUD_NUM_MAX];				// 設定データ//
	DWORD	dwMaskData[MCUD_NUM_MAX];				// 設定データをマスクするためのマスクデータ//
} FFW_MCUD_DATA;
// RevRxNo120910-001 Append End//

//---------------------------
// プログラムダウンロード関連コマンド//
//---------------------------
// フラッシュメモリ初期化ブロック格納構造体//
typedef struct {
	DWORD	dwNum;								// フラッシュブロックアドレス数//
	DWORD	dwmadrBlkStart[FCLR_BLKNUM_MAX];	// フラッシュブロックの先頭アドレス//
} FFW_FCLR_DATA;

// フラッシュROMダウンロード時のチェックサム情報格納構造体//
typedef struct {
	BYTE	byCntMem;
	BYTE	byMemType[DWNP_FLASHTYPE_MAX];
	enum	FFWENM_CHKSUM eCheck[DWNP_FLASHTYPE_MAX];
	DWORD	pdwCheckSum[DWNP_FLASHTYPE_MAX];
} FFW_DWNP_CHKSUM_DATA;

//---------------------------
// ブレーク関連コマンド//
//---------------------------
// ソフトウェアブレークポイント格納構造体//
typedef struct {
	DWORD	dwSetNum;										// ソフトウェアブレークポイント設定点数//
	DWORD	dwmadrAddr[PB_SETNUM_MAX];						// ソフトウェアブレークポイントアドレス//
	BYTE	byCmdData[PB_SETNUM_MAX];						// 退避/復帰する命令コード//
	enum FFWENM_PB_FILLSTATE eFillState[PB_SETNUM_MAX];		// ソフトウェアブレークの設定状態//
} FFW_PB_DATA;

//---------------------------
// プログラム実行関連コマンド//
//---------------------------
// ユーザプログラム実行中の各種情報格納構造体//

// 2008.11.18 INSERT_BEGIN_E20RX600(+NN) {
// ステップ実行時のブレーク要因格納構造体//
typedef struct {
	DWORD	dwPc;									// ステップ実行後のPC値//
	enum FFWENM_RETURN_STEP	eRetunCodeStepFlag;		// ステップ実行開始時の命令コードがリターンサブルーチン命令であることを示す情報//
	enum FFWENM_SUBCALL_STEP eSubCallCodeStepFlag;	// ステップ実行開始時の命令コードがサブルーチンコール命令であることを示す情報//
	DWORD dwSubCallCodeSize;						// ステップ実行したサブルーチンコール命令コードのバイト数//
	enum FFWENM_STEP_RUN	eGoToRunState;			// 一定時間内にステップ実行が完了したかどうかを示す情報//
} FFWE20_STEP_DATA;
// 2008.11.18 INSERT_END_E20RX600 }

//---------------------------
// 指定ルーチン実行関連コマンド//
//---------------------------
// 指定ルーチン実行情報格納構造体//
typedef struct {
	DWORD	dwSrMode;							// 動作モード//
	DWORD	dwmadrBeforeRunFunctionAddr;		// ユーザプログラム実行開始前に実行する指定ルーチン先頭アドレス//
	DWORD	dwmadrAfterBreakFunctionAddr;		// ユーザプログラムブレーク後に実行する指定ルーチン先頭アドレス//
	DWORD	dwmadrBreakFunctionAddr;			// ユーザプログラムブレーク中に繰り返し実行する指定ルーチン先頭アドレス//
	DWORD	dwBreakInterruptFunctionIPL;		// ユーザプログラムブレーク中の割り込み優先レベル//
	DWORD	dwmadrRomWorkAreaStartAddr;			// 指定ルーチン呼び出しのためのプログラムコードを配置する領域//
	DWORD	dwmadrRamWorkAreaStartAddr;			// 指定ルーチン実行で使用するスタック領域の開始アドレス//
} FFW_SRM_DATA;

// RevRx6z4FpgaNo150525-001 Append Start
//---------------------------
// マルチコア関連コマンド//
//---------------------------
// 各コア実行開始条件格納構造体//
typedef struct {
    enum FFWENM_STARTADDR_SET eStartAddrSet;	// 実行開始アドレスの設定有無//
    DWORD dwmadrStartAddr;						// 実行開始アドレス//
  } FFW_RUN_DATA;
// RevRx6z4FpgaNo150525-001 Append End

//---------------------------
// メモリ操作関連コマンド//
//---------------------------
// ベリファイ結果格納構造体//
typedef struct {
	enum FFWENM_VERIFY_RESULT	eErrorFlag;		// ベリファイエラー発生有無情報//
	enum FFWENM_MACCESS_SIZE	eAccessSize;	// ベリファイエラー発生時のアクセスサイズ//
	DWORD	dwErrorAddr;						// ベリファイエラー発生時のアドレス//
	DWORD	dwErrorWriteData;					// ベリファイエラー発生時のライトデータ//
	DWORD	dwErrorReadData;					// ベリファイエラー発生時のリードデータ//
} FFW_VERIFYERR_DATA;

// メモリのコンペア結果格納構造体//
typedef struct {
	enum FFWENM_COMP_RESULT	eErrorFlag;			// コンペアエラー発生有無情報//
	BYTE	byErrorSrcData[MCMP_DATASIZE_MAX];	// コンペアエラー発生時の比較元データ//
	BYTE	byErrorDisData[MCMP_DATASIZE_MAX];	// コンペアエラー発生時の比較先データ//
	DWORD	dwErrorSrcAddr;						// コンペアエラー発生時の比較元アドレス//
	DWORD	dwErrorDisAddr;						// コンペアエラー発生時の比較先アドレス//
} FFW_COMPERR_DATA;

// メモリの検索結果格納構造体//
typedef struct {
	enum FFWENM_MSEARCH_RESULT	eSerchFlag;		// 検索結果//
	DWORD	dwMacthAddr;						// 一致アドレス//
} FFW_SEARCH_DATA;

//---------------------------
// RAMモニタ関連コマンド//
//---------------------------
// RAMモニタ領域格納構造体//
typedef struct {
	enum FFWENM_RRMB_ENABLE	eEnable[RRM_BLKNUM_MAX];	// RAM モニタの動作禁止/許可//
	DWORD	dwmadrBase[RRM_BLKNUM_MAX];					// RAM モニタ領域のベースアドレス//
} FFW_RRMB_DATA;

// RAMモニタデータ取得領域格納構造体//
typedef struct {
	DWORD	dwAreaNum;							// ブロック内のRAM モニタデータを参照する領域の個数//
	DWORD	dwStart[RRM_AREANUM_MAX];			// RAM モニタデータを参照する領域の開始アドレス//
	DWORD	dwEnd[RRM_AREANUM_MAX];				// RAM モニタデータを参照する領域の終了アドレス//
} FFW_RRMD_DATA;

// RAMモニタデータ格納構造体//
typedef struct {
	BYTE	byData[RRM_AREANUM_MAX];			// RAM モニタのデータ//
	BYTE	byAccess[RRM_AREANUM_MAX];			// アクセス履歴//
} FFW_RRMD_GET_DATA;

//////// FFW I/F関数の宣言 ////////

//----- FFW用宣言 -----//
#ifdef FFWE20_EXPORTS

// システムコマンド//
extern FFWE100_API	DWORD FFWE20Cmd_INIT(void* pSend, void* pRecv, void* pRecvH, void* pRecvPolling, void* pSetTimeOut,
									  void* pLock, void* pUnlock, void* pTransSize, DWORD* pdwGageCount, FFW_WINVER_DATA* pWinVer, void* pCallbackFromFfw2Target, char cMsgGageCount[]);
																						///< FFWモジュールの初期化//
extern FFWE100_API	DWORD FFWCmd_SetUSBTS(DWORD dwTransSize);							///< USB転送サイズの設定//
extern FFWE100_API	DWORD FFWCmd_GetUSBTS(DWORD* pdwTransSize);							///< USB転送サイズの取得//
extern FFWE100_API	DWORD FFWCmd_SetTMOUT(WORD wFfwTimeOut, WORD wBfwTimeOut);			///< FFW, BFW のタイムアウト時間の設定//
extern FFWE100_API	DWORD FFWCmd_GetTMOUT(WORD* pwFfwTimeOut, WORD* pwBfwTimeOut);		///< FFW, BFW のタイムアウト時間の参照//

// 2008.11.13 INSERT_BEGIN_E20RX600(+1) {
extern FFWE100_API	DWORD FFWE20Cmd_GetEINF(FFWE20_EINF_DATA* pEinf);					///< エミュレータ情報の取得//
// 2008.11.13 INSERT_END_E20RX600 }
// 2008.11.13 INSERT_BEGIN_E20RX600(+6) {
extern FFWE100_API	DWORD FFWE20Cmd_MONP_OPEN(DWORD dwTotalLength);						///< モニタプログラム(BFW)のダウンロード開始//
extern FFWE100_API	DWORD FFWE20Cmd_MONP_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW)の送信//
extern FFWE100_API	DWORD FFWE20Cmd_MONP_CLOSE(void);									///< モニタプログラム(BFW)のダウンロード終了//
extern FFWE100_API	DWORD FFWE2Cmd_BfwEmlUpdate(char szMcuFileName[], BOOL *bBfwUpdate);	///< BFW(LevelEML)の自動更新(E2E2Lite用)//
// RevRxNo180831-001 Append Line//
extern FFWE100_API	DWORD FFWE20Cmd_BfwEmlUpdate(char szMcuFileName[], BOOL *bBfwUpdate);	///< BFW(LevelEML)の自動更新(E1E20用)//
extern FFWE100_API	DWORD FFWE20Cmd_MONPFDT_OPEN(DWORD dwTotalLength);					///< モニタプログラム(BFW)のダウンロード開始//
extern FFWE100_API	DWORD FFWE20Cmd_MONPFDT_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW)の送信//
extern FFWE100_API	DWORD FFWE20Cmd_MONPFDT_CLOSE(void);								///< モニタプログラム(BFW)のダウンロード終了//
extern FFWE100_API	DWORD FFWE20Cmd_DCNF_OPEN(DWORD dwTotalLength, DWORD dwType);		///< DCSF のコンフィグレーション開始//
extern FFWE100_API	DWORD FFWE20Cmd_DCNF_SEND(DWORD dwLength, const BYTE* pbyDcsfBuff);	///< DCSF 論理データの送信//
extern FFWE100_API	DWORD FFWE20Cmd_DCNF_CLOSE(void);									///< DSCF のコンフィグレーション終了//
// 2008.11.13 INSERT_END_E20RX600 }
extern FFWE100_API	DWORD FFWCmd_GetErrorDetail(DWORD* pdwErrorCode, char szErrorMessage[ERROR_MESSAGE_NUM_MAX][ERROR_MESSAGE_LENGTH_MAX]);		// エラー詳細情報取得 //
// RevRxNo120910-001 Append Line//
extern FFWE100_API	DWORD FFWCmd_SetCPUD(const FFW_CPUD_DATA* pCpud);					///< MCUファイル用モニタCPU領域の設定//

extern FFWE100_API	DWORD FFWE20Cmd_EXTPWR (enum FFWENM_EXPWR_ENABLE eExPwrEnable,
											 enum FFWENM_EXPWR_VAL eExPwrVal);			///< ユーザ基板への電源供給を設定//

extern FFWE100_API	DWORD FFWCmd_SetEXTF(FFW_EXTF_DATA pExtfInfo);						///< 外部フラッシュダウンロード情報の設定//

// 2009.6.22 INSERT_BEGIN_E20RX600(+5) {
extern FFWE100_API	DWORD FFWE20Cmd_TRANS_FDT(void);									///< BFW FDTへの遷移//
extern FFWE100_API	DWORD FFWE20Cmd_TRANS_EML(void);									///< BFW EMLへの遷移//
extern FFWE100_API	DWORD FFWE20Cmd_TRANS_LV0(void);									///< BFW LV0への遷移//
extern FFWE100_API	DWORD FFWE20Cmd_SUMCHECK(FFWE20_SUM_DATA* pSum);
// 2009.6.22 INSERT_END_E20RX600 }

// RevRxNo120910-001 Append Line//
// MCU依存コマンド//
extern FFWE100_API	DWORD FFWE20Cmd_SetHotPlug(void);									///< ホットプラグ機能の設定/解除//
// RevRxNo120910-001 Append Start//
extern FFWE100_API	DWORD FFWMCUCmd_SetMCUD(const FFW_MCUD_DATA* pMcud);				///< ターゲットMCU空間のデータ設定//
extern FFWE100_API	DWORD FFWMCUCmd_GetMCUD(FFW_MCUD_DATA* pMcud);						///< ターゲットMCU空間のデータ参照//
// RevRxNo120910-001 Append End//

// リセットコマンド//
extern FFWE100_API	DWORD FFWMCUCmd_SREST(void);	///< システムリセット//
// 2008.11.18 INSERT_BEGIN_E20RX600(+2) {
extern FFWE100_API	DWORD FFWMCUCmd_REST(void);		///< ターゲットMCU のリセット//
extern FFWE100_API	DWORD FFWMCUCmd_CLR(void);			///< エミュレータの内部設定の初期化//
// 2008.11.18 INSERT_END_E20RX600 }

// プログラムダウンロード関連コマンド//
extern FFWE100_API	DWORD FFWMCUCmd_DWNP_OPEN(void);	///< ユーザプログラムのダウンロード開始//
extern FFWE100_API	DWORD FFWE20Cmd_DWNP_CLOSE(FFW_DWNP_CHKSUM_DATA* chksum);	///< プログラムのダウンロード終了//
extern FFWE100_API	DWORD FFWCmd_SetFCLR(const FFW_FCLR_DATA* pFclr);	///< フラッシュメモリ初期化ブロックの設定//
extern FFWE100_API	DWORD FFWCmd_GetFCLR(FFW_FCLR_DATA* pFclr);			///< フラッシュメモリ初期化ブロックの参照//

// ブレーク関連コマンド//
extern FFWE100_API	DWORD FFWMCUCmd_SetPB(enum FFWENM_PB_SW eSw, DWORD dwmadrAddr);			///< ソフトウェアブレークポイントの設定／解除//
extern FFWE100_API	DWORD FFWMCUCmd_GetPB(FFW_PB_DATA* pPbAddr);							///< ソフトウェアブレークポイントの参照//
extern FFWE100_API	DWORD FFWMCUCmd_ClrPB(void);											///< ソフトウェアブレークポイントの全解除//
// 2008.11.11 MODIFY_BEGIN_E20RX600(-2,+2) {
extern FFWE100_API	DWORD FFWMCUCmd_SetBM(DWORD dwBMode);									///< ブレークモードの設定//
extern FFWE100_API	DWORD FFWMCUCmd_GetBM(DWORD* pdwBMode);									///< ブレークモードの参照//
// 2008.11.11 MODIFY_END_E20RX600 }

// プログラム実行関連コマンド //
extern FFWE100_API	DWORD FFWMCUCmd_GO(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr);		///< プログラムのフリー実行//
extern FFWE100_API	DWORD FFWMCUCmd_GB(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr);		///< プログラムのブレーク付き実行//
extern FFWE100_API	DWORD FFWMCUCmd_RSTG(void);													///< プログラムのリセット付き実行//
extern FFWE100_API	DWORD FFWMCUCmd_GPB(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr,
										DWORD dwmadrBreakAddr);									///< プログラムのプログラムブレークポイント付き実行//
extern FFWE100_API	DWORD FFWMCUCmd_GPBA(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr,
										DWORD dwmadrBreakAddr, FFWE20_STEP_DATA* pStep);		///< プログラムの実行後ブレークポイント付き実行//
extern FFWE100_API	DWORD FFWMCUCmd_STOP(void);													///< プログラム実行停止//
// 2008.11.18 MODIFY_BEGIN_E20RX600(-2,+2) {
extern FFWE100_API	DWORD FFWMCUCmd_STEP(FFWE20_STEP_DATA* pStep);									///< プログラムのインストラクションステップ実行//
extern FFWE100_API	DWORD FFWMCUCmd_STEPOVER(FFWE20_STEP_DATA* pStepOver);							///< プログラムの関数単位ステップ実行//
// RevRxNo120910-001 Append Start//
extern FFWE100_API	DWORD FFWMCUCmd_SetSSST(void);														///< 連続ステップ処理の開始通知(未対応)//
extern FFWE100_API	DWORD FFWMCUCmd_SetSSEN(void);														///< 連続ステップ処理の終了通知(未対応)//
// RevRxNo120910-001 Append End//
// 2008.11.18 MODIFY_END_E20RX600 }

// RevRx6z4FpgaNo150525-001 Append Start
// マルチコア関連コマンド //
extern FFWE100_API	DWORD FFWMCUCmd_MultiGO(DWORD dwCoreNum, const DWORD* pdwCoreInfo, const FFW_RUN_DATA* pRun);	///< 複数コアのプログラム同時フリー実行//
extern FFWE100_API	DWORD FFWMCUCmd_MultiGB(DWORD dwCoreNum, const DWORD* pdwCoreInfo, const FFW_RUN_DATA* pRun);	///< 複数コアのプログラム同時ブレーク付き実行//
extern FFWE100_API	DWORD FFWMCUCmd_MultiSTOP(DWORD dwCoreNum, const DWORD* pdwCoreInfo);	///< 複数コアのプログラム実行同時停止//
// RevRx6z4FpgaNo150525-001 Append End

// 指定ルーチン実行関連コマンド//
extern FFWE100_API	DWORD FFWMCUCmd_SetSRM(const FFW_SRM_DATA* pSrMode);		///< 指定ルーチン実行のモード設定//
extern FFWE100_API	DWORD FFWMCUCmd_GetSRM(FFW_SRM_DATA* pSrMode);				///< 指定ルーチン実行のモード参照//

// RevRxNo120910-001 Append Start//
// プログラム実行中の設定変更コマンド //
extern FFWE100_API	DWORD FFWMCUCmd_SetRSST(void);						///< RUN中のイベントブレーク設定、トレース条件の設定開始通知(未対応)//
extern FFWE100_API	DWORD FFWMCUCmd_SetRSEN(void);						///< RUN中のイベントブレーク設定、トレース条件の設定終了通知(未対応)//
// RevRxNo120910-001 Append End//

// メモリ操作コマンド//
extern FFWE100_API	DWORD FFWMCUCmd_DUMP(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize,
										 BYTE *const pbyReadBuff);				///< メモリダンプ//
extern FFWE100_API	DWORD FFWMCUCmd_FILL(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
										 enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff, 
										 FFW_VERIFYERR_DATA* pVerifyErr);		///< 単一データのメモリ書き込み//
extern FFWE100_API	DWORD FFWMCUCmd_WRITE(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
										  enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, 
										  FFW_VERIFYERR_DATA* pVerifyErr);		///< メモリ書き込み//
extern FFWE100_API	DWORD FFWMCUCmd_MOVE(DWORD dwmadrSrcStartAddr, DWORD dwmadrSrcEndAddr, DWORD dwmadrDisStartAddr, 
									  enum FFWENM_VERIFY_SET eVerify, enum FFWENM_MACCESS_SIZE eAccessSize, 
									  FFW_VERIFYERR_DATA* pVerifyErr);			///< メモリコピー//
extern FFWE100_API	DWORD FFWMCUCmd_MCMP(DWORD dwmadrSrcStartAddr, DWORD dwmadrSrcEndAddr, DWORD dwmadrDisStartAddr, 
									  enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwCompDataSize, FFW_COMPERR_DATA* pCompError);	///< メモリ比較//
extern FFWE100_API	DWORD FFWMCUCmd_MSER(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_MSEARCH_EQU eSearchEqual, 
										 enum FFWENM_MSEARCH_DIR eSerchForward, enum FFWENM_MSEARCH_PATERN eSearchPatern, 
										 enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwSearchDataSize, const BYTE* pbySearchDataBuff, 
										 const BYTE* pbySearchMaskBuff, FFW_SEARCH_DATA* pSearchAns);			///< メモリ検索//
extern FFWE100_API	DWORD FFWMCUCmd_CDUMP(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, const BYTE* pbyReadBuff);		///< 命令コードの並びでダンプ//
extern FFWE100_API	DWORD FFWMCUCmd_CWRITE(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
																					const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);							///< 命令コードの並びで書き込み//

// RAMモニタコマンド //
extern FFWE100_API	DWORD FFWE20Cmd_SetRRMB(DWORD dwSetBlk, const FFW_RRMB_DATA* pRrmb);	///< RAM モニタベースアドレスの設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetRRMB(FFW_RRMB_DATA* pRrmb);							///< RAM モニタベースアドレスの参照//
extern FFWE100_API	DWORD FFWE20Cmd_SetRRMI(DWORD dwInitBlk, const DWORD dwInitInfo[]);		///< RAM モニタの初期化//
extern FFWE100_API	DWORD FFWE20Cmd_GetRRMD(DWORD dwGetBlk, const FFW_RRMD_DATA rrmd[], FFW_RRMD_GET_DATA ramMonData[]);	///< RAM モニタデータの参照//
extern FFWE100_API	DWORD FFWE20Cmd_ClrRRML(void);											///< RAM モニタでのロスト発生クリア//

// 非公開コマンド(デバッグ用)//
// 以下のコマンドは、F/W、H/Wデバッグ用コマンドである。NDEBとの組み合わせでのみ使用し、HEWデバッガでは使用しない。//
extern FFWE100_API	DWORD FFWE20Cmd_SetSELID(DWORD dwNum, const char cNumber[]);		///< エミュレータ製品のシリアル番号設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetSELID(DWORD* pdwNum, char cNumber[]);			///< エミュレータ製品のシリアル番号参照//
extern FFWE100_API	DWORD FFWE20Cmd_CPUW(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									  DWORD dwWriteCount, BYTE* pbyWriteBuff);	///< モニタCPU 空間のライト//
extern FFWE100_API	DWORD FFWE20Cmd_CPUR(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									  DWORD dwReadCount, BYTE *const pbyReadBuff);		///< モニタCPU 空間のリード//
extern FFWE100_API	DWORD FFWE20Cmd_CPUF(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									  DWORD dwWriteCount, const BYTE* pbyWriteBuff);	///< モニタCPU空間メモリへの単一データ書き込み//
// RevRxNo120910-001 Append Start//
extern FFWE100_API	DWORD FFWE20Cmd_CPURF(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									   DWORD dwWriteCount, DWORD dwWriteData, DWORD dwMaskData);	///< モニタCPU空間のリードモディファイライト//
// RevRxNo120910-001 Append End//
extern FFWE100_API	DWORD FFWE20Cmd_MONPALL_OPEN(DWORD dwTotalLength);						///< LEVEL0領域を含むモニタプログラム(BFW)のダウンロード開始//
extern FFWE100_API	DWORD FFWE20Cmd_MONPALL_SEND(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< LEVEL0領域をモニタプログラム(BFW)の送信//
extern FFWE100_API	DWORD FFWE20Cmd_MONPALL_CLOSE(void);									///< LEVEL0モニタプログラム(BFW)のダウンロード終了//

extern FFWE100_API	DWORD FFWE20Cmd_SetEMLMECLK(enum FFWENM_EMLMESCLK_SEL eTclk);				///< 全実行時間測定クロックの設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetEMLMECLK(enum FFWENM_EMLMESCLK_SEL* peTclk);				///< 全実行時間測定クロックの参照//
extern FFWE100_API	DWORD FFWE20Cmd_GetEMLMED(DWORD* pdwEmlTime, BYTE* pbyEmlOverflow, UINT64* pu64PcTime);	///< 全実行時間測定データの取得//
extern FFWE100_API	DWORD FFWE20Cmd_ClrEMLMED(void);											///< 全実行時間測定データのクリア//

// シリアル関連コマンド//
extern FFWE100_API	DWORD FFWE20Cmd_SetEXPORTDIR(WORD wPort,WORD wPortDir,WORD wPortType);		///< E1/E20ポート入出力方向の設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetEXPORTDIR(WORD wPort,WORD* pwPortDir,WORD* pwPortType);	///< E1/E20ポート入出力方向の参照//
extern FFWE100_API	DWORD FFWE20Cmd_SetEXPORTLVL(WORD wPort,WORD wPortLvl);		///< E1/E20ポートレベルの設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetEXPORTLVL(WORD wPort,WORD* pwPortLvl);	///< E1/E20ポートレベルの参照//
extern FFWE100_API	DWORD FFWE20Cmd_SetSCIBR(DWORD dwBaudrateVal);				///< E1/E20－MCU間シリアル通信ボーレートの設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetSCIBR(DWORD* pdwBaudrateValMin,DWORD* pdwBaudrateValMax);				///< E1/E20－MCU間シリアル通信ボーレートの参照//
extern FFWE100_API	DWORD FFWE20Cmd_SetSCICTRL(WORD wSciCtrl);		///< E1/E20シリアル通信情報の設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetSCICTRL(WORD* pwSciCtrl);		///< E1/E20シリアル通信情報の参照//
extern FFWE100_API	DWORD FFWE20Cmd_TGTMCUINF( WORD wMcuInf );					///< E1/E20ターゲットMCU情報番号の設定//
extern FFWE100_API	DWORD FFWE20Cmd_TGTMCUON();									///< E1/E20ターゲットMCUの起動//
extern FFWE100_API	DWORD FFWE20Cmd_SetEXDATAOUT(DWORD dwLen, BYTE* byData);	///< データのシリアル出力//
extern FFWE100_API	DWORD FFWE20Cmd_GetEXDATAIN(DWORD dwLen, BYTE* byData);		///< データのシリアル入力//
extern FFWE100_API	DWORD FFWE20Cmd_SetEXPORTPUP(WORD wPort,WORD wPortPup);		///< E1/E20ポートプルアップの設定//
extern FFWE100_API	DWORD FFWE20Cmd_GetEXPORTPUP(WORD wPort,WORD* pwPortPup);	///< E1/E20ポートプルアップの参照//
extern FFWE100_API	DWORD FFWE20Cmd_SetFRMINTERVAL(WORD wFrmInterval);			///< E1/E20シリアルデータ送信間のウェイトの設定//
extern FFWE100_API	DWORD FFWE20Cmd_SetPKTINTERVAL(WORD wPktInterval);			///< E1/E20パケットデータ送信間のウェイトの設定//
extern FFWE100_API	DWORD FFWE20Cmd_SetEXCONFIG(BYTE byLen,WORD *pwConfigData);	///< E1/E20入出力ポートのコンフィグレーション設定//
extern FFWE100_API	DWORD FFWE20Cmd_EXDATAOUTIN(WORD wSendLen, WORD wRcvLen, BYTE* byData);

// E20 USB FIFOバッファデータ取得コマンド（出荷検査用コマンド）//
extern FFWE100_API	DWORD FFWE20Cmd_GetD0FIFO( BYTE *pFifo );

//----- Target.DLL, NDEB用 -----//
#else

// システムコマンド//
__EXTERN__	DWORD (*FFWE20Cmd_INIT)(void* pSend, void* pRecv, void* pRecvH, void* pRecvPolling, void* pSetTimeOut,
								 void* pLock, void* pUnlock, void* pTransSize, DWORD* pdwGageCount, FFW_WINVER_DATA* pWinVer, void* pCallbackFromFfw2Target, char cMsgGageCount[]);	///< FFWモジュールの初期化//
__EXTERN__	DWORD (*FFWCmd_SetUSBTS)(DWORD dwTransSize);							///< USB転送サイズの設定//
__EXTERN__	DWORD (*FFWCmd_GetUSBTS)(DWORD* pdwTransSize);							///< USB転送サイズの取得//
__EXTERN__	DWORD (*FFWCmd_SetTMOUT)(WORD wFfwTimeOut, WORD wBfwTimeOut);			///< FFW, BFW のタイムアウト時間の設定//
__EXTERN__	DWORD (*FFWCmd_GetTMOUT)(WORD* pwFfwTimeOut, WORD* pwBfwTimeOut);		///< FFW, BFW のタイムアウト時間の参照//
// 2008.11.13 INSERT_BEGIN_E20RX600(+1) {
__EXTERN__	DWORD (*FFWE20Cmd_GetEINF)(FFWE20_EINF_DATA* pEinf);					///< エミュレータ情報の取得//
// 2008.11.13 INSERT_END_E20RX600 }
// 2008.11.13 INSERT_BEGIN_E20RX600(+6) {
__EXTERN__	DWORD (*FFWE20Cmd_MONP_OPEN)(DWORD dwTotalLength);						///< モニタプログラム(BFW)のダウンロード開始//
__EXTERN__	DWORD (*FFWE20Cmd_MONP_SEND)(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW)の送信//
__EXTERN__	DWORD (*FFWE20Cmd_MONP_CLOSE)(void);									///< モニタプログラム(BFW)のダウンロード終了//
__EXTERN__	DWORD (*FFWE2Cmd_BfwEmlUpdate)(char szMcuFileName[], BOOL *bBfwUpdate);		///< BFW(LevelEML)の自動更新(E2E2Lite用)//
// RevRxNo180831-001 Append Line//
__EXTERN__	DWORD (*FFWE20Cmd_BfwEmlUpdate)(char szMcuFileName[], BOOL *bBfwUpdate);	///< BFW(LevelEML)の自動更新(E1E20用)//
__EXTERN__	DWORD (*FFWE20Cmd_MONPFDT_OPEN)(DWORD dwTotalLength);					///< モニタプログラム(BFW)のダウンロード開始//
__EXTERN__	DWORD (*FFWE20Cmd_MONPFDT_SEND)(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< モニタプログラム(BFW)の送信//
__EXTERN__	DWORD (*FFWE20Cmd_MONPFDT_CLOSE)(void);									///< モニタプログラム(BFW)のダウンロード終了//
__EXTERN__	DWORD (*FFWE20Cmd_DCNF_OPEN)(DWORD dwTotalLength, DWORD dwType);		///< DCSF のコンフィグレーション開始//
__EXTERN__	DWORD (*FFWE20Cmd_DCNF_SEND)(DWORD dwLength, const BYTE* pbyDcsfBuff);	///< DCSF 論理データの送信//
__EXTERN__	DWORD (*FFWE20Cmd_DCNF_CLOSE)(void);									///< DSCF のコンフィグレーション終了//
// 2008.11.13 INSERT_END_E20RX600 }
__EXTERN__	DWORD (*FFWE20Cmd_EXTPWR)(enum FFWENM_EXPWR_ENABLE eExPwrEnable,
											 enum FFWENM_EXPWR_VAL eExPwrVal);		///< ユーザ基板への電源供給を設定//
__EXTERN__	DWORD (*FFWCmd_GetErrorDetail)( DWORD* pdwErrorCode, char szErrorMessage[ERROR_MESSAGE_NUM_MAX][ERROR_MESSAGE_LENGTH_MAX] );	// エラー詳細情報取得 //
// RevRxNo120910-001 Append Line//
__EXTERN__	DWORD (*FFWCmd_SetCPUD)(const FFW_CPUD_DATA* pCpud);					///< MCUファイル用モニタCPU領域の設定//

//__EXTERN__	DWORD (*FFWCmd_SetCPUD)(const FFW_CPUD_DATA* pCpud);					///< MCU ファイル用モニタCPU 領域の設定//
__EXTERN__	DWORD (*FFWCmd_SetEXTF)(FFW_EXTF_DATA pExtfInfo);						///< 外部フラッシュダウンロード情報の設定//
// 2009.6.22 INSERT_BEGIN_E20RX600(+2) {
__EXTERN__	DWORD (*FFWE20Cmd_TRANS_FDT)(void);										///< BFW FDTへの遷移//
__EXTERN__	DWORD (*FFWE20Cmd_TRANS_EML)(void);										///< BFW EMLへの遷移//
__EXTERN__	DWORD (*FFWE20Cmd_TRANS_LV0)(void);										///< BFW LV0への遷移//
__EXTERN__	DWORD (*FFWE20Cmd_SUMCHECK)(FFWE20_SUM_DATA* pSum);				///< 指定領域のサムチェック//
// 2009.6.22 INSERT_END_E20RX600 }

// MCU依存コマンド//
__EXTERN__	DWORD (*FFWE20Cmd_SetHotPlug)(void);									///< ホットプラグ機能の設定/解除//
// RevRxNo120910-001 Append Start//
__EXTERN__	DWORD (*FFWMCUCmd_SetMCUD)(const FFW_MCUD_DATA* pMcud);					///< ターゲットMCU空間のデータ設定//
__EXTERN__	DWORD (*FFWMCUCmd_GetMCUD)(FFW_MCUD_DATA* pMcud);						///< ターゲットMCU空間のデータ参照//
// RevRxNo120910-001 Append End//

// リセットコマンド//
__EXTERN__	DWORD (*FFWMCUCmd_SREST)(void);	///< システムリセット//
// 2008.11.18 INSERT_BEGIN_E20RX600(+2) {
__EXTERN__	DWORD (*FFWMCUCmd_REST)(void);		///< ターゲットMCU のリセット//
__EXTERN__	DWORD (*FFWMCUCmd_CLR)(void);		///< エミュレータの内部設定の初期化//
// 2008.11.18 INSERT_END_E20RX600 }

// プログラムダウンロード関連コマンド//
__EXTERN__	DWORD (*FFWMCUCmd_DWNP_OPEN)(void);	///< ユーザプログラムのダウンロード開始//
//__EXTERN__	DWORD (*FFWMCUCmd_DWNP_CLOSE)(void);	///< プログラムのダウンロード終了//
__EXTERN__	DWORD (*FFWE20Cmd_DWNP_CLOSE)(FFW_DWNP_CHKSUM_DATA* chksum);	///< プログラムのダウンロード終了//
__EXTERN__	DWORD (*FFWCmd_SetFCLR)(const FFW_FCLR_DATA* pFclr);	///< フラッシュメモリ初期化ブロックの設定//
__EXTERN__	DWORD (*FFWCmd_GetFCLR)(FFW_FCLR_DATA* pFclr);			///< フラッシュメモリ初期化ブロックの参照//

// ブレーク関連コマンド//
__EXTERN__	DWORD (*FFWMCUCmd_SetPB)(enum FFWENM_PB_SW eSw, DWORD dwmadrAddr);				///< ソフトウェアブレークポイントの設定／解除//
__EXTERN__	DWORD (*FFWMCUCmd_GetPB)(FFW_PB_DATA* pPbAddr);								///< ソフトウェアブレークポイントの参照//
__EXTERN__	DWORD (*FFWMCUCmd_ClrPB)(void);												///< ソフトウェアブレークポイントの全解除//
// 2008.11.11 MODIFY_BEGIN_E20RX600(-2,+2) {
__EXTERN__	DWORD (*FFWMCUCmd_SetBM)(DWORD dwBMode);									///< ブレークモードの設定//
__EXTERN__	DWORD (*FFWMCUCmd_GetBM)(DWORD* pdwBMode);									///< ブレークモードの参照//
// 2008.11.11 MODIFY_END_E20RX600 }

// プログラム実行関連コマンド//
__EXTERN__	DWORD (*FFWMCUCmd_GO)(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr);	///< プログラムのフリー実行//
__EXTERN__	DWORD (*FFWMCUCmd_GB)(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr);	///< プログラムのブレーク付き実行//
__EXTERN__	DWORD (*FFWMCUCmd_RSTG)(void);													///< プログラムのリセット付き実行//
__EXTERN__	DWORD (*FFWMCUCmd_GPB)(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr,
										DWORD dwmadrBreakAddr);								///< プログラムのプログラムブレークポイント付き実行//
__EXTERN__	DWORD (*FFWMCUCmd_GPBA)(enum FFWENM_STARTADDR_SET eStartAddrSet, DWORD dwmadrStartAddr,
										DWORD dwmadrBreakAddr, FFWE20_STEP_DATA* pStep);	///< プログラムの実行後ブレークポイント付き実行//
__EXTERN__	DWORD (*FFWMCUCmd_STOP)(void);													///< プログラム実行停止//
// 2008.11.11 MODIFY_BEGIN_E20RX600(-2,+2) {
__EXTERN__	DWORD (*FFWMCUCmd_STEP)(FFWE20_STEP_DATA* pStep);								///< プログラムのインストラクションステップ実行//
__EXTERN__	DWORD (*FFWMCUCmd_STEPOVER)(FFWE20_STEP_DATA* pStepOver);						///< プログラムの関数単位ステップ実行//
// 2008.11.11 MODIFY_END_E20RX600 }
// RevRxNo120910-001 Append Start//
__EXTERN__	DWORD (*FFWMCUCmd_SetSSST)(void);														///< 連続ステップ処理の開始通知(未対応)//
__EXTERN__	DWORD (*FFWMCUCmd_SetSSEN)(void);														///< 連続ステップ処理の終了通知(未対応)//
// RevRxNo120910-001 Append End//

// RevRx6z4FpgaNo150525-001 Append Start
// マルチコア関連コマンド //
__EXTERN__	DWORD (*FFWMCUCmd_MultiGO)(DWORD dwCoreNum, const DWORD* pdwCoreInfo, const FFW_RUN_DATA* pRun);	///< 複数コアのプログラム同時フリー実行//
__EXTERN__	DWORD (*FFWMCUCmd_MultiGB)(DWORD dwCoreNum, const DWORD* pdwCoreInfo, const FFW_RUN_DATA* pRun);	///< 複数コアのプログラム同時ブレーク付き実行//
__EXTERN__	DWORD (*FFWMCUCmd_MultiSTOP)(DWORD dwCoreNum, const DWORD* pdwCoreInfo);	///< 複数コアのプログラム実行同時停止//
// RevRx6z4FpgaNo150525-001 Append End

// 指定ルーチン実行関連コマンド//
__EXTERN__	DWORD (*FFWMCUCmd_SetSRM)(const FFW_SRM_DATA* pSrMode);				///< 指定ルーチン実行のモード設定//
__EXTERN__	DWORD (*FFWMCUCmd_GetSRM)(FFW_SRM_DATA* pSrMode);					///< 指定ルーチン実行のモード参照//

// RevRxNo120910-001 Append Start//
// プログラム実行中の設定変更コマンド //
__EXTERN__	DWORD (*FFWMCUCmd_SetRSST)(void);						///< RUN中のイベントブレーク設定、トレース条件の設定開始通知(未対応)//
__EXTERN__	DWORD (*FFWMCUCmd_SetRSEN)(void);						///< RUN中のイベントブレーク設定、トレース条件の設定終了通知(未対応)//
// RevRxNo120910-001 Append End//

// メモリ操作コマンド//
__EXTERN__	DWORD (*FFWMCUCmd_DUMP)(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_MACCESS_SIZE eAccessSize,
										 BYTE *const pbyReadBuff);				///< メモリダンプ//
__EXTERN__	DWORD (*FFWMCUCmd_FILL)(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
										 enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwWriteDataSize, const BYTE* pbyWriteBuff, 
										 FFW_VERIFYERR_DATA* pVerifyErr);		///< 単一データのメモリ書き込み//
__EXTERN__	DWORD (*FFWMCUCmd_WRITE)(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
										  enum FFWENM_MACCESS_SIZE eAccessSize, const BYTE* pbyWriteBuff, 
										  FFW_VERIFYERR_DATA* pVerifyErr);		///< メモリ書き込み//
__EXTERN__	DWORD (*FFWMCUCmd_MOVE)(DWORD dwmadrSrcStartAddr, DWORD dwmadrSrcEndAddr, DWORD dwmadrDisStartAddr, 
									enum FFWENM_VERIFY_SET eVerify, enum FFWENM_MACCESS_SIZE eAccessSize, 
									FFW_VERIFYERR_DATA* pVerifyErr);			///< メモリコピー//
__EXTERN__	DWORD (*FFWMCUCmd_MCMP)(DWORD dwmadrSrcStartAddr, DWORD dwmadrSrcEndAddr, DWORD dwmadrDisStartAddr, 
									enum FFWENM_MACCESS_SIZE eAccessSize, DWORD dwCompDataSize, FFW_COMPERR_DATA* pCompError);	///< メモリ比較//
__EXTERN__	DWORD (*FFWMCUCmd_MSER)(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_MSEARCH_EQU eSearchEqual, 
										 enum FFWENM_MSEARCH_DIR eSerchForward, enum FFWENM_MSEARCH_PATERN eSearchPatern, 
										 enum FFWENM_MACCESS_SIZE eAccessSize,  DWORD dwSearchDataSize, const BYTE* pbySearchDataBuff, 
										 const BYTE* pbySearchMaskBuff, FFW_SEARCH_DATA* pSearchAns);			///< メモリ検索//
__EXTERN__	DWORD (*FFWMCUCmd_CDUMP)(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, const BYTE* pbyReadBuff);		///< 命令コードの並びでダンプ//
__EXTERN__	DWORD (*FFWMCUCmd_CWRITE)(DWORD dwmadrStartAddr, DWORD dwmadrEndAddr, enum FFWENM_VERIFY_SET eVerify, 
																					const BYTE* pbyWriteBuff, FFW_VERIFYERR_DATA* pVerifyErr);							///< 命令コードの並びで書き込み//

// RAMモニタ関連コマンド//
__EXTERN__	DWORD (*FFWE20Cmd_SetRRMB)(DWORD dwSetBlk, const FFW_RRMB_DATA* pRrmb);		///< RAM モニタベースアドレスの設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetRRMB)(FFW_RRMB_DATA* pRrmb);							///< RAM モニタベースアドレスの参照//
__EXTERN__	DWORD (*FFWE20Cmd_SetRRMI)(DWORD dwInitBlk, const DWORD dwInitInfo[]);		///< RAM モニタの初期化//
__EXTERN__	DWORD (*FFWE20Cmd_GetRRMD)(DWORD dwGetBlk, const FFW_RRMD_DATA rrmd[], FFW_RRMD_GET_DATA ramMonData[]);	///< RAM モニタデータの参照//
__EXTERN__	DWORD (*FFWE20Cmd_ClrRRML)(void);											///< RAM モニタでのロスト発生クリア//

// シリアルサポート関連コマンド//
__EXTERN__	DWORD (*FFWE20Cmd_SetEXPORTDIR)(WORD wPort,WORD wPortDir,WORD wPortType);		///< E1/E20ポート入出力方向の設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetEXPORTDIR)(WORD wPort,WORD* pwPortDir,WORD* pwPortType);	///< E1/E20ポート入出力方向の参照//
__EXTERN__	DWORD (*FFWE20Cmd_SetEXPORTLVL)(WORD wPort,WORD wPortLvl);		///< E1/E20ポートレベルの設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetEXPORTLVL)(WORD wPort,WORD* pwPortLvl);	///< E1/E20ポートレベルの参照//
__EXTERN__	DWORD (*FFWE20Cmd_SetSCIBR)(DWORD dwBaudrateVal);					///< E1/E20－MCU間シリアル通信ボーレートの設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetSCIBR)(DWORD* pdwBaudrateValMin,DWORD* pdwBaudrateValMax);				///< E1/E20－MCU間シリアル通信ボーレートの参照//
__EXTERN__	DWORD (*FFWE20Cmd_SetSCICTRL)(WORD wSciCtrl);			///< E1/E20シリアル通信情報の設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetSCICTRL)(WORD* pwSciCtrl);		///< E1/E20シリアル通信情報の参照//
__EXTERN__	DWORD (*FFWE20Cmd_TGTMCUINF)(WORD wMcuInf);						///< E1/E20ターゲットMCU情報番号の設定//
__EXTERN__	DWORD (*FFWE20Cmd_TGTMCUON)(void);								///< E1/E20ターゲットMCUの起動//
__EXTERN__	DWORD (*FFWE20Cmd_SetEXDATAOUT)(DWORD dwLen, BYTE* byData);		///< データのシリアル出力//
__EXTERN__	DWORD (*FFWE20Cmd_GetEXDATAIN)(DWORD dwLen, BYTE* byData);		///< データのシリアル入力//
__EXTERN__	DWORD (*FFWE20Cmd_SetEXPORTPUP)(WORD wPort,WORD wPortPup);		///< E1/E20ポートプルアップの設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetEXPORTPUP)(WORD wPort,WORD* pwPortPup);	///< E1/E20ポートプルアップの参照//
__EXTERN__	DWORD (*FFWE20Cmd_SetFRMINTERVAL)(WORD wFrmInterval);			///< E1/E20シリアルデータ送信間のウェイトの設定//
__EXTERN__	DWORD (*FFWE20Cmd_SetPKTINTERVAL)(WORD wPktInterval);			///< E1/E20パケットデータ送信間のウェイトの設定//
__EXTERN__	DWORD (*FFWE20Cmd_SetEXCONFIG)(BYTE byLen,WORD *pwConfigData);	///< E1/E20入出力ポートのコンフィグレーションの設定//
__EXTERN__	DWORD (*FFWE20Cmd_EXDATAOUTIN) (WORD wSendLen, WORD wRcvLen, BYTE* byData); ///< データのシリアル送信直後のシリアル受信//

// 非公開コマンド(デバッグ用)//
__EXTERN__	DWORD (*FFWE20Cmd_SetEMLMECLK)(enum FFWENM_EMLMESCLK_SEL eTclk);				///< 全実行時間測定クロックの設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetEMLMECLK)(enum FFWENM_EMLMESCLK_SEL* peTclk);				///< 全実行時間測定クロックの参照//
__EXTERN__	DWORD (*FFWE20Cmd_GetEMLMED)(DWORD* pdwEmlTime, BYTE* pbyEmlOverflow, UINT64* pu64PcTime);	///< 全実行時間測定データの取得//
__EXTERN__	DWORD (*FFWE20Cmd_ClrEMLMED)(void);												///< 全実行時間測定データのクリア//

__EXTERN__	DWORD (*FFWE20Cmd_SetSELID)(DWORD dwNum, const char cNumber[]);		///< エミュレータ製品のシリアル番号設定//
__EXTERN__	DWORD (*FFWE20Cmd_GetSELID)(DWORD* pdwNum, char cNumber[]);			///< エミュレータ製品のシリアル番号参照//
__EXTERN__	DWORD (*FFWE20Cmd_CPUW)(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									  DWORD dwWriteCount, const BYTE* pbyWriteBuff);	///< モニタCPU 空間のライト//
__EXTERN__	DWORD (*FFWE20Cmd_CPUR)(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									  DWORD dwReadCount, BYTE *const pbyReadBuff);		///< モニタCPU 空間のリード//
__EXTERN__	DWORD (*FFWE20Cmd_CPUF)(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									  DWORD dwWriteCount, const BYTE* pbyWriteBuff);	///< モニタCPU空間メモリへの単一データ書き込み//
// RevRxNo120910-001 Append Start//
__EXTERN__	DWORD (*FFWE20Cmd_CPURF)(DWORD dweadrStartAddr, enum FFWENM_EACCESS_SIZE eAccessSize, 
									   DWORD dwWriteCount, DWORD dwWriteData, DWORD dwMaskData);	///< モニタCPU空間のリードモディファイライト//
// RevRxNo120910-001 Append End//
__EXTERN__	DWORD (*FFWE20Cmd_MONPALL_OPEN)(DWORD dwTotalLength);							///< LEVEL0領域を含むモニタプログラム(BFW)のダウンロード開始//
__EXTERN__	DWORD (*FFWE20Cmd_MONPALL_SEND)(DWORD dweadrAddr, DWORD dwLength, const BYTE* pbyBfwBuff);	///< LEVEL0領域を含むモニタプログラム(BFW)の送信//
__EXTERN__	DWORD (*FFWE20Cmd_MONPALL_CLOSE)(void);										///< LEVEL0領域を含むモニタプログラム(BFW)のダウンロード終了//
__EXTERN__	DWORD (*FFWE20Cmd_GetD0FIFO)( BYTE *pFifo );

#endif

#ifdef	__cplusplus
	};
#endif

#endif	// __FFWE20_H__
