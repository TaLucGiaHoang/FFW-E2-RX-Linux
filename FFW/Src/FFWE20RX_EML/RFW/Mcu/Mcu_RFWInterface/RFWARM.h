/**
 * @file RFWARM.h
 * @brief E2エミュレータRFW I/F(MCU依存)ヘッダファイル
 * @author H.Hatahara, TSSR Y.Kodama
 * @author Copyright (C) 2015-2016 Renesas Electronics Corporation and
 * @author Renesas System Design Corp. All rights reserved.
 * @date 2016/07/14
 */
/*
■改定履歴
・2015/02/03 初版作成 我妻
・2015/03/05 DEFINE定義を新規追加。RFW I/F仕様書 1版 2015/02/26の情報を反映。 H.Hatahara
・2015/03/10 EXTERNができていない問題を解決。 H.Hatahara
・2015/10/01 RFW.hとの重複記述最適化。Y.Agatsuma
・2016/07/14 #267 [課題]ヘッダ定義の誤り TSSR Y.Kodama
*/
#ifndef	__RFWARM_H__
#define	__RFWARM_H__

#define DBG_DATA_IFDAPID_MAX	(16)
#define DBGCORE_PERFUNC_MAX		(8)


//-----------------------------------------------------------------------
// DEFINE定義
//-----------------------------------------------------------------------
// --- DEFINE定義 MCU依存コマンド --------
////// RFWCARM_GetDBG
//bySysExTrc
#define RF_NOT_SUPPORT			(0x00)		// 外部トレース機能がサポートされていない。
#define RF_SUPPORT				(0x01)		// 外部トレース機能がサポートされている。
////// RFWCARM_GetDBGCore
// dwBPType
#define RF_ADR_MATCH_BIT		(0x01)		// アドレス一致
#define RF_ADR_UNMATCH_BIT		(0x02)		// アドレス不一致
#define RF_CTXID_MATCH_BIT		(0x04)		// コンテキストID一致
#define RF_VMID_MATCH_BIT		(0x08)		// VMID一致
#define RF_ADR_RANGE_MASK_BIT	(0x80)		// アドレスレンジマスク
// byBPMode
#define RF_DBG_BPMODE_CM		(0x00)		// Cortex-Mコア
#define RF_DBG_BPMODE_MODE1		(0x01)		// ユーザ、システム、スーパバイザのみ/特権モードのみ/ユーザモードのみ/すべてのモードを選択可能
#define RF_DBG_BPMODE_MODE2			(0x02)		// PL0/PL1/PL2のすべての組み合わせが選択可能
// byWPMode
#define RF_DBG_WPMODE_CM		(0x00)		// Cortex-Mコア
#define RF_DBG_WPMODE_MODE1		(0x01)		// ユーザ、システム、スーパバイザのみ/特権モードのみ/ユーザモードのみ/すべてのモードを選択可能
#define RF_DBG_WPMODE_MODE2			(0x02)		// PL0/PL1/PL2のすべての組み合わせが選択可能
////// RFWCARM_GetStatus
// dwStatKind
#define RF_STATKIND_EML_BIT		(0x01)		// EML情報を取得
#define RF_STATKIND_CPU_BIT		(0x02)		// CPU情報を取得
#define RF_STATKIND_TRC_BIT		(0x04)		// TRC情報を取得
// byInstInfo
#define RF_THUMB_CODE			(0x00)		// Thumb命令セットを選択中
#define RF_ARM_CODE				(0x01)		// ARM命令セットを選択中
// StatCPU
#define RF_STAT_NORMAL			(0x00)		// 正常動作状態(Cortex-A,R,M)
#define RF_STAT_RESET			(0x01)		// リセット状態(Cortex-A,R,M)
#define RF_STAT_STANDBY			(0x02)		// スタンバイ状態(Cortex-A,R)
#define RF_STAT_PDOWN			(0x03)		// パワーダウン状態(Cortex-A,R)
#define RF_STAT_STALL			(0x04)		// ストール状態(Cortex-M)
#define RF_STAT_LOCKUP			(0x05)		// ロックアップ状態(Cortex-M)
#define RF_STAT_SLEEP			(0x06)		// スリープ状態(Cortex-M)
// byStatUVCC1
#define RF_STAT_UVCC1_OFF		(0x00)		// 電源OFF(予約)
#define RF_STAT_UVCC1_ON		(0x01)		// 電源ON
// byStatUVCC2
#define RF_STAT_UVCC2_OFF		(0x00)		// 電源OFF(予約)
#define RF_STAT_UVCC2_ON		(0x01)		// 電源ON
// byStatUCON
#define RF_STAT_TRGT_NCON		(0x00)		// 未接続(予約)
#define RF_STAT_TRGT_CON		(0x01)		// 接続
// dwTraceInfo
#define RF_STATEML_TRCREC_BIT	(0x00000001)	// トレース計測
#define RF_STATEML_TRCFULL_BIT	(0x00000002)	// トレースフル状態
// StatTRC
// dwTraceInfo
#define RF_STATTRC_REC_BIT		(0x01)		// トレース計測状態

