#ifndef __CLOCK_HW_H__
#define __CLOCK_HW_H__

#include "typedef.h"
#define BT_CLOCK_IN(x)          //SFR(JL_CLOCK->CLK_CON4,  16,  2,  x)
#define USB_CLOCK_IN(x)
#define LSB_CLK_DIV(x)
#if 0
// #define CLOCK_HAL_DEBUG

#ifdef CLOCK_HAL_DEBUG

#define RC_EN(x)
#define TEST_SEL(x)
#define OSC_CLOCK_IN(x)
//for MACRO - OSC_CLOCK_IN
enum {
    OSC_CLOCK_IN_BT_OSC = 0,
    OSC_CLOCK_IN_RTOSC_H,
    OSC_CLOCK_IN_RTOSC_L,
    OSC_CLOCK_IN_PAT,
};

#define MAIN_CLOCK_SEL(x)
//for MACRO - CLOCK_IN
enum {
    MAIN_CLOCK_IN_RC = 0,

    MAIN_CLOCK_IN_BTOSC = 4,
    MAIN_CLOCK_IN_RTOSC_H,
    MAIN_CLOCK_IN_RTOSC_L,
    MAIN_CLOCK_IN_PLL,

    MAIN_CLOCK_IN_PAT, //for tes
};

#define SFR_MODE(x)
enum {
    SFR_CLOCK_IDLE = 0,
    SFR_CLOCK_ALWAYS_ON,
};
#define PB0_CLOCK_OUT(x)
#define PA2_CLOCK_OUT(x)


#define USB_CLOCK_IN(x)
//for MACRO - USB_CLOCK_IN
enum {
    USB_CLOCK_IN_PLL48M = 0,
    USB_CLOCK_IN_DISABLE,
    USB_CLOCK_IN_LSB,
    USB_CLOCK_IN_DISABLE_PAD,
};
#define DAC_CLOCK_IN(x)
//for MACRO - DAC_CLOCK_IN
enum {
    DAC_CLOCK_IN_PLL24M = 0,
    DAC_CLOCK_IN_OSC,
    DAC_CLOCK_IN_LSB,
    DAC_CLOCK_IN_DISABLE_PAD,
};
#define APC_CLOCK_IN(x)
//for MACRO - APC_CLOCK_IN
enum {
    APC_CLOCK_IN_PLL64M = 0,
    APC_CLOCK_IN_PLLAPC,
    APC_CLOCK_IN_LSB,
    APC_CLOCK_IN_DISABLE,
};
#define UART_CLOCK_IN(x)
//for MACRO - UART_CLOCK_IN
enum {
    UART_CLOCK_IN_PLL48M = 0,
    UART_CLOCK_IN_OSC,
    UART_CLOCK_IN_LSB,
    UART_CLOCK_IN_DISABLE,
};
#define BT_CLOCK_IN(x)
//for MACRO - BT_CLOCK_IN
enum {
    BT_CLOCK_IN_PLL64M = 0,
    BT_CLOCK_IN_DISABLE,
    BT_CLOCK_IN_LSB,
    BT_CLOCK_IN_DISABLE_PAD,
};


#define SFC_CLOCK_DELAY(x)


#define PLL_SYS_SEL(x)
//for MACRO - PLL_SYS_SEL
enum {
    PLL_SYS_SEL_PLL192M = 0,
    PLL_SYS_SEL_PLL137M,
    PLL_SYS_SEL_PLL480M,
    PLL_SYS_SEL_DISABLE,
};
#define PLL_SYS_DIV(x)
//for MACRO - PLL_SYS_DIV
enum {
    PLL_SYS_DIV1 = 0,
    PLL_SYS_DIV3,
    PLL_SYS_DIV5,
    PLL_SYS_DIV7,

    PLL_SYS_DIV1X2 = 4,
    PLL_SYS_DIV3X2,
    PLL_SYS_DIV5X2,
    PLL_SYS_DIV7X2,

    PLL_SYS_DIV1X4 = 8,
    PLL_SYS_DIV3X4,
    PLL_SYS_DIV5X4,
    PLL_SYS_DIV7X4,

    PLL_SYS_DIV1X8 = 12,
    PLL_SYS_DIV3X8,
    PLL_SYS_DIV5X8,
    PLL_SYS_DIV7X8,
};

