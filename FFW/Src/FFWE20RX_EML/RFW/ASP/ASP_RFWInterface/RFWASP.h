///////////////////////////////////////////////////////////////////////////////
/**
 * @file RFWASP.h
 * @brief E2エミュレータRFW I/F(拡張機能)ヘッダファイル
 * @author S.Nagai
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・2016/09/09 初版作成 S.Nagai
*/
#ifndef	__RFWASP_H__
#define	__RFWASP_H__

#ifdef __GNUC__
#include <inttypes.h>
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(push, 1)
#endif

//-----------------------------------------------------------------------
// DEFINE定義
//-----------------------------------------------------------------------

// --- DEFINE定義 拡張機能コマンドID ------

#define ASPCMD_START			190		// E2拡張機能用コマンドIDの最初。コマンドを追加したら更新すること
#define ASPCMD_GETPWRMONRNG		190
#define ASPCMD_GETMONITOREVDATA	191
#define ASPCMD_SETEXTRGOUTLV	192
#define ASPCMD_GETEXTRGOUTLV	193
#define ASPCMD_SETMONITOREV		194
#define ASPCMD_GETMONITOREV		195
#define ASPCMD_CLRMONITOREV		196
#define ASPCMD_SETMONITOREVFACT	197
#define ASPCMD_GETMONITOREVFACT	198
#define ASPCMD_CLRMONITOREVFACT	199

#define	ASPCMD_ASPIN			200
#define ASPCMD_ASPOUT			201
#define ASPCMD_SETASPSW			202
#define ASPCMD_GETASPSW			203
#define ASPCMD_GETASPFUNC		204
#define ASPCMD_SETASPCONFIG		205
#define ASPCMD_GETASPCONFIG		206
#define ASPCMD_SETEXTRGIN		207
#define ASPCMD_GETEXTRGIN		208
#define ASPCMD_CLREXTRGIN		209
#define ASPCMD_SETEXTRGOUT		210
#define ASPCMD_GETEXTRGOUT		211
#define ASPCMD_CLREXTRGOUT		212
#define ASPCMD_SETPWRMON		213
#define ASPCMD_GETPWRMON		214
#define ASPCMD_CLRPWRMON		215
#define ASPCMD_SETPWRMONEV		216
#define ASPCMD_GETPWRMONEV		217
#define ASPCMD_CLRPWRMONEV		218
#define ASPCMD_SETCANMON		219
#define ASPCMD_GETCANMON		220
#define ASPCMD_CLRCANMON		221
#define ASPCMD_SETCANMONEV		222
#define ASPCMD_GETCANMONEV		223
#define ASPCMD_CLRCANMONEV		224
#define ASPCMD_SETPERINFO		225
#define ASPCMD_GETPERINFO		226
#define ASPCMD_CLRPERINFO		227
#define ASPCMD_GETPERDATA		228
#define ASPCMD_CLRPERDATA		229
#define ASPCMD_SETBRKEV			230
#define ASPCMD_GETBRKEV			231
#define ASPCMD_CLRBRKEV			232
#define ASPCMD_SETSTOPTRCEV		233
#define ASPCMD_GETSTOPTRCEV		234
#define ASPCMD_CLRSTOPTRCEV		235
#define ASPCMD_SETSWTRCEV		236
#define ASPCMD_GETSWTRCEV		237
#define ASPCMD_CLRSWTRCEV		238
#define ASPCMD_SETSAVEFILENAME	239
#define ASPCMD_LOADSAMPLE		240
#define ASPCMD_UNLOADSAMPLE		241
#define ASPCMD_SETSAMPSRC		242
#define ASPCMD_GETSAMP			243
#define ASPCMD_SETBUFSIZE		244
#define ASPCMD_SETSAMPTYPE		245
#define ASPCMD_STOPSAMP			246
#define ASPCMD_RESTARTSAMP		247
#define ASPCMD_GETAVAILTIME		248
#define ASPCMD_GETSAMPSIZE		249
#define ASPCMD_GETSAMPNOW		250
#define ASPCMD_GETSAMPSTAT		251
#define ASPCMD_SAVEFILE			252
#define ASPCMD_SETCLKCALIBRATION 253
#define ASPCMD_VENDORCTRLIN		254
#define ASPCMD_VENDORCTRLOUT	255
#define ASPCMD_END				255		// E2拡張機能用コマンドIDの最後。コマンドを追加したら更新すること

