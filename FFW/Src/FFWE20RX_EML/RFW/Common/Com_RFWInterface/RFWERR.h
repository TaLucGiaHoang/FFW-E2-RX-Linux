///////////////////////////////////////////////////////////////////////////////
/**
 * @file RFWERR.h//
 * @brief E2エミュレータ RFWエラーコードの定義ファイル
 * @author TSSR M.Ogata, RSD M.Shimada H.Hatahara, PA M.Tsuji
 * @author Copyright (C) 2015 Renesas Electronics Corporation. All rights reserved.
 * @date 2017/10/04
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef	__RFWERR_H__
#define	__RFWERR_H__
/*
■改定履歴
・ソース構成改善	2015/03/06 緒方
・2015/12/18 RFWERR_W_FLASH_ALERASE_ID_FAIL(0x1003011D) -> RFWERR_N_FLASH_ALERASE_ID_FAIL(0x0003011D)。Y.Agatsuma
*/
#define RFWERR_OK						(0x00000000)	// コマンド処理を正常終了した
#define RFWERR_ERR						(0x00000001)	// コマンド処理内でエラーが発生した
#define RFWERR_N_RFW_ARG				(0x00000100)	// 引数の指定に誤りがある
#define RFWERR_N_BFW_CMD				(0x00000101)	// 未定義のBFWコマンドを受信した
#define RFWERR_N_BFWFROM_ERASE			(0x00010100)	// BFW格納用フラッシュROMのイレーズエラー
#define RFWERR_N_BFWFROM_WRITE			(0x00010101)	// BFW格納用フラッシュROMの書き込みエラー
#define RFWERR_N_BFWFROM_VERIFY			(0x00010102)	// BFW格納用フラッシュROMのベリファイエラー
#define RFWERR_N_FPGA_CONFIG			(0x00010103)	// FPGAコンフィグレーションエラー
#define RFWERR_W_USB_VBUS_LOW			(0x10010104)	// USBバスパワー電圧が低い(Warning)
#define RFWERR_N_SID_SIZEOVER			(0x00010105)	// シリアル番号の文字数が許容範囲を超えている
#define RFWERR_N_SID_BLANK				(0x00010106)	// シリアル番号がブランク状態で設定されていない
#define RFWERR_N_LID_UNDEFINE			(0x00010107)	// ライセンスIDが定義されていない
#define RFWERR_N_NID_SIZEOVER			(0x00010108)	// ニックネームの文字数が許容範囲を超えている
#define RFWERR_N_NID_BLANK				(0x00010109)	// ニックネームがブランク状態で設定されていない
#define RFWERR_N_MCUROM_ERASE			(0x00010206)	// ターゲットMCU内蔵 フラッシュROMのイレーズエラー
#define RFWERR_N_MCUROM_WRITE			(0x00010207)	// ターゲットMCU内蔵 フラッシュROMの書き込みエラー
#define RFWERR_N_MCUROM_VERIFY			(0x00010208)	// ターゲットMCU内蔵 フラッシュROMのベリファイエラー
#define RFWERR_N_EX1BFWFROM_ERASE		(0x00010300)	// 拡張ボードF/W格納用フラッシュROMのイレーズエラー
#define RFWERR_N_EX1BFWFROM_WRITE		(0x00010301)	// 拡張ボードF/W格納用フラッシュROMの書き込みエラー
#define RFWERR_N_EX1BFWFROM_VERIFY		(0x00010302)	// 拡張ボードF/W格納用フラッシュROMのベリファイエラー
#define RFWERR_N_EX1BFPGA_CONFIG		(0x00010303)	// 拡張ボードFPGAコンフィグレーションエラー
#define RFWERR_N_EX1BEROM_ERASE			(0x00010304)	// 拡張ボードEEPROMのイレーズエラー
#define RFWERR_N_EX1BEROM_WRITE			(0x00010305)	// 拡張ボードEEPROMの書き込みエラー
#define RFWERR_N_EX1BEROM_VERIFY		(0x00010306)	// 拡張ボードEEPROMのベリファイエラー
#define RFWERR_N_EX1BEROM_READ			(0x00010307)	// 拡張ボードEEPROMのリードエラー
#define RFWERR_N_EX1BEROM_NORES			(0x00010308)	// 拡張ボードEEPROMの応答なしエラー
#define RFWERR_N_EX2BFPGA_CONFIG		(0x00010401)	// 2nd拡張ボードFPGAコンフィグレーションエラー
#define RFWERR_N_SPBFPGAFROM_ERASE		(0x00010402)	// SPボードFPGA格納用フラッシュROMのイレーズエラー
#define RFWERR_N_SPBFPGAFROM_WRITE		(0x00010403)	// SPボードFPGA格納用フラッシュROMの書き込みエラー
#define RFWERR_N_SPBFPGAFROM_VERIFY		(0x00010404)	// SPボードFPGA格納用フラッシュROMのベリファイエラー
#define RFWERR_N_EX2BEROM_ERASE			(0x00010405)	// 2nd拡張ボードEEPROMのイレーズエラー
#define RFWERR_N_EX2BEROM_WRITE			(0x00010406)	// 2nd拡張ボードEEPROMの書き込みエラー
#define RFWERR_N_EX2BEROM_VERIFY		(0x00010407)	// 2nd拡張ボードEEPROMのベリファイエラー
#define RFWERR_N_EX2BEROM_READ			(0x00010408)	// 2nd拡張ボードEEPROMのリードエラー
#define RFWERR_N_EX2BEROM_NORES			(0x00010409)	// 2nd拡張ボードEEPROMの応答なしエラー
#define RFWERR_N_PODFPGAFROM_ERASE		(0x00010500)	// エミュレーションポッドFPGA格納用 フラッシュROMのイレーズエラー
#define RFWERR_N_PODFPGAFROM_WRITE		(0x00010501)	// エミュレーションポッドFPGA格納用 フラッシュROMの書き込みエラー
#define RFWERR_N_PODFPGAFROM_VERIFY		(0x00010502)	// エミュレーションポッドFPGA格納用 フラッシュROMのベリファイエラー
#define RFWERR_N_ILLEGAL_IDCODE			(0x00020004)	// ID認証コード不一致
#define RFWERR_N_UNSUPPORT				(0x00030001)	// 指定の機能はサポートしていない。
#define RFWERR_N_NOTGETINFO				(0x00030002)	// 情報を取得する前にIFが呼ばれている。
#define RFWERR_N_CANCEL					(0x00030003)	// 他のコアでエラーが発生したため、処理を中止した。
#define RFWERR_N_FWRITE_NOTEXE_ILGLERR	(0x00030107)	// 内蔵FlashROMへのアクセス違反などが発生しているため。フラッシュ書き換えが実行できない
#define RFWERR_N_FWRITE_FAIL_ILGLERR	(0x00030108)	// 内蔵FlashROMへのアクセス違反が発生し、フラッシュ書き換えが正常に実行できなかった
#define RFWERR_N_ACC_ROM_PEMODE			(0x00030118)	// フラッシュ書き換えモード中のため、MCU内蔵ROM領域の読み出し/書き込み実行はできない
#define RFWERR_W_CLKCHG_DMB_CLR			(0x1003011C)	// メモリアクセス中のクロック切り替えなどによる通信エラーが発生したためリカバリした
#define RFWERR_N_FLASH_ALERASE_ID_FAIL	(0x0003011D)	// Flash 全消去ID によるFlash全消去に失敗した
#define RFWERR_N_DATA_EVPOINT_OVER		(0x00030201)	// データアクセスイベントの設定ポイントが最大設定点数を越えている
#define RFWERR_N_DATA_EV_NOTSET			(0x00030202)	// データアクセスイベントが設定されていない(解除時のみ返送)
#define RFWERR_N_EVPOINT_OVER			(0x00030203)	// 命令フェッチブレークイベントの設定ポイントが最大設定点数を越えている
#define RFWERR_N_EV_NOTSET				(0x00030204)	// イベントが設定されていない(解除時のみ返送)
#define RFWERR_N_WPADDR_SAM				(0x00030205)	// データアクセスイベント設定アドレスがセキュリティーエリア内のため、イベントの設定ができません。
#define RFWERR_N_EV_OVER				(0x00030206)	// イベント本数よりも多いイベントが設定された （命令フェッチ、データアクセス以外）
#define RFWERR_N_EV_UNSUPPORT			(0x00030207)	// サポートしていないイベント機能が設定された
#define RFWERR_N_EV_OVERLAPP			(0x00030208)	// 重複したイベント番号が設定された
#define RFWERR_N_VC_UNSUPPORT			(0x00030209)	// サポートしていないベクタキャッチが設定された
#define RFWERR_N_TRC_EVPOINT_OVER		(0x0003020A)	// トレースイベントの設定ポイントが最大設定点数を越えている
#define RFWERR_N_EV_USECOMB				(0x0003020B)	// 指定の単体イベントが組み合わせイベントで使われている
#define RFWERR_N_EVADDR_SAM				(0x0003020C)	// データイベント設定アドレスがセキュリティーエリア内のため、データイベントの設定ができません。
#define RFWERR_N_EV_USEFUNC				(0x0003020D)	// イベントがハードブレーク、トレース等に設定されている
#define RFWERR_N_EV_UNSUPPORT_ADDR		(0x0003020E)	// 指定されたアドレスがサポートされない値
#define RFWERR_N_SBPOINT_OVER			(0x00030400)	// ソフトウェアブレークの設定ポイントが最大設定点数を超えている
#define RFWERR_N_SBADDR_NOTSET			(0x00030401)	// ソフトウェアブレークが設定されていない
#define RFWERR_N_SBAREA_OUT				(0x00030402)	// 指定領域はソフトウェアブレークポイント設定不可領域である
#define RFWERR_N_SBADDR_SETFAIL			(0x00030403)	// 指定領域へのソフトウェアブレークポイント設定に失敗した
#define RFWERR_N_SBADDR_OVERLAPP		(0x00030404)	// 既にソフトブレークポイントが設定されているアドレスへ別のソフトブレークポイントを重複して設定することはできない
#define RFWERR_W_SBPOINT_DELETE			(0x10030405)	// 設定していたソフトウェアブレークポイントが消失した(Warning)
#define RFWERR_N_SB_ROMAREA_MCURUN		(0x00030406)	// ユーザプログラム実行中にMCU内蔵ROM領域へのソフトウェアブレークは設定/解除できない
#define RFWERR_N_SB_ROMAREA_FLASHDEBUG	(0x00030408)	// フラッシュ書き替えデバッグ中にMCU内蔵ROM領域へのソフトウェアブレークは設定できない
#define RFWERR_N_SB_UCODE_SETFAIL		(0x00030409)	// 指定領域へのユーザ命令コード復帰に失敗した
#define RFWERR_N_SBADDR_SAM				(0x0003040C)	// ソフトウェアブレークの設定ポイントがセキュリティエリア内ため設定できない
#define RFWERR_N_STOP_AMCU_RESET		(0x0003040D)	// リセットがはいっていたため、ブレークできなかった。
#define RFWERR_N_HB_ILLEGAL_EVENT		(0x0003040E)	// ハードウェアブレークポイントとして、指定できないイベントが設定された。
#define RFWERR_N_RTTSTAT_RECORDING		(0x00030702)	// トレース動作中である
#define RFWERR_N_RTTDATA_NON			(0x00030703)	// トレースデータがない
#define RFWERR_N_RTTSTOP_FAIL			(0x0003070D)	// トレースストップに失敗した(トレースストップタイムアウトエラー)
#define RFWERR_N_RTTMODE_UNSUPRT		(0x0003070E)	// サポートしていないトレースモード
#define RFWERR_N_RTTSTAT_DIS			(0x0003070F)	// トレースは動作していません。
#define RFWERR_N_RTT_UNSUPPORT			(0x00030711)	// トレース機能はサポートしていない。
#define RFWERR_N_ATARGET_RESET			(0x00030800)	// ターゲットのリセット端子が制御できない。
#define RFWERR_N_ATARGET_TRSTN			(0x00030801)	// ターゲットのTRSTn端子が制御できない。
#define RFWERR_N_ATARGET_TDI			(0x00030802)	// ターゲットのTDI端子が制御できない。
#define RFWERR_N_ATARGET_TMS			(0x00030803)	// ターゲットのTMS/SWDIO端子が制御できない。
#define RFWERR_N_ATARGET_TCK			(0x00030804)	// ターゲットのTCK/SWCLK端子が制御できない。
#define RFWERR_N_AMCU_NO_HALT			(0x00030805)	// ターゲットの指定コアをHALTできない。
#define RFWERR_N_AMCU_NO_DAP			(0x00030806)	// DAPが見つからない。
#define RFWERR_N_HOTPLUGIN_PWR			(0x00030807)	// ホットプラグイン起動時は電源供給できない。
#define RFWERR_N_BTARGET_UVCC2_POWEROFF	(0x00030808)	// 2電源ターゲットシステムの電源2の電源がOFF状態のため、コマンド処理を実行できない。
#define RFWERR_N_BTARGET_UVCC2_ENABLE	(0x00030809)	// 1電源ターゲットシステム指定で、ターゲット接続が2電源状態になっている。
#define RFWERR_N_COM_USED				(0x0003080A)	// 指定されたエミュレータは既に使用されている
#define RFWERR_N_COM_NOT_FOUND			(0x0003080B)	// 指定されたエミュレータが見つからない
#define RFWERR_D_NON_RFW_INIT			(0x0F03080C)	// RFWのINIT処理が行われていない。
#define RFWERR_N_ALREADY_CONNECT		(0x0003080D)	// 既にターゲットMCUへ接続済みです。
#define RFWERR_N_BTARGET_POWERSUPPLY	(0x0003080E)	// ユーザシステムに外部から電源供給されている。
#define RFWERR_W_TARGET_POWERLOW		(0x1003080F)	// ユーザシステムの電圧が供給電圧値より低い(Warning)
#define RFWERR_N_BTARGET_POWERASP		(0x00030810)	// ユーザシステムに電源供給中にアシストプローブへ電源供給できない
#define RFWERR_N_BASP_POWERTARGET		(0x00030811)	// アシストプローブへ電源供給中にユーザシステムに電源供給できない
#define RFWERR_N_BASP_POWERSUPPLY		(0x00030812)	// アシストプローブに外部から電源供給されている
#define RFWERR_N_BNONASP				(0x00030813)	// アシストプローブ未サポートのエミュレータである
#define RFWERR_N_BMCU_RUN				(0x00040000)	// ユーザプログラム実行中のためコマンド処理を実行できない
#define RFWERR_N_BMCU_STOP				(0x00040001)	// ユーザプログラム停止中のためコマンド処理を実行できない
#define RFWERR_N_BMCU_NONCONNECT		(0x00040002)	// MCUとの接続が未完了
#define RFWERR_N_BTARGET_POWER			(0x00040100)	// ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない
#define RFWERR_N_BTARGET_RESET			(0x00040101)	// ターゲットシステム上のリセット端子が 'L'アクティブ状態のためコマンド処理を実行できない
#define RFWERR_N_BTARGET_NOCONNECT		(0x00040102)	// ターゲットシステムと接続されていない
#define RFWERR_N_BMCU_RESET				(0x00040200)	// MCUがリセット状態のためコマンド処理を実行できない
#define RFWERR_N_BMCU_STANBY			(0x00040201)	// MCUのスタンバイモード中で内部クロックが停止状態である。
#define RFWERR_N_BMCU_SLEEP				(0x00040202)	// MCUのスリープモード中で内部クロックが停止状態である。
#define RFWERR_N_BMCU_PDOWN				(0x00040203)	// MCUがパワーダウン状態のため内部クロックが停止状態である。
#define RFWERR_N_BMCU_AUTH_DIS			(0x00040204)	// MCU内部リセットが発生したため、コマンド処理を実行できない
#define RFWERR_W_BMCU_PDOWN				(0x10040205)	// MCUがパワーダウン状態のためコマンド処理を実行できません。(Warning)
#define RFWERR_W_BMCU_OSDLK				(0x10040206)	// MCUがOS Double Lock状態のためコマンド処理が実行できません。(Warning)
#define RFWERR_W_BMCU_STANBY			(0x10040207)	// MCUがSTANDBY状態のためコマンド処理が実行できません。(Warning)
#define RFWERR_N_BEXE_LEVEL0			(0x00040300)	// BFW内部動作モードがレベル0の状態でない。
#define RFWERR_N_EML_ENDCODE			(0x00040301)	// EML領域にエンドコードがない
#define RFWERR_N_PRG_ENDCODE			(0x00040302)	// PRG領域にエンドコードがない
#define RFWERR_N_EML_TO_PRG				(0x00040303)	// EML動作モードからPRG動作モードへの遷移はできない
#define RFWERR_N_PRG_TO_EML				(0x00040304)	// PRG動作モードからEML動作モードへの遷移はできない