// --- DEFINE定義 プログラム実行関連コマンド----
////// RFWCARM_GetBrkf
// dwBreakFact
#define RF_BRKF_OPTTRCF_BIT		(0x80000000)	// ブレーク要因 オプショントレースフルイベント(予約)
#define RF_BRKF_APROBE_BIT		(0x40000000)	// ブレーク要因 アシストプローブイベント(予約)
#define RF_BRKF_SWPOINT_BIT		(0x00000400)	// ブレーク要因 同期ウォッチポイントデバッグイベント
#define RF_BRKF_OSLOCK_BIT		(0x00000100)	// ブレーク要因 OSロック解除キャッチデバッグイベント
#define RF_BRKF_VECT_BIT		(0x00000020)	// ブレーク要因 ベクタキャッチデバッグイベント
#define RF_BRKF_EXTDEBUG_BIT	(0x00000010)	// ブレーク要因 外部デバッグ要求イベント
#define RF_BRKF_BKPT_BIT		(0x00000008)	// ブレーク要因 BKPT命令デバッグイベント
#define RF_BRKF_ASWPOINT_BIT	(0x00000004)	// ブレーク要因 非同期ウォッチポイントデバッグイベント
#define RF_BRKF_BRKPOINT_BIT	(0x00000002)	// ブレーク要因 ブレークポイントデバッグイベント
#define RF_BRKF_HALT_BIT		(0x00000001)	// ブレーク要因 ホールト要求デバッグイベント

// --- DEFINE定義 トレース関連コマンド--------
////// RFWCARM_GetTrcFuncInfo
// byInfoType
#define RF_SYS_CORE				(0x00)		// コアの情報を指定
#define RF_SYS_CLUSTER			(0x01)		// クラスタ情報を指定
#define RF_SYS_SYSTEM			(0x02)		// システム(MCU全体)の情報を指定
// byTrcDataType
#define RF_INST_TRC_BIT			(0x01)		// 命令トレース
#define RF_DATA_TRC_BIT			(0x02)		// データトレース
#define RF_BUS_TRC_BIT			(0x04)		// バストレース
#define RF_SOFT_TRC_BIT			(0x08)		// ソフトトレース
// byTrcMode
#define RF_REALTIME_TRC_BIT		(0x01)		// リアルタイムトレース
#define RF_NON_REALTIME_TRC_BIT (0x02)		// ノンリアルタイムトレース
////// RFWCARM_GetIntrTrcInfo
// byBuffType
#define RF_TRC_INTR_ETB_BIT		(0x01)		// 専用バッファ(ETB/ETF)
#define RF_TRC_INTR_MTB_BIT		(0x02)		// ユーザメモリ(MTB)
#define RF_TRC_INTR_ETR_BIT		(0x04)		// ユーザメモリ(ETR)
// byBuffMode
#define RF_TRC_FULL_BIT			(0x01)		// フル
#define RF_TRC_FREE_BIT			(0x02)		// フリー
#define RF_TRC_FULLBRK_BIT		(0x04)		// フルブレーク
//////  RFWCARM_GetExtrTrcInfo
// byTrcOutType
#define RF_TRC_EXTR_TPIU		(0x01)		// パラレル端子出力(TPIU)
#define RF_TRC_EXTR_ETR			(0x02)		// デバイス内資源への出力(ETR)
// bySwoTxMode
#define RF_SWO_UART_BIT			(0x01)		// UART
#define RF_SWO_MANCHESTER_BIT	(0x02)		// Manchester
////// RFWCARM_SetSTrc
// byEnable
#define RF_STRC_OFF				(0x00)		// 無効
#define RF_STRC_ON				(0x01)		// 有効
// byTimestamp
#define RF_STRC_TIMESTAMP_OFF	(0x00)		// 無効
#define RF_STRC_TIMESTAMP_ON	(0x01)		// 有効

