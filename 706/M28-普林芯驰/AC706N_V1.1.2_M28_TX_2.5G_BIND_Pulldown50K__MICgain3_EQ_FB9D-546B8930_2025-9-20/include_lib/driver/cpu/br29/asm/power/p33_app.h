/**@file  		p33_app.h
* @brief        hw sfr layer
* @details		include  1.p33_access
					 	 2.p33_analog
					     3.lowpower flow
						 4.pmu wkup
						 5.reset source
* @author		app / ic
* @date     	2021-10-13
* @version    	V1.0
* @copyright  	Copyright(c)2010-2031  JIELI
 */
#ifndef __P33_APP_H__
#define __P33_APP_H__

//
//
//					for p33 access
//
//
//
/**************************************************************/


#define p33_cs_h(x)         do{local_irq_disable();JL_PMU->SPI_CON  |= BIT(0);}while(0)
#define p33_cs_l            do{JL_PMU->SPI_CON  &= ~BIT(0);local_irq_enable();}while(0)

#define P33_OR              0b001
#define P33_AND             0b010
#define P33_XOR             0b011


//ROM
u8 p33_buf(u8 buf);

void p33_xor_1byte(u16 addr, u8 data0);
//#define p33_xor_1byte(addr, data0)      (*((volatile u8 *)&addr + 0x300*4)  = data0)
// #define p33_xor_1byte(addr, data0)      addr ^= (data0)

void p33_or_1byte(u16 addr, u8 data0);
//#define p33_or_1byte(addr, data0)       (*((volatile u8 *)&addr + 0x200*4)  = data0)
// #define p33_or_1byte(addr, data0)       addr |= (data0)

void p33_and_1byte(u16 addr, u8 data0);
//#define p33_and_1byte(addr, data0)      (*((volatile u8 *)&addr + 0x100*4)  = (data0))
//#define p33_and_1byte(addr, data0)      addr &= (data0)

void p33_tx_1byte(u16 addr, u8 data0);
//#define p33_tx_1byte(addr, data0)       addr = data0

u8 p33_rx_1byte(u16 addr);
//#define p33_rx_1byte(addr)              addr

void P33_CON_SET(u16 addr, u8 start, u8 len, u8 data);

#define P33_CON_GET(sfr)    (p33_rx_1byte(sfr))

#define p33_fast_access(reg, data, en)           \
{ 												 \
    if (en) {                                    \
		p33_or_1byte(reg, (u8)(data));           \
    } else {                                     \
		p33_and_1byte(reg,(u8)~(data));          \
    }                                            \
}


//
//
//					for p33_analog
//
//
//

/************************P3_ANA_CON0*****************************/
#define RC250K_EN(en)               p33_fast_access(P3_ANA_CON0, BIT(7), en)

#define MVIO_SHORT_WVIO(en)         p33_fast_access(P3_ANA_CON0, BIT(5), en)

#define VMAX_SW_EN(en)              p33_fast_access(P3_ANA_CON0, BIT(4), en)

#define MVBG_EN(en)                 p33_fast_access(P3_ANA_CON0, BIT(3), en)

#define MVIO_EN(en)                 p33_fast_access(P3_ANA_CON0, BIT(2), en)

#define DCVD_LDO_EN(en)             p33_fast_access(P3_ANA_CON0, BIT(1), en)

#define DVDD_EN(en)                 p33_fast_access(P3_ANA_CON0, BIT(0), en)

/************************P3_ANA_CON1*****************************/
#define WVDD_LOAD_EN(en)            p33_fast_access(P3_ANA_CON1, BIT(5), en)

#define WVDD_EN(en)                 p33_fast_access(P3_ANA_CON1, BIT(4), en)

#define DCVD_IFULLEN(en)            p33_fast_access(P3_ANA_CON1, BIT(2), en)

#define MVIO_IFULLEN(en)            p33_fast_access(P3_ANA_CON1, BIT(1), en)

#define MVIO_VLMTEN(en)             p33_fast_access(P3_ANA_CON1, BIT(0), en)