/* 
 * CmdExecutCheckのparamがbyte
 * MAX_BYTE 越えてしまうのでとりあえず
 */
#define ASPCMD_SCIOPEN			200
#define ASPCMD_SCICLOSE			200
#define ASPCMD_SCISEND			200
#define ASPCMD_SCIRECEIVE		200
#define ASPCMD_SPIOPEN			200
#define ASPCMD_SPICLOSE			200
#define ASPCMD_SPIREAD			200
#define ASPCMD_SPIWRITE			200
#define ASPCMD_SPIWRITEREAD		200
#define ASPCMD_GETTARGETSTATE	200


// --- DEFINE定義 拡張機能コマンド --------

// RFWASP_GetAspFunction pdwAspCommonFunction
#define RF_ASPFUNC_MON_CAN		(1<<0)	// 通信モニタ(CAN)
#define RF_ASPFUNC_MON_UART		(1<<1)	// 通信モニタ(UART)
#define RF_ASPFUNC_MON_SPI		(1<<2)	// 通信モニタ(SPI)
#define RF_ASPFUNC_MON_I2C		(1<<3)	// 通信モニタ(I2C)
#define RF_ASPFUNC_PSEUDO_UART	(1<<4)	// 疑似ターゲット(UART)
#define RF_ASPFUNC_PSEUDO_SPI	(1<<5)	// 疑似ターゲット(SPI)
#define RF_ASPFUNC_PSEUDO_I2C	(1<<6)	// 疑似ターゲット(I2C)
#define RF_ASPFUNC_EXTRG		(1<<7)	// 外部トリガ
#define RF_ASPFUNC_LOGIC		(1<<11)	// ロジアナ
#define RF_ASPFUNC_RAMMON		(1<<13)	// RAMモニタ
#define RF_ASPFUNC_PCSAMP		(1<<14)	// PCサンプリング
#define RF_ASPFUNC_PWRMON		(1<<15)	// パワーモニタ
#define RF_ASPFUNC_RESETZ		(1<<16)	// RESETZ
#define RF_ASPFUNC_PERFORMANCE	(1<<19)	// 時間計測
#define RF_ASPFUNC_STOPSAMP		(1<<20)	// 記録停止
#define RF_ASPFUNC_FACTRUN		(1<<21)	// RUN検出
#define RF_ASPFUNC_FACTBRK		(1<<22)	// BRK検出
#define RF_ASPFUNC_ACTRUN		(1<<23)	// RUN要求
#define RF_ASPFUNC_ACTBRK		(1<<24)	// BRK要求
#define	RF_ASPFUNC_STOPTRC		(1<<25) // トレース停止

// RFWASP_GetAspFunction pdwAspSeriesFunction(RL78)
#define RF_ASPFUNC_RL_MONEV			(1<<2)

// RFWASP_GetAspFunction pdwAspSeriesFunction(RH850)
#define RF_ASPFUNC_RH_SWTRC			(1<<0)
#define RF_ASPFUNC_RH_EVTOZ			(1<<1)

// RFW_ASPCONF_DATA dwStorage
#define RF_ASPSTORAGE_NONE			(0)
#define RF_ASPSTORAGE_E2			(1)
#define RF_ASPSTORAGE_E2_FULLSTOP	(2)
#define RF_ASPSTORAGE_E2_FULLBREAK	(3)
#define RF_ASPSTORAGE_PC			(4)

// RFW_ASPCONF_DATA dwFunction0
#define RF_ASPCONF_CAN_MON_CH0		(1<<0)
#define RF_ASPCONF_UART_MON_CH0		(1<<1)
#define RF_ASPCONF_SPI_MON_CH0		(1<<2)
#define RF_ASPCONF_I2C_MON_CH0		(1<<3)
#define RF_ASPCONF_PSEUDO_UART		(1<<4)
#define RF_ASPCONF_PSEUDO_SPI		(1<<5)
#define RF_ASPCONF_PSEUDO_I2C		(1<<6)
#define RF_ASPCONF_PSEUDO_MASK		(RF_ASPCONF_PSEUDO_UART | RF_ASPCONF_PSEUDO_SPI | RF_ASPCONF_PSEUDO_I2C)

// RFW_ASPCONF_DATA dwFunction1
#define RF_ASPCONF_CAN_MON_CH1		(1<<0)
#define RF_ASPCONF_UART_MON_CH1		(1<<1)
#define RF_ASPCONF_SPI_MON_CH1		(1<<2)
#define RF_ASPCONF_I2C_MON_CH1		(1<<3)

