///////////////////////////////////////////////////////////////////////////////
/*
 * @file   e2_asp_fpga.h
 * @brief  FPGAのASP関連レジスタ定義
 * @author TSSR M.Ogata, REL K.Sasaki
 * @author Copyright (C) 2016, 2017 Renesas Electronics Corporation and
 * @author Renesas System Design Co., Ltd. All rights reserved.
 * @date   2017/10/26
 */
 ///////////////////////////////////////////////////////////////////////////////

#ifndef __E2_ASP_FPGA_H__
#define __E2_ASP_FPGA_H__

#ifdef __cplusplus
extern int e2_CPU_R(ULONG addr, ULONG len, UCHAR *data);
extern int e2_CPU_W(ULONG addr, ULONG len, UCHAR *data);
#else
extern "C" {
	int e2_CPU_R(ULONG addr, ULONG len, UCHAR *data);
	int e2_CPU_W(ULONG addr, ULONG len, UCHAR *data);
}
#endif


#define BULKIN_FIFO_SIZE    (2048)  /* USBAバルクイン側FIFOサイズ */
#define MAX_SRAM_SIZE		(1024*1024*8)	/* SRAMの最大バイト数 */

/*
 * FPGAの定義ファイル(e2_asp.vh)のレジスタ名をそのまま使用する
 * レジスタのbit名はレジスタ名の後ろにbit名を続ける
 * bit位置を定義する場合は最後を_BITに
 * maskを定義する場合は最後を_MASKにしている
 */

#define ASPCTRL         	(0x07001000)
#define ASPCTRL_RSTASPFUNCALL	(1<<31)
#define ASPCTRL_RSTASPFUNCMSK	(1<<30)

#define ASPCTRL_RSTASPFUNCREG	(1<<27)
#define ASPCTRL_RSTASPIFREGMSK	(1<<26)
#define ASPCTRL_RSTASPIFREG		(1<<23)
#define ASPCTRL_RSTELREG		(1<<22)
#define ASPCTRL_RSTPAREG		(1<<21)
#define ASPCTRL_RSTCMREG		(1<<20)
#define ASPCTRL_RSTMEMREG		(1<<19)
#define ASPCTRL_RSTTIMERREG		(1<<18)

#define ASPCTRL_RSTASPFUNC		(1<<11)
#define ASPCTRL_RSTASPIFMSK		(1<<10)
#define ASPCTRL_RSTASPIF		(1<<7)
#define ASPCTRL_RSTEL			(1<<6)
#define ASPCTRL_RSTPA			(1<<5)
#define ASPCTRL_RSTCM			(1<<4)
#define ASPCTRL_RSTMEM			(1<<3)
#define ASPCTRL_RSTTIMER		(1<<2)
#define	ASPCTRL_ASPRUN			(1<<0)
#define ASPCTRL_INIT       	(0x00000F00)

#define ASPP              	(0x07001010)

#define ASPPOE            	(0x07001014)

#define ASPPFSEL0         	(0x07001018)
#define ASPPFSEL0_ASP_D7_SEL_BIT	(28)
#define ASPPFSEL0_ASP_D6_SEL_BIT	(24)
#define ASPPFSEL0_ASP_D5_SEL_BIT	(20)
#define ASPPFSEL0_ASP_D4_SEL_BIT	(16)
#define ASPPFSEL0_ASP_D3_SEL_BIT	(12)
#define ASPPFSEL0_ASP_D2_SEL_BIT	(8)
#define ASPPFSEL0_ASP_D1_SEL_BIT	(4)
#define ASPPFSEL0_ASP_D0_SEL_BIT	(0)
#define ASPPFSEL0_ASP_UART			(3)
#define ASPPFSEL0_ASP_SPI			(4)
#define ASPPFSEL0_ASP_I2C			(5)
#define ASPPFSEL0_ASP_CAN			(6)
#define ASPPFSEL0_ASP_MASK			(0x0000FFFF)

#define ASPPFSEL1         	(0x0700101C)
#define ASPPFSEL1_ASP_D11_SEL_BIT	(12)
#define ASPPFSEL1_ASP_D10_SEL_BIT	(8)
#define ASPPFSEL1_ASP_D9_SEL_BIT	(4)
#define ASPPFSEL1_ASP_D8_SEL_BIT	(0)

#define ASPP_EVLCTRL       	(0x07001030)
#define ASPP_EVLCTRL_ACTEN	(1<<1)
#define ASPP_EVLCTRL_FACTEN	(1<<0)

