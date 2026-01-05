#ifndef _CPU_CLOCK_
#define _CPU_CLOCK_

#include "typedef.h"

#include "clock_hw.h"
#include "asm/clock_define.h"

#if 0
///原生时钟源作系统时钟源
#define         SYS_CLOCK_INPUT_RC  0
#define         SYS_CLOCK_INPUT_BT_OSC  1          //BTOSC 双脚(12-26M)
#define         SYS_CLOCK_INPUT_RTOSCH  2
#define         SYS_CLOCK_INPUT_RTOSCL  3
#define         SYS_CLOCK_INPUT_PAT     4

///衍生时钟源作系统时钟源
#define         SYS_CLOCK_INPUT_PLL_BT_OSC  5
#define         SYS_CLOCK_INPUT_PLL_RTOSCH  6
#define         SYS_CLOCK_INPUT_PLL_PAT     7
#define         SYS_CLOCK_INPUT_PLL_RCL     8
#endif

typedef int SYS_CLOCK_INPUT;
// typedef enum {
// /原生时钟源作系统时钟源
// SYS_CLOCK_INPUT_RC,
// SYS_CLOCK_INPUT_BT_OSC,          BTOSC 双脚(12-26M)
// SYS_CLOCK_INPUT_RTOSCH,
// SYS_CLOCK_INPUT_RTOSCL,
// SYS_CLOCK_INPUT_PAT,

// /衍生时钟源作系统时钟源
// SYS_CLOCK_INPUT_PLL_BT_OSC,
// SYS_CLOCK_INPUT_PLL_RTOSCH,
// SYS_CLOCK_INPUT_PLL_PAT,
// } SYS_CLOCK_INPUT;

typedef enum {
    SYS_ICLOCK_INPUT_BTOSC,          //BTOSC 双脚(12-26M)
    SYS_ICLOCK_INPUT_RTOSCH,
    SYS_ICLOCK_INPUT_RTOSCL,
    SYS_ICLOCK_INPUT_PAT,
} SYS_ICLOCK_INPUT;

typedef enum {
    ALINK_CLOCK_12M288K,  //160M div 13, 48k采样率类型
    ALINK_CLOCK_11M2896K, //192M div 17, 44.1k采样率类型
} ALINK_INPUT_CLK_TYPE;

typedef enum {
    TDM_CLOCK_12M288K,  //160M div 13, 48k采样率类型
    TDM_CLOCK_11M2896K, //192M div 17, 44.1k采样率类型
} TDM_INPUT_CLK_TYPE;


/*
 * system enter critical and exit critical handle
 * */
struct clock_critical_handler {
    void (*enter)();
    void (*exit)();
};

#define CLOCK_CRITICAL_HANDLE_REG(name, enter, exit) \
	const struct clock_critical_handler clock_##name \
		 SEC_USED(.clock_critical_txt) = {enter, exit};

extern struct clock_critical_handler clock_critical_handler_begin[];
extern struct clock_critical_handler clock_critical_handler_end[];

#define list_for_each_loop_clock_critical(h) \
	for (h=clock_critical_handler_begin; h<clock_critical_handler_end; h++)

void clock_set_pll_target_frequency(u32 freq);

int clk_early_init(u8 sys_in, u32 input_freq, u32 out_freq);

int clk_get(const char *name);

int clk_set(const char *name, int clk);

int clk_set_sys_lock(int clk, int lock_en);

enum CLK_OUT_SOURCE {
    NONE_CLK_OUT,
    RTC_OSC_CLK_OUT,
    LRC_CLK_OUT,
    STD_12M_CLK_OUT,
    STD_24M_CLK_OUT,
    STD_48M_CLK_OUT,
    BTOSC_24M_CLK_OUT,
    BTOSC_48M_CLK_OUT,
    HSB_CLK_OUT,
    LSB_CLK_OUT,
    PLL_96M_CLK_OUT,
    RC_250K_CLK_OUT,
    RC_16M_CLK_OUT,
    XOSC_FSCK_CLK_OUT,
    ALNK0_CLK_OUT,
    RF_CKO75M_CLK_OUT,
    USB_CLK_OUT,
};

enum CLK_OUT_SOURCE_2 {
    NONE_CLK_OUT_2,
    RTC_OSC_CLK_OUT_2,
    LRC_CLK_OUT_2,
    BTOSC_24M_CLK_OUT_2,
    BTOSC_48M_CLK_OUT_2,
    SYS_PLL_D3P5_CLK_OUT_2,
    SYS_PLL_D2P5_CLK_OUT_2,
    SYS_PLL_D2P0_CLK_OUT_2,
    SYS_PLL_D1P5_CLK_OUT_2,
    SYS_PLL_D1P0_CLK_OUT_2,
};

void clk_out(u8 gpio, enum CLK_OUT_SOURCE clk);
void clk_out1(u8 gpio, enum CLK_OUT_SOURCE clk);
void clk_out2(u8 gpio, enum CLK_OUT_SOURCE_2 clk, u8 div);

void clock_dump(void);

#define MHz_UNIT    1000000L
#define KHz_UNIT    1000L
#define MHz	(1000000L)
enum sys_clk {
    SYS_6M  = 6 * MHz,
    SYS_8M  = 8 * MHz,
    SYS_12M = 12 * MHz,
    SYS_16M = 16 * MHz,
    SYS_24M = 24 * MHz,
    SYS_32M = 32 * MHz,
    SYS_48M = 48 * MHz,
    SYS_64M = 64 * MHz,
    SYS_76M = 76800000,
    SYS_96M = 96 * MHz,
};

enum clk_mode {
    CLOCK_MODE_ADAPTIVE = 0,
    CLOCK_MODE_USR,
};

//clk : SYS_48M / SYS_24M
void sys_clk_set(enum sys_clk clk);

void clk_voltage_init(u8 mode, u8 sys_dvdd);


void clk_set_osc_cap(u8 sel_l, u8 sel_r);

u32 clk_get_osc_cap();

void audio_link_clock_sel(ALINK_INPUT_CLK_TYPE type);

void tdm_clock_sel(TDM_INPUT_CLK_TYPE type);

/**
 * @brief clock_set_sfc_max_freq
 * 使用前需要保证所使用的flash支持4bit 100Mhz 模式
 *
 * @param dual_max_freq for cmd 3BH BBH
 * @param quad_max_freq for cmd 6BH EBH
 */
void clock_set_sfc_max_freq(u32 dual_max_freq, u32 quad_max_freq);

void psram_clk_init(void);
void udelay(u32 us);
void mdelay(u32 ms);

enum {
    USB_TRIM_HAND,  //手动校准模式
    USB_TRIM_AUTO,  //full_speed自动校准模式
};

#define FUSB_TRIM_CON0      JL_PLL0->TRIM_CON0
#define FUSB_TRIM_CON1      JL_PLL0->TRIM_CON1
#define FUSB_TRIM_PND       JL_PLL0->TRIM_PND
#define FUSB_FRQ_CNT        JL_PLL0->FRQ_CNT
#define FUSB_FRC_SCA        JL_PLL0->FRC_SCA
#define FUSB_PLL_CON0       JL_PLL0->CON0
#define FUSB_PLL_CON1       JL_PLL0->CON1
#define FUSB_PLL_NR         JL_PLL0->NR

u8 fusb_pll_trim(u8 mode, u16 trim_prd);

#endif