#define RFWERR_N_ASP_DISABLE_ASYNC		(0x00040305)	// 非同期デバッグモード中のためE2エミュレータ拡張機能を有効化できない
#define RFWERR_N_ASP_DISABLE_EMU		(0x00040306)	// E2エミュレータ以外では拡張機能は有効化できない
#define RFWERR_N_MEMORY_NOT_SECURED		(0x00040307)	// PC上メモリを確保できなかった

#define RFWERR_N_BASP_ASPOFF			(0x00040500)	// E2エミュレータ拡張機能が無効状態のためコマンド処理を実行できない
#define RFWERR_N_BASP_PINDISABLE		(0x00040501)	// 指定のASP端子は無効状態のため設定できない
#define RFWERR_N_BASP_PINNOUSE			(0x00040502)	// 指定したASP端子は使用されていないためコマンド処理を実行できない
#define RFWERR_N_BASP_CHDISABLE			(0x00040503)	// 指定したchは無効状態のため設定できない
#define RFWERR_N_BASP_CHNOUSE			(0x00040504)	// 指定したchは使用されていないためコマンド処理を実行できない
#define RFWERR_N_BASP_FACT_NOSUPPORT	(0x00040505)	// 指定したE2トリガはサポートしていない

#define RFWERR_N_BASP_FACT_DISABLE		(0x00040506)	// 指定したE2トリガは無効(条件が未設定)であるため使用できない
#define RFWERR_N_BASP_FACT_NONSEL		(0x00040507)	// E2トリガが指定されていない
#define RFWERR_N_BASP_SGLLNK_ILLEGAL	(0x00040508)	// シングルE2トリガに複数個の要因を設定できない
#define RFWERR_N_BASP_MLTLNK_USED		(0x00040509)	// 指定したマルチE2トリガ・アクションは既に使用されているため指定できない
#define RFWERR_N_BASP_FACT_CANTCLR		(0x0004050A)	// E2トリガはE2アクションにリンクされているため消去できない
#define RFWERR_N_BASP_LFACT_CANTCHG		(0x0004050B)	// E2トリガがレベル型アクションにリンクされているため変更できない
#define RFWERR_N_BASP_PFACT_CANTCHG		(0x0004050C)	// E2トリガがパルス型アクションにリンクされているため変更できない
#define RFWERR_N_BASP_LFACT_CANTSET		(0x0004050D)	// パルス型アクションにレベル型E2トリガを設定できない
#define RFWERR_N_BASP_PFACT_CANTSET		(0x0004050E)	// レベル型アクションにパルス型E2トリガを設定できない
#define RFWERR_N_BASP_BRKEV_NOUSE		(0x0004050F)	// 拡張機能のブレークイベントは使用されていないためコマンド処理を実行できない
#define RFWERR_N_BASP_STOPTRCOFF		(0x00040510)	// E2拡張機能のトレース停止機能が無効状態のためコマンド処理を実行できない
#define RFWERR_N_BASP_SWTRCCOMB_CANTUSE	(0x00040511)	// 組み合わせ検出は別chを使用中のため使用できない
#define RFWERR_N_BASP_SWTRCCH_CANTUSE	(0x00040512)	// 指定したchは組み合わせ検出で使用しているためコマンド処理を実行できない
#define RFWERR_N_BASP_STOPTRC_NOUSE		(0x00040513)	// 拡張機能のトレース停止は使用されていないためコマンド処理を実行できない
#define RFWERR_N_BASP_PINEXCLUSIVE		(0x00040514)	// 指定した機能は端子共通のため同時に使用できない
#define RFWERR_N_BASP_FUNCEXCLUSIVE		(0x00040515)	// 指定した機能は排他機能であるため同時に使用できない
#define RFWERR_N_BASP_FUNC_USED			(0x00040516)	// 機能は使用中のため変更できない
#define RFWERR_N_BASP_PWRMONOFF			(0x00040517)	// パワーモニタ機能が無効状態のためコマンド処理を実行できない
#define RFWERR_N_BASP_PWRMON_NOUSE		(0x00040518)	// パワーモニタは使用されていないためコマンド処理を実行できない
#define RFWERR_N_BASP_CANMONOFF			(0x00040519)	// CANモニタ機能が無効状態のためコマンド処理を実行できない
#define RFWERR_N_BASP_CANMON_NOUSE		(0x0004051A)	// 指定したCANモニタのchは使用されていないためコマンドを実行できない
#define RFWERR_N_BASP_SWTRCOFF			(0x0004051B)	// 外部ソフトトレースが無効状態のためコマンド処理を実行できない
#define RFWERR_N_BASP_MONEVOFF			(0x0004051C)	// モニタイベントが無効状態のためコマンド処理を実行できない
#define RFWERR_N_BASP_ROMWRITE_DISABLE	(0x0004051D)	// フラッシュ書き換え禁止デバッグ中のため、ASP機能を使用できない
#define RFWERR_N_BASP_MONEV_NONOP		(0x0004051E)	// 指定したアドレスがNOP命令ではないので、ソフトウェアブレーク方式のモニタイベントが設定できない。
#define RFWERR_N_BASP_MONEV_BREAKADDR	(0x0004051F)	// デバッグ機能のブレークポイントに設定しているアドレスにモニタイベントは設定できない。
#define RFWERR_N_BASP_MONEV_OVERLAPP	(0x00040520)	// 他のモニタイベント条件番号に指定済みのアドレスである
#define RFWERR_N_BASP_MONEV_NOTSET		(0x00040521)	// 指定したモニタイベント条件番号が設定されていない
#define RFWERR_N_BASP_LOAD				(0x00040522)	// 記録ファイル未LOAD
#define RFWERR_N_BASP_ALREADY_LOAD		(0x00040523)	// 記録ファイルLoad済
#define RFWERR_N_BASP_F_OPEN			(0x00040524)	// 記録ファイルOpenエラー
#define RFWERR_N_BASP_F_SEEK			(0x00040525)	// 記録ファイルSeekエラー
#define RFWERR_N_BASP_F_READ			(0x00040526)	// 記録ファイルReadエラー
#define RFWERR_N_BASP_F_WRITE			(0x00040527)	// 記録ファイルWriteエラー
#define RFWERR_N_BASP_F_CLOSE			(0x00040528)	// 記録ファイルCloseエラー
#define RFWERR_N_BASP_F_FORMAT			(0x00040529)	// 記録ファイルフォーマット不正
#define RFWERR_N_BASP_FPGA_START2		(0x0004052A)	// FPGA startbit異常
#define RFWERR_N_BASP_FPGA_SEQ			(0x0004052B)	// FPGA シーケンス異常
#define RFWERR_N_BASP_FPGA_SIZE			(0x0004052C)	// FPGA	サイズ異常
#define RFWERR_N_BASP_PWRMON_E2PWROFF	(0x0004052D)	// E2からターゲットへ電源供給していないため、パワーモニタを有効に設定できない
#define RFWERR_N_BASP_RUNNING			(0x0004052E)	// ASP動作中のため実行不可
#define RFWERR_N_BASP_NO_MEM			(0x0004052F)	// ASP動作のためのメモリが確保できない
#define RFWERR_N_BASP_ASPIF_PWROFF		(0x00040530)	// ASP I/Fに電源供給していないため、E2拡張機能が使用できない
#define RFWERR_N_BASP_ASPFUNCOFF		(0x00040531)	// 指定した機能が無効状態のため、コマンド処理を実行できない
#define RFWERR_N_BASP_TSTAMP_OVER		(0x00040532)	// タイムスタンプがオーバーフローしたため、計測結果は返せない
#define RFWERR_N_BASP_NOSUPPORT			(0x00040533)	// 指定したE2拡張機能はサポートしていない
#define RFWERR_N_BASP_NONSEL_ACTLEVEL	(0x00040534)	// 外部トリガ出力のアクティブレベルが未選択のためコマンド処理を実行できない
#define RFWERR_N_BASP_COMB_NOSUPPORT	(0x00040535)	// 指定したE2トリガアクションの組合せはサポートしていない
#define RFWERR_N_BASP_SRAM_OVF			(0x00040536)	// SRAM_OVFのフレームを検出
#define RFWERR_N_BASP_SFIFO_OVF			(0x00040537)	// SFIFO_OVFのフレームを検出
#define RFWERR_N_BASP_SWT_NOREC			(0x00040538)	// ソフトウェア・トレース(LPD)が記録できない状態を継続中
#define RFWERR_W_RAM_WAIT_TMP_CHANGE	(0x10040539)	// RAMのウェイト設定を一時的に変更してコマンドを実行した(Warning)
#define RFWERR_W_ROM_WAIT_TMP_CHANGE	(0x10040540)	// ROMのウェイト設定を一時的に変更してコマンドを実行した(Warning)
#define RFWERR_N_SRM_USE_WORKRAM		(0x00040541)	// 指定ルーチン実行機能用ワークRAMを別の用途で使用している