#define ASPP_EVLFACTFUNC   	(0x07001034)
#define ASPP_EVLFACTFUNC_TRGI1_ACTFUNC_BIT	(4)
#define ASPP_EVLFACTFUNC_TRGI0_ACTFUNC_BIT	(0)

#define ASPP_EVLACTFUNC0    	(0x07001038)
#define ASPP_EVLACTFUNC1    	(0x0700103C)
#define ASPP_EVLACTFUNC_TRGOWCNT_BIT		(16)
#define ASPP_EVLACTFUNC_LACTIVE				(0)
#define ASPP_EVLACTFUNC_HACTIVE				(1<<0)
#define ASPP_EVLACTFUNC_PULSE				(1<<1)

//ASP_MEM
#define MEM_CTRL       		(0x07001100)
#define MEM_CTRL_MEMBUSMODE_BIT		(16)
#define MEM_CTRL_MEMBUS_PC			(0<<MEM_CTRL_MEMBUSMODE_BIT)
#define MEM_CTRL_MEMBUS_E2			(1<<MEM_CTRL_MEMBUSMODE_BIT)
#define MEM_CTRL_RDMEMEN			(1<<0)


// MEM_SFIFO
#define SFIFO_CTRL        	(0x07001120)
#define SFIFO_CTRL_SFIFO5STOP	(1<<20)
#define SFIFO_CTRL_SFIFO4STOP	(1<<19)
#define SFIFO_CTRL_SFIFO3STOP	(1<<18)
#define SFIFO_CTRL_SFIFO2STOP	(1<<17)
#define SFIFO_CTRL_SFIFO1STOP	(1<<16)
#define SFIFO_CTRL_STOPMASK		(0x001F0000)
#define SFIFO_CTRL_SFIFO5EN		(1<<4)
#define SFIFO_CTRL_SFIFO4EN		(1<<3)
#define SFIFO_CTRL_SFIFO3EN		(1<<2)
#define SFIFO_CTRL_SFIFO2EN		(1<<1)
#define SFIFO_CTRL_SFIFO1EN		(1<<0)
#define SFIFO_CTRL_ENMASK		(0x0000001F)

#define SFIFO_STAT        	(0x07001130)
#define SFIFO_STAT_SFIFO5PROGFULL	(1<<28)
#define SFIFO_STAT_SFIFO4PROGFULL	(1<<27)
#define SFIFO_STAT_SFIFO3PROGFULL	(1<<26)
#define SFIFO_STAT_SFIFO2PROGFULL	(1<<25)
#define SFIFO_STAT_SFIFO1PROGFULL	(1<<24)
#define SFIFO_STAT_SFIFO5PROGEMP	(1<<20)
#define SFIFO_STAT_SFIFO4PROGEMP	(1<<19)
#define SFIFO_STAT_SFIFO3PROGEMP	(1<<18)
#define SFIFO_STAT_SFIFO2PROGEMP	(1<<17)
#define SFIFO_STAT_SFIFO1PROGEMP	(1<<16)
#define SFIFO_STAT_SFIFO5FULL		(1<<12)
#define SFIFO_STAT_SFIFO4FULL		(1<<11)
#define SFIFO_STAT_SFIFO3FULL		(1<<10)
#define SFIFO_STAT_SFIFO2FULL		(1<<9)
#define SFIFO_STAT_SFIFO1FULL		(1<<8)
#define SFIFO_STAT_SFIFO5EMP		(1<<4)
#define SFIFO_STAT_SFIFO4EMP		(1<<3)
#define SFIFO_STAT_SFIFO3EMP		(1<<2)
#define SFIFO_STAT_SFIFO2EMP		(1<<1)
#define SFIFO_STAT_SFIFO1EMP		(1<<0)
#define SFIFO_STAT_EMPMASK			(0x0000001F)

#define SFIFO1_PROG     (0x07001140)
#define SFIFO2_PROG    	(0x07001144)
#define SFIFO3_PROG     (0x07001148)
#define SFIFO4_PROG     (0x0700114C)
#define SFIFO5_PROG     (0x07001150)

#define ASPFIFO_RST     (0x07001170)
#define ASPFIFO_STA     (0x07001174)


// MEM_ASPFIFO

#define ASPFIFO_CTRL	(0x07001170)
#define ASPFIFO_CTRL_ASPFIFOEN	(1<<0)

