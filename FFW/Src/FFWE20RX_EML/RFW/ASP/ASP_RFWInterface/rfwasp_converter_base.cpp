///////////////////////////////////////////////////////////////////////////////
/**
 * @file rfwasp_converter_base.cpp
 * @brief ASP記録ファイル処理
 * @author TSSR M.Ogata
 * @author Copyright (C) 2015, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date 2017/03/28
 */
///////////////////////////////////////////////////////////////////////////////

#include "rfwasp_converter.h"
#include "rfwasp_rec.h"

#if defined(ASP_CONSOLE)
/* consoleの場合はAD変換しない */
#define CalcAd2Current(x) (bit_extract<DWORD, DWORD>(frame._rec[frame._rec.size()-1], 16, 16));
#else
#include "doasp_pwr.h"
#endif

using namespace std;

BYTE* AspConverterBase::_cnv_buf = 0;
UINT64 AspConverterBase::_cnv_buf_max = 0;
UINT64 AspConverterBase::_cnv_cnt = 0;
DWORD AspConverterBase::_store_cnt = 0;
DWORD *AspConverterBase::_pexist_cnt = 0;

//=============================================================================
/**
 * RFWASP_POWER_RECORDへの変換
 * @param power	変換先構造体
 * @param frame 変換元frame
 * @param clk クロック単位の時間
 */
//=============================================================================
void AspConverterBase::ToApiRec(RFWASP_POWER_RECORD *power, const RecFrame &frame, UINT64 clk)
{
	*power = RFWASP_POWER_RECORD();
	if (ToApiCommon(&power->common, frame, clk)) {
		power->dwUa = CalcAd2Current(frame[0]);
	}
}

//=============================================================================
/**
 * RFWASP_EVENT_RECORDへの変換
 * @param event	変換先構造体
 * @param frame 変換元frame
 * @param clk クロック単位の時間
 */
//=============================================================================
void AspConverterBase::ToApiRec(RFWASP_EVENT_RECORD *event, const RecFrame &frame, UINT64 clk)
{
	*event = RFWASP_EVENT_RECORD();
	if (ToApiCommon(&event->common, frame, clk)) {
		event->factor  = bit_extract<DWORD, DWORD>(frame[0], 16, 16) +
			(bit_extract<DWORD, DWORD>(frame[1], 16, 16) << 16);
	}
}

//=============================================================================
/**
 * RFWASP_CAN_RECORDへの変換
 * @param can	変換先構造体
 * @param frame 変換元frame
 * @param clk クロック単位の時間
 */
//=============================================================================
void AspConverterBase::ToApiRec(RFWASP_CAN_RECORD *can, const RecFrame &frame, UINT64 clk)
{
	*can = RFWASP_CAN_RECORD();
	if (ToApiCommon(&can->common, frame, clk)) {
		can->dwId = bit_extract<DWORD, DWORD>(frame[0], 25, 7)
			+ (bit_extract<DWORD, DWORD>(frame[1], 1, 4) << 7)
			+ (bit_extract<DWORD, DWORD>(frame[1], 5, 18) << 11);
		can->bySrr = bit_extract<BYTE, DWORD>(frame[0], 23, 1);
		can->byIde = bit_extract<BYTE, DWORD>(frame[0], 22, 1);
		can->byRtr = bit_extract<BYTE, DWORD>(frame[0], 24, 1);
		can->byDlc = bit_extract<BYTE, DWORD>(frame[0], 16, 4);
		can->byData[0] = bit_extract<BYTE, DWORD>(frame[1], 23, 8);
		can->byData[1] = bit_extract<BYTE, DWORD>(frame[1], 31, 1)
			+ (bit_extract<BYTE, DWORD>(frame[2], 1, 7) << 1);
		can->byData[2] = bit_extract<BYTE, DWORD>(frame[2], 8, 8);
		can->byData[3] = bit_extract<BYTE, DWORD>(frame[2], 16, 8);
		can->byData[4] = bit_extract<BYTE, DWORD>(frame[2], 24, 8);
		can->byData[5] = bit_extract<BYTE, DWORD>(frame[3], 1, 8);
		can->byData[6] = bit_extract<BYTE, DWORD>(frame[3], 9, 8);
		can->byData[7] = bit_extract<BYTE, DWORD>(frame[3], 17, 8);
		can->wCrc = bit_extract<WORD, DWORD>(frame[3], 25, 7)
			+ (bit_extract<WORD, DWORD>(frame[4], 1, 8) << 7);
		can->byCrcDel = bit_extract<BYTE, DWORD>(frame[4], 9, 1);
		can->byAck = bit_extract<BYTE, DWORD>(frame[4], 10, 1);
		can->byAckDel = bit_extract<BYTE, DWORD>(frame[4], 11, 1);
		can->byEof = bit_extract<BYTE, DWORD>(frame[4], 12, 7);
		can->byItm = bit_extract<BYTE, DWORD>(frame[4], 19, 3);
		can->byOlf = bit_extract<BYTE, DWORD>(frame[4], 22, 1);
		can->wErrorStatus = bit_extract<WORD, DWORD>(frame[4], 23, 9);
	} 
}

//=============================================================================
/**
 * RF_ASPTYPE_xxからFrameTypeへの変換
 * @param dwType RF_ASPTYPE_
 * @return FrameType
 */