/************************P3_ANA_CON2*****************************/
#define SWVLD_AUTO_EN(en)           p33_fast_access(P3_ANA_CON2, BIT(7), en)

#define LPMR_RST_DLY(en)            p33_fast_access(P3_ANA_CON2, BIT(5), en)

#define D2SH_EN_SW(en)              p33_fast_access(P3_ANA_CON2, BIT(4), en)
#define CLOCK_KEEP(en)			    D2SH_EN_SW(en)

#define VCM_DET_EN(en)              p33_fast_access(P3_ANA_CON2, BIT(3), en)


/************************P3_PMU_ADC0*****************************/
enum {
    VBG_TEST_SEL_WBG04,
    VBG_TEST_SEL_MBG04,
    VBG_TEST_SEL_LVDBG,
    VBG_TEST_SEL_MBG08,
};
#define VBG_TEST_SEL(sel)			P33_CON_SET(P3_PMU_ADC0, 6, 2, sel)

#define VBG_TEST_EN(en)				p33_fast_access(P3_PMU_ADC0, BIT(5), en)

#define VBG_BUFFER_EN(en)			p33_fast_access(P3_PMU_ADC0, BIT(4), en)

#define ADC_CHANNEL_SEL(ch)     	P33_CON_SET(P3_PMU_ADC0, 0, 4, ch)

#define PMU_DET_BG_BUF_DISABLE() 	{p33_fast_access(P3_PMU_ADC0, BIT(4)|BIT(5), 0);p33_fast_access(P3_PMU_ADC1, BIT(0), 0);}

#define GET_P3_PMU_ADC0() 			P33_CON_GET(P3_PMU_ADC0)

#define SET_P3_PMU_ADC0(val) 		P33_CON_SET(P3_PMU_ADC0, 0, 8, val)

/************************P3_PMU_ADC1*****************************/
#define PMU_TOADC_OE(en)		    p33_fast_access(P3_PMU_ADC1, BIT(1), en)

#define PMU_DET_EN(en)				p33_fast_access(P3_PMU_ADC1, BIT(0), en)
/************************P3_VBG_CON0*****************************/
#define MVBG_SEL(sel)				P33_CON_SET(P3_VBG_CON0, 0, 4, sel)

#define WVBG_SEL(sel)				P33_CON_SET(P3_VBG_CON0, 4, 4, sel)

/************************P3_IOV_CON0*****************************/

enum {
    VDDIOW_VOL_21V = 0,
    VDDIOW_VOL_22V,
    VDDIOW_VOL_23V,
    VDDIOW_VOL_24V,
    VDDIOW_VOL_25V,
    VDDIOW_VOL_26V,
    VDDIOW_VOL_27V,
    VDDIOW_VOL_28V,
    VDDIOW_VOL_29V,
    VDDIOW_VOL_30V,
    VDDIOW_VOL_31V,
    VDDIOW_VOL_32V,
    VDDIOW_VOL_33V,
    VDDIOW_VOL_34V,
    VDDIOW_VOL_35V,
    VDDIOW_VOL_36V,
};

#define VDDIOW_VOL_SEL(lev)         P33_CON_SET(P3_IOV_CON0, 4, 4, lev)

#define GET_VDDIOW_VOL()            (P33_CON_GET(P3_IOV_CON0)>>4 & 0xf)

//vddiom_lev
enum {
    VDDIOM_VOL_21V = 0,
    VDDIOM_VOL_22V,
    VDDIOM_VOL_23V,
    VDDIOM_VOL_24V,
    VDDIOM_VOL_25V,
    VDDIOM_VOL_26V,
    VDDIOM_VOL_27V,
    VDDIOM_VOL_28V,
    VDDIOM_VOL_29V,
    VDDIOM_VOL_30V,
    VDDIOM_VOL_31V,
    VDDIOM_VOL_32V,
    VDDIOM_VOL_33V,
    VDDIOM_VOL_34V,
    VDDIOM_VOL_35V,
    VDDIOM_VOL_36V,
};

#define VDDIOM_VOL_SEL(lev)         P33_CON_SET(P3_IOV_CON0, 0, 4, lev)