#define ASPFIFO_STAT   	(0x07001174)
#define ASPFIFO_STAT_ASPFIFOPROGFULL	(1<<19)
#define ASPFIFO_STAT_ASPFIFOPROGEMP		(1<<18)
#define ASPFIFO_STAT_ASPFIFOFULL		(1<<17)
#define ASPFIFO_STAT_ASPFIFOEMP			(1<<16)
#define ASPFIFO_STAT_ASPFIFOCOUNT_MASK	(0xFFF)

#define ASPFIFO_PROG   	(0x07001178)
#define ASPFIFO_PROG_ASPFIFOPROGFULSET_BIT	(16)


// MEM_DTFIFO

#define DTFIFO_MAX		(0x400)

#define DTFIFO_CTRL		(0x07001180)
#define DTFIFOTESTEN		(1<<16)
#define DTFIFOEN			(1<<0)

#define DTFIFO_STAT     (0x07001184)
#define DTFIFOREQ			(1<<31)
#define DTFIFOPROGFULL		(1<<19)
#define DTFIFOPROGEMP		(1<<18)
#define DTFIFOFULL			(1<<17)
#define DTFIFOEMP			(1<<16)
#define DTFIFOCOUNT_MASK	(0x3FF)

#define DTFIFO_PROG    	(0x07001188)

#define DTFIFO_ACCESS  	(0x07001190)	/* USB-FIFOとDMAするところ */


//MEM_SRAM

#define SRAM_CTRL      	(0x07001200)
#define SRAM_CTRL_SRAMEN	(1<<0)

#define SRAM_RST        (0x07001204)
#define SRAM_RST_SRAMRSTMODE_BIT	(16)
#define SRAM_RST_SRAMRST			(1<<0)

#define SRAM_MODE       (0x07001208)
#define	SRAM_MODE_RING				(0)
#define SRAM_MODE_FULLSTOP			(1)
#define SRAM_MODE_FULLBREAK			(2)

#define SRAM_STAT       (0x0700120C)
#define SRAM_STAT_SRAM_OVF			(1<<31)
#define SRAM_STAT_SRAM_EMPTY		(1<<30)
#define SRAM_STAT_SRAM_LIMIT		(1<<29)
#define SRAM_STAT_SRAMCOUNT_MASK	(0xFFFFFF)

#define SRAM_HIADD      (0x07001240)
#define SRAM_WRADD      (0x07001244)
#define SRAM_RDADD      (0x07001248)

#define SRAM_MAX_BYTE	(8*1024*1024)


//ASP_TIMER

#define TS_CTRL         (0x07001300)
#define TS_CTRL_TSSTARTMODE	(1<<31)	/* 0:soft trigger 1:hard trigger */
#define TS_CTRL_TSREAD		(1<<16)
#define TS_CTRL_TSEN		(1<<0)

#define TS_STAT         (0x07001304)
#define TS_STAT_TSOVF		(1<<16)
#define TS_STAT_TSCLR		(1<<0)

#define TS_DELAY        (0x07001308)

#define TS_VALUEL       (0x07001310)
#define TS_VALUEH       (0x07001314)

#define TM_CTRL         (0x07001320)
#define TM_CTRL_TMACTENCH1	(1<<25)
#define TM_CTRL_TMACTENCH0	(1<<24)
#define TM_CTRL_TMFCTENCH1	(1<<17)
#define TM_CTRL_TMFCTENCH0	(1<<16)
#define TM_CTRL_TMENCH1		(1<<1)
#define TM_CTRL_TMENCH0		(1<<0)

#define TM_STAT         (0x07001324)
#define TM_STAT_TMOVFCH1	(1<<17)
#define TM_STAT_TMOVFCH0	(1<<16)
#define TM_STAT_TMCLRCH1	(1<<1)
#define TM_STAT_TMCLRCH0	(1<<0)

#define TM_FACT         (0x0700132C)
#define TM_FACT_UPPERLMT	(1)
#define TM_FACT_LOWERLMT	(0)

#define TM_CMPL0        (0x07001330)
#define TM_CMPH0        (0x07001334)
#define TM_CMPL1        (0x07001338)
#define TM_CMPH1        (0x0700133C)

#define TM_UPDL0        (0x07001340)
#define TM_UPDH0        (0x07001344)
#define TM_UPDL1        (0x07001348)
#define TM_UPDH1        (0x0700134C)

#define TM_MINL0        (0x07001350)
#define TM_MINH0        (0x07001354)
#define TM_MINL1        (0x07001358)
#define TM_MINH1        (0x0700135C)