//=============================================================================
FrameType AspConverterBase::to_frametype(DWORD dwType)
{
	FrameType type = MAX_FRAME_TYPE;

	switch (dwType) {
	case RF_ASPTYPE_LOGIC_ANALYZER:
		type = IO_CH0_RX;
		break;
	case RF_ASPTYPE_UART_CH0_TX:
		type = IO_CH0_TX;
		break;
	case RF_ASPTYPE_UART_CH0_RX:
		type = IO_CH0_RX;
		break;
	case RF_ASPTYPE_I2C_CH0:
		type = IO_CH0_TX;
		break;
	case RF_ASPTYPE_SPI_CH0_MO:
		type = IO_CH0_TX;
		break;
	case RF_ASPTYPE_SPI_CH0_MI:
		type = IO_CH0_RX;
		break;
	case RF_ASPTYPE_CAN_CH0_RX:
		type = IO_CH0_RX;
		break;
	case RF_ASPTYPE_UART_CH1_TX:
		type = IO_CH1_TX;
		break;
	case RF_ASPTYPE_UART_CH1_RX:
		type = IO_CH1_RX;
		break;
	case RF_ASPTYPE_I2C_CH1:
		type = IO_CH1_TX;
		break;
	case RF_ASPTYPE_SPI_CH1_MO:
		type = IO_CH1_TX;
		break;
	case RF_ASPTYPE_SP1_CH1_MI:
		type = IO_CH1_RX;
		break;
	case RF_ASPTYPE_CAN_CH1_RX:
		type = IO_CH1_RX;
		break;
	case RF_ASPTYPE_POWER:
		type = POWER;
		break;
	case RF_ASPTYPE_EVL:
		type = EVL;
		break;
	case RF_ASPTYPE_PC:
		type = PC_MEM;
		break;
	case RF_ASPTYPE_MEMORY:
		type = PC_MEM;
		break;
	default:
		type = AspConverter::AspTypeToFpgaFrameType(dwType);
		break;
	}
	return type;
}

//=============================================================================
/**
 * FrameTypeからRF_ASPTYPEへの変換
 * @param type FrmaeType
 * @return RF_ASPTYPE_xx
 */
//=============================================================================
DWORD AspConverterBase::to_asptype(FrameType type)
{
	DWORD	dwType = 0;

	switch (type) {
	case SRAM_OVF:
		break;
	case IO_CH0_TX:
		break;
	case IO_CH0_RX:
		/* ToDo UART/I2C/SPI対応 */
		dwType = RF_ASPTYPE_CAN_CH0_RX;
		break;
	case IO_CH1_TX:
		break;
	case IO_CH1_RX:
		/* ToDo UART/I2C/SPI対応 */
		dwType = RF_ASPTYPE_CAN_CH1_RX;
		break;
	case POWER:
		dwType = RF_ASPTYPE_POWER;
		break;
	case EVL:
		dwType = RF_ASPTYPE_EVL;
		break;
	case PC_MEM:
		/* ToDo */
		/* RecFrame内を参照しないとPC/MEM区別付かないのでここでは0 */
		dwType = 0;
		break;
	default:
		dwType = AspConverter::FpgaFrameTypeToAspType(type);
		break;
	}
	return dwType;
}

//=============================================================================
/**
 * RecFrameからRFASP_構造体への変換
 * @param frame RecFrmae
 * @param clk クロック単位の時間
 * @retval true 変換成功
 * @retval false 変換先バッファがfullで変換できず
 */
//=============================================================================
bool AspConverterBase::extract(const RecFrame &frame, UINT64 clk)
{
	switch (frame._type) {
	case SRAM_OVF:
		break;
	case IO_CH0_TX:
		break;
	case IO_CH0_RX:
		/* ToDo UART/I2C/SPI対応 */
		if (ToApiStruct<RFWASP_CAN_RECORD>(frame, clk) == false)
			return false;
		break;
	case IO_CH1_TX:
		break;
	case IO_CH1_RX:
		/* ToDo UART/I2C/SPI対応 */
		if (ToApiStruct<RFWASP_CAN_RECORD>(frame, clk) == false)
			return false;
		break;
	case POWER:
		if (ToApiStruct<RFWASP_POWER_RECORD>(frame, clk) == false)
			return false;
		break;
	case EVL:
		if (ToApiStruct<RFWASP_EVENT_RECORD>(frame, clk) == false)
			return false;
		break;
	case PC_MEM:
		// ToDo
		break;
	default:
		/* 共通部以外のAPI構造体変換 */
		UINT64 struct_size = AspConverter::convert_size(frame._type);
		if (struct_size > 0) {
			if (_cnv_buf) {
				if (_cnv_cnt + struct_size <= _cnv_buf_max) {
					std::fill_n(&_cnv_buf[_cnv_cnt], struct_size, 0);
					if (ToApiCommon(reinterpret_cast<RFWASP_REC_COMMON*>(&_cnv_buf[_cnv_cnt]), frame, clk)) {
						AspConverter::convert(frame._type, &_cnv_buf[_cnv_cnt], frame);
					}
					_cnv_cnt += struct_size;
					++_store_cnt;
				} else {
					return false;
				}
			} else {
				_cnv_cnt += struct_size;
			}
		}
		break;
	}
	return true;
}

//=============================================================================
/**
 * RecFrameからRFASP構造体共通部への変換
 * @param common 変換先構造体
 * @param frame 変換元frame
 * @param clk クロック単位の時間
 */
//=============================================================================
bool AspConverterBase::ToApiCommon(RFWASP_REC_COMMON *common, const RecFrame &frame, UINT64 clk)
{
	common->time_nsec = clk_to_nsec(calib_clk(clk, frame._type), OFF);
	common->type = to_asptype(frame._type);
	if (frame._bsfifo_ovf) {
		common->type |= (1<<31);
		return false;
	} else {
		return true;
	}
}

