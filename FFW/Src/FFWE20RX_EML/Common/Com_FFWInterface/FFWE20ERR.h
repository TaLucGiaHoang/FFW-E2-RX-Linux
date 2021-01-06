///////////////////////////////////////////////////////////////////////////////
/**
 * @file FFWE20ERR.h//
 * @brief E1/E20/E2/E2 Liteエミュレータ FFWエラーコードの定義ファイル//
 * @author PIS T.Okugawa RSD Y.Minami, K.Okita(PA K.Tsumu), H.Hashiguchi, Y.Miyake, S.Ueda, PA M.Tsuji //
 * @author Copyright (C) 2009(2010-2016) Renesas Electronics Corporation. All rights reserved.//
 * @date 2017/10/04 //
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef	__FFWE20ERR_H__
#define	__FFWE20ERR_H__
/*
■改定履歴//
・ソース構成改善	2012/07/10 橋口//
　　ベース:FFWE20RX600.dll V.1.02.00.015//
・V.1.03 RevNo111121-008　2012/07/11 橋口 OFS1 LVDASチェック追加 //
・V.1.04 RevRxNo120606-008　2012/07/12 橋口 USBブートコードが書かれている場合のユーザブート起動処理対応//
・RevRxNo120910-001	2012/09/27 三宅//
  FFW I/F仕様変更に伴うFFWソース変更。//
  ・以下のエラーコードを追加。//
    FFWERR_READ_CPUACC_MCURUN, FFWERR_USB_VBUS_LOW, FFWERR_FFWPARA_OVER, //
    FFWERR_ROM_OFS1_CHANGE, FFWERR_ROM_MDE_OFS1_CHANGE, //
    FFWERR_WRITE_OFS1_LVD1REN_CHANGE, FFWERR_WRITE_OFS1_LVD1REN_ENA。//
・RevRxNo120928-002 2012/09/28 上田 //
　・EZ-CUBE用エラーコード追加。//
	FFWERR_BFW_MCU_UNMATCH, FFWERR_BFW_VER_UNMATCH, FFWERR_FUNC_UNSUPORT //
・RevRxNo120910-006 2012/10/16 三宅 //
  ・以下のエラーコードを追加。//
    FFWERR_WRITE_SRM_WORKRAM //
・RevRxNo130301-001 2013/03/26 上田//
	RX64M対応//
・RevRxNo130308-001 2013/05/20 三宅 //
  ・以下のエラーコードを追加。//
    FFWERR_RTT_UNSUPPORT, FFWERR_CVAREA_SAME, FFWERR_CVBLK_DIS, FFWERR_CVDATA_NON, //
	FFWERR_CV_LOST, FFWERR_CV_MEASURE, FFWERR_CV_LOST_MEASUREFFWERR_CV_UNSUPPORT //
・RevRxNo130301-002 2013/11/18 上田 //
  ・以下のエラーコードを追加。//
	FFWERR_HOTPLUG_OSM_NOT_CHANGE //
・RevRxNo130730-009 2013/11/18 大喜多 //
  ・以下のエラーコードを追加。//
	FFWERR_MEMORY_NOT_SECURED //
・RevRxNo140109-001 2014/01/09 上田//
  ・以下のエラーコードを追加。//
	FFWERR_WRITE_OFS1_LVDAS_CHANGE, FFWERR_DWN_OSMDATA_NON //
・RevRxNo140617-001	2014/06/18 大喜多//
	TrustedMemory機能対応//
・RevRxNo140515-005 2014/06/26 大喜多//
	RX71M対応(メモリウェイト必要領域対応)//
・RevRxE2LNo141104-001 2014/12/22 上田//
	E2 Lite対応//
・RevRxNo150928-001 2015/09/28 PA 辻//
	ROMキャッシュインバリデート Warning対応//
	対応見送りのため、コメントアウト//
・RevRx6z4No160105-001 2016/03/14 PA紡車//
	RX6Z4実チップ対応//
・RevRx6z4No160412-001 2016/04/15 PA 辻, 紡車//
	RX6Z4 6/B客先提供版対応//
・RevRxNo161003-001 2016/12/01 PA 紡車//
　　RX651-2MB 起動時のバンクモード、起動バンク対応//
・RevRxNo170829-001 2017/08/29 山本//
    FFWERR_BFW_FILE_NOT_FOUNDエラー追加//
・RevRxE2No171004-001 2017/10/04 PA 辻//
	E2拡張機能対応//
・RevRxMultiNo180302-001 2018/03/22 PA紡車//
	RXマルチコア対応//
*/

//-----------------------------------------------
// 正常終了//
//-----------------------------------------------
#define FFWERR_OK					0x00000000		// コマンド処理を正常終了した//


//-----------------------------------------------
// コマンドエラー//
//-----------------------------------------------
#define FFWERR_FFW_ARG				0x01000000		// 引数の指定に誤りがある//
#define FFWERR_BFW_CMD				0x01000001		// 未定義のBFWコマンドを受信した//
// RevRxNo120910-001 Append Line//
#define FFWERR_FFWPARA_OVER			0x01000002		// FFWに設定可能なパラメータサイズを超えていた(Warning)//

//-----------------------------------------------
// エミュレータ内部資源のアクセスエラー//
//-----------------------------------------------
// 共通項目//

// ベース基板に関するエラー//
#define FFWERR_BFWFROM_ERASE		0x01010100		// BFW格納用フラッシュROMのイレーズエラー//
#define FFWERR_BFWFROM_WRITE		0x01010101		// BFW格納用フラッシュROMの書き込みエラー//
// RevRxE2LNo141104-001 Append Line//
#define FFWERR_BFWFROM_VERIFY		0x01010102		// BFW格納用フラッシュROMのベリファイエラー//

//RevNo100715-009 Modify Line//
#define FFWERR_FPGA_CONFIG			0x01010103		// FPGAコンフィグレーションエラー//
// RevRxNo120910-001 Append Line//
#define FFWERR_USB_VBUS_LOW			0x01010104		// USBバスパワー電圧が低い(Warning)//
// RevRxE2LNo141104-001 Append Line//
#define FFWERR_LID_UNDEFINE			0x01010105		// ライセンスIDが定義されていない//