#define GET_VDDIOM_VOL()            (P33_CON_GET(P3_IOV_CON0) & 0xf)


/************************P3_IOV_CON1*****************************/
#define VDDIO_HD_SEL(hd)       		P33_CON_SET(P3_IOV_CON1, 0, 2, hd)

/************************P3_DCV_CON0*****************************/
#define DCVD_DEOVSHOT_EN(en)       	//p33_fast_access(P3_DCV_CON0, BIT(7), en)

#define DCVD_HD_SEL(sel)			P33_CON_SET(P3_DCV_CON0, 3, 2, sel)

#define DCVD_LOAD_EN(en)       		p33_fast_access(P3_DCV_CON0, BIT(2), en)

#define DCVD_CAP_EN(en)       		p33_fast_access(P3_DCV_CON0, BIT(1), en)

#define DCVD_ATE_EN(en)       		p33_fast_access(P3_DCV_CON0, BIT(0), en)

enum {
    DCVDD_VOL_SEL_100V = 0,
    DCVDD_VOL_SEL_105V,
    DCVDD_VOL_SEL_110V,
    DCVDD_VOL_SEL_115V,
    DCVDD_VOL_SEL_120V,
    DCVDD_VOL_SEL_125V,
    DCVDD_VOL_SEL_130V,
    DCVDD_VOL_SEL_135V,
    DCVDD_VOL_SEL_140V,
    DCVDD_VOL_SEL_145V,
    DCVDD_VOL_SEL_150V,
    DCVDD_VOL_SEL_155V,
    DCVDD_VOL_SEL_160V,
};

/************************P3_DCV_CON1*****************************/
#define DCVDD_DEFAULT_VOL			DCVDD_VOL_SEL_120V

#define DCVD_TRIM2_0(val)           P33_CON_SET(P3_DCV_CON1, 4, 3, sel)

#define GET_DCVD_TRIME2_0()         (P33_CON_GET(P3_DCV_CON1)>>4 & 0x3)

#define GET_DCVDD_VOL_SEL()      	(P33_CON_GET(P3_DCV_CON1) & 0xf)

#define DCVDD_VOL_SEL(sel)      	P33_CON_SET(P3_DCV_CON1, 0, 4, sel)
void dcvdd_vol_sel(u8 vol);
u8 get_dcvdd_vol_sel();


/************************P3_DVD_CON0*****************************/
enum {
    DVDD_VOL_SEL_084V = 0,
    DVDD_VOL_SEL_087V,
    DVDD_VOL_SEL_090V,
    DVDD_VOL_SEL_093V,
    DVDD_VOL_SEL_096V,
    DVDD_VOL_SEL_099V,
    DVDD_VOL_SEL_102V,
    DVDD_VOL_SEL_105V,
    DVDD_VOL_SEL_108V,
    DVDD_VOL_SEL_111V,
    DVDD_VOL_SEL_114V,
    DVDD_VOL_SEL_117V,
    DVDD_VOL_SEL_120V,
    DVDD_VOL_SEL_123V,
    DVDD_VOL_SEL_126V,
    DVDD_VOL_SEL_129V,
};

#define DVDD_DEFAULT_VOL			DVDD_VOL_SEL_111V

#define DVDD_VOL_SEL(sel)     		P33_CON_SET(P3_DVD_CON0, 0, 4, sel)

#define GET_DVDD_VOL_SEL()     		(P33_CON_GET(P3_DVD_CON0) & 0xf)

#define DVDD_VOL_HD_SEL(sel)  		P33_CON_SET(P3_DVD_CON0, 4, 2, sel)


void dvdd_vol_sel(u8 vol);
u8 get_dvdd_vol_sel();

/************************P3_DVD_CON1*****************************/
#define DVD_LOAD_EN(en)             p33_fast_access(P3_DVD_CON1, BIT(3), en)

#define DVD_ATE_EN(en)              p33_fast_access(P3_DVD_CON1, BIT(1), en)