#define RFWERR_N_AMCU_IABORT			(0x00070000)	// MCUに命令アボートが発生した。
#define RFWERR_N_AMCU_DABORT			(0x00070001)	// MCUにデータアボートが発生した。
#define RFWERR_N_BFWWAIT_TIMEOUT		(0x00070100)	// BFWウェイト処理で指定時間以内に指定状態にならずタイムアウトが発生した。
#define RFWERR_SU_AMCU_RESET			(0x03000001)	// タイムアウトが発生した。MCUがリセット状態である。
#define RFWERR_SU_AMCU_STANBY			(0x03000003)	// タイムアウトが発生した。MCUのスタンバイモード中で内部クロックが停止状態である。
#define RFWERR_SU_AMCU_SLEEP			(0x03000004)	// タイムアウトが発生した。MCUのスリープモード中で内部クロックが停止状態である。
#define RFWERR_SU_AMCU_PDOWN			(0x03000005)	// タイムアウトが発生した。MCUがパワーダウン状態のため内部クロックが停止状態である。
#define RFWERR_SU_AMCU_AUTH_DIS			(0x03000010)	// タイムアウトが発生した。MCUとのデバッガ認証切れ状態である。
#define RFWERR_SU_BMCU_AUTH_DIS			(0x03000011)	// MCUとのデバッガ認証切れ状態のためコマンド処理できない。
#define RFWERR_SU_ATARGET_NOCONNECT		(0x03000308)	// タイムアウトが発生した。ターゲットシステムと接続されていない。
#define RFWERR_SU_ATARGET_POWER			(0x03000309)	// タイムアウトが発生した。ターゲットシステム上のMCU供給電源がOFF状態のためコマンド処理を実行できない。
#define RFWERR_SU_ATARGET_RESET			(0x0300030A)	// タイムアウトが発生した。ターゲットシステム上のリセット端子が 'L'アクティブ状態のためコマンド処理を実行できない。
#define RFWERR_SU_ATARGET_RESET_ST		(0x0300030B)	// タイムアウトが発生した。ターゲットシステム上のリセット端子が一時的に 'L'アクティブ状態となったため以降のコマンド処理を実行できなかった。
#define RFWERR_SU_SWD_TRANSFER			(0x03000500)	// SWD通信でエラーが発生した。
#define RFWERR_SU_DAP_STICKYERR			(0x03000501)	// DAPのSTICKYエラー発生。エラーが特定できない。
#define RFWERR_SU_DAP_WAIT_TIMEOUT		(0x03000502)	// DAPとの通信で、ACKがWAITからOKにならなかった。
#define RFWERR_SU_DAP_PROTOCOL_ERR		(0x03000503)	// DAPとの通信で、想定外のACK(FAULT/WAIT/OK以外)が返ってきた、もしくはプロトコルエラーが発生した。
#define RFWERR_SU_SWD_FAULT				(0x03000504)	// SWDのACKでFAULTが発生した。
#define RFWERR_SU_LPD_TRANSFER			(0x03000505)	// LPD通信でエラーが発生した。
#define RFWERR_SU_MON_TIMEOUT			(0x03FFFF00)	// モニタプログラム処理でタイムアウトが発生した。エラーが特定できない。
#define RFWERR_SU_BFW_TIMEOUT			(0x03FFFF01)	// BFW処理でタイムアウトが発生した。
#define RFWERR_SU_DAP_STICKYERR_TIMEOUT	(0x03FFFF02)	// タイムアウトが発生した。DAPのSTICKYエラー発生。エラーが特定できない。
#define RFWERR_SU_MULTI_RFW_REQ_TIMEOUT (0x03FFFF03)	// RFWの複数処理が、指定された時間内に終了しませんでした。