// MCU基板に関するエラー//
// RevRxE2No171004-001 Append Start//
#define FFWERR_NID_BLANK			0x01010200		// ニックネームがブランク状態で設定されていない//
#define FFWERR_NIDSIZE_OVER			0x01010201		// ニックネームの文字数が許容範囲を超えている//
#define FFWERR_FLAG_BLANK			0x01010202		// フラグがブランク状態で設定されていない//
// RevRxE2No171004-001 Append End//

#define FFWERR_MCUROM_ERASE			0x01010206		// ターゲットMCU内蔵フラッシュROMのイレーズエラー//
#define FFWERR_MCUROM_WRITE			0x01010207		// ターゲットMCU内蔵フラッシュROMの書き込みエラー//
#define FFWERR_MCUROM_PCLK_ILLEGAL	0x01010209		// ターゲットMCU内蔵フラッシュROMの周辺クロック指定範囲外エラー//
#define	FFWERR_CLK_CHG_DISENA		0x01010210		//	ダウンロード時のクロックソース切り替え不可エラー//
#define	FFWERR_EXTAL_OUT			0x01010211		//	ダウンロード時のEXTAL切り替え不可エラー//

// シリアル番号に関するエラー//
#define FFWERR_SNSIZE_OVER			0x01010800		// シリアル番号の文字数が許容範囲を超えている//
// RevRxE2LNo141104-001 Append Line//
#define FFWERR_SN_BLANK				0x01010801		// シリアル番号がブランク状態で設定されていない//

// ベース基板(E1/E20)に関するエラー2//
// RevRxNo120928-002 Append Start//
#define FFWERR_BFW_MCU_UNMATCH		0x01010900		// エミュレータに格納されたF/Wがデバッグ対象のMCU用ではない。※EZ-CUBEでのみ使用(INITでエラーを返す)//
#define FFWERR_BFW_VER_UNMATCH		0x01010901		// エミュレータに格納されたF/Wのバージョンが一致していない。※EZ-CUBEでのみ使用(INITでエラーを返す)//
// RevRxNo120928-002 Append End//
#define FFWERR_TMRCTRL_OVER			0x01010902		// 既にE1/E20内部の全てのタイマ(コンペアマッチA,B)が使用されている。//

// RevRxE2No171004-001 Append Start//
// 拡張ボードに関するエラー//
#define FFWERR_EX1BFWFROM_ERASE		0x01010A00		// 拡張ボードF/W格納用フラッシュROMのイレーズエラー//
#define FFWERR_EX1BFWFROM_WRITE		0x01010A01		// 拡張ボードF/W格納用フラッシュROMの書き込みエラー//
#define FFWERR_EX1BFWFROM_VERIFY	0x01010A02		// 拡張ボードF/W格納用フラッシュROMのベリファイエラー//
#define FFWERR_EX1BFPGA_CONFIG		0x01010A03		// 拡張ボードFPGAコンフィグレーションエラー//
#define FFWERR_EX1BEROM_ERASE		0x01010A04		// 拡張ボードEEPROMのイレーズエラー//
#define FFWERR_EX1BEROM_WRITE		0x01010A05		// 拡張ボードEEPROMの書き込みエラー//
#define FFWERR_EX1BEROM_VERIFY		0x01010A06		// 拡張ボードEEPROMのベリファイエラー//
#define FFWERR_EX1BEROM_READ		0x01010A07		// 拡張ボードEEPROMのリードエラー//
#define FFWERR_EX1BEROM_NORES		0x01010A08		// 拡張ボードEEPROMの応答なしエラー//
#define FFWERR_EEPROM_WRITE			0x01010A09		// EEPROMのライトエラー//
#define FFWERR_EEPROM_READ			0x01010A0A		// EEPROMのリードエラー//

// 2nd拡張ボードに関するエラー//
#define FFWERR_EX2BFPGA_CONFIG		0x01010B00		// 2nd拡張ボードFPGAコンフィグレーションエラー//
#define FFWERR_SPBFPGAFROM_ERASE	0x01010B01		// SPボードFPGA格納用フラッシュROMのイレーズエラー//
#define FFWERR_SPBFPGAFROM_WRITE	0x01010B02		// SPボードFPGA格納用フラッシュROMの書き込みエラー//
#define FFWERR_SPBFPGAFROM_VERIFY	0x01010B03		// SPボードFPGA格納用フラッシュROMのベリファイエラー//
#define FFWERR_EX2BEROM_ERASE		0x01010B04		// 2nd拡張ボードEEPROMのイレーズエラー//
#define FFWERR_EX2BEROM_WRITE		0x01010B06		// 2nd拡張ボードEEPROMの書き込みエラー//
#define FFWERR_EX2BEROM_VERIFY		0x01010B07		// 2nd拡張ボードEEPROMのベリファイエラー//
#define FFWERR_EX2BEROM_READ		0x01010B08		// 2nd拡張ボードEEPROMのリードエラー//
#define FFWERR_EX2BEROM_NORES		0x01010B09		// 2nd拡張ボードEEPROMの応答なしエラー//

#define FFWERR_PODFPGAFROM_ERASE	0x01010C00		// エミュレーションポッドFPGA格納用フラッシュROMのイレーズエラー//
#define FFWERR_PODFPGAFROM_WRITE	0x01010C01		// エミュレーションポッドFPGA格納用フラッシュROMの書き込みエラー//
#define FFWERR_PODFPGAFROM_VERIFY	0x01010C02		// エミュレーションポッドFPGA格納用フラッシュROMのベリファイエラー//

#define FFWERR_N_UNSUPPORT			0x01010D00		// 指定機能はサポートされていません。//
// RevRxE2No171004-001 Append End//