// RFW_ASPCONF_DATA dwFunction2
#define RF_ASPCONF_TRIGGER_OUT_CH0	(1<<7)
#define RF_ASPCONF_TRIGGER_OUT_CH1	(1<<8)
#define RF_ASPCONF_TRIGGER_IN_CH0	(1<<9)
#define RF_ASPCONF_TRIGGER_IN_CH1	(1<<10)
#define RF_ASPCONF_TRIGGER_MASK		(RF_ASPCONF_TRIGGER_OUT_CH0 | RF_ASPCONF_TRIGGER_OUT_CH1 | RF_ASPCONF_TRIGGER_IN_CH0 | RF_ASPCONF_TRIGGER_IN_CH1)
#define RF_ASPCONF_LOGICANALY_CH0	(1<<11)
#define RF_ASPCONF_LOGICANALY_CH1	(1<<12)
#define RF_ASPCONF_LOGICANALY_MASK	(RF_ASPCONF_LOGICANALY_CH0 | RF_ASPCONF_LOGICANALY_CH1)

// RFW_ASPCONF_DATA dwFunction3
#define RF_ASPCONF_BRK_EV			(1<<11)
#define RF_ASPCONF_MON_EV			(1<<12)
#define RF_ASPCONF_LOGGER			(1<<13)
#define RF_ASPCONF_PC_SAMPLING		(1<<14)
#define RF_ASPCONF_POWER_MON		(1<<15)
#define RF_ASPCONF_RESETZ			(1<<16)
#define RF_ASPCONF_EVTOZ			(1<<17)
#define RF_ASPCONF_EXT_STRACE		(1<<18)
#define RF_ASPCONF_STOPTRC_EV		(1<<19)

// RFWASP_SetSampling
#define	RF_ASPSRC_POWER				(1<<0)
#define RF_ASPSRC_MON_CH0			(1<<1)
#define RF_ASPSRC_MON_CH1			(1<<2)
#define RF_ASPSRC_EVL				(1<<3)
#define RF_ASPSRC_FAMILY			(1<<4)

// 記録データ種別 
#define RF_ASPTYPE_LOGIC_ANALYZER	(1<<0)
#define RF_ASPTYPE_UART_CH0_TX		(1<<1)
#define RF_ASPTYPE_UART_CH0_RX		(1<<2)
#define RF_ASPTYPE_I2C_CH0			(1<<3)
#define RF_ASPTYPE_SPI_CH0_MO		(1<<4)
#define RF_ASPTYPE_SPI_CH0_MI		(1<<5)
#define RF_ASPTYPE_CAN_CH0_RX		(1<<6)
#define RF_ASPTYPE_UART_CH1_TX		(1<<7)
#define RF_ASPTYPE_UART_CH1_RX		(1<<8)
#define RF_ASPTYPE_I2C_CH1			(1<<9)
#define RF_ASPTYPE_SPI_CH1_MO		(1<<10)
#define RF_ASPTYPE_SP1_CH1_MI		(1<<11)
#define RF_ASPTYPE_CAN_CH1_RX		(1<<12)
#define RF_ASPTYPE_POWER			(1<<13)
#define RF_ASPTYPE_EVL				(1<<14)
#define RF_ASPTYPE_PC				(1<<15)
#define RF_ASPTYPE_MEMORY			(1<<16)
#define RF_ASPTYPE_STRACE			(1<<17)
#define RF_ASPTYPE_MON_EV			(1<<18)
#define RF_ASPTYPE_MAX				(19)

#define RF_ASPCONF_EXTBOARD2		(1<<29)
#define RF_ASPCONF_EXTBOARD1		(1<<30)

// -RFWASP_SetAspSwitch-
#define RF_ASPSW_OFF			(0)
#define RF_ASPSW_ON				(1)

// RFWASP_SetPowerMonitor
// -Sampling Rate-
#define RF_ASPPWR_FAST_HIGH		(0)
#define RF_ASPPWR_FAST_LOW		(1)
#define RF_ASPPWR_REG_2U		(2)
#define RF_ASPPWR_REG_5U		(3)
#define RF_ASPPWR_REG_10U		(4)
#define RF_ASPPWR_REG_20U		(5)
#define RF_ASPPWR_REG_50U		(6)
#define RF_ASPPWR_REG_100U		(7)
#define RF_ASPPWR_REG_200U		(8)
#define RF_ASPPWR_REG_500U		(9)
#define RF_ASPPWR_REG_1M		(10)