#define PLL_APC_SEL(x)
//for MACRO - PLL_APC_SEL
enum {
    PLL_APC_SEL_PLL192M = 0,
    PLL_APC_SEL_PLL137M,
    PLL_APC_SEL_DISABLE,
};
#define PLL_APC_DIV(x)
//for MACRO - PLL_APC_DIV
enum {
    PLL_APC_DIV1 = 0,
    PLL_APC_DIV3,
    PLL_APC_DIV5,
    PLL_APC_DIV7,

    PLL_APC_DIV1X2 = 4,
    PLL_APC_DIV3X2,
    PLL_APC_DIV5X2,
    PLL_APC_DIV7X2,

    PLL_APC_DIV1X4 = 8,
    PLL_APC_DIV3X4,
    PLL_APC_DIV5X4,
    PLL_APC_DIV7X4,

    PLL_APC_DIV1X8 = 12,
    PLL_APC_DIV3X8,
    PLL_APC_DIV5X8,
    PLL_APC_DIV7X8,
};

#define PLL_ALNK_SEL(x)
//for MACRO - PLL_ALNK_SEL
enum {
    PLL_ALNK_192M_DIV17 = 0,
    PLL_ALNK_480M_DIV39,
};

#define PLL_EN(x)
#define PLL_REST(x)
#define PLL_DIVn(x)
#define PLL_DIVn_EN(x)
#define PLL_REF_SEL(x)
//for MACRO - PLL_RSEL
enum {
    PLL_REF_SEL_BTOSC = 0, 	//bt
    PLL_REF_SEL_RTOSC,		//rt
    PLL_REF_SEL_PAT = 3,
};

#define PLL_TEST(x)
#define PLL_DSMS(x)
#define PLL_DSM_TSEL(x)
#define PLL_DSM_RSEL(x)
#define PLL_DSM_MSEL(x)

#define PLL_DIVSEL(x)

//for MACRO - PLL_RSEL
enum {
    PLL_DIVIDER_INTE = 0,
    PLL_DIVIDER_FRAC,
};

#define PLL_PFD(x)
#define PLL_ICP(x)
#define PLL_LPFR2(x)
#define PLL_LPFR3(x)


#define PLL_RSEL(x)
//for MACRO - PLL_RSEL
enum {
    PLL_RSEL_BTOSC_DIFF = 0,
    PLL_RSEL_RTOSC_DIFF,
    PLL_RSEL_PLL_REF_SEL,
};

#define PLL_DIVm(x)
#define PLL_LD012A(x)
#define PLL_LDO12D(x)
#define PLL_IVCO(x)
#define PLL_LDO_BYPASS(x)
#define PLL_TSSEL(x)
#define PLL_TSOE(x)

#define PLL_CLK480M_OE(x)
#define PLL_CLK320M_OE(x)
#define PLL_CLK192M_OE(x)
#define PLL_CLK137M_OE(x)
#define PLL_CLK107M_OE(x)

#define PLL_FRAC(x)
#define PLL_DMAX(x)
#define PLL_DMIN(x)
#define PLL_DSTP(x)

#define HSB_CLK_DIV(x)
#define LSB_CLK_DIV(x)
#define OTP_CLK_DIV(x)


#else

// TODO
#define RC_EN(x)               // SFR(JL_CLOCK->CLK_CON0,  0,  1,  x)
#define RCH_EN(x)				//SFR(JL_CLOCK->CLK_CON0,  0,  1,  x)
//for MACRO - RCH_EN
enum {
    RCH_EN_250K = 0,
    RCH_EN_16M,
};

#define TEST_SEL(x)				//SFR(JL_CLOCK->CLK_CON0,  2,  2,  x)
//for MACRO - TS_SEL
enum {
    TS_SEL_IN_MAIN = 0,
    TS_SEL_IN_PAT,
};

#define OSC_CLOCK_IN(x)         //SFR(JL_CLOCK->CLK_CON0,  1,  3,  x)
//for MACRO - OSC_CLOCK_IN
enum {
    OSC_CLOCK_IN_BT_OSC = 0,
    OSC_CLOCK_IN_BT_OSC_X2,
    OSC_CLOCK_IN_STD_24M,
    OSC_CLOCK_IN_RTC_OSC,
    OSC_CLOCK_IN_RTOSC_L,
    OSC_CLOCK_IN_PAT,
};