//-----------------------------------------------
// MCU内蔵資源のアクセスエラー//
//-----------------------------------------------
#define FFWERR_ILLEGAL_IDCODE		0x01020004		// ID認証コード不一致//
#define FFWERR_JTAG_ILLEGAL_IR		0x01020005		// JTAG IRの戻り値が不正(MCUとのJTAG通信ができていない)//
#define FFWERR_ILLEGAL_DEVICEID		0x01020006		// デバイスIDコード不一致//

#define FFWERR_AMCU_NO_DBG			0x01020009		// MCUとエミュレータの接続に失敗した。//
#define FFWERR_ROM_MDE_CHANGE		0x0102000A		// MCU内蔵フラッシュROMのMDEレジスタMDEビットを指定された内容に書き換えた(Warning)//
#define FFWERR_ROM_UBCODE_CHANGE	0x0102000B		// MCU内蔵フラッシュROMのUBコードを指定された内容に書き換えた(Warning)//
#define FFWERR_ROM_MDE_UBCODE_CHANGE	0x0102000C	// MCU内蔵フラッシュROMのMDEレジスタ及びUBコードを指定された内容に書き換えた(Warning)//
#define FFWERR_DAUTH_FAIL			0x0102000D		// ID認証が正常に終了しなかった//
// RevRxNo120606-008 Append Line
#define FFWERR_ROM_UBCODE_USBBOOT	0x0102000E		// MCU内蔵フラッシュROMのUBコード領域にUSBブートコードが書かれているため起動できません。 //
// RevRxNo130301-002 Append Line
#define FFWERR_HOTPLUG_OSM_NOT_CHANGE	0x01020018	// MCU内蔵フラッシュROMのオプション設定メモリを指定された値に書き換えずにホットプラグ起動した(Warning) //
#define FFWERR_MCU_CONNECT_BAN		0x01020019		// MCUがエミュレータ接続禁止状態である//
// RevRxNo120910-001 Append Start//
#define FFWERR_ROM_OFS1_CHANGE		0x0102001A		// MCU内蔵フラッシュROMのOFS1レジスタを書き換えた(Warning)//
#define FFWERR_ROM_MDE_OFS1_CHANGE	0x0102001B		// MCU内蔵フラッシュROMのMDEレジスタMDEビットおよびOFS1レジスタを書き換えた(Warning)//
// RevRxNo120910-001 Append End//
// RevRxNo161003-001 Append Start//
#define FFWERR_ROM_BANKMD_CHANGE				0x0102001C		// MCU内蔵フラッシュROMのMDEレジスタBANKMDビットを指定された内容に書き換えた(Warning)//
#define FFWERR_ROM_BANKSWP_CHANGE				0x0102001D		// MCU内蔵フラッシュROMのBANKSELレジスタBANKSWPビットを指定された内容に書き換えた(Warning)//
#define FFWERR_ROM_MDE_BANKMD_CHANGE			0x0102001E		// MCU内蔵フラッシュROMのMDEレジスタMDEビットおよびBANKMDビットを指定された内容に書き換えた(Warning)//
#define FFWERR_ROM_MDE_BANKSWP_CHANGE			0x0102001F		// MCU内蔵フラッシュROMのMDEレジスタMDEビットおよびBANKSELレジスタBANKSWPビットを指定された内容に書き換えた(Warning)//
#define FFWERR_ROM_BANKMD_BANKSWP_CHANGE		0x01020020		// MCU内蔵フラッシュROMのMDEレジスタBANKMDビットおよびBANKSELレジスタBANKSWPビットを指定された内容に書き換えた(Warning)//
#define FFWERR_ROM_BANKMD_OFS1_CHANGE			0x01020021		// MCU内蔵フラッシュROMのMDEレジスタBANKMDビットおよびOFS1レジスタを書き換えた(Warning)//
#define FFWERR_ROM_MDE_BANKMD_BANKSWP_CHANGE	0x01020022		// MCU内蔵フラッシュROMのMDEレジスタMDEビット、BANKMDビットおよびBANKSELレジスタBANKSWPビットを指定された内容に書き換えた(Warning)//
#define FFWERR_ROM_MDE_BANKMD_OFS1_CHANGE		0x01020023		// MCU内蔵フラッシュROMのMDEレジスタMDEビット、BANKMDビットおよびOFS1レジスタを書き換えた(Warning)//
#define FFWERR_ROM_BANKMD_NOT_CHANGE			0x01020024		// TM機能有効のため、MCU内蔵フラッシュROMのMDEレジスタBANKMDビットを指定された内容に書き換えられません。//
#define FFWERR_ROM_BANKSWP_NOT_CHANGE			0x01020025		// TM機能有効かつFFEE0000h～FFEEFFFFhのTM機能無効のため、MCU内蔵フラッシュROMのBANKSELレジスタBANKSWPビットを指定された内容に書き換えられません。//
// RevRxNo161003-001 Append End//

// RevRxNo150928-001 Append Start +2
#if 0
#define FFWERR_ROMC_BAN_INVALIDATE_PERMIT	0x0102000F	// ROMキャッシュ動作を禁止に変更後インバリデート実施し、ROMキャッシュ動作を許可に戻した(Warning)//
#define FFWERR_ROMC_INVALIDATE		0x01020010		// ROMキャッシュインバリデートを実施した(Warning)//
#endif
// RevRxNo150928-001 Append End

//-----------------------------------------------
// デバッグ機能のエラー//
//-----------------------------------------------
// 共通項目//

// メモリアクセスに関するエラー	//
#define FFWERR_WRITE_VERIFY			0x01030100		// ベリファイエラー//
#define FFWERR_WRITE_ROMAREA_MCURUN	0x01030101		// ユーザプログラム実行中にMCU内蔵ROM領域への書き込みは実行できない//
#define FFWERR_INSTCODE_FAIL		0x01030103		// 命令解析できないコードだった//
#define FFWERR_DATAFLASH_MCURUN		0x01030104		// ユーザプログラム実行中のためデータフラッシュROM領域にアクセスできない//
#define FFWERR_READ_DTF_MCURUN		0x01030106		// ユーザプログラム実行中に読み出し禁止状態でデータフラッシュ領域へのリード操作が発生した(Warning)//