// --- DEFINE定義 イベント関連コマンド--------
////// RFWCARM_SetVCatchEv
////// ARMv7-AR
#define RF_VC_R_BIT				(0x00000001)	// リセットベクタキャッチ
#define RF_VC_SU_BIT			(0x00000002)	// 未定義命令ベクタキャッチ
#define RF_VC_SS_BIT			(0x00000004)	// SVC(スーパーバイザコール)ベクタキャッチ
#define RF_VC_SP_BIT			(0x00000008)	// プリフェッチアボートベクタキャッチ
#define RF_VC_SD_BIT			(0x00000010)	// データアボートベクタキャッチ
#define RF_VC_SI_BIT			(0x00000040)	// IRQ割り込みベクタキャッチ
#define RF_VC_SF_BIT			(0x00000080)	// FIQ割り込みベクタキャッチ
#define RF_VC_MS_BIT			(0x00000400)	// セキュアモニタコールベクタキャッチ
#define RF_VC_MP_BIT			(0x00000800)	// プリフェッチアボートベクタキャッチ（モニタモード）
#define RF_VC_MD_BIT			(0x00001000)	// データアボートベクタキャッチ（モニタモード）
#define RF_VC_MI_BIT			(0x00004000)	// IRQ割り込みベクタキャッチ（モニタモード）
#define RF_VC_MF_BIT			(0x00008000)	// FIQ割り込みベクタキャッチ（モニタモード）
#define RF_VC_NSHU_BIT			(0x00020000)	// 未定義命令ベクタキャッチ（Hypモード）
#define RF_VC_NSHC_BIT			(0x00040000)	// HVC(ハイパーバイザコール)ベクタキャッチ
#define RF_VC_NSHP_BIT			(0x00080000)	// プリフェッチアボートベクタキャッチ（Hypモード）
#define RF_VC_NSHD_BIT			(0x00100000)	// データアボートベクタキャッチ（Hypモード）
#define RF_VC_NSHE_BIT			(0x00200000)	// ハイパーバイザモード入出ベクタキャッチ
#define RF_VC_NSHI_BIT			(0x00400000)	// IRQ割り込みベクタキャッチ（Hypモード）
#define RF_VC_NSHF_BIT			(0x00800000)	// FIQ割り込みベクタキャッチ（Hypモード）
#define RF_VC_NSU_BIT			(0x02000000)	// 未定義命令ベクタキャッチ（Non-Secure）
#define RF_VC_NSS_BIT			(0x04000000)	// SVC(スーパーバイザコール)ベクタキャッチ（Non-Secure）
#define RF_VC_NSP_BIT			(0x08000000)	// プリフェッチアボートベクタキャッチ（Non-Secure）
#define RF_VC_NSD_BIT			(0x10000000)	// データアボートベクタキャッチ（Non-Secure）
#define RF_VC_NSI_BIT			(0x40000000)	// IRQ割り込みベクタキャッチ（Non-Secure） 
#define RF_VC_NSF_BIT			(0x80000000)	// FIQ割り込みベクタキャッチ（Non-Secure）
////// ARMv7-M, ARMv6-M
#define RF_VC_CORERESET_BIT		(0x00000001)	// リセットベクタキャッチ
#define RF_VC_MMERR_BIT			(0x00000010)	// MemManage例外ベクタキャッチ
#define RF_VC_NOCPERR_BIT		(0x00000020)	// コプロセッサアクセスによるUsageFault例外ベクタキャッチ
#define RF_VC_CHKERR_BIT		(0x00000040)	// チェックエラーによるUsageFault例外ベクタキャッチ
#define RF_VC_STATERR_BIT		(0x00000080)	// 状態情報エラーによるUsageFault例外ベクタキャッチ
#define RF_VC_BUSERR_BIT		(0x00000100)	// BusFault例外ベクタキャッチ
#define RF_VC_INTERR_BIT		(0x00000200)	// 例外入出失敗ベクタキャッチ
#define RF_VC_HARDERR_BIT		(0x00000400)	// HardFault例外ベクタキャッチ

