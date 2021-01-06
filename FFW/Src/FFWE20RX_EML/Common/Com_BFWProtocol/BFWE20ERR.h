////////////////////////////////////////////////////////////////////////////////
/**
 * @file BFWE20ERR.h
 * @brief BFWインタフェース エラー定義のヘッダファイル
 * @author RSD Y.Minami, K.Okita, H.Hashiguchi, S.Ueda, SDS T.Iwata, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2017) Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/14
 */
////////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo120614-001 2012/07/12 橋口
  クロック分周、切り替え前のビット確認
・RevRxNo121022-001 2012/10/22 SDS 岩田
    EZ-CUBE用エラーコード追加。//
     BFWERR_BFW_MCU_UNMATCH, BFWERR_BFW_VER_UNMATCH,
     BFWERR_FUNC_UNSUPORT, BFWERR_USB_COMERR, BFWERR_USB_COMCODE_UNMATCH
・RevRxE2LNo141104-001 2014/12/22 上田
	E2 Lite対応
・RevRxNo150928-001 2015/09/28 PA 辻
	ROMキャッシュインバリデート Warning対応
	対応見送りのため、コメントアウト
・RevRxNo160527-001 2016/06/10 PA 辻
	RX651 正式対応
・RevRxE2No171004-001 2017/10/04 PA 辻
	E2拡張機能対応
*/

#ifndef	BFWE20ERR_H
#define	BFWE20ERR_H


#define NORMAL 0x0000

//###########################################################
// BFWERR型のエラー定義
//###########################################################
//-----------------------------------------------
// 正常終了
//-----------------------------------------------
#define BFWERR_OK				0x0000		//正常終了

//-----------------------------------------------
// コマンドエラー
//-----------------------------------------------
#define BFWERR_BFW_ARG			0x0001		//引数の指定に誤りがある
#define BFWERR_BFW_CMD			0x0002		//未定義のコマンドを受信した

//-----------------------------------------------
// エミュレータ内部資源のアクセスエラー
//-----------------------------------------------
#define	BFWERR_BFWFROM_ERASE	0x0102		//	BFW格納用 フラッシュROMのイレーズエラー
#define	BFWERR_BFWFROM_WRITE	0x0103		//	BFW格納用 フラッシュROMの書き込みエラー
#define	BFWERR_BFWFROM_VERIFY	0x0104		//	BFW格納用 フラッシュROMのベリファイエラー	// RevRxE2LNo141104-001 Append Line

//RevNo100715-009 Modify Line
#define	BFWERR_FPGA_CONFIG		0x0105		//	FPGAコンフィグレーションエラー
// RevRxE2No171004-001 Modify Line
#define	BFWERR_LID_UNDEFINE		0x0107		//	ライセンスIDが定義されていない	// RevRxE2LNo141104-001 Append Line
// RevRxE2No171004-001 Append Start
#define	BFWERR_NID_BLANK		0x0108		//	ニックネームがブランク状態で設定されていない
#define	BFWERR_NIDSIZE_OVER		0x010B		//	ニックネームの文字数が許容範囲を超えている
#define	BFWERR_FLAG_BLANK		0x010C		//	フラグがブランク状態で設定されていない
// RevRxE2No171004-001 Append End

#define	BFWERR_MCUROM_ERASE		0x010F		//	ターゲットMCU内蔵 フラッシュROMのイレーズエラー
#define	BFWERR_MCUROM_WRITE		0x0110		//	ターゲットMCU内蔵 フラッシュROMの書き込みエラー

#define	BFWERR_MCUROM_PCLK_ILLEGAL	0x0124	//	ターゲットMCU内蔵 フラッシュROMの周辺クロック指定範囲外エラー
#define	BFWERR_CLK_CHG_DISENA		0x0125		//	ダウンロード時のクロックソース切り替え不可エラー
#define	BFWERR_EXTAL_OUT			0x0126		//	ダウンロード時のEXTAL切り替え不可エラー