/////////////////////////////////////////////////////////////////////
// Ver.1.01 2010/08/17 SDS T.Iwata
#define FFWERR_FWRITE_NOTEXE_ILGLERR	0x01030107	// MCU内蔵フラッシュROMへのアクセス違反などが発生しているため、フラッシュ書き換えが実行できない//
#define FFWERR_FWRITE_FAIL_ILGLERR		0x01030108	// MCU内蔵フラッシュROMへのアクセス違反が発生し、フラッシュ書き換えが正常に実行できなかった//
/////////////////////////////////////////////////////////////////////

#define FFWERR_ACC_FLASH_ILGLERR		0x01030112	// MCU内蔵フラッシュROMへのアクセス違反などが発生している(ILGLERRビットが"1")(Warning)//
#define FFWERR_WRITE_MDE_NOTMATCH		0x01030113	// MDEレジスタへのライトデータが起動時に指定されたエンディアンと不一致である//
#define FFWERR_WRITE_UBCODE_NOTMATCH	0x01030114	// UBコード領域へのライトデータが起動時に指定された内容と不一致である//
#define FFWERR_WRITE_MDE_CHANGE			0x01030115	// MDEレジスタへのライトデータを起動時に指定されたエンディアンに変更した(Warning)//
#define FFWERR_WRITE_UBCODE_CHANGE		0x01030116	// UBコード領域へのライトデータを起動時に指定された内容に変更した(Warning)//
#define FFWERR_WRITE_MDE_UBCODE_CHANGE	0x01030117	// MDEレジスタ及びUBコード領域へのライトデータを起動時に指定された内容に変更した(Warning)//
#define FFWERR_ACC_ROM_PEMODE			0x01030118	// フラッシュ書き換えモード中のため、MCU内蔵ROM領域の読み出し/書き込み実行はできない//
#define FFWERR_READ_ROM_FLASHDBG_MCURUN	0x01030119	// フラッシュROM書き換えデバッグのユーザプログラム実行中にMCU内蔵ROM領域へのリード操作が発生した(Warning)//
// V.1.02 RevNo110613-001 Append Line
#define FFWERR_CLKCHG_DMB_CLR			0x0103011C	// メモリアクセス中のクロック切り替えなどによる通信エラーが発生したためリカバリした(Warning)//
// V.1.02 RevNo111121-008 Append Line
#define FFWERR_WRITE_OFS1_TARGETVCC_UNDERVDSEL	0x0103011D	//電圧監視0リセットが発生しエミュレータ動作ができなくなる可能性があるため、書き込み処理を中断した。//
// RevRxNo120910-001 Append Start//
#define FFWERR_WRITE_OFS1_LVD1REN_CHANGE		0x0103011E	// OFS1レジスタへのライトデータを起動時電圧監視1リセット無効に変更した(Warning)//
#define FFWERR_WRITE_OFS1_LVD1REN_ENA	0x0103011F	// OFS1レジスタへの起動時電圧監視1リセット有効設定は不可である//
#define FFWERR_READ_CPUACC_MCURUN		0x01030120	// ユーザプログラム実行中にCPUアクセス領域へのリード操作が発生した(Warning)//
// RevRxNo120910-001 Append End//
// RevRxNo120910-006 Append Line//
#define FFWERR_WRITE_SRM_WORKRAM		0x01030121	// 指定ルーチン実行機能用ワークRAM領域への書き込みは実行できない//
// RevRxNo130301-001 Append Line//
#define FFWERR_WRITE_OFS1_LVDAS_ENA		0x01030122	// OFS1レジスタへのリセット後電圧監視0リセット有効設定は不可である//
// RevRxNo140109-001 Append Start //
#define FFWERR_WRITE_OFS1_LVDAS_CHANGE	0x01030123	// OFS1レジスタへのライトデータをリセット後電圧監視リセット無効に変更した(Warning)//
#define FFWERR_DWN_OSMDATA_NON			0x01030124	// オプション設定メモリへのダウンロードデータがない(Warning)//
// RevRxNo140617-001 Append Start //
#define FFWERR_WRITE_TMAREA				0x01030125	// Trusted Memory領域への書き込みは実行できない//
#define FFWERR_DWN_TMAREA				0x01030126	// Trusted Memory領域にはダウンロードデータを書き込んでいません(Warning)//
// RevRxNo140617-001 Append End //
// RevRxNo140515-005 Append Start //
#define FFWERR_RAM_WAIT_TMP_CHANGE		0x01030127	// RAMのウェイト設定を一時的に変更してコマンドを実行した(Warning)//
#define FFWERR_ROM_WAIT_TMP_CHANGE		0x01030128	// ROMのウェイト設定を一時的に変更してコマンドを実行した(Warning)//
// RevRxNo140515-005 Append End //
// RevRxNo140109-001 Append End //
// RevRxE2LNo141104-001 Append Start//
#define FFWERR_MCURAM_VERIFY			0x01030129	// MCU内蔵RAMのベリファイエラー//
#define FFWERR_EXTRAM_VERIFY			0x0103012A	// 外部RAMのベリファイエラー//
// RevRxE2LNo141104-001 Append End//
// RevRx6z4No160105-001 Append Line
#define FFWERR_WRITE_FAW_FSPR_CHANGE	0x0103012B	// FAWレジスタFSPRビットへのライトデータをプロテクトなしに変更した(Warning)//