//-----------------------------------------------------------------------
// 構造体定義
//-----------------------------------------------------------------------
typedef struct {
	DWORD	dwClstNum;					// クラスタ総数
	BYTE	byClstCoreNum[CORE_MAX];	// クラスタ内のコアの数
	DWORD	dwCoreNum;					// デバイス内のコア総数
	BYTE	bySysExtrTrc;				// 外部トレース実装有無フラグ
	BYTE	bySysIntrTrc;				// 内蔵トレース実装有無フラグ
	BYTE	bySysSoftTrc;				// Printfトレース実装有無フラグ
	DWORD	dwSysCTINum;				// システムCTI数
	BYTE	byL2Cache;					// L2キャッシュ実装有無フラグ
	BYTE	byMMUCore;					// MMU搭載コア実装有無フラグ
	DWORD	dwIfDapNum;					// 接続しているDAPの数
	DWORD	dwIfDapId[DBG_DATA_IFDAPID_MAX];	// 接続しているDAPのIDコード
} RFWCARM_DBG_DATA;
typedef struct {
	BYTE	byCoreNum;					// クラスタ内のコアの数
	BYTE	byClstExtrTrc;				// クラスタ外部へのトレース実装有無フラグ
	BYTE	byClstIntrTrc;				// 内蔵トレース実装有無フラグ
	UINT64	u64L2CacheSize;				// L2キャッシュサイズ
} RFWCARM_DBGCLST_DATA;
typedef struct {
	DWORD	dwCoreID;					// コアID情報
	BYTE	byBPNum;					// 実行ブレーク数
	DWORD	dwBPType[CORE_MAX];			// 実行ブレークイベントタイプ
	BYTE	byBPMode;					// 実行ブレーク特権モード時のサポートモード
	BYTE	byWPNum;					// ウォッチポイント数
	BYTE	byWPMode;					// ウォッチポイントブレークサポートモード
	BYTE	byLinkNum;					// リンク先指定可能ブレーク/ウォッチポイント数
	BYTE	byCorePMU;					// PMU実装有無フラグ
	BYTE	byPerEvNum;					// イベントカウンタ数
	BYTE	byPerCycNum;				// サイクルカウンタ数
	DWORD	dwPerFunc[DBGCORE_PERFUNC_MAX];	// サポート計測項目情報
	BYTE	byCoreETM;					// ETM/PTMトレース実装有無フラグ
	BYTE	byCoreITM;					// ITMトレース実装有無フラグ(Cortex-M系のみ)
	BYTE	byCoreCTI;					// コアのクロストリガ実装有無フラグ
} RFWCARM_DBGCORE_DATA;
typedef struct {
	DWORD	dwTraceInfo;				//[TRC]
} RFWCARM_STAT_TRC_DATA;
typedef struct {
	BYTE	byStatMcu;					//[CPU]
} RFWCARM_STAT_CPU_DATA;
typedef struct {
	BYTE	byStatUVCC1;				//[EML]
	BYTE	byStatUVCC2;				//[EML]
	float	fNowUVCC1;					//[EML]
	float	fNowUVCC2;					//[EML]
	BYTE	byStatUCON;					//[EML]
	DWORD	dwTraceInfo;				//[EML]
} RFWCARM_STAT_EML_DATA;
typedef struct {
	UINT64					u64PcInfo;
	BYTE					byInstInfo;
	RFWCARM_STAT_TRC_DATA	StatTRC;
	RFWCARM_STAT_CPU_DATA	StatCPU;
	RFWCARM_STAT_EML_DATA	StatEML;
} RFWCARM_STAT_DATA;
typedef struct {
	BYTE	byTrcDataType;				// 取得トレースデータタイプ
	BYTE	byTrcMode;					// サポートトレースモードタイプ
	BYTE	byEvNum;					// トレースイベント数
	BYTE	byEvStartType;				// トレース開始イベントの種類
	BYTE	byEvEndType;				// トレース終了イベントの種類
} RFWCARM_TRCFNC_DATA;
typedef struct {
	BYTE	byBuffType;					// トレースバッファタイプ
	UINT64	u64BuffSize;				// トレースバッファサイズ
	BYTE	byBuffMode;					// トレースバッファの動作モード
} RFWCARM_INTRTRC_DATA;
typedef struct {
	BYTE	byExtrTrc;					// 外部トレース(SWOを除く)のサポート
	BYTE	byTrcOutType;				// トレース出力先選択
	DWORD	dwPinInfo;					// トレースサポート端子情報
	BYTE	byTrcMode;					// トレースの動作モード
	BYTE	bySwo;						// SWOのサポート
	BYTE	bySwoTxMode;				// SWOの出力モード
	BYTE	bySwoBuffMode;				// SWOの取得サポートモード
} RFWCARM_EXTRC_DATA;
typedef struct {
	DWORD	bySoftTrcNum;				// ソフトトレースチャネル数
	UINT64	u64maddrPort;				// ソフトトレース出力ポート0アドレス
	BYTE	byTimeStmp;					// タイムスタンプの実装情報
	BYTE	byTrgMode;					// ソフトトレーストリガ情報
} RFWCARM_SFTTRC_DATA;
typedef struct {
	BYTE	byEnable;
	BYTE	byTimestamp;
	UINT64	u64StimulusPort;
} RFW_STRC_DATA;