// RevRxE2No171004-001 Append Start
// 拡張ボードに関するエラー
#define	BFWERR_EX1BFWFROM_ERASE		0x0130		//	拡張ボードF/W格納用フラッシュROMのイレーズエラー
#define	BFWERR_EX1BFWFROM_WRITE		0x0131		//	拡張ボードF/W格納用フラッシュROMの書き込みエラー
#define	BFWERR_EX1BFWFROM_VERIFY	0x0132		//	拡張ボードF/W格納用フラッシュROMのベリファイエラー
#define	BFWERR_EX1BFPGA_CONFIG		0x0133		//	拡張ボードFPGAコンフィグレーションエラー
#define	BFWERR_EX1BEROM_ERASE		0x0134		//	拡張ボードEEPROMのイレーズエラー
#define	BFWERR_EX1BEROM_WRITE		0x0135		//	拡張ボードEEPROMの書き込みエラー
#define	BFWERR_EX1BEROM_VERIFY		0x0136		//	拡張ボードEEPROMのベリファイエラー
#define	BFWERR_EX1BEROM_READ		0x0137		//	拡張ボードEEPROMのリードエラー
#define	BFWERR_EX1BEROM_NORES		0x0138		//	拡張ボードEEPROMの応答なしエラー
#define	BFWERR_EEPROM_WRITE			0x0139		//	EEPROMのライトエラー
#define	BFWERR_EEPROM_READ			0x013A		//	EEPROMのリードエラー
// RevRxE2No171004-001 Append End

// シリアル情報関連エラー
#define BFWERR_SNSIZE_OVER			0x0140		// シリアル番号の文字数が許容範囲を超えている
#define BFWERR_SN_BLANK				0x0145		// シリアル番号がブランク状態で設定されていない	// RevRxE2LNo141104-001 Append Line

// ベース基板に関するエラー2
// RevRxNo121022-001 Append Start
#define BFWERR_BFW_MCU_UNMATCH		0x0142		// エミュレータに格納されたファームウェアがデバッグ対象のMCU用でない。※EZ-CUBEエミュレータでのみ使用(E1/E20 BFWとしてはエラーを返さない)
#define BFWERR_BFW_VER_UNMATCH		0x0143		// エミュレータに格納されたファームウェアのバージョンが一致していない。※EZ-CUBEエミュレータでのみ使用(E1/E20 BFWとしてはエラーを返さない)
// RevRxNo121022-001 Append End
#define BFWERR_TMRCTRL_OVER			0x0144		// 既に全てのタイマ(コンペアマッチA,B)が使用されている。

// RevRxE2No171004-001 Append Start
// 2nd拡張ボードに関するエラー
#define BFWERR_SPBFPGAFROM_ERASE	0x0141		// SPボードFPGA格納用フラッシュROMのイレーズエラー
#define BFWERR_EX2BEROM_WRITE		0x0146		// 2nd拡張ボードEEPROMの書き込みエラー
#define BFWERR_EX2BEROM_VERIFY		0x0147		// 2nd拡張ボードEEPROMのベリファイエラー
#define BFWERR_EX2BEROM_READ		0x0148		// 2nd拡張ボードEEPROMのリードエラー
#define BFWERR_EX2BEROM_NORES		0x0149		// 2nd拡張ボードEEPROMの応答なしエラー

#define BFWERR_PODFPGAFROM_ERASE	0x0150		// エミュレーションポッドFPGA格納用フラッシュROMのイレーズエラー
#define BFWERR_PODFPGAFROM_WRITE	0x0151		// エミュレーションポッドFPGA格納用フラッシュROMの書き込みエラー
#define BFWERR_PODFPGAFROM_VERIFY	0x0152		// エミュレーションポッドFPGA格納用フラッシュROMのベリファイエラー

#define BFWERR_N_UNSUPPORT			0x1600		// 指定機能はサポートされていません。
// RevRxE2No171004-001 Append End

//-----------------------------------------------
// デバッグ機能のエラー
//-----------------------------------------------
// 共通項目

// メモリアクセスに関するエラー

/////////////////////////////////////////////////////////////////////
// Ver.1.01 2010/08/17 SDS T.Iwata
#define BFWERR_FWRITE_NOTEXE_ILGLERR	0x0207	// MCU内蔵フラッシュROMへのアクセス違反などが発生しているため、フラッシュ書き換えが実行できない
#define BFWERR_FWRITE_FAIL_ILGLERR		0x0208	// MCU内蔵フラッシュROMへのアクセス違反が発生し、フラッシュ書き換えが正常に実行できなかった
/////////////////////////////////////////////////////////////////////
// RevRxNo120614-001 Append Line
#define BFWERR_ACC_ROM_PEMODE			0x0218	// フラッシュ書き換えモード中のため、MCU内蔵ROM領域の読み出し/書き込み実行はできない
// Ver.1.02 RevNo110613-001 Append Line
#define BFWERR_CLKCHG_DMB_CLR			0x021C	// メモリアクセス中のクロック切り替えなどによる通信エラーが発生したためリカバリした(Warning)