//for MACRO - CLOCK_IN
enum {
    MAIN_CLOCK_IN_RC = 0,
    MAIN_CLOCK_IN_PAT,
    MAIN_CLOCK_IN_BTOSC,
    MAIN_CLOCK_IN_BTOSC_X2,
    MAIN_CLOCK_IN_PLL,
    MAIN_CLOCK_IN_RESERVED,
    MAIN_CLOCK_IN_RTOSC_L, //keep to fix make
};

#define MAIN_CLOCK_SEL(x) 		//SFR(JL_CLOCK->SYS_SEL,  0,  3,  x); \
asm("csync")

enum {
    HSB_CLOCK_IN_RC = 0,
    HSB_CLOCK_IN_PAT,
    HSB_CLOCK_IN_STD_24M,
    HSB_CLOCK_IN_STD_48M,
    HSB_CLOCK_IN_HSB_PLL,
};

#define HSB_CLOCK_SEL(x) 		SFR(JL_CLOCK->HSB_SEL,  0,  3,  x); \
asm("csync")

enum {
    LSB_CLOCK_IN_RC = 0,
    LSB_CLOCK_IN_PAT,
    LSB_CLOCK_IN_STD_24M,
    LSB_CLOCK_IN_STD_48M,
    LSB_CLOCK_IN_LSB_PLL,
    LSB_CLOCK_IN_PLL_96M,
    LSB_CLOCK_IN_HSB_CLK,
    LSB_CLOCK_IN_NEGATER_PAT,
};

#define LSB_CLOCK_SEL(x) 		SFR(JL_CLOCK->LSB_SEL,  0,  3,  x); \
asm("csync")

#define SFR_MODE(x)             //SFR(JL_CLOCK->CLK_CON2,  8,  1,  x)
enum {
    SFR_CLOCK_IDLE = 0,
    SFR_CLOCK_ALWAYS_ON,
};


#define PLL_48M_SEL(x)          SFR(JL_CLOCK->STD_CON0, 6, 1,  x)
enum {
    PLL_48M_SEL_DIV2 = 0,
    PLL_48M_SEL_DIV1,
};

#define STD_48M_SEL(x)          SFR(JL_CLOCK->STD_CON0, 7, 1,  x)
enum {
    PLL_48M_TO_STD_48M = 0,
    BTOSCX2_TO_STD_48M,
};

#define STD_24M_SEL(x)          SFR(JL_CLOCK->STD_CON0, 8, 1,  x)
enum {
    STD_48M_DIV2_TO_STD_24M = 0,
    BTOSC_TO_STD_24M,
};

#define PLL_48M_SEL_GET()       ((JL_CLOCK->STD_CON0 & BIT(6)) >> 6)

#define PLL_96M_SEL(x)          SFR(JL_CLOCK->STD_CON0, 2, 3,  x)
enum {
    PLL_96M_SEL_NULL = 0,
    PLL_96M_SEL_3DIV2, 	//f2
    PLL_96M_SEL_4DIV2,	//f3
    PLL_96M_SEL_5DIV2,	//f4

    PLL_96M_SEL_END
};

// TODO
#define PLL_96M_SEL_GET()      ((JL_CLOCK->STD_CON0 >> 2) & 0x7)

#define PLL_96M_DS(x)           SFR(JL_CLOCK->STD_CON0, 5, 1,  x)

#define USB_CLOCK_IN(x)         //SFR(JL_CLOCK->CLK_CON5, 0, 2,  x)

#define PLL_ALNK0_SEL(x)       SFR(JL_CLOCK->PRP_CON1, 9, 4,  x)
#define PLL_ALNK0_DIV(x)       SFR(JL_CLOCK->PRP_CON1, 13, 6,  x)

#define PLL_ALNK1_SEL(x)        //SFR(JL_CLOCK->CLK_CON3, 10, 4,  x)
#define PLL_ALNK1_DIV(x)       // SFR(JL_CLOCK->CLK_CON3, 14, 6,  x)

#define PLL_TDM_SEL(x)       // SFR(JL_CLOCK->CLK_CON3, 20, 4,  x)
#define PLL_TDM_DIV(x)        //SFR(JL_CLOCK->CLK_CON3, 24, 6,  x)