// RFWASP_SetPowerMonitorEvent
#define RF_ASPPWREV_RANGEIN_L	(0)
#define RF_ASPPWREV_RANGEOUT_L	(1)
#define RF_ASPPWREV_RANGEIN_P	(2)
#define RF_ASPPWREV_RANGEOUT_P	(3)
#define RF_ASPPWREV_PULSE_LTE	(4)
#define RF_ASPPWREV_PULSE_GTE	(5)

// -Sampling Mode-
#define RF_ASPPWR_NORMAL		(0)
#define RF_ASPPWR_EVLINK		(1)

// RFWASP_SetCanMonitor
// -Can Channel-
#define RF_ASPCANMON_CH0		(0)
#define RF_ASPCANMON_CH1		(1)

// -Can Mode-
#define RF_ASPCAN_STANDARD_ID	(0)
#define RF_ASPCAN_EXTENDED_ID	(1)

// -Baudrate-
#define RF_ASPCAN_1MBPS			(120)
#define RF_ASPCAN_500KBPS		(240)
#define RF_ASPCAN_250KBPS		(480)
#define RF_ASPCAN_125KBPS		(960)

// RFWASP_SetCanMonitorEvent
#define RF_ASPCAN_MATCH_P		(0)
#define RF_ASPCAN_UNMATCH_P		(1)


// RFWASP_SetExTriggerIn byChNo
#define RF_ASPTRGIN_CH0			(0)			// TRGI0
#define RF_ASPTRGIN_CH1			(1)			// TRGI1

// RFWASP_SetExTriggerIn byDetectType
#define RF_ASPTRGIN_LLEVEL		(0)	
#define RF_ASPTRGIN_HLEVEL		(1)	
#define RF_ASPTRGIN_FEDGE		(2)	
#define RF_ASPTRGIN_REDGE		(3)	
#define RF_ASPTRGIN_BEDGE		(4)	

// RFWASP_SetExTriggerOut byChNo
#define RF_ASPTRGOUT_CH0		(0)
#define RF_ASPTRGOUT_CH1		(1)

// RFWASP_SetExTriggerOut byOutputType
#define RF_ASPTRGOUT_LEVEL		(0)	
#define RF_ASPTRGOUT_PULSE		(1)

// RFWASP_SetExTriggerOutLevel byActiveLevel
#define RF_ASPTRGOUT_HACTIVE	(0)
#define RF_ASPTRGOUT_LACTIVE	(1)

// RFWASP_GetSampleState  pdwState
#define RF_ASPSAMPLE_STOP		(0)
#define RF_ASPSAMPLE_RUN		(1)
#define RF_ASPSAMPLE_FULLBREAK	(1<<2)
#define RF_ASPSAMPLE_FULLSTOP	(1<<3)
#define RF_ASPSAMPLE_OVF		(1<<4)
#define RF_ASPSAMPLE_COMERR		(1<<5)
#define RF_ASPSAMPLE_WRAPAROUND (1<<6)

//-----------------------------------------------------------------------
// 変数の型定義
//-----------------------------------------------------------------------
#ifdef __GNUC__
typedef int						BOOL;
typedef uint8_t					BYTE;
typedef uint16_t				WORD;
typedef uint32_t				DWORD;
typedef uint64_t				UINT64;
typedef wchar_t					WCHAR;
#endif

//-----------------------------------------------------------------------
// 構造体定義
//-----------------------------------------------------------------------
typedef struct {
	DWORD	dwStorage;
	DWORD	dwCnt;
	DWORD	dwMaxStorage;
	DWORD	dwFunction0;
	DWORD	dwFunction1;
	DWORD	dwFunction2;
	DWORD	dwFunction3;
} RFW_ASPCONF_DATA;

typedef struct {
	BYTE	bySamplingRate;
	BYTE	bySamplingMode;
	UINT64	u64FactorSingleEvent;
	DWORD	dwFactorMultiEvent;
	WORD	wFilterMin;
	WORD	wFilterMax;
} RFW_PWRMONSAMP_DATA;

typedef struct {
	BYTE	byDetectType;
	WORD	wPowerMin;
	WORD	wPowerMax;
	WORD	wPulseWidthMin;
	WORD	wPulseWidthMax;
} RFW_PWRMONEV_DATA;