#define TM_MAXL0        (0x07001360)
#define TM_MAXH0        (0x07001364)
#define TM_MAXL1        (0x07001368)
#define TM_MAXH1        (0x0700136C)

#define TM_CNT0         (0x07001370)
#define TM_CNT1         (0x07001374)

#define TM_CUML0        (0x07001380)
#define TM_CUMH0        (0x07001384)
#define TM_CUML1        (0x07001388)
#define TM_CUMH1        (0x0700138C)


//Event Link
#define SEVL_CTRL0      (0x07001400)
#define SEVL_CTRL1      (0x07001404)
#define SEVL_CTRL2      (0x07001408)
#define SEVL_CTRL3      (0x0700140C)
#define SEVL_CTRL4      (0x07001410)
#define SEVL_CTRL5      (0x07001414)
#define SEVL_CTRL6      (0x07001418)
#define SEVL_CTRL7      (0x0700141C)

#define MEVL_ACTSEL     (0x07001480)

#define MEVL_ACTLOGIC   (0x07001490)

#define MEVL_ACTCTRL0   (0x070014A0)
#define MEVL_ACTCTRL1   (0x070014A4)
#define MEVL_ACTCTRL2   (0x070014A8)
#define MEVL_ACTCTRL3   (0x070014AC)

#define EVL_ACTDBG_P    (0x070014C0)
#define EVL_ACTDBG_L    (0x070014C4)



//Power Monitor
#define PWRMCTRL        (0x07001580)
#define	PWRMCTRL_READY		(1<<31)
#define	PWRMCTRL_FORCE		(1<<30)
#define	PWRMCTRL_CAL		(1<<4)
#define	PWRMCTRL_REC		(1<<2)
#define	PWRMCTRL_SAMPMODE	(1<<1)
#define	PWRMCTRL_EN			(1<<0)
#define PWRMCTRL_SAMPF_MASK	(0x3FFF0000)
#define PWRMCTRL_SAMPF_10US	(0x00950000)
#define	PWRMCTRL_SAMPCH_MASK (0x0000C000)
#define	PWRMCTRL_SAMP_CH1	(0x00000000)
#define	PWRMCTRL_SAMP_CH2	(0x00004000)
#define	PWRMCTRL_SAMP_CH3	(0x00008000)
#define	PWRMCTRL_SAMP_CH4	(0x0000C000)

#define PWRMFILTER_MIN  (0x07001584)

#define PWRMFILTER_MAX  (0x07001588)

#define PWRMOVC         (0x0700158C)
#define	PWRMOVC_OVC			(1<<16)

#define PWRMDBG0        (0x07001590)
#define PWRMDBG1        (0x07001594)
#define PWRMDBG2        (0x07001598)
#define PWRMDBG3        (0x0700159C)
#define PWRMREF0        (0x070015A0)
#define PWRMREF3        (0x070015A4)
#define PWRMON_AD_MASK		(0x00000FFF)

#define PWRMCALA2       (0x070015B0)
#define PWRMCALA3       (0x070015B4)
#define PWRMCALB2       (0x070015B8)
#define PWRMCALB3       (0x070015BC)

#define PWRM_EVLCTRL     (0x070015D0)
#define	PWRM_EVLCTRL_ACTEN	(1<<1)
#define	PWRM_EVLCTRL_FACTEN	(1<<0)

#define PWRM_EVLFACTFUNC (0x070015D4)
#define PWRM_FACTFUNC_MASK	(0x0000000F)
#define PWRM_EVLP        (0x070015D8)
#define PWRM_PWRMAX_MASK (0x0FFF0000)
#define PWRM_PWRMIN_MASK (0x00000FFF)
#define PWRM_EVLTP       (0x070015DC)
#define PWRM_TP_MASK     (0xFFFFFFFF)

#define PWRM_MAXMIN      (0x070015E8)
#define PWRM_RNGMAX_MASK (0x0FFF0000)
#define PWRM_RNGMIN_MASK (0x00000FFF)