//for MACRO - USB_CLOCK_IN
enum {
    USB_CLOCK_IN_DISABLE = 0,
    USB_CLOCK_IN_STD48M,
    USB_CLOCK_IN_SSC48M,
    USB_CLOCK_IN_PAT,
};
#define AUDIO_CLOCK_IN(x)       //SFR(JL_CLOCK->CLK_CON1,  2,  2,  x)
//for MACRO - AUDIO_CLOCK_IN
enum {
    AUDIO_CLOCK_IN_PLL48M = 0,
    AUDIO_CLOCK_IN_OSC,
    AUDIO_CLOCK_IN_LSB,
    AUDIO_CLOCK_IN_DISABLE,
};
#define GPCNT_CLOCK_IN(x)         //SFR(JL_CLOCK->CLK_CON2,  28,  4,  x)
//for MACRO - DAC_CLOCK_IN
enum {
    //clkrst_mng
    GPCNT_CLOCK_IN_NULL = 0,
    GPCNT_CLOCK_IN_USB_PLL_1DIV1,
    GPCNT_CLOCK_IN_USB_PLL_FM_FBCK,
    GPCNT_CLOCK_IN_SYS_PLL_1DIV1,
    GPCNT_CLOCK_IN_SYS_PLL_FM_FBCK,
    GPCNT_CLOCK_IN_RC_16M,
    GPCNT_CLOCK_IN_P33_RC_250k,
    GPCNT_CLOCK_IN_DPLL_RTC_OSC,
    GPCNT_CLOCK_IN_LRC_CLK,
    GPCNT_CLOCK_IN_PAT_CLK,
    GPCNT_CLOCK_IN_XOSC_FSCK = 10,
    GPCNT_CLOCK_IN_BTOSC_24M,
    GPCNT_CLOCK_IN_BTOSC_48M,
    GPCNT_CLOCK_IN_P33_DGB_OUT,
    GPCNT_CLOCK_IN_DCDC_DCLK,
    GPCNT_CLOCK_IN_FMTX_IO_OUT,
    GPCNT_CLOCK_IN_SRC_CLK,
    GPCNT_CLOCK_IN_HSB_CLK,
    GPCNT_CLOCK_IN_LSB_CLK,
    GPCNT_CLOCK_IN_CMAC_CLK,
    GPCNT_CLOCK_IN_RESERVED0 = 20,
    GPCNT_CLOCK_IN_RESERVED1,
    GPCNT_CLOCK_IN_RESERVED2,
    GPCNT_CLOCK_IN_PLL96M,
    GPCNT_CLOCK_IN_PLL48M,
    GPCNT_CLOCK_IN_PLL24M,
    GPCNT_CLOCK_IN_RESERVED3,
    GPCNT_CLOCK_IN_RESERVED4,
    //lsb_top
    GPCNT_CLOCK_IN_ICH_IRFLT,
    GPCNT_CLOCK_IN_RING_CLK,
    GPCNT_CLOCK_IN_ICH_CLK = 30,
    GPCNT_CLOCK_IN_CAP_MUX_OUT,
};

#define PSRAM_CLK_SEL(x) 	//SFR(JL_CLOCK->CLK_CON4, 0, 4, x)
//for MACRO - PSRAM_CLOCK_IN
enum PSRAM_CLK_SEL_TABLE {
    PSRAM_CLK_SEL_NONE = 0,
    PSRAM_CLK_SEL_HSB,
    PSRAM_CLK_SEL_USB_PLL_D1P5, //FPGA: 320MHz, CHIP: 320MHz
    PSRAM_CLK_SEL_USB_PLL_D2P0, //FPGA: 240MHz, CHIP: 240MHz
    PSRAM_CLK_SEL_USB_PLL_D2P5, //FPGA: 192MHz, CHIP: 192MHz
    PSRAM_CLK_SEL_USB_PLL_D3P5, //FPGA: 137.14MHz, CHIP: 137.14MHz
    PSRAM_CLK_SEL_SYS_PLL_D1P0, //FPGA: 160MHz, CHIP: 192MHz
    PSRAM_CLK_SEL_SYS_PLL_D1P5, //FPGA: 106.67MHz, CHIP: 128MHz
};

#define PSRAM_CLK_DIV(x) 	//SFR(JL_CLOCK->CLK_CON4, 4, 4, x)