/************************P3_WVD_CON0*****************************/
enum {
    WVDD_VOL_SEL_050V = 0,
    WVDD_VOL_SEL_055V,
    WVDD_VOL_SEL_060V,
    WVDD_VOL_SEL_065V,
    WVDD_VOL_SEL_070V,
    WVDD_VOL_SEL_075V,
    WVDD_VOL_SEL_080V,
    WVDD_VOL_SEL_085V,
    WVDD_VOL_SEL_090V,
    WVDD_VOL_SEL_095V,
    WVDD_VOL_SEL_100V,
    WVDD_VOL_SEL_105V,
    WVDD_VOL_SEL_110V,
    WVDD_VOL_SEL_115V,
    WVDD_VOL_SEL_120V,
    WVDD_VOL_SEL_125V,
};

#define WVDD_VOL_SEL_MAX    		WVDD_VOL_SEL_125V

#define WVDD_TRIM_LEVEL				800//mv

/************************P3_LRC_CON0*****************************/
#define LRC_CON0_INIT                                     \
        /*                               */     (0 << 7) |\
        /*RC32K_CAP_S2_33v               */     (0 << 6) |\
        /*RC32K_CAP_S1_33v               */     (1 << 5) |\
        /*RC32K_CAP_S0_33v               */     (0 << 4) |\
        /*                               */     (0 << 3) |\
        /*                               */     (0 << 2) |\
        /*RC32K_RN_TRIM_33v              */     (1 << 1) |\
        /*RC32K_EN_33v                   */     (1 << 0)

#define LRC_CON1_INIT                                     \
        /*                               */     (0 << 7) |\
        /*                               */     (0 << 6) |\
        /*RC32K_RPPS_S1_33v              */     (1 << 5) |\
        /*RC32K_RPPS_S0_33v              */     (1 << 4) |\
        /*                        2bit   */     (0 << 2) |\
        /*RC32K_RNPS_S1_33v              */     (0 << 1) |\
        /*RC32K_RNPS_S0_33v              */     (0 << 0)

#define LRC_Hz_DEFAULT    (200 * 1000L)

#define LRC32K_CAP_SEL(sel)     	P33_CON_SET(P3_LRC_CON0, 4, 3, sel)

#define LRC32K_RN_TRIM(en)       	p33_fast_access(P3_LRC_CON0, BIT(1), en)

#define LRC_EN(en)            		p33_fast_access(P3_LRC_CON0, BIT(0), en)

/************************P3_LRC_CON1*****************************/
#define LRC32K_RPPS_SEL(sel)      	P33_CON_SET(P3_LRC_CON1, 4, 2, sel)

#define LRC32K_RNPS_SEL(sel)     	P33_CON_SET(P3_LRC_CON1, 0, 2, sel)

#define CLOSE_LRC()					p33_tx_1byte(P3_LRC_CON0, 0);\
									p33_tx_1byte(P3_LRC_CON1, 0)

/************************P3_VLVD_CON0*****************************/
#define GET_VLVD_PND()          	((P33_CON_GET(P3_VLVD_CON) & BIT(7)) ? 1 : 0)

#define VLVD_PND_CLR()       		p33_fast_access(P3_VLVD_CON, BIT(6), 1)

#define VLVD_SEL(sel)               P33_CON_SET(P3_VLVD_CON, 3, 3, sel)

#define P33_VLVD_OE(en)             p33_fast_access(P3_VLVD_CON, BIT(2), en)

#define VLVD_EXPIN_EN(en)           p33_fast_access(P3_VLVD_CON, BIT(1), en)

#define P33_VLVD_EN(en)         	p33_fast_access(P3_VLVD_CON, BIT(0), en)

#define GET_P33_VLVD_EN()			((P33_CON_GET(P3_VLVD_CON) & BIT(0)) ? 1:0)

/************************P3_VLVD_FLT*****************************/
#define VLVD_FLT(sel)				P33_CON_SET(P3_VLVD_FLT, 0, 2, sel);

#define GET_VLVD_FLT()              (P33_CON_GET(P3_VLVD_FLT) & 0x3)