//Proptocol Analyzer 0
#define PA0CTRL         (0x07001600)
#define PAnCTRL_EN						(1<<0)
#define PAnCTRL_REC						(1<<2)
#define PAnCTRL_BUSY					(1<<3)
#define PAnCTRL_TYPE_SPI				(0<<4)
#define PAnCTRL_TYPE_I2C				(1<<4)
#define PAnCTRL_TYPE_UART				(2<<4)
#define PAnCTRL_TYPE_CAN				(3<<4)
#define PAnCTRL_MODE_SPI_8BITDATA		(0<<8)
#define PAnCTRL_MODE_SPI_16BITDATA		(1<<8)
#define PAnCTRL_MODE_SPI_32BITDATA		(2<<8)
#define PAnCTRL_MODE_I2C_7BITADDR		(0<<8)
#define PAnCTRL_MODE_I2C_10BITADDR		(1<<8)
#define PAnCTRL_MODE_UART_8BIT			(0<<8)
#define PAnCTRL_MODE_UART_7BIT			(1<<8)
#define PAnCTRL_MODE_UART_9BIT			(2<<8)
#define PAnCTRL_MODE_UART_1STOPBIT		(0<<10)
#define PAnCTRL_MODE_UART_2STOPBIT		(1<<10)
#define PAnCTRL_MODE_UART_NONPARITY		(0<<11)
#define PAnCTRL_MODE_UART_ODDPARITY		(1<<11)
#define PAnCTRL_MODE_UART_EVENPARITY	(2<<11)
#define PAnCTRL_MODE_CAN_STD			(0<<8)
#define PAnCTRL_MODE_CAN_EXT			(1<<8)

#define PA0BAUDRATE     (0x07001604)
#define PA0SAMPPNT		(0x07001608)

#define PA0DBG0         (0x07001610)
#define PA0DBG1         (0x07001614)
#define PA0DBG2         (0x07001618)
#define PA0DBG3         (0x0700161C)
#define PA0DBG4         (0x07001620)

#define PA0_EVLCTRL     (0x07001630)
#define PAn_EVLCTRL_ACTEN			(1<<1)
#define PAn_EVLCTRL_FACTEN			(1<<0)
#define PA0_EVLFACTFUNC (0x07001634)
#define PAn_EVLFACTFUNC_MATCHP		(8)
#define PAn_EVLFACTFUNC_UNMATCHP	(9)
#define PA0_EVLFACTCNT	(0x07001638)

#define PA0_EVLP00      (0x07001640)
#define PA0_EVLP01      (0x07001644)
#define PA0_EVLP02      (0x07001648)
#define PA0_EVLP03      (0x0700164C)
#define PA0_EVLP04      (0x07001650)
#define PA0_EVLPMSK00   (0x07001654)
#define PA0_EVLPMSK01   (0x07001658)
#define PA0_EVLPMSK02   (0x0700165C)
#define PA0_EVLPMSK03   (0x07001660)
#define PA0_EVLPMSK04   (0x07001664)
#define PA0_EVLTP0      (0x07001668)

#define PA0_EVLP10      (0x07001670)
#define PA0_EVLP11      (0x07001674)
#define PA0_EVLP12      (0x07001678)
#define PA0_EVLP13      (0x0700167C)
#define PA0_EVLP14      (0x07001680)
#define PA0_EVLPMSK10   (0x07001684)
#define PA0_EVLPMSK11   (0x07001688)
#define PA0_EVLPMSK12   (0x0700168C)
#define PA0_EVLPMSK13   (0x07001690)
#define PA0_EVLPMSK14   (0x07001694)
#define PA0_EVLTP1      (0x07001698)

#define PA0_EVLP20      (0x070016A0)
#define PA0_EVLP21      (0x070016A4)
#define PA0_EVLP22      (0x070016A8)
#define PA0_EVLP23      (0x070016AC)
#define PA0_EVLP24      (0x070016B0)
#define PA0_EVLPMSK20   (0x070016B4)
#define PA0_EVLPMSK21   (0x070016B8)
#define PA0_EVLPMSK22 	(0x070016BC)
#define PA0_EVLPMSK23   (0x070016C0)
#define PA0_EVLPMSK24   (0x070016C4)
#define PA0_EVLTP2      (0x070016C8)

#define PA0_EVLP30      (0x070016D0)
#define PA0_EVLP31      (0x070016D4)
#define PA0_EVLP32      (0x070016D8)
#define PA0_EVLP33      (0x070016DC)
#define PA0_EVLP34      (0x070016E0)
#define PA0_EVLPMSK30   (0x070016E4)
#define PA0_EVLPMSK31   (0x070016E8)
#define PA0_EVLPMSK32   (0x070016EC)
#define PA0_EVLPMSK33   (0x070016F0)
#define PA0_EVLPMSK34   (0x070016F4)
#define PA0_EVLTP3      (0x070016F8)