#define UART_CLOCK_IN(x)       // SFR(JL_CLOCK->CLK_CON5,  3,  2,  x)
//for MACRO - UART_CLOCK_IN
enum {
    UART_CLOCK_IN_DISABLE = 0,
    UART_CLOCK_IN_STD48M,
    UART_CLOCK_IN_BTOSC_24M,
    UART_CLOCK_IN_EXT,
    UART_CLOCK_IN_LSB,
};
#define BT_CLOCK_IN(x)          //SFR(JL_CLOCK->CLK_CON4,  16,  2,  x)
//for MACRO - BT_CLOCK_IN
enum {
    BT_CLOCK_IN_DISABLE = 0,
    BT_CLOCK_IN_STD_48M,
    BT_CLOCK_IN_SSC_48M,
    BT_CLOCK_IN_PAT_CLK,
};

#define SFC_SCKE(x)             //SFR(JL_CLOCK->CLK_CON1,  17,  1,  x)

#define WL2ADC_CLOCK_IN(x)     // SFR(JL_CLOCK->CLK_CON4,  18,  2,  x)
//for MACRO - WL2ADC_CLOCK_IN
enum {
    WL2ADC_CLOCK_IN_DISABLE = 0,
    WL2ADC_CLOCK_IN_STD_48M,
    WL2ADC_CLOCK_IN_PLL96M,
    WL2ADC_CLOCK_IN_PAT_CLK,
};

#define WL2DAC_CLOCK_IN(x)     // SFR(JL_CLOCK->CLK_CON4,  20,  2,  x)
//for MACRO - WL2DAC_CLOCK_IN
enum {
    WL2DAC_CLOCK_IN_DISABLE = 0,
    WL2DAC_CLOCK_IN_STD_48M,
    WL2DAC_CLOCK_IN_PLL96M,
    WL2DAC_CLOCK_IN_PAT_CLK,
};


#define SFC_CLOCK_DELAY(x)      //SFR(JL_CLOCK->CLK_CON1,  28,  2,  x)


#define PLL_CLK_EN(x)         	//SFR(JL_PLL0->CON1, 31, 1,  x)

// TODO
#define PLL_HSB_SEL(x)          SFR(JL_CLOCK->SYS_CON0,  0,  4,  x)
#define PLL_HSB_SEL_GET()       ((JL_CLOCK->SYS_CON0 & 0xf))

//for MACRO - PLL_SYS_SEL
enum {
    PLL_SYS_SEL_NULL = 0,
    PLL_SYS_SEL_1DIV1, //f1
    PLL_SYS_SEL_3DIV2, //f2
    PLL_SYS_SEL_4DIV2, //f3
    PLL_SYS_SEL_5DIV2, //f4
    PLL_SYS_SEL_7DIV2, //f5

    PLL_SYS_SEL_END
};

//TODO
#define PLL_HSB_DIV(x)          SFR(JL_CLOCK->SYS_CON0,  4,  4,  x)
//for MACRO - PLL_SYS_DIV
enum {
    PLL_SYS_DIV1 = 0,
    PLL_SYS_DIV3,
    PLL_SYS_DIV5,
    PLL_SYS_DIV7,

    PLL_SYS_DIV1X2 = 4,
    PLL_SYS_DIV3X2,
    PLL_SYS_DIV5X2,
    PLL_SYS_DIV7X2,

    PLL_SYS_DIV1X4 = 8,
    PLL_SYS_DIV3X4,
    PLL_SYS_DIV5X4,
    PLL_SYS_DIV7X4,

    PLL_SYS_DIV1X8 = 12,
    PLL_SYS_DIV3X8,
    PLL_SYS_DIV5X8,
    PLL_SYS_DIV7X8,
};

// TODO
#define PLL_LSB_SEL(x)          SFR(JL_CLOCK->SYS_CON1,  0,  4,  x)
#define PLL_LSB_SEL_GET()       ((JL_CLOCK->SYS_CON1 & 0xF))

//for MACRO - PLL_SYS_SEL
enum {
    PLL_LSB_SEL_NULL = 0,
    PLL_LSB_SEL_1DIV1, //f1
    PLL_LSB_SEL_3DIV2, //f2
    PLL_LSB_SEL_4DIV2, //f3
    PLL_LSB_SEL_5DIV2, //f4
    PLL_LSB_SEL_7DIV2, //f5