#define RFWERR_D_COM					(0x0F000000)	// 致命的な通信異常が発生した。(Communi.DLLの応答が異常である)
#define RFWERR_D_COM_1ST				(0x0F000001)	// BFWとの最初のコマンド送受信時に通信エラーが発生した
#define RFWERR_D_COM_USBT_CHANGE		(0x0F000002)	// USB転送サイズ変更後に通信エラーが発生した
#define RFWERR_D_COMDATA_CMDCODE		(0x0F000003)	// BFWから受信したコマンドコードが異常である。
#define RFWERR_D_COMDATA_ERRCODE		(0x0F000004)	// BFWから受信したエラーコードが異常である。
#define RFWERR_N_COM_LIST				(0x00060001)	// 既にエミュレータに接続済のため、リスト取得不可

#define RFWERR_N_BFWFILE_NOT_FOUND		(0x00050000)	// BFWファイルが見つからない、オープンできなかった
#define RFWERR_N_FPGAFILE_NOT_FOUND		(0x00050001)	// FPGAファイルが見つからない、オープンできなかった
#define RFWERR_N_FILE_NOT_FOUND			(0x00050002)	// 指定ファイルが見つからない、オープンできなかった
#define RFWERR_N_SECTION_NOT_FOUND		(0x00050003)	// 外部ファイル内に該当するセクションが見つからない
#define RFWERR_N_NOMESSAGE_ERROR		(0x0FFFFFFF)	// エラーメッセージ未登録のエラー

#endif	// __RFWERR_H__