// RevRx6z4No160412-001 Append Start
#define FFWERR_FAW_FSPR_ALREADY_PROTECT	0x0103012C	// FAWレジスタFSPRビットによるプロテクトがすでに設定されている(Warning)//
#define FFWERR_WRITE_MULTIRUNAREA		0x0103012D	// 同時実行プログラム領域への書き込みはできない//
#define FFWERR_WRITE_MULTIRUNAREA_CHANGE	0x0103012E	// 同時実行プログラム領域データを同時実行プログラムデータに差し替えた(Warning)//
// RevRx6z4No160412-001 Append End
// RevRxNo161003-001 Append Start//
#define	FFWERR_WRITE_BANKMD_CHANGE		0x0103012F	// MDEレジスタBANKMDビットへのライトデータを起動時に指定されたバンクモードに変更した(Warning)//
#define	FFWERR_WRITE_BANKSWP			0x01030130	// BANKSELレジスタBANKSWPビットをライトデータで指定された内容に変更した(Warning)//
#define	FFWERR_WRITE_MDE_BANKSEL_CHANGE		0x01030131	// MDEレジスタまたはBANKSELレジスタへのライトデータを起動時に指定された内容に変更した(Warning)//
#define	FFWERR_WRITE_MDE_BANKMD_CHANGE		0x01030132	// MDEレジスタMDEビットとBANKMDビットへのライトデータを起動時に指定された内容に変更した(Warning)//
#define FFWERR_WRITE_BANKSEL_NOT_CHANGE		0x01030133	// TM機能有効かつFFEE0000h～FFEEFFFFhのTM機能無効のため、BANKSELレジスタBANKSWPビットをライトデータで指定された内容に書き換えられませんでした(Warning)//
// RevRxNo161003-001 Append End//

// RevRxE2No171004-001 Append Start//
#define FFWERR_SRM_USE_WORKRAM			0x01030134	// 指定ルーチン実行機能用ワークRAMを別の用途で使用している//
#define FFWERR_BRKEV_BRK_COMERR			0x01030135	// ブレークイベントによる強制ブレークが発生したため通信エラーが発生した//
// RevRxE2No171004-001 Append End//

// イベントに関するエラー//
#define FFWERR_FFW_EVNO_STATE		0x01030201		// 状態遷移指定イベント番号に誤りがあります//
#define FFWERR_FFW_REPEAT_ADDR_ADDR	0x01030202		// 実行前PCブレークと実行PC通過のイベント番号が重複しています//
#define FFWERR_FFW_REPEAT_ADDR_OPC	0x01030203		// 実行PC通過とオペランドアクセスのイベント番号が重複しています(※累積AND、状態遷移指定時)//
#define FFWERR_FFW_RERFSET_USE_USER	0x01030206		// ユーザプログラム側でパフォーマンス計測カウンタを操作するモードです。パフォーマンス0またはパフォーマンス1の設定を変更することはできません//
#define FFWERR_FFW_REPEAT_COMB_ADDR	0x01030207		// 組み合わせで使用している実行PCイベントと他の実行PCイベントが重複しています。//
#define FFWERR_FFW_REPEAT_COMB_OPC	0x01030208		// 組み合わせで使用しているオペランドイベントと他のオペランドイベントが重複しています。//
// V.1.02 No.10 Append Line
#define FFWERR_EV_UNSUPPORT			0x0103020D		// このイベント機能はサポートしていない//
// RevRxMultiNo180302-001 Append Line //
#define FFWERR_OTHEREV_NOUSE		0x0103020E		// イベント組合せの64ビットデータブレークまたはトレースでは、他のイベントは使用できません。//

// 例外検出に関するエラー//

// ブレーク機能に関するエラー//
#define FFWERR_PBPOINT_OVER			0x01030400		// ソフトウェアブレークの設定ポイントが最大設定点数(4096点)を超えている//
#define FFWERR_PBADDR_NOTSET		0x01030401		// ソフトウェアブレークが設定されていない(解除時のみ返送)//
#define FFWERR_PBAREA_OUT			0x01030402		// 指定領域はソフトウェアブレークポイント設定不可領域である//
#define FFWERR_PB_ROMAREA_MCURUN	0x01030406		// ユーザプログラム実行中にMCU内蔵ROM領域へのソフトウェアブレークは設定/解除できない//
#define FFWERR_PB_ROMAREA_FLASHDEBUG	0x01030408	// フラッシュ書き替えデバッグ中にMCU内蔵ROM領域へのソフトウェアブレークは設定できない//
// RevRxE2No171004-001 Append Line//
#define FFWERR_PBADDR_OVERLAPP		0x01030409		// 通過ポイントが設定されているアドレスへS/Wブレークポイントを設定することはできません//
// V.1.02 覚書 No.34 リセット中のSTOP対応 Append Line 
#define FFWERR_STOP_AMCU_RESET		0x0103040D		// リセットがはいっていたため、ブレークできなかった。//

// RAMモニタ機能に関するエラー//
#define FFWERR_RRMAREA_SAME			0x01030500		// RAMモニタ設定領域が重複している//
#define FFWERR_RRMBLK_DIS			0x01030502		// 指定されたRAMモニタブロックは動作禁止状態である//
#define FFWERR_RRAM_UNSUPPORT		0x01030503		// RAMモニタ機能はサポートしていない。//

// トレース機能に関するエラー//
#define FFWERR_RTTSTAT_RECORDING	0x01030702		// トレース動作中である//
#define FFWERR_RTTDATA_NON			0x01030703		// トレースデータがない//
#define FFWERR_MODE_AND_PIN			0x0103070A		// 指定したトレースモードは現在のピン数で設定できません//
#define FFWERR_ROM_RANGE			0x0103070B		// ROM空間アドレスの範囲指定に誤りがあります//
#define FFWERR_WINTRC_RANGE			0x0103070C		// ウィンドウトレースアドレスの範囲指定に誤りがあります//
#define FFWERR_RTTSTOP_FAIL			0x0103070D		// トレースストップに失敗した(トレースストップタイムアウトエラー)//
#define FFWERR_RTTMODE_UNSUPRT		0x0103070E		// サポートしていないトレースモード//
//RevNo100715-007 Append Line
#define FFWERR_RTTSTAT_DIS			0x0103070F		// トレースは動作していません。//
//RevRxNo130308-001 Append Line
#define FFWERR_RTT_UNSUPPORT		0x01030711		// トレース機能はサポートしていない。//