/************************P3_VLVD_TRIM*****************************/
#define LVDBG_S3_0()                (P33_CON_SET(P3_VLVD_TRIM, 0, 4, sel))

/************************P3_VLD_KEEP*****************************/
#define VLD_KEEP_RTC_WKUP(en)       p33_fast_access(P3_VLD_KEEP, BIT(7), en)

#define VLD_KEEP_WDT_EXPT(en)       p33_fast_access(P3_VLD_KEEP, BIT(6), en)

#define VLD_KEEP_VDD_LEL(en)        p33_fast_access(P3_VLD_KEEP, BIT(5), en)

#define VLD_KEEP_SYS_RST(en)        p33_fast_access(P3_VLD_KEEP, BIT(4), en)

#define VLD_KEEP_PWM_CLK(en)        p33_fast_access(P3_VLD_KEEP, BIT(3), en)

#define VLD_KEEP_RCLK_DIS(en)       p33_fast_access(P3_VLD_KEEP, BIT(2), en)

#define VLD_KEEP_WKUP(en)           p33_fast_access(P3_VLD_KEEP, BIT(1), en)

#define VLD_KEEP_CLK(en)            p33_fast_access(P3_VLD_KEEP, BIT(0), en)


/************************P3_CLK_CON0*****************************/

#define SOFF_RC250K_GATE(en)        p33_fast_access(P3_CLK_CON0, BIT(4), en)

//
//
//					for pmu flow
//
//
//


/********************P3_PMU_CON*********************************/

enum {
    WLDO_LEVEL_050V = 0,
    WLDO_LEVEL_054V,
    WLDO_LEVEL_058V,
    WLDO_LEVEL_062V,
    WLDO_LEVEL_066V,
    WLDO_LEVEL_070V,
    WLDO_LEVEL_085V,
    WLDO_LEVEL_120V,
};

#define PD_CON0_INIT                            \
        /*                       */ ( 0<<7 )  | \
        /*                       */ ( 0<<6 )  | \
        /* PD_RUN FLAG  1bit RO  */ ( 0<<5 )  | \
        /* WL SETL TMR  1bit RW  */ ( 0<<4 )  | \
        /* DIS EN       2bit RW  */ ( 0<<2 )  | \
        /* PD MD        1bit RW  */ ( 0<<1 )  /*0:Power down / 1:Power off*/ | \
        /* PD_EN        1bit RW  */ ( 1<<0 )

#define PD_CON1_INIT_BT                            \
        /* POR FLAG     1bit RO  */ ( 0<<7 )  | \
        /* CLR POR PND  1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 3<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /* PU SLOW      1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 1<<0 )  /* 0:rc250k 1:btosc 2:io_clk_in 3:rtc_osl 4:lrc*/

#define PD_CON1_INIT_RTC_32K                         \
        /* POR FLAG     1bit RO  */ ( 0<<7 )  | \
        /* CLR POR PND  1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /* PU SLOW      1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 3<<0 )  /* 0:rc250k 1:btosc 2:io_clk_in 3:rtc_osl 4:lrc*/

#define PD_CON1_INIT_LRC_32K                         \
        /* POR FLAG     1bit RO  */ ( 0<<7 )  | \
        /* CLR POR PND  1bit RW  */ ( 0<<6 )  | \
        /* CK DIV       2bit RW  */ ( 0<<4 )  /* 0:div0 1:div4 2:div16 3:div64 */  | \
        /* PU SLOW      1bit RW  */ ( 0<<3 )  | \
        /* CK SEL       3bit RW  */ ( 4<<0 )  /* 0:rc250k 1:btosc 2:io_clk_in 3:rtc_osl 4:lrc*/

#define PD_CON4_INIT_HW_KST_TMR0                \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 0<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 1<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 0<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 0<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 0<<0 )

#define PD_CON4_INIT_SW_KST_TMR0                \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 0<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 0<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 0<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 1<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 0<<0 )

#define PD_CON4_INIT_HW_KST_TMR1                \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 1<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 0<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 0<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 0<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 0<<0 )