// RevRxNo160527-001 Append Line
#define BFWERR_FAW_FSPR_ALREADY_PROTECT	0x031C	// FAWレジスタFSPRビットによるプロテクトがすでに設定されている(Warning)

//-----------------------------------------------
// コマンド実行前のエラー
//-----------------------------------------------
// プログラム実行状態に関するエラー
#define BFWERR_BMCU_RUN			0x0300		// ユーザプログラム実行中のためコマンド処理を実行できない
#define BFWERR_BMCU_STOP		0x0301		// ユーザプログラム停止中のためコマンド処理を実行できない
#define BFWERR_STEP_RUNSTATE	0x0304		// ステップ実行(実行後PCブレーク)で所定時間内にブレークしなかった

// ターゲットステータスに関するエラー
#define BFWERR_BTARGET_POWER	0x0310		// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない
#define BFWERR_BTARGET_RESET	0x0311		// ターゲットシステム上のリセット端子が 'L'アクティブ状態のためコマンド処理を実行できない
#define BFWERR_BTARGET_NOCONNECT	0x0314		// ターゲットシステムと接続されていない
#define BFWERR_BTARGET_POWERSUPPLY	0x0315		// ターゲットシステムの電圧は既に供給されている
#define BFWERR_BTARGET_MODE				0x0316		// ターゲットシステム上のMODE端子が 'L'レベル状態のためコマンド処理を実行できない。

// MCUのCPUステータスに関するエラー
#define BFWERR_BMCU_RESET		0x0321		// MCUがリセット状態のためコマンド処理を実行できない
#define BFWERR_BMCU_STANBY		0x0323		// MCUのスタンバイモード中で内部クロックが停止状態である。
#define BFWERR_BMCU_SLEEP		0x0324		// MCUのスリープモード中で内部クロックが停止状態である。
//V.1.02 RevNo110621-001 Append Line
#define BFWERR_BMCU_AUTH_DIS	0x032A		// MCU内部リセットが発生したため、コマンド処理を実行できない
#define BFWERR_EL1_AMCU_NO_DBG	0x1F00		// DBG割り込みに入らなかった。MODE端子状態が正しくない可能性あり。
#define BFWERR_DAUTH			0x0329		// ID認証プログラムが完了しない。
//V.1.02 RevNo110621-001 Append Line
#define BFWERR_MCU_CONNECT_BAN	0x0319		// MCUがエミュレータ接続禁止状態である。

// MCUのバスステータスに関するエラー

// RevRxE2No171004-001 Append Start
#define BFWERR_N_AMCU_IABORT	0x0330		// MCUに命令アボートが発生した
#define BFWERR_N_AMCU_DABORT	0x0331		// MCUにデータアボートが発生した
// RevRxE2No171004-001 Append End

// MCU端子状態に関するエラー
#define BFWERR_EMLE_NOT_HIGH	0x0343		// MCUのEMLE端子が 'L'状態のためJTAG動作がしない
//RevNo100715-009 Append Start
#define BFWERR_ATARGET_TRSTN	0x0344		// MCUのTRSTn端子が コントロールできないので、エミュレータとMCUが接続できない
#define BFWERR_ATARGET_TMS		0x0345		// MCUのTMS端子が コントロールできないので、エミュレータとMCUが接続できない
//RevNo100715-009 Append End
#define BFWERR_ATARGET_UB		0x0346		// MCUのUB端子が コントロールできないので、エミュレータとMCUが接続できない
#define BFWERR_ATARGET_MD		0x0347		// MCUのMD端子が コントロールできないので、エミュレータとMCUが接続できない
#define BFWERR_ATARGET_EMLE		0x0348		// MCUのEMLE端子が コントロールできないので、エミュレータとMCUが接続できない	// RevRxE2LNo141104-001 Append Line

// RevRxE2No171004-001 Append Start
#define BFWERR_N_BFWWAIT_TIMEOUT	0x034A		// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない
// RevRxE2No171004-001 Append End