//パフォーマンス機能に関するエラー//
#define FFWERR_PPC_USR_CTL			0x01030D00		// ユーザプログラムでカウンタコントロールを使用している//
#define FFWERR_PPC_CONNECT			0x01030D01		// パフォーマンスカウンタ連結ON状態なのに、設定が0/1で異なる//
#define FFWERR_PPC_RUN				0x01030D02		// PPC計測中のためｺﾏﾝﾄﾞ処理を実行できない//
#define FFWERR_PPC_UNSUPPORT		0x01030D03		// このパフォーマンス機能はサポートされていない//

//RevRxNo130308-001 Append Start
//カバレッジ機能に関するエラー//
#define FFWERR_CVAREA_SAME			0x01030E00		// カバレッジ計測設定領域が重複している//
#define FFWERR_CVBLK_DIS			0x01030E01		// 指定されたカバレッジ計測ブロックは動作禁止状態である//
#define FFWERR_CVDATA_NON			0x01030E02		// カバレッジデータがない//
#define FFWERR_CV_LOST				0x01030E03		// カバレッジ情報にLOSTが発生した(Warning)//
#define FFWERR_CV_MEASURE			0x01030E04		// リセット発生によるカバレッジ計測異常発生の可能性あり(Warning)//
#define FFWERR_CV_LOST_MEASURE		0x01030E05		// カバレッジ情報にLOSTが発生した、およびリセット発生によるカバレッジ計測異常発生の可能性あり(Warning)//
#define FFWERR_CV_UNSUPPORT			0x01030E06		// カバレッジ機能はサポートしていない。//
//RevRxNo130308-001 Append End

// プログラム実行に関するエラー//
#define FFWERR_GPB_NOT_EVNUM		0x01050000		// 空きイベント番号がないため、プログラムブレークポイント付き実行できない//
// RevRxNo140617-001 Append Line
#define FFWERR_STEPADDR_TMAREA		0x01050003		// ステップ実行開始アドレスがTrusted Memory領域内のため、ステップ実行できない//
// RevRx6z4No160105-001 Append Line
#define FFWERR_CPU1_CAN_NOT_STEP	0x01050004		// CPU1のRAMが停止状態のためステップ実行できない//

// 外部フラッシュダウンロード機能に関するエラー//
#define FFWERR_EXTROM_ERASE			0x01060000		// 外部フラッシュROMのイレーズエラー//
#define FFWERR_EXTROM_WRITE			0x01060001		// 外部フラッシュROMの書き込みエラー//
#define FFWERR_EXTROM_VERIFY		0x01060002		// 外部フラッシュROMのベリファイエラー//
#define FFWERR_EXTROM_BUS			0x01060003		// 外部フラッシュROMのバス設定エラー(実際にリードできなかった)//
#define FFWERR_EXTROM_ID			0x01060004		// 外部フラッシュROMのメーカ・デバイスID不一致エラー//
#define FFWERR_EXTROM_SCRIPT		0x01060005		// 外部フラッシュROMのスクリプト読み込みエラー(//
// ExtFlashModule_012 Append Start
#define FFWERR_EXTROM_WPROG_VERIFY			0x01060006	// 外部フラッシュROM用書き込みプログラムダウンロード時のベリファイエラー //
#define FFWERR_EXTROM_BUSSET_BAT_VRIFYERR	0x01060007	// 外部フラッシュROMダウンロード前後のバッチファイル実行内でベリファイエラー //
// ExtFlashModule_012 Append End
// ExtFlashModule_002b Append Line
#define FFWERR_EXTROM_NOTWRITE_LOCKSECT		0x01060008	// 外部フラッシュROMに、ロックされているため書き込み/消去ができないセクタがあった(Warning) //

// プログラム実行に関するエラー//
#define FFWERR_C2E_BUFF_EMPY		0x01070000		// C2Eバッファが空である。//

// 電源供給機能に関するエラー//
// RevRxE2LNo141104-001 Append Line//
#define FFWERR_TARGET_POWERLOW		0x010A0000		// ユーザシステムの電圧が供給電圧値より低い(Warning)//
// RevRxE2No171004-001 Append Line
#define FFWERR_ASP_NO_CHG_POWERSUPPLY	0x010A0001		// E2拡張機能動作中のエミュレータからの供給電圧変更不可//

//-----------------------------------------------
// コマンド実行前のステータスチェックエラー//
//-----------------------------------------------
// プログラム実行状態に関するエラー//
#define FFWERR_BMCU_RUN				0x02000000		// ユーザプログラム実行中のためコマンド処理を実行できない//
#define FFWERR_BMCU_STOP			0x02000001		// ユーザプログラム停止中のためコマンド処理を実行できない//
#define FFWERR_BMCU_NONCONNECT		0x02000004		// MCUとの接続が未完了//

// ターゲットステータスに関するエラー//
#define FFWERR_BTARGET_POWER		0x02000100		// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない//
#define FFWERR_BTARGET_RESET		0x02000101		// ターゲットシステム上のリセット端子が 'L'アクティブ状態のためコマンド処理を実行できない//
#define FFWERR_BTARGET_NOCONNECT	0x02000104		// ターゲットシステムと接続されていない//
#define FFWERR_BTARGET_POWERSUPPLY	0x02000105		// ターゲットシステムの電圧は既に供給されている//
#define FFWERR_BTARGET_MODE			0x02000106		// ターゲットシステム上のMODE端子が 'L'レベル状態のためコマンド処理を実行できない。//

// MCUのCPUステータスに関するエラー//
#define FFWERR_BMCU_RESET			0x02000201		// MCUがリセット状態のためコマンド処理を実行できない//
#define FFWERR_BMCU_STANBY			0x02000203		// MCUのスタンバイモード中で内部クロックが停止状態である。//
#define FFWERR_BMCU_SLEEP			0x02000204		// MCUのスリープモード中で内部クロックが停止状態である。//
#define FFWERR_BMCU_DEEPSTANBY		0x02000207		// MCUがディープスタンバイ状態である。 エラー出力はしないが残しておく。//
//V.1.02 RevNo110621-001 Append Line
#define FFWERR_BMCU_AUTH_DIS		0x0200020A		// MCU内部リセットが発生したため、コマンド処理を実行できない	//