#define PD_CON4_INIT_SW_KST_TMR1                \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 0<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 0<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 1<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 0<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 0<<0 )

#define PD_CON4_INIT_LP_KST                     \
        /*              1bit RW  */ ( 0<<7 )  | \
        /* HW_KST TMR1  1bit RW  */ ( 0<<6 )  | \
        /* HW_KST TMR0  1bit RW  */ ( 0<<5 )  | \
        /* HW_KST LP    1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* SW_KST TMR1  1bit RW  */ ( 0<<2 )  | \
        /* SW_KST TMR0  1bit RW  */ ( 0<<1 )  | \
        /* SW_KST LP    1bit RW  */ ( 1<<0 )

#define PD_STB0_STB1_INIT(a , b)                       \
        /* STB1 SET     4bit RW  */ ( a<<4 )  | \
        /* STB0 SET     4bit RW  */ ( b<<0 )

#define PD_STB2_STB3_INIT(a, b)                         \
        /* STB3 SET     4bit RW  */ ( a<<4 )  | \
        /* STB2 SET     4bit RW  */ ( b<<0 )

#define PD_STB4_STB5_INIT(a, b)                         \
        /* STB5 SET     4bit RW  */ ( a<<4 )  | \
        /* STB4 SET     4bit RW  */ ( b<<0 )

#define PD_STB6_INIT(a)                            \
        /* STB6 SET     4bit RW  */ ( a<<0 )

#define pd_tmr0_init                            \
        /* TIMEOUT FLAG 1bit RO  */ ( 0<<7 )  | \
        /* CLR TO PND   1bit RW  */ ( 1<<6 )  | \
        /* WK PND       1bit RW  */ ( 0<<5 )  | \
        /* CLR WK PND   1bit RW  */ ( 0<<4 )  | \
        /* TIMEOUT IE   1bit RW  */ ( 1<<3 )  | \
        /* RECOVER IE   1bit RW  */ ( 1<<2 )  | \
        /* CONTINUE     1bit RW  */ ( 0<<1 )  | \
        /* EN           1bit RW  */ ( 1<<0 )

#define pd_tmr1_init                            \
        /* TIMEOUT FLAG 1bit RO  */ ( 0<<7 )  | \
        /* CLR TO PND   1bit RW  */ ( 1<<6 )  | \
        /* WK PND       1bit RW  */ ( 0<<5 )  | \
        /* CLR WK PND   1bit RW  */ ( 1<<4 )  | \
        /* TIMEOUT IE   1bit RW  */ ( 1<<3 )  | \
        /* RECOVER IE   1bit RW  */ ( 1<<2 )  | \
        /* CONTINUE     1bit RW  */ ( 0<<1 )  | \
        /* EN           1bit RW  */ ( 1<<0 )

#define PD_LP0_CNT_READ                         \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /*              1bit RW  */ ( 0<<5 )  | \
        /*              1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* TMR1 CNT READ1bit WO  */ ( 0<<2 )  | \
        /* TMR0 CNT READ1bit WO  */ ( 1<<1 )  | \
        /* STC READ     1bit WO  */ ( 0<<0 )

#define PD_LP1_CNT_READ                         \
        /*              1bit RO  */ ( 0<<7 )  | \
        /*              1bit RW  */ ( 0<<6 )  | \
        /*              1bit RW  */ ( 0<<5 )  | \
        /*              1bit RW  */ ( 0<<4 )  | \
        /*              1bit RW  */ ( 0<<3 )  | \
        /* TMR1 CNT READ1bit WO  */ ( 1<<2 )  | \
        /* TMR0 CNT READ1bit WO  */ ( 0<<1 )  | \
        /* STC READ     1bit WO  */ ( 0<<0 )

#define pd_wvdd_auto0_init                      \
        /* WLDO PRD     3bit RW  */ ( 1<<5 )  | \
        /* AUTO EN      1bit RW  */ ( 1<<4 )  | \
        /*              4bit RW  */ ( 0<<0 )

#define pd_wvdd_auto1_init                      \
        /* WLDO LVL LOW 4bit RW  */ ( 0<<5 )  | \
        /* WLDO LEVEL   4bit RW  */ (15<<0 )