typedef struct {
	BYTE	byOutputType;
	WORD	wPulseWidth;
	UINT64	u64FactorSingleEvent;
	DWORD	dwFactorMultiEvent;
} RFW_EXTRGOUT_DATA;

typedef struct {
	BYTE	byMode;
	UINT64	u64StartEvent;
	UINT64	u64EndEvent;
	UINT64	u64SectionEvent;
	UINT64	u64Threshold;
} RFW_ASPPERCOND_DATA;

typedef struct {
	UINT64	u64MinCount;
	UINT64	u64MaxCount;
	DWORD	dwPassCount;
	UINT64	u64SumCount;
} RFW_ASPPER_DATA;

typedef struct {
	BYTE	byCanMode;
	WORD	wBaudrate;
	WORD	wFastBaudrate;
	BYTE	bySamplingPoint;
} RFW_CANMON_DATA;

typedef struct {
	BYTE	byDetectType;
	BYTE	byDelayCount;
	BYTE	byCanMode;
	BYTE	byDLC;
	BYTE	byRTR;
	BYTE	bySRR;
	DWORD	dwID;
	DWORD	dwData[16];
	DWORD	dwIDMask;
	DWORD	dwMask[16];
} RFW_CANMONEV_DATA;

typedef struct {
	char 	szComVer[16];
	DWORD	dwComVer;
	char 	szFamilyVer[16];
	DWORD	dwFamilyVer;
} RFW_ASPVERSION_DATA;


//-----------------------------------------------------------------------
// I/F関数の宣言
//-----------------------------------------------------------------------
#ifdef RFWE2_INSTANCE
#define    __EXTERN__
#else
#define    __EXTERN__	 extern
#endif