// SCI通信に関するエラー
#define BFWERR_BBR_UNDER		0x0350		// 設定可能なボーレートより低い
#define BFWERR_BBR_OVER			0x0351		// 設定可能なボーレートより高い
// シリアル通信に関するエラー
#define BFWERR_ASCI_COMERR			0x0352		// ターゲット通信エラーが発生した。
#define BFWERR_ASCI_COMOV			0x0353		// 通信中断が発生した。
#define BFWERR_ASCI_FER				0x0354		// フレーミングエラーが発生した。
#define BFWERR_ASCI_ORER			0x0355		// オーバーランエラーが発生した。
#define BFWERR_ASCI_PER				0x0356		// パリティエラーが発生した。
#define BFWERR_ASCI_TRANSMIT		0x0357		// シリアルデータ送信中にタイムアウトエラーが発生した。
#define BFWERR_ASCI_RECEIVE			0x0358		// シリアルデータ受信中にタイムアウトエラーが発生した。
#define BFWERR_ATARGET_RESET		0x0359		// ユーザリセット解除中にタイムアウトエラーが発生した。
#define BFWERR_BAUDRATE_MEASUREMENT	0x03B0		// 通信ボーレート確認コマンドによるボーレート計測モードへの遷移に失敗した。
#define BFWERR_ASCI_HALFEMP			0x0448		// シリアルデータ送信(送信用FIFOの空き(512バイト)待ち)中にタイムアウトエラーが発生した。

// JTAG通信に関するエラー
#define BFWERR_JTAG_ILLEGAL_IR	0x0360		// IR値が正しくない、MCUとのJTAG通信ができていない

// システム状態に関するエラー
#define BFWERR_LV1_DAMAGE		0x0370		// レベル1の内容が破損している。
#define BFWERR_BEXE_LEVEL0		0x0371		// BFW 内部動作モードがレベル0の状態でない。
#define BFWERR_EML_ENDCODE		0x0372		// EML領域にエンドコードがありません。
#define BFWERR_FDT_ENDCODE		0x0373		// FDT領域にエンドコードがありません。
#define BFWERR_EML_TO_FDT		0x0374		// EML動作モードからFDT動作モードへの遷移はできない。
#define BFWERR_FDT_TO_EML		0x0375		// FDT動作モードからEML動作モードへの遷移はできない。
#define BFWERR_MONP_ILLEGALEML	0x0376		// 不正なエミュレータの接続を検出した。

// MCU内蔵資源のアクセスに関するエラー
// RevRxNo150928-001 Append Start +2
#if 0
#define BFWERR_ROMC_BAN_INVALIDATE_PERMIT	0x0393	// ROMキャッシュ動作を禁止に変更後インバリデート実施し、ROMキャッシュ動作を許可に戻した(Warning)
#define BFWERR_ROMC_INVALIDATE				0x0394	// ROMキャッシュインバリデートを実施した(Warning)
#endif
// RevRxNo150928-001 Append End

// ブレーク機能に関係するエラー
#define BFWERR_STOP_AMCU_RESET  0x03C0		// リセットがはいっていたため、ブレークできなかった。

// C2E/E2C 関連のエラー
#define BFWERR_C2E_BUFF_EMPY	0x0450		// C2Eバッファが空

// 外部フラッシュダウンロード関連エラー
#define	BFWERR_EXTROM_ERASE		0x0500		//	外部フラッシュROMのイレーズエラー
#define	BFWERR_EXTROM_WRITE		0x0501		//	外部フラッシュROMの書き込みエラー
#define BFWERR_EXTROM_IDREAD	0x0502		//  外部フラッシュメーカ・デバイスID読み出しエラー
#define BFWERR_EXTROM_IDUNMATCH	0x0503		//  外部フラッシュメーカ・デバイスID不一致エラー

//-----------------------------------------------
// コマンド実行後のエラー
//-----------------------------------------------
// MCUのCPUステータスに関するエラー
#define BFWERR_AMCU_POWER		0x0400		// タイムアウトが発生した。MCU供給電源がOFF状態である。
#define BFWERR_AMCU_RESET		0x0401		// タイムアウトが発生した。MCUがリセット状態である。
#define BFWERR_AMCU_STANBY		0x0403		// タイムアウトが発生した。MCUのスタンバイモード中で内部クロックが停止状態である。
#define BFWERR_AMCU_SLEEP		0x0404		// タイムアウトが発生した。MCUのスリープモード中で内部クロックが停止状態である。
#define BFWERR_EL3_AMCU_AUTH_DIS		0x3010	// タイムアウトが発生した。MCUとのデバッガ認証切れ状態である。
#define BFWERR_EL3_BMCU_AUTH_DIS		0x3011	// MCUとのデバッガ認証切れ状態である。

// MCUのバスステータスに関するエラー

// MCU端子状態に関するエラー