//-----------------------------------------------------------------------
// I/F関数の宣言
//-----------------------------------------------------------------------
#ifdef	__cplusplus
extern	"C" {
#endif

// MCU依存コマンド
DECLARE(RFWCARM_GetDBG)(RFWCARM_DBG_DATA *const pDbg);
DECLARE(RFWCARM_GetDBGCluster)(DWORD dwClstNum, RFWCARM_DBGCLST_DATA *const pDbgClst);
DECLARE(RFWCARM_GetDBGCore)(DWORD dwCoreInfo, RFWCARM_DBGCORE_DATA *const pDbgCore);
DECLARE(RFWCARM_GetStatus)(DWORD dwCoreNum, const DWORD* pdwCoreInfo, DWORD dwStatKind, RFWCARM_STAT_DATA *const pStat);
// メモリ操作コマンド
DECLARE(RFWCARM_ReadPhys)(DWORD dwCoreInfo, UINT64 u64madrAddr, DWORD dwLength, BYTE byAccessSize, BYTE byEndian, BYTE *const pbyReadBuff);
// レジスタ操作コマンド
DECLARE(RFWCAR_SetCpReg)(DWORD dwCoreInfo, DWORD dwCpNum , const DWORD* pdwCpId, const UINT64* pu64RegData);
DECLARE(RFWCAR_GetCpReg)(DWORD dwCoreInfo, DWORD dwCpNum , const DWORD* pdwCpId, UINT64 *const pu64RegData);
// プログラム実行関連コマンド
DECLARE(RFWCARM_GetBrkf)(DWORD dwCoreNum, const DWORD* pdwCoreInfo, RFWCARM_BRKF_DATA *const pBrkf);
// イベント関連コマンド
DECLARE(RFWCARM_SetVCatchEv)(DWORD dwCoreInfo, UINT64 u64VectorCatch);
DECLARE(RFWCARM_GetVCatchEv)(DWORD dwCoreInfo, UINT64 *const pu64VectorCatch);
// トレース関連コマンド
DECLARE(RFWCARM_GetTrcFuncInfo)(BYTE byInfoType, DWORD dwCoreClstInfo, BYTE *const pbySupport, RFWCARM_TRCFNC_DATA *const pTrcFnc);
DECLARE(RFWCARM_GetIntrTrcInfo)(BYTE byInfoType, DWORD dwCoreClstInfo, BYTE *const pbySupport, RFWCARM_INTRTRC_DATA *const pIntrTrc);
DECLARE(RFWCARM_GetExtrTrcInfo)(BYTE byInfoType, DWORD dwInfoNum, BYTE *const pbySupport, RFWCARM_EXTRC_DATA *const pExtrTrc);
DECLARE(RFWCARM_GetSoftTrcInfo)(BYTE byInfoType, DWORD dwInfoNum, BYTE *const pbySupport, RFWCARM_SFTTRC_DATA *const pSftTrc);
DECLARE(RFWCARM_GetTrcData)(DWORD dwStart, DWORD dwSize, BYTE *const pbyTrcData);
DECLARE(RFWCARM_SetSTrc)(const RFW_STRC_DATA* pSTrc);
// 非公開コマンド2(デバッグ用コマンド)
DECLARE(RFWCARM_APRead)(BYTE byDapId, BYTE byApSel, UINT64 u64madrAddr, DWORD dwLength, BYTE byAccessSize, BYTE *const pbyReadBuff);
DECLARE(RFWCARM_APWrite)(BYTE byDapId, BYTE byApSel, UINT64 u64madrAddr, DWORD dwLength, BYTE byAccessSize, const BYTE* pbyWriteBuff);
DECLARE(RFWCARM_WritePhys)(DWORD dwCoreInfo, DWORD dwWriteMode, UINT64 u64madrAddr, DWORD dwLength, BYTE byVerify, BYTE byAccessSize, BYTE byEndian, const BYTE* pbyWriteBuff, RFW_VERIFYERR_DATA *const pVerifyErr);

#ifdef	__cplusplus
}
#endif

#endif	// __RFWARM_H__