#ifdef	__cplusplus
extern	"C" {
#endif

// システムコマンド
DECLARE(RFW_AspIn)(DWORD dwReadByte, DWORD *const pbyReadBuff);
DECLARE(RFW_AspOut)(DWORD dwWriteByte, const DWORD* pbyWriteBuff);

// E2拡張機能コマンド
DECLARE(RFWASP_SetAspSwitch)(BYTE byAspSwitch);
DECLARE(RFWASP_GetAspSwitch)(BYTE *const pbyAspSwitch);
DECLARE(RFWASP_GetAspFunction)(DWORD *const pdwAspComonFunction, DWORD *const pdwAspSeriesFunction, DWORD *const pdwAspStorage, DWORD *const pdwAspOption);
DECLARE(RFWASP_SetAspConfiguration)(const RFW_ASPCONF_DATA* pAspConfiguration);
DECLARE(RFWASP_GetAspConfiguration)(RFW_ASPCONF_DATA const* pAspConfiguration);
DECLARE(RFWASP_SetSamplingSource)(DWORD dwSrc);
DECLARE(RFWASP_SetSaveFileName)(const char *fname);
DECLARE(RFWASP_SaveFile)(void);
DECLARE(RFWASP_LoadSample)(const char *fname);
DECLARE(RFWASP_UnLoadSample)(void);
DECLARE(RFWASP_GetSample)(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, void *pData, DWORD dwSize, DWORD *pdwStoreCnt, DWORD *pdwExistCnt, UINT64 *pu64Read);
DECLARE(RFWASP_SetRealTimeBufferSize)(DWORD dwSize);
DECLARE(RFWASP_SetRealTimeSamplingType)(DWORD dwType);
DECLARE(RFWASP_StopSample)(void);
DECLARE(RFWASP_RestartSample)(void);
DECLARE(RFWASP_GetAvailableTime)(DWORD dwType, UINT64 *const pu64Start, UINT64 *const pu64End, UINT64 *const pu64Sample);
DECLARE(RFWASP_GetSampleDataSize)(DWORD dwType, DWORD dwKeyType, UINT64 u64Start, UINT64 u64End, UINT64 *const pu64DataSize);
DECLARE(RFWASP_GetSampleRealTime)(DWORD dwType, DWORD dwKeyType, UINT64 u64Back, void *pData, DWORD dwSize, DWORD *const pdwStoreCnt, DWORD *const pdwExistCnt);
DECLARE(RFWASP_GetSampleState)(DWORD *const pdwState, UINT64 *const pu64Min, UINT64 *const pu64Max);

DECLARE(RFWASP_SetPowerMonitor)(const RFW_PWRMONSAMP_DATA* pPowerMonitor);
DECLARE(RFWASP_GetPowerMonitor)(RFW_PWRMONSAMP_DATA *const pPowerMonitor);
DECLARE(RFWASP_ClrPowerMonitor)(void);
DECLARE(RFWASP_SetPowerMonitorEvent)(const RFW_PWRMONEV_DATA* pPowerMonitorEvent);
DECLARE(RFWASP_GetPowerMonitorEvent)(RFW_PWRMONEV_DATA *const pPowerMonitorEvent);
DECLARE(RFWASP_ClrPowerMonitorEvent)(void);
DECLARE(RFWASP_SetExTriggerIn)(BYTE byChNo, BYTE byDetectType);
DECLARE(RFWASP_GetExTriggerIn)(BYTE byChNo, BYTE *const pbyDetectType);
DECLARE(RFWASP_ClrExTriggerIn)(BYTE byChNo);
DECLARE(RFWASP_SetExTriggerOut)(BYTE byChNo, const RFW_EXTRGOUT_DATA* pExTriggerOut);
DECLARE(RFWASP_GetExTriggerOut)(BYTE byChNo, RFW_EXTRGOUT_DATA *const pExTriggerOut);
DECLARE(RFWASP_ClrExTriggerOut)(BYTE byChNo);
DECLARE(RFWASP_SetExTriggerOutLevel)(BYTE byChNo, BYTE byActiveLevel);
DECLARE(RFWASP_GetExTriggerOutLevel)(BYTE byChNo, BYTE *const pbyActiveLevel);
DECLARE(RFWASP_SetPerformanceInfo)(BYTE byChNo, const RFW_ASPPERCOND_DATA* pAspPerformance);
DECLARE(RFWASP_GetPerformanceInfo)(BYTE byChNo, RFW_ASPPERCOND_DATA *const pAspPerformance);
DECLARE(RFWASP_ClrPerformanceInfo)(BYTE byChNo);
DECLARE(RFWASP_GetPerformanceData)(BYTE byChNo, RFW_ASPPER_DATA *const pAspPerformanceData);
DECLARE(RFWASP_ClrPerformanceData)(BYTE byChNo);
DECLARE(RFWASP_SetCANMonitor)(BYTE byChNo, const RFW_CANMON_DATA* pCanMonitor);
DECLARE(RFWASP_GetCANMonitor)(BYTE byChNo, RFW_CANMON_DATA *const pCanMonitor);
DECLARE(RFWASP_ClrCANMonitor)(BYTE byChNo);
DECLARE(RFWASP_SetCANMonitorEvent)(BYTE byChNo, const RFW_CANMONEV_DATA* pCanMonitorEvent);
DECLARE(RFWASP_GetCANMonitorEvent)(BYTE byChNo, RFW_CANMONEV_DATA *const pCanMonitorEvent);
DECLARE(RFWASP_ClrCANMonitorEvent)(BYTE byChNo);
DECLARE(RFWASP_SetBreakEvent)(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent);
DECLARE(RFWASP_GetBreakEvent)(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent);
DECLARE(RFWASP_ClrBreakEvent)(void);
DECLARE(RFWASP_SetStopTraceEvent)(UINT64 u64FactorSingleEvent, DWORD dwFactorMultiEvent);
DECLARE(RFWASP_GetStopTraceEvent)(UINT64 *const pu64FactorSingleEvent, DWORD *const pdwFactorMultiEvent);
DECLARE(RFWASP_ClrStopTraceEvent)(void);
DECLARE(RFWASP_GetTargetState)(DWORD *const pdwState);
DECLARE(RFWASP_GetVersion)(RFW_ASPVERSION_DATA* pVer);
DECLARE(RFWASP_SetClkCalibration)(DWORD dwType, INT64 i64Calib);
DECLARE(RFWASP_AddRaw)(const char* rawfile, const char* datfile);
/*
DECLARE(RFWASP_UartOpen)(DWORD dwRate, BYTE bySize, BYTE byParityType, BYTE byStopBits);
DECLARE(RFWASP_UartClose)(void);
DECLARE(RFWASP_UartSend)(WORD wLen, const BYTE *pbyData);
DECLARE(RFWASP_UartReceive)(WORD wLen, BYTE *pbyData);
*/
#ifdef	__cplusplus
}
#endif

#if defined(_MSC_VER) || defined(__GNUC__)
#pragma pack(pop)
#endif

#endif	// __RFWASP_H__