// MCUのバスステータスに関するエラー//

// RevRxE2No171004-001 Append Start//
#define FFWERR_N_AMCU_IABORT		0x02000300		// MCUに命令アボートが発生した//
#define FFWERR_N_AMCU_DABORT		0x02000301		// MCUにデータアボートが発生した//
// RevRxE2No171004-001 Append End//

// MCU端子状態に関するエラー//
#define FFWERR_EMLE_NOT_HIGH		0x02000403		// MCUのEMLE端子が 'H'にならないので、エミュレータとMCUが接続できない//
//RevNo100715-009 Append Start
#define FFWERR_ATARGET_TRSTN		0x02000404		// MCUのTRSTn端子が コントロールできないので、エミュレータとMCUが接続できない//
#define FFWERR_ATARGET_TMS			0x02000405		// MCUのTMS端子が コントロールできないので、エミュレータとMCUが接続できない//
//RevNo100715-009 Append End
#define FFWERR_ATARGET_UB			0x02000406		// MCUのUB端子が コントロールできないので、エミュレータとMCUが接続できない//
#define FFWERR_ATARGET_MD			0x02000407		// MCUのMD端子が コントロールできないので、エミュレータとMCUが接続できない//
// RevRxE2LNo141104-001 Append Line//
#define FFWERR_ATARGET_EMLE			0x02000408		// MCUのEMLE端子が コントロールできないので、エミュレータとMCUが接続できない//

// RevRxE2No171004-001 Append Start//
#define FFWERR_N_BFWWAIT_TIMEOUT	0x0200040A		// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない//
// RevRxE2No171004-001 Append End//

// SCI通信に関するエラー//
#define FFWERR_BBR_UNDER			0x02000500		// 設定可能なボーレートより低い//
#define FFWERR_BBR_OVER				0x02000501		// 設定可能なボーレートより高い//
// シリアル通信に関するエラー//
#define FFWERR_ASCI_COMERR			0x02000502		// ターゲット通信エラーが発生した。//
#define FFWERR_ASCI_COMOV			0x02000503		// 通信中断が発生した。//
#define FFWERR_ASCI_FER				0x02000504		// フレーミングエラーが発生した。//
#define FFWERR_ASCI_ORER			0x02000505		// オーバーランエラーが発生した。//
#define FFWERR_ASCI_PER				0x02000506		// パリティエラーが発生した。//
#define FFWERR_ASCI_TRANSMIT		0x02000507		// シリアルデータ送信中にタイムアウトエラーが発生した。//
#define FFWERR_ASCI_RECEIVE			0x02000508		// シリアルデータ受信中にタイムアウトエラーが発生した。//
#define FFWERR_ATARGET_RESET		0x02000509		// ユーザリセット解除中にタイムアウトエラーが発生した。//
#define FFWERR_ASCI_HALFEMP			0x0200050C		// シリアルデータ送信(送信用FIFOの空き(512バイト)待ち)中にタイムアウトエラーが発生した。//

// FINE通信に関するエラー//
#define FFWERR_BAUDRATE_MEASUREMENT	0x02000600		// 通信ボーレート確認コマンドによるボーレート計測モードへの遷移に失敗した。//

// システム状態に関するエラー//
#define FFWERR_LV1_DAMAGE			0x02000700		// レベル1の内容が破損している。//
#define FFWERR_BEXE_LEVEL0			0x02000701		// BFW 内部動作モードがレベル0の状態でない。//
#define FFWERR_EML_ENDCODE			0x02000702		// EML領域にエンドコードがない。//
#define FFWERR_FDT_ENDCODE			0x02000703		// FDT領域にエンドコードがない。//
#define FFWERR_EML_TO_FDT			0x02000704		// EML動作モードからFDT動作モードへの遷移はできない。//
#define FFWERR_FDT_TO_EML			0x02000705		// FDT動作モードからEML動作モードへの遷移はできない。//
#define FFWERR_MONP_ILLEGALEML		0x02000706		// 不正なエミュレータの接続を検出した。//

// エミュレータ依存サポート機能に関するエラー//


//-----------------------------------------------
// コマンド実行後のステータスチェックエラー//
//-----------------------------------------------
// MCUのCPUステータスに関するエラー//
#define FFWERR_AMCU_POWER			0x03000000		// タイムアウトが発生した。MCU供給電源がOFF状態である。//
#define FFWERR_AMCU_RESET			0x03000001		// タイムアウトが発生した。MCUがリセット状態である。//
#define FFWERR_AMCU_STANBY			0x03000003		// タイムアウトが発生した。MCUのスタンバイモード中で内部クロックが停止状態である。//
#define FFWERR_AMCU_SLEEP			0x03000004		// タイムアウトが発生した。MCUのスリープモード中で内部クロックが停止状態である。//
#define FFWERR_EL3_AMCU_AUTH_DIS	0x03000010		// タイムアウトが発生した。MCUとのデバッガ認証切れ状態である。//
#define FFWERR_EL3_BMCU_AUTH_DIS	0x03000011		// MCUとのデバッガ認証切れ状態である。//

//RevNo100715-039 Append Start
//ターゲット状態に関するエラー//
#define FFWERR_ATARGET_NOCONNECT	0x03000300		// タイムアウトエラーが発生した。ターゲットが外れている。//
#define FFWERR_ATARGET_POWER		0x03000301		// タイムアウトエラーが発生した。ターゲット電源がOFFである。//
//RevNo100715-009 Modify Line
#define FFWERR_TO_ATARGET_RESET		0x03000302		// タイムアウトエラーが発生した。ターゲットリセットがLoである。//
//RevNo100715-039 Append End