//Proptocol Analyzer 1
#define PA1CTRL         (0x07001700)
#define PA1BAUDRATE     (0x07001704)
#define PA1SAMPPNT		(0x07001708)

#define PA1DBG0         (0x07001710)
#define PA1DBG1         (0x07001714)
#define PA1DBG2         (0x07001718)
#define PA1DBG3         (0x0700171C)
#define PA1DBG4         (0x07001720)

#define PA1_EVLCTRL     (0x07001730)
#define PA1_EVLFACTFUNC (0x07001734)
#define PA1_EVLFACTCNT	(0x07001738)

#define PA1_EVLP00      (0x07001740)
#define PA1_EVLP01      (0x07001744)
#define PA1_EVLP02      (0x07001748)
#define PA1_EVLP03      (0x0700174C)
#define PA1_EVLP04      (0x07001750)
#define PA1_EVLPMSK00   (0x07001754)
#define PA1_EVLPMSK01   (0x07001758)
#define PA1_EVLPMSK02   (0x0700175C)
#define PA1_EVLPMSK03   (0x07001760)
#define PA1_EVLPMSK04   (0x07001764)
#define PA1_EVLTP0      (0x07001768)

#define PA1_EVLP10      (0x07001770)
#define PA1_EVLP11      (0x07001774)
#define PA1_EVLP12      (0x07001778)
#define PA1_EVLP13      (0x0700177C)
#define PA1_EVLP14      (0x07001780)
#define PA1_EVLPMSK10   (0x07001784)
#define PA1_EVLPMSK11   (0x07001788)
#define PA1_EVLPMSK12   (0x0700178C)
#define PA1_EVLPMSK13   (0x07001790)
#define PA1_EVLPMSK14   (0x07001794)
#define PA1_EVLTP1      (0x07001798)

#define PA1_EVLP20      (0x070017A0)
#define PA1_EVLP21      (0x070017A4)
#define PA1_EVLP22      (0x070017A8)
#define PA1_EVLP23      (0x070017AC)
#define PA1_EVLP24      (0x070017B0)
#define PA1_EVLPMSK20   (0x070017B4)
#define PA1_EVLPMSK21   (0x070017B8)
#define PA1_EVLPMSK22   (0x070017BC)
#define PA1_EVLPMSK23   (0x070017C0)
#define PA1_EVLPMSK24   (0x070017C4)
#define PA1_EVLTP2      (0x070017C8)

#define PA1_EVLP30      (0x070017D0)
#define PA1_EVLP31      (0x070017D4)
#define PA1_EVLP32      (0x070017D8)
#define PA1_EVLP33      (0x070017DC)
#define PA1_EVLP34      (0x070017E0)
#define PA1_EVLPMSK30   (0x070017E4)
#define PA1_EVLPMSK31   (0x070017E8)
#define PA1_EVLPMSK32   (0x070017EC)
#define PA1_EVLPMSK33   (0x070017F0)
#define PA1_EVLPMSK34   (0x070017F4)
#define PA1_EVLTP3      (0x070017F8)


// CCPU-UART
#define CC_UART_EN		(0x07000070)
#define CC_UART_STAT	(0x07000074)
#define CC_UART_RXDATA	(0x07000078)
#define CC_UART_EN_ENBIT	(1<<0)

/* FPGAアクセスマクロ */
#if defined(_MSC_VER)
static inline DWORD FPGA_READ(ULONG r)
{
	DWORD tmp;
	e2_CPU_R(r, sizeof(tmp), (UCHAR*)&tmp);

	return tmp;
}
static inline void FPGA_WRITE(ULONG r, DWORD v)
{
	e2_CPU_W(r, sizeof(v), (UCHAR*)&v);
}
#else
#define FPGA_READ(r)         *((volatile DWORD*)(r))
#define FPGA_WRITE(r, v)     {*(DWORD*)(r) = (v);}
#endif

#define FPGA_SET_PAT(r, v) {\
       DWORD   tmp;\
       tmp = FPGA_READ((r));\
       tmp |= (v);\
       FPGA_WRITE((r), tmp);\
}

#define FPGA_CLR_PAT(r, v) {\
       DWORD tmp;\
       tmp = FPGA_READ((r));\
       tmp &= (~(v));\
       FPGA_WRITE(r, tmp);\
}

#define FPGA_MDF_PAT(r, v, m) {\
       DWORD tmp;\
       tmp = FPGA_READ((r));\
       tmp &= (~(m));\
       tmp |= (v);\
       FPGA_WRITE(r, tmp);\
}

#endif