    PLL_LSB_SEL_END
};

#define PLL_LSB_DIV(x)          SFR(JL_CLOCK->SYS_CON1,  4,  4,  x)

#define PLL_ALNK_EN(x)         //SFR(JL_CLOCK->CLK_CON2,  6,  1,  x)
#define PLL_ALNK_SEL(x)        //SFR(JL_CLOCK->CLK_CON2,  7,  1,  x)
//for MACRO - PLL_ALNK_SEL
enum {
    PLL_ALNK_192M_DIV17 = 0,
    PLL_ALNK_480M_DIV39,
};

#define PLL_FM_SEL(x)	        //SFR(JL_CLOCK->CLK_CON2,  12,  2,  x)
//for MACRO - PLL_APC_SEL
enum {
    PLL_APC_SEL_PLL192M = 0,
    PLL_APC_SEL_PLL137M,
    PLL_APC_SEL_PLL107M,
    PLL_APC_SEL_DISABLE,
};
#define PLL_FM_DIV(x)	        //SFR(JL_CLOCK->CLK_CON2,  14,  4,  x)
//for MACRO - PLL_APC_DIV
enum {
    PLL_FM_DIV1 = 0,
    PLL_FM_DIV3,
    PLL_FM_DIV5,
    PLL_FM_DIV7,

    PLL_FM_DIV1X2 = 4,
    PLL_FM_DIV3X2,
    PLL_FM_DIV5X2,
    PLL_FM_DIV7X2,

    PLL_FM_DIV1X4 = 8,
    PLL_FM_DIV3X4,
    PLL_FM_DIV5X4,
    PLL_FM_DIV7X4,

    PLL_FM_DIV1X8 = 12,
    PLL_FM_DIV3X8,
    PLL_FM_DIV5X8,
    PLL_FM_DIV7X8,
};

#define DPLL_UDEN(x)           // SFR(JL_CLOCK->CLK_CON2,  30,  1,  x)

// #define DSP_RESET(x)            SFR(JL_CLOCK->CLK_CON3,  0,  1,  x)

// #define DSP_POWER_RESET(x)      SFR(JL_CLOCK->CLK_CON3,  1,  1,  x)

// TODO
#define PLL_EN(x)         		//SFR(JL_SYSPLL->CON0,  0,  1,  x)
#define PLL_REST(x)             //SFR(JL_SYSPLL->CON0,  1,  1,  x)
#define PLL_REF_SEL(x)        	//SFR(JL_SYSPLL->CON1,  9,  1,  x)
//for MACRO - PLL_REF_SEL
typedef enum {
    PLL_REF_SEL_LRC_200K = 0x0,
    PLL_REF_SEL_XOSC,
    PLL_REF_SEL_EXT_CLK,
    PLL_REF_SEL_PAT_CLK,
    PLL_REF_SEL_XOSC_DIFF,
} PLL_REF_SEL;

typedef enum {
    PLL_VCO_SEL_192M,
    PLL_VCO_SEL_240M,
    PLL_VCO_SEL_288M,
} PLL_VCO_SEL;

// TODO
#define SYS_PLL_CKDSM_CORE(x)   //SFR(JL_SYSPLL->CON3,  9,  1,  x)

#define PLL_TEST(x)         	//SFR(JL_SYSPLL->CON0,  10, 1,  x)

#define PLL_DSMS(x)         	//SFR(JL_PLL0->CON0,  11, 1,  x)
#define PLL_TSEL(x)         	//SFR(JL_PLL0->CON0,  12, 4,  x)
#define PLL_RSEL(x)         	//SFR(JL_PLL0->CON0,  16, 2,  x)
#define PLL_MSEL(x)         	//SFR(JL_PLL0->CON0,  18, 2,  x)

#define PLL_DIVS(x)             //SFR(JL_PLL0->CON0,  20, 2,  x)
#define PLL_PFD(x)              //SFR(JL_PLL0->CON0,  22, 2,  x)
#define PLL_ICP(x)              //SFR(JL_PLL0->CON0,  24, 3,  x)
#define PLL_LPFR2(x)            //SFR(JL_PLL0->CON0,  27, 3,  x)