//起動に関するエラー//
#define FFWERR_HOTPLUG_MDE_NOTMATCH		0x03000600		// 指定されたエンディアンと不一致であるためホットプラグ起動できない//
#define FFWERR_HOTPLUG_UBCODE_NOTMATCH	0x03000601		// 指定されたUBコードと不一致であるためホットプラグ起動できない//
#define FFWERR_HOTPLUG_USERBOOT			0x03000602		// 起動モードがユーザブートモードのためホットプラグ起動できない//
// RevRxNo161003-001 Append Start//
#define FFWERR_HOTPLUG_BANKMD_NOTMATCH	0x03000603		// 指定されたバンクモードと不一致であるためホットプラグ起動できない//
#define FFWERR_HOTPLUG_BANKSWP_NOTMATCH	0x03000604		// 指定された起動バンクと不一致であるためホットプラグ起動できない//
// RevRxNo161003-001 Append End//

// RevRxE2No171004-001 Append Start//
//ターゲットとの通信エラー//
#define FFWERR_SU_SWD_TRANSFER		0x03000700		// モニタプログラム処理でタイムアウトが発生した。エラーが特定できない。//
#define FFWERR_SU_DAP_STICKYERR		0x03000701		// DAPのSTICKYエラーが発生しています。エラーが特定できません。//
#define FFWERR_SU_DAP_WAIT_TIMEOUT	0x03000702		// DAPとの通信で、ACKがWAITからOKにならなかった。//
#define FFWERR_SU_DAP_PROTOCOL_ERR	0x03000703		// DAPとの通信で、想定外のACK(FAULT/WAIT/OK以外)が返ってきた、もしくはプロトコルエラーが発生した。//
#define FFWERR_SU_SWD_FAULT			0x03000704		// SWDのACKでFAULTが発生した。//

//ASPに関するエラー//
#define FFWERR_ASP_BREAK			0x03000800		// ASP停止要求//
#define FFWERR_ASP_OVF				0x03000801		// ASPオーバーフロー//

//タイムアウトエラー//
#define FFWERR_SU_MON_TIMEOUT				0x03FFFF03		// モニタプログラム処理でタイムアウトが発生した。エラーが特定できない。//
#define FFWERR_SU_BFW_TIMEOUT				0x03FFFF04		// BFW処理でタイムアウトが発生した。//
#define FFWERR_SU_DAP_STICKYERR_TIMEOUT		0x03FFFF05		// タイムアウトが発生しました。DAPのSTICKYエラーが発生しています。エラーが特定できません。//
// RevRxE2No171004-001 Append End//

//-----------------------------------------------
// タイムアウトエラー	//
//-----------------------------------------------
//RevNo100715-028 Modify Line
#define FFWERR_OFW_TIMEOUT			0x04000000		// OFW処理でタイムアウトが発生した。エラーが特定できない//
#define FFWERR_BFW_TIMEOUT			0x04000001		// BFW処理でタイムアウトが発生した。//
#define FFWERR_SRM_TIMEOUT			0x04000002		// 指定ルーチン実行でタイムアウトが発生した。//
// RevRxE2No171004-001 Append Line
#define FFWERR_MON_POINT_TIMEOUT	0x04000003		// 通過ポイント用モニタプログラム実行でタイムアウトが発生した。//

//-----------------------------------------------
// 処理中断//
//-----------------------------------------------
#define FFWERR_TRGHALT_OK			0x05000000		// TARGET.DLLの処理中断要求によりFFW処理を中断した。//
#define FFWERR_TRGHALT_NGBFWRUN		0x06000000		// TARGET.DLLの処理中断要求を正常に終了することができなかった。現在、BFWの処理を継続実行している。//
#define FFWERR_TRGHALT_NGEFWRUN		0x06000001		// TARGET.DLLの処理中断要求を正常に終了することができなかった。現在、EFWの処理を継続実行している。//

// V.1.02 0x08000001　Delete

//-----------------------------------------------
// 通信エラー//
//-----------------------------------------------
#define FFWERR_COM					0x7F000000		// 致命的な通信異常が発生した//
#define FFWERR_COMDATA_STARTCODE	0x7F000001		// BFWから受信したスタートコードが異常である//
#define FFWERR_COMDATA_CMDCODE		0x7F000002		// BFWから受信したコマンドコードが異常である//
#define FFWERR_COMDATA_ERRCODE		0x7F000003		// BFWから未定義のエラーコード・処理中断コードを受信した。又はエラーコード・処理中断コードの組み合わせが不正である。//
#define FFWERR_COMDATA_DATA			0x7F000004		// BFWから受信したデータ(スタートコード、コマンドコード、エラーコード、処理中断コードを除く)が異常である//
#define FFWERR_COM_RCVSIZE			0x7F000005		// BFWにコマンド送信時、受信バッファにデータが残っている(前回のコマンド結果受信時、想定以上のデータを受信した)//
#define FFWERR_COM_1ST				0x7F000006		// BFWとの最初のコマンド送受信時に通信エラーが発生した//
#define FFWERR_COM_USBT_CHANGE		0x7F000007		// USB転送サイズ変更後に通信エラーが発生した//

//-----------------------------------------------
// その他//
//-----------------------------------------------
#define FFWERR_MONP_FILE_NOT_FOUND		0x80000000		// MONP用ファイルが探せない、オープンできない//
#define FFWERR_WTR_NOTLOAD				0x80000001		// WTR(書き込みプログラム)がロードされていない//
// RevRxNo120928-002 Append Line//
#define FFWERR_FUNC_UNSUPORT			0x80000002		// サポートしていない機能である   ※EZ-CUBEで、BFWDLLの対応できないコマンドが発行された場合に使用する。//
// RevRxNo130730-009 Append Line//
#define FFWERR_MEMORY_NOT_SECURED		0x80000003		// PC上のメモリを確保できなかった //
#define FFWERR_BFW_FILE_NOT_FOUND		0x80000004		// BFWファイルがオープンできない	// RevRxNo170829-001 Append Line
#define FFWERR_MCU_FILE_NOT_FOUND		0x80000005		// MCUファイルがオープンできない	// RevRxNo171023-001 Append Line

#endif	// __FFWE20ERR_H__