#define NV_RAM_POWER_GATE(sw)						\
{													\
    if (sw) {										\
        p33_fast_access(P3_IVS_SET, BIT(4), 1);		\
    } else {										\
        p33_fast_access(P3_IVS_CLR, BIT(4), 1);		\
    }												\
}

#define LP_TMR0_WKUP()		(p33_rx_1byte(P3_LP_TMR0_CON) & BIT(5))
#define LP_TMR0_CLR_WKUP()	(p33_or_1byte(P3_LP_TMR0_CON, BIT(4)))

#define LP_TMR0_TO()		(p33_rx_1byte(P3_LP_TMR0_CON) & BIT(7))
#define LP_TMR0_CLR_TO()	(p33_or_1byte(P3_LP_TMR0_CON, BIT(6)))

//
//
//			for reset_source
//
//
//
/************************P3_PR_PWR*****************************/
#define	P3_SOFT_RESET()				p33_fast_access(P3_PR_PWR, BIT(4), 1)

#define MCLR_EN(en)					p33_fast_access(P3_PR_PWR, BIT(3), en)

#define IS_MCLR_EN()				((P33_CON_GET(P3_PR_PWR) & BIT(3)) ? 1 : 0)

/************************P3_RST_CON0*****************************/
#define DVDDOK_OE(en) 				p33_fast_access(P3_RST_CON0, BIT(4), en)

#define MSYS_TO_P33_RST_MASK(en)    p33_fast_access(P3_RST_CON0, BIT(0), en)

#define FAST_PU_SYS(en)           	p33_fast_access(P3_EFU_FLAG, BIT(0), en)

/************************P3_IVS_CLR*****************************/
#define	P33_SF_KICK_START()			P33_CON_SET(P3_IVS_CLR, 0, 8, 0b00101010)

/************************P3_RST_SRC*****************************/
#define GET_P33_SYS_RST_SRC()		P33_CON_GET(P3_RST_SRC)

/************************P3_RST_FLAG*****************************/
#define GET_P33_SYS_POWER_FLAG() 	((P33_CON_GET(P3_RST_FLAG) & BIT(3)) ? 1 : 0)

//level0>level1>level2
//高优先级的先被复位
#define P33_SYS_RST_LEVEL2_CLEAR()	(p33_or_1byte(P3_RST_FLAG, BIT(2)))
#define GET_P33_SYS_RST_LEVEL2()	((P33_CON_GET(P3_RST_FLAG) & BIT(3)) ? 1 : 0)

#define P33_SYS_RST_LEVEL1_CLEAR()	(p33_or_1byte(P3_RST_FLAG, BIT(4)))
#define GET_P33_SYS_RST_LEVEL1()	((P33_CON_GET(P3_RST_FLAG) & BIT(5)) ? 1 : 0)

#define P33_SYS_RST_LEVEL0_CLEAR()	(p33_or_1byte(P3_RST_FLAG, BIT(6)))
#define GET_P33_SYS_RST_LEVEL0()	((P33_CON_GET(P3_RST_FLAG) & BIT(7)) ? 1 : 0)


//
//
//			for wkup
//
//
//
/************************P3_PCNT_sSET0*****************************/
#define SET_EXCEPTION_FLAG()        P33_CON_SET(P3_SFLAG0, 0, 8, 0xab)

#define GET_EXCEPTION_FLAG()        ((P33_CON_GET(P3_SFLAG0) == 0xab) ? 1 : 0)
#define GET_ASSERT_FLAG()           ((P33_CON_GET(P3_SFLAG0) == 0xac) ? 1 : 0)

#define SOFT_RESET_FLAG_CLEAR()     (P33_CON_SET(P3_SFLAG0, 0, 8, 0))


/************************P3_PINR_CON**********************************/
#define GET_PINR_EN()				(P33_CON_GET(P3_PINR_CON) & BIT(0))

#define GET_PINR_PORT()				P33_CON_GET(P3_PORT_SEL0)


#endif