//ターゲット状態に関するエラー
#define BFWERR_EL3_ATARGET_NOCONNECT	0x3031	// タイムアウトが発生した。ターゲットシステムと接続されていない。
#define BFWERR_EL3_ATARGET_POWER		0x3032	// タイムアウトが発生した。ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない。
#define BFWERR_EL3_ATARGET_RESET		0x3030	// タイムアウトが発生した。ターゲットリセットがLoである。

// RevRxE2No171004-001 Append Start
// ターゲットとの通信エラー
#define BFWERR_SU_SWD_TRANSFER			0x3050	// モニタプログラム処理でタイムアウトが発生した。エラーが特定できない。
#define BFWERR_SU_DAP_STICKYERR			0x3051	// DAPのSTICKYエラーが発生しています。エラーが特定できません。
#define BFWERR_SU_DAP_WAIT_TIMEOUT		0x3052	// DAPとの通信で、ACKがWAITからOKにならなかった。
#define BFWERR_SU_DAP_PROTOCOL_ERR		0x3053	// DAPとの通信で、想定外のACK(FAULT/WAIT/OK以外)が返ってきた、もしくはプロトコルエラーが発生した。
#define BFWERR_SU_SWD_FAULT				0x3054	// SWDのACKでFAULTが発生した。

// ASPに関するエラー
#define BFWERR_ASP_BREAK		0x3060		// ASP停止要求
#define BFWERR_ASP_OVF			0x3061		// ASPオーバーフロー
// RevRxE2No171004-001 Append End

//-----------------------------------------------
// タイムアウトエラー
//-----------------------------------------------
//RevNo100715-028 Modify Line
#define BFWERR_OFW_TIMEOUT		0x3000		// OFW処理でタイムアウトが発生した。
#define BFWERR_BFW_TIMEOUT		0x3001		// BFW処理でタイムアウトが発生した。
#define BFWERR_SRM_TIMEOUT		0x3002		// 指定ルーチン実行でタイムアウトが発生した。
// RevRxE2No171004-001 Append Line
#define BFWERR_MON_POINT_TIMEOUT		0x3003	// 通過ポイント用モニタプログラム実行でタイムアウトが発生した。

// RevRxE2No171004-001 Append Start
#define BFWERR_SU_MON_TIMEOUT			0x3F00	// モニタプログラム処理でタイムアウトが発生した。エラーが特定できない。
#define BFWERR_SU_BFW_TIMEOUT			0x3F01	// BFW処理でタイムアウトが発生した。
#define BFWERR_SU_DAP_STICKYERR_TIMEOUT	0x3F02	// タイムアウトが発生しました。DAPのSTICKYエラーが発生しています。エラーが特定できません。
// RevRxE2No171004-001 Append End

//-----------------------------------------------
// 処理中断エラー
//-----------------------------------------------
#define BFWERR_COMEXE_STOP		0x1100		// コマンド処理を中断した。

// RevRxNo121022-001 Append Start
//-----------------------------------------------
// EZ-CUBE専用エラー
//-----------------------------------------------
#define BFWERR_FUNC_UNSUPORT		0x7F01	// サポートしていない機能である。
#define BFWERR_USB_COMERR			0x7F02	// BFWDLL、BFW間でのUSB通信エラー
#define BFWERR_USB_COMCODE_UNMATCH	0x7F03	// BFWDLL、BFW間でのUSB通信時のコマンドコード不一致
// RevRxNo121022-001 Append End


//###########################################################
// BFWプロトコル内の処理中断フレーム定義
//###########################################################
//-----------------------------------------------
// 処理中断コード定義
//-----------------------------------------------
#define HALT_NON				(0x0000)	// 処理中断なし。
//#define HALT_SYSRST_NORMAL		(0x0001)	// システムリセット入力により処理を中断した。
#define HALT_BFWHALT_NORMAL		(0x0010)	// BFWCmd_HALTにより処理を中断した。
#define HALT_BFWHALT_EFWRUN		(0x0011)	// BFWCmd_HALTにより処理を中断した。
#define HALT_EFW_TIMEOUT		(0x0020)	// EFWのタイムアウトを検出し処理を中断した。
#define HALT_BFW_TIMEOUT		(0x0021)	// BFWのタイムアウトを検出し処理を中断した。
#define HALT_BFW_ILLEGALEML		(0x0022)	// 不正なエミュレータの接続を検出し処理を中断した。


#endif // BFWE20ERR_H
