///////////////////////////////////////////////////////////////////////////////
/**
 * @file instcode.cpp
 * @brief 命令コード解析処理
 * @author RSO Y.Minami, H.Hashiguchi, S.Ueda, PA M.Tsuji
 * @author Copyright (C) 2009(2010-2018) Renesas Electronics Corporation. All rights reserved.
 * @date 2018/02/28
 */
///////////////////////////////////////////////////////////////////////////////
/*
■改定履歴
・ソース構成改善	2012/07/10 橋口
　　ベース:FFWE20RX600.dll V.1.02.00.015
・RevRxNo130301-001 2013/04/23 上田
	RX64M対応
・RevRxNo180228-001 2018/02/28 PA 辻
	RX66T-L対応
*/
#include "instcode.h"

// ファイル内static変数の宣言
// 命令解析テーブルの実体
FFWRX_INST_MST tblRX_INST_CMD[]={
	{	0x75400000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//1 [01_転送1(即値のレジスタセット)] MOV #uimm:u8,Rd
	{	0xfb060000,	0xff0f0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//2 [01_転送1(即値のレジスタセット)] MOV #simm:8,Rd
	{	0xfb0a0000,	0xff0f0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//3 [01_転送1(即値のレジスタセット)] MOV #simm:16,Rd
	{	0xfb0e0000,	0xff0f0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//4 [01_転送1(即値のレジスタセット)] MOV #simm:24,Rd
	{	0xfb020000,	0xff0f0000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//5 [01_転送1(即値のレジスタセット)] MOV #imm:32,Rd
	{	0x66000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//6 [01_転送1(即値のレジスタセット)] MOV #uimm:4,Rd
	{	0xcf000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//7 [02_転送(レジスタ間転送)] MOV Rs,Rd
	{	0xdf000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//8 [02_転送(レジスタ間転送)] MOV Rs,Rd
	{	0xef000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//9 [02_転送(レジスタ間転送)] MOV Rs,Rd
	{	0xb0000000,	0xf8000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//10 [03_転送1(ロード)] MOVU dsp:5[Rs],Rd
	{	0xb8000000,	0xf8000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//11 [03_転送1(ロード)] MOVU dsp:5[Rs],Rd
	{	0x5b000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//12 [02_転送(レジスタ間転送)] MOVU Rs,Rd
	{	0x5f000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//13 [02_転送(レジスタ間転送)] MOVU Rs,Rd
	{	0x88000000,	0xf8000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//14 [03_転送1(ロード)] MOV dsp:5[Rs],Rd
	{	0xcc000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//15 [03_転送1(ロード)] MOV [Rs],Rd
	{	0xcd000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//16 [03_転送1(ロード)] MOV dsp:8[Rs],Rd
	{	0xce000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//17 [03_転送1(ロード)] MOV dsp:16[Rs],Rd
	{	0xfe400000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//18 [03_転送1(ロード)] MOV [Ri,Rb],Rd
	{	0x98000000,	0xf8000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//19 [03_転送1(ロード)] MOV dsp:5[Rs],Rd
	{	0xdc000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//20 [03_転送1(ロード)] MOV [Rs],Rd
	{	0xdd000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//21 [03_転送1(ロード)] MOV dsp:8[Rs],Rd
	{	0xde000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//22 [03_転送1(ロード)] MOV dsp:16[Rs],Rd
	{	0xfe500000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//23 [03_転送1(ロード)] MOV [Ri,Rb],Rd
	{	0x80000000,	0xf8000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//24 [04_転送1(ストア)] MOV Rs,dsp:5[Rd]
	{	0xc3000000,	0xff000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//25 [04_転送1(ストア)] MOV Rs,[Rd]
	{	0xc7000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//26 [04_転送1(ストア)] MOV Rs,dsp:8[Rd]
	{	0xcb000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//27 [04_転送1(ストア)] MOV Rs,dsp:16[Rd]
	{	0xfe000000,	0xfff00000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//28 [04_転送1(ストア)] MOV Rs,[Ri,Rb]
	{	0x90000000,	0xf8000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//29 [04_転送1(ストア)] MOV Rs,dsp:5[Rd]
	{	0xd3000000,	0xff000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//30 [04_転送1(ストア)] MOV Rs,[Rd]
	{	0xd7000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//31 [04_転送1(ストア)] MOV Rs,dsp:8[Rd]
	{	0xdb000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//32 [04_転送1(ストア)] MOV Rs,dsp:16[Rd]
	{	0xfe100000,	0xfff00000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//33 [04_転送1(ストア)] MOV Rs,[Ri,Rb]
	{	0xa8000000,	0xf8000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//34 [03_転送1(ロード)] MOV dsp:5[Rs],Rd
	{	0xec000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//35 [03_転送1(ロード)] MOV [Rs],Rd
	{	0xed000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//36 [03_転送1(ロード)] MOV dsp:8[Rs],Rd
	{	0xee000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//37 [03_転送1(ロード)] MOV dsp:16[Rs],Rd
	{	0xfe600000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//38 [03_転送1(ロード)] MOV [Ri,Rb],Rd
	{	0x58000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//39 [03_転送1(ロード)] MOVU [Rs],Rd
	{	0x59000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//40 [03_転送1(ロード)] MOVU dsp:8[Rs],Rd
	{	0x5a000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//41 [03_転送1(ロード)] MOVU dsp:16[Rs],Rd
	{	0xfec00000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//42 [03_転送1(ロード)] MOVU [Ri,Rb],Rd
	{	0x5c000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//43 [03_転送1(ロード)] MOVU [Rs],Rd
	{	0x5d000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//44 [03_転送1(ロード)] MOVU dsp:8[Rs],Rd
	{	0x5e000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//45 [03_転送1(ロード)] MOVU dsp:16[Rs],Rd
	{	0xfed00000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//46 [03_転送1(ロード)] MOVU [Ri,Rb],Rd
	{	0xa0000000,	0xf8000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//47 [04_転送1(ストア)] MOV Rs,dsp:5[Rd]
	{	0xe3000000,	0xff000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//48 [04_転送1(ストア)] MOV Rs,[Rd]
	{	0xe7000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//49 [04_転送1(ストア)] MOV Rs,dsp:8[Rd]
	{	0xeb000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//50 [04_転送1(ストア)] MOV Rs,dsp:16[Rd]
	{	0xfe200000,	0xfff00000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//51 [04_転送1(ストア)] MOV Rs,[Ri,Rb]
	{	0x3c000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//52 [04_転送1(即値のストア)] MOV #imm:8,dsp:5[Rd]
	{	0x3d000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//53 [04_転送1(即値のストア)] MOV #uimm:8,dsp:5[Rd]
	{	0x3e000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//54 [04_転送1(即値のストア)] MOV #uimm:8,dsp:5[Rd]
	{	0xf8040000,	0xff0f0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//55 [04_転送1(即値のストア)] MOV #imm:8,[Rd]
	{	0xf9040000,	0xff0f0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//56 [04_転送1(即値のストア)] MOV #imm:8,dsp:8[Rd]
	{	0xfa040000,	0xff0f0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//57 [04_転送1(即値のストア)] MOV #imm:8,dsp:16[Rd]
	{	0xf8050000,	0xff0f0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//58 [04_転送1(即値のストア)] MOV #simm:8,[Rd]
	{	0xf9050000,	0xff0f0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//59 [04_転送1(即値のストア)] MOV #simm:8,dsp:8[Rd]
	{	0xfa050000,	0xff0f0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//60 [04_転送1(即値のストア)] MOV #simm:8,dsp:16[Rd]
	{	0xf8090000,	0xff0f0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//61 [04_転送1(即値のストア)] MOV #imm:16,[Rd]
	{	0xf9090000,	0xff0f0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//62 [04_転送1(即値のストア)] MOV #imm:16,dsp:8[Rd]
	{	0xfa090000,	0xff0f0000,	6,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//63 [04_転送1(即値のストア)] MOV #imm:16,dsp:16[Rd]
	{	0xf8060000,	0xff0f0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//64 [04_転送1(即値のストア)] MOV #simm:8,[Rd]
	{	0xf9060000,	0xff0f0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//65 [04_転送1(即値のストア)] MOV #simm:8,dsp:8[Rd]
	{	0xfa060000,	0xff0f0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//66 [04_転送1(即値のストア)] MOV #simm:8,dsp:16[Rd]
	{	0xf80a0000,	0xff0f0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//67 [04_転送1(即値のストア)] MOV #simm:16,[Rd]
	{	0xf90a0000,	0xff0f0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//68 [04_転送1(即値のストア)] MOV #simm:16,dsp:8[Rd]
	{	0xfa0a0000,	0xff0f0000,	6,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//69 [04_転送1(即値のストア)] MOV #simm:16,dsp:16[Rd]
	{	0xf80e0000,	0xff0f0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//70 [04_転送1(即値のストア)] MOV #simm:24,[Rd]
	{	0xf90e0000,	0xff0f0000,	6,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//71 [04_転送1(即値のストア)] MOV #simm:24,dsp:8[Rd]
	{	0xfa0e0000,	0xff0f0000,	7,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//72 [04_転送1(即値のストア)] MOV #simm:24,dsp:16[Rd]
	{	0xf8020000,	0xff0f0000,	6,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//73 [04_転送1(即値のストア)] MOV #imm:32,[Rd]
	{	0xf9020000,	0xff0f0000,	7,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//74 [04_転送1(即値のストア)] MOV #imm:32,dsp:8[Rd]
	{	0xfa020000,	0xff0f0000,	8,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//75 [04_転送1(即値のストア)] MOV #imm:32,dsp:16[Rd]
	{	0xc0000000,	0xff000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//76 [05_転送(メモリ間転送)] MOV [Rs],[Rd]
	{	0xc4000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//77 [05_転送(メモリ間転送)] MOV [Rs],dsp:8[Rd]
	{	0xc8000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//78 [05_転送(メモリ間転送)] MOV [Rs],dsp:16[Rd]
	{	0xc1000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//79 [05_転送(メモリ間転送)] MOV dsp:8[Rs],[Rd]
	{	0xc5000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//80 [05_転送(メモリ間転送)] MOV dsp:8[Rs],dsp:8[Rd]
	{	0xc9000000,	0xff000000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//81 [05_転送(メモリ間転送)] MOV dsp:8[Rs],dsp:16[Rd]
	{	0xc2000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//82 [05_転送(メモリ間転送)] MOV dsp:16[Rs],[Rd]
	{	0xc6000000,	0xff000000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//83 [05_転送(メモリ間転送)] MOV dsp:16[Rs],dsp:8[Rd]
	{	0xca000000,	0xff000000,	6,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//84 [05_転送(メモリ間転送)] MOV dsp:16[Rs],dsp:16[Rd]
	{	0xd0000000,	0xff000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//85 [05_転送(メモリ間転送)] MOV [Rs],[Rd]
	{	0xd4000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//86 [05_転送(メモリ間転送)] MOV [Rs],dsp:8[Rd]
	{	0xd8000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//87 [05_転送(メモリ間転送)] MOV [Rs],dsp:16[Rd]
	{	0xd1000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//88 [05_転送(メモリ間転送)] MOV dsp:8[Rs],[Rd]
	{	0xd5000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//89 [05_転送(メモリ間転送)] MOV dsp:8[Rs],dsp:8[Rd]
	{	0xd9000000,	0xff000000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//90 [05_転送(メモリ間転送)] MOV dsp:8[Rs],dsp:16[Rd]
	{	0xd2000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//91 [05_転送(メモリ間転送)] MOV dsp:16[Rs],[Rd]
	{	0xd6000000,	0xff000000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//92 [05_転送(メモリ間転送)] MOV dsp:16[Rs],dsp:8[Rd]
	{	0xda000000,	0xff000000,	6,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//93 [05_転送(メモリ間転送)] MOV dsp:16[Rs],dsp:16[Rd]
	{	0xe0000000,	0xff000000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//94 [05_転送(メモリ間転送)] MOV [Rs],[Rd]
	{	0xe4000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//95 [05_転送(メモリ間転送)] MOV [Rs],dsp:8[Rd]
	{	0xe8000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//96 [05_転送(メモリ間転送)] MOV [Rs],dsp:16[Rd]
	{	0xe1000000,	0xff000000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//97 [05_転送(メモリ間転送)] MOV dsp:8[Rs],[Rd]
	{	0xe5000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//98 [05_転送(メモリ間転送)] MOV dsp:8[Rs],dsp:8[Rd]
	{	0xe9000000,	0xff000000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//99 [05_転送(メモリ間転送)] MOV dsp:8[Rs],dsp:16[Rd]
	{	0xe2000000,	0xff000000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//100 [05_転送(メモリ間転送)] MOV dsp:16[Rs],[Rd]
	{	0xe6000000,	0xff000000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//101 [05_転送(メモリ間転送)] MOV dsp:16[Rs],dsp:8[Rd]
	{	0xea000000,	0xff000000,	6,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//102 [05_転送(メモリ間転送)] MOV dsp:16[Rs],dsp:16[Rd]
	{	0xfd200000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE | TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//103 [06_転送1(レジスタ更新ストア)] MOV Rs,[Rd+]
	{	0xfd210000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE | TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//104 [06_転送1(レジスタ更新ストア)] MOV Rs,[Rd+]
	{	0xfd220000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE | TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//105 [06_転送1(レジスタ更新ストア)] MOV Rs,[Rd+]
	{	0xfd240000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE | TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//106 [06_転送1(レジスタ更新ストア)] MOV Rs,[-Rd]
	{	0xfd250000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE | TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//107 [06_転送1(レジスタ更新ストア)] MOV Rs,[-Rd]
	{	0xfd260000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE | TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//108 [06_転送1(レジスタ更新ストア)] MOV Rs,[-Rd]
	{	0xfd280000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//109 [06_転送1(レジスタ更新ロード)] MOV [Rs+],Rd
	{	0xfd290000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//110 [06_転送1(レジスタ更新ロード)] MOV [Rs+],Rd
	{	0xfd2a0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//111 [06_転送1(レジスタ更新ロード)] MOV [Rs+],Rd
	{	0xfd380000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//112 [06_転送1(レジスタ更新ロード)] MOVU [Rs+],Rd
	{	0xfd390000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//113 [06_転送1(レジスタ更新ロード)] MOVU [Rs+],Rd
	{	0xfd2c0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//114 [06_転送1(レジスタ更新ロード)] MOV [-Rs],Rd
	{	0xfd2d0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//115 [06_転送1(レジスタ更新ロード)] MOV [-Rs],Rd
	{	0xfd2e0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//116 [06_転送1(レジスタ更新ロード)] MOV [-Rs],Rd
	{	0xfd3c0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//117 [06_転送1(レジスタ更新ロード)] MOVU [-Rs],Rd
	{	0xfd3d0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//118 [06_転送1(レジスタ更新ロード)] MOVU [-Rs],Rd
	{	0x6f000000,	0xff000000,	2,	0,	0,	0,	0,	0,	},							//119 [07_転送2(POP)] POPM Rd-Rd2
	{	0x6e000000,	0xff000000,	2,	0,	0,	0,	0,	0,	},							//120 [08_転送2(PUSH)] PUSHM　 Rs-Rs2
	{	0x7eb00000,	0xfff00000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//121 [07_転送2(POP)] POP Rd
	{	0x7e800000,	0xfff00000,	2,	0,	0,	0,	0,	0,	},							//122 [08_転送2(PUSH)] PUSH Rs
	{	0xf4080000,	0xff0f0000,	2,	0,	0,	0,	0,	0,	},							//123 [08_転送2(PUSH)] PUSH [Rs]
	{	0xf5080000,	0xff0f0000,	3,	0,	0,	0,	0,	0,	},							//124 [08_転送2(PUSH)] PUSH dsp:8[Rs]
	{	0xf6080000,	0xff0f0000,	4,	0,	0,	0,	0,	0,	},							//125 [08_転送2(PUSH)] PUSH dsp:16[Rs]
	{	0x7e900000,	0xfff00000,	2,	0,	0,	0,	0,	0,	},							//126 [08_転送2(PUSH)] PUSH Rs
	{	0xf4090000,	0xff0f0000,	2,	0,	0,	0,	0,	0,	},							//127 [08_転送2(PUSH)] PUSH [Rs]
	{	0xf5090000,	0xff0f0000,	3,	0,	0,	0,	0,	0,	},							//128 [08_転送2(PUSH)] PUSH dsp:8[Rs]
	{	0xf6090000,	0xff0f0000,	4,	0,	0,	0,	0,	0,	},							//129 [08_転送2(PUSH)] PUSH dsp:16[Rs]
	{	0x7ea00000,	0xfff00000,	2,	0,	0,	0,	0,	0,	},							//130 [08_転送2(PUSH)] PUSH Rs
	{	0xf40a0000,	0xff0f0000,	2,	0,	0,	0,	0,	0,	},							//131 [08_転送2(PUSH)] PUSH [Rs]
	{	0xf50a0000,	0xff0f0000,	3,	0,	0,	0,	0,	0,	},							//132 [08_転送2(PUSH)] PUSH dsp:8[Rs]
	{	0xf60a0000,	0xff0f0000,	4,	0,	0,	0,	0,	0,	},							//133 [08_転送2(PUSH)] PUSH dsp:16[Rs]
	{	0xfc430000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//134 [10_転送4(交換)] XCHG Rs,Rd
	{	0xfc400000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//135 [10_転送4(交換)] XCHG [Rs].UB,Rd
	{	0xfc410000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//136 [10_転送4(交換)] XCHG dsp:8[Rs].UB,Rd
	{	0xfc420000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//137 [10_転送4(交換)] XCHG dsp:16[Rs].UB,Rd
	{	0x06201000,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//138 [10_転送4(交換)] XCHG [Rs].memex,Rd
	{	0x06211000,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//139 [10_転送4(交換)] XCHG dsp:8[Rs].memex,Rd
	{	0x06221000,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//140 [10_転送4(交換)] XCHG dsp:16[Rs].memex,Rd
	{	0xfd74e000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//141 [10_転送5(条件付即値設定)] STZ #simm:8,Rd
	{	0xfd78e000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//142 [10_転送5(条件付即値設定)] STZ #simm:16,Rd
	{	0xfd7ce000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//143 [10_転送5(条件付即値設定)] STZ #simm:24,Rd
	{	0xfd70e000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//144 [10_転送5(条件付即値設定)] STZ #imm:32,Rd
	{	0xfd74f000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//145 [10_転送5(条件付即値設定)] STNZ #simm:8,Rd
	{	0xfd78f000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//146 [10_転送5(条件付即値設定)] STNZ #simm:16,Rd
	{	0xfd7cf000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//147 [10_転送5(条件付即値設定)] STNZ #simm:24,Rd
	{	0xfd70f000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//148 [10_転送5(条件付即値設定)] STNZ #imm:32,Rd
	{	0x67000000,	0xff000000,	2,	TYPE_BIT_BRANCH_SUB2,	0,	0,	0,	0,	},		//149 [10_スタックフレーム開放&サブルーチン復帰] RTSD #uimm:8
	{	0x3f000000,	0xff000000,	3,	TYPE_BIT_BRANCH_SUB2 | TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//150 [10_スタックフレーム開放&サブルーチン復帰] RTSD #uimm:8,Rd,Rd2
	{	0x64000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//151 [11_論理演算命令] AND #uimm:4,Rd
	{	0x75200000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//152 [11_論理演算命令] AND #simm:8,Rd
	{	0x76200000,	0xfff00000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//153 [11_論理演算命令] AND #simm:16,Rd
	{	0x77200000,	0xfff00000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//154 [11_論理演算命令] AND #simm:24,Rd
	{	0x74200000,	0xfff00000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//155 [11_論理演算命令] AND #imm:32,Rd
	{	0x53000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//156 [11_論理演算命令] AND Rs,Rd
	{	0x50000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//157 [11_論理演算命令] AND [Rs].UB,Rd
	{	0x51000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//158 [11_論理演算命令] AND dsp:8[Rs].UB,Rd
	{	0x52000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//159 [11_論理演算命令] AND dsp:16[Rs].UB,Rd
	{	0x06100000,	0xff3f0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//160 [11_論理演算命令] AND [Rs].memex,Rd
	{	0x06110000,	0xff3f0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//161 [11_論理演算命令] AND dsp:8[Rs].memex,Rd
	{	0x06120000,	0xff3f0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//162 [11_論理演算命令] AND dsp:16[Rs].memex,Rd
	{	0xff400000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//163 [11_論理演算命令] AND Rs,Rs2,Rd
	{	0x65000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//164 [11_論理演算命令] OR #uimm:4,Rd
	{	0x75300000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//165 [11_論理演算命令] OR #simm:8,Rd
	{	0x76300000,	0xfff00000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//166 [11_論理演算命令] OR #simm:16,Rd
	{	0x77300000,	0xfff00000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//167 [11_論理演算命令] OR #simm:24,Rd
	{	0x74300000,	0xfff00000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//168 [11_論理演算命令] OR #imm:32,Rd
	{	0x57000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//169 [11_論理演算命令] OR Rs,Rd
	{	0x54000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//170 [11_論理演算命令] OR [Rs].UB,Rd
	{	0x55000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//171 [11_論理演算命令] OR dsp:8[Rs].UB,Rd
	{	0x56000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//172 [11_論理演算命令] OR dsp:16[Rs].UB,Rd
	{	0x06140000,	0xff3f0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//173 [11_論理演算命令] OR [Rs].memex,Rd
	{	0x06150000,	0xff3f0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//174 [11_論理演算命令] OR dsp:8[Rs].memex,Rd
	{	0x06160000,	0xff3f0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//175 [11_論理演算命令] OR dsp:16[Rs].memex,Rd
	{	0xff500000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//176 [11_論理演算命令] OR Rs,Rs2,Rd
	{	0xfd74d000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//177 [11_論理演算命令] XOR #simm:8,Rd
	{	0xfd78d000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//178 [11_論理演算命令] XOR #simm:16,Rd
	{	0xfd7cd000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//179 [11_論理演算命令] XOR #simm:24,Rd
	{	0xfd70d000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//180 [11_論理演算命令] XOR #imm:32,Rd
	{	0xfc370000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//181 [11_論理演算命令] XOR Rs,Rd
	{	0xfc340000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//182 [11_論理演算命令] XOR [Rs].UB,Rd
	{	0xfc350000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//183 [11_論理演算命令] XOR dsp:8[Rs].UB,Rd
	{	0xfc360000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//184 [11_論理演算命令] XOR dsp:16[Rs].UB,Rd
	{	0x06200d00,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//185 [11_論理演算命令] XOR [Rs].memex,Rd
	{	0x06210d00,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//186 [11_論理演算命令] XOR dsp:8[Rs].memex,Rd
	{	0x06220d00,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//187 [11_論理演算命令] XOR dsp:16[Rs].memex,Rd
	{	0x7e000000,	0xfff00000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//188 [11_論理演算命令] NOT Rd
	{	0xfc3b0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//189 [11_論理演算命令] NOT Rs,Rd
	{	0xfd74c000,	0xfffff000,	4,	0,	0,	0,	0,	0,	},							//190 [11_論理演算命令] TST #simm:8,Rs
	{	0xfd78c000,	0xfffff000,	5,	0,	0,	0,	0,	0,	},							//191 [11_論理演算命令] TST #simm:16,Rs
	{	0xfd7cc000,	0xfffff000,	6,	0,	0,	0,	0,	0,	},							//192 [11_論理演算命令] TST #simm:24,Rs
	{	0xfd70c000,	0xfffff000,	7,	0,	0,	0,	0,	0,	},							//193 [11_論理演算命令] TST #imm:32,Rs
	{	0xfc330000,	0xffff0000,	3,	0,	0,	0,	0,	0,	},							//194 [11_論理演算命令] TST Rs,Rs2
	{	0xfc300000,	0xffff0000,	3,	0,	0,	0,	0,	0,	},							//195 [11_論理演算命令] TST [Rs].UB,Rs2
	{	0xfc310000,	0xffff0000,	4,	0,	0,	0,	0,	0,	},							//196 [11_論理演算命令] TST dsp:8[Rs].UB,Rs2
	{	0xfc320000,	0xffff0000,	5,	0,	0,	0,	0,	0,	},							//197 [11_論理演算命令] TST dsp:16[Rs].UB,Rs2
	{	0x06200c00,	0xff3fff00,	4,	0,	0,	0,	0,	0,	},							//198 [11_論理演算命令] TST [Rs].memex,Rs2
	{	0x06210c00,	0xff3fff00,	5,	0,	0,	0,	0,	0,	},							//199 [11_論理演算命令] TST dsp:8[Rs].memex,Rs2
	{	0x06220c00,	0xff3fff00,	6,	0,	0,	0,	0,	0,	},							//200 [11_論理演算命令] TST dsp:16[Rs].memex,Rs2
	{	0x7e100000,	0xfff00000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//201 [12_算術演算命令] NEG Rd
	{	0xfc070000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//202 [12_算術演算命令] NEG Rs,Rd
	{	0xfd742000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//203 [12_算術演算命令] ADC #simm:8,Rd
	{	0xfd782000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//204 [12_算術演算命令] ADC #simm:16,Rd
	{	0xfd7c2000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//205 [12_算術演算命令] ADC #simm:24,Rd
	{	0xfd702000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//206 [12_算術演算命令] ADC #imm:32,Rd
	{	0xfc0b0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//207 [12_算術演算命令] ADC Rs,Rd
	{	0x06200200,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//208 [12_算術演算命令] ADC [Rs].L,Rd
	{	0x06210200,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//209 [12_算術演算命令] ADC dsp:8[Rs].L,Rd
	{	0x06220200,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//210 [12_算術演算命令] ADC dsp:16[Rs].L,Rd
	{	0x62000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//211 [12_算術演算命令] ADD #uimm:4,Rd
/*		{	【ERROR: ビットパターンが不正】,	0x00000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//212 [12_算術演算命令] ADD #simm:8,Rd	*/
/*		{	【ERROR: ビットパターンが不正】,	0x00000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//213 [12_算術演算命令] ADD #simm:16,Rd	*/
/*		{	【ERROR: ビットパターンが不正】,	0x00000000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//214 [12_算術演算命令] ADD #simm:24,Rd	*/
/*		{	【ERROR: ビットパターンが不正】,	0x00000000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//215 [12_算術演算命令] ADD #imm:32,Rd	*/
	{	0x4b000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//216 [12_算術演算命令] ADD Rs,Rd
	{	0x48000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//217 [12_算術演算命令] ADD [Rs].UB,Rd
	{	0x49000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//218 [12_算術演算命令] ADD dsp:8[Rs].UB,Rd
	{	0x4a000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//219 [12_算術演算命令] ADD dsp:16[Rs].UB,Rd
	{	0x06080000,	0xff3f0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//220 [12_算術演算命令] ADD [Rs].memex,Rd
	{	0x06090000,	0xff3f0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//221 [12_算術演算命令] ADD dsp:8[Rs].memex,Rd
	{	0x060a0000,	0xff3f0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//222 [12_算術演算命令] ADD dsp:16[Rs].memex,Rd
	{	0x71000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//223 [12_算術演算命令] ADD #simm:8,Rs,Rd
	{	0x72000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//224 [12_算術演算命令] ADD #simm:16,Rs,Rd
	{	0x73000000,	0xff000000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//225 [12_算術演算命令] ADD #simm:24,Rs,Rd
	{	0x70000000,	0xff000000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//226 [12_算術演算命令] ADD #imm:32,Rs,Rd
	{	0xff200000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//227 [12_算術演算命令] ADD Rs,Rs2,Rd
	{	0x61000000,	0xff000000,	2,	0,	0,	0,	0,	0,	},							//228 [12_算術演算命令] CMP #uimm:4,Rs
	{	0x75500000,	0xfff00000,	3,	0,	0,	0,	0,	0,	},							//229 [12_算術演算命令] CMP #uimm:u8,Rs
	{	0x75000000,	0xfff00000,	3,	0,	0,	0,	0,	0,	},							//230 [12_算術演算命令] CMP #simm:8,Rs
	{	0x76000000,	0xfff00000,	4,	0,	0,	0,	0,	0,	},							//231 [12_算術演算命令] CMP #simm:16,Rs
	{	0x77000000,	0xfff00000,	5,	0,	0,	0,	0,	0,	},							//232 [12_算術演算命令] CMP #simm:24,Rs
	{	0x74000000,	0xfff00000,	6,	0,	0,	0,	0,	0,	},							//233 [12_算術演算命令] CMP #imm:32,Rs
	{	0x47000000,	0xff000000,	2,	0,	0,	0,	0,	0,	},							//234 [12_算術演算命令] CMP Rs,Rs2
	{	0x44000000,	0xff000000,	2,	0,	0,	0,	0,	0,	},							//235 [12_算術演算命令] CMP [Rs].UB,Rs2
	{	0x45000000,	0xff000000,	3,	0,	0,	0,	0,	0,	},							//236 [12_算術演算命令] CMP dsp:8[Rs].UB,Rs2
	{	0x46000000,	0xff000000,	4,	0,	0,	0,	0,	0,	},							//237 [12_算術演算命令] CMP dsp:16[Rs].UB,Rs2
	{	0x06040000,	0xff3f0000,	3,	0,	0,	0,	0,	0,	},							//238 [12_算術演算命令] CMP [Rs].memex,Rs2
	{	0x06050000,	0xff3f0000,	4,	0,	0,	0,	0,	0,	},							//239 [12_算術演算命令] CMP dsp:8[Rs].memex,Rs2
	{	0x06060000,	0xff3f0000,	5,	0,	0,	0,	0,	0,	},							//240 [12_算術演算命令] CMP dsp:16[Rs].memex,Rs2
	{	0x60000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//241 [12_算術演算命令] SUB #uimm:4,Rd
	{	0x43000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//242 [12_算術演算命令] SUB Rs,Rd
	{	0x40000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//243 [12_算術演算命令] SUB [Rs].UB,Rd
	{	0x41000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//244 [12_算術演算命令] SUB dsp:8[Rs].UB,Rd
	{	0x42000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//245 [12_算術演算命令] SUB dsp:16[Rs].UB,Rd
	{	0x06000000,	0xff3f0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//246 [12_算術演算命令] SUB [Rs].memex,Rd
	{	0x06010000,	0xff3f0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//247 [12_算術演算命令] SUB dsp:8[Rs].memex,Rd
	{	0x06020000,	0xff3f0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//248 [12_算術演算命令] SUB dsp:16[Rs].memex,Rd
	{	0xff000000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//249 [12_算術演算命令] SUB Rs,Rs2,Rd
	{	0xfc030000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//250 [12_算術演算命令] SBB Rs,Rd
	{	0x06200000,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//251 [12_算術演算命令] SBB [Rs].L,Rd
	{	0x06210000,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//252 [12_算術演算命令] SBB dsp:8[Rs].L,Rd
	{	0x06220000,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//253 [12_算術演算命令] SBB dsp:16[Rs].L,Rd
	{	0x7e200000,	0xfff00000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//254 [12_算術演算命令] ABS Rd
	{	0xfc0f0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//255 [12_算術演算命令] ABS Rs,Rd
	{	0xfd744000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//256 [12_算術演算命令] MAX #simm:8,Rd
	{	0xfd784000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//257 [12_算術演算命令] MAX #simm:16,Rd
	{	0xfd7c4000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//258 [12_算術演算命令] MAX #simm:24,Rd
	{	0xfd704000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//259 [12_算術演算命令] MAX #imm:32,Rd
	{	0xfc130000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//260 [12_算術演算命令] MAX Rs,Rd
	{	0xfc100000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//261 [12_算術演算命令] MAX [Rs].UB,Rd
	{	0xfc110000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//262 [12_算術演算命令] MAX dsp:8[Rs].UB,Rd
	{	0xfc120000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//263 [12_算術演算命令] MAX dsp:16[Rs].UB,Rd
	{	0x06200400,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//264 [12_算術演算命令] MAX [Rs].memex,Rd
	{	0x06210400,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//265 [12_算術演算命令] MAX dsp:8[Rs].memex,Rd
	{	0x06220400,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//266 [12_算術演算命令] MAX dsp:16[Rs].memex,Rd
	{	0xfd745000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//267 [12_算術演算命令] MIN #simm:8,Rd
	{	0xfd785000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//268 [12_算術演算命令] MIN #simm:16,Rd
	{	0xfd7c5000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//269 [12_算術演算命令] MIN #simm:24,Rd
	{	0xfd705000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//270 [12_算術演算命令] MIN #imm:32,Rd
	{	0xfc170000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//271 [12_算術演算命令] MIN Rs,Rd
	{	0xfc140000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//272 [12_算術演算命令] MIN [Rs].UB,Rd
	{	0xfc150000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//273 [12_算術演算命令] MIN dsp:8[Rs].UB,Rd
	{	0xfc160000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//274 [12_算術演算命令] MIN dsp:16[Rs].UB,Rd
	{	0x06200500,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//275 [12_算術演算命令] MIN [Rs].memex,Rd
	{	0x06210500,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//276 [12_算術演算命令] MIN dsp:8[Rs].memex,Rd
	{	0x06220500,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//277 [12_算術演算命令] MIN dsp:16[Rs].memex,Rd
	{	0x63000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//278 [13_乗除算命令] MUL #uimm:4,Rd
	{	0x75100000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//279 [13_乗除算命令] MUL #simm:8,Rd
	{	0x76100000,	0xfff00000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//280 [13_乗除算命令] MUL #simm:16,Rd
	{	0x77100000,	0xfff00000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//281 [13_乗除算命令] MUL #simm:24,Rd
	{	0x74100000,	0xfff00000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//282 [13_乗除算命令] MUL #imm:32,Rd
	{	0x4f000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//283 [13_乗除算命令] MUL Rs,Rd
	{	0x4c000000,	0xff000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//284 [13_乗除算命令] MUL [Rs].UB,Rd
	{	0x4d000000,	0xff000000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//285 [13_乗除算命令] MUL dsp:8[Rs].UB,Rd
	{	0x4e000000,	0xff000000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//286 [13_乗除算命令] MUL dsp:16[Rs].UB,Rd
	{	0x060c0000,	0xff3f0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//287 [13_乗除算命令] MUL [Rs].memex,Rd
	{	0x060d0000,	0xff3f0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//288 [13_乗除算命令] MUL dsp:8[Rs].memex,Rd
	{	0x060e0000,	0xff3f0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//289 [13_乗除算命令] MUL dsp:16[Rs].memex,Rd
	{	0xff300000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//290 [13_乗除算命令] MUL Rs,Rs2,Rd
	{	0xfd746000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//291 [13_乗除算命令] EMUL #simm:8,Rd
	{	0xfd786000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//292 [13_乗除算命令] EMUL #simm:16,Rd
	{	0xfd7c6000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//293 [13_乗除算命令] EMUL #simm:24,Rd
	{	0xfd706000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//294 [13_乗除算命令] EMUL #imm:32,Rd
	{	0xfc1b0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//295 [13_乗除算命令] EMUL Rs,Rd
	{	0xfc180000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//296 [13_乗除算命令] EMUL [Rs].UB,Rd
	{	0xfc190000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//297 [13_乗除算命令] EMUL dsp:8[Rs].UB,Rd
	{	0xfc1a0000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//298 [13_乗除算命令] EMUL dsp:16[Rs].UB,Rd
	{	0x06200600,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//299 [13_乗除算命令] EMUL [Rs].memex,Rd
	{	0x06210600,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//300 [13_乗除算命令] EMUL dsp:8[Rs].memex,Rd
	{	0x06220600,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//301 [13_乗除算命令] EMUL dsp:16[Rs].memex,Rd
	{	0xfd747000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//302 [13_乗除算命令] EMULU #simm:8,Rd
	{	0xfd787000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//303 [13_乗除算命令] EMULU #simm:16,Rd
	{	0xfd7c7000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//304 [13_乗除算命令] EMULU #simm:24,Rd
	{	0xfd707000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//305 [13_乗除算命令] EMULU #imm:32,Rd
	{	0xfc1f0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//306 [13_乗除算命令] EMULU Rs,Rd
	{	0xfc1c0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//307 [13_乗除算命令] EMULU [Rs].UB,Rd
	{	0xfc1d0000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//308 [13_乗除算命令] EMULU dsp:8[Rs].UB,Rd
	{	0xfc1e0000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//309 [13_乗除算命令] EMULU dsp:16[Rs].UB,Rd
	{	0x06200700,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//310 [13_乗除算命令] EMULU [Rs].memex,Rd
	{	0x06210700,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//311 [13_乗除算命令] EMULU dsp:8[Rs].memex,Rd
	{	0x06220700,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//312 [13_乗除算命令] EMULU dsp:16[Rs].memex,Rd
	{	0xfd748000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//313 [13_乗除算命令] DIV #simm:8,Rd
	{	0xfd788000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//314 [13_乗除算命令] DIV #simm:16,Rd
	{	0xfd7c8000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//315 [13_乗除算命令] DIV #simm:24,Rd
	{	0xfd708000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//316 [13_乗除算命令] DIV #imm:32,Rd
	{	0xfc230000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//317 [13_乗除算命令] DIV Rs,Rd
	{	0xfc200000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//318 [13_乗除算命令] DIV [Rs].UB,Rd
	{	0xfc210000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//319 [13_乗除算命令] DIV dsp:8[Rs].UB,Rd
	{	0xfc220000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//320 [13_乗除算命令] DIV dsp:16[Rs].UB,Rd
	{	0x06200800,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//321 [13_乗除算命令] DIV [Rs].memex,Rd
	{	0x06210800,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//322 [13_乗除算命令] DIV dsp:8[Rs].memex,Rd
	{	0x06220800,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//323 [13_乗除算命令] DIV dsp:16[Rs].memex,Rd
	{	0xfd749000,	0xfffff000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//324 [13_乗除算命令] DIVU #simm:8,Rd
	{	0xfd789000,	0xfffff000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//325 [13_乗除算命令] DIVU #simm:16,Rd
	{	0xfd7c9000,	0xfffff000,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//326 [13_乗除算命令] DIVU #simm:24,Rd
	{	0xfd709000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//327 [13_乗除算命令] DIVU #imm:32,Rd
	{	0xfc270000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//328 [13_乗除算命令] DIVU Rs,Rd
	{	0xfc240000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//329 [13_乗除算命令] DIVU [Rs].UB,Rd
	{	0xfc250000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//330 [13_乗除算命令] DIVU dsp:8[Rs].UB,Rd
	{	0xfc260000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//331 [13_乗除算命令] DIVU dsp:16[Rs].UB,Rd
	{	0x06200900,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//332 [13_乗除算命令] DIVU [Rs].memex,Rd
	{	0x06210900,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//333 [13_乗除算命令] DIVU dsp:8[Rs].memex,Rd
	{	0x06220900,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//334 [13_乗除算命令] DIVU dsp:16[Rs].memex,Rd
	{	0x6c000000,	0xfe000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//335 [13_シフト(論理左)] SHLL #imm:5,Rd
	{	0xfd620000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//336 [13_シフト(論理左)] SHLL Rs,Rd
	{	0xfdc00000,	0xffe00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//337 [13_シフト(論理左)] SHLL #imm:5,Rs,Rd
	{	0x6a000000,	0xfe000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//338 [14_シフト(算術右)] SHAR #imm:5,Rd
	{	0xfd610000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//339 [14_シフト(算術右)] SHAR Rs,Rd
	{	0xfda00000,	0xffe00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//340 [14_シフト(算術右)] SHAR #imm:5,Rs,Rd
	{	0x68000000,	0xfe000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//341 [15_シフト(論理右)] SHLR #imm:5,Rd
	{	0xfd600000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//342 [15_シフト(論理右)] SHLR Rs,Rd
	{	0xfd800000,	0xffe00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//343 [15_シフト(論理右)] SHLR #imm:5,Rs,Rd
	{	0x7e500000,	0xfff00000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//344 [16_ローテート(C付左)] ROLC Rd
	{	0x7e400000,	0xfff00000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//345 [17_ローテート(C付右)] RORC Rd
	{	0xfd6e0000,	0xfffe0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//346 [18_ローテート(左)] ROTL #imm:5,Rd
	{	0xfd660000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//347 [18_ローテート(左)] ROTL Rs,Rd
	{	0xfd6c0000,	0xfffe0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//348 [19_ローテート(右)] ROTR #imm:5,Rd
	{	0xfd640000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//349 [19_ローテート(右)] ROTR Rs,Rd
	{	0xfd650000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//350 [20_リバース] REVW Rs,Rd
	{	0xfd670000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//351 [20_リバース] REVL Rs,Rd
	{	0x10000000,	0xf8000000,	1,	TYPE_BIT_BRANCH_COND,	0x07000000,	5,	24,	0,	},	//352 [20_条件分岐相対] Bcnd pcdsp:3(前方分岐3～10)
	{	0x18000000,	0xf8000000,	1,	TYPE_BIT_BRANCH_COND,	0x07000000,	5,	24,	0,	},	//353 [20_条件分岐相対] Bcnd pcdsp:3(前方分岐3～10)
#define AVOID_BCND_BRA
#ifdef AVOID_BCND_BRA
	{	0x2e000000,	0xff000000,	2,	TYPE_BIT_BRANCH_NOCOND,	0x00ff0000,	8,	16,	0,	},	//358 [21_分岐相対] BRA pcdsp:8
#endif
	{	0x20000000,	0xf0000000,	2,	TYPE_BIT_BRANCH_COND,	0x00ff0000,	8,	16,	0,	},	//354 [20_条件分岐相対] Bcnd pcdsp:8
	{	0x3a000000,	0xff000000,	3,	TYPE_BIT_BRANCH_COND,	0x00ffff00,	8,	8,	0,	},	//355 [20_条件分岐相対] Bcnd pcdsp:16
	{	0x3b000000,	0xff000000,	3,	TYPE_BIT_BRANCH_COND,	0x00ffff00,	8,	8,	0,	},	//356 [20_条件分岐相対] Bcnd pcdsp:16
	{	0x08000000,	0xf8000000,	1,	TYPE_BIT_BRANCH_NOCOND,	0x07000000,	5,	24,	0,	},	//357 [21_分岐相対] BRA pcdsp:3(前方分岐3～10)
#ifndef AVOID_BCND_BRA
	{	0x2e000000,	0xff000000,	2,	TYPE_BIT_BRANCH_NOCOND,	0x00ff0000,	8,	16,	0,	},	//358 [21_分岐相対] BRA pcdsp:8
#endif
	{	0x38000000,	0xff000000,	3,	TYPE_BIT_BRANCH_NOCOND,	0x00ffff00,	8,	8,	0,	},	//359 [21_分岐相対] BRA pcdsp:16
	{	0x04000000,	0xff000000,	4,	TYPE_BIT_BRANCH_NOCOND,	0x00ffffff,	8,	0,	0,	},	//360 [21_分岐相対] BRA pcdsp:24
	{	0x7f400000,	0xfff00000,	2,	TYPE_BIT_BRANCH_NOCOND,	0,	0,	0,	0,	},		//361 [21_分岐相対] BRA Rs
	{	0x7f000000,	0xfff00000,	2,	TYPE_BIT_BRANCH_NOCOND,	0,	0,	0,	0,	},		//362 [22_分岐] JMP Rs
	{	0x7f100000,	0xfff00000,	2,	TYPE_BIT_BRANCH_SUB1,	0,	0,	0,	0,	},		//363 [23_分岐（サブルーチン)] JSR Rs
	{	0x39000000,	0xff000000,	3,	TYPE_BIT_BRANCH_SUB1,	0x00ffff00,	8,	8,	0,	},	//364 [23_分岐相対（サブルーチン)] BSR pcdsp:16
	{	0x05000000,	0xff000000,	4,	TYPE_BIT_BRANCH_SUB1,	0x00ffffff,	8,	0,	0,	},	//365 [23_分岐相対（サブルーチン)] BSR pcdsp:24
	{	0x7f500000,	0xfff00000,	2,	TYPE_BIT_BRANCH_SUB1,	0,	0,	0,	0,	},		//366 [23_分岐相対（サブルーチン)] BSR Rs
	{	0x02000000,	0xff000000,	1,	TYPE_BIT_BRANCH_SUB2,	0,	0,	0,	0,	},		//367 [24_リターン（サブルーチン)] RTS -
	{	0x03000000,	0xff000000,	1,	0,	0,	0,	0,	0,	},							//368 [21_分岐相対] NOP -
	{	0x7f830000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//369 [32_ストリング命令] SCMPU -
	{	0x7f870000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//370 [32_ストリング命令] SMOVU -
	{	0x7f8b0000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//371 [32_ストリング命令] SMOVB -
	{	0x7f800000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//372 [32_ストリング命令] SUNTIL -
	{	0x7f810000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//373 [32_ストリング命令] SUNTIL -
	{	0x7f820000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//374 [32_ストリング命令] SUNTIL -
	{	0x7f840000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//375 [32_ストリング命令] SWHILE -
	{	0x7f850000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//376 [32_ストリング命令] SWHILE -
	{	0x7f860000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//377 [32_ストリング命令] SWHILE -
	{	0x7f8f0000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//378 [32_ストリング命令] SMOVF -
	{	0x7f880000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//379 [32_ストリング命令] SSTR -
	{	0x7f890000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//380 [32_ストリング命令] SSTR -
	{	0x7f8a0000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//381 [32_ストリング命令] SSTR -
	{	0x7f8c0000,	0xffff0000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//382 [12_算術演算命令] RMPA -
	{	0x7f8d0000,	0xffff0000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//383 [12_算術演算命令] RMPA -
	{	0x7f8e0000,	0xffff0000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//384 [12_算術演算命令] RMPA -
	{	0xfd000000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							//385 [34_DSP機能命令] MULHI Rs,Rs2 / MULHI  Rs, Rs2, Ad	// RevRxNo130301-001 Modify Line
	{	0xfd010000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							//386 [34_DSP機能命令] MULLO Rs,Rs2 / MULLO  Rs, Rs2, Ad	// RevRxNo130301-001 Modify Line
	{	0xfd040000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							//387 [34_DSP機能命令] MACHI Rs,Rs2 / MACHI  Rs, Rs2, Ad	// RevRxNo130301-001 Modify Line
	{	0xfd050000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							//388 [34_DSP機能命令] MACLO Rs,Rs2 / MACLO  Rs, Rs2, Ad	// RevRxNo130301-001 Modify Line
	{	0xfd170000,	0xffff7000,	3,	0,	0,	0,	0,	0,	},							//389 [34_DSP機能命令] MVTACHI Rs / MVTACHI  Rs, Ad		// RevRxNo130301-001 Modify Line
	{	0xfd171000,	0xffff7000,	3,	0,	0,	0,	0,	0,	},							//390 [34_DSP機能命令] MVTACLO Rs / MVTACLO  Rs, Ad		// RevRxNo130301-001 Modify Line
	{	0xfd1f0000,	0xfffff000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//391 [34_DSP機能命令] MVFACHI Rd
	{	0xfd1f2000,	0xfffff000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//392 [34_DSP機能命令] MVFACMI Rd
	{	0xfd180000,	0xffff6f00,	3,	0,	0,	0,	0,	0,	},							//393 [34_DSP機能命令] RACW #imm:1 / RACW  #imm1, Ad	// RevRxNo130301-001 Modify Line
	{	0x7e300000,	0xfff00000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//394 [12_算術演算命令] SAT Rd
	{	0x7f930000,	0xffff0000,	2,	0,	0,	0,	0,	0,	},							//395 [12_算術演算命令] SATR -
	{	0xfd722000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//396 [50_浮動小数点演算] FADD #imm:32,Rd
	{	0xfc8b0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//397 [50_浮動小数点演算] FADD Rs,Rd
	{	0xfc880000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//398 [50_浮動小数点演算] FADD [Rs].L,Rd
	{	0xfc890000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//399 [50_浮動小数点演算] FADD dsp:8[Rs].L,Rd
	{	0xfc8a0000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//400 [50_浮動小数点演算] FADD dsp:16[Rs].L,Rd
	{	0xfd721000,	0xfffff000,	7,	0,	0,	0,	0,	0,	},							//401 [50_浮動小数点演算] FCMP #imm:32,Rs2
	{	0xfc870000,	0xffff0000,	3,	0,	0,	0,	0,	0,	},							//402 [50_浮動小数点演算] FCMP Rs,Rs2
	{	0xfc840000,	0xffff0000,	3,	0,	0,	0,	0,	0,	},							//403 [50_浮動小数点演算] FCMP [Rs].L,Rs2
	{	0xfc850000,	0xffff0000,	4,	0,	0,	0,	0,	0,	},							//404 [50_浮動小数点演算] FCMP dsp:8[Rs].L,Rs2
	{	0xfc860000,	0xffff0000,	5,	0,	0,	0,	0,	0,	},							//405 [50_浮動小数点演算] FCMP dsp:16[Rs].L,Rs2
	{	0xfd720000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//406 [50_浮動小数点演算] FSUB #imm:32,Rd
	{	0xfc830000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//407 [50_浮動小数点演算] FSUB Rs,Rd
	{	0xfc800000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//408 [50_浮動小数点演算] FSUB [Rs].L,Rd
	{	0xfc810000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//409 [50_浮動小数点演算] FSUB dsp:8[Rs].L,Rd
	{	0xfc820000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//410 [50_浮動小数点演算] FSUB dsp:16[Rs].L,Rd
	{	0xfc970000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//411 [50_浮動小数点演算] FTOI Rs,Rd
	{	0xfc940000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//412 [50_浮動小数点演算] FTOI [Rs].L,Rd
	{	0xfc950000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//413 [50_浮動小数点演算] FTOI dsp:8[Rs].L,Rd
	{	0xfc960000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//414 [50_浮動小数点演算] FTOI dsp:16[Rs].L,Rd
	{	0xfd723000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//415 [50_浮動小数点演算] FMUL #imm:32,Rd
	{	0xfc8f0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//416 [50_浮動小数点演算] FMUL Rs,Rd
	{	0xfc8c0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//417 [50_浮動小数点演算] FMUL [Rs].L,Rd
	{	0xfc8d0000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//418 [50_浮動小数点演算] FMUL dsp:8[Rs].L,Rd
	{	0xfc8e0000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//419 [50_浮動小数点演算] FMUL dsp:16[Rs].L,Rd
	{	0xfd724000,	0xfffff000,	7,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//420 [50_浮動小数点演算] FDIV #imm:32,Rd
	{	0xfc930000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//421 [50_浮動小数点演算] FDIV Rs,Rd
	{	0xfc900000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//422 [50_浮動小数点演算] FDIV [Rs].L,Rd
	{	0xfc910000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//423 [50_浮動小数点演算] FDIV dsp:8[Rs].L,Rd
	{	0xfc920000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//424 [50_浮動小数点演算] FDIV dsp:16[Rs].L,Rd
	{	0xfc9b0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//425 [50_浮動小数点演算] ROUND Rs,Rd
	{	0xfc980000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//426 [50_浮動小数点演算] ROUND [Rs].L,Rd
	{	0xfc990000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//427 [50_浮動小数点演算] ROUND dsp:8[Rs].L,Rd
	{	0xfc9a0000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//428 [50_浮動小数点演算] ROUND dsp:16[Rs].L,Rd
	{	0xfc470000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//429 [50_浮動小数点演算] ITOF Rs,Rd
	{	0xfc440000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//430 [50_浮動小数点演算] ITOF [Rs].UB,Rd
	{	0xfc450000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//431 [50_浮動小数点演算] ITOF dsp:8[Rs].UB,Rd
	{	0xfc460000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//432 [50_浮動小数点演算] ITOF dsp:16[Rs].UB,Rd
	{	0x06201100,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//433 [50_浮動小数点演算] ITOF [Rs].memex,Rd
	{	0x06211100,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//434 [50_浮動小数点演算] ITOF dsp:8[Rs].memex,Rd
	{	0x06221100,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//435 [50_浮動小数点演算] ITOF dsp:16[Rs].memex,Rd
	{	0xf0000000,	0xff080000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//436 [35_ビット操作] BSET #imm:3,[Rd].B
	{	0xf1000000,	0xff080000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//437 [35_ビット操作] BSET #imm:3,dsp:8[Rd].B
	{	0xf2000000,	0xff080000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//438 [35_ビット操作] BSET #imm:3,dsp:16[Rd].B
	{	0xfc630000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//439 [35_ビット操作] BSET Rs,Rd
	{	0xfc600000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//440 [35_ビット操作] BSET Rs,[Rd].B
	{	0xfc610000,	0xffff0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//441 [35_ビット操作] BSET Rs,dsp:8[Rd].B
	{	0xfc620000,	0xffff0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//442 [35_ビット操作] BSET Rs,dsp:16[Rd].B
	{	0x78000000,	0xfe000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//443 [35_ビット操作] BSET #imm:5,Rd
	{	0xf0080000,	0xff080000,	2,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//444 [35_ビット操作] BCLR #imm:3,[Rd].B
	{	0xf1080000,	0xff080000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//445 [35_ビット操作] BCLR #imm:3,dsp:8[Rd].B
	{	0xf2080000,	0xff080000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//446 [35_ビット操作] BCLR #imm:3,dsp:16[Rd].B
	{	0xfc670000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//447 [35_ビット操作] BCLR Rs,Rd
	{	0xfc640000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//448 [35_ビット操作] BCLR Rs,[Rd].B
	{	0xfc650000,	0xffff0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//449 [35_ビット操作] BCLR Rs,dsp:8[Rd].B
	{	0xfc660000,	0xffff0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//450 [35_ビット操作] BCLR Rs,dsp:16[Rd].B
	{	0x7a000000,	0xfe000000,	2,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//451 [35_ビット操作] BCLR #imm:5,Rd
	{	0xf4000000,	0xff080000,	2,	0,	0,	0,	0,	0,	},							//452 [35_ビット操作] BTST #imm:3,[Rs2].B
	{	0xf5000000,	0xff080000,	3,	0,	0,	0,	0,	0,	},							//453 [35_ビット操作] BTST #imm:3,dsp:8[Rs2].B
	{	0xf6000000,	0xff080000,	4,	0,	0,	0,	0,	0,	},							//454 [35_ビット操作] BTST #imm:3,dsp:16[Rs2].B
	{	0xfc6b0000,	0xffff0000,	3,	0,	0,	0,	0,	0,	},							//455 [35_ビット操作] BTST Rs,Rs2
	{	0xfc680000,	0xffff0000,	3,	0,	0,	0,	0,	0,	},							//456 [35_ビット操作] BTST Rs,[Rs2].B
	{	0xfc690000,	0xffff0000,	4,	0,	0,	0,	0,	0,	},							//457 [35_ビット操作] BTST Rs,dsp:8[Rs2].B
	{	0xfc6a0000,	0xffff0000,	5,	0,	0,	0,	0,	0,	},							//458 [35_ビット操作] BTST Rs,dsp:16[Rs2].B
	{	0x7c000000,	0xfe000000,	2,	0,	0,	0,	0,	0,	},							//459 [35_ビット操作] BTST #imm:5,Rs2
	{	0xfce00f00,	0xffe30f00,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//460 [35_ビット操作] BNOT #imm:3,[Rd].B
	{	0xfce10f00,	0xffe30f00,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//461 [35_ビット操作] BNOT #imm:3,dsp:8[Rd].B
	{	0xfce20f00,	0xffe30f00,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//462 [35_ビット操作] BNOT #imm:3,dsp:16[Rd].B
	{	0xfc6f0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//463 [35_ビット操作] BNOT Rs,Rd
	{	0xfc6c0000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//464 [35_ビット操作] BNOT Rs,[Rd].B
	{	0xfc6d0000,	0xffff0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//465 [35_ビット操作] BNOT Rs,dsp:8[Rd].B
	{	0xfc6e0000,	0xffff0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//466 [35_ビット操作] BNOT Rs,dsp:16[Rd].B
	{	0xfde0f000,	0xffe0f000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//467 [35_ビット操作] BNOT #imm:5,Rd
	{	0xfce00000,	0xffe30000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//468 [36_条件ビット転送] BMcnd #imm:3,[Rd].B
	{	0xfce10000,	0xffe30000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//469 [36_条件ビット転送] BMcnd #imm:3,dsp:8[Rd].B
	{	0xfce20000,	0xffe30000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//470 [36_条件ビット転送] BMcnd #imm:3,dsp:16[Rd].B
	{	0xfde00000,	0xffe00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//471 [36_条件ビット転送] BMcnd #imm:5,Rd
	{	0x7fb00000,	0xfff00000,	2,	TYPE_BIT_SPECIAL_PSW_DIRECT,	0,	0,	0,	0,	},	//472 [40_専用レジスタ操作命令] CLRPSW CBd
	{	0x7fa00000,	0xfff00000,	2,	TYPE_BIT_SPECIAL_PSW_DIRECT,	0,	0,	0,	0,	},	//473 [40_専用レジスタ操作命令] SETPSW CBd
	{	0x7ee00000,	0xffe00000,	2,	0,	0,	0,	0,	0,	},							//474 [40_専用レジスタ操作命令] POPC CRd
	{	0x7ec00000,	0xffe00000,	2,	0,	0,	0,	0,	0,	},							//475 [40_専用レジスタ操作命令] PUSHC CRs
	{	0xfd770000,	0xffffe000,	4,	0,	0,	0,	0,	0,	},							//476 [40_専用レジスタ操作命令] MVTC #simm:8,CRd
	{	0xfd7b0000,	0xffffe000,	5,	0,	0,	0,	0,	0,	},							//477 [40_専用レジスタ操作命令] MVTC #simm:16,CRd
	{	0xfd7f0000,	0xffffe000,	6,	0,	0,	0,	0,	0,	},							//478 [40_専用レジスタ操作命令] MVTC #simm:24,CRd
	{	0xfd730000,	0xffffe000,	7,	0,	0,	0,	0,	0,	},							//479 [40_専用レジスタ操作命令] MVTC #imm:32,CRd
	{	0xfd680000,	0xfffe0000,	3,	0,	0,	0,	0,	0,	},							//480 [40_専用レジスタ操作命令] MVTC Rs,CRd
	{	0xfd6a0000,	0xfffe0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//481 [40_専用レジスタ操作命令] MVFC CRs,Rd
	{	0x75700000,	0xfffff000,	3,	0,	0,	0,	0,	0,	},							//482 [40_専用レジスタ操作命令] MVTIPL #imm:4
	{	0x7f940000,	0xffff0000,	2,	TYPE_BIT_BRANCH_SUB2,	0,	0,	0,	0,	},		//483 [42_割り込み] RTFI -
	{	0x7f950000,	0xffff0000,	2,	TYPE_BIT_BRANCH_SUB2,	0,	0,	0,	0,	},		//484 [42_割り込み] RTE -
	{	0x00000000,	0xff000000,	1,	TYPE_BIT_BRANCH_BRK,	0,	0,	0,	0,	},		//485 [42_割り込み] BRK -
	{	0x01000000,	0xff000000,	1,	TYPE_BIT_BRANCH_BRK,	0,	0,	0,	0,	},		//486 [42_割り込み] DBT -
	{	0x75600000,	0xffff0000,	3,	TYPE_BIT_BRANCH_SUB1,	0,	0,	0,	0,	},		//487 [42_割り込み] INT #imm:8
	{	0x7f960000,	0xffff0000,	2,	TYPE_BIT_SPECIAL_WAIT,	0,	0,	0,	0,	},		//488 [30_その他] WAIT -
	{	0xfcdb0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	//489 [31_条件設定(SCcnd)] SCcnd Rd
	{	0xfcd00000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//490 [31_条件設定(SCcnd)] SCcnd [Rd]
	{	0xfcd10000,	0xffff0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//491 [31_条件設定(SCcnd)] SCcnd dsp:8[Rd]
	{	0xfcd20000,	0xffff0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//492 [31_条件設定(SCcnd)] SCcnd dsp:16[Rd]
	{	0xfcd40000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//493 [31_条件設定(SCcnd)] SCcnd [Rd]
	{	0xfcd50000,	0xffff0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//494 [31_条件設定(SCcnd)] SCcnd dsp:8[Rd]
	{	0xfcd60000,	0xffff0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//495 [31_条件設定(SCcnd)] SCcnd dsp:16[Rd]
	{	0xfcd80000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//496 [31_条件設定(SCcnd)] SCcnd [Rd]
	{	0xfcd90000,	0xffff0000,	4,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//497 [31_条件設定(SCcnd)] SCcnd dsp:8[Rd]
	{	0xfcda0000,	0xffff0000,	5,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		//498 [31_条件設定(SCcnd)] SCcnd dsp:16[Rd]
// RevRxNo130301-001 Append Start
	{	0xfca30000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FSQRT Rs, Rd
	{	0xfca00000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FSQRT [Rs].L, Rd
	{	0xfca10000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FSQRT dsp:8[Rs].L, Rd
	{	0xfca20000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FSQRT dsp:16[Rs].L, Rd
	{	0xfca70000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FTOU Rs, Rd
	{	0xfca40000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FTOU [Rs].L, Rd
	{	0xfca50000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FTOU dsp:8[Rs].L, Rd
	{	0xfca60000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FTOU dsp:16[Rs].L, Rd
	{	0xfc570000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] UTOF Rs, Rd
	{	0xfc540000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] UTOF [Rs].memex, Rd (memex==UB)
	{	0x06201500,	0xff3fff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] UTOF [Rs].memex, Rd (memex!=UB)
	{	0xfc550000,	0xffff0000,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] UTOF dsp:8[Rs].memex, Rd (memex==UB)
	{	0x06211500,	0xff3fff00,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] UTOF dsp:8[Rs].memex, Rd (memex!=UB)
	{	0xfc560000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] UTOF dsp:16[Rs].memex, Rd (memex==UB)
	{	0x06221500,	0xff3fff00,	6,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] UTOF dsp:16[Rs].memex, Rd (memex!=UB)
	{	0xffa00000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FADD Rs, Rs2, Rd
	{	0xff800000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FSUB Rs, Rs2, Rd
	{	0xffb00000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [浮動小数点演算] FMUL Rs, Rs2, Rd
	{	0xfd030000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] EMULA Rs, Rs2, Ad
	{	0xfd070000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] EMACA Rs, Rs2, Ad
	{	0xfd470000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] EMSBA Rs, Rs2, Ad
	{	0xfd020000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] MULLH Rs, Rs2, Ad
	{	0xfd060000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] MACLH Rs, Rs2, Ad
	{	0xfd460000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] MSBLH Rs, Rs2, Ad
	{	0xfd440000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] MSBHI Rs, Rs2, Ad
	{	0xfd450000,	0xfff70000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] MSBLO Rs, Rs2, Ad
	{	0xfd184000,	0xffff6f00,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] RDACW #imm1, Ad
	{	0xfd194000,	0xffff6f00,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] RDACL #imm1, Ad
	{	0xfd190000,	0xffff6f00,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] RACL #imm1, Ad
	{	0xfd1e0000,	0xffff7000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [DSP機能命令] MVFACHI #imm2, As, Rd (imm2=2)
	{	0xfd1f0000,	0xffff3000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [DSP機能命令] MVFACHI #imm2, As, Rd (imm2=0,1)
	{	0xfd1e2000,	0xffff7000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [DSP機能命令] MVFACMI #imm2, As, Rd (imm2=2)
	{	0xfd1f2000,	0xffff3000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [DSP機能命令] MVFACMI #imm2, As, Rd (imm2=0,1)
	{	0xfd1e1000,	0xffff7000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [DSP機能命令] MVFACLO #imm2, As, Rd (imm2=2)
	{	0xfd1f1000,	0xffff3000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [DSP機能命令] MVFACLO #imm2, As, Rd (imm2=0,1)
	{	0xfd1e3000,	0xffff7000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [DSP機能命令] MVFACGU #imm2, As, Rd (imm2=2)
	{	0xfd1f3000,	0xffff3000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [DSP機能命令] MVFACGU #imm2, As, Rd (imm2=0,1)
	{	0xfd173000,	0xffff7000,	3,	0,	0,	0,	0,	0,	},							// [DSP機能命令] MVTACGU Rs, Ad
	{	0xfc4b0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [条件付転送命令] STZ Rs, Rd
	{	0xfc4f0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [条件付転送命令] STNZ Rs, Rd
	{	0xfd2f0000,	0xffff0000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0,	},	// [排他制御命令] MOVLI [Rs], Rd
	{	0xfd270000,	0xffff0000,	3,	TYPE_BIT_MEMORY_UPDATE,	0,	0,	0,	0,	},		// [排他制御命令] MOVCO Rs, [Rd]
// RevRxNo130301-001 Append End
// RevRxNo180228-001 Append Start
	{	0xfc5e0000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0, },	// [転送命令] BFMOV  #IMM:5, #IMM:5, #IMM:5, Rs, Rd
	{	0xfc5a0000,	0xffff0000,	5,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0, },	// [転送命令] BFMOVZ  #IMM:5, #IMM:5, #IMM:5, Rs, Rd
	{	0xfd76f000,	0xffffff00,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0, },	// [転送命令] RSTR  #UIMM:8
	{	0xfd76d000,	0xfffff0ff,	4,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0, },	// [転送命令] RSTR  Rs
	{	0xfd76e000,	0xffffff00,	4,	0,	0,	0,	0,	0, },							// [転送命令] SAVE  #UIMM:8
	{	0xfd76c000,	0xfffff0ff,	4,	0,	0,	0,	0,	0, },							// [転送命令] SAVE  Rs
	{	0xff600000,	0xfff00000,	3,	TYPE_BIT_REGISTER_UPDATE,	0,	0,	0,	0, },	// [算術/論理演算命令] XOR  Rs, Rs2, Rd
// RevRxNo180228-001 Append End
/*		{	【ERROR: L.246と同じ命令パターン】,	0xff000000,	1,	TYPE_BIT_MISC,	0,	0,	0,	0,	},	//499 [32_memex] memex -	*/
	{	0xfc000000,	0xff000000,	1,	TYPE_BIT_MISC,	0,	0,	0,	0,	},				//500 [33_id] id24(1) -
/*		{	【ERROR: L.385と同じ命令パターン】,	0xff000000,	1,	TYPE_BIT_MISC,	0,	0,	0,	0,	},	//501 [33_id] id24(2) -	*/
/*		{	【ERROR: L.28と同じ命令パターン】,	0xff000000,	1,	TYPE_BIT_MISC,	0,	0,	0,	0,	},	//502 [33_id] id24(3) -	*/
/*		{	【ERROR: L.249と同じ命令パターン】,	0xff000000,	1,	TYPE_BIT_MISC,	0,	0,	0,	0,	},	//503 [33_id] id24(4) -	*/
	//*** rx600cmdtbl.plで出力した結果を張り付け：ここまで
	{	0xffffffff,	0x00000000,0,0,0,0, },	//End of data
};


///////////////////////////////////////////////////////////////////////////////
// FFWコマンド処理関数
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
/**
 * 命令解析テーブルのポインタとして返送する。
 * @param なし
 * @retval 命令解析テーブルへのポインタ
 */
//=============================================================================
FFWRX_INST_MST* GetINSTCMDPtr()
{
	return	&tblRX_INST_CMD[0];
}