#define PLL_REF_SEL1(x)        	//SFR(JL_PLL0->CON0,  30, 2,  x)
//for MACRO - PLL_RSEL
enum {
    PLL_RSEL_RCLK = 0, 	//
    PLL_RSEL_RCH,
    PLL_RSEL_DPLL_CLK,
    PLL_RSEL_PAT_CLK,
};

#define PLL_IVCO(x)             //SFR(JL_PLL0->CON1, 12, 3,  x)
#define PLL_LDO_BYPASS(x)       //SFR(JL_PLL0->CON1, 15, 1,  x)
#define PLL_TSSEL(x)            //SFR(JL_PLL0->CON1, 16, 2,  x)
#define PLL_TSOE(x)             //SFR(JL_PLL0->CON1, 18, 1,  x)

#define PLL_DIVn(x)          	//SFR(JL_SYSPLL->CON1,  0, 7,  x) //PLL_REFDS
#define PLL_LDO12A(x)           //SFR(JL_PLL0->CON1, 20, 2,  x)
#define PLL_LDO12D(x)           //SFR(JL_PLL0->CON1, 23, 3,  x)
#define PLL_DIVn_EN(x)         	//SFR(JL_PLL0->CON1, 10, 2,  x)
//for MACRO - PLL_DIVn_EN
enum {
    PLL_DIVn_EN_X2 = 0,
    PLL_DIVn_DIS_DIV1,
    PLL_DIVn_EN2_33,
};

#define PLL_CLK_1DIV1_OE(x)       SFR(JL_PLL0->CON2, 11, 1,  x) //f1
#define PLL_CLK_3DIV2_OE(x)       SFR(JL_PLL0->CON2, 12, 1,  x) //f2
#define PLL_CLK_4DIV2_OE(x)       SFR(JL_PLL0->CON2, 13, 1,  x) //f3
#define PLL_CLK_5DIV2_OE(x)       SFR(JL_PLL0->CON2, 14, 1,  x) //f4
#define PLL_CLK_7DIV2_OE(x)       SFR(JL_PLL0->CON2, 15, 1,  x) //f5

#define PLL_DAC_OE(x)           //SFR(JL_PLL0->CON1, 30, 1,  x)

#define PLL_FBDS(x)             //SFR(JL_SYSPLL->CON2, 0,  12,  x) //PLL_DS

// TODO
#define HSB_CLK_DIV(x)			SFR(JL_CLOCK->HSB_DIV,  0,  8,  x)
#define LSB_CLK_DIV(x)			//SFR(JL_CLOCK->SYS_DIV,  8,  4,  x)
#define SFC_CLK_DIV(x)			//SFR(JL_CLOCK->SYS_DIV,  12, 3,  x)

/********************************************************************************/
#define GPCNT_EN(x)             SFR(JL_GPCNT->CON,  0,  1,  x)
#define GPCNT_CSS(x)            SFR(JL_GPCNT->CON,  1,  5,  x)
//for MACRO - GPCNT_CSS
enum {
    GPCNT_CSS_LSB = 0,
    GPCNT_CSS_OSC,
    GPCNT_CSS_INPUT_CH2,
    GPCNT_CSS_INPUT_CH3,
    GPCNT_CSS_CLOCK_IN,
    GPCNT_CSS_RING,
    GPCNT_CSS_PLL,
    GPCNT_CSS_INTPUT_CH1,
};

#define GPCNT_CLR_PEND(x)       SFR(JL_GPCNT->CON,  30,  1,  x)
#define GPCNT_GTS(x)            SFR(JL_GPCNT->CON,  16,  4,  x)

#define GPCNT_GSS(x)            SFR(JL_GPCNT->CON,  8, 5,  x)
//for MACRO - GPCNT_CSS
enum {
    GPCNT_GSS_LSB = 0,
    GPCNT_GSS_OSC,
    GPCNT_GSS_INPUT_CH14,    //iomap con1[27:24]
    GPCNT_GSS_INPUT_CH15,    //iomap con1[31:28]
    GPCNT_GSS_CLOCK_IN,		 //CLK_CON2[31:28]
    GPCNT_GSS_RING,
    GPCNT_GSS_PLL,
    GPCNT_GSS_INPUT_CH13,   //iomap con1[23:20]
};

#endif

#endif
#endif
