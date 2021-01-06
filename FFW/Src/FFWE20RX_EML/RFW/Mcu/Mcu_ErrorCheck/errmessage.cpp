///////////////////////////////////////////////////////////////////////////////
/**
 * @file errmessage.cpp
 * @brief エラーメッセージ文字列定義
 * @author RSD H.Hatahara, PA M.Tsuji
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
/*
   ■改定履歴
   2015/7/14 エラーコード仕様書(2015/7/13版)とエラー(日本語)の一致をとった。H.Hatahara
   2015/8/17 エラーコード仕様書(2015/8/4版)からテーブル生成 M.Ogata
   2015/11/27エラーコード仕様書(2015/11/26版)から反映 Y.Arai
   2017/4/27 エラーコード仕様書(2017/4/26版)から反映 H.Hatahara
   */
#include "errmessage.h"

//------------------------------------------------------------------------------
// static 関数宣言
//------------------------------------------------------------------------------
static const ERRMSG_DATA s_ERRMSG_DATA[] = {
	{
		RFWERR_OK,
		L"コマンド処理を正常終了しました。",
		L"Completed."
	},

	{
		RFWERR_ERR,
		L"−",
		L"−"
	},

	{
		RFWERR_N_RFW_ARG,
		L"引数の指定に誤りがあります。",
		L"The specified parameter is incorrect."
	},

	{
		RFWERR_N_BFW_CMD,
		L"未定義のエミュレータファームウェアコマンドを受信しました。",
		L"Undefined emulator firmware command was received."
	},

	{
		RFWERR_N_BFWFROM_ERASE,
		L"エミュレータファーム書き換え時にフラッシュROMのイレーズエラーが発生しました。",
		L"An erasure error of the flash ROM occurred when rewriting the emulator firmware."
	},

	{
		RFWERR_N_BFWFROM_WRITE,
		L"エミュレータファームの書き込みエラーが発生しました。",
		L"A programming error of the emulator firmware occurred."
	},

	{
		RFWERR_N_BFWFROM_VERIFY,
		L"エミュレータファームの書き込み後のベリファイでエラーが発生しました。",
		L"A verification error occurred after programming the emulator firmware."
	},

	{
		RFWERR_N_FPGA_CONFIG,
		L"FPGAコンフィグレーションエラーが発生しました。",
		L"An FPGA configuration error occurred."
	},

	{
		RFWERR_W_USB_VBUS_LOW,
		L"(Warning)USBバスパワー電圧が低下しています。",
		L"(Warning message)The USB bus power voltage is low."
	},

	{
		RFWERR_N_SID_SIZEOVER,
		L"シリアル番号の文字数が許容範囲を超えています。",
		L"The number of characters used for a serial number exceeds the acceptable range."
	},

	{
		RFWERR_N_SID_BLANK,
		L"シリアル番号が設定されていません。ブランク状態です。",
		L"The serial number is not set; it is in a blank state."
	},

	{
		RFWERR_N_LID_UNDEFINE,
		L"ライセンスIDが定義されていません。",
		L"License ID is not defined."
	},

	{
		RFWERR_N_NID_SIZEOVER,
		L"ニックネームの文字数が許容範囲を超えています。",
		L"The number of characters used for a nickname exceeds the acceptable range."
	},

	{
		RFWERR_N_NID_BLANK,
		L"ニックネームが設定されていません。ブランク状態です。",
		L"The nickname is not set; it is in a blank state."
	},

	{
		RFWERR_N_MCUROM_ERASE,
		L"内蔵フラッシュROMのイレーズエラーが発生しました。フラッシュROMが劣化している可能性があります。",
		L"An erasure error of the internal flash ROM occurred. The flash ROM may have been degraded."
	},

	{
		RFWERR_N_MCUROM_WRITE,
		L"内蔵フラッシュROMの書き込みエラーが発生しました。フラッシュROMが劣化している可能性があります。",
		L"A programming error of the internal flash ROM occurred. The flash ROM may have been degraded."
	},

	{
		RFWERR_N_MCUROM_VERIFY,
		L"内蔵フラッシュROMのベリファイエラーが発生しました。フラッシュROMが劣化している可能性があります。",
		L"A verification error of the internal flash ROM occurred. The flash ROM may have been degraded."
	},

	{
		RFWERR_N_EX1BFWFROM_ERASE,
		L"拡張ボードのファームウェア書き換え時にフラッシュROMのイレーズエラーが発生しました。",
		L"An erasure error of the flash ROM occurred during rewriting the firmware on the extension board."
	},

	{
		RFWERR_N_EX1BFWFROM_WRITE,
		L"拡張ボードのファームウェア書き換え時にフラッシュROMの書き込みエラーが発生しました。",
		L"A programming error of the flash ROM occurred during rewriting the firmware on the extension board."
	},

	{
		RFWERR_N_EX1BFWFROM_VERIFY,
		L"拡張ボードのファームウェア書き換え後のベリファイでエラーが発生しました。",
		L"A verification error occurred after rewriting the firmware on the extension board."
	},

	{
		RFWERR_N_EX1BFPGA_CONFIG,
		L"拡張ボードでFPGAコンフィグレーションエラーが発生しました。",
		L"An FPGA configuration error occurred on the extension board."
	},

	{
		RFWERR_N_EX1BEROM_ERASE,
		L"拡張ボードのEEPROM書き換え時にイレーズエラーが発生しました。",
		L"An erasure error occurred during rewriting EEPROM on the extension board."
	},

	{
		RFWERR_N_EX1BEROM_WRITE,
		L"拡張ボードのEEPROM書き換え時にライトエラーが発生しました。",
		L"A programming error occurred during rewriting EEPROM on the extension board."
	},

	{
		RFWERR_N_EX1BEROM_VERIFY,
		L"拡張ボードのEEPROM書き換え後のベリファイでエラーが発生しました。",
		L"A verification error occurred after rewriting EEPROM on the extension board."
	},

	{
		RFWERR_N_EX1BEROM_READ,
		L"拡張ボードのEEPROMでリードエラーが発生しました。",
		L"A reading error occurred in EEPROM on the extension board."
	},

	{
		RFWERR_N_EX1BEROM_NORES,
		L"拡張ボードのEEPROMから応答がありませんでした。",
		L"There was no response from EEPROM on the extension board."
	},

	{
		RFWERR_N_EX2BFPGA_CONFIG,
		L"2nd拡張ボードでFPGAコンフィグレーションエラーが発生しました。",
		L"An FPGA configuration error occurred on the 2nd extension board."
	},

	{
		RFWERR_N_SPBFPGAFROM_ERASE,
		L"SPボードFPGAデータ格納用フラッシュROM書き換え時にイレーズエラーが発生しました。",
		L"An erasure error occurred during rewriting the flash ROM for storing the FPGA data on the SP board."
	},

	{
		RFWERR_N_SPBFPGAFROM_WRITE,
		L"SPボードFPGAデータ格納用フラッシュROM書き換え時に書き込みエラーが発生しました。",
		L"A programming error occurred during rewriting the flash ROM for storing the FPGA data on the SP board."
	},

	{
		RFWERR_N_SPBFPGAFROM_VERIFY,
		L"SPボードFPGAデータ格納用フラッシュROM書き換え後のベリファイでエラーが発生しました。",
		L"A verification error occurred after rewriting the flash ROM for storing the FPGA data on the SP board."
	},

	{
		RFWERR_N_EX2BEROM_ERASE,
		L"2nd拡張ボードのEEPROM書き換え時にイレーズエラーが発生しました。",
		L"An erasure error occurred during rewriting EEPROM on the 2nd extension board."
	},

	{
		RFWERR_N_EX2BEROM_WRITE,
		L"2nd拡張ボードのEEPROM書き換え時にライトエラーが発生しました。",
		L"A programming error occurred during rewriting EEPROM on the 2nd extension board."
	},

	{
		RFWERR_N_EX2BEROM_VERIFY,
		L"2nd拡張ボードのEEPROM書き換え後のベリファイでエラーが発生しました。",
		L"A verification error occurred after rewriting EEPROM on the 2nd extension board."
	},

	{
		RFWERR_N_EX2BEROM_READ,
		L"2nd拡張ボードのEEPROMでリードエラーが発生しました。",
		L"A reading error occurred in EEPROM on the 2nd extension board."
	},

	{
		RFWERR_N_EX2BEROM_NORES,
		L"2nd拡張ボードのEEPROMから応答がありませんでした。",
		L"There was no response from EEPROM on the 2nd extension board."
	},

	{
		RFWERR_N_PODFPGAFROM_ERASE,
		L"エミュレーションポッドFPGA格納用フラッシュROMのイレーズエラーが発生しました。",
		L"An erasure error occurred in the flash ROM for storing the emulation pod FPGA."
	},

	{
		RFWERR_N_PODFPGAFROM_WRITE,
		L"エミュレーションポッドFPGA格納用フラッシュROMの書き込みエラーが発生しました。",
		L"A programming error occurred in the flash ROM for storing the emulation pod FPGA."
	},

	{
		RFWERR_N_PODFPGAFROM_VERIFY,
		L"エミュレーションポッドFPGA格納用フラッシュROMのベリファイエラーが発生しました。",
		L"A verification error occurred in the flash ROM for storing the emulation pod FPGA."
	},

	{
		RFWERR_N_ILLEGAL_IDCODE,
		L"認証に失敗をしました。エミュレータを接続できません。",
		L"Authentication failed. The emulator cannot be connected."
	},

	{
		RFWERR_N_UNSUPPORT,
		L"サポートしていないデバッグ機能の操作が発生しました。",
		L"An attempt was made to use an unsupported debugging function."
	},

	{
		RFWERR_N_NOTGETINFO,
		L"MCUとの接続前で情報が取得できていません。",
		L"Information has not been obtained since connection with the MCU has not been made."
	},

	{
		RFWERR_N_CANCEL,
		L"他のコアでエラーが発生したため、処理を中止しました。",
		L"Processing is halted since an error occurred with another core."
	},

	{
		RFWERR_N_FWRITE_NOTEXE_ILGLERR,
		L"MCU内蔵フラッシュROMへのアクセス違反などが発生しているため、フラッシュ書き換えが実行できません。",
		L"Rewriting of the MCU's internal flash ROM cannot be executed due to access violation to the MCU's internal flash ROM or other reasons."
	},

	{
		RFWERR_N_FWRITE_FAIL_ILGLERR,
		L"MCU内蔵フラッシュROMへのアクセス違反が発生し、フラッシュ書き換えが正常に実行できませんでした。",
		L"Rewriting of the MCU's internal flash ROM was not normally executed due to access violation to the MCU's internal flash ROM."
	},

	{
		RFWERR_N_ACC_ROM_PEMODE,
		L"フラッシュ書き換えモード中のため、内蔵ROM領域にアクセスできません。",
		L"Access to the internal ROM area cannot be made since the flash ROM is in the rewriting mode."
	},

	{
		RFWERR_W_CLKCHG_DMB_CLR,
		L"メモリアクセス中のクロック切り替えなどによる通信エラーが発生したためリカバリしました。",
		L"The emulator is recovered since a communication error occurred due to switching of the clock source while memory is being accessed or other reasons."
	},

	{
		RFWERR_N_FLASH_ALERASE_ID_FAIL,
		L"Flash 全消去ID によるFlash全消去処理に失敗しました。",
		L"Failed to erase all flash memory by the ID for erasing all flash memory."
	},

	{
		RFWERR_N_DATA_EVPOINT_OVER,
		L"データアクセスイベントの設定数が最大設定点数を越えています。",
		L"The number of data access events has exceeded the maximum setting value."
	},

	{
		RFWERR_N_DATA_EV_NOTSET,
		L"データアクセスイベントが設定されていません。",
		L"Data access event has not been set."
	},

	{
		RFWERR_N_EVPOINT_OVER,
		L"命令フェッチブレークイベントの設定数が最大設定点数を越えています。",
		L"The number of instruction-fetch break events has exceeded the maximum setting value."
	},

	{
		RFWERR_N_EV_NOTSET,
		L"イベントが設定されていません。",
		L"Event has not been set."
	},

	{
		RFWERR_N_WPADDR_SAM,
		L"データアクセスイベント設定アドレスがセキュリティーエリア内のため、ウォッチポイントの設定ができません。",
		L"The watch point cannot be set since the specified address for the data access event is within the security area."
	},

	{
		RFWERR_N_EV_OVER,
		L"イベント本数よりも多いイベントを設定しました。",
		L"The number of events has exceeded the predetermined value."
	},

	{
		RFWERR_N_EV_UNSUPPORT,
		L"サポートしていないイベント機能を設定しました。",
		L"Unsupported event function was set."
	},

	{
		RFWERR_N_EV_OVERLAPP,
		L"重複したイベント番号が設定されました。",
		L"Duplicated event number was set."
	},

	{
		RFWERR_N_VC_UNSUPPORT,
		L"サポートしていないベクタキャッチ機能が設定されました。",
		L"Unsupported vector catch function was set."
	},

	{
		RFWERR_N_TRC_EVPOINT_OVER,
		L"トレースイベントの設定ポイントが最大設定点数を越えています。",
		L"The number of trace events has exceeded the maximum setting value."
	},

	{
		RFWERR_N_EV_USECOMB,
		L"指定の単体イベントが組み合わせイベントで使われています。",
		L"The specified single event has already been used for the combination event."
	},

	{
		RFWERR_N_EVADDR_SAM,
		L"データイベント設定アドレスがセキュリティーエリア内のため、データイベントの設定ができません。",
		L"The data event cannot be set since the specified address for the data event is within the security area."
	},

	{
		RFWERR_N_EV_USEFUNC,
		L"イベントがハードブレーク、トレース等に設定されています。",
		L"The specified event has already been registered for a hardware break, tracing, etc."
	},

	{
		RFWERR_N_EV_UNSUPPORT_ADDR,
		L"未サポートのアドレスが設定されています。",
		L"Unsupported address is set."
	},

	{
		RFWERR_N_SBPOINT_OVER,
		L"ソフトウェアブレークポイントの設定数が最大設定点数を超えています。",
		L"The number of software breakpoints has exceeded the maximum setting value."
	},

	{
		RFWERR_N_SBADDR_NOTSET,
		L"ソフトウェアブレークポイントが設定されていません。",
		L"Software breakpoint has not been set."
	},

	{
		RFWERR_N_SBAREA_OUT,
		L"指定領域はソフトウェアブレークポイント設定不可領域です。",
		L"Software breakpoint cannot be set in the specified area."
	},

	{
		RFWERR_N_SBADDR_SETFAIL,
		L"指定領域へのソフトウェアブレークポイント設定に失敗しました",
		L"Failed to set a software breakpoint in the specified area."
	},

	{
		RFWERR_N_SBADDR_OVERLAPP,
		L"既にソフトブレークポイントが設定されているアドレスへ別のソフトブレークポイントを重複して設定することはできません",
		L"It is impossible to set another software breakpoint at the address where a software breakpoint has already been set."
	},

	{
		RFWERR_W_SBPOINT_DELETE,
		L"(Warning)設定していたソフトウェアブレークポイントが消失しました",
		L"(Warning message)The software breakpoint that has been set disappeared."
	},

	{
		RFWERR_N_SB_ROMAREA_MCURUN,
		L"ユーザプログラム実行中に内部ROM領域へのソフトウェアブレークは設定/解除できません。",
		L"Software breakpoints cannot be set or deleted in the internal ROM area while a user program is running."
	},

	{
		RFWERR_N_SB_ROMAREA_FLASHDEBUG,
		L"「内蔵フラッシュメモリの書換えを伴うプログラム」をデバッグ中は、内部ROM領域へのソフトウェアブレークが設定できません。",
		L"Software breakpoints cannot be set in the internal ROM area while debugging \"a program that involves rewriting of the internal flash memory.\""
	},

	{
		RFWERR_N_SB_UCODE_SETFAIL,
		L"指定領域へのユーザ命令コード復帰に失敗しました。",
		L"Failed to restore the user instruction code to the specified area."
	},

	{
		RFWERR_N_SBADDR_SAM,
		L"ソフトウェアブレーク設定アドレスがセキュリティーエリア内のため、ブレークポイントの設定ができません。",
		L"Software breakpoint cannot be set since the specified address for the software breakpoint is within the security area."
	},

	{
		RFWERR_N_STOP_AMCU_RESET,
		L"リセット中のためブレークできませんでした。",
		L"Failed to break the program since the program was being reset."
	},

	{
		RFWERR_N_HB_ILLEGAL_EVENT,
		L"指定できないイベントが設定されました。",
		L"An event that cannot be specified was set."
	},

	{
		RFWERR_N_RTTSTAT_RECORDING,
		L"トレース動作中です。",
		L"Tracing is in progress."
	},

	{
		RFWERR_N_RTTDATA_NON,
		L"トレースデータがありません。",
		L"There is no trace data."
	},

	{
		RFWERR_N_RTTSTOP_FAIL,
		L"トレース停止できませんでした。",
		L"Failed to stop tracing."
	},

	{
		RFWERR_N_RTTMODE_UNSUPRT,
		L"このデバイスでは、このトレース機能を使用できません。",
		L"The trace function is not supported in this device."
	},

	{
		RFWERR_N_RTTSTAT_DIS,
		L"トレース機能は動作していません。",
		L"The trace function does not work."
	},

	{
		RFWERR_N_RTT_UNSUPPORT,
		L"トレース機能は使用できません。",
		L"The trace function is not available."
	},

	{
		RFWERR_N_ATARGET_RESET,
		L"ターゲットのリセット端子が制御できません。SREST端子状態を確認して下さい。",
		L"The reset pin of the target cannot be controlled. Check the state of the SREST pin."
	},

	{
		RFWERR_N_ATARGET_TRSTN,
		L"ターゲットのTRST端子が制御できません。TRST端子状態を確認して下さい。",
		L"The TRST pin of the target cannot be controlled. Check the state of the TRST pin."
	},

	{
		RFWERR_N_ATARGET_TDI,
		L"ターゲットのTDI端子が制御できません。TDI端子状態を確認して下さい。",
		L"The TDI pin of the target cannot be controlled. Check the state of the TDI pin."
	},

	{
		RFWERR_N_ATARGET_TMS,
		L"ターゲットのTMS/SWDIO端子が制御できません。TMS/SWDIO端子状態を確認して下さい。",
		L"The TMS/SWDIO pin of the target cannot be controlled. Check the state of the TMS/SWDIO pin."
	},

	{
		RFWERR_N_ATARGET_TCK,
		L"ターゲットのTCK/SWCLK端子が制御できません。TCK/SWCLK端子状態を確認して下さい。",
		L"The TCK/SWCLK pin of the target cannot be controlled. Check the state of the TCK/SWCLK pin."
	},

	{
		RFWERR_N_AMCU_NO_HALT,
		L"ターゲットの指定コアをHALTできません。ターゲットのCPUのリセット状態などを確認してください。",
		L"The specified core in the target cannot be halted. Check the reset state of the target CPU and other items."
	},

	{
		RFWERR_N_AMCU_NO_DAP,
		L"ターゲットMCUとの初めての通信接続が正常に行えません。接続ターゲットにDAPが見つかりません。接続ターゲットの確認もしくは、JTAG/SWDクロックを下げるなどを試してください。",
		L"First communications connection with the target MCU cannot be made normally.DAP cannot be found in the connection target.Check the connection target or lower the JTAG/SWD clock frequency."
	},

	{
		RFWERR_N_HOTPLUGIN_PWR,
		L"ホットプラグイン起動時エミュレータからの電源供給は行なえません。電源供給機能をOFFにしてください。",
		L"Power cannot be supplied from the emulator at the time of hot plug-in startup. Turn off the power supply function."
	},

	{
		RFWERR_N_BTARGET_UVCC2_POWEROFF,
		L"ターゲットシステムの電源2がOFF状態です。ターゲットシステムの接続、20-14ピン変換コネクタの2電源切り替えスイッチの状態を確認してください。",
		L"Power supply 2 of the target system is turned off. Check the state of connection with the target system and the state of the switch that is used to switch two power sources of the 20-pin to 14-pin conversion connector."
	},

	{
		RFWERR_N_BTARGET_UVCC2_ENABLE,
		L"ターゲットシステムとの接続が2電源状態になっています。ターゲットシステムの接続、20-14ピン変換コネクタの2電源切り替えスイッチの状態を確認してください。",
		L"Two power sources are used for connection with the target system. Check the state of connection with the target system and state of the switch that is used to switch two power sources of the 20-pin to 14-pin conversion connector."
	},

	{
		RFWERR_N_COM_USED,
		L"指定されたエミュレータは既に使用されています。",
		L"The specified emulator has already been used."
	},

	{
		RFWERR_N_COM_NOT_FOUND,
		L"指定されたエミュレータが見つかりません。",
		L"The specified emulator cannot be found."
	},

	{
		RFWERR_D_NON_RFW_INIT,
		L"ファームウェアの初期化処理が行われていません。",
		L"Initialization processing of the firmware has not been done."
	},

	{
		RFWERR_N_ALREADY_CONNECT,
		L"既にターゲットMCUへ接続済みです。",
		L"Connection with the target MCU has already been made."
	},

	{
		RFWERR_N_BTARGET_POWERSUPPLY,
		L"ユーザシステムに外部から電源が供給されています。",
		L"External power is supplied to the user system."
	},

	{
		RFWERR_W_TARGET_POWERLOW,
		L"ユーザシステムの電圧が指定した供給電圧よりも低くなっています。",
		L"The voltage supplied to the user system is lower than the specified value."
	},

	{
		RFWERR_N_BTARGET_POWERASP,
		L"ユーザシステムに電源供給中、E2拡張インターフェースへ電源供給はできません。",
		L"Power cannot be supplied to the E2 expansion interface while power is being supplied to the user system."
	},

	{
		RFWERR_N_BASP_POWERTARGET,
		L"E2拡張インターフェースに電源供給中、ユーザシステムへ電源供給はできません。",
		L"Power cannot be supplied to the user system while power is being supplied to the E2 expansion interface."
	},

	{
		RFWERR_N_BASP_POWERSUPPLY,
		L"E2拡張インターフェースに外部から電源が供給されています。",
		L"External power is supplied to the E2 expansion interface."
	},

	{
		RFWERR_N_BNONASP,
		L"E2拡張インターフェース未サポートのエミュレータです。",
		L"The emulator does not support the E2 expansion interface."
	},

	{
		RFWERR_N_BMCU_RUN,
		L"ユーザプログラム実行中のためコマンド処理を実行できません。",
		L"The command cannot be executed since the user program is running."
	},

	{
		RFWERR_N_BMCU_STOP,
		L"ユーザプログラム停止中のためコマンド処理を実行できません。",
		L"The command cannot be executed since the user program is halted."
	},

	{
		RFWERR_N_BMCU_NONCONNECT,
		L"MCUとの接続が完了していません。",
		L"Connection with the MCU has not been made yet."
	},

	{
		RFWERR_N_BTARGET_POWER,
		L"ユーザシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できません。",
		L"The command cannot be executed since the power supply to the MCU on the user system is turned off."
	},

	{
		RFWERR_N_BTARGET_RESET,
		L"ユーザシステム上のリセット端子が\"L\"状態のためコマンド処理を実行できません。リセット端子の状態を確認してください。",
		L"The command cannot be executed since the signal level on the reset pin on the user system is low-active. Check the state of the reset pin."
	},

	{
		RFWERR_N_BTARGET_NOCONNECT,
		L"ユーザシステムが接続されていません。",
		L"Not connected to the user system."
	},

	{
		RFWERR_N_BMCU_RESET,
		L"MCUがリセット状態のためコマンド処理を実行できません。",
		L"The command cannot be executed since the MCU is in the reset state."
	},

	{
		RFWERR_N_BMCU_STANBY,
		L"MCUのスタンバイモード中で内部クロックが停止状態です。",
		L"The internal clock is halted since the MCU is in the standby mode."
	},

	{
		RFWERR_N_BMCU_SLEEP,
		L"MCUのスリープモード中で内部クロックが停止状態です。",
		L"The internal clock is halted since the MCU is in the sleep mode."
	},

	{
		RFWERR_N_BMCU_PDOWN,
		L"MCUがパワーダウン状態のため内部クロックが停止状態です。",
		L"Since the power to the MCU is turned off, internal clock is halted."
	},

	{
		RFWERR_N_BMCU_AUTH_DIS,
		L"MCU内部リセットが発生したためコマンド処理を実行できません。",
		L"The command cannot be executed since the internal reset of the MCU is generated."
	},

	{
		RFWERR_W_BMCU_PDOWN,
		L"(Warning)MCUがパワーダウン状態のためコマンド処理を実行できません。デバッグする場合は、パワーダウン状態を解除してください。",
		L"(Warning message)The command cannot be executed since the MCU is turned off. Release the power-down state to conduct debugging."
	},

	{
		RFWERR_W_BMCU_OSDLK,
		L"(Warning)MCUがOS Double Lock状態のためコマンド処理を実行できません。デバッグする場合は、OS Double Lock状態を解除してください。",
		L"(Warning message)The command cannot be executed since the MCU is in the state of OS double-lock. Release the OS double-lock state to conduct debugging."
	},

	{
		RFWERR_W_BMCU_STANBY,
		L"(Warning)MCUがスタンバイ状態のためコマンド処理を実行できません。デバッグする場合は、スタンバイ状態を解除してください。",
		L"(Warning message)The command cannot be executed since the MCU is in standby state. Release the standby state to conduct debugging."
	},

	{
		RFWERR_N_BEXE_LEVEL0,
		L"書き換え可能な動作モードではありません。",
		L"The operation mode cannot be rewritten."
	},

	{
		RFWERR_N_EML_ENDCODE,
		L"EML領域にエンドコードがありません。",
		L"There is no end code in the EML area."
	},

	{
		RFWERR_N_PRG_ENDCODE,
		L"PRG領域にエンドコードがありません。",
		L"There is no end code in the PRG area."
	},

	{
		RFWERR_N_EML_TO_PRG,
		L"EML動作モードからPRG動作モードへの遷移はできません。",
		L"Transition from the EML operation mode to the PRG operation mode is not possible."
	},

	{
		RFWERR_N_PRG_TO_EML,
		L"PRG動作モードからEML動作モードへの遷移はできません。",
		L"Transition from the PRG operation mode to the EML operation mode is not possible."
	},

	{
		RFWERR_N_AMCU_IABORT,
		L"MCUに命令アボートが発生しました。メモリアクセスが正常に行われませんでした。",
		L"Instruction abort is generated in the MCU. Access to memory was not made normally."
	},

	{
		RFWERR_N_AMCU_DABORT,
		L"MCUにデータアボートが発生しました。メモリアクセスが正常に行われませんでした。",
		L"Data abort is generated in the MCU. Access to memory was not made normally."
	},

	{
		RFWERR_N_BFWWAIT_TIMEOUT,
		L"エミュレータファームウェアのウェイト処理で指定時間以内に指定状態にならずタイムアウトが発生しました。",
		L"A timeout occurred since the wait processing by the emulator firmware did not reach the specified state within the specified time."
	},

	{
		RFWERR_SU_AMCU_RESET,
		L"タイムアウトエラーが発生しました。MCUがリセット状態です。",
		L"A timeout error occurred. The MCU is in the reset state."
	},

	{
		RFWERR_SU_AMCU_STANBY,
		L"タイムアウトエラーが発生しました。MCUのスタンバイモード中で内部クロックが停止状態です。",
		L"A timeout error occurred. Since the MCU is in standby mode, internal clock is halted."
	},

	{
		RFWERR_SU_AMCU_SLEEP,
		L"タイムアウトエラーが発生しました。MCUのスリープモード中で内部クロックが停止状態です。",
		L"A timeout error occurred. Since the MCU is in sleep mode, internal clock is halted."
	},

	{
		RFWERR_SU_AMCU_PDOWN,
		L"タイムアウトエラーが発生しました。MCUがパワーダウン状態のため内部クロックが停止状態です。",
		L"A timeout error occurred. Since the MCU is turned off, internal clock is halted."
	},

	{
		RFWERR_SU_AMCU_AUTH_DIS,
		L"タイムアウトエラーが発生しました。MCUとのデバッガ認証切れ状態です。",
		L"A timeout error occurred. Debugger authentication with the MCU is invalid."
	},

	{
		RFWERR_SU_BMCU_AUTH_DIS,
		L"MCUとのデバッガ認証切れ状態です。",
		L"Debugger authentication with the MCU is invalid."
	},

	{
		RFWERR_SU_ATARGET_NOCONNECT,
		L"タイムアウトエラーが発生しました。ターゲットシステムと接続されていません。",
		L"A timeout error occurred. The emulator is not connected to the target system."
	},

	{
		RFWERR_SU_ATARGET_POWER,
		L"タイムアウトエラーが発生しました。ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できません。",
		L"A timeout error occurred. The command cannot be executed since the power supply to the MCU on the target system is turned off."
	},

	{
		RFWERR_SU_ATARGET_RESET,
		L"タイムアウトエラーが発生しました。ターゲットシステム上のリセット端子が 'L'アクティブ状態のためコマンド処理を実行できません。",
		L"A timeout error occurred. The command cannot be executed since the signal level on the reset pin on the target system is low-active."
	},

	{
		RFWERR_SU_ATARGET_RESET_ST,
		L"タイムアウトエラーが発生しました。ターゲットシステム上のリセット端子が一時的に 'L'アクティブ状態となったためコマンド処理を実行できません。",
		L"A timeout error occurred. The command cannot be executed since the signal level on the reset pin on the target system was temporarily low-active."
	},

	{
		RFWERR_SU_SWD_TRANSFER,
		L"SWD通信でエラーが発生しました。エラーが特定できません。",
		L"An error occurred in SWD communications. The error cannot be identified."
	},

	{
		RFWERR_SU_DAP_STICKYERR,
		L"デバッガからのアクセスでアクセス違反(DAPのSTICKYエラー)が発生しました。以下の発生要因が考えられます。・未定義領域へのアクセスが発生した。・アクセス権限がない資源へのアクセスが発生した。・アクセスプロテクト有効資源へのアクセスが発生した。",
		L"Access violation (sticky error in the DAP) occurred with access from the debugger. Any of items listed below might have caused the violation.- Access to an undefined area was made.- Access to an unauthorized resource was made.- Access to a protected resource was made."
	},

	{
		RFWERR_SU_DAP_WAIT_TIMEOUT,
		L"ターゲットMCUとの通信でACKがOKにならずにタイムアウトが発生しました。以下の発生要因が考えられます。・未定義領域へのアクセスが発生した。・リセットと競合して、デバッグバスが停止状態になった。",
		L"A timeout error occurred since ACK is failed in communications with the target MCU. Any of items listed below might have caused the timeout.- Access to an undefined area was made.- The debug bus was stopped since a conflict occurred with a reset."
	},

	{
		RFWERR_SU_DAP_PROTOCOL_ERR,
		L"JTAG/SWD通信でプロトコルエラーが発生しました。エラーが特定できません。",
		L"A protocol error occurred in JTAG/SWD communications. The error cannot be identified."
	},

	{
		RFWERR_SU_SWD_FAULT,
		L"SWD通信のACKでFAULTが発生しました。エラーが特定できません。",
		L"A fault error occurred in ACK of SWD communications. The error cannot be identified."
	},

	{
		RFWERR_SU_MON_TIMEOUT,
		L"モニタプログラム処理でタイムアウトが発生しました。エラーが特定できません。",
		L"A timeout error occurred during monitor program processing. The error cannot be identified."
	},

	{
		RFWERR_SU_BFW_TIMEOUT,
		L"エミュレータファームウェアでタイムアウトが発生しました。",
		L"A timeout error occurred in emulator firmware."
	},

	{
		RFWERR_SU_DAP_STICKYERR_TIMEOUT,
		L"タイムアウトが発生しました。デバッガからのアクセスでアクセス違反(DAPのSTICKYエラー)が発生しました。以下の発生要因が考えられます。・未定義領域へのアクセスが発生した。・アクセス権限がない資源へのアクセスが発生した。・アクセスプロテクト有効資源へのアクセスが発生した。",
		L"A timeout error occurred. Access violation (sticky error in the DAP) occurred with access from the debugger. Any of items listed below might have caused the violation.- Access to an undefined area was made.- Access to an unauthorized resource was made.- Access to a protected resource was made."
	},

	{
		RFWERR_SU_MULTI_RFW_REQ_TIMEOUT,
		L"タイムアウトが発生しました。RFWの複数処理が、指定された時間内に終了しませんでした。",
		L"A timeout error occurred.Multiple processing of RFW did not finish within the specified time."
	},

	{
		RFWERR_D_COM,
		L"エミュレータとの通信時に通信エラーが発生しました(RFWERR_COM)。デバッガを切断した後、USBケーブルを再接続してください。その後エミュレータの電源を入れ直し、デバッガを再起動してください。",
		L"A communication error occurred while communicating with the emulator (RFWERR_COM). Disconnect the debugger and reconnect the USB cable. Then turn the emulator on again to restart the debugger. "
	},

	{
		RFWERR_D_COM_1ST,
		L"エミュレータとの最初のコマンド送受信時に通信エラーが発生しました。・エミュレータと通信できません。エミュレータの電源などを確認してください。・エミュレータと通信できません。インタフェース・ボード用のデバイス・ドライバが正しくインストールされているか確認してください。",
		L"A communication error occurred when sending or receiving the first command to or from the emulator.- Communication with the emulator cannot be made. Check the power to the emulator, etc.- Communication with the emulator cannot be made. Check if the device driver for the interface board is correctly installed."
	},

	{
		RFWERR_D_COM_USBT_CHANGE,
		L"USB転送サイズ変更後に通信エラーが発生ました。",
		L"A communication error occurred after changing the USB transfer size."
	},

	{
		RFWERR_D_COMDATA_CMDCODE,
		L"エミュレータファームから想定外のコマンドコードを受信しました。",
		L"Command code received from the emulator firmware was unexpected."
	},

	{
		RFWERR_D_COMDATA_ERRCODE,
		L"エミュレータファームから想定外のエラーコードを受信しました。",
		L"Error code received from the emulator firmware was unexpected."
	},

	{
		RFWERR_N_COM_LIST,
		L"既にエミュレータに接続済のため、エミュレータの接続リストが取得できません。",
		L"Since the debugger has already been connected to the emulator, a list of emulators to be connected cannot be obtained."
	},

	{
		RFWERR_N_BFWFILE_NOT_FOUND,
		L"エミュレータファームウェアファイルが見つかりません。デバッガを再インストールしてください。",
		L"The emulator firmware file cannot be found. Reinstall the debugger."
	},

	{
		RFWERR_N_FPGAFILE_NOT_FOUND,
		L"エミュレータ用FPGAファイルが見つかりません。デバッガを再インストールしてください。",
		L"The FPGA file for the emulator cannot be found. Reinstall the debugger."
	},

	{
		RFWERR_N_FILE_NOT_FOUND,
		L"エミュレータ用外部ファイル(ファイル拡張子:MCU,ENV,GRP,MLB,MRG,DBG)が見つかりません。デバッガを再インストールしてください。",
		L"External files for the emulator (with file extensions of MCU, ENV, GRP, MLB, MRG, and DBG) are not found. Reinstall the debugger."
	},

	{
		RFWERR_N_SECTION_NOT_FOUND,
		L"エミュレータ用外部ファイル内に該当するセクションが見つかりません。デバッガを再インストールしてください。",
		L"The corresponding section is not found in external files for the emulator. Reinstall the debugger."
	},

	{
		RFWERR_N_BASP_ASPOFF,
		L"E2拡張機能が無効状態のためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the extended facilities of the E2 emulator are disabled.",
	},

	{
		RFWERR_N_BASP_PINDISABLE,
		L"指定のE2拡張インターフェースは無効状態のため設定できません。",
		L"The specified E2 expansion interface pin cannot be set because it is disabled.",
	},

	{
		RFWERR_N_BASP_PINNOUSE,
		L"指定したE2拡張インターフェースは使用されていないためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the specified E2 expansion interface pin is not used.",
	},

	{
		RFWERR_N_BASP_CHNOUSE,
		L"指定したチャネルは使用されていないためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the specified channel is not used.",
	},

	{
		RFWERR_N_BASP_FACT_NOSUPPORT,
		L"指定したE2トリガはサポートしていません。",
		L"The specified E2 trigger is not supported.",
	},

	{
		RFWERR_N_BASP_FACT_DISABLE,
		L"指定したE2トリガは無効(条件が未設定)であるため使用できません。",
		L"The specified E2 trigger cannot be used because it is disabled (no condition is set).",
	},

	{
		RFWERR_N_BASP_FACT_NONSEL,
		L"E2トリガが指定されていません。",
		L"No E2 trigger has been specified.",
	},

	{
		RFWERR_N_BASP_SGLLNK_ILLEGAL,
		L"シングルE2トリガに複数個の要因を設定できません。",
		L"Multiple causes cannot be set for an E2 single-trigger.",
	},

	{
		RFWERR_N_BASP_MLTLNK_USED,
		L"指定したマルチE2トリガ・アクションはすでに使用されているため指定できません。",
		L"The specified E2 multiple-trigger/action is already used.",
	},

	{
		RFWERR_N_BASP_FACT_CANTCLR,
		L"E2トリガはE2アクションにリンクされているため消去できません。",
		L"The E2 trigger cannot be cleared because it is linked to an E2 action.",
	},

	{
		RFWERR_N_BASP_LFACT_CANTCHG,
		L"E2トリガがレベル型アクションにリンクされているため変更できません。",
		L"The E2 trigger cannot be changed because it is linked to a level-type operation.",
	},

	{
		RFWERR_N_BASP_PFACT_CANTCHG,
		L"E2トリガがパルス型アクションにリンクされているため変更できません。",
		L"The E2 trigger cannot be changed because it is linked to a pulse-type operation.",
	},

	{
		RFWERR_N_BASP_LFACT_CANTSET,
		L"パルス型アクションにレベル型E2トリガを設定できません。",
		L"A pulse-type event cannot be set for a level-type operation.",
	},

	{
		RFWERR_N_BASP_PFACT_CANTSET,
		L"レベル型アクションにパルス型E2トリガを設定できません。",
		L"A level-type event cannot be set for a pulse-type operation.",
	},

	{
		RFWERR_N_BASP_BRKEV_NOUSE,
		L"E2拡張機能のブレークイベントは使用されていないためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the break event for extended facilities of the E2 emulator has not been used.",
	},

	{
		RFWERR_N_BASP_STOPTRCOFF,
		L"E2拡張機能のトレース停止機能が無効状態のためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the trace stop event for extended facilities of the E2 emulator is disabled.",
	},

	{
		RFWERR_N_BASP_STOPTRC_NOUSE,
		L"E2拡張機能のトレース停止は使用されていないためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the trace stop event for extended facilities of the E2 emulator has not been used.",
	},

	{
		RFWERR_N_BASP_PINEXCLUSIVE,
		L"指定した機能は端子共通のため同時に使用できません。",
		L"The specified function cannot be used simultaneously because another facility uses the same pin.",
	},

	{
		RFWERR_N_BASP_FUNCEXCLUSIVE,
		L"指定した機能は排他機能であるため同時に使用できません。",
		L"The specified function cannot be used simultaneously with another  facility because it is an exclusive facility.",
	},

	{
		RFWERR_N_BASP_FUNC_USED,
		L"無効化しようとした機能は使用中のため変更できません。",
		L"The function which was attempted to be disabled cannot be changed because it is in use.",
	},

	{
		RFWERR_N_BASP_PWRMONOFF,
		L"消費電流測定機能が無効状態のためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the Current Consumption Measurement facility is disabled.",
	},

	{
		RFWERR_N_BASP_PWRMON_NOUSE,
		L"消費電流測定は使用されていないためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the Current Consumption Measurement is not used.",
	},

	{
		RFWERR_N_BASP_CANMONOFF,
		L"CANモニタ機能が無効状態のためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the CAN monitoring facility is disabled.",
	},

	{
		RFWERR_N_BASP_CANMON_NOUSE,
		L"指定したCANモニタのチャネルは使用されていないためコマンドを実行できません。",
		L"Command processing cannot be executed because the specified CAN monitor channel is not used.",
	},

	{
		RFWERR_N_BASP_SWTRCOFF,
		L"ソフトウェア・トレース(LPD)が無効状態のためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the external software tracing (LPD) facility is disabled.",
	},

	{
		RFWERR_N_BASP_MONEVOFF,
		L"通過ポイントが無効状態のためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the monitor point is disabled.",
	},

	{
		RFWERR_N_BASP_ROMWRITE_DISABLE,
		L"デバッガの「フラッシュ書き換えを許可する」オプションをいいえに選択した場合、コマンドを実行できません。",
		L"Command processing cannot be executed if [No] is selected for the [Permit Flash Programming] option of the debugger.",
	},

	{
		RFWERR_N_BASP_MONEV_NONOP,
		L"指定したアドレスがNOP命令ではないので、ソフトウェアブレーク方式の通過ポイントが設定できません。",
		L"A monitor point cannot be set in software break mode because the specified address does not hold a NOP instruction.",
	},

	{
		RFWERR_N_BASP_MONEV_BREAKADDR,
		L"デバッグ機能のブレークポイントに設定しているアドレスに通過ポイントは設定できません。",
		L"A monitor point cannot be set to address to which a breakpoint of the debugging facility has been set.",
	},

	{
		RFWERR_N_BASP_MONEV_OVERLAPP,
		L"他の通過ポイント条件番号に指定済みのアドレスです。",
		L"The address is already specified for another monitor point condition number.",
	},

	{
		RFWERR_N_BASP_MONEV_NOTSET,
		L"指定した通過ポイント条件番号が設定されていません。",
		L"The specified monitor point condition number was not set.",
	},

	{
		RFWERR_N_BASP_LOAD,
		L"記録ファイルがロードされていません。",
		L"File not loaded.",
	},

	{
		RFWERR_N_BASP_ALREADY_LOAD,
		L"記録ファイルがすでにロードされています。",
		L"File loaded.",
	},

	{
		RFWERR_N_BASP_F_OPEN,
		L"記録ファイルのオープンに失敗しました。",
		L"File open error.",
	},

	{
		RFWERR_N_BASP_F_SEEK,
		L"記録ファイルのデータ検索に失敗しました。",
		L"File seek error.",
	},

	{
		RFWERR_N_BASP_F_READ,
		L"記録ファイルの読み出しに失敗しました。",
		L"File read error.",
	},

	{
		RFWERR_N_BASP_F_WRITE,
		L"記録ファイルの書き込みに失敗しました。",
		L"File write error.",
	},

	{
		RFWERR_N_BASP_F_CLOSE,
		L"記録ファイルのクローズに失敗しました。",
		L"File close error.",
	},

	{
		RFWERR_N_BASP_F_FORMAT,
		L"記録ファイルのフォーマットが不正です。",
		L"File format error.",
	},

	{
		RFWERR_N_BASP_FPGA_START2,
		L"FPGAのフレーム異常です(スタートビット)。",
		L"FPGA frame error (start bit).",
	},

	{
		RFWERR_N_BASP_FPGA_SEQ,
		L"FPGAのフレーム異常です(フレームタイプ)。",
		L"FPGA frame error (frame type).",
	},

	{
		RFWERR_N_BASP_FPGA_SIZE,
		L"FPGAのフレーム異常です(サイズ)。",
		L"FPGA frame error (size).",
	},

	{
		RFWERR_N_BASP_PWRMON_E2PWROFF,
		L"E2エミュレータからターゲットへ電源供給していないため、消費電流測定を有効に設定できません。",
		L"The Current Consumption Measurement facility cannot be set effectively because the E2 emulator does not supply power to the target.",
	},

	{
		RFWERR_N_BASP_ASPIF_PWROFF,
		L"E2拡張インターフェースに電源供給していないため、E2拡張機能が使用できません。",
		L"The extended facilities of the E2 emulator are disabled because power is not supplied to the E2 expansion interface.",
	},

	{
		RFWERR_N_BASP_ASPFUNCOFF,
		L"指定した機能が無効状態のため、コマンド処理を実行できません。",
		L"Command processing cannot be executed because the specified facilities are disabled.",
	},

	{
		RFWERR_N_BASP_TSTAMP_OVER,
		L"タイムスタンプがオーバーフローしたため、計測結果は返せません。",
		L"A measurement result cannot be returned because a timestamp has overflowed.",
	},

	{
		RFWERR_N_BASP_NOSUPPORT,
		L"指定したE2拡張機能はサポートしていません。",
		L"The specified extended facilities of the E2 emulator is not supported.",
	},

	{
		RFWERR_N_BASP_NONSEL_ACTLEVEL,
		L"外部トリガ出力のアクティブレベルが未選択のためコマンド処理を実行できません。",
		L"Command processing cannot be executed because the active level is not selected for the external trigger output.",
	},

	{
		RFWERR_N_BASP_COMB_NOSUPPORT,
		L"指定したE2トリガとの組み合わせはサポートしていません。",
		L"Combination with the specified E2 trigger is not supported.",
	},

	{
		RFWERR_N_BASP_SRAM_OVF,
		L"FPGAのSRAMオーバーフローが発生しました。",
		L"An SRAM overflow has occurred in the FPGA.",
	},

	{
		RFWERR_N_BASP_SFIFO_OVF,
		L"FPGAのSFIFOオーバーフローが発生しました。",
		L"An SFIFO overflow has occurred in the FPGA.",
	},

	{
		RFWERR_N_BASP_SWT_NOREC,
		L"ソフトウェア・トレース(LPD)が記録できない状態です。ソフトウェア・トレース(LPD)の記録を有効にする場合は、デバッガを再起動してください。",
		L"Software tracing (LPD) cannot be recorded. To enable recording software tracing(LPD), restart the debugger.",
	},
	
	{
		RFWERR_W_RAM_WAIT_TMP_CHANGE,
		L"(Warning)RAMのウェイト設定を一時的に変更してコマンドを実行しました。",
		L"(Warning message)The wait setting of RAM was changed temporarily to execute this command."
	},

	{
		RFWERR_W_ROM_WAIT_TMP_CHANGE,
		L"(Warning)ROMのウェイト設定を一時的に変更してコマンドを実行しました。",
		L"(Warning message)The wait setting of ROM was changed temporarily to execute this command."
	},

	{
		RFWERR_N_SRM_USE_WORKRAM,
		L"(Warning)ワークRAMは既に別の用途で使用しています。",
		L"(Warning message)The work RAM is already being used for another purpose."
	},


	{
		RFWERR_N_ASP_DISABLE_ASYNC,
		L"非同期デバッグモード中のためE2拡張機能を有効化できません。",
		L"The extended facilities of the E2 emulator cannot be enabled in asynchronous debugging mode.",
	},

	{
		RFWERR_N_ASP_DISABLE_EMU,
		L"E2エミュレータ以外ではE2拡張機能は有効化できません。",
		L"The extended facilities cannot be enabled in an emulator other than the E2 emulator.",
	},

	{
		RFWERR_N_MEMORY_NOT_SECURED,
		L"PC上のメモリを確保できませんでした。",
		L"Failed to allocate a memory area on the PC.",
	},

	// 以下のエラーは一番最後にすること
	{
		RFWERR_N_NOMESSAGE_ERROR,
		L"エラーメッセージ未登録のエラーが発生しました。",
		L"No message error has occurred."
	}
};

//=============================================================================
/**
 * メッセージテーブル格納構造体のポインタを取得
 *
 * @param なし
 * @return ERRMSG_DATA *  メッセージテーブル格納構造体のポインタ
 */
//=============================================================================
const ERRMSG_DATA* GetErrMsgPtr(void)
{
	return &s_ERRMSG_DATA[0];
}
